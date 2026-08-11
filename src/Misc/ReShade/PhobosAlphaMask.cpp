#include "PhobosAlphaMask.h"
#include <Ext/Convert/Body.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include "Runtime/runtime.hpp"
#include "Runtime/reshade_api_device.hpp"

#include <Ext/AnimType/Body.h>
#include <Utilities/Macro.h>

#include <cstring>

// ===========================================================================
// Lifetime
// ===========================================================================

PhobosAlphaMask& PhobosAlphaMask::Instance()
{
	static PhobosAlphaMask instance;
	return instance;
}

bool PhobosAlphaMask::OnInit(IDirect3DDevice9* pDevice, reshade::api::device* pReShadeDevice, int width, int height)
{
	OnReset();

	if (pDevice == nullptr || pReShadeDevice == nullptr || width <= 0 || height <= 0)
		return false;

	_device = pDevice;
	_reshade_device = pReShadeDevice;
	_width = width;
	_height = height;

	// DIFF: original used D3DPOOL_MANAGED (runtime +0x750). MANAGED is invalid
	// on D3D9Ex and wrong for a texture rewritten every frame; DEFAULT+DYNAMIC
	// is the documented per-frame-update path and is lockable directly, which
	// is why there is no staging surface member here.
	HRESULT hr = _device->CreateTexture(
		static_cast<UINT>(_width), static_cast<UINT>(_height),
		1,                          // no mips
		D3DUSAGE_DYNAMIC,
		D3DFMT_L8,
		D3DPOOL_DEFAULT,
		&_texture,
		nullptr);

	if (FAILED(hr))
	{
		// VERIFY: L8 is effectively universal, but a driver may still refuse it.
		// A8 is the usual fallback and samples in .a instead of .r - if you add
		// that path, the effect side has to know which it got.
		Debug::Log("[PhobosAlphaMask] CreateTexture(L8 %dx%d) failed: 0x%08X\n", _width, _height, hr);
		OnReset();
		return false;
	}

	// Ask the ReShade device for the view rather than hand-encoding a handle.
	// d3d9's resource_view handle encoding is an implementation detail (it
	// packs an sRGB bit) and is not stable across revisions.
	if (!_reshade_device->create_resource_view(
		reshade::api::resource { reinterpret_cast<uintptr_t>(_texture.get()) },
		reshade::api::resource_usage::shader_resource,
		reshade::api::resource_view_desc(reshade::api::format::r8_unorm),
		&_srv))
	{
		Debug::Log("[PhobosAlphaMask] create_resource_view failed\n");
		OnReset();
		return false;
	}

	// BUGFIX: the original never checked its buffer pointer and never verified
	// it was still width*height bytes before memset'ing it. Owning the storage
	// makes both impossible.
	const size_t bytes = static_cast<size_t>(_width) * static_cast<size_t>(_height);
	_draw.assign(bytes, 0);
	_upload.assign(bytes, 0);

	return true;
}

void PhobosAlphaMask::OnReset()
{
	// If a pass is somehow still open, close it before tearing anything down.
	if (_pass_active)
	{
		_pass_active = false;
		_active = nullptr;
		_mutex.unlock();
	}

	if (_srv != 0 && _reshade_device != nullptr)
		_reshade_device->destroy_resource_view(_srv);

	_srv = {};
	_texture.reset();
	_reshade_device = nullptr;
	_device = nullptr;

	_draw.clear();
	_upload.clear();
	_draw.shrink_to_fit();
	_upload.shrink_to_fit();

	_width = 0;
	_height = 0;
	_upload_pending = false;
}

// ===========================================================================
// Draw pass
// ===========================================================================

void PhobosAlphaMask::BeginPass()
{
	if (_texture == nullptr)
		return;

	_mutex.lock();

	// DIFF/BUGFIX: the original read the surface width and height BEFORE taking
	// the lock (1002852A / 1002852E precede the _Mtx_lock call), so a resize
	// racing the pass sized the clear from stale dimensions. Both the clear and
	// the stride now come from members read under the lock.
	std::memset(_draw.data(), 0, _draw.size());

	_active = _draw.data();
	_pass_active = true;
}

void PhobosAlphaMask::EndPass()
{
	// DIFF/BUGFIX: unconditional pairing, latched at BeginPass. See header.
	if (!_pass_active)
		return;

	// Hand the finished frame to the uploader without copying: the buffer the
	// game just filled becomes _upload, and last frame's _upload becomes the
	// next draw target. Nothing is held across the D3D call this way.
	_draw.swap(_upload);

	_active = nullptr;
	_pass_active = false;
	_upload_pending = true;

	_mutex.unlock();
}

bool PhobosAlphaMask::Blit(SHPCaches* pShape, int frame, DWORD flags,
	const RectangleStruct& bounds, int x, int y)
{
	if (_active == nullptr || pShape == nullptr)
		return false;

	// The mask is one byte per pixel, so the width is also the stride.
	const int destStride = _width;

	// 100287CD -- cmovz. Only shapes whose first word is 0xFFFF are accepted.
	// VERIFY: 0xFFFF marks the extended header this DLL installs; a vanilla SHP
	// has 0 there.
	//
	// WARNING: SHPCaches::IsReference() is the exact function that got erased by
	// WPO once before, when its backing field was declared `short` instead of
	// `unsigned short` and the comparison folded to a compile-time false. If
	// this blit ever silently stops producing output, disassemble IsReference
	// and check for a 31 C0 C3 stub before looking anywhere else.
	SHPHeader* const pSource = pShape->IsReference() ? reinterpret_cast<SHPHeader*>(pShape) : nullptr;

	if (pSource == nullptr)
		return false;

	RectangleStruct frameBounds {};
	pShape->GetFrameBounds(frameBounds, frame);

	// 100287ED / 1002880C -- `and ebx, 200h`. IDA types the parameter __int16,
	// which is wrong; the instruction reads the full dword. The cdq/sub/sar 1
	// sequence is signed division truncating toward zero.
	//
	// NOTE: the shadow hooks force DrawFlags::Center in, so a shadow draw always
	// takes this branch regardless of what the caller passed.
	const bool centered = DrawFlags::IsCentered(flags);
	const int centerX = centered ? pSource->Width / 2 : 0;
	const int centerY = centered ? pSource->Height / 2 : 0;

	frameBounds.X += x - centerX;
	frameBounds.Y += y - centerY;

	const RectangleStruct clipped = RectangleStruct::Intersect(frameBounds, bounds, nullptr, nullptr);

	// 10028867 / 10028875 -- plain zero tests, NOT <= 0. A negative width or
	// height sails through here and is only caught by the `rows <= 0` guards in
	// the two blitters. Preserved verbatim.
	if (clipped.Width == 0 || clipped.Height == 0)
		return false;

	const int srcSkipRows = clipped.Y - frameBounds.Y;
	const int srcSkipCols = clipped.X - frameBounds.X;
	const int destX = clipped.X - bounds.X;
	const int destY = clipped.Y - bounds.Y;

	// DIFF/BUGFIX: the original computed pDest from the caller's bounds without
	// ever checking it against the mask buffer. `bounds` comes from the engine
	// and is normally the tactical rect, but a caller passing anything larger
	// wrote outside the allocation. Clamp here; the blitters clamp per row.
	if (destX < 0 || destY < 0 || destX >= _width || destY >= _height)
		return false;

	const int maxWidth = _width - destX;
	const int maxHeight = _height - destY;
	const int blitWidth = clipped.Width > maxWidth ? maxWidth : clipped.Width;
	const int blitHeight = clipped.Height > maxHeight ? maxHeight : clipped.Height;

	// 1002889E -- order preserved: compression is queried BEFORE the frame
	// pointer is null-checked.
	const uint8_t* const pFrame = pShape->GetPixels(frame);
	const bool isRLE = pShape->HasCompression(frame);

	if (pFrame == nullptr)
		return false;

	uint8_t* const pDest = _active + static_cast<size_t>(destStride) * destY + destX;
	const uint8_t* const pBufferEnd = _active + _draw.size();

	return isRLE
		? BlitRLE(pDest, destStride, pFrame, srcSkipCols, srcSkipRows,
			blitWidth, blitHeight, pBufferEnd)
		: BlitRaw(pDest, destStride, pFrame, frameBounds.Width, srcSkipCols,
			srcSkipRows, blitWidth, blitHeight);
}

bool PhobosAlphaMask::BlitRaw(uint8_t* pDest, int destStride, const uint8_t* pFrame,
	int srcStride, int srcSkipCols, int srcSkipRows, int width, int rows)
{
	const uint8_t* pSrc = pFrame + static_cast<size_t>(srcStride) * srcSkipRows + srcSkipCols;

	if (rows <= 0 || width <= 0)
		return true;

	const auto span = static_cast<rsize_t>(width);

	do
	{
		// Return value discarded, matching the original: the error is reported
		// through errno and the invalid-parameter handler, not to this caller.
		static_cast<void>(memmove_s(pDest, span, pSrc, span));

		pSrc += srcStride;
		pDest += destStride;
	}
	while (--rows);

	return true;
}

bool PhobosAlphaMask::BlitRLE(uint8_t* pDest, int destStride, const uint8_t* pRow,
	int srcSkipCols, int srcSkipRows, int width, int rows,
	const uint8_t* pBufferEnd)
{
	// 100288D1 -- walk past fully clipped rows using the length headers.
	for (int skipped = srcSkipRows; skipped > 0; --skipped)
		pRow += *reinterpret_cast<const int16_t*>(pRow);

	if (rows <= 0 || width <= 0)
		return true;

	// 100288E5 -- truncated to 16 bits and reloaded from the stack slot on every
	// iteration, so the width of the type matters.
	const int16_t skipCols = static_cast<int16_t>(srcSkipCols);

	do
	{
		const uint8_t* pIn = pRow + sizeof(int16_t);
		uint8_t* pOut = pDest;
		int remaining = width;

		if (skipCols > 0)
		{
			// 10028905 -- consume runs until the clip edge is reached or passed.
			// `overshoot` ends up >= 0.
			int overshoot = -skipCols;

			do
			{
				const uint8_t code = *pIn++;
				overshoot += code ? 1 : *pIn++;
			}
			while (overshoot < 0);

			// A run straddling the clip edge is dropped whole: the output pointer
			// advances by the overshoot and the row shrinks to match, so those
			// pixels keep whatever the buffer already held.
			pOut = pDest + overshoot;
			remaining = width - overshoot;
		}

		// 10028930 -- BUGFIX: the original bounds-checked nothing here. A frame
		// whose RLE payload disagrees with its header ran straight off the end of
		// the mask allocation - a heap corruption reachable from any malformed
		// SHP in any mod. The row-end and buffer-end clamps below are the only
		// behavioural deviation from the original blitter, and they only ever
		// trigger on input that would have corrupted memory.
		const uint8_t* const pRowEnd = pDest + width;

		for (int left = remaining; left > 0 && pOut < pRowEnd && pOut < pBufferEnd; )
		{
			const uint8_t code = *pIn++;
			int step;

			if (code)
			{
				*pOut = code;
				--left;
				step = 1;
			}
			else
			{
				step = *pIn++;
				left -= step;
			}

			pOut += step;
		}

		pDest += destStride;
		pRow += *reinterpret_cast<const int16_t*>(pRow);
	}
	while (--rows);

	return true;
}

// ===========================================================================
// Upload + bind
// ===========================================================================

void PhobosAlphaMask::UploadAndBind(reshade::runtime* pRuntime)
{
	if (_texture == nullptr || pRuntime == nullptr)
		return;

	if (_upload_pending)
	{
		D3DLOCKED_RECT locked = {};

		// D3DLOCK_DISCARD on a DYNAMIC texture hands back a fresh buffer instead
		// of stalling on the previous frame's sample.
		if (SUCCEEDED(_texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD)))
		{
			const uint8_t* pSrc = _upload.data();
			auto* pDst = static_cast<uint8_t*>(locked.pBits);

			// Never assume Pitch == width. Drivers pad L8 rows aggressively.
			if (locked.Pitch == _width)
			{
				std::memcpy(pDst, pSrc, _upload.size());
			}
			else
			{
				for (int row = 0; row < _height; ++row)
				{
					std::memcpy(pDst, pSrc, static_cast<size_t>(_width));
					pSrc += _width;
					pDst += locked.Pitch;
				}
			}

			_texture->UnlockRect(0);
		}

		_upload_pending = false;
	}

	// Rebind every frame. update_texture_bindings is cheap when nothing changed,
	// and effects reloaded since the last call need the binding re-established.
	// This is the mechanism that replaces the original's runtime+0x750 hack.
	pRuntime->update_texture_bindings(Semantic, _srv, _srv);
}

// ===========================================================================
// Hooks - unchanged call sites, retargeted at the new owner
// ===========================================================================

ASMJIT_PATCH(0x6D8F0F, TacticalClass_UpdateDrawFunc, 6)
{
	PhobosAlphaMask::Instance().BeginPass();
	return 0;
}

ASMJIT_PATCH(0x6D97BF, TacticalClass_UpdateDrawReturn, 6)
{
	PhobosAlphaMask::Instance().EndPass();
	return 0;
}

namespace
{
	// The FXLightEnable lookup is the expensive part of each hook, so skip it
	// entirely when no pass is open.
	inline bool ShouldMask(AnimClass* pAnim)
	{
		if (!PhobosAlphaMask::Instance().IsPassActive() || pAnim == nullptr)
			return false;

		return AnimTypeExtContainer::Instance.Find(pAnim->Type)->FXLightEnable;
	}
}

// ---------------------------------------------------------------------------
// Variant 0. .text:100285A0
//
// Reads four consecutive dwords off ESP: the tail of a CC_Draw_Shape argument
// push, one slot in -- the shape itself comes from EAX rather than the stack.
// Passes the caller's flags through untouched.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4236F0, AnimClass_Draw_SetMaskBuffer, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(SHPCaches*, pShape, EAX);
	GET_STACK(int, frame, 0x0);
	GET_STACK(Point2D*, pPoint, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);
	GET_STACK(DWORD, flags, 0xC);

	if (ShouldMask(pAnim))
		PhobosAlphaMask::Instance().Blit(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y);

	return 0;
}
ASMJIT_PATCH_AGAIN(0x4233E4, AnimClass_Draw_SetMaskBuffer, 5)

// ---------------------------------------------------------------------------
// Variant 1. .text:10028610
//
// Five dwords off ESP -- the full CC_Draw_Shape argument block at the call:
// [0] shape, [1] shapenum, [2] xy, [3] rect1, [4] flags.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x423821, AnimClass_Draw_SetMaskBuffer_1, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET_STACK(SHPCaches*, pShape, 0x0);
	GET_STACK(int, frame, 0x4);
	GET_STACK(Point2D*, pPoint, 0x8);
	GET_STACK(RectangleStruct*, pBounds, 0xC);
	GET_STACK(DWORD, flags, 0x10);

	if (ShouldMask(pAnim))
		PhobosAlphaMask::Instance().Blit(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y);

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 2. .text:10028680 -- the shadow pass.
//
// ESP is Draw_It's frame base here: +0x28 is `shape`, +0x2C is `sz`, +0x118 is
// the `rect1` argument. This is what pins the REGISTERS layout.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x42383C, AnimClass_Draw_SetMaskBuffer_2, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, EDI);
	GET(DWORD, drawFlags, EBX);
	GET_STACK(SHPCaches*, pShape, 0x28);
	GET_STACK(int, frame, 0x2C);
	GET_STACK(RectangleStruct*, pBounds, 0x118);

	if (ShouldMask(pAnim))
	{
		PhobosAlphaMask::Instance().Blit(pShape, frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 3. .text:10028700 -- the extras/shadow pass, 0x20 bytes deeper into
// the same frame, so `shape` sits at ESP+0x48 instead of ESP+0x28.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4237A3, AnimClass_Draw_SetMaskBuffer_3, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(DWORD, drawFlags, EBX);
	GET(int, frame, EDX);
	GET(RectangleStruct*, pBounds, EAX);
	GET_STACK(SHPCaches*, pShape, 0x48);

	if (ShouldMask(pAnim))
	{
		PhobosAlphaMask::Instance().Blit(pShape, frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y);
	}

	return 0;
}
