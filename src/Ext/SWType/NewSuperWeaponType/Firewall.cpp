#include "Firewall.h"

#include <Misc/AresData.h>
#include <Misc/Ares/Hooks/Header.h>

SuperWeaponType SW_Firewall::FirewallType = SuperWeaponType::Invalid;

SW_Firewall::~SW_Firewall() {
	SW_Firewall::FirewallType = SuperWeaponType::Invalid;
}

std::vector<const char*> SW_Firewall::GetTypeString() const
{
	return { "Firestorm" };
}


void SW_Firewall::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::None;
	pData->AttachedToObject->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
};

void SW_Firewall::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	pData->AttachedToObject->Action = Action::None;
	pData->AttachedToObject->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
};

bool SW_Firewall::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (!pThis->Granted)
		return false;

	AresHouseExt::SetFirestormState(pThis->Owner, true);

	if (IsPlayer) {
		pThis->Owner->RecheckTechTree = true;
	}

	return true;
}

void SW_Firewall::Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer)
{
	AresHouseExt::SetFirestormState(pThis->Owner, false);

	if (isPlayer) {
		pThis->Owner->RecheckTechTree = true;
	}
}