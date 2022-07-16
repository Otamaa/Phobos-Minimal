#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class TechnoClass;
namespace PassengersFunctional
{
	void AI(TechnoClass* pThis);
	void CanFire(TechnoClass* pThis, bool& cease);
};
#endif