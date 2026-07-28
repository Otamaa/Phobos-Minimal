#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Foot/Body.h>
#include <Ext/AITriggerType/Body.h>

#include <Utilities/Cast.h>

#include <AITriggerTypeClass.h>
#include <TaskForceClass.h>
#include <TeamTypeClass.h>

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

#include <TriggerTypeClass.h>

NOINLINE bool UpdateTeam(FakeHouseClass* pHouse, int delay)
{
	if (!FakeRulesClass::Instance()->NewTeamsSelector)
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
		bool splitTriggersByCategory = FakeRulesClass::Instance()->NewTeamsSelector_SplitTriggersByCategory;
		bool isFallbackEnabled = FakeRulesClass::Instance()->NewTeamsSelector_EnableFallback;

		if (splitTriggersByCategory)
		{
			mergeUnclassifiedCategoryWith = pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.Get(FakeRulesClass::Instance()->NewTeamsSelector_MergeUnclassifiedCategoryWith);  // Should mixed teams be merged into another category?
			double percentageUnclassifiedTriggers = pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.Get(FakeRulesClass::Instance()->NewTeamsSelector_UnclassifiedCategoryPercentage); // Mixed teams
			double percentageGroundTriggers = pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.Get(FakeRulesClass::Instance()->NewTeamsSelector_GroundCategoryPercentage); // Only ground
			double percentageNavalTriggers = pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.Get(FakeRulesClass::Instance()->NewTeamsSelector_NavalCategoryPercentage); // Only Naval=yes
			double percentageAirTriggers = pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.Get(FakeRulesClass::Instance()->NewTeamsSelector_AirCategoryPercentage); // Only Aircrafts & jumpjets

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
;
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
					|| !FakeFootClass::_IsRecruitable(pFoot, discard_t(), pHouse))
				{
					continue;
				}

				++ownedRecruitables[GET_TECHNOTYPE(pTechno)];
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

			if (!pTrigger || ScenarioClass::Instance->IgnoreGlobalAITriggers == (bool)pTrigger->Type || !pTrigger->Team1)
				continue;

			// Ignore offensive teams if the next trigger must be defensive
			if (onlyPickDefensiveTeams && !pTrigger->IsForBaseDefense)
				continue;

			//remove
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
				if ((triggerHouse == -1 || houseTypeIdx == triggerHouse) && (triggerSide < 0 || sideTypeIdx == triggerSide))
				{
					
					auto ToRange = [](int value) {
						if (value < 0 || value > 19)
							return value; //just proceedit

						return value + 12; // convert the original AITriggerCondition tor fit phobos generalized one
					};

					int condType = ToRange((int)pTrigger->ConditionType);

					if(!AITriggerTypeExtData::CheckConditionType(pTrigger, (AITriggerCondition)condType, pHouse, targetHouse, true))
						continue;
					//

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

									if (pTechnoTypeExt->ConsideredNaval.Get( (entry.Type->Naval
											&& (entry.Type->MovementZone != MovementZone::Amphibious
												&& entry.Type->MovementZone != MovementZone::AmphibiousDestroyer
												&& entry.Type->MovementZone != MovementZone::AmphibiousCrusher))))
									{
										// This unit is from naval category
										entryIsCategory = TeamCategory::Naval;
										//Debug::LogInfo("\t[{}]({}) is in NAVAL category.", entry.Type->ID, entry.Amount);
									}

									if (pTechnoTypeExt->ConsideredVehicle.Get((entryIsCategory != TeamCategory::Naval
											&& entryIsCategory != TeamCategory::Air)))
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

TeamTypeClass *__fastcall Suggested_New_Team(TypeList<TeamTypeClass*> *possible_teams, HouseClass *house, bool alerted){
	JMP_FAST(0x6F0AB0)
}

#include <DiscreteDistributionClass.h>
#include <DiscreteSelectionClass.h>

std::vector<TeamTypeClass*> NOINLINE Suggested_New_Team(HouseClass* forHouse_, bool alerted)
{
	std::vector<TeamTypeClass*> suggestedTeams;

	HouseClass* pEnemy = nullptr;
	if (forHouse_->EnemyHouseIndex != -1)
		pEnemy = HouseClass::Array->Items[forHouse_->EnemyHouseIndex];

	const int Difficulty = static_cast<int>(forHouse_->AIDifficulty);
	const int teamCapValue = RulesClass::Instance->TotalAITeamCap.Items[Difficulty];
	suggestedTeams.reserve(teamCapValue);

	const bool triggerRollPassed =
		ScenarioClass::Instance->Random.RandomRanged(1, 100) <= forHouse_->RatioAITriggerTeam
		&& forHouse_->AITriggersActive;

	if (triggerRollPassed) {
		// --- OPTIMISATION 1: single pass replaces two separate TeamClass loops ---
		int        counter = 0;
		int        baseDefenseCount = 0;
		TeamClass* oldestDefenseTeam = nullptr;
		int        oldestCreationFrame = 0x7FFFFFFF;

		for (int i = 0; i < TeamClass::Array->Count; ++i) {
			auto* team = TeamClass::Array->Items[i];
			if (team->OwnerHouse != forHouse_)
				continue;

			++counter;
			if (team->Type->IsBaseDefense) {
				++baseDefenseCount;
				if (team->CreationFrame < oldestCreationFrame) {
					oldestDefenseTeam = team;
					oldestCreationFrame = team->CreationFrame;
				}
			}
		}
		// --- end single pass ---

		bool skip = false;

		if (counter < teamCapValue || baseDefenseCount < counter / 2) {
			if (baseDefenseCount > RulesClass::Instance->MaximumAIDefensiveTeams.Items[Difficulty])
				skip = true;
		} else {
			if (oldestDefenseTeam) {
				--counter;
				skip = true;
				oldestDefenseTeam->_scalar_dtor(1);
			}
		}

		if (counter < teamCapValue) {
			DiscreteDistribution<AITriggerTypeClass*> triggerDistribution;
			bool foundMaxWeight = false;

			for (int i = 0; i < AITriggerTypeClass::Array->Count; ++i)
			{
				auto* triggerType = AITriggerTypeClass::Array->Items[i];
				if (!triggerType || ((FakeAITriggerTypeClass*)triggerType)->_NewTeam(forHouse_, pEnemy, skip) != 1)
					continue;

				const unsigned int weight = static_cast<unsigned int>(triggerType->Weight_Current);

				if (weight == 5000) {
					if (!foundMaxWeight) {
						foundMaxWeight = true;
						triggerDistribution.clear();
					}
				}
				else if (foundMaxWeight) {
					continue;
				}

				triggerDistribution.add(triggerType, weight);
			}

			AITriggerTypeClass* selectedTrigger = nullptr;
			triggerDistribution.select(ScenarioClass::Instance->Random, &selectedTrigger);

			if (selectedTrigger) {
				if (auto* teamType1 = selectedTrigger->Team1)
					suggestedTeams.push_back(teamType1);
				if (auto* teamType2 = selectedTrigger->Team2)
					suggestedTeams.push_back(teamType2);
			}
		}
	}

	// --- OPTIMISATION 2: set for O(1) duplicate check ---
	// suggestedTeams has at most 2 entries today; set construction cost is negligible.
	if (!suggestedTeams.empty()) {
		const std::unordered_set<TeamTypeClass*> suggestedSet(
			suggestedTeams.begin(), suggestedTeams.end());

		for (int i = 0; i < TeamClass::Array->Count; ++i) {
			auto* team = TeamClass::Array->Items[i];
			if (team->OwnerHouse == forHouse_ && (team->IsReforming || !team->IsMoving)) {
				if (suggestedSet.count(team->Type)) {
					suggestedTeams.clear();
					return suggestedTeams;
				}
			}
		}

		for (auto* suggested : suggestedTeams)
			suggested->Autocreate = 1;
	}

	return suggestedTeams;
}

 ASMJIT_PATCH(0x4F8A63, HouseClass_AI_Team , 7) {
 	GET(FakeHouseClass* , pThis , ESI);

 	auto pHouseExt = pThis->_GetExtData();
 	int delay = pHouseExt->TeamDelay;

	if(delay < 0){
		int playerCount = ScenarioClass::Instance->NumberStartingPoints;
		auto rulesExt = FakeRulesClass::Instance();

		if (playerCount >= 2 && !SessionClass::IsCampaign()) {
			const auto teamDelayType = rulesExt->TeamDelays_DynamicType;

			if (teamDelayType != DynamicTeamDelayType::StartingPoint) {
				playerCount = 0;
				const bool byAlivePlayers = teamDelayType == DynamicTeamDelayType::AliveCount
						|| teamDelayType == DynamicTeamDelayType::AliveAllies
						|| teamDelayType == DynamicTeamDelayType::AliveEnemies;
				const bool checkAllies = teamDelayType == DynamicTeamDelayType::Allies
						|| teamDelayType == DynamicTeamDelayType::AliveAllies;
				const bool checkEnemies = teamDelayType == DynamicTeamDelayType::Enemies
						|| teamDelayType == DynamicTeamDelayType::AliveEnemies;

				for (auto const pHouse : *HouseClass::Array) {
					if ((!byAlivePlayers || !pHouse->Defeated)
						&& !pHouse->IsObserver()
						&& !pHouse->Type->MultiplayPassive
						&& (!checkAllies || (pThis != pHouse && pThis->IsAlliedWith(pHouse)))
						&& (!checkEnemies || !pThis->IsAlliedWith(pHouse))) {
						playerCount += 1;
					}
				}
			}
		}

		if(playerCount < 1 || playerCount > 8)
			delay = (rulesExt->MultipleTeamDelays[playerCount - 1].Get())[pThis->GetAIDifficultyIndex()];
		else
			delay = 0;
	}

 	if(!UpdateTeam(pThis, delay)){

 		std::vector<TeamTypeClass*> possible_teams = Suggested_New_Team(pThis, false);
 		//Debug::LogInfo("[{} - {}] Able to use {} team !", pThis->Type->ID, (void*)pThis, possible_teams.size());

 		for(size_t i = 0; i < possible_teams.size(); ++i){
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
		 //int houseIdx = pHouse->ArrayIndex;

		 int parentCountrySideTypeIdx = parentCountryTypeIdx >= 0 ? pHouse->Type->FindParentCountry()->SideIndex : pHouse->Type->SideIndex;
		 int sideTypeIdx = parentCountrySideTypeIdx >= 0 ? parentCountrySideTypeIdx + 1 : pHouse->Type->SideIndex + 1; // Side indexes in AITriggers section are 1-based
		 //int sideIdx = pHouse->SideIndex + 1; // Side indexes in AITriggers section are 1-based -> unused variable!!
		 auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

		 pHouseExt->AITriggers_ValidList.clear();
		 pHouseExt->AITriggers_ValidList.reserve(AITriggerTypeClass::Array->Count);

		 for (int i = 0; i < AITriggerTypeClass::Array->Count; i++)
		 {
			 if (auto pTrigger = AITriggerTypeClass::Array->Items[i])
			 {
				 if (!pTrigger || (ignoreGlobalAITriggers && pTrigger->Type == AITriggerType::Global && !pTrigger->IsEnabled) || !pTrigger->Team1)
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