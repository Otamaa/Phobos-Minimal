#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransLucent75Alpha)
{
public:
	inline explicit RLEBlitTransLucent75Alpha(T* data, WORD mask, int shadecount) noexcept
	{
		PaletteData = data;
		Mask = mask;
		AlphaRemapper = AlphaLightingRemapClass::FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransLucent75Alpha() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		auto dest = reinterpret_cast<T*>(dst);
		auto adata = Lookup_Alpha_Remapper(alvl, AlphaRemapper);

		Process_Pre_Lines<false, true>(dest, src, len, line, zbuf, abuf);

		auto handler = [this](T& dest, byte srcv, WORD abufv)
		{
			dest = 3 * (Mask & (dest >> 2)) + (Mask & (PaletteData[srcv | adata[*abuf]] >> 2));
		};

		Process_Pixel_Datas<false, true>(dest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	T* PaletteData;
	WORD Mask;
	AlphaLightingRemapClass* AlphaRemapper;
};
