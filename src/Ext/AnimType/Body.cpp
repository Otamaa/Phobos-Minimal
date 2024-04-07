#include "Body.h"

DEFINE_HOOK(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);
	AnimTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x428970, AnimTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x428800, AnimTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(AnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

// Before :
DEFINE_HOOK_AGAIN(0x42892C, AnimTypeClass_Load_Suffix, 0x6)
DEFINE_HOOK(0x428958, AnimTypeClass_Load_Suffix, 0x6)
{
	AnimTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x42898A, AnimTypeClass_Save_Suffix, 0x3)
{
	AnimTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4287E9, AnimTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI, 0xA)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x4287E9);
	return 0;
}