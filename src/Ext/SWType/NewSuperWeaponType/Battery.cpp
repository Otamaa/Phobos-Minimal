#include "Battery.h"

std::vector<const char*> SW_Battery::GetTypeString() const
{
	return { "Battery" , "Generator" };
}

bool SW_Battery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_Battery::Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer)
{ }

void SW_Battery::Initialize(SWTypeExt::ExtData* pData)
{ Debug::Log("Battery[%s] init\n", pData->Get()->ID); }

void SW_Battery::LoadFromINI(SWTypeExt::ExtData * pData,CCINIClass * pINI)
{ }