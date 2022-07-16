#pragma once

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

namespace AttackBeaconFunctional
{
	void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	void OnFire(TechnoExt::ExtData* pExt, AbstractClass* pTarget, int nWeapon);
};