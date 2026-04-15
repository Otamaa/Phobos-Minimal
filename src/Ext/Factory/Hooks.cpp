#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

ASMJIT_PATCH(0x4CA0E3, FactoryClass_AbandonProduction_Invalidate, 0x6)
{
	GET(FactoryClass*, pThis, ESI);

	if (pThis->Owner == HouseClass::CurrentPlayer() &&
		pThis->Object &&
		pThis->Object->WhatAmI() == BuildingClass::AbsID
		)
	{
		pThis->Object->RemoveSidebarObject();
	}

	return 0;
}

// issue 1520: logging stupid shit crashes the game
ASMJIT_PATCH(0x4CA437, FactoryClass_GetCRC, 0x8)
{
	GET(FactoryClass*, pThis, ECX);
	GET_STACK(DWORD, pCRC, 0xC);

	R->ESI<FactoryClass*>(pThis);
	R->EDI(pCRC);

	return 0x4CA501;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
ASMJIT_PATCH(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExtData::Instance()->ExtendedBuildingPlacing)
	{
		const auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);
		const auto pBuilding = cast_to<BuildingClass*>(pFactory->Object);

		if (!pBuilding)
			return 0;

		auto place = &(pBuilding->Type->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat);
		BuildingExtData::ClearPlacingBuildingData(place);
	}

	return 0;
}

ASMJIT_PATCH(0x4CA00D, FactoryClass_AbandonProduction_GetObjectType, 0x9)
{
	//lGET(FactoryClass*, pThis, ESI);
	GET(TechnoClass*, pObject, ECX);

	// use cached type instead of `->GetTechnoType()` the pointer was changed !
	const auto pType = TechnoExtContainer::Instance.Find(pObject)->CurrentType;
	//Debug::LogInfo("[{}]Factory with owner [{} - {}] abandoning production of [{}({}) - {}]",
	//	(void*)pThis,
	//	pThis->Owner->get_ID(), (void*)pThis->Owner,
	//	pType->Name, pType->ID, (void*)pObject);

	R->EAX(pType->GetActualCost(pObject->Owner));
	return 0x4CA03D;
}

ASMJIT_PATCH(0x4CA682, FactoryClass_Total_Techno_Queued_CompareType, 0x8)
{
	GET(TechnoClass*, pObject, ECX);
	GET(TechnoTypeClass*, pTypeCompare, EBX);

	return pObject->GetTechnoType() == pTypeCompare || GET_TECHNOTYPE(pObject) == pTypeCompare ?
		0x4CA68E : 0x4CA693;
}

ASMJIT_PATCH(0x4C9C7B, FactoryClass_QueueProduction_ForceCheckBuilding, 0x7)
{
	enum { SkipGameCode = 0x4C9C9E };
	return RulesExtData::Instance()->ExpandBuildingQueue ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x4CA0B1, FactoryClass_Abandon_NavalProductionFix, 0x6)
{
	enum { SkipUnitTypeCheck = 0x4CA0B7 };

	GET(FactoryClass* const, pThis, ESI);
	GET(UnitClass*, pObject, ECX);

	if (pObject->Type->Naval)
	{
		HouseExtContainer::Instance.Find(pThis->Owner)->ProducingNavalUnitTypeIndex = -1;
	}
	else
	{
		pThis->Owner->ProducingUnitTypeIndex = -1;
	}

	return 0x4CA0B7;
}

ASMJIT_PATCH(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (HouseClass* pOwner = pFactory->Owner)
	{
		HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);

		switch (pFactory->Object->WhatAmI())
		{
		case AbstractType::Building:
			pData->Factory_BuildingType = nullptr;
			break;
		case AbstractType::Unit:
			if (!((UnitClass*)pFactory->Object)->Type->Naval)
				pData->Factory_VehicleType = nullptr;
			else
				pData->Factory_NavyType = nullptr;
			break;
		case AbstractType::Infantry:
			pData->Factory_InfantryType = nullptr;
			break;
		case AbstractType::Aircraft:
			pData->Factory_AircraftType = nullptr;
			break;
		default:
			break;
		}
	}

	return 0;
}
