#include "Body.h"

#include <Helpers\Macro.h>

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

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeSmudgeTypeClass::_Load(IStream* pStm)
{

	SmudgeTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->SmudgeTypeClass::Load(pStm);

	if (SUCCEEDED(res))
		SmudgeTypeExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeSmudgeTypeClass::_Save(IStream* pStm, bool clearDirty)
{

	SmudgeTypeExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->SmudgeTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		SmudgeTypeExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_JUMP(VTABLE, 0x7F353C, MiscTools::to_DWORD(&FakeSmudgeTypeClass::_Load))
DEFINE_JUMP(VTABLE, 0x7F3540, MiscTools::to_DWORD(&FakeSmudgeTypeClass::_Save))

DEFINE_HOOK_AGAIN(0x6B57DA , SmudgeTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6B57CD, SmudgeTypeClass_LoadFromINI, 0xA)
{
	GET(SmudgeTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x208, -0x4));
	SmudgeTypeExtContainer::Instance.LoadFromINI(pItem, pINI , R->Origin() == 0x6B57DA);
	return 0x0;
}