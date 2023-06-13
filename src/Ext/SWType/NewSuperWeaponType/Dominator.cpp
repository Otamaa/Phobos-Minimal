#include "Dominator.h"

bool SW_PsychicDominator::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicDominator);
}

SuperWeaponFlags SW_PsychicDominator::Flags() const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_PsychicDominator::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return false;
}

bool SW_PsychicDominator::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	return false;
}

void SW_PsychicDominator::Initialize(SWTypeExt::ExtData* pData)
{ }

void SW_PsychicDominator::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

WarheadTypeClass* SW_PsychicDominator::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->DominatorWarhead;
}

int SW_PsychicDominator::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->DominatorDamage);
}

SWRange SW_PsychicDominator::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Range->empty())
	{
		return SWRange(RulesClass::Instance->DominatorCaptureRange);
	}
	return pData->SW_Range;
}