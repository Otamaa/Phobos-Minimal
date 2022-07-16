#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

namespace AircraftDiveFunctional
{
	void Init(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	void OnFire(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx);
};
#endif