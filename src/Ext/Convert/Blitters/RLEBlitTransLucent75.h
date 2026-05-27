#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransLucent75, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransLucent75(WORD* data, WORD mask) noexcept
	{
		this->PaletteData = data;
		this->Mask = mask;
	}

	virtual ~RLEBlitTransLucent75() override final = default;

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
		WORD* pPaletteData = this->PaletteData;
		WORD mask = this->Mask;

		RLE_PROCESS_PRE_LINES(false, false, pDest, src, len, line, zbuf, abuf);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i paletteMaxIndex = _mm512_set1_epi32(0xFFFF);
			const __m512i blendMask = _mm512_set1_epi32(static_cast<int>(mask));
			const __m512i three = _mm512_set1_epi32(3);
			const __mmask16 allMask = static_cast<__mmask16>(0xFFFF);

			while (len > 0)
			{
				byte srcv = *src++;
				if (srcv)
				{
					byte* pRunSrc = src - 1;
					int runLen = 1;
					while (runLen < len && pRunSrc[runLen])
						++runLen;

					int remaining = runLen;
					while (remaining >= ChunkSize)
					{
						const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pRunSrc));
						const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
						const __mmask16 paletteMaxIndexMask = _mm512_cmpeq_epi32_mask(srcIndices, paletteMaxIndex);
						const __mmask16 paletteGatherMask = allMask & ~paletteMaxIndexMask;

						__m512i srcColors = _mm512_setzero_si512();
						if (paletteGatherMask)
						{
							srcColors = _mm512_mask_i32gather_epi32(srcColors, paletteGatherMask, srcIndices, pPaletteData, 2);
						}

						srcColors = _mm512_and_si512(srcColors, low16Mask);

						if (paletteMaxIndexMask)
						{
							srcColors = _mm512_mask_set1_epi32(srcColors, paletteMaxIndexMask, static_cast<int>(pPaletteData[0xFFFF]));
						}

						const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));
						const __m512i destQuarter = _mm512_and_si512(_mm512_srli_epi32(destColors, 2), blendMask);
						const __m512i srcQuarter = _mm512_and_si512(_mm512_srli_epi32(srcColors, 2), blendMask);
						const __m512i result32 = _mm512_add_epi32(_mm512_mullo_epi32(destQuarter, three), srcQuarter);
						const __m256i result16 = _mm512_cvtepi32_epi16(result32);
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = 3 * (mask & (*pDest >> 2)) + (mask & (pPaletteData[*pRunSrc++] >> 2));
						++pDest;
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
				}
			}

			return;
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));

			while (len > 0)
			{
				byte srcv = *src++;
				if (srcv)
				{
					byte* pRunSrc = src - 1;
					int runLen = 1;
					while (runLen < len && pRunSrc[runLen])
					{
						++runLen;
					}

					int remaining = runLen;
					while (remaining >= ChunkSize)
					{
						const __m256i srcIndices = Avx2_Expand8ToEpi32(pRunSrc);
						const __m256i srcColors = Avx2_GatherPaletteWord(srcIndices, pPaletteData);
						const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);

						const __m256i destQuarter32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 2), blendMask32);
						const __m256i srcQuarter32 = _mm256_and_si256(_mm256_srli_epi32(srcColors, 2), blendMask32);
						const __m256i result32 = _mm256_add_epi32(_mm256_add_epi32(_mm256_add_epi32(destQuarter32, destQuarter32), destQuarter32), srcQuarter32);
						const __m128i result16 = Avx2_PackU32ToU16(result32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = 3 * (mask & (*pDest >> 2)) + (mask & (pPaletteData[*pRunSrc++] >> 2));
						++pDest;
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
				}
			}

			return;
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			const __m128i blendMask16 = _mm_set1_epi16(static_cast<short>(mask));

			while (len > 0)
			{
				byte srcv = *src++;
				if (srcv)
				{
					byte* pRunSrc = src - 1;
					int runLen = 1;
					while (runLen < len && pRunSrc[runLen])
					{
						++runLen;
					}

					int remaining = runLen;
					while (remaining >= ChunkSize)
					{
						const __m128i srcIndex16 = Sse2_Expand8ToEpi16(pRunSrc);
						const __m128i srcColor16 = Sse2_GatherPaletteWord(srcIndex16, pPaletteData);
						const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
						const __m128i destQuarter16 = _mm_and_si128(_mm_srli_epi16(dest16, 2), blendMask16);
						const __m128i srcQuarter16 = _mm_and_si128(_mm_srli_epi16(srcColor16, 2), blendMask16);
						const __m128i result16 = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(destQuarter16, destQuarter16), destQuarter16), srcQuarter16);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = 3 * (mask & (*pDest >> 2)) + (mask & (pPaletteData[*pRunSrc++] >> 2));
						++pDest;
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
				}
			}

			return;
		}

		// Scalar
		auto handler = [pPaletteData, mask](WORD& dest, byte srcv)
		{
			dest = 3 * (mask & (dest >> 2)) + (mask & (pPaletteData[srcv] >> 2));
		};

		RLE_PROCESS_PIXEL_DATAS(false, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	WORD* PaletteData;
	WORD Mask;
};
