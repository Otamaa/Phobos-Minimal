#include "Body.h"
#include <Utilities/GeneralUtils.h>

void OverlayTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return;

	auto pThis = this->Get();
	INI_EX exINI(pINI);

	auto const pArtINI = &CCINIClass::INI_Art();
	auto pArtSection = pThis->ImageFile;

	this->Palette.Read(exINI , pArtSection, "Palette");
}

// =============================
// load / save

template <typename T>
void OverlayTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Palette)
		;
}


// =============================
// container
OverlayTypeExt::ExtContainer OverlayTypeExt::ExtMap;
OverlayTypeExt::ExtContainer::ExtContainer() : Container("OverlayTypeClass") { }
OverlayTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
DEFINE_HOOK_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)
DEFINE_HOOK(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	OverlayTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEAF0, OverlayTypeClass_SaveLoad_Prefix, 0xA)
DEFINE_HOOK(0x5FEC10, OverlayTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(OverlayTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	OverlayTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x5FEBFA, OverlayTypeClass_Load_Suffix, 0x6)
{
	OverlayTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x5FEC2A, OverlayTypeClass_Save_Suffix, 0x6)
{
	OverlayTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x5FEA1E, OverlayTypeClass_LoadFromINI, 0xA)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));

	OverlayTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x5FEA1E);

	return 0;
}