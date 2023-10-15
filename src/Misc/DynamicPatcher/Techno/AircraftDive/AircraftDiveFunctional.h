#pragma once

class AbstractClass;
class TechnoExtData;
class TechnoTypeExtData;
struct AircraftDiveFunctional
{
	static void Init(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	static void AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt);
	static void OnFire(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt, AbstractClass* pTarget, int nWeaponIDx);
};
