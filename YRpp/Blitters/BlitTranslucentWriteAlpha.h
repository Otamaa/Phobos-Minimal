#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTranslucentWriteAlpha)
{
public:
	inline explicit BlitTranslucentWriteAlpha(T* data) noexcept
	{
		PaletteData = data;
	}

	virtual ~BlitTranslucentWriteAlpha() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		if (len < 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		while (len--)
		{
			if (byte idx = *src++)
			{
				auto srcv = PaletteData[idx];
				WORD alpha = *abuf;
				WORD resalpha = 255 - alpha;
				if (alpha == 255)
					alpha = 256;

				RGBClass fg { srcv, true };
				RGBClass bg { *dest, true };

				RGBClass clr
				{
					(fg.Red * alpha + bg.Red * resalpha) >> 8,
					(fg.Green * alpha + bg.Green * resalpha) >> 8,
					(fg.Blue * alpha + bg.Blue * resalpha) >> 8
				};

				*dest = static_cast<WORD>(clr.ToInt());
			}
			++abuf;
			++dest;

			ABuffer::AdjustPointer(abuf);
		}
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

private:
	T* PaletteData;
};
