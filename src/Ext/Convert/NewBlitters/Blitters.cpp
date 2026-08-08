#include "../Body.h"
#include <Ext/SHP/Body.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

#include "BlitTransLucentUniversalAlphaZRead.h"
#include "BlitTransLucentBufferedAlphaZRead.h"
#include "BlitTransLucentAddZRead.h"
#include "BlitTransLucentMultiplyZRead.h"
#include "BlitTransLucentLunaZRead.h"
#include "BlitTransLucentDoubleMultiplyZRead.h"
#include "BlitterCustom.h"

#include "RLEBlitTransLucentUniversalAlphaZRead.h"
#include "RLEBlitTransLucentBufferedAlphaZRead.h"
#include "RLEBlitTransLucentAddZRead.h"
#include "RLEBlitTransLucentMultiplyZRead.h"
#include "RLEBlitTransLucentLunaZRead.h"
#include "RLEBlitTransLucentDoubleMultiplyZRead.h"
#include "BlitterCustomRLE.h"

#include <memory>
#include <Syringe.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <FileFormats/SHP.h>

#include <ConvertClass.h>
#pragma optimize("", off )

void ConvertExtData::Alloc()
{
	WORD* palData = (WORD*)this->AttachedToObject->ShadeTables;
	int shade = this->AttachedToObject->ShadeCount;

	Blitters[0] = new BlitTransLucentUniversalAlphaZRead<WORD, 2u>(palData, shade);
	Blitters[1] = new BlitTransLucentUniversalAlphaZRead<WORD, 4u>(palData, shade);
	Blitters[2] = new BlitTransLucentUniversalAlphaZRead<WORD, 6u>(palData, shade);
	Blitters[3] = new BlitTransLucentUniversalAlphaZRead<WORD, 10u>(palData, shade);
	Blitters[4] = new BlitTransLucentUniversalAlphaZRead<WORD, 12u>(palData, shade);
	Blitters[5] = new BlitTransLucentUniversalAlphaZRead<WORD, 14u>(palData, shade);
	Blitters[6] = new BlitTransLucentUniversalAlphaZRead<WORD, 18u>(palData, shade);
	Blitters[7] = new BlitTransLucentUniversalAlphaZRead<WORD, 20u>(palData, shade);
	Blitters[8] = new BlitTransLucentUniversalAlphaZRead<WORD, 22u>(palData, shade);
	Blitters[9] = new BlitTransLucentUniversalAlphaZRead<WORD, 26u>(palData, shade);
	Blitters[10] = new BlitTransLucentUniversalAlphaZRead<WORD, 28u>(palData, shade);
	Blitters[11] = new BlitTransLucentUniversalAlphaZRead<WORD, 30u>(palData, shade);
	Blitters[12] = new BlitTransLucentBufferedAlphaZRead<WORD>(palData, shade);
	Blitters[13] = new BlitTransLucentAddZRead<WORD>(palData, shade);
	Blitters[14] = new BlitTransLucentMultiplyZRead<WORD>(palData, shade);
	Blitters[15] = new BlitTransLucentDoubleMultiplyZRead<WORD>(palData, shade);
	Blitters[16] = new BlitTransLucentLunaZRead<WORD>(palData, shade);
	Blitters[17] = new BlitterCustom(palData, shade);

	RLEBlitters[0] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 2u>(palData, shade);
	RLEBlitters[1] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 4u>(palData, shade);
	RLEBlitters[2] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 6u>(palData, shade);
	RLEBlitters[3] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 10u>(palData, shade);
	RLEBlitters[4] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 12u>(palData, shade);
	RLEBlitters[5] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 14u>(palData, shade);
	RLEBlitters[6] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 18u>(palData, shade);
	RLEBlitters[7] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 20u>(palData, shade);
	RLEBlitters[8] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 22u>(palData, shade);
	RLEBlitters[9] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 26u>(palData, shade);
	RLEBlitters[10] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 28u>(palData, shade);
	RLEBlitters[11] = new RLEBlitTransLucentUniversalAlphaZRead<WORD, 30u>(palData, shade);
	RLEBlitters[12] = new RLEBlitTransLucentBufferedAlphaZRead<WORD>(palData, shade);
	RLEBlitters[13] = new RLEBlitTransLucentAddZRead<WORD>(palData, shade);
	RLEBlitters[14] = new RLEBlitTransLucentMultiplyZRead<WORD>(palData, shade);
	RLEBlitters[15] = new RLEBlitTransLucentDoubleMultiplyZRead<WORD>(palData, shade);
	RLEBlitters[16] = new RLEBlitTransLucentLunaZRead<WORD>(palData, shade);
	RLEBlitters[17] = new BlitterCustomRLE(palData, shade);
}

void ConvertExtData::Dealloc()
{
	for (auto& _blitter : Blitters) {
		if(_blitter){
			delete _blitter;
			_blitter = nullptr;
		}
	}

	for (auto& _RLEblitter : RLEBlitters) {
		if (_RLEblitter) {
			delete _RLEblitter;
			_RLEblitter = nullptr;
		}
	}
}

int ConvertExtData::ResolveIndex(DWORD flags)
{
	if (flags <= BlitFlags::SingleBitPathMax) {
		// Anything below the lowest selector bit picks nothing at all - not
		// index 0. `return 0` here would silently hand back Alpha2.
		if (flags < BlitFlags::SingleBitMin)
			return BlitterIndex::Count;

		for (int i = 0; i < BlitFlags::SingleBitCount; ++i) {
			if (flags & BlitFlags::SingleBits[i])
				return i;
		}

		// Reachable: e.g. flags == 0x80000000 exactly, which is <= the max but
		// matches no selector bit.
		return BlitterIndex::Count;
	}

	switch (flags & BlitFlags::FamilyMask)
	{
	case BlitFlags::FamilyAdd:            return BlitterIndex::Add;
	case BlitFlags::FamilyMultiply:       return BlitterIndex::Multiply;
	case BlitFlags::FamilyDoubleMultiply: return BlitterIndex::DoubleMultiply;
	case BlitFlags::FamilyLuna:           return BlitterIndex::Luna;

		// UPDATED: both custom families resolve to the same blitter. Which of its
		// two callback slots gets filled is decided by the DrawSHP handler, not
		// here - see BlitterCustom.h.
	case BlitFlags::FamilyCustomPixel:
	case BlitFlags::FamilyCustomSpan:     return BlitterIndex::Custom;

		// The original splits this as `if (family > 0x88700000)` accepting only
		// 0x888/0x889, plus a switch over 0x884..0x887. Everything else falls
		// through to zero, which the default covers.
	default:                              return BlitterIndex::Count;
	}
}

DWORD TlsSlot_Blitter { TLS_OUT_OF_INDEXES };
DWORD TlsSlot_Custom { TLS_OUT_OF_INDEXES };

void ConvertExtData::AllocTLS()
{
	TlsSlot_Blitter = TlsAlloc();
	TlsSlot_Custom = TlsAlloc();

	if (TlsSlot_Blitter == TLS_OUT_OF_INDEXES || TlsSlot_Custom == TLS_OUT_OF_INDEXES)
		Debug::FatalErrorAndExit("Out of index oc.\n");
}

static AlphaCursor* GetArmedCursor()
{
	auto pBlitter = ::TlsGetValue(TlsSlot_Blitter);
	return pBlitter ? GetAlphaCursor(pBlitter) : nullptr;
}

static void SelectBlitterCommon(REGISTERS* R, bool rle)
{
	const int frameIdx = R->Stack<int>(0xAC);           // v1[43]
	const DWORD flags = R->Stack<DWORD>(0xB8);          // v1[46]
	const auto pShape = R->Stack<SHPCaches*>(0xA8);  // v1[42]
	const auto pConvert = R->Stack<ConvertClass*>(0x60);        // v1[24] - ConvertClass*

	auto pTable = (ConvertExtData*)pConvert->GetPptrFromPad();

	if (!pTable->IsReady(rle))
		pTable->Alloc();

	void* pBlitter = rle
		? static_cast<void*>(pTable->SelectRLE(flags))
		: static_cast<void*>(pTable->Select(flags));

	if (pBlitter)
		R->EAX(reinterpret_cast<DWORD>(pBlitter));

	// -- install the custom blit callback -----------------------------------
	const DWORD family = flags & BlitFlags::FamilyMask;
	if (family == BlitFlags::FamilyCustomPixel || family == BlitFlags::FamilyCustomSpan) {
		// SUSPECT: dereferenced with no null check, two lines after being tested
		// for null. Preserved verbatim.
		// Layout of BlitterCustom and BlitterCustomRLE is identical here.
		auto pCustom = static_cast<BlitterCustom*>(pBlitter);
		auto secondary = ::TlsGetValue(TlsSlot_Custom);

		// Installing a callback, not pointing at a buffer.
		if (family == BlitFlags::FamilyCustomSpan)
		{
			pCustom->SpanBlitter = reinterpret_cast<CustomSpanBlitter>(secondary);
			pCustom->PixelBlender = nullptr;
		}
		else
		{
			pCustom->PixelBlender = reinterpret_cast<CustomPixelBlender>(secondary);
			pCustom->SpanBlitter = nullptr;
		}

		return;
	}

	// -- arm the alpha shadow walk -----------------------------------------
	if (!(flags & BlitFlags::BufferedAlpha))
		return;

	// The 0x40000000 bit is exactly what makes the selector return the
	// buffered blitter of the right family, so the cursor is valid.
	auto pAlpha = GetAlphaCursor(pBlitter);
	const auto it = SHPExtData::Array.find_if([pShape](SHPExtData* pExtShape) {
		return pExtShape->AttachedToObject == pShape;
	});

	if (it == SHPExtData::Array.end()) {
		Debug::Log("SHPExt not valid. Reason : Ext not found\n");
		return;
	}

	auto pExt = *it;

	// BUG (verbatim): pExt->AlphaSHP is read before pExt is tested, so a null
	// ext faults at 0x0C and the "Ext not found" string is unreachable.
	if (!pExt || !pExt->AlphaSHP) {
		Debug::Log("SHPExt not valid. Reason : %s\n",
			pExt ? "Invalid Alpha file" : "Ext not found");

		return;
	}

	RectangleStruct bounds {};
	pShape->GetFrameBounds(bounds, frameIdx);

	if (bounds.Width == 0)
		return;

	// Signed short read; see AlphaBlitState.cpp for the note.
	const int stride = pShape->GetWidth();

	auto pPixels = pExt->AlphaSHP->GetPixels(frameIdx);

	pAlpha->Stride = stride;
	pAlpha->Base = pPixels + bounds.X + stride * bounds.Y;

	::TlsSetValue(TlsSlot_Blitter, pBlitter); // the blitter, not the cursor
}

ASMJIT_PATCH(0x4AF1E2 , DSurface_DrawSHP_GetSelectedBlitter, 8)
{
	SelectBlitterCommon(R, false);
	return 0;
}

ASMJIT_PATCH(0x4AF14C , DSurface_DrawSHP_GetSelectedRLEBlitter, 8)
{
	SelectBlitterCommon(R, true);
	return 0;
}

// 0x100529D0 - TLS is only an arm flag; the blitter comes from Stack(0x90)
ASMJIT_PATCH(0x437D51 ,DSurface_BlitWIthRLE_AdjustHeight, 6)
{
	const int rows = R->Stack<int>(0x28);
	const auto pBlitter = R->Stack<void*>(0x90);

	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
		return 0;

	GetAlphaCursor(pBlitter)->SeedRows(rows);
	return 0;
}

// 0x10052A60 - Cursor += Stride * ECX
ASMJIT_PATCH(0x4376BB , DSurface_BlitWithPlain_AdjustHeight2, 6)
{
	const int rows = static_cast<int>(R->ECX());

	if (auto pCursor = GetArmedCursor())
		pCursor->SkipRows(rows);

	return 0;
}

ASMJIT_PATCH(0x7BC1BC, DSurface_DoubleIntersectLock_AdjustHeight, 5)
{
	const int x = R->Stack<int>(0x0); // VERIFY: clipped X
	const int y = R->Stack<int>(0x4); // VERIFY: clipped Y

	if (auto pCursor = GetArmedCursor())
		pCursor->SeedFrom(x, y);

	return 0;
}

ASMJIT_PATCH(0x437E45 , DSurface_BlitWithRLE_Add, 5)
{
	// VERIFY: cached `this` vs. a real frame pointer. See AlphaBlitState.cpp.
	const auto pBlitter = reinterpret_cast<void*>(R->EBP());

	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
		return 0;

	GetAlphaCursor(pBlitter)->NextRow();
	return 0;
}ASMJIT_PATCH_AGAIN(0x4377E5, DSurface_BlitWithRLE_Add, 8)
ASMJIT_PATCH_AGAIN(0x4378FC, DSurface_BlitWithRLE_Add, 6)

ASMJIT_PATCH(0x43746A, DSurface_BlitWithRLE_Off, 7)
{
	::TlsSetValue(TlsSlot_Blitter, nullptr);
	::TlsSetValue(TlsSlot_Custom, nullptr);
	return 0;
}ASMJIT_PATCH_AGAIN(0x437990, DSurface_BlitWithRLE_Off, 7)
ASMJIT_PATCH_AGAIN(0x437B33, DSurface_BlitWithRLE_Off, 7)
ASMJIT_PATCH_AGAIN(0x437D47, DSurface_BlitWithRLE_Off, 7)

// the newer dll probably does more fancy stuffs
// here is only some part of early sample
class ReShadeRuntime
{
public:
	uint8_t Unknown_00[0x08];   // 0x00  vftable + one dword
	uint8_t IsInitialized;      // 0x08  runtime::_is_initialized
	uint8_t Padding_09[0x03];
	int     Width;              // 0x0C  runtime::_width   (backbuffer)
	int     Height;             // 0x10  runtime::_height  (backbuffer)

	// Further members recovered from the inlined runtime::on_init, kept as
	// comments rather than declared -- nothing here needs them:
	//   +0x14 _window_width      +0x18 _window_height
	//   +0x28 _color_bit_depth   +0x30 _framecount (u64)
	//   +0x88 _input (shared_ptr, _Rep at +0x8C)
	//   +0x180 _last_reload_time
	//   +0x6A8 _app_state        +0x6F4 _device      +0x6F8 _swapchain
	//   +0x714 _backbuffer .. +0x730 _effect_vertex_layout
	//   +0x750 mask texture (D3DFMT_L8, D3DPOOL_MANAGED)  <- AUTHOR INSERTION
	//   +0x754 mask texture surface                       <- AUTHOR INSERTION
	// Object is 0x760 bytes (0x770 allocation minus the 0x10 control header).
};

static_assert(offsetof(ReShadeRuntime, Width) == 0x0C, "read at 1002852A");
static_assert(offsetof(ReShadeRuntime, Height) == 0x10, "read at 1002852E");

// ---------------------------------------------------------------------------
// The per-frame alpha mask buffer.
// ---------------------------------------------------------------------------
struct AlphaMask
{
	// dword_10298580 -- the *active* mask buffer. Non-null only between
	// BeginPass and EndPass, so it doubles as the "are we inside a tactical
	// draw pass" guard every writer tests first.
	static uint8_t* ActiveBuffer;

	// Block -- the backing allocation. VERIFY: allocated elsewhere, and never
	// null-checked by any of these functions.
	static uint8_t* Buffer;

	// dword_102A2E78 -- surface the mask is sized against.
	static ReShadeRuntime* TargetSurface;

	// unk_102A1580. The original calls _Mtx_lock, throws via _Throw_C_error on
	// a non-zero result, and calls _Mtx_unlock ignoring its result -- which is
	// exactly what MSVC emits for std::mutex::lock() and unlock(). This is that
	// mutex, restored to its source form.
	//
	// It cannot be wrapped in lock_guard/unique_lock: BeginPass acquires and
	// EndPass releases, so the critical section spans two separate hooks.
	static std::mutex BufferMutex;

	// CreateAlpha (.text:10028780). __fastcall: pShape in ecx, frame in edx,
	// the rest on the stack (bounds is a 16-byte by-value Rect).
	//
	// `flags` is the same dword BlitFlags/DrawFlags decode. Only
	// DrawFlags::Center is read here; the selector half is ignored.
	static bool __fastcall Blit(SHPCaches* pShape, int frame, DWORD flags,
		RectangleStruct bounds, int x, int y)
	{
		if (!ActiveBuffer || !pShape || !TargetSurface)
			return false;

		// The mask is one byte per pixel, so the surface width is also the stride.
		const int destStride = TargetSurface->Width;

		// 100287CD -- cmovz. Only shapes whose first word is 0xFFFF are accepted.
		// VERIFY: 0xFFFF presumably marks the extended header this DLL installs;
		// a vanilla SHP has 0 there.
		SHPHeader* const pSource = pShape->IsReference() ? (SHPHeader*)pShape : nullptr;

		if (!pSource)
			return false;

		RectangleStruct frameBounds {};
		pShape->GetFrameBounds(frameBounds, frame);

		// 100287ED / 1002880C -- `and ebx, 200h`. IDA types the parameter __int16,
		// which is wrong; the instruction reads the full dword. The `cdq / sub /
		// sar 1` sequence is signed division truncating toward zero.
		//
		// NOTE: the shadow hooks below force DrawFlags::Center in, so a shadow draw
		// always takes this branch regardless of what the caller passed.
		const bool centered = DrawFlags::IsCentered(flags);
		const int centerX = centered ? pSource->Width / 2 : 0;
		const int centerY = centered ? pSource->Height / 2 : 0;

		frameBounds.X += x - centerX;
		frameBounds.Y += y - centerY;

		RectangleStruct clipped = RectangleStruct::Intersect(frameBounds, bounds ,nullptr , nullptr);

		// 10028867 / 10028875 -- plain zero tests, NOT <= 0. A negative width or
		// height sails through here and is only caught by the `rows <= 0` guards in
		// the two blitters. Preserved.
		//
		// This is also what lets BlitRaw hand `width` straight to memmove_s: the
		// original's inlined copy has no count==0 branch because of this check.
		if (clipped.Width == 0 || clipped.Height == 0)
			return false;

		const int srcSkipRows = clipped.Y - frameBounds.Y;
		const int srcSkipCols = clipped.X - frameBounds.X;
		const int destX = clipped.X - bounds.X;
		const int destY = clipped.Y - bounds.Y;

		// 1002889E -- order preserved: Is_RLE_Compressed is called BEFORE the frame
		// pointer is null-checked.
		const uint8_t* const pFrame = pShape->GetPixels(frame);
		const bool isRLE = pShape->HasCompression(frame);

		if (!pFrame)
			return false;

		uint8_t* const pDest = ActiveBuffer + destStride * destY + destX;

		return isRLE
			? BlitRLE(pDest, destStride, pFrame, srcSkipCols, srcSkipRows,
				clipped.Width, clipped.Height)
			: BlitRaw(pDest, destStride, pFrame, frameBounds.Width, srcSkipCols,
				srcSkipRows, clipped.Width, clipped.Height);
	}


	// TacticalClass_UpdateDrawFunc   (.text:10028520)
	static void BeginPass()
	{
		if (!TargetSurface)
			return;

		// HAZARD: read before the lock is taken (1002852A / 1002852E precede the
		// _Mtx_lock call). A surface resize racing this pass sizes the clear off
		// stale dimensions. Preserved.
		const int width = TargetSurface->Width;
		const int height = TargetSurface->Height;

		BufferMutex.lock();

		ActiveBuffer = Buffer;

		// BUG: Buffer is never null-checked, and nothing verifies it is still
		// width * height bytes. Preserved.
		std::memset(Buffer, 0, static_cast<size_t>(width) * height);
	}

	// TacticalClass_UpdateDrawReturn (.text:10028570)
	static void EndPass()
	{
		// BUG: the unlock is gated on the same global the lock was gated on. If
		// TargetSurface is cleared mid-pass the mutex is never released and the
		// next pass deadlocks; if it is set mid-pass this unlocks a mutex this
		// thread does not own, which is undefined behaviour rather than a no-op.
		// Preserved -- the fix is latching a bool in BeginPass and testing that
		// here instead of re-reading TargetSurface.
		if (TargetSurface)
		{
			BufferMutex.unlock();
			ActiveBuffer = nullptr;
		}
	}

private:
	static bool BlitRaw(uint8_t* pDest, int destStride, const uint8_t* pFrame,
		int srcStride, int srcSkipCols, int srcSkipRows, int width, int rows)
	{
		const uint8_t* pSrc = pFrame + srcStride * srcSkipRows + srcSkipCols;

		if (rows <= 0)
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

	static bool BlitRLE(uint8_t* pDest, int destStride, const uint8_t* pRow,
		int srcSkipCols, int srcSkipRows, int width, int rows)
	{
		// 100288D1 -- walk past fully clipped rows using the length headers.
		for (int skipped = srcSkipRows; skipped > 0; --skipped)
			pRow += *reinterpret_cast<const int16_t*>(pRow);

		if (rows <= 0)
			return true;

		// 100288E5 -- truncated to 16 bits and reloaded from the stack slot on
		// every iteration, so the width of the type matters.
		const int16_t skipCols = static_cast<int16_t>(srcSkipCols);

		do
		{
			const uint8_t* pIn = pRow + sizeof(int16_t);
			uint8_t* pOut = pDest;
			int16_t remaining = static_cast<int16_t>(width);

			if (skipCols > 0)
			{
				// 10028905 -- consume runs until the clip edge is reached or
				// passed. `overshoot` ends up >= 0.
				int overshoot = -skipCols;

				do
				{
					const uint8_t code = *pIn++;
					overshoot += code ? 1 : *pIn++;
				}
				while (overshoot < 0);

				// A run straddling the clip edge is dropped whole: the output
				// pointer advances by the overshoot and the row shrinks to match,
				// so those pixels keep whatever the buffer already held.
				pOut = pDest + overshoot;
				remaining = static_cast<int16_t>(width - overshoot);
			}

			// 10028930 -- BUG: nothing bounds-checks pOut against the row width.
			// A frame whose RLE disagrees with its header runs off the end of the
			// mask buffer. Preserved verbatim.
			for (int left = remaining; left > 0; )
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
};

uint8_t* AlphaMask::ActiveBuffer = nullptr;   // dword_10298580
uint8_t* AlphaMask::Buffer = nullptr;   // Block -- VERIFY: allocated elsewhere
ReShadeRuntime* AlphaMask::TargetSurface = nullptr;   // dword_102A2E78
std::mutex AlphaMask::BufferMutex;               // unk_102A1580

static inline bool FXLightEnabled(AnimClass* pAnim)
{
	return AnimTypeExtContainer::Instance.Find(pAnim->Type)->FXLightEnable;
}

ASMJIT_PATCH(0x6D8F0F , TacticalClass_UpdateDrawFunc, 6)
{
	AlphaMask::BeginPass();
	return 0;
}

ASMJIT_PATCH(0x6D97BF , TacticalClass_UpdateDrawReturn, 6)
{
	AlphaMask::EndPass();
	return 0;
}

// ---------------------------------------------------------------------------
// Variant 0. .text:100285A0
//
// Reads four consecutive dwords off ESP: the tail of a CC_Draw_Shape argument
// push, one slot in -- the shape itself comes from EAX rather than the stack.
// Passes the caller's flags through untouched.
// ---------------------------------------------------------------------------

ASMJIT_PATCH(0x4236F0 , AnimClass_Draw_SetMaskBuffer, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(SHPCaches*, pShape, EAX);
	GET(DWORD*, pStack, ESP);

	const int frame = static_cast<int>(pStack[0]);
	auto const pPoint = reinterpret_cast<Point2D*>(pStack[1]);
	auto const pBounds = reinterpret_cast<RectangleStruct*>(pStack[2]);
	const DWORD flags = pStack[3];

	if (FXLightEnabled(pAnim))
		AlphaMask::Blit(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y);

	return 0;
}ASMJIT_PATCH_AGAIN(0x4233E4, AnimClass_Draw_SetMaskBuffer, 5)

// ---------------------------------------------------------------------------
// Variant 1. .text:10028610
//
// Five dwords off ESP -- the full CC_Draw_Shape argument block at the call:
// [0] shape, [1] shapenum, [2] xy, [3] rect1, [4] flags.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x423821 , AnimClass_Draw_SetMaskBuffer_1, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(DWORD*, pStack, ESP);

	if (FXLightEnabled(pAnim)) {
		auto const pShape = reinterpret_cast<SHPCaches*>(pStack[0]);
		const int frame = static_cast<int>(pStack[1]);
		auto const pPoint = reinterpret_cast<Point2D*>(pStack[2]);
		auto const pBounds = reinterpret_cast<RectangleStruct*>(pStack[3]);
		const DWORD flags = pStack[4];
		AlphaMask::Blit(pShape, frame, flags, *pBounds, pPoint->X, pPoint->Y);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 2. .text:10028680 -- the shadow pass.
//
// ESP is Draw_It's frame base here: +0x28 is `shape`, +0x2C is `sz`, +0x118 is
// the `rect1` argument. This is what pins the REGISTERS layout.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x42383C , AnimClass_Draw_SetMaskBuffer_2, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, EDI);
	GET(DWORD, drawFlags, EBX);
	GET(uintptr_t, stackBase, ESP);

	if (FXLightEnabled(pAnim)) {
		auto const pShape = *reinterpret_cast<SHPCaches**>(stackBase + 0x28);
		const int frame = *reinterpret_cast<int*>(stackBase + 0x2C);
		auto const pBounds = *reinterpret_cast<RectangleStruct**>(stackBase + 0x118);

		AlphaMask::Blit(pShape, frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y);
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Variant 3. .text:10028700 -- the extras/shadow pass, 0x20 bytes deeper into
// the same frame, so `shape` sits at ESP+0x48 instead of ESP+0x28.
// ---------------------------------------------------------------------------
ASMJIT_PATCH(0x4237A3 , AnimClass_Draw_SetMaskBuffer_3, 6)
{
	GET(AnimClass*, pAnim, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(DWORD, drawFlags, EBX);
	GET(int, frame, EDX);
	GET(RectangleStruct*, pBounds, EAX);
	GET(uintptr_t, stackBase, ESP);

	if (FXLightEnabled(pAnim)) {
		AlphaMask::Blit(*reinterpret_cast<SHPCaches**>(stackBase + 0x48), frame, DrawFlags::ToShadow(drawFlags),
			*pBounds, pPoint->X, pPoint->Y);
	}

	return 0;
}

DWORD ApplyAnimDrawFlags(AnimClass* pAnim, DWORD flags)
{
	auto pTypeExt = AnimTypeExtContainer::Instance.Find(pAnim->Type);

	// 100527E8 -- FullReplaceBlendFunction is tested first and wins outright.
	if (auto const pEntry = pTypeExt->FullReplaceBlendFunction.get())
	{
		// FARPROC does not implicitly convert to void*; the original just pushes
		// the raw dword, so the cast is the faithful spelling.
		TlsSetValue(TlsSlot_Custom, reinterpret_cast<void*>(pEntry->Proc));

		return flags | BlitFlags::FamilyCustomSpan;
	}

	// 1005280F
	if (auto const pEntry = pTypeExt->BlendFunction.get())
	{
		// FARPROC does not implicitly convert to void*; the original just pushes
		// the raw dword, so the cast is the faithful spelling.
		TlsSetValue(TlsSlot_Custom, reinterpret_cast<void*>(pEntry->Proc));

		return flags | BlitFlags::FamilyCustomPixel;
	}

	// 10052836 -- the Translucency switch. Resolve returns 0 for an
	// unrecognised value, which reproduces the original's default case: it
	// still writes the flags back, just unmodified.
	return flags | TranslucencyKeys::Resolve(pTypeExt->This()->Translucency);
}

ASMJIT_PATCH(0x423051, AnimClass_Draw_SetFlags, 0xA)
{
	GET(AnimClass*, pAnim, ESI);
	GET(DWORD, flags, EBX);

	R->EBX(ApplyAnimDrawFlags(pAnim, flags));

	return 0;
}
#pragma optimize("", on )