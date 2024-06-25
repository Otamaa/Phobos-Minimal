#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>

#pragma region Otamaa
static void __fastcall TriggerCrashWeapon_Wrapper(AircraftClass* pThis, DWORD, int nMult) {
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

#include <Ext/WeaponType/Body.h>
bool IsFlyLoco(const ILocomotion* pLoco)
{
	return (((DWORD*)pLoco)[0] == FlyLocomotionClass::ILoco_vtable);
}

constexpr FORCEINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon)
{
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
}

DEFINE_HOOK(0x415EEE, AircraftClass_FireAt_DropCargo, 0x6) //was 8
{
	GET(AircraftClass*, pThis, EDI);
	GET_BASE(int, nWeaponIdx, 0xC);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	bool DropPassengers = pTypeExt->Paradrop_DropPassangers;

	if (pThis->Passengers.FirstPassenger)
	{
		if (auto pWewapons = pThis->GetWeapon(nWeaponIdx))
		{
			if (pWewapons->WeaponType)
			{
				const auto pExt = WeaponTypeExtContainer::Instance.Find(pWewapons->WeaponType);
				if (pExt->KickOutPassenger.isset())
					DropPassengers = pExt->KickOutPassenger; //#1151
			}
		}

		if (DropPassengers)
		{
			pThis->DropOffParadropCargo();
			return 0x415EFD;
		}
	}

	GET_BASE(AbstractClass*, pTarget, 0x8);

	const auto pBullet = pThis->TechnoClass::Fire(pTarget, nWeaponIdx);

	R->ESI(pBullet);
	R->Stack(0x10, pBullet);

	if (!pBullet)
		return 0x41659E;

	if (AircraftCanStrafeWithWeapon(pBullet->WeaponType)) {
		TechnoExtContainer::Instance.Find(pThis)->ShootCount++;

		if (WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType)->Strafing_UseAmmoPerShot) {
			pThis->loseammo_6c8 = false;
			pThis->Ammo--;
		}
	}

	R->EBX(pTarget);

	if (pTypeExt->Firing_IgnoreGravity.Get())
		return 0x41631F;

	if (!pBullet->Type->ROT)
	{
		if (IsFlyLoco(pThis->Locomotor.GetInterfacePtr()))
		{
			if (pBullet->Type->Cluster)
				return 0x415F4D;

			const auto pLocomotor = static_cast<FlyLocomotionClass*>(pThis->Locomotor.GetInterfacePtr());
			const double currentSpeed = pThis->Type->Speed * pLocomotor->CurrentSpeed * TechnoExtData::GetCurrentSpeedMultiplier(pThis);

			R->EAX(int(currentSpeed));
			return 0x415F5C;
		}

		return 0x415F4D;
	}

	return !(pBullet->Type->ROT == 1) ? 0x41631F : 0x4160CF;
}

DEFINE_HOOK(0x415991, AircraftClass_Mission_Paradrop_Overfly_Radius, 0x6)
{
	enum { ConditionMeet = 0x41599F, ConditionFailed = 0x4159C8 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExtContainer::Instance.Find(pThis->Type)->ParadropOverflRadius.Get(RulesClass::Instance->ParadropRadius);
	return comparator > nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x415934, AircraftClass_Mission_Paradrop_Approach_Radius, 0x6)
{
	enum { ConditionMeet = 0x415942, ConditionFailed = 0x415956 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExtContainer::Instance.Find(pThis->Type)->ParadropRadius.Get(RulesClass::Instance->ParadropRadius);
	return  comparator <= nRadius ? ConditionMeet : ConditionFailed;
}

DEFINE_HOOK(0x416545, AircraftClass_FireAt_AttackRangeSight_1, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, EAX);

	R->Stack(STACK_OFFS(0x94, 0x48), R->ECX());
	R->ECX(TechnoTypeExtContainer::Instance.Find(pThis->Type)->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
	return 0x41654C;
}

DEFINE_HOOK(0x416580, AircraftClass_FireAt_AttackRangeSight_2, 0x7)
{
	GET(AircraftClass*, pThis, EDI);
	GET(RulesClass*, pRules, ECX);

	R->Stack(STACK_OFFS(0x8C, 0x48), R->EDX());
	R->EDX(TechnoTypeExtContainer::Instance.Find(pThis->Type)->AttackingAircraftSightRange.Get(pRules->AttackingAircraftSightRange));
	return 0x416587;
}

DEFINE_HOOK(0x4156F1, AircraftClass_Mission_SpyplaneApproach_camerasound, 0x6)
{
	GET(RulesClass* const, pRules, EAX);
	GET(AircraftClass* const, pThis, ESI);
	R->ECX(TechnoTypeExtContainer::Instance.Find(pThis->Type)->SpyplaneCameraSound.Get(pRules->SpyPlaneCamera));
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
	R->DL(TechnoTypeExtContainer::Instance.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x4183C9;
}

DEFINE_HOOK(0x418671, AircraftClass_CurleyShuffle_B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, EDX);
	R->AL(TechnoTypeExtContainer::Instance.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418677;
}

DEFINE_HOOK(0x418733, AircraftClass_CurleyShuffle_C, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, EAX);
	R->CL(TechnoTypeExtContainer::Instance.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418739;
}

DEFINE_HOOK(0x418782, AircraftClass_CurleyShuffle_D, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(RulesClass*, pRules, ECX);
	R->DL(TechnoTypeExtContainer::Instance.Find(pThis->Type)->CurleyShuffle.Get(pRules->CurleyShuffle));
	return 0x418788;
}

//DEFINE_HOOK(0x41B7F0, AircraftClass_IFlyControl_Is_Strafe, 0x6)
//{
//	GET_STACK(IFlyControl*, ptr, 0x4);
//
//	AircraftClass* pAircraft = static_cast<AircraftClass*>(ptr);
//	bool result = false;
//
//	if (pAircraft->CurrentMission == Mission::Attack && pAircraft->Target)
//	{
//		const WeaponStruct* pWeapon = pAircraft->GetWeapon(pAircraft->SelectWeapon(pAircraft->Target));
//
//		if (pWeapon && pWeapon->WeaponType)
//		{
//			WeaponTypeClass* pWeaponType = pWeapon->WeaponType;
//			auto pWeaponTypeExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);
//
//			if (pWeaponType->Projectile->ROT <= 1 && !pWeaponType->Projectile->Inviso
//				|| pWeaponTypeExt->Strafing_Shots.isset())
//			{
//				result = true;
//			}
//		}
//
//		R->EAX(result);
//		return 0x41B83B;
//	}
//
//	WeaponStruct* pPrimary = pAircraft->GetWeapon(0);
//
//	if (pPrimary && pPrimary->WeaponType)
//	{
//		R->EAX(pPrimary->WeaponType->Projectile->ROT <= 1
//			&& !pPrimary->WeaponType->Projectile->Inviso);
//	}
//
//	return 0x41B83B;
//}

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
	pThis->SetDestination(pTarget ? pTarget->GetCell() : pThis->GoodTargetLoc_(pThis->Target), true);
	return 0x418087;
}
#endif

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

//#pragma optimize("", off )
//int Mission_Attack(AircraftClass* pThis)
//{
//	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
//
//	if (/*!SecondFiringMethod &&*/ !(pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
//		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe))
//	{
//		const auto pWeaponStr = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));
//
//		if (pWeaponStr && pWeaponStr->WeaponType)
//		{
//			int fireCount = pThis->MissionStatus - 4;
//			if (fireCount > 1 &&
//				WeaponTypeExtContainer::Instance.Find(pWeaponStr->WeaponType)->Strafing_Shots < fireCount)
//			{
//
//				if (!pThis->Ammo)
//					pThis->__DoingOverfly = false;
//
//				pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
//			}
//		}
//	}
//
//	switch ((AirAttackStatusP)pThis->MissionStatus)
//	{
//	case AirAttackStatusP::AIR_ATT_VALIDATE_AZ:
//	{
//		pThis->__DoingOverfly = false;
//
//		pThis->MissionStatus = //(pThis->Target ? -9 : 0) + (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE
//			!pThis->Target ? 11 : (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//
//		return 1;
//	}
//	case AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION:
//	{
//		pThis->__DoingOverfly = false;
//
//		if (pThis->loseammo_6c8)
//		{
//			pThis->loseammo_6c8 = false;
//			/*if(!SecondFiringMethod)*/
//				pThis->Ammo -= 1;
//		}
//
//		if (pThis->Target && pThis->Ammo)
//		{
//			pThis->SetDestination(pThis->GoodTargetLoc_(pThis->Target), 1);
//			pThis->MissionStatus = (int)(pThis->Destination != 0 ?
//				AirAttackStatusP::AIR_ATT_FLY_TO_POSITION : AirAttackStatusP::AIR_ATT_RETURN_TO_BASE);
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//		}
//
//		const auto v92 = pThis->GetCurrentMissionControl();
//		return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//	}
//	case AirAttackStatusP::AIR_ATT_FLY_TO_POSITION:
//	{
//		if (pThis->loseammo_6c8)
//		{
//			pThis->loseammo_6c8 = 0;
//
//			/*if(!SecondFiringMethod)*/
//				pThis->Ammo -= 1;
//		}
//
//		pThis->__DoingOverfly = false;
//
//		if (!pThis->Target || !pThis->Ammo)
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//
//		if (pThis->Is_Strafe())
//		{
//			const auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//
//			if (pThis->DistanceFrom(pThis->Target) < pPrimary->Range)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//		}
//		else
//		{
//			if (pThis->Is_Locked())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//
//			if (!pThis->Locomotor->Is_Moving_Now()) {
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				return 1;
//			}
//		}
//
//		if (pThis->Destination)
//		{
//			const auto v13 = pThis->DistanceFrom(pThis->Destination);
//
//			if (v13 >= 512)
//			{
//				auto v16 = pThis->Destination->GetCoords();
//				auto v17 = pThis->GetFLH(0,CoordStruct::Empty);
//
//				DirStruct nDir {};
//
//				if (v17.X != v16.X || v17.Y != v16.Y) {
//					nDir.SetRadian<65536>(Math::atan2(double(v17.Y - v16.Y), double(v16.X - v17.X))); ;
//				}
//
//				pThis->SecondaryFacing.Set_Desired(nDir);
//
//				return 1;
//			}
//			else
//			{
//				pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//
//				if (v13 >= 16)
//				{
//					return 1;
//				}
//				else
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//					pThis->SetDestination(nullptr, 1);
//					return 1;
//				}
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0:
//	{
//		if (!pThis->Target || !pThis->Ammo)
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//
//		if (/*SecondFiringMethod ||*/ !pThis->Is_Strafe())
//		{
//			pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//			pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//		}
//
//		auto v26 = pThis->SelectWeapon(pThis->Target);
//		auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//
//		switch (pThis->GetFireError(pThis->Target, v26, 1))
//		{
//		case FireError::OK:
//		{
//			pThis->loseammo_6c8 = 1;
//
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt , v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if(pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			if (pThis->Is_Strafe())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET2;
//				pThis->__DoingOverfly = 1;
//				return pPrimary->ROF;
//			}
//
//			if (pThis->Is_Locked())
//			{
//				const auto v37 = pThis->Ammo;
//				bool v38 = v37 == 0;
//				bool v39 = v37 < 0;
//				pThis->__DoingOverfly = 1;
//				pThis->MissionStatus = !v39 && !v38 ?
//					(int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION : (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return pPrimary->ROF;
//			}
//
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1;
//			return 1;
//		}
//		case FireError::FACING:
//		{
//			if (!pThis->Ammo)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return 1;
//			}
//
//			if (!pThis->IsCloseEnoughToAttack(pThis->Target) || /*!SecondFiringMethod ||*/ pThis->Is_Strafe())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//			}
//			else if (pThis->Is_Locked())
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//			}
//			else
//			{
//				pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//					(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//			}
//
//			return !pThis->Is_Strafe() ? 1 : 45;
//		}
//		case FireError::REARM:
//		{
//			return 1;
//		}
//		case FireError::CLOAKED:
//		{
//			pThis->Uncloak(0);
//			return 1;
//		}
//		default:
//		{
//			if (!pThis->Ammo)
//			{
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//				return 1;
//			}
//
//			if (pThis->Is_Strafe())
//			{
//				return 1;
//			}
//
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1;
//			return 1;
//		}
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1:
//	{
//		if (pThis->Target)
//		{
//			pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//			pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Target));
//
//			auto v44 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v44, 1))
//			{
//			case FireError::OK:
//			{
//				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt , v44);
//				//pThis->Fire(pThis->Target, v44);
//
//				if (pThis->Target)
//				{
//					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//						->ScatterContent(pThis->Location, true, false, false);
//				}
//
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//					(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			case FireError::FACING:
//			{
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				if (!pThis->IsCloseEnoughToAttack(pThis->Target) || pThis->Is_Strafe())
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				}
//				else
//				{
//					pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//						(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				}
//
//				if (!pThis->Is_Strafe())
//				{
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				return 45;
//			}
//			case FireError::REARM:
//			{
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			case FireError::CLOAKED:
//			{
//				pThis->Uncloak(0);
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			default:
//			{
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					const auto v92 = pThis->GetCurrentMissionControl();
//					return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//				}
//
//				if (pThis->IsCloseEnoughToAttack(pThis->Target))
//				{
//					pThis->MissionStatus = pTypeExt->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle) ?
//						(int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET1 : (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET0;
//				}
//				else
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				}
//
//				const auto v92 = pThis->GetCurrentMissionControl();
//				return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//			}
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET2:
//	{
//		if (pThis->Target)
//		{
//			auto v52 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v52, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe2 , v52);
//			//pThis->Fire(pThis->Target, v52);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET3;
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET3:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe3, v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET4;
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET4:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::CLOAKED:
//				break;
//			case FireError::RANGE:
//				pThis->SetDestination(pThis->Target, 1);
//				break;
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//
//			auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//			AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe4, v26);
//			//pThis->Fire(pThis->Target, v26);
//
//			if (pThis->Target)
//			{
//				MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//					->ScatterContent(pThis->Location, true, false, false);
//			}
//
//			pThis->SetDestination(pThis->Target, 1);
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET5;
//
//			return pPrimary->ROF;
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_FIRE_AT_TARGET5:
//	{
//		if (pThis->Target)
//		{
//			auto v26 = pThis->SelectWeapon(pThis->Target);
//
//			switch (pThis->GetFireError(pThis->Target, v26, 1))
//			{
//			case FireError::OK:
//			case FireError::FACING:
//			case FireError::RANGE:
//			case FireError::CLOAKED:
//			{
//				auto pPrimary = pThis->GetWeapon(0)->WeaponType;
//				AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe5, v26);
//				//pThis->Fire(pThis->Target, v26);
//
//				if (pThis->Target)
//				{
//					MapClass::Instance->GetCellAt(pThis->Target->GetCoords())
//						->ScatterContent(pThis->Location, true, false, false);
//				}
//
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_FLY_TO_POSITION;
//
//				return (pPrimary->Range + 1024) / pThis->Type->Speed;
//			}
//			default:
//				if (!pThis->Ammo)
//				{
//					pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//					pThis->__DoingOverfly = 0;
//				}
//
//				return 1;
//			}
//		}
//		else
//		{
//			pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_RETURN_TO_BASE;
//			return 1;
//		}
//	}
//	case AirAttackStatusP::AIR_ATT_RETURN_TO_BASE:
//	{
//		pThis->__DoingOverfly = 0;
//
//		if (pThis->loseammo_6c8)
//		{
//			const auto v85 = pThis->Ammo;
//			pThis->loseammo_6c8 = 0;
//
//			if (v85 > 0) {
//				pThis->Ammo = v85 - 1;
//			}
//		}
//
//		if (pThis->Ammo) {
//			if (pThis->Target) {
//				pThis->MissionStatus = (int)AirAttackStatusP::AIR_ATT_PICK_ATTACK_LOCATION;
//				return 1;
//			}
//		}
//		else if (pThis->IsALoaner || pThis->Owner->IsControlledByCurrentPlayer())
//		{
//			pThis->SetTarget(nullptr);
//		}
//
//		pThis->__DoingOverfly = false;
//		pThis->SetDestination(
//			MapClass::Instance->GetCellAt(
//			MapClass::Instance->PickCellOnEdge(
//			pThis->Owner->GetCurrentEdge(),
//				CellStruct::Empty,
//				CellStruct::Empty,
//				SpeedType::Winged,
//				true,
//				MovementZone::Normal)), true
//		);
//
//		pThis->retreating_idle = false;
//		if (pThis->Airstrike && pThis->Ammo > 0)
//		{
//			pThis->QueueMission(Mission::Retreat, false);
//			pThis->retreating_idle = true;
//		}
//		else
//		{
//			pThis->EnterIdleMode(false, true);
//			pThis->retreating_idle = true;
//		}
//
//		return 1;
//	}
//	default:
//	{
//		const auto v92 = pThis->GetCurrentMissionControl();
//		return int(v92->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2);
//	}
//	}
//}
//#pragma optimize("", on )

// there is some funky shit happening here
// the code is 90% close to decomp but it result different
// need to investigae
// disabled atm !
//DEFINE_HOOK(0x417FE0, AircraftClass_MI_Attack_Handle, 0x6)
//{
//	R->EAX(Mission_Attack(R->ECX<AircraftClass*>()));
//	return 0x418D54;
//}

#pragma endregion