#include "Hooks.OtamaaBugFix.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
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

//dont bother to clear type pointer
//DEFINE_JUMP(LJMP, 0x4251A3, 0x4251B1);

//static void __fastcall _DrawBehindAnim(TechnoClass* pThis, void* _, Point2D* pWhere, RectangleStruct* pBounds)
//{
//	if (!pThis->GetTechnoType()->Invisible)
//		pThis->DrawBehind(pWhere, pBounds);
//}

//DEFINE_JUMP(CALL,0x6FA2D3, GET_OFFSET(_DrawBehindAnim));

DEFINE_HOOK(0x6FA2C7 , TechnoClass_AI_DrawBehindAnim , 0x8) //was 4
{
	GET(TechnoClass* , pThis , ESI);
	GET_STACK(Point2D , nPoint , STACK_OFFS(0x78 ,0x50));
	GET_STACK(RectangleStruct , nBound , STACK_OFFS(0x78 ,0x50));

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

	Debug::Log("Team[%x] script [%s]=[%d] , Failed to find type[%d] building at idx[%d] ! \n", pThis, nScript->Type->get_ID(), nScript->CurrentMission, nTypeIdx, nBuildingIdx);
	return 0x6EE7C3;
}

//Lunar limitation
DEFINE_JUMP(LJMP,0x546C8B, 0x546CBF);

static DamageAreaResult __fastcall _RocketLocomotionClass_DamageArea(
	CoordStruct* pCoord ,
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

DEFINE_JUMP(CALL,0x6632C7, GET_OFFSET(_RocketLocomotionClass_DamageArea));

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
		ScenarioGlobal->Random(0, pRules->VeinholeShrinkRate / 2) : 0;

	R->EAX(pRules->VeinholeShrinkRate + nRand);
	return 0x74D37C;
}

DEFINE_HOOK(0x74C5E1, VeinholeMonsterClass_CTOR_TSRandomRate, 0x6)
{
	GET(RulesClass*, pRules, EAX);
	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioGlobal->Random(0, pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74C5E7;
}

DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
{
	GET(RulesClass*, pRules, ECX);

	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioGlobal->Random(0, pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74D2AA;
}

static	void __fastcall DrawShape_VeinHole
(	Surface* Surface,
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
	 , TintColor, ZShape, ZShapeFrame,XOffset, YOffset);
}

DEFINE_JUMP(CALL,0x74D5BC, GET_OFFSET(DrawShape_VeinHole));

//static	void __fastcall Replace_VeinholeShapeLoad(TheaterType nTheater)
//{
//	//TheaterTypeClass::GetCharExtension(nTheater)
//	std::string flag { "VEINHOLE." };
//	flag += Theater::GetTheater(nTheater).Extension;
//	if (auto const pImage = FileSystem::LoadSHPFile(flag.c_str()))
//		VeinholeMonsterClass::VeinSHPData = pImage;
//}
//
//DEFINE_JUMP(CALL,0x685136, GET_OFFSET(Replace_VeinholeShapeLoad));

//static void __fastcall DisplayClass_ReadINI_add(TheaterType nTheater)
//{
//	SmudgeTypeClass::TheaterInit(nTheater);
//	Replace_VeinholeShapeLoad(nTheater);
//}
//
//DEFINE_JUMP(CALL,0x4AD0A3, GET_OFFSET(DisplayClass_ReadINI_add));

DEFINE_HOOK(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	auto nTheater = ScenarioGlobal->Theater;
	SmudgeTypeClass::TheaterInit(nTheater);
	VeinholeMonsterClass::TheaterInit(nTheater);
	return 0x4AD0A8;
}

//static int __fastcall SelectParticle(char* pName) {
//	return RulesExt::Global()->VeinholeParticle.Get(ParticleTypeClass::FindIndex(pName));
//}
//
//DEFINE_JUMP(CALL,0x74D0DF, GET_OFFSET(SelectParticle));

DEFINE_HOOK(0x74D0D2, VeinholeMonsterClass_AI_SelectParticle, 0x5)
{
	//overriden instructions
	R->Stack(0x2C, R->EDX());
	R->Stack(0x30, R->EAX());
	auto const pDefault = Make_Global<const char*>(0x84610C);
	R->EAX(RulesExt::Global()->VeinholeParticle.Get(ParticleTypeClass::FindIndex(pDefault)));
	return 0x74D0E4;
}

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(TechnoClass*, pTechnoOwner, EAX);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	//Debug::Log("Wave Receive Damage for Victim [%x] ! \n", pVictim);
	if(const auto pUnit = specific_cast<UnitClass*>(pVictim))
		if(pUnit->DeathFrameCounter > 0)
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

DEFINE_HOOK(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6) {
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

DEFINE_HOOK(0x62A933, ParasiteClass_CanInfect_ParasitePointerGone_Check, 0x5)
{
	GET(ParasiteClass*, pThis, EDI);

	if (!pThis)
		Debug::Log("Found Invalid ParasiteClass Pointer ! , Skipping ! \n");

	return pThis ? 0x0 : 0x62A976;
}

DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackAllies, 0x5) {
	GET(const TechnoClass* , pThis , ESI);
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
											(pExt && pExt->Owner) ? pExt->Owner : nullptr
								, pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner, false);

	}

	return 0x4668BD;
}

DEFINE_HOOK(0x6FF394, TechnoClass_FireAt_FeedbackAnim, 0x8)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		if (auto pAnimType = pWeaponExt->Feedback_Anim.Get()) {
			const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCenterCoord()) + pWeaponExt->Feedback_Anim_Offset;
			if (auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord)) {
				AnimExt::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
				if (pThis->WhatAmI() != AbstractType::Building)
					pFeedBackAnim->SetOwnerObject(pThis);
			}
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
	GET_STACK(BuildingTypeClass*, PlacingObject, STACK_OFFS(0x18 , -0x8));
	GET_STACK(HouseClass*, PlacingOwner, STACK_OFFS(0x18, -0xC));

	enum { Adequate = 0x47CA70, Inadequate = 0x47C94F } Status = Inadequate;

	HouseClass* OverlayOwner = nHouseIDx >= 0 ? HouseClass::Array->GetItem(nHouseIDx) : nullptr;

	if (PlacingObject) {
		bool ContainsWall = idxOverlay != -1 && OverlayTypeClass::Array->GetItem(idxOverlay)->Wall;

		if (ContainsWall && (PlacingObject->Gate || RulesExt::Global()->WallTowers.Contains(PlacingObject))) {
			Status = Adequate;
		}

		if (OverlayTypeClass* ToOverlay = PlacingObject->ToOverlay)	{
			if (ToOverlay->ArrayIndex == idxOverlay) {
				if (pCell->OverlayData >= 0x10)	{
					Status = Adequate;
				}
			}
		}
	}

	if (Status == Inadequate) {
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

	if (Status == Adequate)	{
		if (PlacingOwner != OverlayOwner) {
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
DEFINE_JUMP(LJMP, 0x501640, 0x50174E)

//remove ISHuman check
DEFINE_JUMP(LJMP, 0x6A55BF, 0x6A55C8)
DEFINE_JUMP(LJMP, 0x6A57F6 , 0x6A57FF)

DEFINE_HOOK(0x4FD635, HouseClass_AI_UpdatePlanOnEnemy_FixDistance, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pEnemy, EBX);

	if (pThis->IsAlliedWith(pEnemy))
		R->EAX(INT_MAX);
	else
		R->EAX(pThis->BaseCenter ? pThis->BaseCenter.X : pThis->BaseSpawnCell.X);

	return 0x4FD657;
}

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

static BulletClass* Fuse_Bullet = nullptr;
DEFINE_HOOK(0x467C2A, BulletClass_AI_Fuse_FetchBullet, 0x5)
{
	Fuse_Bullet = R->EBP<BulletClass*>();
	return 0x0;
}

DEFINE_HOOK(0x4E1278, FuseClass_BulletProximity, 0x5)
{
	GET(int, nRange, EAX);
	auto const pBullet = Fuse_Bullet;

	int nProx = 32;
	if (auto pExt = BulletExt::ExtMap.Find(pBullet)) {
		if (pExt->TypeExt->Proximity_Range.isset())
			nProx = pExt->TypeExt->Proximity_Range.Get() * 256;
	}

	Fuse_Bullet = nullptr;
	return (nProx) <= nRange ? 0x4E1289 : 0x4E127D;
}

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
	return R->ESI<TechnoClass*>()->Owner->IsPlayerControl() ? 0x70383C : 0x703828;
}

DEFINE_HOOK(0x54DCD2, JumpetLocomotionClass_DrawMatrix, 0x8)
{
	GET(FootClass*, pFoot, ECX);

	bool Allow = false;
	if (pFoot->GetTechnoType()->TiltCrashJumpjet) {
		Allow = LocomotionClass::End_Piggyback(pFoot->Locomotor);
	}

	return Allow ? 0x54DCE8 : 0x54DF13;
}

//DEFINE_HOOK(0x54C14B, JumpjetLocomotionClass_State3, 0x7)
//{
//	GET(FootClass*, pFoot, EDI);
//
//	if (pFoot->GetTechnoType()->Sensors)
//		pFoot->UpdatePosition(2);
////
//	return 0;
//}

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum
	{
		ContinueCheck = 0x6FC23A,
		BypassCheck = 0x6FC24D
	};

	GET(TechnoClass*, pThis, ESI);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	return pTypeExt->AllowFire_IroncurtainedTarget.Get(pThis->Owner->PlayerControl) ? BypassCheck : ContinueCheck;

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

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pTypeExt->CommandLine_Attack_Color.isset())
		{
			if (pTypeExt->CommandLine_Attack_Color.Get() == ColorStruct::Empty)
				return 0x4DC1A0;
			else
				R->EDI(Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color));
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4DC2AB, FootClass_DrawActionLines_Move, 0x5)
{
	GET(FootClass*, pThis, ESI);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pTypeExt->CommandLine_Attack_Color.isset())
		{
			if (pTypeExt->CommandLine_Attack_Color.Get() == ColorStruct::Empty)
				return 0x4DC328;
			else
				R->EDX(Drawing::RGB2DWORD(pTypeExt->CommandLine_Attack_Color));
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4DBDB6, FootClass_IsCloakable_CloakMove , 0x6)
{
	enum
	{
		Nothing = 0x0,
		ReturnFalse = 0x4DBDEB
	};
	GET(FootClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->CloakMove.Get() && !pThis->Locomotor->Is_Moving()) {
		return ReturnFalse;
	}

	return Nothing;
}

DEFINE_HOOK(0x4F9AF0, HouseClass_IsAlly_AbstractClass, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	bool res = false;
	if (auto pObject = generic_cast<ObjectClass*>(pTarget)) {
		res = pThis->IsAlliedWith(pObject);
	}
	else if (pTarget) {
		if(auto pHouse = pTarget->GetOwningHouse())
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

DEFINE_HOOK(0x4FC585, HouseClass_MPlayerDefeated_3, 0x6)
{
	REF_STACK(int, nHuman, 0x18);

	for (auto pHouse : *HouseClass::Array) {
		nHuman += pHouse->IsPlayerControl() && (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"));
	}

	return 0;
}

DEFINE_HOOK(0x50CA30, HouseClass_Center_50C920, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->BelongsToATeam())
		return 0x50CAB4;

	R->CL(pThis->GetTechnoType()->DeploysInto->ConstructionYard);
	return 0x50CA3C;
}

DEFINE_HOOK(0x518313, InfantryClass_TakeDamage_JumpjetExplode, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pThisType, EAX);

	if (pThisType->JumpJet) {
		TechnoExt::PlayAnim(RulesGlobal->InfantryExplode, pThis);
		return 0x5185F1;
	}

	return 0x518362;
}
//
DEFINE_HOOK(0x4A9004, MouseClass_CanPlaceHere_SkipSelf, 0x6)
{
	if (auto const pHouse = HouseClass::Array->GetItem(R->EAX<int>())) {
		if (pHouse == R->ECX<HouseClass*>())
			return 0x4A902C;
	}

	return 0x0;
}

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

DEFINE_HOOK(0x5F54A8, ObjectClass_ReceiveDamage_ConditionYellow, 0x6)
{
	enum
	{
		ContinueCheck = 0x5F54C4
		, ResultHalf = 0x5F54B8
	};

	GET(int, nOldStr, EDX);
	GET(int, nCurStr, EBP);
	GET(int, nDamage, ECX);

	const auto curstr = static_cast<int>(static_cast<double>(nCurStr) * RulesGlobal->ConditionYellow);
	return (nOldStr <= curstr || !((nOldStr - nDamage) < curstr)) ? ContinueCheck : ResultHalf;
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

	if (HouseClass::Player()->CanBuild(pTech, 0, 0) == CanBuildResult::Buildable)
		return 0;

	ClearShit(pTech);
	return 0x6AB95A;
}

DEFINE_HOOK(0x6A557A, SidebarClass_Init_EnableSkirmish, 0x5) //changed
{
	auto const nMode = SessionClass::Instance->GameMode;
	return (nMode == GameMode::Skirmish || nMode == GameMode::Internet || nMode == GameMode::LAN) ? 0x6A558D : 0x6A5830;
}

DEFINE_HOOK(0x6A9791, StripClass_DrawIt_BuildingFix, 0x6)
{
	GET(BuildingTypeClass*, pTech, EBX);
	auto const pHouse = HouseClass::Player();

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
//	if (!pHouse->IsPlayerControl())
//	{
//		nMoney = static_cast<int>(nMoney * 100.0 / RulesGlobal->MultiplayerAICM.GetItem(static_cast<int>(pHouse->AIDifficulty)));
//	}
//
//	R->EAX(nMoney);
//	return R->Origin() + 6;
//}

DEFINE_HOOK(0x7091FC , TechnoClass_CanPassiveAquire_AI , 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, EAX);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pTypeExt || (pThis->Owner && pThis->Owner->IsPlayerControl()))
		return 0x0;

	R->CL(pTypeExt->PassiveAcquire_AI.Get(pType->CanPassiveAquire));
	return 0x709202;
}

DEFINE_HOOK(0x6FF92F , TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if(const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon)){
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

// TODO : more stack ?
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

DEFINE_HOOK(0x518F90, InfantryClass_DrawIt_HideWhenDeployAnimExist, 0x7) {
	GET(InfantryClass*, pThis, ECX);
	return pThis && pThis->DeployAnim ? 0x5192BC : 0;
}

//DEFINE_HOOK_AGAIN(0x534F4E, ScoreClass_LoadMix, 0x5)
//DEFINE_HOOK(0x6D97BF , ScoreClass_LoadMix, 0x5)
//{
//	Debug::Log("%s\n", R->ESP<char*>());
//	return R->Origin() + 5;
//}

DEFINE_HOOK(0x00, TechnoClass_Deactivate, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	if (auto pType = pThis->GetTechnoType()) {
		if (pType->PoweredUnit && pThis->Owner) {
			pThis->Owner->RecheckPower = true;
		}
	}

	return 0x0;
}

#ifndef ENABLE_TOMSOnOVERLAYWRAPPER
static int __fastcall Isotile_LoadFile_Wrapper(IsometricTileTypeClass* pTile, void* _)
{
	bool available = false;
	int file_size = 0;

	{
		CCFileClass file(pTile->FileName);
		available = file.Exists();
		file_size = file.GetFileSize();
	}

	if (!available) {
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s is missing!\n", pTile->FileName);
		return 0;
	}

	if (file_size == 0) {
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s is a empty file!\n", pTile->FileName);
		return 0;
	}

	int read_size = pTile->LoadTile();

	if (pTile->Image == nullptr) {
		Debug::Log("ISOTILEDEBUG - Failed to load image for Isometric Tile %s!\n", pTile->FileName);
		return 0;
	}

	if (read_size != file_size) {
		Debug::Log("ISOTILEDEBUG - Isometric Tile %s file size %d doesn't match read size!\n", file_size, read_size, pTile->FileName);
	}

	return read_size;
}

//544C3F
DEFINE_JUMP(CALL,0x544C3F, GET_OFFSET(Isotile_LoadFile_Wrapper));
//544C97
DEFINE_JUMP(CALL,0x544C97, GET_OFFSET(Isotile_LoadFile_Wrapper));
//544CC9
DEFINE_JUMP(CALL,0x544CC9, GET_OFFSET(Isotile_LoadFile_Wrapper));
//546FCC
DEFINE_JUMP(CALL,0x546FCC, GET_OFFSET(Isotile_LoadFile_Wrapper));
//549AF7
DEFINE_JUMP(CALL,0x549AF7, GET_OFFSET(Isotile_LoadFile_Wrapper));
//549E67
DEFINE_JUMP(CALL,0x549E67, GET_OFFSET(Isotile_LoadFile_Wrapper));
#endif

#ifdef ENABLE_NEWCHECK

DEFINE_HOOK(0x4A23A8, CreditClass_Graphic_Logic_ReplaceCheck, 0x8)
{
	auto const pHouse = HouseClass::Player();
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ?
		0x4A23B0 : 0x4A24F4;
}

DEFINE_HOOK(0x4A2614, CreditClass_Graphic_AI_ReplaceCheck, 0x8)
{
	auto const pHouse = HouseClass::Player();
	R->EAX(pHouse);
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ?
		0x4A261D : 0x4A267D;
}

DEFINE_HOOK(0x5094F9, HouseClass_AdjustThreats, 0x6)
{
	return R->EBX<HouseClass*>()->IsAlliedWith(R->ESI<HouseClass*>()) ? 0x5095B6 : 0x509532;
}

DEFINE_HOOK(0x4F9432, HouseClass_Attacked, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4F9474 : 0x4F9478;
}

DEFINE_HOOK(0x4FBD1C, HouseClass_DoesEnemyBuildingExist, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x4FBD57 : 0x4FBD47;

}

DEFINE_HOOK(0x5003BA, HouseClass_FindJuicyTarget, 0x6)
{
	return R->EDI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x5003F7 : 0x5004B1;
}

DEFINE_HOOK(0x501548, HouseClass_IsAllowedToAlly, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EDI<HouseClass*>()) ? 0x501575 : 0x50157C;
}

DEFINE_HOOK(0x5015F2, HouseClass_IsAllowedToAlly_2, 0x6)
{
	return R->ESI<HouseClass*>()->IsAlliedWith(R->EAX<HouseClass*>()) ? 0x501627 : 0x501628;

}

DEFINE_HOOK(0x658393, RadarClass_658330, 0x9)
{
	GET(HouseClass*, pHouses, EBX);

	if (pHouses != HouseClass::Observer() && _strcmpi(pHouses->get_ID(), "Observer"))
		return 0x6583A8;

	R->EDX(1);
	return 0x65839C;
}

DEFINE_HOOK(0x658478, RadarClass_658330_2, 0x6)
{
	GET(HouseClass*, pHouses, EBX);
	return (pHouses != HouseClass::Observer() && !_strcmpi(pHouses->get_ID(), "Observer")) ? 0x658480 : 0x65848A;
}

DEFINE_HOOK(0x657EE3, RadarClass_DiplomacyDialog, 0x6)
{
	auto const pHouse = HouseClass::Player();
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ? 0x657F70 : 0x657EF2;
}

DEFINE_HOOK(0x4FCD88, HouseClass_FlagToLose, 0x5)
{
	auto const pHouse = HouseClass::Player();
	return pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer") ?
		0x4FCDA6 : 0x4FCD97;
}

DEFINE_HOOK(0x4FC262, HouseClass_MPlayerDefeated, 0x6)
{
	auto const pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		? 0x4FC2EF : 0x4FC271;
}

DEFINE_HOOK(0x4FC343, HouseClass_MPlayerDefeated_2, 0x5)
{
	GET(HouseClass*, pThis, ESI);

	if (pThis != HouseClass::Observer() && _strcmpi(pThis->get_ID(), "Observer"))
		return 0;

	R->EAX(pThis);
	return 0x4FC348;
}

DEFINE_HOOK(0x4FC4DF, HouseClass_MPlayer_Defeated, 0x6)
{
	GET(HouseClass*, pThis, EDX);
	GET(HouseClass*, pThat, EAX);

	return (!pThis->IsAlliedWith(pThat)
	  || !pThat->IsAlliedWith(pThis)) ? 0x4FC57C : 0x4FC52D;
}

DEFINE_HOOK(0x4F9CFA, HouseClass_MakeAlly_3, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(TechnoClass*, pThat, EAX);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4F9D34 : 0x4F9D40;
}

DEFINE_HOOK(0x4F9E10, HouseClass_MakeAlly_4, 0x8)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EBP);

	return (!pThis || !pThis->IsAlliedWith(pThat))
		? 0x4F9EC9 : 0x4F9E49;
}

DEFINE_HOOK(0x4F9E5A, HouseClass_MakeAlly_5, 0x5)
{
	GET(HouseClass*, pThis, ESI);
	GET(HouseClass*, pThat, EBP);
	return (!pThis->IsAlliedWith(HouseClass::Player()) || !pThat->IsAlliedWith(HouseClass::Player())) ? 0x4F9EBD : 0x4F9EB1;
}

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x4FADD9 : 0x4FAD9E;
}

DEFINE_HOOK(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x50A292 : 0x50A278;
}

DEFINE_HOOK(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->GetOwningHouse()) ? 0x50A096 : 0x50A087;
}

DEFINE_HOOK(0x5047F5, HouseClass_UpdateAngetNodes, 0x6)
{
	GET(HouseClass*, pThis, EAX);
	GET(HouseClass*, pThat, EDX);

	return pThis->IsAlliedWith(pThat) ? 0x504826 : 0x504820;
}

DEFINE_HOOK(0x5C98E5, MultiplayerScore_5C98A0, 0x6)
{
	GET(HouseClass*, pHouse, EDI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x5C9A7E : 0x5C98F1;
}

DEFINE_HOOK(0x6C6F83, SendStatisticsPacket, 0x6)
{
	auto const pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		? 0x6C6F8B : 0x6C6F9D;
}

DEFINE_HOOK(0x6C7402, SendStatisticsPacket2, 0x8)
{
	GET(HouseClass*, pHouse, EAX);
	GET_STACK(int, nPlayerCount, 0x2C);

	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		return 0x6C7414;

	R->EBX(nPlayerCount);
	return 0x6C740A;
}

DEFINE_HOOK(0x6A55B7, SidebarClass_InitIO, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A55CF : 0x6A55BF;
}

DEFINE_HOOK(0x6A5694, SidebarClass_InitIO2, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A569C : 0x6A56AD;
}

DEFINE_HOOK(0x6A57EE, SidebarClass_InitIO3, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A580E : 0x6A57F6;
}

DEFINE_HOOK(0x6A6AA6, SidebarClass_Scroll, 0x6)
{
	auto const pHouse = HouseClass::Player();
	R->EDX(pHouse);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6AB0 : 0x6A6AC6;
}

DEFINE_HOOK(0x6A7BA2, SidebarClass_Update, 0x5)
{
	GET(HouseClass*, pHouse, EBX);
	R->Stack(0x14, R->EDX());
	return  (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A7BAF : 0x6A7BB7;
}

DEFINE_HOOK(0x6A7BE7, SidebarClass_Update_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);

	if (pHouse != HouseClass::Observer() && _strcmpi(pHouse->get_ID(), "Observer"))
		return 0x6A7C07;

	R->EAX(R->EDX());
	return 0x6A7BED;
}

DEFINE_HOOK(0x6A7CD9, SidebarClass_Update_3, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A7CE3 : 0x6A7CE8;
}

DEFINE_HOOK(0x6A6B75, SidebarClass_handlestrips0, 0x6)
{
	R->Stack(0x10, R->EDX());
	auto const pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6B7D : 0x6A6B85;
}

DEFINE_HOOK(0x6A6BCC, SidebarClass_handlestrips0_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->EAX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A6BD2 : 0x6A6BEC;
}

DEFINE_HOOK(0x6A6615, SidebarClass_togglestuff, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A66EA : 0x6A6623;
}

DEFINE_HOOK(0x6A88D2, StripClass_6A8860, 0x6)
{
	auto const pHouse = HouseClass::Player();
	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		R->ESI(pHouse);

	return 0;
}

DEFINE_HOOK(0x8A898E, StripClass_6A8920, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8998 : 0x6A89B2;
}

DEFINE_HOOK(0x6A8A41, StripClass_6A89E0, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->ECX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8A47 : 0x6A8A4C;;
}

DEFINE_HOOK(0x6A8AA8, StripClass_6A89E0_2, 0x6)
{
	GET(HouseClass*, pHouse, EBX);
	R->EAX(R->EDX());
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8AAE : 0x6A8AD2;
}

DEFINE_HOOK(0x6A95BC, StripClass_DrawIt, 0x5)
{
	GET(StripClass*, pThis, ESI);

	auto const pHouse = HouseClass::Player();
	if (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer"))
		R->EAX(pThis->CameoCount);

	return 0x6A95C1;
}

DEFINE_HOOK(0x6AA04F, StripClass_DrawIt_2, 0x8)
{
	GET(HouseClass*, pHouse, EBX);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6AA057 : 0x6AA59B;;
}

DEFINE_HOOK(0x6A964E, StripClass_DrawIt_3, 0x6)
{
	auto const pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6AA05B : 0x6A9654;
}

DEFINE_HOOK(0x6A8BB4, StripClass_Update, 0x5)
{
	GET(HouseClass*, pHouse, EBP);

	R->ESI(2 * R->EAX());

	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A8BB9 : 0x6A8BCB;
}

DEFINE_HOOK(0x6A9038, StripClass_Update_2, 0x6)
{
	auto const pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A904B : 0x6A9258;

}

DEFINE_HOOK(0x6A9142, StripClass_Update_3, 0x6)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A914A : 0x6A915B;
}

DEFINE_HOOK(0x6A91EE, StripClass_Update_4, 0x5)
{
	GET(HouseClass*, pHouse, ESI);
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x6A91F7 : 0x6A9208;
}

#endif

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


DEFINE_HOOK(0x4870D0, CellClass_SensedByHouses_ObserverAlwaysSensed, 0x6)
{
	GET_STACK(int, nHouseIdx, 0x4);

	const auto pHouse = HouseClass::Array->GetItem(nHouseIdx);
	if (!pHouse || pHouse != HouseClass::Observer() && _strcmpi(pHouse->get_ID(), "Observer"))
		return 0;

	R->AL(1);
	return 0x4870DE;
}

DEFINE_HOOK(0x70DA6D, TechnoClass_SensorAI_ObserverSkipWarn, 0x6)
{
	const auto pHouse = HouseClass::Player();
	return (pHouse == HouseClass::Observer() || !_strcmpi(pHouse->get_ID(), "Observer")) ? 0x70DADC : 0x0;
}


DEFINE_HOOK(0x452831 , BuildingClass_Overpowerer_AddUnique, 0x6)
{
	enum {
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


#ifdef DetailsPatch


struct FakeRulesExt
{
	struct ExtData
	{
		Valueable<int>  DetailMinFrameRateMedium { 0 };
		Valueable<bool> DetailLowDisableBullet { false };
	};


private:
	static std::unique_ptr<ExtData> Data;
public:
	static  ExtData* Global() { return Data.get(); }

	static bool DetailsCurrentlyEnabled()
	{
		// not only checks for the min frame rate from the rules, but also whether
		// the low frame rate is actually desired. in that case, don't reduce.
		auto const current = FPSCounter::CurrentFrameRate();
		auto const wanted = static_cast<unsigned int>(
			60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

		return current >= wanted || current >= Detail::GetMinFrameRate();
	}

	static bool DetailsCurrentlyEnabled(int const minDetailLevel)
	{
		return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
			&& DetailsCurrentlyEnabled();
	}

	static inline bool IsFPSEligible()
	{
		auto const wanted = static_cast<unsigned int>(
			60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

		if (FPSCounter::CurrentFrameRate() >= wanted)
			return false;

		auto nMedDetails = Global()->DetailMinFrameRateMedium.Get();
		auto const nBuff = RulesGlobal->DetailBufferZoneWidth;
		static bool nSomeBool = false;

		if (nSomeBool)
		{
			if (FPSCounter::CurrentFrameRate() < nMedDetails + nBuff)
				return 1;
			nSomeBool = false;
		}
		else
		{
			if (FPSCounter::CurrentFrameRate() >= nMedDetails)
				return 0;

			nMedDetails += nBuff;

			nSomeBool = true;
		}

		return FPSCounter::CurrentFrameRate() < nMedDetails;
	}

	static inline bool DetailsCurrentlyEnabled_Changed(int const nCurDetailLevel)
	{
		if (DetailsCurrentlyEnabled() && nCurDetailLevel > 0)
			return false;

		if (IsFPSEligible && nCurDetailLevel > 1)
			return false;

		return true;
	}
};

DEFINE_HOOK(0x422FCC, AnimClass_DrawDetail, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	return FakeRulesExt::DetailsCurrentlyEnabled_Changed(pThis->Type->DetailLevel) ? 0x422FEC : 0x4238A3;
}

DEFINE_HOOK(0x42307D, AnimClass_DrawDetail_Translucency, 0x6)
{
	GET(AnimTypeClass*, pType, EAX);

	if (GameOptionsClass::Instance->DetailLevel < pType->TranslucencyDetailLevel)
		return 0x4230FE;

	if (!FakeRulesExt::IsFPSEligible())
		return 0x42308D;

	return pType->TranslucencyDetailLevel <= 1 ? 0x42308D :0x4230FE;

}

DEFINE_HOOK(0x4680E2, BulletClass_Detail , 0x6)
{
	if (!FakeRulesExt::Global()->DetailLowDisableBullet)
		return 0;

	if (FakeRulesExt::DetailsCurrentlyEnabled())
		return 0x468422;

	return !GameOptionsClass::Instance->DetailLevel ? 0x468422 : 0x0;
}

DEFINE_HOOK(0x53D591 ,  IonBlastClass_Detail , 0x6)
{
	if (GameOptionsClass::Instance->GameSpeed < 2)
		return 0x53D842;
	return (!FakeRulesExt::IsFPSEligible()) ? 0x53D597 : 0x53D842;
}

DEFINE_HOOK(0x550268 , LaserDrawClass_Detail, 0x6)
{
	if (FakeRulesExt::DetailsCurrentlyEnabled())
		return 0x5509CB;

	if (!GameOptionsClass::Instance->GameSpeed)
		return = 0x5509CB;

	return 0x0;
}

DEFINE_HOOK(0x62CFBB, ParticleClass_Detail_Translucency, 0x7)
{
	if (MGameOptionsClass::Instance->GameSpeed < 2)
		return 0x62CFEC;

	return (!FakeRulesExt::IsFPSEligible()) ? 0x62CFC4 : 0x62CFEC;
}
#endif
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

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH)) {
		if (auto pTechnp = generic_cast<TechnoClass*>(pTarget))
			pWHExt->Detonate(pThis->Owner, pHouse, pThis, *pCoords);
	}

	return 0x0;
}
#endif


#endif