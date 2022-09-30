#include "Body.h"

InfantryTypeExt::ExtContainer InfantryTypeExt::ExtMap;

void InfantryTypeExt::ExtData::InitializeConstants()
{
}

void InfantryTypeExt::ExtData::InvalidatePointer(void* const ptr, bool bRemoved)
{
}

void InfantryTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->Get()->ID;

	INI_EX exINI(pINI);

	if (!pINI->GetSection(pID))
		return;

	this->DeathBodies_UseDieSequenceAsIndex.Read(exINI, pID, "DeathBodies.UseDieSequenceAsIndex");
}
// =============================
// load / save

template <typename T>
void InfantryTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathBodies_UseDieSequenceAsIndex)
		;
}

bool InfantryTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool InfantryTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

void InfantryTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<InfantryTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void InfantryTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<InfantryTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

InfantryTypeExt::ExtContainer::ExtContainer() : Container("InfantryTypeClass") { }
InfantryTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x523970, InfantryTypeClass_CTOR, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	InfantryTypeExt::ExtMap.JustAllocate(pItem, pItem, "Failed !");
	return 0;
}

DEFINE_HOOK(0x5239D0, InfantryTypeClass_DTOR, 0x5)
{
	GET(InfantryTypeClass* const, pItem, ESI);
	InfantryTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x524960, InfantryTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x524B60, InfantryTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(InfantryTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x524B57, InfantryTypeClass_Load_Suffix, 0x7)
{
	InfantryTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x524C59, InfantryTypeClass_Save_Suffix, 0x5)
DEFINE_HOOK(0x524C50, InfantryTypeClass_Save_Suffix, 0x5)
{
	InfantryTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x52473F, InfantryTypeClass_LoadFromINI, 0x5)
{
	GET(InfantryTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xD0);
	InfantryTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}