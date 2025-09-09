#include "Body.h"
#include <Utilities/Macro.h>

std::vector<AircraftTypeExtData*> Container<AircraftTypeExtData>::Array;
AircraftTypeExtContainer AircraftTypeExtContainer::Instance;

void Container<AircraftTypeExtData>::Clear()
{
	Array.clear();
}


bool AircraftTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool AircraftTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
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
