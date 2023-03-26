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

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <TunnelLocomotionClass.h>
#include <IsometricTileTypeClass.h>

#include <TiberiumClass.h>
#include <JumpjetLocomotionClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <DriveLocomotionClass.h>
#include <ShipLocomotionClass.h>

#include <Memory.h>
#pragma endregion

DEFINE_HOOK(0x6FA2C7, TechnoClass_AI_DrawBehindAnim, 0x8) //was 4
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(Point2D, nPoint, STACK_OFFS(0x78, 0x50));
	GET_STACK(RectangleStruct, nBound, STACK_OFFS(0x78, 0x50));

	if (pThis->WhatAmI() == AbstractType::Building)
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1)
			return 0x6FA2D8;

	if (!pThis->GetTechnoType()->Invisible)
		pThis->DrawBehind(&nPoint, &nBound);

	return 0x6FA2D8;
}

//DEFINE_HOOK(0x6EE606, TeamClass_TMission_Move_To_Own_Building_index, 0x7)
//{
//	GET(TeamClass*, pThis, EBP);
//	GET(int, nRawData, EAX);
//
//	const auto nBuildingIdx = nRawData & 0xFFFF;
//
//	if (nBuildingIdx < BuildingTypeClass::Array()->Count)
//		return 0x0;
//
//	const auto nTypeIdx = nRawData >> 16 & 0xFFFF;
//	const auto nScript = pThis->CurrentScript;
//
//	Debug::Log("[%x]Team script [%s]=[%d] , Failed to find type[%d] building at idx[%d] ! \n", pThis, nScript->Type->get_ID(), nScript->CurrentMission, nTypeIdx, nBuildingIdx);
//	return 0x6EE7D0;
//}

//Lunar limitation
DEFINE_JUMP(LJMP, 0x546C8B, 0x546CBF);

DEFINE_HOOK(0x74C8FB, VeinholeMonsterClass_CTOR_SetArmor, 0x6)
{
	GET(VeinholeMonsterClass*, pThis, ESI);
	GET(TerrainTypeClass*, pThisTree, EDX);

	if (pThis->GetType() && pThisTree)
		pThis->GetType()->Armor = pThisTree->Armor;

	return 0x0;
}

// thse were removed to completely disable vein
DEFINE_HOOK(0x74D376, VeinholeMonsterClass_AI_TSRandomRate_1, 0x6)
{
	GET(RulesClass*, pRules, EAX);

	auto const nRand = pRules->VeinholeShrinkRate > 0 ?
		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeShrinkRate / 2) : 0;

	R->EAX(pRules->VeinholeShrinkRate + nRand);
	return 0x74D37C;
}

DEFINE_HOOK(0x74C5E1, VeinholeMonsterClass_CTOR_TSRandomRate, 0x6)
{
	GET(RulesClass*, pRules, EAX);
	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74C5E7;
}

DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
{
	GET(RulesClass*, pRules, ECX);

	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74D2AA;
}

static	void __fastcall DrawShape_VeinHole
(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds,
 BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape,
 int ZShapeFrame, int XOffset, int YOffset
)
{
	bool bUseTheaterPal = true;
	CC_Draw_Shape(Surface, bUseTheaterPal ? FileSystem::THEATER_PAL() : Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_JUMP(CALL, 0x74D5BC, GET_OFFSET(DrawShape_VeinHole));

DEFINE_HOOK(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	auto const nTheater = ScenarioClass::Instance->Theater;
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
	auto const pRules = RulesExt::Global();
	auto const pParticle = pRules->VeinholeParticle.Get(pRules->DefaultVeinParticle.Get());
	R->EAX(ParticleSystemClass::Instance->SpawnParticle(pParticle, pCoord));
	return 0x74D100;
}

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(TechnoClass*, pTechnoOwner, EAX);
	GET(CellClass*, pCell, EDI);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	//Debug::Log("Wave Receive Damage for Victim [%x] ! \n", pVictim);
	if (const auto pUnit = specific_cast<UnitClass*>(pVictim))
		if (pUnit->DeathFrameCounter > 0)
			return 0x75F432;

	WarheadTypeExt::DetonateAt(pWarhead, pVictim, pCell->GetCoordsWithBridge(), pTechnoOwner, nDamage);
	//pVictim->ReceiveDamage(&nDamage, 0, pWarhead, pTechnoOwner, false, false, pTechnoOwner ? pTechnoOwner->GetOwningHouse() : nullptr);

	return 0x75F432;
}

DEFINE_HOOK(0x7290AD, TunnelLocomotionClass_Process_Stop, 0x5)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	if (const auto pLinked = pLoco->Owner ? pLoco->Owner : pLoco->LinkedTo)
		if (auto const pCell = MapClass::Instance->GetCellAt(pLinked->GetMapCoords()))
			pCell->CollectCrate(pLinked);

	return 0;
}

DEFINE_HOOK(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

//DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackAllies, 0x5)
//{
//	enum { ClearTarget = 0x0, ExtendChecks = 0x6FA472 };
//	GET(const TechnoClass*, pThis, ESI);
//
//	//if (pThis->GetTechnoType()->AttackFriendlies && pThis->Target)
//	//{
//	//	const int nWeapon = pThis->SelectWeapon(pThis->Target);
//	//	const auto nFireErr = pThis->GetFireError(pThis->Target, nWeapon, false);
//
//	//	if (nFireErr == FireError::CANT || nFireErr == FireError::ILLEGAL)
//	//	{
//	//		return ClearTarget;
//	//	}
//
//	//	return ExtendChecks;
//	//}
//	if(IS_SAME_STR_(pThis->get_ID() , "YUNRU")){
//		if (pThis->GetTechnoType()->AttackFriendlies)
//			return ExtendChecks;
//	}
//
//	return ClearTarget;
//}

DEFINE_HOOK_AGAIN(0x46684A, BulletClass_AI_TrailerInheritOwner, 0x5)
DEFINE_HOOK(0x466886, BulletClass_AI_TrailerInheritOwner, 0x5)
{
	GET(BulletClass*, pThis, EBP);
	//GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x184));

	//Eax is discarded anyway
	if (auto pAnim = GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
	{
		auto const pExt = BulletExt::ExtMap.Find(pThis);
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() :
											(pExt->Owner) ? pExt->Owner : nullptr
								, pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner, false);

	}

	return 0x4668BD;
}

DEFINE_HOOK(0x6FF394, TechnoClass_FireAt_FeedbackAnim, 0x8)
{
	enum
	{
		CreateMuzzleAnim = 0x6FF39C,
		SkipCreateMuzzleAnim = 0x6FF43F
	};

	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(AnimTypeClass*, pMuzzleAnimType, EDI);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		if (auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord))
		{
			AnimExt::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
			if (pThis->WhatAmI() != AbstractType::Building)
				pFeedBackAnim->SetOwnerObject(pThis);
		}
	}

	return pMuzzleAnimType ? CreateMuzzleAnim : SkipCreateMuzzleAnim;
}

DEFINE_HOOK(0x6FF3CD, TechnoClass_FireAt_AnimOwner, 0x7)
{
	enum
	{
		Goto2NdCheck = 0x6FF427,
		DontSetAnim = 0x6FF43F,
		AdjustCoordsForBuilding = 0x6FF3D9,
		Continue = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	//GET(WeaponTypeClass*, pWeapon, EBX);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	if (!pAnim)
		return DontSetAnim;

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

	return pThis->WhatAmI() == AbstractType::Building ? AdjustCoordsForBuilding : Goto2NdCheck;
}

#pragma region WallTower
DEFINE_HOOK(0x4405C1, BuildingClas_Unlimbo_WallTowers_A, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	R->ECX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x4405CF : 0x440606;
}

DEFINE_HOOK(0x440F66, BuildingClass_Unlimbo_WallTowers_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	R->EDX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x440F78 : 0x44104D;
}

DEFINE_HOOK(0x44540D, BuildingClass_ExitObject_WallTowers, 0x5)
{
	GET(BuildingClass*, pThis, EDI);
	R->EDX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x445424 : 0x4454D4;
}

DEFINE_HOOK(0x445ADB, BuildingClass_Limbo_WallTowers, 0x9)
{
	GET(BuildingClass*, pThis, ESI);
	R->ECX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x445AED : 0x445B81;
}

DEFINE_HOOK(0x4514F9, BuildingClass_AnimLogic_WallTowers, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	R->ECX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x45150B : 0x4515E9;
}

DEFINE_HOOK(0x45EF11, BuildingClass_FlushForPlacement_WallTowers, 0x6)
{
	GET(BuildingTypeClass*, pThis, EBX);
	R->EDX(RulesClass::Instance());
	return RulesExt::Global()->WallTowers.Contains(pThis) ? 0x45EF23 : 0x45F00B;
}

DEFINE_HOOK(0x47C89C, CellClass_CanThisExistHere_SomethingOnWall, 0x6)
{
	GET(int, nHouseIDx, EAX);
	GET(CellClass*, pCell, EDI);
	GET(int, idxOverlay, ECX);
	GET_STACK(BuildingTypeClass*, PlacingObject, STACK_OFFS(0x18, -0x8));
	GET_STACK(HouseClass*, PlacingOwner, STACK_OFFS(0x18, -0xC));

	enum { Adequate = 0x47CA70, Inadequate = 0x47C94F } Status = Inadequate;

	HouseClass* OverlayOwner = HouseClass::Array->GetItemOrDefault(nHouseIDx);

	if (PlacingObject)
	{
		const bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;

		if (ContainsWall && (PlacingObject->Gate || RulesExt::Global()->WallTowers.Contains(PlacingObject)))
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
			if (RulesExt::Global()->WallTowers.Contains(PlacingObject) ||
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

DEFINE_HOOK(0x4FE546, BuildingClass_AI_WallTowers, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	return RulesExt::Global()->WallTowers.Contains(pThis) ? 0x4FE554 : 0x4FE6E7;
}

DEFINE_HOOK(0x4FE648, HouseClss_AI_Building_WallTowers, 0x6)
{
	GET(int, nNodeBuilding, EAX);

	if (nNodeBuilding == -1 || RulesExt::Global()->WallTowers.empty())
		return 0x4FE696;

	return std::any_of(RulesExt::Global()->WallTowers.begin(), RulesExt::Global()->WallTowers.end(),
		[&](BuildingTypeClass* const pWallTower) { return pWallTower->ArrayIndex == nNodeBuilding; })
		? 0x4FE656 : 0x4FE696;
}

DEFINE_HOOK(0x5072F8, HouseClass_506EF0_WallTowers, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	return RulesExt::Global()->WallTowers.Contains(pThis) ? 0x50735C : 0x507306;
}

DEFINE_HOOK(0x50A96E, HouseClass_AI_TakeOver_WallTowers_A, 0x6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	return RulesExt::Global()->WallTowers.Contains(pThis) ? 0x50A980 : 0x50AB90;
}

DEFINE_HOOK(0x50A9D2, HouseClass_AI_TakeOver_WallTowers_B, 0x6)
{
	GET(BuildingClass*, pThis, EBX);
	R->EAX(pThis->Type);
	return RulesExt::Global()->WallTowers.Contains(pThis->Type) ? 0x50A9EA : 0x50AB3D;
}
#pragma endregion

//Get_Join_Responses_DuplicateSerianNumber 0x5
//DEFINE_JUMP(LJMP, 0x5E0C24, 0x5E0C4E)

//HouseClass_AllyAIHouses 0x5
//DEFINE_JUMP(LJMP, 0x501640, 0x50174E)

//DEFINE_HOOK(0x5D6BE0, MPGameModeClass_StartingPositionsToHouseBaseCells_Debug, 0x7)
//{
//	Debug::Log("House count = %d", HouseClass::Array->Count);
//	Debug::Log("\n");
//	for (auto pHouse : *HouseClass::Array)
//	{
//		Debug::Log("House start cell = [%d] { %d, %d }",
//		(DWORD)pHouse,
//		pHouse->StartingCell.X,
//		pHouse->StartingCell.Y);
//		Debug::Log("\n");
//	}
//	return 0;
//}

static bool __fastcall AircraftTypeClass_CanUseWaypoint(AircraftTypeClass* pThis, void*)
{
	return !pThis->Spawned;
}

DEFINE_JUMP(VTABLE, 0x7E2908, GET_OFFSET(AircraftTypeClass_CanUseWaypoint));

DEFINE_HOOK(0x467C2E, BulletClass_AI_FuseCheck, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(CoordStruct*, pCoord, ECX);

	R->EAX(BulletExt::FuseCheckup(pThis, pCoord));
	return 0x467C3A;
}

//static BulletClass* Fuse_Bullet = nullptr;
//DEFINE_HOOK(0x467C2A, BulletClass_AI_Fuse_FetchBullet, 0x5)
//{
//	Fuse_Bullet = R->EBP<BulletClass*>();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x4E1278, FuseClass_BulletProximity, 0x5)
//{
//	GET(int, nRange, EAX);
//	auto const pBullet = Fuse_Bullet;
//
//	int nProx = 32;
//	auto pExt = BulletExt::ExtMap.Find(pBullet);
//	if (pExt->TypeExt->Proximity_Range.isset())
//		nProx = pExt->TypeExt->Proximity_Range.Get() * 256;
//
//	Fuse_Bullet = nullptr;
//	return (nProx) <= nRange ? 0x4E1289 : 0x4E127D;
//}

DEFINE_HOOK(0x4B050B, DriveLocomotionClass_Process_Cargo, 0x5)
{
	GET(ILocomotion* const, pILoco, ESI);

	auto const pLoco = static_cast<DriveLocomotionClass* const>(pILoco);

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
	GET(ILocomotion*, pLoco, ESI);
	auto const pDrive = static_cast<DriveLocomotionClass*>(pLoco);
	TechnoExt::PlayAnim(RulesClass::Instance->Wake, pDrive->LinkedTo);
	return 0x4B0828;
}

DEFINE_HOOK(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion*, pLoco, ESI);
	auto const pShip = static_cast<ShipLocomotionClass*>(pLoco);
	TechnoExt::PlayAnim(RulesClass::Instance->Wake, pShip->LinkedTo);
	return 0x69FEF0;
}

DEFINE_HOOK(0x414EAA, AircraftClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0x24));

	GameConstruct(pAnim, TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x414ED0;
}

DEFINE_HOOK(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	GameConstruct(pAnim, TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x7365BB;
}

DEFINE_HOOK(0x70253F, TechnoClass_ReceiveDamage_Metallic_AnimDebris, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x30));
	GET(int, nIdx, EAX);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	GameConstruct(pAnim, RulesClass::Instance->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
	pThis->GetOwningHouse(), false);

	return 0x70256B;
}

DEFINE_HOOK(0x702484, TechnoClass_ReceiveDamage_AnimDebris, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET(AnimClass*, pAnim, EBX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x3C));
	GET(int, nIdx, EDI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	GameConstruct(pAnim, pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim,
	 args.Attacker ?
	 args.Attacker->GetOwningHouse() :args.SourceHouse,
	 pThis->GetOwningHouse(), false);

	return 0x7024AF;
}

//ObjectClass TakeDamage , 5F559C
//UnitClass TakeDamage , 737F0E

//DEFINE_HOOK(0x54DCD2, JumpetLocomotionClass_DrawMatrix, 0x8)
//{
//	GET(FootClass*, pFoot, ECX);
//
//	bool Allow = true;
//	if (pFoot->GetTechnoType()->TiltCrashJumpjet)
//	{
//		Allow = LocomotionClass::End_Piggyback(pFoot->Locomotor);
//	}
//
//	return Allow ? 0x54DCE8 : 0x54DF13;
//}

DEFINE_HOOK(0x703819, TechnoClass_Cloak_Deselect, 0x6)
{
	enum
	{
		Skip = 0x70383C,
		CheckIsSelected = 0x703828,
	};

	return R->ESI<TechnoClass*>()->Owner->IsControlledByHuman()
		? Skip : CheckIsSelected;
}

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum { ContinueCheck = 0x6FC23A, BypassCheck = 0x6FC24D };
	GET(TechnoClass*, pThis, ESI);

	const auto pTypeExt  = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const bool Allow = RulesExt::Global()->AutoAttackICedTarget.Get() || pThis->Owner->ControlledByPlayer();
	return pTypeExt->AllowFire_IroncurtainedTarget.Get(Allow)
		? BypassCheck : ContinueCheck;
}

 DEFINE_HOOK(0x7091FC, TechnoClass_CanPassiveAquire_AI, 0x6)
 {
 	enum { DecideResult  = 0x709202  ,
 		   Continue = 0x0 ,
 		   ContinueCheck = 0x709206,
 		   CantPassiveAcquire = 0x70927D,
 	};

 	GET(TechnoClass*, pThis, ESI);
 	GET(TechnoTypeClass*, pType, EAX);

 	const auto pTypeExt  = TechnoTypeExt::ExtMap.Find(pType);

 	if (pThis->Owner
 		&& !pThis->Owner->IsControlledByHuman()
 		&& !pThis->Owner->IsNeutral()
 		&& pTypeExt->PassiveAcquire_AI.isset()) {
 		return pTypeExt->PassiveAcquire_AI.Get() ?
 		 ContinueCheck : CantPassiveAcquire;
 	}

 	return Continue;
 }

//DEFINE_HOOK(0x45743B, BuildingClass_Infiltrated_StoleMoney_AI, 0xA)
//{
//	GET(BuildingClass*, pThis, EBP);
//	GET(RulesClass*, pRules, EDX);
//	GET_STACK(int, nAvailMoney, 0x18);
//
//	float mult = pRules->SpyMoneyStealPercent;
//	auto const& nAIMult = RulesExt::Global()->AI_SpyMoneyStealPercent;
//
//	if (pThis->Owner && !pThis->Owner->ControlledByPlayer() && nAIMult.isset()) {
//		mult = nAIMult.Get();
//	}
//
//	R->EAX(Game::F2I(nAvailMoney * mult));
//	return 0x45744A;
//}

DEFINE_HOOK(0x6F8260, TechnoClass_EvalObject_LegalTarget_AI, 0x6)
{
	enum
	{
		Continue = 0x0,
		ContinueChecks = 0x6F826E,
		ReturnFalse = 0x6F894F,
		SetAL = 0x6F8266,
	};

	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (pTarget->Owner && pTarget->Owner->IsControlledByHuman())
		return Continue;

	if(pTypeExt->AI_LegalTarget.isset()) {
		return pTypeExt->AI_LegalTarget.Get() ?
			ContinueChecks : ReturnFalse;
	}

	return Continue;
}

//DEFINE_HOOK(0x722FFA, TiberiumClass_Grow_CheckMapCoords, 0x6)
//{
//	enum
//	{
//		increment = 0x72312F,
//		SetCell = 0x723005
//	};
//
//	GET(const MapSurfaceData*, pSurfaceData, EBX);
//	R->EBX(pSurfaceData);
//	const auto nCell = pSurfaceData->MapCoord;
//
//	if (!Map.IsValidCell(nCell))
//	{
//		Debug::Log("Tiberium Growth With Invalid Cell ,Skipping !\n");
//		return increment;
//	}
//
//	R->EAX(Map[nCell]);
//	return SetCell;
//}

//TaskForces_LoadFromINIList_WhySwizzle , 0x5
//DEFINE_JUMP(LJMP, 0x6E8300, 0x6E8315)

DEFINE_HOOK(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
{
	enum
	{
		Skip = 0x4DC1A0,
		Continue = 0x0
	};

	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Attack_Color.isset()) {

		GET(CoordStruct*, pMovingDestCoord, EAX);
		GET(int, nFLH_X, EBP);
		GET(int, nFLH_Y, EBX);
		GET_STACK(int, nFLH_Z, STACK_OFFS(0x34, 0x10));

		if (pTypeExt->CommandLine_Attack_Color.Get() != ColorStruct::Empty) {
			Drawing::Draw_action_lines_7049C0(nFLH_X, nFLH_Y, nFLH_Z, pMovingDestCoord->X, pMovingDestCoord->Y, pMovingDestCoord->Z, Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color.Get()), false, false);
		}

		return Skip;
	}

	return Continue;
}

DEFINE_HOOK(0x4DC280, FootClass_DrawActionLines_Move, 0x5)
{
	enum
	{
		Skip = 0x4DC328,
		Continue = 0x0
	};

	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Move_Color.isset()) {

		GET_STACK(CoordStruct, nCooordDest, STACK_OFFS(0x34, 0x24));
		GET(int, nCoordDest_Adjusted_Z, EDI);
		GET(int, nLoc_X, EBP);
		GET(int, nLoc_Y, EBX);
		GET_STACK(int, nLoc_Z, STACK_OFFS(0x34, 0x10));
		GET_STACK(bool, barg3, STACK_OFFSET(0x34, 0x8));

		if (pTypeExt->CommandLine_Move_Color.Get() != ColorStruct::Empty) {
			Drawing::Draw_action_lines_7049C0(nLoc_X, nLoc_Y, nLoc_Z, nCooordDest.X, nCooordDest.Y, nCoordDest_Adjusted_Z, Drawing::RGB2DWORD(pTypeExt->CommandLine_Move_Color.Get()), barg3, false);
		}

		return Skip;
	}

	return Continue;
}

DEFINE_HOOK(0x4DBDB6, FootClass_IsCloakable_CloakMove, 0x6)
{
	enum { Continue = 0x0, ReturnFalse = 0x4DBDEB };
	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if(pTypeExt->CloakMove.isset()) {
		return pTypeExt->CloakMove.Get() && !pThis->Locomotor->Is_Moving() ?
		ReturnFalse : Continue;
	}

	return Continue;
}

/*
DEFINE_HOOK(0x4F9AF0, HouseClass_IsAlly_AbstractClass, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	bool res = false;
	if (pTarget)
	{
		auto const pWhat = pTarget->WhatAmI();
		if ((pTarget->AbstractFlags & AbstractFlags::Object) != AbstractFlags::None)
		{
			res = pThis->IsAlliedWith(pTarget->GetOwningHouse());
		}
		else if (pWhat == AbstractType::House)
		{
			switch (pWhat)
			{
			case AbstractType::House:
				res = pThis->IsAlliedWith(static_cast<HouseClass*>(pTarget));
			break;
			case AbstractType::Bomb:
				res = pThis->IsAlliedWith(pTarget->GetOwningHouse());
			break;
			}
		}
	}

	R->AL(res);
	return 0x4F9B11;
}*/

//DEFINE_HOOK(0x4F9A50, HouseClass_IsAlly_HouseClass, 0x6)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pTarget, 0x4);
//
//	bool ret = false;
//	if (pTarget) {
//		if (pThis == pTarget)
//			ret = true;
//		else
//			ret = pThis->IsAlliedWith(pTarget->ArrayIndex);
//	}
//	R->AL(ret);
//	return 0x4F9A60;
//}

//DEFINE_HOOK(0x4F9A90 ,HouseClass_IsAlly_ObjectClass , 0x7)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(ObjectClass*, pTarget, 0x4);
//	R->AL(pThis->IsAlliedWith(pTarget->GetOwningHouse()));
//	return 0x4F9AAB;
//}

//DEFINE_HOOK(0x50CA30, HouseClass_Center_50C920, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis->BelongsToATeam())
//		return 0x50CAB4;
//
//	R->CL(pThis->GetTechnoType()->DeploysInto->ConstructionYard);
//	return 0x50CA3C;
//}

//DEFINE_HOOK(0x737E66, UnitClass_ReceiveDamage_Debug, 0x8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x48, -0xC));
//	Debug::Log("[%d] %s Warhead Destroying %s ! \n ", pWH, pWH->ID, pThis->get_ID());
//	return 0x0;
//}
//
//DEFINE_HOOK(0x4A9004, MouseClass_CanPlaceHere_SkipSelf, 0x6)
//{
//	if (auto const pHouse = HouseClass::Array->GetItem(R->EAX<int>()))
//	{
//		if (pHouse == R->ECX<HouseClass*>())
//			return 0x4A902C;
//	}
//
//	return 0x0;
//}

DEFINE_HOOK(0x51CDB9, InfantryClass_RandomAnimate_CheckIdleRate, 0x6)
{
	return R->ESI<InfantryClass*>()->Type->IdleRate == -1 ? 0x51D0A0 : 0x0;
}

static void ClearShit(TechnoTypeClass* a1)
{
	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	auto pNext = pObjectToSelect;
	while (!pNext || pNext->GetTechnoType() != a1)
	{
		pNext = MapClass::Instance->NextObject(pNext);
		if (!pNext || pNext == pObjectToSelect)
			return;
	}

	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	MapClass::UnselectAll();
	pObjectToSelect->Select();
	MapClass::Instance->CenterMap();
	MapClass::Instance->MarkNeedsRedraw(1);
}

//DEFINE_HOOK(0x6AB63B, SelectClass_Action_UnableToBuild, 0x6)
//{
//	GET(TechnoTypeClass*, pTech, EAX);
//
//	if (HouseClass::CurrentPlayer()->CanBuild(pTech, 0, 0) == CanBuildResult::Buildable)
//		return 0;
//
//	ClearShit(pTech);
//	return 0x6AB95A;
//}

//DEFINE_HOOK(0x6A9791, StripClass_DrawIt_BuildingFix, 0x6)
//{
//	GET(BuildingTypeClass*, pTech, EBX);
//	auto const pHouse = HouseClass::CurrentPlayer();
//
//	const auto pFac = pHouse->GetPrimaryFactory(pTech->WhatAmI(), pTech->Naval, pTech->BuildCat);
//	if (pFac && pFac->Object->GetTechnoType() != pTech)
//		R->Stack(0x17, 1);
//
//
//	return 0;
//}

//DEFINE_HOOK_AGAIN(0x6A91E5, SidebarClass_Update_MultiMoney, 0x6)
//DEFINE_HOOK(0x6A9137, SidebarClass_Update_MultiMoney, 0x6)
//{
//	GET(int, nMoney, EAX);
//	GET(HouseClass*, pHouse, ESI);
//
//	if (!pHouse->IsControlledByCurrentPlayer())
//	{
//		nMoney = static_cast<int>(nMoney * 100.0 / RulesGlobal->MultiplayerAICM.GetItem(static_cast<int>(pHouse->AIDifficulty)));
//	}
//
//	R->EAX(nMoney);
//	return R->Origin() + 6;
//}

DEFINE_HOOK(0x6FDE05, TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (TechnoExt::ExtMap.Find(pThis)->DelayedFire_DurationTimer <= 0)
	{
		if (pWeaponExt->RemoveTechnoAfterFiring.Get())
			TechnoExt::KillSelf(pThis, KillMethod::Vanish);
		else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
			TechnoExt::KillSelf(pThis, KillMethod::Explode);
	}

	return 0;
}

DEFINE_HOOK(0x6FA232, TechnoClass_AI_LimboSkipRocking, 0xA)
{
	return !R->ESI<TechnoClass*>()->InLimbo ? 0x0 : 0x6FA24A;
}

//
//DEFINE_HOOK(0x67E875, LoadGame_Start_AfterMouse, 0x6)
//{
//	Unsorted::CursorSize = nullptr;
//	return 0;
//}

//DEFINE_HOOK(0x4CA0F8, FactoryClass_AbandonProduction_RemoveProduct, 0x7)
//{
//	GET(TechnoClass*, pProduct, ECX);
//	pProduct->UnInit();
//	return 0x4CA0FF;
//}

//DEFINE_HOOK_AGAIN(0x534F4E, ScoreClass_LoadMix, 0x5)
//DEFINE_HOOK(0x6D97BF , ScoreClass_LoadMix, 0x5)
//{
//	Debug::Log("%s\n", R->ESP<char*>());
//	return R->Origin() + 5;
//}

DEFINE_HOOK_AGAIN(0x70FC90, TechnoClass_Deactivate, 0x6)
DEFINE_HOOK(0x70FBE0, TechnoClass_Deactivate, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	if (auto pType = pThis->GetTechnoType()) {
		if (pType->PoweredUnit && pThis->Owner) {
			pThis->Owner->RecheckPower = true;
		}
	}

	return 0x0;
}

#ifdef Ares_3_0_p1
DEFINE_HOOK(0x7463A8, UnitClass_Captured_DriverKilled, 0x6)
{
	enum { Failed = 0x7463EC, Continue = 0x0 };
	GET(UnitClass*, pThis, EDI);

	auto const bDriverKilled = (*(bool*)((char*)pThis->align_154 + 0x9C));
	return bDriverKilled ? Failed : Continue;
}
#endif

DEFINE_HOOK(0x701AAD, TechnoClass_ReceiveDamage_WarpedOutBy_Add, 0xA)
{
	enum { NullifyDamage = 0x701AC6, ContinueCheck = 0x701ADB };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(bool, bIgnore, 0xD8);

	const bool IsCurrentlyDamageImmune = pThis->IsBeingWarpedOut()
		|| TechnoExt::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pThis));

	return (IsCurrentlyDamageImmune && !bIgnore) ? NullifyDamage : ContinueCheck;
}

//
//DEFINE_HOOK(0x73B4F4, UnitClass_DrawAsVXL_FiringAnim, 0x8)
//{
//	R->ECX(R->EBX<UnitTypeClass*>()->TurretVoxel.HVA);
//	R->ESI(0);
//	return 0x73B4FC;
//}
//

DEFINE_HOOK(0x4DA9C9, FootClass_Update_DeployToLandSound, 0xA)
{
	GET(TechnoTypeClass*, pType, EAX);
	GET(FootClass*, pThis, ESI);

	return !pType->JumpJet || pThis->GetHeight() <= 0 ? 0x4DAA01 : 0x4DA9D7;
}

DEFINE_HOOK(0x71B14E, TemporalClass_FireAt_ClearTarget, 0x9)
{
	GET(TemporalClass*, pThis, ESI);

	auto pTargetTemp = pThis->Target->TemporalImUsing;

	if (pTargetTemp && pTargetTemp->Target)
		pTargetTemp->LetGo();

	if (pThis->Target->Owner && pThis->Target->Owner->IsControlledByHuman())
		pThis->Target->Deselect();

	return 0x71B17B;
}

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

#ifdef COMPILE_PORTED_DP_FEATURES
DEFINE_HOOK(0x73DDC0, UnitClass_Mi_Unload_DeployIntoSpeed, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (R->AL())
	{
		if (pThis->Type->Speed > 0)
		{
			return 0x73DE20;
		}
		else
		{
			pThis->StopMoving();
			pThis->QueueMission(Mission::Unload, false);
			pThis->NextMission();
			return 0x73DE3A;
		}
	}

	R->ECX(pThis);
	return 0x73DDC6;
}

//DEFINE_HOOK(0x7397E4, UnitClass_DeployIntoBuilding_DesyncFix, 0x5)
//{
//	GET(HouseClass*, pHouse, ECX);
//
//	bool CurPlayer = false;
//	if (SessionGlobal.GameMode != GameMode::Campaign)
//	{
//		CurPlayer = (pHouse->CurrentPlayer);
//	}
//	else
//	{
//		CurPlayer = (pHouse->CurrentPlayer || pHouse->IsInPlayerControl);
//	}
//
//	R->AL(CurPlayer);
//	return 0x7397E9;
//}
#endif

DEFINE_HOOK(0x4DA64D, FootClass_Update_IsInPlayField, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	return pType->BalloonHover || pType->JumpJet ? 0x4DA655 : 0x4DA677;
}

DEFINE_HOOK(0x51D45B, InfantryClass_Scatter_Process, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->JumpJet && pThis->Type->HoverAttack)
	{
		pThis->SetDestination(nullptr, 1);
		return 0x51D47B;
	}

	return 0x0;
}

//
//DEFINE_HOOK(0x4FD635, HouseClass_AI_UpdatePlanOnEnemy_FixDistance, 0x5)
//{
//	GET(HouseClass*, pThis, ESI);
//	GET(HouseClass*, pEnemy, EBX);
//
//	if (pThis->IsAlliedWith(pEnemy))
//		R->EAX(INT_MAX);
//	else
//		R->EAX(pThis->BaseCenter ? pThis->BaseCenter.X : pThis->BaseSpawnCell.X);
//
//	return 0x4FD657;
//}

DEFINE_HOOK(0x5D3ADE, MessageListClass_Init_MessageMax, 0x6)
{
	if (Phobos::Otamaa::IsAdmin)
		R->EAX(14);

	return 0x0;
}

//DEFINE_HOOK(0x5B3614, MissionClass_AssignMission_CheckBuilding, 0x6)
//{
//	GET(MissionClass*, pThis, ESI);
//	GET(Mission, nMission, EAX);
//
//	if (pThis->WhatAmI() == AbstractType::Building && nMission == Mission::Hunt)
//		pThis->QueuedMission = Mission::Guard;
//	else
//		pThis->QueuedMission = nMission;
//
//	return 0x5B361A;
//}

//Ares Hook Here !
//DEFINE_HOOK(0x75F38F, WaveClass_DamageCell_SelectWeapon, 0x6)
//{
//	GET(WaveClass*, pWave, EDX);
//
//	int nWeapon = 0;
//	if (pWave->Target)
//		nWeapon = pWave->Owner->SelectWeapon(pWave->Target);
//
//	R->EDI(R->EAX());
//	R->EAX(pWave->Owner->GetWeapon(nWeapon));
//	return 0x75F39B;
//}

//DEFINE_HOOK(0x6B721F, SpawnManagerClass_Manage_Clear, 0x6)
//{
//	GET(SpawnManagerClass*, pThis, ESI);
//	pThis->Target = nullptr;
//	pThis->NewTarget = nullptr;
//	pThis->Status = SpawnManagerStatus::Idle;
//	return 0x0;
//}

DEFINE_HOOK(0x62A929, ParasiteClass_CanInfect_Additional, 0x6)
{
	enum
	{
		returnfalse = 0x62A976,
		continuecheck = 0x0,
		continuecheckB = 0x62A933
	};

	GET(FootClass*, pVictim, ESI);

	return pVictim->IsIronCurtained() || pVictim->IsBeingWarpedOut() ||
		TechnoExt::IsChronoDelayDamageImmune(pVictim) ? returnfalse : !pVictim->BunkerLinkedItem ? continuecheckB : returnfalse;
}

DEFINE_HOOK(0x6A78F6, SidebarClass_AI_RepairMode_ToggelPowerMode, 0x9)
{
	GET(SidebarClass*, pThis, ESI);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		pThis->SetTogglePowerMode(-1);
	else
		pThis->SetRepairMode(-1);

	return 0x6A78FF;
}

DEFINE_HOOK(0x6A7AE1, SidebarClass_AI_DisableRepairButton_TogglePowerMode, 0x6)
{
	GET(SidebarClass*, pThis, ESI);
	R->AL(Phobos::Config::TogglePowerInsteadOfRepair ? pThis->PowerToggleMode : pThis->RepairMode);
	return 0x6A7AE7;
}

DEFINE_HOOK(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
{
	enum { Continue = 0x0, DoNotDrawRadar = 0x70D407 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building) {
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1) {
			return DoNotDrawRadar;
		}
	}

	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsDummy ?
			DoNotDrawRadar : Continue;
}

DEFINE_HOOK(0x663225, RocketLocomotionClass_DetonateOnTarget_Anim, 0x6)
{
	GET(AnimClass*, pMem, EAX);
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CellStruct const, nCell, STACK_OFFS(0x60, 0x38));
	REF_STACK(CoordStruct const, nCoord, STACK_OFFS(0x60, 0x18));
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0x60, 0x50));

	GET(int, nDamage, EDI);

	const auto pCell = MapClass::Instance->GetCellAt(nCell);
	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, pCell ? pCell->LandType : LandType::Clear, nCoord))
	{
		GameConstruct(pMem, pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false);
		AnimExt::SetAnimOwnerHouseKind(pMem, pThis->LinkedTo->GetOwningHouse(), pThis->LinkedTo->Target ? pThis->LinkedTo->Target->GetOwningHouse() : nullptr, nullptr, false);
	}
	else
	{
		//no constructor called , so it is safe to delete the allocated memory
		GameDelete<false, false>(pMem);
	}

	return 0x66328C;
}

//DEFINE_HOOK(0x6FCF3E, TechnoClass_SetTarget_CheckAccessory , 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(AbstractClass*, pTarget, EDI);
//
//	if (auto pTargetFoot = generic_cast<FootClass*>(pTarget))
//	{
//		auto pExt = TechnoExt::ExtMap.Find(pTargetFoot);
//		if (pExt->Accesory)
//				pTarget = pExt->AccesoryOwner;
//	}
//
//	pThis->Target = pTarget;
//	return 0x6FCF44;
//}

//DEFINE_HOOK(0x70166E, TechnoClass_Captured, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	pThis->UpdatePlacement(PlacementType::Remove);
//	return 0x70167A;
//}

DEFINE_HOOK(0x6F09C4, TeamTypeClass_CreateOneOf_RemoveLog, 0x5)
{
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x8, -0x4));
	R->EDI(pHouse);
	return 0x6F09D5;
}

DEFINE_HOOK(0x6F0A3F, TeamTypeClass_CreateOneOf_CreateLog, 0x6)
{
	GET(TeamTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	Debug::Log("[%x][%s] Creating a new team named '%s'.\n", pHouse, pHouse ? pHouse->get_ID() : NONE_STR2, pThis->ID);
	R->EAX(YRMemory::Allocate(sizeof(TeamClass)));
	return 0x6F0A5A;
}

DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);
//DEFINE_HOOK(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5) { return 0x44DE3C; }

DEFINE_HOOK(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
{
	GET(FactoryClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	Debug::Log("[%x] Factory with Owner '%s' Abandoning production of '%s' \n", pThis, pThis->Owner ? pThis->Owner->get_ID() : NONE_STR2, pType->ID);
	R->ECX(pThis->Object);
	return 0x4CA021;
}

DEFINE_HOOK(0x4430F4, BuildingClass_Destroyed_SurvivourLog, 0x6)
{
	GET(BuildingClass*, pThis, EDI);
	GET(InfantryTypeClass*, pSurvivour, EAX);
	GET(BuildingTypeClass*, pThisType, EDX);
	Debug::Log("[%x][%s] Creating survivor type '%s' \n", pThis, pThisType->ID, pSurvivour->ID);
	return 0x443109;
}

DEFINE_HOOK(0x6E93BE, TeamClass_AI_TransportTargetLog, 0x5)
{
	GET(FootClass*, pThis, EDI);
	Debug::Log("[%x][%s] Transport just recieved orders to go home after unloading \n", pThis, pThis->get_ID());
	return 0x6E93D6;
}

DEFINE_HOOK(0x6EF9BD, TeamMissionClass_GatherAtEnemyCell_Log, 0x5)
{
	GET(int, nCellX, EAX);
	GET(int, nCellY, EDX);
	GET(TeamClass*, pThis, ESI);
	GET(TechnoClass*, pTechno, EDI);
	GET(TeamTypeClass*, pTeamType, ECX);
	Debug::Log("[%x][%s] Team with Owner '%s' has chosen ( %d , %d ) for its GatherAtEnemy cell.\n", pThis, pTeamType->ID, pTechno->Owner ? pTechno->Owner->get_ID() : NONE_STR2, nCellX, nCellY);
	return 0x6EF9D0;
}

DEFINE_JUMP(LJMP, 0x4495FF, 0x44961A);
//DEFINE_HOOK(0x4495FF, BuildingClass_ClearFactoryBib_Log1, 0xA) { return 0x44961A; }

DEFINE_JUMP(LJMP, 0x449657, 0x449672);
//DEFINE_HOOK(0x449657, BuildingClass_ClearFactoryBib_Log2, 0xA) { return 0x449672; }

#pragma region TSL_Test
//struct SHP_AlphaData
//{
//	int Width;
//	BYTE* CurPixel;
//	BYTE* CurData2;
//};
//
//void SetTSLData(DWORD TlsIdx, int nFrame, SHPReference* pAlplhaFile)
//{
//	auto nFrameBounds = pAlplhaFile->GetFrameBounds(nFrame);
//	auto nPixel = pAlplhaFile->GetPixels(nFrame);
//	SHP_AlphaData nData;
//	nData.Width = pAlplhaFile->Width;
//	nData.CurPixel = &nPixel[nFrameBounds.X + nFrameBounds.Y * pAlplhaFile->Width];
//	TlsSetValue(TlsIdx, &nData);
//}
//
//void GetTSLData_1(DWORD TlsIdx, void* pSomePtr, int nVal_y)
//{
//	if (TlsGetValue(TlsIdx))
//	{
//		if (pSomePtr)
//		{
//			if (int nX = (*(int*)((char*)pSomePtr + 0x14)))
//			{
//				if (int nY = (*(int*)((char*)pSomePtr + 0x18)))
//				{
//					int nWidth = 0;
//					if (nVal_y > 0)
//						nWidth = nVal_y;
//
//					(*(int*)((char*)pSomePtr + 0x10)) = nX + nY * nWidth;
//				}
//			}
//		}
//	}
//}
//
//void GetTSLData_2(DWORD TlsIdx, int nVala, int nValb)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData = pData->CurPixel)
//				pData->CurData2 = &nData[nVala * nWidth + nValb];
//		}
//	}
//}
//
//void GetTSLData_3(DWORD TlsIdx, int nVal)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData2 = pData->CurData2)
//				pData->CurData2 = &nData2[nVal * nWidth];
//		}
//	}
//}
//
//void GetTSLData_4(DWORD TlsIdx, void* pSomePtr)
//{
//	if (TlsGetValue(TlsIdx))
//	{
//		if (pSomePtr)
//		{
//			if (int nX = (*(int*)((char*)pSomePtr + 0x10)))
//			{
//				if (int nY = (*(int*)((char*)pSomePtr + 0x18)))
//				{
//					(*(int*)((char*)pSomePtr + 0x10)) = nX + nY;
//				}
//			}
//		}
//	}
//}
//
//void GetTSLData_5(DWORD TlsIdx, int nVal)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData2 = pData->CurData2)
//				pData->CurData2 = &nData2[nWidth];
//		}
//	}
//}
//
//void ClearTSLData(DWORD TlsIdx)
//{
//	TlsSetValue(TlsIdx, nullptr);
//}
//
//void InitTSLData(DWORD& TslIdx)
//{
//	TslIdx = TlsAlloc();
//	if (TslIdx == -1)
//		Debug::FatalErrorAndExit("Failed to Allocate TSL Index ! \n");
//}
#pragma endregion

//DEFINE_HOOK(0x73B5B5, UnitClass_DrawVoxel_AttachmentAdjust, 0x6)
//{
//	enum { Skip = 0x73B5CE };
//
//	GET(UnitClass*, pThis, EBP);
//	LEA_STACK(VoxelIndexKey *, pKey, STACK_OFFS(0x1C4, 0x1B0));
//	LEA_STACK(Matrix3D* , pMtx ,STACK_OFFS(0x1C4, 0xC0));
//	R->EAX(pThis->Locomotor.get()->Draw_Matrix(pMtx,pKey));
//	return Skip;
//}
//
//DEFINE_HOOK(0x73C864, UnitClass_drawcode_AttachmentAdjust, 0x6)
//{
//	enum { Skip = 0x73C87D };
//
//	GET(UnitClass*, pThis, EBP);
//	LEA_STACK(VoxelIndexKey *, pKey, STACK_OFFS(0x128, 0xC8));
//	LEA_STACK(Matrix3D* , pMtx ,STACK_OFFS(0x128, 0x30));
//	R->EAX(pThis->Locomotor.get()->Draw_Matrix(pMtx,pKey));
//	return Skip;
//}

#ifdef ENABLE_NEWHOOKS

#ifdef Eng_captureDelay
// TODO : more hooks

DEFINE_HOOK(0x4D69F5, FootClass_ApproachTarget_EngineerCaptureDelay, 0x6)
{
	GET(FootClass*, pFoot, EBX);

	if (auto pInf = specific_cast<InfantryClass*>(pFoot))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type);
		const auto pExt = TechnoExt::ExtMap.Find(pInf);

		if (pExt->EngineerCaptureDelay.InProgress())
		{
			return 0x4D6A01;
		}
	}

	return 0x0;
}

//set timer here ?
DEFINE_HOOK(0x5206B7, InfantryClass_FiringUpdate_EngCapture, 0x6)
{
	GET(InfantryClass*, pFoot, EBP);
}

DEFINE_HOOK(0x51EEED, InfantryClass_GetCursorOverObject_Infiltrate, 0x7)
{
	GET(InfantryClass*, pFoot, ESI);
	GET(BuildingClass*, pTarget, EDI);

	//BuildingType -> SpyEffect.Times
	// if pTarget -> is on -> BuildingTypeList of pFoot Agent.Allowed = allow ret nothing
	enum
	{
		DontAllow = 0x51F04E,
		Nothing = 0x0
	};

	return Nothing;
}

DEFINE_HOOK(0x51EE97, InfantryClass_WhatAction_Capturable, 0x6)
{
	enum
	{
		Skip = 0x51F04E,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	return Nothing;
}

DEFINE_HOOK(0x51E5AB, InfantryClass_WhatAction_Capturable2, 0x6)
{
	enum
	{
		Skip = 0x51E668,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	return Nothing;
}

DEFINE_HOOK(0x51E6BA, InfantryClass_WhatAction_EngAttack, 0x6)
{
	enum
	{
		Skip = 0x51EB15,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	return Nothing;
}
#endif

//ToDo : building check ?
// What : Attacked -> got 2 states -> spawn OnFireAnim from randomized idx -> Scatter coord -> create anim
/*struct nTempBelow
{

		ValueableVector<AnimTypeClass*> Rules_OnFire_Aircraft;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Infantry;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Unit;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Building;

	ValueableVector<AnimTypeClass*> OnFire_Aircraft;
	ValueableVector<AnimTypeClass*> OnFire_Infantry;
	ValueableVector<AnimTypeClass*> OnFire_Unit;
	ValueableVector<AnimTypeClass*> OnFire_Building;


};
DEFINE_HOOK(0x4D7431, FootClass_ReceiveDamage_OnFire, 0x5)
{
	GET(DamageState, nState, EAX);

	if (nState == DamageState::NowYellow || nState == DamageState::NowRed)
	{
		GET_STACK(TechnoClass*, pAttacker, STACK_OFFS(0x1C, -0x10));
		GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x1C, -0x18));
		GET(FootClass*, pThis, ESI);
		GET(WarheadTypeClass*, pWH, EBP);

		if (pWH->Sparky)
		{
			Iterator<AnimTypeClass*> nVec;
			switch (pThis->WhatAmI())
			{
			default:
				break;
			}

		}

	}
	return 0x0;
}*/
//DEFINE_HOOK(0x4FD8F7, HouseClass_FireSale_UnitCheck, 0x6)
//{
//	GET(HouseClass*, pThis, EBX);
//
//	if (!Unsorted::ShortGame || pThis->OwnedUnits - pThis->OwnedInfantry <= 10)
//		return 0x0;
//
//	pThis->All_To_Hunt();
//	return 0x4FD907;
//}
//

//DEFINE_HOOK(0x519B58, InfantryClass_PerCellProcess_AutoSellCheck, 0x6)
//{
//	enum
//	{
//		Nothing = 0x0,
//		Skip = 0x51A03E
//	};
//
//	GET(InfantryClass*, pThis, ESI);
//	GET(TechnoClass*, pTarget, EDI);
//
//	return Nothing;
//}
//
//DEFINE_HOOK(0x51A002, InfantryClass_PerCellProcess_EventCheck, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	GET(TechnoClass*, pTarget, EDI);
//
//	return 0x0;
//}

//struct DummyBtypeExt
//{
//	static ValueableVector<BuildingTypeClass*> BuildingAdjentBaseOn;
//};
//
//DEFINE_HOOK(0x4A8FEC , MouseClass_CanPlaceHere_SpecifiedBase , 0x7)
//{
//	GET(BuildingTypeClass*, pBuildingType, EDX);
//
//	auto Display_PendingObj = Make_Global<BuildingTypeClass*>(0x880990);
//
//	if (!Display_PendingObj || Display_PendingObj->WhatAmI()  != AbstractType::BuildingType)
//		return 0x0;
//
//	return DummyBtypeExt::BuildingAdjentBaseOn.empty() || DummyBtypeExt::BuildingAdjentBaseOn.Contains(pBuildingType) ? 0x0 : 0x4A8FFA;
//}


DEFINE_HOOK(0x452831, BuildingClass_Overpowerer_AddUnique, 0x6)
{
	enum
	{
		AddItem = 0x45283C,
		Skip = 0x45289C
	};

	GET(const DynamicVectorClass<TechnoClass*>*, pVec, ECX);
	GET(TechnoClass* const, pOverpowerer, ESI);

	return pVec->FindItemIndex(pOverpowerer) == -1 ? AddItem : Skip;
}

/*
DEFINE_HOOK(0x452831, BuildingClass_UpdateOverpowerState, 0x6)
{
	GET(const BuildingClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);

	return pThis->SelectWeapon(pTarget) == -1 ?
		0x45283C : 0x45289C;
}*/


//#include <Ext/WeaponType/Body.h>
//
//crash ?
//DEFINE_HOOK(0x6FCA30 , TechnoClass_GetFireError_DecloakToFire, 0x6)
//{
//	enum
//	{
//		FireErrorCloaked = 0x6FCA4F,
//		ContinueCheck = 0x6FCA5E
//	};
//
//	GET(const TechnoClass*, pThis, ESI);
//	GET(const WeaponTypeClass*, pWeapon, EBX);
//
//	if(const auto pTransport = pThis->Transporter)
//		if(pTransport->CloakState == CloakState::Cloaked) return FireErrorCloaked;
//
//	if (pThis->CloakState == CloakState::Uncloaked)
//		return ContinueCheck;
//
//	const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);
//	if (pExt && pExt->Decloak_InstantFire.isset())
//		if(!pExt->Decloak_InstantFire.Get() && pThis->WhatAmI() != AbstractType::Aircraft)
//			return FireErrorCloaked;
//
//	return (pThis->CloakState == CloakState::Cloaked) ? FireErrorCloaked : ContinueCheck;
//}

DEFINE_HOOK(0x746CD0, UnitClass_SelectWeapon_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->Type;
	R->EAX((pThis->Deployed && pType->DeployFire && pType->DeployFireWeapon != -1) ?
		pType->DeployFireWeapon : pThis->TechnoClass::SelectWeapon(pTarget));

	return 0x746CFD;
}

//struct WeaponWeight
//{
//	short index;
//	bool InRange;
//	float DPF;
//	bool operator < (const WeaponWeight& RHS) const
//	{
//		return (this->InRange < RHS.InRange&& this->DPF < RHS.DPF);
//	}
//};
//
//static float EvalVersesAgainst(TechnoClass* pThis, ObjectClass* pTarget, WeaponTypeClass* W)
//{
//	Armor nArmor = pTarget->GetType()->Armor;
//	if (const auto pTargetTech = generic_cast<TechnoClass*>(pTarget))
//		if (auto const pExt = TechnoExt::ExtMap.Find(pTargetTech))
//			if (auto const pShield = pExt->GetShield())
//				if (pShield->IsActive() && pExt->CurrentShieldType)
//					nArmor = pShield->GetArmor();
//
//	const double Verses = GeneralUtils::GetWarheadVersusArmor(W->Warhead, nArmor);
//	return (W->Damage * TechnoExt::GetDamageMult(pThis)) * Verses / (W->ROF * Helpers_DP::GetROFMult(pThis));
//}
//
//static bool EvalWeaponAgainst(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* W)
//{
//	if (!W || W->NeverUse || !pTarget) { return 0; }
//
//	WarheadTypeClass* WH = W->Warhead;
//	if (!WH) { return 0; }
//
//	if (!pTarget->AsTechno())
//		return 0;
//
//	TechnoTypeClass* pTargetT = ((TechnoClass*)pTarget)->GetTechnoType();
//
//	if (WH->Airstrike)
//	{
//		if (pTarget->WhatAmI() != AbstractType::Building) {
//			return 0;
//		}
//
//		BuildingTypeClass* pBT = ((BuildingClass*)pTarget)->Type;
//		// not my design, leave me alone
//		return pBT->CanC4 && (!pBT->ResourceDestination || !pBT->ResourceGatherer);
//	}
//
//	if (WH->IsLocomotor) {
//		return (pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;
//	}
//
//	if (W->DrainWeapon) {
//		return pTargetT->Drainable && !pThis->DrainTarget && !pThis->Owner->IsAlliedWith(pTarget);
//	}
//
//	if (W->AreaFire) {
//		return pThis->GetCurrentMission() == Mission::Unload;
//	}
//
//	if (pTarget->WhatAmI() == AbstractType::Building && ((BuildingClass*)pTarget)->Type->Overpowerable) {
//		return WH->ElectricAssault && pThis->Owner->CanOverpower(((TechnoClass*)pTarget));
//	}
//
//	if (pTarget->IsInAir() && !W->Projectile->AA) {
//		return 0;
//	}
//
//	if (pTarget->IsOnFloor() && !W->Projectile->AG) {
//		return 0;
//	}
//
//	return 1;
//}
//
//static int EvalDistanceAndVerses(TechnoClass* pThis, ObjectClass* pTarget)
//{
//	auto const pType = pThis->GetTechnoType();
//
//	int WCount = 2;
//	if (pType->WeaponCount > 0) {
//		WCount = pType->WeaponCount;
//	}
//
//	std::vector<WeaponWeight> Weights(WCount);
//
//	for (short i = 0; i < WCount; ++i)
//	{
//		WeaponTypeClass* W = pThis->GetWeapon(i)->WeaponType;
//		Weights[i].index = i;
//		if (W)
//		{
//			CoordStruct xyz1 = pThis->GetCoords();
//			CoordStruct xyz2 = pTarget->GetCoords();
//			float distance = abs(xyz1.DistanceFrom(xyz2));
//			bool CloseEnough = distance <= W->Range && distance >= W->MinimumRange;
//			Weights[i].DPF = EvalVersesAgainst(pThis, pTarget, W);
//			Weights[i].InRange = CloseEnough;
//		}
//		else
//		{
//			Weights[i].DPF = 0.0;
//			Weights[i].InRange = 0;
//		}
//	}
//	std::stable_sort(Weights.begin(), Weights.end());
//	std::reverse(Weights.begin(), Weights.end());
//	return Weights[0].index;
//}
//
//static int SelectWeaponAgainst(TechnoClass* pThis, AbstractClass* pTarget)
//{
//	int Index = 0;
//	if (!pTarget)
//		return 0;
//
//	const TechnoTypeClass* pThisT = pThis->GetTechnoType();
//
//	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThisT)) {
//		if (pTypeExt->Interceptor.Get() && pTarget->WhatAmI() == AbstractType::Bullet) {
//			return(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
//		}
//	}
//
//	//WeaponStruct* W1 = pThis->GetWeapon(0);
//	//WeaponTypeClass* W1T = W1->WeaponType;
//	WeaponStruct* W2 = pThis->GetWeapon(1);
//	WeaponTypeClass* W2T = W2->WeaponType;
//
//	if (pThisT->HasMultipleTurrets() && !pThisT->IsGattling) {
//		return pThis->CurrentWeaponNumber;
//	}
//
//	if (pThis->CanOccupyFire()) {
//		return 0;
//	}
//
//	if (pThis->InOpenToppedTransport) {
//		Index = pThisT->OpenTransportWeapon;
//		if (Index != -1) {
//			return Index;
//		}
//	}
//
//	if (pThisT->IsGattling) {
//		int CurrentStage = pThis->CurrentGattlingStage * 2;
//		if (pTarget->AbstractFlags & AbstractFlags::Techno && pTarget->IsInAir()) {
//			if (W2T && W2T->Projectile->AA) {
//				return CurrentStage + 1;
//			}
//		}
//		return CurrentStage;
//	}
//
//	if (pThis->WhatAmI() == AbstractType::Building && ((BuildingClass*)pThis)->IsOverpowered) {
//		return 1;
//	}
//
//	if (pTarget->WhatAmI() == AbstractType::Aircraft && ((AircraftClass*)pTarget)->IsCrashing) {
//		return 1;
//	}
//
//	// haaaaaaaate
//	if (pTarget->WhatAmI() == AbstractType::Cell) {
//		CellClass* pTargetCell = (CellClass*)pTarget;
//		if (
//
//			(pTargetCell->LandType != LandType::Water && pTargetCell->IsOnFloor())
//			|| ((pTargetCell->ContainsBridge()) && pThisT->Naval)
//
//			&& (!pTargetCell->IsInAir() && pThisT->LandTargeting == LandTargetingType::Land_secondary)
//
//		) {
//			return 1;
//		}
//	}
//
//	auto const pTechnShit = generic_cast<ObjectClass*>(pTarget);
//	const LandType ltTgt = pTechnShit->GetCell()->LandType;
//	bool InWater = !pTechnShit->OnBridge && !pTarget->IsInAir() && (ltTgt == LandType::Water || ltTgt == LandType::Beach);
//
//	if (InWater)
//	{
//		Index = (int)pThis->SelectNavalTargeting(pTarget);
//		if (Index != -1) {
//			return Index;
//		}
//		else {
//			return 0; // what?
//		}
//	}
//
//	if (!pTarget->IsInAir() && pThisT->LandTargeting == LandTargetingType::Land_secondary) {
//		return 1;
//	}
//
//	return EvalDistanceAndVerses(pThis, pTechnShit);
//}
//
//static int HandleSelectWeapon(TechnoClass* pThis, AbstractClass* pTarget)
//{
//	//const TechnoTypeClass* pThisT = pThis->GetTechnoType();
//	//if(pThisT->Spawns)
//	return SelectWeaponAgainst(pThis,pTarget);
//}

/*
//complete replacement ?
DEFINE_HOOK(0x6F3330, TechnoClass_SelectWeapon, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const int v5 = HandleSelectWeapon(pThis, pTarget);
	if (pTarget == pThis->Target)
		pThis->CurrentWeaponNumber = v5;


	R->EAX(v5);
	return 0x6F3813;
}
*/


#pragma region ES_Stuffs



//#include <CCToolTip.h>
//
//DEFINE_HOOK(0x77778D, YRWindoProc_FixToolTip1, 0x5)
//{
//	GET(HWND, nHw, EDI);
//
//	//CCToolTip::Instance
//	GameConstruct<CCToolTip>(CCToolTip::Instance, nHw);
//	CCToolTip::Instance->Delay = 7;
//
//	return 0x7779B5;
//}
//
//DEFINE_HOOK(0x7777F3, YRWindoProc_FixToolTip2, 6) {
//	GameConstruct<ToolTipManager>(CCToolTip::Instance, nullptr);
//	return 0x777809;
//}

//
//DEFINE_HOOK(0x6276A6, LoadPaletteFiles_LoadFile_Debug, 0x5) {
//	GET(void*, pFile, EAX);
//	LEA_STACK(char*, pFilename, STACK_OFFS(0x424, 0x400));
//
//	if (pFile == R->EBX<void*>()) {
//		Debug::Log("Could not find palette file %s \n", pFilename);
//		return 0x6276AA;
//	}
//
//	return 0x6276B9;
//}

#ifdef ES_ExpDamageHook
static void Detonate(TechnoClass* pTarget, HouseClass* pOwner, CoordStruct const& nCoord)
{

}

//TODO , check stack , make this working
DEFINE_HOOK(0x489A97, ExplosionDamage_DetonateOnEachTarget, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(ObjectClass*, pTarget, ESI);
	GET_BASE(HouseClass*, pHouse, 0x14);
	GET_BASE(const CoordStruct*, pCoords, 0x8);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		if (auto pTechnp = generic_cast<TechnoClass*>(pTarget))
			pWHExt->Detonate(pThis->Owner, pHouse, pThis, *pCoords);
	}

	return 0x0;
}
#endif


#endif

DEFINE_HOOK(0x5F54A8, ObjectClass_ReceiveDamage_ConditionYellow, 0x6)
{
	enum { ContinueCheck = 0x5F54C4, ResultHalf = 0x5F54B8 };

	GET(int, nCurStr, EDX);
	GET(int, nMaxStr, EBP);
	GET(int, nDamage, ECX);

	const auto curstr = Game::F2I(nMaxStr * RulesClass::Instance->ConditionYellow);
	return (nCurStr >= curstr && (nCurStr - nDamage) < curstr) ? ResultHalf : ContinueCheck;
}

//DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_RememberAttacker, 0x6)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0xB4,0x4));
//
//	if (auto pTechnoTarget = generic_cast<TechnoClass*>(pTarget))
//	{
//		auto pTargetTExt = TechnoExt::ExtMap.Find(pTechnoTarget);
//		pTargetTExt->LastAttacker = pThis;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x73C6F5, UnitClass_DrawAsSHP_StandFrame, 0x9)
//{
//	GET(UnitTypeClass*, pType, ECX);
//	GET(UnitClass*, pThis, EBP);
//	GET(int, nFrame, EBX);
//
//	auto nStand = pType->StandingFrames;
//	auto nStandStartFrame = pType->StartFiringFrame + nStand * nFrame;
//	if (pType->IdleRate > 0)
//		nStandStartFrame += pThis->WalkedFramesSoFar;
//
//	R->EBX(nStandStartFrame);
//
//	return 0x73C725;
//}

//DEFINE_HOOK(0x529A14, INIClass_ReadPoint2D_CheckDefault, 0x5)
//{
//	GET(Point2D*, pDefault, ECX);
//
//	if (!pDefault)
//		R->EDX(Point2D::Empty);
//	else
//		return 0x0;
//
//	return 0x529A16;
//
//}

////this literally replace everything , omegalul
//DEFINE_HOOK(0x7353C0, UnitClass_CTOR_ReplaceType, 0x7)
//{
//	//GET(UnitClass*, pThis, ECX);
//	GET_STACK(UnitTypeClass*, pSetType, 0x4);
//	GET_STACK(HouseClass*, pOwner, 0x8);
//
//	UnitTypeClass* pDecided = pSetType;
//
//	if (pSetType && pOwner && !pOwner->ControlledByPlayer()) {
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pSetType);
//
//		if (pTypeExt->Unit_AI_AlternateType.isset()) {
//			auto const pNewType = pTypeExt->Unit_AI_AlternateType.Get(pSetType);
//			Debug::Log("Replacing CTORed [%s] House AI Unit [%s] to [%s] ! \n", pOwner->get_ID() , pSetType->ID, pNewType->ID);
//			pDecided = pNewType;
//		}
//	}
//
//	R->Stack(0x4, pDecided);
//	return 0x7353EC;
//}

DEFINE_HOOK(0x6D912B, TacticalClass_Render_BuildingInLimboDeliveryA, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D9159 };

	GET(TechnoClass*, pTechno, ESI);

	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pTechno))->LimboID != -1)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

DEFINE_HOOK(0x6D966A, TacticalClass_Render_BuildingInLimboDeliveryB, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D978F };

	GET(TechnoClass*, pTechno, EBX);

	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pTechno))->LimboID != -1)
		{
			return DoNotDraw;
		}
	}

	return Draw;
}

DEFINE_HOOK(0x6D9466, TacticalClass_Render_BuildingInLimboDeliveryC, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D9587 };

	GET(BuildingClass*, pBuilding, EBX);
	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ? DoNotDraw : Draw;
}

DEFINE_HOOK(0x737F86, UnitClass_ReceiveDamage_DoBeforeAres, 0x6)
{
	GET(UnitTypeClass*, pType, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFS(0x44, -0x18));

	if (!ignoreDefenses)
		return 0x0;

	if (pType->OpenTopped)
	{
		pThis->MarkPassengersAsExited();
	}

	if (!preventPassangersEscape)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		// passenger escape chances
		auto const passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

		// eject or kill all passengers
		while (pThis->Passengers.GetFirstPassenger())
		{
			auto const pPassenger = pThis->RemoveFirstPassenger();
			bool trySpawn = false;
			if (passengerChance > 0)
			{
				trySpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= passengerChance;
			}
			else if (passengerChance == -1 && pThis->WhatAmI() == UnitClass::AbsID)
			{
				Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}
			if (trySpawn && TechnoExt::EjectRandomly(pPassenger, pThis->Location, 128, select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			TechnoExt::HandleRemove(pPassenger, pKiller, true);
		}
	}

	return 0x737F97;
}

DEFINE_HOOK(0x41668B, AircraftClass_ReceiveDamage_DoBeforeAres, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pKiller, 0x28);
	GET_STACK(int, ignoreDefenses, 0x20);
	GET_STACK(bool, preventPassangersEscape, STACK_OFFS(0x14, -0x18));

	if (!ignoreDefenses)
		return 0x0;

	if (!preventPassangersEscape)
	{
		bool select = pThis->IsSelected && pThis->Owner->ControlledByPlayer();
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		// passenger escape chances
		auto const passengerChance = pTypeExt->Survivors_PassengerChance.Get(pThis);

		// eject or kill all passengers
		while (pThis->Passengers.GetFirstPassenger())
		{
			auto const pPassenger = pThis->RemoveFirstPassenger();
			bool trySpawn = false;

			if (passengerChance > 0)
			{
				trySpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= passengerChance;
			}
			else if (passengerChance == -1 && pThis->WhatAmI() == UnitClass::AbsID)
			{
				Move occupation = pPassenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
				trySpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
			}
			if (trySpawn && TechnoExt::EjectRandomly(pPassenger, pThis->Location, 128, select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			TechnoExt::HandleRemove(pPassenger, pKiller, true);
		}
	}

	return 0x0;
}

static int AnimClass_Expired_SpawnsParticle(REGISTERS* R)
{
	GET(AnimClass*, pThis, ESI);
	GET(AnimTypeClass*, pAnimType, EAX);
	GET(int, nNumParticles, ECX);

	const auto pType = ParticleTypeClass::Array->Items[pAnimType->SpawnsParticle];
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pAnimType);
	//should be lepton ?
	const auto nMin = static_cast<int>(pTypeExt->ParticleRangeMin);
	const auto nMax = static_cast<int>(pTypeExt->ParticleRangeMax);

	if (nMin || nMax)
	{
		const auto nCoord = pThis->GetCoords();
		const auto v8 = nCoord.Z - MapClass::Instance->GetCellFloorHeight(nCoord);
		const auto v17 = 6.283185307179586 / nNumParticles;
		double v16 = 0.0;

		if (nNumParticles > 0)
		{
			for (; nNumParticles; --nNumParticles)
			{
				auto v13 = abs(ScenarioClass::Instance->Random.RandomRanged(nMin, nMax));
				auto v10 = ScenarioClass::Instance->Random.RandomDouble() * v17 + v16;
				auto v18 = std::cos(v10);
				auto v9 = std::sin(v10);
				CoordStruct nCoordB { nCoord.X + static_cast<int>(v13 * v18),nCoord.Y - static_cast<int>(v9 * v13), nCoord.Z };
				nCoordB.Z = v8 + MapClass::Instance->GetCellFloorHeight(nCoordB);
				ParticleSystemClass::Instance->SpawnParticle(pType, &nCoordB);
				v16 += v17;
			}
		}
	}

	return 0x42504D;
}

DEFINE_HOOK(0x447E90, BuildingClass_GetDestinationCoord_Helipad, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pCoord, 0x4);
	GET_STACK(TechnoClass*, pDocker, 0x8);

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

//DEFINE_JUMP(LJMP, 0x517FF5, 0x518016);

DEFINE_HOOK(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x8)
{
	GET(UnitClass*, pThis, EBP);
	GET(AnimClass*, pAllocatedMem, ESI);
	GET(LandType const, nLand, EBX);

	auto nLoc = pThis->Location;
	if (auto const pC4AnimType = MapClass::SelectDamageAnimation(pThis->Health, RulesClass::Instance->C4Warhead, nLand, nLoc))
	{
		GameConstruct(pAllocatedMem, pC4AnimType, nLoc, 0, 1, 0x2600, -15, false);
		AnimExt::SetAnimOwnerHouseKind(pAllocatedMem, nullptr, pThis->GetOwningHouse(), true);

	}
	else
	{
		GameDelete<false, false>(pAllocatedMem);
	}

	return 0x73AF4C;
}

void DrawTiberiumPip(TechnoClass* pTechno, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
{
	const auto pType = pTechno->GetTechnoType();
	const auto nMax = pType->GetPipMax();

	if (!nMax)
		return;

	const auto nStorage_0 = pTechno->Tiberium.Tiberiums[0]; //Riparius
	const auto nStorage_1 = pTechno->Tiberium.Tiberiums[1]; //Cruentus
	const auto nStorage_2 = pTechno->Tiberium.Tiberiums[2]; //Vinifera
	const auto nStorage_3 = pTechno->Tiberium.Tiberiums[3]; //Aboreus
	auto const nStorage = pType->Storage;

	auto amount_Riparious = Game::F2I(nStorage_0 / nStorage * nMax + 0.5);
	auto amount_Cruentus = Game::F2I(nStorage_1 / nStorage * nMax + 0.5);
	auto amount_Vinifera = Game::F2I(nStorage_2 / nStorage * nMax + 0.5);
	auto amount_Aboreus = Game::F2I(nStorage_3 / nStorage * nMax + 0.5);

	//const auto totalaccum = nStorage_3 + nStorage_2 + nStorage_0;
	//auto amount_a = Game::F2I(totalaccum / pType->Storage * nMax + 0.5);
	//auto amount_b = Game::F2I(nStorage_1 / pType->Storage * nMax + 0.5);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	Point2D nOffs {};
	auto const pBuilding = pTechno->WhatAmI() == AbstractType::Building ? static_cast<BuildingClass*>(pTechno) : nullptr;
	auto const pShape = pBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS2_SHP());

	ConvertClass* nPal = nullptr;

	if (pBuilding)
	{
		auto const pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuildingTypeExt->PipShapes01Remap)
			nPal = pTechno->GetRemapColour();
		else
			nPal = pBuildingTypeExt->PipShapes01Palette.GetOrDefaultConvert(FileSystem::THEATER_PAL());
	}
	else
	{
		nPal = FileSystem::THEATER_PAL();
	}

	auto GetFrames = [&]()
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (amount_Riparious > 0)
		{
			--amount_Riparious;
			return pTypeExt->Riparius_FrameIDx.Get(2);
		}

		if (amount_Cruentus > 0)
		{
			--amount_Cruentus;
			return pTypeExt->Cruentus_FrameIDx.Get(5);
		}

		if (amount_Vinifera > 0)
		{
			--amount_Vinifera;
			return pTypeExt->Vinifera_FrameIDx.Get(2);
		}

		if (amount_Aboreus > 0)
		{
			--amount_Aboreus;
			return pTypeExt->Aboreus_FrameIDx.Get(2);
		}

		return 0;
	};

	for (int i = nMax; i; --i)
	{
		int nFrame = GetFrames();
		if (nFrame > pShape->Frames)
			nFrame = pShape->Frames;

		Point2D nPointHere { nOffs.X + nPoints->X  , nOffs.Y + nPoints->Y };
		CC_Draw_Shape(
			DSurface::Temp(),
			nPal,
			pShape,
			nFrame,
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

//TODO : dynamic array ?
//void DrawCargoPips_NotBySize(TechnoClass* pThis, int nMaxPip, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
//{
//	if (!nMaxPip)
//		return;
//
//	int nTotal_size = pThis->Passengers.GetTotalSize();
//	int nCur_size = pThis->Passengers.NumPassengers;
//	auto const pBld = specific_cast<BuildingClass*>(pThis);
//	auto const pShape = pBld ? FileSystem::PIPS_SHP() : FileSystem::PIPS2_SHP();
//	auto nPipState = std::make_unique<int>(sizeof(int) * nMaxPip);
//	std::memset(nPipState.get(), 0, sizeof(int) * nMaxPip); //zero the array
//
//	if (pBld && pBld->Absorber())
//	{
//		if (nCur_size > 0)
//		{
//			auto pPassengers = pThis->Passengers.GetFirstPassenger();
//
//			for (int i = 0; i < nCur_size; ++i)
//			{
//
//				if (!pPassengers)
//					break;
//
//				switch (pPassengers->WhatAmI())
//				{
//				case AbstractType::Unit:
//					nPipState.get()[i] = (int)(static_cast<InfantryClass*>(pPassengers)->Type->Pip);
//					break;
//				case AbstractType::Infantry:
//					nPipState.get()[i] = (int)PipIndex::Red;
//					break;
//				default:
//					nPipState.get()[i] = (int)PipIndex::White;
//					break;
//				}
//
//				pPassengers = static_cast<FootClass*>(pPassengers->NextObject);
//			}
//		}
//	}
//	else
//	{
//		if (nTotal_size >= nMaxPip)
//		{
//			nPipState.reset();
//			return;
//		}
//
//		auto pPassengers = pThis->Passengers.GetFirstPassenger();
//
//		for (int i = 0; i < nCur_size; ++i)
//		{
//
//			if (!pPassengers)
//				break;
//
//			switch (pPassengers->WhatAmI())
//			{
//			case AbstractType::Unit:
//			{
//				int nSize = Game::F2I(pPassengers->GetTechnoType()->SizeLimit - 1.0);
//				if (nSize > 0)
//				{
//					auto nArr = &nPipState.get()[nTotal_size];
//					nTotal_size -= nSize;
//					do
//					{
//						*nArr-- = (int)PipIndex::White;
//					}
//					while (--nSize);
//				}
//				nPipState.get()[nTotal_size] = (int)PipIndex::Blue;
//			} break;
//			case AbstractType::Infantry:
//			{
//				int nSize = Game::F2I(pPassengers->GetTechnoType()->SizeLimit - 1.0);
//				if (nSize > 0)
//				{
//					auto nArr = &nPipState.get()[nTotal_size];
//					nTotal_size -= nSize;
//					do
//					{
//						*nArr-- = (int)PipIndex::White;
//					}
//					while (--nSize);
//				}
//				nPipState.get()[nTotal_size] = (int)(static_cast<InfantryClass*>(pPassengers)->Type->Pip);
//			}
//			break;
//			default:
//				nPipState.get()[nTotal_size] = (int)PipIndex::Green;
//				break;
//			}
//
//			pPassengers = static_cast<FootClass*>(pPassengers->NextObject);
//			--nTotal_size;
//		}
//	}
//
//	Point2D nOffs {};
//	Point2D nPointHere {};
//
//	if (pThis->GetTechnoType()->Gunner)
//	{
//		CC_Draw_Shape(
//			DSurface::Temp(),
//			FileSystem::THEATER_PAL(),
//			pShape,
//			*nPipState.get(),
//			nPoints,
//			pRect,
//			0x600,
//			0,
//			0,
//			0,
//			1000,
//			0,
//			0,
//			0,
//			0,
//			0);
//
//		nOffs.X = 1;
//		nOffs.Y = 1;
//		nPointHere.Y = 2 * nOffsetY;
//		nPointHere.X = 2 * nOffsetX;
//
//	}
//
//	Point2D nOffsB {};
//
//	if (nOffs.X < nMaxPip)
//	{
//		int nYOffsHere = nOffsetY * nOffs.Y;
//		int nXOffsHere = nOffsetX * nOffs.X;
//		auto nArr = &nPipState.get()[nOffs.Y];
//		int nMaxPipHere = nMaxPip - nOffs.Y;
//
//		do
//		{
//			Point2D nPointHereB { nXOffsHere + nPointHere.X + nPoints->X ,nYOffsHere + nPoints->Y + nPointHere.Y };
//			CC_Draw_Shape(
//			DSurface::Temp(),
//			FileSystem::THEATER_PAL(),
//			pShape,
//			*nArr,
//			&nPointHereB,
//			pRect,
//			0x600,
//			0,
//			0,
//			0,
//			1000,
//			0,
//			0,
//			0,
//			0,
//			0);
//			nYOffsHere += nOffsetY;
//			nXOffsHere += nOffsetX;
//			nArr++;
//			--nMaxPipHere;
//		}
//		while (!(nMaxPipHere == 1));
//	}
//}

void DrawSpawnerPip(TechnoClass* pTechno, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
{
	const auto pType = pTechno->GetTechnoType();
	const auto nMax = pType->SpawnsNumber;

	if (nMax <= 0)
		return;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	Point2D nOffs {};

	auto const pBuilding = specific_cast<BuildingClass*>(pTechno);
	auto const pShape = pBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS_SHP());

	ConvertClass* nPal = nullptr;

	if (pBuilding)
	{
		auto const pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuildingTypeExt->PipShapes01Remap)
			nPal = pTechno->GetRemapColour();
		else
			nPal = pBuildingTypeExt->PipShapes01Palette.GetOrDefaultConvert(FileSystem::THEATER_PAL());
	}
	else
	{
		nPal = FileSystem::THEATER_PAL();
	}

	for (int i = 0; i < nMax; i++)
	{
		const auto nSpawnMax = pTechno->SpawnManager->CountDockedSpawns();

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

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(PipDataStruct, pDatas, STACK_OFFS(0x74, 0x24));
	//GET_STACK(SHPStruct*, pShapes, STACK_OFFS(0x74, 0x68));
	GET_STACK(RectangleStruct*, pBound, STACK_OFFSET(0x74, 0xC));
	GET(int, nOffsetY, ESI);

	DrawTiberiumPip(pThis, &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	//DrawCargoPips_NotBySize(pThis, pThis->GetTechnoType()->GetPipMax(), &pDatas.nPos, pBound, pDatas.Int, nOffsetY);
	return 0x70A340;
}

//DEFINE_HOOK(0x4870D0, CellClass_SensedByHouses_ObserverAlwaysSensed, 0x6)
//{
//	GET_STACK(int, nHouseIdx, 0x4);
//
//	const auto pHouse = HouseClass::Array->GetItemOrDefault(nHouseIdx);
//	if (HouseExt::IsObserverPlayer(pHouse))
//	{
//		R->AL(1);
//		return 0x4870DE;
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x70DA6D, TechnoClass_SensorAI_ObserverSkipWarn, 0x6)
//{
//	return HouseExt::IsObserverPlayer() ? 0x70DADC : 0x0;
//}

//DEFINE_HOOK(0x6FF1FB, TechnoClass_FireAt_Shield, 0x6)
//{
//	GET_BASE(AbstractClass*, pTarget, 0x8);
//	GET_STACK(CoordStruct, nCoord, 0x44);
//
//	if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
//	{
//		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
//		if (auto const pShield = pExt->GetShield())
//		{
//			if (!pShield->IsActive())
//				return 0x0;
//
//			if(auto pHitAnim = pShield->Anim)
//		}
//	}
//	return 0x0;
//}

//enum class NewVHPScan : int
//{
//	None = 0 ,
//	Normal = 1 ,
//	Strong = 2,
//	Threat = 3,
//	Health = 4,
//	Damage = 5,
//	Value = 6,
//	Locked = 7,
//	Non_Infantry = 8,
//
//
//	count
//};
//
//std::array<const char*, (size_t)NewVHPScan::count> NewVHPScanToString
//{
//   {
//	  {"None" },
//	{ "Normal" },
//	{ "Strong" },
//	{ "Threat" },
//	{ "Health" },
//	{ "Damage" },
//	{ "Value" },
//	{ "Locked" },
//	{ "Non-Infantry" } 
//	}
//};

//DEFINE_HOOK(0x4775F4, CCINIClass_ReadVHPScan_new, 0x5)
//{
//	GET(const char* const, cur, ESI);
//
//	int vHp = 0;
//	for (int i = 0; i < (int)NewVHPScanToString.size(); ++i)
//	{
//		if (IS_SAME_STR_(cur, NewVHPScanToString[i]))
//		{
//			vHp = i;
//			break;
//		}
//	}
//
//	R->EAX(vHp);
//	return 0x4775E9;
//}
//
//DEFINE_HOOK(0x4775B0, CCINIClass_ReadVHPScan_ReplaceArray, 0x7)
//{
//	R->EDX(NewVHPScanToString[R->EDI<int>()]);
//	return 0x4775B7;
//}

//NOINLINE int* AllocArray()
//{
//	return new int[SuperWeaponTypeClass::Array->Count];
//}

//DEFINE_HOOK(0x6F8721, TechnoClass_EvalObject_VHPScan, 0x7)
//{
//	GET(TechnoClass*, pThis, EDI);
//	GET(ObjectClass*, pTarget, ESI);
//	GET(int*, pRiskValue, EBP);
//
//	auto const pTechnoTarget = generic_cast<TechnoClass*>(pTarget);
//
//	
//	return 0x6F875F;
//}

DEFINE_HOOK(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7)
{
	GET(InfantryClass*, pThis, ECX);

	enum { SkipWholeFunction = 0x5192BC, Continue = 0x0 };

	if (!pThis)
		return Continue;

	auto const pTypeExt = InfantryTypeExt::ExtMap.Find(pThis->Type);
	return pTypeExt->HideWhenDeployAnimPresent.Get() && pThis->DeployAnim
		? SkipWholeFunction : Continue;
}

DEFINE_HOOK(0x6F7261, TechnoClass_TargetingInRange_NavalBonus, 0x5)
{
	GET(int, nRangeBonus, EDI);
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);

	if (auto const pFoot = abstract_cast<FootClass*>(pTarget))
	{
		auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		if (pFoot->GetTechnoType()->Naval && pFoot->GetCell()->LandType == LandType::Water)
			nRangeBonus += pThisTypeExt->NavalRangeBonus.Get();
	}

	R->EDI(nRangeBonus);
	return 0x0;
}


// Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250);

DEFINE_HOOK(0x47257C, CaptureManagerClass_TeamChooseAction_Random, 0x6)
{
	GET(FootClass*, pFoot, EAX);

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
	TechnoExt::ApplyDrainMoney(pThis);
	return 0x6FA1C5;
}

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D90 };

	GET(ObjectClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, STACK_OFFSET(0x8, -0x4));
	R->AL(TechnoExt::IsCrushable(pThis, pTechno));

	return SkipGameCode;
}

#include <New/Type/ArmorTypeClass.h>

void NOINLINE ApplyHitAnim(ObjectClass* pTarget, args_ReceiveDamage* args)
{
	if (Unsorted::CurrentFrame % 15 != 0)
		return;

	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(args->WH);
	auto const pTechno = generic_cast<TechnoClass*>(pTarget);
	auto const pType = pTarget->GetType();
	auto const bIgnoreDefense = args->IgnoreDefenses;
	bool bImmune_pt2 = false;
	bool const bImmune_pt1 =
		(pTarget->IsIronCurtained() && !bIgnoreDefense) ||
		(pType->Immune && !bIgnoreDefense) || pTarget->InLimbo
		;

	if (pTechno) {
		const auto pShield = TechnoExt::ExtMap.Find(pTechno)->GetShield();
		bImmune_pt2 = (pShield && pShield->IsActive())
			|| pTechno->TemporalTargetingMe
			|| (pTechno->ForceShielded && !bIgnoreDefense)
			|| pTechno->BeingWarpedOut
			|| pTechno->IsSinking
			;

	}

	if (!bImmune_pt1 && !bImmune_pt2) {
		auto const nArmor = pType->Armor;
		auto const pArmor = ArmorTypeClass::Array[(int)nArmor].get();

		if (pArmor) {
#ifdef COMPILE_PORTED_DP_FEATURES_
			TechnoClass_ReceiveDamage2_DamageText(pTechno, pDamage, pWarheadExt->DamageTextPerArmor[(int)nArmor]);
#endif

			if ((!pWarheadExt->ArmorHitAnim.empty())) {
				AnimTypeClass* pAnimTypeDecided = pWarheadExt->ArmorHitAnim.get_or_default((int)nArmor);

				if (!pAnimTypeDecided && pArmor->DefaultTo != -1) {
					//Holy shit !
					for (auto pDefArmor = ArmorTypeClass::Array[pArmor->DefaultTo].get();
						pDefArmor && pDefArmor->DefaultTo != -1;
						pDefArmor = ArmorTypeClass::Array[pDefArmor->DefaultTo].get()) {
						pAnimTypeDecided = pWarheadExt->ArmorHitAnim.get_or_default(pDefArmor->DefaultTo);
						if (pAnimTypeDecided)
							break;
					}
				}

				if (pAnimTypeDecided) {
					CoordStruct nBuffer { 0, 0 , 0 };

					if (pTechno) {
						auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

						if (!pTechnoTypeExt->HitCoordOffset.empty())
						{
							if ((pTechnoTypeExt->HitCoordOffset.size() > 1) && pTechnoTypeExt->HitCoordOffset_Random.Get())
								nBuffer = pTechnoTypeExt->HitCoordOffset[ScenarioClass::Instance->Random.RandomFromMax(pTechnoTypeExt->HitCoordOffset.size() - 1)];
							else
								nBuffer = pTechnoTypeExt->HitCoordOffset[0];
						}
					}

					auto const nCoord = pTarget->GetCenterCoords() + nBuffer;
					if (auto pAnimPlayed = GameCreate<AnimClass>(pAnimTypeDecided, nCoord)) {
						AnimExt::SetAnimOwnerHouseKind(pAnimPlayed, args->Attacker ? args->Attacker->GetOwningHouse() : args->SourceHouse, pTarget->GetOwningHouse(), args->Attacker, false);
					}
				}
			}
		}
	}
}

DEFINE_HOOK(0x5F53DB, ObjectClass_ReceiveDamage_Handled, 0xA)
{
	enum {
		ContinueChecks = 0x5F5456,
		DecideResult = 0x5F5498,
		SkipDecideResult = 0x5F546A,
		ReturnResultNone = 0x5F545C,
	};

	GET(ObjectClass*, pObject, ESI);
	REF_STACK(args_ReceiveDamage, args, STACK_OFFSET(0x24, 0x4));

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
	const bool bIgnoreDefenses = R->BL();

	ApplyHitAnim(pObject, &args);

	pWHExt->ApplyRelativeDamage(pObject , &args);

	if(!bIgnoreDefenses) {
		MapClass::GetTotalDamage(&args, pObject->GetType()->Armor);
		//this already calculate distance damage from epicenter
		pWHExt->ApplyRecalculateDistanceDamage(pObject, &args);
	}

	if (*args.Damage == 0 && pObject->WhatAmI() == AbstractType::Building) {
		auto const pBld = static_cast<BuildingClass*>(pObject);

		if (!pBld->Type->CanC4) {

			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (!pTypeExt->CanC4_AllowZeroDamage)
				*args.Damage = 1;
		}
	}

	if (!bIgnoreDefenses && args.Attacker && *args.Damage > 0) {
		if (pWHExt->ApplyCulling(args.Attacker, pObject))
			*args.Damage = pObject->Health;
	}

	const int pTypeStr = pObject->GetType()->Strength;
	const int nDamage = *args.Damage;
	R->EBP(pTypeStr);
	R->Stack(0x38, pTypeStr);
	R->ECX(nDamage);

	if (!nDamage)
		return ReturnResultNone;

	return nDamage > 0 ? DecideResult : SkipDecideResult;
}

DEFINE_HOOK(0x629BB2, ParasiteClass_UpdateSquiddy_Culling, 0x8)
{
	GET(ParasiteClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWH, EDI);

	enum { ApplyDamage = 0x629D19, GainExperience = 0x629BF3, SkipGainExperience = 0x629C5D };

	if (!WarheadTypeExt::ExtMap.Find(pWH)->ApplyCulling(pThis->Owner, pThis->Victim))
		return ApplyDamage;

	return pThis->Owner && pThis->Owner->Owner && pThis->Owner->Owner->IsAlliedWith(pThis->Victim)
		? SkipGainExperience : GainExperience;
}

DEFINE_HOOK(0x51A2EF, InfantryClass_PCP_Enter_Bio_Reactor_Sound, 0x6)
{
	GET(BuildingClass*, pBuilding, EDI);

	auto const nSound = BuildingTypeExt::ExtMap.Find(pBuilding->Type)
	->EnterBioReactorSound.Get(RulesClass::Instance->EnterBioReactorSound);

	if (nSound != -1) {
		VocClass::PlayAt(nSound, pBuilding->GetCoords(), 0);
	}

	return 0x51A30F;
}

DEFINE_HOOK(0x44DBBC, InfantryClass_PCP_Leave_Bio_Reactor_Sound, 0x7)
{
	GET(BuildingClass*, pThis, EBP);

	auto const nSound = BuildingTypeExt::ExtMap.Find(pThis->Type)
	->LeaveBioReactorSound.Get(RulesClass::Instance->LeaveBioReactorSound);

	if (nSound != -1) {
		VocClass::PlayAt(nSound, pThis->GetCoords(), 0);
	}

	return 0x44DBDA;
}

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_ReceiveDamage_DamageSound = 0x4426DB,
		BuildingClass_ReceiveDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_ReceiveDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_ReceiveDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_ReceiveDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_ReceiveDamage_Building_DamageSound_02_Handled_ret = 0x702765,

		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (pExt->DisableDamageSound.Get())
		{
			switch (R->Origin())
			{
			case BuildingClass_ReceiveDamage_DamageSound:
				return BuildingClass_ReceiveDamage_DamageSound_Handled_ret;
			case TechnoClass_ReceiveDamage_Building_DamageSound_01:
				return TechnoClass_ReceiveDamage_Building_DamageSound_01_Handled_ret;
			case TechnoClass_ReceiveDamage_Building_DamageSound_02:
				return TechnoClass_ReceiveDamage_Building_DamageSound_02_Handled_ret;
			}
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass*, pProduct, ESI);

	auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->GetTechnoType());

	if (pTechnoTypeExt->Eva_Complete.isset()) {
		VoxClass::PlayIndex(pTechnoTypeExt->Eva_Complete.Get());
	} else {
		VoxClass::Play(GameStrings::EVA_UnitReady());
	}

	return 0x4FB649;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	enum {
		ReturnNoVoiceCreate = 0x4FB804,
		Continue = 0x0
	};

	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pTechno, EBP);

	if (pTechno) {
		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		pTechno->QueueVoice(pTechnoTypeExt->VoiceCreate);

		if (!pTechnoTypeExt->CreateSound_Enable.Get())
			return ReturnNoVoiceCreate;

		if (RulesExt::Global()->CreateSound_PlayerOnly.Get())
			return pThis->IsControlledByCurrentPlayer() ?
				Continue : ReturnNoVoiceCreate;
	}

	return Continue;
}

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass*, pTech, ESI);

	if (pTech && (pTech->WhatAmI() == AbstractType::Building) &&
		pTech->Owner &&
		pTech->Owner->ControlledByPlayer())
	{
		auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTech->GetTechnoType());

		if (pTechnoTypeExt->Eva_Complete.isset()) {
			VoxClass::PlayIndex(pTechnoTypeExt->Eva_Complete.Get());
		} else {
			VoxClass::Play(GameStrings::EVA_ConstructionComplete());
		}

		return 0x6A8E34;
	}

	return 0x0;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
#ifdef COMPILE_PORTED_DP_FEATURES

	GET(BuildingClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));
	LEA_STACK(CoordStruct*, pCoord, STACK_OFFSET(0xF0, -0x8C));

	if (pWeapon && pBullet)
	{
		CoordStruct src = pThis->GetFLH(0, pThis->GetRenderCoords());
		CoordStruct dest = *pCoord;
		auto const pTarget = pBullet->Target ? pBullet->Target : MapClass::Instance->GetCellAt(dest);

		// Draw bullet effect
		Helpers_DP::DrawBulletEffect(pWeapon, src, dest, pThis, pTarget);
		// Draw particle system
		Helpers_DP::AttachedParticleSystem(pWeapon, src, pTarget, pThis, dest);
		// Play report sound
		Helpers_DP::PlayReportSound(pWeapon, src, pThis);
		// Draw weapon anim
		Helpers_DP::DrawWeaponAnim(pWeapon, src, dest, pThis, pTarget);
	}
#endif
	return 0;
}

DEFINE_HOOK(0x518B98, InfantryClass_ReceiveDamage_DeadBodies, 0x8)
{
	GET(InfantryClass*, pThis, ESI);
	// REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xD0, -0x4));

	// if (!InfantryExt::ExtMap.Find(pThis)->IsUsingDeathSequence && !pThis->Type->JumpJet) {
	// 	auto pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
	// 	if (!pWHExt->DeadBodies.empty()) {
	// 		if (AnimTypeClass* pSelected = pWHExt->DeadBodies.at(
	// 			ScenarioClass::Instance->Random.RandomFromMax(pWHExt->DeadBodies.size() - 1)))
	// 		{
	// 			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
	// 				AnimExt::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse, pThis->GetOwningHouse(), true);
	// 			}
	// 		}
	// 	}
	// }

	pThis->UnInit();
	return 0x518BA0;
}

//TODO : Add PerTechnoOverride
DEFINE_HOOK(0x639DD8, TechnoClass_PlanningManager_DecideEligible, 0x5)
{
	enum { CanUse = 0x639DDD, ContinueCheck = 0x639E03 };

	GET(TechnoClass*, pThis, ESI);
	auto const pWhat = pThis->WhatAmI();

	if (pWhat == AbstractType::Infantry || pWhat == AbstractType::Unit || pWhat == AbstractType::Aircraft)
		return CanUse;

	return ContinueCheck;
}

DEFINE_HOOK(0x4242F4, AnimClass_Trail_Override, 0x6)
{
	GET(AnimClass*, pAnim, EDI);
	GET(AnimClass*, pThis, ESI);

	auto nCoord = pThis->GetCoords();
	GameConstruct(pAnim, pThis->Type->TrailerAnim, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* const pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
	HouseClass* const pOwner = !pThis->Owner && pTech ? pTech->Owner : pThis->Owner;
	AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);

	return 0x424322;
}

//DEFINE_HOOK(0x51BCA4, InfantryClass_AI_ReloadInTransporterFix, 0x6)
//{
//	enum { RetFunct = 0x51BF80, CheckLayer = 0x51BDCF, CheckMission = 0x51BCC0 };
//
//	GET(InfantryClass*, pThis, ESI);
//
//	if (!pThis->IsAlive)
//		return RetFunct;
//
//	if (!pThis->InLimbo || pThis->Transporter)
//		pThis->Reload();
//
//	if (pThis->InLimbo)
//		return CheckLayer;
//
//	return CheckMission;
//}

//DEFINE_HOOK(0x51DF82, InfantryClass_FireAt_StartReloading, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	const auto pType = pThis->Type;
//
//	if (pType->Ammo > 0 && pType->Ammo > pThis->Ammo && !pType->ManualReload)
//		pThis->StartReloading();
//
//	return 0;
//}

//static constexpr CompileTimeMatrix3D Mtx {};
//static constexpr Point2D Data {};
//static constexpr Point3D Data2 {};
//static constexpr CoordStruct Data3 {};
//static constexpr CellStruct Data4 {};

DEFINE_HOOK(0x739450, UnitClass_Deploy_LocationFix, 0x7)
{
	GET(UnitClass*, pThis, EBP);
	const auto deploysInto = pThis->Type->DeploysInto;
	CellStruct mapCoords = pThis->GetMapCoords();
	R->Stack(STACK_OFFSET(0x28, -0x10), mapCoords);

	const short width = deploysInto->GetFoundationWidth();
	const short height = deploysInto->GetFoundationHeight(false);

	if (width > 2)
		mapCoords.X -= static_cast<short>(std::ceil(width / 2.0) - 1);
	if (height > 2)
		mapCoords.Y -= static_cast<short>(std::ceil(height / 2.0) - 1);

	R->Stack(STACK_OFFSET(0x28, -0x14), mapCoords);

	return 0x7394BE;
}

DEFINE_HOOK(0x449E8E, BuildingClass_Mi_Selling_UndeployLocationFix, 0x5)
{
	GET(BuildingClass*, pThis, EBP);
	CellStruct mapCoords = pThis->GetMapCoords();

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

//DEFINE_HOOK(0x5D4E3B, DispatchingMessage_ReloadResources, 0x5)
//{
//	LEA_STACK(tagMSG*, pMsg, 0x10);
//	GET_STACK(DWORD, nDW, 0x14);
//
//	if ((nDW == 0x10 || nDW == 0x2 || nDW == 0x112) && pMsg->wParam == (WPARAM)0xF060)
//		ExitProcess(1u);
//
//	//if ((nDW == 0x104 || nDW == 0x100) 
//	//	&& pMsg.wParam == (WPARAM)0xD && ((pMsg.lParam & 0x20000000) != 0))
//	//{
//	//	set critical section here ?
//	//}
//
//	Imports::TranslateMessage.get()(pMsg);
//	Imports::DispatchMessageA.get()(pMsg);
//
//	return 0x5D4E4D;
//}

//DEFINE_HOOK(0x6AB64F, SidebarClass_ClickedAction_Focus, 0x6)
//{
//	GET(TechnoTypeClass*, pItem, EAX);
//
//	const HouseClass* pHouse = HouseClass::CurrentPlayer;
//	if (!pHouse || !pItem)
//		return 0x0;
//
//	const CanBuildResult canBuild = pHouse->CanBuild(pItem, true, false);
//	
//	if (canBuild == CanBuildResult::TemporarilyUnbuildable) {
//		for (auto pTechno : *TechnoClass::Array) {
//
//			if (!pTechno->IsAlive || pTechno->IsCrashing || pTechno->IsSinking)
//				continue;
//			
//			if (pTechno->Owner == pHouse && pTechno->GetTechnoType() == pItem)
//			{
//				CoordStruct coords = pTechno->GetCoords();
//
//				if (!coords)
//					continue;
//		
//				TacticalClass::Instance->SetTacticalPosition(&coords);
//				pTechno->Flash(60);
//				if (pItem->VoiceSelect.Items)
//					pTechno->QueueVoice(pItem->VoiceSelect[0]);
//
//				MapClass::Instance->MarkNeedsRedraw(1);
//				break;
//			}
//		}
//	}
//
//	return 0;
//}

void ObjectClass_ReceiveDamage_NPEXT_EMPulseSparkles(ObjectClass* pTarget)
{
	int nLoopCount = ScenarioClass::Instance->Random.RandomFromMax(25);
	if (auto pSparkel = GameCreate<AnimClass>(RulesClass::Instance->EMPulseSparkles, pTarget->GetCoords(), 0, nLoopCount))
		pSparkel->SetOwnerObject(pTarget);
}

enum class KickOutProductionType : int
{
	Normal = 0,
	Droppod,
	Paradrop,
	Anim,
};

enum class FunctionreturnType : int
{
	Succeeded = 0,
	Failed,
	Nothing,
};

FunctionreturnType KickoutTechnoType(BuildingClass* pProduction, KickOutProductionType nDecided)
{
	bool UnlimboSucceeded = false;
	switch (nDecided)
	{
	case KickOutProductionType::Droppod:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	case KickOutProductionType::Paradrop:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	case KickOutProductionType::Anim:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	}

	return FunctionreturnType::Nothing;
}

//DEFINE_HOOK(0x4445FB, BuildingClass_KickOut_FactoryType_NotAWeaponFactory, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	switch(KickoutTechnoType(pThis, KickOutProductionType::Normal))
//	{
//	case FunctionreturnType::Nothing:
//		return 0x0; // nothing
//	case FunctionreturnType::Succeeded:
//		return 0x4448CE; // set mission
//	case FunctionreturnType::Failed : 
//		return 0x444EDE; // decrease mutex
//	}
//}

//struct TunnelTypeClass
//{
//	static std::vector<std::unique_ptr<TunnelTypeClass>> Array;
//
//	Valueable<int> Size {};
//};
//std::vector<std::unique_ptr<TunnelTypeClass>> TunnelTypeClass::Array {};
//
//struct HouseExt_Ares
//{
//	HouseClass* AttachedTo;
//	static PhobosMap<TunnelTypeClass*, std::vector<FootClass*>> Tunnels;
//};
//
//PhobosMap<TunnelTypeClass*, std::vector<FootClass*>> HouseExt_Ares::Tunnels {};
//
//std::vector<FootClass*>* Isunnel(int Idx, HouseClass* pHouse)
//{
//
//	if (Idx <= TunnelTypeClass::Array.size()) {
//		return &HouseExt_Ares::Tunnels[TunnelTypeClass::Array.at(Idx).get()];
//	}
//
//	return nullptr;
//}
//
//bool SizeEligible(int Idx, HouseClass* pHouse)
//{
//	if (Idx <= TunnelTypeClass::Array.size())
//	{
//		auto const pTunnel = TunnelTypeClass::Array.at(Idx).get();
//		auto const& pTunnelData = HouseExt_Ares::Tunnels[pTunnel];
//		return pTunnelData.size() < pTunnel->Size;
//	}
//}
//
//bool DestroyTunnelContents(std::vector<FootClass*>* pVec , BuildingClass* pTargetBld , TechnoClass* pKiller)
//{
//	if (pVec->begin() == pVec->end())
//		return false;
//
//	auto const pOwner = pTargetBld->Owner;
//	
//	//find same tunnel from current owner
//	auto const bSameTypeExist = std::any_of(pOwner->Buildings.begin(), pOwner->Buildings.end(),
//		[&](BuildingClass* pBld) {
//
//			if (pBld->Health > 0 && !pBld->InLimbo && pBld->IsOnMap)
//			{
//				auto const nCurMission = pBld->CurrentMission;
//				if (nCurMission != Mission::Construction
//				  && nCurMission != Mission::Selling
//				  && pBld != pTargetBld
//				  //&& pBld->Type->BuildingTypeExt_ExtData->TunnelType == pTargetBld->Type->BuildingTypeExt_ExtData->TunnelType
//					)
//				{
//					return true;
//				}
//			}
//			return false;
//		});
//		
//	if (!bSameTypeExist) {
//
//		for (size_t i = 0; i < pVec->size(); ++i) {
//			auto const pFoot = pVec->at(i);
//
//			if (pFoot->OldTeam)
//				pFoot->OldTeam->RemoveMember(pFoot);
//
//			pFoot->RegisterDestruction(pKiller);
//			pFoot->UnInit();
//			pVec->erase(pVec->begin() + i);
//		}
//
//		return true;
//	}
//
//	return false;
//}
//
//DWORD BuildingClass_Demolish_tunnel(REGISTERS* R)
//{
//	GET_STACK(AbstractClass*, pKiller, 0x90);
//	GET(BuildingClass*, pThis, EDI);
//
//	if (auto pTunnel = Isunnel(0, pThis->Owner))
//	{
//		auto const pTechno = generic_cast<TechnoClass*>(pKiller);
//
//		DestroyTunnelContents(pTunnel, pThis, pTechno);
//	}
//	
//	return 0x0;
//}

#include <Powerups.h>

enum class CrateType : unsigned char{
 MONEY = 0,
 UNIT = 1,
 HEAL_BASE = 2,
 CLOAK = 3,
 EXPLOSION = 4,
 NAPALM = 5,
 SQUAD = 6,
 DARKNESS = 7,
 REVEAL = 8,
 ARMOR = 9,
 SPEED = 10,
 FIREPOWER = 11,
 ICBM = 12,
 INVULN = 13,
 VETERAN = 14,
 ION_STORM = 15,
 GAS = 16,
 TIBERIUM = 17,
 POD = 18,
 COUNT = 19,
};

//bool CollectCrate(CellClass* pThis, FootClass* Collector)
//{
//	if (!Collector)
//		return false;
//
//	const auto v3 = pThis->OverlayTypeIndex;
//	if (v3 != -1)
//	{
//		if (!OverlayTypeClass::Array->GetItem(v3)->Crate)
//			return false;
//
//		if (SessionClass::Instance->GameMode == GameMode::Campaign || !Collector->Owner->Type->MultiplayPassive)
//		{
//			if (OverlayTypeClass::Array->GetItem(v3)->CrateTrigger)
//			{
//				if (Collector->AttachedTag)
//				{
//					Debug::Log("Springing trigger on crate at [ %d,%d ] \n", pThis->MapCoords.X, pThis->MapCoords.Y);
//					Collector->AttachedTag->RaiseEvent(TriggerEvent::PickupCrate, Collector, CellStruct::Empty, false, nullptr);
//					if (!Collector->IsAlive) {
//						return false;
//					}
//				}
//				ScenarioClass::Instance->PickedUpAnyCrate = true;
//			}
//
//			CrateType v11 = CrateType::MONEY;
//			if (pThis->OverlayData < static_cast<unsigned char>(CrateType::COUNT))
//			{
//				v11 = (CrateType)pThis->OverlayData;
//			}
//			else
//			{
//				int total_shares = 0;
//				for (auto& share : Powerups::Weights)
//				{
//					total_shares += share;
//				}
//
//				int pick = ScenarioClass::Instance->Random.RandomRanged(1, total_shares);
//
//				int share_count = 0;
//				unsigned int nDummy = 0u;
//				for (auto& share : Powerups::Weights)
//				{
//					share_count += share;
//					if (pick <= share_count)
//						break;
//
//					++nDummy;
//				}
//
//				v11 = (CrateType)nDummy;
//			}
//
//			auto v121 = v11;
//			if (SessionClass::Instance->GameMode == GameMode::Campaign)
//			{
//				auto v14 = Collector->Owner;
//				auto arg0 = v14->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);
//				bool v116 = false;
//
//				if (!v14->OwnedBuildings
//				  && v14->Available_Money() > 1500
//				  && !v14->OwnedUnitTypes.GetItemCount(arg0->ArrayIndex)
//				  && GameOptionsType::Instance->Bases)
//				{
//					v11 = CrateType::UNIT;
//					v121 = CrateType::UNIT;
//					v116 = true;
//				}
//
//				switch (v11)
//				{
//				case CrateType::UNIT:
//					if (Collector->Owner->OwnedUnits <= 50)
//					{
//						auto v16 = pThis->LandType;
//						if (v16 == LandType::Water || v16 == LandType::Beach)
//						{
//							v11 = CrateType::MONEY;
//							v121 = CrateType::MONEY;
//						}
//						break;
//					}
//					goto LABEL_35;
//				case CRATE_CLOAK:
//					v15 = object;
//					if (object->t.IsCloakable)
//					{
//						v11 = 0;
//						v121 = 0;
//					}
//					break;
//				case CRATE_SQUAD:
//					v15 = object;
//					if (object->t.House->CurInfantry > 100)
//					{
//					LABEL_35:
//						v11 = 0;
//						v121 = 0;
//					}
//				LABEL_36:
//					v16 = this->Land;
//					if (v16 == LAND_WATER || v16 == LAND_BEACH)
//					{
//						v11 = 0;
//						v121 = 0;
//					}
//					break;
//				case CRATE_ARMOR:
//					v15 = object;
//					if (LODWORD(object->t.ArmorBias) || HIDWORD(object->t.ArmorBias) != 1072693248)
//					{
//						v11 = 0;
//						v121 = 0;
//					}
//					break;
//				case CRATE_SPEED:
//					v15 = object;
//					if (LODWORD(object->SpeedMultiplier)
//					  || HIDWORD(object->SpeedMultiplier) != 0x3FF00000
//					  || object->t.r.m.o.a.vftable->t.r.m.o.a.Kind_Of(object) == RTTI_AIRCRAFT)
//					{
//						v11 = CRATE_MONEY;
//						v121 = CRATE_MONEY;
//					}
//					break;
//				case CRATE_FIREPOWER:
//					v15 = object;
//					if (LODWORD(object->t.FirepowerBias)
//					  || HIDWORD(object->t.FirepowerBias) != 1072693248
//					  || !object->t.r.m.o.a.vftable->t.Is_Weapon_Equipped(object))
//					{
//						v11 = 0;
//						v121 = 0;
//					}
//					break;
//				case CRATE_VETERAN:
//					v15 = object;
//					if ((object->t.r.m.o.a.TargetBitfield[0] & 1) != 0)
//					{
//						if (object->t.r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(&object->t)->Trainable)
//						{
//							if ((object->t.r.m.o.a.TargetBitfield[0] & 1) != 0 && VeterancyClass::Is_Elite(&object->t.__Veterancy))
//							{
//								v11 = 0;
//								v121 = 0;
//							}
//						}
//						else
//						{
//							v11 = 0;
//							v121 = 0;
//						}
//					}
//					break;
//				default:
//					break;
//				}
//
//				if (pThis->LandType == LandType::Water && !Powerups::Naval[(int)v11])
//				{
//					v11 = 0;
//					v121 = 0;
//				}
//
//				if (SessionClass::Instance->GameMode == GameMode::Internet)
//				{
//					Collector->Owner->CollectedCrates.IncrementUnitCount((int)v11);
//				}
//			}
//			else if (!pThis->OverlayData)
//			{
//				auto v12 = pThis->OverlayTypeIndex;
//				auto nSoloCrateMoney = RulesClass::Instance->SoloCrateMoney;
//				auto v13 = OverlayTypeClass::Array->GetItem(v12);
//
//				if (v13 == RulesClass::Instance->CrateImg) {
//					v121 = (CrateType)RulesClass::Instance->SilverCrate;
//				}
//
//				if (v13 == RulesClass::Instance->WoodCrateImg) {
//					v121 = (CrateType)RulesClass::Instance->WoodCrate;
//				}
//
//				if (v13 == RulesClass::Instance->WaterCrateImg) {
//					v121 = (CrateType)RulesClass::Instance->WaterCrate;
//				}
//			}
//			
//			MapClass::Instance->Remove_Crate(&pThis->MapCoords);
//			if (SessionClass::Instance->GameMode == GameMode::Campaign && GameOptionsType::Instance->Crates)
//			{
//				MapClass::Instance->Place_Random_Crate();
//			}
//
//			if (v11 == (CrateType)6)
//			{
//				v11 = CrateType::MONEY;
//				v121 = CrateType::MONEY;
//			}
//
//			bool nPlaySound = 0;
//
//			switch (v11)
//			{
//			case CrateType::MONEY:
//				goto LABEL_122;
//			case CrateType::UNIT:
//				Debug::Log("Crate at %d,%d contains a unit\n", pThis->MapCoords.X, pThis->MapCoords.Y);
//				utp = 0;
//				if (HIBYTE(v116))
//				{
//					v35 = &object->t;
//					utp = HouseClass_Unitstuff(object->t.House, &Rule->BaseUnit);
//					if (utp)
//					{
//						goto LABEL_93;
//					}
//				}
//				else
//				{
//					v35 = &object->t;
//				}
//
//				if ((CounterClass::Count_Of(&v35->House->BQuantity, *(*Rule->BuildRefinery.dvc.Vector_Item + 894)) > 0
//					|| CounterClass::Count_Of(&v35->House->BQuantity, *(*(Rule->BuildRefinery.dvc.Vector_Item + 1) + 3576)) > 0)
//				  && !CounterClass::Count_Of(&v35->House->UQuantity, *(*Rule->HarvesterUnit.dvc.Vector_Item + 894))
//				  && !CounterClass::Count_Of(&v35->House->UQuantity, *(*(Rule->HarvesterUnit.dvc.Vector_Item + 1) + 3576)))
//				{
//					utp = HouseClass_Unitstuff(v35->House, &Rule->HarvesterUnit);
//				}
//			LABEL_93:
//				if (Rule->UnitCrateType)
//				{
//					utp = Rule->UnitCrateType;
//				}
//				if (utp)
//				{
//					goto LABEL_111;
//				}
//				do
//				{
//					while (1)
//					{
//						do
//						{
//							utp = (*(&UnitTypes + 1))[Random2Class::operator()(&Scen->RandomNumber, 0, *(&UnitTypes + 4) - 1)];
//							v36 = 0;
//							v37 = Rule->BaseUnit.dvc.ActiveCount;
//							if (v37 <= 0)
//							{
//							LABEL_100:
//								v39 = 0;
//							}
//							else
//							{
//								v38 = Rule->BaseUnit.dvc.Vector_Item;
//								while (*v38 != utp)
//								{
//									++v36;
//									++v38;
//									if (v36 >= v37)
//									{
//										goto LABEL_100;
//									}
//								}
//								v39 = 1;
//							}
//							v40 = HouseClass::Is_Player_Control(object->t.House);
//						}
//						while (!utp->IsCrateGoodie);
//						if (GameOptions.Bases)
//						{
//							break;
//						}
//						if (!v39)
//						{
//							goto LABEL_109;
//						}
//					}
//				}
//				while (v39 && !v40 && !HIBYTE(v116));
//			LABEL_109:
//				if (!utp)
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v35 = &object->t;
//			LABEL_111:
//				v41 = utp->tt.ot.at.a.vftable->ot.Create_One_Of(utp, v35->House);
//				if (!v41)
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v125.X = this->Position;
//				Operator_Negate(&v122, (SLOWORD(v125.X) << 8) + 128, (SHIWORD(v125.X) << 8) + 128, 0);
//				v123 = 128;
//				v124 = 128;
//				v42 = CellClass::Get_Z_Offset(this, &v123);
//				v133[0] = v122.X;
//				v122.Z = v42;
//				v133[1] = v122.Y;
//				v133[2] = v42;
//				if (v41->f.t.r.m.o.a.vftable->t.r.m.o.Unlimbo(v41, v133, 0))
//				{
//					if (!HouseClass::Player_Has_Control(object->t.House))
//					{
//						return 0;
//					}
//					v125.X = this->Position;
//					Operator_Negate(&v122, (SLOWORD(v125.X) << 8) + 128, (SHIWORD(v125.X) << 8) + 128, 0);
//					v120.X = 128;
//					v120.Y = 128;
//					v43 = CellClass::Get_Z_Offset(this, &v120);
//					v141[1] = v122.Y;
//					v122.Z = v43;
//					v141[0] = v122.X;
//					v141[2] = v43;
//					HIDWORD(n) = 0;
//					v44 = v141;
//				LABEL_119:
//					VocClass::Play_Ranged(Rule->CrateUnitSound, v44, HIDWORD(n));
//					return 0;
//				}
//				cell2 = this->Position;
//				arg0 = 0;
//				v45 = *MapClass::Nearby_Location(
//						   &Map.sc.t.sb.p.r.d.m,
//						   &cell1,
//						   &cell2,
//						   v41->Class->tt.Speed,
//						   -1,
//						   MZONE_NORMAL,
//						   0,
//						   1,
//						   1,
//						   0,
//						   0,
//						   0,
//						   1,
//						   &arg0,
//						   0,
//						   0);
//				v125.X = v45;
//				if (v45 != CellClass_DefaultCell)
//				{
//					v140[1] = (SHIWORD(v45) << 8) + 128;
//					v140[0] = (v45 << 8) + 128;
//					v140[2] = 0;
//					if (v41->f.t.r.m.o.a.vftable->t.r.m.o.Unlimbo(v41, v140, 0))
//					{
//						if (!HouseClass::Player_Has_Control(object->t.House))
//						{
//							return 0;
//						}
//						v125.X = this->Position;
//						v46 = (SLOWORD(v125.X) << 8) + 128;
//						v47 = (SHIWORD(v125.X) << 8) + 128;
//						v120.X = 128;
//						v120.Y = 128;
//						v48 = CellClass::Get_Z_Offset(this, &v120);
//						a2a[0] = v46;
//						a2a[1] = v47;
//						a2a[2] = v48;
//						HIDWORD(n) = 0;
//						v44 = a2a;
//						goto LABEL_119;
//					}
//				}
//				v41->f.t.r.m.o.a.vftable->t.r.m.o.a.SDTOR(v41, 1);
//				v121 = 0;
//			LABEL_122:
//				Debug::Log("Crate at %d,%d contains money\n", this->Position.X, this->Position.Y);
//				v50 = cell;
//				if (!cell)
//				{
//					v50 = Random2Class::operator()(&Scen->RandomNumber, *&v120, *&v120 + 900);
//				}
//				if (!HouseClass::Player_Has_Control(object->t.House) || Session.Type)
//				{
//					HIDWORD(n) = v50;
//					v51 = object->t.House;
//				}
//				else
//				{
//					v51 = PlayerPtr;
//					HIDWORD(n) = v50;
//				}
//				HouseClass::Refund_Money(v51, SHIDWORD(n));
//				if (HouseClass::Player_Has_Control(object->t.House))
//				{
//					v125.X = this->Position;
//					Operator_Negate(&v122, (SLOWORD(v125.X) << 8) + 128, (SHIWORD(v125.X) << 8) + 128, 0);
//					v120.X = 128;
//					v120.Y = 128;
//					v122.Z = CellClass::Get_Z_Offset(this, &v120);
//					v132 = v122;
//					HIDWORD(n) = 0;
//					v32 = Rule->CrateMoneySound;
//					v33 = &v132;
//				LABEL_222:
//					VocClass::Play_Ranged(v32, v33, HIDWORD(n));
//				}
//			switch_Goodie_Check_DoAnimation:
//				if (CrateAnims[v121] != -1)
//				{
//					v123 = this->Position;
//					v109 = (v123 << 8) + 128;
//					v110 = (SHIWORD(v123) << 8) + 128;
//					v120.X = 128;
//					v120.Y = 128;
//					v111 = CellClass::Get_Z_Offset(this, &v120);
//					v128.X = v109;
//					v128.Y = v110;
//					v128.Z = v111 + 200;
//					v112 = operator new(0x1C8u);
//					if (v112)
//					{
//						AnimClass::AnimClass(v112, (*(&AnimTypes + 1))[CrateAnims[v121]], &v128, 0, 1, AnimFlag_400 | AnimFlag_200, 0, 0);
//					}
//				}
//				break;
//			case CrateType::HEAL_BASE:
//				Debug::Log("Crate at %d,%d contains base healing\n", this->Position.X, this->Position.Y);
//				if (HouseClass::Player_Has_Control(object->t.House))
//				{
//					v123 = this->Position;
//					v81 = (v123 << 8) + 128;
//					v82 = (SHIWORD(v123) << 8) + 128;
//					v120.X = 128;
//					v120.Y = 128;
//					coord.Z = CellClass::Get_Z_Offset(this, &v120);
//					coord.X = v81;
//					coord.Y = v82;
//					VocClass::Play_Ranged(Rule->HealCrateSound, &coord, 0);
//				}
//				for (i = 0; i < Logic.l.Objects.ActiveCount; ++i)
//				{
//					v84 = Logic.l.Objects.Vector_Item[i];
//					if (v84)
//					{
//						HIBYTE(v116) = object->t.r.m.o.a.TargetBitfield[0] & 1;
//						if (HIBYTE(v116))
//						{
//							if (object->t.House == v84->a.vftable->t.r.m.o.a.Owner__Owning_House(v84))
//							{
//								v85 = v84->a.vftable->t.r.m.o.Class_Of(v84);
//								v86 = v84->a.vftable;
//								v123 = (v84->Strength - v85->ot.MaxStrength);
//								v86->t.r.m.o.Take_Damage(v84, &v123, 0, Rule->C4Warhead, 0, 1, 1, 0);
//							}
//						}
//					}
//				}
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::CLOAK:
//				Debug::Log("Crate at %d,%d contains cloaking device\n", this->Position.X, this->Position.Y);
//				for (cell = 0; cell < DisplayClass::Layer[2].ActiveCount; cell = (cell + 1))
//				{
//					damage = DisplayClass::Layer[2].Vector_Item[cell];
//					if (damage)
//					{
//						if (damage->t.r.m.o.IsDown)
//						{
//							HIBYTE(v116) = damage->t.r.m.o.a.TargetBitfield[0] & 1;
//							if (HIBYTE(v116))
//							{
//								v123 = this->Position;
//								v71 = (v123 << 8) + 128;
//								v72 = (SHIWORD(v123) << 8) + 128;
//								v120.X = 128;
//								v120.Y = 128;
//								v126 = CellClass::Get_Z_Offset(this, &v120);
//								v73 = damage->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(damage, &v146);
//								v74 = v73->Z;
//								v122.X = v71 - v73->X;
//								v122.Y = v72 - v73->Y;
//								v122.Z = *&v126 - v74;
//								if (FastMath::Sqrt((*&v126 - v74) * (*&v126 - v74) + v122.Y * v122.Y + v122.X * v122.X) < Rule->CrateRadius)
//								{
//									damage->t.IsCloakable = 1;
//								}
//							}
//						}
//					}
//				}
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::EXPLOSION:
//				Debug::Log("Crate at %d,%d contains explosives\n", this->Position.X, this->Position.Y);
//				v125.X = v120;
//				v52 = Rule->C4Warhead;
//				damage = v125.X;
//				object->t.r.m.o.a.vftable->t.r.m.o.Take_Damage(&object->t.r.m.o, &v125.X, 0, v52, 0, 1, 0, 0);
//				cell = 5;
//				do
//				{
//					v123 = Random2Class::operator()(&Scen->RandomNumber, 0, 512);
//					v125.X = this->Position;
//					v53 = (SLOWORD(v125.X) << 8) + 128;
//					v54 = (SHIWORD(v125.X) << 8) + 128;
//					v120.X = 128;
//					v120.Y = 128;
//					v55 = CellClass::Get_Z_Offset(this, &v120);
//					v138.X = v53;
//					v138.Y = v54;
//					v138.Z = v55;
//					v56 = Coord_Scatter(&v149, &v138, v123, 0);
//					v57 = damage;
//					v122.X = v56->X;
//					v122.Y = v56->Y;
//					*&n = 1i64;
//					v122.Z = v56->Z;
//					Explosion_Damage(&v122, damage, 0, Rule->C4Warhead, 1, 0);
//					v58 = Combat_Anim(v57, Rule->C4Warhead, LAND_CLEAR, &v122);
//					v59 = operator new(0x1C8u);
//					if (v59)
//					{
//						*&n = v122.Z;
//						v60 = Combat_ZAdjust(v122.Y, &v113, v122.X, v122.Y, v122.Z);
//						AnimClass::AnimClass(v59, v58, &v122, 0, 1, AnimFlag_2000 | AnimFlag_400 | AnimFlag_200, v60, SBYTE4(n));
//					}
//					n = 0.0;
//					Do_Flash(damage, Rule->C4Warhead, v122.X, v122.Y, v122.Z, 0, 0);
//					cell = (cell - 1);
//				}
//				while (cell);
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::NAPALM:
//				Debug::Log("Crate at %d,%d contains napalm\n", this->Position.X, this->Position.Y);
//				v61 = this->Position;
//				v62 = (v61.X << 8) + 128;
//				v63 = (v61.Y << 8) + 128;
//				v123 = 128;
//				v124 = 128;
//				v122.Z = CellClass::Get_Z_Offset(this, &v123);
//				v64 = object->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(object, &v145);
//				v65 = v64->X;
//				HIDWORD(n) = 456;
//				v66 = v65 + v62;
//				v67 = v63 + v64->Y;
//				v68 = (v122.Z + v64->Z) / 2;
//				ecx0a.X = v66 / 2;
//				ecx0a.Y = v67 / 2;
//				ecx0a.Z = v68;
//				v69 = operator new(0x1C8u);
//				if (v69)
//				{
//					AnimClass::AnimClass(v69, **(&AnimTypes + 1), &ecx0a, 0, 1, AnimFlag_400 | AnimFlag_200, 0, 0);
//				}
//				v70 = object->t.r.m.o.a.vftable;
//				v123 = *&v120;
//				v70->t.r.m.o.Take_Damage(&object->t.r.m.o, &v123, 0, Rule->FlameDamage, 0, 1, 0, 0);
//				Explosion_Damage(&ecx0a, *&v120, 0, Rule->FlameDamage, 1, 0);
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::DARKNESS:
//				Debug::Log("Crate at %d,%d contains 'shroud'\n", this->Position.X, this->Position.Y);
//				MapClass::Reset_Shroud(&Map.sc.t.sb.p.r.d.m, object->t.House);
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::REVEAL:
//				Debug::Log("Crate at %d,%d contains 'reveal'\n", this->Position.X, this->Position.Y);
//				MapClass::Clear_Shroud(&Map.sc.t.sb.p.r.d.m, object->t.House);
//				v125.X = this->Position;
//				Operator_Negate(&v122, (SLOWORD(v125.X) << 8) + 128, (SHIWORD(v125.X) << 8) + 128, 0);
//				v120.X = 128;
//				v120.Y = 128;
//				v122.Z = CellClass::Get_Z_Offset(this, &v120);
//				v142 = v122;
//				HIDWORD(n) = 0;
//				v32 = Rule->CrateRevealSound;
//				v33 = &v142;
//				goto LABEL_222;
//			case CrateType::ARMOR:
//				Debug::Log("Crate at %d,%d contains armor\n", this->Position.X, this->Position.Y);
//				damage = 0;
//				if (DisplayClass::Layer[2].ActiveCount > 0)
//				{
//					do
//					{
//						cell = DisplayClass::Layer[2].Vector_Item[damage];
//						if (cell)
//						{
//							HIBYTE(v116) = cell->r.m.o.a.TargetBitfield[0] & 1;
//							if (HIBYTE(v116))
//							{
//								v123 = this->Position;
//								v125.X = 128;
//								v125.Y = 128;
//								v129 = CellClass::Get_Z_Offset(this, &v125);
//								v92 = cell->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(cell, v150);
//								v93 = v92->Z;
//								v122.X = (v123 << 8) + 128 - v92->X;
//								v122.Y = (SHIWORD(v123) << 8) + 128 - v92->Y;
//								v122.Z = v129 - v93;
//								if (FastMath::Sqrt((v129 - v93) * (v129 - v93) + v122.Y * v122.Y + v122.X * v122.X) < Rule->CrateRadius
//								  && !LODWORD(cell->ArmorBias)
//								  && HIDWORD(cell->ArmorBias) == 1072693248)
//								{
//									v94 = cell->r.m.o.a.vftable;
//									cell->ArmorBias = *&v120 * cell->ArmorBias;
//									v95 = (v94->t.r.m.o.a.Owner__Owning_House)();
//									if (HouseClass::Player_Has_Control(v95))
//									{
//										HIBYTE(nPlaySound) = 1;
//									}
//								}
//							}
//						}
//						damage = (damage + 1);
//					}
//					while (damage < DisplayClass::Layer[2].ActiveCount);
//					if (HIBYTE(nPlaySound))
//					{
//						VoxClass_Speak_From_Name("EVA_UnitArmorUpgraded", -1, -1);
//					}
//				}
//				if (!HouseClass::Player_Has_Control(object->t.House))
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v123 = this->Position;
//				v96 = (v123 << 8) + 128;
//				v97 = (SHIWORD(v123) << 8) + 128;
//				v120.X = 128;
//				v120.Y = 128;
//				v137[2] = CellClass::Get_Z_Offset(this, &v120);
//				v137[0] = v96;
//				v137[1] = v97;
//				v32 = Rule->CrateArmourSound;
//				HIDWORD(n) = 0;
//				v33 = v137;
//				goto LABEL_222;
//			case CrateType::SPEED:
//				Debug::Log("Crate at %d,%d contains speed\n", this->Position.X, this->Position.Y);
//				damage = 0;
//				if (DisplayClass::Layer[2].ActiveCount > 0)
//				{
//					do
//					{
//						cell = DisplayClass::Layer[2].Vector_Item[damage];
//						if (cell)
//						{
//							HIBYTE(v116) = (cell->r.m.o.a.TargetBitfield[0] & 4) != 0;
//							if (HIBYTE(v116))
//							{
//								v123 = this->Position;
//								v125.X = 128;
//								v125.Y = 128;
//								v129 = CellClass::Get_Z_Offset(this, &v125);
//								v98 = cell->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(cell, v152);
//								v99 = v98->Z;
//								v122.X = (v123 << 8) + 128 - v98->X;
//								v122.Y = (SHIWORD(v123) << 8) + 128 - v98->Y;
//								v122.Z = v129 - v99;
//								if (FastMath::Sqrt((v129 - v99) * (v129 - v99) + v122.Y * v122.Y + v122.X * v122.X) < Rule->CrateRadius
//								  && !cell[1].r.m.o.__AmbientSoundController2.field_10
//								  && cell[1].r.m.o.__AttachedSound == 1072693248
//								  && cell->r.m.o.a.vftable->t.r.m.o.a.Kind_Of(cell) != RTTI_AIRCRAFT)
//								{
//									v100 = cell->House;
//									*&cell[1].r.m.o.__AmbientSoundController2.field_10 = *&v120
//										* *&cell[1].r.m.o.__AmbientSoundController2.field_10;
//									if (v100->IsPlayerControl)
//									{
//										HIBYTE(nPlaySound) = 1;
//									}
//								}
//							}
//						}
//						damage = (damage + 1);
//					}
//					while (damage < DisplayClass::Layer[2].ActiveCount);
//					if (HIBYTE(nPlaySound))
//					{
//						VoxClass_Speak_From_Name("EVA_UnitSpeedUpgraded", -1, -1);
//					}
//				}
//				if (!HouseClass::Player_Has_Control(object->t.House))
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v123 = this->Position;
//				v101 = (v123 << 8) + 128;
//				v102 = (SHIWORD(v123) << 8) + 128;
//				v120.X = 128;
//				v120.Y = 128;
//				v139[2] = CellClass::Get_Z_Offset(this, &v120);
//				v139[0] = v101;
//				v139[1] = v102;
//				v32 = Rule->CrateSpeedSound;
//				HIDWORD(n) = 0;
//				v33 = v139;
//				goto LABEL_222;
//			case CrateType::FIREPOWER:
//				Debug::Log("Crate at %d,%d contains firepower\n", this->Position.X, this->Position.Y);
//				damage = 0;
//				if (DisplayClass::Layer[2].ActiveCount > 0)
//				{
//					do
//					{
//						cell = DisplayClass::Layer[2].Vector_Item[damage];
//						if (cell)
//						{
//							HIBYTE(v116) = cell->r.m.o.a.TargetBitfield[0] & 1;
//							if (HIBYTE(v116))
//							{
//								v123 = this->Position;
//								v125.X = 128;
//								v125.Y = 128;
//								v129 = CellClass::Get_Z_Offset(this, &v125);
//								v103 = cell->r.m.o.a.vftable->t.r.m.o.a.Center_Coord(cell, v154);
//								v104 = v103->Z;
//								v122.X = (v123 << 8) + 128 - v103->X;
//								v122.Y = (SHIWORD(v123) << 8) + 128 - v103->Y;
//								v122.Z = v129 - v104;
//								if (FastMath::Sqrt(v122.X * v122.X + (v129 - v104) * (v129 - v104) + v122.Y * v122.Y) < Rule->CrateRadius
//								  && !LODWORD(cell->FirepowerBias)
//								  && HIDWORD(cell->FirepowerBias) == 07774000000)
//								{
//									v105 = cell->r.m.o.a.vftable;
//									cell->FirepowerBias = *&v120 * cell->FirepowerBias;
//									v106 = (v105->t.r.m.o.a.Owner__Owning_House)();
//									if (HouseClass::Player_Has_Control(v106))
//									{
//										HIBYTE(nPlaySound) = 1;
//									}
//								}
//							}
//						}
//						damage = (damage + 1);
//					}
//					while (damage < DisplayClass::Layer[2].ActiveCount);
//					if (HIBYTE(nPlaySound))
//					{
//						VoxClass_Speak_From_Name("EVA_UnitFirePowerUpgraded", -1, -1);
//					}
//				}
//				if (!HouseClass::Player_Has_Control(object->t.House))
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v123 = this->Position;
//				v107 = (v123 << 8) + 128;
//				v108 = (SHIWORD(v123) << 8) + 128;
//				v120.X = 128;
//				v120.Y = 128;
//				v128.Z = CellClass::Get_Z_Offset(this, &v120);
//				v128.X = v107;
//				v128.Y = v108;
//				v32 = Rule->CrateFireSound;
//				HIDWORD(n) = 0;
//				v33 = &v128;
//				goto LABEL_222;
//			case CrateType::ICBM:
//				Debug::Log("Crate at %d,%d contains ICBM\n", this->Position.X, this->Position.Y);
//				v87 = SuperWeaponTypeClass::From_Action(20);
//				v88 = 0;
//				v89 = object->t.House;
//				v90 = v89->SuperWeapon.ActiveCount;
//				if (v90 <= 0)
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v123 = v89->SuperWeapon.Vector_Item;
//				v91 = v123;
//				while ((*v91)->Class->__ActsLike)
//				{
//					++v88;
//					++v91;
//					if (v88 >= v90)
//					{
//						goto switch_Goodie_Check_DoAnimation;
//					}
//				}
//				if (v88 != -1 && SuperClass::Enable(v123[v87->ID], 1, 0, 0) && object->t.IsOwnedByPlayer)
//				{
//					SidebarClass::Add(&Map.sc.t.sb, RTTI_SPECIAL, v87->ID);
//				}
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::VETERAN:
//				Debug::Log("Crate at %d,%d contains veterancy(TM)\n", this->Position.X, this->Position.Y);
//				for (cell = 0; cell < DisplayClass::Layer[2].ActiveCount; cell = (cell + 1))
//				{
//					damage = DisplayClass::Layer[2].Vector_Item[cell];
//					if (damage)
//					{
//						if (damage->t.r.m.o.IsDown)
//						{
//							HIBYTE(v116) = damage->t.r.m.o.a.TargetBitfield[0] & 1;
//							if (HIBYTE(v116))
//							{
//								v123 = this->Position;
//								v125.X = 128;
//								v125.Y = 128;
//								v129 = CellClass::Get_Z_Offset(this, &v125);
//								v75 = damage->t.r.m.o.a.vftable->t.r.m.o.a.Center_Coord(damage, &v148);
//								v76 = v75->Z;
//								v122.X = (v123 << 8) + 128 - v75->X;
//								v122.Y = (SHIWORD(v123) << 8) + 128 - v75->Y;
//								v122.Z = v129 - v76;
//								if (FastMath::Sqrt((v129 - v76) * (v129 - v76) + v122.Y * v122.Y + v122.X * v122.X) < Rule->CrateRadius)
//								{
//									if (damage->t.r.m.o.a.vftable->t.r.m.o.Techno_Type_Class(&damage->t)->Trainable)
//									{
//										v77 = 0;
//										if (*&v120 > 0.0)
//										{
//											v78 = &damage->t.__Veterancy;
//											do
//											{
//												if (VeterancyClass::Is_Veteran(v78))
//												{
//													VeterancyClass::Set_Elite(v78, 1);
//												}
//												if (VeterancyClass::Is_Rookie(v78))
//												{
//													VeterancyClass::Set_Veteran(v78, 1);
//												}
//												if (VeterancyClass::Is_Dumbass(v78))
//												{
//													VeterancyClass::Set_Rookie(v78, 1);
//												}
//												v123 = ++v77;
//											}
//											while (v77 < *&v120);
//										}
//									}
//								}
//							}
//						}
//					}
//				}
//				if (!HouseClass::Player_Has_Control(object->t.House))
//				{
//					goto switch_Goodie_Check_DoAnimation;
//				}
//				v123 = this->Position;
//				v79 = (v123 << 8) + 128;
//				v80 = (SHIWORD(v123) << 8) + 128;
//				v120.X = 128;
//				v120.Y = 128;
//				v134[2] = CellClass::Get_Z_Offset(this, &v120);
//				v134[0] = v79;
//				v134[1] = v80;
//				v32 = Rule->CratePromoteSound;
//				HIDWORD(n) = 0;
//				v33 = v134;
//				goto LABEL_222;
//			case CrateType::GAS:
//				Debug::Log("Crate at %d,%d contains poison gas\n", this->Position.X, this->Position.Y);
//				v18 = WarheadTypeClass::From_Name("GAS");
//				v19 = this->a.vftable;
//				n = 0.0;
//				v114 = v18;
//				v113 = 0;
//				damage = *&v120;
//				v20 = v19->t.r.m.o.a.Center_Coord(this, &v144);
//				Explosion_Damage(v20, damage, 0, v18, 0, 0);
//				v21 = 0;
//				v22 = 1;
//				do
//				{
//					if (v22)
//					{
//						cell = this->Position;
//						v125.X = *Adjacent_Cell(&v123, &cell, v21);
//						v23 = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &v125);
//					}
//					else
//					{
//						v23 = this;
//					}
//					v24 = v23->a.vftable;
//					n = 0.0;
//					v114 = v18;
//					v113 = 0;
//					v25 = v24->t.r.m.o.a.Center_Coord(v23, &v153);
//					Explosion_Damage(v25, damage, 0, v18, 0, 0);
//					v22 = ++v21 < 8;
//				}
//				while (v21 < 8);
//				goto switch_Goodie_Check_DoAnimation;
//			case CrateType::TIBERIUM:
//				Debug::Log("Crate at %d,%d contains tiberium\n", this->Position.X, this->Position.Y);
//				v26 = Random2Class::operator()(&Scen->RandomNumber, 0, *(&Tiberiums + 4) - 1);
//				if (v26 == 1)
//				{
//					v26 = 0;
//				}
//				CellClass::Place_Tiberium_At_Cell(this, v26, 1);
//				v27 = Random2Class::operator()(&Scen->RandomNumber, 10, 20);
//				if (v27)
//				{
//					v28 = v27;
//					do
//					{
//						*&n = Random2Class::operator()(&Scen->RandomNumber, 0, 768) | 0x100000000i64;
//						v29 = this->a.vftable->t.r.m.o.a.Center_Coord(this, &v147);
//						v30 = Coord_Scatter(&retval, v29, SLODWORD(n), 1);
//						v122.X = v30->X;
//						*&n = v26 | 0x100000000i64;
//						v122.Y = v30->Y;
//						v122.Z = v30->Z;
//						v31 = MapClass::operator[](&Map.sc.t.sb.p.r.d.m, &v122);
//						CellClass::Place_Tiberium_At_Cell(v31, v26, 1);
//						--v28;
//					}
//					while (v28);
//				}
//				goto switch_Goodie_Check_DoAnimation;
//			default:
//				goto switch_Goodie_Check_DoAnimation;
//			}
//		}
//	}
//
//	return true;
//}

DEFINE_HOOK(0x69A797, Game_DisableNoDigestLog, 0x6)
{
	return 0x69A937;
}

DEFINE_HOOK(0x6F9F42, TechnoClass_AI_Berzerk_SetMissionAfterDone, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::SetMissionAfterBerzerk(pThis);
	return 0x6F9F6E;
}
//
//DEFINE_HOOK(0x70F8D8, TechnoClass_GoBerzerkFor_SetMission, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	const Mission nMission = !pThis->IsArmed() ? Mission::Sleep : Mission::Hunt;
//	pThis->QueueMission(nMission, false);
//
//	return 0x70F8E6;
//}