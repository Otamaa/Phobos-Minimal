#include "Battery.h"

const char* SW_Battery::GetTypeString() const
{
	return "Battery";
}

bool SW_Battery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_Battery::Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer)
{ }

void SW_Battery::Initialize(SWTypeExt::ExtData* pData)
{ }

void SW_Battery::LoadFromINI(SWTypeExt::ExtData * pData,CCINIClass * pINI)
{ }