#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransLucent50ZReadWrite)
{
public:
	OPTIONALINLINE explicit RLEBlitTransLucent50ZReadWrite(T* data, WORD mask) noexcept
	{
		PaletteData = data;
		Mask = mask;
	}

	virtual ~RLEBlitTransLucent50ZReadWrite() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		auto dest = reinterpret_cast<T*>(dst);

		Process_Pre_Lines<true, false>(dest, src, len, line, zbuf, abuf);

		auto handler = [this](T& dest, byte srcv, int zbase, WORD& zbufv, byte zadjustv)
		{
			int zval = zbase - zadjustv;
			if (zval < zbufv)
			{
				dest = (Mask & (dest >> 1)) + (Mask & (PaletteData[srcv] >> 1));
				zbufv = zval;
			}
		};

		Process_Pixel_Datas<true, false>(dest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Copy(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	T* PaletteData;
	WORD Mask;
};
