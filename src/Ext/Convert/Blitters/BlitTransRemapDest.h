#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransRemapDest, BlitterPixelByteOnly)
{
public:
	inline explicit BlitTransRemapDest(BYTE* data) noexcept
	{
		this->RemapDest = data;
	}

	virtual ~BlitTransRemapDest() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

private:
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		if (len < 0)
			return;

		BYTE* pDest = reinterpret_cast<BYTE*>(dst);
		BYTE* pRemapDest = this->RemapDest;

		// Scalar
		while (len--)
		{
			if (*src++)
				*pDest = pRemapDest[*pDest];
			++pDest;
		}
	}
	BYTE* RemapDest;
};
