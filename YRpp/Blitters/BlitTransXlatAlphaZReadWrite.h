#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransXlatAlphaZReadWrite)
{
public:
	inline explicit BlitTransXlatAlphaZReadWrite(T * data, int shadecount) noexcept
	{
		PaletteData = data;
		AlphaRemapper = AlphaLightingRemapClass::FindOrAllocate(shadecount);
	}

	virtual ~BlitTransXlatAlphaZReadWrite() override final = default;

	virtual void Blit_Copy(void* dst, byte * src, int len, int zval, WORD * zbuf, WORD * abuf, int alvl, int warp) override final
	{
		if (len < 0)
			return;

		auto dest = reinterpret_cast<T*>(dst);
		auto adata = Lookup_Alpha_Remapper(alvl, AlphaRemapper);

		while (len--)
		{
			WORD zbufv = *zbuf++;
			if (zval < zbufv)
			{
				if (byte idx = *src++)
					*dest = PaletteData[idx | adata[*abuf]];
			}

			++abuf;
			++dest;

			ZBuffer::Instance->AdjustPointer(zbuf);
			ABuffer::Instance->AdjustPointer(abuf);
		}
	}

	virtual void Blit_Copy_Tinted(void* dst, byte * src, int len, int zval, WORD * zbuf, WORD * abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move(void* dst, byte * src, int len, int zval, WORD * zbuf, WORD * abuf, int alvl, int warp)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, byte * src, int len, int zval, WORD * zbuf, WORD * abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

private:
	T* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
