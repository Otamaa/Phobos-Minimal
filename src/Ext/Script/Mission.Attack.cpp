/*
 * Mission.Attack.cpp - Refactored AI Team Attack Mission Logic
 *
 * PERFORMANCE IMPROVEMENTS:
 * - Introduced caching structures to avoid repeated expensive lookups
 * - Reduced redundant calculations in loops
 * - Early returns to minimize deep nesting
 * - More efficient iteration patterns
 *
 * CODE STRUCTURE IMPROVEMENTS:
 * - Broke down large functions into focused helper functions
 * - Added constants for magic numbers
 * - Improved variable naming and type safety
 * - Reduced code duplication
 *
 * BUG FIXES:
 * - Fixed potential null pointer dereferences
 * - Added proper bounds checking
 * - Improved logical condition ordering
 * - Enhanced error handling
 */

#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>

#include <Utilities/Cast.h>

 // Constants for improved readability and maintainability
namespace AttackMissionConstants
{
	constexpr int WAIT_NO_TARGET_FRAMES = 30;
	constexpr double THREAT_MULTIPLIER = 128.0;
	constexpr double DISTANCE_DIVISOR = 256.0;
	constexpr double VHP_DAMAGE_DIVISOR = 2.0;
	constexpr double VHP_DAMAGE_MULTIPLIER = 2.0;
	constexpr double WEAPON_RANGE_MULTIPLIER = 4.0;
	constexpr double GUARD_RANGE_MULTIPLIER = 2.0;
}

// Helper structure to cache frequently accessed team data
struct TeamCacheData
{
	TeamExtData* pTeamData;
	TechnoClass* pFocus;
	FootClass* pTeamLeader;
	TechnoTypeClass* pLeaderType;
	bool bAircraftsWithoutAmmo;
	bool pacifistTeam;
	bool agentMode;
	bool leaderWeaponsHaveAG;
	bool leaderWeaponsHaveAA;
};

// Helper function to initialize team cache data
static TeamCacheData InitializeTeamCache(TeamClass* pTeam)
{
	TeamCacheData cache = {};
	cache.pTeamData = TeamExtContainer::Instance.Find(pTeam);
	cache.pFocus = flag_cast_to<TechnoClass*>(pTeam->ArchiveTarget);
	cache.bAircraftsWithoutAmmo = false;
	cache.pacifistTeam = true;
	cache.agentMode = false;
	cache.leaderWeaponsHaveAG = false;
	cache.leaderWeaponsHaveAA = false;

	return cache;
}

// Helper function to validate and clean team focus target
static void ValidateTeamFocus(TeamClass* pTeam, TeamCacheData& cache)
{
	if (!ScriptExtData::IsUnitAvailable(cache.pFocus, true))
	{
		pTeam->ArchiveTarget = nullptr;
		cache.pFocus = nullptr;
	}
}

// Helper function to process team member kills and awards
static bool ProcessTeamKills(TeamClass* pTeam, TeamCacheData& cache, bool repeatAction)
{
	FootClass* pFirst = pTeam->FirstUnit;
	if (!pFirst) return false;

	bool lastKillTechnoWasTeamTarget = false;
	FootClass* pCur = nullptr;
	FootClass* pNext = pFirst->NextTeamMember;

	do
	{
		auto pKillerTechnoData = TechnoExtContainer::Instance.Find(pFirst);

		if (pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (cache.pTeamData->NextSuccessWeightAward > 0)
			{
				ScriptExtData::IncreaseCurrentTriggerWeight(pTeam, false, cache.pTeamData->NextSuccessWeightAward);
				cache.pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pTeam->ArchiveTarget = nullptr;
			cache.pFocus = nullptr;
			lastKillTechnoWasTeamTarget = true;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pFootTeam = pTeam->FirstUnit; pFootTeam; pFootTeam = pFootTeam->NextTeamMember)
				{
					// Let's reset all Team Members objective
					auto pKillerTeamUnitData = TechnoExtContainer::Instance.Find(pFootTeam);
					pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pFootTeam->WhatAmI() == AbstractType::Aircraft)
					{
						pFootTeam->SetTarget(nullptr);
						pFootTeam->LastTarget = nullptr;
						pFootTeam->QueueMission(Mission::Guard, true);
					}
				}

				cache.pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				return true; // Action completed
			}
		}

		pCur = pNext;
		if (pNext) pNext = pNext->NextTeamMember;
		pFirst = pCur;
	}
	while (pCur);

	return false; // Continue processing
}

// Helper function to analyze team composition and capabilities
static void AnalyzeTeamComposition(TeamClass* pTeam, TeamCacheData& cache)
{
	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (!ScriptExtData::IsUnitAvailable(pFoot, true)) continue;

		auto const pTechnoType = pFoot->GetTechnoType();

		// Check for aircrafts without ammo
		if (pFoot->WhatAmI() == AbstractType::Aircraft
			&& !pFoot->IsInAir()
			&& static_cast<const AircraftTypeClass*>(pTechnoType)->AirportBound
			&& pFoot->Ammo < pTechnoType->Ammo)
		{
			cache.bAircraftsWithoutAmmo = true;
		}

		// Check if team is pacifist
		cache.pacifistTeam &= !ScriptExtData::IsUnitArmed(pFoot);

		// Check for agent mode (special infiltrator units)
		if (pFoot->WhatAmI() == AbstractType::Infantry)
		{
			auto const pTypeInf = static_cast<const InfantryTypeClass*>(pTechnoType);
			if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
			{
				cache.agentMode = true;
			}
		}
	}
}

// Helper function to find and validate team leader
static bool ValidateTeamLeader(TeamClass* pTeam, TeamCacheData& cache)
{
	// Find the Leader if not available
	if (!ScriptExtData::IsUnitAvailable(cache.pTeamData->TeamLeader, true))
	{
		cache.pTeamData->TeamLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	}

	cache.pTeamLeader = cache.pTeamData->TeamLeader;

	if (!cache.pTeamLeader || cache.bAircraftsWithoutAmmo || (cache.pacifistTeam && !cache.agentMode))
	{
		cache.pTeamData->IdxSelectedObjectFromAIList = -1;
		if (cache.pTeamData->WaitNoTargetAttempts != 0)
		{
			cache.pTeamData->WaitNoTargetTimer.Stop();
			cache.pTeamData->WaitNoTargetCounter = 0;
			cache.pTeamData->WaitNoTargetAttempts = 0;
		}
		pTeam->StepCompleted = true;
		return false;
	}

	cache.pLeaderType = cache.pTeamLeader->GetTechnoType();
	ScriptExtData::CheckUnitTargetingCapabilities(cache.pTeamLeader, cache.leaderWeaponsHaveAG, cache.leaderWeaponsHaveAA, cache.agentMode);

	// Special case: a Leader with OpenTopped tag
	if (cache.pLeaderType->OpenTopped && cache.pTeamLeader->Passengers.NumPassengers > 0)
	{
		for (auto pPassenger = cache.pTeamLeader->Passengers.GetFirstPassenger(); pPassenger; pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject))
		{
			bool passengerWeaponsHaveAG = false;
			bool passengerWeaponsHaveAA = false;
			ScriptExtData::CheckUnitTargetingCapabilities(pPassenger, passengerWeaponsHaveAG, passengerWeaponsHaveAA, cache.agentMode);

			cache.leaderWeaponsHaveAG |= passengerWeaponsHaveAG;
			cache.leaderWeaponsHaveAA |= passengerWeaponsHaveAA;
		}
	}

	return true;
}

// Helper function to find new target
static TechnoClass* FindNewTarget(TeamClass* pTeam, const TeamCacheData& cache, int targetMask, DistanceMode calcThreatMode, int attackAITargetType, int idxAITargetTypeItem)
{
	// Favorite Enemy House case. If set, AI will focus against that House
	HouseClass* enemyHouse = nullptr;
	const auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->Owner);
	const bool onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemyMode != -1 ?
		pHouseExt->m_ForceOnlyTargetHouseEnemy : pTeam->Type->OnlyTargetHouseEnemy;

	if (onlyTargetHouseEnemy && (size_t)cache.pTeamLeader->Owner->EnemyHouseIndex < (size_t)HouseClass::Array->Count)
	{
		enemyHouse = HouseClass::Array->Items[cache.pTeamLeader->Owner->EnemyHouseIndex];
	}

	return ScriptExtData::GreatestThreat(cache.pTeamLeader, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, cache.agentMode);
}

// Helper function to assign target to team members
static void AssignTargetToTeamMembers(TeamClass* pTeam, TechnoClass* selectedTarget)
{
	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (!ScriptExtData::IsUnitAvailable(pFoot, false) || !ScriptExtData::IsUnitAvailable(selectedTarget, true)) continue;

		auto const pTechnoType = pFoot->GetTechnoType();

		if (pFoot == selectedTarget || pFoot->Target == selectedTarget)
		{
			pFoot->QueueMission(Mission::Attack, true);
			continue;
		}

		// Naval units check
		if (pTechnoType->Underwater && pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& selectedTarget->GetCell()->LandType != LandType::Water)
		{
			// Naval units like Submarines are unable to target ground targets
			pFoot->SetTarget(nullptr);
			pFoot->SetDestination(nullptr, false);
			pFoot->QueueMission(Mission::Area_Guard, true);
			continue;
		}

		// Aircraft handling
		if (pFoot->WhatAmI() == AbstractType::Aircraft && pFoot->Ammo > 0 && pFoot->GetHeight() <= 0)
		{
			pFoot->SetDestination(selectedTarget, false);
			pFoot->QueueMission(Mission::Attack, true);
		}

		pFoot->SetTarget(selectedTarget);

		// Special mission assignments
		if (pFoot->IsEngineer())
		{
			pFoot->QueueMission(Mission::Capture, true);
		}
		else if (pFoot->WhatAmI() != AbstractType::Aircraft)
		{
			pFoot->QueueMission(Mission::Attack, true);
		}

		// Infantry special cases
		if (pFoot->WhatAmI() == AbstractType::Infantry)
		{
			auto const pInfantryType = static_cast<const InfantryTypeClass*>(pTechnoType);

			// Spy case
			if (pInfantryType && pInfantryType->Infiltrate && pInfantryType->Agent && pFoot->GetCurrentMission() != Mission::Enter)
			{
				pFoot->QueueMission(Mission::Enter, true);
			}

			// Tanya / Commando C4 case
			if ((pInfantryType->C4 || pFoot->HasAbility(AbilityType::C4)) && pFoot->GetCurrentMission() != Mission::Sabotage)
			{
				pFoot->QueueMission(Mission::Sabotage, true);
			}
		}
	}
}

// Helper function to handle no target found scenario
static bool HandleNoTargetFound(TeamClass* pTeam, TeamExtData* pTeamData)
{
	if (pTeamData->WaitNoTargetAttempts > 0 && pTeamData->WaitNoTargetTimer.Completed())
	{
		pTeamData->WaitNoTargetCounter = AttackMissionConstants::WAIT_NO_TARGET_FRAMES;
		pTeamData->WaitNoTargetTimer.Start(AttackMissionConstants::WAIT_NO_TARGET_FRAMES);
		return true; // Wait and return
	}

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
	}

	if (pTeamData->WaitNoTargetAttempts != 0 && pTeamData->WaitNoTargetTimer.Completed())
	{
		pTeamData->WaitNoTargetCounter = AttackMissionConstants::WAIT_NO_TARGET_FRAMES;
		pTeamData->WaitNoTargetTimer.Start(AttackMissionConstants::WAIT_NO_TARGET_FRAMES);
		return true; // Wait and return
	}

	pTeam->StepCompleted = true;
	return true; // Action completed
}

// Helper function to update attack missions for existing target
static bool UpdateAttackMissions(TeamClass* pTeam, const TeamCacheData& cache)
{
	if (!ScriptExtData::IsUnitAvailable(cache.pFocus, true))
	{
		pTeam->ArchiveTarget = nullptr;
		return false; // Target lost, restart
	}

	bool isAirOK = cache.pFocus->IsInAir() && cache.leaderWeaponsHaveAA;
	bool isGroundOK = !cache.pFocus->IsInAir() && cache.leaderWeaponsHaveAG;

	if (!cache.pFocus->GetTechnoType()->Immune
		&& (isAirOK || isGroundOK)
		&& (!cache.pTeamLeader->Owner->IsAlliedWith(cache.pFocus) || ScriptExtData::IsUnitMindControlledFriendly(cache.pTeamLeader->Owner, cache.pFocus)))
	{
		bool bForceNextAction = false;

		for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
		{
			if (!ScriptExtData::IsUnitAvailable(pFoot, true)) continue;

			auto const pTechnoType = pFoot->GetTechnoType();

			// Aircraft case 1
			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& static_cast<const AircraftTypeClass*>(pTechnoType)->AirportBound
				&& pFoot->Ammo > 0
				&& (pFoot->Target != cache.pFocus && !pFoot->InAir))
			{
				pFoot->SetTarget(cache.pFocus);
				continue;
			}

			// Naval units check
			if (pTechnoType->Underwater
				&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
				&& cache.pFocus->GetCell()->LandType != LandType::Water)
			{
				pFoot->SetTarget(nullptr);
				pFoot->SetDestination(nullptr, false);
				pFoot->QueueMission(Mission::Area_Guard, true);
				bForceNextAction = true;
				continue;
			}

			// Aircraft case 2
			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& pFoot->GetCurrentMission() != Mission::Attack
				&& pFoot->GetCurrentMission() != Mission::Enter)
			{
				if (pFoot->Ammo > 0)
				{
					if (pFoot->Target != cache.pFocus)
					{
						pFoot->SetTarget(cache.pFocus);
					}
					pFoot->QueueMission(Mission::Attack, true);
				}
				else
				{
					pFoot->EnterIdleMode(false, true);
				}
				continue;
			}

			// Tanya / Commando C4 case
			if ((pFoot->WhatAmI() == AbstractType::Infantry
				&& static_cast<const InfantryTypeClass*>(pTechnoType)->C4
				|| pFoot->HasAbility(AbilityType::C4)) && pFoot->GetCurrentMission() != Mission::Sabotage)
			{
				pFoot->QueueMission(Mission::Sabotage, true);
				continue;
			}

			// Other cases
			if (pFoot->WhatAmI() != AbstractType::Aircraft)
			{
				if (pFoot->Target != cache.pFocus)
				{
					pFoot->SetTarget(cache.pFocus);
				}

				if (pFoot->GetCurrentMission() != Mission::Attack
					&& pFoot->GetCurrentMission() != Mission::Unload
					&& pFoot->GetCurrentMission() != Mission::Selling)
				{
					pFoot->QueueMission(Mission::Attack, false);
				}
			}
		}

		if (bForceNextAction)
		{
			cache.pTeamData->IdxSelectedObjectFromAIList = -1;
			pTeam->StepCompleted = true;
			return true; // Action completed
		}
	}
	else
	{
		pTeam->ArchiveTarget = nullptr;
	}

	return false; // Continue processing
}

void ScriptExtData::Mission_Attack(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, scriptArgument] = pScript->GetCurrentAction();

	if (!pScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	// Initialize cache data
	TeamCacheData cache = InitializeTeamCache(pTeam);
	bool noWaitLoop = false;

	// Handle wait timer
	if (cache.pTeamData->WaitNoTargetCounter > 0)
	{
		if (cache.pTeamData->WaitNoTargetTimer.InProgress()) return;

		cache.pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		cache.pTeamData->WaitNoTargetCounter = 0;

		if (cache.pTeamData->WaitNoTargetAttempts > 0)
		{
			cache.pTeamData->WaitNoTargetAttempts--;
		}
	}

	// Validate focus target
	ValidateTeamFocus(pTeam, cache);

	// Process team kills and check if action is completed
	if (ProcessTeamKills(pTeam, cache, repeatAction)) return;

	// Analyze team composition
	AnalyzeTeamComposition(pTeam, cache);

	// Validate team leader and capabilities
	if (!ValidateTeamLeader(pTeam, cache)) return;

	// Main logic: find new target or update existing missions
	if (!cache.pFocus && !cache.bAircraftsWithoutAmmo)
	{
		// Find new target
		auto selectedTarget = FindNewTarget(pTeam, cache, scriptArgument, calcThreatMode, attackAITargetType, idxAITargetTypeItem);

		if (selectedTarget)
		{
			pTeam->ArchiveTarget = selectedTarget;
			cache.pFocus = selectedTarget;
			cache.pTeamData->WaitNoTargetAttempts = 0;
			cache.pTeamData->WaitNoTargetTimer.Stop();

			AssignTargetToTeamMembers(pTeam, selectedTarget);
		}
		else
		{
			// No target found
			if (HandleNoTargetFound(pTeam, cache.pTeamData)) return;
		}
	}
	else
	{
		// Update existing target missions
		if (UpdateAttackMissions(pTeam, cache)) return;
	}
}

// Helper struct to cache frequently accessed data in GreatestThreat
struct ThreatCalculationCache
{
	TechnoTypeClass* pTechnoType;
	TechnoTypeExtData* pTypeExt;
	bool leaderArmed;
	int detectionValue;
	HouseClass* pOwner;
	int ownerIndex;

	ThreatCalculationCache(TechnoClass* pTechno)
	{
		if (!pTechno)
		{
			// Initialize with safe defaults
			pTechnoType = nullptr;
			pTypeExt = nullptr;
			leaderArmed = false;
			pOwner = nullptr;
			ownerIndex = -1;
			detectionValue = 0;
			return;
		}

		pTechnoType = pTechno->GetTechnoType();
		pTypeExt = pTechnoType ? TechnoTypeExtContainer::Instance.Find(pTechnoType) : nullptr;
		leaderArmed = pTechno->IsArmed();
		pOwner = pTechno->Owner;
		ownerIndex = pOwner ? pOwner->ArrayIndex : -1;

		if (pOwner && pTypeExt)
		{
			auto const AIDifficulty = static_cast<int>(pOwner->GetAIDifficultyIndex());
			auto const DisguiseDetectionValue = pTypeExt->DetectDisguise_Percent.GetEx(RulesExtData::Instance()->AIDetectDisguise_Percent)->at(AIDifficulty);
			detectionValue = static_cast<int>(std::round(DisguiseDetectionValue * 100.0));
		}
		else
		{
			detectionValue = 0;
		}
	}
};

// Helper function to validate basic target requirements
static bool IsValidTargetCandidate(TechnoClass* object, TechnoClass* pTechno, const ThreatCalculationCache& cache)
{
	if (!ScriptExtData::IsUnitAvailable(object, true) || object == pTechno) return false;

	if (object->Spawned) return false;

	if (cache.pTechnoType && object->EstimatedHealth <= 0 && cache.pTechnoType->VHPScan == 2) return false;

	auto objectType = object->GetTechnoType();
	if (!objectType) return false;

	auto pObjectTypeExt = TechnoTypeExtContainer::Instance.Find(objectType);

	// Check AI legal target settings - only if cache data is valid
	if (cache.pTypeExt && cache.pOwner && cache.pTypeExt->AI_LegalTarget.isset() && !cache.pOwner->IsControlledByHuman() && !pObjectTypeExt->AI_LegalTarget.Get())
	{
		return false;
	}
	else if (!objectType->LegalTarget)
	{
		return false;
	}

	auto missionControl = object->GetCurrentMissionControl();
	if (missionControl && missionControl->NoThreat) return false;

	return true;
}

// Helper function to check weapon capabilities
static bool CheckWeaponCapabilities(TechnoClass* pTechno, TechnoClass* object, const ThreatCalculationCache& cache, bool agentMode, bool& unitWeaponsHaveAA, bool& unitWeaponsHaveAG)
{
	if (!cache.leaderArmed) return true;

	auto weapon = pTechno->GetWeapon(pTechno->SelectWeapon(object));
	if (!weapon) return false;

	const auto weaponType = weapon->WeaponType;
	if (!weaponType) return false;

	if (weaponType->Projectile)
	{
		unitWeaponsHaveAA = weaponType->Projectile->AA;
	}

	if ((weaponType->Projectile) || agentMode)
	{
		unitWeaponsHaveAG = weaponType->Projectile ? weaponType->Projectile->AG : true;
	}

	if (!agentMode)
	{
		if (weaponType->Warhead)
		{
			if (GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead, TechnoExtData::GetTechnoArmor(object, weaponType->Warhead)) == 0.0)
			{
				return false;
			}
		}

		if (object->IsInAir() && !unitWeaponsHaveAA) return false;
		if (!object->IsInAir() && !unitWeaponsHaveAG) return false;
	}

	// Check map zone
	if (!TechnoExtData::AllowedTargetByZone(pTechno, object, cache.pTypeExt->TargetZoneScanType, weaponType))
	{
		return false;
	}

	return true;
}

// Helper function to check stealth and special conditions
static bool CheckStealthAndSpecialConditions(TechnoClass* pTechno, TechnoClass* object, const ThreatCalculationCache& cache, HouseClass* onlyTargetThisHouseEnemy)
{
	auto objectType = object->GetTechnoType();

	// Naval targeting checks
	if (objectType->Naval)
	{
		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (cache.pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
				|| cache.pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
		{
			return false;
		}

		// Land not OK for the Naval unit
		if (cache.pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& (object->GetCell()->LandType != LandType::Water))
		{
			return false;
		}
	}

	// Stealth check
	if (object->CloakState == CloakState::Cloaked)
	{
		if (!object->GetCell()->Sensors_InclHouse(cache.ownerIndex))
		{
			return false;
		}
	}

	// Disguise detection
	if (cache.pTechnoType->DetectDisguise && object->IsDisguised() && cache.detectionValue > 0)
	{
		if (ScenarioClass::Instance->Random.PercentChance(cache.detectionValue))
		{
			return false;
		}
	}

	// House targeting restriction
	if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
	{
		return false;
	}

	// Final validity checks
	if (objectType->Immune || object->TemporalTargetingMe || object->BeingWarpedOut)
	{
		return false;
	}

	if (object->Owner == cache.pOwner)
	{
		return false;
	}

	if (cache.pOwner->IsAlliedWith(object) && !ScriptExtData::IsUnitMindControlledFriendly(cache.pOwner, object))
	{
		return false;
	}

	return true;
}

// Helper function to calculate threat value
static double CalculateThreatValue(TechnoClass* pTechno, TechnoClass* object, DistanceMode calcThreatMode, const ThreatCalculationCache& cache)
{
	double value = 0;

	if (calcThreatMode == DistanceMode::idkZero || calcThreatMode == DistanceMode::idkOne)
	{
		// Threat affected by distance
		double objectThreatValue = object->GetThreatValue();
		auto objectType = object->GetTechnoType();

		if (objectType->SpecialThreatValue > 0)
		{
			objectThreatValue += objectType->SpecialThreatValue * RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
		}

		// Bonus threat if defender is targeting attacker
		if (object->Owner->EnemyHouseIndex >= 0 && cache.pOwner == HouseClass::Array->Items[object->Owner->EnemyHouseIndex])
		{
			objectThreatValue += RulesClass::Instance->EnemyHouseThreatBonus;
		}

		// Extra threat based on current health (more damaged = higher priority)
		objectThreatValue += object->Health * (1.0 - object->GetHealthPercentage());
		value = (objectThreatValue * AttackMissionConstants::THREAT_MULTIPLIER) / ((pTechno->DistanceFrom(object) / AttackMissionConstants::DISTANCE_DIVISOR) + 1.0);

		// VHP scan adjustments
		if (cache.pTechnoType->VHPScan == 1)
		{
			if (object->EstimatedHealth <= 0)
			{
				value /= AttackMissionConstants::VHP_DAMAGE_DIVISOR;
			}
			else if (object->EstimatedHealth <= object->GetTechnoType()->Strength / 2)
			{
				value *= AttackMissionConstants::VHP_DAMAGE_MULTIPLIER;
			}
		}
	}
	else
	{
		// Simple distance-based selection
		value = pTechno->DistanceFrom(object);
	}

	return value;
}

// Helper function to determine if target is better than current best
static bool IsBetterTarget(double value, double bestVal, DistanceMode calcThreatMode)
{
	if (bestVal < 0) return true; // First valid target

	switch (calcThreatMode)
	{
	case DistanceMode::idkZero:
		return value > bestVal; // Higher threat value is better
	case DistanceMode::idkOne:
		return value < bestVal; // Lower threat value is better (inverted logic)
	case DistanceMode::Closest:
		return value < bestVal; // Lower distance is better (closer)
	case DistanceMode::Furtherst:
		return value > bestVal; // Higher distance is better (further)
	default:
		return value > bestVal;
	}
}

TechnoClass* ScriptExtData::GreatestThreat(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	if (!pTechno) return nullptr;

	TechnoClass* bestObject = nullptr;
	double bestVal = -1.0;

	// Cache frequently accessed data
	ThreatCalculationCache cache(pTechno);

	// Iterate through all technos more efficiently
	const int technoCount = TechnoClass::Array->Count;
	for (int i = 0; i < technoCount; ++i)
	{
		auto object = TechnoClass::Array->Items[i];

		// Quick validation checks
		if (!IsValidTargetCandidate(object, pTechno, cache)) continue;

		// Weapon capability checks
		bool unitWeaponsHaveAA = false;
		bool unitWeaponsHaveAG = false;
		if (!CheckWeaponCapabilities(pTechno, object, cache, agentMode, unitWeaponsHaveAA, unitWeaponsHaveAG)) continue;

		// Stealth and special condition checks
		if (!CheckStealthAndSpecialConditions(pTechno, object, cache, onlyTargetThisHouseEnemy)) continue;

		// Evaluate object with mask
		if (!ScriptExtData::EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno)) continue;

		// Calculate threat value
		double value = CalculateThreatValue(pTechno, object, calcThreatMode, cache);

		// Check if this is a better target
		if (IsBetterTarget(value, bestVal, calcThreatMode))
		{
			bestObject = object;
			bestVal = value;
		}
	}

	return bestObject;
}

// Helper struct to cache evaluation data
struct EvaluationCache
{
	TechnoTypeClass* pTechnoType;
	TechnoTypeExtData* pTargetTypeExt;
	AbstractType whatTech;
	bool isBuilding;
	bool buildingIsConsideredVehicle;
	bool isNeutral;

	EvaluationCache(TechnoClass* pTechno)
	{
		if (!pTechno)
		{
			pTechnoType = nullptr;
			pTargetTypeExt = nullptr;
			whatTech = AbstractType::None;
			isBuilding = false;
			buildingIsConsideredVehicle = false;
			isNeutral = true;
			return;
		}

		pTechnoType = pTechno->GetTechnoType();
		pTargetTypeExt = pTechnoType ? TechnoTypeExtContainer::Instance.Find(pTechnoType) : nullptr;
		whatTech = pTechno->WhatAmI();
		isBuilding = (whatTech == AbstractType::Building);
		buildingIsConsideredVehicle = false;
		isNeutral = pTechno->Owner ? pTechno->Owner->IsNeutral() : true;

		if (isBuilding)
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			buildingIsConsideredVehicle = pBuilding->Type ? pBuilding->Type->IsUndeployable() : false;
		}
	}
};

bool ScriptExtData::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{
	if (!ScriptExtData::IsUnitAvailable(pTechno, false) || !ScriptExtData::IsUnitAvailable(pTeamLeader, false))
	{
		return false;
	}

	// Cache evaluation data
	EvaluationCache cache(pTechno);

	if (!cache.pTargetTypeExt || cache.pTargetTypeExt->IsDummy)
	{
		return false;
	}

	// Check building limbo state
	if (cache.isBuilding)
	{
		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1)
		{
			return false;
		}
	}

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	const auto& nAITargetTypes = RulesExtData::Instance()->AITargetTypesLists;
	if ((size_t)attackAITargetType < nAITargetTypes.size())
	{
		const auto nVec = make_iterator(nAITargetTypes[attackAITargetType]);
		return nVec.contains(cache.pTechnoType);
	}

	// Mask evaluation - TODO: Replace with proper enum class for better readability
	switch (mask)
	{
	case 1: // Anything
		return !cache.isNeutral;

	case 2: // Building
		if (cache.isNeutral) return false;
		if (!cache.isBuilding) return false;

		if (!cache.buildingIsConsideredVehicle) return true;

		// Special building types that are not considered regular buildings
		if (const auto pBuilding = static_cast<BuildingClass*>(pTechno))
		{
			const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
			return !(pBuilding->Type->Artillary
				|| pBuilding->Type->TickTank
				|| pBuilding->Type->ICBMLauncher
				|| pBuilding->Type->SensorArray
				|| pBldExt->IsJuggernaut);
		}
		return false;
	case 3: // Harvester
		if (cache.isNeutral) return false;

		switch (cache.whatTech)
		{
		case AbstractType::Unit:
		{
			const auto pType = static_cast<const UnitClass*>(pTechno)->Type;
			return pType->Harvester || pType->Weeder;
		}
		case AbstractType::Building:
		{
			const auto pBuilding = static_cast<const BuildingClass*>(pTechno);
			return pBuilding->SlaveManager && cache.pTechnoType->ResourceGatherer && pBuilding->Type->Enslaves;
		}
		case AbstractType::Infantry:
		{
			const auto pInfantry = static_cast<const InfantryClass*>(pTechno);
			return pInfantry->Type->Slaved && pInfantry->SlaveOwner && cache.pTechnoType->ResourceGatherer;
		}
		default:
			return false;
		}

	case 4: // Infantry
		return !cache.isNeutral && cache.whatTech == AbstractType::Infantry;

	case 5: // Vehicle, Aircraft, Deployed vehicle into structure
		if (cache.isNeutral) return false;

		if (cache.buildingIsConsideredVehicle) return true;

		if (cache.isBuilding)
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			const auto pExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
			return (pBuilding->Type->Artillary
				|| pBuilding->Type->TickTank
				|| pBuilding->Type->ICBMLauncher
				|| pBuilding->Type->SensorArray
				|| pExt->IsJuggernaut);
		}

		return (cache.whatTech == AbstractType::Aircraft || cache.whatTech == AbstractType::Unit);
	case 6: // Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		return static_cast<BuildingClass*>(pTechno)->Type->Factory != AbstractType::None;
	case 7: // Defense
		if (cache.isNeutral || !cache.isBuilding) return false;
		return static_cast<BuildingClass*>(pTechno)->Type->IsBaseDefense;
	case 8: // House threats
		if (pTeamLeader && !cache.isNeutral)
		{
			if (auto pTarget = flag_cast_to<TechnoClass*>(pTechno->Target))
			{
				// The possible Target is aiming against me? Revenge!
				if (pTarget != pTeamLeader)
					return pTarget->Target == pTeamLeader
					|| (pTarget->Owner && pTarget->Owner->EnemyHouseIndex >= 0
						&& pTarget->Owner->EnemyHouseIndex < HouseClass::Array->Count
						&& HouseClass::Array->Items[pTarget->Owner->EnemyHouseIndex] == pTeamLeader->Owner);
			}

			auto const curtargetiter = make_iterator(pTechno->CurrentTargets);
			if (!curtargetiter.empty())
			{
				return std::any_of(curtargetiter.begin(), curtargetiter.end(),
				[pTeamLeader](AbstractClass* pTarget)
 {
	 const auto pTech = flag_cast_to<TechnoClass*>(pTarget);
	 return ScriptExtData::IsUnitAvailable(pTech, true) && pTech->GetOwningHouse() == pTeamLeader->Owner;
				});
			}

			// Then check if this possible target is too near of the Team Leader
			const auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / AttackMissionConstants::DISTANCE_DIVISOR;
			const auto pWeaponPrimary = TechnoExtData::GetCurrentWeapon(pTechno);
			const auto pWeaponSecondary = TechnoExtData::GetCurrentWeapon(pTechno, true);
			const bool primaryCheck = pWeaponPrimary && distanceToTarget <= (WeaponTypeExtData::GetRangeWithModifiers(pWeaponPrimary, pTechno) / AttackMissionConstants::DISTANCE_DIVISOR * AttackMissionConstants::WEAPON_RANGE_MULTIPLIER);
			const bool secondaryCheck = pWeaponSecondary && distanceToTarget <= (WeaponTypeExtData::GetRangeWithModifiers(pWeaponSecondary, pTechno) / AttackMissionConstants::DISTANCE_DIVISOR * AttackMissionConstants::WEAPON_RANGE_MULTIPLIER);
			const bool guardRangeCheck = pTeamLeader->GetTechnoType()->GuardRange > 0 && distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / AttackMissionConstants::DISTANCE_DIVISOR * AttackMissionConstants::GUARD_RANGE_MULTIPLIER);

			return primaryCheck
				|| secondaryCheck
				|| guardRangeCheck;
		}

		return false;

	case 9: // Power Plant
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			for (const auto type : pBuilding->GetTypes())
			{
				if (type && type->PowerBonus > 0)
					return true;
			}
		}
		return false;
	case 10: // Occupied Building
		if (!cache.isBuilding) return false;
		return static_cast<BuildingClass*>(pTechno)->Occupants.Count > 0;
	case 11:
	{
		// Civilian Tech
		if (auto pBld = cast_to<BuildingClass*, false>(pTechno))
		{
			if (RulesClass::Instance->NeutralTechBuildings.Count > 0)
				return RulesClass::Instance->NeutralTechBuildings.Contains(pBld->Type);

			// Other cases of civilian Tech Structures
			return !pBld->Type->InvisibleInGame
				&& !pBld->Type->Immune
				&& pBld->Type->Unsellable
				&& pBld->Type->Capturable
				&& pBld->Type->TechLevel < 0
				&& pBld->Type->NeedsEngineer
				&& !pBld->Type->BridgeRepairHut;
		}
		return false;
	}
	case 12: // Refinery
		if (cache.isNeutral) return false;
		{
			if (cache.whatTech == AbstractType::Unit)
			{
				const auto pUnit = static_cast<UnitClass*>(pTechno);
				return !(pUnit->Type->Harvester || pUnit->Type->Weeder)
					&& pUnit->Type->ResourceGatherer
					&& pUnit->Type->DeploysInto;
			}

			if (cache.isBuilding)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pTechno);
				return pBuilding->Type->ResourceGatherer
					|| (pBuilding->Type->Refinery || (pBuilding->SlaveManager && pBuilding->Type->Enslaves));
			}

			return false;
		}

	case 13: // Mind Control Capable
		if (cache.isNeutral) return false;
		{
			auto const& [WeaponType1, WeaponType2] = ScriptExtData::GetWeapon(pTechno);

			bool CanMC = false;
			if (WeaponType1 && WeaponType1->Warhead)
			{
				auto pWHExt = WarheadTypeExtContainer::Instance.Find(WeaponType1->Warhead);
				CanMC = (pWHExt && pWHExt->PermaMC.Get()) || WeaponType1->Warhead->MindControl;
			}

			if (!CanMC && WeaponType2 && WeaponType2->Warhead)
			{
				auto pWHExt = WarheadTypeExtContainer::Instance.Find(WeaponType2->Warhead);
				CanMC = (pWHExt && pWHExt->PermaMC.Get()) || WeaponType2->Warhead->MindControl;
			}

			return CanMC;
		}
	case 14: // Aircraft and Air Unit
		return (!cache.isNeutral
			&& (cache.whatTech == AbstractType::Aircraft
				|| (cache.pTechnoType && cache.pTechnoType->JumpJet)
				|| (cache.pTechnoType && cache.pTechnoType->BalloonHover)
				|| pTechno->IsInAir()));

	case 15: // Naval Unit & Structure
		return (!cache.isNeutral
			&& ((cache.pTechnoType && cache.pTechnoType->Naval)
				|| (pTechno->GetCell()->LandType == LandType::Water)));
	case 16: // Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		if (cache.isNeutral) return false;
		{
			const auto pTypeBuilding = type_cast<BuildingTypeClass*>(cache.pTechnoType);

			return ((cache.pTargetTypeExt
				&& (cache.pTargetTypeExt->RadarJamRadius > 0
					|| cache.pTargetTypeExt->InhibitorRange.isset()))
		|| (pTypeBuilding && (pTypeBuilding->GapGenerator
			|| pTypeBuilding->CloakGenerator)));
		}
	case 17: // Ground Vehicle
		return !cache.isNeutral
			&& ((cache.whatTech == AbstractType::Unit || cache.buildingIsConsideredVehicle) && !pTechno->IsInAir() && !(cache.pTechnoType && cache.pTechnoType->Naval));
	case 18: // Economy: Harvester, Refinery or Resource helper
		if (cache.isNeutral) return false;
		{
			if (auto pUnitT = type_cast<UnitTypeClass*>(cache.pTechnoType))
				return pUnitT->Harvester || pUnitT->ResourceGatherer;

			if (auto pInfT = type_cast<InfantryTypeClass*>(cache.pTechnoType))
				return pInfT->ResourceGatherer && (pInfT->Slaved && pTechno->SlaveOwner);

			if (cache.isBuilding)
			{
				const auto pBuilding = static_cast<BuildingClass*>(pTechno);
				for (auto const type : pBuilding->GetTypes())
				{
					if (type && (type->ProduceCashAmount > 0 || type->OrePurifier))
						return true;
				}

				return pBuilding->Type->Refinery || pBuilding->Type->ResourceGatherer
					|| (pTechno->SlaveManager && pBuilding->Type->Enslaves);
			}

			return false;
		}
	case 19: // Infantry Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			auto pBuildingType = type_cast<BuildingTypeClass*>(cache.pTechnoType);
			return pBuildingType && pBuildingType->Factory == AbstractType::InfantryType;
		}
	case 20: // Land Vehicle Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			auto pBuildingType = type_cast<BuildingTypeClass*>(cache.pTechnoType);
			return pBuildingType
				&& pBuildingType->Factory == AbstractType::UnitType
				&& !pBuildingType->Naval;
		}

	case 21: // Aircraft Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			auto pBuildingType = type_cast<BuildingTypeClass*>(cache.pTechnoType);
			return pBuildingType
				&& (pBuildingType->Factory == AbstractType::AircraftType
					|| pBuildingType->Helipad);
		}
	case 22: // Radar & SpySat
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			if (pBuilding->Type->Radar)
				return true;

			for (auto const type : pBuilding->GetTypes())
			{
				if (type && type->SpySat)
					return true;
			}
		}
		return false;

	case 23: // Buildable Tech
		if (cache.isNeutral || !cache.isBuilding) return false;
		return (RulesClass::Instance->BuildTech.Count > 0 && RulesClass::Instance->BuildTech.Contains(static_cast<BuildingTypeClass*>(cache.pTechnoType)));
	case 24: // Naval Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			auto pBuildingType = static_cast<BuildingTypeClass*>(cache.pTechnoType);
			return pBuildingType
				&& pBuildingType->Factory == AbstractType::UnitType
				&& pBuildingType->Naval;
		}

	case 25: // Super Weapon building
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			for (auto type : pBuilding->GetTypes())
			{
				if (!type) continue;

				if (auto typeExt = BuildingTypeExtContainer::Instance.Find(const_cast<BuildingTypeClass*>(type)))
				{
					if (typeExt->GetSuperWeaponCount() > 0)
						return true;
				}
			}
		}
		return false;
	case 26: // Construction Yard
		if (cache.isNeutral) return false;
		{
			if (cache.isBuilding)
			{
				auto pTypeBuilding = type_cast<BuildingTypeClass*>(cache.pTechnoType);
				if (pTypeBuilding)
				{
					if (cache.pTargetTypeExt)
					{
						const auto pFake = cache.pTargetTypeExt->Fake_Of;
						if (pFake)
						{
							return ((BuildingTypeClass*)pFake.Get())->Factory == AbstractType::BuildingType && ((BuildingTypeClass*)pFake.Get())->ConstructionYard;
						}
					}
					return pTypeBuilding->Factory == AbstractType::BuildingType && pTypeBuilding->ConstructionYard;
				}
			}

			if (cache.whatTech == AbstractType::Unit)
			{
				return (RulesClass::Instance->BaseUnit.Count > 0 && RulesClass::Instance->BaseUnit.Contains(static_cast<UnitTypeClass*>(cache.pTechnoType)));
			}

			return false;
		}

	case 27: // Any Neutral object
		return cache.isNeutral;
	case 28: // Cloak Generator & Gap Generator
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			for (const auto pBldTypeHere : pBuilding->GetTypes())
			{
				if (pBldTypeHere && (pBuilding->Type->GapGenerator || pBuilding->Type->CloakGenerator))
					return true;
			}
		}
		return false;

	case 29: // Radar Jammer
		if (cache.isNeutral) return false;
		return cache.pTargetTypeExt && (cache.pTargetTypeExt->RadarJamRadius > 0);

	case 30: // Inhibitor
		if (cache.isNeutral) return false;
		return cache.pTargetTypeExt && cache.pTargetTypeExt->InhibitorRange.isset();

	case 31: // Naval Unit
		if (cache.isNeutral) return false;
		return cache.whatTech == AbstractType::Unit
			&& (cache.pTechnoType->Naval || pTechno->GetCell()->LandType == LandType::Water);
	case 32: // Any non-building unit
		if (cache.isNeutral) return false;
		{
			if (cache.whatTech == AbstractType::Unit)
			{
				const auto pUnit = static_cast<UnitClass*>(pTechno);
				return !pUnit->Type->DeploysInto;
			}

			if (cache.buildingIsConsideredVehicle)
				return true;

			if (cache.isBuilding)
			{
				auto pTypeBuilding = type_cast<BuildingTypeClass*>(cache.pTechnoType);
				if (pTypeBuilding)
				{
					const auto pBuildingExt = BuildingTypeExtContainer::Instance.Find(pTypeBuilding);
					return (pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pBuildingExt->IsJuggernaut
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray
						|| pTypeBuilding->ResourceGatherer);
				}
			}

			return false;
		}

	case 33: // Capturable Structure or Repair Hut
		if (!cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
			return pBldExt->EngineerRepairable.Get(pBuilding->Type->Capturable)
				|| (pBuilding->Type->BridgeRepairHut && pBuilding->Type->Repairable);
		}
	case 34: // Inside the Area Guard of the Team Leader
		if (!pTeamLeader || cache.isNeutral) return false;
		{
			const auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / AttackMissionConstants::DISTANCE_DIVISOR;
			const auto pLeaderType = pTeamLeader->GetTechnoType();

			return (pLeaderType->GuardRange > 0
				&& distanceToTarget <= ((pLeaderType->GuardRange / AttackMissionConstants::DISTANCE_DIVISOR) * AttackMissionConstants::GUARD_RANGE_MULTIPLIER));
		}

	case 35: // Land Vehicle Factory & Naval Factory
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			return pBuilding->Type->Factory == AbstractType::UnitType;
		}
	case 36: // Building that isn't a defense
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->IsBaseDefense || cache.buildingIsConsideredVehicle)
				return false;

			auto const pBtypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

			return !(pBuilding->Type->Artillary
				|| pBuilding->Type->TickTank
				|| pBtypeExt->IsJuggernaut
				|| pBuilding->Type->ICBMLauncher
				|| pBuilding->Type->SensorArray);
		}
	case 39: // Occupyable Civilian Building
		if (!cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			if (pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count == 0 && pBuilding->Owner->IsNeutral() && pBuilding->Type->CanOccupyFire && pBuilding->Type->TechLevel == -1 && pBuilding->GetHealthStatus() != HealthState::Red)
				return true;
			if (pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count < pBuilding->Type->MaxNumberOccupants && pBuilding->Owner == pTeamLeader->Owner && pBuilding->Type->CanOccupyFire)
				return true;
		}
		return false;
	case 40: // Self Building with Grinding=yes
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			return pBuilding->Type->Grinding && pBuilding->Owner == pTeamLeader->Owner;
		}
	case 41: // Building with Spyable=yes
		if (cache.isNeutral || !cache.isBuilding) return false;
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);
			return pBuilding->Type->Spyable;
		}
	case 37: // Hero Building
		if (cache.isNeutral || !cache.isBuilding) return false;
		return cache.pTargetTypeExt && cache.pTargetTypeExt->IsHero.Get();
	}

	return false;
}

void ScriptExtData::Mission_Attack_List(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType)
{
	TeamExtContainer::Instance.Find(pTeam)->IdxSelectedObjectFromAIList = -1;
	const auto& [curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();

	if (attackAITargetType < 0)
		attackAITargetType = curArg;

	const auto& targetList = RulesExtData::Instance()->AITargetTypesLists;
	if ((size_t)attackAITargetType < targetList.size() && !targetList[attackAITargetType].empty())
	{
		ScriptExtData::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

static std::vector<int> Mission_Attack_List1Random_validIndexes;

void ScriptExtData::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType)
{
	//auto pScript = pTeam->CurrentScript;
	Mission_Attack_List1Random_validIndexes.clear();
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();

	if (attackAITargetType < 0)
		attackAITargetType = curArgs;

	if ((size_t)attackAITargetType < RulesExtData::Instance()->AITargetTypesLists.size())
	{
		if ((size_t)pTeamData->IdxSelectedObjectFromAIList < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size())
		{
			ScriptExtData::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, pTeamData->IdxSelectedObjectFromAIList);
			return;
		}

		if (!RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].empty())
		{
			// Finding the objects from the list that actually exists in the map
			TechnoClass::Array->for_each([&](TechnoClass* pTechno)
 {
	 if (!ScriptExtData::IsUnitAvailable(pTechno, true))
		 return;

	 //if (pTechno->Spawned)
	 //	return;

	 if (auto pTechnoType = pTechno->GetTechnoType())
	 {
		 bool found = false;

		 for (size_t j = 0u; j < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size() && !found; j++)
		 {
			 auto const pFirstUnit = pTeam->FirstUnit;

			 if (pTechnoType == RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][j] && (!pFirstUnit->Owner->IsAlliedWith(pTechno)
				 || ScriptExtData::IsUnitMindControlledFriendly(pFirstUnit->Owner, pTechno)))
			 {
				 Mission_Attack_List1Random_validIndexes.push_back(j);
				 found = true;
			 }
		 }
	 }
			});

			if (!Mission_Attack_List1Random_validIndexes.empty())
			{
				const int idxSelectedObject = Mission_Attack_List1Random_validIndexes[ScenarioClass::Instance->Random.RandomFromMax(Mission_Attack_List1Random_validIndexes.size() - 1)];
				pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

				//Debug::LogInfo("AI Scripts - AttackListRandom: [{}] [{}] (line: {} = {},{}) Picked a random Techno from the list index [AITargetTypes][{}][{}] = {}",
				//	pTeam->Type->ID,
				//	pTeam->CurrentScript->Type->ID,
				//	pScript->CurrentMission,
				//	(int)curAct,
				//	curArgs,
				//	attackAITargetType,
				//	idxSelectedObject,
				//	RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][idxSelectedObject]->ID);

				ScriptExtData::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
				return;
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
	//Debug::LogInfo("AI Scripts - AttackListRandom: [{}] [{}] (line: {} = {},{}) Failed to pick a random Techno from the list index [AITargetTypes][{}]! Valid Technos in the list: {}",
	//	pTeam->Type->ID,
	//	pTeam->CurrentScript->Type->ID,
	//	pScript->CurrentMission,
	//	(int)curAct,
	//	curArgs,
	//	attackAITargetType,
	//	Mission_Attack_List1Random_validIndexes.size()
	//);
}

#include <Ext/BulletType/Body.h>

void ScriptExtData::CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode)
{
	if (!pTechno || !pTechno->IsAlive)
		return;

	const auto& [pWeaponPrimary, pWeaponSecondary] = ScriptExtData::GetWeapon(pTechno);

	if (pWeaponPrimary && pWeaponPrimary->Projectile)
	{
		hasAntiAir = pWeaponPrimary->Projectile->AA;
	}

	if (!hasAntiAir && pWeaponSecondary && pWeaponSecondary->Projectile)
	{
		hasAntiAir = pWeaponSecondary->Projectile->AA;
	}

	if (agentMode)
	{
		hasAntiGround = true;
		return;
	}

	if (pWeaponPrimary && pWeaponPrimary->Projectile)
	{
		hasAntiGround = pWeaponPrimary->Projectile->AG && !BulletTypeExtContainer::Instance.Find(pWeaponPrimary->Projectile)->AAOnly;
	}

	if (!hasAntiGround && pWeaponSecondary && pWeaponSecondary->Projectile)
	{
		hasAntiGround = pWeaponSecondary->Projectile->AG && !BulletTypeExtContainer::Instance.Find(pWeaponSecondary->Projectile)->AAOnly;
	}

	//if (pWeaponPrimary && !pWeaponPrimary->Projectile)
	//	Debug::FatalError("Weapon[%s] has no projectile !\n", pWeaponPrimary->ID);

	//if(pWeaponSecondary && !pWeaponSecondary->Projectile)
	//	Debug::FatalError("Weapon[%s] has no projectile !\n", pWeaponSecondary->ID);
}

bool ScriptExtData::IsUnitArmed(TechnoClass* pTechno)
{
	if (!pTechno || !pTechno->IsAlive)
		return false;

	const auto pWeapons = ScriptExtData::GetWeapon(pTechno);
	return pWeapons.first || pWeapons.second;
}

bool ScriptExtData::IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno)
{
	return pHouse->IsAlliedWith(pTechno) && pTechno->IsMindControlled() && !pHouse->IsAlliedWith(pTechno->MindControlledBy);
}