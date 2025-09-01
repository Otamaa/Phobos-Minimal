#include "Body.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

#include <Helpers\Macro.h>

bool OverlayTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr) || parseFailAddr)
		return false;

	auto pThis = this->This();
	INI_EX exINI(pINI);

	//auto const pArtINI = &CCINIClass::INI_Art();
	auto pArtSection = pThis->ImageFile;

	this->Palette.Read(exINI , pArtSection, "Palette");
	this->ZAdjust.Read(exINI, pArtSection, "ZAdjust");

	return true;
}

// =============================
// load / save

template <typename T>
void OverlayTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->ZAdjust)
		;
}


// =============================
// container
OverlayTypeExtContainer OverlayTypeExtContainer::Instance;
std::vector<OverlayTypeExtData*> Container< OverlayTypeExtData>::Array;

// =============================
// container hooks

ASMJIT_PATCH(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	OverlayTypeExtContainer::Instance.Allocate(pItem);

	return 0;
} ASMJIT_PATCH_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)

ASMJIT_PATCH(0x5FE3E1, OverlayTypeClass_CTOR_NoIint, 0x7)
{
	GET(OverlayTypeClass*, pItem, EAX);
	OverlayTypeExtContainer::Instance.AllocateNoInit(pItem);
	return 0x0;
}

ASMJIT_PATCH(0x5FE3F6, OverlayTypeClass_DTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);
	OverlayTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

bool FakeOverlayTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->OverlayTypeClass::LoadFromINI(pINI);
	OverlayTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF664, FakeOverlayTypeClass::_ReadFromINI)
