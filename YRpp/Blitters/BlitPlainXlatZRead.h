#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitPlainXlatZRead)
{
public:
	OPTIONALINLINE explicit BlitPlainXlatZRead(T* data) noexcept
	{
		PaletteData = data;
	}

	virtual ~BlitPlainXlatZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		if (len < 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);

		while (len--)
		{
			WORD zbufv = *zbuf++;
			if (zval < zbufv)
				*dest = PaletteData[*src];
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
};
