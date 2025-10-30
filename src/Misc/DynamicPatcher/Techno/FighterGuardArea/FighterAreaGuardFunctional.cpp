#include "FighterAreaGuardFunctional.h"

#ifdef _aaaaaaa 
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Ext/Building/Body.h>

void FighterAreaGuardFunctional::AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt)
{
	if (!pExt->MyFighterData)
		return;

	pExt->MyFighterData->OnUpdate();
}
#endif