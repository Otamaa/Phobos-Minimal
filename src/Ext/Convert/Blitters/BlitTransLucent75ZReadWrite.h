#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransLucent75ZReadWrite, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransLucent75ZReadWrite(WORD* data, WORD mask) noexcept
	{
		this->PaletteData = data;
		this->Mask = mask;
	}

	virtual ~BlitTransLucent75ZReadWrite() override final = default;

	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Impl(dst, src, len, zval, zbuf, abuf, alvl, warp);
	}

private:
	__forceinline void Blit_Impl(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		if (len < 0)
			return;

		WORD* pDest = reinterpret_cast<WORD*>(dst);
		WORD* pPaletteData = this->PaletteData;
		WORD mask = this->Mask;
		WORD zWriteValue = static_cast<WORD>(zval);

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);

			const __m128i zero = _mm_setzero_si128();
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i blendMask = _mm512_set1_epi32(static_cast<int>(mask));
			const __m256i zvalVec = _mm256_set1_epi16(static_cast<short>(zval));
			const __m256i zWriteVec = _mm256_set1_epi16(static_cast<short>(zWriteValue));

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				if (zAddress + ChunkBytes > zTailAddress)
				{
					break;
				}

				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __mmask16 srcMask = _mm_cmpneq_epu8_mask(srcBytes, zero);

				__mmask16 zMask = 0;
				if (zval < 0)
				{
					zMask = static_cast<__mmask16>(0xFFFF);
				}
				else if (zval <= 0xFFFF)
				{
					zMask = _mm256_cmpgt_epu16_mask(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(zbuf)), zvalVec);
				}

				const __mmask16 activeMask = srcMask & zMask;
				if (activeMask)
				{
					const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
					const __mmask16 maxIndexMask = _mm_cmpeq_epu8_mask(srcBytes, _mm_set1_epi8(static_cast<char>(0xFF)));
					const __mmask16 gatherMask = activeMask & ~maxIndexMask;
					__m512i srcColors = _mm512_setzero_si512();
					if (gatherMask)
					{
						srcColors = _mm512_mask_i32gather_epi32(srcColors, gatherMask, srcIndices, pPaletteData, 2);
					}
					srcColors = _mm512_and_si512(srcColors, low16Mask);
					const __mmask16 fillMask = activeMask & maxIndexMask;
					if (fillMask)
					{
						srcColors = _mm512_mask_set1_epi32(srcColors, fillMask, static_cast<int>(pPaletteData[255]));
					}
					const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));

					const __m512i destQuarter = _mm512_and_si512(_mm512_srli_epi32(destColors, 2), blendMask);
					const __m512i srcQuarter = _mm512_and_si512(_mm512_srli_epi32(srcColors, 2), blendMask);
					const __m512i result32 = _mm512_add_epi32(_mm512_add_epi32(_mm512_add_epi32(destQuarter, destQuarter), destQuarter), srcQuarter);
					const __m256i result16 = _mm512_cvtusepi32_epi16(result32);

					_mm256_mask_storeu_epi16(pDest, activeMask, result16);
					_mm256_mask_storeu_epi16(zbuf, activeMask, zWriteVec);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);

			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i blendMask32 = _mm256_set1_epi32(static_cast<int>(mask));
			const __m256i zvalVec32 = _mm256_set1_epi32(zval);
			const __m128i zWriteVec16 = _mm_set1_epi16(static_cast<short>(zWriteValue));

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				if (zAddress + ChunkBytes > zTailAddress)
					break;

				const __m256i srcIndex32 = Avx2_Expand8ToEpi32(src);
				const __m256i srcMask32 = _mm256_cmpgt_epi32(srcIndex32, zero32);

				__m256i zMask32 = _mm256_setzero_si256();
				if (zval < 0)
				{
					zMask32 = _mm256_set1_epi32(-1);
				}
				else if (zval <= 0xFFFF)
				{
					const __m256i zbuf32 = Avx2_Load8WordAsEpi32(zbuf);
					zMask32 = _mm256_cmpgt_epi32(zbuf32, zvalVec32);
				}

				const __m256i activeMask32 = _mm256_and_si256(srcMask32, zMask32);
				if (_mm256_movemask_epi8(activeMask32))
				{
					const __m256i srcColor32 = Avx2_GatherPaletteWord(srcIndex32, pPaletteData);
					const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);

					const __m256i destQuarter32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 2), blendMask32);
					const __m256i srcQuarter32 = _mm256_and_si256(_mm256_srli_epi32(srcColor32, 2), blendMask32);
					const __m256i result32 = _mm256_add_epi32(_mm256_add_epi32(_mm256_add_epi32(destQuarter32, destQuarter32), destQuarter32), srcQuarter32);

					const __m128i result16 = Avx2_PackU32ToU16(result32);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);
					const __m128i oldZ16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
					const __m128i zWrite16 = Avx2_BlendU16(oldZ16, zWriteVec16, writeMask16);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(zbuf), zWrite16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
			}
		}

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);

			const __m128i zero16 = _mm_setzero_si128();
			const __m128i blendMask16 = _mm_set1_epi16(static_cast<short>(mask));
			const __m128i zWriteVec16 = _mm_set1_epi16(static_cast<short>(zWriteValue));

			__m128i zMask16 = _mm_setzero_si128();
			if (zval < 0)
			{
				zMask16 = _mm_set1_epi16(static_cast<short>(-1));
			}
			else if (zval <= 0xFFFF)
			{
				zMask16 = _mm_set1_epi16(static_cast<short>(zval));
			}

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				if (zAddress + ChunkBytes > zTailAddress)
					break;

				const __m128i srcIndex16 = Sse2_Expand8ToEpi16(src);
				const __m128i srcMask16 = _mm_cmpgt_epi16(srcIndex16, zero16);

				__m128i activeMask16 = _mm_setzero_si128();
				if (zval < 0)
				{
					activeMask16 = srcMask16;
				}
				else if (zval <= 0xFFFF)
				{
					const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
					const __m128i zCmpMask16 = Sse2_CmpGtEpu16(zbuf16, zMask16);
					activeMask16 = _mm_and_si128(srcMask16, zCmpMask16);
				}

				if (_mm_movemask_epi8(activeMask16))
				{
					const __m128i srcColor16 = Sse2_GatherPaletteWord(srcIndex16, pPaletteData);
					const __m128i dest16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i destQuarter16 = _mm_and_si128(_mm_srli_epi16(dest16, 2), blendMask16);
					const __m128i srcQuarter16 = _mm_and_si128(_mm_srli_epi16(srcColor16, 2), blendMask16);
					const __m128i result16 = _mm_add_epi16(_mm_add_epi16(_mm_add_epi16(destQuarter16, destQuarter16), destQuarter16), srcQuarter16);
					const __m128i blended16 = Sse2_BlendU16(dest16, result16, activeMask16);

					const __m128i oldZ16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
					const __m128i zBlended16 = Sse2_BlendU16(oldZ16, zWriteVec16, activeMask16);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(zbuf), zBlended16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				zbuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pZBuffer, zbuf);
			}
		}

		// Scalar
		while (len--)
		{
			WORD& zbufv = *zbuf++;
			if (zval < zbufv)
			{
				if (byte idx = *src)
				{
					*pDest = 3 * (mask & (*pDest >> 2)) + (mask & (pPaletteData[idx] >> 2));
					zbufv = zWriteValue;
				}
			}
			++src;
			++pDest;

			ADJUST_POINTER(ZBuffer::Instance, zbuf);
		}
	}
	WORD* PaletteData;
	WORD Mask;
};
