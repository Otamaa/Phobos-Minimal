#include "Body.h"

template<> const DWORD Extension<ParasiteClass>::Canary = 0x99954321;
ParasiteExt::ExtContainer ParasiteExt::ExtMap;

// =============================
// load / save
template <typename T>
void ParasiteExt::ExtData::Serialize(T& Stm) {
	Debug::Log("Processing Element From ParasiteExt ! \n");
}

void ParasiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	Extension<ParasiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ParasiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	Extension<ParasiteClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool ParasiteExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ParasiteExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ParasiteExt::ExtContainer::ExtContainer() : Container("ParasiteClass") { };
ParasiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x62932B, ParasiteClass_CTOR, 0x9)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(ParasiteClass*, pItem, ESI);
	ParasiteExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x62940D, ParasiteClass_DTOR, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(ParasiteClass*, pItem, ESI);
	ParasiteExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET_STACK(ParasiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6296A7, ParasiteClass_Load_Suffix, 0x5)
DEFINE_HOOK(0x6296A0, ParasiteClass_Load_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	ParasiteExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6296C4, ParasiteClass_Save_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	ParasiteExt::ExtMap.SaveStatic();
	return 0;
}
