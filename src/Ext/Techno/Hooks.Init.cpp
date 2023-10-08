#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <Misc/AresData.h>
#include <Ares_TechnoExt.h>

#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

// init inside type check
// should be no problem here
DEFINE_HOOK(0x6F42ED, TechnoClass_Init_Early, 0xA)
{
	GET(TechnoClass*, pThis, ESI);

	auto pType = pThis->GetTechnoType();

	if (!pType)
		return 0x0;

	if (pThis->Owner) {
		HouseExt::ExtMap.Find(pThis->Owner)->LimboTechno.push_back(pThis);
	}

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	//AircraftDiveFunctional::Init(pExt, pTypeExt);

	if (pTypeExt->AttachtoType == AircraftTypeClass::AbsID)
	{
		if (pTypeExt->MyFighterData.Enable)
		{
			pExt->MyFighterData = std::make_unique<FighterAreaGuard>();
			pExt->MyFighterData->OwnerObject = (AircraftClass*)pThis;
		}
	}

	TechnoExt::InitializeItems(pThis, pType);

	return 0x0;
}

DEFINE_HOOK(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto strength = TechnoExt::GetInitialStrength(pThis->Type, pThis->Type->Strength);
	pThis->Health = strength;
	pThis->EstimatedHealth = strength;

	return 0;
}

DEFINE_HOOK(0x7355BA, UnitClass_Init_InitialStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);

	if(TechnoTypeExt::ExtMap.Find(pType)->Initial_DriverKilled)
		pThis->align_154->Is_DriverKilled = true;

	R->EAX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x7355C0;
}

DEFINE_HOOK(0x414051, AircraftClass_Init_InitialStrength, 0x6)
{
	GET(AircraftTypeClass*, pType, EAX);
	R->EAX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x414057;
}

DEFINE_HOOK(0x442C75, BuildingClass_Init_InitialStrength, 0x6)
{
	GET(BuildingTypeClass*, pType, EAX);
	R->ECX(TechnoExt::GetInitialStrength(pType, pType->Strength));
	return 0x442C7B;
}