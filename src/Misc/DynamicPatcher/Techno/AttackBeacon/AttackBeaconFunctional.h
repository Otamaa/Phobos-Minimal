#pragma once

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct AttackBeaconFunctional
{
	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void OnFire(TechnoExt::ExtData* pExt, AbstractClass* pTarget, int nWeapon);
};