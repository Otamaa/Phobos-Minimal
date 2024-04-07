#include "Body.h"

DEFINE_HOOK_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)
DEFINE_HOOK(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	auto Iter = OverlayTypeExtContainer::Instance.Map.find(pItem);

	if (Iter == OverlayTypeExtContainer::Instance.Map.end())
	{
		auto ptr = OverlayTypeExtContainer::Instance.AllocateUnlchecked(pItem);
		Iter = OverlayTypeExtContainer::Instance.Map.emplace(pItem, ptr).first;
	}

	OverlayTypeExtContainer::Instance.SetExtAttribute(pItem, Iter->second);

	return 0;
}

DEFINE_HOOK(0x5FE3F6, OverlayTypeClass_DTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);

	auto extData = OverlayTypeExtContainer::Instance.GetExtAttribute(pItem);
	OverlayTypeExtContainer::Instance.ClearExtAttribute(pItem);
	OverlayTypeExtContainer::Instance.Map.erase(pItem);
	delete extData;

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEAF0, OverlayTypeClass_SaveLoad_Prefix, 0xA)
DEFINE_HOOK(0x5FEC10, OverlayTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(OverlayTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	OverlayTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x5FEBFA, OverlayTypeClass_Load_Suffix, 0x6)
{
	OverlayTypeExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x5FEC2A, OverlayTypeClass_Save_Suffix, 0x6)
{
	OverlayTypeExtContainer::Instance.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x5FEA1E, OverlayTypeClass_LoadFromINI, 0xA)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));

	OverlayTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x5FEA1E);

	return 0;
}