#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransLucent25ZReadWrite)
{
public:
	inline explicit BlitTransLucent25ZReadWrite(T* data, WORD mask) noexcept
	{
		PaletteData = data;
		Mask = mask;
	}

	virtual ~BlitTransLucent25ZReadWrite() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		if (len < 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		while (len--)
		{
			WORD& zbufv = *zbuf++;
			if (zval < zbufv)
			{
				if (byte idx = *src)
				{
					*dest = (Mask & (*dest >> 2)) + 3 * (Mask & (PaletteData[idx] >> 2));
					zbufv = zval;
				}
			}
			++src;
			++dest;

			ZBuffer::Instance->AdjustPointer(zbuf);
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
	WORD Mask;
};
