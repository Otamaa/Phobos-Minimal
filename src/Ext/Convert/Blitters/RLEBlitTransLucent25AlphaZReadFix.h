#pragma once

#include "RLEBlitTransLucent25AlphaZRead.h"

template<BlitterPixelWordOnly T, Simd::Level Level>
class RLEBlitTransLucent25AlphaZReadFix final : public RLEBlitTransLucent25AlphaZRead<T, Level>
{
public:
	inline explicit RLEBlitTransLucent25AlphaZReadFix(WORD* data, WORD mask, int shadecount) noexcept
		: RLEBlitTransLucent25AlphaZRead<T, Level>(data, mask, shadecount)
	{
	}

	virtual ~RLEBlitTransLucent25AlphaZReadFix() override final = default;

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
		constexpr unsigned int RGB565BlendMask = 0xF7DEu;

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
			const __m512i f7de32 = _mm512_set1_epi32(static_cast<int>(RGB565BlendMask));

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

							const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));
							const __m512i halfDiff32 = _mm512_srli_epi32(_mm512_and_si512(_mm512_xor_si512(srcColors, destColors), f7de32), 1);
							const __m512i blended32 = _mm512_add_epi32(halfDiff32, _mm512_and_si512(srcColors, destColors));
							const __m512i result32 = _mm512_sub_epi32(_mm512_or_si512(srcColors, blended32), _mm512_srli_epi32(_mm512_and_si512(_mm512_xor_si512(srcColors, blended32), f7de32), 1));
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
						{
							const unsigned int srcColor = pPaletteData[*pRunSrc | pAData[*abuf]];
							const unsigned int destColor = *pDest;
							const unsigned int blendedColor = (((srcColor ^ destColor) & RGB565BlendMask) >> 1) + (srcColor & destColor);
							const unsigned int refinedColor = (srcColor | blendedColor) - (((srcColor ^ blendedColor) & RGB565BlendMask) >> 1);
							*pDest = static_cast<T>(refinedColor);
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
		if constexpr (Level == Simd::Level::SSE2 || Level == Simd::Level::AVX2)
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
				const __m128i f7de16 = _mm_set1_epi16(static_cast<short>(RGB565BlendMask));

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
								const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));

								const __m128i halfDiff16 = _mm_srli_epi16(_mm_and_si128(_mm_xor_si128(srcColor16, dest16), f7de16), 1);
								const __m128i blended16 = _mm_add_epi16(halfDiff16, _mm_and_si128(srcColor16, dest16));
								const __m128i result16 = _mm_sub_epi16(_mm_or_si128(srcColor16, blended16), _mm_srli_epi16(_mm_and_si128(_mm_xor_si128(srcColor16, blended16), f7de16), 1));
								const __m128i output16 = Sse2_BlendU16(dest16, result16, zMask16);
								_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), output16);
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
								const unsigned int srcColor = pPaletteData[*pRunSrc | pAData[*abuf]];
								const unsigned int destColor = *pDest;
								const unsigned int blendedColor = (((srcColor ^ destColor) & RGB565BlendMask) >> 1) + (srcColor & destColor);
								const unsigned int refinedColor = (srcColor | blendedColor) - (((srcColor ^ blendedColor) & RGB565BlendMask) >> 1);
								*pDest = static_cast<T>(refinedColor);
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
		auto handler = [pPaletteData, pAData](WORD& dest, byte srcv, int zbase, WORD zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
			{
				const unsigned int srcColor = pPaletteData[srcv | pAData[abufv]];
				const unsigned int destColor = static_cast<WORD>(dest);
				const unsigned int blendedColor = (((srcColor ^ destColor) & RGB565BlendMask) >> 1) + (srcColor & destColor);
				const unsigned int refinedColor = (srcColor | blendedColor) - (((srcColor ^ blendedColor) & RGB565BlendMask) >> 1);
				dest = static_cast<T>(refinedColor);
			}
		};

		RLE_PROCESS_PIXEL_DATAS(true, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}


};

static_assert(sizeof(RLEBlitTransLucent25AlphaZReadFix<WORD, Simd::Level::Scalar>) == sizeof(RLEBlitTransLucent25AlphaZRead<WORD, Simd::Level::Scalar>));
