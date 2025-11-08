#include "Firewall.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/SWType/Body.h>

SuperWeaponType SW_Firewall::FirewallType = SuperWeaponType::Invalid;

SW_Firewall::~SW_Firewall() {
	SW_Firewall::FirewallType = SuperWeaponType::Invalid;
}

void SW_Firewall::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action::None;
	pData->This()->UseChargeDrain = true;
	pData->SW_RadarEvent = false;
};

void SW_Firewall::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	pData->This()->Action = Action::None;
	pData->This()->UseChargeDrain = true;
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