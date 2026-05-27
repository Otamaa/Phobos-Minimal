#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransDarkenZReadWrite, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransDarkenZReadWrite(WORD mask) noexcept
	{
		this->Mask = mask;
	}

	virtual ~RLEBlitTransDarkenZReadWrite() override final = default;

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
		WORD mask = this->Mask;

		RLE_PROCESS_PRE_LINES(true, false, pDest, src, len, line, zbuf, abuf);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
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
						if (zAddress + ChunkBytes > zTailAddress)
						{
							break;
						}

						const __m128i zAdjustBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pRunZAdjust));
						const __m512i zValues = _mm512_sub_epi32(zbaseVec, _mm512_cvtepi8_epi32(zAdjustBytes));
						const __m512i zbufValues = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(zbuf)));
						const __mmask16 zMask = _mm512_cmplt_epi32_mask(zValues, zbufValues);

						if (zMask)
						{
							const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));
							const __m512i result32 = _mm512_and_si512(_mm512_srli_epi32(destColors, 1), blendMask);
							const __m256i result16 = _mm512_cvtepi32_epi16(result32);
							const __m256i zValues16 = _mm512_cvtepi32_epi16(zValues);
							_mm256_mask_storeu_epi16(pDest, zMask, result16);
							_mm256_mask_storeu_epi16(zbuf, zMask, zValues16);
						}

						pRunSrc += ChunkSize;
						pRunZAdjust += ChunkSize;
						pDest += ChunkSize;
						zbuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pZBuffer, zbuf);
					}

					while (remaining--)
					{
						int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
						if (zval < *zbuf)
						{
							*pDest = mask & (*pDest >> 1);
							*zbuf = static_cast<WORD>(zval);
						}

						++pRunSrc;
						++pDest;
						++zbuf;
						ADJUST_POINTER(pZBuffer, zbuf);
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
					ADJUST_POINTER(pZBuffer, zbuf);
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
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const __m256i zbaseVec32 = _mm256_set1_epi32(zbase);
			const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));
			const __m256i low16Mask32 = _mm256_set1_epi32(0xFFFF);

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
						if (zAddress + ChunkBytes > zTailAddress)
						{
							break;
						}

						const __m256i zAdjust32 = _mm256_cvtepi8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pRunZAdjust)));
						const __m256i zValues32 = _mm256_sub_epi32(zbaseVec32, zAdjust32);
						const __m256i zbuf32 = Avx2_Load8WordAsEpi32(zbuf);
						const __m256i zMask32 = _mm256_cmpgt_epi32(zbuf32, zValues32);

						if (_mm256_movemask_epi8(zMask32))
						{
							const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);
							const __m256i result32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 1), blendMask32);
							const __m128i result16 = Avx2_PackU32ToU16(result32);
							const __m128i writeMask16 = Avx2_PackMask32ToI16(zMask32);
							const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
							const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);

							const __m256i zWrite32 = _mm256_and_si256(zValues32, low16Mask32);
							const __m128i zWrite16 = Avx2_PackU32ToU16(zWrite32);
							const __m128i oldZ16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
							const __m128i zBlended16 = Avx2_BlendU16(oldZ16, zWrite16, writeMask16);

							_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
							_mm_storeu_si128(reinterpret_cast<__m128i*>(zbuf), zBlended16);
						}

						pRunSrc += ChunkSize;
						pRunZAdjust += ChunkSize;
						pDest += ChunkSize;
						zbuf += ChunkSize;
						remaining -= ChunkSize;
						ADJUST_POINTER(pZBuffer, zbuf);
					}

					while (remaining--)
					{
						int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
						if (zval < *zbuf)
						{
							*pDest = mask & (*pDest >> 1);
							*zbuf = static_cast<WORD>(zval);
						}

						++pRunSrc;
						++pDest;
						++zbuf;
						ADJUST_POINTER(pZBuffer, zbuf);
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
					ADJUST_POINTER(pZBuffer, zbuf);
				}
			}

			return;
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const __m128i zbaseVec16 = _mm_set1_epi16(static_cast<short>(zbase));
			const __m128i blendMask16 = _mm_set1_epi16(static_cast<short>(mask));
			const bool canUseVectorZ = zbase >= 127 && zbase <= 65407;

			if (canUseVectorZ)
			{
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
							if (zAddress + ChunkBytes > zTailAddress)
								break;

							const __m128i zAdjust16 = Sse2_Expand8ToEpi16Signed(pRunZAdjust);
							const __m128i zValues16 = _mm_sub_epi16(zbaseVec16, zAdjust16);
							const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
							const __m128i zMask16 = Sse2_CmpGtEpu16(zbuf16, zValues16);

							if (_mm_movemask_epi8(zMask16))
							{
								const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
								const __m128i result16 = _mm_and_si128(_mm_srli_epi16(oldValue16, 1), blendMask16);
								const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, zMask16);
								const __m128i zBlended16 = Sse2_BlendU16(zbuf16, zValues16, zMask16);
								_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
								_mm_storeu_si128(reinterpret_cast<__m128i*>(zbuf), zBlended16);
							}

							pRunSrc += ChunkSize;
							pRunZAdjust += ChunkSize;
							pDest += ChunkSize;
							zbuf += ChunkSize;
							remaining -= ChunkSize;
							ADJUST_POINTER(pZBuffer, zbuf);
						}

						while (remaining--)
						{
							int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
							if (zval < *zbuf)
							{
								*pDest = mask & (*pDest >> 1);
								*zbuf = static_cast<WORD>(zval);
							}

							++pRunSrc;
							++pDest;
							++zbuf;
							ADJUST_POINTER(pZBuffer, zbuf);
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
						ADJUST_POINTER(pZBuffer, zbuf);
					}
				}

				return;
			}
		}

		// Scalar
		auto handler = [mask](WORD& dest, byte /*srcv*/, int zbase, WORD& zbufv, byte zadjustv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
			{
				dest = mask & (dest >> 1);
				zbufv = static_cast<WORD>(zval);
			}
		};

		RLE_PROCESS_PIXEL_DATAS(true, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	WORD Mask;
};
