#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

#include <InfantryClass.h>

ASMJIT_PATCH(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto strength = TechnoExtData::GetInitialStrength(pThis->Type, pThis->Type->Strength);
	pThis->Health = strength;
	pThis->EstimatedHealth = strength;

	return 0;
}

ASMJIT_PATCH(0x7355BA, UnitClass_Init_InitialStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);

	if(TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x7355C0;
}

ASMJIT_PATCH(0x414051, AircraftClass_Init_InitialStrength, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, EAX);

	if (TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Get_TechnoStateComponent()->IsDriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x414057;
}

#include <Ext/Building/Body.h>

ASMJIT_PATCH(0x442C43, BuildingClass_Init, 0x5)
{
	GET(BuildingClass*, pThis, ESI);

	pThis->TechnoClass::Init();

	auto pBldExt = BuildingExtContainer::Instance.Find(pThis);

	pBldExt->MyPrismForwarding = std::make_unique<PrismForwarding>();
	pBldExt->MyPrismForwarding->Owner = pThis;

	HouseExtData* pHouseExt = nullptr;

	if (pThis->Owner) {
		pThis->OwnerCountryIndex = pThis->Owner->Type->ParentIdx;
		pThis->Owner->AddTracking(pThis);
		pHouseExt = HouseExtContainer::Instance.Find(pThis->Owner);
	}

	if (!pThis->Type)
		return 0x442D1Bl;

	auto const pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);

	pBldExt->Type = pBldTypeExt;

	R->EAX(pThis->Type);
	R->ECX(pBldTypeExt->InitialStrength.Get(pThis->Type->Strength));
	return 0x442C7B;
}