#pragma once

// ============================================================================
//  BlitTransLucentBufferedAlphaZRead<T>
//
//  Backported from 0x1004FF60 (Blit_Copy) and 0x1004FDC0 (Blit_Copy_Tinted).
//  No vanilla counterpart - none of the 99 stock headers has a Buffered variant.
//
//  THIS IS THE CONSUMER OF THE HOOK CLUSTER
//  ----------------------------------------
//  The very first thing both functions do is `this[4]`, i.e. this+0x10 - the
//  Cursor that AlphaBlitState.cpp's eight hooks maintain. That closes the loop
//  on the whole investigation:
//
//      this+0x04  PaletteData     (`this[1]`, indexed 2 * idx)
//      this+0x08  AlphaRemapper   (`this[2] + (level << 9)`)
//      this+0x10  Cursor          (`this[4]`, walked one byte per pixel)
//
//  Where an ordinary alpha blitter takes its per-pixel opacity from the global
//  ABuffer via the `abuf` argument, this one reads it from a PER-SHAPE alpha
//  image - the .APH SHP attached by SHPReference_CTOR. `abuf` is still used,
//  but only to pick the palette bank; the actual blend weight comes from the
//  cursor.
//
//  If the cursor is null the entire function returns immediately, so an
//  unarmed blitter draws NOTHING rather than falling back to opaque.
// ============================================================================

#include "../Blitters/Blitter.h"
// FIX: this file calls BlitterDetail::Lookup_Level / Spread / Pack but only
// included AlphaCursor.h - it compiled solely on include order.
#include "BlitterDetail.h"
#include "AlphaCursor.h"

template<typename T>
class BlitTransLucentBufferedAlphaZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentBufferedAlphaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
		Mask = 0;

		Alpha = AlphaCursor { nullptr, nullptr, 0 };
	}

	virtual ~BlitTransLucentBufferedAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		Blit<false>(dst, src, len, zval, zbuf, abuf, alvl, warp, 0);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		Blit<true>(dst, src, len, zval, zbuf, abuf, alvl, warp, tint);
	}

	// Shared 0x1004B400 / 0x1004B430, as for every other plain blitter.
	virtual void Blit_Move(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		Blit_Copy_Tinted(dst, src, len, zval, zbuf, abuf, alvl, 0, tint);
	}

	// -----------------------------------------------------------------------
	//  0x1004D080 - the sixth vtable slot, and the whole body is:
	//      this[4] = a2; return a2;
	//
	//  A plain absolute setter for Cursor. IDA types the return as int only
	//  because EAX retains the argument, which is what a void setter compiles
	//  to; nothing reads it.
	//
	//  The same address fills slot 3 on RLEBlitTransLucentBufferedAlphaZRead.
	//  The two classes are in unrelated hierarchies, so one shared body works
	//  purely because Cursor sits at +0x10 in both - either a genuine shared
	//  mixin or COMDAT folding of two identical definitions.
	//
	//  Note it sets ONLY Cursor. Base and Stride have no accessor; the DrawSHP
	//  hooks write +0x14 and +0x18 directly.
	// -----------------------------------------------------------------------
	//  NOT `override` - vanilla's Blitter declares no such method. This is a
	//  NEW virtual appended after the base's five slots, which is exactly what
	//  .rdata shows: five inherited entries then 0x1004D080 in slot 5.
	virtual void Set_Alpha_Cursor(byte* cursor)
	{
		Alpha.Cursor = cursor;
	}

private:
	// -----------------------------------------------------------------------
	//  The alpha byte is a PER-PIXEL SOURCE weight out of 32:
	//
	//      alpha  = alphaByte >> 3                  (0 .. 31)
	//      result = (alpha*src + (32-alpha)*dest) / 32
	//
	//  Note the inversion relative to the Universal family, where the template
	//  parameter N is the DESTINATION weight. Here N would be 32 - alpha.
	//
	//  The original computes 2*(32-alpha)*Spread(dest) + 2*alpha*Spread(src)
	//  and shifts by 6/11/16; the *2 cancels against the extra shift, so this
	//  is Pack<5> over the unscaled sum.
	// -----------------------------------------------------------------------
	static constexpr DWORD AlphaSteps = 32;
	static constexpr int AlphaShift = 5; // Log2(AlphaSteps)

	template<bool Tinted>
	void Blit(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		// `warp` is never referenced. Signature compatibility only.
		(void)warp;

		// The whole body is inside `if (this[4])`. An unarmed blitter is a
		// no-op, not a fallback.
		if (!Alpha.Cursor)
			return;

		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		// BUG (preserved): `len != 0`, not `len > 0` - same runaway on a
		// negative length as BlitTransLucentUniversalAlphaZRead.
		if (len == 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		// BUGFIX to an earlier draft of this file, which walked the member
		// directly. The original loads this[4] into EDI once and walks THAT;
		// there is no `mov [ebx+10h], edi` anywhere in 0x1004FF60..0x100500F9,
		// so the member is left exactly where the hooks put it. Walk a local.
		byte* cursor = Alpha.Cursor;

		// SUSPECT: the two variants disagree on BOTH of these, and neither
		// difference looks deliberate.
		//
		//   Blit_Copy         alpha = (         *cursor) >> 3, cutoff < 2
		//   Blit_Copy_Tinted  alpha = ((byte)~ *cursor) >> 3, cutoff < 3
		//
		// The `not al` in the tinted path inverts the opacity outright: an
		// alpha byte of 0x00 becomes fully opaque and 0xFF becomes invisible.
		// One of the two is reading the .APH convention backwards. Preserved
		// verbatim because fixing it guesses which.
		constexpr DWORD MinVisible = Tinted ? 3u : 2u;

		do
		{
			const WORD zbufv = *zbuf++;

			// Unsigned 16-bit compare, as in the Universal blitters.
			bool advanceRings = true;

			if (zbufv > static_cast<WORD>(zval))
			{
				DWORD alpha = *cursor++;

				if constexpr (Tinted)
					alpha = static_cast<byte>(~alpha);

				alpha >>= 3;

				const byte srcv = *src++;

				if (alpha < MinVisible)
				{
					// SUSPECT: this is the only exit from the body that skips
					// the two ring-wrap checks. Several consecutive
					// near-transparent pixels can therefore push zbuf more
					// than one buffer length past its end, and the single
					// subtraction on the next visible pixel will not recover
					// it. Preserved.
					advanceRings = false;
				}
				else if (srcv)
				{
					const WORD destv = static_cast<WORD>(*dest);

					// abuf advances ONLY on a fully drawn pixel, while dest
					// advances every iteration - the same desynchronisation
					// the plain Universal blitter has. The RLE family keeps
					// them in step.
					const WORD abufv = *abuf++;

					WORD colour = PaletteData[srcv | adata[abufv]];

					if constexpr (Tinted)
						colour |= tint;

					if (alpha > 30)
					{
						// Fully opaque: stored without blending.
						*dest = colour;
					}
					else
					{
						*dest = BlitterDetail::Pack<AlphaShift>(
							  alpha * BlitterDetail::Spread(colour)
							+ (AlphaSteps - alpha) * BlitterDetail::Spread(destv));
					}
				}
			}

			if (advanceRings)
			{
				ZBuffer::Instance->AdjustPointer(zbuf);
				ABuffer::Instance->AdjustPointer(abuf);
			}

			++dest;
		}
		while (--len);
	}

	// Layout mirrors the DLL object exactly: PaletteData +0x04, AlphaRemapper
	// +0x08, Mask +0x0C (never read), then the cursor triple at +0x10.
	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
	WORD Mask;
	WORD Padding;

public:
	// Driven from outside by the hook cluster; see AlphaBlitState.cpp and
	// AlphaCursor.h. Blit_Copy only ever READS it - the walk happens on a
	// local - so the hooks' relative `Cursor += Stride` per row is correct and
	// does not compound.
	AlphaCursor Alpha;
};
