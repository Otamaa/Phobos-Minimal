#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

struct FighterAreaGuardFunctional
{
private:
	NO_CONSTRUCT_CLASS(FighterAreaGuardFunctional)
public:

	static void AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
};
#endif