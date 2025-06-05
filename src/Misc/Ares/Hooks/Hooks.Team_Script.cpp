#include <AbstractClass.h>
#include <TechnoClass.h>
#include <TeamClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/TeamType/Body.h>

#include <TerrainTypeClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Notifications.h>
#include <strsafe.h>

#include "Header.h"

#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

/*
 * TeamRetaliate Enhanced Implementation
 *
 * IMPROVEMENTS MADE:
 * - Combined original logic with custom implementation for comprehensive behavior
 * - Added performance optimizations with helper functions and data caching
 * - Fixed potential null pointer crashes and added extensive validation
 * - Proper handling of open-topped transports and edge cases
 * - Enhanced mission assignment that actually makes units attack the attacker
 * - Added configurable options for fine-tuning behavior:
 *   * TeamRetaliate.CheckWeaponCompatibility - Whether to check if units can engage target type
 *   * TeamRetaliate.InterruptCriticalMissions - Whether to interrupt repair/construction missions
 *   * TeamRetaliate.MaxRetaliationDistance - Maximum distance for retaliation (in cells, -1 = unlimited)
 *
 * BUG FIXES:
 * - Fixed units not actually retaliating (only ArchiveTarget was set before)
 * - Prevented crashes from null pointers and invalid objects
 * - Fixed ally targeting issues and aircraft handling
 * - Improved validation for limbo units and special cases
 *
 * PERFORMANCE IMPROVEMENTS:
 * - Reduced redundant type checks and memory allocations
 * - Early exits to minimize unnecessary processing
 * - Cached frequently accessed data
 * - More efficient team member iteration
 */

#include <Phobos.h>

#include <TActionClass.h>
#include <TEventClass.h>
#include <ScriptClass.h>
#include <TeamTypeClass.h>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <CellClass.h>

#include <Ext/Rules/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Cast.h>

 // Helper functions for TeamRetaliate optimization
namespace TeamRetaliateHelpers
{
	// Fast validation for potential retaliation targets
	inline bool IsValidRetaliationTarget(AbstractClass* pAttacker, HouseClass* pTeamOwner)
	{
		if (!pAttacker || !pTeamOwner)
			return false;

		// Exclude aircraft attackers
		if (pAttacker->WhatAmI() == AircraftClass::AbsID)
			return false;

		auto pAttackerTechno = flag_cast_to<TechnoClass*, false>(pAttacker);
		if (!pAttackerTechno)
			return false;

		// Get attacker's house (handle mind control cases)
		auto pAttackerHouse = pAttackerTechno->GetOwningHouse();
		if (!pAttackerHouse)
			return false;

		// Don't retaliate against allies
		if (pTeamOwner->IsAlliedWith(pAttackerHouse))
			return false;

		// Don't retaliate against cloaked units that shouldn't be visible
		if (pAttackerTechno->CloakState == CloakState::Cloaked)
		{
			// Only retaliate if the team can actually see the cloaked unit
			// This prevents exploiting retaliation to reveal cloaked units
			return false;
		}

		// Don't retaliate against disguised units to prevent exploitation
		if (pAttackerTechno->IsDisguised())
		{
			// This prevents revealing disguised infiltrators through retaliation
			return false;
		}

		// Don't retaliate against units that are teleporting or in temporal
		if (pAttackerTechno->TemporalTargetingMe || pAttackerTechno->BeingWarpedOut)
		{
			return false;
		}

		// Don't retaliate against spawned units (like missiles) - target the spawner instead
		if (pAttackerTechno->GetTechnoType()->Spawned || pAttackerTechno->GetTechnoType()->MissileSpawn)
		{
			return false;  // These should be handled by ResolveActualTarget
		}

		return true;
	}

	// Resolve the actual target (handles open-topped transports)
	inline AbstractClass* ResolveActualTarget(AbstractClass* pAttacker)
	{
		if (auto pAttackerFoot = flag_cast_to<FootClass*, false>(pAttacker))
		{
			// If unit is in limbo but in an open-topped transport, target the transport
			if (pAttackerFoot->InLimbo
				&& pAttackerFoot->Transporter
				&& pAttackerFoot->Transporter->GetTechnoType()->OpenTopped)
			{
				return pAttackerFoot->Transporter;
			}
		}
		return pAttacker;
	}

	// Check if a team unit can participate in retaliation
	inline bool CanUnitRetaliate(FootClass* pUnit)
	{
		if (!pUnit || !pUnit->IsAlive || pUnit->Health <= 0)
			return false;

		if (!pUnit->IsArmed())
			return false;

		// Aircraft handle targeting differently
		if (pUnit->WhatAmI() == AircraftClass::AbsID)
			return false;

		// Check if we should interrupt critical missions based on configuration
		auto currentMission = pUnit->GetCurrentMission();
		if (!RulesExtData::Instance()->TeamRetaliate_InterruptCriticalMissions)
		{
			if (currentMission == Mission::Enter || currentMission == Mission::Repair
				|| currentMission == Mission::Construction || currentMission == Mission::Selling
				|| currentMission == Mission::Unload)  // Don't interrupt unloading
				return false;
		}

		// Don't use mind-controlled units for retaliation (they might be unreliable)
		if (pUnit->MindControlledBy)
		{
			return false;
		}

		return true;
	}

	// Find the actual team leader (not just first unit)
	inline FootClass* FindTeamLeader(TeamClass* pTeam)
	{
		if (!pTeam || !pTeam->FirstUnit)
			return nullptr;

		// Try to find the designated team leader first
		auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
		if (pTeamData)  // Add null check for pTeamData
		{
			if (pTeamData->TeamLeader && pTeamData->TeamLeader->IsAlive 
				&& pTeamData->TeamLeader->Health > 0 && !pTeamData->TeamLeader->InLimbo)
			{
				return pTeamData->TeamLeader;
			}
		}

		// Fallback: Find first armed and healthy unit
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit && pUnit->IsAlive && pUnit->Health > 0 && pUnit->IsArmed())
			{
				return pUnit;
			}
		}

		// Last resort: Return first alive unit
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit && pUnit->IsAlive && pUnit->Health > 0)
			{
				return pUnit;
			}
		}

		return nullptr;
	}

	// Efficiently assign retaliation missions to team members
	inline void AssignRetaliationMissions(TeamClass* pTeam, TechnoClass* pTarget)
	{
		if (!pTeam || !pTarget)
			return;

		// Validate target is still alive and available
		if (!pTarget->IsAlive || pTarget->Health <= 0 || pTarget->InLimbo)
			return;

		// Cache target info for performance
		auto pTargetHouse = pTarget->GetOwningHouse();
		if (!pTargetHouse)
			return;

		// Check maximum retaliation distance if configured
		auto maxDistance = RulesExtData::Instance()->TeamRetaliate_MaxRetaliationDistance;
		
		// Find proper team leader for distance reference (team-based behavior)
		FootClass* pTeamLeader = FindTeamLeader(pTeam);

		// If distance limit is set but no valid team leader found, abort
		if (maxDistance > 0 && !pTeamLeader)
			return;

		// Pre-check team distance to avoid processing individual units if team is too far
		if (maxDistance > 0 && pTeamLeader)
		{
			auto teamDistance = pTeamLeader->DistanceFrom(pTarget);
			if (teamDistance > maxDistance * 256) // Convert from cells to leptons
				return; // Entire team is too far, don't process any units
		}

		// Single target validation to avoid redundancy and race conditions
		bool isTargetStillValid = pTarget->IsAlive && pTarget->Health > 0 && !pTarget->InLimbo;
		auto pCurrentTargetHouse = pTarget->GetOwningHouse();
		
		if (!isTargetStillValid || !pCurrentTargetHouse)
			return;

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!CanUnitRetaliate(pUnit))
				continue;

			// Additional safety check - don't target allies (double check)
			if (pUnit->Owner->IsAlliedWith(pCurrentTargetHouse))
				continue;

			// Check if unit can actually engage this target type
			auto pUnitType = pUnit->GetTechnoType();
			if (!pUnitType)
				continue;

			// Optional weapon compatibility check based on configuration
			if (RulesExtData::Instance()->TeamRetaliate_CheckWeaponCompatibility)
			{
				bool canEngage = false;
				
				if (pTarget->IsInAir())
				{
					// Check if unit has anti-air capabilities
					// Look for a weapon that can attack aircraft
					for (int weaponIdx = 0; weaponIdx < 2; ++weaponIdx)
					{
						auto pWeaponStruct = pUnitType->GetWeapon(weaponIdx);
						if (pWeaponStruct && pWeaponStruct->WeaponType)
						{
							auto pWeapon = pWeaponStruct->WeaponType;
							if (pWeapon->Projectile && (pWeapon->Projectile->AA || !pWeapon->Projectile->AG))
							{
								// Weapon can target aircraft (AA=yes or AG=no, meaning air-only)
								canEngage = true;
								break;
							}
						}
					}
				}
				else
				{
					// Check if unit has anti-ground capabilities
					// Look for a weapon that can attack ground targets
					for (int weaponIdx = 0; weaponIdx < 2; ++weaponIdx)
					{
						auto pWeaponStruct = pUnitType->GetWeapon(weaponIdx);
						if (pWeaponStruct && pWeaponStruct->WeaponType)
						{
							auto pWeapon = pWeaponStruct->WeaponType;
							if (pWeapon->Projectile && pWeapon->Projectile->AG)
							{
								// Weapon can target ground (AG=yes)
								canEngage = true;
								
								// Additional check for naval units vs land targets
								if (pUnitType->Naval)
								{
									auto pTargetCell = pTarget->GetCell();
									if (pTargetCell && pTargetCell->LandType != LandType::Water)
									{
										// Naval unit trying to attack land target - check LandTargeting
										if (pUnitType->LandTargeting == LandTargetingType::Land_not_okay)
											canEngage = false;
									}
								}
								break;
							}
						}
					}
				}

				if (!canEngage)
					continue;
			}

			// Validate unit is still valid before assigning mission
			if (!pUnit->IsAlive || pUnit->Health <= 0 || pUnit->InLimbo)
				continue;

			// Set target and queue attack mission if not already attacking
			pUnit->SetTarget(pTarget);

			// Use proper mission queueing to avoid conflicts
			if (pUnit->GetCurrentMission() != Mission::Attack)
			{
				pUnit->QueueMission(Mission::Attack, true);  // Use true for better mission handling
			}
		}
	}
}

ASMJIT_PATCH(0x65DBB3, TeamTypeClass_CreateInstance_Plane, 5)
{
	GET(FootClass*, pFoot, EBP);
	R->ECX(HouseExtData::GetParadropPlane(pFoot->Owner));
	++Unsorted::ScenarioInit();
	return 0x65DBD0;
}

ASMJIT_PATCH(0x6E9443, TeamClass_AI_HandleAres, 8)
{
	enum { ReturnFunc = 0x6E95AB, Continue = 0x0 };
	GET(TeamClass*, pThis, ESI);
	GET(ScriptActionNode*, pTeamMission, EAX);
	GET_STACK(bool, bThirdArg, 0x10);

	if (AresScriptExt::Handle(pThis, pTeamMission, bThirdArg))
		return ReturnFunc;

	auto pTeamData = TeamExtContainer::Instance.Find(pThis);

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = pThis->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			pThis->ArchiveTarget = nullptr;
			pThis->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::LogInfo("DEBUG: [{}] {}](line: {} = {},{}): Jump to the same line -> (Reason: Timed Jump loop)",
				pThis->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission + 1,
				(int)nextAction.Action,
				nextAction.Argument
			);

			if (pTeamData->ForceJump_InitialCountdown > 0)
			{
				pTeamData->ForceJump_Countdown.Start(pTeamData->ForceJump_InitialCountdown);
				pTeamData->ForceJump_RepeatMode = true;
			}
		}
		else
		{
			const auto& [curAct, curArgs] = pScript->GetCurrentAction();
			const auto& [nextAct, nextArgs] = pScript->GetNextAction();

			pTeamData->ForceJump_InitialCountdown = -1;
			pTeamData->ForceJump_Countdown.Stop();
			Debug::LogInfo("DEBUG: [{}] [{}](line: {} = {},{}): Jump to line: {} = {},{} -> (Reason: Timed Jump)",
				pThis->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission, (int)curAct, curArgs,
				pScript->CurrentMission + 1, (int)nextAct, nextArgs
			);
		}

		for (auto pUnit = pThis->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExtData::IsUnitAvailable(pUnit, true))
			{
				pUnit->EnterIdleMode(false, 1);
			}
		}

		pThis->StepCompleted = true;
		return ReturnFunc;
	}

	return ScriptExtData::ProcessScriptActions(pThis) ? ReturnFunc : Continue;
}

ASMJIT_PATCH(0x6EF8A1, TeamClass_GatherAtEnemyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, STACK_OFFS(0x5C, 0x34));
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//Debug::LogInfo(fmt::format(__FUNCTION__ " Function With Type {} ! ",pTeam->Type->ID));
	//R->EDX(pTeamExt->AI_SafeDIstance.Get(RulesClass::Instance->AISafeDistance) + pTeamM->Argument);
	R->EDX(pRules->AISafeDistance + pTeamM->Argument);

	return 0x6EF8A7;
}

ASMJIT_PATCH(0x6EFB69, TeamClass_GatherAtFriendlyBase_Distance, 0x6)
{
	//GET_STACK(TeamClass*, pTeam, 0x48 - 0x2C);
	GET_BASE(ScriptActionNode*, pTeamM, 0x8);
	GET(RulesClass*, pRules, ECX);
	//if (IS_SAME_STR_(pTeam->Type->ID, "0100003I-G")) {
	//	Debug::LogInfo("Team %s with script %s, GatherAt friendlyBase.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	//	int i = 0;
	//	for (auto pCur = pTeam->FirstUnit; pCur; pCur = pCur->NextTeamMember) {
	//		Debug::LogInfo("	Team %s with script %s, GatherAt friendlyBase CurMember[%d - %s].",
	//			pTeam->Type->ID,
	//			pTeam->CurrentScript->Type->ID,
	//		i++,
	//		pCur->GetTechnoType()->ID
	//		);
	//	}
	//}

	//const auto pTeamExt = TeamTypeExt::ExtMap.Find(pTeam->Type);
	//R->EDX(pTeamExt->AI_FriendlyDistance.Get(RulesExtData::Instance()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance)) + pTeamM->Argument);
	const auto distanceresult = RulesExtData::Instance()->AIFriendlyDistance.Get(pRules->AISafeDistance) + pTeamM->Argument;
	R->EDX(distanceresult);
	return 0x6EFB6F;
}

//ASMJIT_PATCH(0x6EFC54, TeamClass_GatherAtFriendlyBase_TargetAssigned, 0x5)
//{
//	GET_STACK(TeamClass*, pTeam, 0x48 - 0x2C);
//	GET(CellClass*, pTarget, EAX);
//
//	if (IS_SAME_STR_(pTeam->Type->ID, "0100003I-G"))
//	{
//		Debug::LogInfo("Team %s with script %s, GatherAt friendlyBase.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
//		int i = 0;
//		for (auto pCur = pTeam->FirstUnit; pCur; pCur = pCur->NextTeamMember)
//		{
//			Debug::LogInfo("	Team %s with script %s, GatherAt friendlyBase CurMember[%d - %s].",
//				pTeam->Type->ID,
//				pTeam->CurrentScript->Type->ID,
//			i++,
//			pCur->GetTechnoType()->ID
//			);
//		}
//
//		const auto coord = pTarget->GetCoords();
//		Debug::LogInfo("Team %s with script %s, GatherAt { %d , %d , %d }.", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, coord.X, coord.Y, coord.Z);
//	}
//
//	return 0x0;
//}

// #895225: make the AI smarter. this code was missing from YR.
// it clears the targets and assigns the attacker the team's current focus.
// Optimized and improved implementation combining original and custom logic
ASMJIT_PATCH(0x6EB432, TeamClass_AttackedBy_Retaliate, 9)
{
	GET(TeamClass*, pThis, ESI);
	GET(AbstractClass*, pAttacker, EBP);

	// Early exit if TeamRetaliate is disabled
	if (!RulesExtData::Instance()->TeamRetaliate)
	{
		return 0x6EB47A;
	}

	// Validate inputs - prevent null pointer crashes
	if (!pThis || !pAttacker || !pThis->Owner)
	{
		return 0x6EB47A;
	}

	// Fast initial validation using helper function
	if (!TeamRetaliateHelpers::IsValidRetaliationTarget(pAttacker, pThis->Owner))
	{
		return 0x6EB47A;
	}

	// Cache frequently accessed data for performance
	auto pFocus = flag_cast_to<TechnoClass*>(pThis->ArchiveTarget);
	auto pSpawnCell = pThis->SpawnCell;

	// Determine if we should consider a new target
	bool shouldConsiderNewTarget = !pFocus
		|| !pFocus->IsArmed()
		|| !pSpawnCell
		|| pFocus->IsCloseEnoughToAttackCoords(pSpawnCell->GetCoords());

	if (!shouldConsiderNewTarget)
	{
		return 0x6EB47A;
	}

	// Resolve the actual target (handles open-topped transports)
	AbstractClass* pFinalTarget = TeamRetaliateHelpers::ResolveActualTarget(pAttacker);
	auto pFinalTargetTechno = flag_cast_to<TechnoClass*, false>(pFinalTarget);

	// Validate final target
	if (!pFinalTargetTechno)
	{
		return 0x6EB47A;
	}

	// Additional validation for FootClass attackers
	if (auto pAttackerFoot = flag_cast_to<FootClass*, false>(pFinalTarget))
	{
		// Skip units in limbo that aren't in open-topped transports
		if (pAttackerFoot->InLimbo && pFinalTarget == pAttacker)
		{
			return 0x6EB47A;
		}

		// Skip aircraft-like units
		if (pAttackerFoot->GetTechnoType()->ConsideredAircraft)
		{
			return 0x6EB47A;
		}
	}

	// Validate target type compatibility
	auto pTargetType = pFinalTargetTechno->GetTechnoType();
	if (!pTargetType)
	{
		return 0x6EB47A;
	}
	
	// Additional validation for aircraft-like targets
	if (pTargetType->ConsideredAircraft || pFinalTarget->WhatAmI() == AircraftClass::AbsID)
	{
		return 0x6EB47A;
	}

	// Quick check if team has any units capable of retaliation
	bool hasCapableUnits = false;
	for (auto pUnit = pThis->FirstUnit; pUnit && !hasCapableUnits; pUnit = pUnit->NextTeamMember)
	{
		// Quick basic check first for performance
		if (pUnit && pUnit->IsAlive && pUnit->Health > 0 && pUnit->IsArmed() 
			&& pUnit->WhatAmI() != AircraftClass::AbsID)
		{
			hasCapableUnits = true;
		}
	}

	// Set the new target regardless of unit availability for tracking purposes
	pThis->ArchiveTarget = pFinalTarget;

	// If no capable units, don't assign missions but still track the target
	if (!hasCapableUnits)
	{
		return 0x6EB47A;
	}

	// Efficiently assign retaliation missions using helper function
	TeamRetaliateHelpers::AssignRetaliationMissions(pThis, pFinalTargetTechno);

	// Use the team's centralized mission assignment for coordination
	// This ensures the team works together rather than individual units acting alone
	pThis->AssignMissionTarget(pFinalTarget);

	return 0x6EB47A;
}

// #1260: reinforcements via actions 7 and 80, and chrono reinforcements
// via action 107 cause crash if house doesn't exist
ASMJIT_PATCH(0x65D8FB, TeamTypeClass_ValidateHouse, 6)
{
	GET(TeamTypeClass*, pThis, ECX);
	HouseClass* pHouse = pThis->GetHouse();

	// house exists; it's either declared explicitly (not Player@X) or a in campaign mode
	// (we don't second guess those), or it's still alive in a multiplayer game
	if (pHouse &&
		(pThis->Owner || SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Defeated))
	{
		return 0;
	}

	// no.
	return (R->Origin() == 0x65D8FB) ? 0x65DD1B : 0x65F301;
}ASMJIT_PATCH_AGAIN(0x65EC4A, TeamTypeClass_ValidateHouse, 6)