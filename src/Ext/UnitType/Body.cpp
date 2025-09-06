#include "Body.h"
#include <Utilities/Macro.h>

UnitTypeExtContainer UnitTypeExtContainer::Instance;
std::vector<UnitTypeExtData*> Container<UnitTypeExtData>::Array;

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

bool UnitTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i) {

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new UnitTypeExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "UnitTypeExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return true;
}

bool UnitTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array) {
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
}

bool FakeUnitTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->UnitTypeClass::LoadFromINI(pINI);
	UnitTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F627C, FakeUnitTypeClass::_ReadFromINI)