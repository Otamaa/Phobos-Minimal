#pragma region Includes
#include "Hooks.Otamaa.h"

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
#include <Ext/InfantryType/Body.h>
#include <Misc/Ares/Hooks/Header.h>

#include <Surface.h>

#include <Audio.h>

#include <Commands/ShowTeamLeader.h>

#include <Commands/ToggleRadialIndicatorDrawMode.h>

#include <ExtraHeaders/AStarClass.h>
#include <BitFont.h>
#include <format>


#include <Ext/SWType/Body.h>
#include <New/Type/CrateTypeClass.h>

#include <SpotlightClass.h>
#include <New/Entity/FlyingStrings.h>
#pragma endregion

DEFINE_HOOK(0x6FA2CF, TechnoClass_AI_DrawBehindAnim, 0x9) //was 4
{
	GET(TechnoClass*, pThis, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(RectangleStruct*, pBound, EAX);

	if (const auto pBld = specific_cast<BuildingClass*>(pThis))
	{
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
		{
			return 0x6FA30C;
		}
	}

	if (pThis->InOpenToppedTransport)
		return 0x6FA30C;

	const auto pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pType->IsDummy)
		return 0x6FA30C;

	pThis->DrawBehindMark(pPoint, pBound);

	return 0x6FA30C;
}

DEFINE_HOOK(0x74C8FB, VeinholeMonsterClass_CTOR_SetArmor, 0x6)
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
	if (auto pManager = RulesExtData::Instance()->VeinholePal)
		Pal = pManager->GetConvert<PaletteManager::Mode::Temperate>();

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_JUMP(CALL, 0x74D5BC, GET_OFFSET(DrawShape_VeinHole));

DEFINE_HOOK(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	const auto nTheater = ScenarioClass::Instance->Theater;
	SmudgeTypeClass::TheaterInit(nTheater);
	VeinholeMonsterClass::TheaterInit(nTheater);
	return 0x4AD0A8;
}

DEFINE_HOOK(0x74D0D2, VeinholeMonsterClass_AI_SelectParticle, 0x5)
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

DEFINE_HOOK(0x7290AD, TunnelLocomotionClass_Process_Stop, 0x5)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	if (const auto pLinked = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo)
		if (auto const pCell = pLinked->GetCell())
			pCell->CollectCrate(pLinked);

	return 0;
}

DEFINE_HOOK(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

DEFINE_HOOK_AGAIN(0x46684A, BulletClass_AI_TrailerInheritOwner, 0x5)
DEFINE_HOOK(0x466886, BulletClass_AI_TrailerInheritOwner, 0x5)
{
	GET(BulletClass* const, pThis, EBP);
	//GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x184));

	//Eax is discarded anyway
	auto const pExt = BulletExtContainer::Instance.Find(pThis);
	AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
		pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner) ? pExt->Owner : nullptr,
		pThis->Target ? pThis->Target->GetOwningHouse() : nullptr,
		pThis->Owner,
		false
	);

	return 0x4668BD;
}

DEFINE_HOOK(0x6FF394, TechnoClass_FireAt_FeedbackAnim, 0x8)
{
	enum { CreateMuzzleAnim = 0x6FF39C, SkipCreateMuzzleAnim = 0x6FF43F };

	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET(AnimTypeClass* const, pMuzzleAnimType, EDI);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		{
			auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord);
			AnimExtData::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
			if (pThis->WhatAmI() != BuildingClass::AbsID)
				pFeedBackAnim->SetOwnerObject(pThis);
		}
	}

	return pMuzzleAnimType ? CreateMuzzleAnim : SkipCreateMuzzleAnim;
}

DEFINE_HOOK(0x6FF3CD, TechnoClass_FireAt_AnimOwner, 0x7)
{
	enum
	{
		Goto2NdCheck = 0x6FF427, DontSetAnim = 0x6FF43F,
		AdjustCoordsForBuilding = 0x6FF3D9, Continue = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	//GET(WeaponTypeClass*, pWeapon, EBX);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	if (!pAnim)
		return DontSetAnim;

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

	return pThis->WhatAmI() == BuildingClass::AbsID ? AdjustCoordsForBuilding : Goto2NdCheck;
}

#pragma region WallTower
DEFINE_HOOK(0x4405C1, BuildingClas_Unlimbo_WallTowers_A, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x4405CF : 0x440606;
}

DEFINE_HOOK(0x440F66, BuildingClass_Unlimbo_WallTowers_B, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x440F78 : 0x44104D;
}

DEFINE_HOOK(0x44540D, BuildingClass_ExitObject_WallTowers, 0x5)
{
	GET(BuildingClass* const, pThis, EDI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445424 : 0x4454D4;
}

DEFINE_HOOK(0x445ADB, BuildingClass_Limbo_WallTowers, 0x9)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445AED : 0x445B81;
}

DEFINE_HOOK(0x4514F9, BuildingClass_AnimLogic_WallTowers, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x45150B : 0x4515E9;
}

DEFINE_HOOK(0x45EF11, BuildingClass_FlushForPlacement_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EBX);
	R->EDX(RulesClass::Instance());
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x45EF23 : 0x45F00B;
}

DEFINE_HOOK(0x47C89C, CellClass_CanThisExistHere_SomethingOnWall, 0x6)
{
	GET(int const, nHouseIDx, EAX);
	GET(CellClass* const, pCell, EDI);
	GET(int const, idxOverlay, ECX);
	GET_STACK(BuildingTypeClass* const, PlacingObject, STACK_OFFS(0x18, -0x8));
	GET_STACK(HouseClass* const, PlacingOwner, STACK_OFFS(0x18, -0xC));

	enum { Adequate = 0x47CA70, Inadequate = 0x47C94F } Status = Inadequate;

	HouseClass* OverlayOwner = HouseClass::Array->GetItemOrDefault(nHouseIDx);
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

DEFINE_HOOK(0x4FE546, HouseClass_BuildingClass_AI_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x4FE554 : 0x4FE6E7;
}

DEFINE_HOOK(0x4FE648, HouseClss_AI_Building_WallTowers, 0x6)
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

DEFINE_HOOK(0x5072F8, HouseClass_506EF0_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50735C : 0x507306;
}

DEFINE_HOOK(0x50A96E, HouseClass_AI_TakeOver_WallTowers_A, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);
	const auto& Nvec = RulesExtData::Instance()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50A980 : 0x50AB90;
}

DEFINE_HOOK(0x50A9D2, HouseClass_AI_TakeOver_WallTowers_B, 0x6)
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

DEFINE_JUMP(VTABLE, 0x7E2908, GET_OFFSET(AircraftTypeClass_CanUseWaypoint));

DEFINE_HOOK(0x467C2E, BulletClass_AI_FuseCheck, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(CoordStruct*, pCoord, ECX);

	R->EAX(BulletExtData::FuseCheckup(pThis, pCoord));
	return 0x467C3A;
}

DEFINE_HOOK(0x4B050B, DriveLocomotionClass_Process_Cargo, 0x5)
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

DEFINE_HOOK(0x4B07CA, DriveLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pDrive = static_cast<DriveLocomotionClass* const>(pLoco);
	TechnoExtData::PlayAnim(RulesClass::Instance->Wake, pDrive->LinkedTo);
	return 0x4B0828;
}

DEFINE_HOOK(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pShip = static_cast<ShipLocomotionClass* const>(pLoco);
	TechnoExtData::PlayAnim(RulesClass::Instance->Wake, pShip->LinkedTo);
	return 0x69FEF0;
}

DEFINE_HOOK(0x414EAA, AircraftClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(AircraftClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0x24));

	pAnim->AnimClass::AnimClass(TechnoTypeExtData::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x414ED0;
}

DEFINE_HOOK(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(UnitClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	pAnim->AnimClass::AnimClass(TechnoTypeExtData::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x7365BB;
}

DEFINE_HOOK(0x738703, UnitClass_Explode_ExplodeAnim, 0x5)
{
	GET(AnimTypeClass*, pExplType, EDI);
	GET(UnitClass*, pThis, ESI);

	if (pExplType)
	{

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExplType, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x738748;
}

DEFINE_HOOK(0x4419A9, BuildingClass_Destroy_ExplodeAnim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, X, ECX);
	GET(int, Y, EDX);
	GET(int, Z, EAX);
	GET(int, zAdd, EDI);

	CoordStruct nLoc { X , Y , Z + zAdd };
	if (auto const pType = pThis->Type->Explosion.Items[ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1)])
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomFromMax(3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, nLoc, nDelay, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
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

DEFINE_HOOK(0x441AC4, BuildingClass_Destroy_Fire3Anim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x64 - 0x54);

	if (auto pType = AnimTypeClass::Find(GameStrings::Anim_FIRE3))
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomRanged(1, 3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoord, nDelay + 3, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x441B1F;
}

DEFINE_HOOK(0x441D1F, BuildingClass_Destroy_DestroyAnim, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	return 0x0;
}

DEFINE_HOOK(0x70253F, TechnoClass_ReceiveDamage_Metallic_AnimDebris, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x30));
	GET(int, nIdx, EAX);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	pAnim->AnimClass::AnimClass(RulesClass::Instance->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
	pThis->GetOwningHouse(), false);

	return 0x70256B;
}

DEFINE_HOOK(0x702484, TechnoClass_ReceiveDamage_AnimDebris, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);
	GET(AnimClass*, pAnim, EBX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x3C));
	GET(int, nIdx, EDI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	pAnim->AnimClass::AnimClass(pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExtData::SetAnimOwnerHouseKind(pAnim,
		args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
		pThis->GetOwningHouse(),
		false
	);

	return 0x7024AF;
}

//ObjectClass TakeDamage , 5F559C
//UnitClass TakeDamage , 737F0E

// DEFINE_HOOK(0x703819, TechnoClass_Cloak_Deselect, 0x6)
// {
// 	enum { Skip = 0x70383C, CheckIsSelected = 0x703828 };
//
// 	return R->ESI<TechnoClass*>()->Owner->IsControlledByHuman()
// 		? CheckIsSelected : Skip;
// }

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
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

DEFINE_HOOK(0x7091FC, TechnoClass_CanPassiveAquire_AI, 0x6)
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

DEFINE_HOOK(0x6F8260, TechnoClass_EvalObject_LegalTarget_AI, 0x6)
{
	enum
	{
		Continue = 0x0,
		ContinueChecks = 0x6F826E,
		ReturnFalse = 0x6F894F,
		SetAL = 0x6F8266,
	};

	GET(TechnoClass* const, pThis, EDI);
	GET(TechnoClass* const, pTarget, ESI);
	GET(TechnoTypeClass* const, pTargetType, EBP);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

	if (pTypeExt->AI_LegalTarget.isset())
	{
		if (pThis->Owner && pThis->Owner->IsControlledByHuman())
			return Continue;

		return pTypeExt->AI_LegalTarget.Get() ?
			ContinueChecks : ReturnFalse;
	}

	return Continue;
}

DEFINE_HOOK(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
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
			Drawing::Draw_action_lines_7049C0(nFLH_X, nFLH_Y, nFLH_Z, pMovingDestCoord->X, pMovingDestCoord->Y, pMovingDestCoord->Z, Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color.Get()), false, false);
		}

		return Skip;
	}

	return Continue;
}

DEFINE_HOOK(0x4DC280, FootClass_DrawActionLines_Move, 0x5)
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
			Drawing::Draw_action_lines_7049C0(nLoc_X, nLoc_Y, nLoc_Z, nCooordDest.X, nCooordDest.Y, nCoordDest_Adjusted_Z, Drawing::RGB2DWORD(pTypeExt->CommandLine_Move_Color.Get()), barg3, false);
		}

		return Skip;
	}

	return Continue;
}

DEFINE_HOOK(0x51CDB9, InfantryClass_RandomAnimate_CheckIdleRate, 0x6)
{
	return R->ESI<InfantryClass* const>()->Type->IdleRate == -1 ? 0x51D0A0 : 0x0;
}

DEFINE_HOOK_AGAIN(0x6FF933, TechnoClass_FireAt_End, 0x5)
DEFINE_HOOK(0x6FDE05, TechnoClass_FireAt_End, 0x5)
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
}

 DEFINE_HOOK(0x701AAD, TechnoClass_ReceiveDamage_WarpedOutBy_Add, 0xA)
{
	enum { NullifyDamage = 0x701AC6, ContinueCheck = 0x701ADB };

	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(bool, bIgnore, 0xD8);

	const bool IsCurrentlyDamageImmune = pThis->IsBeingWarpedOut()
		|| TechnoExtData::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pThis));

	return (IsCurrentlyDamageImmune && !bIgnore) ? NullifyDamage : ContinueCheck;
}

DEFINE_HOOK(0x4DA9C9, FootClass_Update_DeployToLandSound, 0xA)
{
	GET(TechnoTypeClass* const, pType, EAX);
	GET(FootClass* const, pThis, ESI);

	return !pType->JumpJet || pThis->GetHeight() <= 0 ? 0x4DAA01 : 0x4DA9D7;
}

DEFINE_HOOK(0x71B14E, TemporalClass_FireAt_ClearTarget, 0x9)
{
	GET(TemporalClass* const, pThis, ESI);

	const auto pTargetTemp = pThis->Target->TemporalImUsing;

	if (pTargetTemp && pTargetTemp->Target)
		pTargetTemp->LetGo();

	if (pThis->Target->Owner && pThis->Target->Owner->IsControlledByHuman())
		pThis->Target->Deselect();

	return 0x71B17B;
}

// Also fix :
// https://bugs.launchpad.net/ares/+bug/1901825
DEFINE_HOOK(0x71ADE0, TemporalClass_LetGo_Replace, 0x6)
{
	GET(TemporalClass*, pThis, ECX);

	if (auto pTarget = pThis->Target)
	{
		pTarget->BeingWarpedOut = 0;
		pTarget->TemporalTargetingMe = nullptr;
		pThis->Target = nullptr;
	}

	if (auto pPrevTemp = pThis->PrevTemporal)
	{
		if (pThis->NextTemporal == pThis)
			pThis->NextTemporal = nullptr;

		pPrevTemp->Detach();
	}

	if (auto pNextTemp = pThis->NextTemporal)
	{
		if (pNextTemp->PrevTemporal == pThis)
			pNextTemp->PrevTemporal = nullptr;

		pNextTemp->Detach();
	}

	pThis->PrevTemporal = nullptr;
	pThis->NextTemporal = nullptr;
	pThis->unknown_pointer_38 = nullptr;
	pThis->SourceSW = nullptr;

	if (auto pOwner = pThis->Owner)
		pOwner->EnterIdleMode(false, 1);

	return 0x71AE49;

}

//DEFINE_HOOK(0x51D43F, InfantryClass_Scatter_Process, 0x6)
//{
//	GET(InfantryClass* const, pThis, ESI);
//
//	if (pThis->Type->JumpJet && pThis->Type->HoverAttack)
//	{
//		pThis->SetDestination(nullptr, 1);
//		return 0x51D47B;
//	}
//
//	return 0x0;
//}

DEFINE_HOOK(0x5D3ADE, MessageListClass_Init_MessageMax, 0x6)
{
	if (Phobos::Otamaa::IsAdmin)
		R->EAX(14);

	return 0x0;
}

DEFINE_HOOK(0x62A915, ParasiteClass_CanInfect_Parasiteable, 0xA)
{
	enum { continuecheck = 0x62A929, returnfalse = 0x62A976, continuecheckB = 0x62A933 };
	GET(FootClass* const, pVictim, ESI);

	if (TechnoExtData::IsParasiteImmune(pVictim))
		return returnfalse;

	if (pVictim->IsIronCurtained())
		return returnfalse;

	if (pVictim->IsBeingWarpedOut())
		return returnfalse;

	if (TechnoExtData::IsChronoDelayDamageImmune(pVictim))
		return returnfalse;

	return !pVictim->BunkerLinkedItem ? continuecheckB : returnfalse;
}

// replace the repair button fucntion to toggle power
DEFINE_HOOK(0x6A78F6, SidebarClass_AI_RepairMode_ToggelPowerMode, 0x9)
{
	GET(SidebarClass* const, pThis, ESI);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		pThis->SetTogglePowerMode(-1);
	else
		pThis->SetRepairMode(-1);

	return 0x6A78FF;
}

// replace the repair button fucntion to toggle power
DEFINE_HOOK(0x6A7AE1, SidebarClass_AI_DisableRepairButton_TogglePowerMode, 0x6)
{
	GET(SidebarClass* const, pThis, ESI);
	R->AL(Phobos::Config::TogglePowerInsteadOfRepair ? pThis->PowerToggleMode : pThis->RepairMode);
	return 0x6A7AE7;
}

DEFINE_HOOK(0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) ?
		0x508CEE : 0x0;
}

DEFINE_HOOK(0x508EE5, HouseClass_UpdateRadar_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EAX);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) ?
		0x508EEF : 0x0;
}

//DEFINE_HOOK(0x508FCE, HouseClass_SpySat_LimboDeliver, 0x6)
//{
//	GET(BuildingClass*, pBld, ECX);
//
//	return (!pBld->DiscoveredByCurrentPlayer && BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) ?
//		0x508FE1 : 0x0;
//}

DEFINE_HOOK(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
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

DEFINE_HOOK(0x6F09C0, TeamTypeClass_CreateOneOf_Handled, 0x9)
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
	Debug::Log("[%s - %x] Creating a new team named [%s - %x] caller [%x].\n", pHouse->get_ID(), pHouse, pThis->ID, pTeam, caller);
	R->EAX(pTeam);
	return 0x6F0A2C;
}

// DEFINE_HOOK(0x6F09C4, TeamTypeClass_CreateOneOf_RemoveLog, 0x5)
// {
// 	GET_STACK(HouseClass* const, pHouse, STACK_OFFS(0x8, -0x4));
// 	R->EDI(pHouse);
// 	return 0x6F09D5;
// }
//
// DEFINE_HOOK(0x6F0A3F, TeamTypeClass_CreateOneOf_CreateLog, 0x9)
// {
// 	GET(TeamTypeClass* const, pThis, ESI);
// 	GET(HouseClass* const, pHouse, EDI);
// 	const void* ptr = YRMemory::Allocate(sizeof(TeamClass));
// 	Debug::Log("[%s - %x] Creating a new team named [%s - %x].\n", pHouse ? pHouse->get_ID() : NONE_STR2 ,pHouse, pThis->ID, ptr);
// 	R->EAX(ptr);
// 	return 0x6F0A5A;
// }

DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);
//DEFINE_HOOK(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5) { return 0x44DE3C; }

//DEFINE_HOOK(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
//{
//	GET(FactoryClass* const, pThis, ESI);
//	GET(TechnoTypeClass* const, pType, EAX);
//	//Debug::Log("[%x] Factory with Owner '%s' Abandoning production of '%s' \n", pThis, pThis->Owner ? pThis->Owner->get_ID() : NONE_STR2, pType->ID);
//	R->ECX(pThis->Object);
//	return 0x4CA021;
//}

DEFINE_HOOK(0x6E93BE, TeamClass_AI_TransportTargetLog, 0x5)
{
	GET(FootClass* const, pThis, EDI);
	Debug::Log("[%x][%s] Transport just recieved orders to go home after unloading \n", pThis, pThis->get_ID());
	return 0x6E93D6;
}

DEFINE_HOOK(0x6EF9BD, TeamMissionClass_GatherAtEnemyCell_Log, 0x5)
{
	GET(int const, nCellX, EAX);
	GET(int const, nCellY, EDX);
	GET(TeamClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);
	GET(TeamTypeClass* const, pTeamType, ECX);
	Debug::Log("[%x][%s] Team with Owner '%s' has chosen ( %d , %d ) for its GatherAtEnemy cell.\n", pThis, pTeamType->ID, pTechno->Owner ? pTechno->Owner->get_ID() : NONE_STR2, nCellX, nCellY);
	return 0x6EF9D0;
}

DEFINE_HOOK(0x5F54A8, ObjectClass_ReceiveDamage_ConditionYellow, 0x6)
{
	enum { ContinueCheck = 0x5F54C4, ResultHalf = 0x5F54B8 };

	GET(int const, nCurStr, EDX);
	GET(int const, nMaxStr, EBP);
	GET(int const, nDamage, ECX);

	const auto curstr = int(nMaxStr * RulesClass::Instance->ConditionYellow);
	return (nCurStr >= curstr && (nCurStr - nDamage) < curstr) ? ResultHalf : ContinueCheck;
}

DEFINE_HOOK(0x6D912B, TacticalClass_Render_BuildingInLimboDeliveryA, 0x9)
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

DEFINE_HOOK(0x6D966A, TacticalClass_Render_BuildingInLimboDeliveryB, 0x9)
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

DEFINE_HOOK(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x7)
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
namespace Tiberiumpip
{
	struct PackedTibPipData
	{
		int value;
		int pipIdx;
	};

	int GetEmptyShapeIndex(bool isWeeder, TechnoTypeExtData* pTypeData)
	{

		if (isWeeder && pTypeData->Weeder_PipEmptyIndex.isset())
			return pTypeData->Weeder_PipEmptyIndex;
		else if (pTypeData->Tiberium_EmptyPipIdx.isset())
			return pTypeData->Tiberium_EmptyPipIdx;

		return 0;
	}

	int DrawFrames(
		bool IsWeeder,
		TechnoTypeExtData* pTypeData,
		std::vector<PackedTibPipData>& Amounts,
		const Iterator<int> orders)
	{
		for (size_t i = 0; i < (size_t)TiberiumClass::Array->Count; i++)
		{
			size_t index = i;
			if (i < orders.size())
				index = orders[i];

			if (Amounts[index].value > 0)
			{
				--Amounts[index].value;
				return Amounts[index].pipIdx;
			}
		}

		return GetEmptyShapeIndex(IsWeeder, pTypeData);
	};

	int GetShapeIndex(int storageIndex, TechnoTypeExtData* pTypeData)
	{
		auto frames = pTypeData->Tiberium_PipIdx.GetElements(RulesExtData::Instance()->Pips_Tiberiums_Frames);
		const auto pTibExt = TiberiumExtContainer::Instance.Find(TiberiumClass::Array->Items[storageIndex]);

		return (size_t)storageIndex >= frames.size() || frames[storageIndex] < 0 ? pTibExt->PipIndex : frames[storageIndex];
	}

	void DrawTiberiumPip(TechnoClass* pTechno, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
	{
		if (!pTechno)
			return;

		const auto pType = pTechno->GetTechnoType();
		const auto nMax = pType->GetPipMax();

		if (!nMax)
			return;

		auto const nStorage = pType->Storage;
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		const auto what = pTechno->WhatAmI();

		Point2D nOffs {};
		const auto pBuilding = what == BuildingClass::AbsID ? static_cast<BuildingClass*>(pTechno) : nullptr;
		const auto pUnit = what == UnitClass::AbsID ? static_cast<UnitClass*>(pTechno) : nullptr;

		auto pShape = pBuilding ?
			pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS2_SHP());

		if (pTypeExt->Tiberium_PipShapes)
		{
			pShape = pTypeExt->Tiberium_PipShapes;
		}

		ConvertClass* nPal = FileSystem::THEATER_PAL();
		if (auto pConv = pTypeExt->Tiberium_PipShapes_Palette)
		{
			if (auto pConv_ = pConv->GetConvert<PaletteManager::Mode::Temperate>())
			{
				nPal = pConv_;
			}
		}

		auto storage = &TechnoExtContainer::Instance.Find(pTechno)->TiberiumStorage;


		static std::vector<PackedTibPipData> Amounts(TiberiumClass::Array->Count);

		const bool isWeeder = pBuilding&& pBuilding->Type->Weeder || pUnit && pUnit->Type->Weeder;

		for (size_t i = 0; i < Amounts.size(); i++)
		{
			int FrameIdx = 0;
			int amount = 0;

			if (pBuilding && pBuilding->Type->Weeder)
			{
				amount = int(pTechno->Owner->GetWeedStoragePercentage() * nMax + 0.5);
			}
			else
			{
				amount = int(storage->m_values[i] / nStorage * nMax + 0.5);
			}

			if (!isWeeder)
			{
				FrameIdx = GetShapeIndex(i, pTypeExt);
			}
			else
			{
				FrameIdx = pTypeExt->Weeder_PipIndex.Get(1);
			}

			Amounts[i] = { amount , FrameIdx };
		}

		static constexpr std::array<int, 4u> defOrder { {0, 2, 3, 1} };
		const auto displayOrders = RulesExtData::Instance()->Pips_Tiberiums_DisplayOrder.GetElements(make_iterator(&defOrder[0], 4u));

		for (int i = nMax; i; --i)
		{
			Point2D nPointHere { nOffs.X + nPoints->X  , nOffs.Y + nPoints->Y };
			CC_Draw_Shape(
			DSurface::Temp(),
			nPal,
			pShape,
			DrawFrames(isWeeder, pTypeExt, Amounts, displayOrders),
			&nPointHere,
			pRect,
			0x600,
			0,
			0,
			0,
			1000,
			0,
			0,
			0,
			0,
			0);

			nOffs.X += nOffsetX;
			nOffs.Y += nOffsetY;
		}
	}

}

void DrawSpawnerPip(TechnoClass* pTechno, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
	{
		const auto pType = pTechno->GetTechnoType();
		const auto nMax = pType->SpawnsNumber;

		if (nMax <= 0)
			return;

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		Point2D nOffs {};

		const auto pBuilding = specific_cast<BuildingClass*>(pTechno);
		const auto pShape = pBuilding ?
			pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS_SHP());

		ConvertClass* nPal = FileSystem::THEATER_PAL();

		//if (pBuilding)
		//{
		//	const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
		//
		//	if (pBuildingTypeExt->PipShapes01Remap)
		//		nPal = pTechno->GetRemapColour();
		//	else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
		//		nPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
		//
		//}

		for (int i = 0; i < nMax; i++)
		{
			const auto nSpawnMax = pTechno->SpawnManager ? pTechno->SpawnManager->CountDockedSpawns() : 0;

			Point2D nPointHere { nOffs.X + nPoints->X  , nOffs.Y + nPoints->Y };
			CC_Draw_Shape(
				DSurface::Temp(),
				nPal,
				pShape,
				i < nSpawnMax,
				&nPointHere,
				pRect,
				0x600,
				0,
				0,
				0,
				1000,
				0,
				0,
				0,
				0,
				0);

			nOffs.X += nOffsetX;
			nOffs.Y += nOffsetY;
		}
	}

DEFINE_HOOK(0x70A1F6, TechnoClass_DrawPips_Tiberium, 0x6)
{
	struct __declspec(align(4)) PipDataStruct
	{
		Point2D nPos;
		int Int;
		int Number;
	};

	GET(TechnoClass* const, pThis, EBP);
	GET_STACK(PipDataStruct, pDatas, STACK_OFFS(0x74, 0x24));
	//GET_STACK(SHPStruct*, pShapes, STACK_OFFS(0x74, 0x68));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x74, 0xC));
	GET(int, nOffsetY, ESI);

	Tiberiumpip::DrawTiberiumPip(pThis, &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	//DrawCargoPips_NotBySize(pThis, pThis->GetTechnoType()->GetPipMax(), &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	return 0x70A340;
}

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

constexpr std::array<const char*, (size_t)NewVHPScan::count> NewVHPScanToString
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

DEFINE_HOOK(0x477590, CCINIClass_ReadVHPScan_Replace, 0x6)
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

//DEFINE_HOOK(0x6F8721, TechnoClass_EvalObject_VHPScan, 0x7)
//{
//	GET(TechnoClass* const, pThis, EDI);
//	GET(ObjectClass* const, pTarget, ESI);
//	GET(int*, pRiskValue, EBP);
//
//	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
//	const auto pTechnoTarget = generic_cast<TechnoClass* const>(pTarget);
//
//	int nValue = pExt->VHPscan_Value;
//	if (nValue <= 0)
//		nValue = 2;
//
//	switch (NewVHPScan(pExt->AttachedToObject->VHPScan))
//	{
//	case NewVHPScan::Normal:
//	{
//		if (pTarget->EstimatedHealth <= 0)
//		{
//			*pRiskValue /= nValue;
//			break;
//		}
//
//		if (pTarget->EstimatedHealth > (pTarget->GetType()->Strength / 2))
//			break;
//
//		*pRiskValue *= nValue;
//	}
//	break;
//	case NewVHPScan::Threat:
//	{
//		*pRiskValue *= (*pRiskValue / nValue);
//		break;
//	}
//	break;
//	case NewVHPScan::Health:
//	{
//		int nRes = *pRiskValue;
//		if (pTarget->EstimatedHealth > pTarget->GetType()->Strength / 2)
//			nRes = nValue * nRes;
//		else
//			nRes = nRes / nValue;
//
//		*pRiskValue = nRes;
//	}
//	break;
//	case NewVHPScan::Damage:
//	{
//		if (!pTechnoTarget)
//		{
//			*pRiskValue = 0;
//			break;
//		}
//
//		*pRiskValue = pTechnoTarget->CombatDamage(-1) / nValue * (*pRiskValue);
//	}
//	break;
//	case NewVHPScan::Value:
//	{
//		if (!pTechnoTarget)
//		{
//			*pRiskValue = 0;
//			break;
//		}
//
//		const int nSelectedWeapon = pTechnoTarget->SelectWeapon(pThis);
//		const auto nFireError = pTechnoTarget->GetFireError(pThis, nSelectedWeapon, 0);
//		if (nFireError == FireError::NONE ||
//			nFireError == FireError::FACING ||
//			nFireError == FireError::REARM ||
//			nFireError == FireError::ROTATING
//			)
//		{
//			*pRiskValue *= nValue;
//			break;
//		}
//
//		*pRiskValue /= nValue;
//	}
//	break;
//	case NewVHPScan::Non_Infantry:
//	{
//		if (!pTechnoTarget || pTechnoTarget->WhatAmI() == InfantryClass::AbsID)
//		{
//			*pRiskValue = 0;
//		}
//	}
//	break;
//	default:
//		break;
//	}
//
//	return 0x6F875F;
//}

DEFINE_HOOK(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7)
{
	GET(InfantryClass* const, pThis, ECX);

	enum { SkipWholeFunction = 0x5192BC, Continue = 0x0 };

	return InfantryTypeExtContainer::Instance.Find(pThis->Type)->HideWhenDeployAnimPresent.Get()
		&& pThis->DeployAnim ? SkipWholeFunction : Continue;
}

CoordStruct* __fastcall UnitClass_GetFLH(UnitClass* pThis, DWORD, CoordStruct* buffer, int wepon, CoordStruct base)
{
	if (pThis->InOpenToppedTransport && pThis->Transporter)
	{
		const int idx = pThis->Transporter->Passengers.IndexOf(pThis);
		if (idx != 0)
		{
			return pThis->Transporter->GetFLH(buffer, -idx, base);
		}
	}

	return pThis->TechnoClass::GetFLH(buffer, wepon, base);
}

DEFINE_JUMP(VTABLE, 0x7F5D20, GET_OFFSET(UnitClass_GetFLH));

DEFINE_HOOK(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
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

DEFINE_HOOK(0x6FA167, TechnoClass_AI_DrainMoney, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::ApplyDrainMoney(pThis);
	return 0x6FA1C5;
}

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D90 };

	GET(ObjectClass* const, pThis, ECX);
	GET_STACK(TechnoClass* const, pTechno, STACK_OFFSET(0x8, -0x4));
	R->AL(TechnoExtData::IsCrushable(pThis, pTechno));

	return SkipGameCode;
}

DEFINE_HOOK(0x629BB2, ParasiteClass_UpdateSquiddy_Culling, 0x8)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(WarheadTypeClass* const, pWH, EDI);

	enum { ApplyDamage = 0x629D19, GainExperience = 0x629BF3, SkipGainExperience = 0x629C5D };

	if (!WarheadTypeExtContainer::Instance.Find(pWH)->applyCulling(pThis->Owner, pThis->Victim))
		return ApplyDamage;

	return pThis->Owner && pThis->Owner->Owner && pThis->Owner->Owner->IsAlliedWith(pThis->Victim)
		? SkipGainExperience : GainExperience;
}

DEFINE_HOOK(0x702721, TechnoClass_ReceiveDamage_DamagedSound, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pThisType, EAX);
	VoxClass::PlayAtPos(pThisType->DamageSound, &pThis->Location);
	return 0x7027AE;
}

DEFINE_HOOK(0x4426DB, BuildingClass_ReceiveDamage_DisableDamagedSoundFallback, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (BuildingTypeExtContainer::Instance.Find(pThis->Type)->DisableDamageSound.Get())
	{
		return R->Origin() + 0x6;
	}

	return 0x0;
}

// this just an duplicate
DEFINE_JUMP(LJMP, 0x702765, 0x7027AE);

DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass* const, pProduct, ESI);
	VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pProduct->GetTechnoType())->Eva_Complete.Get());
	return 0x4FB649;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
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
					VocClass::PlayAt(pTechnoTypeExt->VoiceCreate, pTechno->Location);
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

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass* const, pTech, ESI);

	if (pTech->WhatAmI() == BuildingClass::AbsID)
	{
		VoxClass::PlayIndex(TechnoTypeExtContainer::Instance.Find(pTech->GetTechnoType())->Eva_Complete.Get());
		return 0x6A8E34;
	}

	return 0x0;
}

DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x6)
{
	GET(AnimClass*, pAnim, EDI);
	GET(AnimClass*, pThis, ESI);

	auto nCoord = pThis->GetCoords();
	pAnim->AnimClass::AnimClass(pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	const auto pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	TechnoClass* const pTech = AnimExtData::GetTechnoInvoker(pThis);
	HouseClass* const pOwner = !pThis->Owner && pTech ? pTech->Owner : pThis->Owner;
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);

	return 0x424322;
}

//DEFINE_HOOK(0x51DF82, InfantryClass_FireAt_StartReloading, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	const auto pType = pThis->Type;
//
//	if(pThis->Transporter) {
//		if (TechnoTypeExtContainer::Instance.Find(pType)->ReloadInTransport
//			&& pType->Ammo > 0
//			&& pThis->Ammo < pType->Ammo
//		)
//			pThis->StartReloading();
//	}
//
//	return 0;
//}

DEFINE_HOOK(0x739450, UnitClass_Deploy_LocationFix, 0x7)
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

DEFINE_HOOK(0x6F9F42, TechnoClass_AI_Berzerk_SetMissionAfterDone, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExtData::SetMissionAfterBerzerk(pThis);
	return 0x6F9F6E;
}

DEFINE_HOOK(0x6FF4B0, TechnoClass_FireAt_TargetLaser, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (!pTypeExt->TargetLaser_WeaponIdx.empty()
		&& !pTypeExt->TargetLaser_WeaponIdx.Contains(pExt->CurrentWeaponIdx))
	{
		return 0x6FF4CC;
	}

	pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
	return 0x6FF4CC;
}

DEFINE_HOOK(0x70FB50, TechnoClass_Bunkerable, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	if (const auto pFoot = generic_cast<FootClass*>(pThis)) {

		const auto pType = pFoot->GetTechnoType();
		if (!pType->Bunkerable || !pType->Turret) {
			R->EAX(false);
			return 0x70FBCA;
		}

		if (!pFoot->IsArmed()) {
			R->EAX(false);
			return 0x70FBCA;
		}

		const auto nSpeedType = pType->SpeedType;
		if (nSpeedType == SpeedType::Hover
			|| nSpeedType == SpeedType::Winged
			|| nSpeedType == SpeedType::None) {
			R->EAX(false);
			return 0x70FBCA;
		}

		if (pFoot->ParasiteEatingMe) {
			R->EAX(false);
			return 0x70FBCA;
		}

		//crash the game , dont allow it
		//maybe because of force_track stuffs,..
		if (locomotion_cast<HoverLocomotionClass*>(pFoot->Locomotor)) {
			R->EAX(false);
			return 0x70FBCA;
		}

		R->EAX(true);
		return 0x70FBCA;
	}

	R->EAX(false);
	return 0x70FBCA;
}

DEFINE_HOOK(0x708F77, TechnoClass_ResponseToSelect_BugFixes, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);

	return pThis->IsCrashing || pThis->Health < 0 ?
		0x708FAD : 0x0;
}

DEFINE_HOOK(0x6EE17E, MoveCrameraToWaypoint_CancelFollowTarget, 0x8)
{
	DisplayClass::Instance->FollowAnObject(nullptr);
	return 0x0;
}

DEFINE_HOOK(0x437C29, sub_437A10_Lock_Bound_Fix, 7)
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

DEFINE_HOOK(0x73D909, UnitClass_Mi_Unload_LastPassengerOut, 8)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Passengers.NumPassengers < pThis->NonPassengerCount)
	{
		pThis->MissionStatus = 4;
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();
		pThis->unknown_bool_B8 = true;
	}

	return 0x0;
}

//TechnoClass_GetWeaponState
DEFINE_HOOK(0x6FCA30, TechnoClass_GetFireError_DecloakToFire, 6)
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

DEFINE_HOOK(0x741554, UnitClass_ApproachTarget_CrushRange, 0x6)
{
	enum { Crush = 0x741599, ContinueCheck = 0x741562 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	return range >= pTypeExt->CrushRange.GetOrDefault(pThis, RulesClass::Instance->Crush) ?
		Crush : ContinueCheck;
}

DEFINE_HOOK(0x7439AD, UnitClass_ShouldCrush_CrushRange, 0x6)
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
DEFINE_HOOK(0x73D6EC, UnitClass_Unload_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x73DCD3 : 0x0;
}

DEFINE_HOOK(0x740015, UnitClass_WhatAction_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualEject.Get() ? 0x7400F0 : 0x0;
}

// DEFINE_HOOK(0x711F0F, TechnoTypeClass_GetCost_AICostMult, 0x8)
// {
// 	GET(HouseClass* const, pHouse, EDI);
// 	GET(TechnoTypeClass* const, pType, ESI);
//
// 	double result = int(pType->GetCost() * pHouse->GetHouseCostMult(pType) * pHouse->GetHouseTypeCostMult(pType));
//
// 	if(!pHouse->IsControlledByHuman())
// 		result *= RulesExtData::Instance()->AI_CostMult;
//
// 	R->EAX(result);
// 	return 0x711F46;
// }

DEFINE_JUMP(LJMP, 0x422A59, 0x422A5F);

DEFINE_HOOK(0x4251AB, AnimClass_Detach_LogTypeDetached, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	Debug::Log("Anim[0x%x] detaching Type Pointer ! \n", pThis);
	return 0x0;
}

DEFINE_HOOK(0x6F357F, TechnoClass_SelectWeapon_DrainWeaponTarget, 0x6)
{
	enum { CheckAlly = 0x6F3589, ContinueCheck = 0x6F35A8, RetPrimary = 0x6F37AD };

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EBP);

	const bool IsTargetEligible = !pThis->DrainTarget && !pTarget->DrainingMe;
	return IsTargetEligible ?
		CheckAlly : ContinueCheck;
}

DEFINE_HOOK(0x71AA13, TemporalClass_AI_BunkerLinked_Check, 0x7)
{
	GET(BuildingClass* const, pBld, EAX);
	return pBld ? 0x0 : 0x71AA1A;
}

// DEFINE_HOOK(0x447195, BuildingClass_SellBack_Silent, 0x6)
// {
// 	GET(BuildingClass* const, pThis, ESI);
// 	return BuildingExtContainer::Instance.Find(pThis)->Silent ? 0x447203 : 0x0;
// }

DEFINE_HOOK(0x51F885, InfantryClass_WhatAction_TubeStuffs_FixGetCellAtCallTwice, 0x7)
{
	enum { retTrue = 0x51F8A6, retFalse = 0x51F8A8 };
	GET(CellClass* const, pCell, EAX);

	return pCell->CellClass_Tube_484AE0() || pCell->CellClass_Tube_484D60() ?
		retTrue : retFalse;
}

DEFINE_HOOK(0x51F9B7, InfantryClass_WhatAction_TubeStuffs_FixGetCellAtCallTwice_part2, 0x7)
{
	enum { retFalse = 0x51F953 };
	GET(CellClass* const, pCell, EAX);
	GET(InfantryClass* const, pThis, EDI);

	if (!pCell->Tile_Is_Tunnel())
		return retFalse;

	R->EAX(pCell->CellClass_484F10(pThis));
	return 0x51F9D5;
}

DEFINE_HOOK(0x7008E5, TechnoClass_WhatAction_FixGetCellAtCallTwice, 0x9)
{
	GET(CellClass* const, pCell, EAX);
	GET(TechnoClass* const, pThis, ESI);

	R->EBP(pThis->SelectWeapon(pCell));
	return 0x7008FB;
}

// Various call of TechnoClass::SetOwningHouse not respecting overloaded 2nd args fix !

DEFINE_HOOK(0x7463DC, UnitClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(UnitClass* const, pThis, EDI);
	GET(HouseClass* const, pNewOwner, EBX);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	R->EAX(pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce));
	return 0x7463E6;
}

DEFINE_HOOK(0x4DBF01, FootClass_SetOwningHouse_FixArgs, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	GET_STACK(HouseClass* const, pNewOwner, 0xC + 0x4);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	//Debug::Log("SetOwningHouse for [%s] announce [%s - %d]\n", pNewOwner->get_ID(), bAnnounce ? "True" : "False" , bAnnounce);
	bool result = false;
	if (pThis->TechnoClass::SetOwningHouse(pNewOwner, bAnnounce))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pThis);

		for (auto& trail : pExt->LaserTrails)
		{
			if (trail.Type->IsHouseColor)
			{
				trail.CurrentColor = pThis->Owner->LaserColor;
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
			pThis->CurrentTargets.Clear();

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

static reference<RectangleStruct, 0x87F8D4> const MapSize {};

 constexpr int GetMapSizeTotals() {
	return (2 * MapSize->Width * (MapSize->Height + 4));
}

 constexpr int CellStruct_totibarray_42B1C0(CellStruct* pCell)
{
	return (((pCell->X - pCell->Y + MapSize->Width - 1) >> 1) + MapSize->Width * (pCell->X - MapSize->Width + pCell->Y - 1));
}

 constexpr void Heapify(TPriorityQueueClass<MapSurfaceData>* pHeap, int index)
{
	int left = 2 * index;
	int right = 2 * index + 1;
	if (2 * index > pHeap->Count || pHeap->Nodes[index]->Score <= (double)pHeap->Nodes[2 * index]->Score)
	{
		left = index;
	}
	if (right <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[right]->Score)
	{
		left = 2 * index + 1;
	}
	while (left != index)
	{
		auto left_of_left = 2 * left + 1;
		auto entry = pHeap->Nodes[index];
		pHeap->Nodes[index] = pHeap->Nodes[left];
		index = left;
		pHeap->Nodes[left] = entry;
		if (2 * left <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[2 * left]->Score)
		{
			left *= 2;
		}
		if (left_of_left <= pHeap->Count && pHeap->Nodes[left]->Score > (double)pHeap->Nodes[left_of_left]->Score)
		{
			left = left_of_left;
		}
	}
}


 struct CellPatch__ : public CellClass
 {
	 bool _SpreadTiberium(bool force)
	 {
		 int tib_ = -1;
		 if (!force && !ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
			 return false;

		 tib_ = OverlayClass::GetTiberiumType(this->OverlayTypeIndex);

		 if (!force) {

			 if (tib_ == -1
				   || this->OverlayData <= tib_ / 2
				   || this->SlopeIndex
				   || TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
				   || this->FirstObject)  {
				 return false;
			 }

		 }  else  {
			 if (tib_ == -1) {
				 tib_ = 0;
			 }
		 }

		 auto pTib = TiberiumClass::Array->Items[tib_];
		 auto facing = (BYTE)ScenarioClass::Instance->Random.RandomRangedSpecific(FacingType::Min, FacingType::Max);
		 int index = 0;
		 CellClass* newcell = nullptr;

		 while (true)
		 {
			 const auto v9 = ((BYTE)index + facing) & 7;

			 if (v9 < 8)
			 {
				 newcell = MapClass::Instance->GetCellAt(CellSpread::AdjacentCell[v9] + this->MapCoords);
			 }
			 else
			 {
				 newcell = this;
			 }
			 if (newcell && newcell->CanTiberiumGerminate(pTib))
			 {
				 break;
			 }

			 if (++index >= 8)
			 {
				 return false;
			 }
		 }

		 return newcell->IncreaseTiberium(tib_, 3);
	 }

	 bool _SpreadTiberium_2(TerrainClass* pTerrain, bool force)
	 {
		 if (!pTerrain)
			 Debug::FatalErrorAndExit(__FUNCTION__" Need `TerrainClass` !\n");

		 auto pTerrainTypeExt = TerrainTypeExtContainer::Instance.Find(pTerrain->Type);

		 int tib_ = pTerrainTypeExt->SpawnsTiberium_Type;

		 if (!force)
		 {
			 if (!ScenarioClass::Instance->SpecialFlags.StructEd.TiberiumSpreads)
			 {
				 return false;
			 }

			 if (tib_ <= -1 || tib_ >= TiberiumClass::Array->Count || (TiberiumClass::Array->Items[tib_]->SlopeFrames <= 0 && this->SlopeIndex))
				 return false;

			 if (TiberiumClass::Array->Items[tib_]->SpreadPercentage < 0.00001
				 || this->FirstObject)
			 {
				 return false;
			 }

		 }
		 else
		 {
			 if (tib_ <= -1 || tib_ >= TiberiumClass::Array->Count)
			 {
				 tib_ = 0;
			 }
		 }

		 auto pTib = TiberiumClass::Array->Items[tib_];
		 auto pTerrainExt = TerrainExtContainer::Instance.Find(pTerrain);
		 size_t size = pTerrainExt->Adjencentcells.size();
		 const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
		 const int growth = pTerrainTypeExt->GetTiberiumGrowthStage();

		 for (int i = 0; i < (int)size; i++)
		 {
			 CellClass* tgtCell = MapClass::Instance->GetCellAt(this->MapCoords + pTerrainExt->Adjencentcells[(i + rand) % size]);

			 if (tgtCell->CanTiberiumGerminate(pTib)) {
				 return tgtCell->IncreaseTiberium(tib_, growth);
			 }
		 }

		 return false;
	 }

	 static void __fastcall __SpreadTiberiumWrapper(CellPatch__* pThis, DWORD, bool force)
	 {
		 pThis->_SpreadTiberium(force);
	 }
 };


struct TibPatch__ : public TiberiumClass {

#pragma region Spread

	void __RecalcSpreadData()
	{
		this->Spread = 0;
		this->SpreadLogic.Heap->Clear();

		const int MapTotal = GetMapSizeTotals();
		for (int i = MapTotal - 1; i >= 0; this->SpreadLogic.States[i + 1] = 0)
		{
			--i;
		}

		MapClass::Instance->CellIteratorReset();
		for (auto j = MapClass::Instance->CellIteratorNext(); j; j = MapClass::Instance->CellIteratorNext())
		{
			if (j->GetContainedTiberiumIndex() == this->ArrayIndex && j->CanTiberiumSpread())
			{
				this->SpreadLogic.Datas[this->Spread].MapCoord = j->MapCoords;
				this->SpreadLogic.Datas[this->Spread].Score = 0.0;
				this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
				++this->Spread;
				this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&j->MapCoords)] = true;
			}
		}
	}

	void __QueueSpreadAt(CellStruct* pCell)
	{
		int tib_arr = CellStruct_totibarray_42B1C0(pCell);
		auto pCellClass = MapClass::Instance->GetCellAt(pCell);

		if (pCellClass->CanTiberiumSpread() && !this->SpreadLogic.States[tib_arr]) {
			if (this->Spread >= GetMapSizeTotals() - 20)
			 __RecalcSpreadData();

			this->SpreadLogic.Datas[this->Spread].MapCoord = *pCell;
			this->SpreadLogic.Datas[this->Spread].Score = (float)(Unsorted::CurrentFrame + (int)abs(abs(ScenarioClass::Instance->Random.Random() % 50)));
			this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
			++this->Spread;
			this->SpreadLogic.States[tib_arr] = true;
		}
	}

	void __Spread()
	{
		auto spreadHeaps = this->SpreadLogic.Heap;

		if (spreadHeaps && spreadHeaps->Count && this->SpreadPercentage > 0.00001)
		{
			int get_percent = int(spreadHeaps->Count * this->SpreadPercentage);

			if (get_percent < 5)
				get_percent = 5;
			else if (get_percent > 25)
				get_percent = 25;

			if (spreadHeaps->Count > GetMapSizeTotals() - 20)
				this->__RecalcSpreadData();

			MapSurfaceData* pSurface = nullptr;
			if (auto poped = spreadHeaps->Top())
			{
				pSurface = poped;
				Heapify(spreadHeaps, 1);
			}

			int size_after = abs(ScenarioClass::Instance->Random.Random()) % get_percent + 1;

			int increment = 0;
			if (size_after > 0)
			{
				while (true)
				{
					while (true)
					{
						if (!pSurface)
							return;

						int getminateIdx = 0;
						auto pNewCell = MapClass::Instance->GetCellAt(pSurface->MapCoord);
						for (int c = 0; c < 8; ++c) {
							auto pAdjencent = pNewCell->GetAdjacentCell((FacingType)c);
							if (pAdjencent->CanTiberiumGerminate(nullptr))
								++getminateIdx;
						}

						if (getminateIdx)
						{
							((CellPatch__*)(pNewCell))->_SpreadTiberium(false);
							++increment;
							if (getminateIdx > 1)
							{
								this->SpreadLogic.Datas[this->Spread].MapCoord = pNewCell->MapCoords;
								this->SpreadLogic.Datas[this->Spread].Score = 0.0f;
								this->SpreadLogic.Heap->WWPush(&this->SpreadLogic.Datas[this->Spread]);
								++this->Spread;
								this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = true;
							}
						}
						else
						{
							this->SpreadLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = false;
						}

						if (++increment >= size_after)
							return;

						if (this->SpreadLogic.Heap->Count)
							break;
					}

					Heapify(this->SpreadLogic.Heap, 1);
				}
			}
		}
	}

#pragma endregion

#pragma region Growth

	void __RecalcGrowthData()
	{
		this->Growth = 0;
		this->GrowthLogic.Heap->Clear();
		for (int i = GetMapSizeTotals() - 1; i >= 0; this->GrowthLogic.States[i + 1] = 0)
		{
			--i;
		}

		MapClass::Instance->CellIteratorReset();
		for (auto j = MapClass::Instance->CellIteratorNext(); j; j = MapClass::Instance->CellIteratorNext())
		{
			if (j->GetContainedTiberiumIndex() == this->ArrayIndex && j->CanTiberiumGrowth())
			{
				auto p_Position = &j->MapCoords;
				auto v16 = p_Position;
				this->GrowthLogic.Datas[this->Growth].MapCoord = j->MapCoords;
				this->GrowthLogic.Datas[this->Growth].Score = 0.0;
				this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
				++this->Growth;
				this->GrowthLogic.States[CellStruct_totibarray_42B1C0(&j->MapCoords)] = true;
			}
		}
	}

	void __QueueGrowthAt(CellStruct* pCell)
	{
		int tib_arr = CellStruct_totibarray_42B1C0(pCell);
		auto pCellClass = MapClass::Instance->GetCellAt(pCell);

		if (pCellClass->OverlayData < 11u)
		{
			if (this->Growth >= GetMapSizeTotals() - 10)
				__RecalcGrowthData();

			this->GrowthLogic.Datas[this->Growth].MapCoord = *pCell;
			this->GrowthLogic.Datas[this->Growth].Score = (float)(Unsorted::CurrentFrame + (int)abs(abs(ScenarioClass::Instance->Random.Random() % 50)));
			this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
			++this->Growth;
			this->GrowthLogic.States[tib_arr] = true;
		}
	}

	void __Growth() {
		auto growthHeaps = this->GrowthLogic.Heap;

		if (growthHeaps && growthHeaps->Count && this->GrowthPercentage > 0.00001)
		{
			int get_percent = int(growthHeaps->Count * this->GrowthPercentage);

			if (get_percent < 5)
				get_percent = 5;
			else if (get_percent > 50)
				get_percent = 50;

			int copy_heapSize = growthHeaps->Count;
			int size_after = abs(ScenarioClass::Instance->Random.Random()) % get_percent + 1;

			if (copy_heapSize > GetMapSizeTotals() - 2 * size_after) {
				this->__RecalcGrowthData();
			}

			MapSurfaceData* pSurface = nullptr;
			if (auto poped = growthHeaps->Top()) {
				pSurface = poped;
				Heapify(growthHeaps, 1);
			}

			int increment = 0;
			if (size_after > 0) {

				while (true)
				{
					while (true)
					{
						if (!pSurface)
							return;

						auto pNewCell = MapClass::Instance->GetCellAt(pSurface->MapCoord);

						if (pNewCell->GetContainedTiberiumIndex() == this->ArrayIndex)
						{
							pNewCell->GrowTiberium();
							if (pNewCell->OverlayData >= 11u) {
								this->GrowthLogic.States[CellStruct_totibarray_42B1C0(&pNewCell->MapCoords)] = false;
							}
							else
							{
								this->GrowthLogic.Datas[this->Growth].MapCoord = pNewCell->MapCoords;
								this->GrowthLogic.Datas[this->Growth].Score = (float)(int)(Unsorted::CurrentFrame() + abs(ScenarioClass::Instance->Random.Random() % 50));
								this->GrowthLogic.Heap->WWPush(&this->GrowthLogic.Datas[this->Growth]);
								++this->Growth;
								__QueueSpreadAt(&pNewCell->MapCoords);
							}
						}
						if (++increment >= size_after)
							return;

						if (this->GrowthLogic.Heap->Count)
							break;
					}

					Heapify(growthHeaps, 1);
				}
			}
		}
	}

#pragma endregion

#pragma region Wrappers
	static void __fastcall __GrowthWrapper(TibPatch__* pThis, DWORD)
	{
		pThis->__Growth();
	}

	static void __fastcall __RecalcGrowthWrapper(TibPatch__* pThis, DWORD)
	{
		pThis->__RecalcGrowthData();
	}

	static void __fastcall __QueueGrowthAtWrapper(TibPatch__* pThis, DWORD, CellStruct* pCell)
	{
		pThis->__QueueGrowthAt(pCell);
	}

	static void __fastcall __SpreadWrapper(TibPatch__* pThis, DWORD)
	{
		pThis->__Spread();
	}

	static void __fastcall __RecalcSpreadWrapper(TibPatch__* pThis, DWORD)
	{
		pThis->__RecalcSpreadData();
	}

	static void __fastcall __QueueSpreadAtWrapper(TibPatch__* pThis, DWORD, CellStruct* pCell)
	{
		pThis->__QueueSpreadAt(pCell);
	}
#pragma endregion

};

DEFINE_JUMP(LJMP, 0x722440, GET_OFFSET(TibPatch__::__SpreadWrapper));
DEFINE_JUMP(LJMP, 0x7228B0, GET_OFFSET(TibPatch__::__RecalcSpreadWrapper));
DEFINE_JUMP(LJMP, 0x722AF0, GET_OFFSET(TibPatch__::__QueueSpreadAtWrapper));
DEFINE_JUMP(LJMP, 0x722F00, GET_OFFSET(TibPatch__::__GrowthWrapper));
DEFINE_JUMP(LJMP, 0x7233A0, GET_OFFSET(TibPatch__::__RecalcGrowthWrapper));
DEFINE_JUMP(LJMP, 0x7235A0, GET_OFFSET(TibPatch__::__QueueGrowthAtWrapper));

DEFINE_JUMP(LJMP, 0x483780, GET_OFFSET(CellPatch__::__SpreadTiberiumWrapper));

DEFINE_HOOK(0x71C84D, TerrainClass_AI_Animated, 0x6)
{
	enum { SkipGameCode = 0x71C8D5 };

	GET(TerrainClass* const, pThis, ESI);

	if (pThis->Type) {
		if (pThis->Type->IsAnimated) {
			auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
			if (auto pImage = pThis->Type->GetImage()) {
				if (pThis->Animation.Value == pTypeExt->AnimationLength.Get(pImage->Frames / (2 * (pTypeExt->HasDamagedFrames + 1)))) {
					pThis->Animation.Value = 0;
					pThis->Animation.Start(0);

					if (pThis->Type->SpawnsTiberium && MapClass::Instance->IsValid(pThis->Location)) {
						for (int i = 0; i < pTypeExt->GetCellsPerAnim(); i++)
							((CellPatch__*)MapClass::Instance->GetCellAt(pThis->Location))->_SpreadTiberium_2(pThis, true);
					}
				}
			}
			else { Debug::Log("Terrain [%s] With Corrupted Image !\n", pThis->Type->get_ID()); }
		}
	}

	return SkipGameCode;
}

BuildingClass* IsAnySpysatActive(HouseClass* pThis)
{
	const bool IsCurrentPlayer = pThis->ControlledByCurrentPlayer();

	//===============reset all
	pThis->CostDefensesMult = 1.0;
	pThis->CostUnitsMult = 1.0;
	pThis->CostInfantryMult = 1.0;
	pThis->CostBuildingsMult = 1.0;
	pThis->CostAircraftMult = 1.0;
	BuildingClass* Spysat = nullptr;
	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis);

	pHouseExt->Building_BuildSpeedBonusCounter.clear();
	pHouseExt->Building_OrePurifiersCounter.clear();
	//==========================
	//const bool LowpOwerHouse = pThis->HasLowPower();

	for (auto const& pBld : pThis->Buildings)
	{
		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto pExt = BuildingExtContainer::Instance.Find(pBld);
			const bool IsLimboDelivered = pExt->LimboID != -1;

			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
				continue;

			if (pBld->TemporalTargetingMe
				|| pExt->AboutToChronoshift
				|| pBld->IsBeingWarpedOut())
				continue;

			const bool Online = pBld->IsPowerOnline(); // check power
			const auto pTypes = pBld->GetTypes(); // building types include upgrades
			const bool Jammered = !pExt->RegisteredJammers.empty();  // is this building jammed

			for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin)
			{

				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(*begin);
				//const auto Powered_ = pBld->IsOverpowered || (!PowerDown && !((*begin)->PowerDrain && LowpOwerHouse));

				const bool IsFactoryPowered = !pTypeExt->FactoryPlant_RequirePower || ((*begin)->Powered && Online);

				//recalculate the multiplier
				if ((*begin)->FactoryPlant && IsFactoryPowered)
				{
					pThis->CostDefensesMult *= (*begin)->DefensesCostBonus;
					pThis->CostUnitsMult *= (*begin)->UnitsCostBonus;
					pThis->CostInfantryMult *= (*begin)->InfantryCostBonus;
					pThis->CostBuildingsMult *= (*begin)->BuildingsCostBonus;
					pThis->CostAircraftMult *= (*begin)->AircraftCostBonus;
				}

				//only pick first spysat
				const bool IsSpySatPowered = !pTypeExt->SpySat_RequirePower || ((*begin)->Powered && Online);
				if (!Spysat && (*begin)->SpySat && !Jammered && IsSpySatPowered)
				{
					const bool IsDiscovered = pBld->DiscoveredByCurrentPlayer && SessionClass::Instance->GameMode == GameMode::Campaign;
					if (IsLimboDelivered || !IsCurrentPlayer || SessionClass::Instance->GameMode != GameMode::Campaign || IsDiscovered)
					{
						Spysat = pBld;
					}
				}

				// add eligible building
				if (pTypeExt->SpeedBonus.Enabled && Online)
					++pHouseExt->Building_BuildSpeedBonusCounter[(*begin)];

				const bool IsPurifierRequirePower = !pTypeExt->PurifierBonus_RequirePower || ((*begin)->Powered && Online);
				// add eligible purifier
				if ((*begin)->OrePurifier && IsPurifierRequirePower)
					++pHouseExt->Building_OrePurifiersCounter[(*begin)];
			}
		}
	}

	int Purifiers = 0;

	//count them
	for (auto& purifier : pHouseExt->Building_OrePurifiersCounter)
		Purifiers += purifier.second;

	pThis->NumOrePurifiers = Purifiers;

	return Spysat;
}

DEFINE_HOOK(0x508F79, HouseClass_AI_CheckSpySat, 0x5)
{
	enum
	{
		ActivateSpySat = 0x509054,
		DeactivateSpySat = 0x509002
	};

	GET(HouseClass*, pThis, ESI);
	return IsAnySpysatActive(pThis) ? ActivateSpySat : DeactivateSpySat;
}

DEFINE_HOOK(0x474964, CCINIClass_ReadPipScale_add, 0x6)
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

DEFINE_HOOK(0x709B79, TechnoClass_DrawPip_Spawner, 0x6)
{
	enum { SkipGameDrawing = 0x709C27 };

	GET(TechnoClass*, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->ShowSpawnsPips.Get(((int)pType->PipScale == 6)))
	{
		return SkipGameDrawing;
	}

	LEA_STACK(RectangleStruct*, offset, STACK_OFFSET(0x74, -0x24));
	GET_STACK(RectangleStruct*, rect, STACK_OFFSET(0x74, 0xC));
	//GET_STACK(SHPStruct*, shape, STACK_OFFSET(0x74, -0x58));
	GET_STACK(bool, isBuilding, STACK_OFFSET(0x74, -0x61));
	GET(int, maxSpawnsCount, EBX);

	ConvertClass* pPal = FileSystem::PALETTE_PAL();
	const auto pShape = isBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) :
		pTypeExt->PipShapes02.Get(FileSystem::PIPS_SHP());

	//if (isBuilding)
	//{
	//	const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find((BuildingTypeClass*)pType);
	//
	//	if (pBuildingTypeExt->PipShapes01Remap)
	//		pPal = pThis->GetRemapColour();
	//	else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
	//		pPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	//
	//}

	int currentSpawnsCount = pThis->SpawnManager->CountDockedSpawns();
	Point2D position { offset->X + pTypeExt->SpawnsPipOffset->X, offset->Y + pTypeExt->SpawnsPipOffset->Y };
	const Point2D size = pTypeExt->SpawnsPipSize.Get(
		isBuilding ?
		RulesExtData::Instance()->Pips_Generic_Buildings_Size :
		RulesExtData::Instance()->Pips_Generic_Size
	);

	for (int i = 0; i < maxSpawnsCount; i++)
	{
		int frame = i < currentSpawnsCount ? pTypeExt->SpawnsPip : pTypeExt->EmptySpawnsPip;

		DSurface::Temp->DrawSHP(pPal, pShape, frame,
			&position, rect, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		position.X += size.X;
		position.Y += size.Y;
	}

	return SkipGameDrawing;
}

DEFINE_HOOK(0x481180, CellClass_GetInfantrySubPos_InvalidCellPointer, 0x5)
{
	enum { retEmpty = 0x4812EC, retContinue = 0x0, retResult = 0x481313 };
	GET(CellClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pResult, 0x4);
	GET_STACK(DWORD, caller, 0x0);

	if (!pThis) {
		Debug::FatalErrorAndExit("CellClass::GetInfantrySubPos please fix ! caller [0x%x] \n" , caller);
	}

	return retContinue;
}

DEFINE_HOOK(0x520E75, InfantryClass_SequenceAI_Sounds, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const int doType = (int)pThis->SequenceAnim;

	// out of bound read fix
	if (doType == -1)
		return 0x520EF4;

	//const auto pSeq = &pThis->Type->Sequence->Data[doType];
	//
	//for (int i = 0; i < pSeq->SoundCount; ++i) {
	//	for (auto at = pSeq->SoundData; at != (&pSeq->SoundData[2]); ++at) {
	//		if (pThis->Animation.HasChanged && at->Index != -1) {
	//			const int count = pSeq->CountFrames < 1 ? 1 : pSeq->CountFrames;
	//			if (pThis->Animation.Value % count == at->StartFrame) {
	//				VoxClass::PlayAtPos(at->Index, &pThis->Location);
	//			}
	//		}
	//	}
	//}
	//
	//return 0x520EF4;
	return 0x0;
}

DEFINE_HOOK(0x5194EF, InfantryClass_DrawIt_InAir_NoShadow, 5)
{
	GET(InfantryClass*, pThis, EBP);
	return pThis->Type->NoShadow ? 0x51958A : 0x0;
}

DEFINE_HOOK(0x746AFF, UnitClass_Disguise_Update_MoveToClear, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->Disguise && pThis->Disguise->WhatAmI() == UnitClass::AbsID ? 0x746A9C : 0;
}

DEFINE_HOOK(0x4249EC, AnimClass_CreateMakeInf_WeirdAssCode, 0x6)
{
	GET(AnimClass*, pThis, ESI);

	if (auto const pInf = RulesClass::Instance->AnimToInfantry.GetItemOrDefault(pThis->Type->MakeInfantry))
	{
		if (auto const pCreatedInf = pInf->CreateObject(pThis->Owner))
		{
			R->EAX(pCreatedInf);
			return 0x424A1F;
		}
	}

	return 0x424B0A;
}

// do some pre-validation evenbefore function going to be executed
// save some cpu cycle
DEFINE_HOOK(0x486920, CellClass_TriggerVein_Precheck, 0x6)
{
	return RulesClass::Instance->VeinAttack ? 0x0 : 0x486A6B;
}

DEFINE_HOOK(0x4869AB, CellClass_TriggerVein_Weight, 0x6)
{
	GET(TechnoTypeClass*, pTechnoType, EAX);
	GET(TechnoClass*, pTechno, ESI);

	if (pTechno->WhatAmI() == BuildingClass::AbsID || !RulesExtData::Instance()->VeinsDamagingWeightTreshold.isset())
		return 0x0;

	if (pTechnoType->Weight < RulesExtData::Instance()->VeinsDamagingWeightTreshold)
	{
		return 0x486A55; //skip damaging
	}

	return 0x0;
}

#ifdef debug_veinstest
DEFINE_JUMP(LJMP, 0x4869AB, 0x4869CA);
#endif

//// 7384C3 ,7385BB UnitClass take damage func
DEFINE_HOOK(0x73D4DA, UnitClass_Harvest_VeinsStorageAmount, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(CellClass*, pCell, EBP);

	auto storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	double amount = 1.0;

	if (pThis->Type->Weeder)
	{

		pCell->RemoveWeed();
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.IncreaseAmount(RulesExtData::Instance()->Veins_PerCellAmount, 0);
		return 0x73D502;
	}

	int tibType = pCell->GetContainedTiberiumIndex();
	double cur = storage->GetAmount(tibType);

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

DEFINE_HOOK(0x73E9A0, UnitClass_Mi_Harvest_IncludeWeeder_1, 6)
{
	GET(UnitTypeClass*, pType, EDX);
	R->AL(pType->Harvester || pType->Weeder);
	return 0x73E9A6;
}

DEFINE_HOOK(0x730E39, GuardCommandClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || pType->Weeder);
	return 0x730E3F;
}

DEFINE_HOOK(0x736823, UnitClass_AI_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	R->CL(pType->Harvester || pType->Weeder);
	return 0x736829;
}

DEFINE_HOOK(0x7368C6, UnitClass_Update_WeederMissionMove2, 0x6)
{
	GET(BuildingTypeClass*, pBuildingType, EDX);
	R->CL(pBuildingType->Refinery || pBuildingType->Weeder);
	return 0x7368CC;
}

DEFINE_HOOK(0x7043E7, TechnoClass_Get_ZAdjustment_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pUnitType, ECX);

	R->AL(pUnitType->Harvester || pUnitType->Weeder);
	return 0x7043ED;
}

//real adjusment place , maybe make this customizable ?
DEFINE_HOOK(0x70440C, TechnoClass_Get_ZAdjustment_IncludeWeederBuilding, 0x6)
{
	GET(BuildingTypeClass*, pBuildingType, EAX);
	R->CL(pBuildingType->Refinery || pBuildingType->Weeder);
	return 0x704412;
}

DEFINE_HOOK(0x74097E, UnitClass_MI_Guard_IncludeWeeder, 0x6)
{
	GET(BuildingTypeClass*, pBuilding, ECX);
	R->DL(pBuilding->Refinery || pBuilding->Weeder);
	return 0x740984;
}

DEFINE_HOOK(0x73D0DB, UnitClass_DrawAt_Oregath_IncludeWeeder, 0x6)
{
	enum { Draw = 0x73D0E9, Skip = 0x73D298 };

	GET(UnitClass*, pUnit, ESI);

	return ((pUnit->Type->Harvester || pUnit->Type->Weeder) && !pUnit->IsHarvesting) ?
		Skip : Draw;
}

DEFINE_HOOK(0x73D2A6, UnitClass_DrawAt_UnloadingClass_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pUnitType, EAX);

	R->CL(pUnitType->Harvester || pUnitType->Weeder);
	return 0x73D2AC;
}

//this one dextroy special anim : 741C32
DEFINE_HOOK(0x73E005, UnitClass_Mi_Unload_PlayBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->AL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x73E00B;
}

DEFINE_HOOK(0x741C32, UnitClass_AssignDestination_DestroyBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->DL(pType->Harvester || (pType->Weeder && TechnoTypeExtContainer::Instance.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x741C38;
}

//allow `VeinholeMonster` to be placed anywhere flat
DEFINE_JUMP(LJMP, 0x74C688, 0x74C697);

DEFINE_HOOK(0x489671, MapClass_DamageArea_Veinhole, 0x6)
{
	GET(CellClass*, pCell, EBX);
	GET(OverlayTypeClass*, pOverlay, EAX);

	if (pOverlay->IsVeinholeMonster)
	{
		GET_STACK(int, nDamage, 0x24);
		GET(WarheadTypeClass*, pWarhead, ESI);
		GET_BASE(TechnoClass*, pSource, 0x8);
		GET_BASE(HouseClass*, pHouse, 0x14);
		GET(CoordStruct*, pCenter, EDI);

		if (VeinholeMonsterClass* pMonster = VeinholeMonsterClass::GetVeinholeMonsterFrom(&pCell->MapCoords))
		{
			if (!pMonster->InLimbo && pMonster->IsAlive && ((int)pMonster->MonsterCell.DistanceFrom(pCell->MapCoords) <= 0))
				if (pMonster->ReceiveDamage(&nDamage,
					(int)pCenter->DistanceFrom(CellClass::Cell2Coord(pMonster->MonsterCell)),
					pWarhead,
					pSource,
					false,
					false,
					pSource && !pHouse ? pSource->Owner : pHouse
				) == DamageState::NowDead)
					Debug::Log("Veinhole at [%d %d] Destroyed!\n", pMonster->MonsterCell.X , pMonster->MonsterCell.Y);

		}

		return 0x4896B2;
	}

	return 0x0;
}

DEFINE_HOOK(0x6FA4E5, TechnoClass_AI_RecoilUpdate, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return !pThis->InLimbo ? 0x0 : 0x6FA4FB;
}

DEFINE_HOOK(0x6F6BD6, TechnoClass_Limbo_UpdateAfterHouseCounter, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	//only update the SW once the techno is really not present
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty() && pThis->Owner->CountOwnedAndPresent(pTypeExt->AttachedToObject) <= 0)
		pThis->Owner->UpdateSuperWeaponsOwned();

	return 0x0;
}

DEFINE_HOOK(0x6E08DE, TActionClass_SellBack_LimboDelivered, 0x6)
{
	enum { forbidden = 0x6E0907, allow = 0x0 };

	GET(BuildingClass*, pBld, ESI);
	return BuildingExtContainer::Instance.Find(pBld)->LimboID != -1 ?
		forbidden : allow;
}

//DEFINE_HOOK(0x6E9832, TeamClass_AI_IsThisExecuted, 0x8)
//{
//	Debug::Log(__FUNCTION__" Called \n");
//
//	return 0x0;
//}

DEFINE_HOOK(0x6E9690, TeamClass_ChangeHouse_nullptrresult, 0x6)
{
	GET(TeamClass*, pThis, ESI);
	GET(int, args, ECX);
	GET(FootClass*, pCurMember, EDI);

	const auto pHouse = HouseClass::FindByCountryIndex(args);
	if (!pHouse)
	{
		const auto nonestr = GameStrings::NoneStr();
		Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]\n",
			pThis->Owner ? pThis->Owner->get_ID() : nonestr, pThis->Owner,
			pThis->get_ID(), pThis, args);
	}

	pCurMember->SetOwningHouse(pHouse);
	R->EBP(pCurMember->NextTeamMember);
	return 0x6E96A8;
}

DEFINE_HOOK(0x4686FA, BulletClass_Unlimbo_MissingTargetPointer, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_BASE(CoordStruct*, pUnlimboCoords, 0x8);

	if (!pThis->Target)
	{
		Debug::Log("Bullet [%s - %x] Missing Target Pointer when Unlimbo! , Fallback To CreationCoord to Prevent Crash\n",
			pThis->get_ID(), pThis);

		pThis->Target = MapClass::Instance->GetCellAt(pUnlimboCoords);
		R->EAX(pUnlimboCoords);
		return 0x46870A;
	}

	return 0x0;
}

DEFINE_HOOK(0x65DD4E, TeamClass_CreateGroub_MissingOwner, 0x7)
{
	GET(TeamClass*, pCreated, ESI);
	GET(TeamTypeClass*, pType, EBX);

	const auto pHouse = pType->GetHouse();
	if (!pHouse)
	{
		Debug::Log("Creating Team[%s] groub without proper Ownership may cause crash , Please check !\n", pType->ID);
	}

	R->EAX(pHouse);
	return 0x65DD55;
}

DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Destination)
	{
		if (pThis->Type->IsDropship)
		{
			CellStruct nCell = CellStruct::Empty;
			if (pThis->Destination->WhatAmI() != CellClass::AbsID)
			{
				if (auto pTech = generic_cast<TechnoClass*>(pThis))
				{
					nCell = CellClass::Coord2Cell(pTech->GetCoords());
					if (nCell.IsValid())
					{
						if (auto pCell = MapClass::Instance->GetCellAt(nCell))
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

//DEFINE_HOOK(0x456376 , BuildingClass_RemoveSpacingAroundArea, 0x6)
//{
//	GET(BuildingTypeClass*, pThisType, EAX);
   //GET(BuildingClass*, pThis, ESI);
//
	//if (!pThisType->UndeploysInto || (!pThisType->ResourceGatherer && !pThis->IsStrange()))
 //		return 0x456398;
//
 //	return pThisType->Adjacent == 0 ? 0x4563A1 : 0x45638A;
 //}

DEFINE_HOOK(0x518607, InfantryClass_TakeDamage_FixOnDestroyedSource, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, 0xD0 + 0x10);
	R->AL(pThis->Crash(pSource));
	return 0x518611;
}

DEFINE_HOOK(0x450B48, BuildingClass_Anim_AI_UnitAbsorb, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->InfantryAbsorb || pThis->UnitAbsorb);
	return 0x450B4E;
}

DEFINE_HOOK(0x73B0C5, UnitClass_Render_nullptrradio, 0x6)
{
	GET(TechnoClass*, pContact, EAX);
	return !pContact ? 0x73B124 : 0x0;
}

// DEFINE_HOOK(0x6F91EC, TechnoClass_GreatestThreat_DeadTechnoInsideTracker, 0x6)
// {
// 	GET(TechnoClass*, pTrackerTechno, EBP);
//
// 	if (!pTrackerTechno->IsAlive)
// 	{
// 		Debug::Log("Found DeadTechno[%x - %s] on AircraftTracker!\n", pTrackerTechno, pTrackerTechno->get_ID());
// 		return 0x6F9377; // next
// 	}
//
// 	return 0x0;//contunye
// }

/**
 *  Draw a radial to the screen.
 *
 *  @authors: CCHyper
 */

static void Tactical_Draw_Radial(
	bool draw_indicator,
	bool animate,
	Coordinate center_coord,
	ColorStruct color,
	float radius,
	bool concentric,
	bool round)
{
	if (round)
	{
		radius = std::round(radius);
	}

	int size;

	if (concentric)
	{
		size = (int)radius;
	}
	else
	{
		size = (int)((radius + 0.5) / Math::sqrt(2.0) * double(Unsorted::CellWidthInPixels)); // should be cell size global?
	}

	Point2D center_pixel {};
	TacticalClass::Instance->CoordsToClient(&center_coord, &center_pixel);

	center_pixel.X += DSurface::ViewBounds().X;
	center_pixel.Y += DSurface::ViewBounds().Y;

	RectangleStruct draw_area(
		center_pixel.Y - size / 2,
		center_pixel.X - size,
		size * 2,
		size
	);

	RectangleStruct intersect = draw_area.IntersectWith(DSurface::ViewBounds());
	if (!intersect.IsValid())
	{
		return;
	}

	ColorStruct draw_color = color;

	if (animate)
	{
		draw_color.Adjust(50, ColorStruct(0, 0, 0));
	}

	unsigned ellipse_color = DSurface::RGB_To_Pixel(draw_color.R, draw_color.G, draw_color.B);

	/**
	 *  Draw the main radial ellipse, then draw one slightly smaller to give a thicker impression.
	 */
	DSurface::Temp->Draw_Ellipse(center_pixel, size, size / 2, DSurface::ViewBounds(), ellipse_color);
	DSurface::Temp->Draw_Ellipse(center_pixel, size - 1, size / 2 - 1, DSurface::ViewBounds(), ellipse_color);

	/**
	 *  Draw the sweeping indicator line.
	 */
	if (!draw_indicator)
	{
		return;
	}

	double d_size = (double)size;
	double size_half = (double)size / 2;

	/**
	   *  The alpha values for the lines (producing the fall-off effect).
	   */
	static const double _line_alpha[] = {
		//0.05, 0.20, 0.40, 1.0                     // original values.
		0.05, 0.10, 0.20, 0.40, 0.60, 0.80, 1.0     // new values.
	};

	static const int _rate = 50;

	for (size_t i = 0; i < ARRAY_SIZE(_line_alpha); ++i)
	{

		static int _offset = 0;
		static MSTimerClass sweep_rate(_rate);

		if (sweep_rate.Expired())
		{
			sweep_rate.Start(_rate);
			++_offset;
		}

		float angle_offset = float((_offset + i) * 0.05);
		int angle_increment = int(angle_offset / DEG_TO_RADF(360));
		float angle = angle_offset - (angle_increment * DEG_TO_RADF(360));

		Point2D line_start {};
		Point2D line_end {};

		if (std::fabs(angle - DEG_TO_RADF(90)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + (-size_half)));

		}
		else if (std::fabs(angle - DEG_TO_RADF(270)) < 0.001)
		{

			line_start = center_pixel;
			line_end = Point2D(center_pixel.X, int(center_pixel.Y + size_half));

		}
		else
		{

			double angle_tan = Math::tan(angle);
			double xdist = Math::sqrt(1.0 / ((angle_tan * angle_tan) / (size_half * size_half) + 1.0 / (d_size * d_size)));
			double ydist = Math::sqrt((1.0 - (xdist * xdist) / (d_size * d_size)) * (size_half * size_half));

			if (angle > DEG_TO_RADF(90) && angle < DEG_TO_RADF(270))
			{
				xdist = -xdist;
			}

			if (angle < DEG_TO_RADF(180))
			{
				ydist = -ydist;
			}

			line_start = center_pixel;
			line_end = Point2D(int(center_pixel.X + xdist), int(center_pixel.Y + ydist));

		}

		line_start.X -= DSurface::ViewBounds().X;
		line_start.Y -= DSurface::ViewBounds().Y;

		line_end.X -= DSurface::ViewBounds().X;
		line_end.Y -= DSurface::ViewBounds().Y;

		bool enable_red_channel = false;
		bool enable_green_channel = true;
		bool enable_blue_channel = false;

		DSurface::Temp->DrawSubtractiveLine_AZ(DSurface::ViewBounds(),
										line_start,
										line_end,
										draw_color,
										-500,
										-500,
										false,
										enable_red_channel,
										enable_green_channel,
										enable_blue_channel,
										(float)_line_alpha[i]);

	}
}

class ObjectClassExt : public ObjectClass
{
public:
	static void __fastcall _DrawFootRadialIndicator(ObjectClass* pThis, DWORD, int val)
	{
		if (auto pTechno = generic_cast<TechnoClass*>(pThis))
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
						ColorStruct* Color = pTypeExt->RadialIndicatorColor.isset() ? pTypeExt->RadialIndicatorColor.GetEx() : &pTechno->Owner->Color;

						if (*Color != ColorStruct::Empty)
						{
							auto nCoord = pTechno->GetCoords();
							Tactical_Draw_Radial(false, true, nCoord, *Color, (nRadius * 1.0f), false, true);
						}
					}
				}
			}
		}
	}
};

DEFINE_JUMP(VTABLE, 0x7F5DA0, GET_OFFSET(ObjectClassExt::_DrawFootRadialIndicator))
DEFINE_JUMP(VTABLE, 0x7EB188, GET_OFFSET(ObjectClassExt::_DrawFootRadialIndicator))

//Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
//Fixes recursive passenger kills not being accredited
//to proper techno but to their transports
//DEFINE_HOOK(0x707CF2, TechnoClass_KillCargo_FixKiller, 0x8)
//{
//	GET(TechnoClass*, pKiller, EBP);
//	GET(TechnoClass*, pCargo, ESI);
//
//	pCargo->KillCargo(pKiller);
//	return 0x707CFA;
//}

DEFINE_PATCH(0x707CF2, 0x55);

DEFINE_HOOK(0x6D47A6, TacticalClass_Render_Techno, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	// if(auto pTargetTech = generic_cast<ObjectClass*>(pThis))
	// 		Drawing::DrawLinesTo(pTargetTech->GetRenderCoords(), pThis->Location, pThis->Owner->LaserColor);

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
		if (auto const pFoot = generic_cast<FootClass*>(pThis))
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

DEFINE_HOOK(0x6F5190, TechnoClass_DrawIt_Add, 0x6)
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
			Point2D nRet;
			Simple_Text_Print_Wide(&nRet, pFormat, DSurface::Temp.get(), pBound, &nPoint, (COLORREF)nColorInt, (COLORREF)0, TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Efnt, true);
		};

	if (ShowTeamLeaderCommandClass::IsActivated())
	{
		if (auto const pFoot = generic_cast<FootClass*>(pThis))
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

DEFINE_HOOK(0x40A5B3, AudioDriverStart_AnnoyingBufferLogDisable_A, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	pAudioChannelTag->dwBufferBytes = R->EAX<int>();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::Log("Sound frame size = %d bytes\n", pAudioChannelTag->dwBufferBytes);

	return 0x40A5C4;
}

DEFINE_HOOK(0x40A554, AudioDriverStart_AnnoyingBufferLogDisable_B, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	LEA_STACK(DWORD*, ptr, STACK_OFFS(0x40, 0x28));
	pAudioChannelTag->soundframesize1 = R->EAX();

	if (Phobos::Otamaa::OutputAudioLogs)
		Debug::Log("Sound frame size = %d bytes\n", pAudioChannelTag->soundframesize1);

	R->EDX(R->EAX());
	R->EAX(ptr);
	return 0x40A56C;
}

DEFINE_HOOK(0x442991, BuildingClass_ReceiveDamage_ReturnFire_EMPulseCannon, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->EMPulseCannon || pThis->Type->NukeSilo ? 0x442A95 : 0x0;
}

DEFINE_HOOK(0x442A08, BuildingClass_ReceiveDamage_ReturnFire, 0x5)
{
	enum { SetTarget = 0x442A34, RandomFacing = 0x442A41 };

	GET(TechnoClass*, pAttacker, EBP);
	GET(BuildingClass*, pThis, ESI);

	//Was pThis->Owner->ControlledByCurrentPlayer(), got desync ed with that
	const bool def = BuildingTypeExtContainer::Instance.Find(pThis->Type)->PlayerReturnFire.Get(
		pAttacker->WhatAmI() == AircraftClass::AbsID ||
		(pThis->Owner->IsControlledByHuman() && !RulesClass::Instance->PlayerReturnFire)
	);

	return !def ? SetTarget : RandomFacing;
}

DEFINE_HOOK(0x6DBE35, TacticalClass_DrawLinesOrCircles, 0x9)
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
			}
		}
	}

	return 0x6DBE74;
}

DEFINE_JUMP(LJMP, 0x50BF60, 0x50C04A)// Disable CalcCost mult

//bool ColorInitEd = false;
//ColorScheme* MainColor = nullptr;
//ColorScheme* BackColor = nullptr;
//
//void InitColorDraw()
//{
//	if (!ColorInitEd)
//	{
//		MainColor = ColorScheme::Find("Gold");
//		BackColor = ColorScheme::Find("NeonGreen");
//		ColorInitEd = true;
//	}
//}
//
//bool Draw_Debug_Test()
//{
//	const TextPrintType style = TextPrintType::FullShadow | TextPrintType::Point6Grad;
//	RectangleStruct rect = DSurface::ViewBounds();
//
//	// top left of tactical display.
//	Point2D screen = rect.Top_Left();
//
//	const auto buffer = std::format(L"RulesClass ptr {}", (uintptr_t)RulesClass::Instance());
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//
//	return true;
//}
//
//void Debug_Draw_Facings()
//{
//	const auto& objArr = ObjectClass::CurrentObjects;
//
//	if (objArr->Count != 1) {
//		return;
//	}
//
//	const auto pTechno = generic_cast<TechnoClass*>(objArr->Items[0]);
//	if (!pTechno)
//		return;
//
//	RectangleStruct rect = DSurface::ViewBounds();
//	const auto pType = pTechno->GetTechnoType();
//
//	CoordStruct lept {};
//	pType->Dimension2(&lept);
//	Point3D lept_center = Point3D(lept.X / 2, lept.Y / 2, lept.Z / 2);
//
//	Point3D pix {};
//	pType->PixelDimensions(&pix);
//	Point3D pixel_center = Point3D(pix.X / 2, pix.Y / 2, pix.Z / 2);
//
//	Coordinate coord = pTechno->GetCoords();
//
//	Point2D screen {};
//	//func_60F150 tspp
//	TacticalClass::Instance->CoordsToClient(coord, &screen);
//
//	screen.X -= TacticalClass::Instance->TacticalPos.X;
//	screen.Y -= TacticalClass::Instance->TacticalPos.Y;
//
//	screen.X += rect.X;
//	screen.Y += rect.Y;
//
//	DSurface::Temp->Fill_Rect(rect, RectangleStruct(screen.X, screen.Y, 2, 2), DSurface::RGB_To_Pixel(255, 0, 0));
//
//	TextPrintType style = TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad;
//	const auto font = BitFont::BitFontPtr(style);
//
//	screen.Y -= font->GetHeight() / 2;
//
//	const auto buffer1 = std::format(L"{}" , (int)pTechno->PrimaryFacing.Current().GetDir());
//	const auto buffer2 = std::format(L"{}", (int)pTechno->PrimaryFacing.Current().Raw);
//
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer1.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//
//	screen.Y += 10;
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer2.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//}

DEFINE_HOOK(0x6D4656, TacticalClass_Render_Veinhole, 0x5)
{
	VeinholeMonsterClass::DrawAll();
	IonBlastClass::DrawAll();
	return 0x6D465B;
}

DEFINE_HOOK(0x6D4669, TacticalClass_Render_Addition, 0x5)
{
	LaserDrawClass::DrawAll();

	//InitColorDraw();

	EBolt::DrawAll();
	ElectricBoltManager::Draw_All();
	return 0x6D4673;
}

DEFINE_HOOK(0x55B4E1, LogicClass_Update_Veinhole, 0x5)
{
	UpdateAllVeinholes();
	return 0;
}

DEFINE_HOOK(0x711F60, TechnoTypeClass_GetSoylent_Disable, 0x8)
{
	GET(TechnoTypeClass*, pThis, ECX);

	if (TechnoTypeExtContainer::Instance.Find(pThis)->Soylent_Zero)
	{
		R->EAX(0);
		return 0x712036;
	}

	return 0x0;
}

DEFINE_HOOK(0x4DB1A0, FootClass_GetMovementSpeed_SpeedMult, 0x6)
{
	GET(FootClass*, pThis, ECX);

	const auto pType = pThis->GetTechnoType();
	const auto pOwner = pThis->Owner;
	const auto houseSpeed = pOwner->GetSpeedMult(pType);
	const auto maxSpeed = pThis->GetDefaultSpeed();
	auto thisMult = pThis->SpeedMultiplier;

	//prevent unit in warfactory stuck
	// if (thisMult < 0.0001 && TechnoExtData::IsInWarfactory(pThis, false)) {
	// 	Debug::Log("Foot[%s] with negative or zero speed mult inside warfactory ,restoring speedmult\n", pType->ID);
	// 	thisMult = 1.0;
	// }

	double result = maxSpeed * houseSpeed * thisMult;

	if (pThis->HasAbility(AbilityType::Faster))
	{
		result *= RulesClass::Instance->VeteranSpeed;
	}

	int speedResult = (int)(result * pThis->SpeedPercentage);

	if (pThis->WhatAmI() == UnitClass::AbsID && ((UnitClass*)pThis)->FlagHouseIndex != -1)
	{
		speedResult /= 2;
	}

	R->EAX((int)speedResult);
	return 0x4DB23D;
}

DEFINE_HOOK(0x71F1A2, TEventClass_HasOccured_DestroyedAll, 6)
{
	GET(HouseClass*, pHouse, ESI);

	if (pHouse->ActiveInfantryTypes.GetTotal() <= 0)
	{
		for (auto& bld : pHouse->Buildings)
		{
			if (bld->Type->CanBeOccupied && bld->Occupants.Count > 0)
				return 0x71F163;
		}
	}

	return 0x71F1B1;
}

DEFINE_HOOK(0x6DEA37, TAction_Execute_Win, 6)
{
	GET(TActionClass*, pThis, ESI);

	if (HouseClass::Index_IsMP(pThis->Value))
	{
		const auto pHouse_ = pThis->Value == 8997 ?
			HouseClass::CurrentPlayer() : HouseClass::FindByIndex(pThis->Value);

		auto pHouseBegin = HouseClass::Array->begin();
		auto pHouseEnd = HouseClass::Array->end();

		if (HouseClass::Array->begin() != pHouseEnd)
		{
			do
			{
				auto v7 = *pHouseBegin;
				if (pHouse_->ArrayIndex == (*pHouseBegin)->ArrayIndex
					|| pHouse_->ArrayIndex != -1 && ((1 << pHouse_->ArrayIndex) & v7->Allies.data) != 0)
					v7->Win(false);

				++pHouseBegin;
			}
			while (pHouseBegin != pHouseEnd);
		}

		return  0x6DEA58;
	}
	else
	{
		if (pThis->Value == HouseClass::CurrentPlayer()->Type->ArrayIndex2)
			HouseClass::CurrentPlayer()->Win(false);
		else
			HouseClass::CurrentPlayer()->Lose(false);

		return 0x6DEA58;
	}
}

// DEFINE_HOOK(0x4D54DD, FootClass_Mi_Hunt_NoPath, 6)
// {
// 	GET(FootClass*, pThis, ESI);
//
// 	const auto pOwner = pThis->Owner;
// 	if (!pOwner->IsControlledByHuman()
// 		&& !(Unsorted::CurrentFrame() % 450)
// 		&& pThis->CurrentMapCoords == pThis->LastMapCoords
// 		&& !pThis->GetTechnoType()->ResourceGatherer
// 		)
// 	{
// 		pThis->SetDestination(nullptr, true);
// 		pThis->SetTarget(nullptr);
// 		pThis->TargetAndEstimateDamage(&pThis->Location, ThreatType::Range);
// 	}
//
// 	return 0x0;
// }

//DEFINE_HOOK(0x4CD747, FlyLocomotionClass_UpdateMoving_OutOfMap, 6)
//{
//	GET(DisplayClass*, pDisplay, ECX);
//	GET(FlyLocomotionClass*, pLoco, ESI);
//	GET_STACK(int, height, 0x58);
//	GET(CellStruct*, pCell, EAX);
//
//	if(pDisplay->CoordinatesLegal(pCell))
//		return 0x4CD751;
//
//	pDisplay->RemoveObject(pLoco->Owner);
//	pLoco->Owner->SetHeight(height);
//	pDisplay->SubmitObject(pLoco->Owner);
//
//	return 0x4CD797;
//}

DEFINE_HOOK(0x73730E, UnitClass_Visceroid_HealthCheckRestore, 0x6)
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

DEFINE_HOOK(0x73666A, UnitClass_AI_Viscerid_ZeroStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);
	return pType->Strength <= 0 || pThis->DeathFrameCounter > 0 ? 0x736685 : 0x0;
}

//this thing checking shit backwards ,..
DEFINE_HOOK(0x6D4764, TechnoClass_PsyhicSensor_DisableWhenTechnoDies, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis)
	{
		const auto vtable = VTable::Get(pThis);
		if (vtable != UnitClass::vtable
			&& vtable != InfantryClass::vtable
			&& vtable != AircraftClass::vtable) {
			return 0x6D4793;
		}

		if (!pThis->IsAlive
		|| pThis->InLimbo
		|| pThis->IsCrashing
		|| pThis->IsSinking
		|| (pThis->WhatAmI() == UnitClass::AbsID && ((UnitClass*)pThis)->DeathFrameCounter > 0)) {
			return 0x6D4793;
		}

		auto pExt = TechnoExtContainer::Instance.Find(pThis);

		if (pExt->AE_Untrackable || TechnoExtData::IsUntrackable(pThis)) {
			return 0x6D4793;
		}

		if (pThis->CurrentlyOnSensor()) {
			return 0x6D478C; //draw dashed line
		}
	}

	return 0x6D4793;
}

// Gives player houses names based on their spawning spot

int GetPlayerPosByName(const char* pName)
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

DEFINE_HOOK_AGAIN(0x74330D, TechnoClass_FromINI_CreateForHouse, 0x7)
DEFINE_HOOK_AGAIN(0x41B17B, TechnoClass_FromINI_CreateForHouse, 0x7)
DEFINE_HOOK_AGAIN(0x51FB6B, TechnoClass_FromINI_CreateForHouse, 0x7)
DEFINE_HOOK(0x44F8A6, TechnoClass_FromINI_CreateForHouse, 0x7)
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
	//	Debug::Log("%s , With House Index [%d]\n", pHouseName, idx);

	if (idx == -1)
	{
		Debug::Log("Failed To fetch house index by name of [%s]\n", pHouseName);
		Debug::RegisterParserError();
	}

	R->EAX(idx);
	return R->Origin() + 0x7;
}

// Skips checking the gamemode or who the player is when assigning houses
DEFINE_JUMP(LJMP, 0x44F8CB, 0x44F8E1)

DEFINE_HOOK(0x73745C, UnitClass_ReceiveRadio_Parasited_WantRide, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { negativemessage = 0x73746A, continueChecks = 0x737476 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return negativemessage;

	return continueChecks;
}

DEFINE_HOOK(0x7375B6, UnitClass_ReceiveRadio_Parasited_CanLoad, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	enum { staticmessage = 0x7375C4, continueChecks = 0x7375D0 };

	if (pThis->IsBeingWarpedOut()
		|| (pThis->ParasiteEatingMe && pThis->ParasiteEatingMe->ParasiteImUsing->GrappleAnim))
		return staticmessage;

	return continueChecks;
}

// DEFINE_HOOK(0x6E23AD, TActionClass_DoExplosionAt_InvalidCell, 0x8)
// {
// 	GET(CellStruct*, pLoc, EAX);
//
// 	//prevent crash
// 	return !pLoc->IsValid() ? 0x6E2510 : 0x0;
// }

DEFINE_STRONG_HOOK(0x4A267D, CreditClass_AI_MissingCurPlayerPtr, 0x6)
{
	if (!HouseClass::CurrentPlayer())
		Debug::FatalError("CurrentPlayer ptr is Missing!\n");

	return 0x0;
}

DEFINE_HOOK(0x5FF93F, SpotlightClass_Draw_OutOfboundSurfaceArrayFix, 0x7)
{
	GET(SpotlightClass*, pThis, EBP);
	GET(int, idx, ECX);

	if (idx > 64)
	{
		Debug::Log("[0x%x]SpotlightClass with OutOfBoundSurfaceArrayIndex[%d] Fixing!\n", pThis, idx);
		idx = 64;
	}

	return 0x0;
}

// DEFINE_HOOK(0x73B0B0, UnitClass_DrawIfVisible, 0xA)
// {
// 	GET(UnitClass*, pThis, ECX);
// 	GET_STACK(RectangleStruct*, pBounds, 0x4);
// 	GET_STACK(bool, ignorecloaked, 0x8);
//
// 	bool result = !pThis->IsTethered;
// 	if (TechnoClass* pContact = pThis->GetNthLink())
// 	{
// 		result |= pContact->WhatAmI() != AbstractType::Building;
// 		result |= pContact->GetCurrentMission() != Mission::Unload && pContact->QueuedMission != Mission::Unload;
// 		result |= !pContact->UnloadTimer.IsOpening()
// 			&& !pContact->UnloadTimer.IsClosing()
// 			&& !pContact->UnloadTimer.IsOpen()
// 			&& !pContact->UnloadTimer.IsClosed();
// 	}
//
// 	result &= pThis->ObjectClass::DrawIfVisible(pBounds, ignorecloaked, 0);
//
// 	R->EAX(result);
//
// 	return 0x73B139;
// }

DEFINE_HOOK(0x6FFD25, TechnoClass_PlayerAssignMission_Capture_InfantryToBld, 0xA)
{
	GET_STACK(ObjectClass*, pTarget, 0x98 + 0x8);
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

#ifdef CHECK_PTR_VALID
DEFINE_STRONG_HOOK_AGAIN(0x4F9A10, HouseClass_IsAlliedWith, 0x6)
DEFINE_STRONG_HOOK_AGAIN(0x4F9A50, HouseClass_IsAlliedWith, 0x6)
DEFINE_STRONG_HOOK_AGAIN(0x4F9AF0, HouseClass_IsAlliedWith, 0x7)
DEFINE_STRONG_HOOK(0x4F9A90, HouseClass_IsAlliedWith, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(DWORD, called, 0x0);

	if (!pThis || VTable::Get(pThis) != HouseClass::vtable)
	{
		Debug::FatalError("HouseClass - IsAlliedWith[%x] , Called from[%x] with `nullptr` pointer !\n", R->Origin(), called);
	}

	return 0;
}
#endif

DEFINE_HOOK(0x737BFB, UnitClass_Unlimbo_SmallVisceroid_DontMergeImmedietely, 0x6)
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

DEFINE_HOOK(0x6FDD0A, TechnoClass_AdjustDamage_Armor, 0x6)
{
	GET(ObjectClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, 0x18 + 0x8);
	GET(int, damage, EBX);
	R->EAX(MapClass::ModifyDamage(damage, pWeapon->Warhead, TechnoExtData::GetTechnoArmor(pThis,pWeapon->Warhead), 0));
	return 0x6FDD2E;
}

DEFINE_HOOK(0x52D36F, RulesClass_init_AIMD, 0x5)
{
	GET(CCFileClass*, pFile, EAX);
	Debug::Log("Init %s file\n", pFile->GetFileName());
	return 0x0;
}

//DEFINE_HOOK(0x4824EF, CellClass_CollecCreate_FlyingStrings, 0x8)
//{
//	GET(CellClass*, pThis, ESI);
//	GET(int, amount, EDI);
//	GET_BASE(FootClass*, pPicker, 0x8);
//	CoordStruct loc = CellClass::Cell2Coord(pThis->MapCoords);
//	loc.Z = pThis->GetFloorHeight({ 128 , 128 });
//
//
//	R->Stack(0x84, loc);
//	R->EAX(RulesClass::Instance());
//	return 0x482551;
//}

DEFINE_HOOK(0x41F783, AITriggerTypeClass_ParseConditionType, 0x5)
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
		Debug::Log("Condition Object[%s - %s] for [%s]\n", pBuffer, result ? result->GetThisClassName() : NONE_STR2, pThis->ID);

	R->ESI(result);
	return 0x41F7DE;
}

DEFINE_HOOK(0x4CA682, FactoryClass_Total_Techno_Queued_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBX);

	return pObject->GetTechnoType() == pTypeCompare || TechnoExtContainer::Instance.Find(pObject)->Type == pTypeCompare ?
		0x4CA68E : 0x4CA693;
}

DEFINE_HOOK(0x4FA5B8, HouseClass_BeginProduction_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBP);

	return pObject->GetTechnoType() == pTypeCompare || TechnoExtContainer::Instance.Find(pObject)->Type == pTypeCompare ?
		0x4FA5C4 : 0x4FA5C8;
}

DEFINE_HOOK(0x4FAB4D, HouseClass_AbandonProduction_GetObjectType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	R->EAX(TechnoExtContainer::Instance.Find(pObject)->Type);
	return R->Origin() + 0x8;
}

DEFINE_HOOK(0x4CA007, FactoryClass_AbandonProduction_GetObjectType, 0x6)
{
	GET(FactoryClass*, pThis, ESI);
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	const auto pType = TechnoExtContainer::Instance.Find(pObject)->Type;
	Debug::Log("[%x]Factory with owner [%s - %x] abandoning production of [%s(%s) - %x]\n",
		pThis,
		pThis->Owner->get_ID(), pThis->Owner,
		pType->Name, pType->ID, pObject);

	R->EAX(pType);
	return 0x4CA029;
}

#include <Ext/Bomb/Body.h>

DEFINE_HOOK(0x5F9652, ObjectTypeClass_GetAplha, 0x6)
{
	GET(ObjectTypeClass*, pThis, EBX);
	R->CL(pThis->AlphaImageFile[0] && strlen(pThis->AlphaImageFile));
	return 0x5F9658;
}

//#ifndef REMOVE_SOURCE_REUqUIREMENT_FROM_FEAR_CHECK
//DEFINE_JUMP(LJMP, 0x518C45, 0x518C49);
//#endif
//DEFINE_HOOK(0x44A332, BuildingClass_MI_Deconstruct_ReasonToSpawnCrews, 0x7)
//{
//	GET(BuildingClass*, pThis, EBP);
//
//	if (pThis && IS_SAME_STR_(pThis->get_ID(), "DBEHIM")) {
//		Debug::FatalError("DBEHIM has Undeploys to but still endup here , WTF! HasNoFocuse %s \n", pThis->Focus ? "Yes" : "No");
//	}
//
//	return 0x0;
//}
//DEFINE_HOOK(0x449C30, BuildingClass_MI_Deconstruct_FatalIt, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//
//	if (pThis && IS_SAME_STR_(pThis->get_ID(), "DBEHIM"))
//		Debug::Log(__FUNCTION__"Caller [%x]\n", R->Stack<DWORD>(0x0));
//
//	return 0x0;
//}
// Enable This when needed
#ifdef DEBUG_STUPID_HUMAN_CHECKS

DEFINE_HOOK(0x50B730, HouseClass_IsControlledByHuman_LogCaller, 0x5)
{
	Debug::Log(__FUNCTION__"Caller [%x]\n", R->Stack<DWORD>(0x0));
	return 0x0;
}

DEFINE_HOOK(0x50B6F0, HouseClass_ControlledByCurrentPlayer_LogCaller, 0x5)
{
	Debug::Log(__FUNCTION__"Caller [%x]\n", R->Stack<DWORD>(0x0));
	return 0x0;
}
#endif

DEFINE_HOOK(0x6E20AC, TActionClass_DetroyAttachedTechno, 0x8)
{
	GET(TechnoClass*, pTarget, ESI);

	if (auto pBld = specific_cast<BuildingClass*>(pTarget))
	{
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
		{
			BuildingExtData::LimboKill(pBld);
			return 0x6E20D8;
		}
	}

	return 0x0;
}

//DEFINE_HOOK(0x448260, Debug_ChangeOwnership_Building, 0x8)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("%s ChangeOwnership For[%s] Caller[%x]\n", __FUNCTION__, pThis->get_ID() , caller);
//	return 0x0;
//}
//
//DEFINE_HOOK(0x4DBED0 , Debug_ChangeOwnership_Foot , 0x5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("%s ChangeOwnership For[%s] Caller[%x]\n", __FUNCTION__, pThis->get_ID(), caller);
//	return 0x0;
//}
//
//DEFINE_HOOK(0x7463A0, Debug_ChangeOwnership_Unit, 0x5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log("%s ChangeOwnership For[%s] Caller[%x]\n", __FUNCTION__, pThis->get_ID(), caller);
//	return 0x0;
//}

// 53AD00 ,5

// https://bugs.launchpad.net/ares/+bug/895893
DEFINE_HOOK(0x4DB37C, FootClass_Limbo_ClearCellJumpjet, 0x6)
{
	GET(FootClass*, pThis, EDI);

	if (pThis->GetTechnoType()->JumpJet)
	{
		const auto pCell = pThis->GetCell();
		if (pCell->Jumpjet == pThis)
		{
			pCell->TryAssignJumpjet(nullptr);
		}
	}

	//FootClass_Remove_Airspace_ares
	return 0x4DB3A4;
}

int __fastcall charToID(char*)
{
	JMP_STD(0x412610);
}

DEFINE_HOOK(0x6E5FA3, TagTypeClass_SwizzleTheID, 0x8)
{
	GET(char*, ID, EDI);
	GET(TagTypeClass*, pCreated, ESI);

	Debug::Log("TagType[%s] Allocated as [%p]!\n", ID, pCreated);

	return 0x6E5FB6;
}

DEFINE_HOOK(0x6E8300, TaskForceClass_SwizzleTheID, 0x5)
{
	LEA_STACK(char*, ID, 0x2C - 0x18);
	GET(TaskForceClass*, pCreated, ESI);

	Debug::Log("TaskForce[%s] Allocated as [%p]\n", ID, pCreated);

	return 0x6E8315;
}

DEFINE_HOOK(0x73ED40, UnitClass_Mi_Harvest_PathfindingFix, 0x7)
{
	GET(UnitClass*, pThis, EBP);
	LEA_STACK(CellStruct*, closeTo, STACK_OFFSET(0x64, -0x4C));
	LEA_STACK(CellStruct*, cell, STACK_OFFSET(0x64, -0x54));
	LEA_STACK(CellStruct*, outBuffer, STACK_OFFSET(0x64, -0x3C));

	R->EAX(MapClass::Instance->NearByLocation(*outBuffer, *cell, pThis->Type->SpeedType, -1, pThis->Type->MovementZone, false, 1, 1, false, false, false, true, *closeTo, false, false));

	return 0x73ED7A;
}

// DEFINE_HOOK(0x5657A0, MapClass_OpBracket_CellStructPtr, 0x5)
// {
// 	GET_STACK(CellStruct*, pCell, 0x4);
// 	GET_STACK(DWORD, callr, 0x0);
//
// 	if (!pCell) {
// 		Debug::FatalErrorAndExit("addr [0x%x] calling MapClass_OpBracket_CellStruct with nullptr cell!\n", callr);
// 	}
//
// 	return 0x0;
// }

//DEFINE_PATCH(0x6443E2, 0xBA, 0x01, 0x00, 0x00, 0x00, 0x90);
//
//DEFINE_HOOK(0x62E430, ParticleSystemClass_AddTovector_nullptrParticle, 0x9)
//{
//	GET_STACK(DWORD, caller ,0x0);
//	GET(ParticleSystemClass*, pThis, ECX);
//
//	if (!pThis)
//		Debug::FatalErrorAndExit("Function [ParticleSystemClass_AddTovector] Has missing pThis Pointer called from [0x%x]\n", caller);
//
//	return 0x0;
//}

//DEFINE_HOOK(0x5AE610, Matrix_OPMultiply, 0x5)
//{
//	GET(Matrix3D*, pThis, ECX);
//	GET_STACK(Matrix3D*, pThat, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pThis || !pThat)
//		Debug::FatalErrorAndExit(__FUNCTION__" Called from(0x%x) with Invalid args ptr!\n", caller);
//
//	return 0x0;
//}

//static FootClass* LastAccessThisFunc;
//
//DEFINE_HOOK(0x42C2BF, AstarClass_FindPath_SaveArgs, 0x6)
//{
//	GET(FootClass*, pFoot, ESI);
//	LastAccessThisFunc = pFoot;
//	return 0x0;
//}

//static constexpr constant_ptr<DynamicVectorClass<SubzoneTrackingStruct>, 0x87F874> const SubzoneTrackingStructVector {};
//#include <ExtraHeaders/AStarClass.h>

#ifdef _PATHFIND
#pragma optimize("", off )
DEFINE_HOOK(0x42C4FE, AstarClass_FindPath_nullptr, 0x9)
{
	GET(int, SubZoneTracking_Idx, EDX);
	GET(int, SubZoneConnection_Idx, ECX);
	GET(int, PassabilityData_To, EBX);
	GET_BASE(CellStruct*, pFrom, 0x8);
	GET_BASE(CellStruct*, pTo, 0xC);
	GET_BASE(MovementZone, movementZone, 0x10);
	GET_BASE(FootClass*, pFoot, 0x14);

	Debug::Log("FindingPath for [%s(0x%x) - Owner[%s(0x%x)] from[%d , %d] to [%d , %d] MovementZone [%s(%d)] DriverKilled[%s] \n",
		pFoot->get_ID(), pFoot,
		pFoot->Owner->get_ID(), pFoot->Owner,
		pFrom->X, pFrom->Y,
		pTo->X, pTo->Y,
		TechnoTypeClass::MovementZonesToString[int(movementZone)], int(movementZone),
		TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled ? "Yes" : "No"
	);

	return 0x0;
	/*
	const auto SubZoneTrackingArray = &SubzoneTrackingStruct::Array[0];
	const auto SubZobneConnectionPtr = SubZoneTrackingArray->Items + SubZoneTracking_Idx;
	const auto SubZobneConnectionPtr_offsetted = SubZobneConnectionPtr + SubZoneConnection_Idx;
	if (SubZoneTrackingArray->Count <= SubZoneConnection_Idx)
		Debug::FatalErrorAndExit("AstarClass_FindPath trying to offset SubzoneConnection array pointer to [%d] but the array only has[%d]!\n" , SubZoneConnection_Idx, SubZoneTrackingArray->Count);

	const auto ptr = SubZobneConnectionPtr_offsetted->SubzoneConnections.Items;
	const auto array_count = SubZobneConnectionPtr_offsetted->SubzoneConnections.Count;

	R->EDX(ptr);
	R->ECX(array_count);

	//this keep the thing clean
	//`SubZobneConnectionPtr` will contain broken pointer at some point tho ,....
	if (array_count > 0 && ptr)
	{
		//if(!SubZobneConnectionPtr)
			//Debug::FatalErrorAndExit("AStarClass will crash because SubZone is nullptr , last access is from [%s(0x%x) - Owner : (%s) \n", LastAccessThisFunc->get_ID(), LastAccessThisFunc, LastAccessThisFunc->Owner->get_ID());

		return 0x42C519;
	}

	return 0x42C740;
	*/

}
#pragma optimize("", on )
#endif

#include <VoxClass.h>

#ifdef OLD_
DEFINE_HOOK(0x48248D, CellClass_CrateBeingCollected_MoneyRandom, 6)
{
	GET(int, nCur, EAX);

	const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;

	if (nAdd > 0)
		nCur += ScenarioClass::Instance->Random.RandomFromMax(nAdd);

	R->EDI(nCur);
	return 0x4824A7;
}

DEFINE_HOOK(0x481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_HOOK(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_HOOK(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult == 1.0) ? 0x481D52 : 0x481C86;
}

DEFINE_HOOK(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || TechnoExtContainer::Instance.Find(Unit)->AE_Cloak)
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExtContainer::Instance.Find(Unit->GetTechnoType())->CloakAllowed)
		? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
DEFINE_HOOK(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	TechnoExtContainer::Instance.Find(Unit)->AE_Cloak = true;
	TechnoExt_ExtData::RecalculateStat(Unit);
	return 0x482956;
}

DEFINE_HOOK(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult = Pow_ArmorMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x482E89;
	}
	return 0x482E92;
}

DEFINE_HOOK(0x48303A, CellClass_CrateBeingCollected_Speed2, 6)
{
	GET(FootClass*, Unit, EDI);
	GET_STACK(double, Pow_SpeedMultiplier, 0x20);

	// removed aircraft check
	// these originally not allow those to gain speed mult

	if (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult = Pow_SpeedMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->CL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

DEFINE_HOOK(0x483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult = Pow_FirepowerMultiplier;
		TechnoExt_ExtData::RecalculateStat(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483258;
	}
	return 0x483261;
}
#endif

#ifndef CRATE_HOOKS
enum class MoveResult : char
{
	cannot, can
};

// what is the boolean return for , heh
MoveResult CollecCrate(CellClass* pCell, FootClass* pCollector)
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
					Debug::Log("Springing trigger on crate at %d,%d\n", pCell->MapCoords.X, pCell->MapCoords.Y);
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
					int totalshares = 0;
					for (auto& pCrate : CrateTypeClass::Array)
					{
						totalshares += pCrate->Weight;
					}

					int random = ScenarioClass::Instance->Random.RandomRanged(1, totalshares);
					int sharecount = 0;
					unsigned int ovelaydata = 0u;

					for (auto& pCrate : CrateTypeClass::Array)
					{
						sharecount += pCrate->Weight;
						if (random <= sharecount)
							break;

						++ovelaydata;
					}

					data = (Powerup)ovelaydata;
				}
#pragma endregion

				if (SessionClass::Instance->GameMode != GameMode::Campaign)
				{
					auto pBase = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);

					if (GameModeOptionsClass::Instance->Bases
						&& !pCollectorOwner->OwnedBuildings
						&& pCollectorOwner->Available_Money() > RulesExtData::Instance()->FreeMCV_CreditsThreshold
						&& !pCollectorOwner->OwnedUnitTypes.GetItemCount(pBase->ArrayIndex)
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

						if (!TechnoTypeExtContainer::Instance.Find(pCollector->GetTechnoType())->CloakAllowed || pCollector->CanICloakByDefault() || TechnoExtContainer::Instance.Find(pCollector)->AE_Cloak)
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
						if (TechnoExtContainer::Instance.Find(pCollector)->AE_ArmorMult != 1.0)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Speed:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE_SpeedMult != 1.0 || pCollector->WhatAmI() == AbstractType::Aircraft)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Firepower:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE_FirePowerMult != 1.0 || !pCollector->IsArmed())
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

					if (landType == LandType::Water && !CrateTypeClass::Array[(int)data]->Naval)
					{
						data = Powerup::Unit;
					}

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

					if (pCollectorOwner->ControlledByCurrentPlayer()) {
						auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
						VocClass::PlayIndexAtPos(CrateTypeClass::Array[(int)idx]->Sound, loc, nullptr);
					}
				};

				auto GeiveMoney = [&]()
					{

						Debug::Log("Crate at %d,%d contains money\n", pCell->MapCoords.X, pCell->MapCoords.Y);

						if (!soloCrateMoney)
						{
							const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;
							int crateMax = 900;

							if (nAdd > 0)
								crateMax += ScenarioClass::Instance->Random.RandomFromMax(nAdd);

							soloCrateMoney = ScenarioClass::Instance->Random.RandomRanged((int)something, (int)something + crateMax);
						}

						const auto pHouseDest = pCollectorOwner->ControlledByCurrentPlayer() || SessionClass::Instance->GameMode != GameMode::Campaign
							? pCollectorOwner : HouseClass::CurrentPlayer();

						pHouseDest->TransactMoney(soloCrateMoney);
						if (pCollectorOwner->ControlledByCurrentPlayer()) {
							auto loc_fly = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
							FlyingStrings::AddMoneyString(true, soloCrateMoney, pHouseDest, AffectedHouse::Owner, loc_fly);
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
					Debug::Log("Crate at %d,%d contains a unit\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					UnitTypeClass* Given = nullptr;
					if (force_mcv)
					{
						Given = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);
					}

					if (!Given)
					{
						if (pCollectorOwner->OwnedBuildingTypes.GetItemCount(RulesClass::Instance->BuildRefinery[0]->ArrayIndex) > 0
						 || pCollectorOwner->OwnedBuildingTypes.GetItemCount(RulesClass::Instance->BuildRefinery[1]->ArrayIndex) > 0
						&& !pCollectorOwner->OwnedUnitTypes.GetItemCount(RulesClass::Instance->HarvesterUnit[0]->ArrayIndex)
						&& !pCollectorOwner->OwnedUnitTypes.GetItemCount(RulesClass::Instance->HarvesterUnit[1]->ArrayIndex)
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

							auto alternative_loc = MapClass::Instance->NearByLocation(pCell->MapCoords, Given->SpeedType, -1, Given->MovementZone, 0, 1, 1, 0, 0, 0, 1, CellStruct::Empty, false, false);

							if (alternative_loc.IsValid())
							{
								if (pCreatedUnit->Unlimbo(CellClass::Cell2Coord(alternative_loc), DirType::Min))
								{
									PlaySoundAffect(Powerup::Unit);
									return MoveResult::cannot;
								}
							}

							GameDelete<false>(pCreatedUnit);
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
					Debug::Log("Crate at %d,%d contains base healing\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					PlaySoundAffect(Powerup::HealBase);
					for (int i = 0; i < LogicClass::Instance->Count; ++i)
					{
						if (auto pTechno = abstract_cast<TechnoClass*>(LogicClass::Instance->Items[i]))
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
					Debug::Log("Crate at %d,%d contains explosives\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
					for (int i = 5; i > 0; --i)
					{
						int scatterDistance = ScenarioClass::Instance->Random.RandomFromMax(512);
						auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
						auto randomCoords = MapClass::GetRandomCoordsNear(loc, scatterDistance, false);
						MapClass::DamageArea(randomCoords, damage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);
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
					Debug::Log("Crate at %d,%d contains napalm\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto loc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
					auto Collector_loc = (pCollector->GetCoords() + loc) / 2;

					GameCreate<AnimClass>(AnimTypeClass::Array->Items[0], Collector_loc, 0, 1, 0x600, 0, 0);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->FlameDamage, nullptr, 1, false, 0);
					MapClass::DamageArea(Collector_loc, damage, nullptr, RulesClass::Instance->FlameDamage, true, false);

					PlayAnimAffect(Powerup::Napalm);
					return MoveResult::can;
				}
				case Powerup::Darkness:
				{
					Debug::Log("Crate at %d,%d contains 'shroud'\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					MapClass::Instance->Reshroud(pCollectorOwner);
					PlayAnimAffect(Powerup::Darkness);
					break;
				}
				case Powerup::Reveal:
				{
					Debug::Log("Crate at %d,%d contains 'reveal'\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					MapClass::Instance->Reveal(pCollectorOwner);
					PlaySoundAffect(Powerup::Reveal);
					PlayAnimAffect(Powerup::Reveal);
					break;
				}
				case Powerup::Armor:
				{
					Debug::Log("Crate at %d,%d contains armor\n", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = generic_cast<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE_ArmorMult == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE_ArmorMult *= something;
									AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pCollector)->AeData, pCollector);

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
					Debug::Log("Crate at %d,%d contains speed\n", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = generic_cast<FootClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->WhatAmI() != AbstractType::Aircraft)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE_SpeedMult == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE_SpeedMult *= something;
									AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pCollector)->AeData, pCollector);

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
					Debug::Log("Crate at %d,%d contains firepower\n", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = generic_cast<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius
									&& TechnoExtContainer::Instance.Find(pCollector)->AE_FirePowerMult == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE_FirePowerMult *= something;
									AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pCollector)->AeData, pCollector);

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
					Debug::Log("Crate at %d,%d contains cloaking device\n", pCell->MapCoords.X, pCell->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = generic_cast<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->IsOnMap)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(pCell->MapCoords, pCell->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;

								if ((int)place.Length() < RulesClass::Instance->CrateRadius)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE_Cloak = true;
									AresAE::RecalculateStat(&TechnoExtContainer::Instance.Find(pCollector)->AeData, pCollector);
								}
							}
						}
					}

					PlayAnimAffect(Powerup::Cloak);
					break;
				}
				case Powerup::ICBM:
				{
					Debug::Log("Crate at %d,%d contains ICBM\n", pCell->MapCoords.X, pCell->MapCoords.Y);

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
					Debug::Log("Crate at %d,%d contains veterancy(TM)\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					const int MaxPromotedCount = (int)something;

					if (MaxPromotedCount > 0)
					{
						for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
						{
							if (auto pTechno = generic_cast<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
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
											do{
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
					Debug::Log("Crate at %d,%d contains poison gas\n", pCell->MapCoords.X, pCell->MapCoords.Y);

					if (auto WH = WarheadTypeClass::Array->GetItemOrDefault(WarheadTypeClass::FindIndexById("GAS")))
					{

						bool randomizeCoord = true;
						auto collector_loc = pCell->GetCoords();

						MapClass::DamageArea(collector_loc, (int)something, nullptr, WH, true, nullptr);

						for (int i = 0; i < 8;)
						{
							CellClass* pDestCell = pCell;
							if (randomizeCoord)
							{
								CellStruct dest {};
								MapClass::GetAdjacentCell(&dest, &pCell->MapCoords, (DirType)i);
								pDestCell = MapClass::Instance->GetCellAt(dest);
							}

							MapClass::DamageArea(pDestCell->GetCoords(), (int)something, nullptr, WH, true, nullptr);
							randomizeCoord = ++i < 8;
						}
					}

					PlaySoundAffect(Powerup::Gas);
					PlayAnimAffect(Powerup::Gas);
					break;
				}
				case Powerup::Tiberium:
				{
					Debug::Log("Crate at %d,%d contains tiberium\n", pCell->MapCoords.X, pCell->MapCoords.Y);
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
					Debug::Log("Crate at %d,%d contains Squad\n", pCell->MapCoords.X, pCell->MapCoords.Y);

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
					Debug::Log("Crate at %d,%d contains Invulnerability\n", pCell->MapCoords.X, pCell->MapCoords.Y);
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
					Debug::Log("Crate at %d,%d contains IonStorm\n", pCell->MapCoords.X, pCell->MapCoords.Y);
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
					Debug::Log("Crate at %d,%d contains Pod\n", pCell->MapCoords.X, pCell->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper) {
						 return (AresNewSuperType)pSuper->Type->Type == AresNewSuperType::DropPod && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
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
					Debug::Log("Crate at %d,%d contains %s\n", pCell->MapCoords.X, pCell->MapCoords.Y , CrateTypeClass::Array[(int)data]->Name);
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

DEFINE_HOOK(0x481A00, CellClass_CollectCrate_Handle, 0x6)
{
	GET(CellClass*, pThis, ECX);
	GET_STACK(FootClass*, pCollector, 0x4);
	R->EAX(CollecCrate(pThis, pCollector));
	return 0x483391;
}

DEFINE_HOOK(0x56BFC2, MapClass_PlaceCrate_MaxVal, 0x5)
{
	return R->EDX<int>() != (int)CrateTypeClass::Array.size()
		? 0x56BFC7 : 0x56BFFF;
}

DEFINE_HOOK(0x475A44, CCINIClass_Put_CrateType, 0x7)
{
	GET_STACK(int, crateType, 0x8);

	const auto pCrate = CrateTypeClass::FindFromIndexFix(crateType);
	if (!pCrate)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType Pointer for[%d]!\n", crateType);
	}

	R->EDX(pCrate->Name.data());
	return 0x475A4B;
}
DEFINE_HOOK(0x475A1F, RulesClass_Put_CrateType, 0x5)
{
	GET(const char*, crate, ECX);

	const int idx = CrateTypeClass::FindIndexById(crate);
	if (idx <= -1)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType index for[%s]!\n", crate);
	}
	R->EAX(idx);
	return 0x475A24;
}

DEFINE_HOOK(0x48DE79, CrateTypeFromName, 0x7)
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

DEFINE_HOOK(0x73844A, UnitClass_Destroyed_PlaceCrate, 0x8)
{
	GET(UnitClass*, pThis, ESI);
	GET(CellStruct, cell, EAX);

	const auto CrateType = &TechnoTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CrateType;
	PowerupEffects crate = CrateType->isset() ? (PowerupEffects)CrateType->Get() : (PowerupEffects)CrateTypeClass::Array.size();
	MapClass::Instance->Place_Crate(cell, crate);
	return 0x738457;
}

DEFINE_HOOK(0x4421F2, BuildingClass_Destroyed_PlaceCrate, 0x6)
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

//DEFINE_HOOK(0x42CC48, AStarClass_RegularFindpathError, 0x5)
//{
//	GET_STACK(CellStruct, from, 0x30 - 0x1C);
//	GET_STACK(CellStruct, to, 0x30 - 0x20);
//
//	Debug::Log("Regular findpath failure: (%d,%d) -> (%d, %d)\n", from.X, from.Y, to.X, to.Y);
//	return 0x42CC6D;
//}

//TechnoClass_CTOR_TiberiumStorage
//DEFINE_JUMP(LJMP, 0x6F2ECE , 0x6F2ED3)
//HouseClass_CTOR_TiberiumStorages
//DEFINE_JUMP(LJMP, 0x4F58CD , 0x4F58D2)

#ifndef STORAGE_HOOKS

#ifndef BUILDING_STORAGE_HOOK
// spread tiberium on building destruction. replaces the
// original code, made faster and spilling is now optional.
DEFINE_HOOK(0x441B30, BuildingClass_Destroy_Refinery, 0x6)
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

DEFINE_HOOK(0x445FE4, BuildingClass_GrandOpening_GetStorageTotalAmount, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x446016;
}

DEFINE_HOOK(0x450CD7, BuildingClass_AnimAI_GetStorageTotalAmount_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts()))
		result = int(double(amount * TiberiumClass::Array->Count) / (pThis->Type->Storage + 0.5));

	R->EAX(result);
	R->EDX(pThis->Type);
	return 0x450D09;
}

DEFINE_HOOK(0x450DAA, BuildingClass_AnimAI_GetStorageTotalAmount_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450DDC;
}

DEFINE_HOOK(0x450E12, BuildingClass_AnimAI_GetStorageTotalAmount_C, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x450E3E;
}

DEFINE_HOOK(0x4589C0, BuildingClass_storage_4589C0, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	int result = 0;
	if (auto amount = TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts())
		result = int(amount * TiberiumClass::Array->Count) / pThis->Type->Storage;

	R->EAX(result);
	return 0x4589DC;
}

DEFINE_HOOK(0x44A232, BuildingClass_BuildingClass_Destruct_Storage, 0x6)
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
DEFINE_HOOK(0x708CD9, TechnoClass_PipCount_GetTotalAmounts, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto storange = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	R->EAX(storange->GetAmounts());
	return 0x708CE4;
}
#endif

//UnitClass
#ifndef UNIT_STORAGE_HOOK
//UnitClass_CreditLoad
DEFINE_HOOK(0x7438B0, UnitClass_CreditLoad_Handle, 0xA)
{
	GET(UnitClass*, pThis, ECX);
	int result = int(TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetTotalTiberiumValue() * pThis->Owner->Type->IncomeMult);
	R->EAX((int)result);
	return 0x7438E1;
}

DEFINE_HOOK(0x73D4A4, UnitClass_Harvest_IncludeWeeder, 0x6)
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

DEFINE_HOOK(0x73E3BF, UnitClass_Mi_Unload_replace, 0x6)
{
	GET(BuildingClass* const, pBld, EDI);
	GET(UnitClass*, pThis, ESI);

	auto unit_storage = &TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage;
	if (!pBld->Type->Weeder)
		HouseExtData::LastHarvesterBalance = pBld->GetOwningHouse()->Available_Money();// Available_Money takes silos into account

	const  auto pType = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	const int idxTiberium = unit_storage->GetFirstSlotUsed();
	const double totalAmount = unit_storage->GetAmount(idxTiberium);
	const float amountRemoved = idxTiberium != -1 ? std::fabs(pType->HarvesterDumpAmount.Get((float)totalAmount)) : 0.0f;//after decreased

	if (idxTiberium == -1 || unit_storage->DecreaseLevel((float)amountRemoved, idxTiberium) <= 0.0)
	{
		if (pBld->Type->Refinery)
		{ //weed ???
			pBld->PlayNthAnim(BuildingAnimSlot::Production, pBld->GetHealthPercentage_() <= RulesClass::Instance->ConditionYellow);
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
			pBld->Owner->GiveWeed((int)amountRemoved, idxTiberium);
			pThis->Animation.Value = 0;
		}
		else
		{
			TechnoExt_ExtData::DepositTiberium(pBld, pBld->Owner,
			(float)amountRemoved,
			(float)(BuildingTypeExtData::GetPurifierBonusses(pBld->Owner) * amountRemoved),
			idxTiberium
			);
			pThis->Animation.Value = 0;

			BuildingExtContainer::Instance.Find(pBld)->AccumulatedIncome +=
				pBld->Owner->Available_Money() - HouseExtData::LastHarvesterBalance;

		}

	return 0x73E539;
}

DEFINE_HOOK(0x708BC0, TechnoClass_GetStoragePercentage_GetTotalAmounts, 0x6)
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

DEFINE_HOOK(0x7414A0, UnitClass_GetStoragePercentage_GetTotalAmounts, 0x9)
{
	GET(UnitClass*, pThis, ECX);
	double result = pThis->Type->Harvester || pThis->Type->Weeder ?
		TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() : 0.0f;

	result /= (double)pThis->Type->Storage;
	__asm fld result;
	return 0x7414DD;
}

DEFINE_HOOK(0x738749, UnitClass_Destroy_TiberiumExplosive, 0x6)
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
				MapClass::DamageArea(crd, morePower, const_cast<UnitClass*>(pThis), pWH, pWH->Tiberium, pThis->Owner);
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
//#pragma optimize("", off )
DEFINE_HOOK(0x522E70, InfantryClass_MissionHarvest_Handle, 0x5)
{
	GET(InfantryClass*, pThis, ECX);

	if (pThis->Type->Storage)
	{
		const auto v4 = pThis->GetCell();
		const auto val = pThis->GetStoragePercentage();

		if (v4->HasTiberium() && val < 1.0)
		{
			if (pThis->SequenceAnim != DoType::Shovel)
			{
				pThis->PlayAnim(DoType::Shovel);
			}

			auto tibType = v4->GetContainedTiberiumIndex();
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
//#pragma optimize("", on )

DEFINE_HOOK(0x522D50, InfantryClass_StorageAI_Handle, 0x5)
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
						pTypeExt->DisplayIncome_Offset
						);
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
DEFINE_HOOK(0x4F69D5, HouseClass_AvaibleStorage_GetStorageTotalAmounts, 0x6)
{
	GET(IHouse*, pThis, ESI);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	R->EAX((int)HouseExtContainer::Instance.Find(pHouse)->TiberiumStorage.GetAmounts());
	return 0x4F69E5;
}

DEFINE_HOOK(0x4F69A3, HouseClass_AvaibleMoney_GetStorageTotalAmounts, 0x6)
{
	GET(IHouse*, pThis, ESI);
	const auto pHouse = static_cast<HouseClass*>(pThis);
	R->EAX(HouseExtContainer::Instance.Find(pHouse)->TiberiumStorage.GetTotalTiberiumValue());
	return 0x4F69AE;
}

DEFINE_HOOK(0x4F6E70, HouseClass_GetTiberiumStorageAmounts, 0xA)
{
	GET(HouseClass*, pThis, ESI);
	double result = 0.0;
	const double amount = HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts();

	if ((int)amount)
	{
		result = double(amount) / double(pThis->TotalStorage);
	}

	__asm fld result;
	return 0x4F6EA2;
}

DEFINE_HOOK(0x4F8C05, HouseeClass_AI_StorageSpeak_GetTiberiumStorageAmounts, 0x6)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX((int)HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F8C15;
}

DEFINE_HOOK(0x4F96BF, HouseClass_FindBestStorage_GetStorageTotalAmounts, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	R->EAX(HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F96C4;
}

DEFINE_HOOK(0x4F9790, HouseClass_SpendMoney_Handle, 0x6)
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

DEFINE_HOOK(0x4F99A6, HouseClass_UpdateAllSilos_GetStorageTotalAmounts, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	R->EAX(HouseExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts());
	return 0x4F99B1;
}

DEFINE_HOOK(0x502821, HouseClass_RegisterLoss_TiberiumStorage, 0x6)
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

DEFINE_HOOK(0x65DE6B, TeamTypeClass_CreateGroup_IncreaseStorage, 0x6)
{
	GET(FootClass*, pFoot, ESI);
	GET(TechnoTypeClass*, pFootType, EDI);
	TechnoExtContainer::Instance.Find(pFoot)->TiberiumStorage.DecreaseLevel((float)pFootType->Storage, 0);
	return 0x65DE82;
}

//DEFINE_HOOK(0x6C96B0, StorageClass_DecreaseAmount_caller, 0x7)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6C9680, StorageClass_GetAmount_caller, 0x7)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6C9650, StorageClass_GetTotalAmount_caller, 0xB)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6C9690, StorageClass_IncreaseAmount_caller, 0x9)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}
//DEFINE_HOOK(0x6C9820, StorageClass_FirstUsedSlot_caller, 0xA)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6C9600, StorageClass_GetTotalValue_caller, 0xA)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::Log(__FUNCTION__" Caller[0x%x]\n");
//	return 0x0;
//}

#endif

// replacing reference of 7EAF7C
// Replacing DoControls* with own
// replace the name 0x8255C8u

#ifndef AdditionalSequence
static constexpr const char* Sequences_ident[] = {
		"Ready",
		"Guard",
		"Prone",
		"Walk",
		"FireUp",
		"Down",
		"Crawl",
		"Up",
		"FireProne",
		"Idle1",
		"Idle2",
		"Die1",
		"Die2",
		"Die3",
		"Die4",
		"Die5",
		"Tread",
		"Swim",
		"WetIdle1",
		"WetIdle2",
		"WetDie1",
		"WetDie2",
		"WetAttack",
		"Hover",
		"Fly",
		"Tumble",
		"FireFly",
		"Deploy",
		"Deployed",
		"DeployedFire",
		"DeployedIdle",
		"Undeploy",
		"Cheer",
		"Paradrop",
		"AirDeathStart",
		"AirDeathFalling",
		"AirDeathFinish",
		"Panic",
		"Shovel",
		"Carry",
		"SecondaryFire",
		"SecondaryProne",
		"SecondaryFireFly",
		"SecondaryWetAttack"
};

static constexpr std::array<DoStruct, std::size(Sequences_ident)> Sequences_Master = { {
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{1, 1, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 1, 1, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 1, 0, 2},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 0, 0, 1},
	{0, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 1, 1, 4},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	//{16, 0, 0, 0}, //leftover ?
	{1, 1, 0, 1}, //SecondaryFireFly
	{1, 1, 0, 1}, //SecondaryWetAttack
}
};

struct NewDoType
{
	constexpr void FORCEINLINE Initialize()
	{
		for (auto at = this->begin(); at != this->end(); ++at)
		{
			at->StartFrame = 0;
			at->CountFrames = 0;
			at->FacingMultiplier = 0;
			at->Facing = DoTypeFacing::None;
			at->SoundCount = 0;
			at->SoundData[0].Index = -1;
			at->SoundData[0].StartFrame = 0;
			at->SoundData[1].Index = -1;
			at->SoundData[1].StartFrame = 0;
		}
	}

	static const char* GetSequenceName(DoType sequence)
	{
		return Sequences_ident[(int)sequence];
	}

	DoInfoStruct GetSequence(DoType sequence) const
	{
		return this->Data[(int)sequence];
	}

	DoInfoStruct& GetSequence(DoType sequence)
	{
		return this->Data[(int)sequence];
	}

	static DoStruct* GetSequenceData(DoType sequence)
	{
		return const_cast<DoStruct*>(&Sequences_Master[(int)sequence]);
	}

	constexpr DoInfoStruct* begin() { return std::begin(Data); }
	constexpr DoInfoStruct* end() { return std::end(Data); }

	DoInfoStruct Data[std::size(Sequences_ident)];
};

DEFINE_HOOK(0x523876, InfantryTypeClass_CTOR_Initialize, 6)
{
	GET(InfantryTypeClass*, pItem, ESI);

	pItem->ArrayIndex = R->ECX<int>();
	pItem->OccupyWeapon.FLH.X = 0;
	pItem->OccupyWeapon.WeaponType = 0;
	pItem->OccupyWeapon.FLH.Y = 0;
	pItem->OccupyWeapon.BarrelLength = 0;
	pItem->OccupyWeapon.BarrelThickness = 0;
	pItem->OccupyWeapon.TurretLocked = 0;
	pItem->OccupyWeapon.FLH.Z = 0;
	pItem->EliteOccupyWeapon.WeaponType = 0;
	pItem->EliteOccupyWeapon.BarrelLength = 0;
	pItem->EliteOccupyWeapon.FLH.X = 0;
	pItem->EliteOccupyWeapon.BarrelThickness = 0;
	pItem->EliteOccupyWeapon.TurretLocked = 0;
	pItem->RotCount = 8;
	pItem->EliteOccupyWeapon.FLH.Y = 0;
	pItem->RadarVisible = 0;
	pItem->Crushable = 1;
	pItem->Repairable = 0;
	pItem->EliteOccupyWeapon.FLH.Z = 0;
	pItem->Crewed = 0;
	pItem->ImmuneToPsionics = 0;
	pItem->ImmuneToPsionicWeapons = 0;
	pItem->ImmuneToPoison = 0;
	pItem->Parasiteable = 1;
	pItem->Organic = 1;
	pItem->ConsideredAircraft = 0;
	pItem->Bunkerable = 0;
	pItem->Sequence = (DoControls*)GameCreate<NewDoType>();
	pItem->Sequence->Initialize();
	return 0x523970;
}

DEFINE_HOOK(0x520820, InfantryClass_FiringAI_SecondaryFireFly, 0x5)
{
	GET(InfantryClass*, pThis, EBP);
	GET_STACK(int, weaponIdx, 0x34 - 0x24);

	DoType result = weaponIdx == 0 ? DoType::FireFly :
		DoType(42);

	if (!pThis->Type->Sequence->GetSequence(result).CountFrames)
		return 0;

	pThis->PlayAnim(result);
	return 0x520831;
}

DEFINE_HOOK(0x51D7E0, InfantryClass_DoAction_SecondaryWetAttack, 0x5)
{
	GET(InfantryClass*, pThis, ESI);
	GET(DoType, type, EDI);

	if (type == DoType::SecondaryFire || type == DoType::SecondaryProne)
	{

		if (!pThis->Type->Sequence->GetSequence(DoType(43)).CountFrames)
			return 0x0;

		type = DoType(43);

		R->EBP(false);
		R->EDI(type);
		return 0x51D842;
	}

	return 0x0;
}

//the fuck is this ,...
//DEFINE_PATCH_TYPED(BYTE, 0x51DAA9, 6u);
//DEFINE_PATCH_TYPED(BYTE, 0x51DAAE, 3u);

#pragma region ReplaceMasterControl

DEFINE_HOOK(0x51C9E4, InfantryClass_FireError_ReplaceMasterControl, 0x7)
{
	GET(DoType, type, EAX);
	R->CL(NewDoType::GetSequenceData(type)->Interrupt);
	return 0x51C9EB;
}

DEFINE_HOOK(0x51D1BA, InfantryClass_Scatter_ReplaceMasterControl, 0x7)
{
	GET(DoType, type, EAX);
	R->CL(NewDoType::GetSequenceData(type)->Interrupt);
	return 0x51D1C1;
}

DEFINE_HOOK(0x521BFC, InfantryClass_ReadyToCommerce_ReplaceMasterControl, 0x7)
{
	GET(DoType, type, ESI);
	R->AL(NewDoType::GetSequenceData(type)->Interrupt);
	return 0x521C03;
}

DEFINE_HOOK(0x51D925, InfantryClass_DoType_ReplaceMasterControl1, 0x7)
{
	GET(DoType, type, EAX);
	R->CL(NewDoType::GetSequenceData(type)->Interrupt);
	return 0x51D92C;
}

DEFINE_HOOK(0x51D9FA, InfantryClass_DoType_ReplaceMasterControl2, 0x7)
{
	GET(InfantryClass*, pThis, ESI);
	R->AL(NewDoType::GetSequenceData(pThis->SequenceAnim)->Rate);
	return 0x51DA01;
}

DEFINE_HOOK(0x51DA27, InfantryClass_DoType_ReplaceMasterControl3, 0x7)
{
	GET(InfantryClass*, pThis, ESI);
	R->AL(NewDoType::GetSequenceData(pThis->SequenceAnim)->Rate);
	return 0x51DA2E;
}

#pragma endregion

#pragma region S/L
DEFINE_HOOK(0x524B10, InfantryTypeClass_Load_DoControls, 0x5)
{
	GET(InfantryTypeClass*, pThis, EDI);
	GET(IStream*, pStream, ESI);

	pThis->Sequence = (DoControls*)GameCreate<NewDoType>();
	pStream->Read(pThis->Sequence, sizeof(NewDoType), nullptr);
	return 0x524B31;
}

DEFINE_HOOK(0x524C3C, InfantryTypeClass_Save_DoControls, 0x6)
{
	GET(InfantryTypeClass*, pThis, EDI);
	GET(IStream*, pStream, ESI);

	pStream->Write((void*)pThis->Sequence, sizeof(NewDoType), nullptr);
	return 0x524C50;
}
#pragma endregion

#pragma region ReadSequence
void ReadSequence(DoControls* pDoInfo, InfantryTypeClass* pInf, CCINIClass* pINI)
{
	INI_EX IniEX(pINI);

	char section[0x100] = {};
	if (pINI->GetString(pInf->ImageFile, "Sequence", section) > 0)
	{
		for (size_t i = 0; i < std::size(Sequences_ident); ++i)
		{
			char sequenceData[0x100] = {};
			if (pINI->GetString(section, Sequences_ident[i], sequenceData) > 0)
			{
				auto& data = pDoInfo->Data[i];
				const std::string basename = Sequences_ident[i];

				char bufferFacing[4];
				if (sscanf(sequenceData, "%d,%d,%d,%s",
					&data.StartFrame,
					&data.CountFrames,
					&data.FacingMultiplier,
					bufferFacing
				) > 3)
				{
					for (size_t a = 0; a < EnumFunctions::FacingType_to_strings.size(); ++a)
					{
						if (IS_SAME_STR_(EnumFunctions::FacingType_to_strings[a], bufferFacing))
						{
							data.Facing = DoTypeFacing(a);
						}
					}
				}

				char bufferSounds[0x100] = {};
				if (pINI->GetString(section, (basename + "Sounds").c_str(), bufferSounds) > 0)
				{
					auto v7 = strtok(bufferSounds, " ,\t");
					while (v7)
					{
						auto v8 = atoi(v7);
						auto v9 = strtok(0, " ,\t");
						if (!v9)
						{
							break;
						}

						data.SoundCount = v8;

						auto v10 = VocClass::FindIndexById(v9);
						v7 = strtok(0, " ,\t");
						if (v10 != -1)
						{
							for (auto at = data.SoundData;
								at != std::end(data.SoundData);
								++at)
							{
								at->Index = v10;
							}
						}
					}
				}
			}
		}
	}
}

DEFINE_HOOK(0x523D00, InfantryTypeClass_ReadSequence, 0x6)
{
	GET(InfantryTypeClass*, pThis, ECX);
	ReadSequence(pThis->Sequence, pThis, &CCINIClass::INI_Art());
	return 0x524096;
}
#pragma endregion

#else
DEFINE_HOOK(0x523932, InfantryTypeClass_CTOR_Initialize, 8)
{
	GET(InfantryTypeClass*, pItem, ESI)
		pItem->Sequence->Initialize();
	return 0x523970;
}
#endif

//unnessesary call wtf ?
DEFINE_JUMP(LJMP, 0x519211, 0x51922F);

//WW skip shadow for visceroids
//DEFINE_JUMP(LJMP, 0x705FED, 0x70600C);

/*
	Main JellyFish Port
	- UnitClass::Fire -> commanding the locomotor to do 0x8C if return false return the `TechnoClass::CanFire` result ??
	- UnitClass::Can_enter_cell -> 0x73FC43 , check if Jellyfish then return 73FD37
	- UnitClass::Can_enter_cell -> 0x73FAEF , check if Jellyfish first , if yes 0x73FA7C , if no continue to check the value
	- TS function 0x653090 ???
	- UnitClass::Unlibo -> if JellyFish set the stage to 0 , set the rate to 1 , set DelayTime to 1
	- TS function 0x64F020 , Jellyfish update func
	- UnitClass::Approach_Target , IF Unit && if Jellyfish -> return
	- TS ExplosionDamage func -> 0x45F22B ??
*/

// AttackMove Only for Foot
DEFINE_PATCH_TYPED(BYTE, 0x731B67, 4u);

DEFINE_HOOK(0x70FF65, TechnoClass_ApplyLocomotorToTarget_CleaupFirst_Crash, 0x6)
{
	GET(TechnoClass*, pFirer, ESI);

	return pFirer->LocomotorTarget ? 0x0 : 0x70FF77;
}

DEFINE_HOOK(0x6F5EAC, TechnoClass_Talkbuble_playVoices, 0x5)
{
	GET(TechnoClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	if (pTypeExt->TalkbubbleVoices.empty())
		return 0x0;

	const auto& vec = pTypeExt->TalkbubbleVoices[FileSystem::TALKBUBL_Frame() - 1];

	if (!vec.empty())
	{
		VocClass::PlayIndexAtPos(Random2Class::Global->RandomFromMax(vec.size() - 1), pThis->GetCoords(), &pThis->Audio3);
	}

	return  0x0;
}

DEFINE_HOOK(0x520BE5, InfantryClass_UpdateSequence_DeadBodies, 0x6)
{
	enum { SkipGameCode = 0x520CA9 };

	GET(InfantryTypeClass*, pType, ECX);

	AnimTypeClass* pAnimType = nullptr;

	if (pType->DeadBodies.Count)
		pAnimType = pType->DeadBodies[ScenarioClass::Instance->Random.RandomFromMax(pType->DeadBodies.Count - 1)];
	else if (!pType->NotHuman && RulesClass::Instance->DeadBodies.Count)
		pAnimType = RulesClass::Instance->DeadBodies[ScenarioClass::Instance->Random.RandomFromMax(RulesClass::Instance->DeadBodies.Count - 1)];

	if (pAnimType)
	{
		GET(InfantryClass*, pThis, ESI);

		if (const auto pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords()))
		{
			const auto pOwner = pThis->Owner;
			pAnim->Owner = pOwner;
			//pAnim->LightConvert = ColorScheme::Array->GetItem(pOwner->ColorSchemeIndex)->LightConvert;
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x7043B9, TechnoClass_GetZAdjustment_Link, 0x6)
{
	GET(TechnoClass*, pNthLink, EAX);
	return pNthLink ? 0 : 0x7043E1;
}

template<class T, class U>
constexpr int8 __CFADD__(T x, U y)
{
	constexpr int size = sizeof(T) > sizeof(U) ? sizeof(T) : sizeof(U);

	if (size == 1)
		return uint8(x) > uint8(x + y);
	if (size == 2)
		return uint16(x) > uint16(x + y);
	if (size == 4)
		return uint32(x) > uint32(x + y);

	return unsigned __int64(x) > unsigned __int64(x + y);
}

// TODO : percent timer draawing , the game dont like it when i do that without adjusting the posisition :S
void DrawSWTimers(int value, ColorScheme* color, int interval, const wchar_t* label, LARGE_INTEGER* _arg, bool* _arg1)
{
	BitFont* pFont = BitFont::BitFontPtr(TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background);

	const int hour = interval / 60 / 60;
	const int minute = interval / 60 % 60;
	const int second = interval % 60;

	const std::wstring timer_ = hour ?
		std::format(L"{:02}:{:02}:{:02}", hour, minute, second) :
		std::format(L"{:02}:{:02}", minute, second);

	const std::wstring buffer =
		std::format(L"{}  ", label)
		//std::format(L"{}  {}  ", label, timer_)
		;

	int width = 0;
	RectangleStruct rect_bound = DSurface::ViewBounds();
	pFont->GetTextDimension(timer_.data(), &width, nullptr, rect_bound.Width);
	ColorScheme* fore = color;

	if (!interval && _arg && _arg1)
	{
		if ((unsigned __int64)_arg->QuadPart < (unsigned __int64)Game::AudioGetTime().QuadPart)
		{
			auto large = Game::AudioGetTime();
			_arg->LowPart = large.LowPart + 1000;
			_arg->HighPart = __CFADD__(large.LowPart, 1000) + large.HighPart;
			*_arg1 = !*_arg1;
		}

		if (*_arg1)
		{
			fore = ColorScheme::Array->Items[RulesExtData::Instance()->TimerBlinkColorScheme];
		}
	}

	int value_plusone = value + 1;
	Point2D point {
		rect_bound.Width - width - 3 ,
		rect_bound.Height - (value_plusone) * (pFont->field_1C + 2)
	};

	auto pComposite = DSurface::Composite();
	auto rect = pComposite->Get_Rect();
	Point2D _temp {};
	Fancy_Text_Print_Wide(
		&_temp,
		buffer.c_str(),
		pComposite,
		&rect,
		&point,
		color,
		0u,
		TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background
	);

	point.X = rect_bound.Width - 3;
	point.Y = rect_bound.Height - value_plusone * (pFont->field_1C + 2);
	rect = pComposite->Get_Rect();
	Fancy_Text_Print_Wide(
	&_temp,
	timer_.c_str(),
	pComposite,
	&rect,
	&point,
	fore,
	0u,
	TextPrintType::UseGradPal | TextPrintType::Right | TextPrintType::NoShadow | TextPrintType::Metal12 | TextPrintType::Background
	);
}

DEFINE_HOOK(0x6D4B50, PrintOnTactical, 0x6)
{
	GET(int, val, ECX);
	GET(ColorScheme*, pScheme, EDX);
	GET_STACK(int, interval, 0x4);
	GET_STACK(const wchar_t*, text, 0x8);
	GET_STACK(LARGE_INTEGER*, _arg, 0xC);
	GET_STACK(bool*, _arg1, 0x10);
	DrawSWTimers(val, pScheme, interval, text, _arg, _arg1);
	return 0x6D4DAC;
}

//DEFINE_HOOK(0x6D4A35, TacticalClass_Render_HandleSWTextPrint, 0x6)
//{
//	GET(SuperClass*, pSuper, ECX);
//	GET(int, value, EBX);
//	GET(int, time_left, ESI);
//
//	//double percent = ((double)time_left / (double)pSuper->Type->RechargeTime)* 100;
//
//	DrawSWTimers(value++,
//		ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex],
//		time_left / 15,
//		pSuper->Type->UIName,
//		&pSuper->BlinkTimer,
//		&pSuper->BlinkState);
//
//	return 0x6D4A71;
//}

//DEFINE_HOOK(0x4FD500, HouseClass_ExpertAI_Add, 0x6)
//{
//	GET(HouseClass*, pThis, ECX);
//
//
//	return 0x0;
//}

//static int retvalue;
//
//void NAKED _BuildingClass_ExitObject_Add_RET()
//{
//	POP_REG(edi);
//	POP_REG(esi);
//	POP_REG(ebp);
//	SET_REG32(eax, retvalue);
//	POP_REG(ebx);
//	ADD_ESP(0x130);
//	JMP(0x4440D4);
//}

//DEFINE_HOOK(0x444F39, BuildingClass_ExitObject_Add, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Type->PowersUpBuilding[0] != '\0')  {
//		return 0x0;
//	}
//
//	retvalue = BuildingClass_Exit_Object_Custom_Position(pThis);
//	return (int)_BuildingClass_ExitObject_Add_RET;
//}

//DEFINE_HOOK(0x530792, Game_InitSecondaryMixes_Maps, 0x5)
//{
//	if (Phobos::Otamaa::NoCD) {
//		return  0x530B76;
//	}
//
//	return 0x530792;
//}
//void NAKED NOINLINE ret___()
//{
//	POP_REG(edi);
//	POP_REG(esi);
//	POP_REG(ebp);
//	POP_REG(ebx);
//	__asm mov eax, 0;
//	__asm setnz al;
//	__asm add esp, 0x198;
//	JMP(0x531292);
//}
//
//DEFINE_JUMP(LJMP, 0x530B61, 0x530B76);
//DEFINE_JUMP(LJMP, 0x530D05, GET_OFFSET(ret___));

#include <CD.h>

template<DWORD addr, DWORD addr_ptr>
struct MixBundle
{
	constant_ptr<const char, addr> MIXName;
	reference<MixFileClass*, addr_ptr> const MIXptr;
};

static constexpr MixBundle<0x826838, 0x884E38 > const CONQMD {};
static constexpr MixBundle<0x8267EC, 0x884E3C > const CONQUER {};

static constexpr MixBundle<0x826820, 0x884E18 > const GENERMD {};
static constexpr MixBundle<0x826814, 0x884E14 > const GENERIC {};

static constexpr MixBundle<0x826804, 0x884E28 > const ISOGENMD {};
static constexpr MixBundle<0x8267F8, 0x884E24 > const ISOGEN {};

static constexpr MixBundle<0x8267D0, 0x884E40 > const CAMEOMD {};
static constexpr MixBundle<0x8267B4, 0x884E44 > const CAMEO {};

static constexpr MixBundle<0x81C284, 0x884DD8 > const MULTIMD {};

static constexpr MixBundle<0x81C24C, 0x87E738 > const THEMEMD {};
static constexpr MixBundle<0x81C220, 0x87E738 > const THEME {};

static constexpr reference<MixFileClass*, 0x884E64> const MapsMix {};
static constexpr reference < MixFileClass*, 0x884E60> const MapsMDMix {};

template<typename TMixBundle>
inline void LoadMixFile(TMixBundle& bb)
{
	CCFileClass _file { bb.MIXName() };
	if (_file.Exists())
	{
		bb.MIXptr = GameCreate<MixFileClass>(bb.MIXName());
	}
	Debug::Log(" Loading %s ... %s !!!\n", bb.MIXName(), !bb.MIXptr ? "FAILED" : "OK");
}

//DEFINE_HOOK(0x53046A, Game_InitSecondaryMix_handle, 0x5)
//{
//	Debug::Log(" \n");
//	LoadMixFile(CONQMD);
//	LoadMixFile(GENERMD);
//	LoadMixFile(GENERIC);
//	LoadMixFile(ISOGENMD);
//	LoadMixFile(ISOGEN);
//	LoadMixFile(CONQUER);
//	LoadMixFile(CAMEOMD);
//	LoadMixFile(CAMEO);
//
//	int cd = Game::Get_Volume_Index(60);
//	if (cd < 0) {
//		cd = 0;
//	}
//
//	cd += 1;
//	char buffer[260];
//
//	if (CD::IsLocal())
//	{
//		std::snprintf(buffer, sizeof(buffer), "MAPSMD*.MIX");
//		if (Game::File_Finder_Start(buffer)) {
//			MapsMDMix = GameCreate<MixFileClass>(buffer);
//
//			while (Game::File_Finder_Next_Name(buffer))
//			{
//				if (auto pMix = GameCreate<MixFileClass>(buffer)) {
//					MixFileClass::Maps->AddItem(pMix);
//				}
//			}
//
//			Game::File_Finder_End();
//		}
//		else
//		{
//			std::snprintf(buffer, sizeof(buffer), "MAPS*.MIX");
//			if (Game::File_Finder_Start(buffer))
//			{
//				MapsMix = GameCreate<MixFileClass>(buffer);
//
//				while (Game::File_Finder_Next_Name(buffer))
//				{
//					if (auto pMix = GameCreate<MixFileClass>(buffer))
//					{
//						MixFileClass::Maps->AddItem(pMix);
//					}
//				}
//
//				Game::File_Finder_End();
//			}
//		}
//	}
//	else
//	{
//		std::snprintf(buffer, sizeof(buffer), "MAPSMD%02d.MIX", cd);
//		CCFileClass MAPSMD_file { buffer };
//
//		if (MAPSMD_file.Exists())
//		{
//			MapsMDMix = GameCreate<MixFileClass>(buffer);
//		}
//
//		std::snprintf(buffer, sizeof(buffer), "MAPS%02d.MIX", cd);
//		CCFileClass MAPS_file { buffer };
//
//		if (MAPS_file.Exists())
//		{
//			MapsMix = GameCreate<MixFileClass>(buffer);
//		}
//	}
//
//	LoadMixFile(MULTIMD);
//	LoadMixFile(THEMEMD);
//	LoadMixFile(THEME);
//
//	R->ESI(cd);
//	return 0x530CF4;
//}

DEFINE_HOOK(0x52C5A1, InitGame_SecondaryMixInit, 0x9)
{
	const bool result = R->AL();
	Debug::Log(" ...%s!!!\n", !result ? "FAILED" : "OK");
	return 0x52C5D3;
}

//DEFINE_HOOK(0x6E5380  ,TagClass_IsTags_Trigger_Validate , 0x)

DEFINE_HOOK(0x60B865, AdjustWindow_Child, 5)
{
	RECT Rect {};
	GetWindowRect(Game::hWnd(), &Rect);
	R->ESI(R->ESI<int>() - Rect.top);
	R->EDX(R->EDX<int>() - Rect.left);
	return 0;
}

int aval;
int bval;
int cval;

DEFINE_HOOK(0x61E00C, TrackBarWndProc_AdjustLength, 7)
{
	int v2 = bval * cval / aval;
	R->Stack(0x84, v2);
	R->Stack(0x28, v2 + 12);
	return 0;
}

DEFINE_HOOK(0x61DA20, TrackbarMsgProc_SetValueRange, 6)
{
	if (R->Stack<int>(0x158) == 15)
	{
		aval = R->EBP<int>();
		bval = R->EBX<int>();
	}
	return 0x0;
}

DEFINE_HOOK(0x61DA6B, TrackbarMsgProc_GetSlideRange, 7)
{
	if (R->Stack<int>(0x158) == 15)
	{
		cval = R->EAX<int>();
	}
	return 0x0;
}

DEFINE_HOOK(0x42499C, AnimClass_AnimToInf_CivialHouse, 0x6)
{
	R->EAX(HouseExtData::FindFirstCivilianHouse());
	return 0x4249D8;
}

 DEFINE_HOOK(0x458230, BuildingClass_GarrisonAI_CivilianHouse, 0x6)
 {
 	R->EBX(HouseExtData::FindFirstCivilianHouse());
 	return 0x45826E;
 }

DEFINE_HOOK(0x50157C, HouseClass_IsAllowedToAlly_CivilianHouse, 0x5)
{
	R->EAX(RulesExtData::Instance()->CivilianSideIndex);
	return 0x501586;
}

DEFINE_HOOK(0x47233C, CaptureManagerClass_SetOwnerToCivilianHouse, 0x5)
{
	R->ECX(HouseExtData::FindFirstCivilianHouse());
	return 0x472382;
}

DEFINE_HOOK(0x6B0AFE, SlaveManagerClass_FreeSlaves_ToCivilianHouse, 0x5)
{
	R->Stack(0x10, HouseExtData::FindFirstCivilianHouse());
	return 0x6B0B3C;
}

DEFINE_HOOK(0x6869AB, split_from_Read_Scenario_INI_Special, 0x5)
{
	R->EAX(HouseExtData::FindSpecial());
	return 0x6869BC;
}

#pragma region House32LimitFix
//HelperedVector<HouseTypeClass*> Allies;
//HelperedVector<HouseTypeClass*> StartingAllies;
//HelperedVector<HouseTypeClass*> RadarVisibleTo;
//HelperedVector<HouseTypeClass*> TechnoClass_DisplayProductionTo;
// 0x5788 Allies
// 0x1605C StartingAllies
// 0x54E4 RadarVisibleTo
// 0x20D TechnoClass_DisplayProductionTo

#ifdef _TODO
// 4F9A10 -> this->Allies
//main part
int __cdecl HouseClass_IsAlly_HouseIndex(REGISTERS* a1)
{
	unsigned int v2; // edi
	int v3; // eax

	if (!ExtendedHouse)
		return 0;
	v2 = *(a1->_ESP.data + 4);
	if (v2 == *(a1->_ECX.data + 48))
	{
		LOBYTE(a1->_EAX.data) = 1;
		return 0x4F9A1D;
	}
	else
	{
		if (v2 == -1)
		{
			LOBYTE(a1->_EAX.data) = 0;
		}
		else
		{
			v3 = ExtMap_Find(&HouseExtContainer, a1->_ECX.data);
			LOBYTE(a1->_EAX.data) = sub_100072A0((v3 + 0x78), v2);
		}
		return 0x4F9A1D;
	}
}

int __cdecl HouseClass_MakeAlly_1(int a1)
{
	int v2; // ecx
	int v3; // eax
	unsigned int v4; // esi
	_DWORD* v5; // edx
	int v6; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = ExtMap_Find(&HouseExtContainer, *(a1 + 12)) + 120;
	v3 = *(a1 + 16);
	if (*(v3 + 48) >= 0x100u)
		sub_10007220();
	v4 = *(v3 + 48) & 0x3F;
	v5 = (v2 + 8 * (*(v3 + 48) >> 6));
	v6 = 0;
	if (v4 >= 0x20)
		v6 = 1 << v4;
	*v5 |= v6 ^ (1 << v4);
	v5[1] |= v6;
	return 0x4F9BAF;
}

int __cdecl HouseClass_MakeAlly_2(int a1)
{
	int v2; // ecx
	int v3; // eax
	unsigned int v4; // esi
	_DWORD* v5; // edx
	int v6; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = ExtMap_Find(&HouseExtContainer, *(a1 + 12)) + 152;
	v3 = *(a1 + 16);
	if (*(v3 + 48) >= 0x100u)
		sub_10007220();
	v4 = *(v3 + 48) & 0x3F;
	v5 = (v2 + 8 * (*(v3 + 48) >> 6));
	v6 = 0;
	if (v4 >= 0x20)
		v6 = 1 << v4;
	*v5 |= v6 ^ (1 << v4);
	v5[1] |= v6;
	return 0x4F9C1F;
}

int __cdecl HouseClass_MakeEnemy(int a1)
{
	DWORD* v1; // esi
	int result; // eax
	DWORD* v3; // edi
	int v4; // eax
	unsigned int v5; // ebx
	_DWORD* v6; // edx
	int v7; // ecx
	unsigned int v8; // ebx
	_DWORD* v9; // edx
	int v10; // ecx
	int v11; // eax
	unsigned int v12; // ebx
	_DWORD* v13; // edx
	int v14; // ecx

	v1 = *(a1 + 8);
	if (ExtendedHouse)
	{
		v3 = *(a1 + 12);
		if (HouseClass::Is_Ally_WithHouse(v3, v1))
		{
			v4 = ExtMap_Find(&HouseExtContainer, v3);
			if (v1[12] >= 0x100)
				sub_10007220();
			v5 = v1[12] & 0x3F;
			v6 = (v4 + 120 + 8 * (v1[12] >> 6));
			v7 = 0;
			if (v5 >= 0x20)
				v7 = 1 << v5;
			*v6 &= ~(v7 ^ (1 << v5));
			v6[1] &= ~v7;
			if (MEMORY[0xA8E7AC])
			{
				if (v1[12] >= 0x100)
					sub_10007220();
				v8 = v1[12] & 0x3F;
				v9 = (v4 + 152 + 8 * (v1[12] >> 6));
				v10 = 0;
				if (v8 >= 0x20)
					v10 = 1 << v8;
				*v9 &= ~(v10 ^ (1 << v8));
				v9[1] &= ~v10;
			}
			HouseClass::Adjust_Threats(v3);
			if (HouseClass::Is_Ally_WithHouse(v1, v3))
			{
				*(a1 + 24) = 1;
				v11 = ExtMap_Find(&HouseExtContainer, v1);
				if (v3[12] >= 0x100)
					sub_10007220();
				v12 = v3[12] & 0x3F;
				v13 = (v11 + 120 + 8 * (v3[12] >> 6));
				v14 = 0;
				if (v12 >= 0x20)
					v14 = 1 << v12;
				*v13 &= ~(v14 ^ (1 << v12));
				v13[1] &= ~v14;
				if (MEMORY[0xA8E7AC])
					sub_10007230((v11 + 152), v3[12], 0);
				HouseClass::Adjust_Threats(v1);
				HouseClass::Update_Anger_Nodes(v1, 1, v1);
			}
			return 0x4FA0E4;
		}
		else
		{
			return 0x4FA1DA;
		}
	}
	else
	{
		result = 0x4FA1DA;
		if (v1)
			return 0x4F9FEF;
	}
	return result;
}

int __cdecl HouseClass_PlayerDefeat_ChangeOwner(REGISTERS* a1)
{
	unsigned int data; // ebx
	DWORD** v2; // edi
	DWORD** v3; // ebp
	DWORD v4; // esi
	_DWORD* v5; // ebp
	int v7; // ecx
	int v8; // eax
	REGISTERS* v9; // [esp+14h] [ebp+4h]

	data = a1->_ESI.data;
	if (!*(RulesExt::Global + 0x12C))
		return 0;
	v2 = MEMORY[0xA8022C];
	v3 = &MEMORY[0xA8022C][MEMORY[0xA80238]];
	v9 = v3;
	if (MEMORY[0xA8022C] == v3)
		return 0;
	while (1)
	{
		v4 = *v2;
		if (*v2 == data
		  || *(v4 + 501)
		  || v4 == MEMORY[0xAC1198]
		  || !strcmpi((*(v4 + 52) + 36), "Observer")
		  || !HouseClass::Is_Player_Control(v4)
		  || !HouseClass::Is_Ally_WithHouse(data, v4))
		{
			goto LABEL_11;
		}
		v5 = ExtMap_Find(&HouseExtContainer, data);
		if (v5)
			break;
		v3 = v9;
	LABEL_11:
		if (++v2 == v3)
			return 0;
	}
	v7 = *(data + 0x24);
	*(data + 502) = 0;
	v8 = (*(v7 + 0x18))(data + 0x24);
	if (v8 <= 0)
		HouseClass::Spend_Money(v4, -v8);
	else
		HouseClass::Refund_Money(v4, v8);
	ResetToNeutral(v5, v4);
	return 0x4F87FF;
}

DWORD __thiscall ResetToNeutral(struct_this_1* this, int a2)
{
	int v2; // esi
	int dword4; // edi
	DWORD** v4; // ebp
	DWORD result; // eax
	DWORD i; // ecx

	v2 = MEMORY[0xA8EC7C];
	dword4 = this->dword4;
	v4 = &MEMORY[0xA8EC7C][MEMORY[0xA8EC88]];
	if (MEMORY[0xA8EC7C] != v4)
	{
		do
		{
			if (*(*v2 + 0x21C) == dword4)
				(*(**v2 + 0x3D4))(*v2, a2, 0);
			v2 += 4;
		}
		while (v2 != v4);
	}
	MapClass::Reset_CellIterator(0x87F7E8u);
	result = MapClass::Get_Cell_From_Iterator(0x87F7E8u);
	for (i = result; result; i = result)
	{
		if (*(i + 0x50) == *(dword4 + 0x30))
			*(i + 0x50) = *(a2 + 48);
		result = MapClass::Get_Cell_From_Iterator(0x87F7E8u);
	}
	return result;
}

int __cdecl HouseClass_ReadINI_MakeAllies(REGISTERS* a1)
{
	unsigned int v2; // esi
	unsigned int v3; // eax
	int v4; // ecx

	if (!ExtendedHouse)
		return 0;
	v2 = *(a1->_EAX.data + 48);
	if (v2 >= 0x100)
		sub_10007220();
	v3 = *(a1->_EAX.data + 48) & 0x3F;
	v4 = 0;
	if (v3 >= 0x20)
		v4 = 1 << v3;
	if (__PAIR64__(
		*(*(a1->_ESP.data + 16) + 8 * (v2 >> 6) + 4) & v4,
		*(*(a1->_ESP.data + 16) + 8 * (v2 >> 6)) & (v4 ^ (1 << v3))))
	{
		return 0x5010DD;
	}
	else
	{
		return 0x5010E7;
	}
}
#endif

#pragma endregion

#ifdef CheckForMapSaveCrash
DEFINE_HOOK(0x50126A , HouseClass_WritetoIni0 , 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if(IS_SAME_STR_("USSR" , pThis->Type->ID))
		Debug::Log("Writing to ini TechLevel for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x501284, HouseClass_WritetoIni2, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini InitialCredit for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012AF, HouseClass_WritetoIni3, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 Control_IQ for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012C9, HouseClass_WritetoIni4, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 Control_Edge for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012E1, HouseClass_WritetoIni5, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 PlayerControl for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x5012F9, HouseClass_WritetoIni6, 0x6)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 Color for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x50134C, HouseClass_WritetoIni7, 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 Allies for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x50136E , HouseClass_WritetoIni8 , 0x5)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 UINAME for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x501380, HouseClass_WritetoIni9, 0xA)
{
	GET(HouseClass*, pThis, EBX);

	if (IS_SAME_STR_("USSR", pThis->Type->ID))
		Debug::Log("Writing to ini0 Base for[%s]\n", pThis->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x42ED72, BaseClass_WriteToINI1, 0x7)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::Log("Writing to ini0 Base PercentBuilt for[%s]\n", ptr->Type->ID);

	return 0x0;
}

DEFINE_HOOK(0x42ED8C , BaseClass_WriteToINI2,0x5)
{
	GET(BaseClass*, pThis, ESI);
	HouseClass* ptr = reinterpret_cast<HouseClass*>((DWORD)pThis - offsetof(HouseClass, Base));

	if (IS_SAME_STR_("USSR", ptr->Type->ID))
		Debug::Log("Writing to ini0 Base NodeCount(%d) for[%s]\n", pThis->BaseNodes.Count, ptr->Type->ID);

	return 0x0;
}
#endif

DEFINE_HOOK(0x449E8E, BuildingClass_Mi_Selling_UndeployLocationFix, 0x5)
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
DEFINE_HOOK(0x522A09, InfantryClass_EnteredThing_Assaulter, 0x6)
{
	enum { retTrue = 0x522A11, retFalse = 0x522A45 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F580
DEFINE_HOOK(0x51F580, InfantryClass_MissionHunt_Assaulter, 0x6)
{
	enum { retTrue = 0x51F58A, retFalse = 0x51F5C0 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F493
DEFINE_HOOK(0x51F493, InfantryClass_MissionAttack_Assaulter, 0x6)
{
	enum { retTrue = 0x51F49D, retFalse = 0x51F4D3 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51968E
DEFINE_HOOK(0x51968E, InfantryClass_sub_519633_Assaulter, 0x6)
{
	enum { retTrue = 0x5196A6, retFalse = 0x519698 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

//4D4BA0
DEFINE_HOOK(0x4D4BA0, InfantryClass_MissionCapture_Assaulter, 0x6)
{
	enum { retTrue = 0x4D4BB4, retFalse = 0x4D4BAA };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExtData::IsAssaulter(pThis) ? retTrue : retFalse;
}

DEFINE_HOOK(0x457DAD, BuildingClass_CanBeOccupied_Assaulter, 0x6)
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

DEFINE_HOOK(0x444159, BuildingClass_KickoutUnit_WeaponFactory_Rubble, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pObj, EDI);

	if (!pThis->Type->WeaponsFactory)
		return 0x4445FB; //not a weapon factory

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	if (pExt->RubbleDestroyed)
	{
		if (pThis->Type->Factory == pObj->GetTechnoType()->WhatAmI() && pThis->Factory && pThis->Factory->Object == pObj)
			return 0x444167; //continue check

		if (pObj->WhatAmI() == InfantryClass::AbsID)
			return 0x4445FB; // just eject
	}

	return 0x444167; //continue check
}


DEFINE_HOOK(0x4580D1, BuildingClass_KickAllOccupants_HousePointerMissing, 0x6)
{
	GET(HouseClass*, pHouse, ECX);
	GET(BuildingClass*, pThis, ESI);
	GET(FootClass*, pOccupier, EDI);

	if (!pHouse)
	{
		Debug::FatalErrorAndExit("BuildingClass::KickAllOccupants for [%x(%s)] Missing Occupier [%x(%s)] House Pointer !\n", pThis, pThis->get_ID(), pOccupier, pOccupier->get_ID());
	}

	return 0x0;
}

DEFINE_HOOK(0x449462, BuildingClass_IsCellOccupied_UndeploysInto, 0x6)
{
	GET(BuildingTypeClass*, pType, EAX);
	LEA_STACK(CellStruct*, pDest, 0x4);
	R->AL(MapClass::Instance->GetCellAt(pDest)
		->IsClearToMove(pType->UndeploysInto->SpeedType, 0, 0, ZoneType::None, MovementZone::Normal, -1, 1)
	);
	return 0x449487;
}

DEFINE_HOOK(0x461225, BuildingTypeClass_ReadFromINI_Foundation, 0x6)
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

		if (pBldext->CustomData.begin() == pBldext->CustomData.end())
		{
			Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n", pSection);
		}
		else
		{
			auto iter = pBldext->CustomData.begin();
			while (iter->X || iter->Y)
			{
				if (++iter == pBldext->CustomData.end())
					Debug::Log("BuildingType %s has a custom foundation which does not include cell 0,0. This breaks AI base building.\n", pSection);

			}
		}
	}

	if (pBldext->IsCustom)
	{
		//Reset
		pThis->Foundation = BuildingTypeExtData::CustomFoundation;
		pThis->FoundationData = pBldext->CustomData.data();
		pThis->FoundationOutside = pBldext->OutlineData.data();
	}

	return 0x46125D;
}

DEFINE_HOOK(0x447110, BuildingClass_Sell_Handled, 0x9)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(int, control, 0x4);

	// #754 - evict Hospital/Armory contents
	TechnoExt_ExtData::KickOutHospitalArmory(pThis);
	BuildingExtContainer::Instance.Find(pThis)->PrismForwarding.RemoveFromNetwork(true);

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

DEFINE_HOOK(0x43D290, BuildingClass_Draw_LimboDelivered, 0x5)
{
	GET(BuildingClass* const, pBuilding, ECX);
	return BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1 ? 0x43D9D5 : 0x0;
}

DEFINE_HOOK(0x442CCF, BuildingClass_Init_Sellable, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	pThis->IsAllowedToSell = !pThis->Type->Unsellable;
	return 0x0;
}

//building abandon sound 458291
//AbandonedSound
DEFINE_HOOK(0x458291, BuildingClass_GarrisonAI_AbandonedSound, 0x6)
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

DEFINE_HOOK(0x4431D3, BuildingClass_Destroyed_removeLog, 0x5)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(int, nData, 0x8C - 0x70);

	R->EBP(--nData);
	R->EDX(pThis->Type);
	return 0x4431EB;
}

//443292
//44177E
DEFINE_HOOK(0x443292, BuildingClass_Destroyed_CreateSmudge_A, 0x6)
{
	GET(BuildingClass*, pThis, EDI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4433F9;
}

DEFINE_HOOK(0x44177E, BuildingClass_Destroyed_CreateSmudge_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4418EC;
}

DEFINE_HOOK(0x44E809, BuildingClass_PowerOutput_Absorber, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, powertotal, 0x8);

	for (auto pPas = pThis->Passengers.GetFirstPassenger();
		pPas;
		pPas = generic_cast<FootClass*>(pPas->NextObject))
	{

		powertotal += abs(TechnoTypeExtContainer::Instance.Find(pPas->GetTechnoType())
			->ExtraPower_Amount.Get(pThis->Type->ExtraPowerBonus));
	}

	R->Stack(0x8, powertotal);
	return 0x44E826;
}

//DEFINE_SKIP_HOOK(0x4417A7, BuildingClass_Destroy_UnusedRandom, 0x0, 44180A);
DEFINE_JUMP(LJMP, 0x4417A7, 0x44180A)

DEFINE_HOOK(0x4D423A, FootClass_MissionMove_SubterraneanResourceGatherer, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pType = pThis->GetTechnoType();
	if (pThis->WhatAmI() == UnitClass::AbsID && pType->ResourceGatherer)
	{
		//https://github.com/Phobos-developers/Phobos/issues/326
		if (pType->IsSubterranean || VTable::Get(((UnitClass*)pThis)->Locomotor.GetInterfacePtr()) == HoverLocomotionClass::vtable)
			pThis->QueueMission(Mission::Harvest, false);
	}
	pThis->EnterIdleMode(false, true);
	return 0x4D4248;
}

DEFINE_HOOK(0x700391, TechnoClass_GetCursorOverObject_AttackFriendies, 6)
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
DEFINE_HOOK(0x6F7EFE, TechnoClass_CanAutoTargetObject_SelectWeapon, 6)
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

DEFINE_HOOK(0x51A2EF, InfantryClass_PCP_Enter_Bio_Reactor_Sound, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(InfantryClass* const, pThis, ESI);
	LEA_STACK(CoordStruct*, pBuffer, 0x44);

	int sound = pThis->Type->EnterBioReactorSound;
	if (sound == -1)
		sound = RulesClass::Instance->EnterBioReactorSound;

	auto coord = pThis->GetCoords(pBuffer);
	VocClass::PlayIndexAtPos(sound, coord, 0);

	return 0x51A30F;
}

DEFINE_HOOK(0x44DBBC, BuildingClass_Mission_Unload_Leave_Bio_Readtor_Sound, 0x7)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(FootClass* const, pPassenger, ESI);
	LEA_STACK(CoordStruct*, pBuffer, 0x40);

	int sound = pPassenger->GetTechnoType()->LeaveBioReactorSound;
	if (sound == -1)
		sound = RulesClass::Instance->LeaveBioReactorSound;

	auto coord = pThis->GetCoords(pBuffer);
	VocClass::PlayIndexAtPos(sound, coord, 0);
	return 0x44DBDA;
}

DEFINE_HOOK(0x447E90, BuildingClass_GetDestinationCoord_Helipad, 0x6)
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

DEFINE_HOOK(0x4DA64D, FootClass_Update_IsInPlayField, 0x6)
{
	GET(UnitTypeClass* const, pType, EAX);
	return pType->BalloonHover || pType->JumpJet ? 0x4DA655 : 0x4DA677;
}

DEFINE_HOOK(0x6FA232, TechnoClass_AI_LimboSkipRocking, 0xA)
{
	return !R->ESI<TechnoClass* const>()->InLimbo ? 0x0 : 0x6FA24A;
}

//DEFINE_HOOK(0x4145B6, AircraftClass_RenderCrash_, 0x6)
//{
//	GET(AircraftTypeClass*, pType, ESI);
//	if (!pType->MainVoxel.HVA) {
//		Debug::Log("Aircraft[%s] Has No HVA ! \n", pType->ID);
//		return 0x4149F6;
//	}
//
//	return 0x0;
//}