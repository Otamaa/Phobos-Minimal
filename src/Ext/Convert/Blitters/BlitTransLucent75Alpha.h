#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransLucent75Alpha, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransLucent75Alpha(WORD* data, WORD mask, int shadecount) noexcept
	{
		this->PaletteData = data;
		this->Mask = mask;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~BlitTransLucent75Alpha() override final = default;

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
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;
		WORD mask = this->Mask;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero = _mm_setzero_si128();
			const __m256i alphaMaxIndex16 = _mm256_set1_epi16(0x00FF);
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i paletteMaxIndex = _mm512_set1_epi32(0xFFFF);
			const __m512i blendMask = _mm512_set1_epi32(static_cast<int>(mask));

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __mmask16 activeMask = _mm_cmpneq_epu8_mask(srcBytes, zero);

				if (activeMask)
				{
					const __m256i abufValues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(abuf));
					const __m512i alphaTableIndices = _mm512_cvtepu16_epi32(abufValues);
					const __mmask16 alphaMaxIndexMask = _mm256_cmpeq_epu16_mask(abufValues, alphaMaxIndex16);
					const __mmask16 alphaGatherMask = activeMask & ~alphaMaxIndexMask;

					__m512i alphaValues = _mm512_setzero_si512();
					if (alphaGatherMask)
					{
						alphaValues = _mm512_mask_i32gather_epi32(alphaValues, alphaGatherMask, alphaTableIndices, pAData, 2);
					}

					alphaValues = _mm512_and_si512(alphaValues, low16Mask);

					const __mmask16 alphaFillMask = activeMask & alphaMaxIndexMask;
					if (alphaFillMask)
					{
						alphaValues = _mm512_mask_set1_epi32(alphaValues, alphaFillMask, static_cast<int>(pAData[255]));
					}

					const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
					const __m512i paletteIndices = _mm512_or_si512(srcIndices, alphaValues);
					const __mmask16 paletteMaxIndexMask = _mm512_cmpeq_epi32_mask(paletteIndices, paletteMaxIndex);
					const __mmask16 paletteGatherMask = activeMask & ~paletteMaxIndexMask;

					__m512i srcColors = _mm512_setzero_si512();
					if (paletteGatherMask)
					{
						srcColors = _mm512_mask_i32gather_epi32(srcColors, paletteGatherMask, paletteIndices, pPaletteData, 2);
					}

					srcColors = _mm512_and_si512(srcColors, low16Mask);

					const __mmask16 paletteFillMask = activeMask & paletteMaxIndexMask;
					if (paletteFillMask)
					{
						srcColors = _mm512_mask_set1_epi32(srcColors, paletteFillMask, static_cast<int>(pPaletteData[0xFFFF]));
					}

					const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));

					const __m512i destQuarter = _mm512_and_si512(_mm512_srli_epi32(destColors, 2), blendMask);
					const __m512i srcQuarter = _mm512_and_si512(_mm512_srli_epi32(srcColors, 2), blendMask);
					const __m512i result32 = _mm512_add_epi32(_mm512_add_epi32(_mm512_add_epi32(destQuarter, destQuarter), destQuarter), srcQuarter);
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
			const __m256i low16Mask32 = _mm256_set1_epi32(0xFFFF);
			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
					break;

				const __m256i srcIndex32 = Avx2_Expand8ToEpi32(src);
				const __m256i activeMask32 = _mm256_cmpgt_epi32(srcIndex32, zero32);
				if (_mm256_movemask_epi8(activeMask32))
				{
					__m256i alphaIndex32 = Avx2_Load8WordAsEpi32(abuf);
					alphaIndex32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(pAData), alphaIndex32, 2);
					alphaIndex32 = _mm256_and_si256(alphaIndex32, low16Mask32);

					const __m256i paletteIndex32 = _mm256_or_si256(srcIndex32, alphaIndex32);
					__m256i srcColor32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(pPaletteData), paletteIndex32, 2);
					srcColor32 = _mm256_and_si256(srcColor32, low16Mask32);

					const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);
					const __m256i destQuarter32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 2), blendMask32);
					const __m256i srcQuarter32 = _mm256_and_si256(_mm256_srli_epi32(srcColor32, 2), blendMask32);
					const __m256i result32 = _mm256_add_epi32(_mm256_add_epi32(_mm256_add_epi32(destQuarter32, destQuarter32), destQuarter32), srcQuarter32);

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

				const __m128i srcIndex16 = Sse2_Expand8ToEpi16(src);
				const __m128i activeMask16 = _mm_cmpgt_epi16(srcIndex16, zero16);
				if (_mm_movemask_epi8(activeMask16))
				{
					const __m128i alphaIndex16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(abuf));

					alignas(16) WORD alphaIndexArray[ChunkSize];
					_mm_store_si128(reinterpret_cast<__m128i*>(alphaIndexArray), alphaIndex16);

					alignas(16) WORD alphaValueArray[ChunkSize];
					for (int lane = 0; lane < ChunkSize; ++lane)
					{
						const WORD index = alphaIndexArray[lane];
						alphaValueArray[lane] = pAData[index == 0x00FF ? 0x00FF : index];
					}

					const __m128i alphaValue16 = _mm_load_si128(reinterpret_cast<const __m128i*>(alphaValueArray));
					const __m128i paletteIndex16 = _mm_or_si128(srcIndex16, alphaValue16);
					const __m128i srcColor16 = Sse2_GatherPaletteWord(paletteIndex16, pPaletteData);
					const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));

					const __m128i destQuarter16 = _mm_and_si128(_mm_srli_epi16(dest16, 2), blendMask16);
					const __m128i srcQuarter16 = _mm_and_si128(_mm_srli_epi16(srcColor16, 2), blendMask16);
					const __m128i result16 = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(destQuarter16, destQuarter16), destQuarter16), srcQuarter16);
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
				*pDest = 3 * (mask & (*pDest >> 2)) + (mask & (pPaletteData[idx | pAData[*abuf]] >> 2));
			++pDest;
			++abuf;

			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
	WORD* PaletteData;
	WORD Mask;
	AlphaLightingRemapClass* AlphaRemapper;
};
