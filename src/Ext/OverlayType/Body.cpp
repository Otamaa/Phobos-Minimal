#include "Body.h"

template<> const DWORD Extension<OverlayTypeClass>::Canary = 0x414B4B4A;
OverlayTypeExt::ExtContainer OverlayTypeExt::ExtMap;

void OverlayTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->get_ID();

	if (!pINI->GetSection(pSection))
		return;

}
// =============================
// load / save

template <typename T>
void OverlayTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void OverlayTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<OverlayTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void OverlayTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<OverlayTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void OverlayTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool OverlayTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool OverlayTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

OverlayTypeExt::ExtContainer::ExtContainer() : Container("OverlayTypeClass") { }
OverlayTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
namespace DummySpaces
{
	int nCount = 0;
};

DEFINE_HOOK(0x5FE3A7, OverlayTypeClass_CTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);
	//Debug::Log_WithBool(false, __FUNCTION__" Called ! [%x] \n", pItem);

	OverlayTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5FE426,OverlayTypeClass_SDDTOR, 0x6)
{
	GET(OverlayTypeClass*, pItem, ESI);
	//Debug::Log_WithBool(false, __FUNCTION__" Called ! [%x] \n", pItem);

	OverlayTypeExt::ExtMap.Remove(pItem);

	return 0x6917E6;
}

DEFINE_HOOK(0x5FE7B1, OverlayTypeClass_LoadFromINI, 0xC)
{
	GET(OverlayTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x294, -0x4));

	//Debug::Log_WithBool(false, __FUNCTION__" Called ! [%x] Name[%s] at[%d]  \n", pItem , pItem->get_ID(), DummySpaces::nCount);
	//++DummySpaces::nCount;
	
	OverlayTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

DEFINE_HOOK_AGAIN(0x5FEC10 ,OverlayTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x5FEAF0 ,OverlayTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(OverlayTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	if (Phobos::Config::MoreDetailSLDebugLog)
	Debug::Log(__FUNCTION__" Called ! [%x] \n", pItem);

	OverlayTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x5FEBF9 ,OverlayTypeClass_Load_Suffix, 0xA)
{
	if (Phobos::Config::MoreDetailSLDebugLog)
	Debug::Log(__FUNCTION__" Called ! \n");

	OverlayTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5FEC2A ,OverlayTypeClass_Save_Suffix, 0x5)
{
	if (Phobos::Config::MoreDetailSLDebugLog)
	Debug::Log(__FUNCTION__" Called ! \n");

	OverlayTypeExt::ExtMap.SaveStatic();
	return 0;
}