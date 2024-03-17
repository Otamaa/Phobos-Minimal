#include "Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>

/*
		to do :
		- idle anim
		- Re-Draw function , to show damaged part
*/
DEFINE_HOOK(0x71B98B, TerrainClass_ReceiveDamage_Add, 0x7)
{
	enum { PostMortemReturn = 0x71B994, CheckNowDead = 0x71B9A7, SetReturn = 0x71BB79 };

	GET(DamageState, nState, EAX);
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x3C, -0x4));

	R->EAX(nState);
	R->Stack(0x10, nState);

	// ignite this terrain object

	if (!pThis->IsBurning && *args.Damage > 0 && args.WH->Sparky)
	{
		const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

		if (!pWarheadExt->Flammability.isset() || ScenarioClass::Instance->Random.PercentChance
		   (abs(pWarheadExt->Flammability.Get())))
			pThis->Ignite();
	}

	//return handle !
	if (nState == DamageState::PostMortem)
		return PostMortemReturn;
	if (nState == DamageState::NowDead)
		return CheckNowDead;

	return SetReturn;
}

#include <New/Entity/FlyingStrings.h>

//this one on Very end of it
//let everything play first
DEFINE_HOOK(0x71BB2C, TerrainClass_ReceiveDamage_NowDead_Add_light, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x3C, -0x4));

	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	auto const nCoords = pThis->GetCenterCoords();
	VocClass::PlayIndexAtPos(pTerrainExt->DestroySound.Get(-1), nCoords);
	const auto pAttackerHoue = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

	if (auto const pAnimType = pTerrainExt->DestroyAnim.Get(nullptr)) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, nCoords),
			args.SourceHouse,
			pThis->GetOwningHouse(),
			args.Attacker,
			false
		);
	}

	if (const auto nBounty = pTerrainExt->Bounty.Get()) {
		if (pAttackerHoue && pAttackerHoue->CanTransactMoney(nBounty)) {
			pAttackerHoue->TransactMoney(nBounty);
			FlyingStrings::AddMoneyString(true, nBounty, pAttackerHoue, AffectedHouse::All, nCoords);
		}
	}

	return 0;
}

DEFINE_HOOK(0x71D09D, TerrainClass_UnLImbo_Light, 0x6)
{
	GET(TerrainClass*, pThis, ECX);
	GET(CoordStruct*, pCoord, EBP);

	TerrainExtData::Unlimbo(pThis, pCoord);
	return 0;
}

DEFINE_HOOK(0x71CA15, TerrainClass_Limbo_Light, 0x6)
{
	GET(bool, nLimboed, EAX);
	GET(TerrainClass*, pThis, EDI);

	if (nLimboed)
	{
		TerrainExtContainer::Instance.Find(pThis)->LighSource.reset(nullptr);
		TerrainExtContainer::Instance.Find(pThis)->AttachedAnim.reset(nullptr);
	}

	return 0;
}

DEFINE_HOOK(0x71C2BC, TerrainClass_Draw_CustomPal, 0x6)
{
	GET(ConvertClass*, pConvert, EDX);
	GET(TerrainTypeClass*, pThisType, EAX);

	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThisType);

	if (const auto pConvertData = pTerrainExt->CustomPalette) {
		pConvert = pConvertData->GetConvert<PaletteManager::Mode::Temperate>();
	}

	R->EDX(pConvert);
	return 0x0;
}

DEFINE_HOOK(0x71B9BB, TerraiClass_ReceiveDamage_IsTiberiumSpawn, 0x5) //A
{
	enum
	{
		DoCellChailReact = 0x71BAC4,
		RetOriginalFunct = 0x0
	};

	GET(const TerrainClass*, pThis, ESI);

	const auto pTerrainTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	const auto nDamage = pTerrainTypeExt->Damage.Get(100);
	const auto pWH = pTerrainTypeExt->Warhead.Get(RulesClass::Instance->C4Warhead);

	if (auto const pAnim = MapClass::SelectDamageAnimation(nDamage, pWH, MapClass::Instance->GetCellAt(pThis->Location)->LandType, pThis->Location))
	{
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnim, pThis->Location, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, 0),
			nullptr,
			nullptr,
			false
		);
	}

	if (pTerrainTypeExt->AreaDamage)
	{
		MapClass::DamageArea(pThis->Location, nDamage, nullptr, pWH, true, nullptr);
		MapClass::FlashbangWarheadAt(nDamage, pWH, pThis->Location);
	}

	return DoCellChailReact;
}

DEFINE_HOOK(0x71B943, TerrainClass_ReceiveDamage_WoodDestroyer_Force, 0x6)
{
	GET_STACK(bool, IgnoreDefenses, STACK_OFFSET(0x3C, 0x14));

	return IgnoreDefenses ? 0x71B951 : 0x0;
}

DEFINE_HOOK(0x5F4FEF, ObjectClass_Put_RegisterLogic_Terrain, 0x6)
{
	GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	enum { FurtherCheck = 0x5F501B, NoUpdate = 0x5F5045 };

	if(pThis->WhatAmI() == AbstractType::VeinholeMonster) {
		return FurtherCheck;
	}

	if (!pType->IsLogic) {
		return NoUpdate;
	}

	if (pType->WhatAmI() == TerrainTypeClass::AbsID) {
		auto const pTerrainType = static_cast<TerrainTypeClass* const>(pType);
		if (!pTerrainType->SpawnsTiberium
			&& !pTerrainType->IsFlammable
			&& !pTerrainType->IsAnimated
			&& !pTerrainType->IsVeinhole) {
			return NoUpdate;
		}
	}

	return FurtherCheck;
}
//#endif