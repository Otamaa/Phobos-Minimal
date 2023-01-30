#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct FighterAreaGuardData
{
	bool AreaGuard { false };
	int GuardRange { 5 };
	bool AutoFire { false };
	int MaxAmmo { 1 };

	void Read(INI_EX& parser, const char* pSection, TechnoTypeClass* pType);

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(AreaGuard)
			.Process(GuardRange)
			.Process(AutoFire)
			.Process(MaxAmmo)
			;
	}
};
#endif