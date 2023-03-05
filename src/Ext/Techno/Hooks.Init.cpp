#include "Body.h"


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
	GET(UnitTypeClass*, pType, EAX);
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