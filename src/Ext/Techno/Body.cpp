#include "Body.h"

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);
	TechnoExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);
	TechnoExtContainer::Instance.Remove(pItem);
	return 0;
}

//DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TechnoExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x70BF6C, TechnoClass_Load_Suffix, 0x6)
{
	TechnoExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x70C250, TechnoClass_Save_Suffix_Prefix, 0x8)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	GET_STACK(BOOL, isDirty, 0xC);

	TechnoExtContainer::Instance.PrepareStream(pItem, pStm);
	const HRESULT res = pItem->RadioClass::Save(pStm, isDirty);

	if(SUCCEEDED(res))
		TechnoExtContainer::Instance.SaveStatic();

	R->EAX(res);
	return 0x70C266;
}

DEFINE_HOOK(0x7077C0, TechnoClass_Detach, 0x7)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, target, 0x4);
	GET_STACK(bool, all, 0x8);

	TechnoExtContainer::Instance.InvalidatePointerFor(pThis, target, all);

	return 0x0;
}


DEFINE_HOOK(0x710415, TechnoClass_AnimPointerExpired_add, 6)

{
	GET(AnimClass*, pAnim, EAX);
	GET(TechnoClass*, pThis, ECX);

	return 0x0;
}