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

#include <Misc/AresData.h>

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <IsometricTileTypeClass.h>

#include <TiberiumClass.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <Memory.h>

#include <Locomotor/Cast.h>
#pragma endregion

DEFINE_HOOK(0x6FA2C7, TechnoClass_AI_DrawBehindAnim, 0x8) //was 4
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(Point2D, nPoint, STACK_OFFS(0x78, 0x50));
	GET_STACK(RectangleStruct, nBound, STACK_OFFS(0x78, 0x50));

	if (const auto pBld = specific_cast<BuildingClass*>(pThis))
		if (BuildingExt::ExtMap.Find(pBld)->LimboID != -1)
			return 0x6FA2D8;

	const auto pType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if(pType->IsDummy)
		return 0x6FA2D8;

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
// DEFINE_JUMP(LJMP, 0x546C8B, 0x546CBF);

DEFINE_HOOK(0x74C8FB, VeinholeMonsterClass_CTOR_SetArmor, 0x6)
{
	GET(VeinholeMonsterClass*, pThis, ESI);
	GET(TerrainTypeClass* const, pThisTree, EDX);

	auto pType = pThis->GetType();
	if (pType && pThisTree)
		pType->Armor = pThisTree->Armor;

	return 0x0;
}

// thse were removed to completely disable vein
//DEFINE_HOOK(0x74D376, VeinholeMonsterClass_AI_TSRandomRate_1, 0x6)
//{
//	GET(RulesClass* const, pRules, EAX);
//
//	const auto nRand = pRules->VeinholeShrinkRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeShrinkRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeShrinkRate + nRand);
//	return 0x74D37C;
//}
//
//DEFINE_HOOK(0x74C5E1, VeinholeMonsterClass_CTOR_TSRandomRate, 0x6)
//{
//	GET(RulesClass* const, pRules, EAX);
//	const auto nRand = pRules->VeinholeGrowthRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeGrowthRate + nRand);
//	return 0x74C5E7;
//}
//
//DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
//{
//	GET(RulesClass* const, pRules, ECX);
//
//	const auto nRand = pRules->VeinholeGrowthRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeGrowthRate + nRand);
//	return 0x74D2AA;
//}

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
	const auto pRules = RulesExt::Global();
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

// TODO : this breaking deploy fire thing
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
//	const auto pType = pThis->GetTechnoType();
//	if (pType->AttackFriendlies && !pType->DeployFire)
//	{
//		return ExtendChecks;
//	}
//
//	return ClearTarget;
//}

DEFINE_HOOK_AGAIN(0x46684A, BulletClass_AI_TrailerInheritOwner, 0x5)
DEFINE_HOOK(0x466886, BulletClass_AI_TrailerInheritOwner, 0x5)
{
	GET(BulletClass* const, pThis, EBP);
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
	enum { CreateMuzzleAnim = 0x6FF39C, SkipCreateMuzzleAnim = 0x6FF43F };

	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET(AnimTypeClass* const, pMuzzleAnimType, EDI);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		if (auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord))
		{
			AnimExt::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
			if (!Is_Building(pThis))
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

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

	return Is_Building(pThis) ? AdjustCoordsForBuilding : Goto2NdCheck;
}

#pragma region WallTower
DEFINE_HOOK(0x4405C1, BuildingClas_Unlimbo_WallTowers_A, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x4405CF : 0x440606;
}

DEFINE_HOOK(0x440F66, BuildingClass_Unlimbo_WallTowers_B, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x440F78 : 0x44104D;
}

DEFINE_HOOK(0x44540D, BuildingClass_ExitObject_WallTowers, 0x5)
{
	GET(BuildingClass* const, pThis, EDI);
	R->EDX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445424 : 0x4454D4;
}

DEFINE_HOOK(0x445ADB, BuildingClass_Limbo_WallTowers, 0x9)
{
	GET(BuildingClass* const, pThis, ESI);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x445AED : 0x445B81;
}

DEFINE_HOOK(0x4514F9, BuildingClass_AnimLogic_WallTowers, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	R->ECX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x45150B : 0x4515E9;
}

DEFINE_HOOK(0x45EF11, BuildingClass_FlushForPlacement_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EBX);
	R->EDX(RulesClass::Instance());
	const auto& Nvec = RulesExt::Global()->WallTowers;
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
	const auto& Nvec = RulesExt::Global()->WallTowers;

	if (PlacingObject)
	{
		const bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;

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

DEFINE_HOOK(0x4FE546, BuildingClass_AI_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis) ? 0x4FE554 : 0x4FE6E7;
}

DEFINE_HOOK(0x4FE648, HouseClss_AI_Building_WallTowers, 0x6)
{
	GET(int const, nNodeBuilding, EAX);
	const auto& Nvec = RulesExt::Global()->WallTowers;

	if (nNodeBuilding == -1 || Nvec.empty())
		return 0x4FE696;

	return std::any_of(Nvec.begin(), Nvec.end(),
		[&](BuildingTypeClass* const pWallTower) { return pWallTower->ArrayIndex == nNodeBuilding; })
		? 0x4FE656 : 0x4FE696;
}

DEFINE_HOOK(0x5072F8, HouseClass_506EF0_WallTowers, 0x6)
{
	GET(BuildingTypeClass* const, pThis, EAX);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50735C : 0x507306;
}

DEFINE_HOOK(0x50A96E, HouseClass_AI_TakeOver_WallTowers_A, 0x6)
{
	GET(BuildingTypeClass* const, pThis, ECX);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis) ? 0x50A980 : 0x50AB90;
}

DEFINE_HOOK(0x50A9D2, HouseClass_AI_TakeOver_WallTowers_B, 0x6)
{
	GET(BuildingClass* const, pThis, EBX);
	R->EAX(pThis->Type);
	const auto& Nvec = RulesExt::Global()->WallTowers;
	return Nvec.Contains(pThis->Type) ? 0x50A9EA : 0x50AB3D;
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
	TechnoExt::PlayAnim(RulesClass::Instance->Wake, pDrive->LinkedTo);
	return 0x4B0828;
}

DEFINE_HOOK(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion* const, pLoco, ESI);
	const auto pShip = static_cast<ShipLocomotionClass* const>(pLoco);
	TechnoExt::PlayAnim(RulesClass::Instance->Wake, pShip->LinkedTo);
	return 0x69FEF0;
}

DEFINE_HOOK(0x414EAA, AircraftClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(AircraftClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0x24));

	GameConstruct(pAnim, TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x414ED0;
}

DEFINE_HOOK(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(UnitClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	GameConstruct(pAnim, TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x7365BB;
}

DEFINE_HOOK(0x738703, UnitClass_Explode_ExplodeAnim, 0x5)
{
	GET(AnimTypeClass*, pExplType, EDI);
	GET(UnitClass*, pThis, ESI);

	if (pExplType)
	{
		if (auto pAnim = GameCreate<AnimClass>(pExplType, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
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

	const auto nRandType = ScenarioClass::Instance->Random.Random() % pThis->Type->Explosion.Count;
	const auto nDelay = ScenarioClass::Instance->Random.RandomFromMax(3);
	CoordStruct nLoc { X , Y , Z + zAdd };
	if (auto const pType = pThis->Type->Explosion.GetItemOrDefault(nRandType))
	{
		if (auto pAnim = GameCreate<AnimClass>(pType, nLoc, nDelay, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	}

	return 0x441A24;
}

DEFINE_HOOK(0x441AC4, BuildingClass_Destroy_Fire3Anim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x64 - 0x54);

	if (auto pType = AnimTypeClass::Find("FIRE3"))
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomRanged(1, 3);
		if (auto pAnim = GameCreate<AnimClass>(pType, pCoord, nDelay + 3, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	}

	return 0x441B1F;
}

DEFINE_HOOK(0x441D1F, BuildingClass_Destroy_DestroyAnim, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
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
	GameConstruct(pAnim, RulesClass::Instance->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
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
	GameConstruct(pAnim, pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim,
	 args.Attacker ?
	 args.Attacker->GetOwningHouse() : args.SourceHouse,
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
	enum { Skip = 0x70383C, CheckIsSelected = 0x703828 };

	return R->ESI<TechnoClass*>()->Owner->IsControlledByHuman()
		? Skip : CheckIsSelected;
}

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum { ContinueCheck = 0x6FC23A, BypassCheck = 0x6FC24D };
	GET(TechnoClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const bool Allow = RulesExt::Global()->AutoAttackICedTarget.Get() || pThis->Owner->ControlledByPlayer();
	return pTypeExt->AllowFire_IroncurtainedTarget.Get(Allow)
		? BypassCheck : ContinueCheck;
}

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

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->PassiveAcquire_AI.isset()
		&& pThis->Owner
		&& !pThis->Owner->IsControlledByHuman()
		&& !pThis->Owner->IsNeutral())
	{
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
//	R->EAX(int(nAvailMoney * mult));
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

	GET(TechnoClass* const, pTarget, ESI);
	GET(TechnoTypeClass* const, pTargetType, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (pTypeExt->AI_LegalTarget.isset())
	{
		if (pTarget->Owner && pTarget->Owner->IsControlledByHuman())
			return Continue;

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
//	if (!MapClass::Instance->IsValidCell(nCell))
//	{
//		Debug::Log("Tiberium Growth With Invalid Cell ,Skipping !\n");
//		return increment;
//	}
//
//	R->EAX(MapClass::Instance->GetCellAt(nCell));
//	return SetCell;
//}

//TaskForces_LoadFromINIList_WhySwizzle , 0x5
//DEFINE_JUMP(LJMP, 0x6E8300, 0x6E8315)

DEFINE_HOOK(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
{
	enum { Skip = 0x4DC1A0, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

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

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

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

DEFINE_HOOK(0x4DBDB6, FootClass_IsCloakable_CloakMove, 0x6)
{
	enum { Continue = 0x0, ReturnFalse = 0x4DBDEB };
	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CloakMove.isset())
	{
		return pTypeExt->CloakMove.Get() && !pThis->Locomotor.GetInterfacePtr()->Is_Moving() ?
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
	return R->ESI<InfantryClass* const>()->Type->IdleRate == -1 ? 0x51D0A0 : 0x0;
}

static void ClearShit(TechnoTypeClass* a1)
{
	const auto pObjectToSelect = MapClass::Instance->NextObject(
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

DEFINE_HOOK_AGAIN(0x6FF933, TechnoClass_FireAt_End, 0x5)
DEFINE_HOOK(0x6FDE05, TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (TechnoExt::ExtMap.Find(pThis)->DelayedFire_DurationTimer <= 0)
	{
		//this may crash the game , since the object got deleted after killself ,..
		if (pWeaponExt->RemoveTechnoAfterFiring.Get())
			TechnoExt::KillSelf(pThis, KillMethod::Vanish);
		else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
			TechnoExt::KillSelf(pThis, KillMethod::Explode);
	}

	return 0;
}

DEFINE_HOOK(0x6FA232, TechnoClass_AI_LimboSkipRocking, 0xA)
{
	return !R->ESI<TechnoClass* const>()->InLimbo ? 0x0 : 0x6FA24A;
}

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

DEFINE_HOOK(0x701AAD, TechnoClass_ReceiveDamage_WarpedOutBy_Add, 0xA)
{
	enum { NullifyDamage = 0x701AC6, ContinueCheck = 0x701ADB };

	GET(TechnoClass* const, pThis, ESI);
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

DEFINE_HOOK(0x73DDC0, UnitClass_Mi_Unload_DeployIntoSpeed, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

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

DEFINE_HOOK(0x4DA64D, FootClass_Update_IsInPlayField, 0x6)
{
	GET(UnitTypeClass* const, pType, EAX);
	return pType->BalloonHover || pType->JumpJet ? 0x4DA655 : 0x4DA677;
}

DEFINE_HOOK(0x51D45B, InfantryClass_Scatter_Process, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);

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

//DEFINE_HOOK(0x6B721F, SpawnManagerClass_Manage_Clear, 0x6)
//{
//	GET(SpawnManagerClass*, pThis, ESI);
//	pThis->Target = nullptr;
//	pThis->NewTarget = nullptr;
//	pThis->Status = SpawnManagerStatus::Idle;
//	return 0x0;
//}

DEFINE_HOOK(0x62A915, ParasiteClass_CanInfect_Parasiteable, 0xA)
{
	enum { continuecheck = 0x62A929, returnfalse = 0x62A976, continuecheckB = 0x62A933 };
	GET(FootClass* const, pVictim, ESI);

	if (TechnoExt::IsParasiteImmune(pVictim))
		return returnfalse;

	if (pVictim->IsIronCurtained())
		return returnfalse;

	if (pVictim->IsBeingWarpedOut())
		return returnfalse;

	if (TechnoExt::IsChronoDelayDamageImmune(pVictim))
		return returnfalse;

	return !pVictim->BunkerLinkedItem ? continuecheckB : returnfalse;
}

DEFINE_HOOK(0x6A78F6, SidebarClass_AI_RepairMode_ToggelPowerMode, 0x9)
{
	GET(SidebarClass* const, pThis, ESI);

	if (Phobos::Config::TogglePowerInsteadOfRepair)
		pThis->SetTogglePowerMode(-1);
	else
		pThis->SetRepairMode(-1);

	return 0x6A78FF;
}

DEFINE_HOOK(0x6A7AE1, SidebarClass_AI_DisableRepairButton_TogglePowerMode, 0x6)
{
	GET(SidebarClass* const, pThis, ESI);
	R->AL(Phobos::Config::TogglePowerInsteadOfRepair ? pThis->PowerToggleMode : pThis->RepairMode);
	return 0x6A7AE7;
}

DEFINE_HOOK(0x508CE6, HouseClass_UpdatePower_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
		0x508CEE : 0x0;
}

DEFINE_HOOK(0x508EE5, HouseClass_UpdateRadar_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EAX);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
		0x508EEF : 0x0;
}

DEFINE_HOOK(0x508FCE, HouseClass_SpySat_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, ECX);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
		0x508FE1 : 0x0;
}

DEFINE_HOOK(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
{
	enum { Continue = 0x0, DoNotDrawRadar = 0x70D407 };

	GET(TechnoClass* const, pThis, ESI);

	if (Is_Building(pThis))
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1)
		{
			return DoNotDrawRadar;
		}
	}

	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsDummy ?
		DoNotDrawRadar : Continue;
}

//DEFINE_HOOK(0x663225, RocketLocomotionClass_DetonateOnTarget_Anim, 0x6)
//{
//	GET(AnimClass*, pMem, EAX);
//	GET(RocketLocomotionClass* const, pThis, ESI);
//	REF_STACK(CellStruct const, nCell, STACK_OFFS(0x60, 0x38));
//	REF_STACK(CoordStruct const, nCoord, STACK_OFFS(0x60, 0x18));
//	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0x60, 0x50));
//
//	GET(int, nDamage, EDI);
//
//	const auto pCell = MapClass::Instance->GetCellAt(nCell);
//	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, pCell ? pCell->LandType : LandType::Clear, nCoord))
//	{
//		GameConstruct(pMem, pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false);
//		AnimExt::SetAnimOwnerHouseKind(pMem, pThis->LinkedTo->GetOwningHouse(), pThis->LinkedTo->Target ? pThis->LinkedTo->Target->GetOwningHouse() : nullptr, nullptr, false);
//	}
//	else
//	{
//		//no constructor called , so it is safe to delete the allocated memory
//		GameDelete<false, false>(pMem);
//	}
//
//	return 0x66328C;
//}

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
	GET_STACK(HouseClass* const, pHouse, STACK_OFFS(0x8, -0x4));
	R->EDI(pHouse);
	return 0x6F09D5;
}

DEFINE_HOOK(0x6F0A3F, TeamTypeClass_CreateOneOf_CreateLog, 0x6)
{
	GET(TeamTypeClass* const, pThis, ESI);
	GET(HouseClass* const, pHouse, EDI);
	Debug::Log("[%x][%s] Creating a new team named '%s'.\n", pHouse, pHouse ? pHouse->get_ID() : NONE_STR2, pThis->ID);
	R->EAX(YRMemory::Allocate(sizeof(TeamClass)));
	return 0x6F0A5A;
}

DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);
//DEFINE_HOOK(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5) { return 0x44DE3C; }

DEFINE_HOOK(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
{
	GET(FactoryClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);
	Debug::Log("[%x] Factory with Owner '%s' Abandoning production of '%s' \n", pThis, pThis->Owner ? pThis->Owner->get_ID() : NONE_STR2, pType->ID);
	R->ECX(pThis->Object);
	return 0x4CA021;
}

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

#endif


#endif

DEFINE_HOOK(0x5F54A8, ObjectClass_ReceiveDamage_ConditionYellow, 0x6)
{
	enum { ContinueCheck = 0x5F54C4, ResultHalf = 0x5F54B8 };

	GET(int const, nCurStr, EDX);
	GET(int const, nMaxStr, EBP);
	GET(int const, nDamage, ECX);

	const auto curstr = int(nMaxStr * RulesClass::Instance->ConditionYellow);
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

DEFINE_HOOK(0x6D912B, TacticalClass_Render_BuildingInLimboDeliveryA, 0x9)
{
	enum { Draw = 0x0, DoNotDraw = 0x6D9159 };

	GET(TechnoClass* const, pTechno, ESI);

	if (Is_Building(pTechno))
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

	GET(TechnoClass* const, pTechno, EBX);

	if (Is_Building(pTechno))
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

	GET(BuildingClass* const, pBuilding, EBX);
	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ? DoNotDraw : Draw;
}

static int AnimClass_Expired_SpawnsParticle(REGISTERS* R)
{
	GET(AnimClass*, pThis, ESI);
	GET(AnimTypeClass* const, pAnimType, EAX);
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
				const auto v13 = abs(ScenarioClass::Instance->Random.RandomRanged(nMin, nMax));
				const auto v10 = ScenarioClass::Instance->Random.RandomDouble() * v17 + v16;
				const auto v18 = std::cos(v10);
				const auto v9 = std::sin(v10);
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

//DEFINE_JUMP(LJMP, 0x517FF5, 0x518016);

DEFINE_HOOK(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x8)
{
	GET(UnitClass* const, pThis, EBP);
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

	auto amount_Riparious = int(nStorage_0 / nStorage * nMax + 0.5);
	auto amount_Cruentus = int(nStorage_1 / nStorage * nMax + 0.5);
	auto amount_Vinifera = int(nStorage_2 / nStorage * nMax + 0.5);
	auto amount_Aboreus = int(nStorage_3 / nStorage * nMax + 0.5);

	//const auto totalaccum = nStorage_3 + nStorage_2 + nStorage_0;
	//auto amount_a = int(totalaccum / pType->Storage * nMax + 0.5);
	//auto amount_b = int(nStorage_1 / pType->Storage * nMax + 0.5);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	Point2D nOffs {};
	const auto pBuilding = Is_Building(pTechno) ? static_cast<BuildingClass*>(pTechno) : nullptr;
	const auto pShape = pBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS2_SHP());

	ConvertClass* nPal = FileSystem::THEATER_PAL();

	if (pBuilding)
	{

		const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuildingTypeExt->PipShapes01Remap)
			nPal = pTechno->GetRemapColour();
		else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
			nPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	}
	else
	{
		nPal = FileSystem::THEATER_PAL();
	}

	auto GetFrames = [&]()
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

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

void DrawSpawnerPip(TechnoClass* pTechno, Point2D* nPoints, RectangleStruct* pRect, int nOffsetX, int nOffsetY)
{
	const auto pType = pTechno->GetTechnoType();
	const auto nMax = pType->SpawnsNumber;

	if (nMax <= 0)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	Point2D nOffs {};

	const auto pBuilding = specific_cast<BuildingClass*>(pTechno);
	const auto pShape = pBuilding ?
		pTypeExt->PipShapes01.Get(FileSystem::PIPS_SHP()) : pTypeExt->PipShapes02.Get(FileSystem::PIPS_SHP());

	ConvertClass* nPal = FileSystem::THEATER_PAL();

	if (pBuilding)
	{
		const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuildingTypeExt->PipShapes01Remap)
			nPal = pTechno->GetRemapColour();
		else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
			nPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();

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

	GET(TechnoClass* const, pThis, EBP);
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

enum class NewVHPScan : int
{
	None = 0,
	Normal = 1,
	Strong = 2,
	Threat = 3,
	Health = 4,
	Damage = 5,
	Value = 6,
	Locked = 7,
	Non_Infantry = 8,


	count
};

constexpr std::array<const char*, (size_t)NewVHPScan::count> NewVHPScanToString
{ {
	{"None" },
	{ "Normal" },
	{ "Strong" },
	{ "Threat" },
	{ "Health" },
	{ "Damage" },
	{ "Value" },
	{ "Locked" },
	{ "Non-Infantry" }
	} };

DEFINE_HOOK(0x4775F4, CCINIClass_ReadVHPScan_new, 0x5)
{
	GET(const char* const, cur, ESI);

	int vHp = 0;
	for (int i = 0; i < (int)NewVHPScanToString.size(); ++i)
	{
		if (IS_SAME_STR_(cur, NewVHPScanToString[i]))
		{
			vHp = i;
			break;
		}
	}

	R->EAX(vHp);
	return 0x4775E9;
}

DEFINE_HOOK(0x4775B0, CCINIClass_ReadVHPScan_ReplaceArray, 0x7)
{
	int nIdx = R->EDI<int>();
	nIdx = (nIdx > (int)NewVHPScanToString.size() ? (int)NewVHPScanToString.size() : nIdx);
	R->EDX(NewVHPScanToString[nIdx]);
	return 0x4775B7;
}

DEFINE_HOOK(0x6F8721, TechnoClass_EvalObject_VHPScan, 0x7)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(ObjectClass* const, pTarget, ESI);
	GET(int*, pRiskValue, EBP);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pTechnoTarget = generic_cast<TechnoClass* const>(pTarget);
	if (!pTechnoTarget)
		pTechnoTarget = nullptr;

	int nValue = pExt->VHPscan_Value;
	if (nValue <= 0)
		nValue = 2;

	switch (NewVHPScan(pExt->Get()->VHPScan))
	{
	case NewVHPScan::Normal:
	{
		if (pTarget->EstimatedHealth <= 0)
		{
			*pRiskValue /= nValue;
			break;
		}

		if (pTarget->EstimatedHealth > (pTarget->GetType()->Strength / 2))
			break;

		*pRiskValue *= nValue;
	}
	break;
	case NewVHPScan::Threat:
	{
		*pRiskValue *= (*pRiskValue / nValue);
		break;
	}
	break;
	case NewVHPScan::Health:
	{
		int nRes = *pRiskValue;
		if (pTarget->EstimatedHealth > pTarget->GetType()->Strength / 2)
			nRes = nValue * nRes;
		else
			nRes = nRes / nValue;

		*pRiskValue = nRes;
	}
	break;
	case NewVHPScan::Damage:
	{
		if (!pTechnoTarget)
		{
			*pRiskValue = 0;
			break;
		}

		*pRiskValue = pTechnoTarget->CombatDamage(-1) / nValue * (*pRiskValue);
	}
	break;
	case NewVHPScan::Value:
	{
		if (!pTechnoTarget)
		{
			*pRiskValue = 0;
			break;
		}

		const int nSelectedWeapon = pTechnoTarget->SelectWeapon(pThis);
		const auto nFireError = pTechnoTarget->GetFireError(pThis, nSelectedWeapon, 0);
		if (nFireError == FireError::NONE ||
			nFireError == FireError::FACING ||
			nFireError == FireError::REARM ||
			nFireError == FireError::ROTATING
			)
		{
			*pRiskValue *= nValue;
			break;
		}

		*pRiskValue /= nValue;
	}
	break;
	case NewVHPScan::Non_Infantry:
	{
		if (!pTechnoTarget || Is_Infantry(pTechnoTarget))
		{
			*pRiskValue = 0;
		}
	}
	break;
	default:
		break;
	}

	return 0x6F875F;
}

DEFINE_HOOK(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7)
{
	GET(InfantryClass* const, pThis, ECX);

	enum { SkipWholeFunction = 0x5192BC, Continue = 0x0 };

	if (!pThis)
		return Continue;

	const auto pTypeExt = InfantryTypeExt::ExtMap.Find(pThis->Type);
	return pTypeExt->HideWhenDeployAnimPresent.Get() && pThis->DeployAnim
		? SkipWholeFunction : Continue;
}

DEFINE_HOOK(0x6F7261, TechnoClass_TargetingInRange_NavalBonus, 0x5)
{
	GET(int, nRangeBonus, EDI);
	GET(TechnoClass* const, pThis, ESI);
	GET(AbstractClass* const, pTarget, ECX);

	if (auto const pFoot = abstract_cast<FootClass* const>(pTarget))
	{
		const auto pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		if (pFoot->GetTechnoType()->Naval && pThisTypeExt->NavalRangeBonus.isset())
		{
			const auto pFootCell = pFoot->GetCell();
			if (pFootCell->LandType == LandType::Water && !pFootCell->ContainsBridge())
				nRangeBonus += pThisTypeExt->NavalRangeBonus.Get();
		}
	}

	R->EDI(nRangeBonus);
	return 0x0;
}

// Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250);

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
	TechnoExt::ApplyDrainMoney(pThis);
	return 0x6FA1C5;
}

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D90 };

	GET(ObjectClass* const, pThis, ECX);
	GET_STACK(TechnoClass* const, pTechno, STACK_OFFSET(0x8, -0x4));
	R->AL(TechnoExt::IsCrushable(pThis, pTechno));

	return SkipGameCode;
}

DEFINE_HOOK(0x629BB2, ParasiteClass_UpdateSquiddy_Culling, 0x8)
{
	GET(ParasiteClass* const, pThis, ESI);
	GET(WarheadTypeClass* const, pWH, EDI);

	enum { ApplyDamage = 0x629D19, GainExperience = 0x629BF3, SkipGainExperience = 0x629C5D };

	if (!WarheadTypeExt::ExtMap.Find(pWH)->ApplyCulling(pThis->Owner, pThis->Victim))
		return ApplyDamage;

	return pThis->Owner && pThis->Owner->Owner && pThis->Owner->Owner->IsAlliedWith(pThis->Victim)
		? SkipGainExperience : GainExperience;
}

DEFINE_HOOK(0x51A2EF, InfantryClass_PCP_Enter_Bio_Reactor_Sound, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);

	const auto nSound = BuildingTypeExt::ExtMap.Find(pBuilding->Type)
		->EnterBioReactorSound.Get(RulesClass::Instance->EnterBioReactorSound);

	VocClass::PlayIndexAtPos(nSound, pBuilding->GetCoords(), 0);

	return 0x51A30F;
}

DEFINE_HOOK(0x44DBBC, InfantryClass_PCP_Leave_Bio_Reactor_Sound, 0x7)
{
	GET(BuildingClass* const, pThis, EBP);

	const auto nSound = BuildingTypeExt::ExtMap.Find(pThis->Type)
		->LeaveBioReactorSound.Get(RulesClass::Instance->LeaveBioReactorSound);

	VocClass::PlayIndexAtPos(nSound, pThis->GetCoords(), 0);

	return 0x44DBDA;
}

DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x6)
DEFINE_HOOK(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if (const auto pBuilding = specific_cast<BuildingClass* const>(pThis))
	{
		if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->DisableDamageSound.Get())
		{
			return R->Origin() + 0x6;
		}
	}

	return 0x0;
}

DEFINE_JUMP(LJMP, 0x702765, 0x7027AE); // this just an duplicate

DEFINE_HOOK(0x4FB63A, HouseClass_PlaceObject_EVA_UnitReady, 0x5)
{
	GET(TechnoClass* const, pProduct, ESI);
	VoxClass::PlayIndex(TechnoTypeExt::ExtMap.Find(pProduct->GetTechnoType())->Eva_Complete.Get());
	return 0x4FB649;
}

DEFINE_HOOK(0x4FB7CA, HouseClass_RegisterJustBuild_CreateSound_PlayerOnly, 0x6) //9
{
	enum { ReturnNoVoiceCreate = 0x4FB804, Continue = 0x0 };

	GET(HouseClass* const, pThis, EDI);
	GET(TechnoClass* const, pTechno, EBP);

	if (pTechno)
	{
		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		pTechno->QueueVoice(pTechnoTypeExt->VoiceCreate);

		if (!pTechnoTypeExt->CreateSound_Enable.Get())
			return ReturnNoVoiceCreate;

		if (!EnumFunctions::IsPlayerTypeEligible((AffectPlayerType::Observer | AffectPlayerType::Player) , HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;

		if(!EnumFunctions::CanTargetHouse(pTechnoTypeExt->CreateSound_afect.Get(RulesExt::Global()->CreateSound_PlayerOnly) , pThis ,HouseClass::CurrentPlayer))
			return ReturnNoVoiceCreate;
	}

	return Continue;
}

DEFINE_HOOK(0x6A8E25, SidebarClass_StripClass_AI_Building_EVA_ConstructionComplete, 0x5)
{
	GET(TechnoClass* const, pTech, ESI);

	if (pTech && Is_Building(pTech)
	 && pTech->Owner
	 && pTech->Owner->ControlledByPlayer()) {
		VoxClass::PlayIndex(TechnoTypeExt::ExtMap.Find(pTech->GetTechnoType())->Eva_Complete.Get());
		return 0x6A8E34;
	}

	return 0x0;
}

// DEFINE_HOOK(0x518B98, InfantryClass_ReceiveDamage_UnInit, 0x8)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	// REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xD0, -0x4));
//
// 	// if (!InfantryExt::ExtMap.Find(pThis)->IsUsingDeathSequence && !pThis->Type->JumpJet) {
// 	// 	auto pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
// 	// 	if (!pWHExt->DeadBodies.empty()) {
// 	// 		if (AnimTypeClass* pSelected = pWHExt->DeadBodies.at(
// 	// 			ScenarioClass::Instance->Random.RandomFromMax(pWHExt->DeadBodies.size() - 1)))
// 	// 		{
// 	// 			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
// 	// 				AnimExt::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse, pThis->GetOwningHouse(), true);
// 	// 			}
// 	// 		}
// 	// 	}
// 	// }
//
// 	R->ECX(pThis);
// 	pThis->UnInit();
// 	return 0x518BA0;
// }

//TODO : Add PerTechnoOverride
DEFINE_HOOK(0x639DD8, TechnoClass_PlanningManager_DecideEligible, 0x5)
{
	enum { CanUse = 0x639DDD, ContinueCheck = 0x639E03 };
	GET(TechnoClass* const, pThis, ESI);
	return Is_Foot(pThis)
		? CanUse : ContinueCheck;
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

// DEFINE_HOOK(0x51DF82, InfantryClass_FireAt_StartReloading, 0x6)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	const auto pType = pThis->Type;
//
// 	if (pType->Ammo > 0 && pType->Ammo > pThis->Ammo && !pType->ManualReload 
// 		&& !pThis->ReloadTimer.HasStarted())
// 		pThis->StartReloading();
//
// 	return 0;
// }

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

DEFINE_SKIP_HOOK(0x69A797, Game_DisableNoDigestLog, 0x6, 69A937);

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

// tester says these make team completely stop protecting ToProtect 
// all of them instead of the skipped one , weird
// Infantry 
// continue 0x708239 , skip 0x7083BC
//DEFINE_HOOK(0x70822B, TechnoClass_ToProtectAttacked_Ignore_Infantry, 0x6)
//{
//	GET(InfantryClass*, pInf, ESI);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type);
//	return pTypeExt->IgnoreToProtect || pTypeExt->IsDummy 
//		? 0x7083BC : 0x0;
//}

// Unit 0x7086F5 
// recuit chance
// 0x708461
// continue 0x708461 , skip 0x708622
//DEFINE_HOOK(0x708461, TechnoClass_ToProtectAttacked_Ignore_Unit, 0x6)
//{
//	GET(UnitClass*, pUnit, ESI);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->Type);
//	return pTypeExt->IgnoreToProtect || pTypeExt->IsDummy
//		? 0x708622 : 0x0;
//}

DEFINE_HOOK(0x6FF4B0, TechnoClass_FireAt_TargetLaser, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);

	if (!pTypeExt->TargetLaser_WeaponIdx.empty()
		&& !pTypeExt->TargetLaser_WeaponIdx.Contains(pExt->CurrentWeaponIdx))
	{
		return 0x6FF4CC;
	}

	pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
	return 0x6FF4CC;
}

DEFINE_HOOK(0x4491D5, BuildingClass_ChangeOwnership_RegisterFunction, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->OrePurifier)
		++pThis->Owner->NumOrePurifiers;

	return 0x4491F1;
}

DEFINE_HOOK(0x448AB2, BuildingClass_ChangeOwnership_UnregisterFunction, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Type->OrePurifier)
		--pThis->Owner->NumOrePurifiers;

	return 0x448AC8;
}

DEFINE_HOOK(0x70FB50, TechnoClass_Bunkerable, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	bool ret = true;
	if (const auto pFoot = generic_cast<FootClass*>(pThis))
	{
		const auto pType = pFoot->GetTechnoType();
		if (!pType->Bunkerable || !pType->Turret)
			ret = false;

		if (!pFoot->IsArmed())
			ret = false;

		const auto nSpeedType = pType->SpeedType;
		if (nSpeedType == SpeedType::Hover
			|| nSpeedType == SpeedType::Winged
			|| nSpeedType == SpeedType::None)
			ret = false;

		if (pFoot->ParasiteEatingMe)
			ret = false;

		//crash the game , dont allow it
		//maybe because of force_track stuffs,..
		if (locomotion_cast<HoverLocomotionClass*>(pFoot->Locomotor))
			ret = false;
	}

	R->EAX(ret);
	return 0x70FBCA;
}

DEFINE_HOOK(0x40A5B3, AudioDriverStar_AnnoyingBufferLogDisable_A, 0x6)
{
	GET(void*, pAudioChannelTag, EBX);
	(*(uintptr_t*)((char*)pAudioChannelTag + 0x68)) = R->EAX();
	return 0x40A5C4;
}

DEFINE_HOOK(0x40A554, AudioDriverStar_AnnoyingBufferLogDisable_B, 0x6)
{
	GET(void*, pAudioChannelTag, EBX);
	LEA_STACK(DWORD*, ptr, STACK_OFFS(0x40, 0x28));
	(*(uintptr_t*)((char*)pAudioChannelTag + 0x6C)) = R->EAX();
	R->EDX(R->EAX());
	R->EAX(ptr);
	return 0x40A56C;
}

//#pragma optimize("", off )
//DEFINE_HOOK(0x722FC2, TiberiumClass_Grow_Validate, 0x5)
//{
//	GET(const TPriorityQueueClass<MapSurfaceData>*, pHeap, ECX);
//	Debug::Log("__FUNCTION__ , Tiberium Logic with HeapSize(%d) \n" , pHeap->HeapSize);
//	return 0x0;
//}
//#pragma optimize("", on)

//struct TechnoExt_Gscript
//{
//	CDTimerClass TargetingDelayTimer {};
//
//	static TechnoExt_Gscript* Get(TechnoClass* pThis)
//	{
//		return (TechnoExt_Gscript*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
//	}
//};

//#include <SpotlightClass.h>
//
//struct TechnoTypeExt_Gscript
//{
//	Valueable<bool> TroopCrawler {};
//	Promotable<bool> Promoted_PalaySpotlight { };
//	Promotable<SpotlightFlags> Promoted_PalaySpotlight_bit
//	{ SpotlightFlags::NoGreen | SpotlightFlags::NoRed,
//		SpotlightFlags::NoRed,
//		SpotlightFlags::NoGreen | SpotlightFlags::NoBlue
//	};
//
//	Promotable<AnimTypeClass*> Promoted_PlayAnim {};
//
//	ValueableIdx<VoxClass> DiscoverEVA { -1 };
//
//	static TechnoTypeExt_Gscript* Get(TechnoTypeClass* pThis)
//	{
//		return (TechnoTypeExt_Gscript*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
//	}
//
//	static void PlayPromoteAffects(TechnoClass* pThis)
//	{
//		auto const pTypeExt = pThis->GetTechnoType();
//		auto const pExt = TechnoTypeExt_Gscript::Get(pTypeExt);
//
//		if (pExt->Promoted_PalaySpotlight.Get(pThis))
//		{
//			if (auto pSpot = GameCreate<SpotlightClass>(pThis->Location, 50))
//			{
//				pSpot->DisableFlags = pExt->Promoted_PalaySpotlight_bit.Get(pThis);
//			}
//		}
//
//
//		if (auto pAnimType = pExt->Promoted_PlayAnim.Get(pThis))
//		{
//			GameCreate<AnimClass>(pAnimType, pThis->Location, 0, 1, 0x600u, 0, 0);
//		}
//	}
//};

//DEFINE_HOOK_AGAIN(0x736A22,UnitClass_UpdateTurret_ApplyTarget_ClearFlags, 0xA)
//DEFINE_HOOK(0x736A09, UnitClass_UpdateTurret_ApplyTarget_ClearFlags, 0x5)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	auto const pTechnoExt = TechnoExt_Gscript::Get(pThis);
//	pTechnoExt->TargetingDelayTimer.Start(ScenarioClass::Instance->Random.RandomRanged(30, 50));
//
//	return 0x0;
//}

//DEFINE_HOOK(0x7394C4, UnitClass_TryToDeploy_EmptyToPlace_Crawler, 0x7)
//{
//	GET(UnitTypeClass*, pType, EAX);
//
//	return TechnoTypeExt_Gscript::Get(pType)->TroopCrawler.Get() ?
//		0x73958A : 0x0;
//}

//DEFINE_HOOK(0x73F015, UnitClass_Mi_Hunt_MCVFindSpotReworked, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	const auto nMissionStatus = pThis->MissionStatus;
//	if (!nMissionStatus)
//	{
//		if (!pThis->GetHeight())
//		{
//			if (pThis->GotoClearSpot() && pThis->TryToDeploy())
//			{
//				pThis->MissionStatus = 1;
//				return 0x73F059;
//			}
//
//			pThis->Scatter(CoordStruct::Empty, true, false);
//		}
//
//		return 0x73F059;
//	}
//
//	if (nMissionStatus != 1)
//		return 0x73F059;
//
//	if (!pThis->Deploying)
//		pThis->MissionStatus = 0;
//
//	return 0x73F059;
//}

//DEFINE_HOOK(0x6F4974, TechnoClass_UpdateDiscovered_ByPlayer, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(HouseClass*, pDiscoverer, EDI);
//
//	const auto pHouseExt = HouseExt::ExtMap.Find(pDiscoverer);
//
//	if (!pHouseExt->DiscoverEvaDelay.IsTicking() || !pHouseExt->DiscoverEvaDelay.GetTimeLeft()){
//		pHouseExt->DiscoverEvaDelay.Start(200);
//
//		if (auto pTypeExt = TechnoTypeExt_Gscript::Get(pThis->GetTechnoType())) {
//			const auto nIdx = pTypeExt->DiscoverEVA.Get();
//
//			if (nIdx != -1)
//				VoxClass::PlayIndex(nIdx);
//		}
//	}
//
//	return 0x0;
//}

// DEFINE_HOOK(0x708F5E, TechnoClass_ResponseToSelect_PlaySound, 0xA)
// {
// 	GET(TechnoClass*, pThis, ESI);
//
// 	return 0x0;
// }

DEFINE_HOOK(0x708F77, TechnoClass_ResponseToSelect_BugFixes, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);

	return pThis->IsCrashing || pThis->Health < 0 ?
		0x708FAD : 0x0;
}

//DEFINE_HOOK(0x518077, InfantryClass_ReceiveDamage_ResultDestroyed, 0x6)
//{
//	return 0x0;
//}

DEFINE_HOOK(0x6D7A4F, TacticalClass_DrawPixelEffects_FullFogged, 0x6)
{
	GET(CellClass* const, pCell, ESI);

	return pCell->IsFogged() ? 0x6D7BB8 : 0x0;
}

DEFINE_HOOK(0x6EE17E, MoveCrameraToWaypoint_CancelFollowTarget, 0x8)
{
	DisplayClass::Instance->FollowAnObject(nullptr);
	return 0x0;
}

#include <Ext/SWType/NewSuperWeaponType/SWStateMachine.h>

DEFINE_HOOK(0x55AFB3, LogicClass_Update_Early, 0x6)
{
	SWStateMachine::UpdateAll();
	return 0x0;
}

//bool CheckDone = false;
//
//DEFINE_HOOK(0x55B582, LogicClass_Update_AfterTeamClass, 0x6)
//{
//	// Uninited techno still playing `EMPulseSparkle` anim ,..
//
//	//if (!CheckDone)
//	//{
//	//	if (SessionClass::Instance->GameMode != GameMode::Campaign && SessionClass::Instance->GameMode != GameMode::Skirmish)
//	//	{
//	//		HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->Seed = Random2Class::Seed();
//	//
//	//		Debug::Log("Scenario Name [%s] , Map Name [%s] \n", ScenarioClass::Instance->FileName, SessionClass::Instance->ScenarioFilename);
//	//		for (auto const& it : *HouseClass::Array)
//	//		{
//	//			auto pExt = HouseExt::ExtMap.TryFind(it);
//	//
//	//			Debug::Log("Player Name: %s IsCurrentPlayer: %u; ColorScheme: %s; ID: %d; HouseType: %s; Edge: %d; StartingAllies: %u; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d Seed: %d\n",
//	//			it->PlainName ? it->PlainName : NONE_STR,
//	//			it->IsHumanPlayer,
//	//			ColorScheme::Array->GetItem(it->ColorSchemeIndex)->ID,
//	//			it->ArrayIndex,
//	//			HouseTypeClass::Array->GetItem(it->Type->ArrayIndex)->Name,
//	//			it->Edge,
//	//			it->StartingAllies.data,
//	//			it->StartingCell.X,
//	//			it->StartingCell.Y,
//	//			it->Visionary,
//	//			it->MapIsClear,
//	//			it->Available_Money(),
//	//			pExt->Seed
//	//			);
//	//
//	//			if (it != HouseClass::CurrentPlayer && pExt->Seed != -1 && pExt->Seed != Random2Class::Seed())
//	//			{
//	//				Debug::FatalError("Player %s with currentPlayer , have different random2 seeds ! \n", it->PlainName, HouseClass::CurrentPlayer->PlainName);
//	//			}
//	//		}
//	//
//	//		CheckDone = true;
//	//	}
//	//}
//
//	for (auto pAnim : *AnimClass::Array)
//	{
//		if (pAnim->IsAlive)
//		{
//			if (pAnim->OwnerObject && !pAnim->OwnerObject->IsAlive)
//				pAnim->TimeToDie = true;
//			else if (pAnim->InLimbo)
//				pAnim->TimeToDie = true;
//		}
//	}
//
//	return 0x0;
//}

#include <Ext/TerrainType/Body.h>

// DEFINE_HOOK(0x73B002, UnitClass_UpdatePosition_CrusherTerrain, 0x6)
// {
// 	GET(UnitClass*, pThis, EBP);
// 	GET(CellClass* const, pCell, EDI);
//
// 	if (const auto pTerrain = pCell->GetTerrain(false))
// 	{
// 		if (pTerrain->IsAlive)
// 		{
// 			const auto pType = pTerrain->Type;
// 			if (!pType->SpawnsTiberium &&
// 				!pType->Immune &&
// 				!TerrainTypeExt::ExtMap.Find(pType)->IsPassable &&
// 				pTerrain->Type->Crushable
// 				)
// 			{
// 				if (TechnoTypeExt::ExtMap.Find(pThis->Type)->CrushLevel.Get(pThis) >
// 					TerrainTypeExt::ExtMap.Find(pType)->CrushableLevel)
// 				{
// 					VocClass::PlayIndexAtPos(pType->CrushSound, pThis->Location);
// 					pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pThis, true, true, pThis->Owner);
//
// 					if (pThis->Type->TiltsWhenCrushes)
// 						pThis->RockingForwardsPerFrame += 0.02f;
//
// 					pThis->iscrusher_6B5 = false;
// 				}
// 			}
// 		}
// 	}
//
// 	R->EAX(pCell->OverlayTypeIndex);
// 	return pCell->OverlayTypeIndex != -1 ? 0x73B00A : 0x73B074;
// }

// 5B1020 , for mechLoco
// 5B1404 , for mechLoco
// 6A1025 , for shipLoco
// DEFINE_HOOK(0x4B1999, DriveLocomotionClass_4B0F20_CrusherTerrain, 0x6)
// {
// 	GET(DriveLocomotionClass*, pLoco, EBP);
// 	GET(CellClass* const, pCell, EBX);
//
// 	const auto pLinkedTo = pLoco->LinkedTo;
// 	if (const auto pTerrain = pCell->GetTerrain(false))
// 	{
// 		if (pTerrain->IsAlive)
// 		{
// 			const auto pType = pTerrain->Type;
// 			if (!pType->SpawnsTiberium &&
// 				!pType->Immune &&
// 				!TerrainTypeExt::ExtMap.Find(pType)->IsPassable &&
// 				pTerrain->Type->Crushable
// 				)
// 			{
// 				if (TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType())->CrushLevel.Get(pLinkedTo) >
// 					TerrainTypeExt::ExtMap.Find(pType)->CrushableLevel)
// 				{
// 					VocClass::PlayIndexAtPos(pType->CrushSound, pLinkedTo->Location);
// 					pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pLinkedTo, true, true, pLinkedTo->Owner);
//
// 					if (pLinkedTo->GetTechnoType()->TiltsWhenCrushes)
// 						pLinkedTo->RockingForwardsPerFrame = -0.05f;
// 				}
// 			}
// 		}
// 	}
//
// 	R->EAX(pCell->OverlayTypeIndex);
// 	return pCell->OverlayTypeIndex != -1 ? 0x4B19A1 : 0x4B1A04;
// }

// this shit is something fuckup check 
// it check if not unit then check if itself is not infantry is building 
// what event shit is this
//DEFINE_HOOK(0x6FA697, TechnoClass_UpdateTarget_ShouldReTarget , 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (!Is_Unit(pThis))
//		return 0x0;
//
//	bool bConditionMet = false;
//	switch (pThis->CurrentMission)
//	{
//	case Mission::Move:
//	case Mission::Guard:
//	case Mission::Harvest:
//	case Mission::Return:
//		return 0x6FA6AC;
//	case Mission::Sabotage:
//	{
//		if (!Is_Infantry(pThis))
//			return 0x6FA6AC;
//
//		break;
//	}
//	case Mission::Missile:
//		bConditionMet = !Is_Building(pThis);
//		break;
//	}
//
//	return bConditionMet ? 0x6FA6AC : 0x6FA6F5;
//}

//void PlayReloadEffects(TechnoClass* pThis, int report, int sound)
//{
//	VocClass::PlayIndexAtPos(report, pThis->Location);
//	VocClass::PlayIndexAtPos(sound, pThis->Location);
//}

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

//DEFINE_HOOK(0x73B02D, UnitClass_UpdatePos_CrushMovementZone, 0x6)
//{
//	GET(MovementZone, nZone, EAX);
//
//	return nZone == MovementZone::CrusherAll || nZone == MovementZone::Subterrannean ?
//		0x73B036 : 0x73B074;
//}

//DEFINE_HOOK(0x73AFE3 , UnitClass_UpdatePosition_Crush, 6)
//{
//	GET(UnitClass*, pUnit, EBP);
//	GET(CellClass*, pCell, EDI);
//
//	const auto pType = pUnit->Type;
//	const auto nMovementZone = pType->MovementZone;
//
//	if (pType->Crusher || pUnit->HasAbility(AbilityType::Crusher))
//	{
//		if (auto pTerrain = pCell->GetTerrain(false))
//		{
//			if (pTerrain->IsAlive)
//			{
//				const auto pTType = pTerrain->Type;
//				if (!pTType->SpawnsTiberium &&
//					!pTType->Immune &&
//					!TerrainTypeExt::ExtMap.Find(pTType)->IsPassable &&
//					pTType->Crushable
//					)
//				{
//					if (TechnoTypeExt::ExtMap.Find(pType)->CrushLevel.Get(pUnit) >
//						TerrainTypeExt::ExtMap.Find(pTType)->CrushableLevel)
//					{
//						VocClass::PlayAt(pType->CrushSound, pUnit->Location);
//						pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pUnit, true, true, pUnit->Owner);
//
//						if (pType->TiltsWhenCrushes)
//							pUnit->RockingForwardsPerFrame += 0.02f;
//
//						pUnit->iscrusher_6B5 = false;
//					}
//				}
//			}
//		}
//
//		if (pCell->OverlayTypeIndex != -1)
//		{
//			const auto pOverlayType = OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex);
//			if (pOverlayType->Crushable
//			  || pOverlayType->Wall
//			  && (nMovementZone == MovementZone::CrusherAll) || nMovementZone == MovementZone::Subterrannean)
//			{
//				VocClass::PlayIndexAtPos(pOverlayType->CrushSound, pUnit->Location, 0);
//				pCell->ReduceWall();
//				pUnit->iscrusher_6B5 = 0;
//				pUnit->RockingForwardsPerFrame += 0.02f;
//			}
//		}
//	}
//
//	return 0x73B074;
//}

// the unit seems dont like it 
// something missing
//bool IsAllowToTurn(UnitClass* pThis, AbstractClass* pTarget, int nMax, DirStruct* pTargetDir)
//{
//	if (!pTargetDir && !pTarget)
//		return true;
//
//	auto nPriFacing = (short)pThis->PrimaryFacing.Current().Raw;
//	auto nTargetDir = pTargetDir ? (short)pTargetDir->Raw : (short)pThis->GetDirectionOverObject(pTarget).Raw;
//
//	int nFrom = (((nPriFacing >> 7) + 1) >> 1);
//	int nTo = (((nTargetDir >> 7) + 1) >> 1);
//
//	if (abs(nFrom - nTo) <= nMax)
//		return true;
//	
//	short n_To_s = (nTo <= 127) ? nTo : (nTo - 256);
//	short n_From_s = (nFrom <= 127) ? nFrom : (nFrom - 256);
//
//	if (abs(n_From_s - n_To_s) > nMax)
//		return false;
//
//	return true;
//}

//DEFINE_HOOK(0x741229, UnitClass_GetFireError_Facing, 6)
//{
//	GET(WeaponTypeClass*, pWeapon, EBX);
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(AbstractClass*, pTarget, 0x20);
//
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pThis->Type->DeployFire
//		&& !pThis->Type->IsSimpleDeployer
//		&& !pThis->Deployed)
//	{
//		if (!pTypeExt->DeployFire_UpdateFacing)
//		{
//			//R->EAX(FireError::OK); //yes , dont return facing error 
//			//return 0x74132B;
//			return 0x741327; //fireOK
//		}
//	}
//
//	if (pWeapon->OmniFire)
//		return 0x741314;
//
//	if (!pThis->IsFiring && pThis->IsRotating && !pWeapon->Projectile->ROT)
//		return 0x74124D;
//
//	const auto pType = pThis->Type;
//
//	if (IsAllowToTurn(pThis, pTarget , 10 , nullptr))
//		return 0x74125C;
//
//	//0x7412F1 FireError::Facing
//
//	return pType->Turret ? 0x74101E : 0x74124D;
//}

//DEFINE_HOOK(0x7369F2, UnitClass_UpdateFacing_TurretLimit, 8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(DirStruct, nDir, 0x8);
//
//	if (IsAllowToTurn(pThis, nullptr, 10, &nDir))
//		return 0x0;
//
//	if (!pThis->IsRotating)
//	{
//		pThis->IsRotating = true;
//		pThis->PrimaryFacing.Set_Desired(nDir);
//	}
//
//	pThis->SecondaryFacing.Set_Desired(nDir);
//
//	return 0x736A8E;
//}

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

DEFINE_HOOK(0x73DDC0, UnitClass_Mi_Unload_DeployIntoPlaceAnywhere, 6)
{
	GET(UnitClass*, pThis, ESI);

	if (R->AL())
	{
		if (pThis->Type->Speed)
			return 0x73DE20;
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

//DEFINE_HOOK(0x714522, TechnoTypeClass_LoadFromINI_RequiredHouses, 9)
//{
//	GET(CCINIClass*, pCCINI, ESI);
//	GET(TechnoTypeClass*, pThis, ESI);
//	GET(const char*, pSection, EBX);
//
//	if (pCCINI->ReadString(pSection, (const char*)0x843BB4, Phobos::readDefval, Phobos::readBuffer)) {
//		if (GameStrings::IsBlank(Phobos::readBuffer)) {
//			pThis->RequiredHouses = -1;
//			return 0x71453C;
//		}
//
//		char* context = nullptr;
//
//		int i = 0;
//		for (auto cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
//			cur;
//			cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
//			pThis->RequiredHouses |= HouseTypeClass::FindIndexOfNameShiftToTheRightOnce(cur);
//			Debug::Log("TechnoType[%s] Reading Owner BitField of %s \n", pSection, cur);
//			++i;
//		}
//
//		Debug::Log("TechnoType[%s] Have %d Owner Bitfield! \n", pSection , i);
//	}
//
//	return 0x71453C;
//}

//TechnoClass_GetWeaponState
DEFINE_OVERRIDE_HOOK(0x6FCA30, TechnoClass_GetFireError_DecloakToFire, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pTransporter = pThis->Transporter;

	if (pTransporter && pTransporter->CloakState != CloakState::Uncloaked)
		return 0x6FCA4F;

	if (pThis->CloakState == CloakState::Uncloaked)
		return 0x6FCA5E;

	if (!pWeapon->DecloakToFire && Is_Aircraft(pThis))
		return 0x6FCA4F;

	return pThis->CloakState == CloakState::Cloaked ? 0x6FCA4F : 0x6FCA5E;
	//return 0x0;
}

DEFINE_HOOK(0x700391, TechnoClass_GetCursorOverObject_AttackFriendies, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);
	GET(WeaponTypeClass* const, pWeapon, EBP);

	if (!pType->AttackFriendlies)
		return 0x70039B;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

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

// //EvalObject
DEFINE_HOOK(0x6F7EFE, TechnoClass_CanAutoTargetObject_SelectWeapon, 6)
{
	enum { AllowAttack = 0x6F7FE9, ContinueCheck = 0x6F7F0C };
	GET_STACK(int const, nWeapon, 0x14);
	GET(TechnoClass* const, pThis, EDI);

	const auto pType = pThis->GetTechnoType();

	if (!pType->AttackFriendlies)
		return ContinueCheck;

	bool Allow = true;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->AttackFriendlies_WeaponIdx != -1)
		Allow = pTypeExt->AttackFriendlies_WeaponIdx == nWeapon;

	return Allow ? AllowAttack : ContinueCheck;
}

DEFINE_HOOK(0x70A3E5, TechnoClass_DrawPipScale_Ammo_Idx, 7)
{
	GET(TechnoClass* const, pThis, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->ESP(pTypeExt->PipScaleIndex.Get(13));
	return 0x0;
}

DEFINE_HOOK(0x70A35D, TechnoClass_DrawPipScale_Ammo, 5)
{
	GET(TechnoClass* const, pThis, EBP);
	GET_STACK(RectangleStruct*, pRect, 0x80);
	GET_STACK(int const, nX, 0x50);
	GET_STACK(int const, nY, 0x54);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeExt->AmmoPip.isset())
	{
		const auto pSHApe = pTypeExt->AmmoPip;
		const int nFrame = int(((1.0 - pThis->Ammo) / pTypeExt->Get()->Ammo) * pSHApe->Frames);
		Point2D offs { nX, nY };
		offs += pTypeExt->AmmoPip_Offset.Get();
		ConvertClass* pConvert = FileSystem::PALETTE_PAL();
		if (const auto pConvertData = pTypeExt->AmmoPip_Palette)
		{
			pConvert = pConvertData->GetConvert<PaletteManager::Mode::Default>();
		}

		DSurface::Temp->DrawSHP(pConvert, pSHApe,
			nFrame, &offs, pRect, (BlitterFlags)0x600u, 0, 0, 1000, 0, 0, nullptr, 0, 0, 0);

		return 0x70A4EC;
	}

	return 0x0;
}

DEFINE_HOOK(0x741554, UnitClass_ApproachTarget_CrushRange, 0x6)
{
	enum { Crush = 0x741599, ContinueCheck = 0x741562 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	return pTypeExt->CrushRange.GetOrDefault(pThis, RulesClass::Instance->Crush) >= range ?
		Crush : ContinueCheck;
}

DEFINE_HOOK(0x7439AD, UnitClass_ShouldCrush_CrushRange, 0x6)
{
	enum { DoNotCrush = 0x743A39, ContinueCheck = 0x7439B9 };
	GET(UnitClass* const, pThis, ESI);
	GET(int const, range, EAX);
	GET(RulesClass* const, pRules, ECX);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	return pTypeExt->CrushRange.GetOrDefault(pThis, pRules->Crush) <= range ?
		ContinueCheck : DoNotCrush;
}
#include <Ext/ParticleType/Body.h>

DEFINE_HOOK(0x62BD69, ParticleClass_ProcessGasBehaviour_NoWind, 6)
{
	GET(ParticleClass* const, pThis, EBP);

	return pThis->Type->WindEffect == -1
		? 0x62C200 : 0x0;
}

DEFINE_HOOK(0x62C361, ParticleClass_ProcessGasBehaviour_DisOnWater, 6)
{
	GET(ParticleClass*, pThis, EBP);
	GET(ParticleTypeClass* const, pType, EDI);

	const auto pTypeExt = ParticleTypeExt::ExtMap.Find(pType);

	if (pType->WindEffect == -1)
	{
		const auto pCell = pThis->GetCell();
		if (pCell->ContainsBridge() || (pCell->LandType != LandType::Beach && pCell->LandType != LandType::Water))
			return 0;
	}
	else
	{

		if (!pTypeExt->DeleteWhenReachWater)
			return 0;

		const auto pCell = pThis->GetCell();
		if (pCell->ContainsBridge() || (pCell->LandType != LandType::Beach && pCell->LandType != LandType::Water))
			return 0;
	}

	pThis->hasremaining = 1;
	//GameDelete<true,false>(pThis);
	pThis->UnInit();
	return 0x62C394;
}

// idk , the ext stuffs is set somewhere ?
//DEFINE_HOOK(0x7009D4 , TechnoClass_GetActionOnCell_Wall, 6)
//{
//	GET(OverlayTypeClass*, pOvl, EDX);
//	GET(WarheadTypeClass*, pWeapon, EDI);
//	int result; // eax
//
//	return !pOvl->Immune || pWeapon->Wall 
//		? 0x7009F3 : 0x7009DE;
//}

//DEFINE_HOOK(70BD34 , TechnoClass_EstimateCannonCoords, 6)
//{
//	// [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]
//
//	v1 = R;
//	v2 = R->_EBX.data;
//	v3 = v2->t.r.m.o.a.vftable->t.r.m.o.Class_Of(v2);
//	v14 = v2->Locomotion;
//	v4 = v3->JumpJet == 0;
//	v5 = *v14;
//	if (v4)
//		v6 = (v5->Is_Moving)(v14);
//	else
//		v6 = (v5->Is_Moving_Now)(v14);
//	if (v6)
//	{
//		v7 = v1->_ESI.data;
//		pStack = v1->_ESP.data;
//		Debug::Log(
//		  "\nTechnoClass_EstimateCannonCoords : Adjusted_X = %d, Adjusted_Y = %d\n",
//		  *(pStack + 0x34),
//		  *(pStack + 0x38));
//		v19 = v2->t.r.m.o.a.vftable->Get_Movement_Speed(v2);
//		v8 = ObjectClass::Distance(v7, v2);
//		v9 = &v7->vftable->t;
//		R = v8;
//		v10 = (v9->Get_Primary_Weapon(v7))->WeaponType;
//		if (v10)
//		{
//			v11 = WeaponTypeClass::Get_Speed(v10, R);
//			Debug::Log(
//			  "TechnoClass_EstimateCannonCoords : BulletSpeed = %d ,  TargetSpeed = %d , TargetDistance = %d\n",
//			  v11,
//			  v19,
//			  R);
//			v18 = R / (v11 * 0.9) * v19;
//			v16 = (*FacingClass::Current(&v2->t.PrimaryFacing, &R) - 0x3FFF) * -0.00009587672516830327;
//			v12 = (FastMath::Sin)(LODWORD(v16), HIDWORD(v16));
//			*(pStack + 0x38) -= MEMORY[0x7C5F00](v12* v18);
//			v15 = (FastMath::Cos)(LODWORD(v16), HIDWORD(v16)) * v18;
//			*(pStack + 0x34) += MEMORY[0x7C5F00](v15);
//			Debug::Log(
//			  "TechnoClass_EstimateCannonCoords : shift = %d ,  direction = %lf , Adjusted_X = %d, Adjusted_Y = %d\n",
//			  LODWORD(v18),
//			  COERCE_LONG_DOUBLE(__PAIR64__(LODWORD(v16), HIDWORD(v18))),
//			  HIDWORD(v16),
//			  *(pStack + 0x34));
//			v1->_EBP.data = *(pStack + 0x38);
//		}
//	}
//	return 0x70BE29;
//}

// UnitClass_Unload_NoManualEject , 0x0 false , 0x73DCD3 true , Typeptr Eax
//  , 0x0 false , 0x7400F0 true , Typeptr Eax

DEFINE_HOOK(0x73D6EC, UnitClass_Unload_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualEject.Get() ? 0x73DCD3 : 0x0;
}

DEFINE_HOOK(0x740015, UnitClass_WhatAction_NoManualEject, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);
	return TechnoTypeExt::ExtMap.Find(pType)->NoManualEject.Get() ? 0x7400F0 : 0x0;
}

//DEFINE_HOOK(0x7818D4, UnitClass_CrushCell_CrushAnim, 0x6)
//{
//	GET(FootClass*, pVictim, ESI);
//
//	if()
//}

DEFINE_HOOK(0x711F0F, TechnoTypeClass_GetCost_AICostMult, 0x8)
{
	GET(HouseClass* const, pHouse, EDI);
	GET(TechnoTypeClass* const, pType, ESI);

	//const double mult = !pHouse->ControlledByPlayer() ? RulesExt::Global()->AI_CostMult : 1.0;
	R->EAX(int(pType->GetCost() * pHouse->GetHouseCostMult(pType) * pHouse->GetHouseTypeCostMult(pType) /* mult*/));
	return 0x711F46;
}

//double FC HouseClass_GetTypeCostMult(HouseClass* pThis, DWORD, TechnoTypeClass* pType)
//{
   // const double mult = !pThis->ControlledByPlayer() ? RulesExt::Global()->AI_CostMult : 1.0;
   // return pThis->GetHouseTypeCostMult(pType) * mult;
//}

//DEFINE_JUMP(CALL,0x711F12, GET_OFFSET(HouseClass_GetTypeCostMult));

//DEFINE_HOOK(0x4179F7, AircraftClass_AssumeTaskComplete_DontCrash, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	if (pThis->Type->Spawned || pThis->Type->Carryall)
//		return 0;
//
//	pThis->SetDestination(nullptr, true);
//	return 0x417B69;
//}

DEFINE_HOOK(0x422A59, AnimClass_DTOR_DoNotClearType, 0x6)
{
	return R->Origin() + 0x6;
	// clearing type pointer will cause animclass AI to crash when it still executed
}

DEFINE_HOOK(0x4251AB, AnimClass_Detach_LogTypeDetached, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	Debug::Log("Anim[0x%x] detaching Type Pointer ! \n", pThis);
	return 0x0;
}

DEFINE_HOOK(0x6F357F, TechnoClass_SelectWeapon_DrainWeaponTarget, 0x6)
{
	enum { CheckAlly = 0x6F3589, ContinueCheck = 0x6F35A8  , RetPrimary = 0x6F37AD };

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

#include <Ext/InfantryType/Body.h>
std::array<const char* const, 6u> DamageState_to_srings
{
{
	"Unaffected", "Unchanged", "NowYellow", "NowRed", "NowDead", "PostMortem"
}
};

//DEFINE_HOOK(0x440333, BuildingClass_AI_C4TimerRanOut_ApplyDamage, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//
//	if (pExt->C4Damage.isset())
//	{
//		int nDamage = pExt->C4Damage.get();
//		const auto nResult = pThis->ReceiveDamage(&nDamage, 0, pExt->C4Warhead, pThis->C4AppliedBy, false, false, pExt->C4Owner);
//		Debug::Log("C4Damage Result [%s] ! \n", DamageState_to_srings[(int)nResult]);
//		return 0x44035E;
//	}
//
//	pExt->C4Damage.clear();
//	return 0x0;
//}

//basically this C4 thing will always one hit kill regardless, 
// because of fcki g weird ass ww code desing ,..
//DEFINE_HOOK(0x442696, BuildingClass_ReceiveDamage_C4, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI)
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//
//	if (pExt->C4Damage.isset())
//	{
//		pExt->C4Damage.clear();
//		return 0x4426A7;
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x701F60, TechnoClass_TakeDamage_IsGoingToBlow, 0x6)
//{
//	Debug::Log("Exec ! \n");
//	return 0x0;
//}

//DEFINE_HOOK(0x4400C1, BuildingClass_AI_C4TimerRanOut_ApplyDamage_B, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	if (pExt->C4Damage.isset())
//	{
//		int nDamage = pExt->C4Damage.get();
//		pThis->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pThis->C4AppliedBy, false, false,
//			pExt->C4Owner);
//
//		pExt->C4Damage.clear();
//		return pThis->IsAlive ? 0x4400F2 : 0x4400EA;
//	}
//
//	pExt->C4Damage.clear();
//	return 0x0;
//}

//DEFINE_HOOK(0x440327, BuildingClass_AI_C4DataClear, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	pExt->C4Damage.clear();
//	pExt->C4Owner = nullptr;
//	return 0x0;
//}

//DEFINE_HOOK(0x457CA0, BuildingClass_ApplyIC_C4DataClear, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	pExt->C4Damage.clear();
//	pExt->C4Owner = nullptr;
//	return 0x0;
//}

DEFINE_HOOK(0x447195, BuildingClass_SellBack_Silent, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExt::ExtMap.Find(pThis)->Silent ? 0x447203 : 0x0;
}

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

#ifdef FOW_HOOKS
// MapClass_RevealShroud as Xkein said
DEFINE_HOOK(0x4ADFF0, DisplayClass_All_To_Look_Ground, 0x5)
{
	// GET(DisplayClass*, pDisplay, ECX);
	GET_STACK(DWORD, dwUnk, 0x4);
	GET_STACK(DWORD, dwUnk2, 0x8);

	for (auto pTechno : *TechnoClass::Array)
	{
		if (!pTechno)
			continue;

		if (!dwUnk || !Is_Building(pTechno))
		{
			if (pTechno->GetTechnoType()->RevealToAll ||
				(
					pTechno->DiscoveredByCurrentPlayer &&
					pTechno->Owner == HouseClass::CurrentPlayer() ||
					pTechno->Owner->IsHumanPlayer ||
					pTechno->Owner->IsInPlayerControl
					) ||
				(
					pTechno->Owner == HouseClass::CurrentPlayer() ||
					HouseClass::CurrentPlayer()->ArrayIndex != -1 &&
					pTechno->Owner->IsAlliedWith(HouseClass::CurrentPlayer())
					))
			{
				pTechno->See(0, dwUnk2);
				if (pTechno->IsInAir())
				{

					auto coord = pTechno->GetCoords();
					MapClass::Instance->RevealArea3(
						&coord,
						pTechno->LastSightRange - 3,
						pTechno->LastSightRange + 3,
						false
					);
				}
			}
		}
	}

	return 0x4AE0A5;
}

DEFINE_HOOK(0x577EBF, MapClass_Reveal, 0x6)
{
	GET(CellClass*, pCell, EAX);

	pCell->ShroudCounter = 0;
	pCell->GapsCoveringThisCell = 0;
	pCell->AltFlags |= (AltCellFlags)0x18u;
	pCell->Flags |= (CellFlags)0x3u;
	pCell->CleanFog();

	return 0x577EE9;
}

DEFINE_HOOK(0x586683, CellClass_DiscoverTechno, 0x8)
{
	GET(TechnoClass*, pTechno, EAX);
	GET(CellClass*, pCell, ESI);
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x18, -0x8));

	bool bDiscovered = false;
	if (pTechno)
		bDiscovered = pTechno->DiscoveredBy(pHouse);
	if (auto const pJumpjet = pCell->Jumpjet)
		bDiscovered |= pJumpjet->DiscoveredBy(pHouse);
	R->EAX(bDiscovered);

	return 0x586696;
}

DEFINE_HOOK(0x4FC1FF, HouseClass_AcceptDefeat_CleanShroudFog, 0x6)
{
	GET(HouseClass*, pHouse, ESI);

	MapClass::Instance->Reveal(pHouse);

	return 0x4FC214;
}

DEFINE_HOOK(0x6B8E7A, ScenarioClass_LoadSpecialFlags, 0x5)
{
	GET(ScenarioClass*, pScenario, ESI);

	pScenario->SpecialFlags.StructEd.FogOfWar =
		RulesClass::Instance->FogOfWar || R->EAX() || GameModeOptionsClass::Instance->FogOfWar;

	R->ECX(pScenario);
	return 0x6B8E8B;
}

DEFINE_HOOK(0x686C03, SetScenarioFlags_FogOfWar, 0x6)
{
	GET(ScenarioFlags, SFlags, EAX);

	SFlags.FogOfWar = RulesClass::Instance->FogOfWar || GameModeOptionsClass::Instance->FogOfWar;
	R->EDX<int>(*reinterpret_cast<int*>(&SFlags)); // stupid!

	return 0x686C0E;
}

DEFINE_HOOK(0x5F4B3E, ObjectClass_DrawIfVisible, 0x6)
{
	GET(ObjectClass*, pObject, ESI);

	enum { Skip = 0x5F4B7F, SkipWithDrawn = 0x5F4D06, DefaultProcess = 0x5F4B48 };

	if (pObject->InLimbo)
		return Skip;

	if (!ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar)
		return DefaultProcess;

	if (pObject->WhatAmI() == AbstractType::Cell)
		return DefaultProcess;

	auto coord = pObject->GetCoords();
	if (!MapClass::Instance->IsLocationFogged(coord))
		return DefaultProcess;

	pObject->NeedsRedraw = false;
	return SkipWithDrawn;
}

DEFINE_HOOK(0x6F5190, TechnoClass_DrawExtras_CheckFog, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	auto coord = pTechno->GetCoords();

	return MapClass::Instance->IsLocationFogged(coord)
		? 0x6F5EEC : 0;
}

DEFINE_HOOK(0x48049E, CellClass_DrawTileAndSmudge_CheckFog, 0x6)
{
	GET(CellClass*, pCell, ESI);

	return (pCell->SmudgeTypeIndex == -1 || pCell->IsFogged()) ?
		0x4804FB : 0x4804A4;
}

DEFINE_HOOK(0x6D6EDA, TacticalClass_Overlay_CheckFog_1, 0xA)
{
	GET(CellClass*, pCell, EAX);

	return (pCell->OverlayTypeIndex == -1 || pCell->IsFogged()) ?
		0x6D7006 : 0x6D6EE4;
}

DEFINE_HOOK(0x6D70BC, TacticalClass_Overlay_CheckFog_2, 0xA)
{
	GET(CellClass*, pCell, EAX);

	return (pCell->OverlayTypeIndex == -1 || pCell->IsFogged()) ?
		0x6D71A4 : 0x6D70C6;
}

DEFINE_HOOK(0x71CC8C, TerrainClass_DrawIfVisible, 0x6)
{
	GET(TerrainClass*, pTerrain, EDI);

	auto coord = pTerrain->GetCoords();

	return (pTerrain->InLimbo || MapClass::Instance->IsLocationFogged(coord)) ?
		0x71CD8D : 0x71CC9A;
}

bool FC IsLocFogged(MapClass* pThis, DWORD, CoordStruct* pCoord)
{
	const auto pCell = pThis->GetCellAt(pCoord);

	if (pCell->Flags & CellFlags::EdgeRevealed)
	{
		return false;
	}

	return((pCell->GetNeighbourCell(3u)->Flags & CellFlags::EdgeRevealed) == CellFlags::Empty);
}

DEFINE_JUMP(LJMP, 0x5865E0, GET_OFFSET(IsLocFogged))

DEFINE_HOOK(0x4ACE3C, MapClass_TryReshroudCell_SetCopyFlag, 0x6)
{
	GET(CellClass*, pCell, EAX);

	auto oldfield = (unsigned int)pCell->AltFlags;
	pCell->AltFlags = (AltCellFlags)(oldfield & 0xFFFFFFEF);

	auto nIndex = TacticalClass::Instance->GetOcclusion(pCell->MapCoords, false);

	if (((oldfield & 0x10) != 0 || pCell->Visibility != nIndex) && nIndex >= 0 && pCell->Visibility >= -1)
	{
		pCell->AltFlags |= (AltCellFlags)8u;
		pCell->Visibility = nIndex;
	}

	TacticalClass::Instance->RegisterCellAsVisible(pCell);

	return 0x4ACE57;
}

DEFINE_HOOK(0x4A9CA0, MapClass_RevealFogShroud, 0x8)
{
	GET(MapClass*, pMap, ECX);
	GET_STACK(CellStruct*, pLocation, 0x4);
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(bool, bUnk, 0xC);

	auto const pCell = pMap->GetCellAt(*pLocation);

	//if (bUnk) {
	//	pCell->IncreaseShroudCounter();
	//} else {
	//	pCell->ReduceShroudCounter();
	//}

	bool bFlag = bool(pCell->Flags & CellFlags::EdgeRevealed);
	bool bReturn = !bFlag || (pCell->AltFlags & AltCellFlags::Clear);
	bool bTemp = bReturn;

	pCell->Flags = CellFlags((unsigned int)pCell->Flags & 0xFFFFFFFF | 2);
	pCell->AltFlags = AltCellFlags((unsigned int)pCell->AltFlags & 0xFFFFFFDF | 8);

	char nOcclusion = TacticalClass::Instance->GetOcclusion(*pLocation, false);
	char nVisibility = pCell->Visibility;
	if (nOcclusion != nVisibility)
	{
		nVisibility = nOcclusion;
		bReturn = true;
		pCell->Visibility = nOcclusion;
	}
	if (nVisibility == -1)
		pCell->AltFlags |= AltCellFlags(0x10u);
	char nFoggedOcclusion = TacticalClass::Instance->GetOcclusion(*pLocation, true);
	char nFoggedness = pCell->Foggedness;
	if (nFoggedOcclusion != nFoggedness)
	{
		nFoggedness = nFoggedOcclusion;
		bReturn = true;
		pCell->Foggedness = nFoggedOcclusion;
	}
	if (nFoggedness == -1)
		pCell->Flags |= CellFlags(1u);

	if (bReturn)
	{
		TacticalClass::Instance->RegisterCellAsVisible(pCell);
		pMap->RevealCheck(pCell, pHouse, bTemp);
	}
	if (!bFlag && ScenarioClass::Instance->SpecialFlags.StructEd.FogOfWar)
		pCell->CleanFog();

	R->AL(bReturn);

	return 0x4A9DC6;
}

#endif

// Various call of TechnoClass::SetOwningHouse not respecting overloaded 2nd args fix !

bool __fastcall InfantryClass_SetOwningHouse(InfantryClass* const pThis, DWORD, HouseClass* pNewOwner, bool bAnnounce)
{
	return pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce);
}

bool __fastcall AircraftClass_SetOwningHouse(AircraftClass* const pThis, DWORD, HouseClass* pNewOwner, bool bAnnounce)
{
	return pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce);
}

//DEFINE_JUMP(LJMP, 0x7E2678, GET_OFFSET(InfantryClass_SetOwningHouse));
//DEFINE_JUMP(LJMP, 0x7EB42C, GET_OFFSET(AircraftClass_SetOwningHouse));

DEFINE_HOOK(0x7463DC, UnitClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(UnitClass* const, pThis, EDI);
	GET(HouseClass* const, pNewOwner, EBX);
	GET_STACK(bool const, bAnnounce, STACK_OFFSET(0xC, 0x8));

	R->EAX(pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce));
	return 0x7463E6;
}

DEFINE_HOOK(0x4DBF05, FootClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(FootClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EAX);
	GET_STACK(bool const, bAnnounce, STACK_OFFSET(0xC, 0x8));

	R->AL(pThis->TechnoClass::SetOwningHouse(pNewOwner, bAnnounce));
	return 0x4DBF0F;
}

DEFINE_HOOK(0x448BE3, BuildingClass_SetOwningHouse_FixArgs, 0x5)
{
	GET(FootClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EDI);
	GET_STACK(bool const, bAnnounce, STACK_OFFSET(0x58, 0x8));

	//discarded
	pThis->TechnoClass::SetOwningHouse(pNewOwner, bAnnounce);
	return 0x448BED;
}

DEFINE_HOOK(0x7225F3, TiberiumClass_Spread_nullptrheap, 0x7)
{
	GET(MapSurfaceData*, ptr, EBP);

	return ptr ? 0x0 : 0x722604;
}

//skip vanilla TurretOffset read
DEFINE_JUMP(LJMP, 0x715876, 0x71589A);

DEFINE_HOOK(0x508F82, HouseClass_AI_checkSpySat_IncludeUpgrades, 0x6)
{
	enum { AdvanceLoop = 0x508FF6, Continue = 0x508F91 };

	GET(BuildingClass const*, pBuilding, ECX);

	if (!pBuilding->Type->SpySat)
	{
		for (const auto& pUpGrade : pBuilding->Upgrades)
		{
			if (pUpGrade && pUpGrade->SpySat)
				return Continue;
		}

		return AdvanceLoop;
	}

	return Continue;
}

#pragma region Assaulter
//https://blueprints.launchpad.net/ares/+spec/assaulter-veterancy
//522A09
DEFINE_HOOK(0x522A09, InfantryClass_EnteredThing_Assaulter, 0x6)
{
	enum { retTrue = 0x522A11, retFalse = 0x522A45 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F580 
DEFINE_HOOK(0x51F580, InfantryClass_MissionHunt_Assaulter, 0x6)
{
	enum { retTrue = 0x51F58A, retFalse = 0x51F5C0 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51F493 
DEFINE_HOOK(0x51F493, InfantryClass_MissionAttack_Assaulter, 0x6)
{
	enum { retTrue = 0x51F49D, retFalse = 0x51F4D3 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::IsAssaulter(pThis) ? retTrue : retFalse;
}

//51968E 
DEFINE_HOOK(0x51968E, InfantryClass_sub_519633_Assaulter, 0x6)
{
	enum { retTrue = 0x5196A6, retFalse = 0x519698 };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::IsAssaulter(pThis) ? retTrue : retFalse;
}

//4D4BA0
DEFINE_HOOK(0x4D4BA0, InfantryClass_MissionCapture_Assaulter, 0x6)
{
	enum { retTrue = 0x4D4BB4, retFalse = 0x4D4BAA };

	GET(InfantryClass*, pThis, ESI);

	return TechnoExt::IsAssaulter(pThis) ? retTrue : retFalse;
}

DEFINE_HOOK(0x457DAD, BuildingClass_CanBeOccupied_Assaulter, 0x6)
{
	enum { retTrue = 0x457DD5, retFalse = 0x457DA3 };

	GET(BuildingClass* const, pThis, ESI);
	GET(InfantryClass* const, pInfantry, EDI);

	if (TechnoExt::IsAssaulter(pInfantry))
	{
		if (!pThis->Owner->IsAlliedWith(pInfantry) && pThis->GetOccupantCount() > 0)
		{
			const auto pBldExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

			// buildings with negative level are not assaultable
			if (pBldExt->AssaulterLevel >= 0)
			{
				// assaultable if infantry has same level or more
				if (pBldExt->AssaulterLevel <= TechnoTypeExt::ExtMap.Find(pInfantry->Type)->AssaulterLevel)
				{
					return retTrue;
				}
			}
		}
	}

	return retFalse;
}
#pragma endregion

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

// TODO : complete replacement !
DEFINE_HOOK(0x709B79, TechnoClass_DrawPip_Spawner, 0x6)
{
	//GET(TechnoClass*, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);

	if ((int)pType->PipScale != 6)
	{
		R->EBX(0);
		return 0x709B7F;
	}

	return 0x0;
}

DEFINE_HOOK(0x481180, CellClass_GetInfantrySubPos_InvalidCellPointer, 0x5)
{
	enum { retEmpty = 0x4812EC, retContinue = 0x0, retResult = 0x481313 };
	GET(CellClass*, pThis, ECX);
	GET_STACK(CoordStruct*, pResult, 0x4);

	if (!pThis)
	{
		*pResult = CoordStruct::Empty;
		R->EAX(pResult);
		return retResult;
	}

	return retContinue;
}

//DEFINE_HOOK(0x4899FE, MapClass_DamageArea_DamageGroup_BeforeTechnoGetDamaged, 0x9)
//{
//	GET_BASE(WarheadTypeClass*, pWH, 0xC);
//	GET(TechnoClass*, pTarget, ESI);
//
//	const auto pType = pTarget->GetTechnoType();
//
//	if (IS_SAME_STR_("HuskWH", pWH->ID) && pTarget->OnBridge)
//	{
//		Debug::Log("[%s] Here !" , pType->ID);
//	}
//
//	return 0x0;
//}

DEFINE_HOOK(0x520E75, InfantryClass_SequenceAI_PreventOutOfBoundArrayRead, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	return (int)pThis->SequenceAnim == -1 ? 0x520EF4 : 0x0;
}

DEFINE_HOOK(0x5194EF, InfantryClass_DrawIt_InAir_NoShadow, 5)
{
	GET(InfantryClass*, pThis, EBP);
	return pThis->Type->NoShadow ? 0x51958A : 0x0;
}

DEFINE_HOOK(0x746AFF, UnitClass_Disguise_Update_MoveToClear, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->Disguise && Is_Unit(pThis->Disguise) ? 0x746A9C : 0;
}

// DEFINE_HOOK(0x746B6B, UnitClass_Disguise_FullName , 7)
//{
//	GET(UnitClass*, pThis, ESI);
//
//    if (pThis->IsDisguised())
//    {
//		if(!pThis->Owner)
//			return 0x746B48;
//
//		const auto pPlayer = HouseClass::CurrentPlayer();
//		if(!pPlayer || (pThis->Owner != pPlayer))
//			return 0x746B48;
//
//		if(!pThis->Owner->IsAlliedWith_(pPlayer))
//			return 0x746B48;
//    }
//
//    return 0;
//}

// it only work when first created , when captured or change owner
// it wont change ,.. need more stuffs
//DEFINE_HOOK(0x45197B, BuildingClass_AnimLogic_SetOwner, 0x6)
//{
//	GET(AnimClass*, pAnim, EBP);
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pAnim && pAnim->Owner != pThis->Owner)
//		pAnim->Owner = pThis->Owner;
//
//	return 0x0;
//}

DEFINE_HOOK(0x4D423A, FootClass_MissionMove_SubterraneanResourceGatherer, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pType = pThis->GetTechnoType();
	if (Is_Unit(pThis) && pType->IsSubterranean && pType->ResourceGatherer)
	{
		pThis->QueueMission(Mission::Harvest, false);
	}

	return 0x0;
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

// DEFINE_HOOK(0x70F837, TechnoClass_GetOriginalOwner_PermaMCed, 0x6)
// {
// 	GET(TechnoClass*, pThis, ECX);
//
// 	if (pThis->MindControlledByAUnit)
// 		return 0x70F841;
//
// 	return 0x0;
// }

// do some pre-validation evenbefore function going to be executed 
// save some cpu cycle
DEFINE_HOOK(0x486920, CellClass_TriggerVein_Precheck, 0x6)
{
	return RulesClass::Instance->VeinAttack ? 0x0 : 0x486A6B;
}

// this thing do some placement check twice 
// this can be bad because the `GrowthLogic` data inside not inited properly !
DEFINE_HOOK(0x74C688, VeinholeMonsterClass_CTOR_SkipPlacementCheck, 0x7)
{
	return 0x74C697;
}

DEFINE_HOOK(0x5FC668, OverlayTypeClass_Mark_Veinholedummies, 0x7)
{
	GET(CellStruct*, pPos, EBX);

	if (VeinholeMonsterClass::IsCellEligibleForVeinHole(pPos))
	{
		auto pCell = MapClass::Instance->GetCellAt(pPos);
		//this dummy overlay placed so it can be replace later with real veins
		for (int i = 0 ; i < 8; ++i)
		{
			auto v11 = pCell->GetAdjacentCell((FacingType)i);
			v11->OverlayTypeIndex = 0x7E; //dummy image -> replaced with vein ? 
			;
			v11->OverlayData = 30u; //max it out
			v11->RedrawForVeins();
			//v11->Flags |= CellFlags(0x80u); //what ?
		}

		pCell->OverlayTypeIndex = 0xA7; //VeiholeDummy -> used to place veinhole monster
		pCell->OverlayData = 0;
		++Unsorted::ScenarioInit();
		GameCreate<VeinholeMonsterClass>(pPos);
		--Unsorted::ScenarioInit();
	}

	return 0x5FD1FA;
}

DEFINE_HOOK(0x489671, MapClass_DamageArea_Veinhole, 0x6)
{
	GET(CellClass*, pCell, EBX);
	GET(OverlayTypeClass*, pOverlay, EAX);

	if (pOverlay->ArrayIndex == 0xA7)
	{

		GET_STACK(int, nDamage, 0x24);
		GET(WarheadTypeClass*, pWarhead, ESI);
		GET_BASE(TechnoClass*, pSource, 0x8);
		GET_BASE(HouseClass*, pHouse, 0x14);
		GET(CoordStruct*, pCenter, EDI);

		for (auto pMonster : *VeinholeMonsterClass::Array)
		{
			if (!pMonster->InLimbo && (pMonster->MonsterCell.DistanceFromI(pCell->MapCoords) <= 0))
				pMonster->ReceiveDamage(&nDamage,
					pCenter->DistanceFromI(CellClass::Cell2Coord(pMonster->MonsterCell)),
					pWarhead,
					pSource,
					false,
					false,
					pSource && !pHouse ? pSource->Owner : pHouse
				);

		}
	}

	return 0x0;
}

DEFINE_HOOK(0x444159, BuildingClass_KickoutUnit_WeaponFactory_Rubble, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pObj, EDI);

	if (!pThis->Type->WeaponsFactory)
		return 0x4445FB; //not a weapon factory

	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->RubbleDestroyed)
	{
		if (pThis->Factory && pThis->Factory->Object == pObj)
			return 0x444167; //continue check

		if (const auto pIf = specific_cast<InfantryClass*>(pObj))
			return 0x4445FB; // just eject
	}

	return 0x444167; //continue check
}

//TODO : Droppod WH explosion 4B5D8F ,4B6028

DEFINE_HOOK(0x4B5CF1, DropPodLocomotionClass_Process_DroppodPuff, 0x5)
{
	//GET(DropPodLocomotionClass*, pLoco, EDI);
	GET(FootClass*, pFoot, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x40 - 0x18);

	if (!pFoot->Unlimbo(*pCoord, ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::Min, DirType::Max)))
		return 0x4B5D0A;

	if (auto pAnimType = RulesClass::Instance->DropPodPuff)
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	}

	const auto& nDroppod = RulesClass::Instance->DropPod;

	if (!nDroppod.Count)
		return 0x4B5E4C;

	//TS random it with the lpvtable ? idk
	if (auto pAnimType = nDroppod[ScenarioClass::Instance->Random.RandomFromMax(nDroppod.Count - 1)])
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	}

	//original game code
	//using static_cast adding some unnessesary check !
	/*pLoco + 0x18*/
	//if (reinterpret_cast<void*>((DWORD)pLoco + 0x18)) {
	//
	//	if(RulesClass::Instance->DropPod.Count == 1)
	//		return 0x4B5E4C;
	//
	//	if (auto pAnimType = RulesClass::Instance->DropPod[1]) {
	//		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
	//			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	//	}
	//} else {
	//
	//	if (auto pAnimType = RulesClass::Instance->DropPod[0]) {
	//		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
	//			AnimExt::SetAnimOwnerHouseKind(pAnim, pFoot->Owner, nullptr, pFoot, false);
	//	}
	//}

	return 0x4B5E4C;
}

DEFINE_HOOK(0x4B619F, DropPodLocomotionClass_MoveTo_AtmosphereEntry, 0x5)
{
	GET(DropPodLocomotionClass*, pLoco, EDI);
	LEA_STACK(CoordStruct*, pCoord, 0x1C - 0xC);

	if (auto pAnimType = RulesClass::Instance->AtmosphereEntry)
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, pCoord, 0, 1, AnimFlag(0x600), 0, 0))
			AnimExt::SetAnimOwnerHouseKind(pAnim, pLoco->Owner->Owner, nullptr, pLoco->Owner, false);
	}

	return 0x4B61D6;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{

	GET(BuildingClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBP);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFSET(0xF0, -0xA4));
	LEA_STACK(CoordStruct*, pCoord, STACK_OFFSET(0xF0, -0x8C));

	if (pWeapon && pBullet)
	{
		pBullet->SetWeaponType(pWeapon);

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

	return 0;
}

//https://bugs.launchpad.net/ares/+bug/1577493
// stack 0x8 seems occupied by something else ?
//DEFINE_HOOK(0x4684FF, BulletClass_InvalidatePointer_CloakOwner, 0xA)
//{
//	GET(BulletClass*, pThis, ESI);
//	GET_STACK(bool, bRemove, 0x8);
//	GET(AbstractClass*, pTarget, EDI);
//	GET(TechnoClass*, pOwner, EAX);
//   //nope , the third ags , seems not used consistenly , it can cause dangling pointer 
//	if (bRemove && pOwner == pTarget)
//		pThis->Owner = nullptr;
//
//	return 0x468509;
//}

//DEFINE_HOOK(0x442282, BuilngClass_TakeDamage_LATIme_SourceHouseptrIsMissing, 0xA)
//{
//	GET(TechnoClass*, pSource, EBP);
//	GET(BuildingClass*, pThis, ESI);
//
//	if (!Is_Techno(pSource)) {
//		Debug::Log("Building[%s] Taking damage from unknown source [%x] ! , skipping this part", pThis->get_ID(), pSource);
//		return 0x4422C1;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x701E0E, TechnoClass_TakeDamage_UpdateAnger_nullptrHouse, 0xA)
//{
//	GET(TechnoClass*, pSource, EAX);
//	GET(TechnoClass*, pThis, ESI);
//
//	if (!Is_Techno(pSource)) {
//		Debug::Log("Techno[%s] Taking damage from unknown source [%x] ! , skipping this part",
//			pThis->get_ID(), pSource);
//		return 0x701E71;
//	}
//
//	return 0x0;
//}

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
	return BuildingTypeExt::ExtMap.Find(pThis->Type)->Destroyed_CreateSmudge 
		? 0x0 : 0x4433F9;
}

DEFINE_HOOK(0x44177E, BuildingClass_Destroyed_CreateSmudge_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return BuildingTypeExt::ExtMap.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4418EC;
}

DEFINE_HOOK(0x44E809, BuildingClass_PowerOutput_Absorber, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, powertotal, 0x8);

	for (auto pPas = pThis->Passengers.GetFirstPassenger(); 
		pPas; 
		pPas = generic_cast<FootClass*>(pPas->NextObject)) {

		powertotal += abs(TechnoTypeExt::ExtMap.Find(pPas->GetTechnoType())
			->ExtraPower_Amount.Get(pThis->Type->ExtraPowerBonus));
	}

	R->Stack(0x8, powertotal);
	return 0x44E826;
}

DEFINE_JUMP(LJMP, 0x4417A7, 0x44180A);

//bool NOINLINE IsLaserFence(BuildingClass* pNeighbour, BuildingClass* pThis, short nFacing)
//{
//	if (!pNeighbour->Owner || !pThis->Owner || pNeighbour->Owner != pThis->Owner)
//		return false;
//
//	const auto pThisExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
//	const auto& nFence = pThisExt->LaserFencePostLinks;
//
//	if (nFence.empty() || !nFence.Contains(pNeighbour->Type))
//		return false;
//
//	const auto pThatExt = BuildingTypeExt::ExtMap.Find(pNeighbour->Type);
//
//	const auto nFacing_ = (nFacing & 3);
//
//	if (pNeighbour->Type->LaserFencePost
//		|| (pThisExt->LaserFenceWEType.Get(pThisExt->LaserFenceType) == pThatExt->LaserFenceWEType.Get(pNeighbour->Type))
//		|| nFacing_ == pThatExt->LaserFenceDirection && pThisExt->LaserFenceType == pNeighbour->Type)
//	{
//		return true;
//	}
//
//	return false;
//}

//DEFINE_HOOK(0x452F60, BuildingClass_CreateLaserPost_Construction, 5)
//{
//	enum { Failed = 0x452F7A, Succeeded = 0x452EAD };
//	GET(BuildingClass*, pThis, ESI);
//	LEA_STACK(CoordStruct*, pCoord, 0x24);
//	GET(DirType, nValue, ECX);
//
//	const bool IsLasefence = pThis->Type->LaserFence;
//
//	if (!IsLasefence)
//		pThis->QueueMission(Mission::Construction, false);
//
//	if (!pThis->Unlimbo(*pCoord, nValue))
//		return Failed;
//
//	if (!IsLasefence)
//	{
//		pThis->DiscoveredBy(pThis->Owner);
//		pThis->IsReadyToCommence = true;
//	}
//
//	return Succeeded;
//}
//
//DEFINE_SKIP_HOOK(0x452E2C, BuildingClass_CreateLaserPost_SkipTypeSearch, 5, 452E58)
//
//DEFINE_HOOK(0x452EFA, BuildingClass_CreateLaserPost_Type, 5)
//{
//	GET(BuildingClass*, pThis, EDI);
//	GET_STACK(short, nFacing, 0x34);
//
//	auto const bTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
//
//	if (!bTypeExt->LaserFenceType)
//		return 0x45304A;
//
//	BuildingTypeClass* pDecided = nullptr;
//
//	if ((nFacing & 3) != 0) {
//		pDecided = bTypeExt->LaserFenceWEType.Get(bTypeExt->LaserFenceType);
//	}
//
//	R->EAX((BuildingClass*)pDecided->CreateObject(pThis->GetOwningHouse()));
//	return 0x452F0F;
//}
//
//DEFINE_HOOK(0x452BB0, BuildingClass_GetNearbyLaserFence, 7)
//{
//	GET(BuildingClass*, pThis, ECX);
//	GET_STACK(short, nFacing, 0x4);
//	GET_STACK(bool, bOnlyCheckPost, 0x8);
//	GET_STACK(int, nThread, 0xC);
//
//	auto const pType = pThis->Type;
//	BuildingClass* pResult = nullptr;
//
//	if (nThread < 0)
//	{
//		if (pType->LaserFencePost)
//		{
//			nThread = 1;
//		}
//
//		auto const nThreadPosed = pType->ThreatPosed >> 8;
//
//		if (nThreadPosed > 1)
//			nThread = nThreadPosed;
//	}
//
//	if (!nThread)
//	{
//		R->EAX(pResult);
//		return 0x452D37;
//	}
//
//	auto nCell = CellClass::Coord2Cell(pThis->GetCoords());
//
//	for (int i = 0; i < nThread; ++i)
//	{
//		nCell += CellSpread::AdjacentCell[nFacing & 7];
//
//		if (auto const pBuilding = MapClass::Instance->GetCellAt(nCell)->GetBuilding())
//		{
//			if (IsLaserFence(pBuilding, pThis, nFacing))
//				pResult = pBuilding;
//
//			const auto nFacing_ = (nFacing & 3);
//
//			if (!bOnlyCheckPost || !pType->LaserFencePost || (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 3) != nFacing_)
//				break;
//		}
//	}
//
//	R->EAX(pResult);
//	return 0x452D37;
//}
//
//DEFINE_HOOK(0x6D5801, TacticalClass_DrawLaserFencePlaceLink, 6)
//{
//	GET(BuildingClass*, pThat, EAX);
//	GET_STACK(short, nFacing, 0x28);
//	return IsLaserFence(pThat, TacticalClass::DisplayPendingObject(), nFacing) ? 0x6D5828 : 0x6D59A6;
//}

// Sink sound //4DAC7B
//todo : 

// https://bugs.launchpad.net/ares/+bug/1840387
// https://bugs.launchpad.net/ares/+bug/1777260
// https://bugs.launchpad.net/ares/+bug/1324156
// https://bugs.launchpad.net/ares/+bug/1911093
// https://blueprints.launchpad.net/ares/+spec/set-veterancy-of-paradropped-units
// https://bugs.launchpad.net/ares/+bug/1525515
// https://bugs.launchpad.net/ares/+bug/896353

//700E47
//740031
//700DA8
// https://bugs.launchpad.net/ares/+bug/1384794

//DEFINE_HOOK(0x423FF6, AnimClass_AI_SpawnTib_Probe, 0x6)
//{
//	Debug::Log("HEreIam ! \n");
//	return 0x0;
//}

// TODO :
//  - Weeder
//  - Ice stuffs using WW pointer heap logic
//  - proper damaging function for veins
// 
//fcking heap pointer is deleted after some time 
//who the fuck doing that ?
//DEFINE_HOOK(0x74D847, VeinholeMonsterClass_AI_HeapIsZero, 0x6)
//{
//	GET(VeinholeMonsterClass*, pThis, ESI);
//
//	pThis->RegisterAffectedCells();
//
//	return pThis->GrowthLogic.Heap->Count ? 0x74D84D : 0x74D877;
//}

//DEFINE_HOOK(0x4CF3CB, FlyLocomotionClass_4CEFB0 , 5)
//{
// GET(DirStruct*, pDir, EAX);
// GET(DirStruct*, pDirB, EDX);
// GET(void**, pPtr , ESI);
//
// TechnoClass* pTechno = (TechnoClass*)(*pPtr);
//
// if (pTechno->IsInAir() &&
//	(pTechno->GetTechnoType()->Spawned || pTechno->CurrentMission != Mission::Enter))
// {
//	 if (pDir)
//	 {
//		 pDir->Raw = (pTechno->TurretFacing().GetValue(), 0u);
//	 }
//	 else if (pDirB)
//	 {
//		 pDirB->Raw = (pTechno->TurretFacing().GetValue(), 0u);
//	 }
// }
//
// return 0;
//}

//  DEFINE_HOOK(0x6F3B5C, UnitClassClass_GetFLH_UnbindTurret , 6)
// {
// 	enum { retNotTechno = 0x6F3C1A , retContinue = 0x6F3B62, retSetMtx = 0x6F3C52};
// 	GET(TechnoClass*, pThis, EBX);
// 	GET_STACK(int , weaponIdx , 0xE0);
// 	LEA_STACK(Matrix3D* , pResult , 0x48);
//
//     if (pThis)
//     {
// 		Matrix3D nDummy {};
// 		std::memcpy(pResult ,&nDummy, sizeof(Matrix3D) );
//         return retSetMtx;
//
// 		return retContinue;
//     }
//
//     return retNotTechno;
// }

// better put it at 0x6F3D22 ,..
// replacing getFLH so it safe some execution time
// DEFINE_HOOK(0x6F3D2F, UnitClassClass_GetFLH_OnTarget , 5)
// {
//     GET(TechnoClass*, pTarget, EBX);
// 	R->EAX<CoordStruct*>();
//     return 0;
// }

//TODO Garrison/Ungarrison eva and sound
// NPExt Stuffs
//DEFINE_HOOK(0x6FC96E, TechnoClass_GetFireError_BurstAsRofDelay, 0x5)
//{
//	GET(TechnoClass*, pThis, ESI);
//	enum { retFireErrRearm = 0x6FC940 , retContinueCheck = 0x6FC981 };
//
//	const auto pType = pThis->GetTechnoType();
//	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);
//
//	if (R->EAX<int>())
//	{
//		if (!pExt->UseROFAsBurstDelays && pThis->CurrentBurstIndex)
//		{
//			auto& nTime = pThis->DiskLaserTimer;
//
//			if (nTime.StartTime != -1)
//			{
//				const auto nSpend = nTime.CurrentTime - nTime.StartTime;
//				if (nSpend >= nTime.TimeLeft)
//					return retContinueCheck;
//
//				nTime.TimeLeft -= nSpend;
//			}
//
//			if (!nTime.TimeLeft)
//				return retContinueCheck;
//		}
//
//		return retFireErrRearm;
//	}
//
//	return retContinueCheck;
//}

//UseAlternateFLH 0x5A0
// 0x6F3C82 ....
// there is some change here , need to check for other for compatibility

// PrismSupportDelay , promotable , deglobalized !
