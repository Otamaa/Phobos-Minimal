#include "Body.h"

template<> const DWORD Extension<SHPReference>::Canary = 0xAB5005BA;
SHPRefExt::ExtContainer SHPRefExt::ExtMap;

void SHPRefExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool SHPRefExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool SHPRefExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

SHPRefExt::ExtContainer::ExtContainer() : Container("SHPReference") {}
SHPRefExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

/*
DEFINE_HOOK(0x69E4F0, SHPReference_CTOR, 0x5)
{
	GET(SHPReference*, pItem, ESI);

	SHPRefExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x69E509 , SHPReference_DTOR, 0x5)
{
	GET(SHPReference*, pItem, ECX);

	SHPRefExt::ExtMap.Remove(pItem);
	return 0;
}*/