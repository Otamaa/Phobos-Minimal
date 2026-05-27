#pragma once

#include "Blitter.h"

DEFINE_RLE_BLITTER(RLEBlitTransZRemapXlatZReadWrite, BlitterPixelByteAndWord)
{
public:
	inline explicit RLEBlitTransZRemapXlatZReadWrite(byte** remapData, T* data) noexcept
	{
		this->RemapData = remapData;
		this->PaletteData = data;
	}

	virtual ~RLEBlitTransZRemapXlatZReadWrite() override final = default;

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
		T* pDest = reinterpret_cast<T*>(dst);
		byte** pRemapData = this->RemapData;
		T* pPaletteData = this->PaletteData;

		RLE_PROCESS_PRE_LINES(true, false, pDest, src, len, line, zbuf, abuf);
#if defined(YR_SIMD_COMPILE_AVX512)
		// AVX512 BYTE
		if constexpr (Level == Simd::Level::AVX512 && std::is_same_v<T, BYTE> && CompileAvx512)
		{
			constexpr int ChunkSize = 16;
			alignas(64) unsigned int paletteLut32[256];
			Avx512_BuildByteLut32(pPaletteData, paletteLut32);
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			alignas(64) unsigned int remappedIndices[ChunkSize];
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const __m512i zbaseVec = _mm512_set1_epi32(zbase);

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

						const __m128i zAdjustBytes = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pRunZAdjust));
						const __m512i zValues = _mm512_sub_epi32(zbaseVec, _mm512_cvtepi8_epi32(zAdjustBytes));
						const __m512i zbufValues = _mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(zbuf)));
						const __mmask16 zMask = _mm512_cmplt_epi32_mask(zValues, zbufValues);

						if (zMask)
						{
							byte* pCurrentRemap = *pRemapData;
							for (int i = 0; i < ChunkSize; ++i)
								remappedIndices[i] = pCurrentRemap[pRunSrc[i]];

							const __m512i remapIndex32 = _mm512_load_si512(reinterpret_cast<const __m512i*>(remappedIndices));
							const __m512i srcColors32 = _mm512_i32gather_epi32(remapIndex32, paletteLut32, 4);
							const __m128i result8 = Avx512_PackU32ToU8(srcColors32);
							const __m256i zValues16 = _mm512_cvtepi32_epi16(zValues);
							_mm_mask_storeu_epi8(pDest, zMask, result8);
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
							*pDest = pPaletteData[(*pRemapData)[*pRunSrc]];
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
#endif
		// AVX2 BYTE
		if constexpr (Level == Simd::Level::AVX2 && std::is_same_v<T, BYTE> && CompileAvx2)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			alignas(32) unsigned int paletteLut32[256];
			Avx2_BuildByteLut32(pPaletteData, paletteLut32);
			alignas(32) unsigned int remappedIndices[8];
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const __m256i zbaseVec = _mm256_set1_epi32(zbase);

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

						const __m128i zAdjustBytes = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pRunZAdjust));
						const __m256i zValues = _mm256_sub_epi32(zbaseVec, _mm256_cvtepi8_epi32(zAdjustBytes));
						const __m256i zbufValues = _mm256_cvtepu16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf)));
						const __m256i zMask32 = _mm256_cmpgt_epi32(zbufValues, zValues);
						const int zMask = _mm256_movemask_ps(_mm256_castsi256_ps(zMask32));

						if (zMask)
						{
							byte* pCurrentRemap = *pRemapData;
							for (int lane = 0; lane < ChunkSize; ++lane)
								remappedIndices[lane] = pCurrentRemap[pRunSrc[lane]];

							const __m256i remapIndex32 = _mm256_load_si256(reinterpret_cast<const __m256i*>(remappedIndices));
							const __m256i index32 = remapIndex32;
							const __m256i value32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(paletteLut32), index32, 4);
							const __m128i value8 = Avx2_PackU32ToU8(value32);
							Avx2_StoreMask8AndZ(pDest, zbuf, zMask, value8, zValues);
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
							*pDest = pPaletteData[(*pRemapData)[*pRunSrc]];
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

		// SSE2 BYTE
		if constexpr (Level == Simd::Level::SSE2 && std::is_same_v<T, BYTE>)
		{
			constexpr int ChunkSize = 8;
			constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
			ZBuffer* pZBuffer = ZBuffer::Instance;
			const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
			const __m128i zbaseVec = _mm_set1_epi16(static_cast<short>(zbase));

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
						const __m128i zValues16 = _mm_sub_epi16(zbaseVec, zAdjust16);
						const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
						const __m128i zMask16 = Sse2_CmpGtEpu16(zbuf16, zValues16);
						const int zMask = _mm_movemask_epi8(zMask16);

						if (zMask)
						{
							alignas(16) BYTE srcArray[16];
							alignas(16) WORD zValueArray[8];
							_mm_storel_epi64(reinterpret_cast<__m128i*>(srcArray), _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pRunSrc)));
							_mm_store_si128(reinterpret_cast<__m128i*>(zValueArray), zValues16);

							for (int lane = 0; lane < ChunkSize; ++lane)
							{
								if ((zMask & (3 << (lane * 2))) != 0)
								{
									pDest[lane] = pPaletteData[(*pRemapData)[srcArray[lane]]];
									zbuf[lane] = zValueArray[lane];
								}
							}
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
							pDest[0] = pPaletteData[(*pRemapData)[pRunSrc[0]]];
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
		if constexpr (Level == Simd::Level::SSE2 && std::is_same_v<T, WORD>)
		{
			const bool canUseVectorZ = zbase >= 127 && zbase <= 65407;
			if (canUseVectorZ)
			{
				constexpr int ChunkSize = 8;
				constexpr uintptr_t ChunkBytes = ChunkSize * sizeof(WORD);
				alignas(16) WORD remappedIndices[ChunkSize];
				ZBuffer* pZBuffer = ZBuffer::Instance;
				const uintptr_t zTailAddress = reinterpret_cast<uintptr_t>(pZBuffer->BufferTail);
				const __m128i zbaseVec16 = _mm_set1_epi16(static_cast<short>(zbase));

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

							const __m128i zAdjust16 = Sse2_Expand8ToEpi16Signed(pRunZAdjust);
							const __m128i zValues16 = _mm_sub_epi16(zbaseVec16, zAdjust16);
							const __m128i zbuf16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(zbuf));
							const __m128i zMask16 = Sse2_CmpGtEpu16(zbuf16, zValues16);

							if (_mm_movemask_epi8(zMask16))
							{
								byte* pCurrentRemap = *pRemapData;
								for (int i = 0; i < ChunkSize; ++i)
								{
									remappedIndices[i] = static_cast<WORD>(pCurrentRemap[pRunSrc[i]]);
								}

								const __m128i remapIndex16 = _mm_load_si128(reinterpret_cast<const __m128i*>(remappedIndices));
								const __m128i srcColors16 = Sse2_GatherPaletteWord(remapIndex16, pPaletteData);
								const __m128i oldValue16 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pDest));
								const __m128i blended16 = Sse2_BlendU16(oldValue16, srcColors16, zMask16);
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
								*pDest = pPaletteData[(*pRemapData)[*pRunSrc]];
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
		auto handler = [pRemapData, pPaletteData](T& dest, byte srcv, int zbase, WORD& zbufv, byte zadjustv)
		{
			int zval = zbase - static_cast<int>(static_cast<signed char>(zadjustv));
			if (zval < zbufv)
			{
				dest = pPaletteData[(*pRemapData)[srcv]];
				zbufv = static_cast<WORD>(zval);
			}
		};

		RLE_PROCESS_PIXEL_DATAS(true, false, pDest, src, len, zbase, zbuf, abuf, zadjust, handler);
	}
	
	byte** RemapData;
	T* PaletteData;
};
