#include "Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Utilities/Macro.h>

#include <Misc/DamageArea.h>

/*
		to do :
		- idle anim
		- Re-Draw function , to show damaged part

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
		   (Math::abs(pWarheadExt->Flammability.Get())))
			pThis->Ignite();
	}

	//return handle !
	if (nState == DamageState::PostMortem)
		return PostMortemReturn;
	if (nState == DamageState::NowDead)
		return CheckNowDead;

	return SetReturn;
}*/

#include <New/Entity/FlyingStrings.h>

//this one on Very end of it
//let everything play first
DEFINE_HOOK(0x71BB2C, TerrainClass_ReceiveDamage_NowDead_Add_light, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x3C, -0x4));

	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	// Skip over the removal of the tree as well as destroy sound/anim (for now) if the tree has crumble animation.
	if (pThis->TimeToDie && pTerrainExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance->AddObject(pThis, false);
		VocClass::PlayIndexAtPos(pTerrainExt->CrumblingSound, pThis->GetCoords());
		pThis->UpdatePlacement(PlacementType::Redraw);
		pThis->Disappear(true);
		return 0x71BB79;
	}

	auto const nCoords = pThis->GetCenterCoords();
	VocClass::PlayIndexAtPos(pTerrainExt->DestroySound, nCoords);
	const auto pAttackerHoue = args.Attacker ? args.Attacker->Owner : args.SourceHouse;

	if (auto const pAnimType = pTerrainExt->DestroyAnim) {
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
	GET(TerrainClass*, pThis, ESI);
	GET(ConvertClass*, pConvert, EDX);
	GET(TerrainTypeClass*, pThisType, EAX);


	const auto pTerrainExt = TerrainTypeExtContainer::Instance.Find(pThisType);

	if (const auto pConvertData = pTerrainExt->CustomPalette) {
		auto const pCell = pThis->GetCell();
		int wallOwnerIndex = pCell->WallOwnerIndex;
		int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

		if (wallOwnerIndex >= 0)
			colorSchemeIndex = HouseClass::Array->GetItem(wallOwnerIndex)->ColorSchemeIndex;


		pConvert = pConvertData->ColorschemeDataVector->Items[colorSchemeIndex]->LightConvert;
		R->EBP(pCell->Intensity_Normal);
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

	GET(TerrainClass*, pThis, ESI);

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
		auto pCoord = &pThis->Location;
		DamageArea::Apply(pCoord, nDamage, nullptr, pWH, true, nullptr);
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
			&& !pTerrainType->IsVeinhole
			) {
			return NoUpdate;
		}
	}

	return FurtherCheck;
}
//#endif


DEFINE_HOOK(0x71C6EE, TerrainClass_FireOut_Crumbling, 0x6)
{
	enum { StartCrumbling = 0x71C6F8, Skip = 0x71C72B };

	GET(TerrainClass*, pThis, ESI);

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);

	if (!pThis->TimeToDie && pTypeExt->HasCrumblingFrames)
	{
		// Needs to be added to the logic layer for the anim to work.
		LogicClass::Instance->AddObject(pThis, false);
		VocClass::PlayIndexAtPos(pTypeExt->CrumblingSound, pThis->GetCoords());

		return StartCrumbling;
	}

	return Skip;
}

double PriorHealthRatio = 0.0;

DEFINE_HOOK(0x71B965, TerrainClass_TakeDamage_SetContext, 0x8)
{
	GET(TerrainClass*, pThis, ESI);

	PriorHealthRatio = pThis->GetHealthPercentage();

	return 0;
}

DEFINE_HOOK(0x71B98B, TerrainClass_TakeDamage_RefreshDamageFrame, 0x7)
{
	GET(TerrainClass*, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x3C, -0x4));

	if (!pThis->IsBurning && *args.Damage > 0 && args.WH->Sparky) {
		const auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(args.WH);

		if (!pWarheadExt->Flammability.isset() || ScenarioClass::Instance->Random.PercentChance
		   (Math::abs(pWarheadExt->Flammability.Get())))
			pThis->Ignite();
	}

	auto const pTypeExt = TerrainTypeExtContainer::Instance.Find(pThis->Type);
	double condYellow = RulesExtData::Instance()->ConditionYellow_Terrain;

	if (!pThis->Type->IsAnimated && pTypeExt->HasDamagedFrames && PriorHealthRatio > condYellow && pThis->GetHealthPercentage() <= condYellow)
	{
		pThis->TimeToDie = true; // Dirty hack to get game to redraw the art reliably.
		LogicClass::Instance->AddObject(pThis, false);
	}

	return 0;
}