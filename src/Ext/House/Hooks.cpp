#include "Body.h"

#include <unordered_map>

#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Misc/Ares/Hooks/Header.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Cast.h>

DEFINE_HOOK_AGAIN(0x4FFA99, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)
DEFINE_HOOK(0x4FF9C9, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)
{
	GET(FakeBuildingClass*, pBuilding, ESI);
	GET(FakeHouseClass*, pThis, EDI);
	GET(bool, isNaval, ECX);

	if (pBuilding->_GetTypeExtData()->ExcludeFromMultipleFactoryBonus) {
		pThis->_GetExtData()->UpdateNonMFBFactoryCounts(pBuilding->Type->Factory, R->Origin()
			== 0x4FF9C9, isNaval);
	}

	return 0;
}

DEFINE_HOOK(0x500910, HouseClass_GetFactoryCount, 0x5)
{
	enum { SkipGameCode = 0x50095D };

	GET(FakeHouseClass*, pThis, ECX);
	GET_STACK(AbstractType, rtti, 0x4);
	GET_STACK(bool, isNaval, 0x8);

	R->EAX(pThis->_GetExtData()
		->GetFactoryCountWithoutNonMFB(rtti, isNaval));

	return SkipGameCode;
}

//remove the SW processing from the original place
// DEFINE_HOOK(0x4FD77C, HouseClass_ExpertAI_Superweapons, 0x5) {
// 	return RulesExtData::Instance()->AISuperWeaponDelay.isset() ?
// 		0x4FD7A0 : 0;
// }
//
//update it separately
DEFINE_HOOK(0x4F9038, HouseClass_AI_Superweapons, 0x5) {

	GET(FakeHouseClass*, pThis, ESI);

	if (!RulesExtData::Instance()->AISuperWeaponDelay.isset() || pThis->IsControlledByHuman() || pThis->Type->MultiplayPassive)
		return 0;

	const int delay = RulesExtData::Instance()->AISuperWeaponDelay.Get();

	if (delay > 0) {
		auto const pExt = pThis->_GetExtData();

		if (pExt->AISuperWeaponDelayTimer.HasTimeLeft())
			return 0;

		pExt->AISuperWeaponDelayTimer.Start(delay);
	}

	if (!SessionClass::IsCampaign() || pThis->IQLevel2 >= RulesClass::Instance->SuperWeapons)
		pThis->AI_TryFireSW();

	return 0;
}

// DEFINE_HOOK(0x4FD77C, HouseClass_ExpertAI_Superweapons, 0x5)
// {
// 	enum { SkipSWProcess = 0x4FD7A0 , RetTryFireSW = 0x4FD799};
// 	GET(HouseClass*, pThis, ESI);
//
// 	bool TryFireSW = !SessionClass::IsCampaign() || pThis->IQLevel2 >= RulesClass::Instance->SuperWeapons;
//
// 	if(RulesExtData::Instance()->AISuperWeaponDelay.isset()) {
// 		const int delay = RulesExtData::Instance()->AISuperWeaponDelay;
//
// 		if (delay > 0) {
// 			auto const pExt = HouseExtContainer::Instance.Find(pThis);
//
// 			if (!pExt->AISuperWeaponDelayTimer.HasTimeLeft())
// 				TryFireSW = false;
// 			else
// 				pExt->AISuperWeaponDelayTimer.Start(delay);
// 		}
// 	}
//
// 	return !TryFireSW  ? SkipSWProcess : RetTryFireSW;
// }

// Gets the superweapons used by AI for Chronoshift script actions.
void GetAIChronoshiftSupers(HouseClass* pThis, SuperClass*& pSuperCSphere, SuperClass*& pSuperCWarp)
{
	int idxCS = RulesExtData::Instance()->AIChronoSphereSW;
	int idxCW = RulesExtData::Instance()->AIChronoWarpSW;

	if (idxCS >= 0)
	{
		pSuperCSphere = pThis->Supers[idxCS];

		if (idxCW < 0)
		{
			auto const pSWTypeExt = SWTypeExtContainer::Instance.Find(pSuperCSphere->Type);
			int idxWarp = SuperWeaponTypeClass::FindIndexById(pSWTypeExt->SW_PostDependent);

			if (idxWarp >= 0)
				pSuperCWarp = pThis->Supers.Items[idxWarp];
		}
	}

	if (idxCW >= 0)
		pSuperCWarp = pThis->Supers[idxCW];

	if (pSuperCSphere && pSuperCWarp)
		return;

	for (auto const pSuper : pThis->Supers)
	{
		if (pSuper->Type->Type == SuperWeaponType::ChronoSphere)
			pSuperCSphere = pSuper;

		if (pSuper->Type->Type == SuperWeaponType::ChronoWarp)
			pSuperCWarp = pSuper;
	}
}

DEFINE_HOOK(0x6EFEFB, TMission_ChronoShiftToBuilding_SuperWeapons, 0x6)
{
	enum { SkipGameCode = 0x6EFF22 };

	GET(HouseClass*, pHouse, EBP);

	SuperClass* pSuperCSphere = nullptr;
	SuperClass* pSuperCWarp = nullptr;
	GetAIChronoshiftSupers(pHouse, pSuperCSphere, pSuperCWarp);
	R->ESI(pSuperCSphere);
	R->EBX(pSuperCWarp);

	return SkipGameCode;
}

DEFINE_HOOK(0x6F01B0, TMission_ChronoShiftToTarget_SuperWeapons, 0x6)
{
	enum { SkipGameCode = 0x6F01D9 };

	GET(HouseClass*, pHouse, EDI);
	REF_STACK(SuperClass*, pSuperCWarp, STACK_OFFSET(0x30, -0x1C));

	SuperClass* pSuperCSphere = nullptr;
	GetAIChronoshiftSupers(pHouse, pSuperCSphere, pSuperCWarp);
	R->EBX(pSuperCSphere);

	return SkipGameCode;
}

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
	GET(FakeHouseClass*, pThis, ECX);

	const auto pHouseExt = pThis->_GetExtData();

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
				|| BuildingExtContainer::Instance.Find(pBld)->AboutToChronoshift
				|| pBld->IsBeingWarpedOut())
				continue;

			for(auto const pType : pBld->GetTypes()){

				if (!pType)
					continue;

				if (!PowerChecked)
				{
					HasPower = pBld->HasPower && !pBld->IsUnderEMP()
						&& (TechnoExtContainer::Instance.Find(pBld)->Is_Operated || TechnoExt_ExtData::IsOperated(pBld))
						;

					PowerChecked = true;
				}

				const auto pExt = BuildingTypeExtContainer::Instance.Find(pType);

				if(HasPower) {
					if (!pExt->PowerPlantEnhancer_Buildings.empty() &&
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

	pThis->PowerOutput += BuildingTypeExtData::GetEnhancedPower(pBld, pThis);

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
//		HouseExtContainer::Instance.Find(pThis->Owner)->RemoveFromLimboTracking(pType);
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
		HouseExtData::LimboTechno.push_back_unique(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	HouseExtData::LimboTechno.remove(pThis);
	return 0;
}

#include <Misc/Ares/Hooks/Header.h>

HouseClass* OldOwner = nullptr;

DEFINE_HOOK(0x70173B , TechnoClass_ChangeOwnership_AfterHouseWasSet, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	auto pNewOwner= pThis->Owner;

	if(OldOwner){
		if (auto pMe = flag_cast_to<FootClass*>(pThis))
		{
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pMe->GetTechnoType());
			bool I_am_human = OldOwner->IsControlledByHuman();
			bool You_are_human = pNewOwner->IsControlledByHuman();
			TechnoTypeClass* pConvertTo = (I_am_human && !You_are_human) ? pTypeExt->Convert_HumanToComputer.Get() :
				(!I_am_human && You_are_human) ? pTypeExt->Convert_ComputerToHuman.Get() : nullptr;

			if (!pConvertTo)
			{
				auto& map = pTypeExt->Convert_ToHouseOrCountry;
				for(auto it = map.begin(); it != map.end(); ++it) {
					if(it->first == pNewOwner->Type ||
					 	it->first == SideClass::Array->Items[pNewOwner->Type->SideIndex]){
						 pConvertTo = it->second;
						 break;
					}
				}
			}

			if (pConvertTo)
				TechnoExt_ExtData::ConvertToType(pMe, pConvertTo,true , false);
		}

		OldOwner = nullptr;
	}

	return 0x0;
}


DEFINE_HOOK(0x7015EB, TechnoClass_ChangeOwnership_UpdateTracking, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();
	auto pOldOwnerExt = HouseExtContainer::Instance.Find(pThis->Owner);
	auto pNewOwnerExt = HouseExtContainer::Instance.Find(pNewOwner);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pNewOwner->Type->MultiplayPassive &&  pThis->WhatAmI() != BuildingClass::AbsID && TechnoTypeExtContainer::Instance.Find(pType)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
		pNewOwner->RecheckTechTree = true;
	}

	//this kind a dangerous
	//const KillMethod nMethod = pTypeExt->Death_Method.Get();
	//if (pTypeExt->Death_IfChangeOwnership && nMethod != KillMethod::None) {
	//	TechnoExtData::KillSelf(pThis, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	//}

	OldOwner = pThis->Owner;
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
//			HouseExtContainer::Instance.Find(pTechno->Owner)->AddToLimboTracking(pType);
//		}
//	}
//;
//
//	return 0;
//}
#pragma endregion

DEFINE_HOOK(0x4FDCE0 , HouseClass_AI_Fire_Sale_OnLastLegs, 0x6){
	GET(HouseClass*, pThis, ECX);
	GET_STACK(UrgencyType, urg , 0x4);

	bool ret = false;
	if(urg == UrgencyType::Critical){
		auto const pRules = RulesExtData::Instance();
		auto const pExt = HouseExtContainer::Instance.Find(pThis);

		if (pRules->AISellAllOnLastLegs)
		{
			if (pRules->AISellAllDelay <= 0 ||
				pExt->AISellAllDelayTimer.Completed())
			{
				pThis->Fire_Sale();
			}
			else if (!pExt->AISellAllDelayTimer.HasStarted())
			{
				pExt->AISellAllDelayTimer.Start(pRules->AISellAllDelay);
			}
		}

		if (pRules->AIAllInOnLastLegs)
			pThis->All_To_Hunt();

		ret = true;
	}

	R->AL(ret);
	return 0x4FDD00;
}

// Sell all and all in.
// DEFINE_HOOK(0x4FD8F7, HouseClass_UpdateAI_OnLastLegs, 0x6)
// {
// 	enum { ret = 0x4FD907 };
//
// 	GET(HouseClass*, pThis, EBX);
//
// 	auto const pRules = RulesExtData::Instance();
// 	auto const pExt = HouseExtContainer::Instance.Find(pThis);
//
// 	if (pRules->AISellAllOnLastLegs)
// 	{
// 		if (pRules->AISellAllDelay <= 0 ||
// 			pExt->AISellAllDelayTimer.Completed())
// 		{
// 			pThis->Fire_Sale();
// 		}
// 		else if (!pExt->AISellAllDelayTimer.HasStarted())
// 		{
// 			pExt->AISellAllDelayTimer.Start(pRules->AISellAllDelay);
// 		}
// 	}
//
// 	if (pRules->AIAllInOnLastLegs)
// 		pThis->All_To_Hunt();
//
// 	return ret;
// }

// I must not regroup my forces.
DEFINE_HOOK(0x739920, UnitClass_TryToDeploy_DisableRegroupAtNewConYard, 0x6)
{
	enum { SkipRegroup = 0x73992B, DoNotSkipRegroup = 0 };

	if (!RulesExtData::Instance()->RegroupWhenMCVDeploy)
		return SkipRegroup;
	else
		return DoNotSkipRegroup;
}