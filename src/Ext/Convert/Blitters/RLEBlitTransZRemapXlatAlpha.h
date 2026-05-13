#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransZRemapXlatAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransZRemapXlatAlpha(byte** remapData, WORD* data, int shadecount) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransZRemapXlatAlpha() override final = default;

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
		byte** pRemapData = this->RemapData;
		WORD* pPaletteData = this->PaletteData;
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);

		RLE_PROCESS_PRE_LINES(false, true, pDest, src, len, line, zbuf, abuf);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __mmask16 allMask = static_cast<__mmask16>(0xFFFF);
			const __m256i alphaMaxIndex16 = _mm256_set1_epi16(0x00FF);
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i paletteMaxIndex = _mm512_set1_epi32(0xFFFF);

			const byte* pRemapTable = *pRemapData;
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
						const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
						if (aAddress + ChunkBytes > aTailAddress)
						{
							break;
						}

						alignas(32) WORD remapIndices[ChunkSize];
						for (int i = 0; i < ChunkSize; ++i)
							remapIndices[i] = pRemapTable[pRunSrc[i]];

						const __m256i remapValues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(remapIndices));
						const __m256i abufValues = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(abuf));

						const __m512i alphaTableIndices = _mm512_cvtepu16_epi32(abufValues);
						const __mmask16 alphaMaxIndexMask = _mm256_cmpeq_epu16_mask(abufValues, alphaMaxIndex16);
						const __mmask16 alphaGatherMask = allMask & ~alphaMaxIndexMask;

						__m512i alphaValues = _mm512_setzero_si512();
						if (alphaGatherMask)
						{
							alphaValues = _mm512_mask_i32gather_epi32(alphaValues, alphaGatherMask, alphaTableIndices, pAData, 2);
						}

						alphaValues = _mm512_and_si512(alphaValues, low16Mask);

						if (alphaMaxIndexMask)
						{
							alphaValues = _mm512_mask_set1_epi32(alphaValues, alphaMaxIndexMask, static_cast<int>(pAData[255]));
						}

						const __m512i remapIndices32 = _mm512_cvtepu16_epi32(remapValues);
						const __m512i paletteIndices = _mm512_or_si512(remapIndices32, alphaValues);
						const __mmask16 paletteMaxIndexMask = _mm512_cmpeq_epi32_mask(paletteIndices, paletteMaxIndex);
						const __mmask16 paletteGatherMask = allMask & ~paletteMaxIndexMask;

						__m512i resultColors = _mm512_setzero_si512();
						if (paletteGatherMask)
						{
							resultColors = _mm512_mask_i32gather_epi32(resultColors, paletteGatherMask, paletteIndices, pPaletteData, 2);
						}

						resultColors = _mm512_and_si512(resultColors, low16Mask);

						if (paletteMaxIndexMask)
						{
							resultColors = _mm512_mask_set1_epi32(resultColors, paletteMaxIndexMask, static_cast<int>(pPaletteData[0xFFFF]));
						}

						const __m256i result16 = _mm512_cvtepi32_epi16(resultColors);
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						abuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pABuffer, abuf);
					}

					while (remaining--)
					{
						*pDest++ = pPaletteData[pRemapTable[*pRunSrc++] | pAData[*abuf++]];
						ADJUST_POINTER(pABuffer, abuf);
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
					abuf += off;
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
			alignas(32) unsigned int remapIndices[ChunkSize];
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
			const byte* pRemapTable = *pRemapData;

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
						const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
						if (aAddress + ChunkBytes > aTailAddress)
						{
							break;
						}

						for (int i = 0; i < ChunkSize; ++i)
						{
							remapIndices[i] = pRemapTable[pRunSrc[i]];
						}

						const __m256i remapIndex32 = _mm256_load_si256(reinterpret_cast<const __m256i*>(remapIndices));
						const __m256i alphaIndex32 = Avx2_Load8WordAsEpi32(abuf);
						const __m256i alphaValue32 = Avx2_GatherWordTable(alphaIndex32, pAData, 0x00FF);
						const __m256i paletteIndex32 = _mm256_or_si256(remapIndex32, alphaValue32);
						const __m256i result32 = Avx2_GatherWordTable(paletteIndex32, pPaletteData, 0xFFFF);
						const __m128i result16 = Avx2_PackU32ToU16(result32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						abuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pABuffer, abuf);
					}

					while (remaining--)
					{
						*pDest++ = pPaletteData[pRemapTable[*pRunSrc++] | pAData[*abuf++]];
						ADJUST_POINTER(pABuffer, abuf);
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
					abuf += off;
					ADJUST_POINTER(pABuffer, abuf);
				}
			}

			return;
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
			const byte* pRemapTable = *pRemapData;

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
						const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
						if (aAddress + ChunkBytes > aTailAddress)
						{
							break;
						}

						alignas(16) WORD remapIndices[ChunkSize];
						for (int i = 0; i < ChunkSize; ++i)
						{
							remapIndices[i] = pRemapTable[pRunSrc[i]];
						}

						const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remapIndices));
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
						const __m128i paletteIndex16 = _mm_or_si128(remapIndex16, alphaValue16);
						const __m128i result16 = Sse2_GatherPaletteWord(paletteIndex16, pPaletteData);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						abuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pABuffer, abuf);
					}

					while (remaining--)
					{
						*pDest++ = pPaletteData[pRemapTable[*pRunSrc++] | pAData[*abuf++]];
						ADJUST_POINTER(pABuffer, abuf);
					}

					src = pRunSrc;
					len -= runLen;
				}
				else
				{
					byte off = *src++;
					len -= off;
					pDest += off;
					abuf += off;
					ADJUST_POINTER(pABuffer, abuf);
				}
			}

			return;
		}

		// Scalar
		auto handler = [pRemapData, pPaletteData, pAData](WORD& dest, byte srcv, WORD abufv)
		{
			dest = pPaletteData[(*pRemapData)[srcv] | pAData[abufv]];
		};

		RLE_PROCESS_PIXEL_DATAS(false, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	byte** RemapData;
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
