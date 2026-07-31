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

#include <FileFormats/SHP.h>

#include <ConvertClass.h>

// ============================================================================
//
//  ConvertExt::ExtData - the 36-entry blitter table - plus the flag decoding
//  that picks an entry out of it.
//
//  Named by the string at .rdata 0x1024D228:
//      "Exception occured at ConvertExt::ExtData::BuildNewBlitters , Type = %s"
//
//  The first three dwords are the standard Ares ExtData prefix, exactly as on
//  SHPExtData. That also settles what Stack(0x60) is in the DrawSHP handlers:
//  a ConvertClass, with this ExtData hanging off it at +0x178 - the same
//  pointer-on-the-object pattern SHPReference uses at +0x24.
// ============================================================================

// ---------------------------------------------------------------------------
//  Position within a family. Both families use the same ordering, which is
//  what lets one resolver serve both selectors.
//
//  IDA cross-reference, for reading the pseudocode:
//      this[N]  ==  Blitters[N - 3]      for N in 3..20
//      this[N]  ==  RLEBlitters[N - 21]  for N in 21..38
// ---------------------------------------------------------------------------
struct BlitterIndex
{
	enum : int
	{
		// Twelve translucency levels. N is the DESTINATION weight out of 32:
		//     result = (N*dst + (32-N)*src) / 32
		// 8, 16 and 24 are absent because those are 25% / 50% / 75%, which
		// vanilla already ships as BlitTransLucent25 / 50 / 75.
		Alpha2 = 0, Alpha4, Alpha6, Alpha10, Alpha12, Alpha14,
		Alpha18, Alpha20, Alpha22, Alpha26, Alpha28, Alpha30,

		BufferedAlpha = 12,
		Add = 13,
		Multiply = 14,
		DoubleMultiply = 15,
		Luna = 16,
		Custom = 17,

		Count = 18,
	};
};

// ---------------------------------------------------------------------------
//  Flag decoding, recovered from sub_1004C270.
//
//  Two disjoint encodings share one dword:
//
//   (a) flags <= 0x80000000 : single-bit selector, LOWEST set bit wins.
//       0x00040000 .. 0x20000000 pick the twelve translucency levels and
//       0x40000000 picks the buffered-alpha blitter. Below 0x00040000
//       selects nothing.
//
//   (b) flags >  0x80000000 : (flags & 0xFFF00000) is a family id.
//
//  Consequence worth keeping in mind: the 0x40000000 alpha path and the
//  0x888/0x889 custom paths can NEVER both be taken for one draw, because the
//  single-bit path only runs for flags <= 0x80000000. That disjointness is the
//  only thing keeping GetAlphaCursor() away from a BlitterCustom, whose
//  callback slots sit at the same +0x10 the alpha cursor uses.
// ---------------------------------------------------------------------------
struct BlitFlags
{
	static constexpr DWORD SingleBitMax = 0x80000000u;
	static constexpr DWORD SingleBitMin = 0x00040000u;
	static constexpr DWORD FamilyMask = 0xFFF00000u;

	static constexpr DWORD BufferedAlpha = 0x40000000u;

	static constexpr DWORD FamilyAdd = 0x88400000u;
	static constexpr DWORD FamilyMultiply = 0x88500000u;
	static constexpr DWORD FamilyDoubleMultiply = 0x88600000u;
	static constexpr DWORD FamilyLuna = 0x88700000u;

	// UPDATED: these were FamilyCustomA / FamilyCustomB back when the two
	// slots on BlitterCustom were mistaken for source buffers. They select
	// which CALLBACK the caller is installing out of TLS slot B.
	static constexpr DWORD FamilyCustomPixel = 0x88800000u; // -> PixelBlender
	static constexpr DWORD FamilyCustomSpan = 0x88900000u; // -> SpanBlitter

	// Ordered exactly as the original's if-chain tests them. The chain returns
	// on the LOWEST set bit; a bit-scan would give the same answer, but only
	// because the order happens to be ascending, so the explicit list
	// documents the priority rather than implying it.
	static constexpr DWORD SingleBits[BlitterIndex::Count - 5] =
	{
		0x00040000u, // Alpha2      N=2, dst weight 2/32
		0x00080000u, // Alpha4
		0x00100000u, // Alpha6
		0x00200000u, // Alpha10
		0x00400000u, // Alpha12
		0x00800000u, // Alpha14
		0x01000000u, // Alpha18
		0x02000000u, // Alpha20
		0x04000000u, // Alpha22
		0x08000000u, // Alpha26
		0x10000000u, // Alpha28
		0x20000000u, // Alpha30
		0x40000000u, // BufferedAlpha
	};

	static constexpr int SingleBitCount = BlitterIndex::Count - 5; // 13
};

struct NewPalData
{
	static constexpr int FamilySize = 18;
	static constexpr int UniversalCount =
		static_cast<int>(std::size(BlitterDetail::UniversalAlphaLevels));

	std::unique_ptr<Blitter> Blitters[FamilySize];
	std::unique_ptr <RLEBlitter> RLEBlitters[FamilySize];

public:

	// ---------------------------------------------------------------------------
	//  Bit -> position within a family.
	//
	//  The original duplicates this logic across sub_1004C130 and sub_1004C270
	//  with different base indices; both families order their entries the same
	//  way, so one resolver serves both.
	// ---------------------------------------------------------------------------
	static int ResolveIndex(DWORD flags)
	{
		if (flags <= BlitFlags::SingleBitMax)
		{
			// Anything below the lowest selector bit picks nothing at all - not
			// index 0. `return 0` here would silently hand back Alpha2.
			if (flags < BlitFlags::SingleBitMin)
				return BlitterIndex::Count;

			for (int i = 0; i < BlitFlags::SingleBitCount; ++i)
			{
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

	static void AllocateNewBlitters(NewPalData* pData, ConvertClass* pConver) {
		pData->Alloc((WORD*)pConver->BufferA, pConver->BytesPerPixel);
	}

	bool IsPlainReady() const { return this->Blitters[0] != nullptr; }
	bool IsRLEReady() const { return this->RLEBlitters[0] != nullptr; }
	bool IsReady(bool rle) const { return rle ? this->IsRLEReady() : this->IsPlainReady(); }

	Blitter* Select(DWORD flags) const
	{
		// VERIFY: only sub_1004C270 (the RLE one) was ever disassembled.
		// sub_1004C130 is assumed to be the same code over the plain array - the
		// two DrawSHP handlers are otherwise byte-identical and the families
		// occupy their arrays in the same order. Confirm before shipping.
		const int index = ResolveIndex(flags);
		return index < BlitterIndex::Count ? this->Blitters[index].get() : nullptr;
	}

	RLEBlitter* SelectRLE(DWORD flags) const
	{
		const int index = ResolveIndex(flags);
		return index < BlitterIndex::Count ? this->RLEBlitters[index].get() : nullptr;
	}

	void Alloc(WORD* palData, int shade)
	{
		Blitters[0] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 2u>>(palData, shade);
		Blitters[1] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 4u>>(palData, shade);
		Blitters[2] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 6u>>(palData, shade);
		Blitters[3] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 10u>>(palData, shade);
		Blitters[4] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 12u>>(palData, shade);
		Blitters[5] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 14u>>(palData, shade);
		Blitters[6] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 18u>>(palData, shade);
		Blitters[7] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 20u>>(palData, shade);
		Blitters[8] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 22u>>(palData, shade);
		Blitters[9] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 26u>>(palData, shade);
		Blitters[10] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 28u>>(palData, shade);
		Blitters[11] = std::make_unique<BlitTransLucentUniversalAlphaZRead<WORD, 30u>>(palData, shade);
		Blitters[12] = std::make_unique<BlitTransLucentBufferedAlphaZRead<WORD>>(palData, shade);
		Blitters[13] = std::make_unique<BlitTransLucentAddZRead<WORD>>(palData, shade);
		Blitters[14] = std::make_unique<BlitTransLucentMultiplyZRead<WORD>>(palData, shade);
		Blitters[15] = std::make_unique<BlitTransLucentDoubleMultiplyZRead<WORD>>(palData, shade);
		Blitters[16] = std::make_unique<BlitTransLucentLunaZRead<WORD>>(palData, shade);
		Blitters[17] = std::make_unique<BlitterCustom>(palData, shade);

		RLEBlitters[0] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 2u>>(palData, shade);
		RLEBlitters[1] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 4u>>(palData, shade);
		RLEBlitters[2] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 6u>>(palData, shade);
		RLEBlitters[3] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 10u>>(palData, shade);
		RLEBlitters[4] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 12u>>(palData, shade);
		RLEBlitters[5] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 14u>>(palData, shade);
		RLEBlitters[6] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 18u>>(palData, shade);
		RLEBlitters[7] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 20u>>(palData, shade);
		RLEBlitters[8] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 22u>>(palData, shade);
		RLEBlitters[9] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 26u>>(palData, shade);
		RLEBlitters[10] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 28u>>(palData, shade);
		RLEBlitters[11] = std::make_unique<RLEBlitTransLucentUniversalAlphaZRead<WORD, 30u>>(palData, shade);
		RLEBlitters[12] = std::make_unique<RLEBlitTransLucentBufferedAlphaZRead<WORD>>(palData, shade);
		RLEBlitters[13] = std::make_unique<RLEBlitTransLucentAddZRead<WORD>>(palData, shade);
		RLEBlitters[14] = std::make_unique<RLEBlitTransLucentMultiplyZRead<WORD>>(palData, shade);
		RLEBlitters[15] = std::make_unique<RLEBlitTransLucentDoubleMultiplyZRead<WORD>>(palData, shade);
		RLEBlitters[16] = std::make_unique<RLEBlitTransLucentLunaZRead<WORD>>(palData, shade);
		RLEBlitters[17] = std::make_unique<BlitterCustomRLE>(palData, shade);
	}

	void Dealloc()
	{
		for (auto& _blitter : Blitters)
		{
			_blitter.reset();
		}

		for (auto& _RLEblitter : RLEBlitters)
		{
			_RLEblitter.reset();
		}
	}
};

#ifdef _Integrate

// ============================================================================
//  AlphaBlitState_TLS.cpp
//
//  FAITHFUL variant of the eight-hook cluster: keeps the two Win32 TLS slots
//  exactly as the original DLL used them, instead of the thread_local
//  AlphaBlitContext in AlphaBlitState.cpp.
//
//  Build ONE of the two, never both - they define the same ASMJIT_PATCH names
//  and would fight over the same injection addresses.
//
//    AlphaBlitState.cpp      thread_local; no TlsAlloc, no failure path.
//                            Preferred for new work.
//    AlphaBlitState_TLS.cpp  this file. Byte-for-byte behavioural match,
//                            including the game-init allocation and the
//                            "Out of index oc." abort. Use it when diffing
//                            against the original DLL.
//
//  Also carries the decluttered vtable map recovered from .rdata (see bottom).
// ============================================================================

#define ADDR_DrawSHP_GetBlitter        0x0 // VERIFY  <- handler 0x10052510
#define SIZE_DrawSHP_GetBlitter        0x6 // VERIFY
#define ADDR_DrawSHP_GetRLEBlitter     0x0 // VERIFY  <- handler 0x10052670
#define SIZE_DrawSHP_GetRLEBlitter     0x6 // VERIFY
#define ADDR_Lock_AdjustHeight         0x0 // VERIFY  <- handler 0x10052A20
#define SIZE_Lock_AdjustHeight         0x6 // VERIFY
#define ADDR_BlitRLE_AdjustHeight      0x0 // VERIFY  <- handler 0x100529D0
#define SIZE_BlitRLE_AdjustHeight      0x6 // VERIFY
#define ADDR_BlitPlain_AdjustHeight2   0x0 // VERIFY  <- handler 0x10052A60
#define SIZE_BlitPlain_AdjustHeight2   0x6 // VERIFY
#define ADDR_BlitRLE_Add               0x0 // VERIFY  <- handler 0x10052AA0
#define SIZE_BlitRLE_Add               0x6 // VERIFY
#define ADDR_BlitPlain_Add             0x0 // VERIFY  <- handler 0x10052AE0
#define SIZE_BlitPlain_Add             0x6 // VERIFY
#define ADDR_BlitRLE_Off               0x0 // VERIFY  <- handler 0x10052B10
#define SIZE_BlitRLE_Off               0x6 // VERIFY
#define ADDR_GameInit_Pre              0x0 // VERIFY  <- YR_GameInit_Pre
#define SIZE_GameInit_Pre              0x6 // VERIFY

#ifndef ALPHABLIT_ADDRESSES_FILLED
#error "AlphaBlitState_TLS.cpp: injection addresses/sizes are placeholders. \
Recover them from the hook descriptor table (off_10279948) first."
#endif

// ---------------------------------------------------------------------------
//  The two TLS index globals, allocated in YR_GameInit_Pre.
//
//    slot A - dword_1028DB60 - the armed buffered blitter (plain or RLE)
//    slot B - dwTlsIndex     - the BlitterCustom source pointer
//
//  VERIFY: only slot A's address appears in the dumps. dwTlsIndex is a named
//          IDA symbol whose address was never shown, so it is declared here
//          rather than bound to a fixed address. If you want a true binary
//          match, point TlsSlot_Custom at the real global instead.
// ---------------------------------------------------------------------------
static DWORD& TlsSlot_Blitter = *reinterpret_cast<DWORD*>(0x1028DB60);
static DWORD TlsSlot_Custom = TLS_OUT_OF_INDEXES; // VERIFY: address unknown

static AlphaCursor* GetArmedCursor()
{
	auto pBlitter = ::TlsGetValue(TlsSlot_Blitter);
	return pBlitter ? GetAlphaCursor(pBlitter) : nullptr;
}

// ===========================================================================
//  Allocation - the part AlphaBlitState.cpp deliberately drops.
//
//  Original, inside YR_GameInit_Pre:
//      dword_1028DB60 = TlsAlloc();
//      if (dword_1028DB60 == -1 || (dwTlsIndex = TlsAlloc(), dwTlsIndex == -1))
//          Fatal("Out of index oc.\n", 208);
//
//  NOTE the short-circuit: when the first TlsAlloc fails the second never runs,
//  so dwTlsIndex is left uninitialised on that path. Harmless only because the
//  Fatal call does not return. Preserved.
//
//  This hook covers ONLY the TLS lines. The rest of YR_GameInit_Pre - the
//  MEMORY[0x82D5xx] = 4 writes and the sub_100131B0 calls - is unrelated to
//  this cluster and is NOT reproduced here.
// ===========================================================================
ASMJIT_PATCH(ADDR_GameInit_Pre, YR_GameInit_Pre_AllocTls, SIZE_GameInit_Pre)
{
	TlsSlot_Blitter = ::TlsAlloc();

	if (TlsSlot_Blitter == TLS_OUT_OF_INDEXES
		|| (TlsSlot_Custom = ::TlsAlloc(), TlsSlot_Custom == TLS_OUT_OF_INDEXES))
	{
		// sub_100BED00("Out of index oc.\n", 208) - VERIFY: 208 is presumably an
		// exit or error code. Does not return.
		reinterpret_cast<void(__cdecl*)(const char*, int)>(0x100BED00)("Out of index oc.\n", 208);
	}

	return 0;
}

// ===========================================================================
//  SHARED SETUP - handlers 0x10052510 (plain) and 0x10052670 (RLE)
//  Identical to the thread_local version except for the two TLS calls.
// ===========================================================================
static void SelectBlitterCommon(REGISTERS* R, bool rle)
{
	const int frameIdx = R->Stack<int>(0xAC);           // v1[43]
	const DWORD flags = R->Stack<DWORD>(0xB8);          // v1[46]
	const auto pShape = R->Stack<SHPReference*>(0xA8);  // v1[42]
	const auto pConvert = R->Stack<char*>(0x60);        // v1[24] - ConvertClass*

	auto pTable = *reinterpret_cast<NewPalData**>(pConvert + 0x178);

	if (!pTable->IsReady(rle))
		pTable->AllocateNewBlitters();

	void* pBlitter = rle
		? static_cast<void*>(pTable->SelectRLE(flags))
		: static_cast<void*>(pTable->Select(flags));

	if (pBlitter)
		R->EAX(reinterpret_cast<DWORD>(pBlitter));

	// -- install the custom blit callback -----------------------------------
	const DWORD family = flags & BlitFlags::FamilyMask;
	if (family == BlitFlags::FamilyCustomPixel || family == BlitFlags::FamilyCustomSpan)
	{
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
	auto pExt = pShape->Ext;

	// BUG (verbatim): pExt->AlphaSHP is read before pExt is tested, so a null
	// ext faults at 0x0C and the "Ext not found" string is unreachable.
	if (!pExt->AlphaSHP)
	{
		Debug::Log("SHPExt not valid. Reason : %s\n",
			pExt ? "Invalid Alpha file" : "Ext not found");
		return;
	}

	RectangleStruct bounds {};
	ShapeCache::Get_Frame_Bounds(pShape, &bounds, frameIdx);

	if (bounds.Width == 0)
		return;

	// Signed short read; see AlphaBlitState.cpp for the note.
	const int stride = pShape->Width;

	auto pPixels = ShapeCache::Get_Frame(pExt->AlphaSHP, frameIdx);

	pAlpha->Stride = stride;
	pAlpha->Base = pPixels + bounds.X + stride * bounds.Y;

	::TlsSetValue(TlsSlot_Blitter, pBlitter); // the blitter, not the cursor
}

ASMJIT_PATCH(ADDR_DrawSHP_GetBlitter, DSurface_DrawSHP_GetSelectedBlitter, SIZE_DrawSHP_GetBlitter)
{
	GET(REGISTERS*, R, R);
	SelectBlitterCommon(R, false);
	return 0;
}

ASMJIT_PATCH(ADDR_DrawSHP_GetRLEBlitter, DSurface_DrawSHP_GetSelectedRLEBlitter, SIZE_DrawSHP_GetRLEBlitter)
{
	GET(REGISTERS*, R, R);
	SelectBlitterCommon(R, true);
	return 0;
}

// 0x10052A20 - Cursor = Base + Stride * Stack(4) + Stack(0)
ASMJIT_PATCH(ADDR_Lock_AdjustHeight, DSurface_DoubleIntersectLock_AdjustHeight, SIZE_Lock_AdjustHeight)
{
	GET(REGISTERS*, R, R);

	const int x = R->Stack<int>(0x0); // VERIFY: clipped X
	const int y = R->Stack<int>(0x4); // VERIFY: clipped Y

	if (auto pCursor = GetArmedCursor())
		pCursor->SeedFrom(x, y);

	return 0;
}

// 0x100529D0 - TLS is only an arm flag; the blitter comes from Stack(0x90)
ASMJIT_PATCH(ADDR_BlitRLE_AdjustHeight, DSurface_BlitWIthRLE_AdjustHeight, SIZE_BlitRLE_AdjustHeight)
{
	GET(REGISTERS*, R, R);

	const int rows = R->Stack<int>(0x28);
	const auto pBlitter = R->Stack<void*>(0x90);

	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
		return 0;

	GetAlphaCursor(pBlitter)->SeedRows(rows);
	return 0;
}

// 0x10052A60 - Cursor += Stride * ECX
ASMJIT_PATCH(ADDR_BlitPlain_AdjustHeight2, DSurface_BlitWithPlain_AdjustHeight2, SIZE_BlitPlain_AdjustHeight2)
{
	GET(REGISTERS*, R, R);

	const int rows = static_cast<int>(R->ECX());

	if (auto pCursor = GetArmedCursor())
		pCursor->SkipRows(rows);

	return 0;
}

// 0x10052AA0 - arm flag again; the blitter comes from EBP
ASMJIT_PATCH(ADDR_BlitRLE_Add, DSurface_BlitWithRLE_Add, SIZE_BlitRLE_Add)
{
	GET(REGISTERS*, R, R);

	// VERIFY: cached `this` vs. a real frame pointer. See AlphaBlitState.cpp.
	const auto pBlitter = reinterpret_cast<void*>(R->EBP());

	if (!::TlsGetValue(TlsSlot_Blitter) || !pBlitter)
		return 0;

	GetAlphaCursor(pBlitter)->NextRow();
	return 0;
}

// 0x10052AE0 - Cursor += Stride; touches no registers at all
ASMJIT_PATCH(ADDR_BlitPlain_Add, DSurface_BlitWithPlain_Add, SIZE_BlitPlain_Add)
{
	if (auto pCursor = GetArmedCursor())
		pCursor->NextRow();

	return 0;
}

// 0x10052B10 - teardown; clears BOTH slots
ASMJIT_PATCH(ADDR_BlitRLE_Off, DSurface_BlitWithRLE_Off, SIZE_BlitRLE_Off)
{
	::TlsSetValue(TlsSlot_Blitter, nullptr);
	::TlsSetValue(TlsSlot_Custom, nullptr);
	return 0;
}

#endif