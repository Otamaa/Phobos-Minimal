#include "Battery.h"

#include <Ext/House/Body.h>

#include <Misc/AresData.h>

std::vector<const char*> SW_Battery::GetTypeString() const
{
	return { "Battery" , "Generator" };
}

bool SW_Battery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

	//this check prevent same SW activated multiple times
	if(std::find_if(pHouseExt->Batteries.begin(),
		pHouseExt->Batteries.end(), [pThis](SuperClass* pSuper)
		{ return pSuper == pThis; }) == pHouseExt->Batteries.end())
		pHouseExt->Batteries.push_back(pThis);

	pThis->Owner->RecheckPower = true;
	return true;
}

void SW_Battery::Deactivate(SuperClass* pSW, CellStruct cell, bool isPlayer)
{
	auto pHouseExt = HouseExt::ExtMap.Find(pSW->Owner);

	for (int i = 0; i < (int)pHouseExt->Batteries.size(); ++i) {
		if (pHouseExt->Batteries[i] == pSW) {
			pHouseExt->Batteries.erase(pHouseExt->Batteries.begin() + i);
		}
	}

	pSW->Owner->RecheckPower = true;
}

void SW_Battery::Initialize(SWTypeExt::ExtData* pData)
{
	pData->OwnerObject()->Action = Action::None;
	pData->OwnerObject()->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::LowPower;
}

void SW_Battery::LoadFromINI(SWTypeExt::ExtData * pData,CCINIClass * pINI)
{ 
	const auto pSection = pData->Get()->ID;
	INI_EX exINI(pINI);

	pData->Battery_Overpower.Read(exINI, pSection, "Battery.Overpower");
	pData->Battery_KeepOnline.Read(exINI, pSection, "Battery.KeepOnline");

	if(!pData->SW_Power.isset())
		pData->SW_Power.Read(exINI, pSection, "Battery.Power");
}