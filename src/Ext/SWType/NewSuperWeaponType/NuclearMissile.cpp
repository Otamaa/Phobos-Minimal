#include "NuclearMissile.h"

bool SW_NuclearMissile::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::Nuke);
}

SuperWeaponFlags SW_NuclearMissile::Flags() const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_NuclearMissile::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	return true;
}

void SW_NuclearMissile::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("NuclearMissile[%s] Init !\n" , pData->Get()->ID);
}

void SW_NuclearMissile::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

bool SW_NuclearMissile::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return pBuilding->Type->NukeSilo && NewSWType::IsLaunchSite(pSWType, pBuilding);
}

WarheadTypeClass* SW_NuclearMissile::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	// is set to non-null?
	return WarheadTypeClass::Find("NUKE");
}

int SW_NuclearMissile::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->AtomDamage;
}
