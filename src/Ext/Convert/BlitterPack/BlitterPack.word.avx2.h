#pragma once

#include "BlitterPack.h"

class NOVTABLE BlitterPack16AVX2 final
{
public:
	// SIMD policy:
	// - keep native AVX2 for clean expand/gather/pack/blend/store WORD paths;
	// - keep ZRemapXlat paths on SSE2 because AVX2 remap index build is scalar-heavy.
	BlitPlainXlat<WORD, Simd::Level::AVX2> BlitterPlainXlat;
	BlitTransXlat<WORD, Simd::Level::AVX2> BlitterTransXlat;
	BlitTransDarken<WORD, Simd::Level::AVX2> BlitterTransDarken;
	BlitTransZRemapXlat<WORD, Simd::Level::SSE2> BlitterTransZRemapXlat;
	BlitTransLucent75<WORD, Simd::Level::AVX2> BlitterTransLucent75;
	BlitTransLucent50<WORD, Simd::Level::AVX2> BlitterTransLucent50;
	BlitTransLucent25<WORD, Simd::Level::AVX2> BlitterTransLucent25;
	BlitPlainXlatZRead<WORD, Simd::Level::AVX2> BlitterPlainXlatZRead;
	BlitTransXlatZRead<WORD, Simd::Level::AVX2> BlitterTransXlatZRead;
	BlitTransDarkenZRead<WORD, Simd::Level::AVX2> BlitterTransDarkenZRead;
	BlitTransZRemapXlatZRead<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatZRead;
	BlitTransLucent75ZRead<WORD, Simd::Level::AVX2> BlitterTransLucent75ZRead;
	BlitTransLucent50ZRead<WORD, Simd::Level::AVX2> BlitterTransLucent50ZRead;
	BlitTransLucent25ZRead<WORD, Simd::Level::AVX2> BlitterTransLucent25ZRead;
	BlitTransLucent75ZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent75ZReadWarp;
	BlitTransLucent50ZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent50ZReadWarp;
	BlitTransLucent25ZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent25ZReadWarp;
	BlitPlainXlatZReadWrite<WORD, Simd::Level::AVX2> BlitterPlainXlatZReadWrite;
	BlitTransXlatZReadWrite<WORD, Simd::Level::AVX2> BlitterTransXlatZReadWrite;
	BlitTransDarkenZReadWrite<WORD, Simd::Level::AVX2> BlitterTransDarkenZReadWrite;
	BlitTransZRemapXlatZReadWrite<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatZReadWrite;
	BlitTransLucent75ZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent75ZReadWrite;
	BlitTransLucent50ZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent50ZReadWrite;
	BlitTransLucent25ZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent25ZReadWrite;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlpha;
	BlitTransXlatAlpha<WORD, Simd::Level::Scalar> BlitterTransXlatAlpha;
	BlitTransZRemapXlatAlpha<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlpha;
	BlitTransLucent75Alpha<WORD, Simd::Level::AVX2> BlitterTransLucent75Alpha;
	BlitTransLucent50Alpha<WORD, Simd::Level::AVX2> BlitterTransLucent50Alpha;
	BlitTransLucent25Alpha<WORD, Simd::Level::AVX2> BlitterTransLucent25Alpha;
	BlitTransXlatWriteAlpha<WORD, Simd::Level::AVX2> BlitterTransXlatWriteAlpha;
	BlitTransXlatMultWriteAlpha<WORD, Simd::Level::AVX2> BlitterTransXlatMultWriteAlpha;
	BlitTranslucentWriteAlpha<WORD, Simd::Level::AVX2> BlitterTranslucentWriteAlpha;
	BlitTranslucent50NonzeroAlpha<WORD, Simd::Level::AVX2> BlitterTranslucent50NonzeroAlpha;
	BlitTranslucent50ZeroAlpha<WORD, Simd::Level::AVX2> BlitterTranslucent50ZeroAlpha;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlphaZReadSeed;
	BlitTransXlatAlphaZRead<WORD, Simd::Level::AVX2> BlitterTransXlatAlphaZRead;
	BlitTransZRemapXlatAlphaZRead<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlphaZRead;
	BlitTransLucent75AlphaZRead<WORD, Simd::Level::AVX2> BlitterTransLucent75AlphaZRead;
	BlitTransLucent50AlphaZRead<WORD, Simd::Level::AVX2> BlitterTransLucent50AlphaZRead;
	BlitTransLucent25AlphaZRead<WORD, Simd::Level::AVX2> BlitterTransLucent25AlphaZRead;
	BlitTransLucent75AlphaZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent75AlphaZReadWarp;
	BlitTransLucent50AlphaZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent50AlphaZReadWarp;
	BlitTransLucent25AlphaZReadWarp<WORD, Simd::Level::AVX2> BlitterTransLucent25AlphaZReadWarp;
	BlitPlainXlatAlpha<WORD, Simd::Level::Scalar> BlitterPlainXlatAlphaZReadWriteSeed;
	BlitTransXlatAlphaZReadWrite<WORD, Simd::Level::AVX2> BlitterTransXlatAlphaZReadWrite;
	BlitTransZRemapXlatAlphaZReadWrite<WORD, Simd::Level::SSE2> BlitterTransZRemapXlatAlphaZReadWrite;
	BlitTransLucent75AlphaZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent75AlphaZReadWrite;
	BlitTransLucent50AlphaZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent50AlphaZReadWrite;
	BlitTransLucent25AlphaZReadWrite<WORD, Simd::Level::AVX2> BlitterTransLucent25AlphaZReadWrite;

	RLEBlitTransXlat<WORD, Simd::Level::AVX2> RleBlitterTransXlat;
	RLEBlitTransDarken<WORD, Simd::Level::AVX2> RleBlitterTransDarken;
	RLEBlitTransZRemapXlat<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlat;
	RLEBlitTransLucent75<WORD, Simd::Level::AVX2> RleBlitterTransLucent75;
	RLEBlitTransLucent50<WORD, Simd::Level::AVX2> RleBlitterTransLucent50;
	RLEBlitTransLucent25<WORD, Simd::Level::AVX2> RleBlitterTransLucent25;
	RLEBlitTransXlatZRead<WORD, Simd::Level::AVX2> RleBlitterTransXlatZRead;
	RLEBlitTransZRemapXlatZRead<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatZRead;
	RLEBlitTransDarkenZRead<WORD, Simd::Level::AVX2> RleBlitterTransDarkenZRead;
	RLEBlitTransLucent75ZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent75ZRead;
	RLEBlitTransLucent50ZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent50ZRead;
	RLEBlitTransLucent25ZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent25ZRead;
	RLEBlitTransLucent75ZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent75ZReadWarp;
	RLEBlitTransLucent50ZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent50ZReadWarp;
	RLEBlitTransLucent25ZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent25ZReadWarp;
	RLEBlitTransXlatZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransXlatZReadWrite;
	RLEBlitTransZRemapXlatZReadWrite<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatZReadWrite;
	RLEBlitTransDarkenZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransDarkenZReadWrite;
	RLEBlitTransLucent75ZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent75ZReadWrite;
	RLEBlitTransLucent50ZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent50ZReadWrite;
	RLEBlitTransLucent25ZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent25ZReadWrite;
	RLEBlitTransXlatAlpha<WORD, Simd::Level::Scalar> RleBlitterTransXlatAlpha;
	RLEBlitTransZRemapXlatAlpha<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlpha;
	RLEBlitTransLucent75Alpha<WORD, Simd::Level::AVX2> RleBlitterTransLucent75Alpha;
	RLEBlitTransLucent50Alpha<WORD, Simd::Level::AVX2> RleBlitterTransLucent50Alpha;
	RLEBlitTransLucent25Alpha<WORD, Simd::Level::AVX2> RleBlitterTransLucent25Alpha;
	RLEBlitTransXlatAlphaZRead<WORD, Simd::Level::AVX2> RleBlitterTransXlatAlphaZRead;
	RLEBlitTransZRemapXlatAlphaZRead<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlphaZRead;
	RLEBlitTransLucent75AlphaZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent75AlphaZRead;
	RLEBlitTransLucent50AlphaZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent50AlphaZRead;
	RLEBlitTransLucent25AlphaZRead<WORD, Simd::Level::AVX2> RleBlitterTransLucent25AlphaZRead;
	RLEBlitTransLucent75AlphaZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent75AlphaZReadWarp;
	RLEBlitTransLucent50AlphaZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent50AlphaZReadWarp;
	RLEBlitTransLucent25AlphaZReadWarp<WORD, Simd::Level::AVX2> RleBlitterTransLucent25AlphaZReadWarp;
	RLEBlitTransXlatAlphaZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransXlatAlphaZReadWrite;
	RLEBlitTransZRemapXlatAlphaZReadWrite<WORD, Simd::Level::SSE2> RleBlitterTransZRemapXlatAlphaZReadWrite;
	RLEBlitTransLucent75AlphaZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent75AlphaZReadWrite;
	RLEBlitTransLucent50AlphaZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent50AlphaZReadWrite;
	RLEBlitTransLucent25AlphaZReadWrite<WORD, Simd::Level::AVX2> RleBlitterTransLucent25AlphaZReadWrite;

	RLEBlitTransLucent75AlphaZReadFix<WORD, Simd::Level::AVX2> RleBlitterTransLucent75AlphaZReadFix;
	RLEBlitTransLucent50AlphaZReadFix<WORD, Simd::Level::AVX2> RleBlitterTransLucent50AlphaZReadFix;
	RLEBlitTransLucent25AlphaZReadFix<WORD, Simd::Level::AVX2> RleBlitterTransLucent25AlphaZReadFix;

public:
	BlitterPack16AVX2(WORD* paletteData, WORD* fullColorData, BYTE** currentZRemap, WORD halfTranslucencyMask, WORD quatTranslucencyMask, int shadeCount);
};
