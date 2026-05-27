#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransZRemapXlatAlphaZRead, BlitterPixelWordOnly)
{
public:
	inline explicit RLEBlitTransZRemapXlatAlphaZRead(byte** remapData, WORD* data, int shadecount) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
		this->AlphaRemapper = AlphaLightingRemapClass::Global->FindOrAllocate(shadecount);
	}

	virtual ~RLEBlitTransZRemapXlatAlphaZRead() override final = default;

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
		byte** pRemapData = this->RemapData;
		WORD* pPaletteData = this->PaletteData;
		WORD* pAData = LOOKUP_ALPHA_REMAPPER(alvl, this->AlphaRemapper);

		RLE_PROCESS_PRE_LINES(true, true, pDest, src, len, line, zbuf, abuf);

		// SSE2
		if constexpr (Level == Simd::Level::SSE2)
		{
			const bool canUseVectorZ = zbase >= 127 && zbase <= 65407;
			if (canUseVectorZ)
			{
				constexpr int ChunkSize = 8;
				constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
				ZBuffer* pZBuffer = ZBuffer::Instance;
				ABuffer* pABuffer = ABuffer::Instance;
				const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
				const uintptr_t aTailAddress = reinterpret_cast<uintptr_t>(pABuffer->BufferTail);
				const __m128i zbaseVec16 = _mm_set1_epi16(static_cast<short>(zbase));
				const byte* pRemapTable = *pRemapData;

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
							const uintptr_t aAddress = reinterpret_cast<uintptr_t>(abuf);
							if (zAddress + ChunkBytes > zTailAddress || aAddress + ChunkBytes > aTailAddress)
							{
								break;
							}

							const __m128i zAdjust16 = Sse2_Expand8ToEpi16Signed(pRunZAdjust);
							const __m128i zValues16 = _mm_sub_epi16(zbaseVec16, zAdjust16);
							const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
							const __m128i zMask16 = Sse2_CmpGtEpu16(zbuf16, zValues16);

							if (_mm_movemask_epi8(zMask16))
							{
								alignas(16) WORD remapIndices[ChunkSize];
								for (int i = 0; i < ChunkSize; ++i)
								{
									remapIndices[i] = pRemapTable[pRunSrc[i]];
								}

								const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remapIndices));
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
								const __m128i blended16 = Sse2_BlendU16(oldValue16, result16, zMask16);
								_mm_storeu_si128(reinterpret_cast<__m128i*>(pDest), blended16);
							}

							pRunSrc += ChunkSize;
							pRunZAdjust += ChunkSize;
							pDest += ChunkSize;
							zbuf += ChunkSize;
							abuf += ChunkSize;
							remaining -= ChunkSize;
							ADJUST_POINTER(pZBuffer, zbuf);
							ADJUST_POINTER(pABuffer, abuf);
						}

						while (remaining--)
						{
							int zval = zbase - static_cast<int>(static_cast<signed char>(*pRunZAdjust++));
							if (zval < *zbuf)
								*pDest = pPaletteData[pRemapTable[*pRunSrc] | pAData[*abuf]];

							++pRunSrc;
							++pDest;
							++zbuf;
							++abuf;
							ADJUST_POINTER(pZBuffer, zbuf);
							ADJUST_POINTER(pABuffer, abuf);
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
						abuf += off;
						ADJUST_POINTER(pZBuffer, zbuf);
						ADJUST_POINTER(pABuffer, abuf);
					}
				}

				return;
			}
		}

		// Scalar
		auto handler = [pRemapData, pPaletteData, pAData](WORD& dest, byte srcv, int zbase, WORD zbufv, byte zadjustv, WORD abufv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
				dest = pPaletteData[(*pRemapData)[srcv] | pAData[abufv]];
		};

		RLE_PROCESS_PIXEL_DATAS(true, true, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	byte** RemapData;
	WORD* PaletteData;
	AlphaLightingRemapClass* AlphaRemapper;
};
