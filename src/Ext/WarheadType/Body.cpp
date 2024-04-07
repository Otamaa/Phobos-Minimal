#include "Body.h"

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);
	WarheadTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);
	WarheadTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x75E2C0, WarheadTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75E0C0, WarheadTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	WarheadTypeExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x75E2AE, WarheadTypeClass_Load_Suffix, 0x7)
{
	WarheadTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x75E39C, WarheadTypeClass_Save_Suffix, 0x5)
{
	WarheadTypeExtContainer::Instance.SaveStatic();
	return 0;
}

// is return not valid
DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0, WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x75DEAF);

	//0x75DE9A do net set isOrganic here , just skip it to next adrress to execute ares hook
	return// R->Origin() == 0x75DE9A ? 0x75DEA0 :
		0;
}
