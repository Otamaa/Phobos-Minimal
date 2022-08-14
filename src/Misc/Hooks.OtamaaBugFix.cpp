#include "Hooks.OtamaaBugFix.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

#include <InfantryClass.h>
#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <TunnelLocomotionClass.h>
#include <IsometricTileTypeClass.h>

#include <TiberiumClass.h>
#include <JumpjetLocomotionClass.h>

#include <Memory.h>

static void __fastcall _DrawBehindAnim(TechnoClass* pThis, void* _, Point2D* pWhere, RectangleStruct* pBounds)
{
	if (!pThis->GetTechnoType()->Invisible)
		pThis->DrawBehind(pWhere, pBounds);
}

//DEFINE_JUMP(CALL,0x6FA2D3, GET_OFFSET(_DrawBehindAnim));

DEFINE_HOOK(0x6FA2C7 , TechnoClass_AI_DrawBehindAnim , 0x4)
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
	const auto nTypeIdx = nRawData >> 16 & 0xFFFF;
	const auto nScript = pThis->CurrentScript;

	if (nBuildingIdx < BuildingTypeClass::Array()->Count)
		return 0x0;

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

static	void __fastcall Replace_VeinholeShapeLoad(TheaterType nTheater)
{
	//TheaterTypeClass::GetCharExtension(nTheater)
	std::string flag { "VEINHOLE." };
	flag += Theater::GetTheater(nTheater).Extension;
	if (auto const pImage = FileSystem::LoadSHPFile(flag.c_str()))
		VeinholeMonsterClass::VeinSHPData = pImage;
}

DEFINE_JUMP(CALL,0x685136, GET_OFFSET(Replace_VeinholeShapeLoad));

static void __fastcall DisplayClass_ReadINI_add(TheaterType nTheater)
{
	SmudgeTypeClass::TheaterInit(nTheater);
	Replace_VeinholeShapeLoad(nTheater);
}

DEFINE_JUMP(CALL,0x4AD0A3, GET_OFFSET(DisplayClass_ReadINI_add));

static int __fastcall SelectParticle(char* pName) {
	return RulesExt::Global()->VeinholeParticle.Get(ParticleTypeClass::FindIndex(pName));
}

DEFINE_JUMP(CALL,0x74D0DF, GET_OFFSET(SelectParticle));

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
		auto const pExt = BulletExt::GetExtData(pThis);
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() :
											(pExt && pExt->Owner) ? pExt->Owner : nullptr
								, pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner, false);

	}

	return 0x4668BD;
}

#ifdef tomsons26_IsotileDebug
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
#endif endregion

#ifdef ENABLE_NEWHOOKS
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
	GET(TechnoClass*, pThis, ESI);
	return pThis->Owner->IsPlayerControl() ? 0x70383C : 0x703828;
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

DEFINE_HOOK(0x54C14B , JumpjetLocomotionClass_UpdateMoving, 0x7)
{
	GET(FootClass*, pFoot, EDI);

	if (pFoot->GetTechnoType()->Sensors)
		pFoot->UpdatePosition(2);

	return 0;
}

DEFINE_HOOK(0x6FC22A, TechnoClass_GetFireError_AttackICUnit, 0x6)
{
	enum {
		ContinueCheck = 0x6FC23A ,
		BypassCheck = 0x6FC24D
	};

	GET(TechnoClass*, pThis, ESI);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	return pTypeExt->AllowFire_IroncurtainedTarget.Get() ? BypassCheck : ContinueCheck;
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

DEFINE_HOOK(0x722FFA, TiberiumClass_Grow_CheckMapCoords, 0x6)
{
	enum { increment = 0x72312F ,
		   SetCell = 0x723005
	};

	GET(const MapSurfaceData*, pSurfaceData, EBX);
	R->EBX(pSurfaceData);
	const auto nCell = pSurfaceData->MapCoord;

	if (!Map.IsValidCell(nCell)) {
		Debug::Log("Tiberium Growth With Invalid Cell ,Skipping !\n");
		return increment;
	}

	R->EAX(Map[nCell]);
	return SetCell;
}

DEFINE_HOOK(0x452831, BuildingClass_UpdateOverpowerState, 0x6)
{
	GET(const BuildingClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);

	return pThis->SelectWeapon(pTarget) == -1 ?
		0x45283C : 0x45289C;
}

#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x6FCA30 , TechnoClass_GetFireError_DecloakToFire, 0x6)
{
	enum
	{
		FireErrorCloaked = 0x6FCA4F,
		ContinueCheck = 0x6FCA5E
	};

	GET(const TechnoClass*, pThis, ESI);
	GET(const WeaponTypeClass*, pWeapon, EBX);

	if(const auto pTransport = pThis->Transporter)
		if(pTransport->CloakState == CloakState::Cloaked) return FireErrorCloaked;

	if (pThis->CloakState == CloakState::Uncloaked)
		return ContinueCheck;

	const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (pExt && pExt->Decloak_InstantFire.isset())
		if(!pExt->Decloak_InstantFire.Get() && pThis->WhatAmI() != AbstractType::Aircraft)
			return FireErrorCloaked;

	return (pThis->CloakState == CloakState::Cloaked) ? FireErrorCloaked : ContinueCheck;
}

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
#endif