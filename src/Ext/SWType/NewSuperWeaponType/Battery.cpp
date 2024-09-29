#include "Battery.h"

#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>

std::vector<const char*> SW_Battery::GetTypeString() const
{
	return { "Battery" , "Generator" };
}

bool SW_Battery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (!pThis->Granted)
		return false;

	auto pHouseExt = HouseExtContainer::Instance.Find(pThis->Owner);

	//this check prevent same SW activated multiple times
	if(!pHouseExt->Batteries.contains(pThis)) {
		pHouseExt->Batteries.push_back(pThis);

		pThis->Owner->RecheckPower = true;
	}
	return true;
}

void SW_Battery::Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer)
{
	auto pHouseExt = HouseExtContainer::Instance.Find(pSW->Owner);

	if(pHouseExt->Batteries.remove(pSW))
		pSW->Owner->RecheckPower = true;
}

void SW_Battery::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::None;
	pData->AttachedToObject->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LowPower;
}

void SW_Battery::LoadFromINI(SWTypeExtData * pData,CCINIClass * pINI)
{
	const auto pSection = pData->AttachedToObject->ID;
	INI_EX exINI(pINI);

	pData->Battery_Overpower.Read(exINI, pSection, "Battery.Overpower");
	pData->Battery_KeepOnline.Read(exINI, pSection, "Battery.KeepOnline");

	if(!pData->SW_Power.isset())
		pData->SW_Power.Read(exINI, pSection, "Battery.Power");

	pData->AttachedToObject->Action = Action::None;
	pData->AttachedToObject->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
}

bool SW_Battery::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}
