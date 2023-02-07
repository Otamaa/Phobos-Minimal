#include "Body.h"

#include <Ext/Techno/Body.h>

SpawnManagerExt::ExtContainer SpawnManagerExt::ExtMap;

// =============================
// load / save
template <typename T>
void SpawnManagerExt::ExtData::Serialize(T& Stm) {
	Debug::Log("Processing Element From SpawnManagerExt ! \n");
}

void SpawnManagerExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<SpawnManagerClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SpawnManagerExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<SpawnManagerClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool SpawnManagerExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool SpawnManagerExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

SpawnManagerExt::ExtContainer::ExtContainer() : Container("SpawnManagerClass") { };
SpawnManagerExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//
//DEFINE_HOOK(0x6B6E38, SpawnManagerClass_CTOR,0x5 )
//{
//	GET(SpawnManagerClass*, pItem, ESI);
//	SpawnManagerExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
//	return 0;
//}
//
//DEFINE_HOOK(0x6B7073, SpawnManagerClass_DTOR, 0x6)
//{
//	GET(SpawnManagerClass*, pItem, ESI);
//	SpawnManagerExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x6B7F10, SpawnManagerClass_SaveLoad_Prefix, 0x6)
//DEFINE_HOOK(0x6B80B0, SpawnManagerClass_SaveLoad_Prefix, 0x5)
//{
//
//	GET_STACK(SpawnManagerClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	SpawnManagerExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x6B80A3, SpawnManagerClass_Load_Suffix, 0x5)
//{
//	GET(HRESULT, nRes, EBP);
//
//	if(SUCCEEDED(nRes))
//		SpawnManagerExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6B810D, SpawnManagerClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		SpawnManagerExt::ExtMap.SaveStatic();
//
//	return 0x0;
//}

//Detach Func ! 0x006B7C60sareyt