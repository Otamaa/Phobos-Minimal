// Issue #5 Permanently stationary units
// Author: Starkku

#include "UnitClass.h"

#include <Utilities/GeneralUtils.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <PlanningTokenClass.h>

bool CannotMove(UnitClass* pThis)
{
	const auto pType = pThis->Type;

	if (pType->Speed == 0)
		return true;

	if (!pThis->IsInAir())
	{
		LandType landType = pThis->GetCell()->LandType;
		const LandType movementRestrictedTo = pType->MovementRestrictedTo;

		if (pThis->OnBridge
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			landType = LandType::Road;
		}

		if (movementRestrictedTo != LandType::None && movementRestrictedTo != landType && landType != LandType::Tunnel)
			return true;
	}

	return false;
}

ASMJIT_PATCH(0x740A93, UnitClass_Mission_Move_DisallowMoving, 0x6)
{
	enum {
		QueueGuardInstead = 0x740AEF,
		ReturnTrue = 0x740AFD,
		ContinueCheck = 0x0
	};

	GET(UnitClass*, pThis, ESI);

	return CannotMove(pThis) == 0
	? QueueGuardInstead : ContinueCheck;
}

ASMJIT_PATCH(0x741AA7, UnitClass_Assign_Destination_DisallowMoving, 0x6)
{
	enum { ClearNavComsAndReturn = 0x743173, ContinueCheck = 0x0 };
	GET(UnitClass*, pThis, EBP);

	return CannotMove(pThis) == 0
	? ClearNavComsAndReturn : ContinueCheck;
}

ASMJIT_PATCH(0x743B4B, UnitClass_Scatter_DisallowMoving, 0x6)
{
	enum { ReleaseReturn = 0x74408E, ContinueCheck = 0x0 };
	GET(UnitClass*, pThis, EBP);

	return CannotMove(pThis) == 0
	? ReleaseReturn : ContinueCheck;
}

ASMJIT_PATCH(0x74038F, UnitClass_What_Action_ObjectClass_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return CannotMove(pThis) ? 0x7403A3 : 0;
}

ASMJIT_PATCH(0x7403B7, UnitClass_What_Action_ObjectClass_DisallowMoving_2, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return CannotMove(pThis) ? 0x7403C1 : 0;
}

ASMJIT_PATCH(0x740709, UnitClass_What_Action_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return CannotMove(pThis) ? 0x740727 : 0;
}

ASMJIT_PATCH(0x740744, UnitClass_What_Action_DisallowMoving_2, 0x6)
{
	enum { AllowAttack = 0x74078E, ReturnNoMove = 0x740769, ReturnResult = 0x740801 };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(Action, result, 0x30);

	if (CannotMove(pThis))
	{
		if (result == Action::Move)
			return ReturnNoMove;
		if (result != Action::Attack)
			return ReturnResult;

		return AllowAttack;
	}

	return 0;
}

ASMJIT_PATCH(0x736B60, UnitClass_Rotation_AI_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return !TechnoTypeExtContainer::Instance.Find(pThis->Type)
			->TurretResponse.Get(pThis->Type->Speed != 0) ? 0x736AFB : 0;
}

ASMJIT_PATCH(0x74416C, UnitClass_Mission_DisallowMoving, 0x7)		//UnitClass::Mission_AreaGuard
{
	GET(UnitClass*, pThis, ESI);

	DWORD address = R->Origin();

	if (CannotMove(pThis))
	{
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();

		R->EAX(pThis->FootClass::Mission_Guard());
	}
	else if (address == 0x74416C)
	{
		R->EAX(pThis->FootClass::Mission_AreaGuard());
	}
	else
	{
		R->EAX(pThis->FootClass::Mission_Hunt());
	}

	return R->Origin() + 0x7;
}ASMJIT_PATCH_AGAIN(0x73F08A, UnitClass_Mission_DisallowMoving, 0x7)	//UnitClass::Mission_Hunt

ASMJIT_PATCH(0x74132B, UnitClass_GetFireError_DisallowMoving, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(FireError, result, EAX);

	if (result == FireError::RANGE && CannotMove(pThis))
		R->EAX(FireError::ILLEGAL);

	return 0;
}

DEFINE_HOOK(0x736E34, UnitClass_UpdateFiring_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EAX);
	GET(int, nWeaponIndex, EDI);

	const auto pType = pThis->Type;

	if (CannotMove(pThis))
	{
		if (!pThis->IsCloseEnough(pTarget, nWeaponIndex))
		{
			if (pType->IsGattling)
				pThis->GattlingRateDown(1);

			pThis->SetTarget(nullptr);
			return 0x737140;
		}
		else
		{
			R->EAX(pThis->GetFireError(pTarget, nWeaponIndex, false));
			return 0x736E40;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x741050, UnitClass_CanFire_DeployToFire, 0x6)
{
	enum { NoNeedToCheck = 0x74132B , SkipGameCode = 0x7410B7,  MustDeploy = 0x7410A8 };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->DeployToFire
		&& pThis->CanDeployNow()
		&& !TechnoExtData::CanDeployIntoBuilding(pThis, true)
		)
	{
		return MustDeploy;
	}

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (!pTypeExt->NoTurret_TrackTarget.Get(RulesExtData::Instance()->NoTurret_TrackTarget)) {
		return NoNeedToCheck;
	}
	
	return SkipGameCode;
}

ASMJIT_PATCH(0x73891D, UnitClass_Active_Click_With_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x738927 : 0;
}