#pragma once
#include <Base/Always.h>

class TechnoClass;
class AbstractClass;
struct SpawnSupportFunctional
{
	static void Construct(TechnoClass* pThis);
	static void AI(TechnoClass* pThis);
	static void OnFire(TechnoClass* pThis, AbstractClass* pTarget);
	static void FireSupportWeaponToSpawn(TechnoClass* pThis, AbstractClass* pTarget , bool useROF = false);
};
