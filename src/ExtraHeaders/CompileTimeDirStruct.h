#pragma once

#include <DirStruct.h>

struct CTimeDirStruct
{
	COMPILETIMEEVAL CTimeDirStruct() noexcept = default;
	COMPILETIMEEVAL CTimeDirStruct(int Raw) noexcept :
		Raw { (unsigned short)Raw }
	{ }

	COMPILETIMEEVAL CTimeDirStruct(const size_t bits, const DirType value) noexcept :
		Raw { 0 }
	{ 
		Raw = (unsigned short)(TranslateFixedPoint::CompileTime(bits, 16u, (size_t)value));
	}

	DirStruct ToDirStruct() const
	{
		DirStruct nDuummy;
		nDuummy.Raw = Raw;
		return nDuummy;
	}

public:
	unsigned short Raw;
private:
	unsigned short Padding {0};
};
