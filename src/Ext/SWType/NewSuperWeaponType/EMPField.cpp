#include "EMPField.h"

const char* SW_EMPField::GetTypeString() const
{
	return "EMPField";
}

bool SW_EMPField::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto pType = pThis->Type;
	auto pData = SWTypeExt::ExtMap.Find(pType);
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pThis->Owner->Buildings) {	
		if (pData->IsLaunchSiteEligible(Coords, pBld, false)) {
			pFirer = pBld;
			break;
		}
	}

	return GameCreate<EMPulseClass>(Coords, int(pType->Range), 100 /*pData->EMPFieldDuration.Get()*/, pFirer);

}

void SW_EMPField::Initialize(SWTypeExt::ExtData* pData)
{

}

void SW_EMPField::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

bool SW_EMPField::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return false;
}

std::pair<double, double> SW_EMPField::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return {};
}

SWRange SW_EMPField::GetRange(const SWTypeExt::ExtData* pData) const
{
	return {};
}
