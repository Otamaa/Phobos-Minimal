#include "Body.h"

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);
	//BulletTypeExtContainer::Instance.Allocate(pItem);

	auto Iter = BulletTypeExtContainer::Instance.Map.find(pItem);

	if (Iter == BulletTypeExtContainer::Instance.Map.end())
	{
		auto ptr = BulletTypeExtContainer::Instance.AllocateUnlchecked(pItem);
		Iter = BulletTypeExtContainer::Instance.Map.emplace(pItem, ptr).first;
	}

	BulletTypeExtContainer::Instance.SetExtAttribute(pItem, Iter->second);
	return 0;
}

DEFINE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ESI);
	auto extData = BulletTypeExtContainer::Instance.GetExtAttribute(pItem);
	BulletTypeExtContainer::Instance.ClearExtAttribute(pItem);
	BulletTypeExtContainer::Instance.Map.erase(pItem);
	delete extData;
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

//// Before : 0x46C722 , 0x4
//// After : 46C70F , 0x6
DEFINE_HOOK(0x46C70F, BulletTypeClass_Load_Suffix, 0x6)
{
	GET(BulletTypeClass*, pThis, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->ShrapnelWeapon);
	BulletTypeExtContainer::Instance.LoadStatic();

	return 0x46C720;
}

//// Before : 0x46C74A , 0x3
//// After : 46C744 , 0x6
DEFINE_HOOK(0x46C744, BulletTypeClass_Save_Suffix, 0x6)
{
	GET(HRESULT, nRes, EAX);

	if (SUCCEEDED(nRes))
	{
		nRes = 0;
		BulletTypeExtContainer::Instance.SaveStatic();
	}

	return 0x46C74A;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);
	BulletTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x46C429);

	return 0;
}
