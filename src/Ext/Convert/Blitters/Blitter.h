#pragma once

#include <Base/Always.h>

#include <Drawing.h>
#include <AlphaLightingRemapClass.h>
#include <Utilities/Simd.h>
#include <concepts>
#include <immintrin.h>
#include <type_traits>

// All of those blitters can be found at 48EBF0!

// All Westwood fucking jesus blitters goes here!
class Blitter
{
public:
	virtual ~Blitter() = default;
	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;
	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;
};

// And those are compressed one :(
class RLEBlitter
{
public:
	virtual ~RLEBlitter() = default;
	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) = 0;
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) = 0;
};

#define ADJUST_POINTER(buffer, ptr)                            \
{                                                              \
	if ((ptr) >= (buffer)->BufferTail)                         \
		reinterpret_cast<char*&>(ptr) -= (buffer)->BufferSize; \
};

#define RLE_PROCESS_PRE_LINES(UseZBuffer, UseABuffer, dest, src, len, line, zbuf, abuf) \
{                                                                                       \
	if ((line) > 0)                                                                     \
	{                                                                                   \
		int off = -(line);                                                              \
		do                                                                              \
		{                                                                               \
			if (*(src)++)                                                               \
				++off;                                                                  \
			else                                                                        \
				off += *(src)++;                                                        \
		}                                                                               \
		while (off < 0);                                                                \
		(dest) += off;                                                                  \
		(len) -= off;                                                                   \
		if constexpr (UseZBuffer)                                                       \
		{                                                                               \
			(zbuf) += off;                                                              \
			ADJUST_POINTER(ZBuffer::Instance, zbuf);                                    \
		}                                                                               \
		if constexpr (UseABuffer)                                                       \
		{                                                                               \
			(abuf) += off;                                                              \
			ADJUST_POINTER(ABuffer::Instance, abuf);                                    \
		}                                                                               \
	}                                                                                   \
};

#define RLE_PROCESS_PIXEL_DATAS(UseZBuffer, UseABuffer, dest, src, len, zbase, zbuf, abuf, zadjust, fn) \
{                                                                                                       \
	if ((len) >= 0)                                                                                     \
	{                                                                                                   \
		while ((len) > 0)                                                                               \
		{                                                                                               \
			if (byte srcv = *(src)++)                                                                   \
			{                                                                                           \
				if constexpr ((UseZBuffer) && (UseABuffer))                                             \
					(fn)(*(dest), srcv, (zbase), *(zbuf)++, *(zadjust)++, *(abuf)++);                   \
				else if constexpr ((UseZBuffer) && !(UseABuffer))                                       \
					(fn)(*(dest), srcv, (zbase), *(zbuf)++, *(zadjust)++);                              \
				else if constexpr (!(UseZBuffer) && (UseABuffer))                                       \
					(fn)(*(dest), srcv, *(abuf)++);                                                     \
				else                                                                                    \
					(fn)(*(dest), srcv);                                                                \
				++(dest);                                                                               \
				--(len);                                                                                \
			}                                                                                           \
			else                                                                                        \
			{                                                                                           \
				byte off = *(src)++;                                                                    \
				(len) -= off;                                                                           \
				(dest) += off;                                                                          \
				if constexpr (UseZBuffer)                                                               \
				{                                                                                       \
					(zbuf) += off;                                                                      \
					(zadjust) += off;                                                                   \
				}                                                                                       \
				if constexpr (UseABuffer)                                                               \
					(abuf) += off;                                                                      \
			}                                                                                           \
			if constexpr (UseZBuffer)                                                                   \
				ADJUST_POINTER(ZBuffer::Instance, zbuf);                                                \
			if constexpr (UseABuffer)                                                                   \
				ADJUST_POINTER(ABuffer::Instance, abuf);                                                \
		}                                                                                               \
	}                                                                                                   \
};

#define LOOKUP_ALPHA_REMAPPER(alvl, remapper) \
	((remapper)->Table[std::min(254, (261 * std::max(0, (alvl))) >> 11)])

template<typename T>
concept BlitterPixelByteAndWord = std::same_as<T, WORD> || std::same_as<T, BYTE>;

template<typename T>
concept BlitterPixelWordOnly = std::same_as<T, WORD>;

template<typename T>
concept BlitterPixelByteOnly = std::same_as<T, BYTE>;

#define DEFINE_BLITTER(x, PixelConstraint) \
template<PixelConstraint T, Simd::Level Level> \
class x final : public Blitter

#define DEFINE_RLE_BLITTER(x, PixelConstraint) \
template<PixelConstraint T, Simd::Level Level> \
class x final : public RLEBlitter

#define DEFINE_RLE_BLITTER_INHERITABLE(x, PixelConstraint) \
template<PixelConstraint T, Simd::Level Level> \
class x : public RLEBlitter

#if defined(YR_SIMD_COMPILE_AVX512)
inline constexpr bool CompileAvx512 = true;
#else
inline constexpr bool CompileAvx512 = false;
#endif

#if defined(YR_SIMD_COMPILE_AVX2)
inline constexpr bool CompileAvx2 = true;
#else
inline constexpr bool CompileAvx2 = false;
#endif

#if defined(YR_SIMD_COMPILE_AVX512)
#define Avx512_Expand16ToEpi32(pSrc) \
	_mm512_cvtepu8_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(pSrc)))

#define Avx512_PackU32ToU8(value32) \
	_mm512_cvtusepi32_epi8((value32))

#define Avx512_BuildByteLut32(pTable, pLut32) \
{                                             \
	for (int i = 0; i < 256; ++i)             \
		(pLut32)[i] = (pTable)[i];            \
}
#endif

#define Sse2_Expand8ToEpi16(pSrc) \
	_mm_unpacklo_epi8(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pSrc)), _mm_setzero_si128())

#define Sse2_GatherPaletteWord(srcIndex16, pPaletteData)                          \
	([](const __m128i srcIndex16Local, const WORD* pPaletteDataLocal) -> __m128i  \
	{                                                                             \
		alignas(16) WORD indexArray[8];                                           \
		_mm_store_si128(reinterpret_cast<__m128i*>(indexArray), srcIndex16Local); \
		alignas(16) WORD valueArray[8];                                           \
		for (int i = 0; i < 8; ++i)                                               \
			valueArray[i] = pPaletteDataLocal[indexArray[i]];                     \
		return _mm_load_si128(reinterpret_cast<const __m128i*>(valueArray));      \
	}((srcIndex16), (pPaletteData)))

#define Sse2_BlendU16(oldValue16, newValue16, writeMask16) \
	_mm_or_si128(_mm_and_si128((writeMask16), (newValue16)), _mm_andnot_si128((writeMask16), (oldValue16)))

#define Sse2_Expand8ToEpi16Signed(pSrc)                                                    \
	([](const byte* pSrcLocal) -> __m128i                                                  \
	{                                                                                      \
		const __m128i src8 = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(pSrcLocal)); \
		const __m128i sign8 = _mm_cmpgt_epi8(_mm_setzero_si128(), src8);                   \
		return _mm_unpacklo_epi8(src8, sign8);                                             \
	}(pSrc))

#define Sse2_CmpGtEpu16(lhs16, rhs16)                                      \
	([](const __m128i lhs16Local, const __m128i rhs16Local) -> __m128i     \
	{                                                                      \
		const __m128i bias16 = _mm_set1_epi16(static_cast<short>(-32768)); \
		const __m128i lhsBiased16 = _mm_xor_si128(lhs16Local, bias16);     \
		const __m128i rhsBiased16 = _mm_xor_si128(rhs16Local, bias16);     \
		return _mm_cmpgt_epi16(lhsBiased16, rhsBiased16);                  \
	}((lhs16), (rhs16)))

#if defined(YR_SIMD_COMPILE_AVX2) || defined(YR_SIMD_COMPILE_AVX512)
#define Avx2_Expand8ToEpi32(pSrc) \
	_mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(pSrc)))

#define Avx2_Load8WordAsEpi32(pSrc) \
	_mm256_cvtepu16_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(pSrc)))

#define Avx2_PackU32ToU16(value32)                                    \
	([](const __m256i value32Local) -> __m128i                        \
	{                                                                 \
		const __m128i lo = _mm256_castsi256_si128(value32Local);      \
		const __m128i hi = _mm256_extracti128_si256(value32Local, 1); \
		return _mm_packus_epi32(lo, hi);                              \
	}(value32))

#define Avx2_PackMask32ToI16(value32Mask)                                 \
	([](const __m256i value32MaskLocal) -> __m128i                        \
	{                                                                     \
		const __m128i lo = _mm256_castsi256_si128(value32MaskLocal);      \
		const __m128i hi = _mm256_extracti128_si256(value32MaskLocal, 1); \
		return _mm_packs_epi32(lo, hi);                                   \
	}(value32Mask))

#define Avx2_BlendU16(oldValue16, newValue16, writeMask16) \
	_mm_or_si128(_mm_and_si128((writeMask16), (newValue16)), _mm_andnot_si128((writeMask16), (oldValue16)))

#define Avx2_GatherWordTable(index32, pTable, maxIndex)                                                        \
	([](const __m256i index32Local, const WORD* pTableLocal, const int maxIndexLocal) -> __m256i               \
	{                                                                                                          \
		const __m256i maxIndex32 = _mm256_set1_epi32(maxIndexLocal);                                           \
		const __m256i maxIndexMask32 = _mm256_cmpeq_epi32(index32Local, maxIndex32);                           \
		const __m256i gatherIndex32 = _mm256_andnot_si256(maxIndexMask32, index32Local);                       \
		__m256i value32 = _mm256_i32gather_epi32(reinterpret_cast<const int*>(pTableLocal), gatherIndex32, 2); \
		value32 = _mm256_and_si256(value32, _mm256_set1_epi32(0xFFFF));                                        \
		const __m256i maxValue32 = _mm256_set1_epi32(static_cast<int>(pTableLocal[maxIndexLocal]));            \
		return _mm256_blendv_epi8(value32, maxValue32, maxIndexMask32);                                        \
	}((index32), (pTable), (maxIndex)))

#define Avx2_GatherPaletteWord(srcIndex32, pPaletteData) \
	Avx2_GatherWordTable((srcIndex32), (pPaletteData), 0xFF)

#define Avx2_PackU32ToU8(value32)                                \
	([](const __m256i value32Local) -> __m128i                   \
	{                                                            \
		const __m128i value16 = Avx2_PackU32ToU16(value32Local); \
		return _mm_packus_epi16(value16, _mm_setzero_si128());   \
	}(value32))

#define Avx2_BuildByteLut32(pTable, pLut32) \
{                                           \
	for (int i = 0; i < 256; ++i)           \
		(pLut32)[i] = (pTable)[i];          \
}

#define Avx2_StoreMask8(pDest, writeMask, value8)                      \
{                                                                      \
	alignas(16) BYTE valueArray[16];                                   \
	_mm_store_si128(reinterpret_cast<__m128i*>(valueArray), (value8)); \
	for (int lane = 0; lane < 8; ++lane)                               \
	{                                                                  \
		if (((writeMask) & (1 << lane)) != 0)                          \
			(pDest)[lane] = valueArray[lane];                          \
	}                                                                  \
}

#define Avx2_StoreMask8AndZ(pDest, pZBuffer, writeMask, value8, zValues32)    \
{                                                                             \
	alignas(16) BYTE valueArray[16];                                          \
	alignas(32) int zValueArray[8];                                           \
	_mm_store_si128(reinterpret_cast<__m128i*>(valueArray), (value8));        \
	_mm256_store_si256(reinterpret_cast<__m256i*>(zValueArray), (zValues32)); \
	for (int lane = 0; lane < 8; ++lane)                                      \
	{                                                                         \
		if (((writeMask) & (1 << lane)) != 0)                                 \
		{                                                                     \
			(pDest)[lane] = valueArray[lane];                                 \
			(pZBuffer)[lane] = static_cast<WORD>(zValueArray[lane]);          \
		}                                                                     \
	}                                                                         \
}
#endif
