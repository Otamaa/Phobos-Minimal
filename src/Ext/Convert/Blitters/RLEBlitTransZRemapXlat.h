#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransZRemapXlat, BlitterPixelByteAndWord)
{
public:
	inline explicit RLEBlitTransZRemapXlat(byte** remapData, T* data) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
	}

	virtual ~RLEBlitTransZRemapXlat() override final = default;

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
		T* pDest = reinterpret_cast<T*>(dst);
		byte** pRemapData = this->RemapData;
		T* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(false, false, pDest, src, len, line, zbuf, abuf);
#if defined(YR_SIMD_COMPILE_AVX512)
		// AVX512 BYTE
		if constexpr (Level == Simd::Level::AVX512 && std::is_same_v<T, BYTE> && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			alignas(64) unsigned int paletteLut32[256];
			Avx512_BuildByteLut32(pPaletteData, paletteLut32);
			alignas(64) unsigned int remappedIndices[ChunkSize];

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
						byte* pCurrentRemap = *pRemapData;
						for (int i = 0; i < ChunkSize; ++i)
							remappedIndices[i] = pCurrentRemap[pRunSrc[i]];

						const __m512i remapIndex32 = _mm512_load_si512(reinterpret_cast<const __m512i*>(remappedIndices));
						const __m512i srcColors32 = _mm512_i32gather_epi32(remapIndex32, paletteLut32, 4);
						const __m128i result8 = Avx512_PackU32ToU8(srcColors32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
						*pDest++ = pPaletteData[(*pRemapData)[*pRunSrc++]];

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
#endif
		// AVX2 BYTE
		if constexpr (Level == Simd::Level::AVX2 && std::is_same_v<T, BYTE> && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			alignas(32) unsigned int paletteLut32[256];
			Avx2_BuildByteLut32(pPaletteData, paletteLut32);
			alignas(32) unsigned int remappedIndices[8];
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
							byte* pCurrentRemap = *pRemapData;
							for (int lane = 0; lane < ChunkSize; ++lane)
								remappedIndices[lane] = pCurrentRemap[pRunSrc[lane]];

							const __m256i remapIndex32 = _mm256_load_si256(reinterpret_cast<const __m256i*>(remappedIndices));
						const __m256i index32 = remapIndex32;
						const __m256i value32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(paletteLut32), index32, 4);
						const __m128i value8 = Avx2_PackU32ToU8(value32);
						_mm_storel_epi64(reinterpret_cast<__m128i*>(pDest), value8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = pPaletteData[(*pRemapData)[*pRunSrc]];

						++pRunSrc;
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
		if constexpr (Level == Simd::Level::SSE2 && std::is_same_v<T, WORD>)
		{
			constexpr int ChunkSize = 8;
			alignas(16) WORD remappedIndices[ChunkSize];

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
						byte* pCurrentRemap = *pRemapData;
						for (int i = 0; i < ChunkSize; ++i)
						{
							remappedIndices[i] = static_cast<WORD>(pCurrentRemap[pRunSrc[i]]);
						}

						const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remappedIndices));
						const __m128i srcColors16 = Sse2_GatherPaletteWord(remapIndex16, pPaletteData);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), srcColors16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest++ = pPaletteData[(*pRemapData)[*pRunSrc++]];
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
		auto handler = [pRemapData, pPaletteData](T& dest, byte srcv)
		{
			dest = pPaletteData[(*pRemapData)[srcv]];
		};

		RLE_PROCESS_PIXEL_DATAS(false, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	byte** RemapData;
	T* PaletteData;
};
