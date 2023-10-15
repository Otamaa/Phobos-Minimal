#include "Body.h"

#include <Ext/Team/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>

void ScriptExtData::ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;
	const auto&[curAct , curArgs] = pScript->GetCurrentAction();

	if (idxAITriggerType < 0)
		idxAITriggerType = curArgs;

	if (idxAITriggerType < 0) {
		pTeam->StepCompleted = true;
		return;
	}

	auto const& triggetList = RulesExtData::Instance()->AITriggersLists;

	if (triggetList.empty() || (size_t)idxAITriggerType >= triggetList.size() ) {
		pTeam->StepCompleted = true;
		return;
	}

	const auto Iter_triggerList_inside = Iterator(triggetList[idxAITriggerType]);

	for (auto pTrigger : *AITriggerTypeClass::Array) {
		if (Iter_triggerList_inside.contains(pTrigger)) {
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse = nullptr, int sideIdx = -1, int houseIdx = -1, bool isEnabled = true)
{
	// if pHouse is set then it overwrites any argument
	if (pHouse)
	{
		houseIdx = pHouse->ArrayIndex;
		sideIdx = pHouse->SideIndex;
	}

	if (sideIdx < 0) {
		pTeam->StepCompleted = true;
		return;
	}

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if ((houseIdx == -1 || houseIdx == pTrigger->HouseIndex) &&
				(sideIdx == 0 || sideIdx == pTrigger->SideIndex))
		{
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx = -1)
{
	auto pScript = pTeam->CurrentScript;
	const auto&[curAct , curArgs] = pScript->GetCurrentAction();

	if (sideIdx < 0)
		sideIdx = curArgs;

	if (sideIdx < -1)
		sideIdx = -1;

	TeamExtContainer::Instance.Find(pTeam)->TriggersSideIdx = sideIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx = 1000000)
{
	auto pScript = pTeam->CurrentScript;
	const auto&[curAct , curArgs] = pScript->GetCurrentAction();

	if (houseIdx == 1000000)
		houseIdx = curArgs;

	houseIdx = HouseExtData::GetHouseIndex(houseIdx, pTeam, nullptr);

	if (houseIdx < -1)
		houseIdx = -1;

	TeamExtContainer::Instance.Find(pTeam)->TriggersHouseIdx = houseIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ManageAITriggers(TeamClass* pTeam, int enabled = -1)
{
	auto pTeamData = TeamExtContainer::Instance.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	const auto&[curAct , curArgs] = pScript->GetCurrentAction();

	{
		int sideIdx = pTeamData->TriggersSideIdx;
		int houseIdx = pTeamData->TriggersHouseIdx;
		pTeamData->TriggersSideIdx = -1;
		pTeamData->TriggersHouseIdx = -1;
		bool isEnabled = false;

		if (enabled < 0)
			enabled = curArgs;

		if (enabled >= 1)
			isEnabled = true;

		ScriptExtData::ManageAllTriggersFromHouse(pTeam, nullptr, sideIdx, houseIdx, isEnabled);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExtData::ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;
	const auto&[curAct , curArgs] = pScript->GetCurrentAction();

	if (idxAITargetType < 0)
		idxAITargetType = curArgs;

	if (idxAITargetType < 0) {
		pTeam->StepCompleted = true;
		return;
	}

	const auto& targetList = RulesExtData::Instance()->AITargetTypesLists;

	if (targetList.empty() || (size_t)idxAITargetType >= targetList.size()) {
		pTeam->StepCompleted = true;
		return;
	}

	const auto targetList_inside = Iterator(targetList[idxAITargetType]);

	if (targetList_inside.empty()) {
		pTeam->StepCompleted = true;
		return;
	}

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
			for (auto entry : entriesList) {
				for(auto const& target_ : targetList_inside) {
					if(TeamExtData::GroupAllowed(entry , target_)) {
						pTrigger->IsEnabled = isEnabled;
						break;
					}
				}
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}