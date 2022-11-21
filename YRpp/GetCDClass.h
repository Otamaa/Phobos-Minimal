#pragma once

#include <Helpers/CompileTime.h>

class GetCDClass
{
public:
	static constexpr reference<GetCDClass, 0xA8E8E8u> const Instance{};

	ArrayWrapper<int, 26u> Drives;
	int Count;
	int unknown_6C;
};
