#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class TechnoClass;
struct PassengersFunctional
{
	static void AI(TechnoClass* pThis);
	static void CanFire(TechnoClass* pThis, bool& cease);
};
#endif