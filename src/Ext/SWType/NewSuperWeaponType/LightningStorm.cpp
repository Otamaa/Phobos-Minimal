#include "LightningStorm.h"

bool SW_LightningStorm::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::LightningStorm);
}

SuperWeaponFlags SW_LightningStorm::Flags() const
{
	return SuperWeaponFlags::NoMessage | SuperWeaponFlags::NoEvent;
}

bool SW_LightningStorm::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return false;
}

bool SW_LightningStorm::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	return false;
}

void SW_LightningStorm::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("LightningStorm[%s] init\n", pData->Get()->ID);
}

void SW_LightningStorm::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

WarheadTypeClass* SW_LightningStorm::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->LightningWarhead;
}

int SW_LightningStorm::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->LightningDamage);
}

SWRange SW_LightningStorm::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Range->empty())
	{
		return SWRange(RulesClass::Instance->LightningCellSpread);
	}
	return pData->SW_Range;
}