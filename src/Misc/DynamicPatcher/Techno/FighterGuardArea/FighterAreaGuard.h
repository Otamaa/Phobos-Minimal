#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <GeneralStructures.h>
#include <CoordStruct.h>

#include <array>

struct FighterAreaGuard
{
	bool isAreaProtecting { false };
	CoordStruct areaProtectTo { 0,0,0 };
	int currentAreaProtectedIndex { 0 };
	bool isAreaGuardReloading { false };
	int areaGuardTargetCheckRof { 20 };

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