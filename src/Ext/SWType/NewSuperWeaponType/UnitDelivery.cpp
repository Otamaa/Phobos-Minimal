#include "UnitDelivery.h"

const char* SW_UnitDelivery::GetTypeString() const
{
	return "UnitDelivery";
}

bool SW_UnitDelivery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_UnitDelivery::Initialize(SWTypeExt::ExtData* pData)
{

}

void SW_UnitDelivery::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}