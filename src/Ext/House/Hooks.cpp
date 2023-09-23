#include "Body.h"

#include <unordered_map>

#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ares_TechnoExt.h>
#include <Misc/Ares/Hooks/Header.h>

DEFINE_HOOK(0x65EB8D, HouseClass_SendSpyPlanes_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65EBE5, SkipGameCodeNoSuccess = 0x65EC12 };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x65E997, HouseClass_SendAirstrike_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65E9EE, SkipGameCodeNoSuccess = 0x65EA8B };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->PowerPlantEnhancerBuildings.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (const auto& pBld : pThis->Buildings)
	{
		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
		{
			bool PowerChecked = false;
			bool HasPower = false;

			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
				continue;

			if (pBld->TemporalTargetingMe
				|| BuildingExt::ExtMap.Find(pBld)->AboutToChronoshift
				|| pBld->IsBeingWarpedOut())
				continue;

			for(auto const pType : pBld->GetTypes()){

				if (!pType)
					continue;

				if (!PowerChecked)
				{
					HasPower = pBld->HasPower && !pBld->IsUnderEMP()
						&& (pBld->align_154->Is_Operated || TechnoExt_ExtData::IsOperated(pBld))
						;

					PowerChecked = true;
				}

				const auto pExt = BuildingTypeExt::ExtMap.TryFind(pType);

				if(HasPower) {
					if (pExt->PowerPlantEnhancer_Buildings.size() &&
						(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
					{
						++pHouseExt->PowerPlantEnhancerBuildings[pType];
					}
				}
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

//DEFINE_HOOK(0x4F844B, HouseClass_Update, 0x6)
//{
//	GET(HouseClass* const, pThis, ESI);
//
//
//
//	return 0;
//}

#pragma region LimboTracking

//namespace LimboTrackingTemp
//{
//	bool IsBeingDeleted = false;
//}
//
//void __fastcall TechnoClass_UnInit_Wrapper(TechnoClass* pThis , DWORD)
//{
//	auto const pType = pThis->GetTechnoType();
//
//	if (pThis->InLimbo && !pType->Insignificant && !pType->DontScore) {
//		HouseExt::ExtMap.Find(pThis->Owner)->RemoveFromLimboTracking(pType);
//	}
//
//	LimboTrackingTemp::IsBeingDeleted = true;
//	pThis->ObjectClass::UnInit();
//	LimboTrackingTemp::IsBeingDeleted = false;
//}
//
//DEFINE_JUMP(CALL, 0x4DE60B, GET_OFFSET(TechnoClass_UnInit_Wrapper));   // FootClass
//DEFINE_JUMP(VTABLE, 0x7E3FB4, GET_OFFSET(TechnoClass_UnInit_Wrapper)); // BuildingClass

DEFINE_HOOK(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if(pThis->IsAlive){
		auto const pType = pThis->GetTechnoType();
		auto pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

		if (!pType->Insignificant && !pType->DontScore) {
			pHouseExt->LimboTechno.push_back_unique(pThis);
		}

		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building
			&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
		{
			pHouseExt->OwnedTransportReloaders.push_back_unique(pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!pType->Insignificant && !pType->DontScore) {
		HouseExt::ExtMap.Find(pThis->Owner)->LimboTechno.remove(pThis);
	}

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building
		&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
	{
		HouseExt::ExtMap.Find(pThis->Owner)->OwnedTransportReloaders.remove(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x7015C9, TechnoClass_Captured_UpdateTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();
	auto pOldOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
	auto pNewOwnerExt = HouseExt::ExtMap.Find(pNewOwner);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	/*auto pExt = TechnoExt::ExtMap.Find(pThis);*/

	//this kind a dangerous
	const KillMethod nMethod = pTypeExt->Death_Method.Get();
	if (pTypeExt->Death_IfChangeOwnership && nMethod != KillMethod::None) {
		TechnoExt::KillSelf(pThis, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	}

	if (pThis->InLimbo && !pType->Insignificant && !pType->DontScore) {
		pOldOwnerExt->LimboTechno.remove(pThis);

		if (pThis->IsAlive)
			pNewOwnerExt->LimboTechno.push_back(pThis);
	}

	const auto Item = pOldOwnerExt->AutoDeathObjects.get_key_iterator(pThis);
	if (Item != pOldOwnerExt->AutoDeathObjects.end())
	{
		pOldOwnerExt->AutoDeathObjects.erase(pThis);

		if(pThis->IsAlive)
			pNewOwnerExt->AutoDeathObjects.insert(pThis, Item->second);
	}

	if (pThis->InLimbo && pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building
		&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
	{
		pOldOwnerExt->OwnedTransportReloaders.remove(pThis);
		if (pThis->IsAlive)
			pNewOwnerExt->OwnedTransportReloaders.push_back(pThis);
	}

	//const auto iter = std::find_if(pOldOwnerExt->OwnedTechno.begin(), pOldOwnerExt->OwnedTechno.end(), [pThis](TechnoClass* pItem) { return pItem == pThis; });
	//if (iter != pOldOwnerExt->OwnedTechno.end())
	//	pOldOwnerExt->OwnedTechno.erase(iter);

	//pNewOwnerExt->OwnedTechno.push_back(pThis);

	return 0;
}

//DEFINE_HOOK(0x687B18, ScenarioClass_ReadINI_StartTracking, 0x7)
//{
//	for (auto const pTechno : *TechnoClass::Array)
//	{
//		auto const pType = pTechno->GetTechnoType();
//
//		if (pTechno->Owner &&
//			!pType->Insignificant &&
//			!pType->DontScore &&
//			pTechno->WhatAmI() != AbstractType::Building &&
//			pTechno->InLimbo
//			)
//		{
//			HouseExt::ExtMap.Find(pTechno->Owner)->AddToLimboTracking(pType);
//		}
//	}
//;
//
//	return 0;
//}
#pragma endregion