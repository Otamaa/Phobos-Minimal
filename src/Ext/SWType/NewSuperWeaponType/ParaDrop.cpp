#include "ParaDrop.h"

bool SW_ParaDrop::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ParaDrop) || (type == SuperWeaponType::AmerParaDrop);
}

bool SW_ParaDrop::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	return true;
}

void SW_ParaDrop::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("ParaDrop[%s] init\n", pData->Get()->ID);
}

void SW_ParaDrop::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}
