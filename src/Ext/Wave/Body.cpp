#include "Body.h"

WaveExt::ExtContainer WaveExt::ExtMap;

void WaveExt::ExtData::Initialize()
{ }

void  WaveExt::ExtData::InitWeaponData()
{

}

// =============================
// load / save
template <typename T>
void WaveExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void WaveExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WaveClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WaveExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WaveClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WaveExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool WaveExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

WaveExt::ExtContainer::ExtContainer() : Container("WaveClass") {}
WaveExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//
//DEFINE_HOOK(0x75EA59, WaveClass_CTOR, 0x5)
//{
//	GET(WaveClass*, pItem, ESI);
//
//	WaveExt::ExtMap.FindOrAllocate(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x75F7D0, WaveClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x75F650, WaveClass_SaveLoad_Prefix, 0x6)
//{
//	GET_STACK(WaveClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	WaveExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x75F7BA, WaveClass_Load_Suffix, 0x5)
//{
//	GET(HRESULT, nRes, EBP);
//
//	if(SUCCEEDED(nRes))
//		WaveExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x75F82F, WaveClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		WaveExt::ExtMap.SaveStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x75ED57 , WaveClass_DTOR, 0x6)
//DEFINE_HOOK(0x763226, WaveClass_DTOR, 0x6)
//{
//	GET(WaveClass*, pItem, EDI);
//
//	WaveExt::ExtMap.Remove(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x75F623, WaveClass_Detach, 0x6)
//{
//	GET(WaveClass*, pItem, ESI);
//	GET_BASE(void*, pTarget, 0x4);
//	GET_BASE(bool, bRemove, 0x8);
//
//	if (auto pExt = WaveExt::ExtMap.Find(pItem))
//		pExt->InvalidatePointer(pTarget, bRemove);
//
//	return 0;
//}
