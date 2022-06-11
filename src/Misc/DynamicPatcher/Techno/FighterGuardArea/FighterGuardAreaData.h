#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct FighterAreaGuardData
{
	bool AreaGuard;
	int GuardRange;
	bool AutoFire;
	int MaxAmmo;

	FighterAreaGuardData() :
		AreaGuard { false }
		, GuardRange { 5 }
		, AutoFire { false }
		, MaxAmmo { 1 }
	{ }

	void Read(INI_EX& parser, const char* pSection ,TechnoTypeClass* pType)
	{
		Valueable<bool> bFAreaGuard { AreaGuard };
		bFAreaGuard.Read(parser,pSection, "Fighter.AreaGuard");
		AreaGuard = bFAreaGuard.Get();

		if (AreaGuard)
		{
			Valueable<int> nRange { GuardRange };
			nRange.Read(parser, pSection, "Fighter.GuardRange");
			GuardRange = nRange.Get();

			bFAreaGuard = AutoFire;
			bFAreaGuard.Read(parser, pSection, "Fighter.AutoFire");
			AutoFire = bFAreaGuard.Get();

			if (pType->Ammo > MaxAmmo)
				MaxAmmo = pType->Ammo;
		}
	}

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