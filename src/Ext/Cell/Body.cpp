#include "Body.h"

template<> const DWORD Extension<CellClass>::Canary = 0x87688621;
CellExt::ExtContainer CellExt::ExtMap;

// ============================ =
// load / save

void CellExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Stm
		//.Process(NewPowerups)
		//.Process(FoggedObjects)
		;
}

void CellExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm
		//.Process(NewPowerups)
		//.Process(FoggedObjects)
		;
}

bool CellExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool CellExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

CellExt::ExtContainer::ExtContainer() : Container("CellClass") { };
CellExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x47BDA3, CellClass_CTOR, 0x5)
{
	GET(CellClass*, pItem, EAX);
	CellExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x47BB60, CellClass_DTOR, 0x6)
{
	GET(CellClass*, pItem, ECX);
	CellExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x483C10, CellClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4839F0, CellClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(CellClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	CellExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x483C00, CellClass_Load_Suffix, 0x5)
{
	CellExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x483C79, CellClass_Save_Suffix, 0x6)
{
	CellExt::ExtMap.SaveStatic();
	return 0;
}