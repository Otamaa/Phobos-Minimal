#include "Body.h"

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

#pragma region UnitsFacing

// Would it be better to rewrite the entire UpdateRotation() ?
ASMJIT_PATCH(0x7369A5, UnitClass_UpdateRotation_CheckTurnToTarget, 0x6)
{
	enum { SkipGameCode = 0x736A8E, ContinueGameCode = 0x7369B3 };

	GET(UnitClass* const, pThis, ESI);

	if (!pThis->IsRotating)
		return ContinueGameCode;

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if(auto idleAc = pExt->Get_IdleActionComponent()) {
		if (idleAc->Timer.IsTicking() || idleAc->GapTimer.IsTicking())
			return ContinueGameCode;
	}

	if(pExt->Get_TechnoStateComponent()->UnitIdleIsSelected)
		return ContinueGameCode;

	return SkipGameCode;
}

ASMJIT_PATCH(0x7369D6, UnitClass_UpdateRotation_StopUnitIdleAction, 0xA)
{
	enum { SkipGameCode = 0x736A8E };

	GET(UnitClass* const, pThis, ESI);
	GET_STACK(DirStruct, dir, STACK_OFFSET(0x10, -0x8));

	if (WeaponStruct* const pWeaponStruct = pThis->GetTurrentWeapon())
	{
		if (WeaponTypeClass* const pWeapon = pWeaponStruct->WeaponType)
		{
			auto pExt = TechnoExtContainer::Instance.Find(pThis);

			pExt->StopIdleAction();

			if (!pWeapon->OmniFire)
			{
				if (pWeaponStruct->TurretLocked)
					pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
				else
					pThis->SecondaryFacing.Set_Desired(dir);
			}
		}
	}

	return SkipGameCode;
}

#include <Ext/Unit/Body.h>

ASMJIT_PATCH(0x736AFB, UnitClass_UpdateRotation_CheckTurnToForward, 0x6)
{
	enum { SkipGameCode = 0x736BE2, ContinueGameCode = 0x736B21 };

	GET(UnitClass* const, pThis, ESI);

	// Repeatedly check TurretSpins and IsRotating() seems unnecessary
	pThis->IsRotating = true;

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (auto idleAc = pExt->Get_IdleActionComponent())
	{
		if (idleAc->Timer.IsTicking() || idleAc->GapTimer.IsTicking())
			return ContinueGameCode;
	}

	if (pExt->Get_TechnoStateComponent()->UnitIdleIsSelected)
		return ContinueGameCode;

	return SkipGameCode;
}

ASMJIT_PATCH(0x736B7E, UnitClass_UpdateRotation_ApplyUnitIdleAction, 0xA)
{
	enum { SkipGameCode = 0x736BE2 };

	GET(UnitClass* const, pThis, ESI);

	WeaponStruct* const pWeaponStruct = pThis->GetTurrentWeapon();
	auto pExt = UnitExtContainer::Instance.Find(pThis);
	const Mission currentMission = pThis->CurrentMission;
	auto pState = pExt->Get_TechnoStateComponent();

	if ((pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->TurretLocked) || (pState->IsDriverKilled))
	{
		// Vanilla TurretLocked state and driver been killed state
		pExt->StopIdleAction();

		pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
	}
	else
	{
		// Point to mouse
		if (pState->UnitIdleActionSelected && pThis->Owner->ControlledByCurrentPlayer())
			pExt->ManualIdleAction();

		if (!pState->UnitIdleIsSelected)
		{
			// Bugfix: Align jumpjet turret's facing with body's
			// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
			// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
			// that's why they will come back to normal when giving stop command explicitly
			// so the best way is to fix the Mission if necessary, but I don't know how to do it
			// so I skipped jumpjets check temporarily
			if (!pThis->Destination || locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				// Idle main
				if (pExt->Get_IdleActionComponent() && (currentMission == Mission::Guard || currentMission == Mission::Sticky))
					pExt->ApplyIdleAction();
				else if (pThis->Type->Speed) // What DisallowMoving used to skip
					pThis->SecondaryFacing.Set_Desired(pThis->PrimaryFacing.Current());
			}
			else if (pThis->Type->Speed) // What DisallowMoving used to skip
			{
				// Turn to destination
				pExt->StopIdleAction();

				pThis->SecondaryFacing.Set_Desired(pThis->GetDirectionOverObject(pThis->Destination));
			}
		}
	}

	return SkipGameCode;
}

#pragma endregion