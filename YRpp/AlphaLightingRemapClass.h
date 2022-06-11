#pragma once
#include <YRPPCore.h>

class AlphaLightingRemapClass
{
public:
	AlphaLightingRemapClass(int steps)
	{ JMP_THIS(0x4202F0); }

	WORD data[0x10000];
	int Steps;
	int RefCount;
};

static_assert(sizeof(AlphaLightingRemapClass) == 0x20008, "Invalid size.");