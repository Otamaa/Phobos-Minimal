#pragma once

// ============================================================================
//  BlitTransLucentMultiplyZRead<T>
//
//  Backported from 0x1004F750 (Blit_Copy) and 0x1004F550 (Blit_Copy_Tinted).
//
//  Modulate blending. Both pixels are expanded from 565 to byte-aligned 888,
//  multiplied per channel and divided by 255, then packed back:
//
//      R = dstR * srcR / 255      and likewise G, B
//
//  Same packed-888 representation as the Add family, different combine. As
//  there, the remap table only turns `abuf` into a palette bank for the
//  lighting level; it takes no part in the combine.
//
//  ON THE DIVISION
//  ---------------
//  `mov eax, 80808081h / mul / shr edx, 7`, three times over, is MSVC's
//  magic-number sequence for an exact unsigned divide by 255 - not the `>> 8`
//  approximation. The two rarely disagree once the result is requantised to
//  565 (14 of 1024 five-bit channel pairs, 200 of 4096 six-bit ones), but the
//  original divides exactly, so this does too. See BlitterMath.h.
//
//  Separately: Expand888 does not replicate bits, so 0xFFFF expands to
//  248/252/248 and white modulated by white yields 0xF7DE, not 0xFFFF. That
//  is vanilla behaviour.
//
//  ON THE IDA OUTPUT
//  -----------------
//  The compiler keeps two copies of the expanded destination - one it mutates
//  channel by channel as results are produced (var_4), one left pristine
//  (var_24) - and reads the red channel from the pristine copy at the end.
//  That is register pressure, not logic: every channel multiplies the ORIGINAL
//  destination value.
// ============================================================================

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T>
class BlitTransLucentMultiplyZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentMultiplyZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);

		// The DLL allocator also stores a WORD 0 at +0x0C - the `Mask` field.
		// Neither function reads it. Not declared here.
	}

	virtual ~BlitTransLucentMultiplyZRead() override final = default;

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
	template<bool Tinted>
	void Blit(void* dst, byte* src, int len, int zval,
		WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		(void)warp; // never referenced; signature compatibility only

		const WORD* adata = AlphaRemapper->Table[BlitterDetail::Lookup_Level(alvl)];

		// BUG (preserved): `len != 0`, not `len > 0`. Shared by every plain
		// blitter in this DLL.
		if (len == 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		do
		{
			const WORD zbufv = *zbuf++;

			if (zbufv > static_cast<WORD>(zval))
			{
				if (const byte srcv = *src++)
				{
					// abuf advances only on a drawn pixel - the plain-family
					// pattern; every RLE blitter keeps it in step instead.
					const WORD abufv = *abuf++;

					WORD colour = PaletteData[srcv | adata[abufv]];

					if constexpr (Tinted)
						colour |= tint;

					*dest = BlitterDetail::Pack565(
						BlitterDetail::Modulate888(
							BlitterDetail::Expand888(static_cast<WORD>(*dest)),
							BlitterDetail::Expand888(colour)));
				}
			}

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);

			++dest;
		}
		while (--len);

		// NOT PORTED: both originals leave a stale ABuffer value in EAX and
		// IDA types them `signed int`. The vtable slot is void; the value is
		// whatever the last wrap check happened to compute.
	}

	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};