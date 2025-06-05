#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>

#include <TeamTypeClass.h>

#include <algorithm>
#include <vector>

// Contains ScriptExtData::Mission_Move and its helper functions.

namespace {
	// Constants to replace magic numbers
	constexpr int WAIT_NO_TARGET_FRAMES = 30;
	constexpr double THREAT_MULTIPLIER = 128.0;
	constexpr double DISTANCE_DIVISOR = 256.0;
	constexpr int KICKOUT_DISTANCE = 10;

	// Helper function to check if any aircraft in team lacks ammo
	bool HasAircraftWithoutAmmo(const TeamClass* pTeam) {
		if (!pTeam) {
			return false;
		}
		
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember) {
			if (!ScriptExtData::IsUnitAvailable(pUnit, false))
				continue;

			if (pUnit->WhatAmI() == AbstractType::Aircraft) {
				auto pTechnoType = pUnit->GetTechnoType();
				auto pAircraftType = static_cast<AircraftTypeClass*>(pTechnoType);
				
				if (!pUnit->IsInAir() 
					&& pAircraftType->AirportBound 
					&& pUnit->Ammo < pTechnoType->Ammo) {
					return true;
				}
			}
		}
		return false;
	}

	// Helper function to clean up dead team members
	void CleanupDeadMembers(TeamClass* pTeam) {
		if (!pTeam) {
			return;
		}
		
		FootClass* pCur = pTeam->FirstUnit;
		while (pCur) {
			FootClass* pNext = pCur->NextTeamMember;
			
			if (!ScriptExtData::IsUnitAvailable(pCur, false)) {
				pTeam->RemoveMember(pCur, -1, 1);
			}
			
			pCur = pNext;
		}
	}

	// Helper function to reset team state when mission completes
	void ResetTeamState(TeamExtData* pTeamData) {
		if (!pTeamData) {
			return;
		}
		
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->CloseEnough > 0) {
			pTeamData->CloseEnough = -1;
		}
		if (pTeamData->WaitNoTargetAttempts != 0) {
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}
	}

	// Helper function to issue move commands to team members
	void IssueMoveCommands(TeamClass* pTeam, TechnoClass* pTarget) {
		if (!pTeam || !pTarget) {
			return;
		}
		
		for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember) {
			if (!ScriptExtData::IsUnitAvailable(pFoot, false))
				continue;

			auto pTechnoType = pFoot->GetTechnoType();
			auto pTargetCell = pTarget->GetCell();

			// Check if naval unit can target the destination
			if (pTargetCell && pTechnoType->Underwater 
				&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay 
				&& pTargetCell->LandType != LandType::Water) {
				
				pFoot->SetTarget(nullptr);
				pFoot->SetDestination(nullptr, false);
				pFoot->QueueMission(Mission::Area_Guard, true);
				continue;
			}

			// Reset previous commands
			pFoot->SetTarget(nullptr);
			pFoot->SetDestination(nullptr, false);
			pFoot->ForceMission(Mission::Guard);

			// Set new move destination
			pFoot->QueueMission(Mission::Move, false);
			CoordStruct coord = TechnoExtData::PassengerKickOutLocation(pTarget, pFoot, KICKOUT_DISTANCE);
			coord = coord != CoordStruct::Empty ? coord : pTarget->Location;

			if (CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord)) {
				pFoot->SetDestination(pCellDestination, true);
			}

			// Special handling for aircraft
			if (pFoot->WhatAmI() == AbstractType::Aircraft 
				&& pFoot->Ammo > 0 
				&& !pFoot->IsInAir()) {
				pFoot->QueueMission(Mission::Move, false);
			}
		}
	}

	// Helper function to check if object is valid for naval targeting
	bool IsValidNavalTarget(TechnoClass* pTechno, TechnoClass* pObj) {
		if (!pTechno || !pObj) {
			return false;
		}
		
		auto pTechnoType = pTechno->GetTechnoType();
		auto pObjectType = pObj->GetTechnoType();
		
		if (!pTechnoType || !pObjectType) {
			return false;
		}

		if (!pObjectType->Naval) {
			return true;
		}

		// Check submarine targeting
		if (pObj->CloakState == CloakState::Cloaked 
			&& pObjectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
				|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_none)) {
			return false;
		}

		// Check land targeting for naval units
		auto pObjCell = pObj->GetCell();
		if (pObjCell && pTechnoType->Underwater 
			&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& pObjCell->LandType != LandType::Water) {
			return false;
		}

		// Check stealth
		if (pObjCell && pObj->CloakState == CloakState::Cloaked 
			&& !pObjCell->Sensors_InclHouse(pTechno->Owner->ArrayIndex)) {
			return false;
		}

		return true;
	}

	// Helper function to calculate threat value
	double CalculateThreatValue(TechnoClass* pTechno, TechnoClass* pObj, DistanceMode calcThreatMode) {
		if (!pTechno || !pObj) {
			return 0.0;
		}
		
		double value = 0.0;

		if (calcThreatMode == DistanceMode::idkZero || calcThreatMode == DistanceMode::idkOne) {
			// Threat affected by distance
			double objectThreatValue = pObj->GetThreatValue();
			auto pObjectType = pObj->GetTechnoType();
			
			if (!pObjectType) {
				return 0.0;
			}

			if (pObjectType->SpecialThreatValue > 0) {
				objectThreatValue += pObjectType->SpecialThreatValue * RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
			}

			// Enemy house targeting bonus
			if (pObj->Owner->EnemyHouseIndex >= 0 
				&& pObj->Owner->EnemyHouseIndex < HouseClass::Array->Count
				&& pTechno->Owner == HouseClass::Array->Items[pObj->Owner->EnemyHouseIndex]) {
				objectThreatValue += RulesClass::Instance->EnemyHouseThreatBonus;
			}

			// Health-based threat bonus
			objectThreatValue += pObj->Health * (1.0 - pObj->GetHealthPercentage());
			
			double distance = pTechno->DistanceFrom(pObj) / DISTANCE_DIVISOR;
			// Ensure we don't divide by zero or very small numbers
			double adjustedDistance = std::max(distance + 1.0, 1.0);
			value = (objectThreatValue * THREAT_MULTIPLIER) / adjustedDistance;
		} else {
			// Distance-only calculation
			value = pTechno->DistanceFrom(pObj);
		}

		return value;
	}

	// Helper function to determine if target is better based on calculation mode
	bool IsBetterTarget(double currentValue, double bestValue, DistanceMode calcThreatMode) {
		if (bestValue < 0) {
			return true; // First valid target
		}

		switch (calcThreatMode) {
			case DistanceMode::idkZero:
			case DistanceMode::Furtherst:
				return currentValue > bestValue;
			case DistanceMode::idkOne:
			case DistanceMode::Closest:
				return currentValue < bestValue;
			default:
				return false;
		}
	}
}

void ScriptExtData::Mission_Move(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem)
{
	if (!pTeam) {
		return;
	}

	auto pScript = pTeam->CurrentScript;
	if (!pScript) {
		pTeam->StepCompleted = true;
		return;
	}

	auto const& [act, scriptArgument] = pScript->GetCurrentAction();
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData) {
		pTeam->StepCompleted = true;
		return;
	}
	
	bool noWaitLoop = false;

	// Handle wait timer for no target scenarios
	if (pTeamData->WaitNoTargetCounter > 0) {
		if (pTeamData->WaitNoTargetTimer.InProgress()) {
			return;
		}

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0) {
			pTeamData->WaitNoTargetAttempts--;
		}
	}

	// Clean up dead team members
	CleanupDeadMembers(pTeam);

	// Check if team has any units left
	if (!pTeam->FirstUnit) {
		ResetTeamState(pTeamData);
		pTeam->StepCompleted = true;
		return;
	}

	// Check for aircraft without ammo
	bool bAircraftsWithoutAmmo = HasAircraftWithoutAmmo(pTeam);

	// Find the team leader
	if (!pTeamData->TeamLeader) {
		pTeamData->TeamLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	}

	// Validate team leader is still available
	if (pTeamData->TeamLeader && !ScriptExtData::IsUnitAvailable(pTeamData->TeamLeader, false)) {
		pTeamData->TeamLeader = nullptr;
	}

	if (!pTeamData->TeamLeader || bAircraftsWithoutAmmo) {
		ResetTeamState(pTeamData);
		pTeam->StepCompleted = true;
		return;
	}

	TechnoClass* pFocus = flag_cast_to<TechnoClass*>(pTeam->ArchiveTarget);

	if (!pFocus && !bAircraftsWithoutAmmo) {
		// Find new target
		int targetMask = scriptArgument;
		auto selectedTarget = ScriptExtData::FindBestObject(
			pTeamData->TeamLeader,
			targetMask,
			calcThreatMode,
			pickAllies,
			attackAITargetType,
			idxAITargetTypeItem
		);

		if (selectedTarget) {
			pTeam->ArchiveTarget = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0;
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;

			IssueMoveCommands(pTeam, selectedTarget);
		} else {
			// No target found - handle waiting
			if (!noWaitLoop && pTeamData->WaitNoTargetTimer.Completed()) {
				pTeamData->WaitNoTargetCounter = WAIT_NO_TARGET_FRAMES;
				pTeamData->WaitNoTargetTimer.Start(WAIT_NO_TARGET_FRAMES);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0) {
				pTeamData->IdxSelectedObjectFromAIList = -1;
			}

			if (pTeamData->WaitNoTargetAttempts != 0 && pTeamData->WaitNoTargetTimer.Completed()) {
				pTeamData->WaitNoTargetCounter = WAIT_NO_TARGET_FRAMES;
				pTeamData->WaitNoTargetTimer.Start(WAIT_NO_TARGET_FRAMES);
				return;
			}

			if (pTeamData->CloseEnough >= 0) {
				pTeamData->CloseEnough = -1;
			}

			pTeam->StepCompleted = true;
			return;
		}
	} else {
		// Update existing move mission
		if (ScriptExtData::MoveMissionEndStatus(pTeam, pFocus, pTeamData->TeamLeader, pTeamData->MoveMissionEndMode)) {
			pTeamData->MoveMissionEndMode = 0;
			pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->CloseEnough >= 0) {
				pTeamData->CloseEnough = -1;
			}

			pTeam->StepCompleted = true;
			return;
		}
	}
}

TechnoClass* ScriptExtData::FindBestObject(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem)
{
	if (!pTechno || !pTechno->Owner) {
		return nullptr;
	}

	TechnoClass* bestObject = nullptr;
	double bestVal = -1.0;
	HouseClass* enemyHouse = nullptr;
	auto pTechnoType = pTechno->GetTechnoType();
	
	if (!pTechnoType) {
		return nullptr;
	}

	// Determine enemy house for focused targeting
	if (!pickAllies && pTechno->BelongsToATeam()) {
		if (auto pFoot = flag_cast_to<FootClass*, false>(pTechno)) {
			const int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;
			const auto pHouseExt = HouseExtContainer::Instance.Find(pFoot->Team->Owner);
			const bool onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemyMode != -1 ?
				pFoot->Team->Type->OnlyTargetHouseEnemy : pHouseExt->m_ForceOnlyTargetHouseEnemy;

			if (onlyTargetHouseEnemy && enemyHouseIndex >= 0 && enemyHouseIndex < HouseClass::Array->Count) {
				enemyHouse = HouseClass::Array->Items[enemyHouseIndex];
			}
		}
	}

	// Cache owner for performance
	HouseClass* pTechnoOwner = pTechno->Owner;

	// Evaluate all potential targets
	TechnoClass::Array->for_each([&](TechnoClass* pObj) {
		// Early exits for invalid targets
		if (!ScriptExtData::IsUnitAvailable(pObj, true) 
			|| pObj == pTechno
			|| pObj->InWhichLayer() == Layer::Underground
			|| (enemyHouse && enemyHouse != pObj->Owner)) {
			return;
		}

		// Check alliance requirements
		bool isAllied = pTechnoOwner->IsAlliedWith(pObj);
		if ((pickAllies && !isAllied) || (!pickAllies && isAllied)) {
			return;
		}

		// Validate naval targeting
		if (!IsValidNavalTarget(pTechno, pObj)) {
			return;
		}

		// Check if object matches targeting criteria
		if (!ScriptExtData::EvaluateObjectWithMask(pObj, method, attackAITargetType, idxAITargetTypeItem, pTechno)) {
			return;
		}

		// Calculate target value
		double value = CalculateThreatValue(pTechno, pObj, calcThreatMode);

		// Check if this is a better target
		if (IsBetterTarget(value, bestVal, calcThreatMode)) {
			bestObject = pObj;
			bestVal = value;
		}
	});

	return bestObject;
}

void ScriptExtData::Mission_Move_List(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType)
{
	if (!pTeam || !pTeam->CurrentScript) {
		return;
	}

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData) {
		return;
	}
	
	pTeamData->IdxSelectedObjectFromAIList = -1;
	
	const auto& [curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();

	if (attackAITargetType < 0) {
		attackAITargetType = curArg;
	}

	const auto& targetLists = RulesExtData::Instance()->AITargetTypesLists;
	if (static_cast<size_t>(attackAITargetType) < targetLists.size() && !targetLists[attackAITargetType].empty()) {
		ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
	}
}

void ScriptExtData::Mission_Move_List1Random(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem)
{
	if (!pTeam || !pTeam->CurrentScript) {
		return;
	}

	auto pScript = pTeam->CurrentScript;
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData) {
		return;
	}
	
	const auto& [curAct, curArg] = pScript->GetCurrentAction();

	if (attackAITargetType < 0) {
		attackAITargetType = curArg;
	}

	const auto& targetLists = RulesExtData::Instance()->AITargetTypesLists;
	if (static_cast<size_t>(attackAITargetType) >= targetLists.size()) {
		return;
	}

	// If we already have a selected target, use it
	const auto& currentList = targetLists[attackAITargetType];
	if (static_cast<size_t>(pTeamData->IdxSelectedObjectFromAIList) < currentList.size()) {
		ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, pTeamData->IdxSelectedObjectFromAIList);
		return;
	}

	// Find available targets from the list
	if (currentList.empty()) {
		return;
	}

	std::vector<int> validIndexes;
	validIndexes.reserve(currentList.size()); // Pre-allocate for better performance

	// Cache team owner for performance
	HouseClass* pTeamOwner = pTeam->FirstUnit ? pTeam->FirstUnit->Owner : nullptr;
	if (!pTeamOwner) {
		return;
	}

	TechnoClass::Array->for_each([&](TechnoClass* pTechno) {
		if (!ScriptExtData::IsUnitAvailable(pTechno, true)) {
			return;
		}

		bool isAllied = pTeamOwner->IsAlliedWith(pTechno);
		if ((pickAllies && !isAllied) || (!pickAllies && isAllied)) {
			return;
		}

		auto pTechnoType = pTechno->GetTechnoType();
		
		// Check if this techno type is in our target list
		for (size_t j = 0; j < currentList.size(); ++j) {
			if (pTechnoType == currentList[j]) {
				validIndexes.push_back(static_cast<int>(j));
				break; // Found match, no need to continue checking
			}
		}
	});

	if (!validIndexes.empty()) {
		const int selectedIndex = validIndexes[ScenarioClass::Instance->Random.RandomFromMax(static_cast<int>(validIndexes.size() - 1))];
		pTeamData->IdxSelectedObjectFromAIList = selectedIndex;
		ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, selectedIndex);
	}
}
