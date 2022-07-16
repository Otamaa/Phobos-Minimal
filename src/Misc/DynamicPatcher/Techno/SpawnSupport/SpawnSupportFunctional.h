#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class TechnoClass;
class AbstractClass;
namespace SpawnSupportFunctional
{
	void Construct(TechnoClass* pThis);
	void AI(TechnoClass* pThis);
	void OnFire(TechnoClass* pThis);
	void FireSupportWeaponToSpawn(TechnoClass* pThis,bool InUpdateFunc = false, bool useROF = false);
};
#endif