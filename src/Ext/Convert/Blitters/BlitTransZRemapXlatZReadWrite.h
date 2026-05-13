#pragma once

#include "Blitter.h"

DEFINE_BLITTER(BlitTransZRemapXlatZReadWrite, BlitterPixelWordOnly)
{
public:
	inline explicit BlitTransZRemapXlatZReadWrite(byte** remapData, WORD* data) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
	}

	virtual ~BlitTransZRemapXlatZReadWrite() override final = default;

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
		WORD* pPaletteData = this->PaletteData;
		WORD zWriteValue = static_cast<WORD>(zval);

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);

			const __m128i zero16 = _mm_setzero_si128();
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
					alignas(16) WORD remapIndexArray[ChunkSize];
					const byte* pRemap = *ppRemapData;
					for (int lane = 0; lane < ChunkSize; ++lane)
					{
						remapIndexArray[lane] = pRemap[src[lane]];
					}

					const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remapIndexArray));
					const __m128i srcColor16 = Sse2_GatherPaletteWord(remapIndex16, pPaletteData);
					const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
					const __m128i blended16 = Sse2_BlendU16(oldValue16, srcColor16, activeMask16);

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
			byte idx = *src++;
			if (zval < zbufv && idx)
			{
				*pDest = pPaletteData[(*ppRemapData)[idx]];
				zbufv = zWriteValue;
			}
			++pDest;

			ADJUST_POINTER(ZBuffer::Instance, zbuf);
		}
	}
	byte** RemapData;
	WORD* PaletteData;
};
