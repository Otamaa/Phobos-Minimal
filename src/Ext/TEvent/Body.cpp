#include "Body.h"

DEFINE_HOOK(0x71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);

	TEventExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71FAA6, TEventClass_SDDTOR, 0x6) // Factory
DEFINE_HOOK(0x71E856, TEventClass_SDDTOR, 0x6)
{
	GET(TEventClass*, pItem, ESI);
	TEventExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71F930, TEventClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x71F8C0, TEventClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TEventClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TEventExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71F92B, TEventClass_Load_Suffix, 0x5)
{
	TEventExtContainer::Instance.LoadStatic();
	return 0x0;
}

DEFINE_HOOK(0x71F94A, TEventClass_Save_Suffix, 0x5)
{
	TEventExtContainer::Instance.SaveStatic();
	return 0x0;
}