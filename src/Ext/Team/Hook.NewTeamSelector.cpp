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
COMPILETIMEEVAL double ValidatePercentage(double percentage)
{
	return (percentage < TeamSelectorConstants::MIN_PERCENTAGE || percentage > TeamSelectorConstants::MAX_PERCENTAGE)
		? TeamSelectorConstants::MIN_PERCENTAGE
		: percentage;
}

// Helper function to check if a team type is already over-represented
COMPILETIMEEVAL bool IsTeamTypeOverRepresented(TeamTypeClass* pTeamType, const HelperedVector<TeamClass*>& activeTeams)
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

struct TriggerElementWeight
{
	double Weight { 0.0 };
	AITriggerTypeClass* Trigger { nullptr };
	TeamCategory Category { TeamCategory::None };

	//need to define a == operator so it can be used in array classes
	COMPILETIMEEVAL bool operator==(const TriggerElementWeight& other) const
	{
		return (Trigger == other.Trigger && Weight == other.Weight && Category == other.Category);
	}

	//unequality
	COMPILETIMEEVAL bool operator!=(const TriggerElementWeight& other) const
	{
		return (Trigger != other.Trigger || Weight != other.Weight || Category != other.Category);
	}

	COMPILETIMEEVAL bool operator<(const TriggerElementWeight& other) const
	{
		return (Weight < other.Weight);
	}

	COMPILETIMEEVAL bool operator<(const double other) const
	{
		return (Weight < other);
	}

	COMPILETIMEEVAL bool operator>(const TriggerElementWeight& other) const
	{
		return (Weight > other.Weight);
	}

	COMPILETIMEEVAL bool operator>(const double other) const
	{
		return (Weight > other);
	}

	COMPILETIMEEVAL bool operator==(const double other) const
	{
		return (Weight == other);
	}

	COMPILETIMEEVAL bool operator!=(const double other) const
	{
		return (Weight != other);
	}
};

COMPILETIMEEVAL bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

COMPILETIMEEVAL bool IsValidTechno(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	// Cache the AbstractType check - it's more efficient to check once
	const auto technoType = pTechno->WhatAmI();
	const bool isValidType = (technoType == AbstractType::Infantry
		|| technoType == AbstractType::Unit
		|| technoType == AbstractType::Building
		|| technoType == AbstractType::Aircraft);

	return !pTechno->Dirty
		&& IsUnitAvailable(pTechno, true)
		&& pTechno->Owner
		&& isValidType;
}

enum class ComparatorOperandTypes
{
	LessThan, LessOrEqual, Equal, MoreOrEqual, More, NotSame
};

COMPILETIMEEVAL void ModifyOperand(bool& result, int counter, AITriggerConditionComparator& cond)
{
	switch ((ComparatorOperandTypes)cond.ComparatorOperand)
	{
	case ComparatorOperandTypes::LessThan:
		result = counter < cond.ComparatorType;
		break;
	case ComparatorOperandTypes::LessOrEqual:
		result = counter <= cond.ComparatorType;
		break;
	case ComparatorOperandTypes::Equal:
		result = counter == cond.ComparatorType;
		break;
	case ComparatorOperandTypes::MoreOrEqual:
		result = counter >= cond.ComparatorType;
		break;
	case ComparatorOperandTypes::More:
		result = counter > cond.ComparatorType;
		break;
	case ComparatorOperandTypes::NotSame:
		result = counter != cond.ComparatorType;
		break;
	default:
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

	if (auto pItemUnit = cast_to<BuildingTypeClass*, false>(pItem))
	{
		if (auto pListBld = cast_to<UnitClass*, false>(list))
		{
			if (pItemUnit->UndeploysInto == pListBld->Type)
				return true;

			if (pListBld->Type->DeploysInto == pItemUnit)
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

// Structure to cache techno data for performance
struct CachedTechnoData
{
	std::vector<TechnoClass*> validTechnos;
	int destroyedBridgesCount = 0;
	int undamagedBridgesCount = 0;

	// Cache recruitable data per house to avoid repeated calculations
	mutable PhobosMap<HouseClass*, PhobosMap<TechnoTypeClass*, int>> recruitableCache;
	mutable bool recruitableCacheValid = false;

	void Initialize()
	{
		validTechnos.clear();
		destroyedBridgesCount = 0;
		undamagedBridgesCount = 0;
		recruitableCache.clear();
		recruitableCacheValid = false;

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

	// Validate that cached technos are still valid
	void ValidateCache()
	{
		// Remove dead/invalid technos from cache
		validTechnos.erase(
			std::remove_if(validTechnos.begin(), validTechnos.end(),
				[](TechnoClass* pTechno) { return !IsValidTechno(pTechno); }),
			validTechnos.end()
		);

		// Invalidate recruitable cache as units might have changed status
		recruitableCacheValid = false;
	}

	// Get recruitable units for a specific house (with proper caching)
	const PhobosMap<TechnoTypeClass*, int>& GetOwnedRecruitables(HouseClass* pHouse)
	{
		// Validate cache first
		if (!recruitableCacheValid)
		{
			recruitableCache.clear();
			recruitableCacheValid = true;
		}

		// Check if we already have data for this house
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

	// Reserve units to prevent race conditions
	bool TryReserveUnits(HouseClass* pHouse, const std::vector<std::pair<TechnoTypeClass*, int>>& requirements)
	{
		auto& recruitables = const_cast<PhobosMap<TechnoTypeClass*, int>&>(GetOwnedRecruitables(pHouse));

		// First check if we have enough units
		for (const auto& req : requirements)
		{
			auto iter = recruitables.get_key_iterator(req.first);
			if (iter == recruitables.end() || iter->second < req.second)
			{
				return false; // Not enough units available
			}
		}

		// Reserve the units by reducing the count
		for (const auto& req : requirements)
		{
			auto iter = recruitables.get_key_iterator(req.first);
			iter->second -= req.second;
		}

		return true;
	}
};

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
					Debug::LogInfo("New AI team category selection: dice {} <= {} (MIXED)", categoryDice, unclassifiedValue);
				}
				else if (percentageGroundTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue))
				{
					validCategory = TeamCategory::Ground;
					Debug::LogInfo("New AI team category selection: dice {} <= {} (mixed: {}%% + GROUND: {}%%)", categoryDice, (unclassifiedValue + groundValue), unclassifiedValue, groundValue);
				}
				else if (percentageAirTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue))
				{
					validCategory = TeamCategory::Air;
					Debug::LogInfo("New AI team category selection: dice {} <= {} (mixed: {}%% + ground: {}%% + AIR: {}%%)", categoryDice, (unclassifiedValue + groundValue + airValue), unclassifiedValue, groundValue, airValue);
				}
				else if (percentageNavalTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue + navalValue))
				{
					validCategory = TeamCategory::Naval;
					Debug::LogInfo("New AI team category selection: dice {} <= {} (mixed: {}%% + ground: {}%% + air: {}%% + NAVAL: {}%%)", categoryDice, (unclassifiedValue + groundValue + airValue + navalValue), unclassifiedValue, groundValue, airValue, navalValue);
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

		if (hasReachedMaxDefensiveTeamsLimit)
			Debug::LogInfo("DEBUG: House [{}] (idx: {}) reached the MaximumAIDefensiveTeams value!", pHouse->Type->ID, pHouse->ArrayIndex);

		if (hasReachedMaxTeamsLimit)
		{
			Debug::LogInfo("DEBUG: House [{}] (idx: {}) reached the TotalAITeamCap value!", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		// Initialize cached techno data for performance
		CachedTechnoData cachedData;
		cachedData.Initialize();

		// Validate cache periodically to remove dead units
		cachedData.ValidateCache();

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
			if (++triggersProcessed % TeamSelectorConstants::CACHE_VALIDATION_INTERVAL == 0)
			{
				cachedData.ValidateCache(); // Periodic cache validation
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
							if (!CountConditionMet(pTrigger, cachedData.destroyedBridgesCount))
								continue;
						}	break;
						case 19:
						{
							// New case 19: Check undamaged bridges
							if (!CountConditionMet(pTrigger, cachedData.undamagedBridgesCount))
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

						// Prepare requirements list
						std::vector<std::pair<TechnoTypeClass*, int>> requirements;
						for (const auto& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							if (entry.Type && entry.Amount > 0)
							{
								requirements.emplace_back(entry.Type, entry.Amount);
							}
						}

						// Try to reserve units atomically to prevent race conditions
						if (!requirements.empty())
						{
							allObjectsCanBeBuiltOrRecruited = cachedData.TryReserveUnits(pHouse, requirements);
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

		if (splitTriggersByCategory)
		{
			switch (validCategory)
			{
			case TeamCategory::Ground:
				Debug::LogInfo("DEBUG: This time only will be picked GROUND teams.");
				break;

			case TeamCategory::Unclassified:
				Debug::LogInfo("DEBUG: This time only will be picked MIXED teams.");
				break;

			case TeamCategory::Naval:
				Debug::LogInfo("DEBUG: This time only will be picked NAVAL teams.");
				break;

			case TeamCategory::Air:
				Debug::LogInfo("DEBUG: This time only will be picked AIR teams.");
				break;

			default:
				Debug::LogInfo("DEBUG: This time teams categories are DISABLED.");
				break;
			}
		}

		if (validTriggerCandidates.empty())
		{
			Debug::LogInfo("DEBUG: [{}] (idx: {}) No valid triggers for now. A new attempt will be done later...", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		if ((validCategory == TeamCategory::Ground && validTriggerCandidatesGroundOnly.empty())
			|| (validCategory == TeamCategory::Unclassified && validTriggerCandidatesUnclassifiedOnly.empty())
			|| (validCategory == TeamCategory::Air && validTriggerCandidatesAirOnly.empty())
			|| (validCategory == TeamCategory::Naval && validTriggerCandidatesNavalOnly.empty()))
		{
			Debug::LogInfo("DEBUG: [{}] (idx: {}) No valid triggers of this category. A new attempt should be done later...", pHouse->Type->ID, pHouse->ArrayIndex);

			if (!isFallbackEnabled)
				return true;

			Debug::LogInfo("... but fallback mode is enabled so now will be checked all available triggers.");
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
			Debug::LogInfo("AI Team Selector: House [{}] (idx: {}) failed to select Trigger. A new attempt Will be done later...", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
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
					Debug::LogInfo("AI Team Selector: Created new team [{}] for house [{}] (total this frame: {})",
						pTriggerTeam1Type->ID, pHouse->Type->ID, teamsCreatedThisFrame);
				}
			}
			else if (count >= pTriggerTeam1Type->Max)
			{
				Debug::LogInfo("AI Team Selector: Skipped team [{}] creation - reached max limit ({}/{})", pTriggerTeam1Type->ID, count, pTriggerTeam1Type->Max);
			}
			else if (underStrengthCount >= 2)
			{
				Debug::LogInfo("AI Team Selector: Skipped team [{}] creation - too many understrength teams ({})", pTriggerTeam1Type->ID, underStrengthCount);
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