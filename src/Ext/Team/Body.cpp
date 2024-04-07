#include "Body.h"

//Everything InitEd
DEFINE_HOOK(0x6E8D05, TeamClass_CTOR, 0x5)
{
	GET(TeamClass*, pThis, ESI);
	TeamExtContainer::Instance.Allocate(pThis);
	return 0;
}

DEFINE_HOOK(0x6E8ECB, TeamClass_DTOR, 0x7)
{
	GET(TeamClass*, pThis, ESI);
	TeamExtContainer::Instance.Remove(pThis);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6EC450, TeamClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6EC540, TeamClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TeamClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TeamExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6EC52F, TeamClass_Load_Suffix, 0x6)
{
	TeamExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6EC55A, TeamClass_Save_Suffix, 0x5)
{
	TeamExtContainer::Instance.SaveStatic();
	return 0;
}

 DEFINE_HOOK(0x6EAE60, TeamClass_Detach, 0x7)
 {
 	GET(TeamClass*, pThis, ECX);
 	GET_STACK(AbstractClass*, target, 0x4);
 	GET_STACK(bool, all, 0x8);

 	TeamExtContainer::Instance.InvalidatePointerFor(pThis, target, true);

 	//return pThis->Target == target ? 0x6EAECC : 0x6EAECF;
 	return 0x0;
 }
