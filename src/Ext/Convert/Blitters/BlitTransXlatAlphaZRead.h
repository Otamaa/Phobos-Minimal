#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransXlatAlphaZRead, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransXlatAlphaZRead(WORD* data, int shadecount) noexcept
	{
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~BlitTransXlatAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl<false>(dst, src, len, zval, zbuf, abuf, alvl, warp, 0);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl<true>(dst, src, len, zval, zbuf, abuf, alvl, warp, tint);
	}

	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl<false>(dst, src, len, zval, zbuf, abuf, alvl, warp, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl<true>(dst, src, len, zval, zbuf, abuf, alvl, warp, tint);
	}

private:
	template<bool UseTint>
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		if (len < 0)
			return;

		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero = _mm_setzero_si128();
			const __m256i zvalVec = _mm256_set1_epi16(static_cast<short>(zval));
			const __m256i alphaMaxIndex16 = _mm256_set1_epi16(0x00FF);
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i paletteMaxIndex = _mm512_set1_epi32(0xFFFF);

			__m256i tintVec;
			if constexpr (UseTint)
			{
				tintVec = _mm256_set1_epi16(static_cast<short>(tint));
			}

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				__mmask16 zMask = 0;
				if (zval < 0)
				{
					zMask = static_cast<__mmask16>(0xFFFF);
				}
				else if (zval <= 0xFFFF)
				{
					zMask = _mm256_cmpgt_epu16_mask(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(zbuf)), zvalVec);
				}

				if (zMask)
				{
					const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
					const __mmask16 srcMask = _mm_cmpneq_epu8_mask(srcBytes, zero);
					const __mmask16 activeMask = srcMask & zMask;

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

						__m512i resultColors = _mm512_setzero_si512();
						if (paletteGatherMask)
						{
							resultColors = _mm512_mask_i32gather_epi32(resultColors, paletteGatherMask, paletteIndices, pPaletteData, 2);
						}

						resultColors = _mm512_and_si512(resultColors, low16Mask);

						const __mmask16 paletteFillMask = activeMask & paletteMaxIndexMask;
						if (paletteFillMask)
						{
							resultColors = _mm512_mask_set1_epi32(resultColors, paletteFillMask, static_cast<int>(pPaletteData[0xFFFF]));
						}

						__m256i result16 = _mm512_cvtusepi32_epi16(resultColors);
						if constexpr (UseTint)
						{
							result16 = _mm256_or_si256(result16, tintVec);
						}

						_mm256_mask_storeu_epi16(pDest, activeMask, result16);
					}
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i zvalVec32 = _mm256_set1_epi32(zval);
			__m128i tintVec16;
			if constexpr (UseTint)
			{
				tintVec16 = _mm_set1_epi16(static_cast<short>(tint));
			}

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				__m256i zMask32 = _mm256_setzero_si256();
				if (zval < 0)
				{
					zMask32 = _mm256_set1_epi32(-1);
				}
				else if (zval <= 0xFFFF)
				{
					const __m256i zbuf32 = Avx2_Load8WordAsEpi32(zbuf);
					zMask32 = _mm256_cmpgt_epi32(zbuf32, zvalVec32);
				}

				if (_mm256_movemask_epi8(zMask32))
				{
					const __m256i srcIndex32 = Avx2_Expand8ToEpi32(src);
					const __m256i srcMask32 = _mm256_cmpgt_epi32(srcIndex32, zero32);
					const __m256i activeMask32 = _mm256_and_si256(srcMask32, zMask32);

					if (_mm256_movemask_epi8(activeMask32))
					{
						const __m256i alphaIndex32 = Avx2_Load8WordAsEpi32(abuf);
						const __m256i alphaValue32 = Avx2_GatherWordTable(alphaIndex32, pAData, 0x00FF);
						const __m256i paletteIndex32 = _mm256_or_si256(srcIndex32, alphaValue32);
						const __m256i result32 = Avx2_GatherWordTable(paletteIndex32, pPaletteData, 0xFFFF);

						__m128i result16 = Avx2_PackU32ToU16(result32);
						if constexpr (UseTint)
						{
							result16 = _mm_or_si128(result16, tintVec16);
						}

						const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
						const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
						const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);

						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
					}
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
			const __m128i zero16 = _mm_setzero_si128();
			const __m128i zvalVec16 = _mm_set1_epi16(static_cast<short>(zval));
			__m128i tintVec16;
			if constexpr (UseTint)
			{
				tintVec16 = _mm_set1_epi16(static_cast<short>(tint));
			}

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
					break;

				__m128i zMask16 = _mm_setzero_si128();
				if (zval < 0)
				{
					zMask16 = _mm_set1_epi16(static_cast<short>(-1));
				}
				else if (zval <= 0xFFFF)
				{
					const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
					zMask16 = Sse2_CmpGtEpu16(zbuf16, zvalVec16);
				}

				if (_mm_movemask_epi8(zMask16))
				{
					const __m128i srcIndex16 = Sse2_Expand8ToEpi16(src);
					const __m128i srcMask16 = _mm_cmpgt_epi16(srcIndex16, zero16);
					const __m128i activeMask16 = _mm_and_si128(srcMask16, zMask16);
					if (_mm_movemask_epi8(activeMask16))
					{
						const __m128i alphaIndex16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(abuf));

						alignas(16) WORD alphaIndexArray[8];
						_mm_store_si128(reinterpret_cast<__m128i*>(alphaIndexArray), alphaIndex16);

						alignas(16) WORD alphaValueArray[8];
						for (int i = 0; i < 8; ++i)
						{
							const WORD index = alphaIndexArray[i];
							alphaValueArray[i] = pAData[index == 0x00FF ? 0x00FF : index];
						}

						const __m128i alphaValue16 = _mm_load_si128(reinterpret_cast<const __m128i*>(alphaValueArray));
						const __m128i paletteIndex16 = _mm_or_si128(srcIndex16, alphaValue16);

						alignas(16) WORD paletteIndexArray[8];
						_mm_store_si128(reinterpret_cast<__m128i*>(paletteIndexArray), paletteIndex16);

						alignas(16) WORD resultArray[8];
						for (int i = 0; i < 8; ++i)
						{
							const WORD index = paletteIndexArray[i];
							resultArray[i] = pPaletteData[index == 0xFFFF ? 0xFFFF : index];
						}

						__m128i result16 = _mm_load_si128(reinterpret_cast<const __m128i*>(resultArray));
						if constexpr (UseTint)
						{
							result16 = _mm_or_si128(result16, tintVec16);
						}

						const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
						const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, activeMask16);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
					}
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// Scalar
		while (len--)
		{
			WORD zbufv = *zbuf++;
			byte idx = *src++;
			if (zval < zbufv && idx)
			{
				WORD value = pPaletteData[idx | pAData[*abuf]];
				if constexpr (UseTint)
					*pDest = tint | value;
				else
					*pDest = value;
			}

			++abuf;
			++pDest;

			ADJUST_POINTER(ZBuffer::Instance, zbuf);
			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
