#include "Body.h"

#include <FactoryClass.h>
#include <TEventClass.h>

// AI Naval queue bugfix hooks

namespace ExitObjectTemp
{
	int ProducingUnitIndex = -1;
}

ASMJIT_PATCH(0x4FB6FC, HouseClass_JustBuilt_NavalProductionFix, 0x6)
{
	enum { SkipGameCode = 0x4FB702 };

	GET(FakeHouseClass* const, pThis, EDI);
	GET(UnitTypeClass* const, pUnitType, EDX);
	GET(int const, ID, EAX);

	if (pUnitType->Naval)
	{
		pThis->_GetExtData()->LastBuiltNavalVehicleType = ID;
		return SkipGameCode;
	}

	return 0;
}

// ASMJIT_PATCH(0x71F003, TEventClass_Execute_NavalProductionFix, 0x6)
// {
// 	enum { Occured = 0x71F014, Skip = 0x71F163 };
//
// 	GET(TEventClass* const, pThis, EBP);
// 	GET(FakeHouseClass* const, pHouse, EAX);
//
// 	if (pHouse->LastBuiltVehicleType != pThis->Value &&
// 		pHouse->_GetExtData()->LastBuiltNavalVehicleType != pThis->Value)
// 	{
// 		return Skip;
// 	}
//
// 	return Occured;
// }

//ASMJIT_PATCH(0x444137, BuildingClass_ExitObject_NavalProductionFix2, 0x6)
//{
//	GET(BuildingClass* const, pThis, ESI);
//	GET(FootClass* const, pObject, EDI);
//
//	auto const pHouse = pThis->Owner;
//
//	if (pObject->WhatAmI() == UnitClass::AbsID && pObject->GetTechnoType()->Naval)
//		pHouse->ProducingUnitTypeIndex = ExitObjectTemp::ProducingUnitIndex;
//
//	return 0;
//}

//skipping call of 0x4FBD80
ASMJIT_PATCH(0x450319, BuildingClass_AI_Factory_NavalProductionFix, 0x6)
{
	enum { SkipGameCode = 0x450332 };

	GET(BuildingClass* const, pThis, ESI);

	auto pHouse = pThis->Owner;
	TechnoTypeClass* pTechnoType = nullptr;
	int index = -1;

	switch (pThis->Type->Factory)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
	{
		index = pHouse->ProducingAircraftTypeIndex;

		if (index >= 0)
			pTechnoType = AircraftTypeClass::Array->Items[index];

		break;
	}
	case AbstractType::Building:
	case AbstractType::BuildingType:
	{
		index = pHouse->ProducingBuildingTypeIndex;

		if (index >= 0)
			pTechnoType = BuildingTypeClass::Array->Items[index];

		break;
	}
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
	{
		index = pHouse->ProducingInfantryTypeIndex;

		if (index >= 0)
			pTechnoType = InfantryTypeClass::Array->Items[index];

		break;
	}
	case AbstractType::Unit:
	case AbstractType::UnitType:
	{
		index = !pThis->Type->Naval ? pHouse->ProducingUnitTypeIndex : HouseExtContainer::Instance.Find(pHouse)->ProducingNavalUnitTypeIndex;

		if (index >= 0)
			pTechnoType = UnitTypeClass::Array->Items[index];

		break;
	}
	default:
		break;
	}


	R->EAX(pTechnoType);
	return SkipGameCode;
}

ASMJIT_PATCH(0x4CA0B1, FactoryClass_Abandon_NavalProductionFix, 0x6)
{
	enum { SkipUnitTypeCheck = 0x4CA0B7 };

	GET(FactoryClass* const, pThis, ESI);
	GET(UnitClass*, pObject, ECX);

	if (pObject->Type->Naval) {
		HouseExtContainer::Instance.Find(pThis->Owner)->ProducingNavalUnitTypeIndex = -1;
	} else {
		pThis->Owner->ProducingUnitTypeIndex = -1;
	}

	return 0x4CA0B7;
}

ASMJIT_PATCH(0x4F91A4, HouseClass_AI_BuildingProductionCheck, 0x6)
{
	enum { SkipGameCode = 0x4F9265, CheckBuildingProduction = 0x4F9240 };

	GET(FakeHouseClass* const, pThis, ESI);

	auto const pExt = pThis->_GetExtData();

	bool cantBuild = pThis->ProducingUnitTypeIndex == -1 && pThis->ProducingInfantryTypeIndex == -1 &&
		pThis->ProducingAircraftTypeIndex == -1 && pExt->ProducingNavalUnitTypeIndex == -1;

	if (pExt->ProducingNavalUnitTypeIndex != -1
		&& !UnitTypeClass::Array->Items[pExt->ProducingNavalUnitTypeIndex]->FindFactory(true, true, true, pThis))
		cantBuild = true;

	if (pThis->ProducingUnitTypeIndex != -1
		&& !UnitTypeClass::Array->Items[pThis->ProducingUnitTypeIndex]->FindFactory(true, true, true, pThis))
		cantBuild = true;

	if (pThis->ProducingInfantryTypeIndex != -1
		&& !InfantryTypeClass::Array->Items[pThis->ProducingInfantryTypeIndex]->FindFactory(true, true, true, pThis))
		cantBuild = true;

	if (pThis->ProducingAircraftTypeIndex != -1
		&& !AircraftTypeClass::Array->Items[pThis->ProducingAircraftTypeIndex]->FindFactory(true, true, true, pThis))
		cantBuild = true;

	if (cantBuild)
		pThis->AI_BaseConstructionUpdate();

	return SkipGameCode;
}

ASMJIT_PATCH(0x4FE0A3, HouseClass_AI_RaiseMoney_NavalProductionFix, 0x6)
{
	GET(FakeHouseClass* const, pThis, ESI);
	pThis->_GetExtData()->ProducingNavalUnitTypeIndex = -1;
	return 0;
}