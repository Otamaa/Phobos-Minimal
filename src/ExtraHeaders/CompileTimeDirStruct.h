#pragma once

#include <DirStruct.h>

struct CTimeDirStruct
{
	constexpr CTimeDirStruct() noexcept = default;
	constexpr CTimeDirStruct(int Raw) noexcept :
		Raw { (unsigned short)Raw }
	{ }

	constexpr CTimeDirStruct(const size_t bits, const DirType value) noexcept :
		Raw { 0 }
	{ 
		Raw = (unsigned short)(TranslateFixedPoint::CompileTime(bits, 16u, (size_t)value));
	}

public:
	unsigned short Raw;
private:
	unsigned short Padding {0};
};
