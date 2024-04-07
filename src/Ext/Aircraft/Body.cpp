#include "Body.h"


AircraftExtContainer AircraftExtContainer::Instance;

DEFINE_HOOK(0x413F6A, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);
	AircraftExtContainer::Instance.Allocate(pItem);
	if (auto pExt = GetExtPtr<AircraftExtData*, AircraftClass*, true>(pItem))
		Debug::Log("%x", pExt);
	return 0;
}

DEFINE_HOOK(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);
	AircraftExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AircraftExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
{
	AircraftExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
{
	AircraftExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AbstractClass*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	AircraftExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	return 0x0;
}
