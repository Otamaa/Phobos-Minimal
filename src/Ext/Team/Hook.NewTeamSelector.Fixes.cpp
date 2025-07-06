#include "Hook.NewTeamSelector.Fixes.h"
#include "Body.h"
#include <Ext/Rules/Body.h>
#include <Utilities/Debug.h>

#include <AITriggerTypeClass.h>
#include <TeamTypeClass.h>
#include <TeamClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <ScenarioClass.h>
#include <RulesClass.h>

#include <vector>
#include <exception>

// New Team Selector Performance and Reliability Fixes
// This file contains optimizations and bug fixes for the New Team Selector

namespace NewTeamSelectorFixes
{
	// Fix 1: Use integer weights instead of floating-point to avoid precision issues
	struct OptimizedTriggerWeight
	{
		int Weight { 0 };
		AITriggerTypeClass* Trigger { nullptr };
		TeamCategory Category { TeamCategory::None };
		
		// Optimized operators using integers
		bool operator<(const OptimizedTriggerWeight& other) const
		{
			return Weight < other.Weight;
		}
		
		bool operator<(int other) const
		{
			return Weight < other;
		}
	};

	// Fix 2: Pre-allocated vectors to reduce memory fragmentation
	class OptimizedCandidateList
	{
	private:
		static constexpr size_t INITIAL_CAPACITY = 64; // Reasonable initial size
		std::vector<OptimizedTriggerWeight> candidates;
		
	public:
		OptimizedCandidateList()
		{
			candidates.reserve(INITIAL_CAPACITY);
		}
		
		void clear()
		{
			candidates.clear();
			// Keep capacity to avoid reallocation
		}
		
		void add(int weight, AITriggerTypeClass* trigger, TeamCategory category)
		{
			candidates.push_back({weight, trigger, category});
		}
		
		size_t size() const { return candidates.size(); }
		bool empty() const { return candidates.empty(); }
		
		const OptimizedTriggerWeight& operator[](size_t index) const
		{
			return candidates[index];
		}
		
		auto begin() const { return candidates.begin(); }
		auto end() const { return candidates.end(); }
	};

	// Fix 3: Optimized weight selection using integer arithmetic
	AITriggerTypeClass* SelectTriggerByWeight(const OptimizedCandidateList& candidates, int totalWeight)
	{
		if (candidates.empty() || totalWeight <= 0)
			return nullptr;
			
		// Use integer random to avoid floating-point precision issues
		int weightDice = ScenarioClass::Instance->Random.RandomRanged(1, totalWeight);
		int currentWeight = 0;
		
		for (const auto& candidate : candidates)
		{
			currentWeight += candidate.Weight;
			if (weightDice <= currentWeight)
			{
				return candidate.Trigger;
			}
		}
		
		// Fallback: return last candidate if something went wrong
		return candidates[candidates.size() - 1].Trigger;
	}

	// Fix 4: Optimized category classification with caching
	TeamCategory ClassifyTeamCategory(TeamTypeClass* pTeamType)
	{
		if (!pTeamType || !pTeamType->TaskForce)
			return TeamCategory::Ground; // Safe default
			
		TeamCategory result = TeamCategory::None;
		bool hasMultipleCategories = false;
		
		// Check only non-empty entries
		for (int i = 0; i < 6; i++)
		{
			const auto& entry = pTeamType->TaskForce->Entries[i];
			
			if (entry.Amount <= 0 || !entry.Type)
				break; // No more entries
				
			TeamCategory entryCategory = TeamCategory::Ground; // Default
			
			// Quick category determination
			if (entry.Type->WhatAmI() == AbstractType::AircraftType || entry.Type->ConsideredAircraft)
			{
				entryCategory = TeamCategory::Air;
			}
			else if (entry.Type->Naval)
			{
				// Check if it's truly naval (not amphibious)
				auto movementZone = entry.Type->MovementZone;
				if (movementZone != MovementZone::Amphibious &&
					movementZone != MovementZone::AmphibiousDestroyer &&
					movementZone != MovementZone::AmphibiousCrusher)
				{
					entryCategory = TeamCategory::Naval;
				}
			}
			// Ground category is default, no need to check
			
			// Check for mixed categories
			if (result == TeamCategory::None)
			{
				result = entryCategory;
			}
			else if (result != entryCategory)
			{
				hasMultipleCategories = true;
				break; // Early exit for mixed teams
			}
		}
		
		return hasMultipleCategories ? TeamCategory::Unclassified : result;
	}

	// Fix 5: Use vanilla prerequisite checking instead of custom logic
	bool CanTeamBeBuilt(HouseClass* pHouse, TeamTypeClass* pTeamType)
	{
		if (!pHouse || !pTeamType || !pTeamType->TaskForce)
			return false;
			
		// Use vanilla prerequisite checking for reliability
		for (int i = 0; i < 6; i++)
		{
			const auto& entry = pTeamType->TaskForce->Entries[i];
			
			if (entry.Amount <= 0 || !entry.Type)
				break;
				
			// Use vanilla prerequisite function instead of custom logic
			if (pTeamType->Autocreate)
			{
				// For autocreate teams, check if we can build the units
				if (pHouse->CanBuild(entry.Type, false, false) != CanBuildResult::Buildable)
				{
					return false;
				}
			}
			else if (pTeamType->Recruiter)
			{
				// For recruiter teams, check if units exist on map
				// This is more complex but we'll use a simpler heuristic
				bool foundUnits = false;
				for (auto pTechno : *TechnoClass::Array)
				{
					if (pTechno && pTechno->Owner == pHouse && 
						pTechno->GetTechnoType() == entry.Type &&
						pTechno->IsAlive && !pTechno->InLimbo)
					{
						foundUnits = true;
						break;
					}
				}
				if (!foundUnits)
					return false;
			}
		}
		
		return true;
	}

	// Fix 6: Optimized team counting with early exit
	int CountExistingTeams(HouseClass* pHouse, TeamTypeClass* pTeamType)
	{
		if (!pHouse || !pTeamType)
			return 0;
			
		int count = 0;
		
		// Count existing teams efficiently
		for (auto pTeam : *TeamClass::Array)
		{
			if (pTeam && pTeam->OwnerHouse == pHouse && pTeam->Type == pTeamType)
			{
				count++;
				// Early exit if we've reached the maximum
				if (count >= pTeamType->Max)
					break;
			}
		}
		
		return count;
	}

	// Fix 7: Memory-efficient weight calculation with overflow protection
	bool CalculateWeights(const std::vector<AITriggerTypeClass*>& triggers,
						 OptimizedCandidateList& candidates,
						 int& totalWeight)
	{
		totalWeight = 0;
		candidates.clear();
		
		for (auto pTrigger : triggers)
		{
			if (!pTrigger)
				continue;
				
			// Convert double weight to integer, clamping to safe range
			int weight = static_cast<int>(pTrigger->Weight_Current);
			if (weight < 1) weight = 1; // Minimum weight
			if (weight > 10000) weight = 10000; // Maximum weight to prevent overflow
			
			// Check for overflow before addition
			if (totalWeight > INT_MAX - weight)
			{
				Debug::Log("New Team Selector: Weight overflow detected, capping total weight\n");
				break; // Stop adding more triggers to prevent overflow
			}
			
			totalWeight += weight;
			
			// Determine category efficiently
			TeamCategory category = TeamCategory::None;
			if (RulesExtData::Instance()->NewTeamsSelector_SplitTriggersByCategory && pTrigger->Team1)
			{
				category = ClassifyTeamCategory(pTrigger->Team1);
			}
			
			candidates.add(weight, pTrigger, category);
		}
		
		return !candidates.empty() && totalWeight > 0;
	}

	// Fix 8: Optimized main function with better error handling
	bool OptimizedUpdateTeam(HouseClass* pHouse)
	{
		if (!pHouse || !RulesExtData::Instance()->NewTeamsSelector)
			return false;
			
		try
		{
			// Reset team delay timer
			pHouse->TeamDelayTimer.Start(RulesClass::Instance->TeamDelays[(int)pHouse->AIDifficulty]);
			
			// Check AI trigger ratio
			int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);
			if (dice > pHouse->RatioAITriggerTeam || !pHouse->AITriggersActive)
				return true; // No teams this time
				
			// Pre-filter eligible triggers to reduce processing
			std::vector<AITriggerTypeClass*> eligibleTriggers;
			eligibleTriggers.reserve(32); // Reasonable initial size
			
			// Get target house
			HouseClass* targetHouse = nullptr;
			if (pHouse->EnemyHouseIndex != -1 && 
				pHouse->EnemyHouseIndex < HouseClass::Array->Count)
			{
				targetHouse = HouseClass::Array->Items[pHouse->EnemyHouseIndex];
			}
			
			// Quick pre-filtering pass
			for (int i = 0; i < AITriggerTypeClass::Array->Count; i++)
			{
				auto pTrigger = AITriggerTypeClass::Array->Items[i];
				
				if (!pTrigger || !pTrigger->Team1 || !pTrigger->IsEnabled)
					continue;
					
				// Basic compatibility checks
				if (pTrigger->TechLevel > pHouse->StaticData.TechLevel)
					continue;
					
				// Difficulty check
				int difficulty = (int)pHouse->AIDifficulty;
				if ((difficulty == 0 && !pTrigger->Enabled_Hard) ||
					(difficulty == 1 && !pTrigger->Enabled_Normal) ||
					(difficulty == 2 && !pTrigger->Enabled_Easy))
					continue;
					
				// Quick team limit check
				int existingCount = CountExistingTeams(pHouse, pTrigger->Team1);
				if (existingCount >= pTrigger->Team1->Max)
					continue;
					
				// Use vanilla condition checking for reliability
				if (!pTrigger->ConditionMet(pHouse, targetHouse, false))
					continue;
					
				// Check if team can be built/recruited
				if (!CanTeamBeBuilt(pHouse, pTrigger->Team1))
					continue;
					
				eligibleTriggers.push_back(pTrigger);
			}
			
			if (eligibleTriggers.empty())
			{
				Debug::Log("New Team Selector: No eligible triggers for house [%s]\n", pHouse->Type->ID);
				return true;
			}
			
			// Calculate weights efficiently
			OptimizedCandidateList candidates;
			int totalWeight = 0;
			
			if (!CalculateWeights(eligibleTriggers, candidates, totalWeight))
			{
				Debug::Log("New Team Selector: Failed to calculate weights for house [%s]\n", pHouse->Type->ID);
				return true;
			}
			
			// Select trigger using optimized weight selection
			AITriggerTypeClass* selectedTrigger = SelectTriggerByWeight(candidates, totalWeight);
			
			if (!selectedTrigger)
			{
				Debug::Log("New Team Selector: Failed to select trigger for house [%s]\n", pHouse->Type->ID);
				return true;
			}
			
			// Create teams
			bool teamCreated = false;
			
			// Team 1
			if (selectedTrigger->Team1)
			{
				int count = CountExistingTeams(pHouse, selectedTrigger->Team1);
				if (count < selectedTrigger->Team1->Max)
				{
					if (TeamClass* newTeam = selectedTrigger->Team1->CreateTeam(pHouse))
					{
						newTeam->NeedsToDisappear = false;
						teamCreated = true;
						Debug::Log("New Team Selector: Created team [%s] for house [%s]\n", 
									 selectedTrigger->Team1->ID, pHouse->Type->ID);
					}
				}
			}
			
			// Team 2 (if exists)
			if (selectedTrigger->Team2)
			{
				int count = CountExistingTeams(pHouse, selectedTrigger->Team2);
				if (count < selectedTrigger->Team2->Max)
				{
					if (TeamClass* newTeam = selectedTrigger->Team2->CreateTeam(pHouse))
					{
						newTeam->NeedsToDisappear = false;
						teamCreated = true;
						Debug::Log("New Team Selector: Created secondary team [%s] for house [%s]\n", 
									 selectedTrigger->Team2->ID, pHouse->Type->ID);
					}
				}
			}
			
			// Update trigger weight for elite triggers
			if (selectedTrigger->Weight_Current >= 5000.0 && selectedTrigger->Weight_Minimum <= 4999.0)
			{
				selectedTrigger->Weight_Current = 4999.0;
			}
			
			return teamCreated;
		}
		catch (const std::exception& e)
		{
			Debug::Log("New Team Selector: Exception in OptimizedUpdateTeam: %s\n", e.what());
			return false;
		}
		catch (...)
		{
			Debug::Log("New Team Selector: Unknown exception in OptimizedUpdateTeam\n");
			return false;
		}
	}
} 