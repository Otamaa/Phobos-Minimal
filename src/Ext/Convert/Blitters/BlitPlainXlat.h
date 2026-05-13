#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitPlainXlat, BlitterPixelByteAndWord)
{
public:
	inline explicit BlitPlainXlat(T* data) noexcept
	{
		this->PaletteData = data;
	}

	virtual ~BlitPlainXlat() override final = default;

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
		T* pPaletteData = this->PaletteData;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && std::is_same_v<T, WORD> && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);

			while (len >= ChunkSize)
			{
				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
				const __mmask16 maxIndexMask = _mm_cmpeq_epu8_mask(srcBytes, _mm_set1_epi8(static_cast<char>(0xFF)));
				const __mmask16 gatherMask = static_cast<__mmask16>(~maxIndexMask);

				__m512i srcColors = _mm512_setzero_si512();
				if (gatherMask)
				{
					srcColors = _mm512_mask_i32gather_epi32(srcColors, gatherMask, srcIndices, pPaletteData, 2);
				}

				srcColors = _mm512_and_si512(srcColors, low16Mask);

				if (maxIndexMask)
				{
					srcColors = _mm512_mask_set1_epi32(srcColors, maxIndexMask, static_cast<int>(pPaletteData[255]));
				}

				const __m256i result16 = _mm512_cvtusepi32_epi16(srcColors);
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDest), result16);

				src += ChunkSize;
				pDest += ChunkSize;
				len -= ChunkSize;
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && std::is_same_v<T, WORD> && CompileAvx2)
		{
			constexpr int ChunkSize = 8;

			while (len >= ChunkSize)
			{
				const __m256i srcIndex32 = Avx2_Expand8ToEpi32(src);
				const __m256i srcColor32 = Avx2_GatherPaletteWord(srcIndex32, pPaletteData);
				const __m128i result16 = Avx2_PackU32ToU16(srcColor32);

				_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

				src += ChunkSize;
				pDest += ChunkSize;
				len -= ChunkSize;
			}
		}

		// Scalar
		while (len--)
		{
			*pDest++ = pPaletteData[*src++];
		}
	}
	T* PaletteData;
};
