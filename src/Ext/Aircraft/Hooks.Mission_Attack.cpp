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

#ifndef MI_ATTACK_HOOKS

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
	if (pExt->Strafe_BombsDroppedThisRound)
	{
		auto const pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
		auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

		int count = pWeaponExt->Strafing_Shots.Get(5);

		if (count > 5)
		{
			if (pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget3_Strafe)
			{
				if ((count - 3 - pExt->Strafe_BombsDroppedThisRound) > 0)
				{
					pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
				}
			}
		}
	}

	return 0;
}

#include <Ext/BulletType/Body.h>

ASMJIT_PATCH(0x4186B6, AircraftClass_MI_Attack_FireAt_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	AircraftExtData::FireWeapon(pThis, pThis->Target);
	return 0x418720;
}

ASMJIT_PATCH(0x418805, AircraftClass_MI_Attack_Strafe2_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !AircraftExtData::FireWeapon(pThis, pThis->Target) ? 0x418883 : 0x418870;
}

ASMJIT_PATCH(0x418914, AircraftClass_MI_Attack_Strafe3_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !AircraftExtData::FireWeapon(pThis, pThis->Target) ? 0x418992 : 0x41897F;
}

ASMJIT_PATCH(0x418A23, AircraftClass_MI_Attack_Strafe4_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	return !AircraftExtData::FireWeapon(pThis, pThis->Target) ? 0x418AA1 : 0x418A8E;
}

ASMJIT_PATCH(0x418B1F, AircraftClass_MI_Attack_Strafe5_Strafe_BurstFix, 0x6)
{
	GET(AircraftClass* const, pThis, ESI);
	AircraftExtData::FireWeapon(pThis, pThis->Target);
	return 0x418B8A;
}

ASMJIT_PATCH(0x418403, AircraftClass_MI_Attack_FireAtTarget_BurstFix, 0x6) //8
{
	GET(AircraftClass*, pThis, ESI);

	pThis->loseammo_6c8 = true;

	AircraftExtData::FireWeapon(pThis, pThis->Target);

	return 0x4184C2;
}

#undef Hook_AircraftBurstFix
#endif


ASMJIT_PATCH(0x4184CC, AircraftClass_MI_Attack_Delay1A, 0x6)
{
	GET(FakeAircraftClass*, pThis, ESI);

	auto pExt = pThis->_GetExtData();
	if (WeaponTypeExtContainer::Instance.Find(pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType)->Strafing_TargetCell)
		pExt->Strafe_TargetCell = MapClass::Instance->GetCellAt(pThis->Target->GetCoords());

	pThis->IsLocked = true;
	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget2_Strafe;
	R->EAX(AircraftExtData::GetDelay(pThis, false));

	return 0x4184F1;
}

ASMJIT_PATCH(0x418506, AircraftClass_MI_Attack_Delay1B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	bool IsEmptyAmmo = pThis->Ammo == 0;
	bool IsNegativeAmmo = pThis->Ammo < 0;
	pThis->IsLocked = true;
	pThis->MissionStatus = !IsNegativeAmmo && !IsEmptyAmmo ? 1 : 10;
	R->EAX(AircraftExtData::GetDelay(pThis, false));

	return 0x418539;
}

ASMJIT_PATCH(0x418883, AircraftClass_MI_Attack_Delay2, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;
	R->EAX(AircraftExtData::GetDelay(pThis, false));

	return 0x4188A1;
}

ASMJIT_PATCH(0x418992, AircraftClass_MI_Attack_Delay3, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget4_Strafe;
	R->EAX(AircraftExtData::GetDelay(pThis, false));

	return 0x4189B0;
}

ASMJIT_PATCH(0x418AA1, AircraftClass_MI_Attack_Delay4, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget5_Strafe;
	R->EAX(AircraftExtData::GetDelay(pThis, false));

	return 0x418ABF;
}

ASMJIT_PATCH(0x418B8A, AircraftClass_MI_Attack_Delay5, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	R->EAX(AircraftExtData::GetDelay(pThis, true));

	return 0x418BBA;
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

#endif
