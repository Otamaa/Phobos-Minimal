#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/TechnoType/Body.h>

#pragma region Otamaa

DEFINE_JUMP(CALL, 0x4CD809, GET_OFFSET(AircraftExt::TriggerCrashWeapon));

//DEFINE_JUMP(LJMP, 0x4CD7DA , 0x4CD7DB)
/* wrong stack ?
DEFINE_HOOK(0x4CD7D6, FlyLocomotionClass_Movement_AI_TriggerCrashWeapon, 0x5)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x6C, 0x18));
	R->Stack(STACK_OFFS(0x6C, 0x3C), CellClass::Coord2Cell(nCoord));
	AircraftExt::TriggerCrashWeapon(pThis, 0);
	return 0x4CD80E;
}*/

DEFINE_HOOK(0x415EEE, AircraftClass_DropCargo, 0x6) //was 8
{
	GET(AircraftClass*, pThis, EDI);

	if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pThis->Passengers.FirstPassenger)
		{
			if (pTypeExt->Paradrop_DropPassangers.Get())
			{
				pThis->DropOffParadropCargo();
				return 0x415EFD;
			}
		}

		return 0x415F08;
	}

	return 0x0;
}

/*DEFINE_HOOK(0x4CD8C9, FlyLocomotionClass_Movement_AI_DisableTSExp, 0x9)
{
	GET(FootClass*, pFoot, EDX);
	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
	return pTypeExt->Disable_C4WarheadExp.Get() ? 0x4CD9C0 : 0x0;
}*/

DEFINE_HOOK(0x415991, AircraftClass_Mission_Paradrop_Overfly_Radius, 0x6)
{
	enum { ConditionMeet = 0x41599F, ConditionFailed = 0x4159C8 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	int nRadius = RulesGlobal->ParadropRadius;
	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		nRadius = pExt->ParadropOverflRadius.Get(nRadius) ;
	}

	return comparator > nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x415934, AircraftClass_Mission_Paradrop_Approach_Radius, 0x6)
{
	enum { ConditionMeet = 0x415942, ConditionFailed = 0x415956 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	int nRadius = RulesGlobal->ParadropRadius;
	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		nRadius = pExt->ParadropRadius.Get(nRadius) ;
	}

	return  comparator <= nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x413F98, AircraftClass_Init, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pThisType, EDI);

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThisType)) {
		if(pExt->Paradrop_MaxAttempt.isset())
			pThis->___paradrop_attempts = pExt->Paradrop_MaxAttempt.Get();
	}

	return 0x0;
}

DEFINE_HOOK(0x415E93, AircraftClass_DropCargo_ParaLeft, 0x7)
{
	GET(AircraftClass*, pThis, EDI);

	BYTE nAttempt = 5;
	if (auto const pType = pThis->Type) {
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pType)) {
			if(pExt->Paradrop_MaxAttempt.isset())
				nAttempt = pExt->Paradrop_MaxAttempt.Get();

		}
	}

	pThis->___paradrop_attempts = nAttempt;
	return 0x415E9A;
}

DEFINE_HOOK(0x416545, AircraftClass_Fire_AttackRangeSight_1, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		R->Stack(STACK_OFFS(0x94, 0x48), R->ECX());
		R->ECX(pTypeExt->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
		return 0x41654C;
	}

	return 0x0;
}

DEFINE_HOOK(0x416580, AircraftClass_Fire_AttackRangeSight_2, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, ECX);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		R->Stack(STACK_OFFS(0x8C, 0x48), R->EDX());
		R->EDX(pTypeExt->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
		return 0x416587;
	}

	return 0x0;
}

DEFINE_HOOK(0x4156F1, AircraftClass_Mission_SpyplaneApproach_camerasound, 0x6) {
	GET(RulesClass* const, pRules, EAX);
	GET(AircraftClass* const, pThis, ESI);

	int nIDx = pRules->SpyPlaneCamera;
	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		if(pTypeExt->SpyplaneCameraSound.isset())
			nIDx = pTypeExt->SpyplaneCameraSound.Get();
	}

	R->ECX(nIDx);
	return 0x4156F7;
}

DEFINE_HOOK(0x417A2E, AircraftClass_EnterIdleMode_Opentopped, 0x5)
{
	GET(AircraftClass*, pThis, ESI);

	R->EDI(2);

	//this plane stuck on mission::Move ! so letst redirect it to other address that deal with this
	return !pThis->Spawned &&
		pThis->Type->OpenTopped &&
		(pThis->QueuedMission != Mission::Attack) && !pThis->Target
		? 0x417944 : 0x417AD4;
}

/*
DEFINE_HOOK(0x416748, AircraftClass_MI_Move_AirportBound, 0x5)
{
	GET(AircraftClass*, pThis, ESI);

	return pThis->Type->AirportBound ? 0x41675D : 0x0;
}
*/
/*
DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Destination) {
		if (pThis->Type->IsDropship) {
			CellStruct nCell = CellStruct::Empty;
			if (pThis->SelectNavalTargeting(pThis->Destination) != 11) {
				if (auto pTech = pThis->Destination->AsTechno()) {
					auto nCoord = pTech->GetCoords();
					nCell = CellClass::Coord2Cell(nCoord);

					if (nCell != CellStruct::Empty) {
						if (auto pCell = Map[nCell]) {
							for (auto pOccupy = pCell->FirstObject;
								pOccupy;
								pOccupy = pOccupy->NextObject) {
								if (pOccupy->WhatAmI() == AbstractType::Building) {
									auto pGoodLZ = pThis->GoodLandingZone();
									pThis->SetDestination(pGoodLZ, true);
								}
								else {
									nCoord = pThis->GetCoords();
									pOccupy->Scatter(nCoord, true, true);
								}
							}
						}
					}
				}
			}
		} else {
			return 0x41531B;
		}
	}

	return 0x41530C;
}*/

DEFINE_HOOK(0x416EC9, AircraftClass_MI_Move_Carryall_AllowWater, 0x6) //was 8
{
	GET(AircraftClass*, pCarryall, ESI);

	if (auto pPassanger = pCarryall->Passengers.GetFirstPassenger())
	{
		if (auto pDest = pCarryall->Destination)
		{
			auto pDestCell = Map[pDest->GetCoords()];
			if (pDestCell->IsClearToMove(pPassanger->GetTechnoType()->SpeedType, false, false, -1, pPassanger->GetTechnoType()->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1
				&& pDestCell->IsValidMapCoords())
			{
				pCarryall->SetDestination(pCarryall->Destination, true);
				return 0x416EE4;
			}
		}
	}

	pCarryall->SetDestination(pCarryall->NewLandingZone(pCarryall->Destination), true);
	return 0x416EE4;
}

DEFINE_HOOK(0x416FFD, AircraftClass_MI_Move_Carryall_AllowWater_LZClear, 0x6) //5
{
	GET(AircraftClass*, pThis, ESI);

	if (auto pPassanger = pThis->Passengers.GetFirstPassenger())
	{
		if (auto pDest = pThis->Destination)
		{
			auto nCoord = pDest->GetCoords();
			auto pDestCell = Map[nCoord];

			R->AL(pDestCell->IsClearToMove(pPassanger->GetTechnoType()->SpeedType, false, false, -1, pPassanger->GetTechnoType()->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1
				&& pDestCell->IsValidMapCoords());

			return 0x41700E;
		}
	}

	R->AL(pThis->IsLandingZoneClear(pThis->Destination));
	return 0x41700E;
}

//size
//#ifdef ENABLE_NEWHOOKS
//Crash
//DEFINE_HOOK(0x444014, AircraftClass_ExitObject_DisableRadioContact_dummy, 0x0)
//{
//	enum { SkipAllSet = 0x444053, Nothing = 0x0 };
//
//	//GET(BuildingClass*, pBuilding, ESI);
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct)
//	{
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->Type);
//		if (pTypeExt && !pProduct->Type->AirportBound && pTypeExt->NoAirportBound_DisableRadioContact.Get())
//		{
//			return SkipAllSet;
//		}
//	}
//
//	return Nothing;
//}
//#endif

#ifdef TEST_CODE
DEFINE_HOOK(0x4197FC, AircraftClass_MI_Attack_GoodFireLoc_Range, 0x6)
{
	GET(AircraftClass*, pThis, EDI);
	R->EAX(pThis->GetWeaponRange(pThis->SelectWeapon(pThis->Target)));
	return 0x419808;
}

DEFINE_HOOK(0x418072, AircraftClass_MI_Attack_BypasPassangersRangeDeterminer, 0x5)
{
	GET(AircraftClass*, pThis, ESI);
	auto const pTarget = generic_cast<TechnoClass*>(pThis->Target);
	pThis->SetDestination(pTarget ? pTarget->GetCell() : pThis->GoodTargetLoc(pThis->Target), true);
	return 0x418087;
}
#endif
#pragma endregion