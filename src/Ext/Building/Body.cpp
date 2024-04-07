#include "Body.h"

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Allocate(pItem);


	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x453ED4, BuildingClass_Load_Suffix, 0x6)
{
	BuildingExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4541B2, BuildingClass_Save_Suffix, 0x6)
{
	BuildingExtContainer::Instance.SaveStatic();
	return 0;
}

void __fastcall BuildingClass_Detach_Wrapper(BuildingClass* pThis , DWORD , AbstractClass* target , bool all)
{
	BuildingExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	pThis->BuildingClass::PointerExpired(target , all);
}
DEFINE_JUMP(VTABLE, 0x7E3EE4, GET_OFFSET(BuildingClass_Detach_Wrapper))