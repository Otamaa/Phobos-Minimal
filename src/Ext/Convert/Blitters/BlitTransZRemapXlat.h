#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransZRemapXlat, BlitterPixelByteAndWord)
{
public:
	inline explicit BlitTransZRemapXlat(byte** remapData, T* data) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
	}

	virtual ~BlitTransZRemapXlat() override final = default;

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

		T* pDest = reinterpret_cast<T*>(dst);
		byte** ppRemapData = this->RemapData;
		T* pPaletteData = this->PaletteData;

		// SSE2
		if constexpr (Level == Simd::Level::SSE2 && std::is_same_v<T, WORD>)
		{
			constexpr int ChunkSize = 8;
			const __m128i zero16 = _mm_setzero_si128();

			while (len >= ChunkSize)
			{
				const __m128i srcIndex16 = Sse2_Expand8ToEpi16(src);
				const __m128i activeMask16 = _mm_cmpgt_epi16(srcIndex16, zero16);
				if (_mm_movemask_epi8(activeMask16))
				{
					alignas(16) WORD remapIndexArray[ChunkSize];
					const byte* pRemap = *ppRemapData;
					for (int lane = 0; lane < ChunkSize; ++lane)
					{
						remapIndexArray[lane] = pRemap[src[lane]];
					}

					const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remapIndexArray));
					const __m128i srcColor16 = Sse2_GatherPaletteWord(remapIndex16, pPaletteData);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Sse2_BlendU16(oldValue16, srcColor16, activeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				len -= ChunkSize;
			}
		}

		// Scalar
		while (len--)
		{
			if (byte idx = *src++)
				*pDest = pPaletteData[(*ppRemapData)[idx]];
			++pDest;
		}
	}
	byte** RemapData;
	T* PaletteData;
};
