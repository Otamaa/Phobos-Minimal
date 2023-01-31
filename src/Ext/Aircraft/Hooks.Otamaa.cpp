#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>

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
	//GET_STACK(int, weaponIdx, STACK_OFFSET(0x7C, 0xC));

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Passengers.FirstPassenger)
	{
		if (pTypeExt->Paradrop_DropPassangers.Get())
		{
			pThis->DropOffParadropCargo();
			return 0x415EFD;
		}
	}

	GET_BASE(AbstractClass*, pTarget, 0x8);
	GET_BASE(int, nWeaponIdx, 0xC);

	const auto pBullet = pThis->TechnoClass::Fire(pTarget, nWeaponIdx);

	R->ESI(pBullet);
	R->Stack(0x10 , pBullet);

	if (!pBullet)
		return 0x41659E;

	R->EBX(pTarget);

	if (pTypeExt->Firing_IgnoreGravity.Get())
		return 0x41631F;

	if (!pBullet->Type->ROT)
		return 0x415F4D;

	return !(pBullet->Type->ROT == 1) ? 0x41631F : 0x4160CF;
}

DEFINE_HOOK(0x415991, AircraftClass_Mission_Paradrop_Overfly_Radius, 0x6)
{
	enum { ConditionMeet = 0x41599F, ConditionFailed = 0x4159C8 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->ParadropOverflRadius.Get(RulesGlobal->ParadropRadius);
	return comparator > nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x415934, AircraftClass_Mission_Paradrop_Approach_Radius, 0x6)
{
	enum { ConditionMeet = 0x415942, ConditionFailed = 0x415956 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->ParadropRadius.Get(RulesGlobal->ParadropRadius);
	return  comparator <= nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x416545, AircraftClass_Fire_AttackRangeSight_1, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	R->Stack(STACK_OFFS(0x94, 0x48), R->ECX());
	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->Type)->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
	return 0x41654C;
}

DEFINE_HOOK(0x416580, AircraftClass_Fire_AttackRangeSight_2, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, ECX);

	R->Stack(STACK_OFFS(0x8C, 0x48), R->EDX());
	R->EDX(TechnoTypeExt::ExtMap.Find(pThis->Type)->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
	return 0x416587;
}

DEFINE_HOOK(0x4156F1, AircraftClass_Mission_SpyplaneApproach_camerasound, 0x6)
{
	GET(RulesClass* const, pRules, EAX);
	GET(AircraftClass* const, pThis, ESI);
	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->Type)->SpyplaneCameraSound.Get(pRules->SpyPlaneCamera));
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


DEFINE_HOOK(0x416748, AircraftClass_AirportBound_SkipValidatingLZ, 0x5)
{
	GET(AircraftClass*, pThis, ESI);
	return pThis->Type->AirportBound ? 0x41675D : 0x0;
}

DEFINE_HOOK(0x4179AA, AircraftClass_EnterIdleMode_AlreadiHasDestination, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(BuildingClass*, pDest, EAX);
	R->EDI(pDest);
	return pThis->Destination == pDest ? 0x4179DD : 0x0;
}

DEFINE_HOOK(0x4CD105, FlyLocomotionClass_StopMoving_AirportBound, 0x5)
{
	GET(AircraftClass*, pThis, EDI);
	return pThis->Type->AirportBound ? 0x4CD12A : 0x0;
}

//DEFINE_HOOK(0x419CC1, AircraftClass_Mi_Enter_AiportBound, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(AbstractClass*, pNavCom, EDI);
//	GET(BuildingClass*, pCellBuilding, EAX);
//
//	if (pNavCom != pCellBuilding)
//	{
//		if (auto pNavBuilding = specific_cast<BuildingClass*>(pNavCom))
//		{
//			pThis->DockedTo = pNavBuilding;
//			pThis->SendToFirstLink(RadioCommand::NotifyUnlink);
//			pThis->SendCommand(RadioCommand::RequestLink, pNavBuilding);
//			pThis->SetDestination(pNavBuilding, true);
//			return 0x419CFF;
//		}
//
//		if (pThis->SendCommand(RadioCommand::QueryCanEnter, pCellBuilding) == RadioCommand::AnswerPositive)
//		{
//			pThis->SendToFirstLink(RadioCommand::NotifyUnlink);
//			pThis->SendToFirstLink(RadioCommand::RequestUntether);
//			pThis->SendCommand(RadioCommand::RequestLink, pCellBuilding);
//			pThis->SetDestination(pCellBuilding, true);
//		}
//	}
//
//	return 0x419D0B;
//}

#pragma endregion