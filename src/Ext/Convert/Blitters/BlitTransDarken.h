#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransDarken, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransDarken(WORD mask) noexcept
	{
		this->Mask = mask;
	}

	virtual ~BlitTransDarken() override final = default;

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

		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD mask = this->Mask;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 32;
			const __m256i zero = _mm256_setzero_si256();
			const __m512i vecMask = _mm512_set1_epi16(static_cast<short>(mask));

			while (len >= ChunkSize)
			{
				const __m256i srcBytes = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src));
				const __mmask32 nonZeroMask = _mm256_cmpneq_epu8_mask(srcBytes, zero);
				const __m512i destValues = _mm512_loadu_si512(reinterpret_cast<const void*>(pDest));
				const __m512i halfValues = _mm512_srli_epi16(destValues, 1);
				const __m512i darkenedValues = _mm512_and_si512(halfValues, vecMask);

				_mm512_mask_storeu_epi16(pDest, nonZeroMask, darkenedValues);

				src += ChunkSize;
				pDest += ChunkSize;
				len -= ChunkSize;
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i darkMask32 = _mm256_set1_epi32(static_cast<int>(mask));

			while (len >= ChunkSize)
			{
				const __m256i srcIndex32 = Avx2_Expand8ToEpi32(src);
				const __m256i activeMask32 = _mm256_cmpgt_epi32(srcIndex32, zero32);
				if (_mm256_movemask_epi8(activeMask32))
				{
					const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);
					const __m256i result32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 1), darkMask32);
					const __m128i result16 = Avx2_PackU32ToU16(result32);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				len -= ChunkSize;
			}
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			const __m128i zero16 = _mm_setzero_si128();
			const __m128i darkMask16 = _mm_set1_epi16(static_cast<short>(mask));

			while (len >= ChunkSize)
			{
				const __m128i src16 = Sse2_Expand8ToEpi16(src);
				const __m128i activeMask16 = _mm_cmpgt_epi16(src16, zero16);
				if (_mm_movemask_epi8(activeMask16))
				{
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i result16 = _mm_and_si128(_mm_srli_epi16(oldValue16, 1), darkMask16);
					const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, activeMask16);
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
			if (*src++)
				*pDest = mask & (*pDest >> 1);
			++pDest;
		}
	}
	WORD Mask;
};
