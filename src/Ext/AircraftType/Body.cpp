#include "Body.h"
#include <Utilities/Macro.h>

std::vector<AircraftTypeExtData*> Container<AircraftTypeExtData>::Array;
AircraftTypeExtContainer AircraftTypeExtContainer::Instance;
std::map<TechnoTypeClass*, AircraftTypeExtData*>  AircraftTypeExtContainer::Mapped;

ASMJIT_PATCH(0x41C91F,AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x41CA18, AircraftTypeClass_NoInt_CTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.AllocateNoInit(pItem);
	return 0;
}

ASMJIT_PATCH(0x41CA46,AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeAircraftTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->AircraftTypeClass::LoadFromINI(pINI);
	AircraftTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E28CC, FakeAircraftTypeClass::_ReadFromINI)

HRESULT __stdcall FakeAircraftTypeClass::_Load(IStream* pStm)
{
	auto hr = AircraftTypeExtContainer::Instance
		.ReadDataFromTheByteStream(this, AircraftTypeExtContainer::Instance.Mapped[this], pStm);

	if (SUCCEEDED(hr)) {
		hr = this->AircraftTypeClass::Load(pStm);
	}

	return hr;
}

HRESULT __stdcall FakeAircraftTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = AircraftTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);

	if (SUCCEEDED(hr)) {
		hr = this->AircraftTypeClass::Save(pStm, clearDirty);
	}

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E287C, FakeAircraftTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2880, FakeAircraftTypeClass::_Save)