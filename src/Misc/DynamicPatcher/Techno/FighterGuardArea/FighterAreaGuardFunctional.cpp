#include "FighterAreaGuardFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Ext/Building/Body.h>

void FighterAreaGuardFunctional::AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt)
{
	if (!pExt->MyFighterData)
		return;

	pExt->MyFighterData->OnUpdate();
}
