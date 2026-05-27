#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransXlatAlphaZRead, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransXlatAlphaZRead(WORD* data, int shadecount) noexcept
	{
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransXlatAlphaZRead() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		Blit_Impl<false>(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust, 0);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Impl<true>(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust, tint);
	}

private:
	template<bool UseTint>
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(true, true, pDest, src, len, line, zbuf, abuf);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m256i alphaMaxIndex16 = _mm256_set1_epi16(0x00FF);
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i paletteMaxIndex = _mm512_set1_epi32(0xFFFF);
			const __m512i zbaseVec = _mm512_set1_epi32(zbase);
			const __m512i tintVec = _mm512_set1_epi32(static_cast<int>(tint));

			while (len > 0)
			{
				byte srcv = *src++;
				if (srcv)
				{
					byte* pRunSrc = src - 1;
					byte* pRunZAdjust = zadjust;
					int runLen = 1;
					while (runLen < len && pRunSrc[runLen])
						++runLen;

					int remaining = runLen;
					while (remaining >= ChunkSize)
					{
						const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
						const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
						if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
						{
							break;
						}

						const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pRunSrc));
						const __m128i zAdjustBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pRunZAdjust));
						const __m512i zValues = _mm512_sub_epi32(zbaseVec, _mm512_cvtepi8_epi32(zAdjustBytes));
						const __m512i zbufValues = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(zbuf)));
						const __mmask16 zMask = _mm512_cmplt_epi32_mask(zValues, zbufValues);

						if (zMask)
						{
							const __m256i abufValues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(abuf));
							const __m512i alphaTableIndices = _mm512_cvtepu16_epi32(abufValues);
							const __mmask16 alphaMaxIndexMask = _mm256_cmpeq_epu16_mask(abufValues, alphaMaxIndex16);
							const __mmask16 alphaGatherMask = zMask & ~alphaMaxIndexMask;

							__m512i alphaValues = _mm512_setzero_si512();
							if (alphaGatherMask)
							{
								alphaValues = _mm512_mask_i32gather_epi32(alphaValues, alphaGatherMask, alphaTableIndices, pAData, 2);
							}

							alphaValues = _mm512_and_si512(alphaValues, low16Mask);

							const __mmask16 alphaFillMask = zMask & alphaMaxIndexMask;
							if (alphaFillMask)
							{
								alphaValues = _mm512_mask_set1_epi32(alphaValues, alphaFillMask, static_cast<int>(pAData[255]));
							}

							const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
							const __m512i paletteIndices = _mm512_or_si512(srcIndices, alphaValues);
							const __mmask16 paletteMaxIndexMask = _mm512_cmpeq_epi32_mask(paletteIndices, paletteMaxIndex);
							const __mmask16 paletteGatherMask = zMask & ~paletteMaxIndexMask;

							__m512i resultColors = _mm512_setzero_si512();
							if (paletteGatherMask)
							{
								resultColors = _mm512_mask_i32gather_epi32(resultColors, paletteGatherMask, paletteIndices, pPaletteData, 2);
							}

							resultColors = _mm512_and_si512(resultColors, low16Mask);

							const __mmask16 paletteFillMask = zMask & paletteMaxIndexMask;
							if (paletteFillMask)
							{
								resultColors = _mm512_mask_set1_epi32(resultColors, paletteFillMask, static_cast<int>(pPaletteData[0xFFFF]));
							}

							if constexpr (UseTint)
							{
								resultColors = _mm512_or_si512(resultColors, tintVec);
							}

							const __m256i result16 = _mm512_cvtepi32_epi16(resultColors);
							_mm256_mask_storeu_epi16(pDest, zMask, result16);
						}

						pRunSrc += ChunkSize;
						pRunZAdjust += ChunkSize;
						pDest += ChunkSize;
						zbuf += ChunkSize;
						abuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}

					while (remaining--)
					{
						int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
						if (zval < *zbuf)
						{
							WORD value = pPaletteData[*pRunSrc | pAData[*abuf]];
							if constexpr (UseTint)
								*pDest = tint | value;
							else
								*pDest = value;
						}

						++pRunSrc;
						++pDest;
						++zbuf;
						++abuf;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}

					src = pRunSrc;
					zadjust = pRunZAdjust;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
					zbuf += off;
					zadjust += off;
					abuf += off;
					ADJUST_POINTER(pZBuffer, zbuf);
					ADJUST_POINTER(pABuffer, abuf);
				}
			}

			return;
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
			const __m256i zbaseVec32 = _mm256_set1_epi32(zbase);
			__m128i tintVec16;
			if constexpr (UseTint)
			{
				tintVec16 = _mm_set1_epi16(static_cast<short>(tint));
			}

			while (len > 0)
			{
				byte srcv = *src++;
				if (srcv)
				{
					byte* pRunSrc = src - 1;
					byte* pRunZAdjust = zadjust;
					int runLen = 1;
					while (runLen < len && pRunSrc[runLen])
					{
						++runLen;
					}

					int remaining = runLen;
					while (remaining >= ChunkSize)
					{
						const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
						const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
						if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
						{
							break;
						}

						const __m256i zAdjust32 = _mm256_cvtepi8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pRunZAdjust)));
						const __m256i zValues32 = _mm256_sub_epi32(zbaseVec32, zAdjust32);
						const __m256i zbuf32 = Avx2_Load8WordAsEpi32(zbuf);
						const __m256i zMask32 = _mm256_cmpgt_epi32(zbuf32, zValues32);

						if (_mm256_movemask_epi8(zMask32))
						{
							const __m256i srcIndices = Avx2_Expand8ToEpi32(pRunSrc);
							const __m256i alphaIndex32 = Avx2_Load8WordAsEpi32(abuf);
							const __m256i alphaValue32 = Avx2_GatherWordTable(alphaIndex32, pAData, 0x00FF);
							const __m256i paletteIndex32 = _mm256_or_si256(srcIndices, alphaValue32);
							const __m256i result32 = Avx2_GatherWordTable(paletteIndex32, pPaletteData, 0xFFFF);
							__m128i result16 = Avx2_PackU32ToU16(result32);
							if constexpr (UseTint)
							{
								result16 = _mm_or_si128(result16, tintVec16);
							}

							const __m128i writeMask16 = Avx2_PackMask32ToI16(zMask32);
							const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
							const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);
							_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
						}

						pRunSrc += ChunkSize;
						pRunZAdjust += ChunkSize;
						pDest += ChunkSize;
						zbuf += ChunkSize;
						abuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}

					while (remaining--)
					{
						int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
						if (zval < *zbuf)
						{
							WORD value = pPaletteData[*pRunSrc | pAData[*abuf]];
							if constexpr (UseTint)
								*pDest = tint | value;
							else
								*pDest = value;
						}

						++pRunSrc;
						++pDest;
						++zbuf;
						++abuf;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}

					src = pRunSrc;
					zadjust = pRunZAdjust;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
					zbuf += off;
					zadjust += off;
					abuf += off;
					ADJUST_POINTER(pZBuffer, zbuf);
					ADJUST_POINTER(pABuffer, abuf);
				}
			}

			return;
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			const bool canUseVectorZ = zbase >= 127 && zbase <= 65407;
			if (canUseVectorZ)
			{
				constexpr int ChunkSize = 8;
				constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
				ZBuffer* pZBuffer = ZBuffer::Instance;
				ABuffer* pABuffer = ABuffer::Instance;
				const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
				const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
				const __m128i zbaseVec16 = _mm_set1_epi16(static_cast<short>(zbase));
				__m128i tintVec16;
				if constexpr (UseTint)
				{
					tintVec16 = _mm_set1_epi16(static_cast<short>(tint));
				}

				while (len > 0)
				{
					byte srcv = *src++;
					if (srcv)
					{
						byte* pRunSrc = src - 1;
						byte* pRunZAdjust = zadjust;
						int runLen = 1;
						while (runLen < len && pRunSrc[runLen])
						{
							++runLen;
						}

						int remaining = runLen;
						while (remaining >= ChunkSize)
						{
							const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
							const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
							if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
							{
								break;
							}

							const __m128i zAdjust16 = Sse2_Expand8ToEpi16Signed(pRunZAdjust);
							const __m128i zValues16 = _mm_sub_epi16(zbaseVec16, zAdjust16);
							const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
							const __m128i zMask16 = Sse2_CmpGtEpu16(zbuf16, zValues16);

							if (_mm_movemask_epi8(zMask16))
							{
								const __m128i srcIndices16 = Sse2_Expand8ToEpi16(pRunSrc);
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
								const __m128i paletteIndex16 = _mm_or_si128(srcIndices16, alphaValue16);
								__m128i result16 = Sse2_GatherPaletteWord(paletteIndex16, pPaletteData);
								if constexpr (UseTint)
								{
									result16 = _mm_or_si128(result16, tintVec16);
								}

								const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
								const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, zMask16);
								_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
							}

							pRunSrc += ChunkSize;
							pRunZAdjust += ChunkSize;
							pDest += ChunkSize;
							zbuf += ChunkSize;
							abuf += ChunkSize;
							remaining -= ChunkSize;
							ADJUST_POINTER(pZBuffer, zbuf);
							ADJUST_POINTER(pABuffer, abuf);
						}

						while (remaining--)
						{
							int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
							if (zval < *zbuf)
							{
								WORD value = pPaletteData[*pRunSrc | pAData[*abuf]];
								if constexpr (UseTint)
									*pDest = tint | value;
								else
									*pDest = value;
							}

							++pRunSrc;
							++pDest;
							++zbuf;
							++abuf;
							ADJUST_POINTER(pZBuffer, zbuf);
							ADJUST_POINTER(pABuffer, abuf);
						}

						src = pRunSrc;
						zadjust = pRunZAdjust;
						len -= runLen;
					}
					else
					{
						byte off = *src++;
						len -= off;
						pDest += off;
						zbuf += off;
						zadjust += off;
						abuf += off;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}
				}

				return;
			}
		}

		// Scalar
		auto handler = [pPaletteData, pAData, tint](WORD& dest, byte srcv, int zbase, WORD zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
			{
				WORD value = pPaletteData[srcv | pAData[abufv]];
				if constexpr (UseTint)
					dest = tint | value;
				else
					dest = value;
			}
		};

		RLE_PROCESS_PIXEL_DATAS(true, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
