#pragma once

#include <HouseClass.h>
#include <TeamTypeClass.h>
#include <TeamClass.h>
#include <AITriggerTypeClass.h>
#include <unordered_map>
#include <vector>
#include <chrono>

// Enhanced Vanilla AI System
// Provides performance optimizations and behavior improvements for vanilla AI

namespace VanillaAI
{
    // Team count caching system for performance
    class TeamCountCache
    {
    private:
        std::unordered_map<TeamTypeClass*, int> cache;
        int lastUpdateFrame = -1;
        HouseClass* cachedHouse = nullptr;
        
    public:
        int GetTeamCount(HouseClass* house, TeamTypeClass* type);
        void InvalidateCache();
        void UpdateCache(HouseClass* house);
    };

    // Resource-aware team building
    class ResourceManager
    {
    private:
        struct ResourceRequirement
        {
            int credits = 0;
            int power = 0;
            bool hasPrerequisites = false;
        };
        
        std::unordered_map<TeamTypeClass*, ResourceRequirement> requirements;
        
    public:
        bool CanAffordTeam(HouseClass* house, TeamTypeClass* teamType);
        int GetTeamCost(TeamTypeClass* teamType);
        bool HasPrerequisites(HouseClass* house, TeamTypeClass* teamType);
    };

    // Smart team composition analyzer
    class TeamCompositionAnalyzer
    {
    private:
        struct BattlefieldState
        {
            int groundUnits = 0;
            int airUnits = 0;
            int navalUnits = 0;
            int defenseUnits = 0;
            int offenseUnits = 0;
            bool underAttack = false;
        };
        
    public:
        BattlefieldState AnalyzeBattlefield(HouseClass* house);
        bool ShouldBuildTeam(HouseClass* house, TeamTypeClass* teamType, const BattlefieldState& state);
        int GetTeamPriority(TeamTypeClass* teamType, const BattlefieldState& state);
    };

    // Performance-optimized team selector
    class OptimizedTeamSelector
    {
    private:
        TeamCountCache teamCache;
        ResourceManager resourceMgr;
        TeamCompositionAnalyzer analyzer;
        
        // Pre-filtered trigger lists per house
        std::unordered_map<HouseClass*, std::vector<AITriggerTypeClass*>> eligibleTriggers;
        int lastTriggerUpdateFrame = -1;
        
    public:
        bool UpdateTeamSelection(HouseClass* house);
        void PrefilterTriggers(HouseClass* house);
        AITriggerTypeClass* SelectBestTrigger(HouseClass* house, const std::vector<AITriggerTypeClass*>& triggers);
    };

    // Enhanced Suggested_New_Team implementation
    class EnhancedSuggestedNewTeam
    {
    private:
        struct TeamCandidate
        {
            TeamTypeClass* teamType;
            int weight;
            int priority;
            bool isDefensive;
        };
        
        std::vector<TeamCandidate> candidates;
        
    public:
        void FindCandidateTeams(HouseClass* house, bool alerted);
        TeamTypeClass* SelectBestTeam(HouseClass* house, bool alerted);
        void SortCandidatesByPriority(HouseClass* house, bool alerted);
    };

    // Main enhanced AI controller
    class EnhancedAI
    {
    private:
        OptimizedTeamSelector teamSelector;
        EnhancedSuggestedNewTeam suggestedTeam;
        
        // Performance tracking
        std::chrono::high_resolution_clock::time_point lastUpdateTime;
        int framesBetweenUpdates = 0;
        
    public:
        // Main AI update function - replaces vanilla HouseClass::AI team logic
        bool UpdateHouseAI(HouseClass* house);
        
        // Enhanced Suggested_New_Team - replaces vanilla function
        TeamTypeClass* SuggestNewTeam(HouseClass* house, bool alerted);
        
        // Performance monitoring
        void UpdatePerformanceMetrics();
        double GetAverageUpdateTime() const;
    };

    // Global enhanced AI instance
    extern EnhancedAI g_EnhancedAI;

    // Configuration flags
    extern bool g_EnablePerformanceOptimizations;
    extern bool g_EnableSmartTeamBuilding;
    extern bool g_EnableResourceAwareness;
    extern bool g_EnableBattlefieldAnalysis;
    
    // Utility functions
    bool IsHouseUnderAttack(HouseClass* house);
    int CountUnitsOfType(HouseClass* house, AbstractType unitType);
    bool HasExcessiveUnitsOfType(HouseClass* house, AbstractType unitType);
    double GetHouseThreatLevel(HouseClass* house, HouseClass* enemy);
    
    // Debug and monitoring
    void LogAIDecision(HouseClass* house, const char* decision, const char* reason);
    void DumpAIStatistics(HouseClass* house);
} 