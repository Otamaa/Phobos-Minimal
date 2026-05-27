#pragma once

#include "BlitterPack.h"

class NOVTABLE BlitterPack8Scalar final
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
	RLEBlitTransXlatZRead<BYTE, Simd::Level::Scalar> RleBlitterTransXlatZRead;
	RLEBlitTransRemapDestZRead<BYTE, Simd::Level::Scalar> RleBlitterTransRemapDestZRead;
	RLEBlitTransZRemapXlatZRead<BYTE, Simd::Level::Scalar> RleBlitterTransZRemapXlatZRead;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZRead75;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZRead50;
	RLEBlitTransRemapXlatZRead<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZRead25;
	RLEBlitTransXlatZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransXlatZReadWrite;
	RLEBlitTransRemapDestZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransRemapDestZReadWrite;
	RLEBlitTransZRemapXlatZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransZRemapXlatZReadWrite;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZReadWrite75;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZReadWrite50;
	RLEBlitTransRemapXlatZReadWrite<BYTE, Simd::Level::Scalar> RleBlitterTransRemapXlatZReadWrite25;

public:
	BlitterPack8Scalar(BYTE* pPaletteData, BYTE* pRemapData, BYTE** ppCurrentZRemap);
};
