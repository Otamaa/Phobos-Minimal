#include "FighterAreaGuardFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Ext/Building/Body.h>

void FighterAreaGuardFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->MyFighterData)
		return;

	pExt->MyFighterData->OnUpdate();
}
