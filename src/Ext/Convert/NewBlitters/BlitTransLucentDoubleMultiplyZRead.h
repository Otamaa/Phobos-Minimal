#pragma once


// ============================================================================
//  BlitTransLucentDoubleMultiplyZRead<T>
//
//  Backported from 0x1004F330 (Blit_Copy) and 0x1004F110 (Blit_Copy_Tinted).
//
//  Per channel:
//
//      result = min(255, 2 * (dst * src / 255))
//
//  Multiply with the result doubled - the name is literal. Mid-grey over
//  mid-grey stays put, anything brighter blows out. See BlitterMath.h for why
//  the divide has to happen before the doubling.
//
//  Same packed-888 representation and the same exact /255 as Multiply and
//  Luna; only the combine differs.
//
//  ON THE IDA OUTPUT
//  -----------------
//  The pseudocode inlines the whole Expand888 chain into each of the three
//  channel expressions, so `v31`, `v21` and `v22` each appear to re-derive the
//  source colour from scratch. The asm computes src888 once at 0x1004F41B and
//  keeps it in EBX. Nothing is recomputed.
// ============================================================================


#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"


template<typename T>
class BlitTransLucentDoubleMultiplyZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentDoubleMultiplyZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);

		// The DLL allocator also stores a WORD 0 at +0x0C - the `Mask` field.
		// Neither function reads it. Not declared here.
	}

	virtual ~BlitTransLucentDoubleMultiplyZRead() override final = default;

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
						BlitterDetail::DoubleMultiply888(
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