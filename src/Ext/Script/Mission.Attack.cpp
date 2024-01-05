#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

// Contains ScriptExtData::Mission_Attack and its helper functions.

void ScriptExtData::Mission_Attack(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	// This is the target type
	const auto& [curAct, scriptArgument] = pScript->GetCurrentAction();
	const auto& [nextAct, nextArg] = pScript->GetNextAction();

	if (!pScript){
		pTeam->StepCompleted = true;
		return;
	}

	//TechnoClass* selectedTarget = nullptr;
	//HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	//FootClass* pLeaderUnit = nullptr;
	//TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	//TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData) {
		pTeam->StepCompleted = true;
		ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n",
			pTeam->Type->ID, pScript->Type->ID,
			pScript->CurrentMission,
			curAct,
			scriptArgument,
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

	// This team has no units!
	if (!pTeam)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			curAct,
			scriptArgument,
			pScript->CurrentMission + 1,
			nextAct,
			nextArg
		);

		return;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!ScriptExtData::IsUnitAvailable(pFocus, true))
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExtContainer::Instance.Find(pFoot);

		if (pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				ScriptExtData::IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
				pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pTeam->Focus = nullptr;
			pFocus = nullptr;

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

				pTeamData->IdxSelectedObjectFromAIList = -1;

				// This action finished
				pTeam->StepCompleted = true;
				ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n",
					pTeam->Type->ID,
					pScript->Type->ID,
					pScript->CurrentMission,
					curAct,
					scriptArgument,
					pScript->CurrentMission + 1,
					nextAct,
					nextArg);

				return;
			}
		}
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (ScriptExtData::IsUnitAvailable(pFoot, true))
		{
			auto const pTechnoType = pFoot->GetTechnoType();

			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& !pFoot->IsInAir()
				&& static_cast<const AircraftTypeClass*>(pTechnoType)->AirportBound
				&& pFoot->Ammo < pTechnoType->Ammo)
			{
				bAircraftsWithoutAmmo = true;
			}

			pacifistTeam &= !ScriptExtData::IsUnitArmed(pFoot);

			if (pFoot->WhatAmI() == AbstractType::Infantry)
			{
				auto const pTypeInf = static_cast<const InfantryTypeClass*>(pTechnoType);

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					agentMode = true;
			}
		}
	}

	// Find the Leader
	if (!ScriptExtData::IsUnitAvailable(pTeamData->TeamLeader, true))
	{
		pTeamData->TeamLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	}

	if (!pTeamData->TeamLeader  || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			curAct,
			scriptArgument,
			pScript->CurrentMission + 1,
			nextAct,
			nextArg);

		return;
	}

	auto pLeaderUnitType = pTeamData->TeamLeader->GetTechnoType();
	bool leaderWeaponsHaveAG = false;
	bool leaderWeaponsHaveAA = false;
	ScriptExtData::CheckUnitTargetingCapabilities(pTeamData->TeamLeader, leaderWeaponsHaveAG, leaderWeaponsHaveAA, agentMode);

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pTeamData->TeamLeader->Passengers.NumPassengers > 0)
	{
		for (NextObject obj(pTeamData->TeamLeader->Passengers.FirstPassenger->NextObject); obj; ++obj)
		{
			auto const passenger = abstract_cast<FootClass*>(*obj);
			bool passengerWeaponsHaveAG = false;
			bool passengerWeaponsHaveAA = false;
			ScriptExtData::CheckUnitTargetingCapabilities(passenger, passengerWeaponsHaveAG, passengerWeaponsHaveAA, agentMode);

			leaderWeaponsHaveAG |= passengerWeaponsHaveAG;
			leaderWeaponsHaveAA |= passengerWeaponsHaveAA;
		}
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		HouseClass* enemyHouse = nullptr;
		const auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->Owner);
		const bool onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemyMode != -1 ?
		pHouseExt->m_ForceOnlyTargetHouseEnemy : pTeam->Type->OnlyTargetHouseEnemy;

		if (onlyTargetHouseEnemy && pTeamData->TeamLeader->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->Items[pTeamData->TeamLeader->Owner->EnemyHouseIndex];

		int targetMask = scriptArgument;
		auto selectedTarget = ScriptExtData::GreatestThreat(pTeamData->TeamLeader, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				curAct,
				scriptArgument,
				pTeamData->TeamLeader->get_ID(), pTeamData->TeamLeader->UniqueID,
				selectedTarget->get_ID(),
				selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pFocus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExtData::IsUnitAvailable(pFoot, false) && ScriptExtData::IsUnitAvailable(selectedTarget, true))
				{
					auto const pTechnoType = pFoot->GetTechnoType();

					if (pFoot != selectedTarget && pFoot->Target != selectedTarget)
					{
						if (pTechnoType->Underwater && pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pFoot->SetTarget(nullptr);
							pFoot->SetDestination(nullptr, false);
							pFoot->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pFoot->WhatAmI() == AbstractType::Aircraft
							&& pFoot->Ammo > 0 && pFoot->GetHeight() <= 0)
						{
							pFoot->SetDestination(selectedTarget, false);
							pFoot->QueueMission(Mission::Attack, true);
						}

						pFoot->SetTarget(selectedTarget);

						if (pFoot->IsEngineer())
							pFoot->QueueMission(Mission::Capture, true);
						else if (pFoot->WhatAmI() != AbstractType::Aircraft) // Aircraft hack. I hate how this game auto-manages the aircraft missions.
							pFoot->QueueMission(Mission::Attack, true);

						if (pFoot->WhatAmI() == AbstractType::Infantry)
						{
							auto const pInfantryType = static_cast<const InfantryTypeClass*>(pTechnoType);

							// Spy case
							if (pInfantryType && pInfantryType->Infiltrate && pInfantryType->Agent && pFoot->GetCurrentMission() != Mission::Enter)
								pFoot->QueueMission(Mission::Enter, true); // Check if target is an structure and see if spiable

							// Tanya / Commando C4 case
							if ((pInfantryType->C4 || pFoot->HasAbility(AbilityType::C4))
								&& pFoot->GetCurrentMission() != Mission::Sabotage)
							{
								pFoot->QueueMission(Mission::Sabotage, true);
							}
						}
					}
					else
					{
						pFoot->QueueMission(Mission::Attack, true);
					}
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
				// No target? let's wait some frames
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);

				return;
			}

			// This action finished
			pTeam->StepCompleted = true;
			ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				curAct,
				scriptArgument,
				pScript->CurrentMission + 1,
				nextAct,
				nextArg,
				pTeamData->TeamLeader->get_ID(),
				pTeamData->TeamLeader->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit
		if (!ScriptExtData::IsUnitAvailable(pFocus, true))
		{
			pTeam->Focus = nullptr;
			pFocus = nullptr;
			return;
		}

		bool isAirOK = pFocus->IsInAir() && leaderWeaponsHaveAA;
		bool isGroundOK = !pFocus->IsInAir() && leaderWeaponsHaveAG;

		if ( !pFocus->GetTechnoType()->Immune
			&& (isAirOK || isGroundOK)
			&& (!pTeamData->TeamLeader->Owner->IsAlliedWith(pFocus) || ScriptExtData::IsUnitMindControlledFriendly(pTeamData->TeamLeader->Owner, pFocus)))
		{
			bool bForceNextAction = false;

			for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExtData::IsUnitAvailable(pFoot, true))
				{
					auto const pTechnoType = pFoot->GetTechnoType();

					// Aircraft case 1
					if ((pFoot->WhatAmI() == AbstractType::Aircraft
						&& static_cast<const AircraftTypeClass*>(pTechnoType)->AirportBound)
						&& pFoot->Ammo > 0
						&& (pFoot->Target != pFocus && !pFoot->InAir))
					{
						pFoot->SetTarget(pFocus);

						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have nti-ground weapons. Ignore the attack
					if (pTechnoType->Underwater
						&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
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
							if (pFoot->Target != pFocus)
								pFoot->SetTarget(pFocus);

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
						if (pFoot->Target != pFocus)
							pFoot->SetTarget(pFocus);

						if (pFoot->GetCurrentMission() != Mission::Attack
							&& pFoot->GetCurrentMission() != Mission::Unload
							&& pFoot->GetCurrentMission() != Mission::Selling)
						{
							pFoot->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				ScriptExtData::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n",
					pTeam->Type->ID,
					pScript->Type->ID,
					pScript->CurrentMission,
					curAct,
					scriptArgument,
					pScript->CurrentMission + 1,
					nextAct,
					nextArg);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExtData::GreatestThreat(TechnoClass* pTechno, int method, DistanceMode calcThreatMode, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{

	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;

	if (!pTechno)
		return nullptr;

	auto pTechnoType = pTechno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	auto const AIDifficulty = static_cast<int>(pTechno->Owner->GetAIDifficultyIndex());
	auto const DisguiseDetectionValue = pTypeExt->DetectDisguise_Percent.GetEx(RulesExtData::Instance()->AIDetectDisguise_Percent)->at(AIDifficulty);
	auto const detectionValue = (int)std::round(DisguiseDetectionValue * 100.0);

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->Items[i];
		if (!ScriptExtData::IsUnitAvailable(object, true) || object == pTechno)
			continue;

		if (object->Spawned)
			continue;

		auto objectType = object->GetTechnoType();

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		int weaponIndex = pTechno->SelectWeapon(object);
		auto weaponType = pTechno->GetWeapon(weaponIndex)->WeaponType;

		if (weaponType && weaponType->Projectile->AA)
			unitWeaponsHaveAA = true;

		if ((weaponType && weaponType->Projectile->AG) || agentMode)
			unitWeaponsHaveAG = true;

		if (!agentMode)
		{
			if (weaponType && GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead, TechnoExtData::GetArmor(object)) == 0.0)
				continue;

			if (object->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!object->IsInAir() && !unitWeaponsHaveAG)
				continue;
		}

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		if (pTechnoType->DetectDisguise && object->IsDisguised() && detectionValue > 0) {
			if (ScenarioClass::Instance->Random.PercentChance(detectionValue))
				continue;
		}

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
				|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& (object->GetCell()->LandType != LandType::Water))
		{
			continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;

		// Check map zone
		if (!TechnoExtData::AllowedTargetByZone(pTechno, object, pTypeExt->TargetZoneScanType, weaponType))
			continue;

		if (!objectType->Immune
			&& !object->TemporalTargetingMe
			&& !object->BeingWarpedOut
			&& object->Owner != pTechno->Owner
			&& (!pTechno->Owner->IsAlliedWith(object) || ScriptExtData::IsUnitMindControlledFriendly(pTechno->Owner, object)))
		{
			double value = 0;

			if (ScriptExtData::EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;

				if (calcThreatMode == DistanceMode::idkZero || calcThreatMode == DistanceMode::idkOne)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = object->GetThreatValue();

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (object->Owner->EnemyHouseIndex >= 0 && pTechno->Owner == HouseClass::Array->Items[object->Owner->EnemyHouseIndex])
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

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
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == DistanceMode::Furtherst)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

bool ScriptExtData::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{

	if (!ScriptExtData::IsUnitAvailable(pTechno , false) || !ScriptExtData::IsUnitAvailable(pTeamLeader, false))
		return false;

	//if (pTechno->Spawned)
	//	return false;

	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	bool buildingIsConsideredVehicle = false;

	if (pTargetTypeExt->IsDummy)
		return false;

	bool IsBuilding = false;
	if (const auto pBuilding = specific_cast<BuildingClass*>(pTechno))
	{
		IsBuilding = true;

		if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1)
			return false;

		buildingIsConsideredVehicle = pBuilding->Type->IsUndeployable();
	}

	const auto whatTech = pTechno->WhatAmI();
	UnitTypeClass* pTypeUnit = whatTech == AbstractType::Unit ? static_cast<UnitTypeClass*>(pTechnoType) : nullptr;

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	const auto& nAITargetTypes = RulesExtData::Instance()->AITargetTypesLists;
	if ((size_t)attackAITargetType < nAITargetTypes.size()) {
		const auto nVec = make_iterator(nAITargetTypes[attackAITargetType]);
		return nVec.contains(pTechnoType);
	}

	// mask shoud be replaced with proper enum class
	// it is more readable
	switch (mask)
	{
	case 1:
	{
		// Anything ;-)
		return !pTechno->Owner->IsNeutral();
	}
	case 2:
		// Building
	{
		if (!pTechno->Owner->IsNeutral())
		{
			if (!buildingIsConsideredVehicle)
				return true;

			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				return !(pBld->Type->Artillary
					|| pBld->Type->TickTank
					|| pBld->Type->ICBMLauncher
					|| pBld->Type->SensorArray
					|| pBldExt->IsJuggernaut);
			}
		}

		return false;
	}
	case 3:
	{
		// Harvester
		if (!pTechno->Owner->IsNeutral())
		{
			switch (whatTech)
			{
			case UnitClass::AbsID:
			{
				const auto pType = static_cast<const UnitClass*>(pTechno)->Type;
				return pType->Harvester || pType->Weeder;
			}
			case BuildingClass::AbsID:
			{
				const auto pBldHere = static_cast<const BuildingClass*>(pTechno);
				return pBldHere->SlaveManager && pTechnoType->ResourceGatherer && pBldHere->Type->Enslaves;
			}
			case InfantryClass::AbsID:
			{
				const auto pInfHere = static_cast<const InfantryClass*>(pTechno);
				return pInfHere->Type->Slaved && pInfHere->SlaveOwner && pTechnoType->ResourceGatherer;
			}
			}
		}

		return false;
	}
	case 4:
	{
		// Infantry
		return !pTechno->Owner->IsNeutral() && whatTech == InfantryClass::AbsID;
	}
	case 5:
	{
		// Vehicle, Aircraft, Deployed vehicle into structure
		if (!pTechno->Owner->IsNeutral())
		{
			if (buildingIsConsideredVehicle)
				return true;

			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				const auto pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
				return (pBld->Type->Artillary
				|| pBld->Type->TickTank
				|| pBld->Type->ICBMLauncher
				|| pBld->Type->SensorArray
				|| pExt->IsJuggernaut);
			}

			return (whatTech == AircraftClass::AbsID || whatTech == UnitClass::AbsID);
		}
		return false;
	}
	case 6:
		// Factory
	{
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
				return pBld->Type->Factory != AbstractType::None;
		}
		return false;
	}
	case 7:
	{
		// Defense
		if (!pTechno->Owner->IsNeutral())
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
				return pBld->Type->IsBaseDefense;

		return false;
	}
	case 8:
	{	// House threats
		if (pTeamLeader && !pTechno->Owner->IsNeutral())
		{

			if (auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target))
			{
				// The possible Target is aiming against me? Revenge!
				if (pTarget != pTeamLeader)
					return pTarget->Target == pTeamLeader
					|| pTarget->Owner && HouseClass::Array->Items[pTarget->Owner->EnemyHouseIndex] == pTeamLeader->Owner;
			}

			auto const curtargetiter = make_iterator(pTechno->CurrentTargets);
			if (!curtargetiter.empty())
			{
				return std::any_of(curtargetiter.begin(), curtargetiter.end(),
				[pTeamLeader](AbstractClass* pTarget) {
				const auto pTech = abstract_cast<TechnoClass*>(pTarget);
				 return ScriptExtData::IsUnitAvailable(pTech , true) && pTech->GetOwningHouse() == pTeamLeader->Owner;
				});
			}

			// Then check if this possible target is too near of the Team Leader
			const auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;
			const auto pWeaponPrimary = TechnoExtData::GetCurrentWeapon(pTechno);
			const auto pWeaponSecondary = TechnoExtData::GetCurrentWeapon(pTechno , true);
			const bool primaryCheck = pWeaponPrimary && distanceToTarget <= (pWeaponPrimary->Range / 256.0 * 4.0);
			const bool secondaryCheck = pWeaponSecondary && distanceToTarget <= (pWeaponSecondary->Range / 256.0 * 4.0);
			const bool guardRangeCheck = pTeamLeader->GetTechnoType()->GuardRange > 0 && distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0);

			return primaryCheck
				|| secondaryCheck
				|| guardRangeCheck;
		}

		return false;
	}
	case 9:
	{
		// Power Plant
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				for (const auto type : pBld->GetTypes())
				{
					if (type)
						return type->PowerBonus > 0;
				}
			}
		}

		return false;
	}
	case 10:
	{
		// Occupied Building
		if (auto pBld = specific_cast<BuildingClass*>(pTechno))
		{
			return (pBld->Occupants.Count > 0);
		}

		return false;
	}
	case 11:
	{
		// Civilian Tech
		if (auto pBld = specific_cast<BuildingClass*>(pTechno))
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
	case 12:
	{
		// Refinery
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnit = specific_cast<UnitClass*>(pTechno))
			{
				return !(pUnit->Type->Harvester || pUnit->Type->Weeder)
					&& pUnit->Type->ResourceGatherer
					&& pUnit->Type->DeploysInto
					;
			}

			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
			{
				return pBuilding->Type->ResourceGatherer
					|| (pBuilding->Type->Refinery || (pBuilding->SlaveManager && pBuilding->Type->Enslaves));
			}

		}

		return false;
	}
	case 13:
	{
		if (!pTechno->Owner->IsNeutral())
		{
			auto const& [WeaponType1, WeaponType2] = ScriptExtData::GetWeapon(pTechno);

			bool CanMC = false;
			if (WeaponType1)
			{
				auto pWHExt = WarheadTypeExtContainer::Instance.Find(WeaponType1->Warhead);
				CanMC = pWHExt && pWHExt->PermaMC.Get() || WeaponType1->Warhead->MindControl;
			}

			if (!CanMC && WeaponType2)
			{
				auto pWHExt = WarheadTypeExtContainer::Instance.Find(WeaponType2->Warhead);
				CanMC = pWHExt && pWHExt->PermaMC.Get() || WeaponType2->Warhead->MindControl;
			}

			return CanMC;
		}

		return false;
	}
	case 14:
	{
		// Aircraft and Air Unit
		return (!pTechno->Owner->IsNeutral()
			&& (whatTech == AircraftClass::AbsID
				|| pTechnoType->JumpJet
				|| pTechnoType->BalloonHover
				|| pTechno->IsInAir()));
	}
	case 15:
	{
		// Naval Unit & Structure
		return (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval
				|| (pTechno->GetCell()->LandType == LandType::Water)));
	}
	case 16:
	{
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		if (!pTechno->Owner->IsNeutral())
		{
			const auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
			const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

			return ((pTechnoTypeExt
				&& (pTechnoTypeExt->RadarJamRadius > 0
					|| pTechnoTypeExt->InhibitorRange.isset()))
					|| (pTypeBuilding && (pTypeBuilding->GapGenerator
						|| pTypeBuilding->CloakGenerator)));
		}
		return false;
	}
	case 17:
	{
		// Ground Vehicle
		return !pTechno->Owner->IsNeutral()
			&& ((pTypeUnit || buildingIsConsideredVehicle) && !pTechno->IsInAir() && !pTechnoType->Naval);
	}
	case 18:
	{
		// Economy: Harvester, Refinery or Resource helper
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnitT = specific_cast<UnitTypeClass*>(pTechnoType))
				return pUnitT->Harvester || pUnitT->ResourceGatherer;

			if (auto pInfT = specific_cast<InfantryTypeClass*>(pTechnoType))
				return pInfT->ResourceGatherer && (pInfT->Slaved && pTechno->SlaveOwner);

			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				for (auto const type : pBld->GetTypes())
				{
					if (type && (type->ProduceCashAmount > 0 || type->OrePurifier))
						return true;
				}

				return  pBld->Type->Refinery || pBld->Type->ResourceGatherer
					|| (pTechno->SlaveManager && pBld->Type->Enslaves)
					;
			}
		}

		return false;
	}
	case 19:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);
		// Infantry Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::InfantryType);
	}
	case 20:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::UnitType
			&& !pBuildingType->Naval);
	}
	case 21:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// is Aircraft Factory
		return (!pTechno->Owner->IsNeutral()
			&& (pBuildingType
				&& (pBuildingType->Factory == AbstractType::AircraftType
					|| pBuildingType->Helipad)));
	}
	case 22:
	{
		// Radar & SpySat
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				if(pBld->Type->Radar)
					return true;

				for (auto const type : pBld->GetTypes()) {
					if (type && type->SpySat)
						return true;
				}
			}
		}

		return false;
	}
	case 23:
	{
		// Buildable Tech
		if (!pTechno->Owner->IsNeutral()
			&& IsBuilding)
		{
			return (RulesClass::Instance->BuildTech.Count > 0 && RulesClass::Instance->BuildTech.Contains(static_cast<BuildingTypeClass*>(pTechnoType)));
		}

		return false;
	}
	case 24:
	{
		if (!IsBuilding)
			return false;

		auto pBuildingType = static_cast<BuildingTypeClass*>(pTechnoType);

		// Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::UnitType
			&& pBuildingType->Naval);
	}
	case 25:
	{
		if (!IsBuilding)
			return false;

		// Super Weapon building
		bool IsOK = false;
		if (!pTechno->Owner->IsNeutral())
		{
			const auto pBld = static_cast<BuildingClass*>(pTechno);

			{
				for (auto type : pBld->GetTypes())
				{
					if (!type)
						continue;

					if (auto typeExt = BuildingTypeExtContainer::Instance.Find(const_cast<BuildingTypeClass*>(type)))
					{
						if (typeExt->GetSuperWeaponCount() > 0)
							return true;
					}
				}
			}
		}

		return IsOK;
	}
	case 26:
	{
		// Construction Yard
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (const auto pFake = TechnoTypeExtContainer::Instance.Find(pTypeBuilding)->Fake_Of.Get(nullptr)) {
					return ((BuildingTypeClass*)pFake)->Factory == AbstractType::BuildingType && ((BuildingTypeClass*)pFake)->ConstructionYard;
				}

				return (pTypeBuilding && pTypeBuilding->Factory == AbstractType::BuildingType && pTypeBuilding->ConstructionYard);
			}
		}

		if (whatTech == UnitClass::AbsID)
		{
			return (RulesClass::Instance->BaseUnit.Count > 0 && RulesClass::Instance->BaseUnit.Contains(static_cast<UnitTypeClass*>(pTechnoType)));
		}

		return false;
	}
	case 27:
	{
		// Any Neutral object
		return pTechno->Owner->IsNeutral();
	}
	case 28:
	{
		if (!IsBuilding)
			return false;

		// Cloak Generator & Gap Generator
		if (!pTechno->Owner->IsNeutral())
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTechno);

			{
				for (const auto pBldTypeHere : pBuilding->GetTypes())
				{
					if (pBldTypeHere && (pBuilding->Type->GapGenerator || pBuilding->Type->CloakGenerator))
						return true;
				}
			}
		}

		return false;
	}
	case 29:
	{
		// Radar Jammer
		const auto pTypeTechnoExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral() &&
			(pTypeTechnoExt && (pTypeTechnoExt->RadarJamRadius > 0)));
	}
	case 30:
	{
		// Inhibitor
		const auto pTypeTechnoExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& pTypeTechnoExt->InhibitorRange.isset()));
	}
	case 31:
	{
		// Naval Unit
		return (!pTechno->Owner->IsNeutral()
			&& whatTech == UnitClass::AbsID
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water));
	}
	case 32:
	{
		// Any non-building unit
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pUnit = specific_cast<UnitClass*>(pTechno))
			{
				return !pUnit->Type->DeploysInto;
			}

			if (buildingIsConsideredVehicle)
				return true;

			if (auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType))
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
	case 33:
	{
		if (!IsBuilding)
			return false;

		const auto pBuilding = static_cast<BuildingClass*>(pTechno);
		const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
		// Capturable Structure or Repair Hut
		return pBldExt->EngineerRepairable.Get(pBuilding->Type->Capturable)
			|| (pBuilding->Type->BridgeRepairHut && pBuilding->Type->Repairable)
			;
	}
	case 34:
	{
		if (!pTeamLeader)
			return false;

		if (!pTechno->Owner->IsNeutral())
		{
			// Inside the Area Guard of the Team Leader
			const auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons
			const auto pLEaderType = pTeamLeader->GetTechnoType();

			return (pLEaderType->GuardRange > 0
					&& distanceToTarget <= ((pLEaderType->GuardRange / 256.0) * 2.0));
		}

		return false;
	}
	case 35:
	{
		if (!IsBuilding)
			return false;

		auto pBuilding = static_cast<BuildingClass*>(pTechno);
		// Land Vehicle Factory & Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuilding->Type->Factory == AbstractType::UnitType);
	}
	case 36:
	{
		if (!IsBuilding)
			return false;

		// Building that isn't a defense
		if (!pTechno->Owner->IsNeutral())
		{
			auto pBuilding = static_cast<BuildingClass*>(pTechno);

			if (pBuilding->Type->IsBaseDefense)
				return false;

			if (buildingIsConsideredVehicle)
				return false;

			auto const pBtypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

			return !(
				pBuilding->Type->Artillary
				|| pBuilding->Type->TickTank
				|| pBtypeExt->IsJuggernaut
				|| pBuilding->Type->ICBMLauncher
				|| pBuilding->Type->SensorArray
				);
		}

		return false;
	}
	case 39:
	{
		if (!IsBuilding)
			return false;

		// Occupyable Civilian  Building
		if (auto pBuilding = static_cast<BuildingClass*>(pTechno))
		{
			if (pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count == 0 && pBuilding->Owner->IsNeutral() && pBuilding->Type->CanOccupyFire && pBuilding->Type->TechLevel == -1 && pBuilding->GetHealthStatus() != HealthState::Red)
				return true;
			if (pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count < pBuilding->Type->MaxNumberOccupants && pBuilding->Owner == pTeamLeader->Owner && pBuilding->Type->CanOccupyFire)
				return true;
		}

		return false;
	}
	case 40:
	{
		if (!IsBuilding)
			return false;

		if (!pTechno->Owner->IsNeutral())
		{
			// Self Building with Grinding=yes
			if (auto pBuilding = static_cast<BuildingClass*>(pTechno))
			{
				return pBuilding->Type->Grinding && pBuilding->Owner == pTeamLeader->Owner;
			}
		}

		return false;
	}
	case 41:
	// Building with Spyable=yes
	{
		if (!IsBuilding)
			return false;

		if (!pTechno->Owner->IsNeutral()) {
			if (auto pBuilding = static_cast<BuildingClass*>(pTechno)) {
				return pBuilding->Type->Spyable;
			}
		}

		return false;
	}
	case 37:
	{
		if (!IsBuilding)
			return false;

		if (!pTechno->Owner->IsNeutral()) {
			return TechnoTypeExtContainer::Instance.Find(pTechnoType)->IsHero.Get();
		}

		return false;
	}
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
		return;
	}

	pTeam->StepCompleted = true;
	ScriptExtData::Log("AI Scripts - Mission_Attack_List: [%s] [%s] (line: %d = %d,%d) Failed to get the list index [AITargetTypes][%d]! out of bound: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArg,
		attackAITargetType,
		targetList.size());
}

static std::vector<int> Mission_Attack_List1Random_validIndexes;

void ScriptExtData::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType)
{

	auto pScript = pTeam->CurrentScript;
	Mission_Attack_List1Random_validIndexes.clear();
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	const auto& [curAct, curArgs] = pTeam->CurrentScript->GetCurrentAction();

	if (attackAITargetType < 0)
		attackAITargetType = curArgs;

	if((size_t)attackAITargetType < RulesExtData::Instance()->AITargetTypesLists.size()) {

		if ((size_t)pTeamData->IdxSelectedObjectFromAIList < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size()) {
			ScriptExtData::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, pTeamData->IdxSelectedObjectFromAIList);
			return;
		}

		if (!RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].empty())
		{
			// Finding the objects from the list that actually exists in the map
			TechnoClass::Array->for_each([&](TechnoClass* pTechno) {

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

				ScriptExtData::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pScript->CurrentMission,
					curAct,
					curArgs,
					attackAITargetType,
					idxSelectedObject,
					RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][idxSelectedObject]->ID);

				ScriptExtData::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
				return;
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExtData::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pScript->CurrentMission,
		curAct,
		curArgs,
		attackAITargetType,
		Mission_Attack_List1Random_validIndexes.size()
	);
}

void ScriptExtData::CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode)
{

	if (!pTechno || !pTechno->IsAlive)
		return;

	const auto&[pWeaponPrimary, pWeaponSecondary] = ScriptExtData::GetWeapon(pTechno);

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AA) || (pWeaponSecondary && pWeaponSecondary->Projectile->AA))
		hasAntiAir = true;

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AG) || (pWeaponSecondary && pWeaponSecondary->Projectile->AG) || agentMode)
		hasAntiGround = true;
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
