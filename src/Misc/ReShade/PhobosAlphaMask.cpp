#include "PhobosAlphaMask.h"
#include <Ext/Convert/Body.h>


#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include "Runtime/runtime.hpp"
#include "Runtime/reshade_api_device.hpp"

#include <Utilities/Macro.h>

#include <algorithm>
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

	// DIFF: L8 -> L16. See the header - the reference fork's buffer is two bytes
	// per pixel and every queued record carries a 16-bit parameter.
	//
	// VERIFY: L16 is far less universally supported than L8. If CreateTexture
	// fails here on a real driver, the fallbacks in order of preference are
	// R5G6B5 (lossy on the low byte) or A8L8 (exact, samples as .a/.r). Whichever
	// you pick, the .fx unpack expression changes with it.
	HRESULT hr = _device->CreateTexture(
		static_cast<UINT>(_width), static_cast<UINT>(_height),
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_L16,
		D3DPOOL_DEFAULT,
		&_texture,
		nullptr);

	if (FAILED(hr))
	{
		Debug::Log("[PhobosAlphaMask] CreateTexture(L16 %dx%d) failed: 0x%08X\n", _width, _height, hr);
		OnReset();
		return false;
	}

	if (!_reshade_device->create_resource_view(
		reshade::api::resource { reinterpret_cast<uintptr_t>(_texture.get()) },
		reshade::api::resource_usage::shader_resource,
		reshade::api::resource_view_desc(reshade::api::format::r16_unorm),
		&_srv))
	{
		Debug::Log("[PhobosAlphaMask] create_resource_view failed\n");
		OnReset();
		return false;
	}

	const size_t pixels = static_cast<size_t>(_width) * static_cast<size_t>(_height);
	_draw.assign(pixels, 0);
	_upload.assign(pixels, 0);

	_queue.clear();
	_queue.reserve(QueueReserve);

	return true;
}

void PhobosAlphaMask::OnReset()
{
	// DIFF: no lock to unwind here any more. The immediate-blit version had to
	// unlock a mutex it might have been holding since BeginPass; the deferred
	// version never holds the lock outside EndPass.
	_pass_active = false;

	if (_srv != 0 && _reshade_device != nullptr)
		_reshade_device->destroy_resource_view(_srv);

	_srv = {};
	_texture.reset();
	_reshade_device = nullptr;
	_device = nullptr;

	_draw.clear();
	_upload.clear();
	_queue.clear();

	_draw.shrink_to_fit();
	_upload.shrink_to_fit();
	_queue.shrink_to_fit();

	_width = 0;
	_height = 0;
	_upload_pending = false;
}

// ===========================================================================
// Draw pass - record
// ===========================================================================

void PhobosAlphaMask::BeginPass()
{
	// clear() keeps capacity, so steady-state frames never touch the allocator.
	_queue.clear();
	_pass_active = _texture != nullptr;
}

bool PhobosAlphaMask::Queue(SHPCaches* pShape, int frame, DWORD flags,
	const RectangleStruct& bounds, int x, int y, uint16_t param)
{
	if (!_pass_active || pShape == nullptr)
		return false;

	// A zero parameter contributes nothing under any of the Combine variants,
	// so reject it at record time rather than paying for a full replay blit.
	if (param == 0)
		return false;

	_queue.push_back(FXEntry { pShape, frame, bounds, x, y, flags, param });
	return true;
}

// ===========================================================================
// Draw pass - replay
// ===========================================================================

void PhobosAlphaMask::EndPass()
{
	// BUGFIX (vs reference fork): the drain runs on EVERY exit path.
	//
	// The fork nested its drain inside `if (EnhancedLight)` while gating its
	// producers on a per-type extension flag instead. Turning the feature off
	// therefore left producers appending to a queue nothing ever emptied. The
	// same hazard exists here for a missing texture or an unpaired EndPass, so
	// the reset is factored out and reached unconditionally.
	const auto drain = [this]() noexcept
		{
			_queue.clear();
			_pass_active = false;
		};

	if (!_pass_active || _texture == nullptr || _draw.empty())
	{
		drain();
		return;
	}

	{
		std::scoped_lock<std::mutex> lock(_mutex);

		std::memset(_draw.data(), 0, _draw.size() * sizeof(uint16_t));

		for (const FXEntry& entry : _queue)
			BlitEntry(entry);

		// The buffer the game just filled becomes the upload source; last
		// frame's upload buffer becomes the next draw target. No copy, and the
		// lock is never held across a D3D call.
		_draw.swap(_upload);
		_upload_pending = true;
	}

	drain();
}

void PhobosAlphaMask::BlitEntry(const FXEntry& entry)
{
	SHPCaches* const pShape = entry.Shape;

	// WARNING: SHPCaches::IsReference() is the function WPO erased once before,
	// when its backing field was declared `short` rather than `unsigned short`
	// and the comparison folded to a compile-time false. If the mask silently
	// goes black, disassemble IsReference and look for a 31 C0 C3 stub before
	// investigating anything else.
	if (!pShape->IsReference())
		return;

	auto* const pSource = reinterpret_cast<SHPHeader*>(pShape);

	RectangleStruct frameBounds {};
	pShape->GetFrameBounds(frameBounds, entry.Frame);

	const bool centered = DrawFlags::IsCentered(entry.Flags);
	const int centerX = centered ? pSource->Width / 2 : 0;
	const int centerY = centered ? pSource->Height / 2 : 0;

	frameBounds.X += entry.X - centerX;
	frameBounds.Y += entry.Y - centerY;

	const RectangleStruct clipped = RectangleStruct::Intersect(frameBounds, entry.Bounds, nullptr, nullptr);

	// Zero tests rather than <= 0, preserved from the original blitter: a
	// negative extent passes here and is caught by the `rows <= 0` guards below.
	if (clipped.Width == 0 || clipped.Height == 0)
		return;

	const int srcSkipRows = clipped.Y - frameBounds.Y;
	const int srcSkipCols = clipped.X - frameBounds.X;
	const int destX = clipped.X - entry.Bounds.X;
	const int destY = clipped.Y - entry.Bounds.Y;

	if (destX < 0 || destY < 0 || destX >= _width || destY >= _height)
		return;

	const int blitWidth = std::min(clipped.Width, _width - destX);
	const int blitHeight = std::min(clipped.Height, _height - destY);

	// Order preserved: compression is queried before the frame pointer is
	// null-checked.
	const uint8_t* const pFrame = pShape->GetPixels(entry.Frame);
	const bool isRLE = pShape->HasCompression(entry.Frame);

	if (pFrame == nullptr)
		return;

	uint16_t* const pDest = _draw.data() + static_cast<size_t>(_width) * destY + destX;
	const uint16_t* const pBufferEnd = _draw.data() + _draw.size();

	if (isRLE)
	{
		BlitRLE(pDest, _width, pFrame, srcSkipCols, srcSkipRows,
			blitWidth, blitHeight, entry.Param, pBufferEnd);
	}
	else
	{
		BlitRaw(pDest, _width, pFrame, frameBounds.Width, srcSkipCols,
			srcSkipRows, blitWidth, blitHeight, entry.Param);
	}
}

void PhobosAlphaMask::BlitRaw(uint16_t* pDest, int destStride, const uint8_t* pFrame,
	int srcStride, int srcSkipCols, int srcSkipRows, int width, int rows,
	uint16_t param)
{
	if (rows <= 0 || width <= 0)
		return;

	const uint8_t* pSrc = pFrame + static_cast<size_t>(srcStride) * srcSkipRows + srcSkipCols;

	// DIFF: the L8 version used memmove_s here, which is no longer possible -
	// source is 8-bit palette indices, destination is 16-bit values, so every
	// pixel has to be expanded individually.
	//
	// BUGFIX: index 0 is SHP transparency and is now skipped. The L8 version
	// copied it wholesale, which was harmless when each frame had one writer but
	// is not once records batch and overlap: a later record's transparent pixels
	// would erase an earlier record's light. This is the one place the deferred
	// design forces a behavioural change rather than inheriting one.
	do
	{
		for (int col = 0; col < width; ++col)
		{
			const uint8_t code = pSrc[col];

			if (code != 0)
				pDest[col] = Combine(pDest[col], Sample(code, param));
		}

		pSrc += srcStride;
		pDest += destStride;
	}
	while (--rows);
}

void PhobosAlphaMask::BlitRLE(uint16_t* pDest, int destStride, const uint8_t* pRow,
	int srcSkipCols, int srcSkipRows, int width, int rows,
	uint16_t param, const uint16_t* pBufferEnd)
{
	// Walk past fully clipped rows using the per-row length headers.
	for (int skipped = srcSkipRows; skipped > 0; --skipped)
		pRow += *reinterpret_cast<const int16_t*>(pRow);

	if (rows <= 0 || width <= 0)
		return;

	// Truncated to 16 bits and reloaded per iteration in the original, so the
	// width of the type is load-bearing.
	const int16_t skipCols = static_cast<int16_t>(srcSkipCols);

	do
	{
		const uint8_t* pIn = pRow + sizeof(int16_t);
		uint16_t* pOut = pDest;
		int remaining = width;

		if (skipCols > 0)
		{
			// Consume runs until the clip edge is reached or passed.
			int overshoot = -skipCols;

			do
			{
				const uint8_t code = *pIn++;
				overshoot += code ? 1 : *pIn++;
			}
			while (overshoot < 0);

			// A run straddling the clip edge is dropped whole: the output pointer
			// advances by the overshoot and the row shrinks to match.
			pOut = pDest + overshoot;
			remaining = width - overshoot;
		}

		// The row-end and buffer-end clamps are the only deviation from the
		// original blitter, and only trigger on RLE payloads that disagree with
		// their headers - input that would otherwise run off the allocation.
		const uint16_t* const pRowEnd = pDest + width;

		for (int left = remaining; left > 0 && pOut < pRowEnd && pOut < pBufferEnd; )
		{
			const uint8_t code = *pIn++;
			int step;

			if (code)
			{
				*pOut = Combine(*pOut, Sample(code, param));
				--left;
				step = 1;
			}
			else
			{
				// Zero code introduces a transparent run: skip it, leaving
				// whatever earlier records already wrote.
				step = *pIn++;
				left -= step;
			}

			pOut += step;
		}

		pDest += destStride;
		pRow += *reinterpret_cast<const int16_t*>(pRow);
	}
	while (--rows);
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

		if (SUCCEEDED(_texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD)))
		{
			const auto* pSrc = reinterpret_cast<const uint8_t*>(_upload.data());
			auto* pDst = static_cast<uint8_t*>(locked.pBits);

			// DIFF: row length is now width * 2. Never assume Pitch matches it.
			const int rowBytes = _width * static_cast<int>(sizeof(uint16_t));

			if (locked.Pitch == rowBytes)
			{
				std::memcpy(pDst, pSrc, _upload.size() * sizeof(uint16_t));
			}
			else
			{
				for (int row = 0; row < _height; ++row)
				{
					std::memcpy(pDst, pSrc, static_cast<size_t>(rowBytes));
					pSrc += rowBytes;
					pDst += locked.Pitch;
				}
			}

			_texture->UnlockRect(0);
		}

		_upload_pending = false;
	}

	pRuntime->update_texture_bindings(Semantic, _srv, _srv);
}

// ===========================================================================
// Hooks
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
	// -----------------------------------------------------------------------
	// EXTENSION: per-frame FX filter, backported from sub_10017ED0.
	//
	// The reference fork does not emit for every frame of an enabled anim. It
	// gates on, in order:
	//
	//   ext+57  bool          enable flag
	//   ext+88  int           if non-zero, emit only while frame <= this
	//   ext+76  vector<int>   otherwise, emit only for frames in this list;
	//   ext+80                an EMPTY list means every frame
	//
	// VERIFY: the four field names below are placeholders. Substitute your
	// actual AnimTypeExtData members - I do not have the layout, only the
	// offsets the pseudocode dereferences.
	// -----------------------------------------------------------------------
	bool ShouldEmit(AnimTypeExtData* pExt, int frame)
	{
		if (!pExt->FXLightEnable)                       // ext+57
			return false;

		if (pExt->FXLightMaxFrame != 0)                 // ext+88
			return frame <= pExt->FXLightMaxFrame;

		const auto& frames = pExt->FXLightFrames;       // ext+76 / ext+80

		if (frames.empty())
			return true;

		return std::find(frames.begin(), frames.end(), frame) != frames.end();
	}

	// -----------------------------------------------------------------------
	// The 16-bit parameter, assembled exactly as the fork does it:
	//
	//     v168[15] + (v168[16] << 8)      // ext+60 | ext+64 << 8
	//
	// VERIFY: both halves are read as full dwords in the pseudocode but must be
	// 0..255 for the packing to be lossless. Clamp defensively - an INI-supplied
	// value above 255 would otherwise bleed into the high byte.
	// -----------------------------------------------------------------------
	uint16_t FXParam(AnimTypeExtData* pExt)
	{
		const auto low = static_cast<uint16_t>(std::clamp(pExt->FXLightIntensity.Get(), 0, 255));   // ext+60
		const auto high = static_cast<uint16_t>(std::clamp(pExt->FXLightSecondary.Get(), 0, 255));  // ext+64

		return static_cast<uint16_t>(low | (high << 8));
	}

	// Returns 0 when this anim should not contribute this frame.
	uint16_t MaskParamFor(AnimClass* pAnim, int frame)
	{
		if (!PhobosAlphaMask::Instance().IsPassActive() || pAnim == nullptr)
			return 0;

		AnimTypeExtData* const pExt = AnimTypeExtContainer::Instance.Find(pAnim->Type);

		return ShouldEmit(pExt, frame) ? FXParam(pExt) : 0;
	}
}

// ---------------------------------------------------------------------------
// Variant 0. Shape in EAX, four dwords off ESP.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4236F0, AnimClass_Draw_SetMaskBuffer, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(SHPCaches*, pShape, EAX);
	GET_STACK(int, frame, 0x0);
	GET_STACK(Point2D*, pPoint, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);
	GET_STACK(DWORD, flags, 0xC);

	if (const uint16_t param = MaskParamFor(pAnim, frame))
		PhobosAlphaMask::Instance().Queue(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y, param);

	return 0;
}
ASMJIT_PATCH_AGAIN(0x4233E4, AnimClass_Draw_SetMaskBuffer, 5)

// ---------------------------------------------------------------------------
// Variant 1. Full CC_Draw_Shape argument block off ESP.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x423821, AnimClass_Draw_SetMaskBuffer_1, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET_STACK(SHPCaches*, pShape, 0x0);
	GET_STACK(int, frame, 0x4);
	GET_STACK(Point2D*, pPoint, 0x8);
	GET_STACK(RectangleStruct*, pBounds, 0xC);
	GET_STACK(DWORD, flags, 0x10);

	if (const uint16_t param = MaskParamFor(pAnim, frame))
		PhobosAlphaMask::Instance().Queue(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y, param);

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 2. Shadow pass. ESP is Draw_It's frame base.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x42383C, AnimClass_Draw_SetMaskBuffer_2, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, EDI);
	GET(DWORD, drawFlags, EBX);
	GET_STACK(SHPCaches*, pShape, 0x28);
	GET_STACK(int, frame, 0x2C);
	GET_STACK(RectangleStruct*, pBounds, 0x118);

	if (const uint16_t param = MaskParamFor(pAnim, frame))
	{
		PhobosAlphaMask::Instance().Queue(pShape, frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y, param);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 3. Extras/shadow pass, 0x20 deeper into the same frame.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4237A3, AnimClass_Draw_SetMaskBuffer_3, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(DWORD, drawFlags, EBX);
	GET(int, frame, EDX);
	GET(RectangleStruct*, pBounds, EAX);
	GET_STACK(SHPCaches*, pShape, 0x48);

	if (const uint16_t param = MaskParamFor(pAnim, frame))
	{
		PhobosAlphaMask::Instance().Queue(pShape, frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y, param);
	}

	return 0;
}