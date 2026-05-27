#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransXlatAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransXlatAlpha(WORD* data, int shadecount) noexcept
	{
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransXlatAlpha() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(false, true, pDest, src, len, line, zbuf, abuf);

		// Scalar
		auto handler = [pPaletteData, pAData](WORD& dest, byte srcv, WORD abufv)
		{
			dest = pPaletteData[srcv | pAData[abufv]];
		};

		RLE_PROCESS_PIXEL_DATAS(false, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
