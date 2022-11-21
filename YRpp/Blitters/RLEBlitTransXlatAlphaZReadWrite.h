#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransXlatAlphaZReadWrite)
{
public:
	inline explicit RLEBlitTransXlatAlphaZReadWrite(T* data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransXlatAlphaZReadWrite() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		auto dest = reinterpret_cast<T*>(dst);
		auto adata = Lookup_Alpha_Remapper(alvl, AlphaRemapper);

		Process_Pre_Lines<true, true>(dest, src, len, line, zbuf, abuf);

		auto handler = [this](T& dest, byte srcv, int zbase, WORD& zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - zadjustv;
			if (zval < zbufv)
			{
				dest = PaletteData[srcv | adata[abufv]];
				zbufv = zval;
			}
		};

		Process_Pixel_Datas<true, true>(dest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		auto dest = reinterpret_cast<T*>(dst);
		auto adata = Lookup_Alpha_Remapper(alvl, AlphaRemapper);

		Process_Pre_Lines<true, true>(dest, src, len, line, zbuf, abuf);

		auto handler = [this](T& dest, byte srcv, int zbase, WORD zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - zadjustv;
			if (zval < zbufv)
				dest = tint | PaletteData[srcv | adata[abufv]];
		};

		Process_Pixel_Datas<true, true>(dest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}

private:
	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
