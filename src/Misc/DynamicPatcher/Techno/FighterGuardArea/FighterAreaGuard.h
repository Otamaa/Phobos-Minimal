#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <GeneralStructures.h>
#include <CoordStruct.h>

#include <array>

struct FighterAreaGuard
{
	bool isAreaProtecting;
	CoordStruct areaProtectTo;
	int currentAreaProtectedIndex;
	bool isAreaGuardReloading;
	int areaGuardTargetCheckRof;

	FighterAreaGuard() :
		isAreaProtecting { false }
		, areaProtectTo { 0,0,0 }
		, currentAreaProtectedIndex { 0 }
		, isAreaGuardReloading { false }
		, areaGuardTargetCheckRof { 20 }
	{ }

	~FighterAreaGuard() = default;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(isAreaProtecting)
			.Process(areaProtectTo)
			.Process(currentAreaProtectedIndex)
			.Process(isAreaGuardReloading)
			.Process(areaGuardTargetCheckRof)
			;
	}

};
#endif