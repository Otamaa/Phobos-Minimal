#include "Body.h"

#include <unordered_map>

#include <Ext/Aircraft/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Event/Body.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

ASMJIT_PATCH(0x4FD95F, HouseClass_CheckFireSale_LimboID, 0x6)
{
	GET(BuildingClass*, pBld, EAX);
	return BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0 ? 0x4FD983 : 0x0;
}

ASMJIT_PATCH(0x4FD203, HouseClass_RecalcCenter_Optimize, 0x6)
{
	GET(BuildingClass*, pBld, ESI);
	LEA_STACK(CoordStruct*, pBuffer_2, 0x38);
	LEA_STACK(CoordStruct*, pBuffer, 0x2C);

	const auto coord = pBld->GetCoords();
	*pBuffer = coord;
	*pBuffer_2 = coord;
	R->EBP(R->EBP<int>() + coord.X);
	R->EBX(R->EBX<int>() + coord.Y);
	R->EAX(R->Stack<int>(0x18));
	return 0x4FD228;
}


ASMJIT_PATCH(0x4FA5B8, HouseClass_BeginProduction_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBP);

	return pObject->GetTechnoType() == pTypeCompare || GET_TECHNOTYPE(pObject) == pTypeCompare ?
		0x4FA5C4 : 0x4FA5C8;
}

ASMJIT_PATCH(0x4FAB4D, HouseClass_AbandonProduction_GetObjectType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	R->EAX(GET_TECHNOTYPE(pObject));
	return R->Origin() + 0x8;
}

//HouseClass_Calc_Cost_Mult_Disable
DEFINE_JUMP(LJMP, 0x50BF60, 0x50C04A)// Disable CalcCost mult

ASMJIT_PATCH(0x4FF9C9, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)
{
	GET(FakeBuildingClass*, pBuilding, ESI);
	GET(FakeHouseClass*, pThis, EDI);
	GET(bool, isNaval, ECX);

	if (pBuilding->_GetTypeExtData()->ExcludeFromMultipleFactoryBonus) {
		pThis->_GetExtData()->UpdateNonMFBFactoryCounts(pBuilding->Type->Factory, R->Origin()
			== 0x4FF9C9, isNaval);
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x4FFA99, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)

ASMJIT_PATCH(0x500910, HouseClass_GetFactoryCount, 0x5)
{
	enum { SkipGameCode = 0x50095D };

	GET(FakeHouseClass*, pThis, ECX);
	GET_STACK(AbstractType, rtti, 0x4);
	GET_STACK(bool, isNaval, 0x8);

	R->EAX(pThis->_GetExtData()
		->GetFactoryCountWithoutNonMFB(rtti, isNaval));

	return SkipGameCode;
}

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

ASMJIT_PATCH(0x6EFEFB, TMission_ChronoShiftToBuilding_SuperWeapons, 0x6)
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

ASMJIT_PATCH(0x6F01B0, TMission_ChronoShiftToTarget_SuperWeapons, 0x6)
{
	enum { SkipGameCode = 0x6F01D9 };

	GET(HouseClass*, pHouse, EDI);
	REF_STACK(SuperClass*, pSuperCWarp, STACK_OFFSET(0x30, -0x1C));

	SuperClass* pSuperCSphere = nullptr;
	GetAIChronoshiftSupers(pHouse, pSuperCSphere, pSuperCWarp);
	R->EBX(pSuperCSphere);

	return SkipGameCode;
}

ASMJIT_PATCH(0x65EB8D, HouseClass_SendSpyPlanes_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65EBE5, SkipGameCodeNoSuccess = 0x65EC12 };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExtData::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

ASMJIT_PATCH(0x65E997, HouseClass_SendAirstrike_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65E9EE, SkipGameCodeNoSuccess = 0x65EA8B };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExtData::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

// ASMJIT_PATCH(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
// {
// 	GET(FakeHouseClass*, pThis, ECX);

// 	const auto pHouseExt = pThis->_GetExtData();
// 	return 0;
// }


#pragma region LimboTracking


ASMJIT_PATCH(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if(pThis->IsAlive){
		HouseExtContainer::Instance.LimboTechno.push_back_unique(pThis);
	}

	return 0;
}

ASMJIT_PATCH(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	HouseExtContainer::Instance.LimboTechno.remove(pThis);
	return 0;
}

HouseClass* OldOwner = nullptr;

ASMJIT_PATCH(0x70173B , TechnoClass_SetOwningHouse_AfterHouseWasSet, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	auto pNewOwner= pThis->Owner;

	if(OldOwner){
		if (auto pMe = flag_cast_to<FootClass* , false>(pThis))
		{
			const auto pTypeExt = GET_TECHNOTYPEEXT(pMe);
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
				TechnoExtData::ConvertToType(pMe, pConvertTo,true , false);
		}

		if (RulesExtData::Instance()->ExtendedBuildingPlacing
			&& pThis->WhatAmI() == AbstractType::Unit
			&& ((UnitClass*)pThis)->Type->DeploysInto)
		{
			HouseExtContainer::Instance.Find(OldOwner)->OwnedDeployingUnits.remove((UnitClass*)pThis);
		}

		OldOwner = nullptr;
	}

	if (GET_TECHNOTYPEEXT(pThis)->Passengers_SyncOwner && pThis->Passengers.NumPassengers > 0) {
		for (NextObject j(pThis->Passengers.GetFirstPassenger());
			j && ((*j)->AbstractFlags & AbstractFlags::Foot);
			++j)
		{
			((FootClass*)(*j))->SetOwningHouse(pNewOwner, false);

		}
	}

	return 0x0;
}


ASMJIT_PATCH(0x7015EB, TechnoClass_SetOwningHouse_UpdateTracking, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = GET_TECHNOTYPE(pThis);
	auto pOldOwnerExt = HouseExtContainer::Instance.Find(pThis->Owner);
	auto pNewOwnerExt = HouseExtContainer::Instance.Find(pNewOwner);
	//auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->Death_Method != KillMethod::None) {
		const bool humanToComputer = pTypeExt->AutoDeath_OnOwnerChange_HumanToComputer.Get(pTypeExt->AutoDeath_OnOwnerChange);
		const bool computerToHuman = pTypeExt->AutoDeath_OnOwnerChange_ComputerToHuman.Get(pTypeExt->AutoDeath_OnOwnerChange);

		if (humanToComputer && computerToHuman)
		{
			TechnoExtData::KillSelf(pThis
				, pTypeExt->Death_Method
				, pTypeExt->AutoDeath_VanishAnimation );
			return 0x70188C;
		} else if (humanToComputer || computerToHuman) {
			const bool I_am_human = pThis->Owner->IsControlledByHuman();

			if (I_am_human != pNewOwner->IsControlledByHuman()) {
				if ((I_am_human && humanToComputer) || (!I_am_human && computerToHuman)) {
					TechnoExtData::KillSelf(pThis
						, pTypeExt->Death_Method
						, pTypeExt->AutoDeath_VanishAnimation);
					return 0x70188C;
				}
			}
		}
	}

	if (!pNewOwner->Type->MultiplayPassive &&  pThis->WhatAmI() != BuildingClass::AbsID && TechnoTypeExtContainer::Instance.Find(pType)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
		pNewOwner->RecheckTechTree = true;
	}

	if (pTypeExt->Harvester_Counted && !HouseClass::IsCurrentPlayerObserver())
	{
		auto& vec = pOldOwnerExt->OwnedCountedHarvesters;
		vec.erase(pThis);

		pNewOwnerExt->OwnedCountedHarvesters.emplace(pThis);
	}

	//this kind a dangerous
	//const KillMethod nMethod = pTypeExt->Death_Method.Get();
	//if (pTypeExt->Death_IfChangeOwnership && nMethod != KillMethod::None) {
	//	TechnoExtData::KillSelf(pThis, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	//}
	if (RulesExtData::Instance()->ExtendedBuildingPlacing && pThis->WhatAmI() == AbstractType::Unit && pType->DeploysInto)
	{
		pOldOwnerExt->OwnedDeployingUnits.remove((UnitClass*)pThis);
	}

	OldOwner = pThis->Owner;
	return 0;
}

#pragma endregion

ASMJIT_PATCH(0x4FDCE0 , HouseClass_AI_Fire_Sale_OnLastLegs, 0x6){
	GET(FakeHouseClass*, pThis, ECX);
	GET_STACK(UrgencyType, urg , 0x4);

	bool ret = false;
	if(urg == UrgencyType::Critical){
		auto const pRules = RulesExtData::Instance();
		auto const pExt = pThis->_GetExtData();

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

// I must not regroup my forces.
ASMJIT_PATCH(0x739920, UnitClass_TryToDeploy_DisableRegroupAtNewConYard, 0x6)
{
	enum { SkipRegroup = 0x73992B, DoNotSkipRegroup = 0 };

	if (!RulesExtData::Instance()->RegroupWhenMCVDeploy)
		return SkipRegroup;
	else
		return DoNotSkipRegroup;
}

ASMJIT_PATCH(0x4F9BFC, HouseClass_ClearForceEnemy, 0xA)	// HouseClass_MakeAlly
{
	GET(FakeHouseClass*, pThis, ESI);

	pThis->_GetExtData()->SetForceEnemy(-1);
	pThis->UpdateAngerNodes(0u,nullptr);
	return R->Origin() + 0xA;
}

ASMJIT_PATCH(0x536FA0, ToggleRepariModeCommandClass_Execute_PlayerAutoRepair, 0x7)
{
	if (Phobos::Config::TogglePowerInsteadOfRepair)
		SidebarClass::Instance->SetTogglePowerMode(-1);
	else if (!RulesExtData::Instance()->ExtendedPlayerRepair)
		SidebarClass::Instance->SetRepairMode(-1);
	else
		EventExt::TogglePlayerAutoRepair::Raise();

	return 0x536FAC;
}

ASMJIT_PATCH(0x6A78F6, SidebarClass_Update_ToggleRepair, 0x9)
{
	GET(SidebarClass* const, pThis, ESI);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		pThis->SetTogglePowerMode(-1);
	else if(!RulesExtData::Instance()->ExtendedPlayerRepair)
		pThis->SetRepairMode(-1);
	else
		EventExt::TogglePlayerAutoRepair::Raise();

	return 0x6A7A82;
}

ASMJIT_PATCH(0x6A7AE1, SidebarClass_Update_RepairButton, 0x6)
{
	GET(SidebarClass* const, pThis, ESI);
	enum { Continue = 0x6A7AFE, TurnOffButton = 0x6A7AF4 };

	auto pButton = &Make_Global<ShapeButtonClass>(0xB0B3A0);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		return !pThis->PowerToggleMode && pButton->IsOn ? TurnOffButton : Continue;

	if(!RulesExtData::Instance()->ExtendedPlayerRepair)
		return !pThis->RepairMode && pButton->IsOn ? TurnOffButton : Continue;

	return !HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer)->PlayerAutoRepair && pButton->IsOn ? TurnOffButton : Continue;
}

ASMJIT_PATCH(0x450651, BuildingClass_UpdateRepairSell_PlayerAutoRepair, 0x8)
{
	enum { CanAutoRepair = 0x450659, CanNotAutoRepair = 0x450813 };
	GET(BuildingClass*, pThis, ESI);

	if (!RulesExtData::Instance()->ExtendedPlayerRepair)
		return pThis->Owner->IQLevel2 >= RulesClass::Instance->RepairSell ? CanAutoRepair : CanNotAutoRepair;

	if (HouseExtContainer::Instance.Find(pThis->Owner)->PlayerAutoRepair) {
		return CanAutoRepair;
	} else {
		pThis->SetRepairState(0);
		return CanNotAutoRepair;
	}
}

ASMJIT_PATCH(0x50CA12, HouseClass_RecalcCenter_DeadTechno, 0xA)
{
	enum { NextLoop = 0x50CAB4, ContinueCheck = 0x0 };
	GET(FootClass*, pTechno, ESI);

	if (!pTechno->IsAlive || pTechno->InLimbo || pTechno->BunkerLinkedItem)
		return NextLoop;

	return ContinueCheck;
}

ASMJIT_PATCH(0x7084E9, HouseClass_BaseIsAttacked_StopRecuiting, 0x6)
{
	GET(UnitClass*, pCandidate, EBX);
	bool allow = true;

	if (pCandidate->IsTethered)
	{
		allow = false;
	}
	else if (auto pContact = pCandidate->GetRadioContact())
	{
		if (auto pBldC = cast_to<BuildingClass*, false>(pContact))
		{
			if (pBldC->Type->Bunker)
				allow = false;
		}
	}
	else if (auto pBld = pCandidate->GetCell()->GetBuilding())
	{
		if (pBld->Type->Bunker)
			allow = false;
	}

	return allow ? 0x0 : 0x708622;//continue
}

ASMJIT_PATCH(0x4F671D, HouseClass_CanAfforBase_MissingPointer, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EAX);

	if (!pBld)
	{
		Debug::FatalErrorAndExit("Cannot Find BuildWeapons For [%s - %ls] , BuildWeapons Count %d\n", pThis->Type->ID, pThis->Type->UIName, RulesClass::Instance->BuildWeapons.Count);
	}

	return 0x0;
}

ASMJIT_PATCH(0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	if (BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0)
		return 0x508CEE; // add the power

	return 0x0;
}
ASMJIT_PATCH(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass* const, pProduct, ESI);
	VoxClass::PlayIndex(GET_TECHNOTYPEEXT(pProduct)->Eva_Complete.Get());
	return 0x4FB649;
}

ASMJIT_PATCH(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	enum { ReturnNoVoiceCreate = 0x4FB804, Continue = 0x0 };

	GET(HouseClass* const, pThis, EDI);
	GET(TechnoClass* const, pTechno, EBP);

	if (pTechno)
	{
		const auto pTechnoTypeExt = GET_TECHNOTYPEEXT(pTechno);

		if (pTechnoTypeExt->VoiceCreate >= 0)
		{

			if (!pTechnoTypeExt->VoiceCreate_Instant)
				pTechno->QueueVoice(pTechnoTypeExt->VoiceCreate);
			else
			{
				if (pThis->IsControlledByHuman() && !pThis->IsCurrentPlayerObserver())
					VocClass::SafeImmedietelyPlayAt(pTechnoTypeExt->VoiceCreate, &pTechno->Location);
			}
		}

		if (!pTechnoTypeExt->CreateSound_Enable.Get())
			return ReturnNoVoiceCreate;

		if (!EnumFunctions::IsPlayerTypeEligible((AffectPlayerType::Observer | AffectPlayerType::Player), HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;

		if (!EnumFunctions::CanTargetHouse(pTechnoTypeExt->CreateSound_afect.Get(RulesExtData::Instance()->CreateSound_PlayerOnly), pThis, HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;
	}

	return Continue;
}
