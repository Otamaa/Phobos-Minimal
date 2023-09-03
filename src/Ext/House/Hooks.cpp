#include "Body.h"

#include <unordered_map>

#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ares_TechnoExt.h>
#include <Misc/Ares/Hooks/Header.h>

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
			bool PowerChecked = false;
			bool HasPower = false;

			for(auto const pType : pBld->GetTypes()){

				if (!pType)
					continue;

				if (!PowerChecked)
				{
					HasPower = pBld->HasPower
						&& !pBld->IsUnderEMP()
						&& (pBld->align_154->Is_Operated || TechnoExt_ExtData::IsOperated(pBld));

					PowerChecked = true;
				}

				const auto pExt = BuildingTypeExt::ExtMap.TryFind(pType);

				if(HasPower) {
					if (pExt->PowerPlantEnhancer_Buildings.size() &&
						(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
					{
						++pHouseExt->PowerPlantEnhancerBuildings[pType];
					}

					if (pExt->SpeedBonus.Enabled)
						++pHouseExt->Building_BuildSpeedBonusCounter[pType];
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

DEFINE_HOOK(0x4F844B, HouseClass_Update, 0x6)
{
	GET(HouseClass* const, pThis, ESI);

	//HouseExt::ExtMap.Find(pThis)->UpdateAutoDeathObjects();

	return 0;
}

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

		if (!pType->Insignificant && !pType->DontScore) {
			auto& limboTechvec = HouseExt::ExtMap.Find(pThis->Owner)->LimboTechno;
			const auto it = std::find_if(limboTechvec.begin(), limboTechvec.end(), [pThis](TechnoClass* ptr) { return ptr == pThis; });

			if (it == limboTechvec.end())
				limboTechvec.push_back(pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (!pType->Insignificant && !pType->DontScore) {
		auto& limboTechvec = HouseExt::ExtMap.Find(pThis->Owner)->LimboTechno;

		const auto it = std::find_if(limboTechvec.begin(), limboTechvec.end(), [pThis](TechnoClass* ptr) { return ptr == pThis; });

		if (it != limboTechvec.end())
			limboTechvec.erase(it);
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

		auto& limboTechvecOld = pOldOwnerExt->LimboTechno;

		const auto it = std::find_if(limboTechvecOld.begin(), limboTechvecOld.end(), [pThis](TechnoClass* ptr) { return ptr == pThis; });

		if (it != limboTechvecOld.end())
			limboTechvecOld.erase(it);

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