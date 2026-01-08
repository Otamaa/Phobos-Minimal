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
#include <Ext/AircraftType/Body.h>

#include <Misc/MapRevealer.h>

#include <Misc/Hooks.Otamaa.h>

#include <Locomotor/FlyLocomotionClass.h>

#include <EventClass.h>

// Idle: should not crash immediately

ASMJIT_PATCH(0x4179F7, AircraftClass_EnterIdleMode_NoCrash, 0x6)
{
	enum { SkipGameCode = 0x417B69 };

	GET(AircraftClass* const, pThis, ESI);

	if (pThis->Airstrike || pThis->Spawned)
		return 0;

	if (AircraftTypeExtContainer::Instance.Find(pThis->Type)->ExtendedAircraftMissions_UnlandDamage
			.Get(RulesExtData::Instance()->ExtendedAircraftMissions_UnlandDamage) < 0)
		return 0;

	if (!pThis->Team && (pThis->CurrentMission != Mission::Area_Guard || !pThis->ArchiveTarget))
	{
		const auto pCell = pThis->GoodLandingZone_();
		pThis->SetDestination(pCell, true);
		pThis->SetArchiveTarget(pCell);
		pThis->QueueMission(Mission::Area_Guard, true);
	}
	else if (!pThis->Destination)
	{
		const auto pCell =  pThis->GoodLandingZone_();
		pThis->SetDestination(pCell, true);
	}

	return SkipGameCode;
}ASMJIT_PATCH_AGAIN(0x417B82, AircraftClass_EnterIdleMode_NoCrash, 0x6)

ASMJIT_PATCH(0x4DF42A, FootClass_UpdateAttackMove_AircraftHoldAttackMoveTarget2, 0x6) // When it have MegaTarget
{
	enum { ContinueCheck = 0x4DF462, HoldTarget = 0x4DF4AB };

	GET(FootClass* const, pThis, ESI);

	// Although if the target selected by CS is an object rather than cell.
	return (RulesExtData::Instance()->ExpandAircraftMission && pThis->WhatAmI() == AbstractType::Aircraft) ? HoldTarget : ContinueCheck;
}

ASMJIT_PATCH(0x41A5C7, AircraftClass_Mission_Guard_StartAreaGuard, 0x6)
{
	enum { SkipGameCode = 0x41A6AC };

	GET(AircraftClass* const, pThis, ESI);

	if (!RulesExtData::Instance()->ExpandAircraftMission || pThis->Team || !pThis->IsArmed() || pThis->Airstrike || pThis->Spawned)
		return 0;

	const auto pArchive = pThis->ArchiveTarget;

	if (!pArchive || !pThis->Ammo)
		return 0;

	pThis->SetDestination(pArchive, true);
	pThis->QueueMission(Mission::Area_Guard, false);
	return SkipGameCode;
}


ASMJIT_PATCH(0x4CE42A, FlyLocomotionClass_StateUpdate_NoLanding, 0x6) // Prevent aircraft from hovering due to cyclic enter Guard and AreaGuard missions when above buildings
{
	enum { SkipGameCode = 0x4CE441 };

	GET(FootClass* const, pLinkTo, EAX);

	if (!RulesExtData::Instance()->ExpandAircraftMission)
		return 0;

	const auto pAircraft = cast_to<AircraftClass*, true>(pLinkTo);

	if (!pAircraft || pAircraft->Airstrike || pAircraft->Spawned || pAircraft->GetCurrentMission() == Mission::Enter)
		return 0;

	return SkipGameCode;
}

// Skip duplicated aircraft check
DEFINE_PATCH(0x4CF033, 0x8B, 0x06, 0xEB, 0x18); // mov eax, [esi] ; jmp short loc_4CF04F ;
DEFINE_JUMP(LJMP, 0x4179E2, 0x417B44);

int __fastcall AircraftClass_MI_Attack_SelectWeapon_BeforeFiring(AircraftClass* pThis, discard_t, AbstractClass* pTarget)
{
	auto pExt = AircraftExtContainer::Instance.Find(pThis);

	// Re-evaluate weapon selection only if not mid-strafing run before firing.
	if (!pExt->Strafe_BombsDroppedThisRound)
		pExt->CurrentAircraftWeaponIndex = MaxImpl(pThis->SelectWeapon(pTarget), 0);


	return pExt->CurrentAircraftWeaponIndex;
}

DEFINE_FUNCTION_JUMP(CALL6, 0x41831E, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);
DEFINE_FUNCTION_JUMP(CALL6, 0x4185F5, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);
DEFINE_FUNCTION_JUMP(CALL6, 0x4187C4, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);
DEFINE_FUNCTION_JUMP(CALL6, 0x4188D3, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);
DEFINE_FUNCTION_JUMP(CALL6, 0x4189E2, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);
DEFINE_FUNCTION_JUMP(CALL6, 0x418AF1, AircraftClass_MI_Attack_SelectWeapon_BeforeFiring);

ASMJIT_PATCH(0x418544, AircraftClass_Mission_Attack_StrafingDestinationFix, 0x6)
{
	GET(FireError, fireError, EAX);
	GET(AircraftClass*, pThis, ESI);

	// The aircraft managed by the spawn manager will not update destination after changing target
	if (fireError == FireError::RANGE && pThis->Is_Strafe())
		pThis->SetDestination(pThis->Target, true);

	return 0;

}ASMJIT_PATCH_AGAIN(0x41874E, AircraftClass_Mission_Attack_StrafingDestinationFix, 0x6)

ASMJIT_PATCH(0x4180F4, AircraftClass_MI_Attack_WeaponRange, 0x5)
{
	enum { SkipGameCode = 0x4180FF };

	GET(AircraftClass*, pThis, ESI);

	R->EAX(pThis->GetWeapon(AircraftExtContainer::Instance.Find(pThis)->CurrentAircraftWeaponIndex));
	return SkipGameCode;
}

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

	auto const pExt = AircraftExtContainer::Instance.Find(pThis);
	auto const fireError = pThis->GetFireError(pTarget, pExt->CurrentAircraftWeaponIndex, false);

	if (fireError != FireError::OK)
		return 0;

	R->EAX(MapClass::Instance->GetCellAt(pObject->GetCoords()));

	return 0;
}

ASMJIT_PATCH(0x4197FC, AircraftClass_MI_Attack_GoodFireLoc_Range, 0x6)
{
	GET(AircraftClass*, pThis, EDI);
	R->EAX(pThis->GetWeaponRange(AircraftExtContainer::Instance.Find(pThis)->CurrentAircraftWeaponIndex));
	return 0x419808;
}

#ifndef SecondMode
ASMJIT_PATCH(0x417FF1, AircraftClass_MI_Attack_StrafeShots, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);

	AirAttackStatus const state = (AirAttackStatus)pThis->MissionStatus;
	auto pExt = AircraftExtContainer::Instance.Find(pThis);

	// Re-evaluate weapon choice due to potentially changing targeting conditions here
	// only when aircraft is adjusting position or picking attack location.
	// This choice is also re-evaluated again every time just before firing UNLESS mid strafing run.
	// See AircraftClass_SelectWeapon_Wrapper and the call redirects below for this.
	if (state > AirAttackStatus::ValidateAZ && state < AirAttackStatus::FireAtTarget)
		pExt->CurrentAircraftWeaponIndex = MaxImpl(pThis->SelectWeapon(pThis->Target), 0);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe
		)
	{
		pExt->Strafe_BombsDroppedThisRound = 0;
	}

	// No need to evaluate this before any strafing shots have been fired.
	if (pExt->Strafe_BombsDroppedThisRound){
		auto const pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		int count = pWeaponExt->Strafing_Shots.Get(5);

		if (count > 5) {
			if (pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget3_Strafe) {
				if ((count - 3 - pExt->Strafe_BombsDroppedThisRound) > 0) {
					pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
				}
			}
		}
	}

	return 0;
}

#include <Ext/BulletType/Body.h>

COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon)
{
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1
			&& !pWeapon->Projectile->Inviso)
			&& !BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->TrajectoryType;
}

bool FireWeapon(AircraftClass* pAir, AbstractClass* pTarget)
{
	const auto pExt = AircraftExtContainer::Instance.Find(pAir);
	const int weaponIndex = pExt->CurrentAircraftWeaponIndex;
	const bool Scatter = TechnoTypeExtContainer::Instance.Find(pAir->Type)->FiringForceScatter ;
	auto pDecideTarget = (pExt->Strafe_TargetCell ? pExt->Strafe_TargetCell : pTarget);

	if (const auto pWeaponStruct = pAir->GetWeapon(weaponIndex)) {
		if (const auto weaponType = pWeaponStruct->WeaponType) {
			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(weaponType);
			bool isStrafe = pAir->Is_Strafe();

			if (weaponType->Burst > 0) {
				for (int i = 0; i < weaponType->Burst; i++) {
					if (isStrafe && weaponType->Burst < 2 && pWeaponExt->Strafing_SimulateBurst)
						pAir->CurrentBurstIndex = pExt->Strafe_BombsDroppedThisRound % 2 == 0;

					pAir->Fire(pDecideTarget, weaponIndex);
				}

				if (isStrafe) {
					pExt->Strafe_BombsDroppedThisRound++;

					if (pWeaponExt->Strafing_UseAmmoPerShot) {
						pAir->Ammo--;
						pAir->loseammo_6c8 = false;

						if (!pAir->Ammo) {
							pAir->SetTarget(nullptr);
							pAir->SetDestination(nullptr, true);
						}
					}
				}
			}

		}
	}

	if (pDecideTarget) {
		if (Scatter) {

			auto coord = pDecideTarget->GetCoords();

			if (auto pCell = MapClass::Instance->TryGetCellAt(coord))
			{
				pCell->ScatterContent(coord, true, false, false);
			}
		}

		return true;
	}

	return false;
}

ASMJIT_PATCH(0x4186B6, AircraftClass_MI_Attack_FireAt_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	FireWeapon(pThis, pThis->Target);
	return 0x418720;
}

ASMJIT_PATCH(0x418805, AircraftClass_MI_Attack_Strafe2_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireWeapon(pThis, pThis->Target) ? 0x418883 : 0x418870;
}

ASMJIT_PATCH(0x418914, AircraftClass_MI_Attack_Strafe3_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireWeapon(pThis, pThis->Target) ? 0x418992 : 0x41897F;
}

ASMJIT_PATCH(0x418A23, AircraftClass_MI_Attack_Strafe4_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !FireWeapon(pThis, pThis->Target) ? 0x418AA1 : 0x418A8E;
}

ASMJIT_PATCH(0x418B1F, AircraftClass_MI_Attack_Strafe5_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	FireWeapon(pThis, pThis->Target);
	return 0x418B8A;
}

ASMJIT_PATCH(0x418403, AircraftClass_MI_Attack_FireAtTarget_BurstFix, 0x6) //8
{
	GET(AircraftClass*, pThis, ESI);

	pThis->loseammo_6c8 = true;

	FireWeapon(pThis, pThis->Target);

	return 0x4184C2;
}

#undef Hook_AircraftBurstFix
#endif

static int GetDelay(AircraftClass* pThis, bool isLastShot)
{
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);
	auto const pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int delay = pWeapon->ROF;

	if (isLastShot || pExt->Strafe_BombsDroppedThisRound == pWeaponExt->Strafing_Shots.Get(5) || (pWeaponExt->Strafing_UseAmmoPerShot && !pThis->Ammo))
	{
		pExt->Strafe_TargetCell = nullptr;
		pThis->MissionStatus = (int)AirAttackStatus::FlyToPosition;
		delay = pWeaponExt->Strafing_EndDelay.Get((pWeapon->Range + 1024) / pThis->Type->Speed);
	}

	return delay;
}

ASMJIT_PATCH(0x4184CC, AircraftClass_MI_Attack_Delay1A, 0x6)
{
	GET(FakeAircraftClass*, pThis, ESI);

	auto pExt = pThis->_GetExtData();
	if (WeaponTypeExtContainer::Instance.Find(pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType)->Strafing_TargetCell)
		pExt->Strafe_TargetCell = MapClass::Instance->GetCellAt(pThis->Target->GetCoords());

	pThis->IsLocked = true;
	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4184F1;
}

ASMJIT_PATCH(0x418506, AircraftClass_MI_Attack_Delay1B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	bool IsEmptyAmmo = pThis->Ammo == 0;
	bool IsNegativeAmmo = pThis->Ammo < 0;
	pThis->IsLocked = true;
	pThis->MissionStatus =  !IsNegativeAmmo && !IsEmptyAmmo ? 1 : 10;
	R->EAX(GetDelay(pThis, false));

	return 0x418539;
}

ASMJIT_PATCH(0x418883, AircraftClass_MI_Attack_Delay2, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4188A1;
}

ASMJIT_PATCH(0x418992, AircraftClass_MI_Attack_Delay3, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget4_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x4189B0;
}

ASMJIT_PATCH(0x418AA1, AircraftClass_MI_Attack_Delay4, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget5_Strafe;
	R->EAX(GetDelay(pThis, false));

	return 0x418ABF;
}

ASMJIT_PATCH(0x418B8A, AircraftClass_MI_Attack_Delay5, 0x6)
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
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false, false);

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

		return 0x4CF6A0;
	}

	return 0;
}

long __stdcall AircraftClass_IFlyControl_IsStrafe(IFlyControl* ifly)
{
	auto pThis = static_cast<AircraftClass*>(ifly);
	WeaponTypeClass* pWeapon = nullptr;
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);

	pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;

	if (pWeapon)
		return (long)AircraftCanStrafeWithWeapon(pWeapon);

	return false;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2268, AircraftClass_IFlyControl_IsStrafe);

static FORCEDINLINE bool CheckSpyPlaneCameraCount(AircraftClass* pThis ,WeaponTypeClass* pWeapon)
{
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!pWeaponExt->Strafing_Shots.isset())
		return true;

	if (pExt->Strafe_BombsDroppedThisRound >= pWeaponExt->Strafing_Shots)
		return false;

	pExt->Strafe_BombsDroppedThisRound++;
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

		VocClass::SafeImmedietelyPlayAt(cameraSound, &pThis->Location);
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
static inline int GetTurningRadius(AircraftClass* pThis)
{
	constexpr double epsilon = 1e-10;
	constexpr double raw2Radian = Math::GAME_TWOPI / 65536;
	// GetRadian<65536>() is an incorrect method
	const double rotRadian = Math::abs(static_cast<double>(pThis->PrimaryFacing.ROT.Raw) * raw2Radian);
	return rotRadian > epsilon ? static_cast<int>(static_cast<double>(pThis->Type->Speed) / rotRadian) : 0;
}

ASMJIT_PATCH(0x41A96C, AircraftClass_Mission_AreaGuard, 0x6)
{
	enum { SkipGameCode = 0x41A97A };

	GET(AircraftClass*, pThis, ESI);

	if (RulesExtData::Instance()->ExpandAircraftMission && !pThis->Team && pThis->Ammo && pThis->IsArmed())
	{
		auto enterIdleMode = [pThis]() -> bool
			{
				// Avoid duplicate checks in Update
				if (pThis->MissionStatus)
					return false;

				pThis->EnterIdleMode(false, true);

				if (pThis->DockedTo)
					return true;

				// Hovering state without any airport
				pThis->MissionStatus = 1;
				return false;
			};
		auto hoverOverArchive = [pThis](const CoordStruct& coords, AbstractClass* pDest)
			{
				const auto& location = pThis->Location;
				const int turningRadius = GetTurningRadius(pThis);
				auto length_ = Point2D(coords.X, coords.Y).DistanceFrom({ location.X, location.Y });
				const double distance = MaxImpl(1.0, length_);

				// Random hovering direction
				const double ratio = (((pThis->LastFireBulletFrame + pThis->UniqueID) & 1) ? turningRadius : -turningRadius) / distance;

				// Fly sideways towards the target, and extend the distance to ensure no deceleration
				const CoordStruct destination
				{
					(static_cast<int>(coords.X - ratio * (location.Y - coords.Y)) - location.X) * 4 + location.X,
					(static_cast<int>(coords.Y + ratio * (location.X - coords.X)) - location.Y) * 4 + location.Y,
					coords.Z
				};

				pThis->Locomotor->Move_To(destination);
				pThis->IsLocked = distance < turningRadius;
			};

		if (const auto pArchive = pThis->ArchiveTarget)
		{
			if (pThis->Ammo)
			{
				auto coords = pArchive->GetCoords();

				if (!pThis->TargetingTimer.HasTimeLeft() && pThis->TargetAndEstimateDamage(&coords, ThreatType::Area)) {
					// Without an airport, there is no need to record the previous location
					if (pThis->MissionStatus)
						pThis->SetArchiveTarget(nullptr);

					pThis->QueueMission(Mission::Attack, false);
				} else {
					// Check dock building
					if (!pThis->MissionStatus && !pThis->FindDockingBayInVector(reinterpret_cast<DynamicVectorClass<TechnoTypeClass*>*>(&pThis->Type->Dock), 0, 0))
						pThis->MissionStatus = 1;

					hoverOverArchive(coords, pArchive);
				}
			}
			else if (!enterIdleMode() && pThis->IsAlive)
			{
				// continue circling
				hoverOverArchive(pArchive->GetCoords(), pArchive);
			}
		}
		else if (!pThis->Destination)
		{
			enterIdleMode();
		}

		R->EAX(1);
		return SkipGameCode;
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x41A982, AircraftClass_Mission_AreaGuard, 0x6)

// AttackMove: return when no ammo or arrived destination
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

	const auto pType = pThis->Type;

	if (pThis->Team || pThis->Airstrike || pThis->Spawned || !pType->AirportBound)
		return 0;

	const auto extendedMissions = RulesExtData::Instance()->ExpandAircraftMission;
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->ExtendedAircraftMissions_SmoothMoving.Get(extendedMissions))
		return 0;

	const auto rotRadian = Math::abs(pThis->PrimaryFacing.ROT.Raw * (Math::GAME_TWOPI / 65536)); // GetRadian<65536>() is an incorrect method
	const auto turningRadius = rotRadian > 1e-10 ? static_cast<int>(pType->Speed / rotRadian) : 0;
	const int distance = int(Point2D { pCoords->X, pCoords->Y }.DistanceFrom(Point2D { pThis->Location.X, pThis->Location.Y }));

	if (distance > MaxImpl((pType->SlowdownDistance / 2), turningRadius))
		return (R->Origin() == 0x4168C7 ? ContinueMoving1 : ContinueMoving2);

	if (!extendedMissions || !pThis->TryNextPlanningTokenNode())
		pThis->EnterIdleMode(false, true);

	return EnterIdleAndReturn;
}ASMJIT_PATCH_AGAIN(0x4168C7, AircraftClass_Mission_Move_SmoothMoving, 0x5)

 ASMJIT_PATCH(0x4DDD66, FootClass_IsLZClear_ReplaceHardcode, 0x6) // To avoid that the aircraft cannot fly towards the water surface normally
 {
 	enum { SkipGameCode = 0x4DDD8A };

 	GET(FootClass* const, pThis, EBP);
 	GET_STACK(CellStruct, cell, STACK_OFFSET(0x20, 0x4));

 	const auto pType = GET_TECHNOTYPE(pThis);

 	// In vanilla, only aircrafts or `foots with fly locomotion` will call this virtual function
 	// So I don't know why WW use hard-coded `SpeedType::Track` and `MovementZone::Normal` to check this
 	R->AL(MapClass::Instance->GetCellAt(cell)->IsClearToMove(pType->SpeedType, false, false, ZoneType::None, pType->MovementZone, -1, true));
 	return SkipGameCode;
 }

ASMJIT_PATCH(0x418CD1, AircraftClass_MI_Attack_ContinueFlyToDestination, 0x6)
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
ASMJIT_PATCH(0x414D36, AircraftClass_Update_ClearTarget, 0x6)
{
	enum { ClearTarget = 0x414D3F, DonotClearTarget = 0x414D4D };
	//GET(AircraftClass* const, pThis, ESI);

	// if (RulesExtData::Instance()->ExpandAircraftMission) {
	// 	if (!pThis->Spawned && !pThis->Airstrike &&
	// 		!pThis->Ammo && !SessionClass::IsCampaign()) {
	//
	// 		if (TeamClass* const pTeam = pThis->Team)
	// 			pTeam->LiberateMember(pThis);
	//
	// 		return ClearTarget;
	// 	}
	// }

	return DonotClearTarget; //AircraftClass_Update_DontloseTargetInAir
}

// GreatestThreat: for all the mission that should let the aircraft auto select a target
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2668, FakeAircraftClass::_GreatestThreat);

// Sleep: return to airbase if in incorrect sleep status

int FakeAircraftClass::_Mission_Sleep()
{
	if (!this->Destination || this->Destination == this->DockedTo)
		return 450; // Vanilla MissionClass_Mission_Sleep value

	this->EnterIdleMode(false, true);
	return 1;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E24A8, FakeAircraftClass::_Mission_Sleep)

// Handle assigning area guard mission to aircraft.
ASMJIT_PATCH(0x4C7403, EventClass_Execute_AircraftAreaGuard, 0x6)
{
	enum { SkipGameCode = 0x4C7435 };

	GET(EventClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);

	if (RulesExtData::Instance()->ExpandAircraftMission
			&& pTechno->WhatAmI() == AbstractType::Aircraft)
	{
		// Skip assigning destination / target here.
		R->ESI(&pThis->Data.MegaMission.Target);
		return 0x4C7426 ;
	}

	return 0;
}

// Do not untether aircraft when assigning area guard mission by default.
ASMJIT_PATCH(0x4C72F2, EventClass_Execute__AircraftAreaGuard_Untether, 0x6)
{
	enum { SkipGameCode = 0x4C7349 };

	GET(EventClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);

	if (RulesExtData::Instance()->ExpandAircraftMission
		&& pTechno->WhatAmI() == AbstractType::Aircraft
		&& pThis->Data.MegaMission.Mission == (char)Mission::Area_Guard
		&& (pTechno->CurrentMission != Mission::Sleep || !pTechno->Ammo)
		)
	{
		// If we're on dock reloading but have ammo, untether from dock and try to scan for targets.
		return SkipGameCode;
	}

	return 0;
}

#include <Locomotor/RocketLocomotionClass.h>
#include <Locomotor/Cast.h>

ASMJIT_PATCH(0x66295A, RocketLocomotionClass_Process_IsHighEnoughForCruise, 0x8)
{
	GET(AircraftClass*, pLinkedTo, ECX);
	GET(ILocomotion*, pThis, ESI);

	auto pLoco = locomotion_cast<RocketLocomotionClass*>(pThis);
	auto heightThis = pLinkedTo->GetHeight();
	auto heightTarget = pLinkedTo->Location.Z - pLoco->MovingDestination.Z;

	if (MapClass::Instance->GetCellAt(pLoco->MovingDestination)->ContainsBridge())
		heightTarget -= CellClass::BridgeHeight;

	R->EAX(MinImpl(heightThis, heightTarget));
	//R->EAX(pLinkedTo->GetHeight()); Vanilla behavior

	return R->Origin() + 0x8;
}