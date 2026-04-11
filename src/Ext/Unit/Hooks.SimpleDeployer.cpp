#include "Body.h"

#include <GameOptionsClass.h>

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

#include <Ext/Anim/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>
#include <Utilities/Macro.h>

#pragma region JUMPJET

ASMJIT_PATCH(0x54C58E, JumpjetLocomotionClass_Descending_PathfindingChecks, 0x7)
{
	enum { SkipGameCode = 0x54C65F };

	GET(JumpjetLocomotionClass*, pThis, ESI);

	auto const pUnit = cast_to<UnitClass*>(pThis->LinkedTo);

	if (pUnit && pUnit->CurrentMission == Mission::Unload && TechnoExtData::SimpleDeployerAllowedToDeploy(pUnit, false, true))
		return SkipGameCode;

	return 0;
}

// Handle DeployDir for Jumpjet vehicles.
ASMJIT_PATCH(0x54C767, JumpjetLocomotionClass_Descending_DeployDir, 0x6)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	UnitExtContainer::CheckDeployRestrictions(pThis->LinkedTo, false);

	return 0x54C7A3;
}

//JJLOco_State3
DEFINE_JUMP(LJMP, 0x54C212, 0x54C22A);
#pragma endregion

#pragma region HOVER

// Do not display hover bobbing when landed during deploying.
ASMJIT_PATCH(0x513D2C, HoverLocomotionClass_ProcessBobbing_DeployToLand2, 0x6)
{
	GET(LocomotionClass*, pThis, ECX);

	if (auto const pUnit = cast_to<UnitClass*>(pThis->LinkedTo))
	{
		if (pUnit->Deploying && pUnit->Type->DeployToLand)
			return 0x513F2A;
	}

	return 0;
}

// Handle DeployToLand for hover locomotor vehicles part #2.
ASMJIT_PATCH(0x514A2A, HoverLocomotionClass_Process_DeployToLand, 0x8)
{
	enum { SkipGameCode = 0x514AC8, Continue = 0x514A32 };

	GET(ILocomotion*, pThis, ESI);
	GET(bool, isMoving, EAX);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (pLinkedTo->InAir)
	{
		auto const pType = GET_TECHNOTYPE(pLinkedTo);

		if (pType->DeployToLand)
		{
			auto const landType = pLinkedTo->GetCell()->LandType;

			if (landType == LandType::Water || landType == LandType::Beach)
			{
				pLinkedTo->InAir = false;
				pLinkedTo->QueueMission(Mission::Guard, true);
			}

			if (isMoving)
			{
				pThis->Stop_Moving();
				pLinkedTo->SetDestination(nullptr, true);
			}

			UnitExtContainer::CheckDeployRestrictions(pLinkedTo, false);

			if (pLinkedTo->GetHeight() <= 0)
			{
				pLinkedTo->InAir = false;
				pThis->Mark_All_Occupation_Bits((int)MarkType::Up);
			}
		}
	}

	// Restore overridden instructions.
	return isMoving ? Continue : SkipGameCode;
}

// Handle DeployToLand for hover locomotor vehicles part #3.
ASMJIT_PATCH(0x514E05, HoverLocomotionClass_MoveTo_DeployToLand, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (GET_TECHNOTYPE(pLinkedTo)->DeployToLand)
		pLinkedTo->InAir = false;

	return 0;
}

#pragma endregion

ASMJIT_PATCH(0x4DA9C9, FootClass_Update_DeployToLandSound, 0xA)
{
	GET(TechnoTypeClass* const, pType, EAX);
	GET(FootClass* const, pThis, ESI);

	return !pType->JumpJet || pThis->GetHeight() <= 0 ? 0x4DAA01 : 0x4DA9D7;
}

// DeployToLand units increment WalkingFramesSoFar on every frame, on hover units this causes weird behaviour with move sounds etc.
ASMJIT_PATCH(0x4DA9F3, FootClass_AI_DeployToLand, 0x6)
{
	enum { SkipGameCode = 0x4DAA01 };

	GET(FootClass*, pThis, ESI);

	if (GET_TECHNOTYPE(pThis)->Locomotor == HoverLocomotionClass::ClassGUID())
		return SkipGameCode;

	return 0;
}

// Allow keeping unit visible while displaying DeployingAnim.
ASMJIT_PATCH(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	enum { Continue = 0x73CF62, DoNotDraw = 0x73D43F };

	GET(UnitClass*, pThis, ESI);

	if (pThis->Deploying || pThis->Undeploying)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

		if (pTypeExt->DeployingAnim_KeepUnitVisible || (pThis->Deploying && !pThis->DeployAnim))
			return Continue;

		return DoNotDraw;
	}

	return Continue;
}