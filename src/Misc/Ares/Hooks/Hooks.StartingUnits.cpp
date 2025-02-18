#include "Header.h"

#include <Ext/Rules/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x5d7048, MPGameMode_SpawnBaseUnit_BuildConst, 5)
{
	GET_STACK(HouseClass*, pHouse, 0x18);

	auto pHouseType = pHouse->Type;

	if (!HouseTypeExtContainer::Instance.Find(pHouseType)->StartInMultiplayer_WithConst)
		return 0;

	const auto v7 = HouseExtData::FindBuildable(
		pHouse,
		pHouseType->FindParentCountryIndex(),
		make_iterator(RulesClass::Instance->BuildConst),
		0
	);

	if (!v7)
	{
		Debug::LogInfo(__FUNCTION__" House of country [{}] cannot build anything from [General]BuildConst=.", pHouse->Type->ID);
		return 0x5D70DB;
	}

	const auto pBld = (BuildingClass*)v7->CreateObject(pHouse);

	if (!pBld)
		return 0x5D70DB;

	pBld->ForceMission(Mission::Guard);

	if (v7->GetFoundationWidth() > 2 || v7->GetFoundationHeight(0) > 2) {
		--pHouse->BaseSpawnCell;
	}

	if (!pHouse->IsControlledByHuman())
	{
		pHouse->Func_505180();
		CellStruct base = pHouse->GetBaseCenter();

		pHouse->Base.Center = base;
		pHouse->Base.BaseNodes.Items->MapCoords = base;
		pHouse->Production = 1;
		pHouse->AITriggersActive = true;
	}

	R->EAX(pBld);
	R->EDI(pHouse);
	return 0x5D707E;
}

DEFINE_HOOK(0x5d7337, MPGameMode_SpawnStartingUnits_NoInfantry, 5)
{
	return R->Stack<int>(0x28) ? 0x0 : 0x5D734F;
}

DEFINE_HOOK(0x5D705E, MPGameMode_SpawnBaseUnit_BaseUnit, 6)
{
	enum { hasBaseUnit = 0x5D7064, hasNoBaseUnit = 0x5D70DB };

	GET(HouseClass*, pHouse, EDI);
	GET(UnitTypeClass*, pBaseUnit, EAX);
	R->ESI(pBaseUnit);

	if (pBaseUnit)
		return hasBaseUnit;

	Debug::LogInfo(__FUNCTION__" House of country [{}] cannot build anything from [General]BaseUnit=.", pHouse->Type->ID);
	return hasNoBaseUnit;
}

DEFINE_HOOK(0x5D6F61, MPGameModeClass_CreateStartingUnits_BaseCenter, 8)
{
	GET(MPGameModeClass*, pMode, ECX);
	GET(HouseClass*, pHouse, ESI);
	GET(int*, AmountToSpend, EAX);

	*AmountToSpend = R->EBP<int>();
	CellStruct nBase = pHouse->BaseSpawnCell;

	if (!pMode->SpawnBaseUnits(pHouse, AmountToSpend))
		return 0x5D701B;

	pHouse->ConYards.for_each([](BuildingClass* pConyards) {
		 pConyards->QueueMission(Mission::Construction, true);
		 ++Unsorted::ScenarioInit();
		 pConyards->EnterIdleMode(false, 1);
		 --Unsorted::ScenarioInit();
	});

	//base spawn cell is broken after conyard spawned as building ,..
	if (!pHouse->BaseSpawnCell.IsValid())
		pHouse->BaseSpawnCell = nBase;

	return 0x5D6F77;
}

DEFINE_HOOK(0x5d7163, MPGameMode_SpawnStartingUnits_Types, 8)
{
	LEA_STACK(DynamicVectorClass<TechnoTypeClass*>*, pInfVec, 0x18);
	LEA_STACK(DynamicVectorClass<TechnoTypeClass*>*, pUnitVec, 0x30);
	GET_STACK(HouseClass*, pHouse, 0x4C);

	const auto pTypeExt = HouseTypeExtContainer::Instance.Find(pHouse->Type);

	if (!pTypeExt->StartInMultiplayer_Types.HasValue())
		return UnitTypeClass::Array->Count  <= 0? 0x5D721A : 0x5D716B; //restore overriden instruction

	if (pTypeExt->StartInMultiplayer_Types.empty())
		return 0x5D743E;

	for (auto& start : pTypeExt->StartInMultiplayer_Types) {
		(start->WhatAmI() == UnitTypeClass::AbsID
			? pUnitVec : pInfVec)->AddItem(start);
	}

	return 0x5D72AB;
}

DEFINE_HOOK(0x5d6d9a, MPGameModeClass_CreateStartingUnits_UnitCost, 6)
{
	R->EBP(AresHouseExt::GetTotalCost(RulesExtData::Instance()->StartInMultiplayerUnitCost));
	return 0x5D6ED6;
}
