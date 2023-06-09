#include "FighterAreaGuardFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Ext/Building/Body.h>

static constexpr std::array<CoordStruct, 6> areaGuardCoords
{
	{
		  {-300,-300,0}
		, { -300 ,0,0 }
		, { 0,0,0 }
		, { 300,0,0 }
		, {300,300,0 }
		, {0 , 300 ,0 }
	}
};

void FighterAreaGuardFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	//if (!pExt->MyFighterData)
	//	return;

	//pExt->MyFighterData->OnUpdate();
}
