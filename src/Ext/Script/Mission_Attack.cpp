#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

// Contains ScriptExt::Mission_Attack and its helper functions.

void ScriptExt::Mission_Attack(TeamClass* pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
	if (!pScript){
		pTeam->StepCompleted = true;
		return;
	}
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass* pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);



	if (!pTeamData) {
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);
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
		ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!ScriptExt::IsUnitAvailable(pFocus, true))
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pFoot);

		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				ScriptExt::IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
				pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pFocus = nullptr;
			pTeam->Focus = nullptr;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pFootTeam = pTeam->FirstUnit; pFootTeam; pFootTeam = pFootTeam->NextTeamMember)
				{
					// Let's reset all Team Members objective
					auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pFootTeam);
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
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (ScriptExt::IsUnitAvailable(pFoot, true))
		{
			auto const pTechnoType = pFoot->GetTechnoType();

			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& !pFoot->IsInAir()
				&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound
				&& pFoot->Ammo < pTechnoType->Ammo)
			{
				bAircraftsWithoutAmmo = true;
			}

			pacifistTeam &= !ScriptExt::IsUnitArmed(pFoot);

			if (pFoot->WhatAmI() == AbstractType::Infantry)
			{
				auto const pTypeInf = static_cast<InfantryTypeClass*>(pTechnoType);

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					agentMode = true;
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;

	if (!ScriptExt::IsUnitAvailable(pLeaderUnit, true))
	{
		pLeaderUnit = ScriptExt::FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
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
		ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	bool leaderWeaponsHaveAG = false;
	bool leaderWeaponsHaveAA = false;
	ScriptExt::CheckUnitTargetingCapabilities(pLeaderUnit, leaderWeaponsHaveAG, leaderWeaponsHaveAA, agentMode);

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
	{
		for (NextObject obj(pLeaderUnit->Passengers.FirstPassenger->NextObject); obj; ++obj)
		{
			auto const passenger = abstract_cast<FootClass*>(*obj);
			bool passengerWeaponsHaveAG = false;
			bool passengerWeaponsHaveAA = false;
			ScriptExt::CheckUnitTargetingCapabilities(passenger, passengerWeaponsHaveAG, passengerWeaponsHaveAA, agentMode);

			leaderWeaponsHaveAG |= passengerWeaponsHaveAG;
			leaderWeaponsHaveAA |= passengerWeaponsHaveAA;
		}
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		if (pTeam->Type->OnlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = scriptArgument;
		selectedTarget = ScriptExt::GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (pFoot->IsAlive && !pFoot->InLimbo)
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
							auto const pInfantryType = static_cast<InfantryTypeClass*>(pTechnoType);

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
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit

		bool isAirOK = pFocus->IsInAir() && leaderWeaponsHaveAA;
		bool isGroundOK = !pFocus->IsInAir() && leaderWeaponsHaveAG;

		if (ScriptExt::IsUnitAvailable(pFocus, true)
			&& !pFocus->GetTechnoType()->Immune
			&& (isAirOK || isGroundOK)
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus) || ScriptExt::IsUnitMindControlledFriendly(pLeaderUnit->Owner, pFocus)))
		{
			bool bForceNextAction = false;

			for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
			{
				auto const pTechnoType = pFoot->GetTechnoType();

				if (ScriptExt::IsUnitAvailable(pFoot, true))
				{
					// Aircraft case 1
					if ((pFoot->WhatAmI() == AbstractType::Aircraft
						&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound)
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
						&& static_cast<InfantryTypeClass*>(pTechnoType)->C4
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
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;

	if (!pTechno)
		return nullptr;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();
		auto pTechnoType = pTechno->GetTechnoType();

		if (!object)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		int weaponIndex = pTechno->SelectWeapon(object);
		auto weaponType = pTechno->GetWeapon(weaponIndex)->WeaponType;

		if (weaponType && weaponType->Projectile->AA)
			unitWeaponsHaveAA = true;

		if ((weaponType && weaponType->Projectile->AG) || agentMode)
			unitWeaponsHaveAG = true;

		// Check verses instead of damage to allow support units etc.
		/*
		int weaponDamage = 0;

		if (weaponType)
			weaponDamage = MapClass::GetTotalDamage(pTechno->CombatDamage(weaponIndex), weaponType->Warhead, objectType->Armor, 0);

		// If the target can't be damaged then isn't a valid target
		if (weaponType && weaponDamage <= 0 && !agentMode)
			continue;
		*/

		if (!agentMode)
		{
			if (weaponType && GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead, objectType->Armor) == 0.0)
				continue;

			if (object->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!object->IsInAir() && !unitWeaponsHaveAG)
				continue;
		}

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

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
		if (!TechnoExt::AllowedTargetByZone(pTechno, object, pTypeExt->TargetZoneScanType, weaponType))
			continue;

		if (object != pTechno
			&& ScriptExt::IsUnitAvailable(object, true)
			&& !objectType->Immune
			&& !object->TemporalTargetingMe
			&& !object->BeingWarpedOut
			&& object->Owner != pTechno->Owner
			&& (!pTechno->Owner->IsAlliedWith(object) || ScriptExt::IsUnitMindControlledFriendly(pTechno->Owner, object)))
		{
			double value = 0;

			if (ScriptExt::EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
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
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
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

bool ScriptExt::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{
	if (!pTechno || !pTechno->Owner || !pTeamLeader || !pTeamLeader->Owner)
		return false;

	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
	bool buildingIsConsideredVehicle = false;

	if (!pTargetTypeExt || pTargetTypeExt->IsDummy.Get())
		return false;

	if (const auto pBuilding = specific_cast<BuildingClass*>(pTechno))
	{
		if (BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1)
			return false;

		buildingIsConsideredVehicle = pBuilding->Type->IsUndeployable();
	}

	UnitTypeClass* pTypeUnit = pTechno->WhatAmI() == AbstractType::Unit ? static_cast<UnitTypeClass*>(pTechnoType) : nullptr;

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	auto const& nAITargetTypes = RulesExt::Global()->AITargetTypesLists;
	if (attackAITargetType >= 0 && !nAITargetTypes.empty() && attackAITargetType < (int)nAITargetTypes.size()) {
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
				const auto pBldExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

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
			switch (GetVtableAddr(pTechno))
			{
			case UnitClass::vtable:
			{
				const auto pType = static_cast<UnitClass*>(pTechno)->Type;
				return pType->Harvester || pType->Weeder;
			}
			case BuildingClass::vtable:
			{
				const auto pBldHere = static_cast<BuildingClass*>(pTechno);
				return pBldHere->SlaveManager && pTechnoType->ResourceGatherer && pBldHere->Type->Enslaves;
			}
			case InfantryClass::vtable:
			{
				const auto pInfHere = static_cast<InfantryClass*>(pTechno);
				return pInfHere->Type->Slaved && pInfHere->SlaveOwner && pTechnoType->ResourceGatherer;
			}
			}
		}

		return false;
	}
	case 4:
	{
		// Infantry
		return !pTechno->Owner->IsNeutral() && Is_Infantry(pTechno);
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
				const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
				return (pBld->Type->Artillary
				|| pBld->Type->TickTank
				|| pBld->Type->ICBMLauncher
				|| pBld->Type->SensorArray
				|| pExt->IsJuggernaut);
			}

			return (Is_Aircraft(pTechno) || Is_Unit(pTechno));
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
					|| pTarget->Owner && HouseClass::Array->GetItem(pTarget->Owner->EnemyHouseIndex) == pTeamLeader->Owner;
			}

			auto const& curtargetiter = make_iterator(pTechno->CurrentTargets);
			if (!curtargetiter.empty())
			{
				return std::any_of(curtargetiter.begin(), curtargetiter.end(),
				[pTeamLeader](AbstractClass* pTarget)
 {
	 auto const pTech = abstract_cast<TechnoClass*>(pTarget);
	 return pTech && pTech->GetOwningHouse() && pTech->GetOwningHouse() == pTeamLeader->Owner;
				});
			}

			// Then check if this possible target is too near of the Team Leader
			const auto distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;
			const auto nRange1 = pTechno->GetWeaponRange(pTechnoType->IsGattling ? pTechno->CurrentWeaponNumber : 0);
			const auto nRange2 = !pTechnoType->IsGattling ? pTechno->GetWeaponRange(1) : 0;

			return (nRange1 > 0 && distanceToTarget <= (nRange1 / 256.0 * 4.0))
				|| (nRange2 > 0 && distanceToTarget <= (nRange2 / 256.0 * 4.0))
				|| (pTeamLeader->GetTechnoType()->GuardRange > 0
					&& distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0));
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
			auto const& [WeaponType1, WeaponType2] = ScriptExt::GetWeapon(pTechno);

			bool CanMC = false;
			if (WeaponType1)
			{
				auto pWHExt = WarheadTypeExt::ExtMap.Find(WeaponType1->Warhead);
				CanMC = pWHExt && pWHExt->PermaMC.Get() || WeaponType1->Warhead->MindControl;
			}

			if (!CanMC && WeaponType2)
			{
				auto pWHExt = WarheadTypeExt::ExtMap.Find(WeaponType2->Warhead);
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
			&& (Is_AircraftType(pTechnoType)
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
			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

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
				for (auto const type : pBld->GetTypes())
				{
					if (type && (type->Radar || type->SpySat))
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
			&& Is_BuildingType(pTechnoType))
		{
			return (RulesClass::Instance->BuildTech.Count > 0 && RulesClass::Instance->BuildTech.Contains(static_cast<BuildingTypeClass*>(pTechnoType)));
		}

		return false;
	}
	case 24:
	{
		auto pBuildingType = specific_cast<BuildingTypeClass*>(pTechnoType);

		// Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pBuildingType
			&& pBuildingType->Factory == AbstractType::UnitType
			&& pBuildingType->Naval);
	}
	case 25:
	{
		// Super Weapon building
		bool IsOK = false;
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBld = specific_cast<BuildingClass*>(pTechno))
			{
				for (auto type : pBld->GetTypes())
				{
					if (auto typeExt = BuildingTypeExt::ExtMap.TryFind(const_cast<BuildingTypeClass*>(type)))
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
				if (const auto pFake = TechnoTypeExt::ExtMap.Find(pTypeBuilding)->Fake_Of.Get(nullptr))
				{
					if (pFake->WhatAmI() == BuildingTypeClass::AbsID)
					{
						return (pFake && ((BuildingTypeClass*)pFake)->Factory == AbstractType::BuildingType && ((BuildingTypeClass*)pFake)->ConstructionYard);
					}
				}

				return (pTypeBuilding && pTypeBuilding->Factory == AbstractType::BuildingType && pTypeBuilding->ConstructionYard);
			}
		}

		if (Is_UnitType(pTechnoType))
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
		// Cloak Generator & Gap Generator
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
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
		auto pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral() &&
			(pTypeTechnoExt && (pTypeTechnoExt->RadarJamRadius > 0)));
	}
	case 30:
	{
		// Inhibitor
		auto pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& pTypeTechnoExt->InhibitorRange.isset()));
	}
	case 31:
	{
		// Naval Unit
		return (!pTechno->Owner->IsNeutral()
			&& Is_Unit(pTechno)
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
				const auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pTypeBuilding);
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
		const auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
		const auto pBldExt = BuildingTypeExt::ExtMap.Find(pTypeBuilding);
		// Capturable Structure or Repair Hut
		return (pTypeBuilding
			&& (pBldExt->EngineerRepairable.Get(pTypeBuilding->Capturable)
				|| (pTypeBuilding->BridgeRepairHut
					&& pTypeBuilding->Repairable)));
	}
	case 34:
	{
		if (!pTechno->Owner->IsNeutral() && pTeamLeader)
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
		auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
		// Land Vehicle Factory & Naval Factory
		return (!pTechno->Owner->IsNeutral()
			&& pTypeBuilding
			&& pTypeBuilding->Factory == AbstractType::UnitType);
	}
	case 36:
	{
		// Building that isn't a defense
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pTypeBuilding->IsBaseDefense)
					return false;

				if (buildingIsConsideredVehicle)
					return false;

				auto const pBtypeExt = BuildingTypeExt::ExtMap.Find(pTypeBuilding);

				return !(pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank || pBtypeExt->IsJuggernaut || pTypeBuilding->ICBMLauncher || pTypeBuilding->SensorArray);

			}
		}

		return false;
	}
	case 39:
		// Occupyable Civilian  Building
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			if (pBuilding && pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count == 0 && pBuilding->Owner->IsNeutral() && pBuilding->Type->CanOccupyFire && pBuilding->Type->TechLevel == -1 && pBuilding->GetHealthStatus() != HealthState::Red)
				return true;
			if (pBuilding && pBuilding->Type->CanBeOccupied && pBuilding->Occupants.Count < pBuilding->Type->MaxNumberOccupants && pBuilding->Owner == pTeamLeader->Owner && pBuilding->Type->CanOccupyFire)
				return true;
		}
		return false;
	case 40:

		if (!pTechno->Owner->IsNeutral())
		{
			// Self Building with Grinding=yes
			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
			{
				return pBuilding->Type->Grinding && pBuilding->Owner == pTeamLeader->Owner;
			}
		}

		return false;

	case 41:
		// Building with Spyable=yes
		if (!pTechno->Owner->IsNeutral())
		{
			if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
			{
				return pBuilding->Type->Spyable;
			}
		}

		return false;


		/*
		case 100:
			pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);

			// Useable Repair Hut
			if (pTypeBuilding)
			{
				auto cell = pTechno->InlineMapCoords();
				if (pTypeBuilding->BridgeRepairHut && pTypeBuilding->Repairable)
					if (MapClass::Instance->IsBridgeRepairable(&cell))
						return true;
			}

			break;*/
	case 37:
	{
		if (!pTechno->Owner->IsNeutral() && Is_Infantry(pTechno))
		{
			return TechnoTypeExt::ExtMap.Find(pTechnoType)->IsHero.Get();
		}

		return false;
	}
	}

	return false;
}

void ScriptExt::Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[attackAITargetType].size() > 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pScript = pTeam->CurrentScript;
	std::vector<int> validIndexes;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, pTeamData->IdxSelectedObjectFromAIList);
		return;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& (size_t)attackAITargetType < RulesExt::Global()->AITargetTypesLists.size())
	{
		const auto& objectsList = RulesExt::Global()->AITargetTypesLists[attackAITargetType];

		if (!objectsList.empty())
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array->Count; i++)
			{
				auto pTechno = TechnoClass::Array->GetItem(i);
				auto pTechnoType = pTechno->GetTechnoType();
				bool found = false;

				for (auto j = 0u; j < objectsList.size() && !found; j++)
				{
					auto const pFirstUnit = pTeam->FirstUnit;

					if (pTechnoType == objectsList[j]
						&& ScriptExt::IsUnitAvailable(pTechno, true)
						&& (!pFirstUnit->Owner->IsAlliedWith(pTechno) || ScriptExt::IsUnitMindControlledFriendly(pFirstUnit->Owner, pTechno)))
					{
						validIndexes.push_back(j);
						found = true;
					}
				}
			}

			if (validIndexes.size() > 0)
			{
				const int idxSelectedObject = validIndexes[ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.size() - 1)];
				pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;
				ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
				ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
					pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList[idxSelectedObject]->ID);
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.size());
}

void ScriptExt::CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode)
{
	auto const& [pWeaponPrimary, pWeaponSecondary] = ScriptExt::GetWeapon(pTechno);

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AA) || (pWeaponSecondary && pWeaponSecondary->Projectile->AA))
		hasAntiAir = true;

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AG) || (pWeaponSecondary && pWeaponSecondary->Projectile->AG) || agentMode)
		hasAntiGround = true;
}

bool ScriptExt::IsUnitArmed(TechnoClass* pTechno)
{
	auto const& [pWeaponPrimary, pWeaponSecondary] = ScriptExt::GetWeapon(pTechno);
	return pWeaponPrimary || pWeaponSecondary;
}

bool ScriptExt::IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno)
{
	return pHouse->IsAlliedWith(pTechno) && pTechno->IsMindControlled() && !pHouse->IsAlliedWith(pTechno->MindControlledBy);
}
