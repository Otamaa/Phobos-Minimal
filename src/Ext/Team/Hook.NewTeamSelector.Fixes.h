#pragma once

#include <HouseClass.h>

// Forward declarations
class AITriggerTypeClass;
class TeamTypeClass;

// Team category enumeration (shared between original and optimized implementations)
enum class TeamCategory
{
	None = 0,
	Ground = 1,
	Air = 2,
	Naval = 3,
	Unclassified = 4
};

// New Team Selector optimized implementation
namespace NewTeamSelectorFixes
{
	bool OptimizedUpdateTeam(HouseClass* pHouse);
} 