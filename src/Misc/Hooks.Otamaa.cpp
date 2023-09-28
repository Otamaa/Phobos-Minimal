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
#include <AircraftTrackerClass.h>

#include <Memory.h>

#include <Locomotor/Cast.h>
#pragma endregion

DEFINE_JUMP(LJMP, 0x546C8B, 0x546CBF);

DEFINE_HOOK(0x687AF4, CCINIClass_InitializeStuffOnMap_AdjustAircrafts, 0x5)
{
	AircraftClass::Array->for_each([](AircraftClass* const pThis)
	{
		 if (pThis && pThis->Type->AirportBound)
		 {
			if (auto pCell = pThis->GetCell())
			{
				if (auto pBuilding = pCell->GetBuilding())
				{
				 if (pBuilding->Type->Helipad)
				 {
						pBuilding->SendCommand(RadioCommand::RequestLink, pThis);
						 pBuilding->SendCommand(RadioCommand::RequestTether, pThis);
						pThis->SetLocation(pBuilding->GetDockCoords(pThis));
						 pThis->DockedTo = pBuilding;
						 pThis->SecondaryFacing.Set_Current(pBuilding->PrimaryFacing.Current());

						if (pThis->GetHeight() > 0)
							 AircraftTrackerClass::Instance->Add(pThis);
					}
					}
				}
			}
	});

	return 0x0;
}

DEFINE_HOOK(0x6FA2CF, TechnoClass_AI_DrawBehindAnim, 0x9) //was 4
{
	GET(TechnoClass*, pThis, ESI);
	GET(Point2D*, pPoint, ECX);
	GET(RectangleStruct*, pBound, EAX);

	if (const auto pBld = specific_cast<BuildingClass*>(pThis))
		if (BuildingExt::ExtMap.Find(pBld)->LimboID != -1)
			return 0x6FA30C;

	if(pThis->InOpenToppedTransport)
		return 0x6FA30C;

	const auto pType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if(pType->IsDummy)
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
	if (auto pManager = RulesExt::Global()->VeinholePal)
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

DEFINE_HOOK_AGAIN(0x46684A, BulletClass_AI_TrailerInheritOwner, 0x5)
DEFINE_HOOK(0x466886, BulletClass_AI_TrailerInheritOwner, 0x5)
{
	GET(BulletClass* const, pThis, EBP);
	//GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x184));

	//Eax is discarded anyway
	auto const pExt = BulletExt::ExtMap.Find(pThis);
	AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
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

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? nFLH : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		if (auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord))
		{
			AnimExt::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
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

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

	return pThis->WhatAmI() == BuildingClass::AbsID ? AdjustCoordsForBuilding : Goto2NdCheck;
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

DEFINE_HOOK(0x4FE546, HouseClass_BuildingClass_AI_WallTowers, 0x6)
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

	return Nvec.Any_Of([&](BuildingTypeClass* const pWallTower) {
		return pWallTower->ArrayIndex == nNodeBuilding; })
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

	pAnim->AnimClass::AnimClass(TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x414ED0;
}

DEFINE_HOOK(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(UnitClass* const, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	pAnim->AnimClass::AnimClass(TechnoTypeExt::GetSinkAnim(pThis), nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);

	return 0x7365BB;
}

DEFINE_HOOK(0x738703, UnitClass_Explode_ExplodeAnim, 0x5)
{
	GET(AnimTypeClass*, pExplType, EDI);
	GET(UnitClass*, pThis, ESI);

	if (pExplType) {

		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExplType, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
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
	if (auto const pType = pThis->Type->Explosion.GetItem(ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1))) {
		const auto nDelay = ScenarioClass::Instance->Random.RandomFromMax(3);
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, nLoc, nDelay, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
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
		AnimExt::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoord, nDelay + 3, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false),
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
	pAnim->AnimClass::AnimClass(RulesClass::Instance->MetallicDebris[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
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
	pAnim->AnimClass::AnimClass(pType->DebrisAnims[nIdx], nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim,
	 	args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse,
	 	pThis->GetOwningHouse(),
		false
	);

	return 0x7024AF;
}

//ObjectClass TakeDamage , 5F559C
//UnitClass TakeDamage , 737F0E

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
	bool DefaultCanPassive = pType->CanPassiveAquire;
	if (pTypeExt->PassiveAcquire_AI.isset()
		&& pThis->Owner
		&& !pThis->Owner->IsControlledByHuman_()
		&& !pThis->Owner->IsNeutral())
	{
		DefaultCanPassive =  pTypeExt->PassiveAcquire_AI.Get();
	}

	if(pType->Naval && pTypeExt->CanPassiveAquire_Naval.isset()) {
		DefaultCanPassive = pTypeExt->CanPassiveAquire_Naval.Get();
	}

	return 	DefaultCanPassive ?	ContinueCheck : CantPassiveAcquire;
	//return Continue;
}

DEFINE_HOOK(0x45743B, BuildingClass_Infiltrated_StoleMoney_AI, 0xA)
{
	GET(BuildingClass*, pThis, EBP);
	GET(RulesClass*, pRules, EDX);
	GET_STACK(int, nAvailMoney, 0x18);

	float mult = pRules->SpyMoneyStealPercent;
	auto const& nAIMult = RulesExt::Global()->AI_SpyMoneyStealPercent;

	if (pThis->Owner && !pThis->Owner->ControlledByPlayer() && nAIMult.isset()) {
		mult = nAIMult.Get();
	}

	R->EAX(int(nAvailMoney * mult));
	return 0x45744A;
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

	GET(TechnoClass* const , pThis , EDI);
	GET(TechnoClass* const, pTarget, ESI);
	GET(TechnoTypeClass* const, pTargetType, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (pTypeExt->AI_LegalTarget.isset())
	{
		if (pThis->Owner && pThis->Owner->IsControlledByHuman_())
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

DEFINE_HOOK(0x51CDB9, InfantryClass_RandomAnimate_CheckIdleRate, 0x6)
{
	return R->ESI<InfantryClass* const>()->Type->IdleRate == -1 ? 0x51D0A0 : 0x0;
}

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

DEFINE_HOOK(0x701AAD, TechnoClass_ReceiveDamage_WarpedOutBy_Add, 0xA)
{
	enum { NullifyDamage = 0x701AC6, ContinueCheck = 0x701ADB };

	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(bool, bIgnore, 0xD8);

	const bool IsCurrentlyDamageImmune = pThis->IsBeingWarpedOut()
		|| TechnoExt::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pThis));

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

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
		0x508CEE : 0x0;
}

DEFINE_HOOK(0x508EE5, HouseClass_UpdateRadar_LimboDeliver, 0x6)
{
	GET(BuildingClass*, pBld, EAX);

	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
		0x508EEF : 0x0;
}

//DEFINE_HOOK(0x508FCE, HouseClass_SpySat_LimboDeliver, 0x6)
//{
//	GET(BuildingClass*, pBld, ECX);
//
//	return (!pBld->DiscoveredByCurrentPlayer && BuildingExt::ExtMap.Find(pBld)->LimboID != -1) ?
//		0x508FE1 : 0x0;
//}

DEFINE_HOOK(0x70D219, TechnoClass_IsRadarVisible_Dummy, 0x6)
{
	enum { Continue = 0x0, DoNotDrawRadar = 0x70D407 };

	GET(TechnoClass* const, pThis, ESI);

	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis))->LimboID != -1)
		{
			return DoNotDrawRadar;
		}
	}

	return
	TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsDummy ?
		DoNotDrawRadar :
		 Continue;
}

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
	const void* ptr = YRMemory::Allocate(sizeof(TeamClass));
	Debug::Log("[%s - %x] Creating a new team named [%s - %x].\n", pHouse ? pHouse->get_ID() : NONE_STR2 ,pHouse, pThis->ID, ptr);
	R->EAX(ptr);
	return 0x6F0A5A;
}

DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);
//DEFINE_HOOK(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5) { return 0x44DE3C; }

DEFINE_HOOK(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
{
	GET(FactoryClass* const, pThis, ESI);
	GET(TechnoTypeClass* const, pType, EAX);
	//Debug::Log("[%x] Factory with Owner '%s' Abandoning production of '%s' \n", pThis, pThis->Owner ? pThis->Owner->get_ID() : NONE_STR2, pType->ID);
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

//DEFINE_SKIP_HOOK(0x4495FF, BuildingClass_ClearFactoryBib_Log1, 0xA , 44961A);
//DEFINE_SKIP_HOOK(0x449657, BuildingClass_ClearFactoryBib_Log2, 0xA , 449672);
DEFINE_JUMP(LJMP, 0x4495FF, 0x44961A);
DEFINE_JUMP(LJMP, 0x449657, 0x449672);

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

	if (pTechno->WhatAmI() == BuildingClass::AbsID)
	{
		if (BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pTechno))->LimboID != -1)
		{
			return DoNotDraw;
		}
	}

	return Draw;
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

DEFINE_HOOK(0x73AED4, UnitClass_PCP_DamageSelf_C4WarheadAnimCheck, 0x8)
{
	GET(UnitClass* const, pThis, EBP);
	GET(AnimClass*, pAllocatedMem, ESI);
	GET(LandType const, nLand, EBX);

	CoordStruct nLoc = pThis->Location;
	if (auto const pC4AnimType = MapClass::SelectDamageAnimation(pThis->Health, RulesClass::Instance->C4Warhead, nLand, nLoc))
	{
		pAllocatedMem->AnimClass::AnimClass(pC4AnimType, nLoc, 0, 1, 0x2600, -15, false);
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
	if (!pTechno)
		return;

	const auto pType = pTechno->GetTechnoType();
	const auto nMax = pType->GetPipMax();

	if (!nMax)
		return;

	auto const nStorage = pType->Storage;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	Point2D nOffs {};
	const auto pBuilding = pTechno->WhatAmI() == BuildingClass::AbsID ? static_cast<BuildingClass*>(pTechno) : nullptr;
	const auto pUnit = pTechno->WhatAmI() == UnitClass::AbsID ? static_cast<UnitClass*>(pTechno) : nullptr;

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

	constexpr int maximumStorage = sizeof(pTechno->Tiberium.Tiberiums) / sizeof(float);
	std::vector<std::pair<int , int>> Amounts(maximumStorage);

	const bool isWeeder = pBuilding ? pBuilding->Type->Weeder : pUnit ? pUnit->Type->Weeder : false;

	for (size_t i = 0; i < Amounts.size(); i++) {

		int FrameIdx = 0;
		int amount = 0;

		if(pBuilding && pBuilding->Type->Weeder) {
			amount = int(pTechno->Owner->GetWeedStoragePercentage() * nMax + 0.5);
		} else {
		    amount = int(pTechno->Tiberium.Tiberiums[i] / nStorage * nMax + 0.5);
		}

		if (!isWeeder)
		{
			//default pip : 5 = blue , 2 gold ?
			switch (i)
			{
			case 0:
				FrameIdx = pTypeExt->Riparius_FrameIDx.Get(2);
				break;
			case 1:
				FrameIdx = pTypeExt->Cruentus_FrameIDx.Get(5);
				break;
			case 2:
				FrameIdx = pTypeExt->Vinifera_FrameIDx.Get(2);
				break;
			case 3:
				FrameIdx = pTypeExt->Aboreus_FrameIDx.Get(2);
				break;
			default:
				FrameIdx = 2;
				break;
			}
		}
		else
		{
			FrameIdx = pTypeExt->Weeder_PipIndex.Get(1); //idk ?
		}

		Amounts[i] = { amount , FrameIdx };
	}

	static constexpr std::array<int, maximumStorage> defOrder { {0, 2, 3, 1} };
	const auto displayOrders = RulesExt::Global()->Pips_Tiberiums_DisplayOrder.GetElements(make_iterator(&defOrder[0], maximumStorage));

	const auto GetFrames = [&](
		const Iterator<int> orders ,
		const ValueableVector<int>& frames)
	{
		int frame = isWeeder ? pTypeExt->Weeder_PipEmptyIndex.Get(0) : frames.empty() ? 0 : frames[0];

		for (size_t i = 0; i < maximumStorage; i++)
		{
			size_t index = i;
			if (i < orders.size())
				index = orders[i];

			if (Amounts[index].first > 0) {
				--Amounts[index].first;
				frame = isWeeder || frames.empty() || index >= (frames.size() - 1) ?
					(Amounts[index].second) : (frames[index + 1]);

				break;
			}
		}

		return frame > pShape->Frames ? pShape->Frames : frame;
	};

	for (int i = nMax; i; --i)
	{
		Point2D nPointHere { nOffs.X + nPoints->X  , nOffs.Y + nPoints->Y };
		CC_Draw_Shape(
			DSurface::Temp(),
			nPal,
			pShape,
			GetFrames(displayOrders, RulesExt::Global()->Pips_Tiberiums_Frames),
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
	{ "None" },
	{ "Normal" },
	{ "Strong" },
	{ "Threat" },
	{ "Health" },
	{ "Damage" },
	{ "Value" },
	{ "Locked" },
	{ "Non_Infantry" }
	} };

DEFINE_HOOK(0x477590, CCINIClass_ReadVHPScan_Replace, 0x6)
{
	GET(CCINIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, default_val, 0xC);

	INI_EX exINI(pThis);

	int vHp = default_val;

	if (exINI.ReadString(pSection, pKey)) {
		for (int i = 0; i < (int)NewVHPScanToString.size(); ++i) {
			if (IS_SAME_STR_(exINI.value(), NewVHPScanToString[i])) {
				R->EAX(i);
				return 0x477613;
			}
		}

		Debug::INIParseFailed(pSection, pKey, exINI.value(), "Expected valid VHPScan value");
	}

	R->EAX(vHp);
	return 0x477613;
}

DEFINE_HOOK(0x6F8721, TechnoClass_EvalObject_VHPScan, 0x7)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(ObjectClass* const, pTarget, ESI);
	GET(int*, pRiskValue, EBP);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto pTechnoTarget = generic_cast<TechnoClass* const>(pTarget);

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
		if (!pTechnoTarget || pTechnoTarget->WhatAmI() == InfantryClass::AbsID)
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

CoordStruct* __fastcall UnitClass_GetFLH(UnitClass* pThis, DWORD, CoordStruct* buffer, int wepon, CoordStruct base)
{
	if (pThis->InOpenToppedTransport && pThis->Transporter) {
		const int idx = pThis->Transporter->Passengers.IndexOf(pThis);
		if (idx != 0){
			return pThis->Transporter->GetFLH(buffer  , -idx, base);
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

	if (!WarheadTypeExt::ExtMap.Find(pWH)->applyCulling(pThis->Owner, pThis->Victim))
		return ApplyDamage;

	return pThis->Owner && pThis->Owner->Owner && pThis->Owner->Owner->IsAlliedWith_(pThis->Victim)
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

DEFINE_HOOK(0x702721, TechnoClass_ReceiveDamage_DamagedSound, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pThisType, EAX);

	if (pThisType->DamageSound != -1) {
		VoxClass::PlayAtPos(pThisType->DamageSound, &pThis->Location);
	}

	return 0x7027AE;
}

DEFINE_HOOK(0x4426DB, BuildingClass_ReceiveDamage_DisableDamagedSoundFallback, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);

	if (BuildingTypeExt::ExtMap.Find(pThis->Type)->DisableDamageSound.Get()) {
		return R->Origin() + 0x6;
	}

	return 0x0;
}

// this just an duplicate
//DEFINE_SKIP_HOOK(0x702765, TechnoClass_TakeDamage_DuplicateDamageSound , 0xA , 7027AE);
DEFINE_JUMP(LJMP, 0x702765, 0x7027AE);

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

		if(pTechnoTypeExt->VoiceCreate >= 0 ){

			if(!pTechnoTypeExt->VoiceCreate_Instant)
				pTechno->QueueVoice(pTechnoTypeExt->VoiceCreate);
			else
			{
				if(pThis->IsControlledByHuman_() && !pThis->IsCurrentPlayerObserver())
					VocClass::PlayAt(pTechnoTypeExt->VoiceCreate, pTechno->Location);
			}
		}

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

	if (pTech && pTech->WhatAmI() == BuildingClass::AbsID
	 && pTech->Owner
	 && pTech->Owner->ControlledByPlayer()) {
		VoxClass::PlayIndex(TechnoTypeExt::ExtMap.Find(pTech->GetTechnoType())->Eva_Complete.Get());
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
	const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	TechnoClass* const pTech = AnimExt::GetTechnoInvoker(pThis, pAnimTypeExt->Damage_DealtByInvoker.Get());
	HouseClass* const pOwner = !pThis->Owner && pTech ? pTech->Owner : pThis->Owner;
	AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner, nullptr, pTech, false);

	return 0x424322;
}

DEFINE_HOOK(0x51DF82, InfantryClass_FireAt_StartReloading, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	const auto pType = pThis->Type;

	if(pThis->Transporter) {
		if (TechnoTypeExt::ExtMap.Find(pType)->ReloadInTransport
			&& pType->Ammo > 0
			&& pThis->Ammo < pType->Ammo
		)
			pThis->StartReloading();
	}

	return 0;
}

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

//DEFINE_SKIP_HOOK(0x69A797, Game_DisableNoDigestLog, 0x6, 69A937);
DEFINE_JUMP(LJMP, 0x69A797, 0x69A937);

DEFINE_HOOK(0x6F9F42, TechnoClass_AI_Berzerk_SetMissionAfterDone, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::SetMissionAfterBerzerk(pThis);
	return 0x6F9F6E;
}

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

#include <Ext/TerrainType/Body.h>

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
DEFINE_OVERRIDE_HOOK(0x6FCA30, TechnoClass_GetFireError_DecloakToFire, 6)
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

// make unit able to deploy fire without ejecting it passengers
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

DEFINE_HOOK(0x711F0F, TechnoTypeClass_GetCost_AICostMult, 0x8)
{
	GET(HouseClass* const, pHouse, EDI);
	GET(TechnoTypeClass* const, pType, ESI);

	double result = int(pType->GetCost() * pHouse->GetHouseCostMult(pType) * pHouse->GetHouseTypeCostMult(pType));

	if(!pHouse->IsControlledByHuman_())
		result *= RulesExt::Global()->AI_CostMult;

	R->EAX(result);
	return 0x711F46;
}

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

// Various call of TechnoClass::SetOwningHouse not respecting overloaded 2nd args fix !

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

#include <Misc/Ares/Hooks/Header.h>

BuildingClass* IsAnySpysatActive(HouseClass* pThis)
{
	const bool IsCurrentPlayer = pThis->ControlledByPlayer_();//inline version

	//===============reset all
	pThis->CostDefensesMult = 1.0;
	pThis->CostUnitsMult = 1.0;
	pThis->CostInfantryMult = 1.0;
	pThis->CostBuildingsMult = 1.0;
	pThis->CostAircraftMult = 1.0;
	BuildingClass* Spysat = nullptr;
	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->Building_BuildSpeedBonusCounter.clear();
	pHouseExt->Building_OrePurifiersCounter.clear();
	//==========================
	const bool LowpOwerHouse = pThis->HasLowPower();

	for (auto const& pBld : pThis->Buildings) {
		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap) {
			const bool IsLimboDelivered = BuildingExt::ExtMap.Find(pBld)->LimboID != -1;

			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
				continue;

			if (pBld->TemporalTargetingMe
				|| BuildingExt::ExtMap.Find(pBld)->AboutToChronoshift
				|| pBld->IsBeingWarpedOut())
				continue;

			const bool PowerDown = !pBld->IsPowerOnline(); // check power
			const auto pTypes = pBld->GetTypes(); // building types include upgrades
			const bool Jammered = !RegisteredJammers(pBld).empty();  // is this building jammed

			for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin) {

				const auto pExt = BuildingTypeExt::ExtMap.Find(*begin);
				const bool IsFactoryPowered = pExt->FactoryPlant_RequirePower ? !PowerDown && !((*begin)->PowerDrain && LowpOwerHouse) : true;

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
				const bool IsSpySatPowered = pExt->SpySat_RequirePower ? !PowerDown && !((*begin)->PowerDrain && LowpOwerHouse) : true;
				if (!Spysat && (*begin)->SpySat && !Jammered && IsSpySatPowered) {
					const bool IsDiscovered = pBld->DiscoveredByCurrentPlayer && SessionClass::Instance->GameMode == GameMode::Campaign;
					if (IsLimboDelivered || !IsCurrentPlayer || SessionClass::Instance->GameMode != GameMode::Campaign || IsDiscovered) {
						Spysat = pBld;
					}
				}

				// add eligible building
				if (pExt->SpeedBonus.Enabled)
					++pHouseExt->Building_BuildSpeedBonusCounter[(*begin)];

				const bool IsPurifierRequirePower = pExt->PurifierBonus_RequirePower ? !PowerDown && !((*begin)->PowerDrain && LowpOwerHouse) : true;
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
	enum {
		ActivateSpySat = 0x509054,
		DeactivateSpySat = 0x509002
	};

	GET(HouseClass*, pThis, ESI);
	return IsAnySpysatActive(pThis) ? ActivateSpySat : DeactivateSpySat;
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
		if (!pThis->Owner->IsAlliedWith_(pInfantry) && pThis->GetOccupantCount() > 0)
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
	enum { SkipGameDrawing = 0x709C27 };

	GET(TechnoClass*, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pTypeExt->ShowSpawnsPips.Get(((int)pType->PipScale == 6))) {
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

	if (isBuilding)
	{
		const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find((BuildingTypeClass*)pType);

		if (pBuildingTypeExt->PipShapes01Remap)
			pPal = pThis->GetRemapColour();
		else if (const auto pConvertData = pBuildingTypeExt->PipShapes01Palette)
			pPal = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();

	}

	int currentSpawnsCount = pThis->SpawnManager->CountDockedSpawns();
	Point2D position { offset->X + pTypeExt->SpawnsPipOffset->X, offset->Y + pTypeExt->SpawnsPipOffset->Y };
	const Point2D size = pTypeExt->SpawnsPipSize.Get(
		isBuilding ?
		RulesExt::Global()->Pips_Generic_Buildings_Size :
		RulesExt::Global()->Pips_Generic_Size
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

	if (!pThis)
	{
		*pResult = CoordStruct::Empty;
		R->EAX(pResult);
		return retResult;
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

	const auto pSeq = &pThis->Type->Sequence->Data[doType];

	for (int i = 0; i < pSeq->SoundCount; ++i) {
		for (auto at = pSeq->SoundData; at != (&pSeq->SoundData[2]); ++at) {
			if (pThis->Animation.HasChanged && at->Index != -1) {
				const int count = pSeq->CountFrames < 1 ? 1 : pSeq->CountFrames;
				if (pThis->Animation.Value % count == at->StartFrame) {
					VoxClass::PlayAtPos(at->Index, &pThis->Location);
				}
			}
		}
	}

	return 0x520EF4;
}

DEFINE_HOOK(0x5194EF, InfantryClass_DrawIt_InAir_NoShadow, 5)
{
	GET(InfantryClass*, pThis, EBP);
	//TODO : add option for this
	return pThis->Type->NoShadow ? 0x51958A : 0x0;
}

DEFINE_HOOK(0x746AFF, UnitClass_Disguise_Update_MoveToClear, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	return pThis->Disguise && pThis->Disguise->WhatAmI() == UnitClass::AbsID ? 0x746A9C : 0;
}

DEFINE_HOOK(0x4D423A, FootClass_MissionMove_SubterraneanResourceGatherer, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pType = pThis->GetTechnoType();
	if (pThis->WhatAmI() == UnitClass::AbsID && pType->IsSubterranean && pType->ResourceGatherer)
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

// do some pre-validation evenbefore function going to be executed
// save some cpu cycle
DEFINE_HOOK(0x486920, CellClass_TriggerVein_Precheck, 0x6)
{
	return RulesClass::Instance->VeinAttack ? 0x0 : 0x486A6B;
}

DEFINE_HOOK(0x4869AB, CellClass_TriggerVein_Weight, 0x6)
{
	GET(TechnoTypeClass*, pTechno, ESI);

	if (pTechno->WhatAmI() == BuildingTypeClass::AbsID || !RulesExt::Global()->VeinsDamagingWeightTreshold.isset())
		return 0x0;

	if (pTechno->Weight <  RulesExt::Global()->VeinsDamagingWeightTreshold) {
		return 0x486A55; //skip damaging
	}

	return 0x0;
}

#ifdef debug_veinstest
DEFINE_JUMP(LJMP, 0x4869AB, 0x4869CA);
#endif

//// 7384C3 ,7385BB UnitClass take damage func
DEFINE_HOOK(0x73D4F1, UnitClass_Harvest_VeinsStorageAmount, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	pThis->Tiberium.AddAmount(RulesExt::Global()->Veins_PerCellAmount, 0);
	return 0x73D502;
}

//TODO , rewrite this to take custom amount
DEFINE_HOOK(0x73D4A4, UnitClass_Harvest_IncludeWeeder, 0x6)
{
	enum { retFalse = 0x73D5FE , retTrue = 0x73D4DA };
	GET(UnitTypeClass*, pType, EDX);
	GET(UnitClass*, pThis, ESI);
	GET(CellClass*, pCell, EBP);
	const bool canharvest = (pType->Harvester && pCell->LandType == LandType::Tiberium) || (pType->Weeder && pCell->LandType == LandType::Weeds);
	const bool canStoreHarvest = pThis->GetStoragePercentage() < 1.0;

	return canharvest && canStoreHarvest ? retTrue : retFalse;
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
	R->AL(pType->Harvester || (pType->Weeder && TechnoTypeExt::ExtMap.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x73E00B;
}

DEFINE_HOOK(0x741C32, UnitClass_AssignDestination_DestroyBuildingProductionAnim_IncludeWeeder, 0x6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->DL(pType->Harvester || (pType->Weeder && TechnoTypeExt::ExtMap.Find(pType)->Weeder_TriggerPreProductionBuildingAnim));
	return 0x741C38;
}

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
			if (!pMonster->InLimbo && pMonster->IsAlive && (pMonster->MonsterCell.DistanceFromI(pCell->MapCoords) <= 0))
				if (pMonster->ReceiveDamage(&nDamage,
					pCenter->DistanceFromI(CellClass::Cell2Coord(pMonster->MonsterCell)),
					pWarhead,
					pSource,
					false,
					false,
					pSource && !pHouse ? pSource->Owner : pHouse
				) == DamageState::NowDead)
					Debug::Log("Veinhole at Destroyed!\n");

		}

		return 0x4896B2;
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
		if (pThis->Type->Factory == pObj->GetTechnoType()->WhatAmI() && pThis->Factory && pThis->Factory->Object == pObj)
			return 0x444167; //continue check

		if (pObj->WhatAmI() == InfantryClass::AbsID)
			return 0x4445FB; // just eject
	}

	return 0x444167; //continue check
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

//DEFINE_SKIP_HOOK(0x4417A7, BuildingClass_Destroy_UnusedRandom, 0x0, 44180A);
DEFINE_JUMP(LJMP , 0x4417A7, 0x44180A)

DEFINE_HOOK(0x6FA4E5, TechnoClass_AI_RecoilUpdate, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	return !pThis->InLimbo ? 0x0 : 0x6FA4FB;
}

//building abandon sound 458291
//AbandonedSound
DEFINE_HOOK(0x458291, BuildingClass_GarrisonAI_AbandonedSound, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	const auto nVal = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
	if (nVal >= 0) {
		VocClass::PlayIndexAtPos(nVal, pThis->Location ,nullptr);
	}

	return 0x4582AE;
}

DEFINE_HOOK(0x6F6BD6, TechnoClass_Limbo_UpdateAfterHouseCounter, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);

	//only update the SW once the techno is really not present
	if (pThis->Owner && pThis->WhatAmI() != BuildingClass::AbsID && !pTypeExt->Linked_SW.empty() && pThis->Owner->CountOwnedAndPresent(pExt->Type) <= 0)
		pThis->Owner->UpdateSuperWeaponsOwned();

	return 0x0;
}

DEFINE_HOOK(0x6EA192, TeamClass_Regroup_LimboDelivered, 0x6)
{
	enum { advance = 0x6EA38C , ret =0x0};
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEC6D, TeamClass_FindTargetBuilding_LimboDelivered, 0x6)
{
	enum { advance = 0x6EEE45, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EE8D9, TeamClass_Scout_LimboDelivered, 0x9)
{
	enum { advance = 0x6EE928, ret = 0x0 };
	GET(BuildingClass**, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(*pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6EEEF2, TeamClass_6EEEA0_LimboDelivered, 0xA)
{
	enum { advance = 0x6EF0D7, ret = 0x0 };
	GET(BuildingClass*, pBuilding, ESI);
	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ?
		advance : ret;
}

DEFINE_HOOK(0x6E9696, TeamClass_ChangeHouse_nullptrresult, 0x9)
{
	GET(TeamClass*, pThis, ESI);
	GET(int, args, ECX);
	GET(FootClass*, pCurMember, EDI);

	const auto pHouse = HouseClass::FindByCountryIndex(args);
	if (!pHouse){
		const auto nonestr =  GameStrings::NoneStr();
		Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]\n",
			pThis->Owner ? pThis->Owner->get_ID() : nonestr , pThis->Owner ,
			pThis->get_ID() , pThis , args);
	}

	pCurMember->SetOwningHouse(pHouse);
	return 0x6E96A8;
}

DEFINE_HOOK(0x4686FA, BulletClass_Unlimbo_MissingTargetPointer, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_BASE(CoordStruct*, pUnlimboCoords, 0x8);

	if (!pThis->Target) {
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
	GET(TeamClass* , pCreated , ESI);
	GET(TeamTypeClass*, pType, EBX);

	const auto pHouse = pType->GetHouse();
	if (!pHouse) {
		Debug::Log("Creating Team[%s] groub without proper Ownership may cause crash , Please check !\n" , pType->ID);
	}

	R->EAX(pHouse);
	return 0x65DD55;
}

DEFINE_HOOK(0x415302, AircraftClass_MissionUnload_IsDropship, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Destination) {
		if (pThis->Type->IsDropship) {
			CellStruct nCell = CellStruct::Empty;
			if (pThis->Destination->WhatAmI() != CellClass::AbsID) {
				if (auto pTech = generic_cast<TechnoClass*>(pThis)) {
					nCell = CellClass::Coord2Cell(pTech->GetCoords());
					if (nCell.IsValid()) {
						if (auto pCell = MapClass::Instance->GetCellAt(nCell)) {
							for (auto pOccupy = pCell->FirstObject;
								pOccupy;
								pOccupy = pOccupy->NextObject) {
								if (pOccupy->WhatAmI() == AbstractType::Building) {
									pThis->SetDestination(pThis->GoodLandingZone_(), true);
								} else {
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

DEFINE_HOOK(0x456376 , BuildingClass_RemoveSpacingAroundArea, 0x6)
{
	GET(BuildingTypeClass*, pThisType, EAX);

	if(!pThisType->UndeploysInto && pThisType->ResourceGatherer)
		return 0x4563A1;

	return pThisType->Adjacent == 0 ? 0x4563A1 : 0x45638A;
}

DEFINE_HOOK(0x518607, InfantryClass_TakeDamage_FixOnDestroyedSource, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xD0, 0x10));
	R->AL(pThis->Crash(pSource));
	return 0x518611;
}

DEFINE_HOOK(0x450B48, BuildingClass_Anim_AI_UnitAbsorb, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->InfantryAbsorb || pThis->UnitAbsorb);
	return 0x450B4E;
}

/*
; Extend IsoMapPack5 decoding size limit
; (Large map support)

; When big sized maps with high details cross about 9750 + few lines in
; IsoMapPack5 section, game doesn't decode those and fills those (typically
; bottom-left corner of the map) with height level 0 clear tiles.
; This patch raises the buffer usage limit to about 3 times the original.
; From 640 (0x280), 400 (0x190) and value of 512000 (= 640 * 400 * 2)
; To 1024 (0x400), 768 (0x300) and 1572864 (= 1024 * 768 * 2).

; Credits: E1 Elite
*/

#include <Surface.h>
class CopyMemoryBuffer
{
public:
	CopyMemoryBuffer(void* pBuffer, int size) noexcept
	{
		JMP_THIS(0x43AD00);
	}

public:
	void* Buffer { nullptr };
	int Size { 0 };
	bool Allocated { false };
};

DEFINE_HOOK(0x4AD33E, DisplayClass_ReadINI_IsoMapPack5_Limit, 0x5)
{
	LEA_STACK(XSurface*, pSurface, 0x134 - 0x124);
	Game::CallBack();
	pSurface->XSurface::XSurface(0x00000300, 0x00000400);
	pSurface->BytesPerPixel = 2;
	CopyMemoryBuffer* buffer = reinterpret_cast<CopyMemoryBuffer*>(&reinterpret_cast<BSurface*>(pSurface)->BufferPtr);
	static_assert(sizeof(CopyMemoryBuffer) == sizeof(MemoryBuffer), "Must Be same !");
	buffer->CopyMemoryBuffer::CopyMemoryBuffer(nullptr, 0x00180000);
	return 0x4AD379;
}

DEFINE_HOOK(0x73B0C5 , UnitClass_Render_nullptrradio , 0x6)
{
	GET(TechnoClass* , pContact , EAX);
	return !pContact ? 0x73B124 : 0x0;
}

DEFINE_HOOK(0x6F91EC, TechnoClass_GreatestThreat_DeadTechnoInsideTracker, 0x6)
{
	GET(TechnoClass*, pTrackerTechno, EBP);

	if (!pTrackerTechno->IsAlive){
		Debug::Log("Found DeadTechno[%x - %s] on AircraftTracker!\n", pTrackerTechno, pTrackerTechno->get_ID());
		return 0x6F9377; // next
	}

	return 0x0;//contunye
}


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
			sweep_rate = _rate;
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

void __fastcall DrawRadialIndicator(ObjectClass* pObj ,DWORD , int somth)
{
	if(const auto pTechno = generic_cast<TechnoClass*>(pObj))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pType->HasRadialIndicator && pTypeExt->AlwayDrawRadialIndicator.Get(!pTechno->Deactivated))
		{
			const auto pOwner = pTechno->Owner;
			ColorStruct defColor = pType->RadialColor;
			bool Draw = true;

			if (pOwner) {

				if(!pOwner->ControlledByPlayer_()){
					 Draw = false;
				}

				defColor = pOwner->Color;
			}

			if (HouseClass::CurrentPlayer()->IsObserver() || Draw)
			{
				int nRadius = 0;

				if (pTypeExt->RadialIndicatorRadius.isset())
					nRadius = pTypeExt->RadialIndicatorRadius.Get();
				else if (pType->GapGenerator)
					nRadius = pTypeExt->GapRadiusInCells.Get();
				else
				{
					const auto pWeapons = pTechno->GetPrimaryWeapon();
					if (!WeaponStruct::IsValid(pWeapons) || pWeapons->WeaponType->Range <= 0)
						return;

					nRadius = pWeapons->WeaponType->Range.ToCell();
				}

				if (nRadius > 0)
				{
					auto nCoord = pTechno->GetCoords();
					const auto Color = pTypeExt->RadialIndicatorColor.Get(defColor);
					Tactical_Draw_Radial(false, true, nCoord, Color, (nRadius * 1.0f), false, true);
				}
			}
		}
	}
}

//7F5DA0 unit
//7EB188 inf
//7E3FEC bld
DEFINE_JUMP(VTABLE, 0x7F5DA0, GET_OFFSET(DrawRadialIndicator))
DEFINE_JUMP(VTABLE, 0x7EB188, GET_OFFSET(DrawRadialIndicator))

//Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
//Fixes recursive passenger kills not being accredited
//to proper techno but to their transports
DEFINE_HOOK(0x707CF2, TechnoClass_KillCargo_FixKiller, 0x8)
{
	GET(TechnoClass*, pKiller, EBP);
	GET(TechnoClass*, pCargo, ESI);

	pCargo->KillCargo(pKiller);
	return 0x707CFA;
}

#include <Commands/ShowTeamLeader.h>

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
			auto nIntersect = Drawing::Intersect(nTextDimension, *pBound);
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

//DEFINE_HOOK(0x407B60, AudioStreamer_Open_LogFileName, 0x5)
//{
//	GET(const char*, pName, EDX);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	Debug::Log(__FUNCTION__"[%s] Caller: %08x \n", pName , callerAddress);
//
//	return 0x0;
//}

//#include <ExtraHeaders/Placeholders/ExtraAudio.h>
#include <Audio.h>

DEFINE_HOOK(0x40A5B3, AudioDriverStart_AnnoyingBufferLogDisable_A, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	pAudioChannelTag->dwBufferBytes = R->EAX<int>();
	//Debug::Log("Sound frame size = %d bytes\n", pAudioChannelTag->dwBufferBytes);
	return 0x40A5C4;
}

DEFINE_HOOK(0x40A554, AudioDriverStart_AnnoyingBufferLogDisable_B, 0x6)
{
	GET(AudioDriverChannelTag*, pAudioChannelTag, EBX);
	LEA_STACK(DWORD*, ptr, STACK_OFFS(0x40, 0x28));
	pAudioChannelTag->soundframesize1 = R->EAX();
	//Debug::Log("Sound frame size = %d bytes\n", pAudioChannelTag->soundframesize1);
	R->EDX(R->EAX());
	R->EAX(ptr);
	return 0x40A56C;
}

DEFINE_HOOK(0x442991, BuildingClass_ReceiveDamage_ReturnFire_EMPulseCannon, 0x6) {
	GET(BuildingClass*, pThis, ESI);
	return pThis->Type->EMPulseCannon || pThis->Type->NukeSilo ? 0x442A95 : 0x0;
}

DEFINE_HOOK(0x442A08, BuildingClass_ReceiveDamage_ReturnFire, 0x5)
{
	enum { SetTarget = 0x442A34, RandomFacing = 0x442A41 };

	GET(TechnoClass*, pAttacker, EBP);
	GET(BuildingClass*, pThis, ESI);

	const bool def = BuildingTypeExt::ExtMap.Find(pThis->Type)->PlayerReturnFire.Get(
		pAttacker->WhatAmI() == AircraftClass::AbsID ||
		(pThis->Owner->ControlledByPlayer_() && !RulesClass::Instance->PlayerReturnFire)
	);

	return !def ? SetTarget : RandomFacing;
}


#include <Commands/ToggleRadialIndicatorDrawMode.h>

DEFINE_HOOK(0x6DBE35, TacticalClass_DrawLinesOrCircles, 0x9)
{
	if (!ToggleRadialIndicatorDrawModeClass::ShowForAll)
	{
		GET(int, intCurObjcount, EBX);

		if (!intCurObjcount)
			return 0x6DBE74;

		for (auto const pObj : ObjectClass::CurrentObjects())
		{
			if (pObj && pObj->GetType() && pObj->GetType()->HasRadialIndicator)
			{
				pObj->DrawRadialIndicator(1);
			}
		}
	}
	else
	{
		if (!TechnoClass::Array->Count)
			return 0x6DBE74;

		for (auto const pObj : *TechnoClass::Array)
		{
			if (pObj && pObj->GetType() && pObj->GetType()->HasRadialIndicator)
			{
				pObj->DrawRadialIndicator(1);
			}
		}
	}

	return 0x6DBE74;
}

//DEFINE_HOOK(0x40A463, AudioDriverStart_NonamePrefill_PeekBuffer, 0x5)
//{
//	GET(AudioDriverChannelTag*, pTag, ECX);
//
//	if (!AudioDriverChannelTag::noname_prefil(pTag, pTag->dwBufferBytes)) {
//		return 0x40A5CB //0x40A470
//			;
//	}
//
//	return 0x40A5CB ;
//}

//called always 0040276a
//DEFINE_HOOK(0x40A340, AudioDriverStart_Log, 0x9)
//{
//	GET(AudioChannelTag*, pTag, ECX);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	Debug::Log(__FUNCTION__" [%x] , Caller: %08x \n", pTag ,callerAddress);
//	return 0x0;
//}
//
//DEFINE_HOOK(0x40A470, AudioDriverStart_Log_Failed, 0x5)
//{
//	GET(AudioDriverChannelTag* , pTag , EBX);
//
//	Debug::Log(__FUNCTION__" [%x]\n", pTag);
//	return 0x0;
//}

//int InfantryClass_Mission_Harvest(InfantryClass* pThis)
//{
//	if (pThis->Type->Slaved)
//	{
//		if (pThis->Type->Storage)
//		{
//			auto pCell = pThis->GetCell();
//			if (!pCell->HasTiberium() || pThis->GetStoragePercentage() == 1.0)
//			{
//				pThis->PlayAnim(DoType::Ready);
//				pThis->QueueMission(Mission::Guard, false);
//				return 1;
//			}
//			else
//			{
//				if (pThis->SequenceAnim != DoType::Shovel)
//				{
//					pThis->PlayAnim(DoType::Shovel);
//				}
//
//				auto tiberium = pCell->GetContainedTiberiumIndex();
//				auto curamount = pThis->Type->Storage - pThis->Tiberium.GetTotalAmount();
//				const auto reduceamount = curamount <= 1.0 ? curamount : 1.0;
//
//				pCell->ReduceTiberium(reduceamount);
//				pThis->Tiberium.AddAmount(reduceamount, tiberium);
//				return pThis->Type->HarvestRate;
//			}
//		}
//	}
//
//	if (!pThis->Destination)
//		return 1;
//
//	auto pCell = pThis->GetCell();
//	if (pThis->Type->ResourceGatherer && pThis->GetStoragePercentage() < 1.0 && pCell->HasTiberium())
//	{
//		auto tiberium = pCell->GetContainedTiberiumIndex();
//	}
//}

//DEFINE_HOOK(0x54C531, JumpjetLocomotionClass_State3_DeployToLand_Convert, 0x6)
//{
//	GET(CellClass*, pCell, EAX);
//	GET(FootClass*, pLinked, EDI);
//
//	if (auto pUnit = specific_cast<UnitClass*>(pLinked)) {
//
//		if (!pUnit->Type->DeployToLand)
//			return 0x0;
//
//		if(MapClass::Instance->IsWithinUsableArea(pCell->MapCoords , true)){
//			if (auto pConvert = TechnoTypeExt::ExtMap.Find(pUnit->Type)->Convert_Deploy) {
//				if (!MapClass::Instance->CanMoveHere(pCell->MapCoords, 1, 1, pConvert->SpeedType, -1, pConvert->MovementZone, -1, false, false))
//					return 0x54C53B;
//				else
//					return 0x54C544;
//			}
//		}
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x6F917D, TechnoClass_GreatestThreat_Occupy, 0x8)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis->WhatAmI() == InfantryClass::AbsID && AresGarrisonedIn(pThis)) {
//		return 0x6F9193;
//	}
//
//	return 0x0;
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

// TODO :
//  - Ice stuffs using WW pointer heap logic

DEFINE_JUMP(LJMP, 0x50BF60, 0x50C04A)// Disable CalcCost mult

#include <ExtraHeaders/AStarClass.h>
#include <BitFont.h>

#include <format>

bool ColorInitEd = false;
ColorScheme* MainColor = nullptr;
ColorScheme* BackColor = nullptr;

void InitColorDraw()
{
	if (!ColorInitEd)
	{
		MainColor = ColorScheme::Find("Gold");
		BackColor = ColorScheme::Find("NeonGreen");
		ColorInitEd = true;
	}
}

bool Draw_Debug_Test()
{
	const TextPrintType style = TextPrintType::FullShadow | TextPrintType::Point6Grad;
	RectangleStruct rect = DSurface::ViewBounds();

	// top left of tactical display.
	Point2D screen = rect.Top_Left();

	const auto buffer = std::format(L"RulesClass ptr {}", (uintptr_t)RulesClass::Instance());
	DSurface::Temp()->DrawColorSchemeText(
		buffer.c_str(),
		rect,
		screen,
		MainColor,
		BackColor,
		style);

	return true;
}

void Debug_Draw_Facings()
{
	const auto& objArr = ObjectClass::CurrentObjects;

	if (objArr->Count != 1) {
		return;
	}

	const auto pTechno = generic_cast<TechnoClass*>(objArr->Items[0]);
	if (!pTechno)
		return;

	RectangleStruct rect = DSurface::ViewBounds();
	const auto pType = pTechno->GetTechnoType();

	CoordStruct lept {};
	pType->Dimension2(&lept);
	Point3D lept_center = Point3D(lept.X / 2, lept.Y / 2, lept.Z / 2);

	Point3D pix {};
	pType->PixelDimensions(&pix);
	Point3D pixel_center = Point3D(pix.X / 2, pix.Y / 2, pix.Z / 2);

	Coordinate coord = pTechno->GetCoords();

	Point2D screen {};
	//func_60F150 tspp
	TacticalClass::Instance->CoordsToClient(coord, &screen);

	screen.X -= TacticalClass::Instance->TacticalPos.X;
	screen.Y -= TacticalClass::Instance->TacticalPos.Y;

	screen.X += rect.X;
	screen.Y += rect.Y;

	DSurface::Temp->Fill_Rect(rect, RectangleStruct(screen.X, screen.Y, 2, 2), DSurface::RGB_To_Pixel(255, 0, 0));

	TextPrintType style = TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad;
	const auto font = BitFont::BitFontPtr(style);

	screen.Y -= font->GetHeight() / 2;

	const auto buffer1 = std::format(L"{}" , (int)pTechno->PrimaryFacing.Current().GetDir());
	const auto buffer2 = std::format(L"{}", (int)pTechno->PrimaryFacing.Current().Raw);

	DSurface::Temp()->DrawColorSchemeText(
		buffer1.c_str(),
		rect,
		screen,
		MainColor,
		BackColor,
		style);

	screen.Y += 10;
	DSurface::Temp()->DrawColorSchemeText(
		buffer2.c_str(),
		rect,
		screen,
		MainColor,
		BackColor,
		style);
}


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
	VeinholeMonsterClass::UpdateAllVeinholes();
	return 0;
}

DEFINE_HOOK(0x711F60, TechnoTypeClass_GetSoylent_Disable, 0x8)
{
	GET(TechnoTypeClass*, pThis, ECX);

	if (TechnoTypeExt::ExtMap.Find(pThis)->Soylent_Zero) {
		R->EAX(0);
		return 0x712036;
	}

	return 0x0;
}

DEFINE_HOOK(0x4FAD64, HouseClass_SpecialWeapon_Update, 0x7)
{
	GET(HouseClass*, pThis, EDI);
	GET(BuildingClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x4FADD9 : 0x4FAD9E;
}

DEFINE_HOOK(0x50A23A, HouseClass_Target_Dominator, 0x6)
{
	GET(HouseClass*, pThis, EDI);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A292 : 0x50A278;
}

DEFINE_HOOK(0x50A04B, HouseClass_Target_GenericMutator, 0x7)
{
	GET(HouseClass*, pThis, EBX);
	GET(TechnoClass*, pThat, ESI);

	return pThis->IsAlliedWith(pThat->Owner) ? 0x50A096 : 0x50A087;
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

struct Phobos_DoControls {
	std::string Name;
	DoInfoStruct Data;
};

void ReadSequence(std::vector<Phobos_DoControls>& Desig, InfantryTypeClass* pInf ,CCINIClass* pINI)
{
	INI_EX IniEX(pINI);

	if (pINI->GetString(pInf->ImageFile, "Sequence", Phobos::readBuffer) > 0)
	{
		for (int i = 0; i < DoControls::MaxCount; ++i)
		{
			if (pINI->GetString(pInf->ImageFile, DoControls::DoType_toStr[i], Phobos::readBuffer))
			{
				auto& data = Desig.emplace_back();
				data.Name = DoControls::DoType_toStr[i];

				char bufferFacing[3];
				sscanf(Phobos::readBuffer, "%d,%d,%d,%s",
					&data.Data.StartFrame,
					&data.Data.CountFrames,
					&data.Data.FacingMultiplier,
					bufferFacing
					);

				if(*bufferFacing && strlen(bufferFacing)) {
					for (size_t i = 0; i < EnumFunctions::FacingType_to_strings.size(); ++i) {
						if (!_strcmpi(EnumFunctions::FacingType_to_strings[i], bufferFacing)) {
							data.Data.Facing = DoTypeFacing(i);
						}
					}
				}

				char bufferSounds[0x100];
				if (pINI->GetString(pInf->ImageFile, (data.Name + "Sounds").c_str(), bufferSounds) > 0)
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

						data.Data.SoundCount = v8;

						auto v10 = VocClass::FindIndexById(v9);
						v7 = strtok(0, " ,\t");
						if (v10 != -1)
						{
							for (auto at = data.Data.SoundData;
								at != std::end(data.Data.SoundData);
								++at) {
								at->Index = v10;
							}
						}
					}
				}
			}
		}
	}
}

//DEFINE_HOOK(0x525703, INIClass_DTOR_DeleteComments, 0x9)
//{
//	GET(INIClass*, pThis, ESI);
//
//	if(pThis->LineComments) {
//		do {
//			const auto next = pThis->LineComments->Next;
//			if (pThis->LineComments->Value)
//				CRT::free(pThis->LineComments->Value);
//
//			YRMemory::Deallocate(pThis->LineComments);
//			pThis->LineComments = next;
//		}
//		while (pThis->LineComments);
//	}
//
//	YRMemory::Deallocate(pThis->SectionIndex.IndexTable);
//
//	return 0x52570C;
//	return 0x0;
//}