#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

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

struct TriggerElementWeight
{
	double Weight { 0.0 };
	AITriggerTypeClass* Trigger { nullptr };
	TeamCategory Category { TeamCategory::None };

	//need to define a == operator so it can be used in array classes
	constexpr bool operator==(const TriggerElementWeight& other) const
	{
		return (Trigger == other.Trigger && Weight == other.Weight && Category == other.Category);
	}

	//unequality
	constexpr bool operator!=(const TriggerElementWeight& other) const
	{
		return (Trigger != other.Trigger || Weight != other.Weight || Category == other.Category);
	}

	constexpr bool operator<(const TriggerElementWeight& other) const
	{
		return (Weight < other.Weight);
	}

	constexpr bool operator<(const double other) const
	{
		return (Weight < other);
	}

	constexpr bool operator>(const TriggerElementWeight& other) const
	{
		return (Weight > other.Weight);
	}

	constexpr bool operator>(const double other) const
	{
		return (Weight > other);
	}

	constexpr bool operator==(const double other) const
	{
		return (Weight == other);
	}

	constexpr bool operator!=(const double other) const
	{
		return (Weight != other);
	}
};

constexpr bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;

}

constexpr bool IsValidTechno(TechnoClass* pTechno)
{
	if (!pTechno)
		return false;

	bool isValid = !pTechno->Dirty
		&& IsUnitAvailable(pTechno, true)
		&& pTechno->Owner
		&& (pTechno->WhatAmI() == AbstractType::Infantry
			|| pTechno->WhatAmI() == AbstractType::Unit
			|| pTechno->WhatAmI() == AbstractType::Building
			|| pTechno->WhatAmI() == AbstractType::Aircraft);

	return isValid;
}

constexpr void ModifyOperand(bool& result, int counter, AITriggerConditionComparator& cond)
{
	switch (cond.ComparatorOperand)
	{
	case 0:
		result = counter < cond.ComparatorType;
		break;
	case 1:
		result = counter <= cond.ComparatorType;
		break;
	case 2:
		result = counter == cond.ComparatorType;
		break;
	case 3:
		result = counter >= cond.ComparatorType;
		break;
	case 4:
		result = counter > cond.ComparatorType;
		break;
	case 5:
		result = counter != cond.ComparatorType;
		break;
	default:
		break;
	}
}

NOINLINE bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, std::vector<TechnoTypeClass*> list)
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
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
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

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
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

			if (pObject->Owner == pCiv && pObject->GetTechnoType() == pItem)
				counter++;
		}
	}

	ModifyOperand(result, counter, *pThis->Conditions);
	return result;
}

NOINLINE bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, std::vector<TechnoTypeClass*>& list)
{
	bool result = true;

	if (list.size() == 0)
		return false;

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

	if (list.size() == 0)
		return false;

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

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
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

	if (list.size() == 0)
		return false;

	auto pCiv = HouseExtData::FindFirstCivilianHouse();

	// Count all objects of the list, like an AND operator
	for (auto const pItem : list) {
		int counter = 0;

		for (auto const pObject : *TechnoClass::Array) {
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

			percentageUnclassifiedTriggers = percentageUnclassifiedTriggers < 0.0 || percentageUnclassifiedTriggers > 1.0 ? 0.0 : percentageUnclassifiedTriggers;
			percentageGroundTriggers = percentageGroundTriggers < 0.0 || percentageGroundTriggers > 1.0 ? 0.0 : percentageGroundTriggers;
			percentageNavalTriggers = percentageNavalTriggers < 0.0 || percentageNavalTriggers > 1.0 ? 0.0 : percentageNavalTriggers;
			percentageAirTriggers = percentageAirTriggers < 0.0 || percentageAirTriggers > 1.0 ? 0.0 : percentageAirTriggers;

			double totalPercengates = percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers;
			if (totalPercengates > 1.0 || totalPercengates <= 0.0)
				splitTriggersByCategory = false;


			if (splitTriggersByCategory)
			{
				int categoryDice = ScenarioClass::Instance->Random.RandomRanged(1, 100);
				int unclassifiedValue = (int)(percentageUnclassifiedTriggers * 100.0);
				int groundValue = (int)(percentageGroundTriggers * 100.0);
				int airValue = (int)(percentageAirTriggers * 100.0);
				int navalValue = (int)(percentageNavalTriggers * 100.0);

				// Pick what type of team will be selected in this round
				if (percentageUnclassifiedTriggers > 0.0 && categoryDice <= unclassifiedValue)
				{
					validCategory = TeamCategory::Unclassified;
					Debug::Log("New AI team category selection: dice %d <= %d (MIXED)\n", categoryDice, unclassifiedValue);
				}
				else if (percentageGroundTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue))
				{
					validCategory = TeamCategory::Ground;
					Debug::Log("New AI team category selection: dice %d <= %d (mixed: %d%% + GROUND: %d%%)\n", categoryDice, (unclassifiedValue + groundValue), unclassifiedValue, groundValue);
				}
				else if (percentageAirTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue))
				{
					validCategory = TeamCategory::Air;
					Debug::Log("New AI team category selection: dice %d <= %d (mixed: %d%% + ground: %d%% + AIR: %d%%)\n", categoryDice, (unclassifiedValue + groundValue + airValue), unclassifiedValue, groundValue, airValue);
				}
				else if (percentageNavalTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue + navalValue))
				{
					validCategory = TeamCategory::Naval;
					Debug::Log("New AI team category selection: dice %d <= %d (mixed: %d%% + ground: %d%% + air: %d%% + NAVAL: %d%%)\n", categoryDice, (unclassifiedValue + groundValue + airValue + navalValue), unclassifiedValue, groundValue, airValue, navalValue);
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
		int sideIdx = pHouse->SideIndex + 1; // Side indexes in AITriggers section are 1-based

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

		/*Debug::Log("\n=====================\n[%s] ACTIVE TEAMS: %d / %d (of them, defensive teams: %d / %d)\n", pHouse->Type->ID, activeTeams, maxTeamsLimit, activeDefenseTeamsCount, defensiveTeamsLimit);
		for (auto team : activeTeamsList)
		{
			Debug::Log("[%s](%d) : %s%s\n", team->Type->ID, team->TotalObjects, team->Type->Name, team->Type->IsBaseDefense ? " -> is DEFENDER team" : "");
			Debug::Log("    IsMoving: %d, IsFullStrength: %d, IsUnderStrength: %d\n", team->IsMoving, team->IsFullStrength, team->IsUnderStrength);
			int i = 0;

			for (auto entry : team->Type->TaskForce->Entries)
			{
				if (entry.Type && entry.Amount > 0)
				{
					if (entry.Type)
						Debug::Log("\t[%s]: %d / %d\n", entry.Type->ID, team->CountObjects[i], entry.Amount);
				}

				i++;
			}
		}
		Debug::Log("\n=====================\n");*/

		// Check if the next team must be a defensive team
		bool onlyPickDefensiveTeams = false;
		int defensiveDice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
		int defenseTeamSelectionThreshold = 50;

		if ((defensiveDice < defenseTeamSelectionThreshold) && !hasReachedMaxDefensiveTeamsLimit)
			onlyPickDefensiveTeams = true;

		if (hasReachedMaxDefensiveTeamsLimit)
			Debug::Log("DEBUG: House [%s] (idx: %d) reached the MaximumAIDefensiveTeams value!\n", pHouse->Type->ID, pHouse->ArrayIndex);

		if (hasReachedMaxTeamsLimit)
		{
			Debug::Log("DEBUG: House [%s] (idx: %d) reached the TotalAITeamCap value!\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		int destroyedBridgesCount = 0;
		int undamagedBridgesCount = 0;
		std::map<TechnoTypeClass*, int> ownedRecruitables;

		for (auto const pTechno : *TechnoClass::Array)
		{
			if (!IsValidTechno(pTechno)) continue;

			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass*>(pTechno);
				if (pBuilding && pBuilding->Type->BridgeRepairHut) {

					CellStruct cell = pTechno->GetCell()->MapCoords;

					if (MapClass::Instance->IsLinkedBridgeDestroyed(cell))
						destroyedBridgesCount++;
					else
						undamagedBridgesCount++;
				}


			} else {

				auto const pFoot = static_cast<FootClass*>(pTechno);

				if (!pFoot
					|| !pTechno->IsAlive
					|| pTechno->Health <= 0
					|| !pTechno->IsOnMap // Note: underground movement is considered "IsOnMap == false"
					|| pTechno->Transporter
					|| pTechno->Absorbed
					|| !pFoot->CanBeRecruited(pHouse))
				{
					continue;
				}

				++ownedRecruitables[pTechno->GetTechnoType()];
			}
		}

		HouseClass* targetHouse = nullptr;
		if (pHouse->EnemyHouseIndex >= 0)
			targetHouse = HouseClass::Array->GetItem(pHouse->EnemyHouseIndex);

		bool onlyCheckImportantTriggers = false;

		// Gather all the trigger candidates into one place for posterior fast calculations
		for (auto const pTrigger : *AITriggerTypeClass::Array)
		{
			if (!pTrigger)
				continue;

			// Ignore offensive teams if the next trigger must be defensive
			if (onlyPickDefensiveTeams && !pTrigger->IsForBaseDefense)
				continue;

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// Ignore the deactivated triggers
			if (pTrigger->IsEnabled)
			{
				// The trigger must be compatible with the owner
				if ((triggerHouse == -1 || houseTypeIdx == triggerHouse) && (triggerSide == 0 || sideTypeIdx == triggerSide))
				{
					// "ConditionType=-1" will be skipped, always is valid
					if ((int)pTrigger->ConditionType >= 0)
					{
						if ((int)pTrigger->ConditionType == 0)
						{
							// Simulate case 0: "enemy owns"
							if (!pTrigger->ConditionObject)
								continue;

							std::vector<TechnoTypeClass*> list;
							list.push_back(pTrigger->ConditionObject);
							bool isConditionMet = EnemyOwns(pTrigger, pHouse, targetHouse, true, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 1)
						{
							// Simulate case 1: "house owns"
							if (!pTrigger->ConditionObject)
								continue;

							std::vector<TechnoTypeClass*> list;
							list.push_back(pTrigger->ConditionObject);
							bool isConditionMet = HouseOwns(pTrigger, pHouse, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 7)
						{
							// Simulate case 7: "civilian owns"
							if (!pTrigger->ConditionObject)
								continue;

							std::vector<TechnoTypeClass*> list;
							list.push_back(pTrigger->ConditionObject);
							bool isConditionMet = NeutralOwns(pTrigger, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 8)
						{
							// Simulate case 0: "enemy owns" but instead of restrict it against the main enemy house it is done against all enemies
							if (!pTrigger->ConditionObject)
								continue;

							std::vector<TechnoTypeClass*> list;
							list.push_back(pTrigger->ConditionObject);
							bool isConditionMet = EnemyOwns(pTrigger, pHouse, nullptr, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 9)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 9: Like in case 0 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the enemy.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwns(pTrigger, pHouse, targetHouse, false, RulesExtData::Instance()->AITargetTypesLists[pTrigger->Conditions[3].ComparatorOperand]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 10)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 10: Like in case 1 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the house.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwns(pTrigger, pHouse, false, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 11)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 11: Like in case 7 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the Civilians.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!NeutralOwns(pTrigger, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 12)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 12: Like in case 0 & 9 but instead of a specific enemy this checks in all enemies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwns(pTrigger, pHouse, nullptr, false, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 13)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 13: Like in case 1 & 10 but instead checking the house now checks the allies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwns(pTrigger, pHouse, true, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 14)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 14: Like in case 9 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwnsAll(pTrigger, pHouse, targetHouse, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 15)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 15: Like in case 10 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!HouseOwnsAll(pTrigger, pHouse, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 16)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 16: Like in case 11 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!NeutralOwnsAll(pTrigger, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 17)
						{
							if ((size_t)pTrigger->Conditions[3].ComparatorOperand < RulesExtData::Instance()->AITargetTypesLists.size())
							{
								// New case 17: Like in case 14 but instead of meet any comparison now is required all. Check all enemies
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								if (!EnemyOwnsAll(pTrigger, pHouse, nullptr, RulesExtData::Instance()->AITargetTypesLists[(pTrigger->Conditions[3].ComparatorOperand)]))
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 18)
						{
							// New case 18: Check destroyed bridges
							if (!CountConditionMet(pTrigger, destroyedBridgesCount))
								continue;
						}
						else if ((int)pTrigger->ConditionType == 19)
						{
							// New case 19: Check undamaged bridges
							if (!CountConditionMet(pTrigger, undamagedBridgesCount))
								continue;
						}
						else
						{
							// Other cases from vanilla game
							if (!pTrigger->ConditionMet(pHouse, targetHouse, hasReachedMaxDefensiveTeamsLimit))
								continue;
						}
					}

					// All triggers below 5000 in current weight will get discarded if this mode is enabled
					if (onlyCheckImportantTriggers)
					{
						if (pTrigger->Weight_Current < 5000)
							continue;
					}

					auto pTriggerTeam1Type = pTrigger->Team1;
					if (!pTriggerTeam1Type)
						continue;

					// No more defensive teams needed
					if (pTriggerTeam1Type->IsBaseDefense && hasReachedMaxDefensiveTeamsLimit)
						continue;

					// If this type of Team reached the max then skip it
					int count = 0;

					for (auto team : activeTeamsList)
					{
						if (team->Type == pTriggerTeam1Type)
							count++;
					}

					if (count >= pTriggerTeam1Type->Max)
						continue;

					TeamCategory teamIsCategory = TeamCategory::None;

					// Analyze what kind of category is this main team if the feature is enabled
					if (splitTriggersByCategory)
					{
						//Debug::Log("DEBUG: TaskForce [%s] members:\n", pTriggerTeam1Type->TaskForce->ID);
						// TaskForces are limited to 6 entries
						for (int i = 0; i < 6; i++)
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
									//Debug::Log("\t[%s](%d) is in AIR category.\n", entry.Type->ID, entry.Amount);
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
										//Debug::Log("\t[%s](%d) is in NAVAL category.\n", entry.Type->ID, entry.Amount);
									}

									if (pTechnoTypeExt->ConsideredVehicle
										|| (entryIsCategory != TeamCategory::Naval
											&& entryIsCategory != TeamCategory::Air))
									{
										// This unit is from ground category
										entryIsCategory = TeamCategory::Ground;
										//Debug::Log("\t[%s](%d) is in GROUND category.\n", entry.Type->ID, entry.Amount);
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

						//Debug::Log("DEBUG: This team is a category %d (1:Ground, 2:Air, 3:Naval, 4:Mixed).\n", teamIsCategory);
						// Si existe este valor y el team es MIXTO se sobreescribe el tipo de categoría
						if (teamIsCategory == TeamCategory::Unclassified
							&& mergeUnclassifiedCategoryWith >= 0)
						{
							//Debug::Log("DEBUG: MIXED category forced to work as category %d.\n", mergeUnclassifiedCategoryWith);
							teamIsCategory = (TeamCategory)mergeUnclassifiedCategoryWith;
						}
					}

					bool allObjectsCanBeBuiltOrRecruited = true;

					if (pTriggerTeam1Type->Autocreate)
					{
						for (auto entry : pTriggerTeam1Type->TaskForce->Entries)
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

						for (auto entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce has the available recruitable units in the map
							if (allObjectsCanBeBuiltOrRecruited && entry.Type && entry.Amount > 0)
							{
								if ((int)ownedRecruitables.count(entry.Type) < entry.Amount)
								{
									allObjectsCanBeBuiltOrRecruited = false;
									break;
								}
							}
						}
					}

					// We can't let AI cheat in this trigger because doesn't have the required tech tree available
					if (!allObjectsCanBeBuiltOrRecruited)
						continue;

					// Special case: triggers become very important if they reach the max priority (value 5000).
					// They get stored in a elitist list and all previous triggers are discarded
					if (pTrigger->Weight_Current >= 5000 && !onlyCheckImportantTriggers)
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
					totalWeight += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
					validTriggerCandidates.emplace_back(totalWeight, pTrigger, teamIsCategory);

					if (splitTriggersByCategory)
					{
						switch (teamIsCategory)
						{
						case TeamCategory::Ground:
							totalWeightGroundOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
							validTriggerCandidatesGroundOnly.emplace_back(totalWeightGroundOnly , pTrigger , teamIsCategory);
							break;

						case TeamCategory::Air:
							totalWeightAirOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
							validTriggerCandidatesAirOnly.emplace_back(totalWeightAirOnly, pTrigger, teamIsCategory);
							break;

						case TeamCategory::Naval:
							totalWeightNavalOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
							validTriggerCandidatesNavalOnly.emplace_back(totalWeightNavalOnly, pTrigger, teamIsCategory);
							break;

						case TeamCategory::Unclassified:
							totalWeightUnclassifiedOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
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
				Debug::Log("DEBUG: This time only will be picked GROUND teams.\n");
				break;

			case TeamCategory::Unclassified:
				Debug::Log("DEBUG: This time only will be picked MIXED teams.\n");
				break;

			case TeamCategory::Naval:
				Debug::Log("DEBUG: This time only will be picked NAVAL teams.\n");
				break;

			case TeamCategory::Air:
				Debug::Log("DEBUG: This time only will be picked AIR teams.\n");
				break;

			default:
				Debug::Log("DEBUG: This time teams categories are DISABLED.\n");
				break;
			}
		}

		if (validTriggerCandidates.empty())
		{
			Debug::Log("DEBUG: [%s] (idx: %d) No valid triggers for now. A new attempt will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		if ((validCategory == TeamCategory::Ground && validTriggerCandidatesGroundOnly.empty())
			|| (validCategory == TeamCategory::Unclassified && validTriggerCandidatesUnclassifiedOnly.empty())
			|| (validCategory == TeamCategory::Air && validTriggerCandidatesAirOnly.empty())
			|| (validCategory == TeamCategory::Naval && validTriggerCandidatesNavalOnly.empty()))
		{
			Debug::Log("DEBUG: [%s] (idx: %d) No valid triggers of this category. A new attempt should be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);

			if (!isFallbackEnabled)
				return true;

			Debug::Log("... but fallback mode is enabled so now will be checked all available triggers.\n");
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
			/*Debug::Log("Weight Dice: %f\n", weightDice);

			// Debug
			Debug::Log("DEBUG: Candidate AI triggers list:\n");
			for (TriggerElementWeight element : validTriggerCandidates)
			{
				Debug::Log("Weight: %f, [%s][%s]: %s\n", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (auto element : validTriggerCandidates)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
				}
			}
			break;

		case TeamCategory::Ground:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightGroundOnly) * 1.0;
			/*Debug::Log("Weight Dice: %f\n", weightDice);

			// Debug
			Debug::Log("DEBUG: Candidate AI triggers list:\n");
			for (TriggerElementWeight element : validTriggerCandidatesGroundOnly)
			{
				Debug::Log("Weight: %f, [%s][%s]: %s\n", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (auto element : validTriggerCandidatesGroundOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
				}
			}
			break;

		case TeamCategory::Unclassified:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightUnclassifiedOnly) * 1.0;
			/*Debug::Log("Weight Dice: %f\n", weightDice);

			// Debug
			Debug::Log("DEBUG: Candidate AI triggers list:\n");
			for (TriggerElementWeight element : validTriggerCandidatesUnclassifiedOnly)
			{
				Debug::Log("Weight: %f, [%s][%s]: %s\n", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (auto element : validTriggerCandidatesUnclassifiedOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
				}
			}
			break;

		case TeamCategory::Naval:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightNavalOnly) * 1.0;
			/*Debug::Log("Weight Dice: %f\n", weightDice);

			// Debug
			Debug::Log("DEBUG: Candidate AI triggers list:\n");
			for (TriggerElementWeight element : validTriggerCandidatesNavalOnly)
			{
				Debug::Log("Weight: %f, [%s][%s]: %s\n", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (auto element : validTriggerCandidatesNavalOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
				}
			}
			break;

		case TeamCategory::Air:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightAirOnly) * 1.0;
			/*Debug::Log("Weight Dice: %f\n", weightDice);

			// Debug
			Debug::Log("DEBUG: Candidate AI triggers list:\n");
			for (TriggerElementWeight element : validTriggerCandidatesAirOnly)
			{
				Debug::Log("Weight: %f, [%s][%s]: %s\n", element.Weight, element.Trigger->ID, element.Trigger->Team1->ID, element.Trigger->Team1->Name);
			}*/

			for (auto element : validTriggerCandidatesAirOnly)
			{
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					selectedTrigger = element.Trigger;
					found = true;
				}
			}
			break;

		default:
			break;
		}

		if (!selectedTrigger)
		{
			Debug::Log("AI Team Selector: House [%s] (idx: %d) failed to select Trigger. A new attempt Will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		if (selectedTrigger->Weight_Current >= 5000.0
			&& selectedTrigger->Weight_Minimum <= 4999.0)
		{
			// Next time this trigger will be out of the elitist triggers list
			selectedTrigger->Weight_Current = 4999.0;
		}

		// We have a winner trigger here
		Debug::Log("AI Team Selector: House [%s] (idx: %d) selected trigger [%s].\n", pHouse->Type->ID, pHouse->ArrayIndex, selectedTrigger->ID);

		// Team 1 creation
		if (auto pTriggerTeam1Type = selectedTrigger->Team1)
		{
			int count = 0;

			for (auto team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam1Type)
					count++;
			}

			if (count < pTriggerTeam1Type->Max)
			{
				if (TeamClass* newTeam1 = pTriggerTeam1Type->CreateTeam(pHouse))
					newTeam1->NeedsToDisappear = false;
			}
		}

		// Team 2 creation (if set)

		if (auto pTriggerTeam2Type = selectedTrigger->Team2)
		{
			int count = 0;

			for (auto team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam2Type)
					count++;
			}

			if (count < pTriggerTeam2Type->Max)
			{
				if (TeamClass* newTeam2 = pTriggerTeam2Type->CreateTeam(pHouse))
					newTeam2->NeedsToDisappear = false;
			}
		}
	}

	return true;
}

DEFINE_HOOK(0x4F8A27, TeamTypeClass_SuggestedNewTeam_NewTeamsSelector, 0x5)
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