#include "Body.h"

template<> const DWORD TExtension<BombClass>::Canary = 0x87659781;
BombExt::ExtContainer BombExt::ExtMap;

// =============================
// load / save

void BombExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{ }

void BombExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{ }

bool BombExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BombExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BombExt::ExtContainer::ExtContainer() : TExtensionContainer("BombClass") { };
BombExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

// BombListClass::Plant()
/*
DEFINE_HOOK_AGAIN(0x438EE9, BombClass_CTOR , 0x6)
DEFINE_HOOK(0x4385FC, BombClass_CTOR, 0x6) // is this inline ?
{
	GET(BombClass*, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<BombExt::ExtData>(pItem);
	return 0;
}

DEFINE_HOOK(0x4393F2, BombClass_SDDTOR, 0x5)
{
	GET(BombClass *, pItem, ECX);
	if (auto pExt = ExtensionWrapper::GetWrapper(pItem)->ExtensionObject)
		pExt->Uninitialize();

	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	return 0;
}

DEFINE_HOOK_AGAIN(0x438B40, BombClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x438BD0, BombClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(BombClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	BombExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x438BBD, BombClass_Load_Suffix, 0xA)
{
	BombExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x438BE4, BombClass_Save_Suffix, 0x5)
{
	BombExt::ExtMap.SaveStatic();
	return 0;
}*/