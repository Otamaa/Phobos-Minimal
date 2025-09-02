#include "Body.h"

#include <GameOptionsClass.h>

#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/Cast.h>

#include <Ext/Anim/Body.h>
#include <Ext/UnitType/Body.h>
#include <Utilities/Macro.h>

static bool CheckRestrictions(FootClass* pUnit, bool isDeploying)
{
	// Movement restrictions.
	if (isDeploying && pUnit->Locomotor->Is_Moving_Now())
		return true;

	FacingClass* currentDir = &pUnit->PrimaryFacing;
	bool isJumpjet = false;

	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
	{
		// Jumpjet rotating is basically half a guarantee it is also moving and
		// may not be caught by the Is_Moving_Now() check.
		if (isDeploying && pJJLoco->Facing.Is_Rotating())
			return true;

		currentDir = &pJJLoco->Facing;
		isJumpjet = true;
	}

	// Facing restrictions.
	auto const pType = pUnit->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const defaultFacing = (FacingType)(RulesClass::Instance->DeployDir >> 5);
	auto const facing = pTypeExt->DeployDir.Get(defaultFacing);

	if (facing == FacingType::None)
		return false;

	if (facing != (FacingType)currentDir->Current().GetFacing<8>())
	{
		auto dir = DirStruct();
		dir.SetFacing<8>((size_t)facing);

		if (isDeploying)
		{
			static_cast<UnitClass*>(pUnit)->Deploying = true;

			if (isJumpjet)
				currentDir->Set_Desired(dir);

			pUnit->Locomotor->Do_Turn(dir);

			return true;
		}
		else
		{
			currentDir->Set_Desired(dir);
		}
	}

	return false;
}

static COMPILETIMEEVAL FORCEDINLINE bool HasDeployingAnim(UnitTypeClass* pUnitType)
{
	return pUnitType->DeployingAnim || !TechnoTypeExtContainer::Instance.Find(pUnitType)->DeployingAnims.empty();
}

static void CreateDeployingAnim(UnitClass* pUnit, bool isDeploying)
{
	if (!pUnit->DeployAnim)
	{
		auto const pType = pUnit->Type;
		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto pAnimType = !pTypeExt->DeployingAnims.empty() ?
		GeneralUtils::GetItemForDirection<AnimTypeClass*>(pTypeExt->DeployingAnims, pUnit->PrimaryFacing.Current())
		: pUnit->Type->DeployingAnim;

		auto const pAnim = GameCreate<AnimClass>(pAnimType, pUnit->Location, 0, 1, 0x600, 0,
			!isDeploying && pTypeExt->DeployingAnim_ReverseForUndeploy);

		pUnit->DeployAnim = pAnim;
		pAnim->SetOwnerObject(pUnit);
		AnimExtData::SetAnimOwnerHouseKind(pAnim, pUnit->Owner, nullptr, pUnit, false , true);
		auto const pExt = TechnoExtContainer::Instance.Find(pUnit);

		if (pTypeExt->DeployingAnim_UseUnitDrawer)
		{
			pAnim->LightConvert = pUnit->GetRemapColour();
			pAnim->IsBuildingAnim = true; // Hack to make it use tint in drawing code.
		}

		// Set deploy animation timer. Required to be independent from animation lifetime due
		// to processing order / pointer invalidation etc. adding additional delay - simply checking
		// if the animation is still there wouldn't work well as it'd lag one frame behind I believe.
		const int rate = pAnimType->Normalized ?
			GameOptionsClass::Instance->GetAnimSpeed(pAnimType->Rate) :
			pAnimType->Rate;

		auto& timer = pExt->SimpleDeployerAnimationTimer;

		if (pAnimType->Reverse || pAnim->Reverse)
			timer.Start(pAnim->Animation.Stage * rate);
		else
			timer.Start(pAnimType->End * rate);
	}
}

// Full function reimplementation.
ASMJIT_PATCH(0x739AC0, UnitClass_SimpleDeployer_Deploy, 0x6)
{
	enum { ReturnFromFunction = 0x739CC3 };

	GET(UnitClass*, pThis, ECX);

	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return ReturnFromFunction;

	if (!pThis->Deployed)
	{
		if (!pThis->InAir && pType->DeployToLand && pThis->GetHeight() > 0)
			pThis->InAir = true;

		if (pThis->Deploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = true;
				pThis->Deploying = false;
			}
		}
		else if (!pThis->InAir)
		{
			if (CheckRestrictions(pThis, true))
				return ReturnFromFunction;

			if (HasDeployingAnim(pType))
			{
				CreateDeployingAnim(pThis, true);
				pThis->Deploying = true;
			}
			else
			{
				pThis->Deployed = true;
				pThis->Deploying = false; // DeployDir != -1 + no DeployingAnim case needs this reset here.
			}
		}
	}

	if (pThis->Deployed) {
		VocClass::SafeImmedietelyPlayAt(pType->DeploySound, pThis->Location);
	}

	return ReturnFromFunction;
}

// Full function reimplementation.
ASMJIT_PATCH(0x739CD0, UnitClass_SimpleDeployer_Undeploy, 0x6)
{
	enum { ReturnFromFunction = 0x739EBD };

	GET(UnitClass*, pThis, ECX);

	auto const pType = pThis->Type;

	if (!pType->IsSimpleDeployer)
		return ReturnFromFunction;

	if (pThis->Deployed)
	{
		if (pThis->Undeploying && pThis->DeployAnim)
		{
			auto const pExt = TechnoExtContainer::Instance.Find(pThis);
			auto& timer = pExt->SimpleDeployerAnimationTimer;

			if (timer.Completed())
			{
				timer.Stop();
				pThis->Deployed = false;
				pThis->Undeploying = false;
				auto cell = CellStruct::Empty;
				pThis->NearbyLocation(&cell, pThis);
				auto const pCell = MapClass::Instance->GetCellAt(cell);
				pThis->SetDestination(pCell, true);
			}
		}
		else
		{
			if (HasDeployingAnim(pType))
			{
				CreateDeployingAnim(pThis, true);
				pThis->Undeploying = true;
			}
			else
			{
				pThis->Deployed = false;
			}
		}

		if (pThis->IsDisguised())
			pThis->Disguised = false;

		if (!pThis->Deployed)
		{
			VocClass::SafeImmedietelyPlayAt(pType->UndeploySound, pThis->Location);
		}
	}

	return ReturnFromFunction;
}

// Disable Ares Jumpjet DeployDir hook.
//DEFINE_PATCH(0x54C767, 0x8B, 0x15, 0xE0, 0x71, 0x88, 0x00);

// Handle DeployDir for Jumpjet vehicles.
ASMJIT_PATCH(0x54C76D, JumpjetLocomotionClass_Descending_DeployDir, 0x7)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	auto const pLinkedTo = pThis->LinkedTo;
	CheckRestrictions(pLinkedTo, false);

	return 0x54C7A3;
}

// Disable DeployToLand=no forcing landing when idle due to what appears to be
// a code oversight and no need for DeployToLand=no to work in vanilla game.
ASMJIT_PATCH(0x54BED4, JumpjetLocomotionClass_Hovering_DeployToLand, 0x7)
{
	enum { SkipGameCode = 0x54BEE0 };

	GET(JumpjetLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinkedTo, ECX);

	auto const pType = pLinkedTo->GetTechnoType();

	if (!pType->BalloonHover || pType->DeployToLand)
		pThis->NextState = JumpjetLocomotionClass::State::Descending;

	pLinkedTo->TryNextPlanningTokenNode();
	return SkipGameCode;
}

// Same as above but at a different state.
ASMJIT_PATCH(0x54C2DF, JumpjetLocomotionClass_Cruising_DeployToLand, 0xA)
{
	enum { SkipGameCode = 0x54C4FD };

	GET(JumpjetLocomotionClass*, pThis, ESI);
	GET(FootClass*, pLinkedTo, ECX);

	auto const pType = pLinkedTo->GetTechnoType();

	if (!pType->BalloonHover || pType->DeployToLand)
	{
		pThis->Height = 0;
		pThis->NextState = JumpjetLocomotionClass::State::Descending;
	}

	pLinkedTo->TryNextPlanningTokenNode();
	return SkipGameCode;
}

// // Disable Ares hover locomotor bobbing processing DeployToLand hook.
// DEFINE_PATCH(0x513EAA, 0xA1, 0xE0, 0x71, 0x88, 0x00);
//
// // Handle DeployToLand for hover locomotor vehicles part #1.
// ASMJIT_PATCH(0x513EAF, HoverLocomotionClass_ProcessBobbing_DeployToLand1, 0x6)
// {
// 	enum { SkipGameCode = 0x513ECD };
//
// 	GET(LocomotionClass*, pThis, ESI);
//
// 	if (pThis->LinkedTo->InAir)
// 		return SkipGameCode;
//
// 	return 0;
// }

// Do not display hover bobbing when landed during deploying.
ASMJIT_PATCH(0x513D2C, HoverLocomotionClass_ProcessBobbing_DeployToLand2, 0x6)
{
	enum { SkipBobbing = 0x513F2A };

	GET(LocomotionClass*, pThis, ECX);

	if (auto const pUnit = cast_to<UnitClass*>(pThis->LinkedTo))
	{
		if (pUnit->Deploying && pUnit->Type->DeployToLand)
			return SkipBobbing;
	}

	return 0;
}

// Disable Ares hover locomotor processing DeployToLand hook.
//DEFINE_PATCH(0x514A21, 0x8B, 0x16, 0x56, 0xFF, 0x92, 0x80, 0x00, 0x00, 0x00);

// Handle DeployToLand for hover locomotor vehicles part #2.
ASMJIT_PATCH(0x514A2A, HoverLocomotionClass_Process_DeployToLand, 0x8)
{
	enum { SkipGameCode = 0x514AC8, Continue = 0x514A32 };

	GET(ILocomotion*, pThis, ESI);
	GET(bool, isMoving, EAX);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (pLinkedTo->InAir)
	{
		auto const pType = pLinkedTo->GetTechnoType();

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

			CheckRestrictions(pLinkedTo, false);

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

// Disable Ares hover locomotor move processing DeployToLand hook.
//DEFINE_PATCH(0x514DFE, 0x8D, 0x4C, 0x24, 0x10, 0x89, 0x46, 0x1C);

// Handle DeployToLand for hover locomotor vehicles part #3.
ASMJIT_PATCH(0x514E05, HoverLocomotionClass_MoveTo_DeployToLand, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	auto const pLinkedTo = static_cast<LocomotionClass*>(pThis)->LinkedTo;

	if (pLinkedTo->GetTechnoType()->DeployToLand)
		pLinkedTo->InAir = false;

	return 0;
}

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

	if (pThis->GetTechnoType()->Locomotor == HoverLocomotionClass::ClassGUID())
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