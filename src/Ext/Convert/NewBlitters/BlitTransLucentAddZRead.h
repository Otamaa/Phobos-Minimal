#pragma once

// ============================================================================
//  BlitTransLucentAddZRead<T>
//
//  Backported from 0x1004FB80 (Blit_Copy) and 0x1004F940 (Blit_Copy_Tinted).
//
//  Despite the "TransLucent" in the name there is no weighting at all: both
//  pixels are expanded from 565 to byte-aligned 888, added per channel with
//  saturation, and packed back. That is the whole operation.
//
//      R = min(255, dstR + srcR)     and likewise G, B
//
//  The alpha remap table is still consulted, but only where every other
//  blitter uses it - to turn `abuf` into a palette bank for the lighting
//  level. It plays no part in the combine.
//
//  ON THE IDA OUTPUT
//  -----------------
//  The pseudocode for these two is a thicket of v36/v37/v38/v39 and nested
//  conditionals. None of it is real. The compiler emitted `src888 >> 8` FOUR
//  times into four stack slots (0x1004FC90..0x1004FCB4), then built a cmov
//  chain to choose between them - so every branch selects the same value.
//  `cmovbe edi, esi` is likewise a no-op: esi and edi both already hold
//  src888 >> 16. Strip the dead selection and three saturating adds remain.
// ============================================================================

#include "../Blitters/Blitter.h"
#include "BlitterDetail.h"

template<typename T>
class BlitTransLucentAddZRead final : public Blitter
{
public:
	inline explicit BlitTransLucentAddZRead(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~BlitTransLucentAddZRead() override final = default;

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

		// BUG (preserved): `len != 0`, not `len > 0`. A negative length runs
		// the do/while ~4 billion times. Shared by every plain blitter here.
		if (len == 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		do
		{
			const WORD zbufv = *zbuf++;

			// Unsigned 16-bit compare - zval is effectively truncated.
			if (zbufv > static_cast<WORD>(zval))
			{
				// src advances only on a z-pass, as in vanilla.
				if (const byte srcv = *src++)
				{
					// abuf advances only on a drawn pixel while dest advances
					// every iteration. Every plain blitter in this DLL does
					// this; every RLE one keeps them in step.
					const WORD abufv = *abuf++;

					WORD colour = PaletteData[srcv | adata[abufv]];

					if constexpr (Tinted)
					{
						// The original ORs the tint into the palette result
						// before expanding, as a WORD.
						colour |= tint;
					}

					*dest = BlitterDetail::Pack565(
						BlitterDetail::AddSaturate888(
							BlitterDetail::Expand888(static_cast<WORD>(*dest)),
							BlitterDetail::Expand888(colour)));
				}
			}

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);

			++dest;
		}
		while (--len);

		// NOT PORTED: both originals leave the zbuf cursor in EAX and IDA
		// types them `signed int`. The vtable slot is void; nothing reads it.
	}

	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
