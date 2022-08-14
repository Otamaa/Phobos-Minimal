#include "Body.h"

SuperExt::ExtContainer SuperExt::ExtMap;

// =============================
// load / save

template <typename T>
void SuperExt::ExtData::Serialize(T& Stm) { }

void SuperExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<SuperClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void SuperExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<SuperClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool SuperExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool SuperExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

SuperExt::ExtContainer::ExtContainer() : Container("SuperClass") { }
SuperExt::ExtContainer::~ExtContainer() = default;

void SuperExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	SuperExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	SuperExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x6CB120, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ECX);
	SuperExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CDEF0, SuperClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6CDFD0, SuperClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SuperClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CDFC7, SuperClass_Load_Suffix, 0x5)
{
	SuperExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CDFEA, SuperClass_Save_Suffix, 0x5)
{
	SuperExt::ExtMap.SaveStatic();
	return 0;
}

/*
DEFINE_HOOK(0x6CE001 , SuperClass_Detach , 0x5)
{
	GET(SuperClass*, pThis, ESI);
	GET(void*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8)); ??
}*/