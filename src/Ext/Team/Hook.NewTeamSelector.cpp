#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <AITriggerTypeClass.h>

// TODO :
// - Optimization a lot of duplicate code ,..
// - Type convert probably not handled properly yet
// - Prereq checking use vanilla function instead

enum class TeamCategory
{
	None = 0, // No category. Should be default value
	Ground = 1,
	Air = 2,
	Naval = 3,
	Unclassified = 4
};

// Constants for magic numbers
namespace TeamSelectorConstants
{
	constexpr double MIN_WEIGHT = 1.0;
	constexpr double VIP_TRIGGER_THRESHOLD = 5000.0;
	constexpr double VIP_TRIGGER_RESET_VALUE = 4999.0;
	constexpr int DEFENSE_TEAM_SELECTION_THRESHOLD = 50;
	constexpr int TASKFORCE_MAX_ENTRIES = 6;
	constexpr double MIN_PERCENTAGE = 0.0;
	constexpr double MAX_PERCENTAGE = 1.0;
	constexpr int PERCENTAGE_MULTIPLIER = 100;
	constexpr int MAX_TEAMS_PER_FRAME = 2; // Prevent too many teams created in one frame
	constexpr int CACHE_VALIDATION_INTERVAL = 5; // Validate cache every N triggers
}

// Helper function to validate and clamp percentages
FORCEDINLINE constexpr double ValidatePercentage(double percentage) noexcept
{
	return (percentage < TeamSelectorConstants::MIN_PERCENTAGE || percentage > TeamSelectorConstants::MAX_PERCENTAGE)
		? TeamSelectorConstants::MIN_PERCENTAGE
		: percentage;
}

// Helper function to check if a team type is already over-represented
FORCEDINLINE bool IsTeamTypeOverRepresented(TeamTypeClass* pTeamType, const HelperedVector<TeamClass*>& activeTeams) noexcept
{
	if (!pTeamType)
		return true;

	int activeCount = 0;
	int movingCount = 0;

	for (const auto& team : activeTeams)
	{
		if (team->Type == pTeamType)
		{
			activeCount++;
			if (team->IsMoving)
				movingCount++;
		}
	}

	// Consider over-represented if we have more than half the max teams of this type,
	// or if most teams are still moving (not deployed)
	return (activeCount > pTeamType->Max / 2) || (movingCount > activeCount / 2 && activeCount > 1);
}

// Pack struct for better cache performance
struct alignas(16) TriggerElementWeight
{
	AITriggerTypeClass* Trigger { nullptr };  // 8 bytes - most accessed
	double Weight { 0.0 };                    // 8 bytes
	TeamCategory Category { TeamCategory::None }; // 4 bytes
	// 4 bytes padding for alignment

	// Constructor to fix emplace_back compatibility
	TriggerElementWeight() = default;
	TriggerElementWeight(AITriggerTypeClass* trigger, double weight, TeamCategory category)
		: Trigger(trigger), Weight(weight), Category(category) { }
	TriggerElementWeight(double weight, AITriggerTypeClass* trigger, TeamCategory category)
		: Trigger(trigger), Weight(weight), Category(category) { }

	//need to define a == operator so it can be used in array classes
	FORCEDINLINE constexpr bool operator==(const TriggerElementWeight& other) const noexcept
	{
		return (Trigger == other.Trigger && Weight == other.Weight && Category == other.Category);
	}

	//unequality
	FORCEDINLINE constexpr bool operator!=(const TriggerElementWeight& other) const noexcept
	{
		return !(*this == other);
	}

	FORCEDINLINE constexpr bool operator<(const TriggerElementWeight& other) const noexcept
	{
		return Weight < other.Weight;
	}

	FORCEDINLINE constexpr bool operator<(double other) const noexcept
	{
		return Weight < other;
	}

	FORCEDINLINE constexpr bool operator>(const TriggerElementWeight& other) const noexcept
	{
		return Weight > other.Weight;
	}

	FORCEDINLINE constexpr bool operator>(double other) const noexcept
	{
		return Weight > other;
	}

	FORCEDINLINE constexpr bool operator==(double other) const noexcept
	{
		return Weight == other;
	}

	FORCEDINLINE constexpr bool operator!=(double other) const noexcept
	{
		return Weight != other;
	}
};

FORCEDINLINE bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed) noexcept
{
	// Fast path: null check first (most common failure case)
	if (!pTechno) [[unlikely]]
		return false;

		// Quick health check (second most common failure)
		if (pTechno->Health <= 0) [[unlikely]]
			return false;

			// Check basic availability (optimized order)
			bool isAvailable = pTechno->IsAlive && !pTechno->InLimbo && pTechno->IsOnMap;

			if (checkIfInTransportOrAbsorbed && isAvailable) [[likely]]
				isAvailable = !pTechno->Absorbed && !pTechno->Transporter;

				return isAvailable;
}

FORCEDINLINE bool IsValidTechno(TechnoClass* pTechno) noexcept
{
	// Fast null check
	if (!pTechno) [[unlikely]]
		return false;

		// Early rejection for common cases
		if (pTechno->Dirty || !pTechno->Owner) [[unlikely]]
			return false;

			// Cache the AbstractType check - it's more efficient to check once
			const auto technoType = pTechno->WhatAmI();

			// Use switch for better branch prediction (compiler optimization)
			switch (technoType)
			{
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Building:
			case AbstractType::Aircraft:
				return IsUnitAvailable(pTechno, true);
			default:
				return false;
			}
}

enum class ComparatorOperandTypes
{
	LessThan, LessOrEqual, Equal, MoreOrEqual, More, NotSame
};

// Helper function to check if a TechnoType is an air unit
FORCEDINLINE bool IsAirUnit(TechnoTypeClass* pTechnoType) noexcept
{
	if (!pTechnoType)
		return false;

	// Direct aircraft types
	if (pTechnoType->WhatAmI() == AbstractType::AircraftType || pTechnoType->ConsideredAircraft)
		return true;

	// Check for jumpjet vehicles
	if (pTechnoType->WhatAmI() == AbstractType::UnitType)
	{
		auto pUnitType = static_cast<UnitTypeClass*>(pTechnoType);
		if (pUnitType->JumpJet)
			return true;
	}

	// Check for hover vehicles and flying movement zones
	if (pTechnoType->MovementZone == MovementZone::Fly)
		return true;

	// Check for hover vehicles that can move over water/terrain
	if (pTechnoType->SpeedType == SpeedType::Hover)
		return true;

	// Check for balloon hover vehicles (specific to RA2/YR)
	if (pTechnoType->BalloonHover)
		return true;

	return false;
}

FORCEDINLINE void ModifyOperand(bool& result, int counter, const AITriggerConditionComparator& cond) noexcept
{
	const auto operandType = static_cast<ComparatorOperandTypes>(cond.ComparatorOperand);
	const int target = cond.ComparatorType;

	switch (operandType)
	{
	case ComparatorOperandTypes::LessThan:
		result = counter < target;
		break;
	case ComparatorOperandTypes::LessOrEqual:
		result = counter <= target;
		break;
	case ComparatorOperandTypes::Equal:
		result = counter == target;
		break;
	case ComparatorOperandTypes::MoreOrEqual:
		result = counter >= target;
		break;
	case ComparatorOperandTypes::More:
		result = counter > target;
		break;
	case ComparatorOperandTypes::NotSame:
		result = counter != target;
		break;
	default:
		result = false; // Explicit default value
		break;
	}
}

bool OwnStuffs(TechnoTypeClass* pItem, TechnoClass* list)
{
	if (auto pItemUnit = cast_to<UnitTypeClass*, false>(pItem))
	{
		if (auto pListBld = cast_to<BuildingClass*, false>(list))
		{
			if (pItemUnit->DeploysInto == pListBld->Type)
				return true;

			if (pListBld->Type->UndeploysInto == pItemUnit)
				return true;
		}
	}

	if (auto pItemBldType = cast_to<BuildingTypeClass*, false>(pItem))
	{
		if (auto pListUnit = cast_to<UnitClass*, false>(list))
		{
			if (pItemBldType->UndeploysInto == pListUnit->Type)
				return true;

			if (pListUnit->Type->DeploysInto == pItemBldType)
				return true;
		}
	}

	return TechnoExtContainer::Instance.Find(list)->Type == pItem || list->GetTechnoType() == pItem;
}

NOINLINE bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, std::vector<TechnoTypeClass*>& list, const std::vector<TechnoClass*>& validTechnos)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto pItem : list)
	{
		for (auto pObject : validTechnos)
		{
			if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& OwnStuffs(pItem, pObject))
			{
				counter++;
			}
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

// Overload for backward compatibility
NOINLINE bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, std::vector<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& OwnStuffs(pItem, pObject))
			{
				counter++;
			}
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, TechnoTypeClass* pItem)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator

	for (auto pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject)) continue;

		if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
			&& !pObject->Owner->Type->MultiplayPassive
			&& OwnStuffs(pItem, pObject))
		{
			counter++;
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, TechnoTypeClass* pItem)
{
	bool result = false;
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto const pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject)) continue;

		// Fixed logic: if pEnemy is set, check if object owner is pEnemy; otherwise check if not allied
		bool isValidEnemy = false;
		if (pEnemy)
		{
			isValidEnemy = (pObject->Owner == pEnemy);
		}
		else
		{
			isValidEnemy = (pObject->Owner != pHouse && !pHouse->IsAlliedWith(pObject->Owner));
		}

		if (isValidEnemy && !pObject->Owner->Type->MultiplayPassive && OwnStuffs(pItem, pObject))
		{
			counter++;
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, std::vector<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto const pItem : list)
	{
		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			// Fixed logic: if pEnemy is set, check if object owner is pEnemy; otherwise check if not allied
			bool isValidEnemy = false;
			if (pEnemy)
			{
				isValidEnemy = (pObject->Owner == pEnemy);
			}
			else
			{
				isValidEnemy = (pObject->Owner != pHouse && !pHouse->IsAlliedWith(pObject->Owner));
			}

			if (isValidEnemy && !pObject->Owner->Type->MultiplayPassive && OwnStuffs(pItem, pObject))
			{
				counter++;
			}
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool NeutralOwns(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;
	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	// Count all objects of the list, like an OR operator
	for (auto const pItem : list)
	{
		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			if (pObject->Owner == pCiv && OwnStuffs(pItem, pObject))
				counter++;
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool NeutralOwns(AITriggerTypeClass* pThis, TechnoTypeClass* pItem)
{
	bool result = false;
	int counter = 0;
	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	for (auto const pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject)) continue;

		if (pObject->Owner == pCiv && pObject->GetTechnoType() == pItem)
			counter++;
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			if (pObject->Owner == pHouse && pObject->GetTechnoType() == pItem)
				counter++;
		}

		ModifyOperand(result, counter, *pThis->Conditions);
	}

	return result;
}

NOINLINE bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			// Fixed logic: if pEnemy is set, check if object owner is pEnemy; otherwise check if not allied
			bool isValidEnemy = false;
			if (pEnemy)
			{
				isValidEnemy = (pObject->Owner == pEnemy);
			}
			else
			{
				isValidEnemy = (pObject->Owner != pHouse && !pHouse->IsAlliedWith(pObject->Owner));
			}

			if (isValidEnemy && !pObject->Owner->Type->MultiplayPassive && pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		ModifyOperand(result, counter, *pThis->Conditions);
	}

	return result;
}

NOINLINE bool NeutralOwnsAll(AITriggerTypeClass* pThis, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list)
	{
		int counter = 0;

		for (auto const pObject : *TechnoClass::Array)
		{
			if (!IsValidTechno(pObject)) continue;

			if (pObject->Owner == pCiv && pObject->GetTechnoType() == pItem)
				counter++;
		}

		ModifyOperand(result, counter, *pThis->Conditions);
	}

	return result;
}

NOINLINE bool CountConditionMet(AITriggerTypeClass* pThis, int nObjects)
{
	bool result = true;

	if (nObjects < 0)
		return false;

	ModifyOperand(result, nObjects, *pThis->Conditions);
	return result;
}



// Universal helper function to check if any unit is in attacking/threatening state
FORCEDINLINE bool IsUnitThreatening(TechnoClass* pTechno, HouseClass* pTargetHouse) noexcept
{
	if (!pTechno || !pTargetHouse)
		return false;

	// Check basic validity
	if (pTechno->Health <= 0 || !pTechno->IsAlive || pTechno->InLimbo)
		return false;

	// Check if unit is in attacking missions
	auto mission = pTechno->GetCurrentMission();
	if (mission == Mission::Attack || mission == Mission::Hunt || 
		mission == Mission::Area_Guard || mission == Mission::Patrol)
		return true;

	// Check if unit has a target that belongs to the target house
	if (auto pTarget = pTechno->Target)
	{
		// Check if target is a TechnoClass by checking AbstractClass type
		if (pTarget->WhatAmI() == AbstractType::Unit || 
			pTarget->WhatAmI() == AbstractType::Aircraft || 
			pTarget->WhatAmI() == AbstractType::Infantry ||
			pTarget->WhatAmI() == AbstractType::Building)
		{
			auto pTargetTechno = static_cast<TechnoClass*>(pTarget);
			if (pTargetTechno->Owner == pTargetHouse || pTargetHouse->IsAlliedWith(pTargetTechno->Owner))
				return true;
		}
	}

	// Check if unit is within threatening range of target house's units/buildings
	auto location = pTechno->GetCoords();
	int weaponRange = 0;
	
	// Get maximum weapon range
	if (auto pWeapon = pTechno->GetWeapon(0))
	{
		if (pWeapon->WeaponType)
			weaponRange = pWeapon->WeaponType->Range;
	}
	if (auto pWeapon = pTechno->GetWeapon(1))
	{
		if (pWeapon->WeaponType && pWeapon->WeaponType->Range > weaponRange)
			weaponRange = pWeapon->WeaponType->Range;
	}

	// Add mobility bonus based on unit type
	int mobilityBonus = 256; // Base mobility for infantry/buildings
	if (pTechno->WhatAmI() == AbstractType::Aircraft || IsAirUnit(pTechno->GetTechnoType()))
		mobilityBonus = 1024; // Air units get larger threat radius
	else if (pTechno->WhatAmI() == AbstractType::Unit)
		mobilityBonus = 512; // Vehicles get medium threat radius

	int totalThreatRange = weaponRange + mobilityBonus;

	// Check if within threatening distance of target house's assets
	for (auto const pTargetObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pTargetObject))
			continue;

		if (pTargetObject->Owner != pTargetHouse && !pTargetHouse->IsAlliedWith(pTargetObject->Owner))
			continue;

		auto targetLocation = pTargetObject->GetCoords();
		int distance = static_cast<int>(location.DistanceFrom(targetLocation));
		
		if (distance <= totalThreatRange)
			return true;
	}

	return false;
}

// Universal check if enemy has attacking/threatening units (any type)
// If pThis->ConditionObject is set = check specific unit type  
// If pThis->ConditionObject is null = check all unit types
// SUPPORTS: Air units, ground units, navy, V3 missiles, spawned units (all TechnoClass)
NOINLINE bool EnemyHasAttackingUnits(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy)
{
	bool result = false;
	int counter = 0;
	TechnoTypeClass* pSpecificUnitType = pThis->ConditionObject;

	// Count threatening units owned by enemies
	for (auto const pObject : *TechnoClass::Array)
	{
		if (!IsValidTechno(pObject))
			continue;

		// Check if this unit belongs to an enemy
		bool isValidEnemy = false;
		if (pEnemy)
		{
			isValidEnemy = (pObject->Owner == pEnemy);
		}
		else
		{
			isValidEnemy = (pObject->Owner != pHouse && !pHouse->IsAlliedWith(pObject->Owner));
		}

		if (!isValidEnemy || pObject->Owner->Type->MultiplayPassive)
			continue;

		// Check if unit is threatening
		if (!IsUnitThreatening(pObject, pHouse))
			continue;

		// If specific unit type is requested, check for it
		if (pSpecificUnitType)
		{
			if (OwnStuffs(pSpecificUnitType, pObject))
				counter++;
		}
		else
		{
			// Count all threatening units (air, ground, navy, missiles, etc.)
			counter++;
		}
	}

	// Always use proper comparator - requires valid ComparatorOperand and ComparatorType
	ModifyOperand(result, counter, *pThis->Conditions);
	
	return result;
}

// Structure to cache techno data for performance
struct CachedTechnoData
{
	std::vector<TechnoClass*> validTechnos;
	int destroyedBridgesCount = 0;
	int undamagedBridgesCount = 0;

	// Cache recruitable data per house to avoid repeated calculations
	mutable PhobosMap<HouseClass*, PhobosMap<TechnoTypeClass*, int>> recruitableCache;
	mutable int lastValidationFrame = -1; // Track when cache was last validated

	void Initialize()
	{
		validTechnos.clear();
		destroyedBridgesCount = 0;
		undamagedBridgesCount = 0;
		recruitableCache.clear();
		lastValidationFrame = -1;

		// Single pass through all technos to cache data
		for (auto const pTechno : *TechnoClass::Array)
		{
			if (!IsValidTechno(pTechno))
				continue;

			validTechnos.push_back(pTechno);

			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass*>(pTechno);
				if (pBuilding && pBuilding->Type->BridgeRepairHut)
				{
					CellStruct cell = pTechno->GetCell()->MapCoords;
					if (MapClass::Instance->IsLinkedBridgeDestroyed(cell))
						destroyedBridgesCount++;
					else
						undamagedBridgesCount++;
				}
			}
		}
	}

	// Validate that cached technos are still valid (with throttling)
	void ValidateCache()
	{
		int currentFrame = Unsorted::CurrentFrame;

		// Don't validate too frequently (max once per 5 frames)
		if (currentFrame - lastValidationFrame < 5)
			return;

		lastValidationFrame = currentFrame;

		// Clean up defeated houses from cache to prevent memory leak
		std::vector<HouseClass*> housesToRemove;
		for (auto iter = recruitableCache.begin(); iter != recruitableCache.end(); ++iter)
		{
			auto pHouse = iter->first;
			if (!pHouse || pHouse->Defeated)
			{
				housesToRemove.push_back(pHouse);
			}
		}

		for (auto pHouse : housesToRemove)
		{
			recruitableCache.erase(pHouse);
		}

		// Only remove obviously dead technos to avoid expensive iteration
		validTechnos.erase(
			std::remove_if(validTechnos.begin(), validTechnos.end(),
				[](TechnoClass* pTechno)
				{
					return !pTechno || pTechno->Health <= 0 || !pTechno->IsAlive || pTechno->IsCrashing || pTechno->IsSinking;
				}),
			validTechnos.end()
		);

		// Invalidate all house caches as unit status might have changed
		recruitableCache.clear();
	}

	// Get recruitable units for a specific house (with proper caching)
	const PhobosMap<TechnoTypeClass*, int>& GetOwnedRecruitables(HouseClass* pHouse)
	{
		// Safety check for null/invalid house
		if (!pHouse || pHouse->Defeated)
		{
			static PhobosMap<TechnoTypeClass*, int> emptyMap;
			return emptyMap;
		}

		// Check if we already have FRESH data for this house
		auto houseIter = recruitableCache.get_key_iterator(pHouse);
		if (houseIter != recruitableCache.end())
		{
			return houseIter->second;
		}

		// Calculate fresh data for this house
		PhobosMap<TechnoTypeClass*, int> ownedRecruitables;

		for (auto const pTechno : validTechnos)
		{
			// Skip buildings for recruitment
			if (pTechno->WhatAmI() == AbstractType::Building)
				continue;

			auto const pFoot = static_cast<FootClass*>(pTechno);

			// Quick validity check first
			if (pTechno->IsSinking || pTechno->IsCrashing || !pTechno->IsAlive
				|| pTechno->Health <= 0 || !pTechno->IsOnMap
				|| pTechno->Transporter || pTechno->Absorbed)
				continue;

			// Check if already assigned to a team (prevent race conditions)
			if (pFoot->BelongsToATeam())
				continue;

			bool allow = true;
			if (auto pContact = pFoot->GetRadioContact())
			{
				if (auto pBldC = cast_to<BuildingClass*, false>(pContact))
				{
					if (pBldC->Type->Bunker)
						allow = false;
				}
			}
			else if (auto pBld = pFoot->GetCell()->GetBuilding())
			{
				if (pBld->Type->Bunker)
					allow = false;
			}

			if (allow && pFoot->CanBeRecruited(pHouse))
			{
				++ownedRecruitables[pTechno->GetTechnoType()];
			}
		}

		// Cache the result
		recruitableCache[pHouse] = ownedRecruitables;
		return recruitableCache[pHouse];
	}

	// Reserve units to prevent race conditions (with better error handling)
	bool TryReserveUnits(HouseClass* pHouse, const std::vector<std::pair<TechnoTypeClass*, int>>& requirements)
	{
		// Safety checks
		if (!pHouse || pHouse->Defeated || requirements.empty())
			return false;

		auto& recruitables = const_cast<PhobosMap<TechnoTypeClass*, int>&>(GetOwnedRecruitables(pHouse));

		// First check if we have enough units
		for (const auto& req : requirements)
		{
			if (!req.first || req.second <= 0)
				continue;

			auto iter = recruitables.get_key_iterator(req.first);
			if (iter == recruitables.end() || iter->second < req.second)
			{
				return false; // Not enough units available
			}
		}

		// Reserve the units by reducing the count (atomic operation)
		for (const auto& req : requirements)
		{
			if (!req.first || req.second <= 0)
				continue;

			auto iter = recruitables.get_key_iterator(req.first);
			if (iter != recruitables.end())
			{
				iter->second = std::max(0, iter->second - req.second);
			}
		}

		return true;
	}
};

// Global cache for better performance - shared between calls
static CachedTechnoData g_cachedData;
static int g_lastCacheFrame = -1;
static constexpr int CACHE_REFRESH_INTERVAL = 15; // Refresh every 15 frames

NOINLINE bool UpdateTeam(HouseClass* pHouse)
{
	if (!RulesExtData::Instance()->NewTeamsSelector)
		return false;

	auto pHouseTypeExt = HouseTypeExtContainer::Instance.Find(pHouse->Type);
	// Reset Team selection countdown
	pHouse->TeamDelayTimer.Start(RulesClass::Instance->TeamDelays[(int)pHouse->AIDifficulty]);

	HelperedVector<TriggerElementWeight> validTriggerCandidates;
	HelperedVector<TriggerElementWeight> validTriggerCandidatesGroundOnly;
	HelperedVector<TriggerElementWeight> validTriggerCandidatesNavalOnly;
	HelperedVector<TriggerElementWeight> validTriggerCandidatesAirOnly;
	HelperedVector<TriggerElementWeight> validTriggerCandidatesUnclassifiedOnly;

	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	// This house must have the triggers enabled
	if (dice <= pHouse->RatioAITriggerTeam && pHouse->AITriggersActive)
	{
		int mergeUnclassifiedCategoryWith = -1;
		TeamCategory validCategory = TeamCategory::None;
		bool splitTriggersByCategory = RulesExtData::Instance()->NewTeamsSelector_SplitTriggersByCategory;
		bool isFallbackEnabled = RulesExtData::Instance()->NewTeamsSelector_EnableFallback;

		if (splitTriggersByCategory)
		{
			mergeUnclassifiedCategoryWith = pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.Get(RulesExtData::Instance()->NewTeamsSelector_MergeUnclassifiedCategoryWith);  // Should mixed teams be merged into another category?
			double percentageUnclassifiedTriggers = pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.Get(RulesExtData::Instance()->NewTeamsSelector_UnclassifiedCategoryPercentage); // Mixed teams
			double percentageGroundTriggers = pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.Get(RulesExtData::Instance()->NewTeamsSelector_GroundCategoryPercentage); // Only ground
			double percentageNavalTriggers = pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.Get(RulesExtData::Instance()->NewTeamsSelector_NavalCategoryPercentage); // Only Naval=yes
			double percentageAirTriggers = pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.Get(RulesExtData::Instance()->NewTeamsSelector_AirCategoryPercentage); // Only Aircrafts & jumpjets

			// Merge mixed category with another category, if set
			if (mergeUnclassifiedCategoryWith >= 0)
			{
				switch (mergeUnclassifiedCategoryWith)
				{
				case (int)TeamCategory::Ground:
					percentageGroundTriggers += percentageUnclassifiedTriggers;
					break;

				case (int)TeamCategory::Air:
					percentageAirTriggers += percentageUnclassifiedTriggers;
					break;

				case (int)TeamCategory::Naval:
					percentageNavalTriggers += percentageUnclassifiedTriggers;
					break;

				default:
					break;
				}

				percentageUnclassifiedTriggers = 0.0;
			}

			percentageUnclassifiedTriggers = ValidatePercentage(percentageUnclassifiedTriggers);
			percentageGroundTriggers = ValidatePercentage(percentageGroundTriggers);
			percentageNavalTriggers = ValidatePercentage(percentageNavalTriggers);
			percentageAirTriggers = ValidatePercentage(percentageAirTriggers);

			double totalPercentages = percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers;
			if (totalPercentages > TeamSelectorConstants::MAX_PERCENTAGE || totalPercentages <= TeamSelectorConstants::MIN_PERCENTAGE)
				splitTriggersByCategory = false;

			if (splitTriggersByCategory)
			{
				int categoryDice = ScenarioClass::Instance->Random.RandomRanged(1, TeamSelectorConstants::PERCENTAGE_MULTIPLIER);
				int unclassifiedValue = (int)(percentageUnclassifiedTriggers * TeamSelectorConstants::PERCENTAGE_MULTIPLIER);
				int groundValue = (int)(percentageGroundTriggers * TeamSelectorConstants::PERCENTAGE_MULTIPLIER);
				int airValue = (int)(percentageAirTriggers * TeamSelectorConstants::PERCENTAGE_MULTIPLIER);
				int navalValue = (int)(percentageNavalTriggers * TeamSelectorConstants::PERCENTAGE_MULTIPLIER);

				// Pick what type of team will be selected in this round
				if (percentageUnclassifiedTriggers > 0.0 && categoryDice <= unclassifiedValue)
				{
					validCategory = TeamCategory::Unclassified;
				}
				else if (percentageGroundTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue))
				{
					validCategory = TeamCategory::Ground;
				}
				else if (percentageAirTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue))
				{
					validCategory = TeamCategory::Air;
				}
				else if (percentageNavalTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue + navalValue))
				{
					validCategory = TeamCategory::Naval;
				}
				else
				{
					// If the sum of all percentages is less than 100% then that empty space will work like "no categories"
					splitTriggersByCategory = false;
				}
			}
		}

		int parentCountryTypeIdx = pHouse->Type->FindParentCountryIndex(); // ParentCountry can change the House in a SP map
		int houseTypeIdx = parentCountryTypeIdx >= 0 ? parentCountryTypeIdx : pHouse->Type->ArrayIndex; // Indexes in AITriggers section are 1-based
		int houseIdx = pHouse->ArrayIndex;

		int parentCountrySideTypeIdx = pHouse->Type->FindParentCountry()->SideIndex;
		int sideTypeIdx = parentCountrySideTypeIdx >= 0 ? parentCountrySideTypeIdx + 1 : pHouse->Type->SideIndex + 1; // Side indexes in AITriggers section are 1-based
		//int sideIdx = pHouse->SideIndex + 1; // Side indexes in AITriggers section are 1-based

		auto houseDifficulty = pHouse->AIDifficulty;
		int minBaseDefenseTeams = RulesClass::Instance->MinimumAIDefensiveTeams[(int)houseDifficulty];
		int maxBaseDefenseTeams = RulesClass::Instance->MaximumAIDefensiveTeams[(int)houseDifficulty];
		int activeDefenseTeamsCount = 0;
		int maxTeamsLimit = RulesClass::Instance->TotalAITeamCap[(int)houseDifficulty];
		double totalWeight = 0.0;
		double totalWeightGroundOnly = 0.0;
		double totalWeightNavalOnly = 0.0;
		double totalWeightAirOnly = 0.0;
		double totalWeightUnclassifiedOnly = 0.0;

		// Check if the running teams by the house already reached all the limits
		HelperedVector<TeamClass*> activeTeamsList;

		for (auto const pRunningTeam : *TeamClass::Array)
		{
			if (!pRunningTeam || !pRunningTeam->Owner)
				continue;

			int teamHouseIdx = pRunningTeam->Owner->ArrayIndex;

			if (teamHouseIdx != houseIdx)
				continue;

			activeTeamsList.push_back(pRunningTeam);

			if (pRunningTeam->Type->IsBaseDefense)
				activeDefenseTeamsCount++;
		}

		// We will use these values for discarding triggers
		int defensiveTeamsLimit = RulesClass::Instance->UseMinDefenseRule ? minBaseDefenseTeams : maxBaseDefenseTeams;
		bool hasReachedMaxTeamsLimit = (int)activeTeamsList.size() < maxTeamsLimit ? false : true;
		bool hasReachedMaxDefensiveTeamsLimit = activeDefenseTeamsCount < defensiveTeamsLimit ? false : true;

		/*Debug::LogInfo("=====================[{}] ACTIVE TEAMS: {} / {} (of them, defensive teams: {} / {})", pHouse->Type->ID, activeTeams, maxTeamsLimit, activeDefenseTeamsCount, defensiveTeamsLimit);
		for (auto team : activeTeamsList)
		{
			Debug::LogInfo("[{}]({}) : {}{}", team->Type->ID, team->TotalObjects, team->Type->Name, team->Type->IsBaseDefense ? " -> is DEFENDER team" : "");
			Debug::LogInfo("    IsMoving: {}, IsFullStrength: {}, IsUnderStrength: {}", team->IsMoving, team->IsFullStrength, team->IsUnderStrength);
			int i = 0;

			for (auto entry : team->Type->TaskForce->Entries)
			{
				if (entry.Type && entry.Amount > 0)
				{
					if (entry.Type)
						Debug::LogInfo("\t[{}]: {} / {}", entry.Type->ID, team->CountObjects[i], entry.Amount);
				}

				i++;
			}
		}
		Debug::LogInfo("=====================");*/

		// Check if the next team must be a defensive team
		bool onlyPickDefensiveTeams = false;
		int defensiveDice = ScenarioClass::Instance->Random.RandomRanged(0, 99);

		if ((defensiveDice < TeamSelectorConstants::DEFENSE_TEAM_SELECTION_THRESHOLD) && !hasReachedMaxDefensiveTeamsLimit)
			onlyPickDefensiveTeams = true;

		// Early return for performance - avoid expensive trigger processing
		if (hasReachedMaxTeamsLimit)
		{
			return true;
		}

		// Use global cache for better performance
		int currentFrame = Unsorted::CurrentFrame;

		// Refresh cache periodically or if this is the first time
		if (g_lastCacheFrame == -1 || (currentFrame - g_lastCacheFrame) >= CACHE_REFRESH_INTERVAL)
		{
			g_cachedData.Initialize();
			g_lastCacheFrame = currentFrame;
		}

		// Validate cache to remove dead units (throttled internally)
		g_cachedData.ValidateCache();

		HouseClass* targetHouse = nullptr;
		if (pHouse->EnemyHouseIndex >= 0)
			targetHouse = HouseClass::Array->GetItem(pHouse->EnemyHouseIndex);

		bool onlyCheckImportantTriggers = false;
		int triggersProcessed = 0;
		int teamsCreatedThisFrame = 0;

		// Gather all the trigger candidates into one place for posterior fast calculations
		for (auto const pTrigger : *AITriggerTypeClass::Array)
		{
			if (!pTrigger || ScenarioClass::Instance->IgnoreGlobalAITriggers == (bool)pTrigger->IsGlobal || !pTrigger->Team1)
				continue;

			// Throttling: Don't process too many triggers per frame
			if (++triggersProcessed % (TeamSelectorConstants::CACHE_VALIDATION_INTERVAL * 2) == 0)
			{
				g_cachedData.ValidateCache(); // Less frequent validation to improve performance
			}

			// Ignore offensive teams if the next trigger must be defensive
			if (onlyPickDefensiveTeams && !pTrigger->IsForBaseDefense)
				continue;

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// Ignore the deactivated triggers
			if (pTrigger->IsEnabled)
			{
				//pTrigger->OwnerHouseType;
				if (pTrigger->Team1->TechLevel > pHouse->StaticData.TechLevel)
					continue;

				// ignore it if isn't set for the house AI difficulty
				if ((int)houseDifficulty == 0 && !pTrigger->Enabled_Hard
					|| (int)houseDifficulty == 1 && !pTrigger->Enabled_Normal
					|| (int)houseDifficulty == 2 && !pTrigger->Enabled_Easy)
				{
					continue;
				}

				// The trigger must be compatible with the owner
				if ((triggerHouse == -1 || houseTypeIdx == triggerHouse) && (triggerSide == 0 || sideTypeIdx == triggerSide))
				{
					// "ConditionType=-1" will be skipped, always is valid
					if ((int)pTrigger->ConditionType >= 0)
					{
						switch ((int)pTrigger->ConditionType)
						{
						case 0:
						{
							// Simulate case 0: "enemy owns"
							if (!pTrigger->ConditionObject)
								continue;

							if (!EnemyOwns(pTrigger, pHouse, targetHouse, true, pTrigger->ConditionObject))
								continue;
						}
						break;
						case 1:
						{
							// Simulate case 1: "house owns"
							if (!pTrigger->ConditionObject)
								continue;

							if (!HouseOwns(pTrigger, pHouse, false, pTrigger->ConditionObject))
								continue;
						}	break;
						case 7:
						{
							// Simulate case 7: "civilian owns"
							if (!pTrigger->ConditionObject)
								continue;

							if (!NeutralOwns(pTrigger, pTrigger->ConditionObject))
								continue;
						}	break;
						case 8:
						{
							// Simulate case 0: "enemy owns" but instead of restrict it against the main enemy house it is done against all enemies
							if (!pTrigger->ConditionObject)
								continue;

							if (!EnemyOwns(pTrigger, pHouse, nullptr, false, pTrigger->ConditionObject))
								continue;
						}	break;
						case 9:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 9: Like in case 0 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the enemy.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwns(pTrigger, pHouse, targetHouse, false, RulesExtData::Instance()->AITargetTypesLists[pTrigger->Conditions[3].ComparatorOperand]))
									continue;
							}
						}	break;
						case 10:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 10: Like in case 1 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the house.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwns(pTrigger, pHouse, false, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 11:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 11: Like in case 7 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the Civilians.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!NeutralOwns(pTrigger, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 12:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 12: Like in case 0 & 9 but instead of a specific enemy this checks in all enemies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwns(pTrigger, pHouse, nullptr, false, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 13:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 13: Like in case 1 & 10 but instead checking the house now checks the allies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwns(pTrigger, pHouse, true, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 14:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 14: Like in case 9 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwnsAll(pTrigger, pHouse, targetHouse, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 15:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 15: Like in case 10 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwnsAll(pTrigger, pHouse, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 16:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 16: Like in case 11 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!NeutralOwnsAll(pTrigger, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 17:
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 17: Like in case 14 but instead of meet any comparison now is required all. Check all enemies
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwnsAll(pTrigger, pHouse, nullptr, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}	break;
						case 18:
						{
							// New case 18: Check destroyed bridges
							if (!CountConditionMet(pTrigger, g_cachedData.destroyedBridgesCount))
								continue;
						}	break;
						case 19:
						{
							// New case 19: Check undamaged bridges
							if (!CountConditionMet(pTrigger, g_cachedData.undamagedBridgesCount))
								continue;
						}	break;
						case 21:
						{
							// New case 21: Enemy has attacking/threatening units (universal condition)
							// Requires proper ComparatorOperand and ComparatorType to be set
							// If ConditionObject is set = check specific unit type
							// If ConditionObject is null = check all unit types  
							if (!EnemyHasAttackingUnits(pTrigger, pHouse, targetHouse))
								continue;
						}	break;
						default:
						{
							// Other cases from vanilla game
							if (!pTrigger->ConditionMet(pHouse, targetHouse, hasReachedMaxDefensiveTeamsLimit))
								continue;
						}	break;
						}
					}

					// All triggers below VIP threshold in current weight will get discarded if this mode is enabled
					if (onlyCheckImportantTriggers)
					{
						if (pTrigger->Weight_Current < TeamSelectorConstants::VIP_TRIGGER_THRESHOLD)
							continue;
					}

					auto pTriggerTeam1Type = pTrigger->Team1;
					if (!pTriggerTeam1Type)
						continue;

					// No more defensive teams needed
					if (pTriggerTeam1Type->IsBaseDefense && hasReachedMaxDefensiveTeamsLimit)
						continue;

					// If this type of Team reached the max or is over-represented then skip it
					int count = 0;

					for (auto team : activeTeamsList)
					{
						if (team->Type == pTriggerTeam1Type)
							count++;
					}

					if (count >= pTriggerTeam1Type->Max)
						continue;

					// Additional check to prevent over-creation of certain team types
					if (IsTeamTypeOverRepresented(pTriggerTeam1Type, activeTeamsList))
						continue;

					TeamCategory teamIsCategory = TeamCategory::None;

					// Analyze what kind of category is this main team if the feature is enabled
					if (splitTriggersByCategory)
					{
						//Debug::LogInfo("DEBUG: TaskForce [{}] members:", pTriggerTeam1Type->TaskForce->ID);
						// TaskForces are limited to specific number of entries
						for (int i = 0; i < TeamSelectorConstants::TASKFORCE_MAX_ENTRIES; i++)
						{
							auto entry = pTriggerTeam1Type->TaskForce->Entries[i];
							TeamCategory entryIsCategory = TeamCategory::Ground;

							if (entry.Amount > 0)
							{
								if (!entry.Type)
									continue;

								if (entry.Type->WhatAmI() == AbstractType::AircraftType
									|| entry.Type->ConsideredAircraft)
								{
									// This unit is from air category
									entryIsCategory = TeamCategory::Air;
									//Debug::LogInfo("\t[{}]({}) is in AIR category.", entry.Type->ID, entry.Amount);
								}
								else
								{
									auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(entry.Type);

									if (pTechnoTypeExt->ConsideredNaval
										|| (entry.Type->Naval
											&& (entry.Type->MovementZone != MovementZone::Amphibious
												&& entry.Type->MovementZone != MovementZone::AmphibiousDestroyer
												&& entry.Type->MovementZone != MovementZone::AmphibiousCrusher)))
									{
										// This unit is from naval category
										entryIsCategory = TeamCategory::Naval;
										//Debug::LogInfo("\t[{}]({}) is in NAVAL category.", entry.Type->ID, entry.Amount);
									}

									if (pTechnoTypeExt->ConsideredVehicle
										|| (entryIsCategory != TeamCategory::Naval
											&& entryIsCategory != TeamCategory::Air))
									{
										// This unit is from ground category
										entryIsCategory = TeamCategory::Ground;
										//Debug::LogInfo("\t[{}]({}) is in GROUND category.", entry.Type->ID, entry.Amount);
									}
								}

								// if a team have multiple categories it will be a mixed category
								teamIsCategory = teamIsCategory == TeamCategory::None || teamIsCategory == entryIsCategory ? entryIsCategory : TeamCategory::Unclassified;

								if (teamIsCategory == TeamCategory::Unclassified)
									break;
							}
							else
							{
								break;
							}
						}

						//Debug::LogInfo("DEBUG: This team is a category {} (1:Ground, 2:Air, 3:Naval, 4:Mixed).", teamIsCategory);
						// Si existe este valor y el team es MIXTO se sobreescribe el tipo de categoría
						if (teamIsCategory == TeamCategory::Unclassified
							&& mergeUnclassifiedCategoryWith >= 0)
						{
							//Debug::LogInfo("DEBUG: MIXED category forced to work as category {}.", mergeUnclassifiedCategoryWith);
							teamIsCategory = (TeamCategory)mergeUnclassifiedCategoryWith;
						}
					}

					bool allObjectsCanBeBuiltOrRecruited = true;

					if (pTriggerTeam1Type->Autocreate)
					{
						for (const auto& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce meets the structure prerequisites
							if (entry.Amount > 0)
							{
								if (!entry.Type)
									continue;

								bool canBeBuilt = HouseExtData::PrerequisitesMet(pHouse, entry.Type);

								if (!canBeBuilt)
								{
									allObjectsCanBeBuiltOrRecruited = false;
									break;
								}
							}
							else
							{
								break;
							}
						}
					}
					else
					{
						allObjectsCanBeBuiltOrRecruited = false;
					}

					if (!allObjectsCanBeBuiltOrRecruited && pTriggerTeam1Type->Recruiter)
					{
						allObjectsCanBeBuiltOrRecruited = true;

						// Prepare requirements list efficiently
						std::vector<std::pair<TechnoTypeClass*, int>> requirements;
						requirements.reserve(TeamSelectorConstants::TASKFORCE_MAX_ENTRIES); // Pre-allocate

						for (const auto& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							if (entry.Type && entry.Amount > 0)
							{
								requirements.emplace_back(entry.Type, entry.Amount);
							}
							else if (entry.Amount <= 0)
							{
								break; // Stop at first invalid entry
							}
						}

						// Try to reserve units atomically to prevent race conditions
						if (!requirements.empty())
						{
							allObjectsCanBeBuiltOrRecruited = g_cachedData.TryReserveUnits(pHouse, requirements);
						}
						else
						{
							allObjectsCanBeBuiltOrRecruited = false; // No valid requirements
						}
					}

					// We can't let AI cheat in this trigger because doesn't have the required tech tree available
					if (!allObjectsCanBeBuiltOrRecruited)
						continue;

					// Special case: triggers become very important if they reach the max priority threshold.
					// They get stored in a elitist list and all previous triggers are discarded
					if (pTrigger->Weight_Current >= TeamSelectorConstants::VIP_TRIGGER_THRESHOLD && !onlyCheckImportantTriggers)
					{
						// First time only
						if (validTriggerCandidates.size() > 0)
						{
							validTriggerCandidates.clear();
							validTriggerCandidatesGroundOnly.clear();
							validTriggerCandidatesNavalOnly.clear();
							validTriggerCandidatesAirOnly.clear();
							validTriggerCandidatesUnclassifiedOnly.clear();
							validCategory = TeamCategory::None;
						}

						// Reset the current ones and now only will be added important triggers to the list
						onlyCheckImportantTriggers = true;
						totalWeight = 0.0;
						splitTriggersByCategory = false; // VIP teams breaks the categories logic (on purpose)
					}

					// Passed all checks, save this trigger for later.
					// The idea behind this is to simulate an ordered list of weights and once we throw the dice we'll know the winner trigger: More weight means more possibilities to be selected.
					double effectiveWeight = pTrigger->Weight_Current < TeamSelectorConstants::MIN_WEIGHT ? TeamSelectorConstants::MIN_WEIGHT : pTrigger->Weight_Current;
					totalWeight += effectiveWeight;
					validTriggerCandidates.emplace_back(totalWeight, pTrigger, teamIsCategory);

					if (splitTriggersByCategory)
					{
						switch (teamIsCategory)
						{
						case TeamCategory::Ground:
							totalWeightGroundOnly += effectiveWeight;
							validTriggerCandidatesGroundOnly.emplace_back(totalWeightGroundOnly, pTrigger, teamIsCategory);
							break;

						case TeamCategory::Air:
							totalWeightAirOnly += effectiveWeight;
							validTriggerCandidatesAirOnly.emplace_back(totalWeightAirOnly, pTrigger, teamIsCategory);
							break;

						case TeamCategory::Naval:
							totalWeightNavalOnly += effectiveWeight;
							validTriggerCandidatesNavalOnly.emplace_back(totalWeightNavalOnly, pTrigger, teamIsCategory);
							break;

						case TeamCategory::Unclassified:
							totalWeightUnclassifiedOnly += effectiveWeight;
							validTriggerCandidatesUnclassifiedOnly.emplace_back(totalWeightUnclassifiedOnly, pTrigger, teamIsCategory);
							break;

						default:
							break;
						}
					}
				}
			}
		}

		// Category selection debug logging (disabled for performance)

		if (validTriggerCandidates.empty())
		{
			return true; // No valid triggers - early return for performance
		}

		if ((validCategory == TeamCategory::Ground && validTriggerCandidatesGroundOnly.empty())
			|| (validCategory == TeamCategory::Unclassified && validTriggerCandidatesUnclassifiedOnly.empty())
			|| (validCategory == TeamCategory::Air && validTriggerCandidatesAirOnly.empty())
			|| (validCategory == TeamCategory::Naval && validTriggerCandidatesNavalOnly.empty()))
		{
			if (!isFallbackEnabled)
				return true;

			// Fallback to all categories
			validCategory = TeamCategory::None;
		}

		AITriggerTypeClass* selectedTrigger = nullptr;
		double weightDice = 0.0;
		double lastWeight = 0.0;
		bool found = false;

		switch (validCategory)
		{
		case TeamCategory::None:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeight) * 1.0;
			/*Debug::LogInfo("Weight Dice: {}", weightDice);

			// Debug
			Debug::LogInfo("DEBUG: Candidate AI triggers list:");
			for (TriggerElementWeight element : validTriggerCandidates)
			{
				Debug::LogInfo("Weight: {}, [{}][{}]: {}", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (const auto& element : validTriggerCandidates)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
					break;
				}
			}
			break;

		case TeamCategory::Ground:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightGroundOnly) * 1.0;
			/*Debug::LogInfo("Weight Dice: {}", weightDice);

			// Debug
			Debug::LogInfo("DEBUG: Candidate AI triggers list:");
			for (TriggerElementWeight element : validTriggerCandidatesGroundOnly)
			{
				Debug::LogInfo("Weight: {}, [{}][{}]: {}", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (const auto& element : validTriggerCandidatesGroundOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
					break;
				}
			}
			break;

		case TeamCategory::Unclassified:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightUnclassifiedOnly) * 1.0;
			/*Debug::LogInfo("Weight Dice: {}", weightDice);

			// Debug
			Debug::LogInfo("DEBUG: Candidate AI triggers list:");
			for (TriggerElementWeight element : validTriggerCandidatesUnclassifiedOnly)
			{
				Debug::LogInfo("Weight: {}, [{}][{}]: {}", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (const auto& element : validTriggerCandidatesUnclassifiedOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
					break;
				}
			}
			break;

		case TeamCategory::Naval:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightNavalOnly) * 1.0;
			/*Debug::LogInfo("Weight Dice: {}", weightDice);

			// Debug
			Debug::LogInfo("DEBUG: Candidate AI triggers list:");
			for (TriggerElementWeight element : validTriggerCandidatesNavalOnly)
			{
				Debug::LogInfo("Weight: {}, [{}][{}]: {}", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (const auto& element : validTriggerCandidatesNavalOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
					break;
				}
			}
			break;

		case TeamCategory::Air:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightAirOnly) * 1.0;
			/*Debug::LogInfo("Weight Dice: {}", weightDice);

			// Debug
			Debug::LogInfo("DEBUG: Candidate AI triggers list:");
			for (TriggerElementWeight element : validTriggerCandidatesAirOnly)
			{
				Debug::LogInfo("Weight: {}, [{}][{}]: {}", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (const auto& element : validTriggerCandidatesAirOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
					break;
				}
			}
			break;

		default:
			break;
		}

		if (!selectedTrigger)
		{
			return true; // Failed to select trigger - early return
		}

		if (selectedTrigger->Weight_Current >= TeamSelectorConstants::VIP_TRIGGER_THRESHOLD
			&& selectedTrigger->Weight_Minimum <= TeamSelectorConstants::VIP_TRIGGER_RESET_VALUE)
		{
			// Next time this trigger will be out of the elitist triggers list
			selectedTrigger->Weight_Current = TeamSelectorConstants::VIP_TRIGGER_RESET_VALUE;
		}

		// We have a winner trigger here
		Debug::LogInfo("AI Team Selector: House [{}] (idx: {}) selected trigger [{}].", pHouse->Type->ID, pHouse->ArrayIndex, selectedTrigger->ID);

		// Team 1 creation
		if (auto pTriggerTeam1Type = selectedTrigger->Team1)
		{
			int count = 0;
			int underStrengthCount = 0;

			for (const auto& team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam1Type)
				{
					count++;
					// Count teams that are under strength and could use reinforcement
					if (team->IsUnderStrength && !team->IsFullStrength)
						underStrengthCount++;
				}
			}

			// Only create new team if we haven't reached the max AND there aren't too many understrength teams
			// This helps prevent unit stacking by prioritizing reinforcing existing teams
			if (count < pTriggerTeam1Type->Max && underStrengthCount < 2 && teamsCreatedThisFrame < TeamSelectorConstants::MAX_TEAMS_PER_FRAME)
			{
				if (TeamClass* newTeam1 = pTriggerTeam1Type->CreateTeam(pHouse))
				{
					newTeam1->NeedsToDisappear = false;
					teamsCreatedThisFrame++;
					Debug::LogInfo("AI Team Selector: Created new team [{}] for house [{}] (existing: {}, understrength: {}, frame total: {})",
						pTriggerTeam1Type->ID, pHouse->Type->ID, count, underStrengthCount, teamsCreatedThisFrame);
				}
				else
				{
					Debug::LogInfo("AI Team Selector: Failed to create team [{}] for house [{}] - CreateTeam returned null",
						pTriggerTeam1Type->ID, pHouse->Type->ID);
				}
			}
			else
			{
				if (count >= pTriggerTeam1Type->Max)
				{
					Debug::LogInfo("AI Team Selector: Skipped team [{}] creation - reached max limit ({}/{})",
						pTriggerTeam1Type->ID, count, pTriggerTeam1Type->Max);
				}
				else if (underStrengthCount >= 2)
				{
					Debug::LogInfo("AI Team Selector: Skipped team [{}] creation - too many understrength teams ({})",
						pTriggerTeam1Type->ID, underStrengthCount);
				}
				else if (teamsCreatedThisFrame >= TeamSelectorConstants::MAX_TEAMS_PER_FRAME)
				{
					Debug::LogInfo("AI Team Selector: Skipped team [{}] creation - frame limit reached ({}/{})",
						pTriggerTeam1Type->ID, teamsCreatedThisFrame, TeamSelectorConstants::MAX_TEAMS_PER_FRAME);
				}
			}
		}

		// Team 2 creation (if set)
		if (auto pTriggerTeam2Type = selectedTrigger->Team2)
		{
			int count = 0;
			int underStrengthCount = 0;

			for (const auto& team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam2Type)
				{
					count++;
					if (team->IsUnderStrength && !team->IsFullStrength)
						underStrengthCount++;
				}
			}

			if (count < pTriggerTeam2Type->Max && underStrengthCount < 2 && teamsCreatedThisFrame < TeamSelectorConstants::MAX_TEAMS_PER_FRAME)
			{
				if (TeamClass* newTeam2 = pTriggerTeam2Type->CreateTeam(pHouse))
				{
					newTeam2->NeedsToDisappear = false;
					teamsCreatedThisFrame++;
					Debug::LogInfo("AI Team Selector: Created new secondary team [{}] for house [{}] (total this frame: {})",
						pTriggerTeam2Type->ID, pHouse->Type->ID, teamsCreatedThisFrame);
				}
			}
		}
	}

	return true;
}

ASMJIT_PATCH(0x4F8A27, TeamTypeClass_SuggestedNewTeam_NewTeamsSelector, 0x5)
{
	enum { UseOriginalSelector = 0x4F8A63, SkipCode = 0x4F8B08 };
	GET(HouseClass*, pHouse, ESI);

	bool houseIsHuman = pHouse->IsHumanPlayer;
	if (SessionClass::IsCampaign())
		houseIsHuman = pHouse->IsHumanPlayer || pHouse->IsInPlayerControl;

	if (houseIsHuman || pHouse->Type->MultiplayPassive)
		return SkipCode;

	return UpdateTeam(pHouse) ? SkipCode : UseOriginalSelector;
}

#include <ExtraHeaders/StackVector.h>

ASMJIT_PATCH(0x687C9B, ReadScenarioINI_AITeamSelector_PreloadValidTriggers, 0x7)
{
	// For each house save a list with only AI Triggers that can be used
	for (HouseClass* pHouse : *HouseClass::Array)
	{
		StackVector<int, 256> list {};
		const int houseIdx = pHouse->ArrayIndex;
		const int sideIdx = pHouse->SideIndex + 1;

		for (int i = 0; i < AITriggerTypeClass::Array->Count; i++)
		{
			if (auto pTrigger = AITriggerTypeClass::Array->Items[i])
			{
				if (ScenarioClass::Instance->IgnoreGlobalAITriggers == (bool)pTrigger->IsGlobal || !pTrigger->Team1)
					continue;

				const int triggerHouse = pTrigger->HouseIndex;
				const int triggerSide = pTrigger->SideIndex;

				// The trigger must be compatible with the owner
				if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
					list->push_back(i);
			}
		}

		Debug::LogInfo("House {} [{}] could use {} AI triggers in this map.", pHouse->ArrayIndex, pHouse->Type->ID, list->size());
	}

	return 0;
}