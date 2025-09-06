#include "Body.h"
#include <Utilities/Macro.h>

std::vector<AircraftTypeExtData*> Container<AircraftTypeExtData>::Array;
AircraftTypeExtContainer AircraftTypeExtContainer::Instance;

bool AircraftTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new AircraftTypeExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "AircraftTypeExtContainer")
			ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
			newPtr->LoadFromStream(Stm);
			Array.push_back(newPtr);
	}

	return true;
}

bool AircraftTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
}

ASMJIT_PATCH(0x41C91F,AircraftTypeClass_CTOR, 0x5)
{
	GET(AircraftTypeClass*, pItem, ESI);
	AircraftTypeExtContainer::Instance.Allocate(pItem);
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

//HRESULT __stdcall FakeAircraftTypeClass::_Load(IStream* pStm)
//{
//	auto hr = this->AircraftTypeClass::Load(pStm);
//
//	if (SUCCEEDED(hr)) {
//		hr = AircraftTypeExtContainer::Instance
//			.ReadDataFromTheByteStream(this, AircraftTypeExtContainer::Instance.Mapped[this], pStm);
//	}
//
//	return hr;
//}
//
//HRESULT __stdcall FakeAircraftTypeClass::_Save(IStream* pStm, BOOL clearDirty)
//{
//	auto hr = this->AircraftTypeClass::Save(pStm, clearDirty);
//
//	if (SUCCEEDED(hr)) {
//		hr = AircraftTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
//	}
//
//	return hr;
//}
//
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E287C, FakeAircraftTypeClass::_Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2880, FakeAircraftTypeClass::_Save)