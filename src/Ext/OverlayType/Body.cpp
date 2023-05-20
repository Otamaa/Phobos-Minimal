#include "Body.h"

void OverlayTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return;

	//auto pThis = this->Get();
	//const char* pSection = pThis->get_ID();
}

// =============================
// load / save

template <typename T>
void OverlayTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		;
}


// =============================
// container
OverlayTypeExt::ExtContainer OverlayTypeExt::ExtMap;
OverlayTypeExt::ExtContainer::ExtContainer() : Container("OverlayTypeClass") { }
OverlayTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//namespace DummySpaces
//{
//	int nCount = 0;
//};
//
//DEFINE_HOOK(0x5FE32A, OverlayTypeClass_CTOR, 0x5)
//{
//	GET(OverlayTypeClass*, pItem, ESI);
//	OverlayTypeExt::ExtMap.FindOrAllocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK(0x5FE426,OverlayTypeClass_SDDTOR, 0x6)
//{
//	GET(OverlayTypeClass*, pItem, ESI);
//	OverlayTypeExt::ExtMap.Remove(pItem);
//	return 0x6917E6;
//}
//
//DEFINE_HOOK_AGAIN(0x5FEA1E , OverlayTypeClass_LoadFromINI, 0xA)
//DEFINE_HOOK(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
//{
//	GET(OverlayTypeClass*, pItem, ESI);
//	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));
//
//	OverlayTypeExt::ExtMap.LoadFromINI(pItem, pINI);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x5FEC10 ,OverlayTypeClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x5FEAF0 ,OverlayTypeClass_SaveLoad_Prefix, 0xA)
//{
//	GET_STACK(OverlayTypeClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	OverlayTypeExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x5FEBF7,OverlayTypeClass_Load_Suffix, 0x9)
//{
//	OverlayTypeExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//DEFINE_HOOK(0x5FEC28,OverlayTypeClass_Save_Suffix, 0x5)
//{
//	OverlayTypeExt::ExtMap.SaveStatic();
//	return 0;
//}