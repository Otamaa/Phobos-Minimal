#pragma region Includes
#include "Hooks.Otamaa.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/Unit/Body.h>

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <IsometricTileTypeClass.h>

#include <TiberiumClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <AircraftTrackerClass.h>

#include <Memory.h>

#include <Locomotor/Cast.h>

#include <Ext/TerrainType/Body.h>
#include <Misc/DamageArea.h>
#include <Misc/Ares/Hooks/Header.h>

#include <Surface.h>

#include <Audio.h>

#include <Commands/ShowTeamLeader.h>

#include <Commands/ToggleRadialIndicatorDrawMode.h>

#include <ExtraHeaders/AStarClass.h>
#include <TextDrawing.h>
#include <format>

#include <Ext/SWType/Body.h>
#include <New/Type/CrateTypeClass.h>

#include <SpotlightClass.h>
#include <New/Entity/FlyingStrings.h>

#include <ExtraHeaders/StackVector.h>

#include <Ext/RadSite/Body.h>

#pragma endregion

ASMJIT_PATCH(0x74C8FB, VeinholeMonsterClass_CTOR_SetArmor, 0x6)
{
	GET(VeinholeMonsterClass*, pThis, ESI);
	GET(TerrainTypeClass* const, pThisTree, EDX);

	auto pType = pThis->GetType();
	if (pType && pThisTree)
		pType->Armor = pThisTree->Armor;

	return 0x0;
}

static	void __fastcall DrawShape_VeinHole
(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds,
 BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape,
 int ZShapeFrame, int XOffset, int YOffset
)
{
	if (auto pManager = RulesExtData::Instance()->VeinholePal.GetConvert())
		Pal = pManager;

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_FUNCTION_JUMP(CALL, 0x74D5BC, DrawShape_VeinHole);

ASMJIT_PATCH(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	const auto nTheater = ScenarioClass::Instance->Theater;
	SmudgeTypeClass::TheaterInit(nTheater);
	VeinholeMonsterClass::TheaterInit(nTheater);
	return 0x4AD0A8;
}

ASMJIT_PATCH(0x74D0D2, VeinholeMonsterClass_AI_SelectParticle, 0x5)
{
	//overriden instructions
	R->Stack(0x2C, R->EDX());
	R->Stack(0x30, R->EAX());
	LEA_STACK(CoordStruct*, pCoord, 0x28);
	const auto pRules = RulesExtData::Instance();
	const auto pParticle = pRules->VeinholeParticle.Get(pRules->DefaultVeinParticle.Get());
	R->EAX(ParticleSystemClass::Instance->SpawnParticle(pParticle, pCoord));
	return 0x74D100;
}

ASMJIT_PATCH(0x7290AD, TunnelLocomotionClass_Process_Stop, 0x5)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	if (const auto pLinked = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo)
		if (auto const pCell = pLinked->GetCell())
			pCell->CollectCrate(pLinked);

	return 0;
}

ASMJIT_PATCH(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

#pragma region WallTower
ASMJIT_PATCH(0x4405C1, BuildingClas_Unlimbo_WallTowers_A, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x4405CF : 0x440606;
}

ASMJIT_PATCH(0x440F66, BuildingClass_Unlimbo_WallTowers_B, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x440F78 : 0x44104D;
}

ASMJIT_PATCH(0x445ADB, BuildingClass_Limbo_WallTowers, 0x9)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445AED : 0x445B81;
}

ASMJIT_PATCH(0x4514F9, BuildingClass_AnimLogic_WallTowers, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x45150B : 0x4515E9;
}

ASMJIT_PATCH(0x45EF11, BuildingClass_FlushForPlacement_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EBX);
	R->EDX(RulesClass::Instance());
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x45EF23 : 0x45F00B;
}

ASMJIT_PATCH(0x47C89C, CellClass_CanThisExistHere_SomethingOnWall, 0x6)
{
	GET(int const, nHouseIDx, EAX);
	GET(CellClass* const, pCell, EDI);
	GET(int const, idxOverlay, ECX);
	GET_STACK(BuildingTypeClass* const, PlacingObject, STACK_OFFS(0x18, -0x8));
	GET_STACK(HouseClass* const, PlacingOwner, STACK_OFFS(0x18, -0xC));

	enum { Adequate = 0x47CA70, Inadequate = 0x47C94F } Status = Inadequate;

	HouseClass* OverlayOwner = HouseClass::Array->get_or_default(nHouseIDx);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;

	if (PlacingObject)
	{
		const bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->Items[idxOverlay]->Wall;

		if (ContainsWall && (PlacingObject->Gate || Nvec.Contains(PlacingObject)))
		{
			Status = Adequate;
		}

		if (OverlayTypeClass* ToOverlay = PlacingObject->ToOverlay)
		{
			if (ToOverlay->ArrayIndex == idxOverlay)
			{
				if (pCell->OverlayData >= 0x10)
				{
					Status = Adequate;
				}
			}
		}
	}

	if (Status == Inadequate)
	{
		switch (idxOverlay)
		{
		case OVERLAY_GASAND:
		case OVERLAY_GAWALL:
			if (Nvec.Contains(PlacingObject) ||
					PlacingObject == RulesClass::Instance->GDIGateOne ||
					PlacingObject == RulesClass::Instance->GDIGateTwo)
			{
				Status = Adequate;
			}
			break;
		case OVERLAY_NAWALL:
			if (PlacingObject == RulesClass::Instance->NodGateOne ||
				PlacingObject == RulesClass::Instance->NodGateTwo)
			{
				Status = Adequate;
			}
			break;
		}
	}

	if (Status == Adequate)
	{
		if (PlacingOwner != OverlayOwner)
		{
			Status = Inadequate;
		}
	}

	return Status;
}

ASMJIT_PATCH(0x4FE546, HouseClass_BuildingClass_AI_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x4FE554 : 0x4FE6E7;
}

ASMJIT_PATCH(0x4FE648, HouseClss_AI_Building_WallTowers, 0x6)
{
	GET(int const, nNodeBuilding, EAX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;

	if (nNodeBuilding == -1 || Nvec.empty())
		return 0x4FE696;

	return Nvec.Any_Of([&](BuildingTypeClass* const pWallTower)
 {
	 return pWallTower->ArrayIndex == nNodeBuilding;
 })
		? 0x4FE656 : 0x4FE696;
}

ASMJIT_PATCH(0x5072F8, HouseClass_506EF0_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50735C : 0x507306;
}

ASMJIT_PATCH(0x50A96E, HouseClass_AI_TakeOver_WallTowers_A, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50A980 : 0x50AB90;
}

ASMJIT_PATCH(0x50A9D2, HouseClass_AI_TakeOver_WallTowers_B, 0x6)
{
	GET(BuildingClass* const, pThis, EBX);
	R->EAX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x50A9EA : 0x50AB3D;
}
#pragma endregion

static bool __fastcall AircraftTypeClass_CanUseWaypoint(AircraftTypeClass* pThis, void*)
{
	return !pThis->Spawned;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2908, AircraftTypeClass_CanUseWaypoint);

ASMJIT_PATCH(0x4B050B, DriveLocomotionClass_Process_Cargo, 0x5)
{
	GET(ILocomotion* const, pILoco, ESI);

	const auto pLoco = static_cast<DriveLocomotionClass* const>(pILoco);

	if (const auto pFoot = pLoco->LinkedTo)
	{
		if (const auto pTrans = pFoot->Transporter)
		{
			R->EAX(pTrans->GetCell());
			return 0x4B0516;
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x4B07CA, DriveLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pDrive = static_cast<DriveLocomotionClass* const>(pLoco);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pDrive->LinkedTo->GetTechnoType());
	TechnoExtData::PlayAnim(pTypeExt->Wake.Get(RulesClass::Instance->Wake), pDrive->LinkedTo);
	return 0x4B0828;
}

ASMJIT_PATCH(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pShip = static_cast<ShipLocomotionClass* const>(pLoco);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pShip->LinkedTo->GetTechnoType());
	TechnoExtData::PlayAnim(pTypeExt->Wake.Get(RulesClass::Instance->Wake), pShip->LinkedTo);
	return 0x69FEF0;
}

ASMJIT_PATCH(0x414EAA, AircraftClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(AircraftClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0x24));

	pAnim->AnimClass::AnimClass(TechnoTypeExtData::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x414ED0;
}

ASMJIT_PATCH(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(UnitClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	pAnim->AnimClass::AnimClass(TechnoTypeExtData::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_600, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x7365BB;
}

ASMJIT_PATCH(0x738703, UnitClass_Explode_ExplodeAnim, 0x5)
{
	GET(AnimTypeClass*, pExplType, EDI);
	GET(UnitClass*, pThis, ESI);

	if (pExplType)
	{

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExplType, pThis->Location, 0, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x738748;
}

ASMJIT_PATCH(0x4419A9, BuildingClass_Destroy_ExplodeAnim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, X, ECX);
	GET(int, Y, EDX);
	GET(int, Z, EAX);
	GET(int, zAdd, EDI);

	CoordStruct nLoc { X , Y , Z + zAdd };
	const int idx = pThis->Type->Explosion.Count == 1 ?
		0 : ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1);

	if (auto const pType = pThis->Type->Explosion.Items[idx])
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomFromMax(3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, nLoc, nDelay, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	R->Stack(0x20, nLoc.X);
	R->Stack(0x24, nLoc.Y);
	R->Stack(0x28, nLoc.Z);
	return 0x441A24;
}

ASMJIT_PATCH(0x441AC4, BuildingClass_Destroy_Fire3Anim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x64 - 0x54);

	if (auto pType = RulesExtData::Instance()->DefaultExplodeFireAnim)
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomRanged(1, 3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoord, nDelay + 3, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x441B1F;
}

ASMJIT_PATCH(0x441D1F, BuildingClass_Destroy_DestroyAnim, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	return 0x0;
}

ASMJIT_PATCH(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum { ContinueCheck = 0x6FC23A, BypassCheck = 0x6FC24D };
	GET(TechnoClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	//TODO : re-eval check below  , i if desync/the behaviour is not good , change it to pThis->Owner->IsControlledByCurrentPlayer()
	const bool Allow = RulesExtData::Instance()->AutoAttackICedTarget.Get() || pThis->Owner->IsControlledByHuman();
	return pTypeExt->AllowFire_IroncurtainedTarget.Get(Allow)
		? BypassCheck : ContinueCheck;
}

static_assert(offsetof(HouseClass, IsHumanPlayer) == 0x1EC, "ClassMember Shifted !");
static_assert(offsetof(HouseClass, IsInPlayerControl) == 0x1ED, "ClassMember Shifted !");

ASMJIT_PATCH(0x7091FC, TechnoClass_CanPassiveAquire_AI, 0x6)
{
	enum
	{
		DecideResult = 0x709202,
		Continue = 0x0,
		ContinueCheck = 0x709206,
		CantPassiveAcquire = 0x70927D,
	};

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	const auto owner = pThis->Owner;

	if (pTypeExt->PassiveAcquire_AI.isset())
	{
		if (owner
			&& !owner->Type->MultiplayPassive
			&& !owner->IsControlledByHuman()
			)
		{

			R->CL(pTypeExt->PassiveAcquire_AI.Get());
			return 0x709202;
		}
	}

	R->CL((pType->Naval && pTypeExt->CanPassiveAquire_Naval.isset()) ?
		pTypeExt->CanPassiveAquire_Naval.Get() : pType->CanPassiveAquire);
	return 0x709202;
}

ASMJIT_PATCH(0x6F8260, TechnoClass_EvaluateObject_LegalTarget_AI, 0x6)
{
	enum
	{
		Continue = 0x0,
		ContinueChecks = 0x6F826E,
		ReturnFalse = 0x6F894F,
		SetAL = 0x6F8266,
	};

	GET(TechnoClass* const, pThis, EDI);
	//GET(TechnoClass* const, pTarget, ESI);
	GET(TechnoTypeClass* const, pTargetType, EBP);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	if (pTypeExt->AI_LegalTarget.isset() && !pThis->Owner->IsControlledByHuman())
	{
		return pTypeExt->AI_LegalTarget.Get() ?
			ContinueChecks : ReturnFalse;
	}

	return Continue;
}

ASMJIT_PATCH(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
{
	enum { Skip = 0x4DC1A0, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Attack_Color.isset())
	{
		GET(CoordStruct*, pMovingDestCoord, EAX);
		GET(int, nFLH_X, EBP);
		GET(int, nFLH_Y, EBX);
		GET_STACK(int, nFLH_Z, STACK_OFFS(0x34, 0x10));

		if (pTypeExt->CommandLine_Attack_Color.Get() != ColorStruct::Empty)
		{
			Drawing::Draw_action_lines_7049C0(nFLH_X, nFLH_Y, nFLH_Z, pMovingDestCoord->X, pMovingDestCoord->Y, pMovingDestCoord->Z,
				pTypeExt->CommandLine_Attack_Color->ToInit(), false, false);
		}

		return Skip;
	}

	return Continue;
}

ASMJIT_PATCH(0x4DC280, FootClass_DrawActionLines_Move, 0x5)
{
	enum { Skip = 0x4DC328, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Move_Color.isset())
	{
		GET_STACK(CoordStruct, nCooordDest, STACK_OFFS(0x34, 0x24));
		GET(int, nCoordDest_Adjusted_Z, EDI);
		GET(int, nLoc_X, EBP);
		GET(int, nLoc_Y, EBX);
		GET_STACK(int, nLoc_Z, STACK_OFFS(0x34, 0x10));
		GET_STACK(bool, barg3, STACK_OFFSET(0x34, 0x8));

		if (pTypeExt->CommandLine_Move_Color.Get() != ColorStruct::Empty)
		{
			Drawing::Draw_action_lines_7049C0(nLoc_X, nLoc_Y, nLoc_Z, nCooordDest.X, nCooordDest.Y, nCoordDest_Adjusted_Z,
				pTypeExt->CommandLine_Move_Color->ToInit(), barg3, false);
		}

		return Skip;
	}

	return Continue;
}

ASMJIT_PATCH(0x51CDB9, InfantryClass_RandomAnimate_CheckIdleRate, 0x6)
{
	return R->ESI<InfantryClass* const>()->Type->IdleRate == -1 ? 0x51D0A0 : 0x0;
}

ASMJIT_PATCH(0x6FDE05, TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0;
} ASMJIT_PATCH_AGAIN(0x6FF933, TechnoClass_FireAt_End, 0x5);

ASMJIT_PATCH(0x5D3ADE, MessageListClass_Init_MessageMax, 0x6)
{
	if (Phobos::Otamaa::IsAdmin)
		R->EAX(14);

	return 0x0;
}

// replace the repair button fucntion to toggle power
ASMJIT_PATCH(0x6A78F6, SidebarClass_AI_RepairMode_ToggelPowerMode, 0x9)
{
	GET(SidebarClass* const, pThis, ESI);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		pThis->SetTogglePowerMode(-1);
	else
		pThis->SetRepairMode(-1);

	return 0x6A78FF;
}

// replace the repair button fucntion to toggle power
ASMJIT_PATCH(0x6A7AE1, SidebarClass_AI_DisableRepairButton_TogglePowerMode, 0x6)
{
	GET(SidebarClass* const, pThis, ESI);

	return Phobos::Config::TogglePowerInsteadOfRepair ? pThis->PowerToggleMode : pThis->RepairMode ?
		0x6A7AFE : 0x6A7AE7;
}

ASMJIT_PATCH(0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
		return 0x508CEE; // add the power

	return 0x0;
}

// ASMJIT_PATCH(0x508EE5, HouseClass_UpdateRadar_LimboDeliver, 0x6)
// {
// 	GET(FakeBuildingClass*, pBld, EAX);
// 	enum
// 	{
// 		ContinueLoop = 0x508F08,
// 		ContinueCheck = 0x0,
// 		EligibleRadar = 0x508F2A
// 	};
//
// 	if (TechnoExtContainer::Instance.Find(pBld)->AE.DisableRadar)
// 		return ContinueLoop;
//
// 	if (!pBld->_GetExtData()->RegisteredJammers.empty())
// 		return ContinueLoop;
//
// 	if (pBld->EMPLockRemaining > 0)
// 		return ContinueLoop;
//
// 	// if the `Limboed` Building has radar , just accept it
// 	if(pBld->_GetExtData()->LimboID != -1)
// 		return EligibleRadar;
//
// 	return ContinueCheck;
// }

ASMJIT_PATCH(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
{
	enum { Continue = 0x0, DoNotDrawRadar = 0x70D407 };

	GET(TechnoClass* const, pThis, ESI);

	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1)
		{
			return DoNotDrawRadar;
		}
	}

	return
		TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->IsDummy ?
		DoNotDrawRadar :
		Continue;
}

ASMJIT_PATCH(0x6F09C0, TeamTypeClass_CreateOneOf_Handled, 0x9)
{
	GET(TeamTypeClass*, pThis, ECX);
	GET_STACK(DWORD, caller, 0x0);
	GET_STACK(HouseClass*, pHouse, 0x4);

	if (!pHouse)
	{
		pHouse = pThis->Owner;
		if (!pHouse)
		{
			if (HouseClass::Index_IsMP(pThis->idxHouse))
			{
				pHouse = HouseClass::FindByPlayerAt(pThis->idxHouse);
			}
		}
	}

	if (!pHouse)
	{
		R->EAX<TeamClass*>(nullptr);
		return 0x6F0A2C;
	}

	if (!Unsorted::ScenarioInit)
	{
		if (pThis->Max >= 0)
		{
			if (SessionClass::Instance->GameMode != GameMode::Campaign)
			{
				if (pHouse->GetTeamCount(pThis) >= pThis->Max)
				{
					R->EAX<TeamClass*>(nullptr);
					return 0x6F0A2C;
				}
			}
			else if (pThis->cntInstances >= pThis->Max)
			{
				R->EAX<TeamClass*>(nullptr);
				return 0x6F0A2C;
			}
		}
	}

	const auto pTeam = GameCreate<TeamClass>(pThis, pHouse, false);
	Debug::LogInfo("[{0} - {1}] Creating a new team named [{2} -{3}] caller [{4:x}].",
		pHouse->get_ID(), (void*)pHouse, pThis->ID, (void*)pTeam, caller);
	R->EAX(pTeam);
	return 0x6F0A2C;
}

DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);

// ASMJIT_PATCH(0x6E93BE, TeamClass_AI_TransportTargetLog, 0x5)
// {
// 	GET(FootClass* const, pThis, EDI);
// 	Debug::LogInfo("[{}][{}] Transport just recieved orders to go home after unloading ", (void*)pThis, pThis->get_ID());
// 	return 0x6E93D6;
// }

// ASMJIT_PATCH(0x6EF9B0, TeamMissionClass_GatherAtEnemyCell_Log, 0x5)
// {
// 	GET_STACK(short const, nCellX, 0x10);
// 	GET_STACK(short const, nCellY, 0x12);
// 	GET(TeamClass* const, pThis, ESI);
// 	GET(TechnoClass* const, pTechno, EDI);
// 	Debug::LogInfo("[{}][{}] Team with Owner '{}' has chosen ({} , {}) for its GatherAtEnemy cell.", (void*)pThis, pThis->Type->ID, pTechno->Owner ? pTechno->Owner->get_ID() : GameStrings::NoneStrb(), nCellX, nCellY);
// 	return 0x6EF9D0;
// }

ASMJIT_PATCH(0x6D912B, TacticalClass_Render_BuildingInLimboDeliveryA, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D9159 };

	GET(TechnoClass* const, pTechno, ESI);

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pTechno))->LimboID != -1)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

ASMJIT_PATCH(0x6D966A, TacticalClass_Render_BuildingInLimboDeliveryB, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D978F };

	GET(TechnoClass* const, pTechno, EBX);

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pTechno))->LimboID != -1)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

ASMJIT_PATCH(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x7)
{
	GET(UnitClass* const, pThis, EBP);
	GET(AnimClass*, pAllocatedMem, ESI);
	GET(LandType const, nLand, EBX);

	CoordStruct nLoc = pThis->Location;
	if (auto const pC4AnimType = MapClass::SelectDamageAnimation(pThis->Health, RulesClass::Instance->C4Warhead, nLand, nLoc))
	{
		pAllocatedMem->AnimClass::AnimClass(pC4AnimType, nLoc, 0, 1, 0x2600, -15, false);
		AnimExtData::SetAnimOwnerHouseKind(pAllocatedMem, nullptr, pThis->GetOwningHouse(), true);
	}
	else
	{
		GameDelete<false, false>(pAllocatedMem);
	}

	return 0x73AF4C;
}

#include <Ext/Tiberium/Body.h>

enum class NewVHPScan : int
{
	None = 0,
	Normal = 1,
	Strong = 2,
	//Threat = 3,
	//Health = 4,
	//Damage = 5,
	//Value = 6,
	//Locked = 7,
	//Non_Infantry = 8,

	count
};

COMPILETIMEEVAL std::array<const char*, (size_t)NewVHPScan::count> NewVHPScanToString
{ {
	{ "None" }
	,{ "Normal" }
	,{ "Strong" }
	//,{ "Threat" }
	//,{ "Health" }
	//,{ "Damage" }
	//,{ "Value" }
	//,{ "Locked" }
	//,{ "Non_Infantry" }
	} };

ASMJIT_PATCH(0x477590, CCINIClass_ReadVHPScan_Replace, 0x6)
{
	GET(CCINIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, default_val, 0xC);

	INI_EX exINI(pThis);

	int vHp = default_val;

	if (exINI.ReadString(pSection, pKey) > 0)
	{
		for (int i = 0; i < (int)NewVHPScanToString.size(); ++i)
		{
			if (IS_SAME_STR_(exINI.value(), NewVHPScanToString[i]))
			{
				R->EAX(i);
				return 0x477613;
			}
		}

		Debug::INIParseFailed(pSection, pKey, exINI.value(), "Expected valid VHPScan value");
	}

	R->EAX(vHp);
	return 0x477613;
}

#include <Ext/Cell/Body.h>

ASMJIT_PATCH(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7)
{
	GET(InfantryClass* const, pThis, ECX);

	enum { SkipWholeFunction = 0x5192BC, Continue = 0x0 };

	return InfantryTypeExtContainer::Instance.Find(pThis->Type)->HideWhenDeployAnimPresent.Get()
		&& pThis->DeployAnim ? SkipWholeFunction : Continue;
}

CoordStruct* FakeUnitClass::_GetFLH(CoordStruct* outBuffer, int weaponIdx, CoordStruct base)
{
	const auto pThis = static_cast<UnitClass*>(this);

	do
	{
		const auto pTransporter = pThis->Transporter;

		if (pThis->InOpenToppedTransport && pTransporter && TechnoTypeExtContainer::Instance.Find(pTransporter->GetTechnoType())->AlternateFLH_ApplyVehicle)
		{
			if (const int idx = pTransporter->Passengers.IndexOf(pThis)) {
				pTransporter->GetFLH(outBuffer , -idx, CoordStruct::Empty);
				break;
			}
		}

		pThis->TechnoClass::GetFLH(outBuffer, weaponIdx, CoordStruct::Empty);
	}
	while (false);

	return outBuffer;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D20, FakeUnitClass::_GetFLH);

// issue #895788: cells' high occupation flags are marked only if they
// actually contains a bridge while unmarking depends solely on object
// height above ground. this mismatch causes the cell to become blocked.
void FakeUnitClass::_SetOccupyBit(CoordStruct* pCrd)
{
	CellClass* pCell = MapClass::Instance->GetCellAt(pCrd);
	int height = MapClass::Instance->GetCellFloorHeight(pCrd) + Unsorted::BridgeHeight;
	bool alt = (pCrd->Z >= height && pCell->ContainsBridge());
	//auto pCellExt = CellExtContainer::Instance.TryFind(pCell);

	// remember which occupation bit we set
	this->_GetExtData()->AltOccupation = alt;

	if (alt)
	{
		pCell->AltOccupationFlags |= 0x20;
		//if(pCellExt && !TechnoExtData::DoesntOccupyCellAsChild(this))
		//	pCellExt->IncomingUnitAlt = this;
	}
	else
	{
		pCell->OccupationFlags |= 0x20;

		//if(pCellExt && !TechnoExtData::DoesntOccupyCellAsChild(this))
		//	pCellExt->IncomingUnit = this;
	}
}

void FakeUnitClass::_ClearOccupyBit(CoordStruct* pCrd)
{
	enum { obNormal = 1, obAlt = 2 };

	CellClass* pCell = MapClass::Instance->GetCellAt(pCrd);
	//auto pCellExt = CellExtContainer::Instance.TryFind(pCell);
	int height = MapClass::Instance->GetCellFloorHeight(pCrd) + Unsorted::BridgeHeight;
	int alt = (pCrd->Z >= height) ? obAlt : obNormal;

	// also clear the last occupation bit, if set

	if (!this->_GetExtData()->AltOccupation.empty())
	{
		int lastAlt = this->_GetExtData()->AltOccupation ? obAlt : obNormal;
		alt |= lastAlt;
		this->_GetExtData()->AltOccupation.clear();
	}

	if (alt & obAlt)
	{
		pCell->AltOccupationFlags &= ~0x20;
		//if(pCellExt && !TechnoExtData::DoesntOccupyCellAsChild(this))
		//	pCellExt->IncomingUnitAlt= this;
	}

	if (alt & obNormal)
	{
		pCell->OccupationFlags &= ~0x20;
		//if(pCellExt && !TechnoExtData::DoesntOccupyCellAsChild(this))
		//	pCellExt->IncomingUnit = this;
	}

}

DEFINE_FUNCTION_JUMP(LJMP, 0x744210, FakeUnitClass::_ClearOccupyBit);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D64, FakeUnitClass::_ClearOccupyBit);

DEFINE_FUNCTION_JUMP(LJMP, 0x7441B0, FakeUnitClass::_SetOccupyBit);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D60, FakeUnitClass::_SetOccupyBit);

ASMJIT_PATCH(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(FootClass* const, pFoot, EAX);

	if (const auto pTeam = pFoot->Team)
	{
		if (auto nTeamDecision = pTeam->Type->MindControlDecision)
		{
			if (nTeamDecision > 5)
				nTeamDecision = ScenarioClass::Instance->Random.RandomRanged(1, 5);

			R->EAX(nTeamDecision);
			return 0x47258F;
		}
	}

	return 0x4725B0;
}

ASMJIT_PATCH(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	GET(ObjectClass* const, pThis, ECX);
	GET_STACK(TechnoClass* const, pTechno, 0x4);
	R->AL(TechnoExtData::IsCrushable(pThis, pTechno));
	return 0x5F6D90;
}

ASMJIT_PATCH(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass* const, pProduct, ESI);
	VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pProduct->GetTechnoType())->Eva_Complete.Get());
	return 0x4FB649;
}

ASMJIT_PATCH(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	enum { ReturnNoVoiceCreate = 0x4FB804, Continue = 0x0 };

	GET(HouseClass* const, pThis, EDI);
	GET(TechnoClass* const, pTechno, EBP);

	if (pTechno)
	{
		const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

		if (pTechnoTypeExt->VoiceCreate >= 0)
		{

			if (!pTechnoTypeExt->VoiceCreate_Instant)
				pTechno->QueueVoice(pTechnoTypeExt->VoiceCreate);
			else
			{
				if (pThis->IsControlledByHuman() && !pThis->IsCurrentPlayerObserver())
					VocClass::SafeImmedietelyPlayAt(pTechnoTypeExt->VoiceCreate, &pTechno->Location);
			}
		}

		if (!pTechnoTypeExt->CreateSound_Enable.Get())
			return ReturnNoVoiceCreate;

		if (!EnumFunctions::IsPlayerTypeEligible((AffectPlayerType::Observer | AffectPlayerType::Player), HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;

		if (!EnumFunctions::CanTargetHouse(pTechnoTypeExt->CreateSound_afect.Get(RulesExtData::Instance()->CreateSound_PlayerOnly), pThis, HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;
	}

	return Continue;
}

ASMJIT_PATCH(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass* const, pTech, ESI);

	if (pTech->WhatAmI() == BuildingClass::AbsID)
	{
		VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pTech->GetTechnoType())->Eva_Complete.Get());
		return 0x6A8E34;
	}

	return 0x0;
}

// ASMJIT_PATCH(0x4242F4, AnimClass_Trail_Override, 0x6)
// {
// 	GET(AnimClass*, pAnim, EDI);
// 	GET(AnimClass*, pThis, ESI);
//
// 	auto nCoord = pThis->GetCoords();
// 	pAnim->AnimClass::AnimClass(pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_600, 0, false);
// 	//const auto pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
// 	TechnoClass* const pTech = AnimExtData::GetTechnoInvoker(pThis);
// 	HouseClass* const pOwner = !pThis->Owner && pTech ? pTech->Owner : pThis->Owner;
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);
//
// 	return 0x424322;
// }

ASMJIT_PATCH(0x51DF82, InfantryClass_FireAt_StartReloading, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	const auto pType = pThis->Type;

	if (pThis->Transporter)
	{
		if (TechnoTypeExtContainer::Instance.Find(pType)->ReloadInTransport
			&& pType->Ammo > 0
			&& pThis->Ammo < pType->Ammo
		)
			pThis->StartReloading();
	}

	return 0;
}

ASMJIT_PATCH(0x739450, UnitClass_Deploy_LocationFix, 0x7)
{
	GET(UnitClass* const, pThis, EBP);
	const auto deploysInto = pThis->Type->DeploysInto;
	CellStruct mapCoords = pThis->InlineMapCoords();
	R->Stack(STACK_OFFSET(0x28, -0x10), mapCoords.Pack());

	const short width = deploysInto->GetFoundationWidth();
	const short height = deploysInto->GetFoundationHeight(false);

	if (width > 2)
		mapCoords.X -= static_cast<short>(std::ceil(width / 2.0) - 1);
	if (height > 2)
		mapCoords.Y -= static_cast<short>(std::ceil(height / 2.0) - 1);

	R->Stack(STACK_OFFSET(0x28, -0x14), mapCoords.Pack());

	return 0x7394BE;
}

// Skip log spam "Unable to locate scenario %s - No digest info"
DEFINE_JUMP(LJMP, 0x69A797, 0x69A937);

ASMJIT_PATCH(0x70FB50, TechnoClass_Bunkerable, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
	{

		const auto pType = pFoot->GetTechnoType();
		if (pType->Bunkerable)
		{
			const auto nSpeedType = pType->SpeedType;
			if (nSpeedType == SpeedType::Hover
				|| nSpeedType == SpeedType::Winged
				|| nSpeedType == SpeedType::None)
			{
				R->EAX(false);
				return 0x70FBCA;
			}

			if (pFoot->ParasiteEatingMe)
			{
				R->EAX(false);
				return 0x70FBCA;
			}

			//crash the game , dont allow it
			//maybe because of force_track stuffs,..
			const auto loco = VTable::Get(pFoot->Locomotor.GetInterfacePtr());
			if (loco == HoverLocomotionClass::vtable
				|| loco == MechLocomotionClass::vtable
				|| loco == FlyLocomotionClass::vtable
				|| loco == DropPodLocomotionClass::vtable
				|| loco == RocketLocomotionClass::vtable
				|| loco == ShipLocomotionClass::vtable)
			{

				R->EAX(false);
				return 0x70FBCA;
			}

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pTypeExt->BunkerableAnyway)
			{
				R->EAX(true);
				return 0x70FBCA;
			}

			if (!pType->Turret || !pFoot->IsArmed())
			{
				R->EAX(false);
				return 0x70FBCA;
			}

			R->EAX(true);
			return 0x70FBCA;
		}
	}

	R->EAX(false);
	return 0x70FBCA;
}

ASMJIT_PATCH(0x708F77, TechnoClass_ResponseToSelect_BugFixes, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);

	return pThis->IsCrashing || pThis->Health < 0 ?
		0x708FAD : 0x0;
}

ASMJIT_PATCH(0x6EE17E, MoveCrameraToWaypoint_CancelFollowTarget, 0x8)
{
	DisplayClass::Instance->FollowAnObject(nullptr);
	return 0x0;
}

ASMJIT_PATCH(0x437C29, sub_437A10_Lock_Bound_Fix, 7)
{
	GET_STACK(int const, nX_comp, 0x30);
	GET_STACK(int const, nY_comp, 0x58);
	GET(Surface*, pSurface, ECX);
	GET(int, nX, EAX);
	GET(int, nY, EDX);

	if (nX >= nX_comp || nX < 0)
		nX = 0;
	if (nY >= nY_comp || nY < 0)
		nY = 0;

	R->EAX(pSurface->Lock(nX, nY));
	return 0x437C30;
}

//TechnoClass_GetWeaponState
ASMJIT_PATCH(0x6FCA30, TechnoClass_GetFireError_DecloakToFire, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pTransporter = pThis->Transporter;

	if (pTransporter && pTransporter->CloakState != CloakState::Uncloaked)
		return 0x6FCA4F;

	if (pThis->CloakState == CloakState::Uncloaked)
		return 0x6FCA5E;

	if (!pWeapon->DecloakToFire && pThis->WhatAmI() == AircraftClass::AbsID)
		return 0x6FCA4F;

	return pThis->CloakState == CloakState::Cloaked ? 0x6FCA4F : 0x6FCA5E;
	//return 0x0;
}

ASMJIT_PATCH(0x741554, UnitClass_ApproachTarget_CrushRange, 0x6)
{
	enum { Crush = 0x741599, ContinueCheck = 0x741562 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return range >= pTypeExt->CrushRange.GetOrDefault(pThis, RulesClass::Instance->Crush) ?
		Crush : ContinueCheck;
}

ASMJIT_PATCH(0x7439AD, UnitClass_ShouldCrush_CrushRange, 0x6)
{
	enum { DoNotCrush = 0x743A39, ContinueCheck = 0x7439B9 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);
	GET(RulesClass* const, pRules, ECX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return range <= pTypeExt->CrushRange.GetOrDefault(pThis, pRules->Crush) ?
		ContinueCheck : DoNotCrush;
}

// make unit able to deploy fire without ejecting it passengers
ASMJIT_PATCH(0x73D6EC, UnitClass_Unload_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x73DCD3 : 0x0;
}

ASMJIT_PATCH(0x740015, UnitClass_WhatAction_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x7400F0 : 0x0;
}

DEFINE_JUMP(LJMP, 0x422A59, 0x422A5F);

ASMJIT_PATCH(0x6F357F, TechnoClass_SelectWeapon_DrainWeaponTarget, 0x6)
{
	enum { CheckAlly = 0x6F3589, ContinueCheck = 0x6F35A8, RetPrimary = 0x6F37AD };

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EBP);

	const bool IsTargetEligible = !pThis->DrainTarget && !pTarget->DrainingMe;
	return IsTargetEligible ?
		CheckAlly : ContinueCheck;
}

ASMJIT_PATCH(0x51F885, InfantryClass_WhatAction_TubeStuffs_FixGetCellAtCallTwice, 0x7)
{
	enum { retTrue = 0x51F8A6, retFalse = 0x51F8A8 };
	GET(CellClass* const, pCell, EAX);

	return pCell->CellClass_Tube_484AE0() || pCell->CellClass_Tube_484D60() ?
		retTrue : retFalse;
}

ASMJIT_PATCH(0x51F9B7, InfantryClass_WhatAction_TubeStuffs_FixGetCellAtCallTwice_part2, 0x7)
{
	enum { retFalse = 0x51F953 };
	GET(CellClass* const, pCell, EAX);
	GET(InfantryClass* const, pThis, EDI);

	if (!pCell->Tile_Is_Tunnel())
		return retFalse;

	R->EAX(pCell->CellClass_484F10(pThis));
	return 0x51F9D5;
}

ASMJIT_PATCH(0x7008E5, TechnoClass_WhatAction_FixGetCellAtCallTwice, 0x9)
{
	GET(CellClass* const, pCell, EAX);
	GET(TechnoClass* const, pThis, ESI);

	R->EBP(pThis->SelectWeapon(pCell));
	return 0x7008FB;
}

// Various call of TechnoClass::SetOwningHouse not respecting overloaded 2nd args fix !

ASMJIT_PATCH(0x7463DC, UnitClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(UnitClass* const, pThis, EDI);
	GET(HouseClass* const, pNewOwner, EBX);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	R->EAX(pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce));
	return 0x7463E6;
}

ASMJIT_PATCH(0x4DBF01, FootClass_SetOwningHouse_FixArgs, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	GET_STACK(HouseClass* const, pNewOwner, 0xC + 0x4);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	//Debug::LogInfo("SetOwningHouse for [%s] announce [%s - %d]", pNewOwner->get_ID(), bAnnounce ? "True" : "False" , bAnnounce);
	bool result = false;
	if (pThis->TechnoClass::SetOwningHouse(pNewOwner, bAnnounce))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pThis);

		for (auto& trail : pExt->LaserTrails)
		{
			if (trail->Type->IsHouseColor)
			{
				trail->CurrentColor = pThis->Owner->LaserColor;
			}
		}

		if (pThis->Owner->IsControlledByHuman())
		{
			// This is not limited to mind control, could possibly affect many map triggers
			// This is still not even correct, but let's see how far this can help us

			pThis->ShouldScanForTarget = false;
			pThis->ShouldEnterAbsorber = false;
			pThis->ShouldEnterOccupiable = false;
			pThis->ShouldLoseTargetNow = false;
			pThis->ShouldGarrisonStructure = false;
			pThis->CurrentTargets.clear();

			if (pThis->HasAnyLink() || pThis->GetTechnoType()->ResourceGatherer) // Don't want miners to stop
				return 0x4DBF13;

			switch (pThis->GetCurrentMission())
			{
			case Mission::Harvest:
			case Mission::Sleep:
			case Mission::Harmless:
			case Mission::Repair:
				return 0x4DBF13;
			}

			pThis->Override_Mission(pThis->GetTechnoType()->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, nullptr, nullptr); // I don't even know what this is, just clear the target and destination for me
		}

		result = true;
	}

	R->AL(result);
	return 0x4DBF0F;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x722440, FakeTiberiumClass::__Spread);
DEFINE_FUNCTION_JUMP(LJMP, 0x7228B0, FakeTiberiumClass::__RecalcSpreadData);
DEFINE_FUNCTION_JUMP(LJMP, 0x722AF0, FakeTiberiumClass::__QueueSpreadAt);
DEFINE_FUNCTION_JUMP(LJMP, 0x722F00, FakeTiberiumClass::__Growth);
DEFINE_FUNCTION_JUMP(LJMP, 0x7233A0, FakeTiberiumClass::__RecalcGrowthData);
DEFINE_FUNCTION_JUMP(LJMP, 0x7235A0, FakeTiberiumClass::__QueueGrowthAt);
DEFINE_FUNCTION_JUMP(LJMP, 0x483780, FakeCellClass::_SpreadTiberium);
DEFINE_FUNCTION_JUMP(LJMP, 0x480A80, FakeCellClass::_Reduce_Tiberium);

DEFINE_FUNCTION_JUMP(LJMP, 0x722770, FakeTiberiumClass::__Initialize_Spread);
DEFINE_FUNCTION_JUMP(LJMP, 0x723260, FakeTiberiumClass::__Initialize_Growth);
DEFINE_FUNCTION_JUMP(LJMP, 0x723510, FakeTiberiumClass::__Clear_Growth);
DEFINE_FUNCTION_JUMP(LJMP, 0x722A20, FakeTiberiumClass::__Clear_Spread);

static void _Initialize_Tiberium_Spread_System()
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
	{
		TiberiumClass::Array->Items[i]->Initialize_Spread();
	}
}

static void _Deinitialize_Tiberium_Spread_System()
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
		TiberiumClass::Array->Items[i]->Clear_Spread();
}

static void _Initialize_Tiberium_Growth_System()
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
		TiberiumClass::Array->Items[i]->Initialize_Growth();
}

static void _Deinitialize_Tiberium_Growth_System()
{
	for (int i = 0; i < TiberiumClass::Array->Count; i++)
		TiberiumClass::Array->Items[i]->Clear_Growth();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x722240, _Initialize_Tiberium_Spread_System);
DEFINE_FUNCTION_JUMP(LJMP, 0x722390, _Deinitialize_Tiberium_Spread_System);
DEFINE_FUNCTION_JUMP(LJMP, 0x722D00, _Initialize_Tiberium_Growth_System);
DEFINE_FUNCTION_JUMP(LJMP, 0x722E50, _Deinitialize_Tiberium_Growth_System);
DEFINE_FUNCTION_JUMP(LJMP, 0x722AB0, TiberiumExtData::Clear_Tiberium_Spread_State);

ASMJIT_PATCH(0x71C84D, TerrainClass_AI_Animated, 0x6)
{
	enum { SkipGameCode = 0x71C8D5 };

	GET(TerrainClass* const, pThis, ESI);

	if (pThis->Type)
	{
		if (pThis->Type->IsAnimated)
		{
			auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
			if (auto pImage = pThis->Type->GetImage())
			{
				if (pThis->Animation.Stage == (pTypeExt->AnimationLength
					.Get(pImage->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)))))
				{
					pThis->Animation.Stage = 0;
					pThis->Animation.Start(0);

					if (pThis->Type->SpawnsTiberium && MapClass::Instance->IsValid(pThis->Location))
					{
						for (int i = 0; i < pTypeExt->GetCellsPerAnim(); i++)
							((FakeCellClass*)MapClass::Instance->GetCellAt(pThis->Location))->_SpreadTiberium_2(pThis, true);

						const int particleIdx = pTypeExt->SpawnsTiberium_Particle;

						if (particleIdx >= 0)
						{
							ParticleSystemClass::Instance->SpawnParticle(ParticleTypeClass::Array->Items[particleIdx], &pThis->Location);
						}
					}
				}
			}
			else { Debug::LogInfo("Terrain [%s] With Corrupted Image !", pThis->Type->ID); }
		}
	}

	return SkipGameCode;
}

// static BuildingClass* IsAnySpysatActive(HouseClass* pThis)
// {
// 	const bool IsCurrentPlayer = pThis->ControlledByCurrentPlayer();
// 	const bool IsCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
// 	const bool IsSpysatActulallyAllowed = !IsCampaign ? pThis == HouseClass::CurrentPlayer() :  IsCurrentPlayer;

// 	//===============reset all
// 	pThis->CostDefensesMult = 1.0;
// 	pThis->CostUnitsMult = 1.0;
// 	pThis->CostInfantryMult = 1.0;
// 	pThis->CostBuildingsMult = 1.0;
// 	pThis->CostAircraftMult = 1.0;
// 	BuildingClass* Spysat = nullptr;
// 	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis);

// 	pHouseExt->Building_BuildSpeedBonusCounter.clear();
// 	pHouseExt->Building_OrePurifiersCounter.clear();
// 	pHouseExt->RestrictedFactoryPlants.clear();
// 	pHouseExt->BattlePointsCollectors.clear();

// 	//==========================
// 	//const bool LowpOwerHouse = pThis->HasLowPower();

// 	for (auto const& pBld : pThis->Buildings)
// 	{
// 		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
// 		{
// 			const auto pExt = BuildingExtContainer::Instance.Find(pBld);
// 			const bool IsLimboDelivered = pExt->LimboID != -1;

// 			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
// 				continue;

// 			if (pBld->TemporalTargetingMe
// 				|| pExt->AboutToChronoshift
// 				|| pBld->IsBeingWarpedOut())
// 				continue;

// 			const bool Online = pBld->IsPowerOnline(); // check power
// 			const auto pTypes = pBld->GetTypes(); // building types include upgrades
// 			const bool Jammered = !pExt->RegisteredJammers.empty();  // is this building jammed

// 			for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin)
// 			{

// 				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(*begin);
// 				//const auto Powered_ = pBld->IsOverpowered || (!PowerDown && !((*begin)->PowerDrain && LowpOwerHouse));

// 				const bool IsBattlePointsCollectorPowered = !pTypeExt->BattlePointsCollector_RequirePower || ((*begin)->Powered && Online);
// 				if (pTypeExt->BattlePointsCollector && IsBattlePointsCollectorPowered)
// 				{
// 					++pHouseExt->BattlePointsCollectors[(*begin)];
// 				}

// 				const bool IsFactoryPowered = !pTypeExt->FactoryPlant_RequirePower || ((*begin)->Powered && Online);

// 				//recalculate the multiplier
// 				if ((*begin)->FactoryPlant && IsFactoryPowered)
// 				{
// 					if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
// 					{
// 						pHouseExt->RestrictedFactoryPlants.emplace(pBld);
// 					}

// 					pThis->CostDefensesMult *= (*begin)->DefensesCostBonus;
// 					pThis->CostUnitsMult *= (*begin)->UnitsCostBonus;
// 					pThis->CostInfantryMult *= (*begin)->InfantryCostBonus;
// 					pThis->CostBuildingsMult *= (*begin)->BuildingsCostBonus;
// 					pThis->CostAircraftMult *= (*begin)->AircraftCostBonus;
// 				}

// 				if(IsSpysatActulallyAllowed && !Spysat) {
// 					//only pick avaible spysat
// 					if (!TechnoExtContainer::Instance.Find(pBld)->AE.DisableSpySat) {
// 						const bool IsSpySatPowered = !pTypeExt->SpySat_RequirePower || ((*begin)->Powered && Online);
// 						if ((*begin)->SpySat && !Jammered && IsSpySatPowered) {
// 							if (IsLimboDelivered || !IsCampaign || pBld->DiscoveredByCurrentPlayer) {
// 								Spysat = pBld;
// 							}
// 						}
// 					}
// 				}

// 				// add eligible building
// 				if (pTypeExt->SpeedBonus.Enabled && Online)
// 					++pHouseExt->Building_BuildSpeedBonusCounter[(*begin)];

// 				const bool IsPurifierRequirePower = !pTypeExt->PurifierBonus_RequirePower || ((*begin)->Powered && Online);
// 				// add eligible purifier
// 				if ((*begin)->OrePurifier && IsPurifierRequirePower)
// 					++pHouseExt->Building_OrePurifiersCounter[(*begin)];
// 			}
// 		}
// 	}

// 	//count them
// 	for (auto& purifier : pHouseExt->Building_OrePurifiersCounter)
// 		pThis->NumOrePurifiers += purifier.second;

// 	return Spysat;
// }

// ASMJIT_PATCH(0x508F79, HouseClass_AI_CheckSpySat, 0x5)
// {
// 	enum
// 	{
// 		ActivateSpySat = 0x509054,
// 		DeactivateSpySat = 0x509002
// 	};

// 	GET(HouseClass*, pThis, ESI);
// 	return IsAnySpysatActive(pThis) ? ActivateSpySat : DeactivateSpySat;
// }

ASMJIT_PATCH(0x474964, CCINIClass_ReadPipScale_add, 0x6)
{
	enum { seteax = 0x47499C, retzero = 0x47498B, seteaxwithEdi = 0x474995 };

	LEA_STACK(char*, buffer, 0x8);
	GET_STACK(const char* const, pSection, STACK_OFFSET(0x28, 0x4));
	GET_STACK(const char* const, pKey, STACK_OFFSET(0x28, 0x8));

	static const auto AdditionalPip =
	{
		GameStrings::Ammo() ,
		GameStrings::Tiberium() ,
		GameStrings::Passengers() ,
		GameStrings::Power() ,
		GameStrings::MindControl() ,
		"Spawner"
	};

	auto it = AdditionalPip.begin();

	for (size_t i = 0; i < AdditionalPip.size(); ++i)
	{
		if (CRT::strcmpi(buffer, *it++) == 0)
		{
			R->EAX(PipScale(i + 1));
			return seteax;
		}
	}

	if (!GameStrings::IsBlank(buffer))
		Debug::INIParseFailed(pSection, pKey, buffer, "Expect Valid PipScaleType !");

	return retzero;
}

ASMJIT_PATCH(0x481180, CellClass_GetInfantrySubPos_InvalidCellPointer, 0x5)
{
	enum { retEmpty = 0x4812EC, retContinue = 0x0, retResult = 0x481313 };
	GET(CellClass*, pThis, ECX);
	//GET_STACK(CoordStruct*, pResult, 0x4);
	GET_STACK(DWORD, caller, 0x0);

	if (!pThis)
	{
		Debug::FatalErrorAndExit("CellClass::GetInfantrySubPos please fix ! caller [0x%x] ", caller);
	}

	return retContinue;
}

ASMJIT_PATCH(0x5194EF, InfantryClass_DrawIt_InAir_NoShadow, 5)
{
	GET(InfantryClass*, pThis, EBP);
	return pThis->Type->NoShadow || pThis->CloakState != CloakState::Uncloaked ? 0x51958A : 0x0;
}

ASMJIT_PATCH(0x746AFF, UnitClass_Disguise_Update_MoveToClear, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->Disguise && pThis->Disguise->WhatAmI() == UnitClass::AbsID ? 0x746A9C : 0;
}

ASMJIT_PATCH(0x4249EC, AnimClass_CreateMakeInf_WeirdAssCode, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pInf = RulesClass::Instance->AnimToInfantry.get_or_default(pThis->Type->MakeInfantry))
	{
		if (auto const pCreatedInf = pInf->CreateObject(pThis->Owner))
		{
			R->EAX(pCreatedInf);
			return 0x424A1F;
		}
	}

	return 0x424B0A;
}

//// 7384C3 ,7385BB UnitClass take damage func
ASMJIT_PATCH(0x73D4DA, UnitClass_Harvest_VeinsStorageAmount, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(FakeCellClass*, pCell, EBP);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	if (storage->m_values.empty())
		storage->m_values.resize(TiberiumClass::Array->Count);

	double amount = 1.0;

	if (pThis->Type->Weeder)
	{
		pCell->RemoveWeed();
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.IncreaseAmount(RulesExtData::Instance()->Veins_PerCellAmount, 0);
		return 0x73D502;
	}

	int tibType = pCell->_GetTiberiumType();
	double cur = storage->GetAmounts();

	if (((double)pThis->Type->Storage - cur) <= 1.0)
	{
		amount = (double)pThis->Type->Storage - cur;
	}

	int reduced = pCell->ReduceTiberium((int)amount);

	if (reduced > 0)
	{
		storage->IncreaseAmount((float)amount, tibType);
		return 0x73D5BE;
	}
	return 0x73D623;
}

ASMJIT_PATCH(0x730E39, GuardCommandClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || pType->Weeder);
	return 0x730E3F;
}

ASMJIT_PATCH(0x736823, UnitClass_AI_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	R->CL(pType->Harvester || pType->Weeder);
	return 0x736829;
}

ASMJIT_PATCH(0x7368C6, UnitClass_Update_WeederMissionMove2, 0x6)
{
	GET(BuildingTypeClass*, pBuildingType, EDX);
	R->CL(pBuildingType->Refinery || pBuildingType->Weeder);
	return 0x7368CC;
}

ASMJIT_PATCH(0x7043E7, TechnoClass_Get_ZAdjustment_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pUnitType, ECX);

	R->AL(pUnitType->Harvester || pUnitType->Weeder);
	return 0x7043ED;
}

//real adjusment place , maybe make this customizable ?
ASMJIT_PATCH(0x70440C, TechnoClass_Get_ZAdjustment_IncludeWeederBuilding, 0x6)
{
	GET(BuildingTypeClass*, pBuildingType, EAX);
	R->CL(pBuildingType->Refinery || pBuildingType->Weeder);
	return 0x704412;
}

ASMJIT_PATCH(0x74097E, UnitClass_MI_Guard_IncludeWeeder, 0x6)
{
	GET(BuildingTypeClass*, pBuilding, ECX);
	R->DL(pBuilding->Refinery || pBuilding->Weeder);
	return 0x740984;
}

ASMJIT_PATCH(0x73D0DB, UnitClass_DrawAt_Oregath_IncludeWeeder, 0x6)
{
	enum { Draw = 0x73D0E9, Skip = 0x73D298 };

	GET(UnitClass*, pUnit, ESI);

	return ((pUnit->Type->Harvester || pUnit->Type->Weeder) && !pUnit->IsHarvesting) ?
		Skip : Draw;
}

ASMJIT_PATCH(0x73D2A6, UnitClass_DrawAt_UnloadingClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pUnitType, EAX);

	R->CL(pUnitType->Harvester || pUnitType->Weeder);
	return 0x73D2AC;
}

//this one dextroy special anim : 741C32
ASMJIT_PATCH(0x73E005, UnitClass_Mi_Unload_PlayBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x73E00B;
}

ASMJIT_PATCH(0x741C32, UnitClass_AssignDestination_DestroyBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->DL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x741C38;
}

//allow `VeinholeMonster` to be placed anywhere flat
DEFINE_JUMP(LJMP, 0x74C688, 0x74C697);

ASMJIT_PATCH(0x6F6BD6, TechnoClass_Limbo_UpdateAfterHouseCounter, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	//const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	//only update the SW once the techno is really not present
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty() && pThis->Owner->CountOwnedAndPresent(pTypeExt->This()) <= 0)
		pThis->Owner->UpdateSuperWeaponsOwned();

	return 0x0;
}

ASMJIT_PATCH(0x6E08DE, TActionClass_SellBack_LimboDelivered, 0x6)
{
	enum { forbidden = 0x6E0907, allow = 0x0 };

	GET(BuildingClass*, pBld, ESI);
	return BuildingExtContainer::Instance.Find(pBld)->LimboID != -1 ?
		forbidden : allow;
}

// ASMJIT_PATCH(0x6ECF67, TeamClass_ChangeHouse_nullptrresult, 0x6)
// {
// 	GET(TeamClass*, pThis, ESI);
// 	GET(int, args, ECX);
// 	GET(FootClass*, pCurMember, EDI);
//
// 	const auto pHouse = HouseClass::FindByCountryIndex(args);
// 	if (!pHouse)
// 	{
// 		const auto nonestr = GameStrings::NoneStr();
// 		Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]",
// 			pThis->OwnerHouse ? pThis->OwnerHouse->get_ID() : nonestr, pThis->OwnerHouse,
// 			pThis->get_ID(), pThis, args);
// 	}
//
// 	pCurMember->SetOwningHouse(pHouse);
// 	R->EBP(pCurMember->NextTeamMember);
// 	return 0x6E96A8;
// }

ASMJIT_PATCH(0x65DD4E, TeamTypeClass_CreateGroub_MissingOwner, 0x7)
{
	//GET(TeamClass*, pCreated, ESI);
	GET(TeamTypeClass*, pType, EBX);

	const auto pHouse = pType->GetHouse();
	if (!pHouse)
	{
		Debug::FatalErrorAndExit("Creating Team[%s] groub without proper Ownership may cause crash , Please check !", pType->ID);
	}

	R->EAX(pHouse);
	return 0x65DD55;
}

ASMJIT_PATCH(0x415302, AircraftClass_MissionUnload_IsDropship, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Destination)
	{
		if (pThis->Type->IsDropship)
		{
			CellStruct nCell = CellStruct::Empty;
			if (pThis->Destination->WhatAmI() != CellClass::AbsID)
			{
				if (auto pTech = flag_cast_to<TechnoClass*, false>(pThis))
				{
					nCell = CellClass::Coord2Cell(pTech->GetCoords());
					if (nCell.IsValid())
					{
						if (auto pCell = MapClass::Instance->TryGetCellAt(nCell))
						{
							for (auto pOccupy = pCell->FirstObject;
								pOccupy;
								pOccupy = pOccupy->NextObject)
							{
								if (pOccupy->WhatAmI() == AbstractType::Building)
								{
									pThis->SetDestination(pThis->GoodLandingZone_(), true);
								}
								else
								{
									pOccupy->Scatter(pThis->GetCoords(), true, true);
								}
							}
						}
					}
				}
			}
		}
		else
		{
			return 0x41531B;
		}
	}

	return 0x41530C;
}

ASMJIT_PATCH(0x450B48, BuildingClass_Anim_AI_UnitAbsorb, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->InfantryAbsorb || pThis->UnitAbsorb);
	return 0x450B4E;
}

ASMJIT_PATCH(0x73B0C5, UnitClass_Render_nullptrradio, 0x6)
{
	GET(TechnoClass*, pContact, EAX);
	return !pContact ? 0x73B124 : 0x0;
}

void __fastcall Draw_Radial_Indicator(bool draw_line, bool adjust_color, CoordStruct coord, ColorStruct rgb, float line_mult, bool a8, bool a9) {
	JMP_STD(0x456980);
}

void __fastcall FakeObjectClass::_DrawRadialIndicator(ObjectClass* pThis, discard_t, int val)
{
	if (auto pTechno = flag_cast_to<TechnoClass*, false>(pThis))
	{
		auto pType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		if (pType->HasRadialIndicator && pTypeExt->AlwayDrawRadialIndicator.Get(!pTechno->Deactivated))
		{
			if (HouseClass::IsCurrentPlayerObserver()
				|| pTechno->Owner->ControlledByCurrentPlayer())
			{
				int nRadius = 0;

				if (pTypeExt->RadialIndicatorRadius.isset())
					nRadius = pTypeExt->RadialIndicatorRadius.Get();
				else if (pType->GapGenerator)
					nRadius = pTypeExt->GapRadiusInCells.Get();
				else
				{
					const auto pWeapons = pTechno->GetPrimaryWeapon();
					if (!pWeapons || !pWeapons->WeaponType)
						return;

					const int range_ = WeaponTypeExtData::GetRangeWithModifiers(pWeapons->WeaponType, pTechno);
					if (range_ <= 0)
						return;

					nRadius = range_ / Unsorted::LeptonsPerCell;;
				}

				if (nRadius > 0)
				{
					const auto Color = pTypeExt->RadialIndicatorColor.Get(pTechno->Owner->Color);

					if (Color != ColorStruct::Empty)
					{
						auto nCoord = pTechno->GetCoords();
						FakeTacticalClass::__DrawRadialIndicator(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
					}
				}
			}
		}
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5DA0, FakeObjectClass::_DrawRadialIndicator)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB188, FakeObjectClass::_DrawRadialIndicator)

DEFINE_PATCH(0x707CF2, 0x55);

ASMJIT_PATCH(0x6D47A6, TacticalClass_Render_Techno, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	// draw line if the techno has target
	 //if(auto pTargetTech = flag_cast_to<ObjectClass*>(pThis->Target))
	 //		Drawing::DrawLinesTo(pTargetTech->GetRenderCoords(), pThis->Location, pThis->Owner->LaserColor);

	if (pThis->InLimbo)
		return 0x0;

	if (auto const pOwner = pThis->SlaveOwner)
	{
		if (!pOwner->IsSelected)
			return 0x0;

		Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
	}

	if (Phobos::Otamaa::IsAdmin)
	{
		if (auto const pOwner = pThis->SpawnOwner)
		{
			if (!pOwner->IsSelected)
				return 0x0;

			Drawing::DrawLinesTo(pOwner->GetRenderCoords(), pThis->Location, pOwner->Owner->Color);
		}
	}

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
		{
			if (!pFoot->BelongsToATeam())
				return 0x0;

			if (auto pTeam = pFoot->Team)
			{
				if (auto const pTeamLeader = pTeam->FetchLeader())
				{

					if (pTeamLeader != pFoot)
						Drawing::DrawLinesTo(pTeamLeader->GetRenderCoords(), pThis->Location, pTeamLeader->Owner->Color);
				}
			}
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x6F5190, TechnoClass_DrawIt_Add, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBound, 0x8);

	auto DrawTheStuff = [&pLocation, &pThis, &pBound](const wchar_t* pFormat)
		{
			auto nPoint = *pLocation;
			//DrawingPart
			RectangleStruct nTextDimension;
			Drawing::GetTextDimensions(&nTextDimension, pFormat, nPoint, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, 4, 2);
			auto nIntersect = RectangleStruct::Intersect(nTextDimension, *pBound, nullptr, nullptr);
			auto nColorInt = pThis->Owner->Color.ToInit();//0x63DAD0

			DSurface::Temp->Fill_Rect(nIntersect, (COLORREF)0);
			DSurface::Temp->Draw_Rect(nIntersect, (COLORREF)nColorInt);
			TextDrawing::Simple_Text_Print_Wide(pFormat, DSurface::Temp.get(), pBound, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt);
		};

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = flag_cast_to<FootClass*, false>(pThis))
		{
			if (auto pTeam = pFoot->Team)
			{
				if (auto const pTeamLeader = pTeam->FetchLeader())
				{
					if (pTeamLeader == pThis)
					{
						DrawTheStuff(L"Team Leader");
					}
				}
			}
		}
	}

	//if(pThis->IsTethered)
	//	DrawTheStuff(L"IsTethered");

	return 0x0;
}

ASMJIT_PATCH(0x40A5B3, AudioDriverStart_AnnoyingBufferLogDisable_A, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	pAudioChannelTag->dwBufferBytes = R->EAX<int>();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::LogInfo("Sound frame size = {} bytes", pAudioChannelTag->dwBufferBytes);

	return 0x40A5C4;
}

ASMJIT_PATCH(0x40A554, AudioDriverStart_AnnoyingBufferLogDisable_B, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	LEA_STACK(DWORD*, ptr, STACK_OFFS(0x40, 0x28));
	pAudioChannelTag->soundframesize1 = R->EAX();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::LogInfo("Sound frame size = {} bytes", pAudioChannelTag->soundframesize1);

	R->EDX(R->EAX());
	R->EAX(ptr);
	return 0x40A56C;
}

ASMJIT_PATCH(0x6DBE35, TacticalClass_DrawLinesOrCircles, 0x9)
{
	ObjectClass** items = !ToggleRadialIndicatorDrawModeClass::ShowForAll ? ObjectClass::CurrentObjects->Items : (ObjectClass**)TechnoClass::Array->Items;
	const int count = !ToggleRadialIndicatorDrawModeClass::ShowForAll ? ObjectClass::CurrentObjects->Count : TechnoClass::Array->Count;

	if (count <= 0)
		return 0x6DBE74;

	ObjectClass** items_end = &items[count];

	for (ObjectClass** walk = items; walk != items_end; ++walk)
	{
		if (*walk)
		{
			if (auto pObjType = (*walk)->GetType())
			{
				if (pObjType->HasRadialIndicator)
				{
					(*walk)->DrawRadialIndicator(1);
				}
				//else if (auto pTechno = flag_cast_to<TechnoClass*>(*walk))
				//{
				//	auto pTypeExt = TechnoTypeExtContainer::Instance.Find((TechnoTypeClass*)(pObjType));

				//	if (pTypeExt->DesignatorRange > 0)
				//	{
				//		if (HouseClass::IsCurrentPlayerObserver()
				//		|| pTechno->Owner->ControlledByCurrentPlayer())
				//		{
				//			int nRadius = pTypeExt->DesignatorRange;
				//			const auto Color = pTypeExt->RadialIndicatorColor.Get(pTechno->Owner->Color);

				//			if (Color != ColorStruct::Empty)
				//			{
				//				auto nCoord = pTechno->GetCoords();
				//				FakeTacticalClass::__DrawRadialIndicator(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
				//			}
				//		}
				//	}
				//}
			}
		}
	}

	return 0x6DBE74;
}

DEFINE_JUMP(LJMP, 0x50BF60, 0x50C04A)// Disable CalcCost mult


ASMJIT_PATCH(0x55B4E1, LogicClass_Update_Veinhole, 0x5)
{
	UpdateAllVeinholes();
	return 0;
}

ASMJIT_PATCH(0x711F60, TechnoTypeClass_GetSoylent_Disable, 0x8)
{
	GET(TechnoTypeClass*, pThis, ECX);

	if (TechnoTypeExtContainer::Instance.Find(pThis)->Soylent_Zero)
	{
		R->EAX(0);
		return 0x712036;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4DB1A0, FootClass_GetMovementSpeed_SpeedMult, 0x6)
{
	GET(FootClass*, pThis, ECX);

	const auto maxSpeed = pThis->GetDefaultSpeed();
	int speedResult = int(maxSpeed * TechnoExtData::GetCurrentSpeedMultiplier(pThis));

	if (pThis->WhatAmI() == UnitClass::AbsID && ((UnitClass*)pThis)->FlagHouseIndex != -1)
	{
		speedResult /= 2;
	}

	R->EAX((int)speedResult);
	return 0x4DB245;
}

ASMJIT_PATCH(0x73730E, UnitClass_Visceroid_HealthCheckRestore, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (!pThis->Destination && !pThis->Locomotor.GetInterfacePtr()->Is_Moving())
	{
		if (pThis->IsRedHP() && (pThis->Type->TiberiumHeal || pThis->HasAbility(AbilityType::TiberiumHeal)))
		{

			if (pThis->GetCell()->LandType != LandType::Tiberium)
			{
				// search tiberium and abort current mission
				pThis->MoveToTiberium(pThis->Type->Sight, false);

				if (pThis->Destination)
				{
					if (pThis->ShouldLoseTargetNow)
						pThis->SetTarget(nullptr);

					pThis->unknown_int_6D4 = -1;
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}

			}
			else
			{
				pThis->unknown_int_6D4 = -1;
				pThis->QueueMission(pThis->IsArmed() ? Mission::Hunt : Mission::Area_Guard, false);
				pThis->NextMission();
				return 0x73741F;
			}
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x73666A, UnitClass_AI_Viscerid_ZeroStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);
	return pType->Strength <= 0 || pThis->DeathFrameCounter > 0 ? 0x736685 : 0x0;
}

//this thing checking shit backwards ,..
ASMJIT_PATCH(0x6D4764, TechnoClass_PsyhicSensor_DisableWhenTechnoDies, 0x7)
{
	GET(TechnoClass*, pThis, ESI);


	if (pThis->InLimbo || pThis->IsCrashing || pThis->IsSinking
		|| (pThis->WhatAmI() == UnitClass::AbsID && ((UnitClass*)pThis)->DeathFrameCounter > 0))
	{
		return 0x6D4793;
	}

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AE.flags.Untrackable || TechnoExtData::IsUntrackable(pThis))
	{
		return 0x6D4793;
	}

	if (pThis->CurrentlyOnSensor())
	{
		return 0x6D478C; //draw dashed line
	}

	return 0x6D4793;
}

// Gives player houses names based on their spawning spot

static int GetPlayerPosByName(const char* pName)
{
	if (pName[0] != '<' || strlen(pName) != 12)
		return -1;

	for (size_t i = 0; i < GameStrings::PlayerAt.size(); ++i)
	{
		if (IS_SAME_STR_(GameStrings::PlayerAt[7u - i], pName))
			return i;
	}

	return -1;
}

ASMJIT_PATCH(0x44F8A6, TechnoClass_FromINI_CreateForHouse, 0x7)
{
	GET(const char*, pHouseName, EAX);

	const int startingPoints = GetPlayerPosByName(pHouseName);

	const int idx = startingPoints != -1
		?
		//Hopefully the HouseIndices is fine
		ScenarioClass::Instance->HouseIndices[startingPoints] :
		HouseClass::FindIndexByName(pHouseName)
		;

	//if (Phobos::Otamaa::IsAdmin)
	//	Debug::LogInfo("%s , With House Index [%d]", pHouseName, idx);

	if (idx == -1)
	{
		Debug::LogInfo("Failed To fetch house index by name of [{}]", pHouseName);
		Debug::RegisterParserError();
	}

	R->EAX(idx);
	return R->Origin() + 0x7;
}
ASMJIT_PATCH_AGAIN(0x74330D, TechnoClass_FromINI_CreateForHouse, 0x7)
ASMJIT_PATCH_AGAIN(0x41B17B, TechnoClass_FromINI_CreateForHouse, 0x7)
ASMJIT_PATCH_AGAIN(0x51FB6B, TechnoClass_FromINI_CreateForHouse, 0x7)

// Skips checking the gamemode or who the player is when assigning houses
DEFINE_JUMP(LJMP, 0x44F8CB, 0x44F8E1)

ASMJIT_PATCH(0x73745C, UnitClass_ReceiveRadio_Parasited_WantRide, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { negativemessage = 0x73746A, continueChecks = 0x737476 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return negativemessage;

	return continueChecks;
}

ASMJIT_PATCH(0x7375B6, UnitClass_ReceiveRadio_Parasited_CanLoad, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { staticmessage = 0x7375C4, continueChecks = 0x7375D0 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return staticmessage;

	return continueChecks;
}

ASMJIT_PATCH(0x4A267D, CreditClass_AI_MissingCurPlayerPtr, 0x6)
{
	if (!HouseClass::CurrentPlayer())
		Debug::FatalError("CurrentPlayer ptr is Missing!");

	return 0x0;
}

ASMJIT_PATCH(0x5FF93F, SpotlightClass_Draw_OutOfboundSurfaceArrayFix, 0x7)
{
	//GET(SpotlightClass*, pThis, EBP);
	GET(int, idx, ECX);

	if (idx > 64)
	{
		//Debug::LogInfo("[0x{}]SpotlightClass with OutOfBoundSurfaceArrayIndex[{}] Fixing!", (void*)pThis, idx);
		idx = 64;
	}

	return 0x0;
}

ASMJIT_PATCH(0x6FFD25, TechnoClass_PlayerAssignMission_Capture_InfantryToBld, 0xA)
{
	GET_STACK(ObjectClass*, pTarget, 0x98 + 0xC);
	GET(TechnoClass*, pThis, ESI);

	if (pThis->WhatAmI() == InfantryClass::AbsID && pTarget && pTarget->WhatAmI() == BuildingClass::AbsID)
	{
		auto pInf = ((InfantryClass*)pThis);
		if (pInf->Type->Assaulter || pInf->Type->Infiltrate || pInf->Type->Engineer)
			return 0x0;

		auto pInfTypeExt = InfantryTypeExtContainer::Instance.Find(pInf->Type);

		if (!pInfTypeExt->VoiceGarrison.empty())
		{
			if (((BuildingClass*)pTarget)->Type->MaxNumberOccupants > 0)
			{
				pThis->QueueVoice(pInfTypeExt->VoiceGarrison[Random2Class::NonCriticalRandomNumber->Random() % pInfTypeExt->VoiceGarrison.size()]);
				return 0x6FFDA5;
			}
		}
	}

	return 0x0;
}

static_assert(offsetof(TechnoClass, Airstrike) == 0x294, "ClassMember Shifted !");

ASMJIT_PATCH(0x737BFB, UnitClass_Unlimbo_SmallVisceroid_DontMergeImmedietely, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pThisType, EAX);

	if (pThisType->SmallVisceroid)
	{
		TechnoExtContainer::Instance.Find(pThis)->MergePreventionTimer.Start(1000);
		return 0x737C38;
	}

	return pThisType->LargeVisceroid ? 0x737C38 : 0x737C0B;
}

// ASMJIT_PATCH(0x6FDB80, TechnoClass_AdjustDamage_Handle, 0x6)
// {
// 	GET(TechnoClass*, pThis, ECX);
// 	GET_STACK(TechnoClass*, pVictim, 0x4);
// 	GET_STACK(WeaponTypeClass*, pWeapon, 0x8);
//
//
// 	R->EAX(damage);
// 	return 0x6FDD35;
// }

ASMJIT_PATCH(0x6FE354, TechnoClass_FireAt_DamageMult, 0x6)
{
	GET(int, damage, EDI);
	GET(TechnoClass*, pThis, ESI);

	int _damage = (int)TechnoExtData::GetDamageMult(pThis, (double)damage);
	R->Stack(0x28, pThis->GetTechnoType());
	R->EDI(_damage);
	R->EAX(_damage);
	return 0x6FE3DF;
}

ASMJIT_PATCH(0x52D36F, RulesClass_init_AIMD, 0x5)
{
	GET(CCFileClass*, pFile, EAX);
	Debug::LogInfo("Init {} file", pFile->GetFileName());
	return 0x0;
}

ASMJIT_PATCH(0x41F783, AITriggerTypeClass_ParseConditionType, 0x5)
{
	GET(const char*, pBuffer, ECX);
	GET(AITriggerTypeClass*, pThis, EBP);

	TechnoTypeClass* result = InfantryTypeClass::Find(pBuffer);
	if (!result)
		result = UnitTypeClass::Find(pBuffer);
	if (!result)
		result = AircraftTypeClass::Find(pBuffer);
	if (!result)
		result = BuildingTypeClass::Find(pBuffer);

	if (Phobos::Otamaa::IsAdmin)
		Debug::LogInfo("Condition Object[{} - {}] for [{}]", pBuffer, result ? result->GetThisClassName() : GameStrings::NoneStrb(), pThis->ID);

	R->ESI(result);
	return 0x41F7DE;
}

ASMJIT_PATCH(0x4CA682, FactoryClass_Total_Techno_Queued_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBX);

	return pObject->GetTechnoType() == pTypeCompare || TechnoExtContainer::Instance.Find(pObject)->Type == pTypeCompare ?
		0x4CA68E : 0x4CA693;
}

ASMJIT_PATCH(0x4FA5B8, HouseClass_BeginProduction_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBP);

	return pObject->GetTechnoType() == pTypeCompare || TechnoExtContainer::Instance.Find(pObject)->Type == pTypeCompare ?
		0x4FA5C4 : 0x4FA5C8;
}

ASMJIT_PATCH(0x4FAB4D, HouseClass_AbandonProduction_GetObjectType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	R->EAX(TechnoExtContainer::Instance.Find(pObject)->Type);
	return R->Origin() + 0x8;
}

ASMJIT_PATCH(0x4CA00D, FactoryClass_AbandonProduction_GetObjectType, 0x9)
{
	//lGET(FactoryClass*, pThis, ESI);
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	const auto pType = TechnoExtContainer::Instance.Find(pObject)->Type;
	//Debug::LogInfo("[{}]Factory with owner [{} - {}] abandoning production of [{}({}) - {}]",
	//	(void*)pThis,
	//	pThis->Owner->get_ID(), (void*)pThis->Owner,
	//	pType->Name, pType->ID, (void*)pObject);

	R->EAX(pType->GetActualCost(pObject->Owner));
	return 0x4CA03D;
}

#include <Ext/Bomb/Body.h>

ASMJIT_PATCH(0x5F9652, ObjectTypeClass_GetAplha, 0x6)
{
	GET(ObjectTypeClass*, pThis, EBX);
	R->CL(pThis->AlphaImageFile[0] && strlen(pThis->AlphaImageFile));
	return 0x5F9658;
}

//  ASMJIT_PATCH(0x6E20AC, TActionClass_DetroyAttachedTechno, 0x8)
//  {
//  	GET(TechnoClass*, pTarget, ESI);
//
//  	if (auto pBld = cast_to<BuildingClass*>(pTarget))
//  	{
//  		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
//  		{
//  			BuildingExtData::LimboKill(pBld);
//  			return 0x6E20D8;
//  		}
//  	}
//
//  	return 0x0;
//  }

// https://bugs.launchpad.net/ares/+bug/895893
ASMJIT_PATCH(0x4DB37C, FootClass_Limbo_ClearCellJumpjet, 0x6)
{
	GET(FootClass*, pThis, EDI);
	auto pCell = pThis->GetCell();

	if (pThis->GetTechnoType()->JumpJet)
	{
		if (pCell->Jumpjet == pThis)
		{
			pCell->TryAssignJumpjet(nullptr);
		}
	}

	//FootClass_Remove_Airspace_ares
	return pCell->MapCoords.IsValid() ? 0x4DB3A4 : 0x4DB3AF;
}

ASMJIT_PATCH(0x73ED40, UnitClass_Mi_Harvest_PathfindingFix, 0x7)
{
	GET(UnitClass*, pThis, EBP);
	LEA_STACK(CellStruct*, closeTo, STACK_OFFSET(0x64, -0x4C));
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x64, -0x54));
	LEA_STACK(CellStruct*, outBuffer, STACK_OFFSET(0x64, -0x3C));

	if (pThis->Type->Teleporter)
		return 0x0;

	auto zone = MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), pThis->Type->MovementZone, pThis->OnBridge);
	R->EAX(MapClass::Instance->NearByLocation(*outBuffer, *cell, pThis->Type->SpeedType, zone, pThis->Type->MovementZone, false, 1, 1, false, false, false, true, *closeTo, false, false));

	return 0x73ED7A;
}

ASMJIT_PATCH(0x62E430, ParticleSystemClass_AddTovector_nullptrParticle, 0x9)
{
	//GET_STACK(DWORD, caller, 0x0);
	GET(ParticleSystemClass*, pThis, ECX);

	if (!pThis)
	{
		// Fuck off
		//Debug::LogInfo("Function [ParticleSystemClass_AddTovector] Has missing pThis Pointer called from [0x%x]", caller);
		return 0x62E4B4;
	}

	return 0x0;
}

#include <VoxClass.h>

#ifndef CRATE_HOOKS
enum class MoveResult : char
{
	cannot, can
};

// what is the boolean return for , heh
static MoveResult CollecCrate(CellClass* pCell, FootClass* pCollector)
{
	if (pCollector && pCell->OverlayTypeIndex > -1)
	{
		const auto pOverlay = OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex];

		if (pOverlay->Crate)
		{
			const auto pCollectorOwner = pCollector->Owner;
			bool force_mcv = false;
			int soloCrateMoney = 0;

			if (SessionClass::Instance->GameMode == GameMode::Campaign || !pCollectorOwner->Type->MultiplayPassive)
			{
				if (pOverlay->CrateTrigger && pCollector->AttachedTag)
				{
					Debug::LogInfo("Springing trigger on crate at {},{}", pCell->MapCoords.X, pCell->MapCoords.Y);
					pCollector->AttachedTag->SpringEvent(TriggerEvent::PickupCrate, pCollector, CellStruct::Empty);
					if (!pCollector->IsAlive)
						return MoveResult::cannot;

					ScenarioClass::Instance->PickedUpAnyCrate = true;
				}

				Powerup data = Powerup::Money;

#pragma region DetermineTheRewardType
				if (pCell->OverlayData < CrateTypeClass::Array.size())
					data = (Powerup)pCell->OverlayData;
				else
				{
					int total_shares = 0;

					StackVector<Powerup, 256> crates {};

					for (size_t i = 0; i < CrateTypeClass::Array.size(); i++)
					{
						auto crate = CrateTypeClass::Array[i].get();

						if (pCell->LandType == LandType::Water && !crate->Naval)
						{
							continue;
						}

						if (!pCell->IsClearToMove(crate->Speed,
							true, true,
							ZoneType::None,
							MovementZone::Normal, -1, true)) continue;

						if (crate->Weight > 0)
						{
							total_shares += crate->Weight;
							crates->push_back((Powerup)i);
						}
					}

					int random = ScenarioClass::Instance->Random.RandomRanged(1, total_shares);
					int share_count = 0;

					for (size_t i = 0; i < crates->size(); i++)
					{
						share_count += CrateTypeClass::Array[(size_t)crates[i]]->Weight;
						if (random <= share_count)
						{
							data = (Powerup)crates[i];
							break;
						}
					}
				}
#pragma endregion

				if (SessionClass::Instance->GameMode != GameMode::Campaign)
				{
					auto pBase = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);

					if (GameModeOptionsClass::Instance->Bases
						&& !pCollectorOwner->OwnedBuildings
						&& pCollectorOwner->Available_Money() > RulesExtData::Instance()->FreeMCV_CreditsThreshold
						&& !pCollectorOwner->OwnedUnitTypes.get_count(pBase->ArrayIndex)
						)
					{
						data = Powerup::Unit;
						force_mcv = true;
					}
					const auto landType = pCell->LandType;

#pragma region EVALUATE_FIST_TIME
					switch ((Powerup)data)
					{
					case Powerup::Unit:
					{
						if (RulesExtData::Instance()->UnitCrateVehicleCap < 0)
							break;

						if (pCollectorOwner->OwnedUnits >= RulesExtData::Instance()->UnitCrateVehicleCap
							|| landType == LandType::Water
							|| landType == LandType::Beach)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Cloak:
					{

						if (!TechnoTypeExtContainer::Instance.Find(pCollector->GetTechnoType())->CloakAllowed || pCollector->CanICloakByDefault() || TechnoExtContainer::Instance.Find(pCollector)->AE.flags.Cloakable)
							data = Powerup::Money;

						break;
					}
					case Powerup::Squad:
					{
						if (pCollectorOwner->OwnedInfantry > 100
							|| landType == LandType::Water
							|| landType == LandType::Beach)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Armor:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.ArmorMultiplier != 1.0)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Speed:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.SpeedMultiplier != 1.0 || pCollector->WhatAmI() == AbstractType::Aircraft)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Firepower:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.FirepowerMultiplier != 1.0 || !pCollector->IsArmed())
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Veteran:
					{
						if (!pCollector->GetTechnoType()->Trainable || pCollector->Veterancy.IsElite())
						{
							data = Powerup::Money;
						}

						break;
					}
					//both of these are useless for AI , really
					case Powerup::Darkness:
					case Powerup::Reveal:
					{
						if (!pCollectorOwner->IsControlledByHuman())
							data = Powerup::Money;

						break;
					}
					default:
						break;
					}
#pragma endregion

					HouseExtData::IncremetCrateTracking(pCollectorOwner, data);

				}
				else if (!pCell->OverlayData)
				{
					soloCrateMoney = RulesClass::Instance->SoloCrateMoney;

					if (pOverlay == RulesClass::Instance->CrateImg)
					{
						pCell->OverlayData = (unsigned char)RulesClass::Instance->SilverCrate;
					}

					if (pOverlay == RulesClass::Instance->WoodCrateImg)
					{
						pCell->OverlayData = (unsigned char)RulesClass::Instance->WoodCrate;
					}

					if (pOverlay == RulesClass::Instance->WaterCrateImg)
					{
						pCell->OverlayData = (unsigned char)RulesClass::Instance->WaterCrate;
					}

					data = (Powerup)pCell->OverlayData;
				}

				MapClass::Instance->Remove_Crate(&pCell->MapCoords);

				if (SessionClass::Instance->GameMode != GameMode::Campaign && GameModeOptionsClass::Instance->Crates)
				{
					MapClass::Instance->Place_Random_Crate();
				}

#pragma region MainAffect
				const auto something = CrateTypeClass::Array[(int)data]->Argument;
				//not always get used same way ?

				auto PlayAnimAffect = [pCell, pCollector, pCollectorOwner](Powerup idx)
					{
						if (const auto pAnimType = CrateTypeClass::Array[(int)idx]->Anim)
						{
							auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }) + 200);

							GameCreate<AnimClass>(pAnimType, loc, 0, 1, 0x600, 0, 0);
						}
					};

				auto PlaySoundAffect = [pCell, pCollector, pCollectorOwner](Powerup idx)
					{
						if (CrateTypeClass::Array[(int)idx]->Sound <= -1)
							return;

						if (pCollectorOwner->ControlledByCurrentPlayer())
						{
							auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
							VocClass::SafeImmedietelyPlayAt(CrateTypeClass::Array[(int)idx]->Sound, &loc, nullptr);
						}
					};

				auto GeiveMoney = [&]()
					{

						Debug::LogInfo("Crate at {},{} contains money", pCell->MapCoords.X, pCell->MapCoords.Y);

						if (!soloCrateMoney)
						{
							const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;
							int crateMax = 900;

							if (nAdd > 0)
								crateMax += ScenarioClass::Instance->Random.RandomFromMax<int>(nAdd);

							soloCrateMoney = ScenarioClass::Instance->Random.RandomRanged((int)something, (int)something + crateMax);
						}

						const auto pHouseDest = pCollectorOwner->ControlledByCurrentPlayer() || SessionClass::Instance->GameMode != GameMode::Campaign
							? pCollectorOwner : HouseClass::CurrentPlayer();

						pHouseDest->TransactMoney(soloCrateMoney);
						if (pCollectorOwner->ControlledByCurrentPlayer())
						{
							auto loc_fly = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
							FlyingStrings::AddMoneyString(true, soloCrateMoney, pHouseDest, AffectedHouse::Owner, loc_fly, Point2D::Empty, ColorStruct::Empty);
						}
						PlaySoundAffect(Powerup::Money);
						PlayAnimAffect(Powerup::Money);
					};

				switch (data)
				{
				case Powerup::Money:
				{
					GeiveMoney();
					break;
				}
				//TODO :
				// this thing confusing !
				case Powerup::Unit:
				{
					Debug::LogInfo("Crate at {},{} contains a unit", pCell->MapCoords.X, pCell->MapCoords.Y);
					UnitTypeClass* Given = nullptr;
					if (force_mcv)
					{
						Given = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);
					}

					if (!Given)
					{
						if ((pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[0]->ArrayIndex) > 0
							|| pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[1]->ArrayIndex) > 0)
						&& !pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[0]->ArrayIndex)
						&& !pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[1]->ArrayIndex)
						)
						{
							Given = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->HarvesterUnit);
						}
					}

					if (RulesClass::Instance->UnitCrateType)
					{
						Given = RulesClass::Instance->UnitCrateType;
					}

					bool finish = false;
					bool currentPlayer = false;
					if (!Given)
					{
						while (true)
						{
							do
							{
								Given = UnitTypeClass::Array->Items[ScenarioClass::Instance->Random.RandomFromMax(UnitTypeClass::Array->Count - 1)];
								int count = 0;

								if (RulesClass::Instance->BaseUnit.Count > 0)
								{
									auto begin = RulesClass::Instance->BaseUnit.begin();
									while (*begin != Given)
									{
										++begin;
										++count;
										if (count >= RulesClass::Instance->BaseUnit.Count)
										{
											finish = false;
											break;
										}
									}

									finish = true;
								}

								currentPlayer = pCollectorOwner->ControlledByCurrentPlayer();
							}
							while (!Given->CrateGoodie || TechnoTypeExtContainer::Instance.Find(Given)->CrateGoodie_RerollChance > 0.0 && TechnoTypeExtContainer::Instance.Find(Given)->CrateGoodie_RerollChance < ScenarioClass::Instance->Random.RandomDouble());

							if (GameModeOptionsClass::Instance->Bases)
								break;

							if (!finish)
								break;
						}(finish && !currentPlayer && !force_mcv);
					}

					if (Given)
					{
						if (auto pCreatedUnit = Given->CreateObject(pCollectorOwner))
						{
							auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
							if (pCreatedUnit->Unlimbo(loc, DirType::Min))
							{
								PlaySoundAffect(Powerup::Unit);
								return MoveResult::cannot;
							}

							auto alternative_loc = MapClass::Instance->NearByLocation(pCell->MapCoords, Given->SpeedType, ZoneType::None, Given->MovementZone, 0, 1, 1, 0, 0, 0, 1, CellStruct::Empty, false, false);

							if (alternative_loc.IsValid())
							{
								if (pCreatedUnit->Unlimbo(CellClass::Cell2Coord(alternative_loc), DirType::Min))
								{
									PlaySoundAffect(Powerup::Unit);
									return MoveResult::cannot;
								}
							}

							GameDelete<true, false>(pCreatedUnit);
							GeiveMoney();
							break;
						}
						else
						{
							PlayAnimAffect(Powerup::Unit);
							return MoveResult::can;
						}
					}
				}
				case Powerup::HealBase:
				{
					Debug::LogInfo("Crate at {},{} contains base healing", pCell->MapCoords.X, pCell->MapCoords.Y);
					PlaySoundAffect(Powerup::HealBase);
					for (int i = 0; i < MapClass::Logics->Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::Logics->Items[i]))
						{
							if (pTechno->IsAlive && pTechno->GetOwningHouse() == pCollectorOwner)
							{
								int heal = pTechno->Health - pTechno->GetTechnoType()->Strength;
								pTechno->ReceiveDamage(&heal, 0, RulesClass::Instance->C4Warhead, 0, 1, 1, nullptr);
							}
						}
					}
					PlayAnimAffect(Powerup::HealBase);
					break;
				}
				case Powerup::Explosion:
				{
					Debug::LogInfo("Crate at {},{} contains explosives", pCell->MapCoords.X, pCell->MapCoords.Y);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
					for (int i = 5; i > 0; --i)
					{
						int scatterDistance = ScenarioClass::Instance->Random.RandomFromMax(512);
						auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
						auto randomCoords = MapClass::GetRandomCoordsNear(loc, scatterDistance, false);
						DamageArea::Apply(&randomCoords, damage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);
						if (auto pAnim = MapClass::SelectDamageAnimation(damage, RulesClass::Instance->C4Warhead, LandType::Clear, randomCoords))
						{
							GameCreate<AnimClass>(pAnim, randomCoords, 0, 1, 0x2600, -15, false);
						}
						MapClass::FlashbangWarheadAt(damage, RulesClass::Instance->C4Warhead, randomCoords);
					}
					PlayAnimAffect(Powerup::Explosion);
					break;
				}
				case Powerup::Napalm:
				{
					Debug::LogInfo("Crate at {},{} contains napalm", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
					auto Collector_loc = (pCollector->GetCoords() + loc) / 2;

					GameCreate<AnimClass>(AnimTypeClass::Array->Items[0], Collector_loc, 0, 1, 0x600, 0, 0);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->FlameDamage, nullptr, 1, false, 0);
					DamageArea::Apply(&Collector_loc, damage, nullptr, RulesClass::Instance->FlameDamage, true, nullptr);

					PlayAnimAffect(Powerup::Napalm);
					return MoveResult::can;
				}
				case Powerup::Darkness:
				{
					Debug::LogInfo("Crate at {},{} contains 'shroud'", pCell->MapCoords.X, pCell->MapCoords.Y);
					MapClass::Instance->Reshroud(pCollectorOwner);
					PlayAnimAffect(Powerup::Darkness);
					break;
				}
				case Powerup::Reveal:
				{
					Debug::LogInfo("Crate at {},{} contains 'reveal'", pCell->MapCoords.X, pCell->MapCoords.Y);
					MapClass::Instance->Reveal(pCollectorOwner);
					PlaySoundAffect(Powerup::Reveal);
					PlayAnimAffect(Powerup::Reveal);
					break;
				}
				case Powerup::Armor:
				{
					Debug::LogInfo("Crate at {},{} contains armor", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE.ArmorMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.ArmorMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitArmorUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Armor);
					PlayAnimAffect(Powerup::Armor);
					break;
				}
				case Powerup::Speed:
				{
					Debug::LogInfo("Crate at {},{} contains speed", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<FootClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->WhatAmI() != AbstractType::Aircraft)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE.SpeedMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.SpeedMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitArmorUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Speed);
					PlayAnimAffect(Powerup::Speed);
					break;
				}
				case Powerup::Firepower:
				{
					Debug::LogInfo("Crate at {},{} contains firepower", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius
									&& TechnoExtContainer::Instance.Find(pCollector)->AE.FirepowerMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.FirepowerMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitFirePowerUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Firepower);
					PlayAnimAffect(Powerup::Firepower);
					break;
				}
				case Powerup::Cloak:
				{
					Debug::LogInfo("Crate at {},{} contains cloaking device", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->IsOnMap)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;

								if ((int)place.Length() < RulesClass::Instance->CrateRadius)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.flags.Cloakable = true;
									AEProperties::Recalculate(pCollector);
								}
							}
						}
					}

					PlayAnimAffect(Powerup::Cloak);
					break;
				}
				case Powerup::ICBM:
				{
					Debug::LogInfo("Crate at {},{} contains ICBM", pCell->MapCoords.X, pCell->MapCoords.Y);

					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::Nuke && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::ICBM);
					return MoveResult::can;
				}
				case Powerup::Veteran:
				{
					Debug::LogInfo("Crate at {},{} contains veterancy(TM)", pCell->MapCoords.X, pCell->MapCoords.Y);
					const int MaxPromotedCount = (int)something;

					if (MaxPromotedCount > 0)
					{
						for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
						{
							if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
							{
								if (pTechno->IsAlive && pTechno->IsOnMap && pTechno->GetTechnoType()->Trainable)
								{
									auto LayersCoords = pTechno->GetCoords();
									auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
									auto place = cellLoc - LayersCoords;

									if ((int)place.Length() < RulesClass::Instance->CrateRadius)
									{
										int PromotedCount = 0;
										if (MaxPromotedCount > 0.0)
										{
											do
											{
												if (pTechno->Veterancy.IsVeteran())
													pTechno->Veterancy.SetElite();
												else
													if (pTechno->Veterancy.IsRookie())
														pTechno->Veterancy.SetVeteran();
													else
														if (pTechno->Veterancy.IsNegative())
															pTechno->Veterancy.SetRookie();

												++PromotedCount;
											}
											while ((double)PromotedCount < MaxPromotedCount);
										}
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Veteran);
					PlayAnimAffect(Powerup::Veteran);
					break;
				}
				case Powerup::Gas:
				{
					Debug::LogInfo("Crate at {},{} contains poison gas", pCell->MapCoords.X, pCell->MapCoords.Y);

					if (auto WH = WarheadTypeClass::Array->get_or_default(WarheadTypeClass::FindIndexById("GAS")))
					{

						bool randomizeCoord = true;
						auto collector_loc = pCell->GetCoords();

						DamageArea::Apply(&collector_loc, (int)something, nullptr, WH, true, nullptr);

						for (int i = 0; i < 8;)
						{
							CellClass* pDestCell = pCell;
							if (randomizeCoord)
							{
								CellStruct dest {};
								MapClass::GetAdjacentCell(&dest, &pCell->MapCoords, (FacingType)i);
								pDestCell = MapClass::Instance->GetCellAt(dest);
							}

							auto damagearea = pDestCell->GetCoords();
							DamageArea::Apply(&damagearea, (int)something, nullptr, WH, true, nullptr);
							randomizeCoord = ++i < 8;
						}
					}

					PlaySoundAffect(Powerup::Gas);
					PlayAnimAffect(Powerup::Gas);
					break;
				}
				case Powerup::Tiberium:
				{
					Debug::LogInfo("Crate at {},{} contains tiberium", pCell->MapCoords.X, pCell->MapCoords.Y);
					int tibToSpawn = ScenarioClass::Instance->Random.RandomFromMax(TiberiumClass::Array->Count - 1);
					if (tibToSpawn == 1)
						tibToSpawn = 0;

					pCell->IncreaseTiberium(tibToSpawn, 1);

					for (int i = ScenarioClass::Instance->Random.RandomRanged(10, 20); i > 0; --i)
					{
						int distance = ScenarioClass::Instance->Random.RandomFromMax(300);
						auto center = pCell->GetCoords();
						auto destLoc = MapClass::GetRandomCoordsNear(center, distance, true);
						MapClass::Instance->GetCellAt(destLoc)->IncreaseTiberium(tibToSpawn, 1);
					}

					PlayAnimAffect(Powerup::Tiberium);
					break;
				}
				case Powerup::Squad:
				{
					Debug::LogInfo("Crate at {},{} contains Squad", pCell->MapCoords.X, pCell->MapCoords.Y);

					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
 {
	 return pSuper->Type->Type == SuperWeaponType::AmerParaDrop && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}
					else
					{
						GeiveMoney();
						break;
					}

					PlayAnimAffect(Powerup::Squad);
					break;
				}
				case Powerup::Invulnerability:
				{
					Debug::LogInfo("Crate at {},{} contains Invulnerability", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::IronCurtain && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::Invulnerability);
					break;
				}
				case Powerup::IonStorm:
				{
					Debug::LogInfo("Crate at {},{} contains IonStorm", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::LightningStorm && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::IonStorm);
					break;
				}
				case Powerup::Pod:
				{
					Debug::LogInfo("Crate at {},{} contains Pod", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
 {
	 return (NewSuperType)pSuper->Type->Type == NewSuperType::DropPod && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::Pod);
					return MoveResult::can;
				}
				default:
					//TODO :: the affects
					Debug::LogInfo("Crate at {},{} contains {}", pCell->MapCoords.X, pCell->MapCoords.Y, CrateTypeClass::Array[(int)data]->Name.data());
					PlaySoundAffect(data);
					PlayAnimAffect(data);
					break;
				}
#pragma endregion
			}
		}
	}

	return MoveResult::can;
}

ASMJIT_PATCH(0x481A00, CellClass_CollectCrate_Handle, 0x6)
{
	GET(CellClass*, pThis, ECX);
	GET_STACK(FootClass*, pCollector, 0x4);
	R->EAX(CollecCrate(pThis, pCollector));
	return 0x483391;
}

ASMJIT_PATCH(0x56BFC2, MapClass_PlaceCrate_MaxVal, 0x5)
{
	return R->EDX<int>() != (int)CrateTypeClass::Array.size()
		? 0x56BFC7 : 0x56BFFF;
}

ASMJIT_PATCH(0x475A44, CCINIClass_Put_CrateType, 0x7)
{
	GET_STACK(int, crateType, 0x8);

	const auto pCrate = CrateTypeClass::FindFromIndexFix(crateType);
	if (!pCrate)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType Pointer for[%d]!", crateType);
	}

	R->EDX(pCrate->Name.data());
	return 0x475A4B;
}
ASMJIT_PATCH(0x475A1F, RulesClass_Put_CrateType, 0x5)
{
	GET(const char*, crate, ECX);

	const int idx = CrateTypeClass::FindIndexById(crate);
	if (idx <= -1)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType index for[%s]!", crate);
	}
	R->EAX(idx);
	return 0x475A24;
}

ASMJIT_PATCH(0x48DE79, CrateTypeFromName, 0x7)
{
	GET(const char*, readedName, EBX);

	const auto type = CrateTypeClass::FindIndexById(readedName);

	if (type != -1)
	{
		R->EDI(type);
		return 0x48DEA2;
	}

	return 0x48DE9C;
}

ASMJIT_PATCH(0x73844A, UnitClass_Destroyed_PlaceCrate, 0x8)
{
	GET(UnitClass*, pThis, ESI);
	GET(CellStruct, cell, EAX);

	const auto CrateType = &TechnoTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CrateType;
	PowerupEffects crate = CrateType->isset() ? (PowerupEffects)CrateType->Get() : (PowerupEffects)CrateTypeClass::Array.size();
	MapClass::Instance->Place_Crate(cell, crate);
	return 0x738457;
}

ASMJIT_PATCH(0x4421F2, BuildingClass_Destroyed_PlaceCrate, 0x6)
{
	//GET(BuildingClass*, pThis, ESI);
	GET(BuildingTypeClass*, pThisType, EDX);
	GET_STACK(CellStruct, cell, 0x10);

	const PowerupEffects defaultcrate = pThisType->CrateBeneathIsMoney ? PowerupEffects::Money : (PowerupEffects)CrateTypeClass::Array.size();
	const auto CrateType = &TechnoTypeExtContainer::Instance.Find(pThisType)->Destroyed_CrateType;
	PowerupEffects crate = CrateType->isset() ? (PowerupEffects)CrateType->Get() : defaultcrate;
	R->EAX(MapClass::Instance->Place_Crate(cell, crate));
	return 0x442226;
}
#endif

#ifndef STORAGE_HOOKS

#ifndef BUILDING_STORAGE_HOOK
// spread tiberium on building destruction. replaces the
// original code, made faster and spilling is now optional.
ASMJIT_PATCH(0x441B30, BuildingClass_Destroy_Refinery, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto& store = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	auto& total = HouseExtContainer::Instance.Find(pThis->Owner)->TiberiumStorage;

	// remove the tiberium contained in this structure from the house's owned
	// tiberium. original code does this one bail at a time, we do bulk.
	if (store.GetAmounts() >= 1.0)
	{
		for (size_t i = 0u; i < (size_t)TiberiumClass::Array->Count; ++i)
		{
			auto const amount = std::ceil(store.GetAmount(i));

			if (amount > 0.0)
			{

				store.DecreaseLevel((float)amount, i);
				total.DecreaseLevel((float)amount, i);

				// spread bail by bail
				if (pExt->TiberiumSpill)
				{
					for (auto j = static_cast<int>(amount); j; --j)
					{
						auto const dist = ScenarioClass::Instance->Random.RandomRanged(256, 768);
						auto const crd = MapClass::GetRandomCoordsNear(pThis->Location, dist, true);

						auto const pCell = MapClass::Instance->GetCellAt(crd);
						pCell->IncreaseTiberium(i, 1);
					}
				}
			}
		}
	}

	if (!TechnoTypeExtContainer::Instance.Find(pThis->Type)->DontShake.Get() && RulesClass::Instance->ShakeScreen)
	{
		int cost = pThis->Type->GetCost();
		ShakeScreenHandle::ShakeScreen(pThis, cost, RulesClass::Instance->ShakeScreen);
	}

	return 0x441C39;
}

ASMJIT_PATCH(0x445FE4, BuildingClass_GrandOpening_GetStorageTotalAmount, 0x6)
{
	GET(FakeBuildingClass*, pThis, EBP);

	if (pThis->_GetTypeExtData()->Refinery_UseNormalActiveAnim)
		return 0x446183;

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x446016;
}

ASMJIT_PATCH(0x450CD7, BuildingClass_AnimAI_GetStorageTotalAmount_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts()))
		result = int(double(amount * TiberiumClass::Array->Count) / (pThis->Type->Storage + 0.5));

	R->EAX(result);
	R->EDX(pThis->Type);
	return 0x450D09;
}

ASMJIT_PATCH(0x450DAA, BuildingClass_AnimAI_GetStorageTotalAmount_B, 0x6)
{
	GET(FakeBuildingClass*, pThis, ESI);

	if (pThis->_GetTypeExtData()->Refinery_UseNormalActiveAnim)
		return 0x446183;

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450DDC;
}

ASMJIT_PATCH(0x450E12, BuildingClass_AnimAI_GetStorageTotalAmount_C, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450E3E;
}

ASMJIT_PATCH(0x4589C0, BuildingClass_storage_4589C0, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x4589DC;
}

ASMJIT_PATCH(0x44A232, BuildingClass_BuildingClass_Destruct_Storage, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto decreaase = storage->DecreaseLevel((float)storage->GetAmount(i), i);
		HouseExtContainer::Instance.Find(pThis->Owner)->TiberiumStorage.DecreaseLevel(decreaase, i);
	}
	return 0x44A287;
}
#endif

//TechnoClass
#ifndef TECHNO_STORAGE_HOOK
#endif

//UnitClass
#ifndef UNIT_STORAGE_HOOK
//UnitClass_CreditLoad
ASMJIT_PATCH(0x7438B0, UnitClass_CreditLoad_Handle, 0xA)
{
	GET(UnitClass*, pThis, ECX);
	int result = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetTotalTiberiumValue() * pThis->Owner->Type->IncomeMult);
	R->EAX((int)result);
	return 0x7438E1;
}

ASMJIT_PATCH(0x73D4A4, UnitClass_Harvest_IncludeWeeder, 0x6)
{
	enum { retFalse = 0x73D5FE, retTrue = 0x73D4DA };
	GET(UnitTypeClass*, pType, EDX);
	GET(UnitClass*, pThis, ESI);
	GET(CellClass*, pCell, EBP);
	const bool canharvest = (pType->Harvester && pCell->LandType == LandType::Tiberium) || (pType->Weeder && pCell->LandType == LandType::Weeds);
	const auto storagesPercent = pThis->GetStoragePercentage();
	const bool canStoreHarvest = storagesPercent < 1.0;

	return canharvest && canStoreHarvest ? retTrue : retFalse;
}

ASMJIT_PATCH(0x73E3BF, UnitClass_Mi_Unload_replace, 0x6)
{
	GET(BuildingClass* const, pBld, EDI);
	GET(UnitClass*, pThis, ESI);

	auto unit_storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	if (!pBld->Type->Weeder)
		HouseExtData::LastHarvesterBalance = pBld->GetOwningHouse()->Available_Money();// Available_Money takes silos into account

	const  auto pType = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	const int idxTiberium = unit_storage->GetFirstSlotUsed();
	float dumpAmount = pType->HarvesterDumpAmount.Get(RulesExtData::Instance()->HarvesterDumpAmount.Get());
	const float amountCanBeRemoved = idxTiberium != -1 ?
		Math::abs((float)unit_storage->GetAmount(idxTiberium)) : 0.0f;//after decreased

	if (dumpAmount > 0.0f)
		dumpAmount = std::min(dumpAmount, amountCanBeRemoved);
	else
		dumpAmount = amountCanBeRemoved;

	if (idxTiberium == -1 || unit_storage->DecreaseLevel((float)dumpAmount, idxTiberium) <= 0.0)
	{
		if (pBld->Type->Refinery)
		{ //weed ???
			pBld->Game_PlayNthAnim(BuildingAnimSlot::Production,
				pBld->GetHealthPercentage_() <= RulesClass::Instance->ConditionYellow,
				false, false);
		}
		pThis->MissionStatus = 4;

		if (pBld->Anims[10])
		{
			pBld->DestroyNthAnim(BuildingAnimSlot::Special);
		}
	}
	else
		if (pBld->Type->Weeder)
		{
			pBld->Owner->GiveWeed((int)dumpAmount, idxTiberium);
			pThis->Animation.Stage = 0;
		}
		else
		{
			TechnoExt_ExtData::DepositTiberium(pBld, pBld->Owner,
			(float)dumpAmount,
			(float)(BuildingTypeExtData::GetPurifierBonusses(pBld->Owner) * dumpAmount),
			idxTiberium
			);
			pThis->Animation.Stage = 0;

			BuildingExtContainer::Instance.Find(pBld)->AccumulatedIncome +=
				pBld->Owner->Available_Money() - HouseExtData::LastHarvesterBalance;

		}

	return 0x73E539;
}

ASMJIT_PATCH(0x708BC0, TechnoClass_GetStoragePercentage_GetTotalAmounts, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	const auto pType = pThis->GetTechnoType();
	double result = 0.0;
	if (pType->Storage > 0)
	{
		result = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() / pType->Storage;
	}

	__asm fld result;
	return 0x708C0A;
}

ASMJIT_PATCH(0x7414A0, UnitClass_GetStoragePercentage_GetTotalAmounts, 0x9)
{
	GET(UnitClass*, pThis, ECX);
	double result = pThis->Type->Harvester || pThis->Type->Weeder ?
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() : 0.0f;

	result /= (double)pThis->Type->Storage;
	__asm fld result;
	return 0x7414DD;
}

ASMJIT_PATCH(0x738749, UnitClass_Destroy_TiberiumExplosive, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;

	if (RulesClass::Instance->TiberiumExplosive
		&& !pThis->Type->Weeder
		&& !ScenarioClass::Instance->SpecialFlags.StructEd.HarvesterImmune
		&& storage->GetAmounts() > 0.0f)
	{
		// multiply the amounts with their powers and sum them up
		int morePower = 0;

		for (int i = 0; i < TiberiumClass::Array->Count; ++i)
		{
			morePower += int(storage->m_values[i] * TiberiumClass::Array->Items[i]->Power);
		}

		if (morePower > 0)
		{

			CoordStruct crd = pThis->GetCoords();
			if (auto pWH = RulesExtData::Instance()->Tiberium_ExplosiveWarhead)
			{
				DamageArea::Apply(&crd, morePower, const_cast<UnitClass*>(pThis), pWH, pWH->Tiberium, pThis->Owner);
			}

			if (auto pAnim = RulesExtData::Instance()->Tiberium_ExplosiveAnim)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, crd, 0, 1, AnimFlag(0x2600), -15, false),
					pThis->Owner,
					nullptr,
					false
				);
			}
		}
	}

	return 0x7387C4;
}
#endif

//IfantryClass
#ifndef INFANTRY_STROAGE_HOOK

ASMJIT_PATCH(0x522E70, InfantryClass_MissionHarvest_Handle, 0x5)
{
	GET(InfantryClass*, pThis, ECX);

	if (pThis->Type->Storage)
	{
		const auto v4 = (FakeCellClass*)pThis->GetCell();
		const auto val = pThis->GetStoragePercentage();

		if (v4->HasTiberium() && val < 1.0)
		{
			if (pThis->SequenceAnim != DoType::Shovel)
			{
				pThis->PlayAnim(DoType::Shovel);
			}

			auto tibType = v4->_GetTiberiumType();
			auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
			const auto amount = storage->GetAmount(tibType);
			double result = 1.0;

			if (((double)pThis->Type->Storage - amount) <= 1.0)
			{
				result = (double)pThis->Type->Storage - amount;
			}

			auto v10 = v4->ReduceTiberium((int)result);

			if (v10 > 0)
			{
				storage->IncreaseAmount((float)v10, tibType);
			}

			R->EAX(pThis->Type->HarvestRate);
			return 0x522EAB;
		}
	}

	pThis->PlayAnim(DoType::Ready);
	pThis->QueueMission(Mission::Guard, false);
	R->EAX(1);

	return 0x522EAB;
}

ASMJIT_PATCH(0x522D50, InfantryClass_StorageAI_Handle, 0x5)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(TechnoClass* const, pDest, 0x4);

	//be carefull , that slave sometime do unload with different owner
	//this can become troublesome later ,..

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	bool updateSmoke = false;
	int balanceBefore = pDest->Owner->Available_Money();

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto const amountRemoved = storage->DecreaseLevel((float)storage->GetAmount(i), i);

		if (amountRemoved > 0.0)
		{
			TechnoExt_ExtData::DepositTiberium(pThis, pDest->Owner, amountRemoved,
				BuildingTypeExtData::GetPurifierBonusses(pDest->Owner) * amountRemoved,
				i);

			// register for refinery smoke
			updateSmoke = true;
		}
	}

	if (updateSmoke)
	{
		pDest->UpdateRefinerySmokeSystems();

		int money = pDest->Owner->Available_Money() - balanceBefore;
		const auto what = pDest->WhatAmI();

		if (what == BuildingClass::AbsID)
		{
			BuildingExtContainer::Instance.Find(static_cast<BuildingClass*>(pDest))->AccumulatedIncome += money;
		}
		else if (what == UnitClass::AbsID && money)
		{
			auto pUnit = static_cast<UnitClass*>(pDest);

			if (pUnit->Type->DeploysInto)
			{
				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pUnit->Type->DeploysInto);

				if (pTypeExt->DisplayIncome.Get(RulesExtData::Instance()->DisplayIncome))
				{
					if (pThis->Owner->IsControlledByHuman() || RulesExtData::Instance()->DisplayIncome_AllowAI)
					{
						FlyingStrings::AddMoneyString(
						money,
						money,
						pThis,
						pTypeExt->DisplayIncome_Houses.Get(RulesExtData::Instance()->DisplayIncome_Houses),
						pThis->GetCoords(),
						pTypeExt->DisplayIncome_Offset, ColorStruct::Empty);
					}
				}
			}
		}
	}

	return 0x522E61;
}
#endif

//HouseClass
#ifndef HOUSE_STORAGE_HOOK
ASMJIT_PATCH(0x4F69D0, HouseClass_AvaibleStorage_GetStorageTotalAmounts, 0x5)
{
	GET(IHouse*, pThis, ESI);
	const int value = *reinterpret_cast<int*>(((DWORD)pThis) + 0x2EC);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	auto pExt = HouseExtContainer::Instance.Find(pHouse);

	R->EAX(value - (int)pExt->TiberiumStorage.GetAmounts());
	return 0x4F69F0;
}

ASMJIT_PATCH(0x4F69A3, HouseClass_AvaibleMoney_GetStorageTotalAmounts, 0x6)
{
	GET(IHouse*, pThis, ESI);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	auto pExt = HouseExtContainer::Instance.Find(pHouse);

	R->EAX(pExt->TiberiumStorage.GetTotalTiberiumValue());
	return 0x4F69AE;
}

ASMJIT_PATCH(0x4F6E70, HouseClass_GetTiberiumStorageAmounts, 0xA)
{
	GET(HouseClass*, pThis, ESI);
	auto pExt = HouseExtContainer::Instance.Find(pThis);

	double result = 0.0;
	const double amount = pExt->TiberiumStorage.GetAmounts();

	if ((int)amount)
	{
		result = amount / double(pThis->TotalStorage);
	}

	__asm fld result;
	return 0x4F6EA2;
}

ASMJIT_PATCH(0x4F8C05, HouseeClass_AI_StorageSpeak_GetTiberiumStorageAmounts, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX((int)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F8C15;
}

ASMJIT_PATCH(0x4F96BF, HouseClass_FindBestStorage_GetStorageTotalAmounts, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX((float)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F96C4;
}

ASMJIT_PATCH(0x4F9790, HouseClass_SpendMoney_Handle, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(int, money, 0x4);

	auto storage = &HouseExtContainer::Instance.Find(pThis)->TiberiumStorage;

	int total = (int)storage->GetAmounts();
	int total_balance = pThis->TotalStorage;
	int credits = pThis->Balance;
	int blance_before = money;

	if (money <= credits)
	{
		pThis->Balance = credits - money;
	}
	else
	{
		blance_before = pThis->Balance;
		int deduced = money - credits;
		pThis->Balance = 0;
		if (deduced > 0 && total > 0.0)
		{
			for (auto& pBld : pThis->Buildings)
			{
				if (pBld)
				{
					auto bldStorage = &TechnoExtContainer::Instance.Find(pBld)->TiberiumStorage;

					if (bldStorage->GetAmounts() > 0.0)
					{
						while (bldStorage->GetAmounts() > 0.0)
						{
							if (deduced <= 0)
								break;

							int used = bldStorage->GetFirstSlotUsed();
							while (bldStorage->GetAmount(used) > 0.0)
							{
								auto decrease_str = bldStorage->DecreaseLevel(1.0, used);
								storage->DecreaseLevel(decrease_str, used);
								int mult = int(TiberiumClass::Array->Items[used]->Value * pThis->Type->IncomeMult * decrease_str);
								deduced -= mult;
								blance_before += mult;
								if (deduced < 0)
								{
									pThis->Balance -= deduced;
									blance_before += deduced;
									deduced = 0;
									break;
								}

								if (deduced <= 0)
									break;
							}
						}
					}
				}

				if (deduced == 0)
					break;
			}
		}
	}

	pThis->UpdateAllSilos(total, total_balance);
	pThis->CreditsSpent += blance_before;
	return 0x4F9941;
}

ASMJIT_PATCH(0x4F99A6, HouseClass_UpdateAllSilos_GetStorageTotalAmounts, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	R->EAX((float)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F99B1;
}

ASMJIT_PATCH(0x502821, HouseClass_RegisterLoss_TiberiumStorage, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	auto storage = &HouseExtContainer::Instance.Find(pThis)->TiberiumStorage;

	for (int i = storage->GetFirstSlotUsed(); i != -1; i = storage->GetFirstSlotUsed())
	{
		auto decreaase = storage->DecreaseLevel(2147483600.0f, i);
		pThis->SiloMoney += int(decreaase * 5.0);
		pThis->TotalStorage += int(TiberiumClass::Array->Items[i]->Value * pThis->Type->IncomeMult * decreaase);
	}

	return 0x5028A7;
}

#endif

ASMJIT_PATCH(0x65DE6B, TeamTypeClass_CreateGroup_IncreaseStorage, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TechnoTypeClass*, pFootType, EDI);
	TechnoExtContainer::Instance.Find(pFoot)->TiberiumStorage.DecreaseLevel((float)pFootType->Storage, 0);
	return 0x65DE82;
}

#endif

//unnessesary call wtf ?
DEFINE_JUMP(LJMP, 0x519211, 0x51922F);

// AttackMove Only for Foot
DEFINE_PATCH_TYPED(BYTE, 0x731B67, 4u);

ASMJIT_PATCH(0x70FF65, TechnoClass_ApplyLocomotorToTarget_CleaupFirst_Crash, 0x6)
{
	GET(TechnoClass*, pFirer, ESI);

	return pFirer->LocomotorTarget ? 0x0 : 0x70FF77;
}

ASMJIT_PATCH(0x6F5EAC, TechnoClass_Talkbuble_playVoices, 0x5)
{
	GET(TechnoClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	if (pTypeExt->TalkbubbleVoices.empty())
		return 0x0;

	const auto& vec = pTypeExt->TalkbubbleVoices[FileSystem::TALKBUBL_Frame() - 1];

	if (!vec.empty())
	{
		auto coord = pThis->GetCoords();
		VocClass::SafeImmedietelyPlayAt(Random2Class::Global->RandomFromMax(vec.size() - 1)
			, &coord, &pThis->Audio3);
	}

	return  0x0;
}

ASMJIT_PATCH(0x7043B9, TechnoClass_GetZAdjustment_Link, 0x6)
{
	GET(TechnoClass*, pNthLink, EAX);
	return pNthLink ? 0 : 0x7043E1;
}

ASMJIT_PATCH(0x6D4A35, TacticalClass_Render_SWText, 0x6)
{
	GET(SuperClass*, pSuper, ECX);
	GET(int, val, EBX);
	GET(int, interval, ESI);

	FakeTacticalClass::__DrawTimersSW(pSuper, val, interval / 15);

	return 0x6D4A70;
}

ASMJIT_PATCH(0x52C5A1, InitGame_SecondaryMixInit, 0x9)
{
	const bool result = R->AL();
	Debug::LogInfo(" ...{} !!!", !result ? "FAILED" : "OK");
	return 0x52C5D3;
}

ASMJIT_PATCH(0x60B865, AdjustWindow_Child, 5)
{
	RECT Rect {};
	Imports::GetWindowRect.invoke()(Game::hWnd(), &Rect);
	R->ESI(R->ESI<int>() - Rect.top);
	R->EDX(R->EDX<int>() - Rect.left);
	return 0;
}

int aval;
int bval;
int cval;

ASMJIT_PATCH(0x61E00C, TrackBarWndProc_AdjustLength, 7)
{
	int v2 = bval * cval / aval;
	R->Stack(0x84, v2);
	R->Stack(0x28, v2 + 12);
	return 0;
}

ASMJIT_PATCH(0x61DA20, TrackbarMsgProc_SetValueRange, 6)
{
	if (R->Stack<int>(0x158) == 15)
	{
		aval = R->EBP<int>();
		bval = R->EBX<int>();
	}
	return 0x0;
}

ASMJIT_PATCH(0x61DA6B, TrackbarMsgProc_GetSlideRange, 7)
{
	if (R->Stack<int>(0x158) == 15)
	{
		cval = R->EAX<int>();
	}
	return 0x0;
}

ASMJIT_PATCH(0x42499C, AnimClass_AnimToInf_CivialHouse, 0x6)
{
	R->EAX(HouseExtData::FindFirstCivilianHouse());
	return 0x4249D8;
}

ASMJIT_PATCH(0x458230, BuildingClass_GarrisonAI_CivilianHouse, 0x6)
{
	R->EBX(HouseExtData::FindFirstCivilianHouse());
	return 0x45826E;
}

ASMJIT_PATCH(0x41ECB0, AITriggerClass_NeutralOwns_CivilianHouse, 0x5)
{
	R->EBX(HouseExtData::FindFirstCivilianHouse());
	return 0x41ECE8;
}

ASMJIT_PATCH(0x50157C, HouseClass_IsAllowedToAlly_CivilianHouse, 0x5)
{
	HouseExtData::FindFirstCivilianHouse();
	R->EAX(RulesExtData::Instance()->CivilianSideIndex);
	return 0x501586;
}

ASMJIT_PATCH(0x47233C, CaptureManagerClass_SetOwnerToCivilianHouse, 0x5)
{
	R->ECX(HouseExtData::FindFirstCivilianHouse());
	return 0x472382;
}

ASMJIT_PATCH(0x6B0AFE, SlaveManagerClass_FreeSlaves_ToCivilianHouse, 0x5)
{
	R->Stack(0x10, HouseExtData::FindFirstCivilianHouse());
	return 0x6B0B3C;
}

ASMJIT_PATCH(0x5A920D, galite_5A91E0_SpecialHouse, 0x5)
{
	R->EAX(HouseExtData::FindSpecial());
	return 0x5A921E;
}

ASMJIT_PATCH(0x449E8E, BuildingClass_Mi_Selling_UndeployLocationFix, 0x5)
{
	GET(BuildingClass*, pThis, EBP);
	CellStruct mapCoords = pThis->InlineMapCoords();

	const short width = pThis->Type->GetFoundationWidth();
	const short height = pThis->Type->GetFoundationHeight(false);

	if (width > 2)
		mapCoords.X += static_cast<short>(std::ceil(width / 2.0) - 1);
	if (height > 2)
		mapCoords.Y += static_cast<short>(std::ceil(height / 2.0) - 1);

	REF_STACK(CoordStruct, location, STACK_OFFSET(0xD0, -0xC0));
	auto coords = (CoordStruct*)&location.Z;
	coords->X = (mapCoords.X << 8) + 128;
	coords->Y = (mapCoords.Y << 8) + 128;
	coords->Z = pThis->Location.Z;

	return 0x449F12;
}

#pragma region Assaulter
//https://blueprints.launchpad.net/ares/+spec/assaulter-veterancy
//522A09
ASMJIT_PATCH(0x522A09, InfantryClass_EnteredThing_Assaulter, 0x6)
{
	enum { retTrue = 0x522A11, retFalse = 0x522A45 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F580
ASMJIT_PATCH(0x51F580, InfantryClass_MissionHunt_Assaulter, 0x6)
{
	enum { retTrue = 0x51F58A, retFalse = 0x51F5C0 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F493
ASMJIT_PATCH(0x51F493, InfantryClass_MissionAttack_Assaulter, 0x6)
{
	enum { retTrue = 0x51F49D, retFalse = 0x51F4D3 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51968E
ASMJIT_PATCH(0x51968E, InfantryClass_UpdatePosition_Assaulter, 0x6)
{
	enum { retTrue = 0x5196A6, retFalse = 0x519698 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//4D4BA0
ASMJIT_PATCH(0x4D4BA0, InfantryClass_MissionCapture_Assaulter, 0x6)
{
	enum { retTrue = 0x4D4BB4, retFalse = 0x4D4BAA };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

ASMJIT_PATCH(0x457DAD, BuildingClass_CanBeOccupied_Assaulter, 0x6)
{
	enum { retTrue = 0x457DD5, retFalse = 0x457DA3 };

	GET(BuildingClass* const, pThis, ESI);
	GET(InfantryClass* const, pInfantry, EDI);

	if (TechnoExtData::IsAssaulter(pInfantry))
	{
		if (!pThis->Owner->IsAlliedWith(pInfantry) && pThis->GetOccupantCount() > 0)
		{
			const auto pBldExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

			// buildings with negative level are not assaultable
			if (pBldExt->AssaulterLevel >= 0)
			{
				// assaultable if infantry has same level or more
				if (pBldExt->AssaulterLevel <= TechnoTypeExtContainer::Instance.Find(pInfantry->Type)->AssaulterLevel)
				{
					return retTrue;
				}
			}
		}
	}

	return retFalse;
}
#pragma endregion

// ASMJIT_PATCH(0x4580CB, BuildingClass_KickAllOccupants_HousePointerMissing, 0x6)
// {
// 	GET(BuildingClass*, pThis, ESI);
// 	GET(FootClass*, pOccupier, EDI);

// 	if (!pThis->Owner)
// 	{
// 		Debug::FatalErrorAndExit("BuildingClass::KickAllOccupants for [%x(%s)] Missing Occupier [%x(%s)] House Pointer !",
// 			pThis,
// 			pThis->get_ID(),
// 			pOccupier,
// 			pOccupier->get_ID()
// 		);
// 	}

// 	return 0x0;
// }

ASMJIT_PATCH(0x449462, BuildingClass_IsCellOccupied_UndeploysInto, 0x6)
{
	enum { PlacingCheck = 0x449493, SkipGameCode = 0x449487 };

	GET(BuildingClass*, pThis, ECX);

	if (pThis->CurrentMission == Mission::None)
		return PlacingCheck;

	GET(BuildingTypeClass*, pType, EAX);
	LEA_STACK(CellStruct*, pDest, 0x4);

	const auto pUndeploysInto = pType->UndeploysInto;
	R->AL(MapClass::Instance->GetCellAt(pDest)
		->IsClearToMove(pUndeploysInto->SpeedType, 0, 0, ZoneType::None, pUndeploysInto->MovementZone, -1, 1)
	);

	return SkipGameCode;
}

#include <SlaveManagerClass.h>

ASMJIT_PATCH(0x6AF588, SlaveManagerClass_Enslave_MissingOriginalOwner, 0xD)
{
	GET(SlaveManagerClass*, pManage, ESI);

	if (pManage->Owner)
		pManage->Owner->SlaveManager = nullptr;

	return 0x6AF595;
}

ASMJIT_PATCH(0x474DEE, INIClass_GetFoundation, 7)
{
	GET_STACK(const char*, Section, 0x2C);
	GET_STACK(const char*, Key, 0x30);
	LEA_STACK(const char*, Value, 0x8);

	if (!IS_SAME_STR_(Value, "Custom") && !FindFoundation(Value))
	{
		Debug::INIParseFailed(Section, Key, Value);
	}

	return 0;
}

ASMJIT_PATCH(0x461225, BuildingTypeClass_ReadFromINI_Foundation, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBP);

	INI_EX exINi(&CCINIClass::INI_Art.get());
	auto pBldext = BuildingTypeExtContainer::Instance.Find(pThis);

	if (pBldext->IsCustom)
	{
		//Reset
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pThis->FoundationData = pBldext->CustomData.data();
		pThis->FoundationOutside = pBldext->OutlineData.data();
	}

	const auto pSection = pThis->ImageFile && pThis->ImageFile[0] && strlen(pThis->ImageFile) ? pThis->ImageFile : pThis->ID;

	detail::read(pThis->Foundation, exINi, pSection, GameStrings::Foundation());

	if (auto pAdd = FindFoundation(Phobos::readBuffer))
	{
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pBldext->IsCustom = true;
		pBldext->CustomWidth = pAdd->Size.X;
		pBldext->CustomHeight = pAdd->Size.Y;

		pBldext->CustomData.assign(pAdd->CellCount + 1, CellStruct::Empty);
		pBldext->OutlineData.assign(pAdd->OutlineCount + 1, CellStruct::Empty);

		for (size_t i = 0; i < pAdd->CellCount; ++i)
		{
			pBldext->CustomData[i] = pAdd->Cells[i];
		}

		for (size_t i = 0; i < pAdd->OutlineCount; ++i)
		{
			pBldext->OutlineData[i] = pAdd->Outline[i];
		}

		pBldext->CustomData[pAdd->CellCount] = BuildingTypeExtData::FoundationEndMarker;
		pBldext->OutlineData[pAdd->OutlineCount] = BuildingTypeExtData::FoundationEndMarker;

	}
	else if (IS_SAME_STR_(Phobos::readBuffer, "Custom"))
	{

		char strbuff[0x80];

		if (pThis->Foundation == BuildingTypeExtData::CustomFoundation)
		{
			//Custom Foundation!
			pBldext->IsCustom = true;

			//Load Width and Height
			detail::read(pBldext->CustomWidth, exINi, pSection, "Foundation.X");
			detail::read(pBldext->CustomHeight, exINi, pSection, "Foundation.Y");

			int outlineLength = exINi->ReadInteger(pSection, "FoundationOutline.Length", 0);

			// at len < 10, things will end very badly for weapons factories
			if (outlineLength < 10)
			{
				outlineLength = 10;
			}

			//Allocate CellStruct array
			const int dimension = pBldext->CustomWidth * pBldext->CustomHeight;

			pBldext->CustomData.assign(dimension + 1, CellStruct::Empty);
			pBldext->OutlineData.assign(outlineLength + 1, CellStruct::Empty);

			using Iter = std::vector<CellStruct>::iterator;

			auto ParsePoint = [](Iter& cell, const char* str) -> void
				{
					int x = 0, y = 0;
					switch (sscanf_s(str, "%d,%d", &x, &y))
					{
					case 0:
						x = 0;
						[[fallthrough]];
					case 1:
						y = 0;
					}
					*cell++ = CellStruct { static_cast<short>(x), static_cast<short>(y) };
				};

			//Load FoundationData
			auto itData = pBldext->CustomData.begin();
			//char key[0x20];

			for (int i = 0; i < dimension; ++i)
			{
				if (exINi->ReadString(pSection, (std::string("Foundation.") + std::to_string(i)).c_str(), Phobos::readDefval, strbuff))
				{
					ParsePoint(itData, strbuff);
				}
				else
				{
					break;
				}
			}

			//Sort, remove dupes, add end marker
			std::sort(pBldext->CustomData.begin(), itData,
			[](const CellStruct& lhs, const CellStruct& rhs)
			{
				if (lhs.Y != rhs.Y)
				{
					return lhs.Y < rhs.Y;
				}
				return lhs.X < lhs.X;
			});

			itData = std::unique(pBldext->CustomData.begin(), itData);
			*itData = BuildingTypeExtData::FoundationEndMarker;
			pBldext->CustomData.erase(itData + 1, pBldext->CustomData.end());

			auto itOutline = pBldext->OutlineData.begin();
			for (size_t i = 0; i < (size_t)outlineLength; ++i)
			{
				if (exINi->ReadString(pSection, (std::string("FoundationOutline.") + std::to_string(i)).c_str(), "", strbuff))
				{
					ParsePoint(itOutline, strbuff);
				}
				else
				{
					//Set end vector
					// can't break, some stupid functions access fixed offsets without checking if that offset is within the valid range
					*itOutline++ = BuildingTypeExtData::FoundationEndMarker;
				}
			}

			//Set end vector
			*itOutline = BuildingTypeExtData::FoundationEndMarker;
			bool hasOrigin = false;
			for (auto begin = pBldext->CustomData.begin(); begin < pBldext->CustomData.end(); begin++) {
				if (begin->X == 0 && begin->Y == 0) {
					hasOrigin = true;
					break;
				}
			}

			if(!hasOrigin){
				Debug::LogInfo("BuildingType {} has a custom foundation which does not include cell 0,0. This breaks AI base building.", pSection);
			}
		}
	}

	return 0x46125D;
}

#include <Misc/BuildingFoundations.h>

ASMJIT_PATCH(0x46152C, BuildingTypeClass_SetOccupy, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBP);

	auto pBldext = BuildingTypeExtContainer::Instance.Find(pThis);

	if (pBldext->IsCustom)
	{
		//Reset
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pThis->FoundationData = pBldext->CustomData.data();
		pThis->FoundationOutside = pBldext->OutlineData.data();

	}
	else
	{
		pThis->FoundationData = BuildingTypeClass::FoundationlinesData[(int)pThis->Foundation].Datas;
		pThis->FoundationOutside = BuildingTypeClass::FoundationOutlinesData[(int)pThis->Foundation].Datas;

		//pThis->FoundationData = FoundationDataStruct::Cells[(int)pThis->Foundation].Datas;
		//pThis->FoundationOutside = FoundationDataStruct::Outlines[(int)pThis->Foundation].Datas;

	}

	CCINIClass::INI_Art->ReadString(pThis->ImageFile, "Buildup", "", Phobos::readBuffer);
	if (strlen(Phobos::readBuffer))
	{
		PhobosCRT::strCopy(pThis->BuildupFile, Phobos::readBuffer);
	}

	return 0x4615B6;
}

ASMJIT_PATCH(0x447110, BuildingClass_Sell_Handled, 0x9)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(int, control, 0x4);

	// #754 - evict Hospital/Armory contents
	TechnoExt_ExtData::KickOutHospitalArmory(pThis);

	BuildingExtContainer::Instance.Find(pThis)->MyPrismForwarding->RemoveFromNetwork(true);

	if (pThis->HasBuildup)
	{

		switch (control)
		{
		case -1:
		{
			if (pThis->GetCurrentMission() != Mission::Selling)
			{

				pThis->QueueMission(Mission::Selling, false);
				pThis->NextMission();
			}

			break;
		}
		case 0:
		{
			if (pThis->GetCurrentMission() != Mission::Selling)
			{
				return 0x04471C2;
			}

			break;
		}
		case 1:
		{
			if (pThis->GetCurrentMission() != Mission::Selling && !pThis->IsGoingToBlow)
			{
				pThis->QueueMission(Mission::Selling, false);
				pThis->NextMission();
			}

			break;
		}
		default:
			break;
		}

		if (!BuildingExtContainer::Instance.Find(pThis)->Silent)
		{
			if (pThis->Owner->ControlledByCurrentPlayer())
			{
				VocClass::PlayGlobal(RulesClass::Instance->GenericClick, Panning::Center, 1.0, 0);
			}
		}

		return 0x04471C2;
	}
	else
	{
		if (pThis->Type->FirestormWall || BuildingTypeExtContainer::Instance.Find(pThis->Type)->Firestorm_Wall)
		{
			//if(const auto pBomb = pThis->AttachedBomb) {
			//	if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get()){
			//		pBomb->Detonate();// Otamaa : detonate may kill the techno before this function
			//		// so this can possibly causing some weird crashes if that happening
			//	}
			//}

			pThis->Limbo();
			pThis->UnInit();
		}
	}

	return 0x04471C2;
}

ASMJIT_PATCH(0x442CCF, BuildingClass_Init_Sellable, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	pThis->IsAllowedToSell = !pThis->Type->Unsellable;
	return 0x0;
}

//building abandon sound 458291
//AbandonedSound
ASMJIT_PATCH(0x458291, BuildingClass_GarrisonAI_AbandonedSound, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	const auto nVal = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
	if (nVal >= 0)
	{
		VocClass::PlayGlobal(nVal, Panning::Center, 1.0, 0);
	}

	return 0x4582AE;
}

ASMJIT_PATCH(0x4431D3, BuildingClass_Destroyed_removeLog, 0x5)
{
	GET(InfantryClass*, pSurvivor, ESI);
	GET_STACK(int, nData, 0x8C - 0x70);
	Debug::Log("Survivor[(%x - %s) - %s] unlimbo OK\n", pSurvivor, pSurvivor->Type->ID, pSurvivor->Owner->Type->ID);

	R->EBP(--nData);
	R->EDX(pSurvivor->Type);
	return 0x4431EB;
}

//443292
//44177E
ASMJIT_PATCH(0x443292, BuildingClass_Destroyed_CreateSmudge_A, 0x6)
{
	GET(BuildingClass*, pThis, EDI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4433F9;
}

ASMJIT_PATCH(0x44177E, BuildingClass_Destroyed_CreateSmudge_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4418EC;
}

ASMJIT_PATCH(0x44E809, BuildingClass_PowerOutput_Absorber, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, powertotal, 0x8);

	for (auto pPas = pThis->Passengers.GetFirstPassenger();
		pPas;
		pPas = flag_cast_to<FootClass*>(pPas->NextObject))
	{

		powertotal += abs(TechnoTypeExtContainer::Instance.Find(pPas->GetTechnoType())
			->ExtraPower_Amount.Get(pThis->Type->ExtraPowerBonus));
	}

	R->Stack(0x8, powertotal);
	return 0x44E826;
}

DEFINE_JUMP(LJMP, 0x4417A7, 0x44180A)

ASMJIT_PATCH(0x700391, TechnoClass_GetCursorOverObject_AttackFriendies, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);
	GET(WeaponTypeClass* const, pWeapon, EBP);

	if (!pType->AttackFriendlies)
		return 0x70039B;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->AttackFriendlies_WeaponIdx != -1)
	{
		const auto pWeapons = pThis->GetWeapon(pTypeExt->AttackFriendlies_WeaponIdx);
		if (!pWeapons || pWeapon != pWeapons->WeaponType)
		{
			return 0x70039B;
		}
	}

	return 0x7003BB;
}

//EvalObject
ASMJIT_PATCH(0x6F7EFE, TechnoClass_EvaluateObject_AttackFriendliesWeapon, 6)
{
	enum { AllowAttack = 0x6F7FE9, ContinueCheck = 0x6F7F0C };
	GET_STACK(int const, nWeapon, 0x14);
	GET(TechnoClass* const, pThis, EDI);

	const auto pType = pThis->GetTechnoType();

	if (!pType->AttackFriendlies)
		return ContinueCheck;

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	return pTypeExt->AttackFriendlies_WeaponIdx <= -1
		|| pTypeExt->AttackFriendlies_WeaponIdx == nWeapon
		? AllowAttack : ContinueCheck;
}

ASMJIT_PATCH(0x51A2EF, InfantryClass_UpdatePosition_Bio_Reactor_Sound, 0x6)
{
	//GET(BuildingClass* const, pBuilding, EDI);
	GET(InfantryClass* const, pThis, ESI);
	LEA_STACK(CoordStruct*, pBuffer, 0x44);

	int sound = pThis->Type->EnterBioReactorSound;
	if (sound <= -1)
		sound = RulesClass::Instance->EnterBioReactorSound;

	auto coord = pThis->GetCoords(pBuffer);
	VocClass::SafeImmedietelyPlayAt(sound, coord, 0);

	return 0x51A30F;
}

ASMJIT_PATCH(0x44DBBC, BuildingClass_Mission_Unload_Leave_Bio_Readtor_Sound, 0x7)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(FootClass* const, pPassenger, ESI);
	LEA_STACK(CoordStruct*, pBuffer, 0x40);

	int sound = pPassenger->GetTechnoType()->LeaveBioReactorSound;
	if (sound == -1)
		sound = RulesClass::Instance->LeaveBioReactorSound;

	auto coord = pThis->GetCoords(pBuffer);
	VocClass::SafeImmedietelyPlayAt(sound, coord, 0);
	return 0x44DBDA;
}

ASMJIT_PATCH(0x447E90, BuildingClass_GetDestinationCoord_Helipad, 0x6)
{
	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(CoordStruct*, pCoord, 0x4);
	GET_STACK(TechnoClass* const, pDocker, 0x8);

	auto const pType = pThis->Type;
	if (pType->Helipad)
	{
		pThis->GetDockCoords(pCoord, pDocker);
		pCoord->Z = 0;
	}
	else if (pType->UnitRepair || pType->Bunker)
	{
		pThis->GetDockCoords(pCoord, pDocker);
	}
	else
	{
		pThis->GetCoords(pCoord);
	}

	R->EAX(pCoord);
	return 0x447F06;
}

DEFINE_JUMP(LJMP, 0x4495FF, 0x44961A);
DEFINE_JUMP(LJMP, 0x449657, 0x449672);

// ASMJIT_PATCH(0x4DA64D, FootClass_Update_IsInPlayField, 0x6)
// {
// 	GET(UnitTypeClass* const, pType, EAX);
// 	return pType->BalloonHover || pType->JumpJet ? 0x4DA655 : 0x4DA677;
// }

ASMJIT_PATCH(0x4145B6, AircraftClass_RenderCrash_, 0x6)
{
	GET(AircraftTypeClass*, pType, ESI);

	if (!pType->MainVoxel.HVA)
	{
		Debug::LogInfo("Aircraft[{}] Has No HVA ! ", pType->ID);
		return 0x4149F6;
	}

	return 0x0;
}

ASMJIT_PATCH(0x467C2E, BulletClass_AI_FuseCheck, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(CoordStruct*, pCoord, ECX);

	R->EAX(BulletExtData::FuseCheckup(pThis, pCoord));

	return 0x467C3A;
}

#include <SlaveManagerClass.h>

ASMJIT_PATCH(0x6EDA50, Team_DoMission_Harvest, 0x5)
{
	GET(Mission, setTo, EBX);
	GET(FootClass*, pMember, ESI);

	if (setTo == Mission::Harvest)
	{
		pMember->EnterIdleMode(false, true);
		return 0x6EDA77;
	}

	return 0x0;
}

// ENTRY  , 42C954 , stack 3C TECHNO , size 7 , return 0
// END 42CB3F 5 , 42CCCB
#include <Misc/PhobosGlobal.h>

#ifdef _aaa
ASMJIT_PATCH(0x42D197, AStarClass_Attempt_Entry, 0x5)
{
	GET_STACK(TechnoClass*, pTech, 0x24);
	GET_STACK(CellStruct*, from, 0x1C);
	GET_STACK(CellStruct*, to, 0x20);

	PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };
	return 0x0;
}

ASMJIT_PATCH(0x42D45B, AStarClass_Attempt_Exit, 0x6)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42D44C, AStarClass_Attempt_Exit, 0x6)

ASMJIT_PATCH(0x42C8ED, AStarClass_FindHierarcial_Exit, 0x5)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42C8E2, AStarClass_FindHierarcial_Exit, 0x5)

static COMPILETIMEEVAL constant_ptr<float, 0x7E3794> _pathfind_adjusment {};

class NOVTABLE FakeAStarPathFinderClass : public AStarPathFinderClass
{
public:

	PathType* __AStarClass__Find_Path(CellStruct* a2,
		CellStruct* dest,
		TechnoClass* a4,
		int* path,
		int max_count,
		MovementZone a7,
		ZoneType cellPath)
	{
		PhobosGlobal::Instance()->PathfindTechno = { a4  ,*a2 , *dest };

		return this->AStarClass__Find_Path(a2, dest, a4, path, max_count, a7, cellPath);
	}
#pragma optimize("", off )
#ifdef _WIP
	bool Find_Path_Hierarchical(CellStruct* from, CellStruct* to, MovementZone mzone, FootClass* foot)
	{
		const double threat = foot ? foot->GetThreatAvoidance() : 0.0;
		const bool avaible = !foot || threat <= 0.00001 ? false : true;
		HouseClass* pHouse = foot ? foot->Owner : nullptr;
		bool continueOP = true;

		for (int idx_star = 2; idx_star >= 0; --idx_star)
		{
			for (int i = 0; i < this->HierarchicalQueue->Count; ++i)
			{
				this->HierarchicalQueue->Heap[i] = 0;
			}
			this->HierarchicalQueue->Count = 0;

			auto zone_from = MapClass::Instance->MapClass_zone_56D3F0(from);
			auto passabilityDataFrom = MapClass::GlobalPassabilityDatas() + zone_from;
			auto pPassabilityFrom = passabilityDataFrom->data[idx_star];
			auto zone_to = MapClass::Instance->MapClass_zone_56D3F0(from);
			auto passabilityDataTo = MapClass::GlobalPassabilityDatas() + zone_to;
			auto pPassabilityTo = passabilityDataTo->data[idx_star];

			const bool isFirst = idx_star == 2;
			const auto next_cost_ptr = !isFirst ? nullptr : this->ints_40_costs[idx_star + 1];
			const auto cur_cost_ptr = this->ints_40_costs[idx_star];
			const auto cur_const_ptr_b = this->ints_4C_costs[idx_star];
			const auto cur_cost_hirarcial_ptr = this->HierarchicalCosts[idx_star];

			cur_cost_ptr[pPassabilityFrom] = this->initedcount;
			cur_cost_ptr[pPassabilityTo] = this->initedcount;

			if (pPassabilityFrom == pPassabilityTo)
			{

				if (!idx_star)
				{
					this->BufferForHierarchicalQueue->Number = 0;
					this->BufferForHierarchicalQueue->Index = pPassabilityFrom;
				}

				this->somearray_BC[500 * idx_star] = pPassabilityFrom;
				this->maxvalues_field_C74[idx_star] = 1;

				if (--idx_star >= 0)
				{
					continue;
				}

				return 1;
			}

			//reset
			this->BufferForHierarchicalQueue->BufferDelta = -1;
			this->BufferForHierarchicalQueue->Index = pPassabilityFrom;
			this->BufferForHierarchicalQueue->Score = 0.0f;
			this->BufferForHierarchicalQueue->Number = 0;

			int ele = this->HierarchicalQueue->Count + 1;
			int ele_shift = ele >> 1;

			if (ele < this->HierarchicalQueue->Capacity)
			{
				for (; ele > 1; ele_shift >>= 1)
				{
					auto pEle = this->HierarchicalQueue->Heap;
					if (pEle[ele_shift]->Score <= 0.0f)
					{
						break;
					}

					this->HierarchicalQueue->Heap[ele] = this->HierarchicalQueue->Heap[ele_shift];
					ele = ele_shift;
				}

				this->HierarchicalQueue->Heap[ele] = this->BufferForHierarchicalQueue;
				++this->HierarchicalQueue->Count;
				if (this->BufferForHierarchicalQueue > this->HierarchicalQueue->MaxNodePointer)
					this->HierarchicalQueue->MaxNodePointer = this->BufferForHierarchicalQueue;
				if (this->BufferForHierarchicalQueue < this->HierarchicalQueue->MinNodePointer)
					this->HierarchicalQueue->MinNodePointer = this->BufferForHierarchicalQueue;
			}

			int _idxstart_here = 1;
			cur_const_ptr_b[pPassabilityFrom] = this->initedcount;
			cur_cost_hirarcial_ptr[pPassabilityFrom] = 0.0;
			AStarQueueNodeHierarchical* first = nullptr;

			if (this->HierarchicalQueue->Count)
			{
				first = this->HierarchicalQueue->Heap[1];
				this->HierarchicalQueue->Heap[1] = this->HierarchicalQueue->Heap[this->HierarchicalQueue->Count];
				this->HierarchicalQueue->Heap[this->HierarchicalQueue->Count--] = 0;
				this->HierarchicalQueue->Heapify();
			}

			if (!first)
				return false;

			int cell_IndexesVecIsEmpty = this->CellIndexesVector[idx_star].Count == 0;

			while (1)
			{
				int _first_idx = first->Index;
				if (_first_idx == pPassabilityTo)
				{
					break;
				}

				auto sub_zone = SubzoneTrackingStruct::Array[0].Items + (24 * idx_star);
				auto conn_begin = sub_zone[_first_idx].SubzoneConnections.Items;
				auto conn_count = sub_zone[_first_idx].SubzoneConnections.Count;
				if (conn_count > 0)
				{
					do
					{
						auto _conn_ = SubzoneTrackingStruct::Array[0].Items + idx_star;
						auto __conn__first = _conn_[conn_begin->unknown_dword_0].unknown_word_18;
						auto __conn__next = _conn_[conn_begin->unknown_dword_0].unknown_dword_1C;
						int zone_ = 0;
						if (avaible)
						{
							zone_ = int(MapClass::Instance->subZone_585F40(pHouse, idx_star, _first_idx, conn_begin->unknown_dword_0) * threat);
						}

						double score = conn_begin->unknown_byte_4 ? 0.001 : 0.0;
						int _vala = conn_begin->unknown_dword_0;
						double adj__ = _pathfind_adjusment[__conn__next] + first->Score + zone_ + score;
						if ((cur_const_ptr_b[conn_begin->unknown_dword_0] != this->initedcount
							|| cur_cost_hirarcial_ptr[conn_begin->unknown_dword_0] > adj__)
							 && (isFirst || next_cost_ptr[__conn__first] == this->initedcount || __conn__next == 1)
							 && MapClass::MovementAdjustArray[(int)mzone][__conn__next] == 1)
						{
							if (cell_IndexesVecIsEmpty)
							{
								goto LABEL_49;
							}

							int ___first_idx = _first_idx;

							if (_vala < _first_idx)
							{
								___first_idx = _vala;
								_vala = _first_idx;
							}

							int idxx_ = _first_idx | (_vala << 16);
							int countxx_ = this->CellIndexesVector[idx_star].Count;
							if (countxx_ < 0)
							{
							LABEL_49:
								auto pBuffer = this->BufferForHierarchicalQueue;
								pBuffer[_idxstart_here].Index = _vala;
								pBuffer[_idxstart_here].BufferDelta = first - pBuffer;
								pBuffer[_idxstart_here].Score = adj__;
								pBuffer[_idxstart_here].Number = first->Number + 1;

								int ele_B = this->HierarchicalQueue->Count + 1;
								int ele_shift_B = ele_B >> 1;

								if (ele_B < this->HierarchicalQueue->Capacity)
								{
									for (; ele_B > 1; ele_shift_B >>= 1)
									{
										auto pEle_B = this->HierarchicalQueue->Heap;
										if (pEle_B[ele_shift_B]->Score <= adj__)
										{
											break;
										}

										this->HierarchicalQueue->Heap[ele_B] = this->HierarchicalQueue->Heap[ele_shift_B];
										ele_B = ele_shift_B;
									}

									this->HierarchicalQueue->Heap[ele_B] = pBuffer;
									++this->HierarchicalQueue->Count;
									if (pBuffer > this->HierarchicalQueue->MaxNodePointer)
										this->HierarchicalQueue->MaxNodePointer = pBuffer;
									if (pBuffer < this->HierarchicalQueue->MinNodePointer)
										this->HierarchicalQueue->MinNodePointer = pBuffer;

								}

								cur_const_ptr_b[_vala] = this->initedcount;
								cur_cost_hirarcial_ptr[_vala] = adj__;
								++_idxstart_here;
							}
							else
							{
								auto v36 = &this->CellIndexesVector[idx_star].Items[countxx_];
								while (*v36 != CellStruct::UnPack(idxx_))
								{
									--countxx_;
									--v36;
									if (countxx_ < 0)
									{
										goto LABEL_49;
									}
								}
							}
						}

						continueOP = conn_count == 1;
						++conn_begin;
						--conn_count;
					}
					while (!continueOP);
				}

				if (this->HierarchicalQueue->Count == 0)
					return false;

				first = this->HierarchicalQueue->Heap[1];
				this->HierarchicalQueue->Heap[1] = this->HierarchicalQueue->Heap[this->HierarchicalQueue->Count];
				this->HierarchicalQueue->Heap[this->HierarchicalQueue->Count] = 0;
				--this->HierarchicalQueue->Count;
				this->HierarchicalQueue->Heapify();

				if (!first)
				{
					return 0;
				}
			}

			if (!first)
			{
				return 0;
			}

			auto _copyFirst = first;
			if (first->BufferDelta != -1)
			{
				do
				{
					cur_cost_ptr[first->Index] = this->initedcount;
					first = &this->BufferForHierarchicalQueue[first->BufferDelta];
				}
				while (first->BufferDelta != -1);
			}

			int num__ = _copyFirst->Number + 1;
			this->maxvalues_field_C74[idx_star] = num__;
			int _num__ = num__ - 1;
			if (_num__ > 0)
			{
				auto __ff = &this->somearray_BC[500 * idx_star + num__];
				do
				{
					*__ff-- = _copyFirst->Index;
					_copyFirst = &this->BufferForHierarchicalQueue[_copyFirst->BufferDelta];
					--_num__;
				}
				while (_num__);
			}

			this->somearray_BC[500 * idx_star] = _copyFirst->Index;
		}

		return 1;
	}

#endif
#pragma optimize("", on )
};

DEFINE_FUNCTION_JUMP(CALL, 0x4CBC31, FakeAStarPathFinderClass::__AStarClass__Find_Path)

#ifdef _WIP
DEFINE_FUNCTION_JUMP(LJMP, 0x42C290, FakeAStarPathFinderClass::Find_Path_Hierarchical)
#endif

ASMJIT_PATCH(0x42C2A7, AStarClass_FindHierarcial_Entry, 0x5)
{
	GET(TechnoClass*, pTech, ESI);
	GET_BASE(CellStruct*, from, 0x8);
	GET_BASE(CellStruct*, to, 0xC);

	if (pTech)
		PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };

	return 0x0;
}

ASMJIT_PATCH(0x42C954, AStarClass_FindPath_Entry, 0x7)
{
	GET_STACK(TechnoClass*, pTech, 0x3C);
	GET_STACK(CellStruct*, from, 0x34);
	GET_STACK(CellStruct*, to, 0x38);

	PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };
	R->ESI(pTech);
	R->EBX(R->EAX());
	return R->EDI<int>() == -1 ? 0x42C963 : 0x42C95F;
}

ASMJIT_PATCH(0x42CCC8, AStarClass_FindPath_Exit, 0x6)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42CB3C, AStarClass_FindPath_Exit, 0x6)
#endif

ASMJIT_PATCH(0x7410D6, UnitClass_CanFire_Tethered, 0x7)
{
	GET(TechnoClass*, pLink, EAX);
	return !pLink ? 0x7410DD : 0x0;
}

ASMJIT_PATCH(0x4FD203, HouseClass_RecalcCenter_Optimize, 0x6)
{
	GET(BuildingClass*, pBld, ESI);
	LEA_STACK(CoordStruct*, pBuffer_2, 0x38);
	LEA_STACK(CoordStruct*, pBuffer, 0x2C);

	const auto coord = pBld->GetCoords();
	*pBuffer = coord;
	*pBuffer_2 = coord;
	R->EBP(R->EBP<int>() + coord.X);
	R->EBX(R->EBX<int>() + coord.Y);
	R->EAX(R->Stack<int>(0x18));
	return 0x4FD228;
}

static COMPILETIMEEVAL void ShakeScreen(GScreenClass* pScreen)
{
	/**
	 *   TibSun style.
	 */

	if (pScreen->ScreenShakeX >= 0)
	{
		if (pScreen->ScreenShakeX > 0)
		{
			pScreen->ScreenShakeX = pScreen->ScreenShakeX - 1;
		}
	}
	else
	{
		pScreen->ScreenShakeX = pScreen->ScreenShakeX + 1;
	}

	if (pScreen->ScreenShakeY >= 0)
	{
		if (pScreen->ScreenShakeY > 0)
		{
			pScreen->ScreenShakeY = pScreen->ScreenShakeY - 1;
		}
	}
	else
	{
		pScreen->ScreenShakeY = pScreen->ScreenShakeY + 1;
	}
}

ASMJIT_PATCH(0x4F4BB9, GSCreenClass_AI_ShakescreenMode, 0x5)
{

	GET(GScreenClass*, pThis, ECX);

	if (RulesExtData::Instance()->ShakeScreenUseTSCalculation)
	{
		ShakeScreen(pThis);
		return 0x4F4BEF;
	}

	return 0x0;
}

static OPTIONALINLINE COMPILETIMEEVAL CoordStruct RelativeCenterCoord { 128 , 128 , 0 };

void FakeBuildingClass::_Spawn_Refinery_Smoke_Particles()
{
	auto pType = this->Type;
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

	if (pTypeExt->RefinerySmokeParticleSystemOne && pType->RefinerySmokeOffsetOne.IsValid() && pType->RefinerySmokeOffsetOne != RelativeCenterCoord)
	{
		auto coord1 = pType->RefinerySmokeOffsetOne + this->Location;
		auto particle1 = GameCreate<ParticleSystemClass>(pTypeExt->RefinerySmokeParticleSystemOne, coord1, nullptr, this);
		particle1->Lifetime = pType->RefinerySmokeFrames;
	}

	if (pTypeExt->RefinerySmokeParticleSystemTwo && pType->RefinerySmokeOffsetTwo.IsValid() && pType->RefinerySmokeOffsetTwo != RelativeCenterCoord)
	{
		auto coord2 = pType->RefinerySmokeOffsetTwo + this->Location;
		auto particle2 = GameCreate<ParticleSystemClass>(pTypeExt->RefinerySmokeParticleSystemTwo, coord2, nullptr, this);
		particle2->Lifetime = pType->RefinerySmokeFrames;
	}

	if (pTypeExt->RefinerySmokeParticleSystemThree && pType->RefinerySmokeOffsetThree.IsValid() && pType->RefinerySmokeOffsetThree != RelativeCenterCoord)
	{
		auto coord3 = pType->RefinerySmokeOffsetThree + this->Location;
		auto particle3 = GameCreate<ParticleSystemClass>(pTypeExt->RefinerySmokeParticleSystemThree, coord3, nullptr, this);
		particle3->Lifetime = pType->RefinerySmokeFrames;
	}

	if (pTypeExt->RefinerySmokeParticleSystemFour && pType->RefinerySmokeOffsetFour.IsValid() && pType->RefinerySmokeOffsetFour != RelativeCenterCoord)
	{
		auto coord4 = pType->RefinerySmokeOffsetFour + this->Location;
		auto particle4 = GameCreate<ParticleSystemClass>(pTypeExt->RefinerySmokeParticleSystemFour, coord4, nullptr, this);
		particle4->Lifetime = pType->RefinerySmokeFrames;
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4324, FakeBuildingClass::_Spawn_Refinery_Smoke_Particles);

#include <VoxelIndex.h>

bool FakeUnitClass::_Paradrop(CoordStruct* pCoords)
{
	if (!this->ObjectClass::SpawnParachuted(*pCoords))
	{
		return false;
	}

	auto pExt = TechnoExtContainer::Instance.Find(this);
	if (pExt->Is_DriverKilled || !RulesExtData::Instance()->AssignUnitMissionAfterParadropped)
		return true;

	if (this->Type->ResourceGatherer || this->Type->Harvester)
	{
		this->QueueMission(Mission::Harvest, false);
	}
	else if (this->IsArmed())
	{
		this->QueueMission(Mission::Hunt, false);
	}
	else if (this->Owner->IsControlledByHuman())
	{
		this->QueueMission(Mission::Guard, false);
	}
	else
	{
		this->QueueMission(Mission::Area_Guard, false);
	}

	return true;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D58, FakeUnitClass::_Paradrop);

#include <Notifications.h>

ASMJIT_PATCH(0x453E02, BuildingClass_Clear_Occupy_Spot_Skip, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(CellClass*, pCell, EAX);

	ObjectClass* pObject = pCell->FirstObject;

	do
	{
		if (pObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Building:
			{
				if (pObject != pTechno)
				{
					// skip change the OccFlag of this cell
					return 0x453E12;
				}
				break;
			}
			}
		}
	}
	while (pObject && (pObject = pObject->NextObject) != nullptr);

	return 0;
}

ASMJIT_PATCH(0x418072, AircraftClass_Mission_Attack_PickAttackLocation, 0x5)
{
	GET(AircraftClass*, pAir, ESI);

	if (!pAir->Type->MissileSpawn && !pAir->Type->Fighter && !pAir->Is_Strafe())
	{
		AbstractClass* pTarget = pAir->Target;

		int weaponIdx = pAir->SelectWeapon(pTarget);
		if (pAir->IsCloseEnough(pTarget, weaponIdx))
		{
			pAir->IsLocked = true;
			CoordStruct pos = pAir->GetCoords();
			CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);
			pAir->SetDestination(pCell, true);
			return 0x418087;
		}
		else if (WeaponTypeClass* pWeapon = pAir->GetWeapon(weaponIdx)->WeaponType)
		{
			int dest = pAir->DistanceFrom(pAir->Target);
			CoordStruct nextPos = CoordStruct::Empty;

			if (dest < pWeapon->MinimumRange)
			{
				CoordStruct flh = CoordStruct::Empty;
				flh.X = (int)(pWeapon->Range * 0.5);
				nextPos = TechnoExtData::GetFLHAbsoluteCoords(pAir, flh, true);
			}
			else if (dest > pWeapon->Range)
			{
				int length = (int)(pWeapon->Range * 0.5);
				int flipY = 1;

				if (ScenarioClass::Instance->Random.RandomRanged(0, 1) == 1)
				{
					flipY *= -1;
				}

				CoordStruct sourcePos = pAir->GetCoords();
				int r = (dest - length) * Unsorted::LeptonsPerCell;
				r = ScenarioClass::Instance->Random.RandomRanged(0, r);
				CoordStruct flh { 0, r * flipY, 0 };
				CoordStruct targetPos = pAir->Target->GetCoords();
				DirStruct dir = Helpers_DP::Point2Dir(sourcePos, targetPos);
				sourcePos = Helpers_DP::GetFLHAbsoluteCoords(sourcePos, flh, dir);
				sourcePos.Z = 0;
				targetPos.Z = 0;
				nextPos = Helpers_DP::GetForwardCoords(targetPos, sourcePos, length);
			}
			if (!nextPos.IsEmpty())
			{
				CellClass* pCell = MapClass::Instance->TryGetCellAt(nextPos);
				pAir->SetDestination(pCell, true);
				return 0x418087;
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4181CF, AircraftClass_Mission_Attack_FlyToPostion, 0x5)
{
	GET(AircraftClass*, pAir, ESI);
	if (!pAir->Type->MissileSpawn && !pAir->Type->Fighter)
	{
		pAir->MissionStatus = 0x4; // AIR_ATT_FIRE_AT_TARGET0
		return 0x4181E6;
	}

	return 0;
}

DEFINE_JUMP(LJMP, 0x4184FC, 0x418506);
// ASMJIT_PATCH(0x4184FC, AircraftClass_Mission_Attack_Fire_Zero, 0x6) {
// 	return 0x418506;
// }

ASMJIT_PATCH(0x4CDCFD, FlyLocomotionClass_MovingUpdate_HoverAttack, 0x7)
{
	GET(FlyLocomotionClass*, pFly, ESI);

	AircraftClass* pAir = cast_to<AircraftClass*, false>(pFly->LinkedTo);

	if (pAir && !pAir->Type->MissileSpawn && !pAir->Type->Fighter && !pAir->Is_Strafe() && pAir->CurrentMission == Mission::Attack)
	{
		if (AbstractClass* pDest = pAir->Destination)
		{
			CoordStruct sourcePos = pAir->GetCoords();
			int dist = pAir->DistanceFrom(pDest);

			if (dist < 64 && dist >= 16)
			{
				CoordStruct targetPos = pDest->GetCoords();
				sourcePos.X = targetPos.X;
				sourcePos.Y = targetPos.Y;
				dist = 0;
			}

			if (dist < 16)
			{
				R->Stack(0x50, sourcePos);
			}
		}
	}
	return 0;
}

#include <WeaponTypeClass.h>

ASMJIT_PATCH(0x4FD95F, HouseClass_CheckFireSale_LimboID, 0x6)
{
	GET(BuildingClass*, pBld, EAX);
	return BuildingExtContainer::Instance.Find(pBld)->LimboID != -1 ? 0x4FD983 : 0x0;
}

// ASMJIT_PATCH(0x4C2A02, Ebolt_DTOR_TechnoIsNotTechno, 0x6)
// {
// 	GET(TechnoClass*, pTr, ECX);
// 	const auto vtable = VTable::Get(pTr);

// 	if (vtable != AircraftClass::vtable
// 		&& vtable != UnitClass::vtable
// 		&& vtable != BuildingClass::vtable
// 		&& vtable != InfantryClass::vtable
// 		)
// 	{
// 		return R->Origin() + 0x6; //skip setting ebolt for the techno because it corrupted pointer
// 	}

// 	return 0x0;
// }ASMJIT_PATCH_AGAIN(0x4C2C19, Ebolt_DTOR_TechnoIsNotTechno, 0x6)

ASMJIT_PATCH(0x674028, RulesClass_ReadLandTypeData_Additionals, 0x7)
{
	GET(CCINIClass*, pINI, EDI);
	GET(const char**, pSection_iter, ESI);
	INI_EX ex_INI(pINI);
	RulesExtData::Instance()->LandTypeConfigExts[PhobosGlobal::Instance()->LandTypeParseCounter].Bounce_Elasticity.Read(ex_INI, *pSection_iter, "Bounce.Elasticity");
	Debug::LogInfo("Reading LandTypeData of [{} - {}]", *pSection_iter, PhobosGlobal::Instance()->LandTypeParseCounter);
	++PhobosGlobal::Instance()->LandTypeParseCounter;
	return 0;
}

ASMJIT_PATCH(0x4AED70, Game_DrawSHP_WhoCallMe, 0x6)
{
	GET(ConvertClass*, pConvert, EDX);
	GET_STACK(SHPStruct*, pSHP, 0xA4);
	GET_STACK(DWORD, caller, 0x0);

	if (!pConvert)
	{
		auto pSHPref = pSHP->AsReference();
		Debug::FatalErrorAndExit("Draw SHP[%s] missing Convert , caller [%0x]", pSHPref ? pSHPref->Filename : "unknown", caller);
	}

	return 0x0;
}

//ASMJIT_PATCH(0x4AED70, Game_DrawSHP_WhoCallMe, 0x6)
//{
//	GET(ConvertClass*, pConvert, EDX);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pConvert) {
//		Debug::FatalErrorAndExit("Draw SHP missing Convert , caller [%0x]" , caller);
//	}
//
//	return 0x0;
//}
ASMJIT_PATCH(0x42CB61, AstarClass_Find_Path_FailLog_Hierarchical, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Hierarchical findpath failure: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CB86;
}

ASMJIT_PATCH(0x42CBC9, AstarClass_Find_Path_FailLog_WithoutHierarchical, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Warning.  A* without HS: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CBE6;
}

ASMJIT_PATCH(0x42CC48, AstarClass_Find_Path_FailLog_FindPath, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CellStruct, cellFrom, 0x14);
	GET_STACK(CellStruct, cellTo, 0x10);
	Debug::LogInfo("[{} - {}][{}][{}] Regular findpath failure: ({},{}) to ({}, {})", (void*)pFoot, pFoot->get_ID(), pFoot->GetThisClassName(), pFoot->Owner->get_ID(), cellFrom.X, cellFrom.Y, cellTo.X, cellTo.Y);
	return 0x42CC6D;
}

//DEFINE_JUMP(LJMP, 0x052CAD7, 0x52CAE9);
//ASMJIT_PATCH(0x50B6F0, HouseClass_Player_Has_Control_WhoTheFuckCalling, 0x5)
//{
//	GET(HouseClass*, pHouyse, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pHouyse)
//		Debug::FatalError("Fucking no House %x", caller);
//
//	return 0x0;
//}

ASMJIT_PATCH(0x5F5A56, ObjectClass_ParachuteAnim, 0x7)
{
	GET(CoordStruct*, pCoord, EDI);
	GET(ObjectClass*, pThis, ESI);

	AnimClass* pParach = nullptr;
	bool IsBullet = false;

	if (auto pBullet = cast_to<BulletClass*, false>(pThis))
	{
		IsBullet = true;
		auto pParach_type = ((FakeBulletClass*)pBullet)->_GetTypeExtData()->Parachute.Get(RulesClass::Instance->BombParachute);

		pParach = GameCreate<AnimClass>(pParach_type, pCoord, 0, 1, AnimFlag::AnimFlag_600, 0, false);

	}
	else
	{

		auto coord = *pCoord;
		coord.Z += 75;
		auto pParach_type = RulesClass::Instance->Parachute;

		if (const auto pTechno = flag_cast_to<TechnoClass*, false>(pThis))
		{
			auto pType = pTechno->GetTechnoType();
			auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pTypeExt->IsBomb)
				pThis->IsABomb = true;

			pParach_type = pTypeExt->ParachuteAnim ? pTypeExt->ParachuteAnim : HouseExtData::GetParachuteAnim(pTechno->Owner);
		}

		pParach = GameCreate<AnimClass>(pParach_type, coord);
	}

	pThis->Parachute = pParach;

	if (pParach)
	{
		bool AllowRemap = !IsBullet;
		HouseClass* pOwn = pThis->GetOwningHouse();

		pParach->SetOwnerObject(pThis);

		if (IsBullet)
		{
			auto pTypeExt = BulletTypeExtContainer::Instance.Find(((BulletClass*)pThis)->Type);
			AllowRemap = pTypeExt->Parachuted_Remap;

			if (AllowRemap)
			{
				auto pExt = BulletExtContainer::Instance.Find((BulletClass*)pThis);
				pOwn = ((BulletClass*)pThis)->Owner ? ((BulletClass*)pThis)->Owner->Owner : pExt->Owner;
			}
		}

		const int idx = pOwn ? pOwn->ColorSchemeIndex : RulesExtData::Instance()->AnimRemapDefaultColorScheme;

		if (AllowRemap && idx >= 0)
		{
			pParach->LightConvert = ColorScheme::Array->Items[idx]->LightConvert;
			pParach->TintColor = pThis->GetCell()->Color1.Red;
		}
	}

	return 0x5F5B36;
}

ASMJIT_PATCH(0x687000, ScenarioClass_CheckEmptyUIName, 0x5)
{
	if (!strlen(ScenarioClass::Instance->UIName))
	{
		sprintf_s(ScenarioClass::Instance->UIName, "MISSINGMAPUINAME");
		const auto name = PhobosCRT::StringToWideString(ScenarioClass::Instance->FileName);
		swprintf_s(ScenarioClass::Instance->UINameLoaded, L"%ls Missing UI Name", name.c_str());
	}

	return 0x0;
}

//ASMJIT_PATCH(0x6F89D1, TechnoClass_EvaluateCell_DeadTechno, 0x6)
//{
//	GET(ObjectClass*, pCellObj, EDI);
//
//	if (pCellObj && !pCellObj->IsAlive)
//		pCellObj = nullptr;
//
//	R->EDI(pCellObj);
//
//	return 0x0;
//}

// ASMJIT_PATCH(0x51C251, InfantryClass_CanEnterCell_InvalidObject, 0x8)
// {
// 	GET(ObjectClass*, pCellObj, ESI);
//
// 	if (!pCellObj->IsAlive)
// 	{
// 		return 0x51C78F;
// 	}
//
// 	return R->ESI() == R->EBP() ? 0x51C70F : 0x51C259;
// }

ASMJIT_PATCH(0x417CC0, AircraftClass_WhatAction_caller, 0x5)
{
	GET(AircraftClass*, pThis, ECX);
	GET_STACK(DWORD, caller, 0);

	if (!pThis->IsAlive)
		Debug::LogInfo(__FUNCTION__" DeadTechno[{}] is used , called from [{}]", (void*)pThis, (unsigned)caller);

	return 0x0;
}

ASMJIT_PATCH(0x6B7759, SpawnManagerClass_AI_State4And3_DeadTechno, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(int, idx, EBX);

	if (!pThis->SpawnedNodes.Items[idx]->Unit || !pThis->SpawnedNodes.Items[idx]->Unit->IsAlive)
	{
		pThis->SpawnedNodes.Items[idx]->Status = SpawnNodeStatus::Dead;
		pThis->SpawnedNodes.Items[idx]->Unit = nullptr;

		if (!pThis->SpawnedNodes.Items[idx]->Unit->IsAlive)
			pThis->SpawnedNodes.Items[idx]->NodeSpawnTimer.Start(pThis->RegenRate);

		return 0x6B727F;
	}

	return 0x0;
}ASMJIT_PATCH_AGAIN(0x6B770D, SpawnManagerClass_AI_State4And3_DeadTechno, 0x7)

//ASMJIT_PATCH(0x6F7CA0, TechnoClass_EvalObject_EarlyObjectEval, 0x5)
//{
//	GET_STACK(AbstractClass*, pTarget, 0x10);
//	retfunc_fixed<bool> _return (R, 0x6F8958, false);
//
//	if(!pTarget) {
//		return _return();
//	}
//
//	if (auto pObj = flag_cast_to<ObjectClass* , false>(pTarget)) {
//		if (!pObj->IsAlive) {
//			return _return();
//		}
//	}
//
//	if (const auto pTechno = flag_cast_to<TechnoClass*, false>(pTarget))
//	{
//		if (pTechno->IsCrashing || pTechno->IsSinking) {
//			return _return();
//		}
//
//
//		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());
//
//		if (pTypeExt->IsDummy) {
//			return _return();
//		}
//
//		switch (pTechno->WhatAmI())
//		{
//		case AbstractType::Building:
//		{
//			const auto pBld = (BuildingClass*)pTarget;
//
//			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) {
//				return _return();
//			}
//
//			break;
//		}
//		case AbstractType::Unit:
//		{
//
//			const auto pUnit = (UnitClass*)pTarget;
//
//			if (pUnit->DeathFrameCounter > 0) {
//				return _return();
//			}
//
//			break;
//		}
//		default:
//			break;
//		}
//	}
//
//	return 0x0;
//}

ASMJIT_PATCH(0x6B7867, SpawnManagerClass_AI_MoveTo7ifDies, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(TechnoClass*, pSpawnee, EDI);
	GET(int, idx, EBX);

	if (!pSpawnee)
	{
		pThis->SpawnedNodes.Items[idx]->Status = SpawnNodeStatus::Dead;
		return 0x6B727F;
	}

	return 0x0;
}

ASMJIT_PATCH(0x6B71E7, SpawnManagerClass_Manage_AlreadyNull, 0xA)
{
	GET(SpawnNode*, pNode, EDX);

	if (pNode->Unit && pNode->Unit->IsAlive)
	{
		pNode->Unit->UnInit(); // call detach function for everyone
	}

	return 0x6B71F1;
}

ASMJIT_PATCH(0x451932, BuildingClass_AnimLogic_Ownership, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, _X, ECX);
	GET(int, _Y, EDX);
	GET(int, _Z, EAX);
	GET(int, animIdx, EBP);
	GET_STACK(int, delay, 0x48);

	CoordStruct coord { _X , _Y , _Z };
	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(AnimTypeClass::Array->Items[animIdx]);

	auto pAnim = GameCreate<AnimClass>(AnimTypeClass::Array->Items[animIdx], coord, delay, 1, AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_1000, 0, 0);
	if (!pTypeExt->NoOwner)
	{
		((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
		pAnim->SetHouse(pThis->Owner);
	}

	((FakeAnimClass*)pAnim)->_GetExtData()->ParentBuilding = pThis;
	R->EBP(pAnim);
	return 0x45197B;
}

ASMJIT_PATCH(0x466834, BulletClass_AI_TrailerAnim, 0x6)
{
	GET(BulletClass* const, pThis, EBP);
	const int delay = pThis->Type->ScaledSpawnDelay ? pThis->Type->ScaledSpawnDelay : pThis->Type->SpawnDelay;

	if (delay < 0)
		return 0x4668BD;

	if (!(Unsorted::CurrentFrame % delay))
	{

		auto const pExt = BulletExtContainer::Instance.Find(pThis);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner) ? pExt->Owner : nullptr,
			pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner,
			false,
			false
		);
	}

	return 0x4668BD;
}

#include <OverlayClass.h>

ASMJIT_PATCH(0x48724F, CellClass_PlaceTiberiumAt_RandomMax, 0x9)
{
	GET(CellClass*, pThis, ESI);
	GET(TiberiumClass*, pTib, EDI);
	GET(OverlayClass*, pMemory, EBP);
	const int random = ScenarioClass::Instance->Random.RandomFromMax(pTib->NumImages - 1);
	pMemory->OverlayClass::OverlayClass(OverlayTypeClass::Array->Items[pTib->Image->ArrayIndex + random], pThis->MapCoords, -1);
	R->Stack(0x10, pThis->MapCoords);
	return 0x487291;
}

//ASMJIT_PATCH(0x7C8B3D, game_Dele_whoCall, 0x9)
//{
//	GET_STACK(void* , ptr , 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("Caller 0x%x \n" , caller);
//	CRT::free(ptr);
//	return 0x007C8B47;
//}
//
//ASMJIT_PATCH(0x7C93E8, game_freeMem_caller, 0x5) {
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("CRT::free Caller 0x%x \n", caller);
//	return 0x0;
//}
//ASMJIT_PATCH(0x5C5070, DVC_NoneNameType_clear_, 0x6)
//{
//	GET(DynamicVectorClass<NodeNameType*>*, pThis, ECX);
//
//	pThis->Count = 0;
//	if (pThis->Items && pThis->IsAllocated)
//	{
//		Debug::Log("Caller DVC_NoneNameType_clear_ \n");
//		CRT::free(pThis->Items);
//
//		pThis->Items = nullptr;
//	}
//	pThis->IsAllocated = 0;
//	pThis->Capacity = 0;
//	return 0x5C5099;
//}

//#pragma optimize("", off )
//std::string _tempName = GameStrings::NoneStr();
//ASMJIT_PATCH(0x69E149, SHPStruct_deleteptr_check_getName, 0x5)
//{
//	GET(SHPStruct*, ptr, ESI);
//	_tempName = ptr->AsReference()->Filename;
//	return 0x0;
//}
//int count_ = 0;
//ASMJIT_PATCH(0x69E1EC, SHPStruct_deleteptr_check, 0x6)
//{
//	GET(SHPStruct*, ptr, ESI);
//	Debug::Log("Caller SHPStruct_deleteptr_check deleting [%d][0x%x][%s]\n", count_++,ptr , _tempName.c_str());
//
//	if (count_ == 251)
//	{
//		auto as_ = ptr->AsReference();
//		DebugBreak();
//	}
//
//
//	_tempName = GameStrings::NoneStr();
//	CRT::free(ptr);
//	return 0x69E1F5;
//}
//#pragma optimize("", on )

ASMJIT_PATCH(0x581646, MapClass_CollapseCliffs_DefaultAnim, 0x5)
{
	R->Stack(0x1C, RulesExtData::Instance()->XGRYMED1_);//med1
	R->Stack(0x28, RulesExtData::Instance()->XGRYMED2_);//med2
	R->EDX(RulesExtData::Instance()->XGRYSML1_);//0x2C sml
	return 0x58168F;
}

ASMJIT_PATCH(0x581D4E, MapClass_CollapseCliffs_DefaultAnimB, 0x5)
{
	R->Stack(0x20, RulesExtData::Instance()->XGRYMED1_);//med1
	R->Stack(0x24, RulesExtData::Instance()->XGRYMED2_);//med2
	R->EDX(RulesExtData::Instance()->XGRYSML1_);//0x2C sml
	return 0x581D97;
}

//ASMJIT_PATCH(0x7B4940, WString_OperatorSet_empty, 0x5)
//{
//	GET_STACK(Wstring*, pString, 0x4);
//	GET_STACK(DWORD, caller , 0x0);
//
//	if (!pString)
//		Debug::FatalError("Empty String set for wstring caller  %x\n", caller);
//
//	return 0x0;
//}

ASMJIT_PATCH(0x4F671D, HouseClass_CanAfforBase_MissingPointer, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EAX);

	if (!pBld)
	{
		Debug::FatalErrorAndExit("Cannot Find BuildWeapons For [%s - %ls] , BuildWeapons Count %d\n", pThis->Type->ID, pThis->Type->UIName, RulesClass::Instance->BuildWeapons.Count);
	}

	return 0x0;
}

//ASMJIT_PATCH(0x7BB350, XSurface_DrawSurface_InvalidSurface, 0x5)
//{
//	GET(XSurface*, pThis, ESI);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pThis ||
//		(
//			VTable::Get(pThis) != XSurface::vtable &&
//			VTable::Get(pThis) != DSurface::vtable &&
//			VTable::Get(pThis) != BSurface::vtable &&
//			VTable::Get(pThis) != Surface::vtable
//
//		))
//		Debug::FatalError("Invalid XSurface Caller %x\n", caller);
//
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x7086C3, HouseClass_Is_Attacked_Exclude, 0x6)
//{
//	GET(UnitClass*, pCandidate, ESI);
//
//
//	return 0x70874C;//increment
//}

//ASMJIT_PATCH(0x4E0052, FootClass_TryBunkering, 0x5)
//{
//	GET(FootClass*, pThis, EDI);
//	GET(TechnoClass*, pRecipient, ESI);
//
//	if (!pThis->__ProtectMe_3CF) {
//		if (pThis->SendCommand(RadioCommand::RequestLink, pRecipient) == RadioCommand::AnswerPositive)
//		{
//			//check it first
//			return 0x4E005F;
//		}
//	}
//
//	return 0x4E003B;
//}

//bunker state AI 458E50

ASMJIT_PATCH(0x7084E9, HouseClass_BaseIsAttacked_StopRecuiting, 0x6)
{
	GET(UnitClass*, pCandidate, EBX);
	bool allow = true;

	if (pCandidate->IsTethered)
	{
		allow = false;
	}
	else if (auto pContact = pCandidate->GetRadioContact())
	{
		if (auto pBldC = cast_to<BuildingClass*, false>(pContact))
		{
			if (pBldC->Type->Bunker)
				allow = false;
		}
	}
	else if (auto pBld = pCandidate->GetCell()->GetBuilding())
	{
		if (pBld->Type->Bunker)
			allow = false;
	}

	return allow ? 0x0 : 0x708622;//continue
}

//ASMJIT_PATCH(0x4DEBB0, ObjectClass_Crash_Probe, 0x6)
//{
//	GET(ObjectClass*, pThis, ECX);
//
//	//return false case
//	if(pThis->GetHeight() <= 0 || Unsorted::ScenarioInit())
//		Debug::FatalError("Crash Probe ssomething is wrong\n");
//
//	return 0x0;
//}

// ASMJIT_PATCH(0x41D9A0, AirstrikClass_Setup, 0x6)
// {
// 	//GET(AirstrikeClass*, pThis, EDI);
// 	GET(BuildingClass*, pTarget, ESI);

// 	pTarget->IsAirstrikeTargetingMe = true;
// 	pTarget->Mark(MarkType::Redraw);

// 	return 0x41DA0B;
// }

int FakeUnitClass::_Mission_Attack()
{
	if (this->BunkerLinkedItem && this->Target)
	{
		auto err = this->GetFireError(this->Target, this->SelectWeapon(this->Target), false);

		if (err == FireError::CANT || err == FireError::RANGE)
		{

			this->SetTarget(nullptr);
			this->EnterIdleMode(false, 1u);

			auto control = this->GetCurrentMissionControl();
			double rate = control->Rate * 900.0;
			return (int)rate + ScenarioClass::Instance->Random.RandomRanged(0, 2);
		}
	}

	return FootClass::Mission_Attack();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x7447A0, FakeUnitClass::_Mission_Attack);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5E80, FakeUnitClass::_Mission_Attack);

static void ProcessColorAdd(CCINIClass* pINI)
{
	const int count = pINI->GetKeyCount(GameStrings::ColorAdd);

	if (count > 0)
	{
		struct temp_rgb
		{
			byte r, g, b;

			operator ColorStruct()
			{
				return *reinterpret_cast<ColorStruct*>(this);
			}

			operator byte* ()
			{
				return reinterpret_cast<byte*>(this);
			}
		};

		//this was for debugging purposes
		//the code below can be simplified
		std::vector<temp_rgb> v_buffer(count);

		for (size_t i = 0; i < v_buffer.size(); ++i)
		{
			pINI->Read3Bytes((v_buffer[i]).operator unsigned char* ()
				, GameStrings::ColorAdd
				, pINI->GetKeyName(GameStrings::ColorAdd, i)
				, (v_buffer[i]).operator unsigned char* ());
		}

		if (v_buffer.size() >= RulesClass::Instance->ColorAdd.size())
		{
			Debug::LogInfo("Attempt to read ColorAdd more than array size {}", count);
			Debug::RegisterParserError();
		}

		for (size_t a = 0; a < RulesClass::Instance->ColorAdd.size(); ++a)
		{
			RulesClass::Instance->ColorAdd[a] = v_buffer[a];
		}

	}
	else
	{
		Debug::FatalErrorAndExit("Empty ColorAdd\n");
	}
}

ASMJIT_PATCH(0x668C24, RulesClass_Process_ColorAdd, 0x6)
{
	GET(CCINIClass*, pINI, ESI);
	ProcessColorAdd(pINI);
	return 0x668C8B;
}

ASMJIT_PATCH(0x668B29, RulesClass_Init_ColorAdd, 0x6)
{
	GET(CCINIClass*, pINI, EDI);
	ProcessColorAdd(pINI);
	return 0x668B8E;
}

ASMJIT_PATCH(0x50CA12, HouseClass_RecalcCenter_DeadTechno, 0xA)
{
	enum { NextLoop = 0x50CAB4, ContinueCheck = 0x0 };
	GET(FootClass*, pTechno, ESI);

	if (!pTechno->IsAlive || pTechno->InLimbo || pTechno->BunkerLinkedItem)
		return NextLoop;

	return ContinueCheck;
}

//ASMJIT_PATCH(0x42C2B8 , FootClass_Find_Path_Hirarcial_Dies, 0x7){
//	GET(FootClass* , pFoot , ESI);
//
//	if(!pFoot->IsAlive || pFoot->IsCrashing || pFoot->IsSinking)
//		return 0x42C2CF;
//
//	R->EAX(pFoot->GetThreatAvoidance());
//	return 0x42C2BF;
//}

//#include <TubeClass.h>
//static int idx_pathfind;
//static DWORD calladdr;
//
//CellStruct* __fastcall TubeFacing_429780(CellStruct* pRet, CellStruct* pLoc , int idx, int* path) {
//
//	if (size_t(idx) < 24)
//	{
//		for (int count = idx; count > 0; --count)
//		{
//			for (auto begin = path; begin < (path + 24); ++begin)
//			{
//				if (size_t(*begin) > 7u)
//				{
//					const int TubeIndex = MapClass::Instance->GetCellAt(pLoc)->TubeIndex;
//					if ((size_t)TubeIndex < TubeClass::Array.size())
//					{
//						*pRet = TubeClass::Array.Items[TubeIndex]->ExitCell;
//						return pRet;
//					}
//				}
//				else
//				{
//					*pRet = pLoc->operator+(CellSpread::AdjacentCell[*begin]);
//					return pRet;
//				}
//			}
//		}
//	}
//	else {
//		Debug::Log("%x FindPath with idx %d : in %d\n", calladdr , idx , idx_pathfind);
//	}
//
//	*pRet = *pLoc;
//	return pRet;
//}
//
//DEFINE_FUNCTION_JUMP(LJMP, 0x429780, TubeFacing_429780);
//
//ASMJIT_PATCH(0x4D3920, FootClass_basic_rememberIdx, 0x5)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(int, idx, 0x8);
//
//	if(size_t(idx) > 24 ){
//		idx_pathfind = idx;
//		calladdr = caller;
//	}
//
//	return 0x0;
//}
//#pragma optimize("", off )
//ASMJIT_PATCH(0x4D3E5A, FootClass_basic_idkCrash, 0x5)
//{
//	GET(int, idx, EBX);
//	GET(FootClass*, pFoot, EBP);
//	GET(PathType*, pFind, EDI);
//	LEA_STACK(PathType*, pPath, 0x4C);
//	std::memcpy(pPath, pFind, sizeof(PathType));
//
//	pFoot->FixupPath((DWORD)pPath);
//
//	int _idx = 24 - idx;
//	if (pPath->Length < _idx)
//		_idx = pPath->Length;
//
//	LEA_STACK(int*, _dummy, 0x6C);
//
//	if (size_t(_idx) > 24) {
//		Debug::Log("Pathfind for ([%x]%s - %s) trying to find path with overflow index %d !\n" , pFoot, pFoot->get_ID() , pFoot->GetThisClassName() , _idx);
//		//_idx = pPath->Length;
//		//idx = 0; //??
//	}
//
//	std::memcpy(&pFoot->PathDirections[idx],  _dummy, 4 * _idx);
//	return 0x4D3EA1;
//}
//#pragma optimize("", on )

//ASMJIT_PATCH(0x5D4E3B, DispatchingMessage_ReloadResources, 0x5)
//{
//	LEA_STACK(LPMSG, pMsg, 0x10);
//
//	if (pMsg->message == 16 || pMsg->message == 2 || pMsg->message == 0x112 && pMsg->wParam == 0xF060)
//		ExitProcess(1u);
//
//	//const bool altDown = (pMsg->lParam & 0x20000000) != 0;
//	//if ((pMsg->message == 0x104 || pMsg->message == 0x100) && pMsg->wParam == 0xD && altDown) {}
//
//	TranslateMessage(pMsg);
//	DispatchMessageA(pMsg);
//	return 0x5D4E4D;
//}

ASMJIT_PATCH(0x6FBB35, TechnoClass_CloakingAI_detachsensed, 0x6)
{
	GET(TechnoClass*, pTechno, EDI);
	GET(TechnoClass*, pThis, ESI);

	if (!pTechno || pTechno->Target != pThis || !pTechno->Owner)
		return 0x6FBBC3; // to next check

	if (!pTechno->IsAlive || pTechno->IsCrashing || pTechno->IsSinking)
		return 0x6FBBC3;

	return 0x6FBB3D;
}

// class NOVTABLE FakeLayerClass : LayerClass
// {
// public:


// };
// static_assert(sizeof(FakeLayerClass) == sizeof(LayerClass), "Invalid Size !");

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E607C, FakeLayerClass::_Submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x55BABB, FakeLayerClass::_Submit);
//DEFINE_FUNCTION_JUMP(CALL, 0x4A9759, FakeLayerClass::_Submit);

class NOVTABLE FakeDriveLocomotionClass final : DriveLocomotionClass
{
public:

	bool __stdcall _Is_Moving_Now()
	{
		if (!this->Owner || !this->Owner->IsAlive)
			return false;

		if (this->Owner->PrimaryFacing.Is_Rotating())
			return true;

		return this->Is_Moving()
			&& this->HeadToCoord.IsValid()
			&& this->Owner->GetCurrentSpeed() > 0;
	}
};

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E7F30, FakeDriveLocomotionClass::_Is_Moving_Now);


#pragma region ElectricAssultStuffs

void ElectrictAssaultCheck(FootClass* pThis, bool updateIdleAction)
{
	if (pThis->Target)
		return;

	auto pWeapon = pThis->GetWeapon(1);

	if (pWeapon && pWeapon->WeaponType && pWeapon->WeaponType->Warhead->ElectricAssault)
	{

		auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->WeaponType->Warhead);
		auto myLoc = pThis->GetMapCoords();

		for (int i = 0; i < 8; ++i)
		{
			if (auto pBld = MapClass::Instance->GetCellAt(myLoc + CellSpread::AdjacentCell[i])->GetBuilding())
			{
				if (pBld->Type->Overpowerable && pBld->Owner == pThis->Owner)
				{

					if (pWHExt->ElectricAssault_Requireverses && pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(pBld, pWeapon->WeaponType->Warhead))
					.Verses <= 0.0)
						continue;

					pThis->SetTarget(pBld);
					pThis->__AssignNewThreat = true;
					pThis->QueueMission(Mission::Attack, false);
					return;
				}
			}
		}

	}
	else if (updateIdleAction)
	{
		pThis->UpdateIdleAction();
	}
}

ASMJIT_PATCH(0x4D6F38, FootClass_ElectricAssultFix_SetWeaponType, 0x6)
{
	GET(FootClass*, pThis, ESI);
	ElectrictAssaultCheck(pThis, false);
	return 0x4D7025;
}

ASMJIT_PATCH(0x4D50E1, FootClass_MI_Guard_ElectrictAssault, 0xA)
{
	GET(FootClass*, pThis, ESI);
	ElectrictAssaultCheck(pThis, true);
	return 0x4D5225;
}

ASMJIT_PATCH(0x691A32, ReadScenarion_RemoveInline, 0x5)
{
	LEA_STACK(char*, pName, 0x18);
	R->ESI(GameCreate<ScriptTypeClass>(pName));
	return 0x691B01;
}

ASMJIT_PATCH(0x691C62, ScriptTypeClass_CreateFromName_RemoveInline, 0x5)
{
	GET(char*, pName, EDI);
	R->ESI(GameCreate<ScriptTypeClass>(pName));
	return 0x691D2C;
}

// ASMJIT_PATCH(0x534849, Game_Destroyvector_SpawnManage, 0x6)
// {
// 	for (int i = 0; i < SpawnManagerClass::Array->Count; ++i)
// 	{
// 		if (auto pManager = SpawnManagerClass::Array->Items[i])
// 		{
//
// 			if (VTable::Get(pManager) != 0x7F3650)
// 				continue;
//
// 			pManager->~SpawnManagerClass();
// 		}
// 	}
//
// 	return 0x53486B;
// }

//#pragma optimize("", off )
//ASMJIT_PATCH(0x6ED155, TMissionAttack_WhatTarget, 0x5) {
//	GET(FootClass*, pTeam, EDI);
//	GET(TeamClass*, pThis, EBP);
//	GET(ThreatType, threat, EAX);
//	LEA_STACK(CoordStruct*, pCoord, 0x18);
//
//	auto pTarget = pTeam->GreatestThreat(threat, pCoord, (bool)R->CL());
//
//	if (IS_SAME_STR_("HTNK", pTeam->get_ID()) && flag_cast_to<TechnoClass*>(pTarget))
//		Debug::Log("HTNK Target %s - %s \n", pTarget->GetThisClassName() , ((TechnoClass*)pTarget)->get_ID());
//
//	pThis->AssignMissionTarget(pTarget);
//
//	return 0x6ED16C;
//}
//#pragma optimize("", on )

#ifndef disabled_
// ASMJIT_PATCH(0x6F9C80, TechnoClass_GreatestThread_DeadTechno, 0x9)
// {

// 	GET(TechnoClass*, pThis, ESI);

// 	auto pTechno = TechnoClass::Array->Items[R->EBX<int>()];

// 	if (!pTechno->IsAlive)
// 	{
// 		//Debug::LogInfo("TechnoClass::GreatestThread Found DeadTechno[{} - {}] on TechnoArray!", (void*)pTechno, pTechno->get_ID());
// 		return  0x6F9D93; // next
// 	}

// 	R->ECX(pThis->Owner);
// 	R->EDI(pTechno);
// 	return 0x6F9C89;//contunye
// }

// ASMJIT_PATCH(0x6F91EC, TechnoClass_GreatestThreat_DeadTechnoInsideTracker, 0x6)
// {
// 	GET(TechnoClass*, pTrackerTechno, EBP);

// 	if (!pTrackerTechno->IsAlive)
// 	{
// 		//Debug::LogInfo("Found DeadTechno[{} - {}] on AircraftTracker!", (void*)pTrackerTechno, pTrackerTechno->get_ID());
// 		return 0x6F9377; // next
// 	}

// 	return 0x0;//contunye
// }

WeaponTypeClass* GetWeaponType(TechnoClass* pThis, int which)
{
	WeaponTypeClass* pBuffer = nullptr;

	if (which == -1)
	{
		auto const pType = pThis->GetTechnoType();

		if (pType->TurretCount > 0 || pType->WeaponCount > 2)
		{
			if (auto const pCurWeapon = pThis->GetWeapon(pThis->CurrentGattlingStage))
			{
				pBuffer = pCurWeapon->WeaponType;
			}
		}
		else
		{
			if (auto const pPriStruct = pThis->GetWeapon(0))
			{
				pBuffer = pPriStruct->WeaponType;
			}

			if (auto const pSecStruct = pThis->GetWeapon(1))
			{
				pBuffer = pSecStruct->WeaponType;
			}
		}
	}
	else
	{
		if (auto const pSelected = pThis->GetWeapon(which))
		{
			pBuffer = pSelected->WeaponType;
		}
	}

	return  pBuffer;
}

// ASMJIT_PATCH(0x6F9039, TechnoClass_GreatestThreat_GuardRange, 0x9)
// {
// 	GET(TechnoClass*, pTechno, ESI);
// 	auto const pTypeGuardRange = pTechno->GetTechnoType()->GuardRange;
// 	auto nGuarRange = pTypeGuardRange == -1 ? 512 : pTypeGuardRange;

// 	if (auto pPri = GetWeaponType(pTechno, 0))
// 	{
// 		if (pPri->Range > nGuarRange)
// 			nGuarRange = pPri->Range;
// 	}

// 	if (auto pSec = GetWeaponType(pTechno, 1))
// 	{
// 		if (pSec->Range > nGuarRange)
// 			nGuarRange = pSec->Range;
// 	}

// 	R->EDI(nGuarRange);
// 	return 0x6F903E;
// }
#endif

#ifdef __old

ASMJIT_PATCH(0x5F6500, AbstractClass_Distance2DSquared_1, 8)
{
	GET(AbstractClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pThat, 0x4);

	int nResult = 0;
	if (pThat)
	{
		auto nThisCoord = pThis->GetCoords();
		auto nThatCoord = pThat->GetCoords();
		nResult = //(int)nThisCoord.DistanceFromXY(nThatCoord)
			cell_Distance_Squared(nThisCoord, nThatCoord);
		;
	}

	R->EAX(nResult);
	return 0x5F655D;
}

ASMJIT_PATCH(0x5F6560, AbstractClass_Distance2DSquared_2, 5)
{
	GET(AbstractClass*, pThis, ECX);
	auto nThisCoord = pThis->GetCoords();
	GET_STACK(CoordStruct*, pThatCoord, 0x4);
	R->EAX(
		//(int)nThisCoord.DistanceFromXY(*pThatCoord)
		cell_Distance_Squared(nThisCoord, *pThatCoord)
	);
	return 0x5F659B;
}

#else
DEFINE_FUNCTION_JUMP(LJMP, 0x5F6500, FakeObjectClass::_GetDistanceOfObj);
DEFINE_FUNCTION_JUMP(CALL, 0x6EB2DC, FakeObjectClass::_GetDistanceOfObj);
DEFINE_FUNCTION_JUMP(CALL, 0x4DEFF4, FakeObjectClass::_GetDistanceOfObj);

DEFINE_FUNCTION_JUMP(LJMP, 0x5F6560, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EABCB, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EAC96, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x6EAD4B, FakeObjectClass::_GetDistanceOfCoord);
DEFINE_FUNCTION_JUMP(CALL, 0x741801, FakeObjectClass::_GetDistanceOfCoord);

#endif
#pragma endregion

#ifdef DEBUG_STUPID_HUMAN_CHECKS

ASMJIT_PATCH(0x50B730, HouseClass_IsControlledByHuman_LogCaller, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	if (!pThis)
		Debug::LogInfo(__FUNCTION__"Caller [{}]", (uintptr_t)R->Stack<DWORD>(0x0));

	return 0x0;
}

ASMJIT_PATCH(0x50B6F0, HouseClass_ControlledByCurrentPlayer_LogCaller, 0x5)
{
	GET(HouseClass*, pThis, ECX);

	if (!pThis)
		Debug::LogInfo(__FUNCTION__"Caller [{}]", (uintptr_t)R->Stack<DWORD>(0x0));

	return 0x0;
}
#endif

#ifdef IONSHITS

#include <RectangleStruct.h>

struct IonBlastData
{
	int PixX;
	int PixY;
};

ASMJIT_PATCH(0x53CB91, IonBlastClass_DTOR, 6)
{
	GET(IonBlastClass*, IB, ECX);
	WarheadTypeExtData::IonBlastExt.erase(IB);
	return 0;
}

class NOVTABLE FakeIonBlastClass : public IonBlastClass
{
public:

	//static bool IonBlastClass_inited;
	//static Surface* IonBlastClass_Surfaces[80];
	//static uint16_t ionblast_A9FAE8[289];
	//static size_t LUT_SIZE;
	//static int IonBlastPitch;
	static COMPILETIMEEVAL reference<bool, 0xAA014C> IonBlastClass_inited {};
	static COMPILETIMEEVAL reference<Surface*, 0xA9FFC8, 80u> IonBlastClass_Surfaces {};
	static COMPILETIMEEVAL reference<int, 0xA9FAE8, 289u> ionblast_A9FAE8 {};
	static COMPILETIMEEVAL reference<int, 0xAA0150> IonBlastPitch {};

	static COMPILETIMEEVAL IonBlastData IonBlastData_53D8E0(int number)
	{
		int index = number - 1;
		int spiralLayer = 1;

		// Find the spiral layer
		if (index >= 8)
		{
			for (int step = 8; step <= index; step += 8)
			{
				index -= step;
				++spiralLayer;
			}
		}

		if (index >= 2 * spiralLayer + 1)
		{
			if (index >= 4 * spiralLayer + 1)
			{
				if (index >= 6 * spiralLayer + 1)
				{
					// Right side
					return { index - 7 * spiralLayer  ,-spiralLayer };
				}
				else
				{
					// Bottom side
					return { -spiralLayer  ,5 * spiralLayer - index };
				}
			}
			else
			{
				// Left side
				return { 3 * spiralLayer - index  ,spiralLayer };
			}
		}

		// Top side
		return { spiralLayer  ,index - spiralLayer };
	}

	static COMPILETIMEEVAL int IonBlastData_Index(int x, int y)
	{
		if (x == 0 && y == 0)
		{
			return 0;
		}

		int absX = Math::abs(x);
		int absY = Math::abs(y);
		int layer = std::max(absX, absY);  // Spiral layer based on distance from center

		int index = 1;
		for (int i = 1; i < layer; ++i)
		{
			index += 8 * i;
		}

		if (x == layer)
		{
			return index + y + layer;
		}
		if (y == layer)
		{
			return index + 3 * layer - x;
		}
		if (x == -layer)
		{
			return index + 5 * layer - y;
		}
		// y == -layer
		return index + x + 7 * layer;
	}

	static void __fastcall InitOneTime()
	{
		if (IonBlastClass_inited)
			return;

		constexpr int SurfaceCount = 80;
		constexpr int Width = 512;
		constexpr int Height = 256;
		constexpr double RadiusStep = 7.1125;

		double currentMaxRadius = 0.0;
		double currentMinRadius = -57.0;
		int step = 0;

		for (int i = 0; i < SurfaceCount; ++i)
		{
			IonBlastClass_Surfaces[i] = GameCreate<BSurface>(Width, Height, 1, nullptr);
			IonBlastClass_Surfaces[i]->Fill(0xFFFFFFFF);

			auto* lockPtr = IonBlastClass_Surfaces[i]->Lock(0, 0);
			auto* pixels = reinterpret_cast<uint8_t*>(lockPtr) + 0x8080;

			for (int y = 127; y >= 0; --y)
			{
				for (int x = 255; x >= 0; --x)
				{
					int dx = x;
					int dy = y;
(					double dist = Math::sqrt(dx * dx + 4 * dy * dy);

					if (dist >= currentMinRadius && dist <= currentMaxRadius)
					{
						double wave = (dist - step * RadiusStep + 38.0) * 0.11;
						double val = (Math::sin(wave) * 3.5 + 3.0) / (dist / 51.0 + 1.0) + 0.5;

						// originally IonBlastData_53D960 was used, but this call would always return 0
						// since a1 == 0 && a2 == 0 → adjust if you have coordinates instead
						uint8_t pixelVal = static_cast<uint8_t>(val);

						// 4-way symmetry
						pixels[256 * y + x] = pixelVal;
						pixels[256 * y + (255 - x)] = pixelVal;
						pixels[256 * (255 - y) + x] = pixelVal;
						pixels[256 * (255 - y) + (255 - x)] = pixelVal;
					}
				}
			}

			if ((256.0 - RadiusStep) > currentMaxRadius)
				currentMaxRadius += RadiusStep;

			currentMinRadius += RadiusStep * 1.2;
			if (currentMinRadius > currentMaxRadius)
				currentMinRadius = currentMaxRadius;

			++step;
		}

		IonBlastClass_inited = true;
	}

	static void __fastcall DestroySurfaces()
	{
		constexpr int SurfaceCount = 80;
		for (int i = 0; i < SurfaceCount; ++i)
		{
			GameAllocator<BSurface> alloc {};
			std::allocator_traits<GameAllocator<BSurface>>::destroy(alloc, IonBlastClass_Surfaces[i]);
			IonBlastClass_Surfaces[i] = nullptr;
		}
	}

	static void _DrawAll()
	{
		if (DSurface::Temp->Get_Pitch() != IonBlastPitch)
		{
			IonBlastPitch = DSurface::Temp->Get_Pitch();
			ionblast_A9FAE8[0] = 0;

			for (int i = 1; i < ionblast_A9FAE8.size(); ++i)
			{
				IonBlastData data = IonBlastData_53D8E0(i);
				ionblast_A9FAE8[i] = data.PixX + IonBlastPitch * data.PixY;
			}
		}

		for (int i = IonBlastClass::Array->Count - 1; i >= 0; --i)
		{
			static_cast<FakeIonBlastClass*>(IonBlastClass::Array->Items[i])->_Draw();
		}
	}

	void _Draw()
	{
		if (!RulesExtData::DetailsCurrentlyEnabled())
			return;

		auto [screenPos, IsIn] = TacticalClass::Instance->GetCoordsToClientSituation(this->Location);

		if (!IsIn)
			return;

		DSurface* targetSurface = DSurface::Temp();
		DSurface* sourceSurface = static_cast<DSurface*>(IonBlastClass_Surfaces[this->Lifetime]);

		RectangleStruct viewportRect {
			.X = DSurface::ViewBounds->X,
			.Y = DSurface::ViewBounds->Y,
			.Width = DSurface::ViewBounds->Width,
			.Height = DSurface::ViewBounds->Height - 7
		};

		RectangleStruct destRect {
			.X = screenPos.X - 256,
			.Y = screenPos.Y - 128,
			.Width = 512,
			.Height = 256
		};

		RectangleStruct srcRect {
			.X = 0,
			.Y = 0,
			.Width = 512,
			.Height = 256
		};

		RectangleStruct srcSubRect {
			.X = 0,
			.Y = 0,
			.Width = 512,
			.Height = 256
		};

		bool regionClipped = false;
		int32_t destBufferOffset = 0;
		int32_t srcBufferOffset = 0;

		if (!Blit_helper_lockregion(
			targetSurface,
			&viewportRect,
			&destRect,
			sourceSurface,
			&srcRect,
			&srcSubRect,
			&regionClipped,
			(int16_t*)(&destBufferOffset),
			(int16_t*)(&srcBufferOffset)))
		{
			return;
		}

		uint16_t* destBuffer = reinterpret_cast<uint16_t*>(destBufferOffset);
		int32_t* srcBuffer = reinterpret_cast<int32_t*>(srcBufferOffset);
		int8_t* srcBuffer_8 = reinterpret_cast<int8_t*>(srcBufferOffset);

		const int pitch = targetSurface->Get_Pitch();
		const int surfaceWidth = targetSurface->Get_Width();
		const int zBufferWidth = ZBuffer::Instance->Width;

		const int zCoord = this->Location.Z;
		const int16_t zRef = static_cast<int16_t>(ZBuffer::Instance->MaxValue - Game::AdjustHeight(zCoord));
		int16_t zThreshold = zRef - static_cast<int16_t>(destRect.Y) - 3;

		int16_t* zBufferRow = (int16_t*)ZBuffer::Instance->GetBuffer(0, destRect.Y);

		// Safety bounds check before doing optimized access
		uintptr_t zBufferCheck = reinterpret_cast<uintptr_t>(&zBufferRow[surfaceWidth + (srcSubRect.Height + 1) * zBufferWidth]);
		if (zBufferCheck >= reinterpret_cast<uintptr_t>(ZBuffer::Instance->BufferTail))
		{
			// Fallback path for conservative access
			for (int row = 0; row < srcSubRect.Height; ++row)
			{
				for (int col = 0; col < srcSubRect.Width; ++col)
				{
					uint8_t pixel = *srcBuffer_8++;
					if (pixel > 0)
					{
						uint16_t* zPtr = (uint16_t*)ZBuffer::Instance->GetBuffer(destRect.X + col, row + destRect.Y);
						if (*zPtr > zThreshold && pixel < ionblast_A9FAE8.size())
						{
							destBuffer[col] = destBuffer[ionblast_A9FAE8[pixel]];
						}
					}
				}

				destBuffer = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destBuffer) + pitch);
				srcBuffer += 512 - srcSubRect.Width;
				srcBuffer_8 = reinterpret_cast<int8_t*>(srcBuffer);
				--zThreshold;
			}
		}
		else
		{
			// Fast path: linear access
			uint16_t* zPtr = reinterpret_cast<uint16_t*>(&zBufferRow[destRect.X]);

			for (int row = 0; row < srcSubRect.Height; ++row)
			{
				for (int col = 0; col < srcSubRect.Width; ++col)
				{
					uint8_t pixel = *srcBuffer_8++;
					if (pixel > 0 && zPtr[col] > zThreshold && pixel < ionblast_A9FAE8.size())
					{
						destBuffer[col] = destBuffer[ionblast_A9FAE8[pixel]];
					}
				}

				destBuffer = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(destBuffer) + pitch);
				zPtr += zBufferWidth;
				srcBuffer += 512 - srcSubRect.Width;
				srcBuffer_8 = reinterpret_cast<int8_t*>(srcBuffer);
				--zThreshold;
			}
		}

		targetSurface->Unlock();
		sourceSurface->Unlock();
	}

	void _AI()
	{

		const auto pData = WarheadTypeExtData::IonBlastExt.get_or_default(this);
		const int Ripple_Radius = pData ? MinImpl((int)ionblast_A9FAE8.Size, pData->Ripple_Radius + 1) : ionblast_A9FAE8.Size;

		if (this->Lifetime >= Ripple_Radius)
		{
			GameDelete<true, false>(this);
			return;
		}

		const auto screenPos = TacticalClass::Instance->CoordsToClient(this->Location);

		if (!this->DisableIonBeam && this->Lifetime == 0)
		{
			CoordStruct spawnCoord = this->Location;
			spawnCoord.Z += 5;
			const auto Rules = RulesClass::Instance();

			auto* mapCell = MapClass::Instance->GetCellAt(this->Location);
			const bool isWater = mapCell->LandType == LandType::Water;

			if (const auto animId = isWater ? Rules->SplashList[Rules->SplashList.Count - 1] :
				pData ? pData->Ion_Blast.Get(Rules->IonBlast) : Rules->IonBlast)
			{
				GameCreate<AnimClass>(animId, spawnCoord);
			}

			if (const auto pBeam = pData ? pData->Ion_Beam.Get(Rules->IonBeam) : Rules->IonBeam)
			{
				GameCreate<AnimClass>(pBeam, spawnCoord);
			}

			if (const auto pWH = pData ? pData->Ion_WH.Get(Rules->IonCannonWarhead) : Rules->IonCannonWarhead)
			{

				const int nDamage = pData ? pData->Ion_Damage.Get(Rules->IonCannonDamage) : Rules->IonCannonDamage;

				if (mapCell->ContainsBridge())
				{
					CoordStruct target = this->Location;
					target.Z += CellClass::BridgeHeight;
					DamageArea::Apply(&target, nDamage, nullptr, pWH, true, nullptr);
				}

				DamageArea::Apply(&this->Location, nDamage, nullptr, pWH, true, nullptr);
				MapClass::FlashbangWarheadAt(nDamage, pWH, this->Location, false, SpotlightFlags::None);
			}
		}

		if (!pData || pData->Ion_Rocking)
		{
			int16_t centerX = static_cast<int16_t>(this->Location.X / 256);
			int16_t centerY = static_cast<int16_t>(this->Location.Y / 256);

			for (int16_t dy = -3; dy <= 3; ++dy)
			{
				for (int16_t dx = -3; dx <= 3; ++dx)
				{
					CellStruct cell { static_cast<int16_t>(centerX + dx), static_cast<int16_t>(centerY + dy) };
					auto* mapCell = MapClass::Instance->GetCellAt(cell);
					FootClass* unit = flag_cast_to<FootClass*>(mapCell->FirstObject);

					while (unit)
					{
						if (unit->WhatAmI() == InfantryClass::AbsID || unit->WhatAmI() == UnitClass::AbsID)
						{

							CoordStruct unitCoord = unit->Location;
							Point2D unitScreen = TacticalClass::Instance->CoordsToClient(unitCoord);

							int dxPix = unitScreen.X - screenPos.X;
							int dyPix = unitScreen.Y - screenPos.Y;
							int dist = static_cast<int>(Math::sqrt(dxPix * dxPix + dyPix * dyPix)) + 8;

							if (dist < 256)
							{

								Surface* surf = IonBlastClass_Surfaces[this->Lifetime];
								char* locked = static_cast<char*>(surf->Lock(dist + 0x100, 128));
								if (*locked > 0)
								{
									unit->SetSpeedPercentage(0.0f);
									IonBlastData data = IonBlastData_53D8E0(*locked);
									unit->height_subtract_6B4 = 2 * data.PixY;
								}

								auto vox = unit->GetTechnoType()->MainVoxel.VXL;

								if (vox && !vox->LoadFailed && *locked >= 0)
								{
									float deltax = static_cast<float>(this->Location.X - unit->Location.X);
									float deltay = static_cast<float>(this->Location.Y - unit->Location.Y);
									float deltaz = static_cast<float>(this->Location.Z - unit->Location.Z);
									const float len = Math::sqrt(deltax * deltax + deltay * deltay + deltaz * deltaz);

									if (Math::abs(len) > 0.00002f)
									{
										deltax /= len;
										deltay /= len;
										deltaz /= len;

										const auto& facing_ = unit->PrimaryFacing;
										const auto facing_Current = facing_.Current();

										const float facingAngle = (facing_Current.Raw - Math::BINARY_ANGLE_MASK) * -0.0000958767f;
										const float sinA = Math::sin((double)facingAngle);
										const float cosA = Math::cos((double)facingAngle);

										const float ux = deltax * cosA + deltay * sinA;
										const float uz = deltax * sinA - deltay * cosA;
										const float uy = deltaz;

										float proj = Math::sqrtux * ux + uz * uz + uy * uy);
										const float align = cosA * ux - sinA * proj;

										if (Math::abs(align - deltax) > 0.0002f || Math::abs(cosA * proj + sinA * ux - deltay) > 0.0002f)
										{
											proj = -proj;
										}

										const float blastDist = len + 51.0f;
										const float blastOffset = (Math::sin(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f) * 3.5f + 3.0f) * 51.0f;
										const float blastFactor = Math::cos(double(len - static_cast<float>(this->Lifetime) * 7.1125f + 38.0f) * 0.11f);
										const float curve = (blastFactor * 0.11f * 51.0f * 3.5f * blastDist - blastOffset) / (blastDist * blastDist);

										unit->AngleRotatedSideways = proj * curve * Math::GAME_TWOPIf;
										unit->AngleRotatedForwards = -ux * curve * Math::GAME_TWOPIf;
									}
								}
							}
						}
					}
				}
			}
		}

		++this->Lifetime;
	}

	// Helper structures
	struct Vector3D
	{
		float X, Y, Z;
	};

	//DEFINE_FUNCTION_JUMP(CALL, 0x531758, FakeIonBlastClass::InitOneTime)
	//DEFINE_FUNCTION_JUMP(CALL, 0x6BE3CE, FakeIonBlastClass::DestroySurfaces)
	DEFINE_FUNCTION_JUMP(CALL, 0x53D326, FakeIonBlastClass::_AI)

		//bool FakeIonBlastClass::IonBlastClass_inited {};
		//Surface* FakeIonBlastClass::IonBlastClass_Surfaces[80] {};
		//uint16_t FakeIonBlastClass::ionblast_A9FAE8[289] {};
		//size_t FakeIonBlastClass::LUT_SIZE { std::size(FakeIonBlastClass::ionblast_A9FAE8) };
		//int FakeIonBlastClass::IonBlastPitch {};
#endif

		static void __fastcall IonBlastDrawAll()
	{
		VeinholeMonsterClass::DrawAll();
		IonBlastClass::DrawAll();
	}
	DEFINE_FUNCTION_JUMP(CALL, 0x6D4656, IonBlastDrawAll)

		static void __fastcall LaserDrawclassDrawAll()
	{
		LaserDrawClass::DrawAll();
		EBolt::DrawAll();
		TacticalExtData::Instance()->Screen_Flash_AI();
		//ElectricBoltManager::Draw_All();
	}
	DEFINE_FUNCTION_JUMP(CALL, 0x6D4669, LaserDrawclassDrawAll)

		ASMJIT_PATCH(0x7BB350, XSurface_Func_check, 0x6)
	{
		GET(XSurface*, pThis, ECX);
		GET_STACK(uintptr_t, caller, 0x0);

		if (!pThis || VTable::Get(pThis) != XSurface::vtable)
		{
			Debug::LogInfo("XSurface Invalid caller [0x{0:x}]!!", caller);
		}

		return 0x0;
	} ASMJIT_PATCH_AGAIN(0x7BBAF0, XSurface_Func_check, 0x5)

		ASMJIT_PATCH(0x6D471A, TechnoClass_Render_dead, 0x6)
	{
		GET(TechnoClass*, pTech, ESI);

		if (!pTech->IsAlive)
			return 0x6D48FA;
		auto vtable = VTable::Get(pTech);

		if (vtable != AircraftClass::vtable
			&& vtable != BuildingClass::vtable
			&& vtable != InfantryClass::vtable
			&& vtable != UnitClass::vtable)
			return 0x6D48FA;

		return 0x0;
	}

	ASMJIT_PATCH(0x438D72, BombListClass_DetectorMissingHouse, 0x7)
	{
		GET(HouseClass*, pDetectorOwner, EAX);
		GET(TechnoClass*, pDetector, ESI);

		if (!pDetectorOwner)
		{
			Debug::FatalErrorAndExit("BombListClass Detector[%s - %s] Missing Ownership !\n", pDetector->GetThisClassName(), pDetector->get_ID());
			//return 0x438E11;
		}

		R->AL(pDetectorOwner->ControlledByCurrentPlayer());
		return 0x438D79;
	}

	ASMJIT_PATCH(0x70D0D0, TechnoClass_HasAbility_Check, 0x5)
	{
		GET_STACK(AbilityType, abi, 0x4);
		GET_STACK(DWORD, caller, 0x0);

		if (abi >= AbilityType::count)
			Debug::FatalError("TechnoClass HasAbility input is too big ! %d [caller %x]\n", abi, caller);

		return 0x0;
	}



	//ASMJIT_PATCH(0x7399EE, UnitClass_TryToDeploy_BrokenEBP, 0x5)
	//{
	   // GET(UnitClass*, pThis, EBP);

	   // if (pThis->AttachedTag)
	   // {
	   //	 R->EAX(pThis->AttachedTag);
	   //	 return 0x7399F5;
	   // }

	   // return 0x739A0E;
	//}
#include <Utilities/Swizzle.h>

	DWORD LastKnown;
	AbstractClass* pAbs;

	ASMJIT_PATCH(0x4103D0, AbstractClass_Load_LogValue, 0x5)
	{
		GET(AbstractClass*, pThis, ESI);
		//GET_STACK(IStream*, pStream, 0x0);

		//immedietely update the extension pointer value and the extension AttachedToObject itself !
		ExtensionSwizzleManager::SwizzleExtensionPointer(reinterpret_cast<void**>(&pThis->unknown_18), pThis);
		LastKnown = pThis->unknown_18;
		pAbs = pThis;

		return 0x0;
	}

	//more specific
	//ASMJIT_PATCH(0x41096D, AbstractTypeClass_NoInt_cleaupPtr,0x6)
	//{
	   //  GET(AbstractClass*, pThis, EAX);

	   //  if (Phobos::Otamaa::DoingLoadGame) {
	   //	  if (pAbs != pThis)  //avoid missmatching
	   //		  LastKnown = 0;
	   //  }

	   //  pThis->unknown_18 = std::exchange(LastKnown, 0u);
	   //  return 0x0;
	//}

	ASMJIT_PATCH(0x410182, AbstractClass_cleaupPtr_B, 0x6)
	{
		GET(AbstractClass*, pThis, EAX);

		if (Phobos::Otamaa::DoingLoadGame)
		{
			if (pAbs != pThis) //avoid missmatching
				LastKnown = 0;
		}

		pThis->unknown_18 = std::exchange(LastKnown, 0u);
		pThis->RefCount = 0l;
		return 0x410188;
	}

	ASMJIT_PATCH(0x4101E4, AbstractClass_cleaupPtr, 0x7)
	{
		GET(AbstractClass*, pThis, EAX);

		if (Phobos::Otamaa::DoingLoadGame)
		{

			if (pAbs != pThis) //avoid missmatching
				LastKnown = 0;
		}

		pThis->unknown_18 = std::exchange(LastKnown, 0u);
		return 0x0;
	}

	//ASMJIT_PATCH(0x521960, InfantryClass_Load_test, 0x5)
	// {
	   // GET(InfantryClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x521A11, InfantryClass_NoInit_test, 0x6)
	//{
	   // GET(AbstractClass*, pThis, EAX);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5219E8, InfantryClass_Load_test, 0x5)
	//{
	   // GET(InfantryClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5F3B5D, ObjectClass_Load_checkExt, 0x5)
	//{
	   // GET(ObjectClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x65A7F7, RadioClass_Load_checkExt, 0x5)
	//{
	   // GET(RadioClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x6F44E9, TechnoClass_Load_checkExt, 0x5)
	//{
	   // GET(TechnoClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x4D3568, FootClass_Load_checkExt, 0x6)
	//{
	   // GET(FootClass*, pThis, ESI);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x744527, UnitClass_Load_checkExt, 0x6)
	//{
	   // GET(UnitClass*, pThis, ESI);
	   // return 0x0;
	//}
	//ASMJIT_PATCH(0x410361, AbstractClass_Save_LogValue, 0x5)
	//{
	   // GET(AbstractClass*, pThis, ESI);
	   // Debug::Log("Saving Ext of %x", pThis->unknown_18);
	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x6D4912, TechnoClass_Render_deadRemoval, 0x6)
	//{
	   // TechnoClass::Array->remove_if([](TechnoClass* ptr) {
	   //	 auto vtable = VTable::Get(ptr);
	   //	 if (vtable != AircraftClass::vtable
	   //		 && vtable != BuildingClass::vtable
	   //		 && vtable != InfantryClass::vtable
	   //		 && vtable != UnitClass::vtable)
	   //		 return true;

	   //	 return false;
	   //});

	   // return 0x0;
	//}

	//ASMJIT_PATCH(0x5F4870, ObjectClass_func_BrokenObj, 0x5)
	//{
	   // GET(ObjectClass*, pObj, ECX);
	   // GET_STACK(DWORD, caller, 0x0);

	   // if (!pObj->IsAlive)
	   //	 Debug::Log("Dead obj %x caller %x\n", pObj , caller);
	   // //auto vtable = VTable::Get(pObj);
	   // //BulletClass
	   //	// IsometricTileClass
	   //	// OverlayClass
	   //	// ParticleClass
	   //	// ParticleSystemClass
	   //	// SmudgeClass
	   //	// TerrainClass
	   //	// VeinholeMonsterClass
	   //	// VoxelAnimClass
	   //	// WaveClass
	   // //if (&& vtable != BuildingLightClass::vtable  && vtable != AnimClass::vtable
	   //	// && vtable != AircraftClass::vtable
	   //	// && vtable != BuildingClass::vtable
	   //	// && vtable != InfantryClass::vtable
	   //	// && vtable != UnitClass::vtable)

	   // return 0x0;
	//}

	/*******************************************************************************
	* Cohen-Sutherland Line Clipping Algorithm
	* Cleaned up version
	******************************************************************************/

	/*
	 * Build bits that indicate which end points lie outside the clipping rectangle.
	 * Quick checks against these flag bits will speed the clipping process.
	 */
	constexpr inline int CODE_INSIDE = 0;  // 0000
	constexpr inline int CODE_LEFT = 1;    // 0001
	constexpr inline int CODE_RIGHT = 2;   // 0010
	constexpr inline int CODE_BOTTOM = 4;  // 0100
	constexpr inline int CODE_TOP = 8;     // 1000

	/***********************************************************************************************
	 * Compute_Out_Code
	 *
	 * Compute the bit code for a point (x, y) using the clip rectangle.
	 * Bounded diagonally by (xmin, ymin), and (xmax, ymax).
	 *
	 * INPUT:   x     -- X coordinate of point
	 *          y     -- Y coordinate of point
	 *          rect  -- Clipping rectangle
	 *
	 * OUTPUT:  OutCode indicating which boundaries the point is outside of
	 ***********************************************************************************************/
	int __forceinline Compute_Out_Code(double x, double y, RectangleStruct* rect)
	{
		int code = CODE_INSIDE;

		int right_edge = rect->X + rect->Width;
		if (x >= right_edge)
		{
			code |= CODE_RIGHT;
		}
		else if (x < rect->X)
		{
			code |= CODE_LEFT;
		}

		int bottom_edge = rect->Y + rect->Height;
		if (y >= bottom_edge)
		{
			code |= CODE_BOTTOM;
		}
		else if (y < rect->Y)
		{
			code |= CODE_TOP;
		}

		return code;
	}

	/***********************************************************************************************
	 * Clip_Line
	 *
	 * Cohen-Sutherland line clipping algorithm implementation.
	 * Clips a line segment to fit within the specified rectangle.
	 *
	 * INPUT:   pt1   -- First point of line segment (modified in place)
	 *          pt2   -- Second point of line segment (modified in place)
	 *          rect  -- Clipping rectangle
	 *
	 * OUTPUT:  true if line segment intersects rectangle, false otherwise
	 *
	 * NOTES:   Based on algorithm from "Computer Graphics: Principles and Practice in C"
	 *          Modified version of: https://en.wikipedia.org/wiki/Cohen–Sutherland_algorithm
	 ***********************************************************************************************/
	bool __fastcall Clip_Line(Point2D& point1, Point2D& point2, RectangleStruct& rect)
	{
		int outcode0 = Compute_Out_Code(point1.X, point1.Y, &rect);
		int outcode1 = Compute_Out_Code(point2.X, point2.Y, &rect);
	
		double x0 = point1.X;
		double y0 = point1.Y;
		double x1 = point2.X;
		double y1 = point2.Y;
	
		while (true)
		{
			// Trivial accept
			if (outcode0 == CODE_INSIDE && outcode1 == CODE_INSIDE)
			{
				point1.X = x0; point1.Y = y0;
				point2.X = x1; point2.Y = y1;
				return true;
			}
	
			// Trivial reject
			if (outcode0 & outcode1)
				return false;
	
			// Choose endpoint outside rect
			int outcodeOut = (outcode0 != CODE_INSIDE) ? outcode0 : outcode1;
	
			double x = 0.0;
			double y = 0.0;
	
			// Find intersection
			if (outcodeOut & CODE_TOP)           // above clip window
			{
				double dy = y1 - y0;
				if (dy == 0.0) return false; // horizontal line outside
				double slope_y = (x1 - x0) / dy;
				y = rect.Y;
				x = x0 + (y - y0) * slope_y;
			}
			else if (outcodeOut & CODE_BOTTOM)   // below clip window
			{
				double dy = y1 - y0;
				if (dy == 0.0) return false;
				double slope_y = (x1 - x0) / dy;
				y = rect.Y + rect.Height - 1;
				x = x0 + (y - y0) * slope_y;
			}
			else if (outcodeOut & CODE_RIGHT)    // to the right of clip window
			{
				double dx = x1 - x0;
				if (dx == 0.0) return false; // vertical line outside
				double slope_x = (y1 - y0) / dx;
				x = rect.X + rect.Width - 1;
				y = y0 + (x - x0) * slope_x;
			}
			else if (outcodeOut & CODE_LEFT)     // to the left of clip window
			{
				double dx = x1 - x0;
				if (dx == 0.0) return false;
				double slope_x = (y1 - y0) / dx;
				x = rect.X;
				y = y0 + (x - x0) * slope_x;
			}
			else
			{
				// Safety net: outcodeOut has no directional bits? -> break
				return false;
			}
	
			// Move the outside point to intersection and recalc code
			if (outcodeOut == outcode0)
			{
				x0 = x; y0 = y;
				outcode0 = Compute_Out_Code(x0, y0, &rect);
			}
			else
			{
				x1 = x; y1 = y;
				outcode1 = Compute_Out_Code(x1, y1, &rect);
			}
		}
	}

DEFINE_FUNCTION_JUMP(LJMP, 0x7BC2B0, Clip_Line)

DEFINE_FUNCTION_JUMP(CALL, 0x436123, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x43617B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BBD72, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BC933, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BE060, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BEBA6, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BF7CD, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4BFDFF, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C0EAA, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C27D0, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4C28F4, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x4DC694, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x63D8A3, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6595F6, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6598F5, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x659DA0, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x660272, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6605EB, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DAC2B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DACD2, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DAD1B, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6DB0E1, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x6FFB70, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704BE9, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704DD4, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x704E14, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x70516F, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x70552E, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x705772, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x7BA689, Clip_Line)
DEFINE_FUNCTION_JUMP(CALL, 0x7BAC06, Clip_Line)

ASMJIT_PATCH(0x6B0B81, SlaveManagerClass_FreeSlaves_dead, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	return pTech->IsAlive ? 0x0 : 0x6B0C0B;
}

#ifndef CHECK_PTR_VALID


ASMJIT_PATCH(0x4F9A90, HouseClass_IsAlly_ObjectClass, 0x7)
{
	GET_STACK(ObjectClass*, pTarget, 0x4);
	GET(HouseClass*, pThis, ECX);
	GET_STACK(DWORD, caller, 0x0);

	bool result = false;

	if (pTarget) {

		//if(flag_cast_to<TechnoClass*>(pTarget)){
		//	if ((VTable::Get(pTarget) != AircraftClass::vtable &&
		//		VTable::Get(pTarget) != BuildingClass::vtable &&
		//		VTable::Get(pTarget) != UnitClass::vtable &&
		//		VTable::Get(pTarget) != InfantryClass::vtable))
		//	{
		//		Debug::FatalError("Missing valid vtable %x , caller %x", pTarget, caller);
		//	}
		//}

		auto pTargetOwner = pTarget->GetOwningHouse();
		result = pThis->IsAlliedWith(pTargetOwner);
	}

	R->AL(result);
	return 0x4F9ADE;
}

ASMJIT_PATCH(0x6F8A0F, TechnoClass_EvalCell_deadTechno, 0x8)
{
	GET(ObjectClass*, pCellObj, EDI);
	return !pCellObj || !pCellObj->IsAlive ? 0x6F8B4D : 0x6F8A17;
}
	//ASMJIT_PATCH(0x4F9A90, HouseClass_IsAlliedWith, 0x7)
	//{
	//	GET(HouseClass*, pThis, ECX);
	//	GET_STACK(DWORD, called, 0x0);
	//	GET_STACK(AbstractClass*, pAbs, 0x4);
	//
	//	if (!pThis || VTable::Get(pThis) != HouseClass::vtable) {
	//		Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` pointer !", R->Origin(), called);
	//	}
	//	else if (auto pTechno = flag_cast_to<TechnoClass*>(pAbs)){
	//			if(VTable::Get(pTechno) != AircraftClass::vtable &&
	//				VTable::Get(pTechno) != BuildingClass::vtable &&
	//				VTable::Get(pTechno) != UnitClass::vtable &&
	//				VTable::Get(pTechno) != InfantryClass::vtable
	//			) {
	//			Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` abstract pointer !", R->Origin(), called);
	//		}
	//	}
	//
	//	return 0;
	//}
	//ASMJIT_PATCH_AGAIN(0x4F9AF0, HouseClass_IsAlliedWith, 0x7)
	//ASMJIT_PATCH_AGAIN(0x4F9A10, HouseClass_IsAlliedWith, 0x6)
	//ASMJIT_PATCH_AGAIN(0x4F9A50, HouseClass_IsAlliedWith, 0x6)

#endif

//ASMJIT_PATCH(0x7564B0, VoxClass_GetData, 7) {
//	GET(VoxLib*, pVox, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(int, header, 0x4);
//	GET_STACK(int, layer, 0x8);
//
//	if (!pVox->HeaderData || !pVox->TailerData)
//		Debug::FatalError("VoxelLibraryClass::Get_Voxel_Layer_Info input is broken ! caller 0x%x", caller);
//
//	auto pData = &pVox->TailerData[layer + pVox->HeaderData[header].limb_number];
//
//	R->EAX(pData);
//	return 0x7564CF;
//}

//loading save game will crash after this function
//not sure atm, weird shit
ASMJIT_PATCH(0x5F7577, ObjectTypeClass_DTOR_Voxel, 0x6) {
	GET(AbstractTypeClass*, pThis, ESI);

	Debug::Log("Destroying Voxel for %s ! \n", pThis->ID);
	return 0x0;
}

//ASMJIT_PATCH(0x6f4974, TechnoClass_UpdateDiscovered_ByPlayer_Announce, 0x6) {
//	//play eva , once ?
//}

//ASMJIT_PATCH(0x5F6360, ObjectClass_Distance, 0x5)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(ObjectClass*, pTarget, 0x4);
//
//	if (!pTarget || !pTarget->IsAlive && pTarget->AbstractFlags != AbstractFlags::None) {
//		Debug::Log("Caller %x\n", caller);
//		R->EAX(0);
//		return 0x5F6376;
//	}
//
//	return 0x0;
//}