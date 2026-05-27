
#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Cast.h>

#include <DiskLaserClass.h>

#include <Ext/Infantry/Body.h>
#include <Ext/DiskLaser/Body.h>

#pragma region Otamaa

ASMJIT_PATCH(0x6FD15E, TechnoClass_RearmDelay_RofMult, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, nROF, 0x14);

	auto const Building = cast_to<BuildingClass*, false>(pThis);

	if (pThis->CanOccupyFire()) {
		const auto occupant = pThis->GetOccupantCount();

		if (occupant > 0) {
			nROF /= occupant;
		}

		auto OccupyRofMult = RulesClass::Instance->OccupyROFMultiplier;

		if (Building) {
			OccupyRofMult = BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyROFMult.Get(OccupyRofMult);
		}

		if (OccupyRofMult > 0.0)
			nROF = int(float(nROF) / OccupyRofMult);

	}

	if (pThis->BunkerLinkedItem && !Building) {
		auto BunkerMult = RulesClass::Instance->BunkerROFMultiplier;
		if (auto const pBunkerIsBuilding = cast_to<BuildingClass* , false>(pThis->BunkerLinkedItem)) {
			BunkerMult = BuildingTypeExtContainer::Instance.Find(pBunkerIsBuilding->Type)->BuildingBunkerROFMult.Get(BunkerMult);
		}

		if(BunkerMult != 0.0)
			nROF = int(float(nROF) / BunkerMult);
	}

	R->EAX(nROF);
	return 0x6FD1F3;
}

#pragma region BunkerSounds
ASMJIT_PATCH(0x45933D, BuildingClass_BunkerWallUpSound, 0x5)
{
	GET(FakeBuildingClass* const, pThis, ESI);
	const auto nSound = pThis->_GetTypeExtData()->BunkerWallsUpSound.Get(RulesClass::Instance->BunkerWallsUpSound);
	VocClass::SafeImmedietelyPlayAt(nSound, &pThis->Location);
	return 0x459374;
}

ASMJIT_PATCH(0x4595D9, BuildingClass_4595C0_BunkerDownSound, 0x5)
{
	GET(FakeBuildingClass* const, pThis, EDI);
	const auto nSound = pThis->_GetTypeExtData()->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
	VocClass::SafeImmedietelyPlayAt(nSound, &pThis->Location);
	return 0x459612;
}

ASMJIT_PATCH(0x459494, BuildingClass_459470_BunkerDownSound, 0x5)
{
	GET(FakeBuildingClass* const, pThis, ESI);
	const auto nSound = pThis->_GetTypeExtData()->BunkerWallsDownSound.Get(RulesClass::Instance->BunkerWallsDownSound);
	VocClass::SafeImmedietelyPlayAt(nSound, &pThis->Location);
	return 0x4594CD;
}
#pragma endregion

ASMJIT_PATCH(0x505F6C, HouseClass_GenerateAIBuildList_AIBuildInstead, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	if (!pHouse->IsControlledByHuman() && !pHouse->IsNeutral()) {
		for (auto& nNodes : pHouse->Base.BaseNodes) {
			auto nIdx = nNodes.BuildingTypeIndex;
			if (nIdx >= 0) {

				const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items[nIdx]);

				if (!pBldTypeExt->AIBuildInsteadPerDiff.empty() && pBldTypeExt->AIBuildInsteadPerDiff[pHouse->GetCorrectAIDifficultyIndex()] != -1)
					nIdx = pBldTypeExt->AIBuildInsteadPerDiff[pHouse->GetCorrectAIDifficultyIndex()];

				nNodes.BuildingTypeIndex = nIdx;
			}
		}
	}

	return 0;
}

#pragma endregion