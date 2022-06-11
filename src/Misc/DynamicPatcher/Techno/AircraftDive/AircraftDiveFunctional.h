#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct AircraftDiveFunctional
{
private:
	NO_CONSTRUCT_CLASS(AircraftDiveFunctional)
public:
	static void Init(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void OnFire(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx);
};
#endif