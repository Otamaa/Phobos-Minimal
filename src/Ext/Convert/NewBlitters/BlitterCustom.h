#pragma once

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

// ============================================================================
//  BlitterCallbacks.h
//
//  The two function-pointer slots on BlitterCustom / BlitterCustomRLE.
//
//  CORRECTION: earlier passes through this codebase called these SourceA and
//  SourceB and described them as "source buffer pointers fed from TLS slot B".
//  They are not buffers - they are CALLBACKS. TLS slot B carries a function
//  pointer, and the two blit-flag families choose which of the two hooks the
//  caller is installing:
//
//      family 0x88800000 -> this[4] (+0x10) = PixelBlender, this[5] cleared
//      family 0x88900000 -> this[5] (+0x14) = SpanBlitter,  this[4] cleared
//
//  So they are mutually exclusive by construction, which is why DrawSHP always
//  zeroes the other one.
// ============================================================================

// +0x10. Called once per surviving pixel; the engine still does the z-test,
// the transparency test, the palette lookup and all the pointer walking.
// Returns the value to store.
//
// IDA types the return `int`, but the caller keeps only AX (`mov [ebx], ax`).
using CustomPixelBlender = WORD(__cdecl*)(int source, WORD dest);

// +0x14. Replaces the ENTIRE blit - when set, Blit_Copy forwards and returns
// immediately, doing no walking of its own.
//
// One 12-argument cdecl signature serves both the plain and RLE families;
// unused slots are passed as 0:
//
//        arg          plain                RLE
//    1   dst          dst                  dst
//    2   src          src                  src
//    3   len          len                  len
//    4   line         0                    line
//    5   zbase        zval  (!)            zbase
//    6   zbuf         zbuf                 zbuf
//    7   abuf         abuf                 abuf
//    8   alvl         alvl                 alvl
//    9   zadjust      0                    zadjust
//   10   tint         tint or 0            tint or 0
//   11   remapper     this[2]              this[2]
//   12   paletteData  this[1]              this[1]
//
// VERIFY: the plain family passes its `zval` in the `zbase` slot, so a span
// callback has to know which family invoked it to read argument 5 correctly.
// Nothing in the supplied dumps disambiguates that for the callee.
using CustomSpanBlitter = int(__cdecl*)(
	void* dst, byte* src, int len, int line, int zbase,
	WORD* zbuf, WORD* abuf, int alvl, byte* zadjust, int tint,
	AlphaLightingRemapClass* remapper, void* paletteData);

// ============================================================================
//  BlitterCustom
//
//  Backported from 0x1004B170 (Blit_Copy) and 0x1004B2C0 (Blit_Copy_Tinted).
//
//  Not a blend mode - an extension point. Two escape hatches, installed by the
//  DrawSHP handlers from TLS slot B and mutually exclusive:
//
//    SpanBlitter  (+0x14)  replaces the whole blit. Checked first; if set the
//                          function forwards and returns without walking.
//    PixelBlender (+0x10)  called per surviving pixel. The engine keeps the
//                          z-test, transparency test, palette lookup and
//                          pointer walking; the callback only decides the
//                          output value.
//
//  With NEITHER set the loop still runs - reading, testing and advancing every
//  pointer - but never writes anything. A silent, fairly expensive no-op.
//
//  Note this is one of only two blitters that is 0x18 rather than 0x10, and it
//  has its own destructor (0x10013BA0) rather than the shared 0x10052460.
// ============================================================================

class BlitterCustom final : public Blitter
{
public:
	inline explicit BlitterCustom(WORD* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
		Mask = 0;

		PixelBlender = nullptr;
		SpanBlitter = nullptr;
	}

	virtual ~BlitterCustom() override final = default;

	// -----------------------------------------------------------------------
	virtual void Blit_Copy(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		if (SpanBlitter)
		{
			// The plain family passes zval in the `zbase` slot and zero for
			// line/zadjust. See BlitterCallbacks.h.
			SpanBlitter(dst, src, len, 0, static_cast<WORD>(zval),
				zbuf, abuf, alvl, nullptr, 0, AlphaRemapper, PaletteData);
			return;
		}

		// BUG: the remap level here is ALWAYS 254, whatever alvl is. Two
		// inverted comparisons:
		//
		//   cmp si, ax / cmovb ecx, edx   with si == 0, so the level input is
		//                                 forced to 0 whenever alvl != 0
		//   cmp eax, 0FEh / cmovb eax,ecx forces the result UP to 254 instead
		//                                 of clamping down to it
		//
		// Both are the reverse of what every other blitter does
		// (min(254, 261*max(0,alvl) >> 11)), and they compound: the input is
		// zeroed, then the zero is raised to 254. Blit_Copy_Tinted below and
		// both BlitterCustomRLE entry points get it right, so this is a
		// one-function slip, not a family convention. Preserved.
		constexpr int BrokenLevel = 254;
		const WORD* adata = AlphaRemapper->Table[BrokenLevel];

		Walk(dst, src, len, zval, zbuf, abuf, adata, 0, false);
	}

	// -----------------------------------------------------------------------
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) override final
	{
		if (SpanBlitter)
		{
			SpanBlitter(dst, src, len, 0, static_cast<WORD>(zval),
				zbuf, abuf, alvl, nullptr, tint, AlphaRemapper, PaletteData);
			return;
		}

		// DIFF: this reads alvl ZERO-extended from 16 bits and clamps only the
		// upper end - no max(0, ...) - where BlitterDetail::Lookup_Level
		// sign-extends and clamps both. They agree over the documented
		// [0, 2000] range but not for negative alvl, so Lookup_Level is
		// deliberately not reused here.
		int level = (261 * static_cast<WORD>(alvl)) >> 11;

		if (level > 254)
			level = 254;

		const WORD* adata = AlphaRemapper->Table[level];

		Walk(dst, src, len, zval, zbuf, abuf, adata, tint, true);
	}

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

private:
	void Walk(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf,
		const WORD* adata, WORD tint, bool tinted) const
	{
		// BUG (preserved): `len != 0`, not `len > 0`. Shared by every plain
		// blitter in this DLL.
		if (len == 0)
			return;

		auto dest = static_cast<WORD*>(dst);

		do
		{
			const WORD zbufv = *zbuf++;

			if (zbufv > static_cast<WORD>(zval))
			{
				if (const byte srcv = *src++)
				{
					// Note this advances even when PixelBlender is null - the
					// callback check happens after. Same plain-family
					// behaviour of only advancing on a surviving pixel.
					const WORD abufv = *abuf++;

					WORD colour = PaletteData[srcv | adata[abufv]];

					if (tinted)
						colour |= tint;

					if (PixelBlender)
						*dest = PixelBlender(colour, *dest);
				}
			}

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);

			++dest;
		}
		while (--len);
	}

	// Layout mirrors the DLL object: 0x18 total.
	WORD* PaletteData;                  // +0x04
	AlphaLightingRemapClass* AlphaRemapper; // +0x08
	WORD Mask;                          // +0x0C, never read
	WORD Padding;

public:
	CustomPixelBlender PixelBlender;    // +0x10, family 0x88800000
	CustomSpanBlitter SpanBlitter;      // +0x14, family 0x88900000
};

static_assert(sizeof(BlitterCustom) == 0x18, "must match operator new(0x18)");