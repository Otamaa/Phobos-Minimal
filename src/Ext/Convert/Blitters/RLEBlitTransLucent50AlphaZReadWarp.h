#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransLucent50AlphaZReadWarp, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransLucent50AlphaZReadWarp(WORD* data, WORD mask, int shadecount) noexcept
	{
		this->PaletteData = data;
		this->Mask = mask;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransLucent50AlphaZReadWarp() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;
		WORD mask = this->Mask;

		RLE_PROCESS_PRE_LINES(true, true, pDest, src, len, line, zbuf, abuf);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			if (warp >= 0)
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
				const __m512i blendMask = _mm512_set1_epi32(static_cast<int>(mask));

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

								__m512i srcColors = _mm512_setzero_si512();
								if (paletteGatherMask)
								{
									srcColors = _mm512_mask_i32gather_epi32(srcColors, paletteGatherMask, paletteIndices, pPaletteData, 2);
								}

								srcColors = _mm512_and_si512(srcColors, low16Mask);

								const __mmask16 paletteFillMask = zMask & paletteMaxIndexMask;
								if (paletteFillMask)
								{
									srcColors = _mm512_mask_set1_epi32(srcColors, paletteFillMask, static_cast<int>(pPaletteData[0xFFFF]));
								}

								const __m512i warpColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest + warp)));
								const __m512i warpHalf = _mm512_and_si512(_mm512_srli_epi32(warpColors, 1), blendMask);
								const __m512i srcHalf = _mm512_and_si512(_mm512_srli_epi32(srcColors, 1), blendMask);
								const __m512i result32 = _mm512_add_epi32(warpHalf, srcHalf);
								const __m256i result16 = _mm512_cvtepi32_epi16(result32);
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
								*pDest = (mask & (pDest[warp] >> 1)) + (mask & (pPaletteData[*pRunSrc | pAData[*abuf]] >> 1));

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

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			if (warp >= 0)
			{
				constexpr int ChunkSize = 8;
				constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
				ZBuffer* pZBuffer = ZBuffer::Instance;
				ABuffer* pABuffer = ABuffer::Instance;
				const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
				const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
				const __m256i zbaseVec32 = _mm256_set1_epi32(zbase);
				const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));

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
								const __m256i srcColors = Avx2_GatherWordTable(paletteIndex32, pPaletteData, 0xFFFF);
								const __m256i warpColors = Avx2_Load8WordAsEpi32(pDest + warp);
								const __m256i warpHalf32 = _mm256_and_si256(_mm256_srli_epi32(warpColors, 1), blendMask32);
								const __m256i srcHalf32 = _mm256_and_si256(_mm256_srli_epi32(srcColors, 1), blendMask32);
								const __m256i result32 = _mm256_add_epi32(warpHalf32, srcHalf32);
								const __m128i result16 = Avx2_PackU32ToU16(result32);
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
								*pDest = (mask & (pDest[warp] >> 1)) + (mask & (pPaletteData[*pRunSrc | pAData[*abuf]] >> 1));

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

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			if (warp >= 0)
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
					const __m128i blendMask16 = _mm_set1_epi16(static_cast<short>(mask));

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
									const __m128i srcIndex16 = Sse2_Expand8ToEpi16(pRunSrc);
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
									const __m128i warp16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest + warp));

									const __m128i warpHalf16 = _mm_and_si128(_mm_srli_epi16(warp16, 1), blendMask16);
									const __m128i srcHalf16 = _mm_and_si128(_mm_srli_epi16(srcColor16, 1), blendMask16);
									const __m128i result16 = _mm_add_epi16(warpHalf16, srcHalf16);
									const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
									const __m128i blended16 = Sse2_BlendU16(dest16, result16, zMask16);
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
									*pDest = (mask & (pDest[warp] >> 1)) + (mask & (pPaletteData[*pRunSrc | pAData[*abuf]] >> 1));

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
		}

		// Scalar
		auto handler = [pPaletteData, pAData, mask, warp](WORD& dest, byte srcv, int zbase, WORD zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
				dest = (mask & ((&dest)[warp] >> 1)) + (mask & (pPaletteData[srcv | pAData[abufv]] >> 1));
		};

		RLE_PROCESS_PIXEL_DATAS(true, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	WORD* PaletteData;
	WORD Mask;
	AlphaLightingRemapClass* AlphaRemapper;
};
