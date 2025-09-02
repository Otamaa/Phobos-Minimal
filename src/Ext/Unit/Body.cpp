#include "Body.h"

#include <Utilities/Macro.h>

UnitExtContainer UnitExtContainer::Instance;
std::vector<UnitExtData*> Container<UnitExtData>::Array;

ASMJIT_PATCH(0x73544D, UnitClass_CTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x7359DC, UnitClass_DTOR, 0x7)
{
	GET(UnitClass*, pItem, ESI);
	UnitExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeUnitClass::_Detach(AbstractClass* target, bool all)
{
	UnitExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->UnitClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5C98, FakeUnitClass::_Detach)