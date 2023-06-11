#include "ChronoWarp.h"

bool SW_ChronoWarp::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ChronoWarp);
}

SuperWeaponFlags SW_ChronoWarp::Flags() const
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEvent | SuperWeaponFlags::PostClick;
}

bool SW_ChronoWarp::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_ChronoWarp::Initialize(SWTypeExt::ExtData* pData)
{

}
