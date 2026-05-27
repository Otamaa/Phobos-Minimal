#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransRemapXlat, BlitterPixelByteOnly)
{
public:
	inline explicit RLEBlitTransRemapXlat(BYTE* remap, BYTE* palette) noexcept
	{
		this->RemapData = remap;
		this->PaletteData = palette;
	}

	virtual ~RLEBlitTransRemapXlat() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint)
	{
		Blit_Impl(dst, src, len, line, zbase, zbuf, abuf, alvl, warp, zadjust);
	}

private:
	FORCEDINLINE void Blit_Impl(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust)
	{
		BYTE* pDest = reinterpret_cast<BYTE*>(dst);
		BYTE* pRemapData = this->RemapData;
		BYTE* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(false, false, pDest, src, len, line, zbuf, abuf);

#if defined(YR_SIMD_COMPILE_AVX512)
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			alignas(64) unsigned int paletteLut32[256];
			for (int i = 0; i < 256; ++i)
				paletteLut32[i] = pPaletteData[pRemapData[i]];

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
						const __m512i srcIndex32 = Avx512_Expand16ToEpi32(pRunSrc);
						const __m512i srcColors32 = _mm512_i32gather_epi32(srcIndex32, paletteLut32, 4);
						const __m128i result8 = Avx512_PackU32ToU8(srcColors32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
						*pDest++ = pPaletteData[pRemapData[*pRunSrc++]];

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
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			alignas(32) unsigned int paletteLut32[256];
			for (int i = 0; i < 256; ++i)
				paletteLut32[i] = pPaletteData[pRemapData[i]];
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
						*pDest = pPaletteData[pRemapData[*pRunSrc]];

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

		auto handler = [pRemapData, pPaletteData](BYTE& pDest, byte srcv)
		{
			pDest = pPaletteData[pRemapData[srcv]];
		};

		RLE_PROCESS_PIXEL_DATAS(false, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}


	BYTE* RemapData;
	BYTE* PaletteData;

};
