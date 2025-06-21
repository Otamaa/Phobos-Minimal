#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Script/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Ares/Hooks/Header.h>

#include <AITriggerTypeClass.h>

void TeamExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TeamLeader, ptr, bRemoved);
	AnnounceInvalidPointer(LastFoundSW, ptr);
	AnnounceInvalidPointer(PreviousScript, ptr);
}

bool TeamExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto const& pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject))
				continue;

			if (((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto const& pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject))
				continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExtData::NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list)
{
	bool result = false;
	int counter = 0;

	for (auto pHouse : *HouseClass::Array)
	{
		if (IS_SAME_STR_(SideClass::Array->Items[pHouse->Type->SideIndex]->Name, GameStrings::Civilian()))
			continue;

		// Count all objects of the list, like an OR operator
		for (auto const& pItem : list)
		{
			for (auto pObject : *TechnoClass::Array)
			{
				if (!TechnoExtData::IsAlive(pObject))
					continue;

				if (pObject->Owner == pHouse
					&& pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}
		}
	}

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions[0].ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions[0].ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions[0].ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions[0].ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions[0].ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions[0].ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExtData::HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (list.empty())
		return false;

	// Count all objects of the list, like an AND operator
	for (auto const& pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject))
				continue;

			if (pObject->Owner == pHouse &&
				pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions[0].ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions[0].ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions[0].ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions[0].ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions[0].ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions[0].ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions[0].ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExtData::EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	if (list.empty())
		return false;

	// Count all objects of the list, like an AND operator
	for (auto const& pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject) || !pObject->Owner)
				continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions[0].ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions[0].ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions[0].ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions[0].ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions[0].ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions[0].ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions[0].ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExtData::NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list)
{
	bool result = true;

	if (list.empty())
		return false;

	// Any neutral house should be capable to meet the prerequisites
	for (auto pHouse : *HouseClass::Array)
	{
		if (!result)
			break;

		bool foundAll = true;

		if (IS_SAME_STR_(SideClass::Array->Items[pHouse->Type->SideIndex]->Name, GameStrings::Civilian()))
			continue;

		// Count all objects of the list, like an AND operator
		for (auto const& pItem : list)
		{
			if (!foundAll)
				break;

			int counter = 0;

			for (auto pObject : *TechnoClass::Array)
			{
				if (!TechnoExtData::IsAlive(pObject))
					continue;

				if (pObject->Owner == pHouse &&
					pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}

			switch (pThis->Conditions[0].ComparatorOperand)
			{
			case 0:
				foundAll = counter < pThis->Conditions[0].ComparatorType;
				break;
			case 1:
				foundAll = counter <= pThis->Conditions[0].ComparatorType;
				break;
			case 2:
				foundAll = counter == pThis->Conditions[0].ComparatorType;
				break;
			case 3:
				foundAll = counter >= pThis->Conditions[0].ComparatorType;
				break;
			case 4:
				foundAll = counter > pThis->Conditions[0].ComparatorType;
				break;
			case 5:
				foundAll = counter != pThis->Conditions[0].ComparatorType;
				break;
			default:
				break;
			}
		}

		if (!foundAll)
			result = false;
	}

	return result;
}

bool TeamExtData::GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat)
{
	const auto pThatTechExt = TechnoTypeExtContainer::Instance.TryFind(pThat);
	const auto pThisTechExt = TechnoTypeExtContainer::Instance.TryFind(pThis);

	if (!pThatTechExt || !pThisTechExt)
		return false;

	if (GeneralUtils::IsValidString(pThatTechExt->GroupAs.c_str())) return IS_SAME_STR_(pThis->ID, pThatTechExt->GroupAs.c_str());
	else if (GeneralUtils::IsValidString(pThisTechExt->GroupAs.c_str())) return  IS_SAME_STR_(pThat->ID, pThisTechExt->GroupAs.c_str());

	return false;
}

#include <TaskForceClass.h>

void FakeTeamClass::_TeamClass_6EA080()
{
	FootClass* Member = this->FirstUnit;
	this->IsMoving = true;
	this->IsHasBeen = true;
	for (this->IsUnderStrength = false; Member; Member = Member->NextTeamMember) {
		if (this->IsReforming || this->IsForcedActive) {
			Member->IsTeamLeader = 1;
		}
	}

	this->CurrentScript->ClearMission();
	this->StepCompleted = 1;
}

void FakeTeamClass::_TMission_Guard(ScriptActionNode* nNode, bool arg3)
{
	if (arg3) {
		this->GuardAreaTimer.Start(nNode->Argument * 15);
	}

	this->_CoordinateRegroup();

	if (!this->GuardAreaTimer.IsTicking() || this->GuardAreaTimer.GetTimeLeft() <= 0)
		this->StepCompleted = true;

	//Debug::LogInfo("Script {} GuardAreaTimer Left {}", this->CurrentScript->Type->ID, );
}

void FakeTeamClass::_AI()
{
	HouseExtContainer::HousesTeams[this->Owner].emplace(this);

	if (this->IsSuspended)
	{
		int Started = this->SuspendTimer.StartTime;
		int DelayTime = this->SuspendTimer.TimeLeft;
		bool shouldUnsuspend = false;

		if (Started != -1)
		{
			if (Unsorted::CurrentFrame() - Started >= DelayTime)
			{
				shouldUnsuspend = true;
			}
			else
			{
				DelayTime -= Unsorted::CurrentFrame() - Started;
			}
		}

		if (DelayTime == 0)
		{
			shouldUnsuspend = true;
		}

		if (shouldUnsuspend)
		{
			this->IsSuspended = 0;
		}
		else
		{
			return; // Still suspended, exit early
		}
	}

	if (this->NeedsToDisappear && !this->Reacalculate())
	{
		return;
	}

	// Handle team movement and strength logic
	bool shouldProcessMovement = true;
	if (!this->IsMoving)
	{
		shouldProcessMovement = false;
	}
	else if (this->IsUnderStrength)
	{
		this->Regroup();
		if (!this->IsMoving)
		{
			shouldProcessMovement = false;
		}
	}

	if (!shouldProcessMovement && (this->IsFullStrength || this->IsForcedActive))
	{
		this->_TeamClass_6EA080();
	}

	if (this->IsReforming || this->IsMoving || !this->Zone || !this->ClosestMember)
	{
		this->CalCulateCenter(&this->Zone, &this->ClosestMember);
	}

	if ((!this->IsMoving || (!this->IsFullStrength && this->Type->Reinforce))
		&& (!this->Owner->IsControlledByHuman() || !this->IsHasBeen))
	{

		int v5 = 0;
		TaskForceClass* TaskForce = this->Type->TaskForce;

		if (TaskForce->CountEntries > 0)
		{
			int* Quantity = this->CountObjects;
			do
			{

				if (*Quantity < TaskForce->Entries[v5].Amount)
				{
					this->Recuit(v5);
				}
				++v5;
				++Quantity;
			}
			while (v5 < TaskForce->CountEntries);
		}
	}

	FootClass* v8 = this->FirstUnit;
	bool hasValidMember = false;

	if (!v8 ) {
		if (this->IsHasBeen ||
				(SessionClass::Instance->GameMode != GameMode::Campaign && Unsorted::CurrentFrame() - this->CreationFrame > RulesClass::Instance->DissolveUnfilledTeamDelay))
		{
			if (this->IsLeavingMap)
			{
				for (int i = 0; i < TagClass::ActiveTags->Count; ++i)
				{
					if (TagClass::ActiveTags->Items[i]->SpringEvent(TriggerEvent::TeamLeavesMap, nullptr, CellStruct::Empty))
					{
						--i;
						if (!TagClass::ActiveTags->Count)
							break;
					}
				}
			}

			((TeamClass*)this)->~TeamClass();
			return;
		}
	}
	else
	{
		hasValidMember = v8->IsNotWarping();
	}

	if (!this->IsMoving)
	{
		this->CoordinateMove();
		return;
	}

	if (this->IsReforming || this->IsUnderStrength || !hasValidMember)
	{
		this->IsReforming = !this->_CoordinateRegroup();
		return;
	}

	bool arg4 = false;
	if (this->StepCompleted)
	{
		arg4 = true;
		this->StepCompleted = false;
		this->CurrentScript->NextMission();

		for (FootClass* j = this->FirstUnit; j; j = j->NextTeamMember)
		{
			if (this->Type->TransportsReturnOnUnload && j->GetTechnoType()->Passengers > 0)
			{
				if (j->ArchiveTarget)
				{
					Debug::Log("A Transport just received orders to go home after unloading\n");
				}
			}
			else
			{
				j->SetArchiveTarget(nullptr);
			}
		}

		if (!this->CurrentScript->HasMissionsRemaining())
		{
			if (this)
			{
				((TeamClass*)this)->~TeamClass();
			}
			return;
		}

		this->AssignMissionTarget(nullptr);
		this->ArchiveTarget = nullptr;
	}
	else if (!this->ArchiveTarget)
	{
		this->ArchiveTarget = this->QueuedFocus;
	}

	if (this->TargetNotAssigned)
	{
		arg4 = true;
		this->TargetNotAssigned = false;
		this->AssignMissionTarget(nullptr);
		this->ArchiveTarget = 0;
	}

	auto pTeamData = this->_GetExtData();

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = this->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			this->ArchiveTarget = nullptr;
			this->QueuedFocus = nullptr;
			const auto nextAction = pScript->GetNextAction();
			Debug::LogInfo("DEBUG: [{}] {}](line: {} = {},{}): Jump to the same line -> (Reason: Timed Jump loop)",
				this->Type->ID,
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
				this->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission, (int)curAct, curArgs,
				pScript->CurrentMission + 1, (int)nextAct, nextArgs
			);
		}

		for (auto pUnit = this->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (ScriptExtData::IsUnitAvailable(pUnit, true))
			{
				pUnit->EnterIdleMode(false, 1);
			}
		}

		this->StepCompleted = true;
		return;
	}

	ScriptActionNode node {};
	this->CurrentScript->GetCurrentAction(&node);

	//dont prematurely finish the `Script` ,...
	//bailout the script if the `Action` already -1
	//this will free the Member and allow them to be recuited
	if ((TeamMissionType)node.Action >= TeamMissionType::count && (AresScripts)node.Action >= AresScripts::count && (PhobosScripts)node.Action >= PhobosScripts::count)
	{
		// Unknown action. This action finished
		this->StepCompleted = true;
		this->NeedsToDisappear = true;

		auto const pAction = this->CurrentScript->GetCurrentAction();
		Debug::LogInfo("AI Scripts : [{}] Team [{}][{}]  ( {} CurrentScript {} / {} line {}): Unknown Script Action: {} (action={}, arg={})",
			(void*)this,
			this->Type->ID,
			this->Type->Name,

			(void*)this->CurrentScript,
			this->CurrentScript->Type->ID,
			this->CurrentScript->Type->Name,
			this->CurrentScript->CurrentMission,

			(int)pAction.Action, (int)node.Action, node.Argument);

		((TeamClass*)this)->~TeamClass();
		return;
	}

	if (AresScriptExt::Handle(this, &node, arg4))
		return;

	if (ScriptExtData::ProcessScriptActions(this, &node, arg4))
		return;

#define fillTMission(miss) \
	case TeamMissionType::## miss: { \
		this->TMission_## miss ##(&node, arg4);\
		break;\
	}

	switch (node.Action)
	{
		fillTMission(Attack)
			fillTMission(Att_waypt)
			fillTMission(Go_bezerk)
			fillTMission(Move)
			fillTMission(Movecell)
	case TeamMissionType::Guard:
		{
			this->_TMission_Guard(&node, arg4);
			break;

		}
			fillTMission(Loop)
			fillTMission(Player_wins)
			fillTMission(Unload)
			fillTMission(Deploy)
			fillTMission(Hound_dog)
			fillTMission(Do)
			fillTMission(Set_global)
			fillTMission(Idle_anim)
			fillTMission(Load)
			fillTMission(Spy)
			fillTMission(Patrol)
			fillTMission(Change_script)
			fillTMission(Change_team)
			fillTMission(Panic)
			fillTMission(Change_house)
			fillTMission(Scatter)
			fillTMission(Goto_nearby_shroud)
			fillTMission(Player_loses)
			fillTMission(Play_speech)
			fillTMission(Play_sound)
			fillTMission(Play_movie)
			fillTMission(Play_music)
			fillTMission(Reduce_tiberium)
			fillTMission(Begin_production)
			fillTMission(Fire_sale)
			fillTMission(Self_destruct)
			fillTMission(Ion_storm_start_in)
			fillTMission(Ion_storn_end)
			fillTMission(Center_view_on_team)
			fillTMission(Reshroud_map)
			fillTMission(Reveal_map)
			fillTMission(Delete_team_members)
			fillTMission(Clear_global)
			fillTMission(Set_local)
			fillTMission(Clear_local)
			fillTMission(Unpanic)
			fillTMission(Force_facing)
			fillTMission(Wait_till_fully_loaded)
			fillTMission(Truck_unload)
			fillTMission(Truck_load)
			fillTMission(Attack_enemy_building)
			fillTMission(Moveto_enemy_building)
			fillTMission(Scout)
			fillTMission(Success)
			fillTMission(Flash)
			fillTMission(Play_anim)
			fillTMission(Talk_bubble)
			fillTMission(Gather_at_enemy)
			fillTMission(Gather_at_base)
			fillTMission(Iron_curtain_me)
			fillTMission(Chrono_prep_for_abwp)
			fillTMission(Chrono_prep_for_aq)
			fillTMission(Move_to_own_building)
			fillTMission(Attack_building_at_waypoint)
			fillTMission(Enter_grinder)
			fillTMission(Occupy_tank_bunker)
			fillTMission(Enter_bio_reactor)
			fillTMission(Occupy_battle_bunker)
			fillTMission(Garrison_building)
	default:
		break;
	}

#undef fillTMission
}

#include <Misc/Hooks.Otamaa.h>

NOINLINE int GetStrayDistanceForMission(ScriptClass* script)
{
	auto[TMission, arg] = script->GetCurrentAction();

	if (TMission == TeamMissionType::Gather_at_base || TMission == TeamMissionType::Gather_at_enemy) {
		return RulesClass::Instance->RelaxedStray;
	}

	return RulesClass::Instance->Stray;
}

bool FakeTeamClass::_CoordinateRegroup()
{
	bool allMembersRegrouped = true;
	FootClass* Member = this->FirstUnit;

	// If no members exist, mark as regrouped and return true
	if (!Member)
	{
		this->NeedsReGrouping = 0;
		return true;
	}

	int RelaxedStrayDistance = GetStrayDistanceForMission(this->CurrentScript);

	// Process each member in the team
	do
	{
		if (Member->IsAlive)
		{
			// Check if member is valid and ready for regrouping
			if (Member->Health && (Unsorted::ScenarioInit || !Member->InLimbo) && !Member->IsTeamLeader) {
				// Check if member is close enough to initiate
				if (((FakeObjectClass*)Member)->_GetDistanceOfObj(this->Zone) <= RelaxedStrayDistance)
				{
					Member->IsTeamLeader = 1;
				}
				else if (!Member->Destination)
				{
					// Send member to regroup location
					Member->QueueMission(Mission::Move, 0);
					Member->SetTarget(nullptr);
					Member->SetDestination(this->Zone, 1);
				}
			}

			// Process initiated members or aircraft
			if (Member->IsAlive
				&& Member->Health
				&& (Unsorted::ScenarioInit || !Member->InLimbo)
				&& (Member->IsTeamLeader || Member->WhatAmI() == AbstractType::Aircraft))
			{
				// Check if member is in position or guarding with target
				if (((FakeObjectClass*)Member)->_GetDistanceOfObj(this->Zone) <= RelaxedStrayDistance
					|| (Member->GetCurrentMission()  == Mission::Area_Guard && Member->Target))
				{
					// Set to guard mission if not already guarding
					if (Member->GetCurrentMission() != Mission::Area_Guard)
					{
						Member->QueueMission(Mission::Guard, false);
						Member->SetDestination(0, 1);
					}
				}
				else if (!Member->Destination)
				{
					// Member needs to move to regroup - mark regrouping as incomplete
					allMembersRegrouped = false;
					Member->QueueMission(Mission::Move, 0);

					// Calculate destination coordinates
					CoordStruct centerCoord = this->Zone->GetCoords();
					CellStruct targetCell = CellClass::Coord2Cell(centerCoord);
					Member->SetDestination(MapClass::Instance->GetCellAt(targetCell), 1);
				}
			}
		}
		Member = Member->NextTeamMember;
	}
	while (Member);

	// If all members are regrouped, clear the regrouping flag
	if (allMembersRegrouped)
	{
		this->NeedsReGrouping = 0;
	}

	return allMembersRegrouped;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F478C, FakeTeamClass::_AI)
DEFINE_FUNCTION_JUMP(CALL, 0x6ED7A2 , FakeTeamClass::_CoordinateRegroup)

ASMJIT_PATCH(0x55B4F5, LogicClass_Update_Teams, 0x6)
{
	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		TeamClass::Array->Items[i]->Update();
	}

	//if(Phobos::Otamaa::IsAdmin){
	//	for (auto& [house, vec] : HouseExtContainer::HousesTeams) {
	//		Debug::LogInfo("House {} - {} has {} valid Teams!", (void*)house, house->Type->ID, vec.size());
	//	}
	//}

	return 0x55B5A1;
}

// =============================
// load / save

template <typename T>
void TeamExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)

		.Process(this->WaitNoTargetAttempts)
		.Process(this->NextSuccessWeightAward)
		.Process(this->IdxSelectedObjectFromAIList)
		.Process(this->CloseEnough)
		.Process(this->Countdown_RegroupAtLeader)
		.Process(this->MoveMissionEndMode)
		.Process(this->WaitNoTargetTimer)
		.Process(this->ForceJump_Countdown)
		.Process(this->ForceJump_InitialCountdown)
		.Process(this->ForceJump_RepeatMode)
		.Process(this->TeamLeader, true)

		.Process(this->LastFoundSW, true)

		.Process(this->ConditionalJump_Evaluation)
		.Process(this->ConditionalJump_ComparatorMode)
		.Process(this->ConditionalJump_ComparatorValue)
		.Process(this->ConditionalJump_Counter)
		.Process(this->ConditionalJump_Index)
		.Process(this->AbortActionAfterKilling)
		.Process(this->ConditionalJump_EnabledKillsCount)
		.Process(this->ConditionalJump_ResetVariablesIfJump)

		.Process(this->TriggersSideIdx)
		.Process(this->TriggersHouseIdx)

		.Process(this->AngerNodeModifier)
		.Process(this->OnlyTargetHouseEnemy)
		.Process(this->OnlyTargetHouseEnemyMode)

		.Process(this->PreviousScript)
		.Process(this->BridgeRepairHuts)
		;
}

// =============================
// container
TeamExtContainer TeamExtContainer::Instance;

// =============================
// container hooks

//Everything InitEd
ASMJIT_PATCH(0x6E8D05, TeamClass_CTOR, 0x5)
{
	GET(TeamClass*, pThis, ESI);
	TeamExtContainer::Instance.Allocate(pThis);
	return 0;
}

ASMJIT_PATCH(0x6E8ECB, TeamClass_DTOR, 0x7)
{
	GET(TeamClass*, pThis, ESI);
	HouseExtContainer::HousesTeams[pThis->Owner].erase(pThis);
	TeamExtContainer::Instance.Remove(pThis);
	return 0;
}

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeTeamClass::_Load(IStream* pStm)
{

	TeamExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TeamClass::Load(pStm);

	if (SUCCEEDED(res))
		TeamExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeTeamClass::_Save(IStream* pStm, bool clearDirty)
{

	TeamExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TeamClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		TeamExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4744, FakeTeamClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4748, FakeTeamClass::_Save)

ASMJIT_PATCH(0x6EAE60, TeamClass_Detach, 0x7)
{
	GET(TeamClass*, pThis, ECX);
	GET_STACK(AbstractClass*, target, 0x4);
	GET_STACK(bool, all, 0x8);

	TeamExtContainer::Instance.InvalidatePointerFor(pThis, target, all);

	//return pThis->Target == target ? 0x6EAECC : 0x6EAECF;
	return 0x0;
}

//void __fastcall TeamClass_Detach_Wrapper(TeamClass* pThis ,DWORD , AbstractClass* target , bool all)\
//{
//	TeamExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//	pThis->TeamClass::PointerExpired(target , all);
//}
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4758, GET_OFFSET(TeamClass_Detach_Wrapper))