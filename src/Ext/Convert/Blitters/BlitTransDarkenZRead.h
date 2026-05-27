#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransDarkenZRead, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransDarkenZRead(WORD mask) noexcept
	{
		this->Mask = mask;
	}

	virtual ~BlitTransDarkenZRead() override final = default;

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
		WORD mask = this->Mask;

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);

			constexpr int ChunkSize = 32;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			const __m256i zero = _mm256_setzero_si256();
			const __m512i darkMask = _mm512_set1_epi16(static_cast<short>(mask));
			const __m512i zvalVec = _mm512_set1_epi16(static_cast<short>(zval));

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				if (zAddress + ChunkBytes > zTailAddress)
				{
					break;
				}

				const __m512i zbufValues = _mm512_loadu_si512(reinterpret_cast<const void*>(zbuf));
				const __mmask32 srcMask = _mm256_cmpneq_epu8_mask(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src)), zero);

				__mmask32 zMask = 0;
				if (zval < 0)
				{
					zMask = static_cast<__mmask32>(0xFFFFFFFFu);
				}
				else if (zval <= 0xFFFF)
				{
					zMask = _mm512_cmpgt_epu16_mask(zbufValues, zvalVec);
				}

				const __mmask32 activeMask = srcMask & zMask;
				if (activeMask)
				{
					const __m512i destValues = _mm512_loadu_si512(reinterpret_cast<const void*>(pDest));
					const __m512i darkenedValues = _mm512_and_si512(_mm512_srli_epi16(destValues, 1), darkMask);
					_mm512_mask_storeu_epi16(pDest, activeMask, darkenedValues);
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
			const __m256i darkMask32 = _mm256_set1_epi32(static_cast<int>(mask));
			const __m256i zvalVec32 = _mm256_set1_epi32(zval);

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
					const __m256i dest32 = Avx2_Load8WordAsEpi32(pDest);
					const __m256i result32 = _mm256_and_si256(_mm256_srli_epi32(dest32, 1), darkMask32);

					const __m128i result16 = Avx2_PackU32ToU16(result32);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);

					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
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
			const __m128i darkMask16 = _mm_set1_epi16(static_cast<short>(mask));
			const __m128i bias16 = _mm_set1_epi16(static_cast<short>(-32768));
			const __m128i zvalVec16 = _mm_set1_epi16(static_cast<short>(zval));
			const __m128i zvalBiased16 = _mm_xor_si128(zvalVec16, bias16);

			while (len >= ChunkSize)
			{
				const uintptr_t zAddress = reinterpret_cast<uintptr_t>(zbuf);
				if (zAddress + ChunkBytes > zTailAddress)
					break;

				const __m128i src16 = Sse2_Expand8ToEpi16(src);
				const __m128i srcMask16 = _mm_cmpgt_epi16(src16, zero16);

				__m128i zMask16 = _mm_setzero_si128();
				if (zval < 0)
				{
					zMask16 = _mm_set1_epi16(static_cast<short>(-1));
				}
				else if (zval <= 0xFFFF)
				{
					const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
					const __m128i zbufBiased16 = _mm_xor_si128(zbuf16, bias16);
					zMask16 = _mm_cmpgt_epi16(zbufBiased16, zvalBiased16);
				}

				const __m128i activeMask16 = _mm_and_si128(srcMask16, zMask16);
				if (_mm_movemask_epi8(activeMask16))
				{
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i result16 = _mm_and_si128(_mm_srli_epi16(oldValue16, 1), darkMask16);
					const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, activeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
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
			WORD zbufv = *zbuf++;
			if (zval < zbufv)
			{
				if (*src)
					*pDest = mask & (*pDest >> 1);
			}
			++src;
			++pDest;

			ADJUST_POINTER(ZBuffer::Instance, zbuf);
		}
	}
	WORD Mask;
};
