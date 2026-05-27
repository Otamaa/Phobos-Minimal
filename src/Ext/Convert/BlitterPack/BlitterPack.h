#pragma once

#include "../Blitters/index.h"
#include <ConvertClass.h>
#include <Phobos.h>

enum BlitterIndex8
{
	BlitterIndex8_PlainXlat = 0,
	BlitterIndex8_TransXlat,
	BlitterIndex8_TransRemapDest,
	BlitterIndex8_TransZRemapXlat,
	BlitterIndex8_TransRemapXlat75,
	BlitterIndex8_TransRemapXlat50,
	BlitterIndex8_TransRemapXlat25
};

enum RleBlitterIndex8
{
	RleBlitterIndex8_TransXlat = 0,
	RleBlitterIndex8_TransRemapDest,
	RleBlitterIndex8_TransZRemapXlat,
	RleBlitterIndex8_TransRemapXlat75,
	RleBlitterIndex8_TransRemapXlat50,
	RleBlitterIndex8_TransRemapXlat25,
	RleBlitterIndex8_TransXlatZRead,
	RleBlitterIndex8_TransZRemapXlatZRead,
	RleBlitterIndex8_TransRemapDestZRead,
	RleBlitterIndex8_TransRemapXlatZRead75,
	RleBlitterIndex8_TransRemapXlatZRead50,
	RleBlitterIndex8_TransRemapXlatZRead25,

	RleBlitterIndex8_TransXlatZReadWrite = 15,
	RleBlitterIndex8_TransZRemapXlatZReadWrite,
	RleBlitterIndex8_TransRemapDestZReadWrite,
	RleBlitterIndex8_TransRemapXlatZReadWrite75,
	RleBlitterIndex8_TransRemapXlatZReadWrite50,
	RleBlitterIndex8_TransRemapXlatZReadWrite25
};

enum BlitterIndex16
{
	BlitterIndex16_PlainXlat = 0,
	BlitterIndex16_TransXlat,
	BlitterIndex16_TransDarken,
	BlitterIndex16_TransZRemapXlat,
	BlitterIndex16_TransLucent75,
	BlitterIndex16_TransLucent50,
	BlitterIndex16_TransLucent25,
	BlitterIndex16_PlainXlatZRead,
	BlitterIndex16_TransXlatZRead,
	BlitterIndex16_TransDarkenZRead,
	BlitterIndex16_TransZRemapXlatZRead,
	BlitterIndex16_TransLucent75ZRead,
	BlitterIndex16_TransLucent50ZRead,
	BlitterIndex16_TransLucent25ZRead,
	BlitterIndex16_TransLucent75ZReadWarp,
	BlitterIndex16_TransLucent50ZReadWarp,
	BlitterIndex16_TransLucent25ZReadWarp,
	BlitterIndex16_PlainXlatZReadWrite,
	BlitterIndex16_TransXlatZReadWrite,
	BlitterIndex16_TransDarkenZReadWrite,
	BlitterIndex16_TransZRemapXlatZReadWrite,
	BlitterIndex16_TransLucent75ZReadWrite,
	BlitterIndex16_TransLucent50ZReadWrite,
	BlitterIndex16_TransLucent25ZReadWrite,
	BlitterIndex16_PlainXlatAlpha,
	BlitterIndex16_TransXlatAlpha,
	BlitterIndex16_TransZRemapXlatAlpha,
	BlitterIndex16_TransLucent75Alpha,
	BlitterIndex16_TransLucent50Alpha,
	BlitterIndex16_TransLucent25Alpha,
	BlitterIndex16_TransXlatWriteAlpha,
	BlitterIndex16_TransXlatMultWriteAlpha,
	BlitterIndex16_TranslucentWriteAlpha,
	BlitterIndex16_Translucent50NonzeroAlpha,
	BlitterIndex16_Translucent50ZeroAlpha,
	BlitterIndex16_PlainXlatAlphaZReadSeed,
	BlitterIndex16_TransXlatAlphaZRead,
	BlitterIndex16_TransZRemapXlatAlphaZRead,
	BlitterIndex16_TransLucent75AlphaZRead,
	BlitterIndex16_TransLucent50AlphaZRead,
	BlitterIndex16_TransLucent25AlphaZRead,
	BlitterIndex16_TransLucent75AlphaZReadWarp,
	BlitterIndex16_TransLucent50AlphaZReadWarp,
	BlitterIndex16_TransLucent25AlphaZReadWarp,
	BlitterIndex16_PlainXlatAlphaZReadWriteSeed,
	BlitterIndex16_TransXlatAlphaZReadWrite,
	BlitterIndex16_TransZRemapXlatAlphaZReadWrite,
	BlitterIndex16_TransLucent75AlphaZReadWrite,
	BlitterIndex16_TransLucent50AlphaZReadWrite,
	BlitterIndex16_TransLucent25AlphaZReadWrite
};

enum RleBlitterIndex16
{
	RleBlitterIndex16_TransXlat = 0,
	RleBlitterIndex16_TransDarken,
	RleBlitterIndex16_TransZRemapXlat,
	RleBlitterIndex16_TransLucent75,
	RleBlitterIndex16_TransLucent50,
	RleBlitterIndex16_TransLucent25,
	RleBlitterIndex16_TransXlatZRead,
	RleBlitterIndex16_TransZRemapXlatZRead,
	RleBlitterIndex16_TransDarkenZRead,
	RleBlitterIndex16_TransLucent75ZRead,
	RleBlitterIndex16_TransLucent50ZRead,
	RleBlitterIndex16_TransLucent25ZRead,
	RleBlitterIndex16_TransLucent75ZReadWarp,
	RleBlitterIndex16_TransLucent50ZReadWarp,
	RleBlitterIndex16_TransLucent25ZReadWarp,
	RleBlitterIndex16_TransXlatZReadWrite,
	RleBlitterIndex16_TransZRemapXlatZReadWrite,
	RleBlitterIndex16_TransDarkenZReadWrite,
	RleBlitterIndex16_TransLucent75ZReadWrite,
	RleBlitterIndex16_TransLucent50ZReadWrite,
	RleBlitterIndex16_TransLucent25ZReadWrite,
	RleBlitterIndex16_TransXlatAlpha,
	RleBlitterIndex16_TransZRemapXlatAlpha,
	RleBlitterIndex16_TransLucent75Alpha,
	RleBlitterIndex16_TransLucent50Alpha,
	RleBlitterIndex16_TransLucent25Alpha,
	RleBlitterIndex16_TransXlatAlphaZRead,
	RleBlitterIndex16_TransZRemapXlatAlphaZRead,
	RleBlitterIndex16_TransLucent75AlphaZRead,
	RleBlitterIndex16_TransLucent50AlphaZRead,
	RleBlitterIndex16_TransLucent25AlphaZRead,
	RleBlitterIndex16_TransLucent75AlphaZReadWarp,
	RleBlitterIndex16_TransLucent50AlphaZReadWarp,
	RleBlitterIndex16_TransLucent25AlphaZReadWarp,
	RleBlitterIndex16_TransXlatAlphaZReadWrite,
	RleBlitterIndex16_TransZRemapXlatAlphaZReadWrite,
	RleBlitterIndex16_TransLucent75AlphaZReadWrite,
	RleBlitterIndex16_TransLucent50AlphaZReadWrite,
	RleBlitterIndex16_TransLucent25AlphaZReadWrite
};

template <typename TPack>
__forceinline void MapBlitterPack8(TPack* pPack, ConvertClass* pConvertClass)
{
	pConvertClass->Blitters[BlitterIndex8_PlainXlat] = &pPack->BlitterPlainXlat;
	pConvertClass->Blitters[BlitterIndex8_TransXlat] = &pPack->BlitterTransXlat;
	pConvertClass->Blitters[BlitterIndex8_TransRemapDest] = &pPack->BlitterTransRemapDest;
	pConvertClass->Blitters[BlitterIndex8_TransZRemapXlat] = &pPack->BlitterTransZRemapXlat;
	pConvertClass->Blitters[BlitterIndex8_TransRemapXlat75] = &pPack->BlitterTransRemapXlat75;
	pConvertClass->Blitters[BlitterIndex8_TransRemapXlat50] = &pPack->BlitterTransRemapXlat50;
	pConvertClass->Blitters[BlitterIndex8_TransRemapXlat25] = &pPack->BlitterTransRemapXlat25;

	pConvertClass->RLEBlitters[RleBlitterIndex8_TransXlat] = &pPack->RleBlitterTransXlat;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapDest] = &pPack->RleBlitterTransRemapDest;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransZRemapXlat] = &pPack->RleBlitterTransZRemapXlat;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlat75] = &pPack->RleBlitterTransRemapXlat75;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlat50] = &pPack->RleBlitterTransRemapXlat50;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlat25] = &pPack->RleBlitterTransRemapXlat25;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransXlatZRead] = &pPack->RleBlitterTransXlatZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransZRemapXlatZRead] = &pPack->RleBlitterTransZRemapXlatZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapDestZRead] = &pPack->RleBlitterTransRemapDestZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZRead75] = &pPack->RleBlitterTransRemapXlatZRead75;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZRead50] = &pPack->RleBlitterTransRemapXlatZRead50;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZRead25] = &pPack->RleBlitterTransRemapXlatZRead25;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransXlatZReadWrite] = &pPack->RleBlitterTransXlatZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransZRemapXlatZReadWrite] = &pPack->RleBlitterTransZRemapXlatZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapDestZReadWrite] = &pPack->RleBlitterTransRemapDestZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZReadWrite75] = &pPack->RleBlitterTransRemapXlatZReadWrite75;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZReadWrite50] = &pPack->RleBlitterTransRemapXlatZReadWrite50;
	pConvertClass->RLEBlitters[RleBlitterIndex8_TransRemapXlatZReadWrite25] = &pPack->RleBlitterTransRemapXlatZReadWrite25;
}

template <typename TPack>
__forceinline void MapBlitterPack16(TPack* pPack, ConvertClass* pConvertClass)
{
	pConvertClass->Blitters[BlitterIndex16_PlainXlat] = &pPack->BlitterPlainXlat;
	pConvertClass->Blitters[BlitterIndex16_TransXlat] = &pPack->BlitterTransXlat;
	pConvertClass->Blitters[BlitterIndex16_TransDarken] = &pPack->BlitterTransDarken;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlat] = &pPack->BlitterTransZRemapXlat;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75] = &pPack->BlitterTransLucent75;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50] = &pPack->BlitterTransLucent50;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25] = &pPack->BlitterTransLucent25;
	pConvertClass->Blitters[BlitterIndex16_PlainXlatZRead] = &pPack->BlitterPlainXlatZRead;
	pConvertClass->Blitters[BlitterIndex16_TransXlatZRead] = &pPack->BlitterTransXlatZRead;
	pConvertClass->Blitters[BlitterIndex16_TransDarkenZRead] = &pPack->BlitterTransDarkenZRead;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlatZRead] = &pPack->BlitterTransZRemapXlatZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75ZRead] = &pPack->BlitterTransLucent75ZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50ZRead] = &pPack->BlitterTransLucent50ZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25ZRead] = &pPack->BlitterTransLucent25ZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75ZReadWarp] = &pPack->BlitterTransLucent75ZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50ZReadWarp] = &pPack->BlitterTransLucent50ZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25ZReadWarp] = &pPack->BlitterTransLucent25ZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_PlainXlatZReadWrite] = &pPack->BlitterPlainXlatZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransXlatZReadWrite] = &pPack->BlitterTransXlatZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransDarkenZReadWrite] = &pPack->BlitterTransDarkenZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlatZReadWrite] = &pPack->BlitterTransZRemapXlatZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75ZReadWrite] = &pPack->BlitterTransLucent75ZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50ZReadWrite] = &pPack->BlitterTransLucent50ZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25ZReadWrite] = &pPack->BlitterTransLucent25ZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_PlainXlatAlpha] = &pPack->BlitterPlainXlatAlpha;
	pConvertClass->Blitters[BlitterIndex16_TransXlatAlpha] = &pPack->BlitterTransXlatAlpha;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlatAlpha] = &pPack->BlitterTransZRemapXlatAlpha;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75Alpha] = &pPack->BlitterTransLucent75Alpha;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50Alpha] = &pPack->BlitterTransLucent50Alpha;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25Alpha] = &pPack->BlitterTransLucent25Alpha;
	pConvertClass->Blitters[BlitterIndex16_TransXlatWriteAlpha] = &pPack->BlitterTransXlatWriteAlpha;
	pConvertClass->Blitters[BlitterIndex16_TransXlatMultWriteAlpha] = &pPack->BlitterTransXlatMultWriteAlpha;
	pConvertClass->Blitters[BlitterIndex16_TranslucentWriteAlpha] = &pPack->BlitterTranslucentWriteAlpha;
	pConvertClass->Blitters[BlitterIndex16_Translucent50NonzeroAlpha] = &pPack->BlitterTranslucent50NonzeroAlpha;
	pConvertClass->Blitters[BlitterIndex16_Translucent50ZeroAlpha] = &pPack->BlitterTranslucent50ZeroAlpha;
	pConvertClass->Blitters[BlitterIndex16_PlainXlatAlphaZReadSeed] = &pPack->BlitterPlainXlatAlphaZReadSeed;
	pConvertClass->Blitters[BlitterIndex16_TransXlatAlphaZRead] = &pPack->BlitterTransXlatAlphaZRead;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlatAlphaZRead] = &pPack->BlitterTransZRemapXlatAlphaZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75AlphaZRead] = &pPack->BlitterTransLucent75AlphaZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50AlphaZRead] = &pPack->BlitterTransLucent50AlphaZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25AlphaZRead] = &pPack->BlitterTransLucent25AlphaZRead;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75AlphaZReadWarp] = &pPack->BlitterTransLucent75AlphaZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50AlphaZReadWarp] = &pPack->BlitterTransLucent50AlphaZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25AlphaZReadWarp] = &pPack->BlitterTransLucent25AlphaZReadWarp;
	pConvertClass->Blitters[BlitterIndex16_PlainXlatAlphaZReadWriteSeed] = &pPack->BlitterPlainXlatAlphaZReadWriteSeed;
	pConvertClass->Blitters[BlitterIndex16_TransXlatAlphaZReadWrite] = &pPack->BlitterTransXlatAlphaZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransZRemapXlatAlphaZReadWrite] = &pPack->BlitterTransZRemapXlatAlphaZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent75AlphaZReadWrite] = &pPack->BlitterTransLucent75AlphaZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent50AlphaZReadWrite] = &pPack->BlitterTransLucent50AlphaZReadWrite;
	pConvertClass->Blitters[BlitterIndex16_TransLucent25AlphaZReadWrite] = &pPack->BlitterTransLucent25AlphaZReadWrite;

	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlat] = &pPack->RleBlitterTransXlat;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransDarken] = &pPack->RleBlitterTransDarken;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlat] = &pPack->RleBlitterTransZRemapXlat;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75] = &pPack->RleBlitterTransLucent75;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50] = &pPack->RleBlitterTransLucent50;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25] = &pPack->RleBlitterTransLucent25;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlatZRead] = &pPack->RleBlitterTransXlatZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlatZRead] = &pPack->RleBlitterTransZRemapXlatZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransDarkenZRead] = &pPack->RleBlitterTransDarkenZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75ZRead] = &pPack->RleBlitterTransLucent75ZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50ZRead] = &pPack->RleBlitterTransLucent50ZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25ZRead] = &pPack->RleBlitterTransLucent25ZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75ZReadWarp] = &pPack->RleBlitterTransLucent75ZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50ZReadWarp] = &pPack->RleBlitterTransLucent50ZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25ZReadWarp] = &pPack->RleBlitterTransLucent25ZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlatZReadWrite] = &pPack->RleBlitterTransXlatZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlatZReadWrite] = &pPack->RleBlitterTransZRemapXlatZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransDarkenZReadWrite] = &pPack->RleBlitterTransDarkenZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75ZReadWrite] = &pPack->RleBlitterTransLucent75ZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50ZReadWrite] = &pPack->RleBlitterTransLucent50ZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25ZReadWrite] = &pPack->RleBlitterTransLucent25ZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlatAlpha] = &pPack->RleBlitterTransXlatAlpha;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlatAlpha] = &pPack->RleBlitterTransZRemapXlatAlpha;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75Alpha] = &pPack->RleBlitterTransLucent75Alpha;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50Alpha] = &pPack->RleBlitterTransLucent50Alpha;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25Alpha] = &pPack->RleBlitterTransLucent25Alpha;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlatAlphaZRead] = &pPack->RleBlitterTransXlatAlphaZRead;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlatAlphaZRead] = &pPack->RleBlitterTransZRemapXlatAlphaZRead;
	if (Phobos::Config::FixTransparencyBlitters)
	{
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75AlphaZRead] = &pPack->RleBlitterTransLucent75AlphaZReadFix;
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50AlphaZRead] = &pPack->RleBlitterTransLucent50AlphaZReadFix;
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25AlphaZRead] = &pPack->RleBlitterTransLucent25AlphaZReadFix;
	}
	else
	{
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75AlphaZRead] = &pPack->RleBlitterTransLucent75AlphaZRead;
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50AlphaZRead] = &pPack->RleBlitterTransLucent50AlphaZRead;
		pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25AlphaZRead] = &pPack->RleBlitterTransLucent25AlphaZRead;
	}
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75AlphaZReadWarp] = &pPack->RleBlitterTransLucent75AlphaZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50AlphaZReadWarp] = &pPack->RleBlitterTransLucent50AlphaZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25AlphaZReadWarp] = &pPack->RleBlitterTransLucent25AlphaZReadWarp;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransXlatAlphaZReadWrite] = &pPack->RleBlitterTransXlatAlphaZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransZRemapXlatAlphaZReadWrite] = &pPack->RleBlitterTransZRemapXlatAlphaZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent75AlphaZReadWrite] = &pPack->RleBlitterTransLucent75AlphaZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent50AlphaZReadWrite] = &pPack->RleBlitterTransLucent50AlphaZReadWrite;
	pConvertClass->RLEBlitters[RleBlitterIndex16_TransLucent25AlphaZReadWrite] = &pPack->RleBlitterTransLucent25AlphaZReadWrite;
}
