#include "Body.h"

AircraftTypeExtContainer AircraftTypeExtContainer::Instance;

//TODO :Check

DEFINE_HOOK(0x41C9E3, AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x41CA20, AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ECX);
	AircraftTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x41CE20, AircraftTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x41CE90, AircraftTypeClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(AircraftTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AircraftTypeExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

// S/L very early so we properly trigger `Load3DArt` without need to reconstruct the ExtData !
DEFINE_HOOK(0x41CE5F, AircraftTypeClass_Load_Suffix, 0x6)
{
	AircraftTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x41CEA8, AircraftTypeClass_Save_Suffix, 0x5)
{
	AircraftTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x41CD97, AircraftTypeClass_LoadFromINI, 0xA) // this should make the techno unusable ? becase the game will return false when it
DEFINE_HOOK(0x41CD89, AircraftTypeClass_LoadFromINI, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EBX);
	AircraftTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x716132);
	return 0;
}