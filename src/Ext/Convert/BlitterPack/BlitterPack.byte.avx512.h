#pragma once

#include "BlitterPack.h"
#include <Utilities/Debug.h>

class NOVTABLE BlitterPack8AVX512 final
{
public:
	// SIMD policy:
	// - keep non-RLE BYTE paths scalar because per-call LUT/gather setup is not worth it there;
	// - keep RLE BYTE paths on native AVX512 where continuous runs can amortize SIMD setup.
	BlitPlainXlat<BYTE, Simd::Level::Scalar> BlitterPlainXlat;
	BlitTransXlat<BYTE, Simd::Level::Scalar> BlitterTransXlat;
	BlitTransRemapDest<BYTE, Simd::Level::Scalar> BlitterTransRemapDest;
	BlitTransZRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransZRemapXlat;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat75;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat50;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat25;

	RLEBlitTransXlat<BYTE, Simd::Level::AVX512> RleBlitterTransXlat;
	RLEBlitTransRemapDest<BYTE, Simd::Level::AVX512> RleBlitterTransRemapDest;
	RLEBlitTransZRemapXlat<BYTE, Simd::Level::AVX512> RleBlitterTransZRemapXlat;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlat75;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlat50;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlat25;
	RLEBlitTransXlatZRead<BYTE, Simd::Level::AVX512> RleBlitterTransXlatZRead;
	RLEBlitTransRemapDestZRead<BYTE, Simd::Level::AVX512> RleBlitterTransRemapDestZRead;
	RLEBlitTransZRemapXlatZRead<BYTE, Simd::Level::AVX512> RleBlitterTransZRemapXlatZRead;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZRead75;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZRead50;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZRead25;
	RLEBlitTransXlatZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransXlatZReadWrite;
	RLEBlitTransRemapDestZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransRemapDestZReadWrite;
	RLEBlitTransZRemapXlatZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransZRemapXlatZReadWrite;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZReadWrite75;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZReadWrite50;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::AVX512> RleBlitterTransRemapXlatZReadWrite25;

public:
	BlitterPack8AVX512(BYTE* paletteData, BYTE* remapData, BYTE** currentZRemap);
};
