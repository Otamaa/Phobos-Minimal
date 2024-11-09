#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransRemapXlat)
{
public:
	inline explicit RLEBlitTransRemapXlat(T* remap, T* palette) noexcept
	{
		RemapData = remap;
		PaletteData = palette;
	}

	virtual ~RLEBlitTransRemapXlat() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		auto dest = reinterpret_cast<T*>(dst);

		Process_Pre_Lines<false, false>(dest, src, len, line, zbuf, abuf);

		auto handler = [this](T& dest, byte srcv)
		{
			dest = PaletteData[RemapDest[srcv]];
		};

		Process_Pixel_Datas<false, false>(dest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	T* RemapData;
	T* PaletteData;
};
