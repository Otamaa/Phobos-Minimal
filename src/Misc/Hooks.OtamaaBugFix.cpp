#include "Hooks.OtamaaBugFix.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

#include <VeinholeMonsterClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>
#include <TunnelLocomotionClass.h>
#include <IsometricTileTypeClass.h>

static void __fastcall _DrawBehindAnim(TechnoClass* pThis, void* _, Point2D* pWhere, RectangleStruct* pBounds)
{
	if (!pThis->GetTechnoType()->Invisible)
		pThis->DrawBehind(pWhere, pBounds);
}

DEFINE_POINTER_CALL(0x6FA2D3, &_DrawBehindAnim)

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
DEFINE_LJMP(0x546C8B, 0x546CBF);

/*
static AnimClass* __fastcall _InfantryClass_DoingAI_AnimCtor(
AnimClassCopy* pThis,
void* _,
AnimTypeClass* pType,
CoordStruct* pCoord,
int LoopDelay,
int LoopCount,
DWORD flags,
int ForceZAdjust,
bool reverse
)
{
	GET_REGISTER_STATIC_TYPE(InfantryClass*, pThisInf, esi);
	auto const pAnim = pThis->_AnimClass_CTOR(pType, pCoord, LoopDelay, LoopCount, flags, ForceZAdjust, reverse);

	if (pAnim && pThisInf)
	{
		AnimExt::SetAnimOwnerHouseKind(pAnim, pThisInf->GetOwningHouse(),
			pThisInf->Target ? pThisInf->Target->GetOwningHouse() : nullptr, false);
	}

	return pAnim;
}

DEFINE_POINTER_CALL(0x520CA4, _InfantryClass_DoingAI_AnimCtor);
*/

static DamageAreaResult __fastcall _RocketLocomotionClass_DamageArea
(
	CoordStruct* pCoord,
	int nDamage,
	TechnoClass* pSource,
	WarheadTypeClass* pWarhead,
	bool AffectTiberium, //false
	HouseClass* pSourceHouse //nullptr
)
{
	HouseClass* pHouseOwner = pSource ? pSource->GetOwningHouse() : nullptr;
	auto nCoord = *pCoord;
	return Map.DamageArea
	(nCoord, nDamage, pSource, pWarhead, pWarhead->Tiberium, pHouseOwner);
}

DEFINE_POINTER_CALL(0x6632C7, _RocketLocomotionClass_DamageArea);


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

DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
{
	GET(RulesClass*, pRules, ECX);

	auto const nRand = pRules->VeinholeGrowthRate > 0 ?
		ScenarioGlobal->Random(0, pRules->VeinholeGrowthRate / 2) : 0;

	R->EAX(pRules->VeinholeGrowthRate + nRand);
	return 0x74D2AA;
}

static	void __fastcall DrawShape_VeinHole
(Surface* pSurface, ConvertClass* pPal, SHPStruct* SHP, int FrameIndex,
const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags,
int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor,
SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset
)
{
	bool bUseTheaterPal = true;
	CC_Draw_Shape(pSurface, bUseTheaterPal ? FileSystem::THEATER_PAL() : pPal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_POINTER_CALL(0x74D5BC, DrawShape_VeinHole);

static	void __fastcall Replace_VeinholeShapeLoad(TheaterType nTheater)
{
	//TheaterTypeClass::GetCharExtension(nTheater)
	char flag[0x100];
	_snprintf_s(flag, _TRUNCATE, "VEINHOLE.%s", Theater::GetTheater(nTheater).Extension);
	if (auto const pImage = FileSystem::LoadSHPFile(flag))
		VeinholeMonsterClass::VeinSHPData = pImage;
}

DEFINE_POINTER_CALL(0x685136, Replace_VeinholeShapeLoad);

static	void __fastcall DisplayClass_ReadINI_add(TheaterType nTheater)
{
	SmudgeTypeClass::TheaterInit(nTheater);
	Replace_VeinholeShapeLoad(nTheater);
}

DEFINE_POINTER_CALL(0x4AD0A3, DisplayClass_ReadINI_add)

static	int __fastcall SelectParticle(char* pName)
{
	return RulesExt::Global()->VeinholeParticle.Get(ParticleTypeClass::FindIndex(pName));
}

DEFINE_POINTER_CALL(0x74D0DF, SelectParticle);

DEFINE_HOOK(0x75F415, WaveClass_DamageCell_FixNoHouseOwner, 0x6)
{
	GET(TechnoClass*, pTechnoOwner, EAX);
	GET(ObjectClass*, pVictim, ESI);
	GET_STACK(int, nDamage, STACK_OFFS(0x18, 0x4));
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0x18, 0x8));

	//Debug::Log("Wave Receive Damage for Victim [%x] ! \n", pVictim);
	if(auto pUnit = specific_cast<UnitClass*>(pVictim))
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

DEFINE_HOOK(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

DEFINE_HOOK(0x62A933, ParasiteClass_CanInfect_ParasitePointerGone_Check, 0x5)
{
	GET(ParasiteClass*, pThis, EDI);

	if (!pThis)
		Debug::Log("Found Invalid ParasiteClass Pointer ! , Skipping ! \n");

	return pThis ? 0x0 : 0x62A976;
}

DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackAllies, 0x5)
{
	return R->ESI<TechnoClass*>()->GetTechnoType()->AttackFriendlies ? 0x6FA472 : 0x0;
}

DEFINE_HOOK(0x6F3481, TechnoClass_WhatWeaponShoudIUse_CheckWH, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EAX);
	GET(WarheadTypeClass*, pWarhead, ECX);

	if (!pWarhead)
		Debug::FatalErrorAndExit("Weapon [%s] , Has Missing Warhead at %s ! \n", pWeapon->get_ID(), __FUNCTION__);

	return 0x0;
}

DEFINE_HOOK_AGAIN(0x46684A, BulletClass_AI_TrailerInheritOwner, 0x5)
DEFINE_HOOK(0x466886, BulletClass_AI_TrailerInheritOwner, 0x5)
{
	GET(BulletClass*, pThis, EBP);
	//GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x1AC, 0x184));

	//Eax is discarded anyway
	if (auto pAnim = GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, 0x600, 0, false))
	{
		if (auto const pExt = BulletExt::GetExtData(pThis))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner ? pThis->Owner->GetOwningHouse() : pExt->Owner
							, pThis->Target ? pThis->GetOwningHouse() : nullptr, false);
		}

		if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnim)) {
			pAnimExt->Invoker = pThis->Owner;
		}
	}

	return 0x4668BD;
}

static AnimTypeClass* GetSinkAnim(TechnoClass* pThis) {
	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
		return (pTypeExt->SinkAnim.Get(RulesGlobal->Wake));
	}

	return RulesGlobal->Wake;
}

DEFINE_HOOK(0x414EAA, AircraftClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClassCopy*, pAnim, EAX);
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0x24));

	pAnim->_AnimClass_CTOR(GetSinkAnim(pThis), nCoord, 0, 1, 0x600, 0, false);

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnim)) {
		pAnimExt->Invoker = pThis;
	}

	return 0x414ED0;
}

DEFINE_HOOK(0x736595, TechnoClass_IsSinking_SinkAnim, 0x6)
{
	GET(AnimClassCopy*, pAnim, EAX);
	GET(UnitClass*, pThis, ESI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0x18));

	pAnim->_AnimClass_CTOR(GetSinkAnim(pThis), nCoord, 0, 1, 0x600, 0, false);

	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	if (auto const pAnimExt = AnimExtAlt::GetExtData(pAnim)) {
		pAnimExt->Invoker = pThis;
	}

	return 0x7365BB;
}

#pragma region tomsons26_IsotileDebug
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
DEFINE_POINTER_CALL(0x544C3F, &Isotile_LoadFile_Wrapper);
//544C97
DEFINE_POINTER_CALL(0x544C97, &Isotile_LoadFile_Wrapper);
//544CC9
DEFINE_POINTER_CALL(0x544CC9, &Isotile_LoadFile_Wrapper);
//546FCC
DEFINE_POINTER_CALL(0x546FCC, &Isotile_LoadFile_Wrapper);
//549AF7
DEFINE_POINTER_CALL(0x549AF7, &Isotile_LoadFile_Wrapper);
//549E67
DEFINE_POINTER_CALL(0x549E67, &Isotile_LoadFile_Wrapper);
#pragma endregion

DEFINE_HOOK(0x7BAE60 , Surface_GetPixel_CheckParameters, 0x5)
{
	GET(Surface*, pSurface, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);

	R->EAX(NULL);

	if ((pPoint->X < 0 || pPoint->X >= pSurface->Width) ||
		(pPoint->Y < 0 || pPoint->Y >= pSurface->Height)) {
		return 0x7BAE9A;
	}

	if (auto pResult = pSurface->Lock(pPoint->X, pPoint->Y))
	{
		if (pSurface->Get_Bytes_Per_Pixel() == 1) {
			R->EAX<int>(*reinterpret_cast<int*>(pResult));
			pSurface->Unlock();
			return 0x7BAE9A;
		}

		R->EAX<int>(*reinterpret_cast<int*>(pResult));
		pSurface->Unlock();
	}

	return 0x7BAE9A;
}