#pragma once

// ============================================================================
//  BlitTransLucentLunaZRead<T>
//
//  Backported from 0x1004EE90 (Blit_Copy) and 0x1004EC00 (Blit_Copy_Tinted).
//
//  Per channel:
//
//      result = src*src/255 + dst*(255 - src)/255
//
//  which is source-over compositing with the source's own value serving as its
//  alpha, colour already premultiplied by it - `s*s + d*(1-s)` normalised. A
//  bright source pixel is nearly opaque, a dark one nearly invisible, so this
//  is a glow/light blend rather than a uniform translucency. See BlitterMath.h.
//
//  Same packed-888 representation and the same exact /255 as the Multiply
//  family; only the combine differs.
//
//  ON THE IDA OUTPUT
//  -----------------
//  As with Multiply, the compiler keeps a mutating copy of the expanded
//  destination alongside a pristine one and reads red from the pristine copy.
//  Every channel uses the ORIGINAL destination value.
//
//  The pseudocode's `v22 = BYTE1(v20) & 0xF8` looks like it re-derives red
//  from the raw palette word rather than from the expanded source - it is the
//  same quantity either way, and the asm just reads src888's low byte.
// ============================================================================

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T>
class BlitTransLucentLunaZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentLunaZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);

		// The DLL allocator also stores a WORD 0 at +0x0C - the `Mask` field.
		// Neither function reads it. Not declared here.
	}

	virtual ~BlitTransLucentLunaZRead() override final = default;

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
						BlitterDetail::Luna888(
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
		// IDA types them `signed int`. The vtable slot is void.
	}

	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};