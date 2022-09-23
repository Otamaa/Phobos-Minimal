#include "Body.h"

SmudgeTypeExt::ExtContainer SmudgeTypeExt::ExtMap;

void SmudgeTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	this->Clearable.Read(exINI, pSection, "Clearable");
}

// =============================
// load / save

template <typename T>
void SmudgeTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Clearable)
		;

}

void SmudgeTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<SmudgeTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void SmudgeTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<SmudgeTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool SmudgeTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool SmudgeTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

SmudgeTypeExt::ExtContainer::ExtContainer() : Container("SmudgeTypeClass") { }
SmudgeTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
#ifndef ENABLE_NEWHOOKS
DEFINE_HOOK(0x6B52E1, SmudgeTypeClass_CTOR, 0x5)
{
	GET(SmudgeTypeClass*, pItem, ESI);
#ifndef ENABLE_NEWHOOKS
	SmudgeTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	SmudgeTypeExt::ExtMap.FindOrAllocate(pItem);
#endif

	return 0;
}

DEFINE_HOOK(0x6B61B5, SmudgeTypeClass_SDDTOR, 0x7)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6B5850, SmudgeTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6B58B0, SmudgeTypeClass_SaveLoad_Prefix, 0x8)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	GET_STACK(SmudgeTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	SmudgeTypeExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x6B589F, SmudgeTypeClass_Load_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	SmudgeTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6B58CA, SmudgeTypeClass_Save_Suffix, 0x5)
{
	Debug::Log("%s Executed ! \n", __FUNCTION__);
	SmudgeTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x6B57C7, SmudgeTypeClass_LoadFromINI, 0x6)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x208, -0x4));
	pItem->Image = R->EAX<SHPStruct*>();
	SmudgeTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0x6B57CD;
}
#endif