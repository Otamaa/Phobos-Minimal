#include "Protect.h"

std::vector<const char*> SW_Protect::GetTypeString() const
{
	return { "Protect" };
}

bool SW_Protect::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::IronCurtain) || 
			(type == SuperWeaponType::ForceShield);
}

bool SW_Protect::CanFireAt(TargetingData const& data, const CellStruct& cell, bool manual) const
{
	return false;
}

bool SW_Protect::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return false;
}

void SW_Protect::Initialize(SWTypeExt::ExtData* pData)
{ }

void SW_Protect::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ }

AnimTypeClass* SW_Protect::GetAnim(const SWTypeExt::ExtData* pData) const
{
	if (pData->OwnerObject()->Type == SuperWeaponType::ForceShield)
	{
		return RulesClass::Instance->ForceShieldInvokeAnim;
	}
	else
	{
		return RulesClass::Instance->IronCurtainInvokeAnim;
	}
}

SWRange  SW_Protect::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (!pData->SW_Range->empty()) {
		return pData->SW_Range;
	}
	else if (pData->OwnerObject()->Type == SuperWeaponType::ForceShield) {
		return { RulesClass::Instance->ForceShieldRadius, -1 };
	}

	return { 3, 3 };
}