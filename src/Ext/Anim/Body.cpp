#include "Body.h"

//Only Extend Anim that Has "Type" Pointer
DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_CTOR, 0x5)
DEFINE_HOOK(0x422131, AnimClass_CTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);

	if (pItem)
	{
		if (pItem->Fetch_ID() == -2 && pItem->Type) {
			Debug::Log("Anim[%s - %x] with some weird ID\n", pItem->Type->ID , pItem);
		}

		if(!pItem->Type) {
			Debug::Log("Anim[%x] with no Type pointer\n", pItem);
			return 0x0;
		}

		if (auto pExt = AnimExtContainer::Instance.Allocate(pItem)) {
			// Something about creating this in constructor messes with debris anims, so it has to be done for them later.
			if (!pItem->HasExtras)
				pExt->CreateAttachedSystem();
		}
	}

	return 0;
}

DEFINE_HOOK(0x422A52, AnimClass_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	AnimExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x425280, AnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AnimExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_Load_Suffix, 0x7)
{
	AnimExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_Save_Suffix, 0x5)
{
	AnimExtContainer::Instance.SaveStatic();
	return 0;
}

void __fastcall AnimClass_Detach_Wrapper(AnimClass* pThis, DWORD, AbstractClass* target, bool all)
{
	AnimExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	pThis->AnimClass::PointerExpired(target, all);
}

DEFINE_JUMP(VTABLE, 0x7E337C, GET_OFFSET(AnimClass_Detach_Wrapper));
DEFINE_JUMP(VTABLE, 0x7E3390, GET_OFFSET(AnimExtData::GetOwningHouse_Wrapper));