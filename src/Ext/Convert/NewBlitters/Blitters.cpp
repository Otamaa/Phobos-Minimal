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

//#include <Misc/ReShade/Runtime/d3d9/runtime_d3d9.hpp>
//#include <Misc/ReShade/Runtime/d3d9/d3d9def.h>

#include <ConvertClass.h>

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

//ASMJIT_PATCH(0x4AF1E2 , DSurface_DrawSHP_GetSelectedBlitter, 8)
//{
//	SelectBlitterCommon(R, false);
//	return 0;
//}
//
//ASMJIT_PATCH(0x4AF14C , DSurface_DrawSHP_GetSelectedRLEBlitter, 8)
//{
//	SelectBlitterCommon(R, true);
//	return 0;
//}
//
//// 0x100529D0 - TLS is only an arm flag; the blitter comes from Stack(0x90)
//ASMJIT_PATCH(0x437D51 ,DSurface_BlitWIthRLE_AdjustHeight, 6)
//{
//	const int rows = R->Stack<int>(0x28);
//	const auto pBlitter = R->Stack<void*>(0x90);
//
//	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
//		return 0;
//
//	GetAlphaCursor(pBlitter)->SeedRows(rows);
//	return 0;
//}
//
//// 0x10052A60 - Cursor += Stride * ECX
//ASMJIT_PATCH(0x4376BB , DSurface_BlitWithPlain_AdjustHeight2, 6)
//{
//	const int rows = static_cast<int>(R->ECX());
//
//	if (auto pCursor = GetArmedCursor())
//		pCursor->SkipRows(rows);
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x7BC1BC, DSurface_DoubleIntersectLock_AdjustHeight, 5)
//{
//	const int x = R->Stack<int>(0x0); // VERIFY: clipped X
//	const int y = R->Stack<int>(0x4); // VERIFY: clipped Y
//
//	if (auto pCursor = GetArmedCursor())
//		pCursor->SeedFrom(x, y);
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x437E45 , DSurface_BlitWithRLE_Add, 5)
//{
//	// VERIFY: cached `this` vs. a real frame pointer. See AlphaBlitState.cpp.
//	const auto pBlitter = reinterpret_cast<void*>(R->EBP());
//
//	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
//		return 0;
//
//	GetAlphaCursor(pBlitter)->NextRow();
//	return 0;
//}ASMJIT_PATCH_AGAIN(0x4377E5, DSurface_BlitWithRLE_Add, 8)
//ASMJIT_PATCH_AGAIN(0x4378FC, DSurface_BlitWithRLE_Add, 6)
//
//ASMJIT_PATCH(0x43746A, DSurface_BlitWithRLE_Off, 7)
//{
//	::TlsSetValue(TlsSlot_Blitter, nullptr);
//	::TlsSetValue(TlsSlot_Custom, nullptr);
//	return 0;
//}ASMJIT_PATCH_AGAIN(0x437990, DSurface_BlitWithRLE_Off, 7)
//ASMJIT_PATCH_AGAIN(0x437B33, DSurface_BlitWithRLE_Off, 7)
//ASMJIT_PATCH_AGAIN(0x437D47, DSurface_BlitWithRLE_Off, 7)
//
//
//DWORD ApplyAnimDrawFlags(AnimClass* pAnim, DWORD flags)
//{
//	auto pTypeExt = AnimTypeExtContainer::Instance.Find(pAnim->Type);
//
//	// 100527E8 -- FullReplaceBlendFunction is tested first and wins outright.
//	if (auto const pEntry = pTypeExt->FullReplaceBlendFunction.get())
//	{
//		// FARPROC does not implicitly convert to void*; the original just pushes
//		// the raw dword, so the cast is the faithful spelling.
//		TlsSetValue(TlsSlot_Custom, reinterpret_cast<void*>(pEntry->Proc));
//
//		return flags | BlitFlags::FamilyCustomSpan;
//	}
//
//	// 1005280F
//	if (auto const pEntry = pTypeExt->BlendFunction.get())
//	{
//		// FARPROC does not implicitly convert to void*; the original just pushes
//		// the raw dword, so the cast is the faithful spelling.
//		TlsSetValue(TlsSlot_Custom, reinterpret_cast<void*>(pEntry->Proc));
//
//		return flags | BlitFlags::FamilyCustomPixel;
//	}
//
//	// 10052836 -- the Translucency switch. Resolve returns 0 for an
//	// unrecognised value, which reproduces the original's default case: it
//	// still writes the flags back, just unmodified.
//	return flags | TranslucencyKeys::Resolve(pTypeExt->This()->Translucency);
//}
//
//ASMJIT_PATCH(0x423051, AnimClass_Draw_SetFlags, 0xA)
//{
//	GET(AnimClass*, pAnim, ESI);
//	GET(DWORD, flags, EBX);
//
//	R->EBX(ApplyAnimDrawFlags(pAnim, flags));
//
//	return 0;
//}