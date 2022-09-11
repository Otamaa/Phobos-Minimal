 #include "Body.h"

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
	return true;// Stm.Success();
}

bool TemporalExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return true;//Stm.Success();
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
#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK_AGAIN(0x71A4D6, TemporalClass_CTOR, 0x4)
DEFINE_HOOK(0x71A594, TemporalClass_CTOR, 0x7)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET(TemporalClass*, pItem, ESI);
	TemporalExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
	return 0;
}

DEFINE_HOOK_AGAIN(0x71B218, TemporalClass_SDDTOR, 0x7)
DEFINE_HOOK(0x71A638, TemporalClass_SDDTOR, 0x7)
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
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	TemporalExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x71A714, TemporalClass_Save_Suffix, 0x5)
{
	//Debug::Log("%s Executed ! \n", __FUNCTION__);
	TemporalExt::ExtMap.SaveStatic();
	return 0;
}
#endif