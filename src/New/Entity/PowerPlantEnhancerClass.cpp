#include "PowerPlantEnhancerClass.h"

// Replace these includes with your actual paths
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <Unsorted.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Tactical/Body.h>

#include <algorithm>
#include <cmath>

void PowerPlantEnhancerClass::DrawIndicators(BuildingClass* pCurrentBuilding) {

	if(!RulesExtData::Instance()->ShowPowerPlantEnhancerRange.Get(Phobos::Config::ShowPowerPlantEnhancerRange))
		return;
	
	const auto pCurrentExt = HouseExtContainer::Instance.Find(HouseClass::CurrentPlayer());
	const auto center = DisplayClass::Instance->CurrentFoundation_CenterCell;

		for (const auto& pEnhancer : pCurrentExt->PowerPlantEnhancers)
		{
			if (!TechnoExtData::IsActive(pEnhancer) || pEnhancer->InLimbo || !pEnhancer->HasPower)
				continue;

			const auto pExt = BuildingExtContainer::Instance.Find(pEnhancer);

			if(pExt->LimboID >= 0)
				continue;

			const auto pEnhancerTypeExt = BuildingTypeExtContainer::Instance.Find(pEnhancer->Type);
			const int range = pEnhancerTypeExt->PowerPlantEnhancer_Range.Get() / Unsorted::LeptonsPerCell;

			if (range <= 0 || !pEnhancerTypeExt->PowerPlantEnhancer_Buildings.Contains(pCurrentBuilding->Type))
				continue;

			CoordStruct enhancerCoords = pEnhancer->GetCoords();

			if (center.DistanceFrom(CellClass::Coord2Cell(enhancerCoords)) > range * 1.5)
				continue;

			enhancerCoords.Z = MapClass::Instance->GetCellFloorHeight(enhancerCoords);
			const auto& color = pEnhancer->Owner->Color;
			FakeTacticalClass::__DrawRadialIndicator(false, true, enhancerCoords, color, range, false, true);
		}
}

// ========================================================================
// IsValidEnhancer - checks if the attached building qualifies as an enhancer
// ========================================================================
bool PowerPlantEnhancerClass::IsValidEnhancer() const
{
	if (!this->AttachedToObject)
		return false;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);

	return !pTypeExt->PowerPlantEnhancer_Buildings.empty()
		&& (pTypeExt->PowerPlantEnhancer_Amount != 0
			|| pTypeExt->PowerPlantEnhancer_Factor != 1.0f);
}

// ========================================================================
// Register - adds this enhancer to the owner's HouseExt tracking vector
// ========================================================================
void PowerPlantEnhancerClass::Register()
{
	if (this->Registered || !this->AttachedToObject)
		return;

	if (!this->IsValidEnhancer())
		return;

	const auto pOwnerExt = HouseExtContainer::Instance.Find(this->AttachedToObject->Owner);
	pOwnerExt->PowerPlantEnhancers.emplace(this->AttachedToObject);
	this->Registered = true;
}

// ========================================================================
// Unregister - removes this enhancer from the owner's HouseExt tracking vector
// ========================================================================
void PowerPlantEnhancerClass::Unregister()
{
	if (!this->Registered || !this->AttachedToObject)
		return;

	if (!this->AttachedToObject->Owner)
		return;

	const auto pOwnerExt = HouseExtContainer::Instance.Find(this->AttachedToObject->Owner);
	pOwnerExt->PowerPlantEnhancers.erase(this->AttachedToObject);

	this->Registered = false;
}

// ========================================================================
// OnOwnerChanged - transfers registration from old house to new house
// ========================================================================
void PowerPlantEnhancerClass::OnOwnerChanged(HouseClass* pOldOwner, HouseClass* pNewOwner)
{
	if (!this->AttachedToObject || !this->IsValidEnhancer())
		return;

	// Remove from old owner
	if (this->Registered && pOldOwner)
	{
		const auto pOldExt = HouseExtContainer::Instance.Find(pOldOwner);
		pOldExt->PowerPlantEnhancers.erase(this->AttachedToObject);
	}

	// Add to new owner
	if (pNewOwner)
	{
		const auto pNewExt = HouseExtContainer::Instance.Find(pNewOwner);
		pNewExt->PowerPlantEnhancers.emplace(this->AttachedToObject);
		this->Registered = true;
	}
	else
	{
		this->Registered = false;
	}
}

// ========================================================================
// GetEnhancedPower - static calculation of enhanced power output
//
// Iterates all enhancer buildings owned by the house, checks validity,
// building type match, range, and max count, then accumulates factor/amount.
// ========================================================================
std::pair<int, int> PowerPlantEnhancerClass::GetEnhancedPower(
	BuildingTypeClass* pBuilding,
	int output,
	HouseClass* pHouse,
	BuildingClass* pPowerPlant)
{
	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	int amount = 0;
	float factor = 1.0f;
	static std::unordered_map<int, int> applied; // typeArrayIndex -> applied count

	applied.clear();

	for (const auto pEnhancer : pHouseExt->PowerPlantEnhancers)
	{
		// Skip inactive/limbo/unpowered enhancers
		if (!TechnoExtData::IsActive(pEnhancer) || pEnhancer->InLimbo || !pEnhancer->HasPower)
			continue;

		const auto pEnhancerType = pEnhancer->Type;
		const auto pEnhancerTypeExt = BuildingTypeExtContainer::Instance.Find(pEnhancerType);

		// Check if this enhancer applies to the target building type
		if (!pEnhancerTypeExt->PowerPlantEnhancer_Buildings.Contains(pBuilding))
			continue;

		// Range check — skip if range is set and target is out of range
		const int range = pEnhancerTypeExt->PowerPlantEnhancer_Range.Get();

		if (range > 0 && (!pPowerPlant || pEnhancer->DistanceFrom(pPowerPlant) > range))
			continue;

		// MaxCount check — skip if this enhancer type has hit its cap
		const int max = pEnhancerTypeExt->PowerPlantEnhancer_MaxCount;

		auto& counter = applied[pEnhancerType->ArrayIndex];

		if (max > 0) {
			if (counter >= max)
				continue;
		}

		// Accumulate effects
		factor *= pEnhancerTypeExt->PowerPlantEnhancer_Factor;
		amount += pEnhancerTypeExt->PowerPlantEnhancer_Amount;
		++counter;
	}

	return std::make_pair(static_cast<int>(std::round(output * factor)), amount);
}

// ========================================================================
// GetRangeInCells - returns the enhancer range converted to cells
// ========================================================================
int PowerPlantEnhancerClass::GetRangeInCells() const
{
	if (!this->AttachedToObject)
		return 0;

	const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(this->AttachedToObject->Type);
	return pTypeExt->PowerPlantEnhancer_Range.Get() / Unsorted::LeptonsPerCell;
}