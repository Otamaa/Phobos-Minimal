#include "SpyPlane.h"

std::vector<const char*> SW_SpyPlane::GetTypeString() const
{
	return { "Airstrike" };
}

bool SW_SpyPlane::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::SpyPlane);
}

bool SW_SpyPlane::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_SpyPlane::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("SpyPlane[%s] Init !\n", pData->Get()->ID);
}

void SW_SpyPlane::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}