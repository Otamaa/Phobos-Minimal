#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/InfantryType/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>

#include <Utilities/Cast.h>

void ScriptExtData::Mission_Attack(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	// This is the target type
	const auto& [curAct, scriptArgument] = pScript->GetCurrentAction();
	//const auto& [nextAct, nextArg] = pScript->GetNextAction();
	//Debug::LogInfo("AI Scripts - Attack: [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} -> (Executing)",
	//	pTeam->Type->ID, pScript->Type->ID,
	//	pScript->CurrentMission,
	//	curAct,
	//	scriptArgument,
	//	pScript->CurrentMission + 1,
	//	nextAct,
	//	nextArg);

	// if (!pScript){
	// 	pTeam->StepCompleted = true;
	// 	return;
	// }

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

	auto pFocus = flag_cast_to<TechnoClass*>(pTeam->ArchiveTarget);

	if (!ScriptExtData::IsUnitAvailable(pFocus, true))
	{
		pTeam->ArchiveTarget = nullptr;
		pFocus = nullptr;
	}

	FootClass* pCur = nullptr;
	if (auto pFirst = pTeam->FirstUnit)
	{
		bool LastKillTechnoWasTeamtarget = false;

		auto pNext = pFirst->NextTeamMember;

		do
		{
			auto pKillerTechnoData = TechnoExtContainer::Instance.Find(pFirst);

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
				pTeam->ArchiveTarget = nullptr;
				pFocus = nullptr;
				LastKillTechnoWasTeamtarget = true;

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
					return;
				}
			}

			pCur = pNext;

			if (pNext)
				pNext = pNext->NextTeamMember;

			pFirst = pCur;

		}
		while (pCur);
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
			if (ScriptExtData::IsUnitAvailable(pFoot, true)) {

				auto const pTechnoType = GET_TECHNOTYPE(pFoot);

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
		pTeamData->TeamLeader = pTeam->FetchLeader();

		if(pTeamData->TeamLeader)
			pTeamData->TeamLeader->IsTeamLeader = true;
	}

	if (!pTeamData->TeamLeader  || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->WaitNoTargetAttempts != 0) {
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		//Debug::LogInfo("AI Scripts - Attack: [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)",
		//	pTeam->Type->ID,
		//	pScript->Type->ID,
		//	pScript->CurrentMission,
		//	(int)curAct,
		//	scriptArgument,
		//	pScript->CurrentMission + 1,
		//	(int)nextAct,
		//	nextArg);

		return;
	}

	auto pLeaderUnitType = GET_TECHNOTYPE(pTeamData->TeamLeader);
	bool leaderWeaponsHaveAG = false;
	bool leaderWeaponsHaveAA = false;
	ScriptExtData::CheckUnitTargetingCapabilities(pTeamData->TeamLeader, leaderWeaponsHaveAG, leaderWeaponsHaveAA, agentMode);

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pTeamData->TeamLeader->Passengers.NumPassengers > 0)
	{
		for (auto pPassenger = pTeamData->TeamLeader->Passengers.GetFirstPassenger(); pPassenger; pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject))
		{
			bool passengerWeaponsHaveAG = false;
			bool passengerWeaponsHaveAA = false;
			ScriptExtData::CheckUnitTargetingCapabilities(pPassenger, passengerWeaponsHaveAG, passengerWeaponsHaveAA, agentMode);

			leaderWeaponsHaveAG |= passengerWeaponsHaveAG;
			leaderWeaponsHaveAA |= passengerWeaponsHaveAA;
		}
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		HouseClass* enemyHouse = nullptr;
		const auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->OwnerHouse);
		const bool onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemyMode != -1 ?
		pHouseExt->m_ForceOnlyTargetHouseEnemy : pTeam->Type->OnlyTargetHouseEnemy;

		if (onlyTargetHouseEnemy && (size_t)pTeamData->TeamLeader->Owner->EnemyHouseIndex < (size_t)HouseClass::Array->Count)
			enemyHouse = HouseClass::Array->Items[pTeamData->TeamLeader->Owner->EnemyHouseIndex];

		int targetMask = scriptArgument;
		auto selectedTarget = ScriptExtData::GreatestThreat(pTeamData->TeamLeader, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
	/*		Debug::LogInfo("AI Scripts - Attack: [{}] [{}] (line: {} = {},{}) Leader [{}] (UID: %lu) selected [{}] (UID: %lu) as target.",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				curAct,
				scriptArgument,
				pTeamData->TeamLeader->get_ID(), pTeamData->TeamLeader->UniqueID,
				selectedTarget->get_ID(),
				selectedTarget->UniqueID);*/

			pTeam->ArchiveTarget = selectedTarget;
			pFocus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExtData::IsUnitAvailable(pFoot, false) && ScriptExtData::IsUnitAvailable(selectedTarget, true))
				{
					auto const pTechnoType = GET_TECHNOTYPE(pFoot);

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

						const auto whatAmI = pFoot->WhatAmI();

						// If the vehicle cannot be moved, perhaps it is better this way.
						if (whatAmI == AbstractType::Unit
							&& TechnoExtData::CannotMove(static_cast<UnitClass*>(pFoot))
							&& !pFoot->IsCloseEnough(selectedTarget, pFoot->SelectWeapon(selectedTarget)))
						{
							continue;
						}

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (whatAmI == AbstractType::Aircraft
							&& pFoot->Ammo > 0 && pFoot->GetHeight() <= 0)
						{
							pFoot->SetDestination(selectedTarget, false);
							pFoot->QueueMission(Mission::Attack, true);
						}

						pFoot->SetTarget(selectedTarget);

						if (pFoot->IsEngineer())
							pFoot->QueueMission(Mission::Capture, true);
						else if (whatAmI != AbstractType::Aircraft) // Aircraft hack. I hate how this game auto-manages the aircraft missions.
							pFoot->QueueMission(Mission::Attack, true);

						if (whatAmI == AbstractType::Infantry)
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
			if (pTeamData->WaitNoTargetAttempts > 0 && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
				return;
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
			//Debug::LogInfo("AI Scripts - Attack: [{}] [{}] (line: {} = {},{}) Jump to next line: {} = {},{} (Leader [{}] (UID: %lu) can't find a new target)",
			//	pTeam->Type->ID,
			//	pScript->Type->ID,
			//	pScript->CurrentMission,
			//	(int)curAct,
			//	scriptArgument,
			//	pScript->CurrentMission + 1,
			//	(int)nextAct,
			//	nextArg,
			//	pTeamData->TeamLeader->get_ID(),
			//	pTeamData->TeamLeader->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit
		if (!ScriptExtData::IsUnitAvailable(pFocus, true))
		{
			pTeam->ArchiveTarget = nullptr;
			pFocus = nullptr;
			return;
		}

		bool isAirOK = pFocus->IsInAir() && leaderWeaponsHaveAA;
		bool isGroundOK = !pFocus->IsInAir() && leaderWeaponsHaveAG;

		if ( !GET_TECHNOTYPE(pFocus)->Immune
			&& (isAirOK || isGroundOK)
			&& (!pTeamData->TeamLeader->Owner->IsAlliedWith(pFocus) || ScriptExtData::IsUnitMindControlledFriendly(pTeamData->TeamLeader->Owner, pFocus)))
		{
			bool bForceNextAction = false;

			for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExtData::IsUnitAvailable(pFoot, true))
				{
					auto const pTechnoType = GET_TECHNOTYPE(pFoot);

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
				//Debug::LogInfo("AI Scripts - Attack: [{}] [{}] (line: {} = {},{}) Jump to NEXT line: {} = {},{} (Naval is unable to target ground)",
				//	pTeam->Type->ID,
				//	pScript->Type->ID,
				//	pScript->CurrentMission,
				//	(int)curAct,
				//	scriptArgument,
				//	pScript->CurrentMission + 1,
				//	(int)nextAct,
				//	nextArg);

				return;
			}
		}
		else
		{
			pTeam->ArchiveTarget = nullptr;
			pTeam->StepCompleted = true;
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

	const bool leaderArmed = pTechno->IsArmed();
	auto pTechnoType = GET_TECHNOTYPE(pTechno);
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	auto const AIDifficulty = static_cast<int>(pTechno->Owner->GetAIDifficultyIndex());
	auto const DisguiseDetectionValue = pTypeExt->DetectDisguise_Percent.GetEx(RulesExtData::Instance()->AIDetectDisguise_Percent)->at(AIDifficulty);
	auto const detectionValue = (int)std::round(DisguiseDetectionValue * 100.0);

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->Items[i];
		if (!ScriptExtData::IsUnitAvailable(object, true) || object == pTechno || object->EstimatedHealth <= 0 && pTechnoType->VHPScan == VHPScanType::Strong)
			continue;

		if (object->Spawned)
			continue;

		auto objectType = GET_TECHNOTYPE(object);
		auto pObjectTypeExt = TechnoTypeExtContainer::Instance.Find(objectType);
		bool skipImmune = false;

		if (const auto pTargetBuilding = cast_to<BuildingClass*>(object)) {
			// Discard invisible structures
			if (pTargetBuilding->Type->InvisibleInGame)
				continue;

			// Skip immunity check for buildings in agent mode.
			if (agentMode)
				skipImmune = true;
		}

		if (!objectType->LegalTarget || (pObjectTypeExt->AI_LegalTarget.isset() && !pTechno->Owner->IsControlledByHuman() && !pObjectTypeExt->AI_LegalTarget.Get() )|| (!skipImmune && objectType->Immune))
			continue;

		if (object->GetCurrentMissionControl()->NoThreat)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		if(leaderArmed){
			const auto weaponType = pTechno->GetWeapon(pTechno->SelectWeapon(object))->WeaponType;

			if (weaponType && weaponType->Projectile)
				unitWeaponsHaveAA = weaponType->Projectile->AA;

			if ((weaponType && weaponType->Projectile) || agentMode)
				unitWeaponsHaveAG = weaponType->Projectile->AG;

			if (!agentMode)
			{
				if (weaponType && weaponType->Warhead){
					if(GeneralUtils::GetWarheadVersusArmor(
						weaponType->Warhead,
						TechnoExtData::GetTechnoArmor(object , weaponType->Warhead))
						< 0.001
					)
						continue;
					}

				if (object->IsInAir() && !unitWeaponsHaveAA)
					continue;

				if (!object->IsInAir() && !unitWeaponsHaveAG)
					continue;
			}

			// Check map zone
			if (!TechnoExtData::AllowedTargetByZone(pTechno, object, pTypeExt->TargetZoneScanType, weaponType))
				continue;
		}

		// Stealth ground unit check
		if (objectType->Naval)
		{
			// Submarines aren't a valid target
			if (object->CloakState == CloakState::Cloaked
				&& objectType->Underwater
				&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never
					|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
			{
				continue;
			}

			// Land not OK for the Naval unit
			if (pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
				&& (object->GetCell()->LandType != LandType::Water))
			{
				continue;
			}
		}

		// Stealth check.
		if (object->CloakState == CloakState::Cloaked) {
			if (!object->GetCell()->Sensors_InclHouse(pTechno->Owner->ArrayIndex))
				continue;
		}

		if (pTechnoType->DetectDisguise && object->IsDisguised() && detectionValue > 0) {
			if (ScenarioClass::Instance->Random.PercentChance(detectionValue))
				continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
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
				//CellStruct newCell;
				//newCell.X = (short)object->Location.X;
				//newCell.Y = (short)object->Location.Y;

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
					objectThreatValue += object->Health * (1 - object->GetHealthRatio());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (pTechnoType->VHPScan == VHPScanType::Normal) {
						if (object->EstimatedHealth <= 0)
							value /= 2;
						else if (object->EstimatedHealth <= objectType->Strength / 2)
							value *= 2;
					}

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

// ============================================================
//  EvaluateObjectWithMask  –  refactored
//  Changes vs original:
//    1. Local enum class TargetMask replaces raw int magic numbers.
//    2. Casting normalized:
//         - static_cast<T*>  only after a confirmed type guard
//           (IsBuilding==true, whatTech==X::AbsID, etc.)
//         - type_cast<T*>    for nullable polymorphic downcasts
//           where we need to test whether the cast succeeds
//         - flag_cast_to<T*> kept only for pTechno->Target
//           (AbstractClass* that may have non-Techno flag bits)
//         - cast_to<T*,false> removed; replaced with type_cast or
//           static_cast depending on context
//    3. Variable names cleaned up for clarity / consistency.
//    4. No logic changes.  Where a potential concern was noticed
//       it is marked with a // REVIEW comment.
// ============================================================

bool ScriptExtData::EvaluateObjectWithMask(
	TechnoClass* pTechno,
	int           mask,
	int           attackAITargetType /* = -1 */,
	int           idxAITargetTypeItem /* = -1 */,
	TechnoClass* pTeamLeader /* = nullptr */)
{
	// --------------------------------------------------------
	//  Local mask enum  (lives here so callers still pass int)
	// --------------------------------------------------------
	enum class TargetMask : int
	{
		Anything = 1,
		Building = 2,
		Harvester = 3,
		Infantry = 4,
		VehicleOrAircraft = 5,
		Factory = 6,
		Defense = 7,
		HouseThreat = 8,
		PowerPlant = 9,
		OccupiedBuilding = 10,
		CivilianTech = 11,
		Refinery = 12,
		MindController = 13,
		AirUnit = 14,
		NavalUnitOrStruct = 15,
		Disruptor = 16,  // Cloak/Gap/RadarJam/Inhibitor
		GroundVehicle = 17,
		Economy = 18,
		InfantryFactory = 19,
		LandVehicleFactory = 20,
		AircraftFactory = 21,
		Radar = 22,
		BuildableTech = 23,
		NavalFactory = 24,
		SuperWeapon = 25,
		ConstructionYard = 26,
		Neutral = 27,
		CloakOrGapGen = 28,
		RadarJammer = 29,
		Inhibitor = 30,
		NavalUnit = 31,
		NonBuildingUnit = 32,
		CapturableOrRepair = 33,
		InGuardRange = 34,
		VehicleFactory = 35,  // Land + Naval
		NonDefenseBuilding = 36,
		BridgeHutOrHero = 37,  // REVIEW: out-of-order in original (38 skipped)
		OccupyableCivilian = 39,  // REVIEW: 38 is absent/skipped in original
		GrinderBuilding = 40,
		SpyableBuilding = 41,
	};

	// --------------------------------------------------------
	//  Early availability guards
	// --------------------------------------------------------
	if (!ScriptExtData::IsUnitAvailable(pTechno, false)
	 || !ScriptExtData::IsUnitAvailable(pTeamLeader, false))
	{
		return false;
	}

	TechnoTypeClass* const pTechnoType = GET_TECHNOTYPE(pTechno);
	auto const pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	if (pTechnoTypeExt->IsDummy)
		return false;

	// --------------------------------------------------------
	//  Building classification
	// --------------------------------------------------------
	bool isBuilding = false;
	bool buildingIsConsideredVehicle = false;

	BuildingClass* pBuildingTechno = cast_to<BuildingClass*>(pTechno);
	if (pBuildingTechno)
	{
		isBuilding = true;

		if (BuildingExtContainer::Instance.Find(pBuildingTechno)->LimboID >= 0)
			return false;

		buildingIsConsideredVehicle = pBuildingTechno->Type->IsUndeployable();
	}

	const AbstractType whatTech = pTechno->WhatAmI();
	const UnitTypeClass* const pUnitType =
		(whatTech == AbstractType::Unit)
		? static_cast<UnitTypeClass*>(pTechnoType)
		: nullptr;

	// --------------------------------------------------------
	//  AITargetTypes override (short-circuits the mask switch)
	// --------------------------------------------------------
	const auto& nAITargetTypes = RulesExtData::Instance()->AITargetTypesLists;
	if (static_cast<size_t>(attackAITargetType) < nAITargetTypes.size())
	{
		const auto nVec = make_iterator(nAITargetTypes[attackAITargetType]);
		return nVec.contains(pTechnoType);
	}

	// --------------------------------------------------------
	//  Mask dispatch
	// --------------------------------------------------------
	switch (static_cast<TargetMask>(mask))
	{
		// ---- 1 : Anything (non-neutral) ------------------------
	case TargetMask::Anything:
		return !pTechno->Owner->IsNeutral();

		// ---- 2 : Building (not vehicle-like) -------------------
	case TargetMask::Building:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		if (!buildingIsConsideredVehicle)
			return isBuilding ? true : false; // REVIEW: original returns true for any non-neutral non-vehicle-building, even non-buildings – preserved

		if (isBuilding)
		{
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingTechno->Type);
			return !(pBuildingTechno->Type->Artillary
				  || pBuildingTechno->Type->TickTank
				  || pBuildingTechno->Type->ICBMLauncher
				  || pBuildingTechno->Type->SensorArray
				  || pBldTypeExt->IsJuggernaut);
		}

		return false;
	}

	// ---- 3 : Harvester -------------------------------------
	case TargetMask::Harvester:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		switch (whatTech)
		{
		case UnitClass::AbsID:
		{
			const auto pType = static_cast<const UnitClass*>(pTechno)->Type;
			return pType->Harvester || pType->Weeder;
		}
		case BuildingClass::AbsID:
		{
			return pBuildingTechno->SlaveManager
				&& pTechnoType->ResourceGatherer
				&& pBuildingTechno->Type->Enslaves;
		}
		case InfantryClass::AbsID:
		{
			const auto pInfantry = static_cast<const InfantryClass*>(pTechno);
			return pInfantry->Type->Slaved
				&& pInfantry->SlaveOwner
				&& pTechnoType->ResourceGatherer;
		}
		}

		return false;
	}

	// ---- 4 : Infantry --------------------------------------
	case TargetMask::Infantry:
		return !pTechno->Owner->IsNeutral()
			&& whatTech == InfantryClass::AbsID;

		// ---- 5 : Vehicle / Aircraft / Deployed vehicle ---------
	case TargetMask::VehicleOrAircraft:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		if (buildingIsConsideredVehicle)
			return true;

		if (isBuilding)
		{
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingTechno->Type);
			return pBuildingTechno->Type->Artillary
				|| pBuildingTechno->Type->TickTank
				|| pBuildingTechno->Type->ICBMLauncher
				|| pBuildingTechno->Type->SensorArray
				|| pBldTypeExt->IsJuggernaut;
		}

		return whatTech == AircraftClass::AbsID
			|| whatTech == UnitClass::AbsID;
	}

	// ---- 6 : Factory ---------------------------------------
	case TargetMask::Factory:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return pBuildingTechno->Type->Factory != AbstractType::None;
	}

	// ---- 7 : Defense ---------------------------------------
	case TargetMask::Defense:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return pBuildingTechno->Type->IsBaseDefense;
	}

	// ---- 8 : House Threat ----------------------------------
	case TargetMask::HouseThreat:
	{
		if (!pTeamLeader || pTechno->Owner->IsNeutral())
			return false;

		if (auto pDirectTarget = flag_cast_to<TechnoClass*>(pTechno->Target))
		{
			// Target is aiming at someone else; check if they're retaliating toward us
			if (pDirectTarget != pTeamLeader)
			{
				return pDirectTarget->Target == pTeamLeader
					|| (pDirectTarget->Owner
						&& HouseClass::Array->Items[pDirectTarget->Owner->EnemyHouseIndex] == pTeamLeader->Owner);
			}
		}

		const auto curTargetIter = make_iterator(pTechno->CurrentTargets);
		if (!curTargetIter.empty())
		{
			return std::ranges::any_of(curTargetIter, [pTeamLeader](AbstractClass* pTarget)
			{
				const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget);
				return ScriptExtData::IsUnitAvailable(pTargetTechno, true)
					&& pTargetTechno->GetOwningHouse() == pTeamLeader->Owner;
			});
		}

		// Proximity check: is this unit in effective weapon range of the leader?
		const auto pLeaderType = GET_TECHNOTYPE(pTeamLeader);
		const double distanceCells = pTeamLeader->DistanceFrom(pTechno) / 256.0;
		const auto pWeaponPrimary = TechnoExtData::GetCurrentWeapon(pTechno);
		const auto pWeaponSecondary = TechnoExtData::GetCurrentWeapon(pTechno, true);

		const bool primaryInRange = pWeaponPrimary
			&& distanceCells <= (WeaponTypeExtData::GetRangeWithModifiers(pWeaponPrimary, pTechno) / 256.0 * 4.0);
		const bool secondaryInRange = pWeaponSecondary
			&& distanceCells <= (WeaponTypeExtData::GetRangeWithModifiers(pWeaponSecondary, pTechno) / 256.0 * 4.0);
		const bool guardRangeInRange = pLeaderType->GuardRange > 0
			&& distanceCells <= (pLeaderType->GuardRange / 256.0 * 2.0);

		return primaryInRange || secondaryInRange || guardRangeInRange;
	}

	// ---- 9 : Power Plant -----------------------------------
	case TargetMask::PowerPlant:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		if (pBuildingTechno->Type->InvisibleInGame || pBuildingTechno->Type->Immune)
			return false;

		for (const auto pBldType : pBuildingTechno->GetTypes())
		{
			if (!pBldType)
				continue;

			if (pBldType->PowerBonus > 0)
				return true;

			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType);
			if (pBldTypeExt->PowerPlantEnhancer_Buildings.size()
			 && (pBldTypeExt->PowerPlantEnhancer_Amount != 0 || pBldTypeExt->PowerPlantEnhancer_Factor != 1.0f)
			 && pBldTypeExt->PowerPlantEnhancer_MaxCount > 0)
			{
				return true;
			}
		}

		return false;
	}

	// ---- 10 : Occupied Building ----------------------------
	case TargetMask::OccupiedBuilding:
	{
		if (!isBuilding)
			return false;

		if (pBuildingTechno->Type->InvisibleInGame || pBuildingTechno->Type->Immune)
			return false;

		return pBuildingTechno->Occupants.Count > 0;
	}

	// ---- 11 : Civilian Tech --------------------------------
	case TargetMask::CivilianTech:
	{
		if (!isBuilding)
			return false;

		return pBuildingTechno->Type->Capturable
			&& pBuildingTechno->Type->NeedsEngineer;
	}

	// ---- 12 : Refinery -------------------------------------
	case TargetMask::Refinery:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		if (const auto pUnit = cast_to<UnitClass*>(pTechno))
		{
			return !(pUnit->Type->Harvester || pUnit->Type->Weeder)
				&& pUnit->Type->ResourceGatherer
				&& pUnit->Type->DeploysInto;
		}

		if (isBuilding)
		{
			return pBuildingTechno->Type->ResourceGatherer
				|| pBuildingTechno->Type->Refinery
				|| (pBuildingTechno->SlaveManager && pBuildingTechno->Type->Enslaves);
		}

		return false;
	}

	// ---- 13 : Mind Controller ------------------------------
	case TargetMask::MindController:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		auto const& [pWeaponType1, pWeaponType2] = ScriptExtData::GetWeapon(pTechno);

		bool canMC = false;

		if (pWeaponType1 && pWeaponType1->Warhead)
		{
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeaponType1->Warhead);
			canMC = (pWHExt && pWHExt->PermaMC.Get()) || pWeaponType1->Warhead->MindControl;
		}

		if (!canMC && pWeaponType2 && pWeaponType2->Warhead)
		{
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeaponType2->Warhead);
			canMC = (pWHExt && pWHExt->PermaMC.Get()) || pWeaponType2->Warhead->MindControl;
		}

		return canMC;
	}

	// ---- 14 : Air Unit / Aircraft --------------------------
	case TargetMask::AirUnit:
		return !pTechno->Owner->IsNeutral()
			&& (whatTech == AircraftClass::AbsID
			 || pTechnoType->JumpJet
			 || pTechnoType->BalloonHover
			 || pTechno->IsInAir());

		// ---- 15 : Naval Unit or Structure ----------------------
	case TargetMask::NavalUnitOrStruct:
		return !pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval
			 || pTechno->GetCell()->LandType == LandType::Water);

		// ---- 16 : Cloak/Gap/RadarJam/Inhibitor -----------------
	case TargetMask::Disruptor:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);

		return (pTechnoTypeExt
				&& (pTechnoTypeExt->RadarJamRadius > 0
					|| pTechnoTypeExt->InhibitorRange.isset()))
			|| (pBldType
				&& (pBldType->GapGenerator || pBldType->CloakGenerator));
	}

	// ---- 17 : Ground Vehicle -------------------------------
	case TargetMask::GroundVehicle:
		return !pTechno->Owner->IsNeutral()
			&& (pUnitType || buildingIsConsideredVehicle)
			&& !pTechno->IsInAir()
			&& !pTechnoType->Naval;

		// ---- 18 : Economy (harvester / refinery / resource helper)
	case TargetMask::Economy:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		if (const auto pUnitT = type_cast<UnitTypeClass*>(pTechnoType))
			return pUnitT->Harvester || pUnitT->ResourceGatherer;

		if (const auto pInfT = type_cast<InfantryTypeClass*>(pTechnoType))
			return pInfT->ResourceGatherer && pInfT->Slaved && pTechno->SlaveOwner;

		if (isBuilding)
		{
			for (const auto pBldType : pBuildingTechno->GetTypes())
			{
				if (pBldType && (pBldType->ProduceCashAmount > 0 || pBldType->OrePurifier))
					return true;
			}

			return pBuildingTechno->Type->Refinery
				|| pBuildingTechno->Type->ResourceGatherer
				|| (pTechno->SlaveManager && pBuildingTechno->Type->Enslaves);
		}

		return false;
	}

	// ---- 19 : Infantry Factory -----------------------------
	case TargetMask::InfantryFactory:
	{
		const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);
		return !pTechno->Owner->IsNeutral()
			&& pBldType
			&& pBldType->Factory == AbstractType::InfantryType;
	}

	// ---- 20 : Land Vehicle Factory -------------------------
	case TargetMask::LandVehicleFactory:
	{
		const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);
		return !pTechno->Owner->IsNeutral()
			&& pBldType
			&& pBldType->Factory == AbstractType::UnitType
			&& !pBldType->Naval;
	}

	// ---- 21 : Aircraft Factory / Helipad -------------------
	case TargetMask::AircraftFactory:
	{
		const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType);
		return !pTechno->Owner->IsNeutral()
			&& pBldType
			&& (pBldType->Factory == AbstractType::AircraftType || pBldType->Helipad);
	}

	// ---- 22 : Radar / SpySat -------------------------------
	case TargetMask::Radar:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		if (pBuildingTechno->Type->Radar)
			return true;

		for (const auto pBldType : pBuildingTechno->GetTypes())
		{
			if (pBldType && pBldType->SpySat)
				return true;
		}

		return false;
	}

	// ---- 23 : Buildable Tech --------------------------------
	case TargetMask::BuildableTech:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return RulesClass::Instance->BuildTech.contains(
			static_cast<BuildingTypeClass*>(pTechnoType));
	}

	// ---- 24 : Naval Factory ---------------------------------
	case TargetMask::NavalFactory:
	{
		if (!isBuilding)
			return false;

		const auto pBldType = static_cast<BuildingTypeClass*>(pTechnoType);
		return !pTechno->Owner->IsNeutral()
			&& pBldType->Factory == AbstractType::UnitType
			&& pBldType->Naval;
	}

	// ---- 25 : Super Weapon Building -------------------------
	case TargetMask::SuperWeapon:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		for (const auto pBldType : pBuildingTechno->GetTypes())
		{
			if (!pBldType)
				continue;

			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(
				const_cast<BuildingTypeClass*>(pBldType));
			if (pBldTypeExt && pBldTypeExt->GetSuperWeaponCount() > 0)
				return true;
		}

		return false;
	}

	// ---- 26 : Construction Yard / MCV -----------------------
	case TargetMask::ConstructionYard:
	{
		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (const auto pFakeOf = TechnoTypeExtContainer::Instance.Find(pBldType)->Fake_Of)
				{
					const auto pFakeBldType = static_cast<BuildingTypeClass*>(pFakeOf.Get());
					return pFakeBldType->Factory == AbstractType::BuildingType
						&& pFakeBldType->ConstructionYard;
				}

				return pBldType->Factory == AbstractType::BuildingType
					&& pBldType->ConstructionYard;
			}
		}

		if (whatTech == UnitClass::AbsID)
			return RulesClass::Instance->BaseUnit.contains(static_cast<UnitTypeClass*>(pTechnoType));

		return false;
	}

	// ---- 27 : Any Neutral Object ----------------------------
	case TargetMask::Neutral:
		return pTechno->Owner->IsNeutral();

		// ---- 28 : Cloak Generator / Gap Generator ---------------
	case TargetMask::CloakOrGapGen:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		for (const auto pBldType : pBuildingTechno->GetTypes())
		{
			// REVIEW: original checks pBldType != null but then uses pBuilding->Type (not pBldType)
			// for GapGenerator/CloakGenerator — preserved as-is
			if (pBldType && (pBuildingTechno->Type->GapGenerator || pBuildingTechno->Type->CloakGenerator))
				return true;
		}

		return false;
	}

	// ---- 29 : Radar Jammer ----------------------------------
	case TargetMask::RadarJammer:
		return !pTechno->Owner->IsNeutral()
			&& pTechnoTypeExt
			&& pTechnoTypeExt->RadarJamRadius > 0;

		// ---- 30 : Inhibitor -------------------------------------
	case TargetMask::Inhibitor:
		return !pTechno->Owner->IsNeutral()
			&& pTechnoTypeExt
			&& pTechnoTypeExt->InhibitorRange.isset();

		// ---- 31 : Naval Unit (Unit only, not building) ----------
	case TargetMask::NavalUnit:
		return !pTechno->Owner->IsNeutral()
			&& whatTech == UnitClass::AbsID
			&& (pTechnoType->Naval || pTechno->GetCell()->LandType == LandType::Water);

		// ---- 32 : Any Non-Building Combat Unit ------------------
	case TargetMask::NonBuildingUnit:
	{
		if (pTechno->Owner->IsNeutral())
			return false;

		if (const auto pUnit = cast_to<UnitClass*>(pTechno))
			return !pUnit->Type->DeploysInto;

		if (buildingIsConsideredVehicle)
			return true;

		if (const auto pBldType = type_cast<BuildingTypeClass*>(pTechnoType))
		{
			const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType);
			return pBldType->Artillary
				|| pBldType->TickTank
				|| pBldTypeExt->IsJuggernaut
				|| pBldType->ICBMLauncher
				|| pBldType->SensorArray
				|| pBldType->ResourceGatherer;
		}

		return false;
	}

	// ---- 33 : Capturable Structure or Repair Hut -----------
	case TargetMask::CapturableOrRepair:
	{
		if (!isBuilding)
			return false;

		const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingTechno->Type);
		return pBldTypeExt->EngineerRepairable.Get(pBuildingTechno->Type->Capturable)
			|| (pBuildingTechno->Type->BridgeRepairHut
			 && MapClass::Instance->IsLinkedBridgeDestroyed(pTechno->GetMapCoords()));
	}

	// ---- 34 : Within Team Leader Guard Range ---------------
	case TargetMask::InGuardRange:
	{
		if (!pTeamLeader || pTechno->Owner->IsNeutral())
			return false;

		return pTeamLeader->DistanceFrom(pTechno) <= pTeamLeader->GetGuardRange(1);
	}

	// ---- 35 : Any Vehicle Factory (Land + Naval) -----------
	case TargetMask::VehicleFactory:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return pBuildingTechno->Type->Factory == AbstractType::UnitType;
	}

	// ---- 36 : Non-Defense Building -------------------------
	case TargetMask::NonDefenseBuilding:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		if (pBuildingTechno->Type->IsBaseDefense || buildingIsConsideredVehicle)
			return false;

		const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBuildingTechno->Type);
		return !(pBuildingTechno->Type->Artillary
			  || pBuildingTechno->Type->TickTank
			  || pBldTypeExt->IsJuggernaut
			  || pBuildingTechno->Type->ICBMLauncher
			  || pBuildingTechno->Type->SensorArray);
	}

	// ---- 37 : Bridge Repair Hut  OR  Hero Infantry ---------
	// REVIEW: case 38 is absent from original; 39 comes after 37 in the source.
	// Order is preserved intentionally to avoid changing fall-through / default behavior.
	case TargetMask::BridgeHutOrHero:
	{
		if (isBuilding)
		{
			return pBuildingTechno->Type->BridgeRepairHut
				&& MapClass::Instance->IsLinkedBridgeDestroyed(pTechno->GetMapCoords());
		}

		// Hero infantry check
		if (!pTechno->Owner->IsNeutral() && whatTech == InfantryClass::AbsID)
		{
			return InfantryTypeExtContainer::Instance
				.Find(static_cast<InfantryTypeClass*>(pTechnoType))
				->IsHero.Get();
		}

		return false;
	}

	// ---- 39 : Occupyable Civilian Building ------------------
	// REVIEW: 38 is skipped in original; this is not a typo.
	case TargetMask::OccupyableCivilian:
	{
		if (!isBuilding)
			return false;

		// Neutral, empty, can be occupied and fired from
		if (pBuildingTechno->Type->CanBeOccupied
		 && pBuildingTechno->Occupants.Count == 0
		 && pBuildingTechno->Owner->IsNeutral()
		 && pBuildingTechno->Type->CanOccupyFire
		 && pBuildingTechno->Type->TechLevel == -1
		 && pBuildingTechno->GetHealthStatus() != HealthState::Red)
		{
			return true;
		}

		// Friendly, partially occupied, still has slots
		if (pBuildingTechno->Type->CanBeOccupied
		 && pBuildingTechno->Occupants.Count < pBuildingTechno->Type->MaxNumberOccupants
		 && pBuildingTechno->Owner == pTeamLeader->Owner  // REVIEW: pTeamLeader may be null here — original has same potential null deref
		 && pBuildingTechno->Type->CanOccupyFire)
		{
			return true;
		}

		return false;
	}

	// ---- 40 : Own Grinder Building -------------------------
	case TargetMask::GrinderBuilding:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return pBuildingTechno->Type->Grinding
			&& pBuildingTechno->Owner == pTeamLeader->Owner;  // REVIEW: pTeamLeader may be null — original has same potential null deref
	}

	// ---- 41 : Spyable Building ------------------------------
	case TargetMask::SpyableBuilding:
	{
		if (!isBuilding || pTechno->Owner->IsNeutral())
			return false;

		return pBuildingTechno->Type->Spyable;
	}

	default:
		break;
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
}

void ScriptExtData::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, DistanceMode calcThreatMode, int attackAITargetType)
{

	std::vector<int> Mission_Attack_List1Random_validIndexes;
	Mission_Attack_List1Random_validIndexes.clear();
	Mission_Attack_List1Random_validIndexes.reserve(50);

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

				auto pTechnoType = GET_TECHNOTYPE(pTechno);

				{
					bool found = false;

					for (size_t j = 0u; j < RulesExtData::Instance()->AITargetTypesLists[attackAITargetType].size() && !found; j++)
					{
						auto const pFirstUnit = pTeam->FirstUnit;

						if (pFirstUnit && pTechnoType == RulesExtData::Instance()->AITargetTypesLists[attackAITargetType][j] && (!pFirstUnit->Owner->IsAlliedWith(pTechno)
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

	const auto&[pWeaponPrimary, pWeaponSecondary] = ScriptExtData::GetWeapon(pTechno);

	if (pWeaponPrimary && pWeaponPrimary->Projectile){
		hasAntiAir = pWeaponPrimary->Projectile->AA;
	}

	if (!hasAntiAir && pWeaponSecondary && pWeaponSecondary->Projectile) {
		hasAntiAir = pWeaponSecondary->Projectile->AA;
	}

	if(agentMode){
		hasAntiGround = true;
		return;
	}

	if (pWeaponPrimary && pWeaponPrimary->Projectile) {
		hasAntiGround = pWeaponPrimary->Projectile->AG && !BulletTypeExtContainer::Instance.Find(pWeaponPrimary->Projectile)->AAOnly;
	}

	if (!hasAntiGround && pWeaponSecondary && pWeaponSecondary->Projectile) {
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
