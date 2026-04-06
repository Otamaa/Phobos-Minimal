#include "FactoryPlantClass.h"

// Replace these includes with your actual paths
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <BuildingTypeClass.h>
#include <TechnoTypeClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>
#include <AircraftTypeClass.h>
#include <HouseClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

#include <algorithm>

// ========================================================================
// HasRestrictions - checks if the attached building has allow/disallow lists
// ========================================================================
bool FactoryPlantClass::HasRestrictions() const
{
	if (!this->AttachedToObject)
		return false;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

	return pTypeExt->FactoryPlant_AllowTypes.size() > 0
		|| pTypeExt->FactoryPlant_DisallowTypes.size() > 0;
}

// ========================================================================
// IsAffecting - checks if this plant's bonus applies to the given TechnoType
//
// Allow/Disallow logic:
//   - If AllowTypes is set and the type is NOT in the list → skip
//   - If DisallowTypes is set and the type IS in the list → skip
// ========================================================================
bool FactoryPlantClass::IsAffecting(TechnoTypeClass* pTechnoType) const
{
	if (!this->AttachedToObject || !pTechnoType)
		return false;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0
		&& !pTypeExt->FactoryPlant_AllowTypes.Contains(pTechnoType))
	{
		return false;
	}

	if (pTypeExt->FactoryPlant_DisallowTypes.size() > 0
		&& pTypeExt->FactoryPlant_DisallowTypes.Contains(pTechnoType))
	{
		return false;
	}

	return true;
}

// ========================================================================
// GetRawCostBonus - extracts the appropriate vanilla cost bonus field
//
// The vanilla BuildingType has separate bonus fields per category:
//   BuildingsCostBonus / DefensesCostBonus / InfantryCostBonus /
//   UnitsCostBonus / AircraftCostBonus
// ========================================================================
float FactoryPlantClass::GetRawCostBonus(TechnoTypeClass* pTechnoType) const
{
	if (!this->AttachedToObject)
		return 0.0f;

	const auto pBuildingType = this->AttachedToObject->Type;

	switch (pTechnoType->WhatAmI())
	{
	case AbstractType::BuildingType:
		if (static_cast<BuildingTypeClass*>(pTechnoType)->BuildCat == BuildCat::Combat)
			return pBuildingType->DefensesCostBonus;
		else
			return pBuildingType->BuildingsCostBonus;

	case AbstractType::AircraftType:
		return pBuildingType->AircraftCostBonus;

	case AbstractType::InfantryType:
		return pBuildingType->InfantryCostBonus;

	case AbstractType::UnitType:
		return pBuildingType->UnitsCostBonus;

	default:
		return 0.0f;
	}
}

// ========================================================================
// Register - adds this plant to the owner's restricted list
//
// Returns true if this is a restricted plant (has allow/disallow types),
// so the caller hook can skip vanilla FactoryPlant registration.
// ========================================================================
bool FactoryPlantClass::Register()
{
	if (this->Registered || !this->AttachedToObject)
		return false;

	if (!this->HasRestrictions())
		return false;

	const auto pOwnerExt = HouseExtContainer::Instance.Find(this->AttachedToObject->Owner);
	pOwnerExt->RestrictedFactoryPlants.emplace(this->AttachedToObject);
	this->Registered = true;

	return true;
}

// ========================================================================
// Unregister - removes this plant from the owner's restricted list
// ========================================================================
void FactoryPlantClass::Unregister()
{
	if (!this->Registered || !this->AttachedToObject)
		return;

	if (!this->AttachedToObject->Owner)
		return;

	const auto pOwnerExt = HouseExtContainer::Instance.Find(this->AttachedToObject->Owner);
	pOwnerExt->RestrictedFactoryPlants.erase(this->AttachedToObject);

	this->Registered = false;
}

// ========================================================================
// OnOwnerChanged - transfers registration from old house to new house
//
// Returns true if this is a restricted plant, so the caller can skip
// vanilla FactoryPlant ownership transfer.
// ========================================================================
void FactoryPlantClass::OnOwnerChanged(HouseClass* pOldOwner, HouseClass* pNewOwner)
{
	if (!this->AttachedToObject || !this->HasRestrictions())
		return;

	// Remove from old owner
	if (this->Registered && pOldOwner)
	{
		const auto pOldExt = HouseExtContainer::Instance.Find(pOldOwner);
		pOldExt->RestrictedFactoryPlants.erase(this->AttachedToObject);
	}

	// Add to new owner
	if (pNewOwner)
	{
		const auto pNewExt = HouseExtContainer::Instance.Find(pNewOwner);
		pNewExt->RestrictedFactoryPlants.emplace(this->AttachedToObject);
		this->Registered = true;
	}
	else
	{
		this->Registered = false;
	}
}

// ========================================================================
// GetCostMultiplier - static calculation of combined cost multiplier
//
// Iterates all restricted factory plants owned by the house.
// For each one that passes the allow/disallow filter:
//   1. Gets the raw cost bonus for the techno category
//   2. Applies the per-TechnoType FactoryPlant.Multiplier
//   3. Accumulates into the final multiplier
//
// The formula per plant:
//   currentMult = 1.0 - rawCostBonus
//   result *= (1.0 - currentMult * FactoryPlant_Multiplier)
//
// This matches the vanilla FactoryPlant cost reduction logic, just with
// the allow/disallow filtering and per-type multiplier layered on top.
// ========================================================================
float FactoryPlantClass::GetCostMultiplier(HouseClass* pHouse, TechnoTypeClass* pTechnoType)
{
	if (!pHouse || !pTechnoType)
		return 1.0f;

	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (pHouseExt->RestrictedFactoryPlants.empty())
		return 1.0f;

	float mult = 1.0f;
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	for (const auto pBuilding : pHouseExt->RestrictedFactoryPlants)
	{
		const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		// Allow/Disallow filter
		if (pTypeExt->FactoryPlant_AllowTypes.size() > 0
			&& !pTypeExt->FactoryPlant_AllowTypes.Contains(pTechnoType))
		{
			continue;
		}

		if (pTypeExt->FactoryPlant_DisallowTypes.size() > 0
			&& pTypeExt->FactoryPlant_DisallowTypes.Contains(pTechnoType))
		{
			continue;
		}

		// Calculate cost bonus for this category
		float currentMult = 1.0f;

		switch (pTechnoType->WhatAmI())
		{
		case AbstractType::BuildingType:
			if (static_cast<BuildingTypeClass*>(pTechnoType)->BuildCat == BuildCat::Combat)
				currentMult -= pBuilding->Type->DefensesCostBonus;
			else
				currentMult -= pBuilding->Type->BuildingsCostBonus;
			break;

		case AbstractType::AircraftType:
			currentMult -= pBuilding->Type->AircraftCostBonus;
			break;

		case AbstractType::InfantryType:
			currentMult -= pBuilding->Type->InfantryCostBonus;
			break;

		case AbstractType::UnitType:
			currentMult -= pBuilding->Type->UnitsCostBonus;
			break;

		default:
			break;
		}

		// Apply per-TechnoType multiplier (Ares feature)
		mult *= (1.0f - currentMult * pTechnoTypeExt->FactoryPlant_Multiplier);
	}

	return mult;
}