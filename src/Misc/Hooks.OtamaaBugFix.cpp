#include "Hooks.OtamaaBugFix.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>

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

DEFINE_HOOK(0x6EE606, TeamClass_TMission_Move_To_Own_Building_index, 0x7)
{
	GET(TeamClass*, pThis, EBP);
	GET(int, nRawData, EAX);

	const auto nBuildingIdx = nRawData & 0xFFFF;

	if (nBuildingIdx < BuildingTypeClass::Array()->Count)
		return 0x0;

	const auto nTypeIdx = nRawData >> 16 & 0xFFFF;
	const auto nScript = pThis->CurrentScript;

	Debug::Log("[%x]Team script [%s]=[%d] , Failed to find type[%d] building at idx[%d] ! \n", pThis, nScript->Type->get_ID(), nScript->CurrentMission, nTypeIdx, nBuildingIdx);
	return 0x6EE7D0;
}

//Lunar limitation
DEFINE_JUMP(LJMP, 0x546C8B, 0x546CBF);

static DamageAreaResult __fastcall _RocketLocomotionClass_DamageArea(
	CoordStruct* pCoord,
	int Damage,
	TechnoClass* Source,
	WarheadTypeClass* Warhead,
	bool AffectTiberium,
	HouseClass* SourceHouse

)
{
	HouseClass* pHouseOwner = Source ? Source->GetOwningHouse() : nullptr;
	auto nCoord = *pCoord;
	return Map.DamageArea
	(nCoord, Damage, Source, Warhead, Warhead->Tiberium, pHouseOwner);
}

DEFINE_JUMP(CALL, 0x6632C7, GET_OFFSET(_RocketLocomotionClass_DamageArea));

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
		ScenarioGlobal->Random.RandomFromMax(pRules->VeinholeShrinkRate / 2) : 0;

	R->EAX(pRules->VeinholeShrinkRate + nRand);
	return 0x74D37C;
}

DEFINE_HOOK(0x74C5E1, VeinholeMonsterClass_CTOR_TSRandomRate, 0x6)
{
	GET(RulesClass*, pRules, EAX);
	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioGlobal->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74C5E7;
}

DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
{
	GET(RulesClass*, pRules, ECX);

	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioGlobal->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74D2AA;
}

static	void __fastcall DrawShape_VeinHole
(Surface* Surface,
	ConvertClass* Pal,
	SHPStruct* SHP,
	int FrameIndex,
	const Point2D* const Position,
	const RectangleStruct* const Bounds,
	BlitterFlags Flags,
	int Remap,
	int ZAdjust,
	ZGradient ZGradientDescIndex,
	int Brightness,
	int TintColor,
	SHPStruct* ZShape,
	int ZShapeFrame,
	int XOffset,
	int YOffset
)
{
	bool bUseTheaterPal = true;
	CC_Draw_Shape(Surface, bUseTheaterPal ? FileSystem::THEATER_PAL() : Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_JUMP(CALL, 0x74D5BC, GET_OFFSET(DrawShape_VeinHole));

DEFINE_HOOK(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	auto nTheater = ScenarioGlobal->Theater;
	SmudgeTypeClass::TheaterInit(nTheater);
	VeinholeMonsterClass::TheaterInit(nTheater);
	return 0x4AD0A8;
}

//TODO : test
DEFINE_HOOK(0x74D0D2, VeinholeMonsterClass_AI_SelectParticle, 0x5)
{
	//overriden instructions
	R->Stack(0x2C, R->EDX());
	R->Stack(0x30, R->EAX());
	LEA_STACK(CoordStruct* , pCoord, 0x28);
	auto const pRules = RulesExt::Global();
	auto const pParticle  = pRules->VeinholeParticle.Get(pRules->DefaultVeinParticle.Get());
	R->EAX(ParticleSystemClass::Instance->SpawnParticle(pParticle, pCoord));
	return 0x74D100;
}

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(TechnoClass*, pTechnoOwner, EAX);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	//Debug::Log("Wave Receive Damage for Victim [%x] ! \n", pVictim);
	if (const auto pUnit = specific_cast<UnitClass*>(pVictim))
		if (pUnit->DeathFrameCounter > 0)
			return 0x75F432;

	pVictim->ReceiveDamage(&nDamage, 0, pWarhead, pTechnoOwner, false, false, pTechnoOwner ? pTechnoOwner->GetOwningHouse() : nullptr);

	return 0x75F432;
}

DEFINE_HOOK(0x7290AD, TunnelLocomotionClass_Process_Stop, 0x5)
{
	GET(TunnelLocomotionClass* const, pLoco, ESI);

	if (const auto pLinked = pLoco->Owner)
		if (auto const pCell = pLinked->GetCell())
			pCell->CollectCrate(pLinked);

	return 0;
}

DEFINE_HOOK(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

DEFINE_HOOK(0x62A933, ParasiteClass_CanInfect_ParasitePointerGone_Check, 0x5)
{
	GET(ParasiteClass*, pThis, EDI);

	if (!pThis) {
		Debug::Log("Found Invalid ParasiteClass Pointer ! , Skipping ! \n");
		return 0x62A976;
	}

	return 0x0;
}

DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackAllies, 0x5)
{
	GET(const TechnoClass*, pThis, ESI);
	return pThis->GetTechnoType()->AttackFriendlies ? 0x6FA472 : 0x0;
}

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
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCenterCoord()) + pWeaponExt->Feedback_Anim_Offset;
		if (auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord))
		{
			AnimExt::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
			if (pThis->WhatAmI() != AbstractType::Building)
				pFeedBackAnim->SetOwnerObject(pThis);
		}
	}

	return R->EDI<AnimTypeClass*>() ? 0x6FF39C : 0x6FF43F;
}

DEFINE_HOOK(0x6FF3CD, TechnoClass_FireAt_AnimOwner, 0x7)
{
	enum
	{
		Goto2NdCheck = 0x6FF427,
		DontSetAnim = 0x6FF43F
		, Continue = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	//GET(WeaponTypeClass*, pWeapon, EBX);
	//GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	if (!pAnim)
		return DontSetAnim;

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

	return pThis->WhatAmI() == AbstractType::Building ? 0x6FF3D9 : Goto2NdCheck;
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
	R->EDX(RulesGlobal);
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

	HouseClass* OverlayOwner = nHouseIDx >= 0 ? HouseClass::Array->Items[nHouseIDx] : nullptr;

	if (PlacingObject)
	{
		bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;

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

	auto const it =
		std::find_if(RulesExt::Global()->WallTowers.begin(), RulesExt::Global()->WallTowers.end(),
		[&](BuildingTypeClass* const pWallTower) { return pWallTower->ArrayIndex == nNodeBuilding; });

	return (it != RulesExt::Global()->WallTowers.end()) ? 0x4FE656 : 0x4FE696;
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
DEFINE_JUMP(LJMP, 0x5E0C24, 0x5E0C4E)

//HouseClass_AllyAIHouses 0x5
//DEFINE_JUMP(LJMP, 0x501640, 0x50174E)

DEFINE_HOOK(0x5D6BE0, MPGameModeClass_StartingPositionsToHouseBaseCells_Debug, 0x7)
{
	Debug::Log("House count = %d", HouseClass::Array->Count);
	Debug::Log("\n");
	for (auto pHouse : *HouseClass::Array)
	{
		Debug::Log("House start cell = [%d] { %d, %d }",
		(DWORD)pHouse,
		pHouse->StartingCell.X,
		pHouse->StartingCell.Y);
		Debug::Log("\n");
	}
	return 0;
}

static bool __fastcall AircraftTypeClass_CanUseWaypoint(AircraftTypeClass* pThis, void*)
{
	return !pThis->Spawned;
}

DEFINE_JUMP(VTABLE, 0x7E2908, GET_OFFSET(AircraftTypeClass_CanUseWaypoint));

static Fuse FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation)
{
	auto& nFuse = pBullet->Data;

	int v3 = nFuse.ArmTimer.StartTime;
	int v4 = nFuse.ArmTimer.TimeLeft;
	if (v3 == -1)
	{
	LABEL_4:
		if (v4)
		{
			return Fuse::DontIgnite;
		}
		goto LABEL_5;
	}
	if (Unsorted::CurrentFrame - v3 < v4)
	{
		v4 -= Unsorted::CurrentFrame - v3;
		goto LABEL_4;
	}
LABEL_5:
	int y = newlocation->Y - nFuse.Location.Y;
	int z = newlocation->Z - nFuse.Location.Z;
	int proximity = Game::F2I(Math::sqrt(static_cast<double>((newlocation->X - nFuse.Location.X)) * static_cast<double>((newlocation->X - nFuse.Location.X)) + static_cast<double>(y) * static_cast<double>(y) + static_cast<double>(z) * static_cast<double>(z)) / 2);

	int nProx = 32;
	auto pExt = BulletExt::ExtMap.Find(pBullet);
	if (pExt->TypeExt->Proximity_Range.isset())
		nProx = pExt->TypeExt->Proximity_Range.Get() * 256;

	if (proximity < nProx) {
		return Fuse::Ignite;
	}

	if (proximity < 256 && proximity > nFuse.Distance) {
		return Fuse::Ignite_DistaceFactor;
	}

	nFuse.Distance = proximity;

	return Fuse::DontIgnite;
}

DEFINE_HOOK(0x467C2E, BulletClass_AI_FuseCheck, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(CoordStruct*, pCoord, EAX);

	R->EAX(FuseCheckup(pThis, pCoord));
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

DEFINE_HOOK(0x4B0523, DriveLocomotionClass_Process_Cargo, 0x5)
{
	GET(DriveLocomotionClass*, pLoco, EDI);

	if (auto pFoot = pLoco->LinkedTo)
	{
		if (auto pTrans = pFoot->Transporter)
		{
			R->EAX(pTrans->GetCell()->SlopeIndex);
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4B07CA, DriveLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion*, pLoco, ESI);
	auto const pDrive = static_cast<DriveLocomotionClass*>(pLoco);
	TechnoExt::PlayAnim(RulesGlobal->Wake, pDrive->LinkedTo);
	return 0x4B0828;
}

DEFINE_HOOK(0x69FE92, ShipLocomotionClass_Process_WakeAnim, 0x5)
{
	GET(ILocomotion*, pLoco, ESI);
	auto const pShip = static_cast<ShipLocomotionClass*>(pLoco);
	TechnoExt::PlayAnim(RulesGlobal->Wake, pShip->LinkedTo);
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

DEFINE_HOOK(0x70253F, TechnoClass_TakeDamage_Metallic_AnimDebris, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EDI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x30));
	GET(int, nIdx, EAX);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	GameConstruct(pAnim, RulesGlobal->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, Receivedamageargs.Attacker ? Receivedamageargs.Attacker->GetOwningHouse() : Receivedamageargs.SourceHouse, pThis->GetOwningHouse(), false);

	return 0x70256B;
}

DEFINE_HOOK(0x702484, TechnoClass_TakeDamage_AnimDebris, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	GET(AnimClass*, pAnim, EBX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0xC4, 0x3C));
	GET(int, nIdx, EDI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFS(0xC4, -0x4));

	//well , the owner dies , so taking Invoker is not nessesary here ,..
	GameConstruct(pAnim, pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, Receivedamageargs.Attacker ? Receivedamageargs.Attacker->GetOwningHouse() : Receivedamageargs.SourceHouse, pThis->GetOwningHouse(), false);

	return 0x7024AF;
}

//ObjectClass TakeDamage , 5F559C
//UnitClass TakeDamage , 4428E4 , 737F0E

DEFINE_HOOK(0x703819, TechnoClass_Cloak_Deselect, 0x6)
{
	return R->ESI<TechnoClass*>()->Owner->IsControlledByCurrentPlayer() ? 0x70383C : 0x703828;
}

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

DEFINE_HOOK(0x54DCE8, JumpetLocomotionClass_DrawMatrix, 0x9)
{
	GET(ILocomotion*, pILoco , ESI);
	auto const pLoco = static_cast<JumpjetLocomotionClass*>(pILoco);
	return LocomotionClass::End_Piggyback(pLoco->Owner->Locomotor) ? 0x0 : 0x54DF13;
}

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum
	{
		ContinueCheck = 0x6FC23A,
		BypassCheck = 0x6FC24D
	};

	GET(TechnoClass*, pThis, ESI);
	const bool Allow = RulesExt::Global()->AutoAttackICedTarget.Get() || pThis->Owner->IsInPlayerControl;
	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->AllowFire_IroncurtainedTarget.Get(Allow) ? BypassCheck : ContinueCheck;
}

DEFINE_HOOK(0x722FFA, TiberiumClass_Grow_CheckMapCoords, 0x6)
{
	enum
	{
		increment = 0x72312F,
		SetCell = 0x723005
	};

	GET(const MapSurfaceData*, pSurfaceData, EBX);
	R->EBX(pSurfaceData);
	const auto nCell = pSurfaceData->MapCoord;

	if (!Map.IsValidCell(nCell))
	{
		Debug::Log("Tiberium Growth With Invalid Cell ,Skipping !\n");
		return increment;
	}

	R->EAX(Map[nCell]);
	return SetCell;
}

//TaskForces_LoadFromINIList_WhySwizzle , 0x5
DEFINE_JUMP(LJMP, 0x6E8300, 0x6E8315)

DEFINE_HOOK(0x4DC124, FootClass_DrawActionLines_Attack, 0x5)
{
	GET(FootClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Attack_Color.isset())
	{
		if (pTypeExt->CommandLine_Attack_Color.Get() == ColorStruct::Empty)
			return 0x4DC1A0;
		else
			R->EDI(Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color));
	}

	return 0x0;
}

DEFINE_HOOK(0x4DC2AB, FootClass_DrawActionLines_Move, 0x5)
{
	GET(FootClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CommandLine_Attack_Color.isset())
	{
		if (pTypeExt->CommandLine_Attack_Color.Get() == ColorStruct::Empty)
			return 0x4DC328;
		else
			R->EDX(Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color));
	}

	return 0x0;
}

DEFINE_HOOK(0x4DBDB6, FootClass_IsCloakable_CloakMove, 0x6)
{
	enum
	{
		Nothing = 0x0,
		ReturnFalse = 0x4DBDEB
	};

	GET(FootClass*, pThis, ESI);
	return (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->CloakMove.Get() && !pThis->Locomotor->Is_Moving()) ? ReturnFalse : Nothing;
}

DEFINE_HOOK(0x4F9AF0, HouseClass_IsAlly_AbstractClass, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	bool res = false;
	if (auto pObject = generic_cast<ObjectClass*>(pTarget))
	{
		res = pThis->IsAlliedWith(pObject);
	}
	else if (pTarget)
	{
		if (auto pHouse = pTarget->GetOwningHouse())
			res = pThis->IsAlliedWith(pHouse);
	}

	R->AL(res);
	return 0x4F9B11;
}

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

DEFINE_HOOK(0x50CA30, HouseClass_Center_50C920, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->BelongsToATeam())
		return 0x50CAB4;

	R->CL(pThis->GetTechnoType()->DeploysInto->ConstructionYard);
	return 0x50CA3C;
}

//DEFINE_HOOK(0x737E66, UnitClass_TakeDamage_Debug, 0x8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x48, -0xC));
//	Debug::Log("[%d] %s Warhead Destroying %s ! \n ", pWH, pWH->ID, pThis->get_ID());
//	return 0x0;
//}

DEFINE_HOOK(0x518313, InfantryClass_TakeDamage_JumpjetExplode, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pThisType, EAX);

	if (pThisType->JumpJet)
	{
		TechnoExt::PlayAnim(RulesGlobal->InfantryExplode, pThis);
		return 0x5185F1;
	}

	return 0x518362;
}

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

DEFINE_HOOK(0x54D20F, JumpjetLocomotionClass_MovementAI_Deactivated_Wobble, 0x9)
{
	GET(JumpjetLocomotionClass*, pThis, ESI);

	if (const auto pUnit = specific_cast<UnitClass*>(pThis->LinkedTo))
		return pUnit->IsDeactivated() ? 0x54D23A : 0x0;

	return 0x0;
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

DEFINE_HOOK(0x6AB63B, SelectClass_Action_UnableToBuild, 0x6)
{
	GET(TechnoTypeClass*, pTech, EAX);

	if (HouseClass::CurrentPlayer()->CanBuild(pTech, 0, 0) == CanBuildResult::Buildable)
		return 0;

	ClearShit(pTech);
	return 0x6AB95A;
}

DEFINE_HOOK(0x6A9791, StripClass_DrawIt_BuildingFix, 0x6)
{
	GET(BuildingTypeClass*, pTech, EBX);
	auto const pHouse = HouseClass::CurrentPlayer();

	const auto pFac = pHouse->GetPrimaryFactory(pTech->WhatAmI(), pTech->Naval, pTech->BuildCat);
	if (pFac && pFac->Object->GetTechnoType() != pTech)
		R->Stack(0x17, 1);


	return 0;
}

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

DEFINE_HOOK(0x4CA0F8, FactoryClass_AbandonProduction_RemoveProduct, 0x7)
{
	GET(TechnoClass*, pProduct, ECX);
	pProduct->UnInit();
	return 0x4CA0FF;
}

// TODO : more ?
//DEFINE_HOOK(0x489A97, ExplosionDamage_DetonateOnEachTarget, 0x7)
//{
//	GET(ObjectClass*, pTarget, ESI);
//	GET_BASE(const WarheadTypeClass*, pWH, 0xC);
//	GET_BASE(TechnoClass*, pSource, 0x8);
//	//GET_BASE(HouseClass*, pHouse, 0x14);
//
//	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
//		if (auto pTechnp = generic_cast<TechnoClass*>(pTarget)) {
//			Debug::Log("[%x]=%s MapClass::ExplosionDamage Detonating To [%x]= %s \n", pSource , pSource ? pSource->get_ID() : NONE_STR, pTechnp, pTechnp->get_ID());
//		}
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7) {
//	GET(InfantryClass*, pThis, ECX);
//	return pThis && pThis->DeployAnim ? 0x5192BC : 0;
//}

//DEFINE_HOOK_AGAIN(0x534F4E, ScoreClass_LoadMix, 0x5)
//DEFINE_HOOK(0x6D97BF , ScoreClass_LoadMix, 0x5)
//{
//	Debug::Log("%s\n", R->ESP<char*>());
//	return R->Origin() + 5;
//}

//DEFINE_HOOK_AGAIN(0x70FC90, TechnoClass_Deactivate, 0x6)
//DEFINE_HOOK(0x70FBE0, TechnoClass_Deactivate, 0x6)
//{
//	GET(TechnoClass*, pThis, ECX);
//
//	if (auto pType = pThis->GetTechnoType()) {
//		if (pType->PoweredUnit && pThis->Owner) {
//			pThis->Owner->RecheckPower = true;
//		}
//	}
//
//	return 0x0;
//}

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
	GET(TechnoClass*, pThis, ESI);

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	R->AL(pThis->IsBeingWarpedOut() || (pTechnoTypeExt->Get()->Locomotor == LocomotionClass::CLSIDs::Teleport &&
		pThis->IsWarpingIn() && pTechnoTypeExt->ChronoDelay_Immune.Get()));

	return 0x701AB7;
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

DEFINE_HOOK(0x71B14E, TemporalClass_Fire_ClearTarget, 0x9)
{
	GET(TemporalClass*, pThis, ESI);

	auto pTargetTemp = pThis->Target->TemporalImUsing;

	if (pTargetTemp && pTargetTemp->Target)
		pTargetTemp->LetGo();

	if (pThis->Target->Owner == HouseClass::CurrentPlayer())
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

//
//DEFINE_HOOK(0x73F7DD, UnitClass_IsCellOccupied_Bib, 0x8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET(TechnoClass*, pThat, EBX);
//	return pThis && pThat->Owner && pThat->Owner->IsAlliedWith(pThis) ? 0x0 : 0x73F823;
//}
//

DEFINE_HOOK(0x51D45B, InfantryClass_Scatter_Process, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->JumpJet && pThis->Type->HoverAttack)
	{
		pThis->SetDestination(nullptr, 1);
	}
	else
	{
		//Interfance already checked above
		pThis->Locomotor.get()->Process();
	}

	return 0x51D47B;
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

DEFINE_HOOK(0x7091FC, TechnoClass_CanPassiveAquire_AI, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	if ((pThis->Owner && !pThis->Owner->IsControlledByCurrentPlayer()))
	{
		R->CL(TechnoTypeExt::ExtMap.Find(pType)->PassiveAcquire_AI.Get(pType->CanPassiveAquire));
		return 0x709202;
	}

	return 0x0;
}

DEFINE_HOOK(0x5D3ADE, MessageListClass_Init_MessageMax, 0x6)
{
	if (Phobos::Otamaa::IsAdmin)
		R->EAX(14);

	return 0x0;
}

DEFINE_HOOK(0x5B3614, MissionClass_AssignMission_CheckBuilding, 0x6)
{
	GET(MissionClass*, pThis, ESI);
	GET(Mission, nMission, EAX);

	if (pThis->WhatAmI() == AbstractType::Building && nMission == Mission::Hunt)
		pThis->QueuedMission = Mission::Guard;
	else
		pThis->QueuedMission = nMission;

	return 0x5B361A;
}

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

DEFINE_HOOK(0x6B721F, SpawnManagerClass_Manage_Clear, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	pThis->Target = nullptr;
	pThis->NewTarget = nullptr;
	pThis->Status = SpawnManagerStatus::Idle;
	return 0x0;
}

DEFINE_HOOK(0x62A929, ParasiteClass_CanInfect_Additional, 0x6)
{
	enum
	{
		returnfalse = 0x62A976,
		continuecheck = 0x0,
		continuecheckB = 0x62A933
	};

	GET(FootClass*, pVictim, ESI);
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());

	return pVictim->IsIronCurtained() || pVictim->IsBeingWarpedOut() || (pTechnoTypeExt->Get()->Locomotor == LocomotionClass::CLSIDs::Teleport &&
		pVictim->IsWarpingIn() && pTechnoTypeExt->ChronoDelay_Immune.Get()) ? returnfalse : !pVictim->BunkerLinkedItem ? continuecheckB : returnfalse;
}

//DEFINE_HOOK(0x6A78F6, SidebarClass_AI_RepairMode_ToggelPowerMode, 0x9)
//{
//	GET(SidebarClass*, pThis, ESI);
//	if (Phobos::Config::TogglePowerInsteadOfRepair)
//		pThis->SetTogglePowerMode(-1);
//	else
//		pThis->SetRepairMode(-1);
//	return 0x6A78FF;
//}
//
//DEFINE_HOOK(0x6A7AE1, SidebarClass_AI_DisableRepairButton_TogglePowerMode, 0x6)
//{
//	GET(SidebarClass*, pThis, ESI);
//	R->AL(Phobos::Config::TogglePowerInsteadOfRepair ? pThis->PowerToggleMode : pThis->RepairMode);
//	return 0x6A7AE7;
//}

DEFINE_HOOK(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1)
		{
			return 0x70D407;
		}
	}

	return TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsDummy ? 0x70D407 : 0x0;
}

DEFINE_HOOK(0x663225, RocketLocomotionClass_DetonateOnTarget_Anim, 0x6)
{
	GET(AnimClass*, pMem, EAX);
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CellStruct const, nCell, STACK_OFFS(0x60, 0x38));
	REF_STACK(CoordStruct const, nCoord, STACK_OFFS(0x60, 0x18));
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0x60, 0x50));

	GET(int, nDamage, EDI);

	const auto pCell = Map.GetCellAt(nCell);
	if (auto pAnimType = Map.SelectDamageAnimation(nDamage, pWarhead, pCell ? pCell->LandType : LandType::Clear, nCoord))
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

DEFINE_HOOK(0x70166E, TechnoClass_Captured, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	pThis->UpdatePlacement(PlacementType::Remove);
	return 0x70167A;
}

DEFINE_HOOK(0x6F09C4, TeamTypeClass_CreateOneOf_RemoveLog, 0x5)
{
	GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x8, -0x4));
	R->EDI(pHouse);
	return 0x6F09D5;
}

// ToDO : Make Optional
DEFINE_HOOK(0x6F0A3F, TeamTypeClass_CreateOneOf_CreateLog, 0x6)
{
	GET(TeamTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	Debug::Log("[%x][%s] Creating a new team named '%s'.\n", pHouse, pHouse ? pHouse->get_ID() : NONE_STR2, pThis->ID);
	R->EAX(YRMemory::Allocate(sizeof(TeamClass)));
	return 0x6F0A5A;
}

// ToDO : Make Optional
DEFINE_HOOK(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5)
{
	return 0x44DE3C;
}

// ToDO : Make Optional
DEFINE_HOOK(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
{
	GET(FactoryClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);
	Debug::Log("[%x] Factory with Owner '%s' Abandoning production of '%s' \n", pThis, pThis->Owner ? pThis->Owner->get_ID() : NONE_STR2, pType->ID);
	R->ECX(pThis->Object);
	return 0x4CA021;
}

// ToDO : Make Optional
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

DEFINE_HOOK(0x4495FF, BuildingClass_ClearFactoryBib_Log1, 0xA)
{
	return 0x44961A;
}

DEFINE_HOOK(0x449657, BuildingClass_ClearFactoryBib_Log2, 0xA)
{
	return 0x449672;
}

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
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type))
		{
			if (auto pExt = TechnoExt::ExtMap.Find(pInf))
			{
				if (pExt->EngineerCaptureDelay.InProgress())
				{
					return 0x4D6A01;
				}
			}
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
	enum { ContinueCheck = 0x5F54C4 , ResultHalf = 0x5F54B8 };

	GET(int, nOldStr, EDX);
	GET(int, nCurStr, EBP);
	GET(int, nDamage, ECX);

	const auto curstr = Game::F2I(nCurStr * RulesGlobal->ConditionYellow);
	return (nOldStr <= curstr || !((nOldStr - nDamage) < curstr)) ? ContinueCheck : ResultHalf;
}

//DEFINE_HOOK(0x6FDDC0, TechnoClass_Fire_RememberAttacker, 0x6)
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

//DEFINE_HOOK(0x5F53ED, ObjectClass_ReceiveDamage_DisableComplierOptimization, 0x6)
//{
//	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));
//
//	return args->IgnoreDefenses ? 0x5F5416 : 0x5F53F3;
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

// Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
// Fixes recursive passenger kills not being accredited
// to proper techno but to their transports
//DEFINE_PATCH(0x707CF2, 0x55);

//DEFINE_HOOK(0x417FD0, AircraftClass_PoseDIr, 0x5)
//{
//	GET(AircraftClass*, pThis, ECX);
//	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//	R->EAX(pExt->PoseDir.Get(RulesGlobal->PoseDir));
//	return 0x417FD8;
//}

CoordStruct GetPutLocation(CoordStruct current, int distance)
{
	// this whole thing does not at all account for cells which are completely occupied.
	auto tmpCoords = CellSpread::GetCell(ScenarioClass::Instance->Random.RandomRanged(0, 7));

	current.X += tmpCoords.X * distance;
	current.Y += tmpCoords.Y * distance;

	auto tmpCell = MapClass::Instance->GetCellAt(current);
	auto target = tmpCell->FindInfantrySubposition(current, false, false, false);

	target.Z = current.Z;
	return target;
}

bool EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select)
{
	CellClass* pCell = MapClass::Instance->TryGetCellAt(loc);

	if (!pCell)
	{
		return false;
	}

	Survivor->OnBridge = pCell->ContainsBridge();

	int floorZ = pCell->GetCoordsWithBridge().Z;
	bool chuted = (loc.Z - floorZ > 2 * Unsorted::LevelHeight);
	if (chuted)
	{
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Limbo();

		if (!Survivor->SpawnParachuted(loc) || pCell->GetBuilding())
		{
			return false;
		}
	}
	else
	{
		loc.Z = floorZ;
		if (!Survivor->Unlimbo(loc, static_cast<DirType>(ScenarioGlobal->Random.RandomFromMax(7))))
		{
			return false;
		}
	}

	Survivor->Transporter = nullptr;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if (chuted)
	{
		bool scat = Survivor->OnBridge;
		auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;
		if ((occupation & 0x1C) == 0x1C)
		{
			pCell->ScatterContent(CoordStruct::Empty, true, true, scat);
		}
	}
	else
	{
		Survivor->Scatter(CoordStruct::Empty, true, false);
		Survivor->QueueMission(Survivor->Owner->IsControlledByHuman() ? Mission::Guard : Mission::Hunt, 0);
	}

	Survivor->ShouldEnterOccupiable = false;
	Survivor->ShouldGarrisonStructure = false;

	if (Select)
	{
		Survivor->Select();
	}

	return true;
	//! \todo Tag
}

bool EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select)
{
	CoordStruct destLoc = GetPutLocation(location, distance);
	return EjectSurvivor(pEjectee, destLoc, select);
}

DEFINE_HOOK(0x737F86, UnitClass_TakeDamage_DoBeforeAres, 0x6)
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
		auto const location = pThis->Location;

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
			if (trySpawn && EjectRandomly(pPassenger, location, 128, select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			pPassenger->UnInit();
		}
	}

	return 0x737F97;
}

DEFINE_HOOK(0x41668B, AircraftClass_TakeDamage_DoBeforeAres, 0x6)
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
		auto const location = pThis->Location;

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
			if (trySpawn && EjectRandomly(pPassenger, location, 128, select))
			{
				continue;
			}

			// kill passenger, if not spawned
			pPassenger->RegisterDestruction(pKiller);
			pPassenger->UnInit();
		}
	}

	return 0x0;
}

int AnimClass_Expired_SpawnsParticle(REGISTERS* R)
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
		const auto v8 = nCoord.Z - Map.GetCellFloorHeight(nCoord);
		const auto v17 = 6.283185307179586 / nNumParticles;
		double v16 = 0.0;

		if (nNumParticles > 0)
		{
			for(; nNumParticles; --nNumParticles)
			{
				auto v13 = abs(ScenarioGlobal->Random.RandomRanged(nMin, nMax));
				auto v10 = ScenarioGlobal->Random.RandomDouble() * v17 + v16;
				auto v18 = Math::cos(v10);
				auto v9 = Math::sin(v10);
				CoordStruct nCoordB { nCoord.X + static_cast<int>(v13 * v18),nCoord.Y - static_cast<int>(v9 * v13), nCoord.Z };
				nCoordB.Z = v8 + Map.GetCellFloorHeight(nCoordB);
				ParticleSystemClass::Instance->SpawnParticle(pType, &nCoordB);
				v16 += v17;
			}
		}
	}

	return 0x42504D;
}