#include "Body.h"

#include <Ext/Team/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>

void ScriptExtData::ResetAngerAgainstHouses(TeamClass* pTeam)
{
	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		angerNode.AngerLevel = 0;
	}

	pTeam->Owner->EnemyHouseIndex = -1;
	ScriptExtData::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true; // This action finished - FS-21
}

void ScriptExtData::SetHouseAngerModifier(TeamClass* pTeam, int modifier = 0)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (modifier <= 0)
		modifier = curArgs;

	if (modifier < 0)
		modifier = 0;

	pTeamData->AngerNodeModifier = modifier;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	bool changeFailed = true;
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (idxHousesList <= 0)
		idxHousesList = curArgs;

	const auto& houseLists = RulesExtData::Instance()->AIHousesLists;

	if ((size_t)idxHousesList < houseLists.size())
	{
		if (const auto houselist = Iterator(houseLists[idxHousesList]))
		{
			for (const auto pHouseType : houselist)
			{
				for (auto& angerNode : pTeam->Owner->AngerNodes)
				{
					if (angerNode.House->IsObserver())
						continue;

					if (angerNode.House->Type == pHouseType)
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
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArgs
		);

		return;
	}

	ScriptExtData::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExtData::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	int changes = 0;
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (idxHousesList <= 0)
		idxHousesList = curArgs;

	const auto& houseLists = RulesExtData::Instance()->AIHousesLists;

	if ((size_t)idxHousesList < houseLists.size())
	{
		if (auto const objectsList = Iterator(houseLists[idxHousesList]))
		{
			int IdxSelectedObject = ScenarioClass::Instance->Random.RandomFromMax(objectsList.size() - 1);

			for (auto& angerNode : pTeam->Owner->AngerNodes)
			{
				if (angerNode.House->Defeated || angerNode.House->IsObserver())
					continue;

				if (angerNode.House->Type == objectsList[IdxSelectedObject])
				{
					angerNode.AngerLevel += pTeamData->AngerNodeModifier;
					changes++;
				}
			}
		}
	}

	// This action finished
	if (changes == 0)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArgs
		);

		return;
	}

	ScriptExtData::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExtData::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::SetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1, bool random = false)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (mask == 0)
		mask = curArgs;

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
	for (const auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (pTeam->Owner == angerNode.House
			|| angerNode.House->Defeated
			|| pTeam->Owner->IsAlliedWith(angerNode.House)
			|| angerNode.House->Type->MultiplayPassive
			|| angerNode.House->IsObserver())
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
			selectedHouse = objectsList[IdxSelectedObject];
		}
	}
	else
	{
		selectedHouse = ScriptExtData::GetTheMostHatedHouse(pTeam, mask, mode);
	}

	if (selectedHouse)
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->Defeated || angerNode.House->IsObserver())
				continue;

			if (angerNode.House == selectedHouse)
			{
				angerNode.AngerLevel = newHateLevel;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Picked a new house as enemy [%s]\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				pTeam->CurrentScript->CurrentMission,
				curAct,
				curArgs,
				angerNode.House->Type->ID);
			}
		}

		ScriptExtData::UpdateEnemyHouseIndex(pTeam->Owner);
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArgs
		);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

HouseClass* ScriptExtData::GetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1)
{
	if (mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	// Note regarding "mode": 1 is used for ">" comparisons and 0 for "<"
	if (mode <= 0)
		mode = 0;
	else
		mode = 1;

	// Find the Team Leader

	FootClass* pLeaderUnit = pTeamData->TeamLeader;

	if (!pLeaderUnit)
	{
		pTeamData->TeamLeader = ScriptExtData::FindTheTeamLeader(pTeam);
	}

	double objectDistance = -1;
	double enemyDistance = -1;
	double enemyThreatValue[8] = { 0 };
	HouseClass* enemyHouse = nullptr;
	const double& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
	long houseMoney = -1;
	int enemyPower = -1000000000;
	int enemyKills = -1;
	int enemyAirDocks = -1;
	int enemyStructures = -1;
	int enemyNavalUnits = -1;

	switch (mask)
	{
	case -2:
	{
		// Based on House economy
		for (const auto& pHouse : *HouseClass::Array)
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);
	}
	break;
	case -3:
	{
		// Based on Human Controlled check
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| !pHouse->IsControlledByHuman()
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			CoordStruct houseLocation;
			houseLocation.X = pHouse->BaseSpawnCell.X;
			houseLocation.Y = pHouse->BaseSpawnCell.Y;
			houseLocation.Z = 0;
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);
	}
	break;
	case -4:
	case -5:
	case -6:
	{
		int checkedHousePower = 0;

		// House power check
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->Defeated
				|| pHouse->IsObserver()
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mask == -4)
				checkedHousePower = pHouse->Power_Drain();
			else
				if (mask == -5)
					checkedHousePower = pHouse->PowerOutput;
				else
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);
	}
	break;
	case -7:
	{
		// Based on House kills
		for (const auto& pHouse : *HouseClass::Array)
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);
	}
	break;
	case -8:
	{
		// Based on number of House naval units
		for (const auto& pHouse : *HouseClass::Array)
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

			for (const auto& pUnit : *TechnoClass::Array)
			{
				if (pUnit->IsAlive
					&& pUnit->Health > 0
					&& pUnit->Owner == pHouse
					&& !pUnit->InLimbo
					&& pUnit->IsOnMap
					&& ScriptExtData::EvaluateObjectWithMask(pUnit, 31, -1, -1, nullptr))
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex);
	}
	break;
	case -9:
	{
		// Based on number of House aircraft docks
		for (const auto& pHouse : *HouseClass::Array)
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);

	}
	break;
	case -10:
	{
		// Based on number of House factories (except aircraft factories)
		for (const auto& pHouse : *HouseClass::Array)
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
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);
	}
	break;
	default:
	{
		double value = -1;
		// Depending the mode check what house will be selected as the most hated
		for (auto pTechno : *TechnoClass::Array)
		{
			if (!ScriptExtData::IsUnitAvailable(pTechno, true))
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
					if (ScriptExtData::EvaluateObjectWithMask(pTechno, mask, -1, -1, pLeaderUnit))
					{
						auto pTechnoType = pTechno->GetTechnoType();

						enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->ThreatPosed;

						if (pTechnoType->SpecialThreatValue > 0)
							enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}
				}
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (mode == 0)
			{
				// Select House with LESS threat
				if ((enemyThreatValue[i] < value || value == -1)
					&& !HouseClass::Array->Items[i]->Defeated
					&& !HouseClass::Array->Items[i]->IsObserver())
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->Items[i];
				}
			}
			else
			{
				// Select House with MORE threat
				if ((enemyThreatValue[i] > value || value == -1)
					&& !HouseClass::Array->Items[i]->Defeated
					&& !HouseClass::Array->Items[i]->IsObserver())
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->Items[i];
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			curAct,
			curArgs,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex
			);

	}
	break;
	}

	return enemyHouse;
}

void ScriptExtData::OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (mode < 0 || mode > 2)
		mode = curArgs;

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
		pTeamData->OnlyTargetHouseEnemy = (bool)ScenarioClass::Instance->Random.RandomRanged(0, 1);
		break;

	default:
		pTeamData->OnlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;
		pTeamData->OnlyTargetHouseEnemyMode = -1;
		break;
	}

	Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): New Team -> OnlyTargetHouseEnemy value: %d\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		pTeam->CurrentScript->CurrentMission,
		curAct,
		curArgs,
		pTeamData->OnlyTargetHouseEnemy
	);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto& [curAct, curArgs] = pScript->GetCurrentAction();

	if (idxHouse < 0)
		idxHouse = curArgs;

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
			if (angerNode.House->ArrayIndex == idxHouse
				&& !angerNode.House->Defeated
				&& !angerNode.House->IsObserver())
			{
				angerNode.AngerLevel += pTeamData->AngerNodeModifier;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Modified anger level against [%s](index: %d) with value: %d\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pTeam->CurrentScript->CurrentMission,
		curAct,
		curArgs,
					angerNode.House->Type->ID,
					angerNode.House->ArrayIndex,
					angerNode.AngerLevel
				);
			}
		}
	}

	ScriptExtData::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExtData::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

// The selected house will become the most hated of the map (the effects are only visible if the other houses are enemy of the selected house)
void ScriptExtData::AggroHouse(TeamClass* pTeam, int index = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	std::vector<HouseClass*> objectsList;
	HouseClass* selectedHouse = nullptr;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Store the list of playable houses for later
	for (const auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !angerNode.House->Type->MultiplayPassive
			&& !angerNode.House->IsObserver())
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
		for (auto pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated && pHouse->ArrayIndex == index)
				selectedHouse = pHouse;
		}
	}

	if (selectedHouse || index == -3)
	{
		// For each playable house set the selected house as the one with highest hate value;
		for (auto& pHouse : objectsList)
		{
			int highestHateLevel = 0;

			for (const auto& angerNode : pHouse->AngerNodes)
			{
				if (angerNode.AngerLevel > highestHateLevel)
					highestHateLevel = angerNode.AngerLevel;
			}

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (index == -3)
				{
					if (angerNode.House->IsControlledByHuman())
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
				}
				else
				{
					if (selectedHouse == angerNode.House)
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
				}
			}

			ScriptExtData::UpdateEnemyHouseIndex(pHouse);
		}
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house with index: %d\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument,
			index
		);
	}

	ScriptExtData::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::UpdateEnemyHouseIndex(HouseClass* pHouse)
{
	int angerLevel = 0;
	int index = -1;

	for (const auto& angerNode : pHouse->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !angerNode.House->IsObserver()
			&& !pHouse->IsAlliedWith(angerNode.House)
			&& angerNode.AngerLevel > angerLevel)
		{
			angerLevel = angerNode.AngerLevel;
			index = angerNode.House->ArrayIndex;
		}
	}

	pHouse->EnemyHouseIndex = index;
}

void ScriptExtData::DebugAngerNodesData()
{
#ifdef DebugThese
	Debug::Log("DEBUG: Updated AngerNodes lists of every playable House:\n");

	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->IsObserver())
			Debug::Log("Player %d [Observer] ", pHouse->ArrayIndex);
		else
			Debug::Log("Player %d [%s]: ", pHouse->ArrayIndex, pHouse->Type->ID);

		int i = 0;

		for (const auto& angerNode : pHouse->AngerNodes)
		{
			if (!pHouse->IsObserver())
				Debug::Log("%d:%d", angerNode.House->ArrayIndex, angerNode.AngerLevel);

			if (i < HouseClass::Array->Count - 2 && !pHouse->IsObserver())
				Debug::Log(", ");

			i++;
		}

		if (!pHouse->IsObserver())
			Debug::Log(" -> Main Enemy House: %d\n", pHouse->EnemyHouseIndex);
		else
			Debug::Log("\n");
	}
#endif
}