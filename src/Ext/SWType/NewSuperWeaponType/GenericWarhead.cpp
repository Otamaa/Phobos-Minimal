#include "GenericWarhead.h"

#include <Ext/WarheadType/Body.h>
#include <Misc/AresData.h>

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

SuperWeaponFlags SW_GenericWarhead::Flags() const
{
	return SuperWeaponFlags::None;
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

	BuildingClass* pFirer = nullptr;
	for (auto const& pBld : pThis->Owner->Buildings)
	{
		if (this->IsLaunchSiteEligible(pData, Coords, pBld, false))
		{
			pFirer = pBld;
			break;
		}
	}

	auto pOwnerHouse = pThis->Owner;

	AresData::applyIonCannon(pWarhead, &coords);
	AresData::applyIC(pWarhead, &coords, pOwnerHouse, damage);
	AresData::applyEMP(pWarhead, &coords, pFirer);
	AresData::applyAE(pWarhead, &coords, pOwnerHouse);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (!pWHExt->applyPermaMC(pThis->Owner, pCell->GetContent()))
	{
		MapClass::DamageArea(coords, damage, pFirer, pWarhead, true, pThis->Owner);
		if (auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, coords)) {
			//Otamaa Added
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords))
				pAnim->Owner = pThis->Owner;
		}

		MapClass::FlashbangWarheadAt(damage, pWarhead, coords, false, SpotlightFlags::None);
	}

	return true;
}