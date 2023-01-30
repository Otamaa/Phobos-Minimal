#include "FighterGuardAreaData.h"
#ifdef COMPILE_PORTED_DP_FEATURES
void FighterAreaGuardData::Read(INI_EX& parser, const char* pSection, TechnoTypeClass* pType)
{
	Valueable<bool> bFAreaGuard { AreaGuard };
	bFAreaGuard.Read(parser, pSection, "Fighter.AreaGuard");
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
#endif