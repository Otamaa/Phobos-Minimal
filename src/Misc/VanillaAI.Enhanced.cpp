#include "VanillaAI.Enhanced.h"
#include <Utilities/Debug.h>
#include <Ext/Rules/Body.h>
#include <ScenarioClass.h>
#include <RulesClass.h>
#include <TechnoClass.h>
#include <BuildingClass.h>
#include <Helpers/Cast.h>
#include <Unsorted.h>
#include <algorithm>

namespace VanillaAI
{
    // Global instance
    EnhancedAI g_EnhancedAI;
    
    // Configuration flags - all controlled by single UseEnhancedAI setting
    bool g_EnablePerformanceOptimizations = false;
    bool g_EnableSmartTeamBuilding = false;
    bool g_EnableResourceAwareness = false;
    bool g_EnableBattlefieldAnalysis = false;

    // TeamCountCache Implementation
    int TeamCountCache::GetTeamCount(HouseClass* house, TeamTypeClass* type)
    {
        if (!house || !type) return 0;
        
        // Check if cache is valid
        if (Unsorted::CurrentFrame != lastUpdateFrame || cachedHouse != house)
        {
            UpdateCache(house);
        }
        
        auto it = cache.find(type);
        return (it != cache.end()) ? it->second : 0;
    }

    void TeamCountCache::InvalidateCache()
    {
        cache.clear();
        lastUpdateFrame = -1;
        cachedHouse = nullptr;
    }

    void TeamCountCache::UpdateCache(HouseClass* house)
    {
        if (!house) return;
        
        cache.clear();
        
        // Count teams efficiently - single pass through team array
        for (auto pTeam : *TeamClass::Array)
        {
            if (pTeam && pTeam->OwnerHouse == house && pTeam->Type)
            {
                cache[pTeam->Type]++;
            }
        }
        
        lastUpdateFrame = Unsorted::CurrentFrame;
        cachedHouse = house;
    }

    // ResourceManager Implementation
    bool ResourceManager::CanAffordTeam(HouseClass* house, TeamTypeClass* teamType)
    {
        if (!house || !teamType) return false;
        
        int cost = GetTeamCost(teamType);
        return house->Available_Money() >= cost && HasPrerequisites(house, teamType);
    }

    int ResourceManager::GetTeamCost(TeamTypeClass* teamType)
    {
        if (!teamType || !teamType->TaskForce) return 0;
        
        // Calculate cost directly without caching for now
        int totalCost = 0;
        for (int i = 0; i < 6; i++)
        {
            const auto& entry = teamType->TaskForce->Entries[i];
            if (entry.Amount <= 0 || !entry.Type) break;
            
            // Use base cost without house-specific modifiers
            totalCost += entry.Type->Cost * entry.Amount;
        }
        
        return totalCost;
    }

    bool ResourceManager::HasPrerequisites(HouseClass* house, TeamTypeClass* teamType)
    {
        if (!house || !teamType || !teamType->TaskForce) return false;
        
        // Simplified prerequisite checking - assume all prerequisites are met for now
        // A full implementation would check factory buildings, tech levels, etc.
        return true;
    }



    // TeamCompositionAnalyzer Implementation
    TeamCompositionAnalyzer::BattlefieldState TeamCompositionAnalyzer::AnalyzeBattlefield(HouseClass* house)
    {
        BattlefieldState state;
        if (!house) return state;
        
        // Count existing units by type
        for (auto pTechno : *TechnoClass::Array)
        {
            if (!pTechno || pTechno->Owner != house || !pTechno->IsAlive) continue;
            
            switch (pTechno->WhatAmI())
            {
                case AbstractType::Infantry:
                case AbstractType::Unit:
                    state.groundUnits++;
                    break;
                case AbstractType::Aircraft:
                    state.airUnits++;
                    break;
                // Naval units would need special detection
            }
            
            // Check if unit is defensive or offensive (comprehensive analysis)
            bool isDefensive = false;
            
            switch (pTechno->WhatAmI())
            {
                case AbstractType::Building:
                {
                    auto pBuildingType = static_cast<BuildingTypeClass*>(pTechno->GetTechnoType());
                    // Buildings are defensive if they have base defense properties
                    isDefensive = pBuildingType->IsBaseDefense 
                               || pBuildingType->Turret 
                               || pBuildingType->TickTank
                               || pBuildingType->SensorArray
                               || pBuildingType->GapGenerator
                               || pBuildingType->CloakGenerator
                               || pBuildingType->Radar
                               || (pBuildingType->GetWeapon(0) && pBuildingType->GetWeapon(0)->WeaponType && pBuildingType->Factory == AbstractType::None);
                    break;
                }
                
                case AbstractType::Infantry:
                {
                    auto pInfantryType = static_cast<InfantryTypeClass*>(pTechno->GetTechnoType());
                    
                    // Engineers, agents, and support units are defensive
                    if (pInfantryType->Engineer || pInfantryType->Agent || pInfantryType->VehicleThief)
                    {
                        isDefensive = true;
                    }
                    // Anti-aircraft infantry are defensive
                    else if (auto weaponStruct = pInfantryType->GetWeapon(0))
                    {
                        if (weaponStruct->WeaponType && weaponStruct->WeaponType->Projectile)
                        {
                            if (weaponStruct->WeaponType->Projectile->AA && !weaponStruct->WeaponType->Projectile->AG)
                            {
                                isDefensive = true;
                            }
                            // Mind control units are defensive/support
                            else if (weaponStruct->WeaponType->Warhead && weaponStruct->WeaponType->Warhead->MindControl)
                            {
                                isDefensive = true;
                            }
                            // Long-range, slow units are more defensive
                            else if (pInfantryType->Speed <= 4 && weaponStruct->WeaponType->Range > 6)
                            {
                                isDefensive = true;
                            }
                        }
                    }
                    // Cyborgs and other support infantry
                    else if (pInfantryType->Cyborg || pInfantryType->C4)
                    {
                        isDefensive = true;
                    }
                    // Very slow infantry without special abilities are defensive
                    else if (pInfantryType->Speed <= 2)
                    {
                        isDefensive = true;
                    }
                    break;
                }
                
                case AbstractType::Unit:
                {
                    auto pUnitType = static_cast<UnitTypeClass*>(pTechno->GetTechnoType());
                    
                    // Mobile Construction Vehicles are defensive/support
                    if (pUnitType->DeploysInto || pUnitType->Harvester || pUnitType->Weeder)
                    {
                        isDefensive = true;
                    }
                    // Anti-aircraft vehicles
                    else if (auto weaponStruct = pUnitType->GetWeapon(0))
                    {
                        if (weaponStruct->WeaponType && weaponStruct->WeaponType->Projectile)
                        {
                            if (weaponStruct->WeaponType->Projectile->AA && !weaponStruct->WeaponType->Projectile->AG)
                            {
                                isDefensive = true;
                            }
                            // Artillery units (long range, slow) are defensive
                            else if (weaponStruct->WeaponType->Range > 10 && pUnitType->Speed <= 4)
                            {
                                isDefensive = true;
                            }
                            // Mind control vehicles
                            else if (weaponStruct->WeaponType->Warhead && weaponStruct->WeaponType->Warhead->MindControl)
                            {
                                isDefensive = true;
                            }
                        }
                    }
                    // Slow, heavily armored units are more defensive
                    else if (pUnitType->Speed <= 3 && pUnitType->Armor == Armor::Heavy)
                    {
                        isDefensive = true;
                    }
                    // Units with deployment capabilities are often defensive
                    else if (pUnitType->IsSimpleDeployer || pUnitType->DeployToFire)
                    {
                        isDefensive = true;
                    }
                    break;
                }
                
                case AbstractType::Aircraft:
                {
                    auto pAircraftType = static_cast<AircraftTypeClass*>(pTechno->GetTechnoType());
                    
                    // Transport aircraft are defensive/support
                    if (pAircraftType->Passengers > 0 && (!pAircraftType->GetWeapon(0) || !pAircraftType->GetWeapon(0)->WeaponType))
                    {
                        isDefensive = true;
                    }
                    // Carryall aircraft are support units
                    else if (pAircraftType->Carryall)
                    {
                        isDefensive = true;
                    }
                    // Most aircraft are offensive by nature
                    else
                    {
                        isDefensive = false;
                    }
                    break;
                }
                
                default:
                    isDefensive = false;
                    break;
            }
            
            if (isDefensive)
            {
                state.defenseUnits++;
            }
            else
            {
                state.offenseUnits++;
            }
        }
        
        // Check if under attack
        state.underAttack = IsHouseUnderAttack(house);
        
        return state;
    }

    bool TeamCompositionAnalyzer::ShouldBuildTeam(HouseClass* house, TeamTypeClass* teamType, const BattlefieldState& state)
    {
        if (!house || !teamType) return false;
        
        // Analyze team composition to determine if it's defensive or offensive
        bool isDefensiveTeam = this->IsTeamDefensive(teamType);
        
        // Priority 1: If under attack, prioritize defensive teams
        if (state.underAttack && isDefensiveTeam)
        {
            return true;
        }
        
        // Priority 2: If under attack, don't build expensive offensive teams
        if (state.underAttack && !isDefensiveTeam && GetTeamPriority(teamType, state) < 50)
        {
            return false;
        }
        
        // Priority 3: Balance team composition
        if (isDefensiveTeam && state.defenseUnits > state.offenseUnits * 2)
        {
            return false; // Too many defensive units
        }
        
        if (!isDefensiveTeam && state.offenseUnits > state.defenseUnits * 3)
        {
            return false; // Too many offensive units
        }
        
        return true;
    }

    bool TeamCompositionAnalyzer::IsTeamDefensive(TeamTypeClass* teamType)
    {
        if (!teamType) return false;
        
        int defensiveUnits = 0;
        int totalUnits = 0;
        
        // Analyze the task force composition
        for (int i = 0; i < teamType->TaskForce->CountEntries; i++)
        {
            auto& entry = teamType->TaskForce->Entries[i];
            if (!entry.Type) continue;
            
            bool isDefensiveUnit = false;
            
            switch (entry.Type->WhatAmI())
            {
                case AbstractType::InfantryType:
                {
                    auto pInfType = static_cast<InfantryTypeClass*>(entry.Type);
                    // Engineers, support units, AA infantry, mind control
                    isDefensiveUnit = pInfType->Engineer || pInfType->Agent || pInfType->VehicleThief
                                   || pInfType->Cyborg || pInfType->C4;
                    
                    // Check weapons for AA or mind control
                    if (!isDefensiveUnit && pInfType->GetWeapon(0) && pInfType->GetWeapon(0)->WeaponType)
                    {
                        auto weapon = pInfType->GetWeapon(0)->WeaponType;
                        if (weapon->Projectile && weapon->Projectile->AA && !weapon->Projectile->AG)
                            isDefensiveUnit = true;
                        else if (weapon->Warhead && weapon->Warhead->MindControl)
                            isDefensiveUnit = true;
                    }
                    break;
                }
                
                case AbstractType::UnitType:
                {
                    auto pUnitType = static_cast<UnitTypeClass*>(entry.Type);
                    // MCVs, harvesters, AA vehicles, artillery, repair units
                    isDefensiveUnit = pUnitType->DeploysInto || pUnitType->Harvester || pUnitType->Weeder
                                   || pUnitType->IsSimpleDeployer || pUnitType->DeployToFire;
                    
                    // Check weapons for AA, artillery, or mind control
                    if (!isDefensiveUnit && pUnitType->GetWeapon(0) && pUnitType->GetWeapon(0)->WeaponType)
                    {
                        auto weapon = pUnitType->GetWeapon(0)->WeaponType;
                        if (weapon->Projectile && weapon->Projectile->AA && !weapon->Projectile->AG)
                            isDefensiveUnit = true;
                        else if (weapon->Range > 10 && pUnitType->Speed <= 4)
                            isDefensiveUnit = true;
                        else if (weapon->Warhead && weapon->Warhead->MindControl)
                            isDefensiveUnit = true;
                    }
                    break;
                }
                
                case AbstractType::AircraftType:
                {
                    auto pAircraftType = static_cast<AircraftTypeClass*>(entry.Type);
                    // Transport aircraft, carryall aircraft
                    isDefensiveUnit = (pAircraftType->Passengers > 0 && (!pAircraftType->GetWeapon(0) || !pAircraftType->GetWeapon(0)->WeaponType))
                                   || pAircraftType->Carryall;
                    break;
                }
                
                default:
                    isDefensiveUnit = false;
                    break;
            }
            
            totalUnits += entry.Amount;
            if (isDefensiveUnit)
            {
                defensiveUnits += entry.Amount;
            }
        }
        
        // Team is considered defensive if majority of units are defensive
        // or if it's a small team (likely patrol/guard duty)
        return (totalUnits > 0 && defensiveUnits * 2 >= totalUnits) || (totalUnits <= 2);
    }

    int TeamCompositionAnalyzer::GetTeamPriority(TeamTypeClass* teamType, const BattlefieldState& state)
    {
        if (!teamType) return 0;
        
        int priority = 50; // Base priority
        
        // Analyze team composition
        bool isDefensiveTeam = this->IsTeamDefensive(teamType);
        
        // Increase priority for defensive teams when under attack
        if (state.underAttack && isDefensiveTeam)
        {
            priority += 30;
        }
        
        // Increase priority for offensive teams when we have defensive advantage
        if (!state.underAttack && !isDefensiveTeam && state.defenseUnits > state.offenseUnits)
        {
            priority += 20;
        }
        
        // Adjust priority based on team size and current unit balance
        int teamSize = 0;
        if (teamType->TaskForce)
        {
            for (int i = 0; i < teamType->TaskForce->CountEntries; i++)
            {
                teamSize += teamType->TaskForce->Entries[i].Amount;
            }
        }
        
        // Small teams get priority when resources are likely limited
        if (teamSize <= 3)
        {
            priority += 10;
        }
        // Large teams get lower priority unless we have good unit balance
        else if (teamSize > 6 && state.defenseUnits + state.offenseUnits < 20)
        {
            priority -= 15;
        }
        
        return std::clamp(priority, 0, 100);
    }

    // OptimizedTeamSelector Implementation
    bool OptimizedTeamSelector::UpdateTeamSelection(HouseClass* house)
    {
        if (!house || !g_EnablePerformanceOptimizations) return false;
        
        try
        {
            // Update trigger cache if needed
            if (Unsorted::CurrentFrame != lastTriggerUpdateFrame)
            {
                PrefilterTriggers(house);
            }
            
            // Get eligible triggers for this house
            auto it = eligibleTriggers.find(house);
            if (it == eligibleTriggers.end() || it->second.empty())
            {
                return false;
            }
            
            // Analyze battlefield state
            auto battlefieldState = analyzer.AnalyzeBattlefield(house);
            
            // Filter triggers based on current state
            std::vector<AITriggerTypeClass*> contextualTriggers;
            for (auto pTrigger : it->second)
            {
                if (pTrigger->Team1 && analyzer.ShouldBuildTeam(house, pTrigger->Team1, battlefieldState))
                {
                    if (!g_EnableResourceAwareness || resourceMgr.CanAffordTeam(house, pTrigger->Team1))
                    {
                        contextualTriggers.push_back(pTrigger);
                    }
                }
            }
            
            if (contextualTriggers.empty())
            {
                return false;
            }
            
            // Select best trigger
            AITriggerTypeClass* selectedTrigger = SelectBestTrigger(house, contextualTriggers);
            if (!selectedTrigger || !selectedTrigger->Team1)
            {
                return false;
            }
            
            // Create team if we haven't reached the limit
            int existingCount = teamCache.GetTeamCount(house, selectedTrigger->Team1);
            if (existingCount < selectedTrigger->Team1->Max)
            {
                if (TeamClass* newTeam = selectedTrigger->Team1->CreateTeam(house))
                {
                    newTeam->NeedsToDisappear = false;
                    
                    if (g_EnableSmartTeamBuilding)
                    {
                        LogAIDecision(house, "Created team", selectedTrigger->Team1->ID);
                    }
                    
                    // Update weight for elite triggers
                    if (selectedTrigger->Weight_Current >= 5000.0 && selectedTrigger->Weight_Minimum <= 4999.0)
                    {
                        selectedTrigger->Weight_Current = 4999.0;
                    }
                    
                    return true;
                }
            }
            
            return false;
        }
        catch (...)
        {
            Debug::Log("Enhanced AI: Exception in UpdateTeamSelection for house [%s]\n", 
                      house->Type ? house->Type->ID : "Unknown");
            return false;
        }
    }

    void OptimizedTeamSelector::PrefilterTriggers(HouseClass* house)
    {
        if (!house) return;
        
        std::vector<AITriggerTypeClass*> eligible;
        eligible.reserve(32);
        
        // Get enemy house for condition checking
        HouseClass* enemyHouse = nullptr;
        if (house->EnemyHouseIndex != -1 && house->EnemyHouseIndex < HouseClass::Array->Count)
        {
            enemyHouse = HouseClass::Array->Items[house->EnemyHouseIndex];
        }
        
        // Filter triggers efficiently
        for (int i = 0; i < AITriggerTypeClass::Array->Count; i++)
        {
            auto pTrigger = AITriggerTypeClass::Array->Items[i];
            
            if (!pTrigger || !pTrigger->Team1 || !pTrigger->IsEnabled) continue;
            
            // Basic compatibility checks
            if (pTrigger->TechLevel > house->StaticData.TechLevel) continue;
            
            // Difficulty check
            int difficulty = (int)house->AIDifficulty;
            if ((difficulty == 0 && !pTrigger->Enabled_Hard) ||
                (difficulty == 1 && !pTrigger->Enabled_Normal) ||
                (difficulty == 2 && !pTrigger->Enabled_Easy))
                continue;
            
            // Check conditions
            if (!pTrigger->ConditionMet(house, enemyHouse, false)) continue;
            
            eligible.push_back(pTrigger);
        }
        
        eligibleTriggers[house] = std::move(eligible);
        lastTriggerUpdateFrame = Unsorted::CurrentFrame;
    }

    AITriggerTypeClass* OptimizedTeamSelector::SelectBestTrigger(HouseClass* house, const std::vector<AITriggerTypeClass*>& triggers)
    {
        if (triggers.empty()) return nullptr;
        
        // Calculate total weight
        int totalWeight = 0;
        for (auto pTrigger : triggers)
        {
            int weight = static_cast<int>(pTrigger->Weight_Current);
            weight = std::clamp(weight, 1, 10000); // Prevent overflow
            totalWeight += weight;
        }
        
        if (totalWeight <= 0) return nullptr;
        
        // Select by weight
        int dice = ScenarioClass::Instance->Random.RandomRanged(1, totalWeight);
        int currentWeight = 0;
        
        for (auto pTrigger : triggers)
        {
            int weight = static_cast<int>(pTrigger->Weight_Current);
            weight = std::clamp(weight, 1, 10000);
            currentWeight += weight;
            
            if (dice <= currentWeight)
            {
                return pTrigger;
            }
        }
        
        return triggers.back(); // Fallback
    }

    // EnhancedAI Implementation
    bool EnhancedAI::UpdateHouseAI(HouseClass* house)
    {
        if (!house) return false;
        
        // Debug logging
        Debug::Log("Enhanced AI: UpdateHouseAI called for house [%s]\n", house->Type ? house->Type->ID : "Unknown");
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Reset team delay timer
        house->TeamDelayTimer.Start(RulesClass::Instance->TeamDelays[(int)house->AIDifficulty]);
        
        // Check AI trigger ratio
        int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);
        if (dice > house->RatioAITriggerTeam || !house->AITriggersActive)
        {
            Debug::Log("Enhanced AI: House [%s] skipping team creation (dice: %d, ratio: %d, active: %d)\n", 
                      house->Type->ID, dice, house->RatioAITriggerTeam, house->AITriggersActive);
            return true; // No teams this time
        }
        
        bool result = false;
        
        if (g_EnablePerformanceOptimizations)
        {
            Debug::Log("Enhanced AI: Using optimized team selector for house [%s]\n", house->Type->ID);
            result = teamSelector.UpdateTeamSelection(house);
        }
        else
        {
            Debug::Log("Enhanced AI: Performance optimizations disabled, using basic selection for house [%s]\n", house->Type->ID);
            // Fallback to basic team selection
            result = false; // Would implement basic selection here
        }
        
        // Update performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        lastUpdateTime = endTime;
        framesBetweenUpdates++;
        
        Debug::Log("Enhanced AI: House [%s] team creation result: %s (took %lld microseconds)\n", 
                  house->Type->ID, result ? "SUCCESS" : "FAILED", duration.count());
        
        return result;
    }

    // EnhancedSuggestedNewTeam Implementation
    void EnhancedSuggestedNewTeam::FindCandidateTeams(HouseClass* house, bool alerted)
    {
        if (!house) return;
        
        candidates.clear();
        candidates.reserve(32);
        
        // Find all possible team types for this house
        for (int i = 0; i < TeamTypeClass::Array->Count; i++)
        {
            auto pTeamType = TeamTypeClass::Array->Items[i];
            if (!pTeamType) continue;
            
            // Basic checks
            if (pTeamType->Owner != house) continue;
            if (pTeamType->TechLevel > house->StaticData.TechLevel) continue;
            
            // Check if we already have enough teams of this type
            int existingCount = 0;
            for (auto pTeam : *TeamClass::Array)
            {
                if (pTeam && pTeam->Type == pTeamType && pTeam->OwnerHouse == house)
                {
                    existingCount++;
                }
            }
            
            if (existingCount >= pTeamType->Max) continue;
            
            // Calculate priority and weight
            TeamCandidate candidate;
            candidate.teamType = pTeamType;
            candidate.weight = static_cast<int>(pTeamType->Priority);
            candidate.priority = candidate.weight;
            candidate.isDefensive = this->analyzer.IsTeamDefensive(pTeamType); // Use comprehensive analysis
            
            // Adjust priority based on alert status
            if (alerted && candidate.isDefensive)
            {
                candidate.priority += 20; // Boost defensive teams when alerted
            }
            
            candidates.push_back(candidate);
        }
    }

    void EnhancedSuggestedNewTeam::SortCandidatesByPriority(HouseClass* house, bool alerted)
    {
        if (candidates.empty()) return;
        
        // Sort by priority (highest first)
        std::sort(candidates.begin(), candidates.end(), 
                  [](const TeamCandidate& a, const TeamCandidate& b) {
                      return a.priority > b.priority;
                  });
    }

    TeamTypeClass* EnhancedSuggestedNewTeam::SelectBestTeam(HouseClass* house, bool alerted)
    {
        if (!house) return nullptr;
        
        FindCandidateTeams(house, alerted);
        if (candidates.empty()) return nullptr;
        
        SortCandidatesByPriority(house, alerted);
        
        // Calculate total weight for weighted selection
        int totalWeight = 0;
        for (const auto& candidate : candidates)
        {
            totalWeight += std::max(1, candidate.weight);
        }
        
        if (totalWeight <= 0) return nullptr;
        
        // Weighted random selection
        int dice = ScenarioClass::Instance->Random.RandomRanged(1, totalWeight);
        int currentWeight = 0;
        
        for (const auto& candidate : candidates)
        {
            currentWeight += std::max(1, candidate.weight);
            if (dice <= currentWeight)
            {
                return candidate.teamType;
            }
        }
        
        // Fallback to first candidate
        return candidates[0].teamType;
    }

    TeamTypeClass* EnhancedAI::SuggestNewTeam(HouseClass* house, bool alerted)
    {
        if (!house) return nullptr;
        
        Debug::Log("Enhanced AI: SuggestNewTeam called for house [%s], alerted: %s\\n", 
                  house->Type->ID, alerted ? "true" : "false");
        
        // Use the enhanced team selection logic
        TeamTypeClass* selectedTeam = suggestedTeam.SelectBestTeam(house, alerted);
        
        if (selectedTeam) {
            Debug::Log("Enhanced AI: Selected team [%s] for house [%s]\\n", 
                      selectedTeam->ID, house->Type->ID);
        } else {
            Debug::Log("Enhanced AI: No suitable team found for house [%s]\\n", house->Type->ID);
        }
        
        return selectedTeam;
    }

    // Utility Functions
    bool IsHouseUnderAttack(HouseClass* house)
    {
        if (!house) return false;
        
        // Check if any of the house's buildings are under attack
        // Note: IsUnderAttack() doesn't exist, so we'll use a different approach
        for (auto pBuilding : *BuildingClass::Array)
        {
            if (pBuilding && pBuilding->Owner == house && pBuilding->IsAlive)
            {
                // Check if building is taking damage or has low health
                if (pBuilding->Health < pBuilding->GetTechnoType()->Strength * 0.8)
                {
                    return true;
                }
            }
        }
        
        return false;
    }

    int CountUnitsOfType(HouseClass* house, AbstractType unitType)
    {
        if (!house) return 0;
        
        int count = 0;
        for (auto pTechno : *TechnoClass::Array)
        {
            if (pTechno && pTechno->Owner == house && pTechno->WhatAmI() == unitType && pTechno->IsAlive)
            {
                count++;
            }
        }
        
        return count;
    }

    void LogAIDecision(HouseClass* house, const char* decision, const char* reason)
    {
        if (house && decision && reason)
        {
            Debug::Log("Enhanced AI [%s]: %s - %s\n", house->Type->ID, decision, reason);
        }
    }

    void DumpAIStatistics(HouseClass* house)
    {
        if (!house) return;
        
        Debug::Log("=== AI Statistics for House [%s] ===\n", house->Type->ID);
        Debug::Log("Ground Units: %d\n", CountUnitsOfType(house, AbstractType::Unit));
        Debug::Log("Infantry: %d\n", CountUnitsOfType(house, AbstractType::Infantry));
        Debug::Log("Aircraft: %d\n", CountUnitsOfType(house, AbstractType::Aircraft));
        Debug::Log("Under Attack: %s\n", IsHouseUnderAttack(house) ? "Yes" : "No");
        Debug::Log("Available Money: %d\n", house->Available_Money());
        Debug::Log("=== End Statistics ===\n");
    }
} 