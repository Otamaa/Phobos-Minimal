#include "Body.h"

#include <AircraftClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Misc/MapRevealer.h>

#include <Misc/Hooks.Otamaa.h>

#include <Locomotor/FlyLocomotionClass.h>

#include <EventClass.h>

// If strafing weapon target is in air, consider the cell it is on as the firing position instead of the object itself if can fire at it.
ASMJIT_PATCH(0x4197F3, AircraftClass_GetFireLocation_Strafing, 0x5)
{
	GET(AircraftClass*, pThis, EDI);
	GET(AbstractClass*, pTarget, EAX);

	if (!pTarget)
		return 0;

	auto const pObject = flag_cast_to<ObjectClass* , false>(pTarget);

	if (!pObject || !pObject->IsInAir())
		return 0;

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex;

	if (weaponIndex < 0)
		weaponIndex = pThis->SelectWeapon(pTarget);

	if (pThis->GetFireError(pTarget, weaponIndex, false) != FireError::OK)
		return 0;

	R->EAX(MapClass::Instance->GetCellAt(pObject->GetCoords()));

	return 0;
}

//
//DEFINE_FUNCTION_JUMP(CALL6, 0x4180F9, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x4184E3, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x41852B, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x418893, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x4189A2, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x418AB1, FakeAircraftClass::_GetWeapon));
//DEFINE_FUNCTION_JUMP(CALL6, 0x418B9A, FakeAircraftClass::_GetWeapon));


DEFINE_FUNCTION_JUMP(VTABLE, 0x7E266C, FakeAircraftClass::_SetTarget);

#ifndef SecondMode
ASMJIT_PATCH(0x417FF1, AircraftClass_Mission_Attack_StrafeShots, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	int weaponIndex = pExt->CurrentAircraftWeaponIndex;

	if (weaponIndex < 0) {
		pExt->CurrentAircraftWeaponIndex = weaponIndex = pThis->SelectWeapon(pThis->Target);
	}

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe
		)
	{
		pExt->ShootCount = 0;
		return 0;
	}

	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	int count = pWeaponExt->Strafing_Shots.Get(5);

	if (count > 5) {
		if (pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget3_Strafe) {
			if ((count - 3 - pExt->ShootCount) > 0) {
				pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
			}
		}
	}

	return 0;
}

COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon)
{
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
}

bool FireBurst(AircraftClass* pAir, AbstractClass* pTarget)
{
	auto WeaponIdx = TechnoExtContainer::Instance.Find(pAir)->CurrentAircraftWeaponIndex;
	if (WeaponIdx < 0)
		WeaponIdx = pAir->SelectWeapon(pAir->Target);

	const auto pWeaponStruct = pAir->GetWeapon(WeaponIdx);
	bool Scatter = true;

	if (pWeaponStruct)
	{
		const auto weaponType = pWeaponStruct->WeaponType;

		if (weaponType)
		{
			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(weaponType);
			bool isStrafe = pAir->Is_Strafe();

			if (weaponType->Burst > 0)
			{
				for (int i = 0; i < weaponType->Burst; i++)
				{
					if (isStrafe && weaponType->Burst < 2 && pWeaponExt->Strafing_SimulateBurst)
						pAir->CurrentBurstIndex = TechnoExtContainer::Instance.Find(pAir)->ShootCount % 2 == 0;

					pAir->Fire(pTarget, WeaponIdx);
				}

				if (isStrafe)
				{
					TechnoExtContainer::Instance.Find(pAir)->StrafeFireCunt++;
					
					if (pWeaponExt->Strafing_UseAmmoPerShot)
					{
						pAir->Ammo--;
						pAir->loseammo_6c8 = false;

						if (!pAir->Ammo)
						{
							pAir->SetTarget(nullptr);
							pAir->SetDestination(nullptr, true);
						}
					}
				}
			}

		}
	}

	if (pAir->Target)
	{
		if (Scatter)
		{
			auto coord = pAir->Target->GetCoords();
			if (auto pCell = MapClass::Instance->TryGetCellAt(coord))
			{
				pCell->ScatterContent(coord, true, false, false);
			}
		}

		return true;
	}

	return false;
}

ASMJIT_PATCH(0x4186B6, AircraftClass_Mission_Attack_FireAt_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	FireBurst(pThis, pThis->Target);
	return 0x418720;
}

ASMJIT_PATCH(0x418805, AircraftClass_Mission_Attack_Strafe2_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireBurst(pThis, pThis->Target) ? 0x418883 : 0x418870;
}

ASMJIT_PATCH(0x418914, AircraftClass_Mission_Attack_Strafe3_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireBurst(pThis, pThis->Target) ? 0x418992 : 0x41897F;
}

ASMJIT_PATCH(0x418A23, AircraftClass_Mission_Attack_Strafe4_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireBurst(pThis, pThis->Target) ? 0x418AA1 : 0x418A8E;
}

ASMJIT_PATCH(0x418B1F, AircraftClass_Mission_Attack_Strafe5_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	FireBurst(pThis, pThis->Target);
	return 0x418B8A;
}

ASMJIT_PATCH(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x6) //8
{
	GET(AircraftClass*, pThis, ESI);

	pThis->loseammo_6c8 = true;

	FireBurst(pThis, pThis->Target);

	return 0x4184C2;
}

#undef Hook_AircraftBurstFix
#endif

static int GetDelay(AircraftClass* pThis, bool isLastShot)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex >= 0 ? pExt->CurrentAircraftWeaponIndex : pThis->SelectWeapon(pThis->Target);
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int delay = pWeapon->ROF;

	if (isLastShot || pExt->ShootCount == pWeaponExt->Strafing_Shots || (pWeaponExt->Strafing_UseAmmoPerShot && !pThis->Ammo))
	{
		pThis->MissionStatus = (int)AirAttackStatus::FlyToPosition;
		delay = pWeaponExt->Strafing_EndDelay.Get((pWeapon->Range + 1024) / pThis->Type->Speed);
	}

	return delay;
}

ASMJIT_PATCH(0x4184CC, AircraftClass_Mission_Attack_Delay1A, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->IsLocked = true;
	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4184F1;
}

ASMJIT_PATCH(0x418506, AircraftClass_Mission_Attack_Delay1B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	bool IsEmptyAmmo = pThis->Ammo == 0;
	bool IsNegativeAmmo = pThis->Ammo < 0;
	pThis->IsLocked = true;
	pThis->MissionStatus =  !IsNegativeAmmo && !IsEmptyAmmo ? 1 : 10;
	R->EAX(GetDelay(pThis, false));

	return 0x418539;
}

ASMJIT_PATCH(0x418883, AircraftClass_Mission_Attack_Delay2, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4188A1;
}

ASMJIT_PATCH(0x418992, AircraftClass_Mission_Attack_Delay3, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget4_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4189B0;
}

ASMJIT_PATCH(0x418AA1, AircraftClass_Mission_Attack_Delay4, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget5_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x418ABF;
}

ASMJIT_PATCH(0x418B8A, AircraftClass_Mission_Attack_Delay5, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	R->EAX(GetDelay(pThis, true));

	return 0x418BBA;
}

ASMJIT_PATCH(0x414F21, AircraftClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0xC));

	pAnim->AnimClass::AnimClass(pThis->Type->Trailer, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x414F47;
}

enum class AirAttackStatusP : int
{
	AIR_ATT_VALIDATE_AZ = 0x0,
	AIR_ATT_PICK_ATTACK_LOCATION = 0x1,
	AIR_ATT_TAKE_OFF = 0x2,
	AIR_ATT_FLY_TO_POSITION = 0x3,
	AIR_ATT_FIRE_AT_TARGET0 = 0x4,
	AIR_ATT_FIRE_AT_TARGET1 = 0x5,
	AIR_ATT_FIRE_AT_TARGET2 = 0x6,
	AIR_ATT_FIRE_AT_TARGET3 = 0x7,
	AIR_ATT_FIRE_AT_TARGET4 = 0x8,
	AIR_ATT_FIRE_AT_TARGET5 = 0x9,
	AIR_ATT_RETURN_TO_BASE = 0xA,
};

ASMJIT_PATCH(0x4CF68D, FlyLocomotionClass_DrawMatrix_OnAirport, 0x5)
{
	GET(ILocomotion*, iloco, ESI);
	auto loco = static_cast<FlyLocomotionClass*>(iloco);
	auto pAir = cast_to<AircraftClass* , false>(loco->LinkedTo);

	if (pAir && pAir->GetHeight() <= 0)
	{
		float ars = pAir->AngleRotatedSideways;
		float arf = pAir->AngleRotatedForwards;
		REF_STACK(Matrix3D, mat, STACK_OFFSET(0x38, -0x30));
		auto slope_idx = MapClass::Instance->GetCellAt(pAir->Location)->SlopeIndex;
		mat = Game::VoxelRampMatrix[slope_idx] * mat;

		if (Math::abs(ars) > 0.005 || Math::abs(arf) > 0.005)
		{
			mat.TranslateZ(float(Math::abs(Math::sin(ars))
				* pAir->Type->VoxelScaleX
				+ Math::abs(Math::sin(arf)) * pAir->Type->VoxelScaleY));

			R->ECX(pAir);
			return 0x4CF6AD;
		}
	}

	return 0;
}

long __stdcall AircraftClass_IFlyControl_IsStrafe(IFlyControl* ifly)
{
	auto pThis = static_cast<AircraftClass*>(ifly);
	WeaponTypeClass* pWeapon = nullptr;
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
	else if (pThis->Target)
		pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target))->WeaponType;

	if (pWeapon)
		return (long)AircraftCanStrafeWithWeapon(pWeapon);

	return false;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2268, AircraftClass_IFlyControl_IsStrafe);

static FORCEDINLINE bool CheckSpyPlaneCameraCount(AircraftClass* pThis ,WeaponTypeClass* pWeapon)
{
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!pWeaponExt->Strafing_Shots.isset())
		return true;

	if (pExt->ShootCount >= pWeaponExt->Strafing_Shots)
		return false;

	pExt->ShootCount++;
	return true;
}

ASMJIT_PATCH(0x41564C, AircraftClass_Mission_SpyPlaneApproach_MaxCount, 0x6) {
	GET(AircraftClass*, pThis, ESI);
	GET(int, range, EBX);

	const auto pPrimary = pThis->GetWeapon(0);

	if (range <= pPrimary->WeaponType->Range.value ) {

		if (!CheckSpyPlaneCameraCount(pThis, pPrimary->WeaponType))
			return 0x41570C;

		pThis->vt_entry_48C(nullptr ,0u,false , nullptr);
		pThis->UpdateSight(false , 0 , false , nullptr , pPrimary->WeaponType->Damage);

		MapRevealer const revealer(pThis->Location);
		revealer.UpdateShroud(0u, static_cast<size_t>(MaxImpl(pThis->LastSightRange + 3, 0)), false);

		auto cameraSound = TechnoTypeExtContainer::Instance.Find(pThis->Type)
				->SpyplaneCameraSound.Get(RulesClass::Instance->SpyPlaneCamera);

		VocClass::PlayAt(cameraSound, pThis->Location);
	}

	return 0x415700;
}

ASMJIT_PATCH(0x4157D3, AircraftClass_Mission_SpyPlaneOverfly_MaxCount, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(int, range, EAX);

	R->EDI(range);

	const auto pPrimary = pThis->GetWeapon(0);

	if (range <= pPrimary->WeaponType->Range.value) {

		if (!CheckSpyPlaneCameraCount(pThis, pPrimary->WeaponType))
			return 0x415863;

		pThis->vt_entry_48C(nullptr, 0u, false, nullptr);
		pThis->UpdateSight(false, 0, false, nullptr, pPrimary->WeaponType->Damage);

		MapRevealer const revealer(pThis->Location);
		revealer.UpdateShroud(0u, static_cast<size_t>(MaxImpl(pThis->LastSightRange + 3, 0)), false);
	}

	return 0x415859;
}

// AreaGuard: return when no ammo or first target died

ASMJIT_PATCH(0x41A96C, AircraftClass_Mission_AreaGuard, 0x6)
{
	enum { SkipGameCode = 0x41A97A };

	GET(AircraftClass*, pThis, ESI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->MyFighterData) {
		pExt->MyFighterData->StartAreaGuard();
		return SkipGameCode;
	}

	if (RulesExtData::Instance()->ExpandAircraftMission && !pThis->Team && pThis->Ammo && pThis->IsArmed())
	{
		CoordStruct coords = pThis->GetCoords();

		if (pThis->TargetAndEstimateDamage(&coords, ThreatType::Normal))
		{
			pThis->QueueMission(Mission::Attack, false);
			return SkipGameCode;
		}
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x41A982, AircraftClass_Mission_AreaGuard, 0x6)

// AttackMove: return when no ammo or arrived destination
#include <Ext/AircraftTypeClass/Body.h>

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E290C,  FakeAircraftTypeClass::_CanAttackMove)

ASMJIT_PATCH(0x4DF3BA, FootClass_UpdateAttackMove_AircraftHoldAttackMoveTarget, 0x6)
{
	enum { LoseCurrentTarget = 0x4DF3D3, HoldCurrentTarget = 0x4DF4AB };

	GET(FootClass* const, pThis, ESI);

	return (RulesExtData::Instance()->ExpandAircraftMission && pThis->WhatAmI() == AbstractType::Aircraft
		|| pThis->IsCloseEnoughToAttackWithNeverUseWeapon(pThis->Target)) ? HoldCurrentTarget : LoseCurrentTarget; // pThis->InAuxiliarySearchRange(pThis->Target)
}

ASMJIT_PATCH(0x416A0A, AircraftClass_Mission_Move_SmoothMoving, 0x5)
{
	enum { EnterIdleAndReturn = 0x416AC0, ContinueMoving1 = 0x416908, ContinueMoving2 = 0x416A47 };

	GET(AircraftClass* const, pThis, ESI);
	GET(CoordStruct* const, pCoords, EAX);

	if (!RulesExtData::Instance()->ExpandAircraftMission)
		return 0;

	const auto pType = pThis->Type;

	if (!pType->AirportBound || pThis->Airstrike || pThis->Spawned)
		return 0;

	const int distance = int(Point2D { pCoords->X, pCoords->Y }.DistanceFrom(Point2D { pThis->Location.X, pThis->Location.Y }));

	if (distance > MaxImpl((pType->SlowdownDistance >> 1), (2048 / pType->ROT)))
		return (R->Origin() == 0x4168C7 ? ContinueMoving1 : ContinueMoving2);

	if (!pThis->planing_6385C0())
		pThis->EnterIdleMode(false, true);

	return EnterIdleAndReturn;
}ASMJIT_PATCH_AGAIN(0x4168C7, AircraftClass_Mission_Move_SmoothMoving, 0x5)

ASMJIT_PATCH(0x4DDD66, FootClass_IsLZClear_ReplaceHardcode, 0x6) // To avoid that the aircraft cannot fly towards the water surface normally
{
	enum { SkipGameCode = 0x4DDD8A };

	GET(FootClass* const, pThis, EBP);
	GET_STACK(CellStruct, cell, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();

	// In vanilla, only aircrafts or `foots with fly locomotion` will call this virtual function
	// So I don't know why WW use hard-coded `SpeedType::Track` and `MovementZone::Normal` to check this
	R->AL(MapClass::Instance->GetCellAt(cell)->IsClearToMove(pType->SpeedType, false, false, ZoneType::None, pType->MovementZone, -1, true));
	return SkipGameCode;
}

ASMJIT_PATCH(0x418CD1, AircraftClass_Mission_Attack_ContinueFlyToDestination, 0x6)
{
	enum { Continue = 0x418C43, Return = 0x418CE8 };

	GET(AircraftClass* const, pThis, ESI);

	if (!pThis->Target)
	{
		if (!RulesExtData::Instance()->ExpandAircraftMission
			|| !pThis->MegaMissionIsAttackMove()
			|| !pThis->MegaDestination) // (!pThis->MegaMissionIsAttackMove() || !pThis->MegaDestination)
			return Continue;

		pThis->SetDestination(reinterpret_cast<AbstractClass*>(pThis->MegaDestination), false); // pThis->MegaDestination
		pThis->QueueMission(Mission::Move, true);
		pThis->QueueMission(Mission::Move, true);
		pThis->HaveAttackMoveTarget = false; // pThis->HaveAttackMoveTarget
	}
	else
	{
		pThis->MissionStatus = 1;
	}

	R->EAX(1);
	return Return;
}

// Idle: clear the target if no ammo
ASMJIT_PATCH(0x414D36, AircraftClass_Update_ClearTargetIfNoAmmo, 0x6)
{
	enum { ClearTarget = 0x414D3F };

	GET(AircraftClass* const, pThis, ESI);

	if (RulesExtData::Instance()->ExpandAircraftMission) {
		if (!pThis->Spawned && !pThis->Airstrike &&
			!pThis->Ammo && !SessionClass::IsCampaign()) {

			if (TeamClass* const pTeam = pThis->Team)
				pTeam->LiberateMember(pThis);

			return ClearTarget;
		}
	}

	return 0x414D4D; //AircraftClass_Update_DontloseTargetInAir
}

// GreatestThreat: for all the mission that should let the aircraft auto select a target
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2668, FakeAircraftClass::_GreatestThreat);

// Handle assigning area guard mission to aircraft.
ASMJIT_PATCH(0x4C7403, EventClass_Execute_AircraftAreaGuard, 0x6)
{
	enum { SkipGameCode = 0x4C7435 };

	GET(TechnoClass* const, pTechno, EDI);

	if (RulesExtData::Instance()->ExpandAircraftMission && pTechno->WhatAmI() == AbstractType::Aircraft)
	{
		// If we're on dock reloading but have ammo, untether from dock and try to scan for targets.
		if (pTechno->CurrentMission == Mission::Sleep && pTechno->Ammo)
			pTechno->SendToEachLink(RadioCommand::NotifyUnlink);

		// Skip assigning destination / target here.
		return SkipGameCode;
	}

	return 0;
}

// Do not untether aircraft when assigning area guard mission by default.
ASMJIT_PATCH(0x4C72F2, EventClass_Execute__AircraftAreaGuard_Untether, 0x6)
{
	enum { SkipGameCode = 0x4C7349 };

	GET(EventClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);

	if (RulesExtData::Instance()->ExpandAircraftMission && pTechno->WhatAmI() == AbstractType::Aircraft
		&& pThis->Data.MegaMission.Mission == (char)Mission::Area_Guard)
	{
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x4CF408, FlyLocomotionClass_FlightUpdate_SetFlightLevel, 0x6) // Make aircraft not have to fly directly above the airport before starting to descend
{
	enum { SkipGameCode = 0x4CF40E };

	GET(FlyLocomotionClass* const, pThis, EBP);
	GET(TechnoTypeClass* const, pType, EAX);

	R->ECX(RulesExtData::Instance()->ExpandAircraftMission && pThis->LinkedTo->CurrentMission == Mission::Enter || pType->IsDropship);
	return SkipGameCode;
}