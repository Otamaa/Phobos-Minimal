#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>

#pragma region Otamaa
static void FC TriggerCrashWeapon_Wrapper(AircraftClass* pThis, DWORD, int nMult) {
	AircraftExt::TriggerCrashWeapon(pThis, nMult);
}

DEFINE_JUMP(CALL, 0x4CD809, GET_OFFSET(TriggerCrashWeapon_Wrapper));

//DEFINE_HOOK(0x4CD7D6, FlyLocomotionClass_Movement_AI_TriggerCrashWeapon, 0x5)
//{
//	GET(AircraftClass*, pLinked, ECX);
//
//	AircraftExt::TriggerCrashWeapon(pLinked, 0);
//	R->EAX(MapClass::Instance->GetCellAt(pLinked->Location)); //restore overriden instructions
//	return 0x4CD81D;
//}

DEFINE_HOOK(0x415EEE, AircraftClass_FireAt_DropCargo, 0x6) //was 8
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

	const int nRadius = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->ParadropOverflRadius.Get(RulesClass::Instance->ParadropRadius);
	return comparator > nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x415934, AircraftClass_Mission_Paradrop_Approach_Radius, 0x6)
{
	enum { ConditionMeet = 0x415942, ConditionFailed = 0x415956 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->ParadropRadius.Get(RulesClass::Instance->ParadropRadius);
	return  comparator <= nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x416545, AircraftClass_FireAt_AttackRangeSight_1, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	R->Stack(STACK_OFFS(0x94, 0x48), R->ECX());
	R->ECX(TechnoTypeExt::ExtMap.Find(pThis->Type)->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
	return 0x41654C;
}

DEFINE_HOOK(0x416580, AircraftClass_FireAt_AttackRangeSight_2, 0x7)
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

	AbstractClass* const pDest = AircraftExt::IsValidLandingZone(pCarryall) ?
		pCarryall->Destination : pCarryall->NewLandingZone_(pCarryall->Destination);

	pCarryall->SetDestination(pDest, true);
	return 0x416EE4;
}

DEFINE_HOOK(0x416FFD, AircraftClass_MI_Move_Carryall_AllowWater_LZClear, 0x6) //5
{
	GET(AircraftClass*, pThis, ESI);

	if (AircraftExt::IsValidLandingZone(pThis)) {
		R->AL(true);
	} else {
		R->AL(pThis->IsLandingZoneClear(pThis->Destination));
	}

	return 0x41700E;
}

DEFINE_HOOK(0x4183C3, AircraftClass_CurleyShuffle_A, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, ECX);
	R->DL(TechnoTypeExt::ExtMap.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x4183C9;
}

DEFINE_HOOK(0x418671, AircraftClass_CurleyShuffle_B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, EDX);
	R->AL(TechnoTypeExt::ExtMap.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418677;
}

DEFINE_HOOK(0x418733, AircraftClass_CurleyShuffle_C, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, EAX);
	R->CL(TechnoTypeExt::ExtMap.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418739;
}

DEFINE_HOOK(0x418782, AircraftClass_CurleyShuffle_D, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, ECX);
	R->DL(TechnoTypeExt::ExtMap.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418788;
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

//DEFINE_HOOK(0x416748, AircraftClass_AirportBound_SkipValidatingLZ, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//	return pThis->Type->AirportBound ? 0x41675D : 0x0;
//}
//
//DEFINE_HOOK(0x4179AA, AircraftClass_EnterIdleMode_AlreadiHasDestination, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(BuildingClass*, pDest, EAX);
//	R->EDI(pDest);
//	return pThis->Destination == pDest ? 0x4179DD : 0x0;
//}
//
//DEFINE_HOOK(0x4CD105, FlyLocomotionClass_StopMoving_AirportBound, 0x5)
//{
//	GET(AircraftClass*, pThis, EDI);
//	return pThis->Type->AirportBound ? 0x4CD12A : 0x0;
//}
//
//DEFINE_HOOK(0x73C71D, UnitClass_DrawSHP_FacingDir, 0x6)
//{
//	GET(TechnoClass*, pThis, EBP);
//	GET(int, nFacingOffs, EDX);
//
//	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer()))
//	{
//		auto const pTargetType = pThis->GetDisguise(true);
//		if (pTargetType && pTargetType->WhatAmI() == AbstractType::UnitType)
//		{
//			auto const pDisguiseUnit = static_cast<UnitTypeClass*>(pTargetType);
//			int nFacing = pDisguiseUnit->Facings;
//			int nIdx = Helpers_DP::Dir2FrameIndex(pThis->PrimaryFacing.Current(), nFacing);
//
//			if (nFacingOffs == 0)
//			{
//				nFacingOffs += nIdx;
//			}
//			else
//			{
//				auto nWakFrames = pDisguiseUnit->WalkFrames;
//				nFacingOffs += (nIdx * nWakFrames + pDisguiseUnit->StartWalkFrame);
//			}
//
//			R->EDX(nFacingOffs);
//		}
//	}
//
//	return 0;
//}
//
//static UnitTypeClass* GetUnitDisguise(TechnoClass* pThis)
//{
//	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer())) {
//		ObjectTypeClass* pDisguiseType = pThis->GetDisguise(true);
//		if (pDisguiseType && pDisguiseType->WhatAmI() == AbstractType::UnitType) {
//			return static_cast<UnitTypeClass*>(pDisguiseType);
//		}
//	}
//
//	return nullptr;
//}
//
//DEFINE_HOOK(0x73C655, UnitClass_DrawSHP_TechnoType, 0x6)
//{
//	GET(TechnoClass*, pThis, EBP);
//	if (auto pDisguiseType = GetUnitDisguise(pThis)) {
//		R->ECX(pDisguiseType);
//		return 0x73C65B;
//	}
//	return 0x0;
//}
//DEFINE_HOOK(0x73C69D, UnitClass_DrawSHP_TechnoType2, 0x6)
//{
//	GET(TechnoClass*, pThis, EBP);
//	if (auto pDisguiseType = GetUnitDisguise(pThis)) {
//		R->ECX(pDisguiseType);
//		return 0x73C6A3;
//	}
//	return 0x0;
//}
//DEFINE_HOOK(0x73C702, UnitClass_DrawSHP_TechnoType3, 0x6)
//{
//	GET(TechnoClass*, pThis, EBP);
//	if (auto pDisguiseType = GetUnitDisguise(pThis)) {
//		R->ECX(pDisguiseType);
//		return 0x73C708;
//	}
//	return 0x0;
//}
//
//DEFINE_HOOK(0x73C725, UnitClass_DrawSHP_HasTurret, 0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//	if (auto pDisguiseType = GetUnitDisguise(pThis)) {
//		if(!pDisguiseType->Turret)
//			return 0x73CE0D;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x73B765, UnitClass_DrawVoxel_TurretFacing, 0x5)
//DEFINE_HOOK_AGAIN(0x73BA78, UnitClass_DrawVoxel_TurretFacing, 0x6)
//DEFINE_HOOK_AGAIN(0x73BD8B, UnitClass_DrawVoxel_TurretFacing, 0x5)
//DEFINE_HOOK(0x73BDA3, UnitClass_DrawVoxel_TurretFacing, 0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//	
//	if (!pThis->GetTechnoType()->Turret) {
//		if (auto pDisguiseType = GetUnitDisguise(pThis)) { 
//			if (pDisguiseType->Turret) {
//				GET(DirStruct*, pDir, EAX);
//				*pDir = (pThis->PrimaryFacing.Current());
//			}
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x73B8E3, UnitClass_DrawVoxel_HasChargeTurret , 0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//	GET(TechnoTypeClass*, pThisType, EBX);
//
//	if (pThisType != pThis->GetTechnoType()) {
//		return pThisType->TurretCount > 0 && !pThisType->IsGattling ? 0x73B8EC : 0x73B92F;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x73BC28, UnitClass_DrawVoxel_HasChargeTurret2,0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//	GET(TechnoTypeClass*, pThisType, EBX);
//
//	if (pThisType != pThis->GetTechnoType()) {
//		if (pThisType->TurretCount > 0 && !pThisType->IsGattling)
//		{
//			if (pThis->CurrentTurretNumber < 0)
//			{
//				R->Stack<int>(0x1C, 0);
//				return 0x73BC35;
//			}
//		}
//		else
//		{
//			return 0x73BD79;
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x73BA63, UnitClass_DrawVoxel_TurretOffset, 0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//	GET(TechnoTypeClass*, pThisType, EBX);
//
//	if (pThisType != pThis->GetTechnoType())
//	{
//		if (pThisType->TurretCount > 0 && !pThisType->IsGattling)
//		{
//			if (pThis->CurrentTurretNumber < 0)
//			{
//				R->Stack<int>(0x1C, 0);
//				return 0x73BC35;
//			}
//			else
//			{
//				return 0x73BD79;
//			}
//		}
//	}
//
//	return 0x0;
//}
//
//
////TechnoClass_Draw_VXL_Disguise_Blit_Flags 0x5
//DEFINE_JUMP(LJMP, 0x706724, 0x706731);

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