#include "Body.h"

#include <unordered_map>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	if (!pHouseExt)
		return 0x0;

	pHouseExt->BuildingCounter.clear();
	pHouseExt->Building_BuildSpeedBonusCounter.clear();
	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (pBld && !pBld->InLimbo && pBld->IsOnMap)
		{
			if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type))
			{
				if (pExt->PowerPlantEnhancer_Buildings.size() &&
					(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
				{
					++pHouseExt->BuildingCounter[pExt];
				}

				if (pExt->SpeedBonus.isset() && !pExt->SpeedBonusTo.empty())
					++pHouseExt->Building_BuildSpeedBonusCounter[pExt];
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

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	if (!pBuilding || !pBuilding->Owner)
		return 0;

	if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
	{
		auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

		if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
		{
			BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
			amount = 0.0f;
		}
	}

	return 0;
}
