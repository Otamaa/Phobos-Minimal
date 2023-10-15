#include "Body.h"

void SmudgeTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->AttachedToObject;
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);
	this->Clearable.Read(exINI, pSection, "Clearable");
}

// =============================
// load / save

template <typename T>
void SmudgeTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Clearable)
		;

}

// =============================
// container
SmudgeTypeExtContainer SmudgeTypeExtContainer::Instance;

// =============================
// container hooks
//
DEFINE_HOOK(0x6B52E1, SmudgeTypeClass_CTOR, 0x5)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6B61B5, SmudgeTypeClass_SDDTOR, 0x7)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	SmudgeTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6B5850, SmudgeTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6B58B0, SmudgeTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SmudgeTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	SmudgeTypeExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

// Before : DEFINE_HOOK(0x6B589F, SmudgeTypeClass_Load_Suffix, 0x5)
DEFINE_HOOK(0x6B589D , SmudgeTypeClass_Load_Suffix, 0x6)
{
	SmudgeTypeExtContainer::Instance.LoadStatic();
	return 0;
}

// Before : DEFINE_HOOK(0x6B58CA, SmudgeTypeClass_Save_Suffix, 0x5)
DEFINE_HOOK(0x6B58C8, SmudgeTypeClass_Save_Suffix, 0x5)
{
	SmudgeTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6B57DA , SmudgeTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6B57CD, SmudgeTypeClass_LoadFromINI, 0xA)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x208, -0x4));
	SmudgeTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x6B57DA);
	return 0x0;
}