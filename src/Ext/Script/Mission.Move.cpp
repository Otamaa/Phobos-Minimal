#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>

// Contains ScriptExtData::Mission_Move and its helper functions.

void ScriptExtData::Mission_Move(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	bool noWaitLoop = false;
	bool bAircraftsWithoutAmmo = false;

	if (!pScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	auto const& [act, scriptArgument] = pScript->GetCurrentAction();// This is the target type

	// This team has no units!
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		auto const& [nextAct, nextArg] = pScript->GetNextAction();
		ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			act,
			scriptArgument,
			pScript->CurrentMission + 1,
			nextAct,
			nextArg);

		return;
	}

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		auto const& [nextAct, nextArg] = pScript->GetNextAction();
		ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n",
			pTeam->Type->ID,
			pScript->CurrentMission,
			act,
			scriptArgument,
			pScript->Type->ID,
			pScript->CurrentMission + 1,
			nextAct,
			nextArg);

		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}


	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (ScriptExtData::IsUnitAvailable(pFoot,false))
		{
			auto const pTechnoType = pFoot->GetTechnoType();

			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& !pFoot->IsInAir()
				&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound
				&& pFoot->Ammo < pTechnoType->Ammo)
			{
				bAircraftsWithoutAmmo = true;
			}
		}
	}

	// Find the Leader
	if (!ScriptExtData::IsUnitAvailable(pTeamData->TeamLeader, true)) {
		pTeamData->TeamLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	}

	if (!pTeamData->TeamLeader || bAircraftsWithoutAmmo)
	{

		pTeamData->IdxSelectedObjectFromAIList = -1;

		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		auto const& [nextAct, nextArg] = pScript->GetNextAction();
		ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reasons: No Leader | Aircrafts without ammo)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			act,
			scriptArgument,
			pScript->CurrentMission + 1,
			nextAct,
			nextArg);

		return;
	}

	TechnoTypeClass * pLeaderUnitType = pTeamData->TeamLeader->GetTechnoType();
	TechnoClass* pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.
		int targetMask = scriptArgument;
		auto selectedTarget = ScriptExtData::FindBestObject(
			pTeamData->TeamLeader,
			targetMask,
			calcThreatMode,
			pickAllies,
			attackAITargetType,
			idxAITargetTypeItem
		);

		if (selectedTarget)
		{
			ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as destination target.\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				act,
				scriptArgument,
				pLeaderUnitType->get_ID(),
				pTeamData->TeamLeader->UniqueID,
				selectedTarget->GetTechnoType()->get_ID(),
				selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExtData::IsUnitAvailable(pFoot, false)
					&& ScriptExtData::IsUnitAvailable(selectedTarget, false)
					)
				{
					auto const pTechnoType = pFoot->GetTechnoType();

					if (pTechnoType->Underwater && pTechnoType->LandTargeting == LandTargetingType::Land_not_okay && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
						pFoot->SetTarget(nullptr);
						pFoot->SetDestination(nullptr, false);
						pFoot->QueueMission(Mission::Area_Guard, true);

						continue;
					}

					// Reset previous command
					pFoot->SetTarget(nullptr);
					pFoot->SetDestination(nullptr, false);
					pFoot->ForceMission(Mission::Guard);

					// Get a cell near the target
					pFoot->QueueMission(Mission::Move, false);
					CoordStruct coord = TechnoExtData::PassengerKickOutLocation(selectedTarget, pFoot, 10);
					coord = coord != CoordStruct::Empty ? coord : selectedTarget->Location;

					if(CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord))
						pFoot->SetDestination(pCellDestination, true);

					// Aircraft hack. I hate how this game auto-manages the aircraft missions.
					if (pFoot->WhatAmI() == AbstractType::Aircraft && pFoot->Ammo > 0 && !pFoot->IsInAir())
						pFoot->QueueMission(Mission::Move, false);
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.

			if (!noWaitLoop && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0 && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30); // No target? let's wait some frames

				return;
			}

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			auto const& [nextAct, nextArg] = pScript->GetNextAction();
			ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (new target NOT FOUND)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				act,
				scriptArgument,
				pScript->CurrentMission + 1,
				nextAct,
				nextArg);

			return;
		}
	}
	else
	{

		// This part of the code is used for updating the "Move" mission in each team unit
		int moveDestinationMode = 0;
		moveDestinationMode = pTeamData->MoveMissionEndMode;
		bool bForceNextAction = ScriptExtData::MoveMissionEndStatus(pTeam, pFocus, pTeamData->TeamLeader, moveDestinationMode);


		if (bForceNextAction)
		{
			pTeamData->MoveMissionEndMode = 0;
			pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			auto const& [nextAct, nextArg] = pScript->GetNextAction();
			ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Reached destination)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				act,
				scriptArgument,
				pScript->CurrentMission + 1,
				nextAct,
				nextArg
			);
			return;
		}
	}
}

TechnoClass* ScriptExtData::FindBestObject(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	HouseClass* enemyHouse = nullptr;
	auto pTechnoType = pTechno->GetTechnoType();

	// Favorite Enemy House case. If set, AI will focus against that House
	if (!pickAllies && pTechno->BelongsToATeam())
	{
		if (auto pFoot = abstract_cast<FootClass*>(pTechno))
		{
			const int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;
			const auto pHouseExt = HouseExtContainer::Instance.Find(pFoot->Team->Owner);
			const bool onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemyMode != -1 ?
			pFoot->Team->Type->OnlyTargetHouseEnemy : pHouseExt->m_ForceOnlyTargetHouseEnemy ;

			if (onlyTargetHouseEnemy && enemyHouseIndex >= 0)
				enemyHouse = HouseClass::Array->GetItem(enemyHouseIndex);
		}
	}

	// Generic method for targeting
	TechnoClass::Array->for_each([&](TechnoClass* pObj) {
		if (!ScriptExtData::IsUnitAvailable(pObj, true) || pObj == pTechno)
			return;

		//if (pTechno->Spawned)
		//	return;

		if (enemyHouse && enemyHouse != pObj->Owner)
			return;

		// Don't pick underground units
		if (pObj->InWhichLayer() == Layer::Underground)
			return;

		if (auto objectType = pObj->GetTechnoType())
		{
			// Stealth ground unit check
			if (pObj->CloakState == CloakState::Cloaked && !objectType->Naval)
				return;

			// Submarines aren't a valid target
			if (pObj->CloakState == CloakState::Cloaked
				&& objectType->Underwater
				&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
					|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
			{
				return;
			}

			// Land not OK for the Naval unit
			if (objectType->Naval
				&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
				&& pObj->GetCell()->LandType != LandType::Water)
			{
				return;
			}

			if ((pickAllies && pTechno->Owner->IsAlliedWith(pObj)) || (!pickAllies && !pTechno->Owner->IsAlliedWith(pObj)))
			{
				double value = 0;

				if (ScriptExtData::EvaluateObjectWithMask(pObj, method, attackAITargetType, idxAITargetTypeItem, pTechno))
				{
					bool isGoodTarget = false;

					if (calcThreatMode == DistanceMode::idkZero || calcThreatMode == DistanceMode::idkOne)
					{
						// Threat affected by distance
						double threatMultiplier = 128.0;
						double objectThreatValue = pObj->GetThreatValue();

						if (objectType->SpecialThreatValue > 0)
						{
							double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
							objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
						}

						// Is Defender house targeting Attacker House? if "yes" then more Threat
						if (pTechno->Owner == HouseClass::Array->GetItem(pObj->Owner->EnemyHouseIndex))
						{
							double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
							objectThreatValue += EnemyHouseThreatBonus;
						}

						// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
						objectThreatValue += pObj->Health * (1 - pObj->GetHealthPercentage());
						value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(pObj) / 256.0) + 1.0);

						if (calcThreatMode == DistanceMode::idkZero)
						{
							// Is this object very FAR? then LESS THREAT against pTechno.
							// More CLOSER? MORE THREAT for pTechno.
							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
						else
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							if (value < bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
					else
					{
						// Selection affected by distance
						if (calcThreatMode == DistanceMode::Closest)
						{
							// Is this object very FAR? then LESS THREAT against pTechno.
							// More CLOSER? MORE THREAT for pTechno.
							value = pTechno->DistanceFrom(pObj); // Note: distance is in leptons (*256)

							if (value < bestVal || bestVal < 0)
								isGoodTarget = true;
						}
						else
						{
							if (calcThreatMode == DistanceMode::Furtherst)
							{
								// Is this object very FAR? then MORE THREAT against pTechno.
								// More CLOSER? LESS THREAT for pTechno.
								value = pTechno->DistanceFrom(pObj); // Note: distance is in leptons (*256)

								if (value > bestVal || bestVal < 0)
									isGoodTarget = true;
							}
						}
					}

					if (isGoodTarget)
					{
						bestObject = pObj;
						bestVal = value;
					}
				}
			}
		}
	});

	return bestObject;
}

void ScriptExtData::Mission_Move_List(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType)
{
	TeamExtContainer::Instance.Find(pTeam)->IdxSelectedObjectFromAIList = -1;
	const auto& [curAct, curArg] = pTeam->CurrentScript->GetCurrentAction();

	if (attackAITargetType < 0)
		attackAITargetType = curArg;

	const auto& Arr = RulesExtData::Instance()->AITargetTypesLists;
	if ((size_t)attackAITargetType < Arr.size() && !Arr[attackAITargetType].empty())
	{
		ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
		return;
	}

	pTeam->StepCompleted = true;
	ScriptExtData::Log("AI Scripts - Mission_Move_List: [%s] [%s] (line: %d = %d,%d) Failed to get the list index [AITargetTypes][%d]! out of bound: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArg,
		attackAITargetType,
		Arr.size());
}

void ScriptExtData::Mission_Move_List1Random(TeamClass* pTeam, DistanceMode calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	std::vector<int> validIndexes;
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	const auto& [curAct, curArg] = pScript->GetCurrentAction();

	if (attackAITargetType < 0)
		attackAITargetType = curArg;

	if ((size_t)attackAITargetType < RulesExtData::Instance()->AITargetTypesLists.size())
	{

		if ((size_t)pTeamData->IdxSelectedObjectFromAIList < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size()) {
			ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, pTeamData->IdxSelectedObjectFromAIList);
			return;
		}

		// Still no random target selected
		if (!RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].empty()) {
			// Finding the objects from the list that actually exists in the map
			TechnoClass::Array->for_each([&](TechnoClass* pTechno)
			{
				if(!ScriptExtData::IsUnitAvailable(pTechno, true))
					return;

				//if (pTechno->Spawned)
				//	return;

				auto pTechnoType = pTechno->GetTechnoType();
				bool found = false;

				for (auto j = 0u; j < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size() && !found; j++)
				{
					if (pTechnoType == RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][j]
						&& ((pickAllies
							&& pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))
							|| (!pickAllies
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))))
					{
						validIndexes.push_back(j);
						found = true;
					}
				}
			});

			if (!validIndexes.empty())
			{
				const int idxsel = validIndexes[ScenarioClass::Instance->Random.RandomFromMax(validIndexes.size() - 1)];
				pTeamData->IdxSelectedObjectFromAIList = idxsel;
				ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pScript->CurrentMission,
					curAct,
					curArg,
					attackAITargetType, idxsel,
					RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][idxsel]->ID);

				ScriptExtData::Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, idxsel);

				return;
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExtData::Log("AI Scripts - Move: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pScript->CurrentMission,
		curAct,
		curArg,
		attackAITargetType,
		validIndexes.size());
}
