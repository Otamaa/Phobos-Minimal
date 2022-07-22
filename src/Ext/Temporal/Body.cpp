#include "Body.h"

template<> const DWORD Extension<TemporalClass>::Canary = 0x82229781;
TemporalExt::ExtContainer TemporalExt::ExtMap;

// =============================
// load / save

template <typename T>
void TemporalExt::ExtData::Serialize(T& Stm) {
	Debug::Log("Processing Element From TemporalExt ! \n");


}

void TemporalExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TemporalClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void TemporalExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TemporalClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool TemporalExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool TemporalExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

void TemporalExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{
}

TemporalExt::ExtContainer::ExtContainer() : Container("TemporalClass") { };
TemporalExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x71A482, TemporalClass_CTOR, 0xB)
DEFINE_HOOK(0x71A538, TemporalClass_CTOR, 0xB)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TemporalClass*, pItem, ESI);
	TemporalExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71B1E6 , TemporalClass_SDDTOR, 0x5)
DEFINE_HOOK(0x71A606, TemporalClass_SDDTOR, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TemporalClass*, pItem, ESI);
	TemporalExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71A660, TemporalClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x71A700, TemporalClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TemporalClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	TemporalExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x71A6F8, TemporalClass_Load_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	TemporalExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x71A714, TemporalClass_Save_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	TemporalExt::ExtMap.SaveStatic();
	return 0;
}