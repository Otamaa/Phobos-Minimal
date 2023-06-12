#include "EMPulse.h"

#include <Utilities/Helpers.h>

std::vector<const char*> SW_EMPulse::GetTypeString() const
{
	return { "EMPulse" , "ChemLauncher" , "MultiLauncher" };
}

bool SW_EMPulse::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{

	return true;
}

void SW_EMPulse::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("EMPulse[%s] init\n", pData->Get()->ID);
}

void SW_EMPulse::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

bool SW_EMPulse::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return true;
}

std::pair<double, double> SW_EMPulse::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return {};
}