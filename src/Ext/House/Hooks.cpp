#include "Body.h"

#include <unordered_map>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	pHouseExt->PowerPlantEnhancerBuildings.clear();
	pHouseExt->Building_BuildSpeedBonusCounter.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (const auto& pBld : pThis->Buildings)
	{
		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if(pBld->HasPower) {
				if (pExt->PowerPlantEnhancer_Buildings.size() &&
					(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
				{
					++pHouseExt->PowerPlantEnhancerBuildings[pBld->Type];
				}
			}

			if (pExt->SpeedBonus.Enabled)
				++pHouseExt->Building_BuildSpeedBonusCounter[pBld->Type];

			if (BuildingExt::ExtMap.Find(pBld)->LimboID != -1)
			{
				pHouseExt->AvaibleDocks += pBld->Type->NumberOfDocks;
			}
		}
	}


	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x4F8440, HouseClass_Update, 0x5)
{
	GET(HouseClass* const, pThis, ECX);

	if(auto const pHouseExt = HouseExt::ExtMap.TryFind(pThis))
		pHouseExt->UpdateAutoDeathObjects();

	return 0;
}