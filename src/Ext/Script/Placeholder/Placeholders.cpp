#include "Body.h"

#include <Ext/Techno/Body.h>

void NOINLINE ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (TechnoExtData::IsInWarfactory(pUnit))
				continue;

			pUnit->QueueMission(Mission::Area_Guard, true);
		}

		pTeam->GuardAreaTimer.Start(15 * pTeam->CurrentScript->GetCurrentAction().Argument);
	}

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}
NOINLINE TechnoClass* ScriptExt::GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	if (!pTechno || !pTechno->Owner || !Is_Techno(pTechno))
		return nullptr;

	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	const auto pTechnoType = pTechno->GetTechnoType();

	if (!pTechnoType)
		return nullptr;

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		const auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();

		if (object == pTechno)
			continue;

		if (!IsUnitAvailable(object, true, true)
			|| !object->Owner
			|| objectType->Immune
			|| object->TemporalTargetingMe
			|| object->Owner == pTechno->Owner
			|| object->BeingWarpedOut)
			continue;

		if (object->Location == CoordStruct::Empty)
			continue;

		const auto nFootMapCoords = object->InlineMapCoords();
		if (nFootMapCoords == CellStruct::Empty)
			continue;

		//if (pTechno->GetFireError(object, pTechno->SelectWeapon(object), true) != FireError::OK)
		//	continue;

		//// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		const int weaponIndex = pTechno->SelectWeapon(object);

		if (const auto nWeaponType = pTechno->GetWeapon(weaponIndex))
		{

			auto weaponType = nWeaponType->WeaponType;

			auto const& [unitWeaponsHaveAA, unitWeaponsHaveAG] = CheckWeaponsTargetingCapabilites(weaponType, weaponType, agentMode);
			int weaponDamage = 0;

			const auto pVictimExt = TechnoExtContainer::Instance.Find(object);
			Armor nArmor = objectType->Armor;
			if (auto const pShield = pVictimExt->GetShield())
			{
				if (pShield->IsActive())
					nArmor = pShield->GetArmor();
			}

			if (weaponType)
			{
				weaponDamage = MapClass::GetTotalDamage(pTechno->CombatDamage(weaponIndex), weaponType->Warhead, nArmor, 0);
			}

			// If the target can't be damaged then isn't a valid target
			if (weaponType && weaponDamage <= 0 && !agentMode)
				continue;

			if (!agentMode)
			{
				if (objectType->Immune)
					continue;

				if (weaponType)
				{
					auto const& nVerses =
						//GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead , nArmor);
						std::abs(WarheadTypeExtContainer::Instance.Find(weaponType->Warhead)->GetVerses(nArmor).Verses);
					if (!(nVerses >= 0.001))
						continue;
				}

				if (object->IsInAir() && !unitWeaponsHaveAA)
					continue;

				if (!object->IsInAir() && !unitWeaponsHaveAG)
					continue;
			}

			auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(objectType);

			// Check map zone
			if (!TechnoExtData::AllowedTargetByZone(pTechno, object, pTypeExt->TargetZoneScanType, weaponType))
				continue;
		}

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_never || pTechnoType->NavalTargeting == NavalTargetingType::Naval_none))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_not_okay
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;

		if ((!pTechno->Owner->IsAlliedWith(object) || IsUnitMindControlledFriendly(pTechno->Owner, object)))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				bool isGoodTarget = false;
				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						objectThreatValue += objectType->SpecialThreatValue * RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						objectThreatValue += RulesClass::Instance->EnemyHouseThreatBonus;
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

#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/Team/Body.h>

/*
PhobosScripts::RepeatAttackCloserThreat:
PhobosScripts::RepeatAttackFartherThreat:
PhobosScripts::RepeatAttackCloser:
PhobosScripts::RepeatAttackFarther:
PhobosScripts::SingleAttackCloserThreat:
PhobosScripts::SingleAttackFartherThreat:
PhobosScripts::SingleAttackCloser:
PhobosScripts::SingleAttackFarther:
*/
static PhobosMap<TeamClass*, TechnoTypeClass*> TeamUnit {};
void NOINLINE ScriptExt::Mission_Attack(TeamClass* pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TeamUnit.erase(pTeam);

	const auto pScript = pTeam->CurrentScript;
	auto pTeamData = TeamExtContainer::Instance.TryFind(pTeam);

	if (!pScript || !pTeam || !pTeamData)
	{
		if (!pTeam)
		{
			pTeamData->CloseEnough.reset();
		}

		pTeam->StepCompleted = true;
		return;
	}

	int scriptArgument = pScript->GetCurrentAction().Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass* pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;

	auto pHouseExt = HouseExtContainer::Instance.Find(pTeam->Owner);

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

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!IsUnitAvailable(pFocus, true, false))
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExtContainer::Instance.Find(pUnit);
		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward.has_value())
			{
				ScriptExt::IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward.value());
				pTeamData->NextSuccessWeightAward.reset();
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pFocus = nullptr;
			pTeam->Focus = nullptr;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pTeamUnit = pTeam->FirstUnit;
					pTeamUnit;
					pTeamUnit = pTeamUnit->NextTeamMember)
				{
					// Let's reset all Team Members objective
					if (auto pKillerTeamUnitData = TechnoExtContainer::Instance.Find(pTeamUnit))
						pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (Is_Aircraft(pTeamUnit))
					{
						pTeamUnit->SetTarget(nullptr);
						pTeamUnit->LastTarget = nullptr;
						pTeamUnit->QueueMission(Mission::Guard, true);
					}
				}

				pTeamData->IdxSelectedObjectFromAIList.reset();

				// This action finished
				pTeam->StepCompleted = true;
				return;
			}
		}
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (const auto pAir = specific_cast<AircraftClass*>(pUnit))
			{
				TeamUnit[pTeam] = pUnitType;

				if (!pAir->IsInAir() && pAir->Type->AirportBound && pAir->Ammo < pAir->Type->Ammo)
				{
					bAircraftsWithoutAmmo = true;
				}
			}

			pacifistTeam &= !IsUnitArmed(pUnit);

			// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
			if (const auto pTypeInf = specific_cast<InfantryTypeClass*>(pUnitType))
			{
				if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
				{
					agentMode = true;
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!IsUnitAvailable(pLeaderUnit, true, true))
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	//not cleaned up
	if (!Is_Techno(pLeaderUnit))
		pLeaderUnit = nullptr;

	//leader unit doest exist anymore
	if (!pLeaderUnit || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList.reset();
		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		pTeam->StepCompleted = true;
		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();

	auto const& [WeaponType1, WeaponType2] = ScriptExt::GetWeapon(pLeaderUnit);

	// Weapon check used for filtering Leader targets.
	// Note: the Team Leader is picked for this task, be careful with leadership rating values in your mod
	auto [leaderWeaponsHaveAA, leaderWeaponsHaveAG] = CheckWeaponsTargetingCapabilites(WeaponType1, WeaponType2, agentMode);

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
	{
		const bool IsAircraft = pLeaderUnit->WhatAmI() == AircraftClass::AbsID;
		for (NextObject j(pLeaderUnit->Passengers.FirstPassenger->NextObject);
				abstract_cast<FootClass*>(*j);
			++j)
		{

			if (IsAircraft && TechnoExtContainer::Instance.Find(static_cast<FootClass*>(*j))->IsInterceptor())
				continue;

			auto const& [passengerWeaponType1, passengerWeaponType2] = ScriptExt::GetWeapon(static_cast<FootClass*>(*j));

			// Used for filtering targets.
			// Note: the units inside a openTopped Leader are used for this task
			auto const& [PassengersWPHaveAA, PassengersWPHaveAG] = CheckWeaponsTargetingCapabilites(passengerWeaponType1, passengerWeaponType2, agentMode);
			leaderWeaponsHaveAA |= PassengersWPHaveAA;
			leaderWeaponsHaveAG |= PassengersWPHaveAG;
		}
	}

	bool onlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;

	if (pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
		onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		if (onlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = scriptArgument;
		selectedTarget = ScriptExt::GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit;
				pUnit;
				pUnit = pUnit->NextTeamMember)
			{
				if (pUnit->IsAlive && !pUnit->InLimbo)
				{
					const auto pUnitType = pUnit->GetTechnoType();

					if (pUnit != selectedTarget && pUnit->Target != selectedTarget)
					{
						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_not_okay
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pUnit->SetTarget(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						auto const nWhat = pUnit->WhatAmI();

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						auto pAir = specific_cast<AircraftClass*>(pUnit);
						if (pAir)
						{
							if (pUnit->CurrentMission == Mission::Enter || pUnit->Ammo <= 0)
								continue;
							else if (!pUnit->IsInAir() && pUnit->CurrentMission != Mission::Attack)
							{
								pUnit->SetTarget(selectedTarget);
								pUnit->QueueMission(Mission::Attack, true);
							}
						}

						pUnit->SetTarget(selectedTarget);

						if (nWhat == InfantryClass::AbsID && pUnit->IsEngineer())
						{
							if (selectedTarget->Owner->IsAlliedWith(pUnit))
								pUnit->QueueMission(Mission::Enter, true);
							else
								pUnit->QueueMission(Mission::Capture, true);
						}
						else if (nWhat != AircraftClass::AbsID)
						{
							pUnit->QueueMission(Mission::Attack, true);

							if (pUnit->GetCurrentMission() != Mission::Attack)
								pUnit->QueueMission(Mission::Attack, true);

							if (pUnit->GetCurrentMission() == Mission::Move && pUnitType->JumpJet)
								pUnit->QueueMission(Mission::Attack, true);
						}

						// Spy case
						if (nWhat == InfantryClass::AbsID)
						{
							const auto pInfantryType = static_cast<InfantryTypeClass*>(pUnitType);

							if (pInfantryType->Infiltrate &&
								pInfantryType->Agent &&
								pUnit->GetCurrentMission() != Mission::Enter &&
								pUnit->Destination)
							{
								// Check if target is an structure and see if spiable
								pUnit->QueueMission(Mission::Enter, true);
							}
						}

						// Tanya / Commando C4 case
						if ((nWhat == InfantryClass::AbsID
							&& (static_cast<InfantryTypeClass*>(pUnitType)->C4 || pUnit->HasAbility(AbilityType::C4)))
							&& pUnit->GetCurrentMission() != Mission::Sabotage && pUnit->Destination)
						{
							pUnit->QueueMission(Mission::Sabotage, true);
						}
					}
				}
			}
		}
		else
		{
			pTeamData->IdxSelectedObjectFromAIList.reset();

			if (noWaitLoop)
			{
				// This action finished
				pTeam->StepCompleted = true;
				return;
			}
			// No target was found with the specific criteria.
			if (pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);;
			}

			if (pTeamData->WaitNoTargetAttempts != 0)
			{
				// No target? let's wait some frames
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);

				return;
			}

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit
		bool isAirOK = pFocus->IsInAir() && leaderWeaponsHaveAA;
		bool isGroundOK = !pFocus->IsInAir() && leaderWeaponsHaveAG;

		if (IsUnitAvailable(pFocus, true, true)
			&& !pFocus->GetTechnoType()->Immune
			&& (isAirOK || isGroundOK)
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus) || IsUnitMindControlledFriendly(pLeaderUnit->Owner, pFocus)))
		{

			bool bForceNextAction = false;

			for (auto pUnit = pTeam->FirstUnit; pUnit && !bForceNextAction; pUnit = pUnit->NextTeamMember)
			{
				if (!IsUnitAvailable(pUnit, true, true))
					continue;

				auto pUnitType = pUnit->GetTechnoType();
				auto nWhat = pUnit->WhatAmI();

				{
					// Aircraft case 1
					if ((nWhat == AircraftClass::AbsID
						&& static_cast<AircraftTypeClass*>(pUnitType)->AirportBound)
						&& pUnit->Ammo > 0
						&& (pUnit->Target != pFocus && !pUnit->InAir)
						&& pUnit->CurrentMission != Mission::Attack)
					{
						pUnit->SetTarget(pFocus);
						pUnit->QueueMission(Mission::Attack, true);
						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have nti-ground weapons. Ignore the attack
					if (pUnitType->Underwater
						&& pUnitType->LandTargeting == LandTargetingType::Land_not_okay
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);
						bForceNextAction = true;

						continue;
					}

					// Aircraft case 2
					if (nWhat == AircraftClass::AbsID
						&& pUnit->GetCurrentMission() != Mission::Attack
						&& pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pUnit->InAir)
						{
							if (pUnit->InAir)
							{
								if (pUnit->Target != pFocus)
									pUnit->SetTarget(pFocus);

								pUnit->QueueMission(Mission::Attack, true);
							}
						}
						else
						{
							pUnit->EnterIdleMode(false, true);
						}

						continue;
					}

					// Tanya / Commando C4 case
					if ((nWhat == InfantryClass::AbsID
						&& static_cast<InfantryTypeClass*>(pUnitType)->C4
						|| pUnit->HasAbility(AbilityType::C4)) && pUnit->GetCurrentMission() != Mission::Sabotage)
					{
						pUnit->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (nWhat != AircraftClass::AbsID)
					{
						if (pUnit->Target != pFocus)
							pUnit->SetTarget(pFocus);

						if (pUnit->GetCurrentMission() != Mission::Attack
							&& pUnit->GetCurrentMission() != Mission::Unload
							&& pUnit->GetCurrentMission() != Mission::Selling)
						{
							pUnit->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList.reset();
				pTeam->StepCompleted = true;
				auto const& nCur = pScript->GetCurrentAction();
				auto const& nNext = pScript->GetNextAction();
				Debug::Log("AI Scripts - Mission_Attack: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n",
					pTeam->Type->ID,
					pScript->Type->ID,
					pScript->CurrentMission,
					nCur.Action,
					nCur.Argument,
					pScript->CurrentMission + 1,
					nNext.Action,
					nNext.Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
			// No target? let's wait some frames
			pTeamData->WaitNoTargetCounter = 30;
			pTeamData->WaitNoTargetTimer.Start(30);
		}
	}
}

/*
#include <SlaveManagerClass.h>

namespace AresHouseExt
{
	struct Data
	{
		 Data(HouseClass* pWho) :OwnerObject { pWho }
		 { }

		HouseClass* OwnerObject { nullptr };
		int TeamPowerAdd { 0 };

	};

	struct ExtMap
	{
		static AresHouseExt::Data* Find(HouseClass* Own)
		{
			return GameCreate<AresHouseExt::Data>(Own);
		}
	};
};

namespace AresTechnoExt
{
	struct Data
	{
		Data(TechnoClass* pWho) :OwnerObject { pWho }
		{ }

		TechnoClass* OwnerObject { nullptr };
		bool DriverKilled { false };
		CDTimerClass CloakSkipTimer { };
		CDTimerClass DisableWeaponTimer { };
	};

	struct ExtMap
	{
		static AresTechnoExt::Data* Find(TechnoClass* Own)
		{
			return GameCreate<AresTechnoExt::Data>(Own);
		}
	};
};

namespace TechnoTypeExtAres
{
	Nullable<double> ProtectedTreshold { };
	Valueable<bool> Protected { false };
	Promotable<bool> ProtectedRiver { false };
	Valueable<TechnoTypeClass*> Convert_Script { nullptr };
	Valueable<TechnoTypeClass*> Convert_Land { nullptr };
	Valueable<TechnoTypeClass*> Convert_Water { nullptr };

}

bool IsKillDriverAdvisable(FootClass* pThis, double HPTreshold)
{
	if (pThis->WhatAmI() == AbstractType::Infantry ||
		pThis->GetTechnoType()->Natural ||
		pThis->GetTechnoType()->Organic ||
		pThis->BeingWarpedOut ||
		pThis->IsIronCurtained()
		) {
		return false;
	}

	if (pThis->WhatAmI() == AbstractType::Aircraft) {
		auto const pAir = static_cast<AircraftClass*>(pThis);
		if (pAir->Type->AirportBound || pAir->Type->Dock.Count)
			return false;

	}else if (pThis->WhatAmI() == AbstractType::Unit) {

		if (const auto pBuilding = pThis->GetCell()->GetBuilding()) {
			if (pBuilding == pThis->GetNthLink()) {
				auto const pType = pBuilding->Type;
				if (pType->WeaponsFactory && !pType->Naval)
					return false;
			}
		}
	}

	if (TechnoTypeExtAres::ProtectedRiver.Get(pThis))
		return false;

	const auto nOtherTresh = TechnoTypeExtAres::ProtectedTreshold.Get(TechnoTypeExtAres::Protected ? 0.0 : 1.0);

	if (nOtherTresh <= HPTreshold)
		HPTreshold = nOtherTresh;

	//Check if HP is good ?
	if (pThis->GetHealthPercentage() > HPTreshold)
		return false;

	return true;
}

bool ConvertType(TechnoClass* pThis, TechnoTypeClass* pToType)
{
	AbstractType nCurType = AbstractType::None;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
	{
		nCurType = AbstractType::InfantryType;
		break;
	}
	case AbstractType::Unit:
	{
		nCurType = AbstractType::UnitType;
		break;
	}
	case AbstractType::Aircraft:
	{
		nCurType = AbstractType::AircraftType;
		break;
	}
	default:
		return false;
	}

	if (nCurType == AbstractType::None || nCurType != pToType->WhatAmI())
		return false;

	if (auto pTemp = pThis->TemporalImUsing)
		if (pTemp->Target)
			pTemp->Detach();

	auto pOwner = pThis->Owner;

	if (!pThis->InLimbo)
	{
		pOwner->RegisterLoss(pThis, false);
	}

	pOwner->RemoveTracking(pThis);

	auto nHealth = pThis->Health/ pThis->GetType()->Strength ;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry: {
		static_cast<InfantryClass*>(pThis)->Type = static_cast<InfantryTypeClass*>(pToType);
		break;
	}
	case AbstractType::Unit: {
		static_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pToType);
		break;
	}
	case AbstractType::Aircraft: {
		static_cast<AircraftClass*>(pThis)->Type = static_cast<AircraftTypeClass*>(pToType);
		break;
	}
	}

	pThis->AdjustStrength(nHealth); //health adjusted after type change
	pThis->EstimatedHealth = pThis->Health;
	pOwner->AddTracking(pThis);

	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, true);

	pOwner->RecheckTechTree = true;

	//clearing some AresStateHere
	//
	//

	//ammo
	auto nNewAmmo = pToType->Ammo;
	if (nNewAmmo >= pThis->Ammo)
		nNewAmmo = pThis->Ammo;

	pThis->Ammo = nNewAmmo;

	//Update Ares BuildingLightClass

	//ROT
	pThis->PrimaryFacing.turn_rate(pToType->ROT);

	//Set Turrent Facing Here

	//Replace Locomotor
	GUID nCurID { };
	static_cast<LocomotionClass*>(static_cast<FootClass*>(pThis)->Locomotor.get())->GetClassID(&nCurID);

	if (nCurID != pToType->Locomotor)
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(static_cast<FootClass*>(pThis)->Locomotor)) { };

		if (auto NewLoco = LocomotionClass::CreateInstance(pToType->Locomotor))
		{
			static_cast<FootClass*>(pThis)->Locomotor = std::move(NewLoco);
			static_cast<FootClass*>(pThis)->Locomotor->Link_To_Object(pThis);
		}

		// handling for Locomotor weapons: since we took this unit from the Magnetron
		// in an unfriendly way, set these fields here to unblock the unit
		if (static_cast<FootClass*>(pThis)->IsAttackedByLocomotor || static_cast<FootClass*>(pThis)->IsLetGoByLocomotor)
		{
			static_cast<FootClass*>(pThis)->IsAttackedByLocomotor = false;
			static_cast<FootClass*>(pThis)->IsLetGoByLocomotor = false;
			static_cast<FootClass*>(pThis)->FrozenStill = false;
		}
	}

	return true;
}

void TechnoClassUpdate(TechnoClass* pThis)
{
	auto const pCurType = pThis->GetTechnoType();

	if (TechnoTypeExtAres::Convert_Land || TechnoTypeExtAres::Convert_Water)
	{
		if (pThis->WhatAmI() != AbstractType::Aircraft || pThis->WhatAmI() != AbstractType::Building)
		{
			auto pNewType = pThis->OnBridge ? TechnoTypeExtAres::Convert_Land : TechnoTypeExtAres::Convert_Water;
			if (pCurType && pCurType != pNewType)
				ConvertType(pThis, pNewType);
		}
	}
}

bool KillTheDriver(TechnoClass* pVictim, TechnoClass* pDestroyer, HouseClass* pHouseAfter)
{
	if (!((pVictim->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None))
		return false;

	auto pVictimExt = TechnoExtContainer::Instance.Find(pVictim);

	auto const passive = pHouseAfter->Type->MultiplayPassive;
	pVictimExt->DriverKilled = passive;

	// exit if owner would not change
	if (pVictim->Owner == pHouseAfter) {
		return false;
	}
	auto pType = pVictim->GetTechnoType();
	auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pTarget = pVictim;
	auto pSource = pDestroyer;
	// If this vehicle uses Operator=, we have to take care of actual "physical" drivers, rather than theoretical ones
	if (pVictimTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt && pVictim->Passengers.GetFirstPassenger())
	{
		// kill first passenger
		auto const pPassenger = pVictim->RemoveFirstPassenger();
		pPassenger->RegisterDestruction(pDestroyer);
		pPassenger->UnInit();
	}
	else if (auto const pOperatorType = pVictimTypeExt->Operator)
	{
		// find the driver cowardly hiding among the passengers, then kill him
		for (NextObject passenger(pVictim->Passengers.GetFirstPassenger()); passenger; ++passenger)
		{
			auto const pPassenger = static_cast<FootClass*>(*passenger);
			if (pPassenger->GetTechnoType() == pOperatorType)
			{
				pVictim->RemovePassenger(pPassenger);
				pPassenger->RegisterDestruction(pDestroyer);
				pPassenger->UnInit();
				break;
			}
		}
	}
	// if passengers remain in the vehicle, operator-using or not, they should leave
	if (pVictim->Passengers.GetFirstPassenger())
	{
		TechnoExtData::EjectPassengers(pVictim, -1);
	}
	pTarget->HijackerInfantryType = -1;
	// If this unit is driving under influence, we have to free it first
	if (auto const pController = pTarget->MindControlledBy)
	{
		if (auto const pCaptureManager = pController->CaptureManager)
		{
			pCaptureManager->FreeUnit(pTarget);
		}
	}
	pTarget->MindControlledByAUnit = false;
	pTarget->MindControlledByHouse = nullptr;
	// remove the mind-control ring anim
	if (pTarget->MindControlRingAnim)
	{
		pTarget->MindControlRingAnim->UnInit();
		pTarget->MindControlRingAnim = nullptr;
	}
	// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
	if (pTarget->CaptureManager)
	{
		pTarget->CaptureManager->FreeAll();
	}
	// This unit will be freed of its duties
	static_cast<FootClass*>(pTarget)->LiberateMember();
	// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
	if (auto const pSpawnManager = pTarget->SpawnManager)
	{
		pSpawnManager->KillNodes();
		pSpawnManager->ResetTarget();
	}
	// If this unit enslaves stuff, we should free the slaves, since they still belong to the previous owner
			// <DCoder> SlaveManagerClass::Killed() sets the manager's Owner to NULL
			// <Renegade> okay, does Killed() also destroy the slave manager, or just unlink it from the unit?
			// <DCoder> unlink
			// <Renegade> so on principle, I could just re-link it?
			// <DCoder> yes you can
	if (auto const pSlaveManager = pTarget->SlaveManager)
	{
		pSlaveManager->Killed(pSource);
		pSlaveManager->ZeroOutSlaves();
		pSlaveManager->Owner = pTarget;
		if (passive)
		{
			pSlaveManager->SuspendWork();
		}
		else
		{
			pSlaveManager->ResumeWork();
		}
	}
	// Hand over to a different house
	pTarget->SetOwningHouse(pHouseAfter);
	if (passive) {
		pTarget->QueueMission(Mission::Harmless, true);
	}

	pTarget->SetTarget(nullptr);
	pTarget->SetDestination(nullptr, false);

	if (auto pTag = pTarget->AttachedTag) {
		pTag->RaiseEvent(static_cast<TriggerEvent>(0x44), pTarget, CellStruct::Empty, false, pDestroyer); //new
	}

	if(pTarget->IsAlive) {
		if (auto pTag = pTarget->AttachedTag) {
			pTag->RaiseEvent(static_cast<TriggerEvent>(0x43), pTarget, CellStruct::Empty, false, nullptr); //new
		}
	}

	return true;
}

bool ProcessAction_Ares(TeamClass* pTeam , ScriptActionNode nNode)
{
	auto const& [action, argument] = nNode;
	enum class AresScripts : int
	{
		AuxPower = 65 ,
		KillDriver = 66,
		TakeVehicle = 67,
		ConvertType = 68 ,
		DisableWeapon = 69 ,
		SonarReveal
	};

	//ares reuse this

	if (action > 64)
	{
		switch (static_cast<AresScripts>(action))
		{
		case AresScripts::AuxPower:
		{
			auto const pOwner = pTeam->Owner;
			auto const pExt = AresHouseExt::ExtMap::Find(pOwner);
			pExt->TeamPowerAdd += argument;
			pOwner->RecheckPower = 1;
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::KillDriver:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (pCur->Health > 0 && pCur->IsAlive && pCur->IsOnMap && !pCur->InLimbo)
				{
					auto pExt = AresTechnoExt::ExtMap::Find(pCur);
					if (!pExt->DriverKilled)
					{
						if (IsKillDriverAdvisable(pCur,1.0))
						{
							//KillTheDriver(pCur,nullptr,HouseExtData::FindSpecial());
						}
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::TakeVehicle:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				//auto pUnitExt = TechnoExtContainer::Instance.Find(pCur);
				//pUnitExt->TakeVehicle = true;//used on GarrisonStructure function hook , if yes replace it wit take vehicle func
				//if (pCur->GarrisonStructure()) {
				//	pTeam->LiberateMember(pCur,-1,1);
				//}
			}

			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::ConvertType:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pConvertType = TechnoTypeExtAres::Convert_Script.Get()) {
					ConvertType(pCur, pConvertType);
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::SonarReveal:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pExt = AresTechnoExt::ExtMap::Find(pCur))
				{
					auto const delay = MaxImpl(pExt->CloakSkipTimer.GetTimeLeft(), argument);
					pExt->CloakSkipTimer.Start(delay);

					// actually detect this
					if (pCur->CloakState != CloakState::Uncloaked)
					{
						pCur->Uncloak(true);
						pCur->NeedsRedraw = true;
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		case AresScripts::DisableWeapon:
		{
			for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
			{
				if (auto pExt = AresTechnoExt::ExtMap::Find(pCur))
				{
					auto const delay = MaxImpl(pExt->DisableWeaponTimer.GetTimeLeft(), argument);
					pExt->DisableWeaponTimer.Start(delay);

					// actually detect this
					if (pCur->CloakState != CloakState::Uncloaked)
					{
						pCur->Uncloak(true);
						pCur->NeedsRedraw = true;
					}
				}
			}
			pTeam->StepCompleted = 1;
			return true;
		}
		}
	}else
	if (action == 64)
	{
		for (auto pCur = pTeam->FirstUnit; pCur; pCur->NextTeamMember)
		{
			if (pCur->GarrisonStructure()) {
				pTeam->LiberateMember(pCur,-1,1);
			}
		}
		pTeam->StepCompleted = 1;
		return true;
	}

	return false;
}*/

static void ExecuteTimedAreaGuardAction(TeamClass* pTeam);
static void LoadIntoTransports(TeamClass* pTeam);
static void WaitUntilFullAmmoAction(TeamClass* pTeam);
static void Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown);
static void Mission_Attack(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType, int IdxAITargetTypeItem);
static TechnoClass* GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode);
static bool EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass* pTeamLeader);

static void DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
static void IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
static void WaitIfNoTarget(TeamClass* pTeam, int attempts);
static void TeamWeightReward(TeamClass* pTeam, std::optional<double> award = std::nullopt);
static void PickRandomScript(TeamClass* pTeam, int idxScriptsList);
static void Mission_Move(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
static TechnoClass* FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
static void UnregisterGreatSuccess(TeamClass* pTeam);

static void Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
static void Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
static void Mission_Move_List(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType);
static void Mission_Move_List1Random(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
static void SetCloseEnoughDistance(TeamClass* pTeam, std::optional<double> closeDistance = std::nullopt);
static void SetMoveMissionEndMode(TeamClass* pTeam, SetMoveMissionEndModes mode);
static void SkipNextAction(TeamClass* pTeam, int successPercentage);
static FootClass* FindTheTeamLeader(TeamClass* pTeam);
static void Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine, int count);
static void Stop_ForceJump_Countdown(TeamClass* pTeam);

static void ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);

static bool IsExtVariableAction(int action);
static void VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);

template<bool IsGlobal, class _Pr>
static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);

static void RallyUnitInMap(TeamClass* pTeam, int nArg);

static bool IsValidFriendlyTarget(TeamClass* pTeam, int group, TechnoClass* target, bool isSelfNaval, bool isSelfAircraft, bool isFriendly);
static bool StopTeamMemberMoving(TeamClass* pTeam);
static bool IsValidRallyTarget(TeamClass* pTeam, FootClass* pFoot, int nType);
static void TeamMemberSetGroup(TeamClass* pTeam, int group);
static void DistributedLoadOntoTransport(TeamClass* pTeam, int type);
static void FollowFriendlyByGroup(TeamClass* pTeam, int group);

static ScriptActionNode GetSpecificAction(ScriptClass* pScript, int nIdx);
static std::pair<WeaponTypeClass*, WeaponTypeClass*> GetWeapon(TechnoClass* pTechno);
static void CreateNewCurrentScript(TeamClass* pThis, ScriptTypeClass* pNewType);
static void ClearCurrentScript(TeamClass* pThis);
static ScriptTypeClass* GetFromAIScriptList(size_t nIdx);
static std::pair<bool, bool> CheckWeaponsTargetingCapabilites(WeaponTypeClass* pWeaponPrimary, WeaponTypeClass* pWeaponSecondary, bool agentMode);

static void ResetAngerAgainstHouses(TeamClass* pTeam);
static void SetHouseAngerModifier(TeamClass* pTeam, int modifier);
static void ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList);
static void ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList);
static void ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse);
static void SetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode, bool random);
static void OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);
static void AggroHouse(TeamClass* pTeam, int index);
static HouseClass* GetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode);
static void UpdateEnemyHouseIndex(HouseClass* pHouse);

static bool ConditionalJump_MakeEvaluation(int comparatorMode, int studiedValue, int comparatorValue);
static void ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine);
static void ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine);
static void ConditionalJump_KillEvaluation(TeamClass* pTeam);
static void ConditionalJump_ManageKillsCounter(TeamClass* pTeam, int enable);
static void ConditionalJump_SetIndex(TeamClass* pTeam, int index);
static void ConditionalJump_SetComparatorValue(TeamClass* pTeam, int value);
static void ConditionalJump_SetComparatorMode(TeamClass* pTeam, int value);
static void ConditionalJump_SetCounter(TeamClass* pTeam, int value);
static void SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable);
static void ConditionalJump_ResetVariables(TeamClass* pTeam);
static void ConditionalJump_CheckHumanIsMostHated(TeamClass* pTeam);
static void ConditionalJump_CheckAliveHumans(TeamClass* pTeam, int mode);
static void ConditionalJump_CheckObjects(TeamClass* pTeam);
static void ConditionalJump_CheckCount(TeamClass* pTeam, int modifier);
static void ConditionalJump_ManageResetIfJump(TeamClass* pTeam, int enable);

static void JumpBackToPreviousScript(TeamClass* pTeam);

static void ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType, bool isEnabled);
static void ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse, int sideIdx, int houseIdx, bool isEnabled);
static void SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx);
static void SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx);
static void ManageAITriggers(TeamClass* pTeam, int enabled);
static void ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType, bool isEnabled);

static void RepairDestroyedBridge(TeamClass* pTeam, int mode);
static bool FindLinkedPath(TeamClass* pTeam, TechnoClass* pThis, TechnoClass* pTarget);

static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, MoveMissionEndModes mode);

static bool IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed, bool allowSubterranean);

static void ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance);
static void ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget);

static bool IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno);
static bool IsUnitArmed(TechnoClass* pTechno);
//int WaitNoTargetAttempts { 0 };
//std::optional<double> NextSuccessWeightAward {};
//std::optional<int> IdxSelectedObjectFromAIList { };
//std::optional<int> CloseEnough {};
//int Countdown_RegroupAtLeader { -1 };
//MoveMissionEndModes MoveMissionEndMode { MoveMissionEndModes::Fetch };
//int WaitNoTargetCounter { 0 };
//CDTimerClass WaitNoTargetTimer {};
//CDTimerClass ForceJump_Countdown {};
//int ForceJump_InitialCountdown { -1 };
//bool ForceJump_RepeatMode { false };
//FootClass* TeamLeader { nullptr };
//int GenericStatus { 0 };
//int FailedCounter { -1 };

//// #442
//int AngerNodeModifier { 5000 };
//bool OnlyTargetHouseEnemy { false };
//int OnlyTargetHouseEnemyMode { -1 };

//// #599
//bool ConditionalJump_Evaluation { false };
//int ConditionalJump_ComparatorMode { 3 };
//int ConditionalJump_ComparatorValue { 1 };
//int ConditionalJump_Counter { 0 };
//int ConditionalJump_Index { -1000000 };
//bool AbortActionAfterKilling { false };
//bool ConditionalJump_EnabledKillsCount { false };
//bool ConditionalJump_ResetVariablesIfJump { false };

////#691
//std::vector<ScriptClass*> PreviousScriptList { };

////#565
//int TriggersSideIdx { -1 };
//int TriggersHouseIdx { -1 };

////#791
//std::vector<std::vector<bool>> MapPath_Grid { }; // Used for marking visited/analyzed cells
//std::vector<MapPathCellElement> MapPath_Queue { }; // Cells that will be analyzed for finding a path
//bool MapPath_InProgress { false };
//TechnoClass* MapPath_StartTechno { nullptr };
//TechnoClass* MapPath_EndTechno { nullptr };
//std::vector<TechnoClass*> MapPath_BridgeRepairHuts { };
//std::vector<TechnoClass*> MapPath_ValidBridgeRepairHuts { };
//std::vector<TechnoClass*> MapPath_CheckedBridgeRepairHuts { };

		//.Process(this->WaitNoTargetAttempts)
		//.Process(this->NextSuccessWeightAward)
		//.Process(this->IdxSelectedObjectFromAIList)
		//.Process(this->CloseEnough)
		//.Process(this->Countdown_RegroupAtLeader)
		//.Process(this->MoveMissionEndMode)
		//.Process(this->WaitNoTargetCounter)
		//.Process(this->WaitNoTargetTimer)
		//.Process(this->ForceJump_Countdown)
		//.Process(this->ForceJump_InitialCountdown)
		//.Process(this->ForceJump_RepeatMode)
		//.Process(this->TeamLeader)
		//.Process(this->GenericStatus)
		//.Process(this->FailedCounter)

		//.Process(this->AngerNodeModifier)
		//.Process(this->OnlyTargetHouseEnemy)
		//.Process(this->OnlyTargetHouseEnemyMode)

		//.Process(this->ConditionalJump_Evaluation)
		//.Process(this->ConditionalJump_ComparatorMode)
		//.Process(this->ConditionalJump_ComparatorValue)
		//.Process(this->ConditionalJump_EnabledKillsCount)
		//.Process(this->ConditionalJump_Counter)
		//.Process(this->AbortActionAfterKilling)
		//.Process(this->ConditionalJump_Index)
		//.Process(this->ConditionalJump_ResetVariablesIfJump)

		//.Process(this->PreviousScriptList)

		//.Process(this->TriggersSideIdx)
		//.Process(this->TriggersHouseIdx)

		//.Process(this->MapPath_Grid)
		//.Process(this->MapPath_Queue)
		//.Process(this->MapPath_InProgress)
		//.Process(this->MapPath_StartTechno)
		//.Process(this->MapPath_EndTechno)
		//.Process(this->MapPath_BridgeRepairHuts)
		//.Process(this->MapPath_ValidBridgeRepairHuts)
		//.Process(this->MapPath_CheckedBridgeRepairHuts)

AnnounceInvalidPointer(TeamLeader, ptr);
AnnounceInvalidPointer(MapPath_StartTechno, ptr);
AnnounceInvalidPointer(MapPath_EndTechno, ptr);
AnnounceInvalidPointer(MapPath_BridgeRepairHuts, ptr);
AnnounceInvalidPointer(MapPath_ValidBridgeRepairHuts, ptr);
AnnounceInvalidPointer(MapPath_CheckedBridgeRepairHuts, ptr);
AnnounceInvalidPointer(PreviousScriptList, ptr);

#define TECHNO_IS_ALIVE(tech) TechnoExtData::IsAlive(tech)

ScriptActionNode NOINLINE ScriptExt::GetSpecificAction(ScriptClass* pScript, int nIdx)
{
	if (nIdx == -1 || nIdx > (pScript->Type->ActionsCount - 1))
		return { -1 , 0 };

	return pScript->Type->ScriptActions[nIdx];
	//	nIdx = pScript->Type->ActionsCount;
	//
	//auto const nIdxR = nIdx - ScriptTypeClass::MaxActions;
	//auto const pTypeExt = ScriptTypeExt::ExtMap.Find(pScript->Type);
	//
	//if (!pTypeExt->PhobosNode.empty() && nIdxR < (int)pTypeExt->PhobosNode.size()) {
	//	return pTypeExt->PhobosNode[nIdxR];
	//}
	//constexpr auto const nMax = ScriptTypeClass::MaxActions - 1;
	//return pScript->Type->ScriptActions[nMax];
}

static inline bool IsEmpty(TeamClass* pTeam)
{
	int nCount = 0;

	for (auto const& UnitCount : pTeam->CountObjects)
		nCount += UnitCount;

	return nCount <= 0;
}


#pragma region TransportStuffs

void ScriptExt::TeamMemberSetGroup(TeamClass* pTeam, int group)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		pUnit->Group = group;
		pTeam->AddMember(pUnit, false);
	}

	pTeam->StepCompleted = true;
}

bool ScriptExt::StopTeamMemberMoving(TeamClass* pTeam)
{
	bool stillMoving = false;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (TechnoExtData::IsInWarfactory(pUnit))
			continue;

		if (pUnit->CurrentMission == Mission::Move || pUnit->Locomotor->Is_Moving())
		{
			pUnit->ForceMission(Mission::Wait);
			pUnit->CurrentTargets.Clear();
			pUnit->SetTarget(nullptr);
			pUnit->SetFocus(nullptr);
			pUnit->SetDestination(nullptr, true);
			stillMoving = true;
		}
	}
	return stillMoving;
}

void ScriptExt::DistributedLoadOntoTransport(TeamClass* pTeam, int nArg)
{
	const int T_NO_GATHER = 0, T_COUNTDOWN = 1, T_AVGPOS = 2;
	/*
	type 0: stop member from gathering and begin load now
	type 1: don't stop, maintain previous action, countdown LOWORD seconds then begin load
	type 2: don't stop, gether around first member's position range LOWORD then begin load
	*/

	// can proceed to load calculate logic
	const int R_CAN_PROCEED = 1;
	// type 1 waiting stage, start timer and wait timer stop, when stop, proceed to loading
	const int R_WAITING = 2;
	// type 2 waiting stage, start timer and wait timer stop, when stop, check average position
	const int R_WAIT_POS = 3;

	int nType = HIWORD(nArg);
	int nNum = LOWORD(nArg);
	auto pExt = TeamExtContainer::Instance.Find(pTeam);

	if (!pExt)
	{
		pTeam->StepCompleted = true;
		return;
	}

	const auto remainingSize = [](FootClass* src)
	{
		auto type = src->GetTechnoType();
		return type->Passengers - src->Passengers.GetTotalSize();
	};

	auto pFoot = pTeam->FirstUnit;

	if (!pFoot)
	{
		pTeam->StepCompleted = true;
		return;
	}

	//auto timer = pFoot->BlockagePathTimer;
	// Wait for timer stop
	if (pTeam->GuardAreaTimer.TimeLeft > 0)
	{
		pTeam->GuardAreaTimer.TimeLeft--;
		return;
	}
	// type 1 times up
	else if (pExt->GenericStatus == R_WAITING)
	{
		bool stillMoving = StopTeamMemberMoving(pTeam);
		if (!stillMoving)
		{
			pExt->GenericStatus = R_CAN_PROCEED;
			goto beginLoad;
		}
	}
	// type 2 times up, check distance
	else if (pExt->GenericStatus == R_WAIT_POS)
	{
		bool canProceed = true;
		auto pCell = pFoot->GetCell();

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			// no blockage, just keep moving
			if (pUnit->Locomotor.GetInterfacePtr()->Is_Moving_Now())
			{
				canProceed = false;
				continue;
			}

			if (pUnit->DistanceFrom(pFoot) / 256 <= nNum)
			{
				pUnit->StopMoving();
				pUnit->CurrentTargets.Clear();
				pUnit->SetTarget(nullptr);
				pUnit->SetFocus(nullptr);
				pUnit->SetDestination(nullptr, true);
			}
			else
			{
				auto nCoord = pCell->GetCoords();
				pUnit->MoveTo(&nCoord);
				canProceed = false;
			}
		}
		if (canProceed)
		{
			pExt->GenericStatus = R_CAN_PROCEED;
			return;
		}
		else
		{
			pTeam->GuardAreaTimer.Start(5);
			return;
		}
	}

	// Check if this script can skip this frame
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		// If anyone is entering transport, means this script is now proceeding
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
	}

	// Status jump
	if (pExt->GenericStatus == R_CAN_PROCEED)
		goto beginLoad;

	// switch mode
	// 0: stop member from gathering
	if (nType == T_NO_GATHER)
	{
		// If anyone is moving, stop now, and add timer
		// why team member will converge upon team creation
		// fuck you westwood
		pTeam->Focus = nullptr;
		pTeam->QueuedFocus = nullptr;
		bool stillMoving = StopTeamMemberMoving(pTeam);
		if (stillMoving)
		{
			pTeam->GuardAreaTimer.Start(45);
			return;
		}
	}
	// 1: don't stop, maintain previous action
	// default: gather near team center position (upon team creation, it's auto), countdown timer LOWORD seconds
	else if (nType == T_COUNTDOWN)
	{
		if (pExt->GenericStatus == R_WAITING)
			return;
		pTeam->GuardAreaTimer.Start(nNum * 15);
		pExt->GenericStatus = R_WAITING;
		return;
	}
	// 2: don't stop, manually gather around first member's position
	else if (nType == T_AVGPOS)
	{
		auto pCell = pFoot->GetCell();
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto nCoord = pCell->GetCoords();
			pUnit->MoveTo(&nCoord);
		}
		pTeam->GuardAreaTimer.Start(5);
		pExt->GenericStatus = R_WAIT_POS;
		return;
	}

beginLoad:
	// Now we're talking
	std::vector<FootClass*> transports, passengers;
	std::unordered_map<FootClass*, double> transportSpaces;
	// Find max SizeLimit to determine which type is considered as transport
	double maxSizeLimit = 0;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pType = pUnit->GetTechnoType();
		maxSizeLimit = MaxImpl(maxSizeLimit, pType->SizeLimit);
	}
	// No transports remaining
	if (maxSizeLimit == 0)
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
		return;
	}
	// All member share this SizeLimit will consider as transport
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pType = pUnit->GetTechnoType();
		if (pType->SizeLimit == maxSizeLimit)
		{
			int space = remainingSize(pUnit);
			transports.push_back(pUnit);
			transportSpaces[pUnit] = space;
		}
		else
			passengers.push_back(pUnit);
	}
	// If there are no passengers
	// then this script is done
	if (passengers.empty())
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
		return;
	}
	// If transport is on building, scatter, and discard this frame
	for (auto const& pUnit : transports)
	{
		auto const pCell = pUnit->GetCell();
		if (pCell->GetBuilding())
		{
			pUnit->Scatter(pCell->GetCoordsWithBridge(), true, false);
			return;
		}
	}

	// Load logic
	// range prioritize
	bool passengerLoading = false;
	// larger size first
	std::sort(passengers.begin(), passengers.end(), [](FootClass* const a, FootClass* const b)
 {
	 return a->GetTechnoType()->Size > b->GetTechnoType()->Size;
	});

	for (auto const& pPassenger : passengers)
	{
		auto pPassengerType = pPassenger->GetTechnoType();
		// Is legal loadable unit ?
		if (!Is_Aircraft(pPassenger) &&
			!pPassengerType->ConsideredAircraft &&
			TECHNO_IS_ALIVE(pPassenger))
		{
			FootClass* targetTransport = nullptr;
			double distance = INFINITY;
			for (auto pTransport : transports)
			{
				auto pTransportType = pTransport->GetTechnoType();
				double currentSpace = transportSpaces[pTransport];
				// Can unit load onto this car ?
				if (currentSpace > 0 &&
					pPassengerType->Size > 0 &&
					pPassengerType->Size <= pTransportType->SizeLimit &&
					pPassengerType->Size <= currentSpace)
				{
					double d = pPassenger->DistanceFrom(pTransport);
					if (d < distance)
					{
						targetTransport = pTransport;
						distance = d;
					}
				}
			}
			// This is nearest available transport
			if (targetTransport)
			{
				// Get on the car
				if (pPassenger->GetCurrentMission() != Mission::Enter)
				{
					pPassenger->QueueMission(Mission::Enter, true);
					pPassenger->SetTarget(nullptr);
					pPassenger->SetDestination(targetTransport, false);
					transportSpaces[targetTransport] -= pPassengerType->Size;
					passengerLoading = true;
				}
			}
		}
	}
	// If no one is loading, this script is done
	if (!passengerLoading)
	{
		pTeam->StepCompleted = true;
		pExt->GenericStatus = 0;
	}
	// Load logic
	// speed prioritize
	//for (auto pTransport : transports)
	//{
	//	DynamicVectorClass<FootClass*> loadedUnits;
	//	auto pTransportType = pTransport->GetTechnoType();
	//	double currentSpace = pTransportType->Passengers - pTransport->Passengers.GetTotalSize();
	//	if (currentSpace == 0) continue;
	//	for (auto pUnit : passengers)
	//	{
	//		auto pUnitType = pUnit->GetTechnoType();
	//		// Is legal loadable unit ?
	//		if (pUnitType->WhatAmI() != AbstractType::AircraftType &&
	//			!pUnit->InLimbo &&
	//			!pUnitType->ConsideredAircraft &&
	//			pUnit->Health > 0 &&
	//			pUnit != pTransport)
	//		{
	//			// Can unit load onto this car ?
	//			if (pUnitType->Size > 0
	//				&& pUnitType->Size <= pTransportType->SizeLimit
	//				&& pUnitType->Size <= currentSpace)
	//			{
	//				// Get on the car
	//				if (pUnit->GetCurrentMission() != Mission::Enter)
	//				{
	//					transportsNoSpace = false;
	//					pUnit->QueueMission(Mission::Enter, true);
	//					pUnit->SetTarget(nullptr);
	//					pUnit->SetDestination(pTransport, false);
	//					currentSpace -= pUnitType->Size;
	//					loadedUnits.AddItem(pUnit);
	//				}
	//			}
	//		}
	//		if (currentSpace == 0) break;
	//	}
	//	for (auto pRemove : loadedUnits) passengers.Remove(pRemove);
	//	loadedUnits.Clear();
	//}
	//if (transportsNoSpace)
	//{
	//	pTeam->StepCompleted = true;
	//	return;
	//}
}

bool ScriptExt::IsValidFriendlyTarget(TeamClass* pTeam, int group, TechnoClass* target, bool isSelfNaval, bool isSelfAircraft, bool isFriendly)
{
	if (!target) return false;
	if (TECHNO_IS_ALIVE(target) && target->Group == group)
	{
		auto pType = target->GetTechnoType();
		// Friendly?
		if (isFriendly ^ pTeam->Owner->IsAlliedWith(target->Owner))
			return false;
		// Only aircraft team can follow friendly aircraft
		if (isSelfAircraft)
			return true;
		else if (pType->ConsideredAircraft)
			return false;
		// If team is naval, only follow friendly naval
		if (isSelfNaval ^ pType->Naval)
			return false;
		// No underground
		if (target->InWhichLayer() == Layer::Underground)
			return false;

		return true;
	}
	return false;
}

void NOINLINE ScriptExt::FollowFriendlyByGroup(TeamClass* pTeam, int group)
{
	bool isSelfNaval = true, isSelfAircraft = true;
	double distMin = std::numeric_limits<double>::infinity();
	CellStruct* teamPosition = nullptr;
	TechnoClass* target = nullptr;

	// Use timer to reduce unnecessary cycle
	if (pTeam->GuardAreaTimer.TimeLeft <= 0)
	{
		// If all member is naval, will only follow friendly navals
		for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
		{
			if (!teamPosition)
				pMember->GetMapCoords(teamPosition);

			auto pMemberType = pMember->GetTechnoType();
			isSelfNaval = isSelfNaval && pMemberType->Naval;
			isSelfAircraft = isSelfAircraft && pMemberType->ConsideredAircraft;
		}
		// If previous target is valid, skip this check
		const auto dest = abstract_cast<TechnoClass*>(pTeam->Focus);
		if (IsValidFriendlyTarget(pTeam, group, dest, isSelfNaval, isSelfAircraft, true))
		{
			for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
			{
				double d = pMember->InlineMapCoords().DistanceFrom(dest->InlineMapCoords());
				if (d * 256 > RulesClass::Instance->CloseEnough)
				{
					if (isSelfAircraft)
					{
						pMember->SetDestination(dest, false);
						pMember->QueueMission(Mission::Move, true);
					}
					else
					{
						pMember->QueueMission(Mission::Area_Guard, true);
						pMember->SetTarget(nullptr);
						pMember->SetFocus(dest);
					}
				}
				else
				{
					if (!isSelfAircraft)
					{
						pMember->SetDestination(nullptr, false);
						pMember->CurrentMission = Mission::Area_Guard;
						pMember->Focus = nullptr;
					}
					else
						pMember->QueueMission(Mission::Area_Guard, true);
				}
			}
			pTeam->GuardAreaTimer.Start(30);
			return;
		}
		// Now looking for target
		for (int i = 0; i < TechnoClass::Array->Count; i++)
		{
			auto pTechno = TechnoClass::Array->GetItem(i);

			if (IsValidFriendlyTarget(pTeam, group, pTechno, isSelfNaval, isSelfAircraft, true))
			{
				// candidate
				if (!teamPosition)
				{
					auto pScript = pTeam->CurrentScript;
					pTeam->StepCompleted = true;
					auto const& nCur = pScript->GetCurrentAction();
					Debug::Log("AI Scripts - FollowFriendlyByGroup: [%s] [%s](line: %d = %d,%d) Attempting To derefence nullptr teamPosition ! )\n",
						pTeam->Type->ID,
						pScript->Type->ID,
						pScript->CurrentMission,
						nCur.Action,
						nCur.Argument);
					return;
				}

				double distance = pTechno->InlineMapCoords().DistanceFromSquared(*teamPosition);
				if (distance < distMin)
				{
					target = pTechno;
					distMin = distance;
				}
			}
		}

		if (target)
		{
			if (isSelfAircraft)
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					pMember->SetDestination(target, false);
					pMember->QueueMission(Mission::Move, true);
				}
			}
			else
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					pMember->QueueMission(Mission::Area_Guard, true);
					pMember->SetTarget(nullptr);
					pMember->SetFocus(target);
				}
			}
			pTeam->Focus = target;
			pTeam->GuardAreaTimer.Start(30);
		}
		// If there's no valid target, continue script
		else
			pTeam->StepCompleted = true;
	}
	else
		pTeam->GuardAreaTimer.TimeLeft--;
}

void ScriptExt::RallyUnitInMap(TeamClass* pTeam, int nArg)
{
	auto pType = pTeam->Type;
	for (int i = 0; i < FootClass::Array->Count; i++)
	{
		auto pFoot = FootClass::Array->GetItem(i);

		// Must be owner and with same group
		if (pFoot
			&& TECHNO_IS_ALIVE(pFoot)
			&& pFoot->Owner == pTeam->Owner
			&& pFoot->Group == pType->Group
			&& IsValidRallyTarget(pTeam, pFoot, nArg))
		{
			// This will bypass any recruiting restrictions, except group (this action's purpose)
			if (pType->Recruiter)
				pTeam->AddMember(pFoot, true);
			// Only rally unit's parent team set Recruitable = yes and Priority less than this team
			else if (pFoot->RecruitableB)
			{
				if (pFoot->Team)
				{
					if (pFoot->Team->Type->Priority < pType->Priority)
						pTeam->AddMember(pFoot, true);
				}
				else
					pTeam->AddMember(pFoot, true);
			}
		}
	}
	pTeam->StepCompleted = true;
}

bool ScriptExt::IsValidRallyTarget(TeamClass* pTeam, FootClass* pFoot, int nType)
{
	const auto type = VTable::Get(pFoot);
	auto pTechnoType = pFoot->GetTechnoType();
	TaskForceClass* pTaskforce = pTeam->Type->TaskForce;
	if (!pTechnoType)
		return false;

	switch (nType)
	{
	case 0: // Anything
		return true;
	case 1: // Infantry
		return type == InfantryClass::vtable;
	case 2: // Vehicles
		return type == UnitClass::vtable;
	case 3: // Air Units
		return pTechnoType->ConsideredAircraft || type == AircraftClass::vtable;
	case 4: // Naval units
		return type == UnitClass::vtable && pTechnoType->Naval;
	case 5: // Ground units
		return (type == UnitClass::vtable || type == InfantryClass::vtable)
			&& (!pTechnoType->ConsideredAircraft && !pTechnoType->Naval);
	case 6: // Dockable fighters
		return type == AircraftClass::vtable && static_cast<AircraftClass*>(pFoot)->Type->AirportBound;
	case 7: // Same type in taskforce
		if (pTaskforce)
		{
			for (auto const& pEntry : pTaskforce->Entries)
			{
				if (pEntry.Type && pEntry.Type == pTechnoType)
					return true;
			}
		}
		return false;
	default:
		return false;
	}
}
#pragma endregion

/*
	if (auto v28 = pUnit->Passengers.FirstPassenger)
		{
			do
			{
				pTeam->AddMember(v28, 0);
				v28 = static_cast<FootClass*>(v28->NextObject);
			}
			while (v28 && (v28->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None);
		}
*/

void NOINLINE ScriptExt::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine = false, int count = 0)
{
	bool bSucceeded = false;
	if (auto pTeamData = TeamExtContainer::Instance.Find(pTeam))
	{
		if (count <= 0)
			//PR #724
			count = 15 * pTeam->CurrentScript->GetCurrentAction().Argument;

		if (count > 0)
		{
			pTeamData->ForceJump_InitialCountdown = count;
			pTeamData->ForceJump_Countdown.Start(count);
			pTeamData->ForceJump_RepeatMode = repeatLine;
		}
		else
		{
			pTeamData->ForceJump_InitialCountdown = -1;
			pTeamData->ForceJump_Countdown.Stop();
			pTeamData->ForceJump_Countdown = -1;
			pTeamData->ForceJump_RepeatMode = false;
		}

		bSucceeded = true;

	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = bSucceeded;
	auto const& nCur = pScript->GetCurrentAction();
	Debug::Log("AI Scripts - Set_ForceJump_Countdown: [%s] [%s](line: %d = %d,%d) %s Set Timed Jump -> (Countdown: %d, repeat action: %d)\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		nCur.Action,
		nCur.Argument,
		bSucceeded ? "Done" : "Failed",
		count,
		repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_Countdown = -1;
	pTeamData->ForceJump_RepeatMode = false;
	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	auto const& nCur = pScript->GetCurrentAction();
	Debug::Log("AI Scripts - Stop_ForceJump_Countdown: [%s] [%s](line: %d = %d,%d): Done Stopped Timed Jump\n",
		pTeam->Type->ID,
		pScript->Type->ID,
		pScript->CurrentMission,
		nCur.Action,
		nCur.Argument);
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	std::vector<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pUnitType = pUnit->GetTechnoType();
		if (pUnitType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers)
		{
			transports.push_back(pUnit);
		}
	}

	if (transports.empty())
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Now load units into transports
	for (auto const& pTransport : transports)
	{
		//if (!TechnoExtData::IsActive(pTransport, true, true, false, true))
		//	continue;

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			//if (!TechnoExtData::IsActive(pUnit, true, true, false, true))
			//	continue;

			auto const pTransportType = pTransport->GetTechnoType();
			auto const pUnitType = pUnit->GetTechnoType();
			if (pTransport != pUnit
				&& (((DWORD*)pUnitType)[0]) != AircraftTypeClass::vtable
				&& !pUnitType->ConsideredAircraft
				&& !pUnit->InLimbo
				&& pUnit->Health > 0)
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					if (pTransport->IsInAir())
						return;

					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pTransportType->OpenTopped)
						{
							pUnit->EnteredOpenTopped(pTransport);
							pUnit->Transporter = pTransport;
						}

						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

						return;
					}
				}
			}
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;

	FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);
	TeamExtContainer::Instance.Find(pTeam)->TeamLeader = pLeaderUnit;

	pTeam->StepCompleted = true;
}

void NOINLINE ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		//if (!TechnoExtData::IsActive(pUnit, true, true, false, true))
		//	continue;

		if (!pUnit->InLimbo && pUnit->Health > 0 && !pUnit->Spawned && pUnit->Owner)
		{
			const auto pType = pUnit->GetTechnoType();
			if (pType->Ammo > 0 && pUnit->Ammo < pType->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (const auto pAircraft = specific_cast<AircraftClass*>(pUnit))
				{
					if (pAircraft->Type->AirportBound)
					{
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pAircraft->SetTarget(nullptr);
						pAircraft->LastTarget = nullptr;

						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pAircraft->IsInAir() && pAircraft->CurrentMission != Mission::Enter)
						{
							bool dock = false;
							if (auto const pContact = pAircraft->GetNthLink())
							{
								if (auto const pCell = pAircraft->GetCell())
								{
									if (auto pBld = pCell->GetBuilding())
									{
										if (pBld == pContact && pBld->Type->Helipad)
										{
											dock = true;
											pAircraft->QueueMission(Mission::Enter, true);
										}
									}
								}
							}

							if (!dock)
								pAircraft->EnterIdleMode(false, true);

						}

						return;
					}
				}
				else if (pType->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void NOINLINE ScriptExt::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown = -1)
{
	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->GetCurrentAction().Argument;
	bool gatherUnits = false;
	const auto pExt = TeamExtContainer::Instance.Find(pTeam);

	// Load countdown
	if (pExt->Countdown_RegroupAtLeader >= 0)
		countdown = pExt->Countdown_RegroupAtLeader;

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
		gatherUnits = true;

	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else if (countdown == 0) // Countdown ended
			countdown = -1;
		else // Start countdown.
		{
			countdown = initialCountdown * 15;
			gatherUnits = true;
		}

		// Save counter
		pExt->Countdown_RegroupAtLeader = countdown;
	}

	if (!gatherUnits)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
		int nTogether = 0;
		int nUnits = -1; // Leader counts here

		// Find the Leader
		pLeaderUnit = pExt->TeamLeader;
		if (!IsUnitAvailable(pLeaderUnit, true, true))
		{
			pLeaderUnit = FindTheTeamLeader(pTeam);
			pExt->TeamLeader = pLeaderUnit;
		}

		if (!pLeaderUnit)
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}

		double closeEnough = RulesClass::Instance->CloseEnough.ToCell();

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pExt->CloseEnough.has_value())
		{
			// This a one-time-use value
			closeEnough = pExt->CloseEnough.value();
		}

		// The leader should stay calm & be the group's center
		const bool AllowToStop = !TechnoExtData::IsInWarfactory(pLeaderUnit);

		if (pLeaderUnit->Locomotor.GetInterfacePtr()->Is_Moving_Now() && AllowToStop)
			pLeaderUnit->SetDestination(nullptr, false);

		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (IsUnitAvailable(pUnit, true, true))
			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				// Aircraft case
				if (Is_Aircraft(pUnit) && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = static_cast<AircraftClass*>(pUnit);

					if (pAircraft->Type->AirportBound)
					{
						// This aircraft won't count for the script action
						pUnit->EnterIdleMode(false, true);

						continue;
					}
				}

				nUnits++;

				if (TechnoExtData::IsInWarfactory(pUnit))
					continue;

				if (pUnit->DistanceFrom(pLeaderUnit) / 256.0 > closeEnough)
				{
					// Leader's location is too far from me. Regroup
					if (pUnit->Destination != pLeaderUnit)
					{
						pUnit->SetDestination(pLeaderUnit, false);
						pUnit->QueueMission(Mission::Move, false);
					}
				}
				else
				{
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}

		if (nUnits >= 0
			&& nUnits == nTogether
			&& (initialCountdown == 0
				|| (initialCountdown > 0
					&& countdown <= 0)))
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}
	}
}

std::pair<WeaponTypeClass*, WeaponTypeClass*> ScriptExt::GetWeapon(TechnoClass* pTechno)
{
	if (!pTechno)
		return { nullptr , nullptr };

	return { TechnoExtData::GetCurrentWeapon(pThis, false),TechnoExtData::GetCurrentWeapon(pThis, true) };
}

std::pair<bool, bool> ScriptExt::CheckWeaponsTargetingCapabilites(WeaponTypeClass* pWeaponPrimary, WeaponTypeClass* pWeaponSecondary, bool agentMode)
{
	bool hasAntiGround = false;
	bool hasAntiAir = false;

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AA) || (pWeaponSecondary && pWeaponSecondary->Projectile->AA))
		hasAntiAir = true;

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AG) || (pWeaponSecondary && pWeaponSecondary->Projectile->AG) || agentMode)
		hasAntiGround = true;

	return { hasAntiAir  ,hasAntiGround };
}

bool NOINLINE ScriptExt::IsUnitArmed(TechnoClass* pTechno)
{
	auto const pWeaponPrimary = TechnoExtData::GetCurrentWeapon(pTechno);
	auto const pWeaponSecondary = TechnoExtData::GetCurrentWeapon(pTechno, true);

	return pWeaponPrimary || pWeaponSecondary;
}

bool NOINLINE ScriptExt::IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno)
{
	return pHouse->IsAlliedWith(pTechno) && pTechno->IsMindControlled() && !pHouse->IsAlliedWith(pTechno->MindControlledBy);
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{
	if (!pTechno || !pTechno->Owner || !pTeamLeader || !pTeamLeader->Owner)
		return false;

	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);
	bool buildingIsConsideredVehicle = false;

	if (!pTargetTypeExt || pTargetTypeExt->IsDummy.Get())
		return false;

	if (const auto pBuilding = specific_cast<BuildingClass*>(pTechno))
	{
		if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1)
			return false;

		buildingIsConsideredVehicle = pBuilding->Type->IsUndeployable();
	}

	UnitTypeClass* pTypeUnit = pTechno->WhatAmI() == AbstractType::Unit ? static_cast<UnitTypeClass*>(pTechnoType) : nullptr;

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	auto const& nAITargetTypes = RulesExtData::Instance()->AITargetTypesLists;
	if (attackAITargetType >= 0 && !nAITargetTypes.empty() && attackAITargetType < (int)nAITargetTypes.size())
	{
		auto const& nVec = nAITargetTypes[attackAITargetType];
		return std::any_of(nVec.begin(), nVec.end(), [pTechnoType](TechnoTypeClass* pTech)
		{ return pTech == pTechnoType; });
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
				const auto pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);
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
					if (auto typeExt = BuildingTypeExtContainer::Instance.TryFind(const_cast<BuildingTypeClass*>(type)))
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
				if (const auto pFake = TechnoTypeExtContainer::Instance.Find(pTypeBuilding)->Fake_Of.Get(nullptr))
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
		auto pTypeTechnoExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

		return (!pTechno->Owner->IsNeutral() &&
			(pTypeTechnoExt && (pTypeTechnoExt->RadarJamRadius > 0)));
	}
	case 30:
	{
		// Inhibitor
		auto pTypeTechnoExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

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
		const auto pTypeBuilding = specific_cast<BuildingTypeClass*>(pTechnoType);
		const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pTypeBuilding);
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

				auto const pBtypeExt = BuildingTypeExtContainer::Instance.Find(pTypeBuilding);

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
			return TechnoTypeExtContainer::Instance.Find(pTechnoType)->IsHero.Get();
		}

		return false;
	}
	}

	return false;
}

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier <= 0)
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	const auto pTeamType = pTeam->Type;

	auto const Iter =
		std::find_if(AITriggerTypeClass::Array->begin(), AITriggerTypeClass::Array->end(),
			[pTeamType](AITriggerTypeClass* const pTrig)
 {
	 return (pTeamType && ((pTrig->Team1 && pTrig->Team1 == pTeamType) || (pTrig->Team2 && pTrig->Team2 == pTeamType)));
		});

	if (Iter != AITriggerTypeClass::Array->end())
	{
		AITriggerTypeClass* pTriggerType = *Iter;

		pTriggerType->Weight_Current += modifier;

		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum)
		{
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
		}
		else
		{
			if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
				pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
		}
	}
}

void ScriptExt::WaitIfNoTarget(TeamClass* pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (attempts <= 0)
		TeamExtContainer::Instance.Find(pTeam)->WaitNoTargetAttempts = -1; // Infinite waits if no target
	else
		TeamExtContainer::Instance.Find(pTeam)->WaitNoTargetAttempts = attempts;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass* pTeam, std::optional<double> award)
{
	double Award = 1.0 * pTeam->CurrentScript->GetCurrentAction().Argument;
	if (award.has_value())
		Award = award.value();

	if (award > 0.0)
		TeamExtContainer::Instance.Find(pTeam)->NextSuccessWeightAward = Award;

	// This action finished
	pTeam->StepCompleted = true;
	return;
}

void ScriptExt::ClearCurrentScript(TeamClass* pThis)
{
	pThis->CurrentScript = nullptr;
}

void ScriptExt::CreateNewCurrentScript(TeamClass* pThis, ScriptTypeClass* pNewType)
{
	bool bSucceeded = false;
	if (auto pNew = GameCreate<ScriptClass>(pNewType))
	{
		pThis->CurrentScript = std::move(pNew);
		bSucceeded = true;
	}

	auto const bStatus = bSucceeded ? "Succeeded" : "Failed";

	Debug::Log("AI Scripts - CreateNewCurrentScript: [%x] Team %s / %s , Owner [%s] , %s create new Script [%s / %s] ! \n",
		pThis,
		pThis->Type->ID,
		pThis->Type->Name,
		pThis->Owner->get_ID(),
		bStatus,
		pNewType->ID,
		pNewType->Name
	);
}

NOINLINE ScriptTypeClass* ScriptExt::GetFromAIScriptList(size_t nIdx)
{
	const auto& nBaseVec = RulesExtData::Instance()->AIScriptsLists;

	if (nBaseVec.empty() || nIdx < 0 || nIdx > nBaseVec.size())
	{
		Debug::Log("ScriptExt::GetFromAIScriptList Failed getting index [%d] of AIScriptsLists with vector size[%d] !\n", nIdx, nBaseVec.size());
		return nullptr;
	}

	Debug::Log("ScriptExt::GetFromAIScriptList Successfully getting index [%d] of AIScriptsLists !\n", nIdx);

	const auto& objectsList = nBaseVec[nIdx];

	if (objectsList.empty())
		return nullptr;

	Debug::Log("ScriptExt::GetFromAIScriptList Successfully getting ItemVector at [%d] of AIScriptsLists with size[%d] !\n", nIdx, objectsList.size());

	return objectsList[ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1)];
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
{
	if (idxScriptsList <= 0)
		idxScriptsList = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (ScriptTypeClass* pNewScript = GetFromAIScriptList(idxScriptsList))
	{
		Debug::Log("ScriptExt::PickRandomScript Successfully GetFromAIScriptList with index [%d] !\n", idxScriptsList);

		if (pNewScript->ActionsCount > 0)
		{
			//remember previous script
			TeamExtContainer::Instance.Find(pTeam)->PreviousScriptList.push_back(pTeam->CurrentScript);

			ClearCurrentScript(pTeam);
			CreateNewCurrentScript(pTeam, pNewScript);

			// Ready for jumping to the first line of the new script
			pTeam->CurrentScript->CurrentMission = -1;
			pTeam->StepCompleted = true;

			return;

		}
		else
		{
			pTeam->StepCompleted = true;
			Debug::Log("AI Scripts - PickRandomScript: [%s] Aborting Script change because [%s] has 0 Action scripts!\n",
				pTeam->Type->ID,
				pNewScript->ID);

			return;
		}
	}
	else
	{
		pTeam->StepCompleted = true;
		Debug::Log("AI Scripts - PickRandomScript: [%s] [%s] Failed to change the Team Script with a random one because the vector or the index is invalid [%d]!\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			idxScriptsList
		);
	}
}

void ScriptExt::Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	TeamExtContainer::Instance.Find(pTeam)->IdxSelectedObjectFromAIList.reset();

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	auto const& nList = RulesExtData::Instance()->AITargetTypesLists;
	if (!nList.empty() && attackAITargetType < (int)nList.size()
		&& !nList[attackAITargetType].empty())
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void NOINLINE ScriptExt::Mission_Move(TeamClass* pTeam, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	const auto pScript = pTeam->CurrentScript;

	if (!pScript)
	{
		return;
	}

	int scriptArgument = pScript->GetCurrentAction().Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	bool noWaitLoop = false;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;

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

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (Is_AircraftType(pUnitType)
				&& !pUnit->IsInAir()
				&& static_cast<AircraftTypeClass*>(pUnitType)->AirportBound
				&& pUnit->Ammo < pUnitType->Ammo)
			{
				bAircraftsWithoutAmmo = true;
			}
		}
	}

	// Find the Leader
	if (!IsUnitAvailable(pTeamData->TeamLeader, true, true))
	{
		pTeamData->TeamLeader = FindTheTeamLeader(pTeam);
	}

	if (!pTeamData->TeamLeader || bAircraftsWithoutAmmo)
	{
		pTeamData->IdxSelectedObjectFromAIList.reset();
		pTeamData->CloseEnough.reset();

		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		int targetMask = scriptArgument;
		selectedTarget = FindBestObject(pLeaderUnit, targetMask, calcThreatMode, pickAllies, attackAITargetType, idxAITargetTypeItem);

		if (selectedTarget)
		{
			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (IsUnitAvailable(pUnit, true, true))
				{
					auto pUnitType = pUnit->GetTechnoType();

					if (pUnitType)
					{
						pUnit->CurrentTargets.Clear();

						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_not_okay && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						pUnit->SetDestination(selectedTarget, false);
						auto const pWhat = GetVtableAddr(pUnit);

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pWhat == AircraftClass::vtable && pUnit->Ammo > 0 && !pUnit->IsInAir())
							pUnit->QueueMission(Mission::Move, false);

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pWhat != AircraftClass::vtable)
						{
							pUnit->QueueMission(Mission::Move, false);
							pUnit->ObjectClickedAction(Action::Move, selectedTarget, false);

							if (pUnit->GetCurrentMission() != Mission::Move)
								pUnit->Mission_Move();
						}
					}
				}
			}
		}
		else
		{
			pTeamData->IdxSelectedObjectFromAIList.reset();

			if (noWaitLoop)
			{
				pTeamData->CloseEnough.reset();
				pTeam->StepCompleted = true;
				return;
			}

			// No target was found with the specific criteria.

			if (pTeamData->WaitNoTargetTimer.Completed() && pTeamData->WaitNoTargetAttempts != 0)
			{

				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			pTeamData->CloseEnough.reset();
			pTeam->StepCompleted = true;
			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Move" mission in each team unit
		bool bForceNextAction = ScriptExt::MoveMissionEndStatus(pTeam, pFocus, pTeamData->MoveMissionEndMode);

		if (bForceNextAction)
		{
			pTeamData->MoveMissionEndMode = MoveMissionEndModes::TeamLeaderInRange;
			pTeamData->IdxSelectedObjectFromAIList.reset();

			pTeamData->CloseEnough.reset();

			// This action finished
			pTeam->StepCompleted = true;
			const auto& nCur = pScript->GetCurrentAction();
			const auto& nNext = pScript->GetNextAction();

			Debug::Log("AI Scripts - Mission_Move: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Reached destination)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				nCur.Action,
				nCur.Argument,
				pScript->CurrentMission + 1,
				nNext.Action,
				nNext.Argument);

			return;
		}
	}
}

TechnoClass* ScriptExt::FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	HouseClass* enemyHouse = nullptr;
	auto pTechnoType = pTechno->GetTechnoType();

	if (!pTechno->Owner || !pTechnoType)
	{
		Debug::Log("AI Scripts - FindBestObject:  Error Occured ! \n");
		return nullptr;
	}

	// Favorite Enemy House case. If set, AI will focus against that House
	if (!pickAllies && pTechno->BelongsToATeam())
	{
		if (auto pFoot = abstract_cast<FootClass*>(pTechno))
		{
			if (pFoot->Team && pFoot->Team->FirstUnit->Owner)
			{
				int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;

				bool onlyTargetHouseEnemy = pFoot->Team->Type->OnlyTargetHouseEnemy;
				auto pHouseExt = HouseExtContainer::Instance.Find(pFoot->Team->Owner);

				if (pHouseExt && pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
					onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

				if (onlyTargetHouseEnemy && enemyHouseIndex >= 0)
				{
					enemyHouse = HouseClass::Array->GetItem(enemyHouseIndex);
				}
			}
		}
	}

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		const auto object = TechnoClass::Array->GetItem(i);

		if (!IsUnitAvailable(object, true, false) || object == pTechno || !object->Owner)
			continue;

		const auto objectType = object->GetTechnoType();

		if (!objectType)
			continue;

		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(objectType);

		if (pTypeExt->IsDummy)
			continue;

		if (const auto pBld = specific_cast<BuildingClass*>(object))
		{
			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				continue;
		}

		if (enemyHouse && !enemyHouse->Defeated && enemyHouse != object->Owner)
			continue;

		if (IsUnitAvailable(object, true, false)
			&& ((pickAllies && pTechno->Owner->IsAlliedWith(object))
				|| (!pickAllies && !pTechno->Owner->IsAlliedWith(object))))
		{

			// Don't pick underground units
			if (object->InWhichLayer() == Layer::Underground)
				continue;

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
				&& object->GetCell()->LandType != LandType::Water)
			{
				continue;
			}

			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						objectThreatValue += objectType->SpecialThreatValue * RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
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

void ScriptExt::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pScript = pTeam->CurrentScript;
	if (!pScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		return;
	}

	if (pTeamData->IdxSelectedObjectFromAIList.has_value())
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList.value();
	}
	else
	{

		if (attackAITargetType < 0)
			attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

		auto const& ObjList = RulesExtData::Instance()->AITargetTypesLists;

		if (attackAITargetType >= 0 && !ObjList.empty() && attackAITargetType < (int)ObjList.size())
		{
			auto const objectsListIter = make_iterator(ObjList[(attackAITargetType)]);

			if (!objectsListIter.empty())
			{
				// Finding the objects from the list that actually exists in the map

				auto const objectFromIter = std::find_if(TechnoClass::Array->begin(),
					TechnoClass::Array->end(), [objectsListIter, pTeam](TechnoClass* objectFromList)
						{
							if (TechnoExtData::IsActive(objectFromList, false, false, false, true) && objectsListIter.contains(objectFromList->GetTechnoType()))
							{
								if (pTeam->FirstUnit->Owner)
								{
									if (!objectFromList->IsMindControlled())
										return !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList)
										|| pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList);
									else
										return !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList->MindControlledBy) ||
										pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList->MindControlledBy);
								}
								else
									return true;
							}

							return false;

				});

				if (objectFromIter != TechnoClass::Array->end() && (*objectFromIter))
				{
					auto const nObjListIter = std::find_if(objectsListIter.begin(), objectsListIter.end(),
						[objectFromIter](TechnoTypeClass* pTech)
	 {
		 return pTech == (*objectFromIter)->GetTechnoType();
						});

					if (nObjListIter != objectsListIter.end())
						validIndexes.push_back(std::distance(objectsListIter.begin(), nObjListIter));
				}

				if (!validIndexes.empty())
				{
					const int res = validIndexes[(ScenarioClass::Instance->Random.RandomFromMax(validIndexes.size() - 1))];
					pTeamData->IdxSelectedObjectFromAIList = res;
					idxSelectedObject = res;

					const auto& nCur = pScript->GetCurrentAction();
					Debug::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
						pTeam->Type->ID,
						pTeam->CurrentScript->Type->ID,
						pScript->CurrentMission,
						nCur.Action,
						nCur.Argument,
						attackAITargetType,
						idxSelectedObject,
						objectsListIter[(idxSelectedObject)]->ID);
				}
			}
		}
	}

	if (idxSelectedObject >= 0)
		Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
	else
	{
		pTeamData->IdxSelectedObjectFromAIList.reset();
		pTeam->StepCompleted = true;
		const auto& nCur = pScript->GetCurrentAction();
		Debug::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pScript->CurrentMission,
			nCur.Action,
			nCur.Argument,
			attackAITargetType,
			validIndexes.size());
	}
}

void ScriptExt::Mission_Move_List(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType)
{
	TeamExtContainer::Instance.Find(pTeam)->IdxSelectedObjectFromAIList.reset();

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

	auto const& nList = RulesExtData::Instance()->AITargetTypesLists;

	if (!nList.empty() && attackAITargetType < (int)nList.size()
		&& !nList[attackAITargetType].empty())
	{
		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move_List1Random(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	if (!pScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (pTeamData->IdxSelectedObjectFromAIList.has_value())
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList.value();
	}
	else
	{

		if (attackAITargetType < 0)
			attackAITargetType = pTeam->CurrentScript->GetCurrentAction().Argument;

		auto const& AITypeLists = RulesExtData::Instance()->AITargetTypesLists;

		if (attackAITargetType >= 0
			&& !AITypeLists.empty() && attackAITargetType < (int)AITypeLists.size())
		{
			auto const objectsListIter = make_iterator(RulesExtData::Instance()->AITargetTypesLists[(attackAITargetType)]);

			// Still no random target selected
			if (!objectsListIter.empty())
			{
				// Finding the objects from the list that actually exists in the map
				auto const objectFromIter = std::find_if(
					TechnoClass::Array->begin(),
					TechnoClass::Array->end(), [objectsListIter, pTeam, pickAllies](TechnoClass* objectFromList)
				{
					if (objectFromList && (objectsListIter.contains(objectFromList->GetTechnoType())
						&& TechnoExtData::IsActive(objectFromList, false, false, false, true)))
					{
						if (pTeam->FirstUnit->Owner)
						{
							return (pickAllies && pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList)) || (!pickAllies
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(objectFromList));
						}
						else
							return !pickAllies;
					}

					return false;
				});

				if (objectFromIter != TechnoClass::Array->end())
				{
					auto const nITerHere = std::find_if(objectsListIter.begin(), objectsListIter.end(),
					[objectFromIter](TechnoTypeClass* pTech)
					 { return pTech == (*objectFromIter)->GetTechnoType(); });

					if (nITerHere != objectsListIter.end())
						validIndexes.push_back(std::distance(objectsListIter.begin(), nITerHere));
				}

				if (!validIndexes.empty())
				{
					const int res = validIndexes[(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.size() - 1))];
					pTeamData->IdxSelectedObjectFromAIList = res;
					idxSelectedObject = res;
					const auto& nCur = pScript->GetCurrentAction();
					Debug::Log("AI Scripts - Mission_Move_List1Random: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
						pTeam->Type->ID,
						pTeam->CurrentScript->Type->ID,
						pScript->CurrentMission,
						nCur.Action,
						nCur.Argument,
						attackAITargetType,
						idxSelectedObject,
						objectsListIter[(idxSelectedObject)]->ID);
				}
			}
		}
	}

	if (idxSelectedObject >= 0)
		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, idxSelectedObject);
	else
		// This action finished
	{
		pTeamData->IdxSelectedObjectFromAIList.reset();
		pTeam->StepCompleted = true;
		const auto& nCur = pScript->GetCurrentAction();
		Debug::Log("AI Scripts - Mission_Move_List1Random: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pScript->CurrentMission,
			nCur.Action,
			nCur.Argument,
			attackAITargetType,
			validIndexes.size());
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass* pTeam, std::optional<double> closeDistance)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (closeDistance.has_value())
	{
		const int distance = (int)closeDistance.value();

		if (distance >= 0)
			TeamExtContainer::Instance.Find(pTeam)->CloseEnough = distance;
	}
	else
	{
		if (pTeam->CurrentScript->GetCurrentAction().Argument > 0)
			TeamExtContainer::Instance.Find(pTeam)->CloseEnough = pTeam->CurrentScript->GetCurrentAction().Argument;
		else
			TeamExtContainer::Instance.Find(pTeam)->CloseEnough = RulesClass::Instance->CloseEnough.ToCell();
	}

	// This action finished
	pTeam->StepCompleted = true;
	return;
}

void ScriptExt::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, MoveMissionEndModes mode)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode != MoveMissionEndModes::Fetch)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;

	}
	else
	{
		const auto args = pTeam->CurrentScript->GetCurrentAction().Argument;

		if (args <= 0 || args > (int)MoveMissionEndModes::count)
		{
			// This action finished
			pTeam->StepCompleted = true;
			return;
		}

		TeamExtContainer::Instance.Find(pTeam)->MoveMissionEndMode = (MoveMissionEndModes)args;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, MoveMissionEndModes mode = 0)
{
	if (!pTeam || !pFocus || mode == MoveMissionEndModes::Fetch)
		return false;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData->TeamLeader)
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough.ToCell();

	if (pTeamData->CloseEnough.has_value())
		closeEnough = pTeamData->CloseEnough.value();

	bool bForceNextAction = mode == MoveMissionEndModes::AllMemberInRange;

	// Team already have a focused target

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (TechnoExtData::IsActive(pUnit, true, true, false, true))
		{
			auto const pWhat = GetVtableAddr(pUnit);

			if (!pUnit->Locomotor.GetInterfacePtr()->Is_Moving_Now())
				pUnit->SetDestination(pFocus, false);

			if (mode == MoveMissionEndModes::AllMemberInRange)
			{
				// Default mode: all members in range
				if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
				{
					bForceNextAction = false;

					if (pWhat == AircraftClass::vtable && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

					continue;
				}
				else
				{
					if (pWhat == AircraftClass::vtable && pUnit->Ammo <= 0)
					{
						pUnit->EnterIdleMode(false, 1);
						pUnit->Team->RemoveMember(pUnit);
						continue;
					}
				}
			}
			else
			{
				if (mode == MoveMissionEndModes::AnyMemberInRange)
				{
					// Any member in range
					if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
					{
						if (pWhat == AircraftClass::vtable && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, false);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pWhat == AircraftClass::vtable && pUnit->Ammo <= 0)
						{
							pUnit->EnterIdleMode(false, 1);
							pUnit->Team->RemoveMember(pUnit);
							continue;
						}
					}
				}
				else
				{
					// All other cases: Team Leader mode in range
					if (pLeader)
					{
						if (pUnit->DistanceFrom(pUnit->Destination) / 256.0 > closeEnough)
						{
							if (pWhat == AircraftClass::vtable && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, false);

							continue;
						}
						else
						{
							if (pUnit->IsTeamLeader)
								bForceNextAction = true;

							if (pWhat == AircraftClass::vtable && pUnit->Ammo <= 0)
							{
								pUnit->EnterIdleMode(false, 1);
								pUnit->Team->RemoveMember(pUnit);
								continue;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
		else
		{
			pUnit->Team->RemoveMember(pUnit);
			continue;
		}
	}

	return bForceNextAction;
}

void ScriptExt::SkipNextAction(TeamClass* pTeam, int successPercentage = 0)
{
	// This team has no units! END
	const auto pTeamExt = TeamExtContainer::Instance.Find(pTeam);

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	if (ScenarioClass::Instance->Random.PercentChance(successPercentage))
	{
		const auto& specific = ScriptExt::GetSpecificAction(pTeam->CurrentScript, 2);
		Debug::Log("AI Scripts - SkipNextAction: ScripType: [%s] [%s] (line: %d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->CurrentMission + 2,
			specific.Action,
			specific.Argument);

		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
{
	struct operation_set { int operator()(const int& a, const int& b) { return b; } };
	struct operation_add { int operator()(const int& a, const int& b) { return a + b; } };
	struct operation_minus { int operator()(const int& a, const int& b) { return a - b; } };
	struct operation_multiply { int operator()(const int& a, const int& b) { return a * b; } };
	struct operation_divide { int operator()(const int& a, const int& b) { return a / b; } };
	struct operation_mod { int operator()(const int& a, const int& b) { return a % b; } };
	struct operation_leftshift { int operator()(const int& a, const int& b) { return a << b; } };
	struct operation_rightshift { int operator()(const int& a, const int& b) { return a >> b; } };
	struct operation_reverse { int operator()(const int& a, const int& b) { return ~a; } };
	struct operation_xor { int operator()(const int& a, const int& b) { return a ^ b; } };
	struct operation_or { int operator()(const int& a, const int& b) { return a | b; } };
	struct operation_and { int operator()(const int& a, const int& b) { return a & b; } };

	int nLoArg = LOWORD(nArg);
	int nHiArg = HIWORD(nArg);

	switch (eAction)
	{
	case PhobosScripts::LocalVariableSet:
		VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	}
}

template<bool IsGlobal, class _Pr>
void ScriptExt::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	const auto nVar = ScenarioExt::GetVariables(IsGlobal);
	auto itr = nVar->find(nVariable);

	if (itr != nVar->end())
	{
		itr->second.Value = _Pr()(itr->second.Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}
	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExt::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	auto const nVar = ScenarioExt::GetVariables(IsGlobal);
	auto const itr = nVar->find(nVarToOperate);

	if (itr != nVar->end())
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

NOINLINE FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;
	bool teamLeaderFound = false;

	if (!pTeam)
		return pLeaderUnit;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit)
			continue;

		bool isValidUnit = IsUnitAvailable(pUnit, true, true);

		// Preventing >1 leaders in teams
		if (teamLeaderFound || !isValidUnit)
		{
			pUnit->IsTeamLeader = false;
			continue;
		}

		if (pUnit->IsTeamLeader)
		{
			pLeaderUnit = pUnit;
			teamLeaderFound = true;

			continue;
		}

		// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
		int unitLeadershipRating = pUnit->GetTechnoType()->LeadershipRating;

		if (unitLeadershipRating > bestUnitLeadershipValue)
		{
			pLeaderUnit = pUnit;
			bestUnitLeadershipValue = unitLeadershipRating;
		}
	}

	if (pLeaderUnit)
		pLeaderUnit->IsTeamLeader = true;

	return pLeaderUnit;
}


bool ScriptExt::IsExtVariableAction(int action)
{
	const auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (mode < 0 || mode > 2)
		mode = ScriptExt::GetSpecificAction(pTeam->CurrentScript, pTeam->CurrentScript->CurrentMission).Argument;

	if (mode < -1 || mode > 2)
		mode = -1;

	HouseExtData::ForceOnlyTargetHouseEnemy(pTeam->Owner, mode);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ResetAngerAgainstHouses(TeamClass* pTeam)
{
	// Invalid team
	if (!pTeam)
	{
		return;
	}

	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		angerNode.AngerLevel = 0;
	}

	pTeam->Owner->EnemyHouseIndex = -1;

	// This action finished
	pTeam->StepCompleted = true; // This action finished - FS-21
}

void ScriptExt::SetHouseAngerModifier(TeamClass* pTeam, int modifier = 0)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (modifier <= 0)
		modifier = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (modifier < 0)
		modifier = 0;

	pTeamData->AngerNodeModifier = modifier;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	bool changeFailed = true;

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (idxHousesList >= 0)
	{
		auto const& nHouseList = RulesExtData::Instance()->AIHateHousesLists;
		if (!nHouseList.empty() && idxHousesList < (int)nHouseList.size())
		{
			for (auto& angerNode : pTeam->Owner->AngerNodes)
			{
				if (!angerNode.House->Type)
					continue;

				for (const auto& pData : nHouseList[(idxHousesList)])
				{
					if (angerNode.House->Type == pData)
					{
						angerNode.AngerLevel += pTeamData->AngerNodeModifier;
						changeFailed = false;
					}
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
		Debug::Log(" __FUNCTION__ : [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			nCurAction.Action,
			nCurAction.Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	int changes = 0;

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (idxHousesList >= 0)
	{
		auto const& nHouseList = RulesExtData::Instance()->AIHateHousesLists;

		if (!nHouseList.empty() && idxHousesList < (int)nHouseList.size())
		{
			auto const& objectsList = nHouseList[(idxHousesList)];

			if (!objectsList.empty())
			{
				const int IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

				for (auto& angerNode : pTeam->Owner->AngerNodes)
				{
					if (angerNode.House->Defeated || !angerNode.House->Type)
						continue;

					if (angerNode.House->Type == objectsList[IdxSelectedObject])
					{
						angerNode.AngerLevel += pTeamData->AngerNodeModifier;
						changes++;
					}
				}
			}
		}
	}

	// This action finished
	if (changes == 0)
	{
		pTeam->StepCompleted = true;
		auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
		Debug::Log("__FUNCTION__ : [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			nCurAction.Action,
			nCurAction.Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1, bool random = false)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mask == 0)
		mask = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	std::vector<HouseClass*> objectsList;
	int IdxSelectedObject = -1;
	HouseClass* selectedHouse = nullptr;
	int highestHateLevel = 0;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Find the highest House hate value
	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (pTeam->Owner == angerNode.House
			|| angerNode.House->Defeated
			|| pTeam->Owner->IsAlliedWith(angerNode.House)
			|| angerNode.House->Type->MultiplayPassive)
		{
			continue;
		}

		if (random)
		{
			objectsList.push_back(angerNode.House);
		}
		else
		{
			if (angerNode.AngerLevel > highestHateLevel)
				highestHateLevel = angerNode.AngerLevel;
		}
	}

	newHateLevel += highestHateLevel;

	// Pick a enemy house
	if (random)
	{
		if (!objectsList.empty())
		{
			IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);
			selectedHouse = objectsList[(IdxSelectedObject)];
		}
	}
	else
	{
		selectedHouse = GetTheMostHatedHouse(pTeam, mask, mode);
	}

	if (selectedHouse)
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->Defeated)
				continue;

			if (angerNode.House == selectedHouse)
			{
				angerNode.AngerLevel = newHateLevel;
				auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
				Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): Picked a new house as enemy [%s]\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pTeam->CurrentScript->CurrentMission,
					nCurAction.Action,
					nCurAction.Argument,
					angerNode.House->Type->ID);
			}
		}

		ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	}
	else
	{
		auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
		Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			nCurAction.Action,
			nCurAction.Argument);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

HouseClass* ScriptExt::GetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1)
{

	if (!pTeam)
		return nullptr;

	// Note regarding "mode": 1 is used for ">" comparisons and 0 for "<"
	if (mode <= 0)
		mode = 0;
	else
		mode = 1;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;

	if (!pTeamData || mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	// Find the Team Leader
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!TechnoExtData::IsAlive(pUnit))
			continue;

		if (TechnoExtData::IsInWarfactory(pUnit))
			continue;

		if (const auto pUnitType = pUnit->GetTechnoType())
		{
			// The team leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
			const int unitLeadershipRating = pUnitType->LeadershipRating;
			if (unitLeadershipRating > bestUnitLeadershipValue)
			{
				pLeaderUnit = pUnit;
				bestUnitLeadershipValue = unitLeadershipRating;
			}
		}
	}

	if (!pLeaderUnit)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	double objectDistance = -1;
	double enemyDistance = -1;
	double enemyThreatValue[8] = { 0 };
	HouseClass* enemyHouse = nullptr;
	long houseMoney = -1;
	int enemyPower = -1000000000;
	int enemyKills = -1;
	int enemyAirDocks = -1;
	int enemyStructures = -1;
	int enemyNavalUnits = -1;

	if (mask == -2)
	{
		// Based on House economy
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mode == 0)
			{
				// The poorest is selected
				if (pHouse->Available_Money() < houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The richest is selected
				if (pHouse->Available_Money() > houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): Selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -3)
	{
		// Based on Human Controlled check
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| !pHouse->IsControlledByHuman()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			const CoordStruct houseLocation { pHouse->BaseSpawnCell.X  , pHouse->BaseSpawnCell.Y , 0 };
			objectDistance = pLeaderUnit->Location.DistanceFrom(houseLocation); // Note: distance is in leptons (*256)

			if (mode == 0)
			{
				// mode 0: Based in NEAREST human enemy unit
				if (objectDistance < enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Based in FARTHEST human enemy unit
				if (objectDistance > enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -4 || mask == -5 || mask == -6)
	{


		// House power check
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int checkedHousePower = 0;

			if (mask == -4)
				checkedHousePower = pHouse->Power_Drain();

			if (mask == -5)
				checkedHousePower = pHouse->PowerOutput;

			if (mask == -6)
				checkedHousePower = pHouse->PowerOutput - pHouse->Power_Drain();

			if (mode == 0)
			{
				// mode 0: Selection based in lower value power in house
				if ((checkedHousePower < enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Selection based in higher value power in house
				if ((checkedHousePower > enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -7)
	{
		// Based on House kills
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentKills = pHouse->TotalKilledUnits + pHouse->TotalKilledUnits;

			if (mode == 0)
			{
				// The pacifist is selected
				if (currentKills < enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The major killer is selected
				if (currentKills > enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -8)
	{
		// Based on number of House naval units
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentNavalUnits = 0;

			for (auto& pUnit : *TechnoClass::Array)
			{
				if (!TechnoExtData::IsAlive(pUnit))
					continue;

				if (ScriptExt::EvaluateObjectWithMask(pUnit, 31, -1, -1, nullptr))
				{
					currentNavalUnits++;
				}
			}

			if (mode == 0)
			{
				// The House with less naval units is selected
				if (currentNavalUnits < enemyNavalUnits || enemyNavalUnits < 0)
				{
					enemyNavalUnits = currentNavalUnits;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more naval units is selected
				if (currentNavalUnits > enemyNavalUnits || enemyNavalUnits < 0)
				{
					enemyNavalUnits = currentNavalUnits;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -9)
	{
		// Based on number of House aircraft docks
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentAirDocks = pHouse->AirportDocks;

			if (mode == 0)
			{
				// The House with less Aircraft docks is selected
				if (currentAirDocks < enemyAirDocks || enemyAirDocks < 0)
				{
					enemyAirDocks = currentAirDocks;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more Aircraft docks is selected
				if (currentAirDocks > enemyAirDocks || enemyAirDocks < 0)
				{
					enemyAirDocks = currentAirDocks;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -10)
	{
		// Based on number of House factories (except aircraft factories)
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentFactories = pHouse->NumWarFactories + pHouse->NumConYards + pHouse->NumShipyards + pHouse->NumBarracks;

			if (mode == 0)
			{
				// The House with less factories is selected
				if (currentFactories < enemyStructures || enemyStructures < 0)
				{
					enemyStructures = currentFactories;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more factories is selected
				if (currentFactories > enemyStructures || enemyStructures < 0)
				{
					enemyStructures = currentFactories;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
		{
			auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
			Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				nCurAction.Action,
				nCurAction.Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	// Depending the mode check what house will be selected as the most hated
	for (auto pTechno : *TechnoClass::Array)
	{
		if (!TechnoExtData::IsAlive(pTechno))
			continue;

		if (!pTechno->Owner)
			continue;

		if (!pTechno->Owner->Defeated
			&& pTechno->Owner != pTeam->Owner
			&& !pTechno->Owner->IsAlliedWith(pTeam->Owner)
			&& !pTechno->Owner->Type->MultiplayPassive)
		{
			if (mask < 0)
			{
				if (mask == -1)
				{
					// mask -1: Based on object distances
					objectDistance = pLeaderUnit->DistanceFrom(pTechno); // Note: distance is in leptons (*256)

					if (mode == 0)
					{
						// mode 0: Based in NEAREST enemy unit
						if (objectDistance < enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
					else
					{
						// mode 1: Based in FARTHEST enemy unit
						if (objectDistance > enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
				}
			}
			else
			{
				// mask > 0 : Threat based on the new types in the new attack actions
				if (ScriptExt::EvaluateObjectWithMask(pTechno, mask, -1, -1, pLeaderUnit))
				{
					auto pTechnoType = pTechno->GetTechnoType();

					if (pTechnoType)
					{
						enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->ThreatPosed;

						if (pTechnoType->SpecialThreatValue > 0)
						{
							enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->SpecialThreatValue * RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						}
					}
				}
			}
		}
	}

	if (mask > 0)
	{
		double value = -1;

		for (int i = 0; i < 8; i++)
		{
			if (mode == 0)
			{
				// Select House with LESS threat
				if ((enemyThreatValue[i] < value || value == -1) && !HouseClass::Array->GetItem(i)->Defeated)
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
			else
			{
				// Select House with MORE threat
				if ((enemyThreatValue[i] > value || value == -1) && !HouseClass::Array->GetItem(i)->Defeated)
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
		}
	}

	if (enemyHouse)
	{
		auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
		Debug::Log("__FUNCTION__: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			nCurAction.Action,
			nCurAction.Argument,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex);
	}

	return enemyHouse;
}

void ScriptExt::OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (mode < -1 || mode > 2)
		mode = -1;

	pTeamData->OnlyTargetHouseEnemyMode = mode;
	/*
	Modes:
		0  -> Force "False"
		1  -> Force "True"
		2  -> Force "Random boolean"
		-1 -> Use default value in OnlyTargetHouseEnemy tag
		Note: only works for new Actions, not vanilla YR actions
	*/
	switch (mode)
	{
	case 0:
		pTeamData->OnlyTargetHouseEnemy = false;
		break;

	case 1:
		pTeamData->OnlyTargetHouseEnemy = true;
		break;

	case 2:
		pTeamData->OnlyTargetHouseEnemy = ScenarioClass::Instance->Random.RandomBool();
		break;

	default:
		pTeamData->OnlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;
		pTeamData->OnlyTargetHouseEnemyMode = -1;
		break;
	}

	auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
	Debug::Log("OverrideOnlyTargetHouseEnemy : [%s] [%s] (line: %d = %d,%d): New Team -> OnlyTargetHouseEnemy value: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		nCurAction.Action,
		nCurAction.Argument,
		pTeamData->OnlyTargetHouseEnemy);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHouse < 0)
		idxHouse = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (idxHouse < 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->ArrayIndex == idxHouse && !angerNode.House->Defeated)
			{
				angerNode.AngerLevel += pTeamData->AngerNodeModifier;
				auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Modified anger level against [%s](index: %d) with value: %d\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pTeam->CurrentScript->CurrentMission,
					nCurAction.Action,
					nCurAction.Argument,
					angerNode.House->Type->ID,
					angerNode.House->ArrayIndex,
					angerNode.AngerLevel);
			}
		}
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);

	// This action finished
	pTeam->StepCompleted = true;
}

// The selected house will become the most hated of the map (the effects are only visible if the other houses are enemy of the selected house)
void ScriptExt::AggroHouse(TeamClass* pTeam, int index = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	std::vector<HouseClass*> objectsList;
	HouseClass* selectedHouse = nullptr;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Store the list of playable houses for later
	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (!angerNode.House->Defeated && !angerNode.House->Type->MultiplayPassive && !angerNode.House->IsObserver())
		{
			objectsList.push_back(angerNode.House);
		}
	}

	// Include the own House if we are looking for ANY Human player
	if (index == -3)
	{
		if (!pTeam->Owner->Defeated
			&& !pTeam->Owner->Type->MultiplayPassive
			&& !pTeam->Owner->IsObserver()
			&& !pTeam->Owner->IsControlledByHuman())
		{
			objectsList.push_back(pTeam->Owner);
		}
	}

	// Positive indexes are specific house indexes. -1 is translated as "pick 1 random" & -2 is the owner of the Team executing the script action
	if (!objectsList.empty())
	{
		if (index < 0)
		{
			if (index == -1)
				index = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

			if (index == -2)
				index = pTeam->Owner->ArrayIndex;
		}
	}
	else
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Note: at most each "For" lasts 10 loops: 8 players + Civilian + Special houses
	if (index != -3)
	{
		for (auto& pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated && pHouse->ArrayIndex == index)
				selectedHouse = pHouse;
		}
	}

	if (selectedHouse || index == -3)
	{
		// For each playable house set the selected house as the one with highest hate value;
		for (const auto& pHouse : objectsList)
		{
			int highestHateLevel = 0;

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (angerNode.AngerLevel > highestHateLevel)
					highestHateLevel = angerNode.AngerLevel;
			}

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (index == -3)
				{
					if (angerNode.House->IsControlledByHuman())
					{
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
					}
				}
				else
				{
					if (selectedHouse == angerNode.House)
					{
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
					}
				}
			}

			ScriptExt::UpdateEnemyHouseIndex(pHouse);
		}
	}
	else
	{
		auto const& nCurAction = pTeam->CurrentScript->GetCurrentAction();
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house with index: %d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			nCurAction.Action,
			nCurAction.Argument,
			index);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::UpdateEnemyHouseIndex(HouseClass* pHouse)
{
	int angerLevel = 0;
	int index = -1;

	for (auto& angerNode : pHouse->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !pHouse->IsAlliedWith(angerNode.House)
			&& angerNode.AngerLevel > angerLevel)
		{
			angerLevel = angerNode.AngerLevel;
			index = angerNode.House->ArrayIndex;
		}
	}

	pHouse->EnemyHouseIndex = index;
}

// 1-based like the original action '6,n' (so the first script line is n=1)
void ScriptExt::ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = newScriptLine;

	if (scriptArgument < 1)
		scriptArgument = pScript->GetCurrentAction().Argument;

	// if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	if (scriptArgument < 1)
		scriptArgument = 1;

	if (pTeamData->ConditionalJump_Evaluation)
	{
		auto const& nAcurAct = pScript->GetCurrentAction();
		auto const& nAPrevAct = ScriptExt::GetSpecificAction(pScript, (scriptArgument - 1));
		Debug::Log("ConditionalJumpIfTrue : [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nAcurAct.Action,
			nAcurAct.Argument,
			scriptArgument - 1,
			nAPrevAct.Action,
			nAPrevAct.Argument);

		// Start conditional jump!
		// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
		pScript->CurrentMission = scriptArgument - 2;

		// Cleaning Conditional Jump related variables
		if (pTeamData->ConditionalJump_ResetVariablesIfJump)
			ScriptExt::ConditionalJump_ResetVariables(pTeam);
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// 1-based like the original action '6,n' (so the first script line is n=1)
void ScriptExt::ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = newScriptLine;

	if (scriptArgument < 1)
		scriptArgument = pScript->GetCurrentAction().Argument;

	// if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	if (scriptArgument < 1)
		scriptArgument = 1;

	if (!pTeamData->ConditionalJump_Evaluation)
	{
		auto const& nAcurAct = pScript->GetCurrentAction();
		auto const& nAPrevAct = ScriptExt::GetSpecificAction(pScript, (scriptArgument - 1));
		Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nAcurAct.Action,
			nAcurAct.Argument,
			scriptArgument - 1,
			nAPrevAct.Action,
			nAPrevAct.Argument);

		// Start conditional jump!
		// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
		pScript->CurrentMission = scriptArgument - 2;

		// Cleaning Conditional Jump related variables
		if (pTeamData->ConditionalJump_ResetVariablesIfJump)
			ScriptExt::ConditionalJump_ResetVariables(pTeam);
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_KillEvaluation(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (!pTeamData->ConditionalJump_EnabledKillsCount)
		return;

	if (pTeamData->ConditionalJump_Counter < 0)
		pTeamData->ConditionalJump_Counter = 0;

	int counter = pTeamData->ConditionalJump_Counter;
	int comparator = pTeamData->ConditionalJump_ComparatorValue;
	pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, counter, comparator);

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ManageKillsCounter(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = enable;

	if (scriptArgument < 0 || scriptArgument > 1)
		scriptArgument = pScript->GetCurrentAction().Argument;

	if (scriptArgument <= 0)
		scriptArgument = 0;
	else
		scriptArgument = 1;

	if (scriptArgument <= 0)
		pTeamData->ConditionalJump_EnabledKillsCount = false;
	else
		pTeamData->ConditionalJump_EnabledKillsCount = true;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetIndex(TeamClass* pTeam, int index = -1000000)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = index;

	if (scriptArgument == -1000000)
		scriptArgument = pScript->GetCurrentAction().Argument;

	//	if (scriptArgument < 0)
	//		scriptArgument = -1;

	pTeamData->ConditionalJump_Index = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetComparatorValue(TeamClass* pTeam, int value = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = value;

	if (scriptArgument < 0)
		scriptArgument = pScript->GetCurrentAction().Argument;

	//	if (scriptArgument < 0)
	//		scriptArgument = 0;

	pTeamData->ConditionalJump_ComparatorValue = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Possible values are 3:">=" -> 0:"<", 1:"<=", 2:"==", 3:">=", 4:">", 5:"!="
void ScriptExt::ConditionalJump_SetComparatorMode(TeamClass* pTeam, int value = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = value;

	if (scriptArgument < 0 || scriptArgument > 5)
		scriptArgument = pScript->GetCurrentAction().Argument;

	if (scriptArgument < 0 || scriptArgument > 5)
		scriptArgument = 3; // >=

	pTeamData->ConditionalJump_ComparatorMode = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetCounter(TeamClass* pTeam, int value = -100000000)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;

	if (value == -100000000)
		value = pScript->GetCurrentAction().Argument;

	pTeamData->ConditionalJump_Counter = value;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ResetVariables(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	// Cleaning Conditional Jump related variables
	pTeamData->ConditionalJump_Evaluation = false;
	pTeamData->ConditionalJump_ComparatorMode = 3; // >=
	pTeamData->ConditionalJump_ComparatorValue = 1;
	pTeamData->ConditionalJump_EnabledKillsCount = false;
	pTeamData->ConditionalJump_Counter = 0;
	pTeamData->AbortActionAfterKilling = false;
	pTeamData->ConditionalJump_Index = -1000000;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ManageResetIfJump(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;

	if (enable < 0)
		enable = pScript->GetCurrentAction().Argument;

	if (enable > 0)
		pTeamData->ConditionalJump_ResetVariablesIfJump = true;
	else
		pTeamData->ConditionalJump_ResetVariablesIfJump = false;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = enable;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->GetCurrentAction().Argument;
	}

	if (scriptArgument >= 1)
		pTeamData->AbortActionAfterKilling = true;
	else
		pTeamData->AbortActionAfterKilling = false;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Count objects from [AITargetTypes] lists
void ScriptExt::ConditionalJump_CheckObjects(TeamClass* pTeam)
{
	long countValue = 0;

	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	int index = pTeamData->ConditionalJump_Index;
	auto const& nAITargetType = RulesExtData::Instance()->AITargetTypesLists;

	if (!nAITargetType.empty() && index >= 0)
	{
		const auto objectsList = make_iterator(nAITargetType[(index)]);

		if (objectsList.empty())
		{
			pTeam->StepCompleted = true;
			return;
		}

		countValue = std::count_if(TechnoClass::Array->begin(), TechnoClass::Array->end(), [&](TechnoClass* pTechno)
 {
	 if (TechnoExtData::IsAlive(pTechno))
	 {

		 const auto pTechnoType = pTechno->GetTechnoType();
		 if ((!pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
			 || (pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
				 && pTechno->IsMindControlled()
				 && !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno->MindControlledBy))))
		 {
			 return objectsList.contains(pTechnoType);
		 }
	 }

	 return false;
		});

		pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, countValue, pTeamData->ConditionalJump_ComparatorValue);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

// A simple counter. The count can be increased or decreased
void ScriptExt::ConditionalJump_CheckCount(TeamClass* pTeam, int modifier = 0)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;

	if (modifier == 0)
		modifier = pScript->GetCurrentAction().Argument;

	if (modifier == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	pTeamData->ConditionalJump_Counter += modifier;
	int currentCount = pTeamData->ConditionalJump_Counter;
	int comparatorValue = pTeamData->ConditionalJump_ComparatorValue;
	pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, currentCount, comparatorValue);

	// This action finished
	pTeam->StepCompleted = true;
}

bool ScriptExt::ConditionalJump_MakeEvaluation(int comparatorMode, int studiedValue, int comparatorValue)
{
	// Comparators are like in [AITriggerTypes] from aimd.ini
	switch (comparatorMode)
	{
	case 0:
		// <
		return (studiedValue < comparatorValue);
		break;
	case 1:
		// <=
		return (studiedValue <= comparatorValue);
		break;
	case 2:
		// ==
		return (studiedValue == comparatorValue);
		break;
	case 3:
		// >=
		return (studiedValue >= comparatorValue);
		break;
	case 4:
		// >
		return (studiedValue > comparatorValue);
		break;
	case 5:
		// !=
		return (studiedValue != comparatorValue);
		break;
	}

	return false;
}

void ScriptExt::ConditionalJump_CheckHumanIsMostHated(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	HouseClass* pEnemyHouse = nullptr;

	if (auto pHouse = pTeam->Owner)
	{
		int angerLevel = -1;
		bool isHumanHouse = false;

		for (auto& pNode : pHouse->AngerNodes)
		{
			if (!pNode.House->Type->MultiplayPassive
				&& !pNode.House->Defeated
				&& !pNode.House->IsObserver()
				&& ((pNode.AngerLevel > angerLevel
					&& !pHouse->IsAlliedWith(pNode.House))
					|| angerLevel < 0))
			{
				angerLevel = pNode.AngerLevel;
				pEnemyHouse = pNode.House;
			}
		}

		if (pEnemyHouse && pEnemyHouse->IsControlledByHuman())
		{
			isHumanHouse = true;
		}

		pTeamData->ConditionalJump_Evaluation = isHumanHouse;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ConditionalJump_CheckAliveHumans(TeamClass* pTeam, int mode = 0)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->GetCurrentAction().Argument;

	if (mode < 0 || mode > 2)
		mode = 0;

	if (auto pHouse = pTeam->Owner)
	{
		pTeamData->ConditionalJump_Evaluation = false;

		// Find an alive human House
		for (auto& pNode : pHouse->AngerNodes)
		{
			if (!pNode.House->Type->MultiplayPassive
				&& !pNode.House->Defeated
				&& !pNode.House->IsObserver()
				&& pNode.House->IsControlledByHuman())
			{
				if (mode == 1 && !pHouse->IsAlliedWith(pNode.House)) // Mode 1: Enemy humans
				{
					pTeamData->ConditionalJump_Evaluation = true;
					break;
				}
				else if (mode == 2 && !pHouse->IsAlliedWith(pNode.House)) // Mode 2: Friendly humans
				{
					pTeamData->ConditionalJump_Evaluation = true;
					break;
				}

				// mode 0: Any human
				pTeamData->ConditionalJump_Evaluation = true;
				break;
			}
		}

		// If we are looking for any human the own House should be checked
		if (mode == 0 && pHouse->IsControlledByHuman())
			pTeamData->ConditionalJump_Evaluation = true;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::JumpBackToPreviousScript(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData->PreviousScriptList.empty())
	{
		ClearCurrentScript(pTeam);
		pTeam->CurrentScript = pTeamData->PreviousScriptList.back();
		pTeamData->PreviousScriptList.pop_back();
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		const auto pScript = pTeam->CurrentScript;
		auto const& nCurAct = pScript->GetCurrentAction();
		Debug::Log("JumpBackToPreviousScript : [%s] [%s](line: %d = %d,%d): Can't find the previous script! This script action must be used after PickRandomScript.\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nCurAct.Action,
			nCurAct.Argument);

		pTeam->StepCompleted = true;
		return;
	}
}

void ScriptExt::ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType = -1, bool isEnabled = false)
{
	if (!pTeam)
		return;

	const auto pScript = pTeam->CurrentScript;
	if (!pScript)
	{
		pTeam->StepCompleted = true;
		return;
	}

	if (idxAITriggerType < 0)
		idxAITriggerType = pScript->GetCurrentAction().Argument;

	if (idxAITriggerType < 0)
		return;

	auto const& nAITriggers = RulesExtData::Instance()->AITriggersLists;

	if (nAITriggers.empty())
		return;

	auto const& objectsList = make_iterator(nAITriggers[(idxAITriggerType)]);

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if (objectsList.contains(pTrigger))
		{
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse = nullptr, int sideIdx = -1, int houseIdx = -1, bool isEnabled = true)
{
	if (!pTeam)
		return;

	// if pHouse is set then it overwrites any argument
	if (pHouse)
	{
		houseIdx = pHouse->ArrayIndex;
		sideIdx = pHouse->SideIndex;
	}

	if (sideIdx < 0)
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if ((houseIdx == -1 || houseIdx == pTrigger->HouseIndex) && (sideIdx == 0 || sideIdx == pTrigger->SideIndex))
		{
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx = -1)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (sideIdx < 0)
		sideIdx = pScript->GetCurrentAction().Argument;

	if (sideIdx < -1)
		sideIdx = -1;

	if (auto pTeamData = TeamExtContainer::Instance.Find(pTeam))
		pTeamData->TriggersSideIdx = sideIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx = 1000000)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (houseIdx == 1000000)
		houseIdx = pScript->GetCurrentAction().Argument;

	houseIdx = HouseExtData::GetHouseIndex(houseIdx, pTeam, nullptr);

	if (houseIdx < -1)
		houseIdx = -1;

	if (auto pTeamData = TeamExtContainer::Instance.Find(pTeam))
		pTeamData->TriggersHouseIdx = houseIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAITriggers(TeamClass* pTeam, int enabled = -1)
{
	if (!pTeam)
		return;

	if (auto pTeamData = TeamExtContainer::Instance.Find(pTeam))
	{
		int sideIdx = pTeamData->TriggersSideIdx;
		int houseIdx = pTeamData->TriggersHouseIdx;
		pTeamData->TriggersSideIdx = -1;
		pTeamData->TriggersHouseIdx = -1;
		auto pScript = pTeam->CurrentScript;
		bool isEnabled = false;

		if (enabled < 0)
			enabled = pScript->GetCurrentAction().Argument;

		if (enabled >= 1)
			isEnabled = true;

		ScriptExt::ManageAllTriggersFromHouse(pTeam, nullptr, sideIdx, houseIdx, isEnabled);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType = -1, bool isEnabled = false)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (idxAITargetType < 0)
		idxAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITargetType < 0)
		return;

	auto const& nAITriggers = RulesExtData::Instance()->AITargetTypesLists;

	if (nAITriggers.empty())
		return;

	auto const& objectsList = make_iterator(nAITriggers[(idxAITargetType)]);

	if (objectsList.empty())
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		std::vector<TechnoTypeClass*> entriesList;

		if (pTrigger->Team1)
		{
			for (auto entry : pTrigger->Team1->TaskForce->Entries)
			{
				if (entry.Amount > 0)
				{
					entriesList.push_back(entry.Type);
				}
			}
		}

		if (pTrigger->Team2)
		{
			for (auto entry : pTrigger->Team2->TaskForce->Entries)
			{
				if (entry.Amount > 0)
				{
					entriesList.push_back(entry.Type);
				}
			}
		}

		if (!entriesList.empty())
		{
			for (auto const& entry : entriesList)
			{
				if (objectsList.contains(entry))
				{
					pTrigger->IsEnabled = isEnabled;
					break;
				}
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::RepairDestroyedBridge(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	const auto pScript = pTeam->CurrentScript;
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);

	if (!pTeamData)
	{
		auto const& nCur = pScript->GetCurrentAction();
		auto const& nNext = pScript->GetNextAction();

		pTeam->StepCompleted = true;
		Debug::Log("RepairDestroyedBridge : [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nCur.Action,
			nCur.Argument,
			pScript->CurrentMission + 1,
			nNext.Action,
			nNext.Argument);

		return;
	}

	// Save all Bridge Repair Hut Structures for saving time in the team actions. The worst case-scenario is if the map doesn't have bridges
	if (pTeamData->MapPath_BridgeRepairHuts.empty())
	{
		for (auto pBld : *BuildingClass::Array)
		{
			if (pBld->Type->BridgeRepairHut)
				pTeamData->MapPath_BridgeRepairHuts.push_back(pBld);
		}
	}

	//DynamicVectorClass<TechnoClass*> bridgeRepairHuts;
	bool isReachable = false;

	if (pTeamData->MapPath_InProgress
		&& pTeamData->MapPath_StartTechno
		&& pTeamData->MapPath_EndTechno)
	{
		// Continue the search process
		isReachable = ScriptExt::FindLinkedPath(pTeam, pTeamData->MapPath_StartTechno, pTeamData->MapPath_EndTechno);

		// The operation didn't end, it will continue in the next game frame
		if (pTeamData->MapPath_InProgress)
			return;

		if (isReachable)
		{
			const CellStruct cell = pTeamData->MapPath_EndTechno->InlineMapCoords();

			if (MapClass::Instance->IsLinkedBridgeDestroyed(cell))
				pTeamData->MapPath_ValidBridgeRepairHuts.push_back(pTeamData->MapPath_EndTechno);
		}

		auto const iter = std::find_if(pTeamData->MapPath_CheckedBridgeRepairHuts.begin(), pTeamData->MapPath_CheckedBridgeRepairHuts.end(),
			[&](TechnoClass* const pTechHere) { return pTechHere == pTeamData->MapPath_EndTechno; });

		if (iter == pTeamData->MapPath_CheckedBridgeRepairHuts.end())
			pTeamData->MapPath_CheckedBridgeRepairHuts.push_back(pTeamData->MapPath_EndTechno);
	}
	else
	{
		// Or is the first run or someone in the operation died so it should start again
		pTeamData->MapPath_InProgress = false;
		pTeamData->MapPath_StartTechno = nullptr;
		pTeamData->MapPath_EndTechno = nullptr;
		pTeamData->MapPath_ValidBridgeRepairHuts.clear();
		pTeamData->MapPath_CheckedBridgeRepairHuts.clear();
	}

	if (pTeam->Focus)
	{
		if (!Is_Building(pTeam->Focus))
		{
			pTeam->Focus = nullptr;
		}
		else
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTeam->Focus);

			if (!pBuilding->Type->BridgeRepairHut)
			{
				pTeam->Focus = nullptr;
			}
			else
			{
				const CellStruct cell = pBuilding->InlineMapCoords();

				// If the Bridge was repaired then isn't valid anymore
				if (!MapClass::Instance->IsLinkedBridgeDestroyed(cell))
					pTeam->Focus = nullptr;
			}
		}
	}

	TechnoClass* selectedTarget = pTeam->Focus ? static_cast<TechnoClass*>(pTeam->Focus) : nullptr;
	std::vector<InfantryClass*> engineers;
	std::vector<FootClass*> otherTeamMembers;

	// If there are no engineers end this script action
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!TechnoExtData::IsActive(pUnit, true, false, false, true))
			continue;

		if (Is_Infantry(pUnit))
		{

			const auto pInf = static_cast<InfantryClass*>(pUnit);

			if (pInf->IsEngineer())
			{
				engineers.push_back(pInf);
				continue;
			}
		}

		// These units will receive a different command
		otherTeamMembers.push_back(pUnit);
	}

	if (engineers.empty())
	{
		auto const nCur = pScript->GetCurrentAction();
		auto const nNext = pScript->GetNextAction();

		pTeam->StepCompleted = true;
		Debug::Log("RepairDestroyedBridge : [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: Team has no engineers)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nCur.Action,
			nCur.Argument,
			pScript->CurrentMission + 1,
			nNext.Action,
			nNext.Argument);

		pTeamData->MapPath_ValidBridgeRepairHuts.clear();
		pTeamData->MapPath_CheckedBridgeRepairHuts.clear();

		return;
	}

	if (!selectedTarget)
	{
		// Looking for BridgeRepairHut=yes structures
		//auto anyEngineer = engineers.GetItem(0);
		for (auto const& pTechno : pTeamData->MapPath_BridgeRepairHuts)
		{
			auto const MapPath_ValidBridgeRepairHuts_iter = std::find_if(
				pTeamData->MapPath_ValidBridgeRepairHuts.begin(),
				pTeamData->MapPath_ValidBridgeRepairHuts.end(),
				[&](TechnoClass* const pTechHere) { return pTechHere == pTechno; });

			// If it was previously inserted we will ignore it
			if (MapPath_ValidBridgeRepairHuts_iter != pTeamData->MapPath_ValidBridgeRepairHuts.end())
				continue;

			auto const MapPath_CheckedBridgeRepairHuts_iter = std::find_if(
				pTeamData->MapPath_CheckedBridgeRepairHuts.begin(),
				pTeamData->MapPath_CheckedBridgeRepairHuts.end(),
				[&](TechnoClass* const pTechHere) { return pTechHere == pTechno; });

			// If it was previously checked we will ignore it
			if (MapPath_CheckedBridgeRepairHuts_iter != pTeamData->MapPath_CheckedBridgeRepairHuts.end())
				continue;

			const auto engineer = static_cast<TechnoClass*>(engineers[0]);
			isReachable = ScriptExt::FindLinkedPath(pTeam, engineer, pTechno);

			// This process didn't end. It will continue in the next game frame
			if (pTeamData->MapPath_InProgress)
				return;

			if (isReachable)
			{
				const CellStruct cell = pTechno->InlineMapCoords();
				if (MapClass::Instance->IsLinkedBridgeDestroyed(cell))
					pTeamData->MapPath_ValidBridgeRepairHuts.push_back(pTechno);
			}

			pTeamData->MapPath_CheckedBridgeRepairHuts.push_back(pTechno);
		}

		if (pTeamData->MapPath_ValidBridgeRepairHuts.empty())
		{
			auto const nCur = pScript->GetCurrentAction();
			auto const nNext = pScript->GetNextAction();

			pTeam->StepCompleted = true;
			Debug::Log("RepairDestroyedBridge: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: This map has no Bridge Repair Huts)\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				nCur.Action,
				nCur.Argument,
				pScript->CurrentMission + 1,
				nNext.Action,
				nNext.Argument);

			pTeamData->MapPath_ValidBridgeRepairHuts.clear();
			pTeamData->MapPath_CheckedBridgeRepairHuts.clear();

			return;
		}

		if (mode < 0)
			mode = pScript->GetCurrentAction().Argument;

		// Pick the nearest destroyed bridge
		int bestVal = -1;

		if (mode < 0)
		{
			// Pick a random bridge
			selectedTarget =
				pTeamData->MapPath_ValidBridgeRepairHuts[
					ScenarioClass::Instance->
						Random.RandomFromMax(pTeamData->MapPath_ValidBridgeRepairHuts.size() - 1)];
		}
		else
		{
			for (auto& pHut : pTeamData->MapPath_ValidBridgeRepairHuts)
			{
				if (mode > 0)
				{
					// Pick the farthest target
					const int value = engineers[0]->DistanceFrom(pHut); // Note: distance is in leptons (*256)

					if (value >= bestVal || bestVal < 0)
					{
						bestVal = value;
						selectedTarget = pHut;
					}
				}
				else
				{
					// Pick the closest target
					const int value = engineers[0]->DistanceFrom(pHut); // Note: distance is in leptons (*256)

					if (value < bestVal || bestVal < 0)
					{
						bestVal = value;
						selectedTarget = pHut;
					}
				}
			}
		}
	}

	if (!selectedTarget)
	{
		auto const nCur = pScript->GetCurrentAction();
		auto const nNext = pScript->GetNextAction();

		pTeam->StepCompleted = true;
		Debug::Log("RepairDestroyedBridge: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: Can not select a Bridge Repair Hut)\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			nCur.Action,
			nCur.Argument,
			pScript->CurrentMission + 1,
			nNext.Action,
			nNext.Argument);

		pTeamData->MapPath_ValidBridgeRepairHuts.clear();
		pTeamData->MapPath_CheckedBridgeRepairHuts.clear();

		return;
	}

	pTeam->Focus = selectedTarget;
	pTeamData->MapPath_ValidBridgeRepairHuts.clear();
	pTeamData->MapPath_CheckedBridgeRepairHuts.clear();

	for (auto const& engineer : engineers)
	{
		if (engineer->Destination != selectedTarget)
		{
			engineer->SetTarget(selectedTarget);
			engineer->QueueMission(Mission::Capture, true);
		}
	}

	if (!otherTeamMembers.empty())
	{
		double closeEnough = 1.0 * RulesClass::Instance->CloseEnough; // Note: this value is in leptons (*256)
		if (pTeamData->CloseEnough.has_value())
			closeEnough = pTeamData->CloseEnough.value() * 256.0;

		for (auto const& pFoot : otherTeamMembers)
		{


			if (!pFoot->Destination
				|| (selectedTarget->DistanceFrom(pFoot->Destination) > closeEnough))
			{
				// Reset previous command
				pFoot->SetTarget(nullptr);
				pFoot->SetFocus(nullptr);
				pFoot->SetDestination(nullptr, false);
				pFoot->ForceMission(Mission::Guard);

				// Get a cell near the target
				pFoot->QueueMission(Mission::Move, false);
				CoordStruct coord = TechnoExtData::PassengerKickOutLocation(selectedTarget, pFoot);
				CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);
				pFoot->SetDestination(pCellDestination, true);
			}

			// Reached destination, stay in guard until next action
			if (pFoot->DistanceFrom(pFoot->Destination) < closeEnough)
				pFoot->QueueMission(Mission::Area_Guard, false);
		}
	}
}

// Find the shortest valid path to the destination, if possible
bool ScriptExt::FindLinkedPath(TeamClass* pTeam, TechnoClass* pThis = nullptr, TechnoClass* pTarget = nullptr)
{
	if (!pTeam)
		return false;

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	CellStruct startCell = CellStruct::Empty;
	CellStruct endCell = CellStruct::Empty;

	if (!pTeamData->MapPath_InProgress)
	{
		// First time that is execued this process
		if (pThis == nullptr || pTarget == nullptr)
			return false;

		pTeamData->MapPath_StartTechno = pThis;
		pTeamData->MapPath_EndTechno = pTarget;

		pTeamData->MapPath_InProgress = true;
		pTeamData->MapPath_Grid.clear();
		pTeamData->MapPath_Queue.clear();

		// Creates a "map" with the same size of the original and each "cell" is a boolean value that means if the cell was evaluated or not. Evaluated "cells" get ignored in posterior checks
		int matrixX = MapClass::Instance->MapCoordBounds.Right;
		int matrixY = MapClass::Instance->MapCoordBounds.Bottom;

		pTeamData->MapPath_Grid = std::vector<std::vector<bool>>(matrixX, std::vector<bool>(matrixY, false));

		startCell = pThis->InlineMapCoords();
		endCell = pTarget->InlineMapCoords();

		// The first element of this path is the unit's location
		// Note: distance is in leptons (*256)
		pTeamData->MapPath_Queue.emplace_back(pThis->DistanceFrom(pTarget), startCell.X, startCell.Y);
	}
	else
	{
		// We'll resume the previous unfinished analysis that was so time expensible that was splitted in multiple parts
		if (pThis == nullptr || pTarget == nullptr)
		{
			pTeamData->MapPath_InProgress = false;
			pTeamData->MapPath_Grid.clear();
			pTeamData->MapPath_Queue.clear();
			pTeamData->MapPath_StartTechno = nullptr;
			pTeamData->MapPath_EndTechno = nullptr;

			return false;
		}

		endCell = pTarget->InlineMapCoords();
	}

	bool found = false;

	// If we don't split this operation in multiple frames the game will stop half a second in the worst case scenarios for finding the destination (or if directly there is no valid path) and that's unaceptable.
	// We'll use a number of checks limiter and when it reaches 0 we will stop the process and continue it in the next frame.
	// Lower value == more frames required for calculating a valid path (if exists).
	// Higher value == less game frames required for calculating a valid path but noticeable FPS drops.
	int nChecksLeft = 512;

	while ((pTeamData->MapPath_Queue.size() > 0) && !found)
	{
		// If counter reached the limit we will stop the process and continue it in the next frame.
		if (nChecksLeft <= 0)
			return false;

		nChecksLeft--;

		// Extract the first element of MapPath_Queue for analyzing it
		MapPathCellElement element = pTeamData->MapPath_Queue[0];
		pTeamData->MapPath_Queue.erase(pTeamData->MapPath_Queue.begin());

		// Check cells around the selected cell, it only stops if we reach the destination of the queue is empty
		for (int i = element.X - 1; (i <= element.X + 1) && !found; i++)
		{
			for (int j = element.Y - 1; (j <= element.Y + 1) && !found; j++)
			{
				const CellStruct nCell { (short)i   , (short)j };

				// If reached the destination end the process or the target moved into an evaluated area the process finished
				if ((nCell.X == endCell.X && nCell.Y == endCell.Y)
					|| (pTeamData->MapPath_Grid[endCell.X][endCell.Y]))
				{
					found = true;
					break;
				}

				// Only check nonvisited cells
				if (!pTeamData->MapPath_Grid[i][j])
				{
					if (MapClass::Instance->IsWithinUsableArea(nCell, false))
					{
						const auto pCell = MapClass::Instance->TryGetCellAt(nCell);
						if (pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) != Move::OK)
							pTeamData->MapPath_Grid[i][j] = true;

						if (!pTeamData->MapPath_Grid[i][j])
						{
							// If is a valid cell we'll queue it for future checks
							// Note: distance is in leptons (*256)
							MapPathCellElement newElement { pTarget->DistanceFrom(pCell)  , (short)i , (short)j };
							pTeamData->MapPath_Grid[i][j] = true;

							// Find the right position in the vector. Sorted by ascendent distance;
							if (pTeamData->MapPath_Queue.size() == 0)
							{
								pTeamData->MapPath_Queue.push_back(newElement);
							}
							else
							{
								auto index = pTeamData->MapPath_Queue.begin();
								bool inserted = false;

								for (unsigned int k = 0; k < pTeamData->MapPath_Queue.size(); k++)
								{
									if (newElement < pTeamData->MapPath_Queue[k])
									{
										pTeamData->MapPath_Queue.insert(index, newElement);
										inserted = true;

										break;
									}

									++index;
								}

								if (!inserted)
									pTeamData->MapPath_Queue.push_back(newElement);
							}
						}
					}
					else
					{
						// Mark the unusable cells as visited
						pTeamData->MapPath_Grid[i][j] = true;
					}
				}
			}
		}
	}

	// Ended. Cleanning the mess
	pTeamData->MapPath_InProgress = false;
	pTeamData->MapPath_Grid.clear();
	pTeamData->MapPath_Queue.clear();

	return found;
}
#undef TECHNO_IS_ALIVE

bool NOINLINE ScriptExt::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed, bool allowSubterranean)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0
		&& !pTechno->InLimbo
		&& !pTechno->Transporter
		&& !pTechno->Absorbed
		&& !pTechno->IsCrashing
		&& !pTechno->IsSinking;

	if (!isAvailable)
		return false;

	bool isSubterranean = allowSubterranean && pTechno->InWhichLayer() == Layer::Underground;
	isAvailable &= pTechno->IsOnMap || isSubterranean;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

#include <Ext/SWType/Body.h>

void DebugHere(const char* pWho, const char* pReason, TeamClass* pTeam)
{
	auto pScript = pTeam->CurrentScript;
	const auto nCur = pScript->GetCurrentAction();
	const auto nNext = pScript->GetNextAction();

	Debug::Log("AI Scripts - %s: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: %s)\n",
	pWho, pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission,
	nCur.Action,
	nCur.Argument,
	pScript->CurrentMission + 1,
	nNext.Action,
	nNext.Argument,
	pReason

	);
}

void NOINLINE ScriptExt::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	auto const pLeader = ScriptExt::FindTheTeamLeader(pTeam);

	if (!pLeader)
	{
		DebugHere("ChronoshiftToEnemyBase", "No Leader found", pTeam);
		pTeam->StepCompleted = true;
		return;
	}

	HouseClass* pEnemy = HouseClass::Array->GetItemOrDefault(pLeader->Owner->EnemyHouseIndex);

	if (!pEnemy)
	{
		DebugHere("ChronoshiftToEnemyBase", "No enemy house found", pTeam);
		pTeam->StepCompleted = true;
		return;
	}

	auto const pTargetCell = HouseExtData::GetEnemyBaseGatherCell(pEnemy, pLeader->Owner, pLeader->GetCoords(), pLeader->GetTechnoType()->SpeedType, extraDistance);

	if (!pTargetCell)
	{
		DebugHere("ChronoshiftToEnemyBase", "No target cell found", pTeam);
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExt::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

void NOINLINE ScriptExt::ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget)
{
	if (!pTeamLeader || !pTarget)
	{
		pTeam->StepCompleted = true;
		return;
	}

	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperChronosphere = nullptr;

	for (auto const pSuper : pOwner->Supers)
	{
		if (!SWTypeExtContainer::Instance.Find(pSuper->Type)->IsAvailable(pOwner))
			continue;

		if (!pSuperChronosphere && pSuper->Type->Type == SuperWeaponType::ChronoSphere)
		{
			pSuperChronosphere = pSuper;
			break;
		}
	}

	if (!pSuperChronosphere)
	{
		DebugHere("ChronoshiftTeamToTarget", "No Chronosphere or ChronoWarp superweapon found", pTeam);
		return;
	}

	if (!pSuperChronosphere->IsCharged || (pSuperChronosphere->IsPowered() && !pOwner->Is_Powered()))
	{
		if (pSuperChronosphere->Granted)
		{

			int rechargeTime = pSuperChronosphere->GetRechargeTime();
			double timeLeft = (double)pSuperChronosphere->RechargeTimer.GetTimeLeft();

			if ((1.0 - RulesClass::Instance->AIMinorSuperReadyPercent) < (timeLeft / rechargeTime))
			{
				DebugHere("ChronoshiftTeamToTarget", "Chronosphere superweapon charge not at AIMinorSuperReadyPercent yet, not jumping to next line yet", pTeam);
				return;
			}

		}
		else
		{
			DebugHere("ChronoshiftTeamToTarget", "Chronosphere superweapon not available", pTeam);
			pTeam->StepCompleted = true;
			return;
		}
	}

	auto pTargetCell = MapClass::Instance->TryGetCellAt(pTarget->GetCoords());

	if (pTargetCell)
	{
		int idxWarp = SuperWeaponTypeClass::FindIndexById(SWTypeExtContainer::Instance.Find(pSuperChronosphere->Type)->SW_PostDependent);
		auto const& Types = *SuperWeaponTypeClass::Array;

		if (!Types.ValidIndex(idxWarp) || Types[idxWarp]->Type != SuperWeaponType::ChronoWarp)
		{
			for (auto const& pWarp : Types)
			{
				if (pWarp->Type == SuperWeaponType::ChronoWarp)
				{
					idxWarp = Types.GetItemIndex(&pWarp);
					break;
				}
			}
		}

		if (idxWarp == -1)
		{
			pTeam->StepCompleted = true;
			Debug::Log("[ChronoSphere::Activate] There is no SuperWeaponType with Type=ChronoWarp. Aborted.\n");
			return;
		}

		pOwner->Fire_SW(pSuperChronosphere->Type->ArrayIndex, pTeam->SpawnCell->MapCoords);
		pOwner->Fire_SW(idxWarp, pTargetCell->MapCoords);
		pTeam->AssignMissionTarget(pTargetCell);
		DebugHere("ChronoshiftTeamToTarget", "Finished successfully", pTeam);
	}
	else
	{
		DebugHere("ChronoshiftTeamToTarget", "No target cell found", pTeam);
	}

	pTeam->StepCompleted = true;
	return;
}
