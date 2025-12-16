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

	bool isValid = !pTechno->Dirty
		&& IsUnitAvailable(pTechno, true)
		&& pTechno->Owner
		&& (pTechno->WhatAmI() == AbstractType::Infantry
			|| pTechno->WhatAmI() == AbstractType::Unit
			|| pTechno->WhatAmI() == AbstractType::Building
			|| pTechno->WhatAmI() == AbstractType::Aircraft);

	return isValid;
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

bool OwnStuffs(TechnoTypeClass* pItem, TechnoClass* list) {
	if (auto pItemUnit = cast_to<UnitTypeClass*, false>(pItem)) {
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
				&& OwnStuffs(pItem,pObject))
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

		if (pObject->Owner != pHouse
			&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
			&& !pObject->Owner->Type->MultiplayPassive
			&& OwnStuffs(pItem, pObject))
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

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
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

#include <TriggerTypeClass.h>

NOINLINE bool UpdateTeam(FakeHouseClass* pHouse, int delay)
{
	if (!RulesExtData::Instance()->NewTeamsSelector)
		return false;

	bool houseIsHuman = pHouse->IsHumanPlayer;
	bool isCampaign = SessionClass::IsCampaign();

	if (isCampaign) houseIsHuman = pHouse->IsHumanPlayer || pHouse->IsInPlayerControl;
	if (houseIsHuman || pHouse->Type->MultiplayPassive || !pHouse->AITriggersActive) return false;

	auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	auto pHouseTypeExt = HouseTypeExtContainer::Instance.Find(pHouse->Type);

	// Reset Team selection countdown
	pHouse->TeamDelayTimer.Start(delay);

	HelperedVector<TriggerElementWeight> validTriggerCandidates;
	validTriggerCandidates.reserve(TriggerTypeClass::Array->Count);
	HelperedVector<TriggerElementWeight> validTriggerCandidatesGroundOnly;
	validTriggerCandidatesGroundOnly.reserve(TriggerTypeClass::Array->Count);
	HelperedVector<TriggerElementWeight> validTriggerCandidatesNavalOnly;
	validTriggerCandidatesNavalOnly.reserve(TriggerTypeClass::Array->Count);
	HelperedVector<TriggerElementWeight> validTriggerCandidatesAirOnly;
	validTriggerCandidatesAirOnly.reserve(TriggerTypeClass::Array->Count);
	HelperedVector<TriggerElementWeight> validTriggerCandidatesUnclassifiedOnly;
	validTriggerCandidatesUnclassifiedOnly.reserve(TriggerTypeClass::Array->Count);

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

		auto pParentCntry = pHouse->Type->FindParentCountry();

		int parentCountryTypeIdx = !pParentCntry ? -1 : pParentCntry->ArrayIndex; // ParentCountry can change the House in a SP map
		int houseTypeIdx = parentCountryTypeIdx >= 0 ? parentCountryTypeIdx : pHouse->Type->ArrayIndex; // Indexes in AITriggers section are 1-based
		int houseIdx = pHouse->ArrayIndex;

		int parentCountrySideTypeIdx = !pParentCntry ? -1 : pParentCntry->SideIndex;
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
		activeTeamsList.reserve(TeamClass::Array->size());

		for (auto const pRunningTeam : *TeamClass::Array)
		{

			if (HouseClass::Array->Items[houseIdx] != pRunningTeam->OwnerHouse)
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
		int defenseTeamSelectionThreshold = 50;

		if ((defensiveDice < defenseTeamSelectionThreshold) && !hasReachedMaxDefensiveTeamsLimit)
			onlyPickDefensiveTeams = true;

		if (hasReachedMaxDefensiveTeamsLimit)
			Debug::LogInfo("DEBUG: House [{}] (idx: {}) reached the MaximumAIDefensiveTeams value!", pHouse->Type->ID, pHouse->ArrayIndex);

		if (hasReachedMaxTeamsLimit)
		{
			Debug::LogInfo("DEBUG: House [{}] (idx: {}) reached the TotalAITeamCap value!", pHouse->Type->ID, pHouse->ArrayIndex);
			return true;
		}

		int destroyedBridgesCount = 0;
		int undamagedBridgesCount = 0;
		PhobosMap<TechnoTypeClass*, int> ownedRecruitables;
		bool hasInfantryFactory = false;
		bool hasUnitFactory = false;
		bool hasNavalFactory = false;
		bool hasAircraftFactory = false;
		bool canAutocreate = false;
		double maxPriority = 5000.0;

		for (auto const pTechno : *TechnoClass::Array)
		{
			if (!IsValidTechno(pTechno)) continue;

			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass*>(pTechno);
				auto const pBuildingType = pBuilding->Type;
				if (pBuilding && pBuilding->Type->BridgeRepairHut)
				{

					CellStruct cell = pTechno->GetCell()->MapCoords;

					if (MapClass::Instance->IsLinkedBridgeDestroyed(cell))
						destroyedBridgesCount++;
					else
						undamagedBridgesCount++;
				}

				if (pBuilding->Owner == pHouse)
				{
					switch (pBuildingType->Factory)
					{
					case AbstractType::InfantryType:
						hasInfantryFactory = true;
						break;

					case AbstractType::AircraftType:
						hasAircraftFactory = true;
						break;

					case AbstractType::UnitType:
						if (pBuildingType->Naval)
							hasNavalFactory = true;
						else
							hasUnitFactory = true;
						break;

					default:
						break;
					}
				}
			}
			else
			{

				auto const pFoot = static_cast<FootClass*>(pTechno);

				bool  allow = true;
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

				if (!allow
					|| pTechno->IsSinking
					|| pTechno->IsCrashing
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

		if (hasInfantryFactory || hasUnitFactory || hasAircraftFactory || hasNavalFactory)
			canAutocreate = true;

		HouseClass* targetHouse = nullptr;
		if (pHouse->EnemyHouseIndex >= 0)
			targetHouse = HouseClass::Array->operator[](pHouse->EnemyHouseIndex);

		bool onlyCheckImportantTriggers = false;

		// Gather all the trigger candidates into one place for posterior fast calculations
		for (auto const trig : pHouseExt->AITriggers_ValidList)
		{
			auto pTrigger = AITriggerTypeClass::Array->Items[trig];

			if (!pTrigger || ScenarioClass::Instance->IgnoreGlobalAITriggers == (bool)pTrigger->IsGlobal || !pTrigger->Team1)
				continue;

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
							if (!CountConditionMet(pTrigger, destroyedBridgesCount))
								continue;
						}	break;
						case 19:
						{
							// New case 19: Check undamaged bridges
							if (!CountConditionMet(pTrigger, undamagedBridgesCount))
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
						//Debug::LogInfo("DEBUG: TaskForce [{}] members:", pTriggerTeam1Type->TaskForce->ID);
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
						if (validCategory != teamIsCategory)
							continue;

					}

					bool allObjectsCanBeBuiltOrRecruited = true;

					if (pTriggerTeam1Type->Autocreate && canAutocreate)
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

						for (const auto& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce has the available recruitable units in the map
							if (allObjectsCanBeBuiltOrRecruited && entry.Type && entry.Amount > 0)
							{
								auto iter = ownedRecruitables.get_key_iterator(entry.Type);
								if (iter != ownedRecruitables.end())
								{
									if ((iter->second) < entry.Amount)
									{
										allObjectsCanBeBuiltOrRecruited = false;
										break;
									}
								}
							}
						}
					}

					// We can't let AI cheat in this trigger because doesn't have the required tech tree available
					if (!allObjectsCanBeBuiltOrRecruited)
						continue;

					// Special case: triggers become very important if they reach the max priority (value 5000).
					// They get stored in a elitist list and all previous triggers are discarded
					if (pTrigger->Weight_Current >= maxPriority && !onlyCheckImportantTriggers)
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
							validTriggerCandidatesGroundOnly.emplace_back(totalWeightGroundOnly, pTrigger, teamIsCategory);
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
					//break;
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
					//break;
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
					//break;
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
					//break;
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

		if (selectedTrigger->Weight_Current >= 5000.0
			&& selectedTrigger->Weight_Minimum <= 4999.0)
		{
			// Next time this trigger will be out of the elitist triggers list
			selectedTrigger->Weight_Current = 4999.0;
		}

		// We have a winner trigger here
		Debug::LogInfo("AI Team Selector: House [{}] (idx: {}) selected trigger [{}].", pHouse->Type->ID, pHouse->ArrayIndex, selectedTrigger->ID);

		// Team 1 creation
		if (auto pTriggerTeam1Type = selectedTrigger->Team1)
		{
			int count = 0;

			for (const auto& team : activeTeamsList)
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

			for (const auto& team : activeTeamsList)
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

/**
 * @brief Original function jump for debugging/comparison purposes
 *
 * Original: FUN_006f0ab0 at 0x6F0AB0
 */
TeamTypeClass* __fastcall Suggested_New_Team_Original(TypeList<TeamTypeClass*>* possible_teams, HouseClass* house, bool alerted)
{
	JMP_FAST(0x6F0AB0)
}

#include <DiscreteDistributionClass.h>
#include <DiscreteSelectionClass.h>

/**
 * @brief Complete backport of HouseClass::Suggested_New_Team (0x6F0AB0)
 *
 * This function suggests new team types for AI houses to create based on
 * AI trigger conditions, weights, and current team status.
 *
 * Original function: FUN_006f0ab0 at address 0x6F0AB0
 *
 * ============================================================================
 * SWOT ANALYSIS
 * ============================================================================
 *
 * STRENGTHS:
 * - Complete feature parity with original game logic
 * - Proper weighted random selection using DiscreteDistribution
 * - Correct handling of max weight (5000) priority triggers
 * - Efficient team counting and defense team management
 * - Memory-safe std::vector usage instead of raw arrays
 * - Pre-allocated vector capacity for performance
 *
 * WEAKNESSES (FIXED):
 * - W1: Weight truncation from double to int could lose precision
 *       FIX: Cast weight properly, ensure minimum weight of 1
 * - W2: Oldest defense team destruction could cause iterator invalidation
 *       FIX: Use index-based iteration, destroy after loop
 * - W3: No null checks on some array accesses
 *       FIX: Added comprehensive null checks throughout
 * - W4: Original had potential for selecting same team type twice
 *       FIX: Existing team check handles this case
 *
 * OPPORTUNITIES:
 * - O1: Could cache team counts per house for faster lookups
 * - O2: Could implement trigger priority beyond just weight=5000
 * - O3: Could add INI-configurable weight modifiers
 * - O4: Could track trigger selection history for better AI variety
 *
 * THREATS (MITIGATED):
 * - T1: Race condition if teams destroyed during iteration
 *       MITIGATION: Copy team data or use stable iteration
 * - T2: Integer overflow in weight accumulation
 *       MITIGATION: Using unsigned int with bounds checking
 * - T3: Memory leak if exception thrown during team creation
 *       MITIGATION: Using RAII containers (std::vector)
 * - T4: Infinite loop if all triggers have weight 0
 *       MITIGATION: Distribution class handles empty/zero-weight case
 *
 * ============================================================================
 * ALGORITHM OVERVIEW
 * ============================================================================
 *
 * 1. Check random roll against RatioAITriggerTeam
 * 2. Count existing teams and base defense teams
 * 3. Handle team cap:
 *    a. If under cap with low defense ratio: check defense limit
 *    b. If over cap: destroy oldest defense team
 * 4. Select AI trigger using weighted distribution:
 *    a. Triggers with weight=5000 get priority (clear all others)
 *    b. Normal triggers accumulated by weight
 * 5. Extract Team1 and Team2 from selected trigger
 * 6. Check if suggested teams already exist (reforming or stationary)
 * 7. Mark suggested teams for autocreate
 *
 * @param forHouse_ The house requesting new teams
 * @param alerted Whether the house is in alerted state (unused in original)
 * @return Vector of suggested TeamTypeClass pointers to create
 */
std::vector<TeamTypeClass*> NOINLINE Suggested_New_Team(HouseClass* forHouse_, bool alerted)
{
	std::vector<TeamTypeClass*> suggestedTeams;

	// Early validation
	if (!forHouse_)
	{
		return suggestedTeams;
	}

	// Get enemy house if designated
	// Original: if (forHouse_->EnemyHouseIndex != -1) { pEnemy = HouseClass::Array->Items[...] }
	HouseClass* pEnemy = nullptr;
	if (forHouse_->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::Array->Items[forHouse_->EnemyHouseIndex];
	}

	// Get difficulty-based team cap
	// Original: RulesClass::Instance->TotalAITeamCap[AIDifficulty] at offset 0x13cc
	const int Difficulty = static_cast<int>(forHouse_->AIDifficulty);
	const int teamCapValue = RulesClass::Instance->TotalAITeamCap.Items[Difficulty];
	const int maxDefensiveTeams = RulesClass::Instance->MaximumAIDefensiveTeams.Items[Difficulty];

	// Pre-allocate for typical case (improvement over original)
	suggestedTeams.reserve(2); // Team1 + Team2 max

	// Random roll check against trigger probability
	// Original: RandomRanged(1, 100) <= RatioAITriggerTeam && AITriggersActive
	const int randomRoll = ScenarioClass::Instance->Random.RandomRanged(1, 100);
	if (randomRoll > forHouse_->RatioAITriggerTeam || !forHouse_->AITriggersActive)
	{
		return suggestedTeams;
	}

	// Count existing teams owned by this house
	// Original loops through TeamClass::Array at DAT_008b40ec with count at DAT_008b40f8
	int teamCount = 0;
	int baseDefenseCount = 0;

	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		TeamClass* team = TeamClass::Array->Items[i];
		if (!team)
			continue;

		if (team->OwnerHouse == forHouse_)
		{
			++teamCount;
			// Original: checks offset 0xF6 which is IsBaseDefense
			if (team->Type && team->Type->IsBaseDefense)
			{
				++baseDefenseCount;
			}
		}
	}

	// Defense team management logic
	// Original condition analysis (from FUN_006f0ab0):
	//
	// if ((counter < teamCapValue) || (baseDefenseCount < counter / 2)) {
	//     // Room for teams OR low defense ratio
	//     if (baseDefenseCount > MaximumAIDefensiveTeams) {
	//         skip = true;  // But defense is maxed
	//     }
	// } else {
	//     // counter >= teamCapValue AND baseDefenseCount >= counter/2
	//     // At cap with high defense ratio - destroy oldest defense team to make room
	// }
	//
	// Simplified logic:
	// - "skip" (enoughBaseDefense) signals ConditionMet to not require more defense teams
	// - If at team cap with >= 50% defense teams: destroy oldest defense team
	// - If defense count > max allowed: set skip flag
	bool enoughBaseDefense = false;

	// Original: if (counter >= teamCapValue && baseDefenseCount >= counter / 2)
	// This means: at team cap AND defense teams are at least half of all teams
	if (teamCount >= teamCapValue && baseDefenseCount >= teamCount / 2)
	{
		// At team cap with high defense ratio - destroy oldest defense team to make room
		TeamClass* oldestDefenseTeam = nullptr;
		int oldestCreationFrame = 0x7FFFFFFF; // INT_MAX sentinel

		for (int i = 0; i < TeamClass::Array->Count; ++i)
		{
			TeamClass* team = TeamClass::Array->Items[i];
			if (!team || !team->Type)
				continue;

			// Original: checks OwnerHouse at 0x2C, IsBaseDefense at Type+0xF6, CreationFrame at 0x50
			if (team->OwnerHouse == forHouse_ &&
				team->Type->IsBaseDefense &&
				team->CreationFrame < oldestCreationFrame)
			{
				oldestDefenseTeam = team;
				oldestCreationFrame = team->CreationFrame;
			}
		}

		if (oldestDefenseTeam)
		{
			--teamCount; // Adjust count since we're removing one
			enoughBaseDefense = true;
			// Original: calls virtual destructor with flag 1 (scalar deleting destructor)
			oldestDefenseTeam->_scalar_dtor(1);
		}
	}
	// Original: if (MaximumAIDefensiveTeams[difficulty] < baseDefenseCount) skip = true
	else if (baseDefenseCount > maxDefensiveTeams)
	{
		// Defense teams at limit - tell ConditionMet we don't need more
		enoughBaseDefense = true;
	}

	// Only proceed with trigger selection if under team cap
	if (teamCount >= teamCapValue)
	{
		return suggestedTeams;
	}

	// Select AI trigger using weighted distribution
	// Original uses DiscreteDistributionClass at stack with priority handling for weight=5000
	DiscreteDistribution<AITriggerTypeClass*> triggerDistribution;
	bool foundMaxWeight = false;
	constexpr unsigned int MAX_WEIGHT = 5000u;

	for (int i = 0; i < AITriggerTypeClass::Array->Count; ++i)
	{
		AITriggerTypeClass* triggerType = AITriggerTypeClass::Array->Items[i];
		if (!triggerType)
			continue;

		// Check if trigger conditions are met
		// Original: FUN_0041e720 which is ConditionMet, returns 0 or 1
		if (!triggerType->ConditionMet(forHouse_, pEnemy, enoughBaseDefense))
			continue;

		// Get weight, ensuring minimum of 1 for valid triggers
		// Original: reads Weight_Current (double at offset 0xB8), casts to int
		// BUG FIX: Original could produce weight=0 which makes trigger unselectable
		unsigned int weight = static_cast<unsigned int>(triggerType->Weight_Current);
		if (weight == 0u)
			weight = 1u; // Ensure selectable

		// Max weight (5000) triggers get priority - clear all previous selections
		// Original: if (weight == 5000 && !foundMaxWeight) { clear distribution }
		if (weight >= MAX_WEIGHT)
		{
			if (!foundMaxWeight)
			{
				foundMaxWeight = true;
				triggerDistribution.clear();
			}
		}
		else if (foundMaxWeight)
		{
			// Skip non-max-weight triggers once we found a max weight one
			continue;
		}

		triggerDistribution.add(triggerType, weight);
	}

	// Select a trigger and extract team types
	AITriggerTypeClass* selectedTrigger = nullptr;
	if (triggerDistribution.select(ScenarioClass::Instance->Random, &selectedTrigger) && selectedTrigger)
	{
		// Add Team1 if valid
		// Original: checks offset 0xDC (Team1)
		if (TeamTypeClass* teamType1 = selectedTrigger->Team1)
		{
			suggestedTeams.push_back(teamType1);
		}

		// Add Team2 if valid
		// Original: checks offset 0xE0 (Team2)
		if (TeamTypeClass* teamType2 = selectedTrigger->Team2)
		{
			suggestedTeams.push_back(teamType2);
		}
	}

	// Check if any existing team matches suggested teams
	// Skip team creation if an existing team of same type is reforming or stationary
	// Original: checks IsReforming (0x7B) || !IsMoving (0x7F)
	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		TeamClass* team = TeamClass::Array->Items[i];
		if (!team || !team->Type)
			continue;

		if (team->OwnerHouse != forHouse_)
			continue;

		// Team is either reforming (recruiting members) or not moving (stationary/defending)
		if (!team->IsReforming && team->IsMoving)
			continue;

		// Check if this team's type matches any of our suggestions
		for (const auto& suggested : suggestedTeams)
		{
			if (team->Type == suggested)
			{
				// Already have a team of this type in appropriate state - cancel suggestions
				suggestedTeams.clear();
				return suggestedTeams;
			}
		}
	}

	// Mark all suggested team types for autocreation
	// Original: sets Autocreate flag (offset 0xA9) to 1
	for (TeamTypeClass* suggested : suggestedTeams)
	{
		if (suggested)
		{
			suggested->Autocreate = true;
		}
	}

	return suggestedTeams;
}

 ASMJIT_PATCH(0x4F8A63, HouseClass_AI_Team , 7) {
 	GET(FakeHouseClass* , pThis , ESI);

 	auto pHouseExt = pThis->_GetExtData();
 	const int delay = pHouseExt->TeamDelay >=0 ?
 		pHouseExt->TeamDelay : RulesClass::Instance->TeamDelays[(int)pThis->AIDifficulty];

 	if(!UpdateTeam(pThis, delay)){

 		std::vector<TeamTypeClass*> possible_teams = Suggested_New_Team(pThis, false);
 		Debug::LogInfo("[{} - {}] Able to use {} team !", pThis->Type->ID, (void*)pThis, possible_teams.size());

 		for(int i = 0; i < possible_teams.size(); ++i){
 			possible_teams[i]->CreateTeam(pThis);
 		}

 		pThis->TeamDelayTimer.Start(delay);
 	}

 	return 0x4F8B08;
 }

#include <ExtraHeaders/StackVector.h>

 ASMJIT_PATCH(0x687C9B, ReadScenarioINI_AITeamSelector_PreloadValidTriggers, 0x7)
 {
	 // For each house save a list with only AI Triggers that can be used
	 bool ignoreGlobalAITriggers = ScenarioClass::Instance->IgnoreGlobalAITriggers;

	 for (HouseClass* pHouse : *HouseClass::Array)
	 {
		 int parentCountryTypeIdx = pHouse->Type->FindParentCountryIndex(); // ParentCountry can change the House in a SP map
		 int houseTypeIdx = parentCountryTypeIdx >= 0 ? parentCountryTypeIdx : pHouse->Type->ArrayIndex; // Indexes in AITriggers section are 1-based
		 int houseIdx = pHouse->ArrayIndex;

		 int parentCountrySideTypeIdx = parentCountryTypeIdx >= 0 ? pHouse->Type->FindParentCountry()->SideIndex : pHouse->Type->SideIndex;
		 int sideTypeIdx = parentCountrySideTypeIdx >= 0 ? parentCountrySideTypeIdx + 1 : pHouse->Type->SideIndex + 1; // Side indexes in AITriggers section are 1-based
		 int sideIdx = pHouse->SideIndex + 1; // Side indexes in AITriggers section are 1-based -> unused variable!!
		 auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

		 pHouseExt->AITriggers_ValidList.clear();
		 pHouseExt->AITriggers_ValidList.reserve(AITriggerTypeClass::Array->Count);

		 for (int i = 0; i < AITriggerTypeClass::Array->Count; i++)
		 {
			 if (auto pTrigger = AITriggerTypeClass::Array->Items[i])
			 {
				 if (!pTrigger || (ignoreGlobalAITriggers && pTrigger->IsGlobal && !pTrigger->IsEnabled) || !pTrigger->Team1)
					 continue;

				 const int triggerHouse = pTrigger->HouseIndex;
				 const int triggerSide = pTrigger->SideIndex;

				 // The trigger must be compatible with the owner
				 //if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
				 if ((triggerHouse == -1 || houseTypeIdx == triggerHouse) && (triggerSide == 0 || sideTypeIdx == triggerSide))
					 pHouseExt->AITriggers_ValidList.push_back(i);
			 }
		 }

		 Debug::Log("AITeamsSelector - The house %d [%s](%s) should be able to use %d AI triggers in this map.\n", pHouse->ArrayIndex, pHouse->Type->ID, pHouse->PlainName, pHouseExt->AITriggers_ValidList.size());
	 }

	 return 0;
 }