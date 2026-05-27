#pragma once

#include "BlitterPack.h"
#include <Utilities/Debug.h>

class NOVTABLE BlitterPack8SSE2 final
{
public:
	BlitPlainXlat<BYTE, Simd::Level::Scalar> BlitterPlainXlat;
	BlitTransXlat<BYTE, Simd::Level::Scalar> BlitterTransXlat;
	BlitTransRemapDest<BYTE, Simd::Level::Scalar> BlitterTransRemapDest;
	BlitTransZRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransZRemapXlat;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat75;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat50;
	BlitTransRemapXlat<BYTE, Simd::Level::Scalar> BlitterTransRemapXlat25;

	RLEBlitTransXlat<BYTE, Simd::Level::Scalar> RleBlitterTransXlat;
	RLEBlitTransRemapDest<BYTE, Simd::Level::Scalar> RleBlitterTransRemapDest;
	RLEBlitTransZRemapXlat<BYTE, Simd::Level::Scalar> RleBlitterTransZRemapXlat;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlat75;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlat50;
	RLEBlitTransRemapXlat<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlat25;
	RLEBlitTransXlatZRead<BYTE, Simd::Level::SSE2> RleBlitterTransXlatZRead;
	RLEBlitTransRemapDestZRead<BYTE, Simd::Level::SSE2> RleBlitterTransRemapDestZRead;
	RLEBlitTransZRemapXlatZRead<BYTE, Simd::Level::SSE2> RleBlitterTransZRemapXlatZRead;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZRead75;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZRead50;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZRead25;
	RLEBlitTransXlatZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransXlatZReadWrite;
	RLEBlitTransRemapDestZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransRemapDestZReadWrite;
	RLEBlitTransZRemapXlatZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransZRemapXlatZReadWrite;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZReadWrite75;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZReadWrite50;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::SSE2> RleBlitterTransRemapXlatZReadWrite25;

public:
	BlitterPack8SSE2(BYTE* paletteData, BYTE* remapData, BYTE** currentZRemap);
};
