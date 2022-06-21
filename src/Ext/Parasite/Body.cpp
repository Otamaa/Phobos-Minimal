#include "Body.h"

template<> const DWORD TExtension<ParasiteClass>::Canary = 0x99954321;
ParasiteExt::ExtContainer ParasiteExt::ExtMap;

// =============================
// load / save

void ParasiteExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) { }

void ParasiteExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) { }

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

ParasiteExt::ExtContainer::ExtContainer() : TExtensionContainer("ParasiteClass") { };
ParasiteExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks


DEFINE_HOOK(0x62932B, ParasiteClass_CTOR, 0x9)
{
	GET(ParasiteClass*, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<ParasiteExt::ExtData>(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(ParasiteClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}
DEFINE_HOOK(0x6296A0, ParasiteClass_Load_Suffix, 0x5)
{
	ParasiteExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6296C4, ParasiteClass_Save_Suffix, 0x5)
{
	ParasiteExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x62940D, ParasiteClass_DTOR, 0x5)
{
	GET(ParasiteClass*, pItem, ESI);
	if (auto pExt = ExtensionWrapper::GetWrapper(pItem)->ExtensionObject)
		pExt->Uninitialize();

	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	return 0;
}