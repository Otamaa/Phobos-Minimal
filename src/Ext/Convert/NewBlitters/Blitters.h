#pragma once

#include "../Blitters/Blitter.h"
#include <Base/Always.h>

// ============================================================================
//  Blitters.h
//
//  Blitter hierarchies and the ConvertExt::ExtData blitter table, from
//  AllocateNewBlitters (sub_1004B710), sub_1004C270 and the .rdata vtables.
//
//  TWO INDEPENDENT HIERARCHIES, exactly like vanilla's Blitter / RLEBlitter:
//
//      BlitterCore                    RLEBlitterCore   (RLEBlitterCoreTemplate<T>)
//        -> Blitter<T>                  -> RLEBlitterTemplate<T>
//          -> ...AlphaZReadCore<T>        -> RLE...AlphaZReadCore<T>
//            -> ...AlphaZRead<T,N>          -> RLE...AlphaZRead<T,N>
//
//  They share no base. The .rdata proves it: BlitterCore's vtable has 5 slots
//  (dtor + Blit_Copy/_Tinted + Blit_Move/_Tinted) while RLEBlitterCoreTemplate
//  has 3 (dtor + Blit_Copy/_Tinted), and AllocateNewBlitters never writes
//  BlitterCore's vtable into an RLE object.
//
//  The DATA layout of the two roots is identical - AllocateNewBlitters does the
//  same three writes for both - but they are declared separately here rather
//  than sharing a base, so neither hierarchy can be passed where the other is
//  expected.
// ============================================================================

// Plain family root. Vtable: dtor, Blit_Copy, Blit_Copy_Tinted,
//                            Blit_Move, Blit_Move_Tinted.
class BlitterCore : public Blitter {
	/* +0x04 */ unsigned short* PaletteData;
	/* Confirmed by use: `this[2] + (level << 9)` indexes Table[level], a
		256-entry WORD row, so Table sits at offset 0 of the remapper. */
	/* +0x08 */ AlphaLightingRemapClass* AlphaRemapper;
	/* RESOLVED: the Universal blitters never read this. Their blit body
		touches only +0x04 and +0x08 - they blend by weighted channel
		spreading, not vanilla's `Mask & (*dest >> 1)`, so the hard-zero is
		harmless there. VERIFY still open for Add / Multiply / DoubleMultiply
		Luna, which may be the mask-style blenders. */
	/* +0x0C */ WORD Mask;
	/* +0x0E */ WORD Padding_0E;
};

// RLE family root. Vtable: dtor, Blit_Copy, Blit_Copy_Tinted. No Move pair.
class RLEBlitterCore : public RLEBlitter {
	/* +0x04 */ unsigned short* PaletteData;
	/* Confirmed by use: `this[2] + (level << 9)` indexes Table[level], a
		256-entry WORD row, so Table sits at offset 0 of the remapper. */
	/* +0x08 */ AlphaLightingRemapClass* AlphaRemapper;
	/* RESOLVED: the Universal blitters never read this. Their blit body
		touches only +0x04 and +0x08 - they blend by weighted channel
		spreading, not vanilla's `Mask & (*dest >> 1)`, so the hard-zero is
		harmless there. VERIFY still open for Add / Multiply / DoubleMultiply
		Luna, which may be the mask-style blenders. */
	/* +0x0C */ WORD Mask;
	/* +0x0E */ WORD Padding_0E;
};

// ---------------------------------------------------------------------------
//  Field lens. The shared 0x10 prefix is identical for both hierarchies, so
//  one accessor serves either. Use this where you need PaletteData /
//  AlphaRemapper / Mask off an object you only have as a Blitter* - or off a
//  raw pointer the hooks pulled out of TLS.
//
//  A function, not a base class, for exactly the reason in the block above.
// ---------------------------------------------------------------------------
inline BlitterCore* GetBlitterFields(Blitter* pBlitter)
{
	return reinterpret_cast<BlitterCore*>(pBlitter);
}

inline RLEBlitterCore* GetBlitterFields(RLEBlitter* pBlitter)
{
	return reinterpret_cast<RLEBlitterCore*>(pBlitter);
}

// FIX: restored. These are the only thing tying the overlay to the 0x10 the
// allocator actually requests - drop them and a stray field or padding change
// goes unnoticed until it corrupts the heap at runtime.
static_assert(sizeof(BlitterCore) == 0x10, "must match operator new(0x10)");
static_assert(sizeof(RLEBlitterCore) == 0x10, "must match operator new(0x10)");
