#include "Firewall.h"

#include <Misc/AresData.h>

//TODO : set some static var of AresData
// since FW not yet reimplement ,we need to make sure ares stuffs set properly here !

SW_Firewall::~SW_Firewall() {
	SW_Firewall_Type = SuperWeaponType::Invalid;
}

std::vector<const char*> SW_Firewall::GetTypeString() const
{
	return { "Firestorm" };
}

void SW_Firewall::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	pData->OwnerObject()->Action = Action::None;
	pData->OwnerObject()->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
};

bool SW_Firewall::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	AresData::RespondToFirewall(pThis->Owner, true);

	if (IsPlayer) {
		pThis->Owner->RecheckTechTree = true;
	}

	return true;
}

void SW_Firewall::Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer)
{
	AresData::RespondToFirewall(pThis->Owner, false);

	if (isPlayer) {
		pThis->Owner->RecheckTechTree = true;
	}
}