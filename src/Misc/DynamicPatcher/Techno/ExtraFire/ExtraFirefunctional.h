#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class AbstractClass;
class TechnoClass;
struct ExtraFirefunctional
{
	static void GetWeapon(TechnoClass* pThis, AbstractClass* pTarget , int nWeaponIdx);
};
#endif