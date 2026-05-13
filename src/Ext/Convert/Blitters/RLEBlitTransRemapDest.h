#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransRemapDest, BlitterPixelByteOnly)
{
public:
	inline explicit RLEBlitTransRemapDest(BYTE* data) noexcept
	{
		this->RemapDest = data;
	}

	virtual ~RLEBlitTransRemapDest() override final = default;

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
		BYTE* pDest = reinterpret_cast<BYTE*>(dst);
		BYTE* pRemapDest = this->RemapDest;

		RLE_PROCESS_PRE_LINES(false, false, pDest, src, len, line, zbuf, abuf);

#if defined(YR_SIMD_COMPILE_AVX512)
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			alignas(64) unsigned int remapLut32[256];
			Avx512_BuildByteLut32(pRemapDest, remapLut32);

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
						const __m512i index32 = Avx512_Expand16ToEpi32(reinterpret_cast<const byte*>(pDest));
						const __m512i remapped32 = _mm512_i32gather_epi32(index32, remapLut32, 4);
						const __m128i result8 = Avx512_PackU32ToU8(remapped32);
						_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), result8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = pRemapDest[*pDest];
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
#endif

		// AVX2 BYTE
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			alignas(32) unsigned int remapLut32[256];
			Avx2_BuildByteLut32(pRemapDest, remapLut32);
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
						const __m256i index32 = Avx2_Expand8ToEpi32(reinterpret_cast<const byte*>(pDest));
						const __m256i value32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(remapLut32), index32, 4);
						const __m128i value8 = Avx2_PackU32ToU8(value32);
						_mm_storel_epi64(reinterpret_cast<__m128i*>(pDest), value8);

						pRunSrc += ChunkSize;
						pDest += ChunkSize;
						remaining -= ChunkSize;
					}

					while (remaining--)
					{
						*pDest = pRemapDest[*pDest];
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



		auto handler = [pRemapDest](BYTE& pDest, byte srcv)
		{
			pDest = pRemapDest[pDest];
		};

		RLE_PROCESS_PIXEL_DATAS(false, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	BYTE* RemapDest;

};
