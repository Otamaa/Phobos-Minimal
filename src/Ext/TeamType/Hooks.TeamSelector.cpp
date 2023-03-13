#include <Ext/Script/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Rules/Body.h>

// https://github.com/Phobos-developers/Phobos/pull/810
// Todo : Otamaa : Warning ! , Performance heavy 

enum TeamCategory
{
	None = 0, // No category. Should be default value
	Ground = 1,
	Air = 2,
	Naval = 3,
	Unclassified = 4
};

struct TriggerElementWeight
{
	double Weight = 0.0;
	AITriggerTypeClass* Trigger = nullptr;
	TeamCategory Category = TeamCategory::None;

	//need to define a == operator so it can be used in array classes
	bool operator==(const TriggerElementWeight& other) const
	{
		return (Trigger == other.Trigger && Weight == other.Weight && Category == other.Category);
	}

	//unequality
	bool operator!=(const TriggerElementWeight& other) const
	{
		return (Trigger != other.Trigger || Weight != other.Weight || Category == other.Category);
	}

	bool operator<(const TriggerElementWeight& other) const
	{
		return (Weight < other.Weight);
	}

	bool operator<(const double other) const
	{
		return (Weight < other);
	}

	bool operator>(const TriggerElementWeight& other) const
	{
		return (Weight > other.Weight);
	}

	bool operator>(const double other) const
	{
		return (Weight > other);
	}

	bool operator==(const double other) const
	{
		return (Weight == other);
	}

	bool operator!=(const double other) const
	{
		return (Weight != other);
	}
};

DEFINE_HOOK(0x4F8A27, TeamTypeClass_SuggestedNewTeam_NewTeamsSelector, 0x5)
{
	enum { UseOriginalSelector = 0x4F8A63, SkipCode = 0x4F8B08 };

	GET(HouseClass*, pHouse, ESI);

	const bool houseIsHuman = SessionClass::IsCampaign() ?
		pHouse->IsHumanPlayer || pHouse->IsInPlayerControl : pHouse->IsHumanPlayer;

	if (houseIsHuman || pHouse->Type->MultiplayPassive)
		return SkipCode;

	const auto pHouseTypeExt = HouseTypeExt::ExtMap.Find(pHouse->Type);

	if (!pHouseTypeExt)
		return SkipCode;

	auto const pRulexExt = RulesExt::Global();

	if (!pRulexExt->NewTeamsSelector)
		return UseOriginalSelector;

	// Reset Team selection countdown
	const int countdown = RulesClass::Instance->TeamDelays[(int)pHouse->AIDifficulty];
	pHouse->TeamDelayTimer.Start(countdown);

	int totalActiveTeams = 0;
	int activeTeams = 0;

	int totalGroundCategoryTriggers = 0;
	int totalUnclassifiedCategoryTriggers = 0;
	int totalNavalCategoryTriggers = 0;
	int totalAirCategoryTriggers = 0;

	std::vector<TriggerElementWeight> validTriggerCandidates;
	std::vector<TriggerElementWeight> validTriggerCandidatesGroundOnly;
	std::vector<TriggerElementWeight> validTriggerCandidatesNavalOnly;
	std::vector<TriggerElementWeight> validTriggerCandidatesAirOnly;
	std::vector<TriggerElementWeight> validTriggerCandidatesUnclassifiedOnly;

	const int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	// This house must have the triggers enabled
	if (dice <= pHouse->RatioAITriggerTeam && pHouse->AITriggersActive)
	{
		bool splitTriggersByCategory = pRulexExt->NewTeamsSelector_SplitTriggersByCategory;
		bool isFallbackEnabled = pRulexExt->NewTeamsSelector_EnableFallback;
		TeamCategory validCategory = TeamCategory::None;
		int mergeUnclassifiedCategoryWith = -1;

		double percentageUnclassifiedTriggers = 0.0;
		double percentageGroundTriggers = 0.0;
		double percentageNavalTriggers = 0.0;
		double percentageAirTriggers = 0.0;

		if (splitTriggersByCategory)
		{
			mergeUnclassifiedCategoryWith = pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.isset() ? pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.Get() : RulesExt::Global()->NewTeamsSelector_MergeUnclassifiedCategoryWith;  // Should mixed teams be merged into another category?
			percentageUnclassifiedTriggers = pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_UnclassifiedCategoryPercentage; // Mixed teams
			percentageGroundTriggers = pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_GroundCategoryPercentage; // Only ground
			percentageNavalTriggers = pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_NavalCategoryPercentage; // Only Naval=yes
			percentageAirTriggers = pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_AirCategoryPercentage; // Only Aircrafts & jumpjets

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

			// Note: if the sum of all percentages is less than 100% then that empty space will work like "no categories"
			if (splitTriggersByCategory)
			{
				const int categoryDice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

				if (categoryDice == 0)
				{
					splitTriggersByCategory = false;
				}
				else if (categoryDice <= (int)(percentageUnclassifiedTriggers * 100.0))
				{
					validCategory = TeamCategory::Unclassified;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers) * 100.0))
				{
					validCategory = TeamCategory::Ground;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers) * 100.0))
				{
					validCategory = TeamCategory::Naval;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers) * 100.0))
				{
					validCategory = TeamCategory::Air;
				}
				else
				{
					splitTriggersByCategory = false;
				}
			}
		}

		int houseIdx = pHouse->ArrayIndex;
		int sideIdx = pHouse->SideIndex + 1;
		auto houseDifficulty = pHouse->AIDifficulty;
		int maxBaseDefenseTeams = RulesClass::Instance->MaximumAIDefensiveTeams.GetItem((int)houseDifficulty);
		int activeDefenseTeamsCount = 0;
		int maxTeamsLimit = RulesClass::Instance->TotalAITeamCap.GetItem((int)houseDifficulty);
		double totalWeight = 0.0;
		double totalWeightGroundOnly = 0.0;
		double totalWeightNavalOnly = 0.0;
		double totalWeightAirOnly = 0.0;
		double totalWeightUnclassifiedOnly = 0.0;

		// Check if the running teams by the house already reached all the limits
		auto& activeTeamsList = HouseExt::ExtMap.Find(pHouse)->ActiveTeams;

		for (auto const pRunningTeam : *TeamClass::Array)
		{
			totalActiveTeams++;

			int teamHouseIdx = pRunningTeam->Owner->ArrayIndex;

			if (teamHouseIdx != houseIdx)
				continue;

			activeTeamsList.push_back(pRunningTeam);

			if (pRunningTeam->Type->IsBaseDefense && activeDefenseTeamsCount < maxBaseDefenseTeams)
				activeDefenseTeamsCount++;
		}

		activeTeams = activeTeamsList.size();

		// We will use these values for discarding triggers
		bool hasReachedMaxTeamsLimit = activeTeams < maxTeamsLimit ? false : true;
		bool hasReachedMaxDefensiveTeamsLimit = activeDefenseTeamsCount < maxBaseDefenseTeams ? false : true;

		if (hasReachedMaxDefensiveTeamsLimit)
			Debug::Log("__FUNCTION__ : House [%s] (idx: %d) reached the MaximumAIDefensiveTeams value!\n", pHouse->Type->ID, pHouse->ArrayIndex);

		if (hasReachedMaxTeamsLimit)
		{
			Debug::Log("__FUNCTION__ : House [%s] (idx: %d) reached the TotalAITeamCap value!\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		// Obtain the real list of structures the house have
		ValueableVector<BuildingTypeClass*> ownedBuildingTypes;
		for (auto const& building : pHouse->Buildings)
		{

			if (!ownedBuildingTypes.Contains(building->Type))
				ownedBuildingTypes.push_back(building->Type);
		}

		struct recruitableUnit
		{
			TechnoTypeClass* object = nullptr;
			int count = 1;

			bool operator==(const TechnoTypeClass* other) const
			{
				return (object == other);
			}

			bool operator==(const recruitableUnit other) const
			{
				return (object == other.object);
			}
		};

		// Build a list of recruitable units by the house
		std::vector<recruitableUnit> recruitableUnits;

		for (auto pFoot : *FootClass::Array)
		{
			//if (pTechno->WhatAmI() == AbstractType::Building)
			//	continue;

			//FootClass* pFoot = static_cast<FootClass*>(pTechno);

			if (!TechnoExt::IsActive(pFoot))
				continue;

			if (!pFoot->CanBeRecruited(pHouse))
				continue;

			auto pTechnoType = pFoot->GetTechnoType();
			bool found = false;

			for (int i = 0; i < (int)recruitableUnits.size(); i++)
			{
				if (recruitableUnits[i].object == pTechnoType)
				{
					recruitableUnits[i].count++;
					found = true;
					break;
				}
			}

			if (!found)
			{
				recruitableUnit newRecruitable;
				newRecruitable.object = pTechnoType;
				recruitableUnits.push_back(newRecruitable);
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

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// Ignore the deactivated triggers
			if (pTrigger->IsEnabled)
			{
				// The trigger must be compatible with the owner
				if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
				{
					// "ConditionType=-1" will be skipped, always is valid
					if ((int)pTrigger->ConditionType >= 0)
					{
						auto const& nAIList = RulesExt::Global()->AITargetTypesLists;

						if ((int)pTrigger->ConditionType == 0)
						{
							// Simulate case 0: "enemy owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, true, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 1)
						{
							// Simulate case 1: "house owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 7)
						{
							// Simulate case 7: "civilian owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::NeutralOwns(pTrigger, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 8)
						{
							// Simulate case 0: "enemy owns" but instead of restrict it against the main enemy house it is done against all enemies
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 9)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 9: Like in case 0 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the enemy.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 10)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 10: Like in case 1 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the house.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 11)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 11: Like in case 7 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the Civilians.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::NeutralOwns(pTrigger, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 12)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 12: Like in case 0 & 9 but instead of a specific enemy this checks in all enemies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 13)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 13: Like in case 1 & 10 but instead checking the house now checks the allies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, true, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 14)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 14: Like in case 9 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::EnemyOwnsAll(pTrigger, pHouse, targetHouse, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 15)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 15: Like in case 10 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::HouseOwnsAll(pTrigger, pHouse, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 16)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 16: Like in case 11 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[(pTrigger->Conditions[3].ComparatorOperand)];
								bool isConditionMet = TeamExt::NeutralOwnsAll(pTrigger, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 17)
						{
							if (!nAIList.empty() && pTrigger->Conditions[3].ComparatorOperand < (int)nAIList.size())
							{
								// New case 17: Like in case 14 but instead of meet any comparison now is required all. Check all enemies
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								auto const& list = nAIList[pTrigger->Conditions[3].ComparatorOperand];
								bool isConditionMet = TeamExt::EnemyOwnsAll(pTrigger, pHouse, nullptr, list);

								if (!isConditionMet)
									continue;
							}
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

					for (auto const& team : activeTeamsList)
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
							auto& entry = pTriggerTeam1Type->TaskForce->Entries[i];
							TeamCategory entryIsCategory = TeamCategory::Ground;

							if (entry.Amount > 0)
							{
								if (entry.Type->WhatAmI() == AbstractType::AircraftType
									|| entry.Type->ConsideredAircraft)
								{
									// This unit is from air category
									entryIsCategory = TeamCategory::Air;
									//Debug::Log("\t[%s](%d) is in AIR category.\n", entry.Type->ID, entry.Amount);
								}
								else
								{
									const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(entry.Type);

									if (!pTechnoTypeExt)
										continue;

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
						for (auto const& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce meets the structure prerequisites
							if (entry.Amount > 0)
							{
								if (!entry.Type)
									continue;

								TechnoTypeClass* object = entry.Type;
								bool canBeBuilt = HouseExt::PrerequisitesMet(pHouse, object, ownedBuildingTypes);

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

						for (auto const& entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce has the available recruitable units in the map
							if (allObjectsCanBeBuiltOrRecruited && entry.Amount > 0)
							{
								bool canBeRecruited = false;

								for (auto const& item : recruitableUnits)
								{
									if (item.object == entry.Type)
									{
										if (item.count >= entry.Amount)
											canBeRecruited = true;

										break;
									}
								}

								if (!canBeRecruited)
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
						if (!validTriggerCandidates.empty())
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
						TriggerElementWeight itemGroundOnly;
						TriggerElementWeight itemAirOnly;
						TriggerElementWeight itemNavalOnly;
						TriggerElementWeight itemUnclassifiedOnly;

						switch (teamIsCategory)
						{
						case TeamCategory::Ground:
							totalWeightGroundOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemGroundOnly.Trigger = pTrigger;
							itemGroundOnly.Weight = totalWeightGroundOnly;
							itemGroundOnly.Category = teamIsCategory;

							validTriggerCandidatesGroundOnly.push_back(itemGroundOnly);
							totalGroundCategoryTriggers++;
							break;

						case TeamCategory::Air:
							totalWeightAirOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemAirOnly.Trigger = pTrigger;
							itemAirOnly.Weight = totalWeightAirOnly;
							itemAirOnly.Category = teamIsCategory;

							validTriggerCandidatesAirOnly.push_back(itemAirOnly);
							totalAirCategoryTriggers++;
							break;

						case TeamCategory::Naval:
							totalWeightNavalOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemNavalOnly.Trigger = pTrigger;
							itemNavalOnly.Weight = totalWeightNavalOnly;
							itemNavalOnly.Category = teamIsCategory;

							validTriggerCandidatesNavalOnly.push_back(itemNavalOnly);
							totalNavalCategoryTriggers++;
							break;

						case TeamCategory::Unclassified:
							totalWeightUnclassifiedOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemUnclassifiedOnly.Trigger = pTrigger;
							itemUnclassifiedOnly.Weight = totalWeightUnclassifiedOnly;
							itemUnclassifiedOnly.Category = teamIsCategory;

							validTriggerCandidatesUnclassifiedOnly.push_back(itemUnclassifiedOnly);
							totalUnclassifiedCategoryTriggers++;
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
				Debug::Log("__FUNCTION__ : This time only will be picked GROUND teams.\n");
				break;

			case TeamCategory::Unclassified:
				Debug::Log("__FUNCTION__: This time only will be picked MIXED teams.\n");
				break;

			case TeamCategory::Naval:
				Debug::Log("__FUNCTION__: This time only will be picked NAVAL teams.\n");
				break;

			case TeamCategory::Air:
				Debug::Log("__FUNCTION__: This time only will be picked AIR teams.\n");
				break;

			default:
				Debug::Log("__FUNCTION__: This time teams categories are DISABLED.\n");
				break;
			}
		}

		if (validTriggerCandidates.empty())
		{
			Debug::Log("__FUNCTION__: [%s] (idx: %d) No valid triggers for now. A new attempt will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		if ((validCategory == TeamCategory::Ground && totalGroundCategoryTriggers == 0)
			|| (validCategory == TeamCategory::Unclassified && totalUnclassifiedCategoryTriggers == 0)
			|| (validCategory == TeamCategory::Air && totalAirCategoryTriggers == 0)
			|| (validCategory == TeamCategory::Naval && totalNavalCategoryTriggers == 0))
		{
			Debug::Log("__FUNCTION__: [%s] (idx: %d) No valid triggers of this category. A new attempt should be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);

			if (!isFallbackEnabled)
				return SkipCode;

			Debug::Log("__FUNCTION__ : ... BUT fallback mode is enabled so now will be checked all available triggers.\n");
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

			for (auto const& element : validTriggerCandidates)
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

			for (auto const& element : validTriggerCandidatesGroundOnly)
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

			for (auto const& element : validTriggerCandidatesUnclassifiedOnly)
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

			for (auto const& element : validTriggerCandidatesNavalOnly)
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

			for (auto const& element : validTriggerCandidatesAirOnly)
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
			Debug::Log("__FUNCTION__ : House [%s] (idx: %d) failed to select Trigger. A new attempt Will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		if (selectedTrigger->Weight_Current >= 5000.0
			&& selectedTrigger->Weight_Minimum <= 4999.0)
		{
			// Next time this trigger will be out of the elitist triggers list
			selectedTrigger->Weight_Current = 4999.0;
		}

		// We have a winner trigger here
		Debug::Log("__FUNCTION__: House [%s] (idx: %d) selected trigger [%s].\n", pHouse->Type->ID, pHouse->ArrayIndex, selectedTrigger->ID);

		// Team 1 creation
		auto pTriggerTeam1Type = selectedTrigger->Team1;
		if (pTriggerTeam1Type)
		{
			int count = 0;

			for (auto const& team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam1Type)
					count++;
			}

			if (count < pTriggerTeam1Type->Max)
			{
				if (auto newTeam = pTriggerTeam1Type->CreateTeam(pHouse))
					newTeam->NeedsToDisappear = false;
			}
		}

		// Team 2 creation (if set)
		auto pTriggerTeam2Type = selectedTrigger->Team2;
		if (pTriggerTeam2Type)
		{
			int count = 0;

			for (auto const& team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam2Type)
					count++;
			}

			if (count < pTriggerTeam2Type->Max)
			{
				if (auto newTeam = pTriggerTeam2Type->CreateTeam(pHouse))
					newTeam->NeedsToDisappear = false;
			}
		}
	}

	return SkipCode;
}
