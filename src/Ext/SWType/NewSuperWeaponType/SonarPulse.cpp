#include "SonarPulse.h"

std::vector<const char*> SW_SonarPulse::GetTypeString() const
{
	return { "SonarPulse" };
}

bool SW_SonarPulse::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_SonarPulse::Initialize(SWTypeExt::ExtData* pData)
{ }

void SW_SonarPulse::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ }

SWRange SW_SonarPulse::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange{ 10, -1} :pData->SW_Range.Get();
}
