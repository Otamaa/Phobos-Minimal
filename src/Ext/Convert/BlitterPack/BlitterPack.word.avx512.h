#pragma once

#include "BlitterPack.h"

class NOVTABLE BlitterPack16AVX512 final
{
public:
	// SIMD policy:
	// - keep native AVX512 for clean expand/gather/pack/blend/store WORD paths;
	// - keep ZRemapXlat paths on SSE2 because AVX512 remap index build is scalar-heavy;
	// - keep lookup-heavy Alpha Xlat paths scalar.
	BlitPlainXlat<WORD, Simd::Level::AVX512> BlitterPlainXlat;
	BlitTransXlat<WORD, Simd::Level::AVX512> BlitterTransXlat;
	BlitTransDarken<WORD, Simd::Level::AVX512> BlitterTransDarken;
	BlitTransZRemapXlat<WORD, Simd::Level::SSE2> BlitterTransZRemapXlat;
	BlitTransLucent75<WORD, Simd::Level::AVX512> BlitterTransLucent75;
	BlitTransLucent50<WORD, Simd::Level::AVX512> BlitterTransLucent50;
	BlitTransLucent25<WORD, Simd::Level::AVX512> BlitterTransLucent25;
	BlitPlainXlatZRead<WORD, Simd::Level::AVX512> BlitterPlainXlatZRead;
	BlitTransXlatZRead<WORD, Simd::Level::AVX512> BlitterTransXlatZRead;
	BlitTransDarkenZRead<WORD, Simd::Level::AVX512> BlitterTransDarkenZRead;
	BlitTransZRemapXlatZRead<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatZRead;
	BlitTransLucent75ZRead<WORD, Simd::Level::AVX512> BlitterTransLucent75ZRead;
	BlitTransLucent50ZRead<WORD, Simd::Level::AVX512> BlitterTransLucent50ZRead;
	BlitTransLucent25ZRead<WORD, Simd::Level::AVX512> BlitterTransLucent25ZRead;
	BlitTransLucent75ZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent75ZReadWarp;
	BlitTransLucent50ZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent50ZReadWarp;
	BlitTransLucent25ZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent25ZReadWarp;
	BlitPlainXlatZReadWrite<WORD, Simd::Level::AVX512> BlitterPlainXlatZReadWrite;
	BlitTransXlatZReadWrite<WORD, Simd::Level::AVX512> BlitterTransXlatZReadWrite;
	BlitTransDarkenZReadWrite<WORD, Simd::Level::AVX512> BlitterTransDarkenZReadWrite;
	BlitTransZRemapXlatZReadWrite<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatZReadWrite;
	BlitTransLucent75ZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent75ZReadWrite;
	BlitTransLucent50ZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent50ZReadWrite;
	BlitTransLucent25ZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent25ZReadWrite;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlpha;
	BlitTransXlatAlpha<WORD, Simd::Level::Scalar> BlitterTransXlatAlpha;
	BlitTransZRemapXlatAlpha<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlpha;
	BlitTransLucent75Alpha<WORD, Simd::Level::AVX512> BlitterTransLucent75Alpha;
	BlitTransLucent50Alpha<WORD, Simd::Level::AVX512> BlitterTransLucent50Alpha;
	BlitTransLucent25Alpha<WORD, Simd::Level::AVX512> BlitterTransLucent25Alpha;
	BlitTransXlatWriteAlpha<WORD, Simd::Level::AVX512> BlitterTransXlatWriteAlpha;
	BlitTransXlatMultWriteAlpha<WORD, Simd::Level::AVX512> BlitterTransXlatMultWriteAlpha;
	BlitTranslucentWriteAlpha<WORD, Simd::Level::AVX512> BlitterTranslucentWriteAlpha;
	BlitTranslucent50NonzeroAlpha<WORD, Simd::Level::AVX512> BlitterTranslucent50NonzeroAlpha;
	BlitTranslucent50ZeroAlpha<WORD, Simd::Level::AVX512> BlitterTranslucent50ZeroAlpha;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlphaZReadSeed;
	BlitTransXlatAlphaZRead<WORD, Simd::Level::AVX512> BlitterTransXlatAlphaZRead;
	BlitTransZRemapXlatAlphaZRead<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlphaZRead;
	BlitTransLucent75AlphaZRead<WORD, Simd::Level::AVX512> BlitterTransLucent75AlphaZRead;
	BlitTransLucent50AlphaZRead<WORD, Simd::Level::AVX512> BlitterTransLucent50AlphaZRead;
	BlitTransLucent25AlphaZRead<WORD, Simd::Level::AVX512> BlitterTransLucent25AlphaZRead;
	BlitTransLucent75AlphaZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent75AlphaZReadWarp;
	BlitTransLucent50AlphaZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent50AlphaZReadWarp;
	BlitTransLucent25AlphaZReadWarp<WORD, Simd::Level::AVX512> BlitterTransLucent25AlphaZReadWarp;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlphaZReadWriteSeed;
	BlitTransXlatAlphaZReadWrite<WORD, Simd::Level::AVX512> BlitterTransXlatAlphaZReadWrite;
	BlitTransZRemapXlatAlphaZReadWrite<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlphaZReadWrite;
	BlitTransLucent75AlphaZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent75AlphaZReadWrite;
	BlitTransLucent50AlphaZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent50AlphaZReadWrite;
	BlitTransLucent25AlphaZReadWrite<WORD, Simd::Level::AVX512> BlitterTransLucent25AlphaZReadWrite;

	RLEBlitTransXlat<WORD, Simd::Level::AVX512> RleBlitterTransXlat;
	RLEBlitTransDarken<WORD, Simd::Level::AVX512> RleBlitterTransDarken;
	RLEBlitTransZRemapXlat<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlat;
	RLEBlitTransLucent75<WORD, Simd::Level::AVX512> RleBlitterTransLucent75;
	RLEBlitTransLucent50<WORD, Simd::Level::AVX512> RleBlitterTransLucent50;
	RLEBlitTransLucent25<WORD, Simd::Level::AVX512> RleBlitterTransLucent25;
	RLEBlitTransXlatZRead<WORD, Simd::Level::AVX512> RleBlitterTransXlatZRead;
	RLEBlitTransZRemapXlatZRead<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatZRead;
	RLEBlitTransDarkenZRead<WORD, Simd::Level::AVX512> RleBlitterTransDarkenZRead;
	RLEBlitTransLucent75ZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent75ZRead;
	RLEBlitTransLucent50ZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent50ZRead;
	RLEBlitTransLucent25ZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent25ZRead;
	RLEBlitTransLucent75ZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent75ZReadWarp;
	RLEBlitTransLucent50ZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent50ZReadWarp;
	RLEBlitTransLucent25ZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent25ZReadWarp;
	RLEBlitTransXlatZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransXlatZReadWrite;
	RLEBlitTransZRemapXlatZReadWrite<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatZReadWrite;
	RLEBlitTransDarkenZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransDarkenZReadWrite;
	RLEBlitTransLucent75ZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent75ZReadWrite;
	RLEBlitTransLucent50ZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent50ZReadWrite;
	RLEBlitTransLucent25ZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent25ZReadWrite;
	RLEBlitTransXlatAlpha<WORD, Simd::Level::Scalar> RleBlitterTransXlatAlpha;
	RLEBlitTransZRemapXlatAlpha<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlpha;
	RLEBlitTransLucent75Alpha<WORD, Simd::Level::AVX512> RleBlitterTransLucent75Alpha;
	RLEBlitTransLucent50Alpha<WORD, Simd::Level::AVX512> RleBlitterTransLucent50Alpha;
	RLEBlitTransLucent25Alpha<WORD, Simd::Level::AVX512> RleBlitterTransLucent25Alpha;
	RLEBlitTransXlatAlphaZRead<WORD, Simd::Level::AVX512> RleBlitterTransXlatAlphaZRead;
	RLEBlitTransZRemapXlatAlphaZRead<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlphaZRead;
	RLEBlitTransLucent75AlphaZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent75AlphaZRead;
	RLEBlitTransLucent50AlphaZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent50AlphaZRead;
	RLEBlitTransLucent25AlphaZRead<WORD, Simd::Level::AVX512> RleBlitterTransLucent25AlphaZRead;
	RLEBlitTransLucent75AlphaZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent75AlphaZReadWarp;
	RLEBlitTransLucent50AlphaZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent50AlphaZReadWarp;
	RLEBlitTransLucent25AlphaZReadWarp<WORD, Simd::Level::AVX512> RleBlitterTransLucent25AlphaZReadWarp;
	RLEBlitTransXlatAlphaZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransXlatAlphaZReadWrite;
	RLEBlitTransZRemapXlatAlphaZReadWrite<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlphaZReadWrite;
	RLEBlitTransLucent75AlphaZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent75AlphaZReadWrite;
	RLEBlitTransLucent50AlphaZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent50AlphaZReadWrite;
	RLEBlitTransLucent25AlphaZReadWrite<WORD, Simd::Level::AVX512> RleBlitterTransLucent25AlphaZReadWrite;

	RLEBlitTransLucent75AlphaZReadFix<WORD, Simd::Level::AVX512> RleBlitterTransLucent75AlphaZReadFix;
	RLEBlitTransLucent50AlphaZReadFix<WORD, Simd::Level::AVX512> RleBlitterTransLucent50AlphaZReadFix;
	RLEBlitTransLucent25AlphaZReadFix<WORD, Simd::Level::AVX512> RleBlitterTransLucent25AlphaZReadFix;

public:
	BlitterPack16AVX512(WORD* paletteData, WORD* fullColorData, BYTE** currentZRemap, WORD halfTranslucencyMask, WORD quatTranslucencyMask, int shadeCount);
};
