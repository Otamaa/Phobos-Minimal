#include "Body.h"

UnitTypeExtContainer UnitTypeExtContainer::Instance;
std::vector<UnitTypeExtData*> Container<UnitTypeExtData>::Array;

ASMJIT_PATCH(0x74AF5C, UnitTypeClass_CTOR, 0x7)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x7472E1, UnitTypeClass_CTOR_NoInt, 0x7)
{
	GET(UnitTypeClass*, pItem, ESI);
	UnitTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x7472B1, UnitTypeClass_DTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

ASMJIT_PATCH(0x747E90, UnitTypeClass_LoadFromINI, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x4);

	UnitTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x747E9F);

	return 0;
}ASMJIT_PATCH_AGAIN(0x747E9F, UnitTypeClass_LoadFromINI, 0xA)