#include "Body.h"

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x46AF97, BulletClass_Load_Suffix, 0x7)
{
	BulletExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		BulletExtContainer::Instance.SaveStatic();

	return 0;
}

void __fastcall BulletClass_Detach_Wrapper(BulletClass* pThis ,DWORD , AbstractClass* target , bool all)\
{
	BulletExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	pThis->BulletClass::PointerExpired(target , all);
}
DEFINE_JUMP(VTABLE, 0x7E470C, GET_OFFSET(BulletClass_Detach_Wrapper))

static void __fastcall BulletClass_AnimPointerExpired(BulletClass* pThis, void* _, AnimClass* pTarget)
{
	pThis->ObjectClass::AnimPointerExpired(pTarget);
}

DEFINE_JUMP(VTABLE, 0x7E4744, GET_OFFSET(BulletClass_AnimPointerExpired))