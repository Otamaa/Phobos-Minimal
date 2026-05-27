#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTranslucent50NonzeroAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTranslucent50NonzeroAlpha(WORD* data, WORD mask) noexcept
	{
		this->PaletteData = data;
		this->Mask = mask;
	}

	virtual ~BlitTranslucent50NonzeroAlpha() override final = default;

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
		WORD* pPaletteData = this->PaletteData;
		WORD mask = this->Mask;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero8 = _mm_setzero_si128();
			const __m256i zero16 = _mm256_setzero_si256();
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i blendMask = _mm512_set1_epi32(static_cast<int>(mask));

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __mmask16 srcMask = _mm_cmpneq_epu8_mask(srcBytes, zero8);
				const __m256i alphaValues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(abuf));
				const __mmask16 alphaMask = _mm256_cmpneq_epu16_mask(alphaValues, zero16);
				const __mmask16 activeMask = srcMask & alphaMask;

				if (activeMask)
				{
					const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
					__m512i srcColors = _mm512_setzero_si512();
					srcColors = _mm512_mask_i32gather_epi32(srcColors, activeMask, srcIndices, pPaletteData, 2);
					srcColors = _mm512_and_si512(srcColors, low16Mask);

					const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));
					const __m512i destHalf = _mm512_and_si512(_mm512_srli_epi32(destColors, 1), blendMask);
					const __m512i srcHalf = _mm512_and_si512(_mm512_srli_epi32(srcColors, 1), blendMask);
					const __m512i result32 = _mm512_add_epi32(destHalf, srcHalf);
					const __m256i result16 = _mm512_cvtusepi32_epi16(result32);

					_mm256_mask_storeu_epi16(pDest, activeMask, result16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m256i srcIndices = Avx2_Expand8ToEpi32(src);
				const __m256i srcMask32 = _mm256_cmpgt_epi32(srcIndices, zero32);
				const __m256i alphaValues = Avx2_Load8WordAsEpi32(abuf);
				const __m256i alphaMask32 = _mm256_cmpgt_epi32(alphaValues, zero32);
				const __m256i activeMask32 = _mm256_and_si256(srcMask32, alphaMask32);

				if (_mm256_movemask_epi8(activeMask32))
				{
					const __m256i srcColors = Avx2_GatherPaletteWord(srcIndices, pPaletteData);
					const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);
					const __m256i destHalf32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 1), blendMask32);
					const __m256i srcHalf32 = _mm256_and_si256(_mm256_srli_epi32(srcColors, 1), blendMask32);
					const __m256i result32 = _mm256_add_epi32(destHalf32, srcHalf32);
					const __m128i result16 = Avx2_PackU32ToU16(result32);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero16 = _mm_setzero_si128();
			const __m128i blendMask16 = _mm_set1_epi16(static_cast<short>(mask));

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
					break;

				const __m128i srcIndices16 = Sse2_Expand8ToEpi16(src);
				const __m128i srcMask16 = _mm_cmpgt_epi16(srcIndices16, zero16);
				const __m128i alphaValues16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(abuf));
				const __m128i alphaMask16 = _mm_cmpgt_epi16(alphaValues16, zero16);
				const __m128i activeMask16 = _mm_and_si128(srcMask16, alphaMask16);

				if (_mm_movemask_epi8(activeMask16))
				{
					const __m128i srcColors16 = Sse2_GatherPaletteWord(srcIndices16, pPaletteData);
					const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i destHalf16 = _mm_and_si128(_mm_srli_epi16(dest16, 1), blendMask16);
					const __m128i srcHalf16 = _mm_and_si128(_mm_srli_epi16(srcColors16, 1), blendMask16);
					const __m128i result16 = _mm_add_epi16(destHalf16, srcHalf16);
					const __m128i blended16 = Sse2_BlendU16(dest16, result16, activeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// Scalar
		while (len--)
		{
			if (byte idx = *src++)
			{
				if (*abuf)
					*pDest = (mask & (*pDest >> 1)) + (mask & (pPaletteData[idx] >> 1));
			}
			++abuf;
			++pDest;

			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
	WORD* PaletteData;
	WORD Mask;
};
