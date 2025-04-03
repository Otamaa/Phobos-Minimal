#pragma once

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

namespace AttackBeaconFunctional
{
	void AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	void OnFire(TechnoExtData* pExt, AbstractClass* pTarget, int nWeapon);
};
