#pragma once

#include <ArrayClasses.h>
#include <Helpers/CompileTime.h>

class AlphaLightingRemapClass
{
public:

	COMPILETIMEEVAL static reference<DynamicVectorClass<AlphaLightingRemapClass*>, 0x88A080> const Array {};
	COMPILETIMEEVAL static reference<AlphaLightingRemapClass , 0x87E8A8> const Global {};
	// Notice:
	// When a ConvertClass is constructed by the game, it will generate [IntensityCount] color
	// tables from dark to bright. Each of them just changes the intensity of the source palette.
	//
	// If we have a point, whose value in ABuffer is A
	AlphaLightingRemapClass* FindOrAllocate(int intensityCount) {
		JMP_THIS(0x420140);
	}

	static void __stdcall Release(AlphaLightingRemapClass* pItem) {
		JMP_STD(0x420270); }

	AlphaLightingRemapClass(int steps) noexcept
	{ JMP_THIS(0x4202F0); }

public:
	union
	{
		// Intensity - AlphaValue
		WORD Table[256][256];
		WORD DataArray[65536];
	};
	int Steps;
	int RefCount;
};

static_assert(sizeof(AlphaLightingRemapClass) == 0x20008, "Invalid size.");