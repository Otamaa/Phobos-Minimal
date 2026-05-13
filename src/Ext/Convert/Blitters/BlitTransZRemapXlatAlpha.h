#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransZRemapXlatAlpha, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransZRemapXlatAlpha(byte** remapData, WORD* data, int shadecount) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~BlitTransZRemapXlatAlpha() override final = default;

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
		byte** ppRemapData = this->RemapData;
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);
		WORD* pPaletteData = this->PaletteData;

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ABuffer* pABuffer = ABuffer::Instance;
			const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
			const __m128i zero16 = _mm_setzero_si128();

			while (len >= ChunkSize)
			{
				const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
				if (aAddress + ChunkBytes > aTailAddress)
					break;

				const __m128i srcIndex16 = Sse2_Expand8ToEpi16(src);
				const __m128i activeMask16 = _mm_cmpgt_epi16(srcIndex16, zero16);
				if (_mm_movemask_epi8(activeMask16))
				{
					const byte* pRemap = *ppRemapData;
					alignas(16) WORD remapIndexArray[ChunkSize];
					for (int lane = 0; lane < ChunkSize; ++lane)
					{
						remapIndexArray[lane] = pRemap[src[lane]];
					}

					const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remapIndexArray));
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

					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, activeMask16);
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
				*pDest = pPaletteData[(*ppRemapData)[idx] | pAData[*abuf]];
			++pDest;
			++abuf;

			ADJUST_POINTER(ABuffer::Instance, abuf);
		}
	}
	byte** RemapData;
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
