#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransXlatMultWriteAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransXlatMultWriteAlpha() noexcept
	{

	}

	virtual ~BlitTransXlatMultWriteAlpha() override final = default;

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

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero = _mm_setzero_si128();
			const __m512i alvlVec = _mm512_set1_epi32(alvl);
			const __m512i zvalVec = _mm512_set1_epi32(zval);
			const __m512i max255 = _mm512_set1_epi32(255);

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __mmask16 activeMask = _mm_cmpneq_epu8_mask(srcBytes, zero);
				if (activeMask)
				{
					const __m512i srcValues = _mm512_cvtepu8_epi32(srcBytes);
					const __m512i multValues = _mm512_mullo_epi32(srcValues, alvlVec);
					const __m512i alphaValues = _mm512_min_epi32(_mm512_add_epi32(multValues, zvalVec), max255);
					const __m256i alphaValues16 = _mm512_cvtepi32_epi16(alphaValues);

					_mm256_mask_storeu_epi16(abuf, activeMask, alphaValues16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pABuffer, abuf);
			}
		}

		// AVX2
		if constexpr (Level == Simd::Level::AVX2 && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m256i zero32 = _mm256_setzero_si256();
			const __m256i alvlVec32 = _mm256_set1_epi32(alvl);
			const __m256i zvalVec32 = _mm256_set1_epi32(zval);
			const __m256i max255 = _mm256_set1_epi32(255);

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m256i srcValues32 = Avx2_Expand8ToEpi32(src);
				const __m256i activeMask32 = _mm256_cmpgt_epi32(srcValues32, zero32);
				if (_mm256_movemask_epi8(activeMask32))
				{
					const __m256i multValues = _mm256_mullo_epi32(srcValues32, alvlVec32);
					const __m256i alphaValues = _mm256_min_epi32(_mm256_add_epi32(multValues, zvalVec32), max255);
					const __m128i alphaValues16 = Avx2_PackU32ToU16(alphaValues);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldAlpha16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(abuf));
					const __m128i blendedAlpha16 = Avx2_BlendU16(oldAlpha16, alphaValues16, writeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(abuf), blendedAlpha16);
				}

				src += ChunkSize;
				pDest += ChunkSize;
				abuf += ChunkSize;
				len -= ChunkSize;
				ADJUST_POINTER(pABuffer, abuf);
			}
		}


		// Scalar
		while (len--)
		{
			if (byte idx = *src++)
				*abuf = static_cast<WORD>(std::min(idx * alvl + zval, 255));
			++abuf;
			++pDest;

			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
};
