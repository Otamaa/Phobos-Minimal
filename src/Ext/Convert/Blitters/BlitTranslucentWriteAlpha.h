#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTranslucentWriteAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTranslucentWriteAlpha(WORD* data) noexcept
	{
		this->PaletteData = data;
	}

	virtual ~BlitTranslucentWriteAlpha() override final = default;

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

		// AVX512
		if constexpr (Level == Simd::Level::AVX512 && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);

			const __m128i zero8 = _mm_setzero_si128();
			const __m512i low16Mask = _mm512_set1_epi32(0xFFFF);
			const __m512i low8Mask = _mm512_set1_epi32(0xFF);
			const __m512i max255 = _mm512_set1_epi32(255);

			const __m512i redShiftLeft = _mm512_set1_epi32(RGBClass::RedShiftLeft);
			const __m512i redShiftRight = _mm512_set1_epi32(RGBClass::RedShiftRight);
			const __m512i greenShiftLeft = _mm512_set1_epi32(RGBClass::GreenShiftLeft);
			const __m512i greenShiftRight = _mm512_set1_epi32(RGBClass::GreenShiftRight);
			const __m512i blueShiftLeft = _mm512_set1_epi32(RGBClass::BlueShiftLeft);
			const __m512i blueShiftRight = _mm512_set1_epi32(RGBClass::BlueShiftRight);

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m128i srcBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				const __mmask16 activeMask = _mm_cmpneq_epu8_mask(srcBytes, zero8);

				if (activeMask)
				{
					const __m512i srcIndices = _mm512_cvtepu8_epi32(srcBytes);
					__m512i srcColors = _mm512_setzero_si512();
					srcColors = _mm512_mask_i32gather_epi32(srcColors, activeMask, srcIndices, pPaletteData, 2);
					srcColors = _mm512_and_si512(srcColors, low16Mask);

					const __m512i destColors = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(pDest)));
					const __m512i alphaRaw = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(abuf)));
					const __m512i resAlpha = _mm512_and_si512(_mm512_sub_epi32(max255, alphaRaw), low16Mask);
					const __mmask16 alpha255Mask = _mm512_cmpeq_epi32_mask(alphaRaw, max255);
					const __m512i alpha = _mm512_mask_set1_epi32(alphaRaw, alpha255Mask, 256);

					const __m512i fgRed = _mm512_sllv_epi32(_mm512_srlv_epi32(srcColors, redShiftLeft), redShiftRight);
					const __m512i fgGreen = _mm512_sllv_epi32(_mm512_srlv_epi32(srcColors, greenShiftLeft), greenShiftRight);
					const __m512i fgBlue = _mm512_sllv_epi32(_mm512_srlv_epi32(srcColors, blueShiftLeft), blueShiftRight);

					const __m512i bgRed = _mm512_sllv_epi32(_mm512_srlv_epi32(destColors, redShiftLeft), redShiftRight);
					const __m512i bgGreen = _mm512_sllv_epi32(_mm512_srlv_epi32(destColors, greenShiftLeft), greenShiftRight);
					const __m512i bgBlue = _mm512_sllv_epi32(_mm512_srlv_epi32(destColors, blueShiftLeft), blueShiftRight);

					const __m512i redBlend = _mm512_srli_epi32(_mm512_add_epi32(_mm512_mullo_epi32(fgRed, alpha), _mm512_mullo_epi32(bgRed, resAlpha)), 8);
					const __m512i greenBlend = _mm512_srli_epi32(_mm512_add_epi32(_mm512_mullo_epi32(fgGreen, alpha), _mm512_mullo_epi32(bgGreen, resAlpha)), 8);
					const __m512i blueBlend = _mm512_srli_epi32(_mm512_add_epi32(_mm512_mullo_epi32(fgBlue, alpha), _mm512_mullo_epi32(bgBlue, resAlpha)), 8);

					const __m512i redPacked = _mm512_sllv_epi32(_mm512_srlv_epi32(_mm512_and_si512(redBlend, low8Mask), redShiftRight), redShiftLeft);
					const __m512i greenPacked = _mm512_sllv_epi32(_mm512_srlv_epi32(_mm512_and_si512(greenBlend, low8Mask), greenShiftRight), greenShiftLeft);
					const __m512i bluePacked = _mm512_sllv_epi32(_mm512_srlv_epi32(_mm512_and_si512(blueBlend, low8Mask), blueShiftRight), blueShiftLeft);

					const __m512i resultColors = _mm512_or_si512(_mm512_or_si512(redPacked, greenPacked), bluePacked);
					const __m256i result16 = _mm512_cvtepi32_epi16(resultColors);

					_mm256_mask_storeu_epi16(pDest, activeMask, result16);
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
			const __m256i low16Mask = _mm256_set1_epi32(0xFFFF);
			const __m256i low8Mask = _mm256_set1_epi32(0xFF);
			const __m256i max255 = _mm256_set1_epi32(255);
			const __m256i twoFiftySix = _mm256_set1_epi32(256);

			const __m256i redShiftLeft = _mm256_set1_epi32(RGBClass::RedShiftLeft);
			const __m256i redShiftRight = _mm256_set1_epi32(RGBClass::RedShiftRight);
			const __m256i greenShiftLeft = _mm256_set1_epi32(RGBClass::GreenShiftLeft);
			const __m256i greenShiftRight = _mm256_set1_epi32(RGBClass::GreenShiftRight);
			const __m256i blueShiftLeft = _mm256_set1_epi32(RGBClass::BlueShiftLeft);
			const __m256i blueShiftRight = _mm256_set1_epi32(RGBClass::BlueShiftRight);

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
				{
					break;
				}

				const __m256i srcIndices = Avx2_Expand8ToEpi32(src);
				const __m256i activeMask32 = _mm256_cmpgt_epi32(srcIndices, zero32);
				if (_mm256_movemask_epi8(activeMask32))
				{
					const __m256i srcColors = Avx2_GatherPaletteWord(srcIndices, pPaletteData);
					const __m256i destColors = Avx2_Load8WordAsEpi32(pDest);
					const __m256i alphaRaw = Avx2_Load8WordAsEpi32(abuf);
					const __m256i resAlpha = _mm256_and_si256(_mm256_sub_epi32(max255, alphaRaw), low16Mask);
					const __m256i alpha255Mask = _mm256_cmpeq_epi32(alphaRaw, max255);
					const __m256i alpha = _mm256_blendv_epi8(alphaRaw, twoFiftySix, alpha255Mask);

					const __m256i fgRed = _mm256_sllv_epi32(_mm256_srlv_epi32(srcColors, redShiftLeft), redShiftRight);
					const __m256i fgGreen = _mm256_sllv_epi32(_mm256_srlv_epi32(srcColors, greenShiftLeft), greenShiftRight);
					const __m256i fgBlue = _mm256_sllv_epi32(_mm256_srlv_epi32(srcColors, blueShiftLeft), blueShiftRight);

					const __m256i bgRed = _mm256_sllv_epi32(_mm256_srlv_epi32(destColors, redShiftLeft), redShiftRight);
					const __m256i bgGreen = _mm256_sllv_epi32(_mm256_srlv_epi32(destColors, greenShiftLeft), greenShiftRight);
					const __m256i bgBlue = _mm256_sllv_epi32(_mm256_srlv_epi32(destColors, blueShiftLeft), blueShiftRight);

					const __m256i redBlend = _mm256_srli_epi32(_mm256_add_epi32(_mm256_mullo_epi32(fgRed, alpha), _mm256_mullo_epi32(bgRed, resAlpha)), 8);
					const __m256i greenBlend = _mm256_srli_epi32(_mm256_add_epi32(_mm256_mullo_epi32(fgGreen, alpha), _mm256_mullo_epi32(bgGreen, resAlpha)), 8);
					const __m256i blueBlend = _mm256_srli_epi32(_mm256_add_epi32(_mm256_mullo_epi32(fgBlue, alpha), _mm256_mullo_epi32(bgBlue, resAlpha)), 8);

					const __m256i redPacked = _mm256_sllv_epi32(_mm256_srlv_epi32(_mm256_and_si256(redBlend, low8Mask), redShiftRight), redShiftLeft);
					const __m256i greenPacked = _mm256_sllv_epi32(_mm256_srlv_epi32(_mm256_and_si256(greenBlend, low8Mask), greenShiftRight), greenShiftLeft);
					const __m256i bluePacked = _mm256_sllv_epi32(_mm256_srlv_epi32(_mm256_and_si256(blueBlend, low8Mask), blueShiftRight), blueShiftLeft);
					const __m256i resultColors = _mm256_or_si256(_mm256_or_si256(redPacked, greenPacked), bluePacked);
					const __m128i result16 = Avx2_PackU32ToU16(resultColors);
					const __m128i writeMask16 = Avx2_PackMask32ToI16(activeMask32);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Avx2_BlendU16(oldValue16, result16, writeMask16);
					_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
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
			{
				auto srcv = pPaletteData[idx];
				WORD alpha = *abuf;
				WORD resalpha = 255 - alpha;
				if (alpha == 255)
					alpha = 256;

				RGBClass fg { srcv, true };
				RGBClass bg { *pDest, true };

				RGBClass clr
				{
					(fg.Red * alpha + bg.Red * resalpha) >> 8,
					(fg.Green * alpha + bg.Green * resalpha) >> 8,
					(fg.Blue * alpha + bg.Blue * resalpha) >> 8
				};

				*pDest = static_cast<WORD>(clr.ToInit());
			}
			++abuf;
			++pDest;

			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
	WORD* PaletteData;
};
