#include "Body.h"
#include <Utilities/Macro.h>

UnitTypeExtContainer UnitTypeExtContainer::Instance;
std::vector<UnitTypeExtData*> Container<UnitTypeExtData>::Array;
std::map<TechnoTypeClass*, UnitTypeExtData*> UnitTypeExtContainer::Mapped;

ASMJIT_PATCH(0x7472B1, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x747316, UnitTypeClass_DTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

ASMJIT_PATCH(0x7472E8, UnitTypeClass_NoInt_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.AllocateNoInit(pItem);
	return 0;
}

bool FakeUnitTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->UnitTypeClass::LoadFromINI(pINI);
	UnitTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F627C, FakeUnitTypeClass::_ReadFromINI)

HRESULT __stdcall FakeUnitTypeClass::_Load(IStream* pStm)
{
	auto hr = UnitTypeExtContainer::Instance.ReadDataFromTheByteStream(this,
			UnitTypeExtContainer::Instance.Mapped[this], pStm);

	if (SUCCEEDED(hr)) {
		hr = this->UnitTypeClass::Load(pStm);
	}

	return hr;
}

HRESULT __stdcall FakeUnitTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = UnitTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);

	if (SUCCEEDED(hr)) {
		hr = this->UnitTypeClass::Save(pStm, clearDirty);
	}

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F622C, FakeUnitTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6230, FakeUnitTypeClass::_Save)