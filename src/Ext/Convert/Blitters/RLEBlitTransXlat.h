#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransXlat, BlitterPixelByteAndWord)
{
public:
	inline explicit RLEBlitTransXlat(T* data) noexcept
	{
		this->PaletteData = data;
	}

	virtual ~RLEBlitTransXlat() override final = default;

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
		T* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(false, false, pDest, src, len, line, zbuf, abuf);
#if defined(YR_SIMD_COMPILE_AVX512)
		// AVX512 BYTE
		if constexpr (Level == Simd::Level::AVX512 && std::is_same_v<T, BYTE> && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			alignas(64) unsigned int paletteLut32[256];
			Avx512_BuildByteLut32(pPaletteData, paletteLut32);

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
						const __m512i srcIndices32 = Avx512_Expand16ToEpi32(pRunSrc);
						const __m512i srcColors32 = _mm512_i32gather_epi32(srcIndices32, paletteLut32, 4);
						const __m128i result8 = Avx512_PackU32ToU8(srcColors32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
						*pDest++ = pPaletteData[*pRunSrc++];

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
						const __m256i index32 = Avx2_Expand8ToEpi32(pRunSrc);
						const __m256i value32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(paletteLut32), index32, 4);
						const __m128i value8 = Avx2_PackU32ToU8(value32);
						_mm_storel_epi64(reinterpret_cast<__m128i*>(pDest), value8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = pPaletteData[*pRunSrc];
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

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && std::is_same_v<T, WORD> && CompileAvx512)
		{
			constexpr int ChunkSize = 16;

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
						const __m512i srcColors = _mm512_i32gather_epi32(srcIndices, pPaletteData, 2);
						const __m256i result16 = _mm512_cvtepi32_epi16(srcColors);
						_mm256_storeu_si256(reinterpret_cast<__m256i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
						*pDest++ = pPaletteData[*pRunSrc++];

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
		if constexpr (Level == Simd::Level::AVX2 && std::is_same_v<T, WORD> && CompileAvx2)
		{
			constexpr int ChunkSize = 8;

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
						const __m128i result16 = Avx2_PackU32ToU16(srcColors);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result16);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest++ = pPaletteData[*pRunSrc++];
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
		auto handler = [pPaletteData](T& dest, byte srcv)
		{
			dest = pPaletteData[srcv];
		};

		RLE_PROCESS_PIXEL_DATAS(false, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	T* PaletteData;
};
