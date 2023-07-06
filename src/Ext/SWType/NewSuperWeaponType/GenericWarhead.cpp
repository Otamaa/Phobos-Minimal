#include "GenericWarhead.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Misc/AresData.h>

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData* pData)
{
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Offensive;
}

WarheadTypeClass* SW_GenericWarhead::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Warhead.isset())
		return pData->SW_Warhead;

	if (pData->Get()->WeaponType)
		return pData->Get()->WeaponType->Warhead;

	return nullptr;
}

int SW_GenericWarhead::GetDamage(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Damage.isset())
		return pData->SW_Damage;

	if (pData->Get()->WeaponType)
		return pData->Get()->WeaponType->Damage;

	return 0;
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	auto pWarhead = GetWarhead(pData);
	auto damage = GetDamage(pData);

	if (!pWarhead) {
		Debug::Log("Couldn't launch GenericWarhead SW ([%s])\n", pType->ID);
		return false;
	}

	auto const pCell = MapClass::Instance->GetCellAt(Coords);
	auto coords = pCell->GetCoordsWithBridge();

	auto pFirer = this->GetFirer(pThis,Coords, false);

	auto pOwnerHouse = pThis->Owner;
    WarheadTypeExt::CreateIonBlast(pWarhead, coords);
	AresData::applyEMP(pWarhead, &coords, pFirer);
	AresData::applyAE(pWarhead, &coords, pOwnerHouse);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (!pWHExt->applyPermaMC(pThis->Owner, pCell->GetContent()))
	{
		MapClass::DamageArea(coords, damage, pFirer, pWarhead, true, pThis->Owner);
		if (auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, coords)) {
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords))
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, pFirer, false);
		}

		MapClass::FlashbangWarheadAt(damage, pWarhead, coords, false, SpotlightFlags::None);
	}

	return true;
}

bool SW_GenericWarhead::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}
