#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class AbstractClass;
class TechnoClass;
struct ExtraFirefunctional
{
private:
	NO_CONSTRUCT_CLASS(ExtraFirefunctional)
public:

	static void GetWeapon(TechnoClass* pThis, AbstractClass* pTarget , int nWeaponIdx);
};
#endif