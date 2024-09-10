#include "Body.h"
#include <Utilities/GeneralUtils.h>

void OverlayTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	if (parseFailAddr)
		return;

	auto pThis = this->AttachedToObject;
	INI_EX exINI(pINI);

	auto const pArtINI = &CCINIClass::INI_Art();
	auto pArtSection = pThis->ImageFile;

	this->Palette.Read(exINI , pArtSection, "Palette");
	this->ZAdjust.Read(exINI, pArtSection, "ZAdjust");
}

// =============================
// load / save

template <typename T>
void OverlayTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Palette)
		.Process(this->ZAdjust)
		;
}


// =============================
// container
OverlayTypeExtContainer OverlayTypeExtContainer::Instance;

bool OverlayTypeExtContainer::Load(OverlayTypeClass* key, IStream* pStm)
{
	if (!key)
		return false;

	auto ptr = OverlayTypeExtContainer::Instance.Map.get_or_default(key);

	if (!ptr) {
		ptr = OverlayTypeExtContainer::Instance.Map.insert_unchecked(key,
			  this->AllocateUnchecked(key));
	}

	this->ClearExtAttribute(key);
	this->SetExtAttribute(key, ptr);

	PhobosByteStream loader { 0 };
	if (loader.ReadBlockFromStream(pStm)) {
		PhobosStreamReader reader { loader };
		if (reader.Expect(OverlayTypeExtData::Canary)
			&& reader.RegisterChange(ptr)) {
			ptr->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock()) {
				return true;
			}
		}
	}

	return false;
}

// =============================
// container hooks
DEFINE_HOOK_AGAIN(0x5FE3AF, OverlayTypeClass_CTOR, 0x5)
DEFINE_HOOK(0x5FE3A2, OverlayTypeClass_CTOR, 0x5)
{
	GET(OverlayTypeClass*, pItem, EAX);

	auto ptr = OverlayTypeExtContainer::Instance.Map.get_or_default(pItem);

	if (!ptr) {
		ptr = OverlayTypeExtContainer::Instance.Map.insert_unchecked(pItem,
			  OverlayTypeExtContainer::Instance.AllocateUnchecked(pItem));
	}

	OverlayTypeExtContainer::Instance.SetExtAttribute(pItem, ptr);

	return 0;
}

DEFINE_HOOK(0x5FE3F6, OverlayTypeClass_DTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);

	auto extData = OverlayTypeExtContainer::Instance.GetExtAttribute(pItem);
	OverlayTypeExtContainer::Instance.ClearExtAttribute(pItem);
	OverlayTypeExtContainer::Instance.Map.erase(pItem);
	if(extData)
		DLLDelete(extData);

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEAF0, OverlayTypeClass_SaveLoad_Prefix, 0xA)
DEFINE_HOOK(0x5FEC10, OverlayTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(OverlayTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	OverlayTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x5FEBFA, OverlayTypeClass_Load_Suffix, 0x6)
{
	OverlayTypeExtContainer::Instance.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x5FEC2A, OverlayTypeClass_Save_Suffix, 0x6)
{
	OverlayTypeExtContainer::Instance.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEA11, OverlayTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x5FEA1E, OverlayTypeClass_LoadFromINI, 0xA)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFSET(0x28C, 0x4));

	OverlayTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x5FEA1E);

	return 0;
}