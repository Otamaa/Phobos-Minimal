#include "Firewall.h"

const char* SW_Firewall::GetTypeString() const
{
	return "Firestorm";
}

void SW_Firewall::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	pData->OwnerObject()->Action = Action::None;
	pData->OwnerObject()->UseChargeDrain = true;
	//pData->SW_RadarEvent = false;
	// what can we possibly configure here... warhead/damage inflicted? anims?
};

bool SW_Firewall::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_Firewall::Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer)
{ 

}
