// Issue #5 Permanently stationary units
// Author: Starkku
#include "Body.h"

#include <InfantryClass.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <Ext/Foot/Body.h>
#include <Ext/TechnoType/Body.h>

#include <PlanningTokenClass.h>

ASMJIT_PATCH(0x741AA7, UnitClass_Assign_Destination_DisallowMoving, 0x6)
{
	enum { ClearNavComsAndReturn = 0x743173, ContinueCheck = 0x0 };
	GET(UnitClass*, pThis, EBP);

	return  TechnoExtData::CannotMove(pThis, false)
	? ClearNavComsAndReturn : ContinueCheck;
}

ASMJIT_PATCH(0x743B4B, UnitClass_Scatter_DisallowMoving, 0x6)
{
	enum { ReleaseReturn = 0x74408E, ContinueCheck = 0x0 };
	GET(UnitClass*, pThis, EBP);

	return  TechnoExtData::CannotMove(pThis, true)
	? ReleaseReturn : ContinueCheck;
}

ASMJIT_PATCH(0x74038F, UnitClass_What_Action_ObjectClass_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return  TechnoExtData::CannotMove(pThis, false) ? 0x7403A3 : 0;
}

ASMJIT_PATCH(0x7403B7, UnitClass_What_Action_ObjectClass_DisallowMoving_2, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return  TechnoExtData::CannotMove(pThis, false) ? 0x7403C1 : 0;
}

ASMJIT_PATCH(0x740709, UnitClass_What_Action_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return  TechnoExtData::CannotMove(pThis,false) ? 0x740727 : 0;
}

ASMJIT_PATCH(0x740744, UnitClass_What_Action_DisallowMoving_2, 0x6)
{
	enum { AllowAttack = 0x74078E, ReturnNoMove = 0x740769, ReturnResult = 0x740801 };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(Action, result, 0x30);

	if (TechnoExtData::CannotMove(pThis, false))
	{
		if (result == Action::Move)
			return ReturnNoMove;
		if (result != Action::Attack)
			return ReturnResult;

		return AllowAttack;
	}

	return 0;
}

// Issue #5 Permanently stationary units
// Author: Starkku

#include <Ext/UnitType/Body.h>

#pragma region Helpers

static int inline HandleHunt(FootClass* pThis)
{
	if (TechnoExtData::CannotMove(pThis, true))
	{
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();
		return pThis->Mission_Guard();
	}

	return 0;
}

#pragma endregion

#pragma region Infantry

static bool __fastcall InfantryClass_CantMove_Wrapper(InfantryClass* pThis)
{
	return pThis->IsUnderEMP() || TechnoExtData::CannotMove(pThis, true);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x75ACBD, InfantryClass_CantMove_Wrapper);

ASMJIT_PATCH(0x51AA84, InfantryClass_AssignDestination_DisallowMoving, 0x6)
{
	GET(InfantryClass*, pThis, EBP);

	if (TechnoExtData::CannotMove(pThis, false))
	{
		pThis->AbortMotion();
		return 0x51B1D7;
	}

	return 0;
}

ASMJIT_PATCH(0x51EB94, InfantryClass_WhatAction_ObjectClass_DisallowMoving_1, 0x6)
{
	GET(InfantryClass*, pThis, EDI);

	return TechnoExtData::CannotMove(pThis, false) ? 0x51EBC6 : 0;
}

ASMJIT_PATCH(0x51EBDF, InfantryClass_WhatAction_ObjectClass_DisallowMoving_2, 0x6)
{
	GET(InfantryClass*, pThis, EDI);

	return TechnoExtData::CannotMove(pThis, false) ? 0x51EBE9 : 0;
}

ASMJIT_PATCH(0x51F8A8, InfantryClass_WhatAction_DisallowMoving, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(Action, action, EBX);

	if (TechnoExtData::CannotMove(pThis, false))
	{
		if (pThis->Owner->ControlledByCurrentPlayer() && action == Action::Attack)
			return 0x51F905;
		else if (action == Action::Move)
			return 0x51F94E;
	}

	return 0;
}

ASMJIT_PATCH(0x51F543, InfantryClass_Mission_Hunt_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (int delay = HandleHunt(pThis))
	{
		R->EAX(delay);
		return 0x51F5BE;
	}

	return 0;
}

ASMJIT_PATCH(0x51CB8C, InfantryClass_GetFireError_DisallowMoving, 0x6)
{
	GET(InfantryClass*, pThis, EBX);
	GET(const FireError, result, EAX);

	if (result == FireError::RANGE && TechnoExtData::CannotMove((FootClass*)pThis, true))
		R->EAX(FireError::ILLEGAL);

	return 0;
}

#pragma endregion

#pragma region Unit

ASMJIT_PATCH(0x741AA7, UnitClass_AssignDestination_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	return TechnoExtData::CannotMove(pThis, false) ? 0x743173 : 0;
}

ASMJIT_PATCH(0x736B60, UnitClass_RotationAI_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTypeExt = UnitTypeExtContainer::Instance.Find(pThis->Type);

	if (pTypeExt->TurretResponse.isset() ? !pTypeExt->TurretResponse.Fetch() : TechnoExtData::CannotMove(pThis, false))
		return 0x736AFB;

	return 0;
}

ASMJIT_PATCH(0x73EFC4, UnitClass_Mission_Hunt_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (int delay = HandleHunt(pThis))
	{
		R->EAX(delay);
		return 0x73F091;
	}

	return 0;
}

//ASMJIT_PATCH(0x74132B, UnitClass_GetFireError_DisallowMoving, 0x7)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET(const FireError, result, EAX);
//
//	if (result == FireError::RANGE && TechnoExtData::CannotMove(pThis, true))
//		R->EAX(FireError::ILLEGAL);
//
//	return 0;
//}

#pragma endregion

#pragma region Foot

ASMJIT_PATCH(0x4D4203, FootClass_Mission_Move_DisallowMoving, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (TechnoExtData::CannotMove(pThis, true))
	{
		pThis->ForceMission(Mission::Guard);
		return 0x4D4248;
	}

	return 0;
}

ASMJIT_PATCH(0x4D9563, FootClass_AssignDestination_DisallowMoving, 0x6)
{
	GET(FootClass*, pThis, EBP);

	// Prevent locomotor processing when assigned target if temporarily immobilized.
	if (FootExtContainer::Instance.Find(pThis)->IsZeroSpeed)
		return 0x4D96C2;

	return 0;
}

ASMJIT_PATCH(0x4D7EB5, FootClass_ActiveClickWith_DisallowMoving, 0x5)
{
	GET(FootClass*, pThis, ESI);

	return TechnoExtData::CannotMove(pThis, false) ? 0x4D7DC1 : 0;
}

// 3 Sep, 2025 - Starkku: Separated from above, do not change to guard mission
// and only handle the target acquisition part of area guard for immobile units.
ASMJIT_PATCH(0x4D6AAB, FootClass_Mission_AreaGuard_DisallowMoving, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (TechnoExtData::CannotMove(pThis, true))
	{
		if (pThis->CanPassiveAcquireTargets() && pThis->TargetingTimer.Completed())
			pThis->TargetAndEstimateDamage(&pThis->Location, ThreatType::Range);

		int delay = 1;

		if (!pThis->Target)
		{		
			pThis->UpdateIdleAction();
			auto const control = MissionControlClass::GetMissionControl(Mission::Area_Guard);
			delay = static_cast<int>(control->Rate * 900) + ScenarioClass::Instance->Random(1, 5);
		}

		R->EBP(delay);
		return 0x4D715A;
	}

	return 0;
}

ASMJIT_PATCH(0x4D5716, FootClass_ApproachTarget_DisallowMoving, 0x7)
{
	GET(FootClass*, pThis, EBX);

	if (TechnoExtData::CannotMove(pThis, true))
		return 0x4D571D;

	return 0;
}

#pragma endregion