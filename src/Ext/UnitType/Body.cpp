#include "Body.h"
#include <Utilities/Macro.h>

UnitTypeExtContainer UnitTypeExtContainer::Instance;
std::vector<UnitTypeExtData*> Container<UnitTypeExtData>::Array;

void Container<UnitTypeExtData>::Clear()
{
	Array.clear();
}

bool UnitTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool UnitTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

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

bool FakeUnitTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	UnitTypeExtContainer::Instance.Find(this)->Initialize();
	bool status = this->UnitTypeClass::LoadFromINI(pINI);
	UnitTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F627C, FakeUnitTypeClass::_ReadFromINI)