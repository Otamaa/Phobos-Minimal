#include "ChronoSphere.h"

bool SW_ChronoSphere::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ChronoSphere);
}

SuperWeaponFlags SW_ChronoSphere::Flags() const
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEVA | SuperWeaponFlags::NoMoney
		| SuperWeaponFlags::NoEvent | SuperWeaponFlags::NoCleanup | SuperWeaponFlags::NoMessage
		| SuperWeaponFlags::PreClick;
}

bool SW_ChronoSphere::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	return true;
}

void SW_ChronoSphere::Initialize(SWTypeExt::ExtData* pData)
{

}

void SW_ChronoSphere::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

AnimTypeClass* SW_ChronoSphere::GetAnim(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->ChronoPlacement;
}

SWRange SW_ChronoSphere::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange{3, 3} : pData->SW_Range;
}