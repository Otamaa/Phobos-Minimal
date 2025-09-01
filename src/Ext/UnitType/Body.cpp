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

ASMJIT_PATCH(0x7472E1, UnitTypeClass_CTOR_NoInt, 0x7)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.AllocateNoInit(pItem);
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
	bool status = this->UnitTypeClass::LoadFromINI(pINI);
	UnitTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F627C, FakeUnitTypeClass::_ReadFromINI)
