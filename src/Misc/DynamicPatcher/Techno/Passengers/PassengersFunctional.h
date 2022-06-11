#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Base/Always.h>

class TechnoClass;
struct PassengersFunctional
{
private:
	NO_CONSTRUCT_CLASS(PassengersFunctional)
public:
	static void AI(TechnoClass* pThis);
	static void CanFire(TechnoClass* pThis, bool& cease);
};
#endif