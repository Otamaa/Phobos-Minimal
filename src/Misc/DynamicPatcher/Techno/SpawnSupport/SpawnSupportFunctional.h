#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class TechnoClass;
class AbstractClass;
struct SpawnSupportFunctional
{
private:
	NO_CONSTRUCT_CLASS(SpawnSupportFunctional)
public:
	static void Construct(TechnoClass* pThis);
	static void AI(TechnoClass* pThis);
	static void OnFire(TechnoClass* pThis);
private:
	static void FireSupportWeaponToSpawn(TechnoClass* pThis,bool InUpdateFunc = false, bool useROF = false);
};
#endif