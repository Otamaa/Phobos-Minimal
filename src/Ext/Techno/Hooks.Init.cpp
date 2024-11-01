#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <Ares_TechnoExt.h>

#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveFunctional.h>
#include <Misc/DynamicPatcher/Techno/DriveData/DriveDataFunctional.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>

DEFINE_HOOK_AGAIN(0x43B75C, Techno_CTOR_SetOriginalType, 0x6)
DEFINE_HOOK_AGAIN(0x7353EC, Techno_CTOR_SetOriginalType, 0x6)
DEFINE_HOOK_AGAIN(0x413D3A, Techno_CTOR_SetOriginalType, 0x6)
DEFINE_HOOK(0x517A7F, Techno_CTOR_SetOriginalType, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoTypeClass*, pType, ECX);

	if(!Phobos::Otamaa::DoingLoadGame)
		TechnoExtContainer::Instance.Find(pThis)->Type = (pType);

	return 0x0;
}


DEFINE_HOOK(0x517D69, InfantryClass_Init_InitialStrength, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	const auto strength = TechnoExtData::GetInitialStrength(pThis->Type, pThis->Type->Strength);
	pThis->Health = strength;
	pThis->EstimatedHealth = strength;

	return 0;
}

DEFINE_HOOK(0x7355BA, UnitClass_Init_InitialStrength, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(UnitTypeClass*, pType, EAX);

	if(TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x7355C0;
}

DEFINE_HOOK(0x414051, AircraftClass_Init_InitialStrength, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, EAX);

	if (TechnoTypeExtContainer::Instance.Find(pType)->Initial_DriverKilled)
		TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled = true;

	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x414057;
}

DEFINE_HOOK(0x442C75, BuildingClass_Init_InitialStrength, 0x6)
{
	GET(BuildingTypeClass*, pType, EAX);
	R->ECX(TechnoTypeExtContainer::Instance.Find(pType)->InitialStrength.Get(pType->Strength));
	return 0x442C7B;
}