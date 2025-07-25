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

void FakeTeamClass::_AssignMissionTarget(AbstractClass* new_target)
{
	// If the new target is different than current mission target
	if (new_target != this->QueuedFocus)
	{
		FootClass* unit = this->FirstUnit;

		if (this->QueuedFocus)
		{
			while (unit)
			{
				FootClass* oldTarget = static_cast<FootClass*>(this->QueuedFocus);

				bool navMatch = (unit->Destination == oldTarget);
				bool tarMatch = (unit->Target == oldTarget);

				if (navMatch || tarMatch)
				{
					// Put unit into guard mode so it's easier to switch missions
					unit->QueueMission(Mission::Guard, false);

					if (navMatch)
						unit->SetDestination(nullptr, true);

					if (tarMatch)
						unit->SetTarget(nullptr);
				}
				unit = unit->NextTeamMember;
			}
		}
	}

	// If Target was linked to previous MissionTarget or is null, update Target as well
	if (this->ArchiveTarget == this->QueuedFocus || !this->ArchiveTarget)
	{
		this->ArchiveTarget = new_target;
	}

	this->QueuedFocus = new_target;

	// If new target is a CellClass (special case for map cells)
	if (new_target && new_target->WhatAmI() == CellClass::AbsID)
	{
		CellClass* cell = static_cast<CellClass*>(new_target);

		if (MapClass::Instance->IsWithinUsableArea(cell, true))
		{
			this->IsLeavingMap = false;
		}
		else
		{
			this->IsLeavingMap = true;

			// Clear destinations for all team members
			FootClass* member = this->FirstUnit;
			while (member)
			{
				member->SetDestination(nullptr, true);
				member = member->NextTeamMember;
			}
		}
	}
}

void FakeTeamClass::_TMission_GatherAtBase(ScriptActionNode* nNode, bool arg3)
{
	if (arg3) {
		FootClass* member = this->FirstUnit;
		TechnoClass* bestLeader = nullptr;
		int bestLeadership = -1;

		// Find best leader
		while (member)
		{
			TechnoTypeClass* type = member->GetTechnoType();
			int leadership = type->LeadershipRating;

			if (member->IsThisBreathing() &&
				(member->IsTeamLeader || type->WhatAmI() == AircraftClass::AbsID) &&
				leadership > bestLeadership)
			{
				bestLeader = member;
				bestLeadership = leadership;
			}
			member = member->NextTeamMember;
		}

		if (!bestLeader) {
			this->StepCompleted = true;
			return;
		}

		HouseClass* enemyHouse = nullptr;
		if (bestLeader->Owner->EnemyHouseIndex != -1) {
			enemyHouse = HouseClass::Array->Items[bestLeader->Owner->EnemyHouseIndex];
		}

		CoordStruct baseCenter { };
		this->OwnerHouse->GetBaseCenterCoords(&baseCenter);
		int baseX = baseCenter.X;
		int baseY = baseCenter.Y;

		double angleRad = 0.0;
		if (enemyHouse)
		{
			CoordStruct enemyBase {};
			enemyHouse->GetBaseCenterCoords(&enemyBase);
			int dx = enemyBase.X - baseX;
			int dy = enemyBase.Y - baseY;

			angleRad = Math::atan2((double)-dy, (double)dx) - Math::DEG90_AS_RAD;
		}
		else
		{
			int randomAngle = ScenarioClass::Instance->Random.RandomFromMax(256);
			angleRad = ((randomAngle - 127) * 256) * Math::BINARY_ANGLE_MAGIC; // assuming BINARY_ANGLE_MAGIC == 256 and angle centered at 0
		}

		int safeDistance = (RulesExtData::Instance()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance) + nNode->Argument) << 8;
		double distance = static_cast<double>(safeDistance);
		double targetX = baseX + Math::cos(angleRad) * distance;
		double targetY = baseY - Math::sin(angleRad) * distance;

		CellStruct targetCell { (short)(static_cast<int>(targetX) / 256), (short)(static_cast<int>(targetY) / 256) };

		TechnoTypeClass* leaderType = bestLeader->GetTechnoType();
		CellStruct finalCell = MapClass::Instance->NearByLocation(
			targetCell,
			leaderType ? leaderType->SpeedType : SpeedType::Track,
			ZoneType::None, MovementZone::Normal,
			0, 3, 3, 0, 0, 0, 1, CellStruct::Empty, 0, 0
		);

		this->AssignMissionTarget(MapClass::Instance->GetCellAt(finalCell));
	}
	this->CoordinateMove();
}

void FakeTeamClass::_TMission_GatherAtEnemy(ScriptActionNode* nNode, bool arg3)
{
	if (arg3)
	{
		FootClass* member = this->FirstUnit;
		TechnoClass* bestLeader = nullptr;
		int bestLeadership = -1;

		// Find best leader
		while (member)
		{
			TechnoTypeClass* type = member->GetTechnoType();
			int leadership = type->LeadershipRating;

			if (member->IsThisBreathing() &&
				(member->IsTeamLeader || type->WhatAmI() == AircraftClass::AbsID) &&
				leadership > bestLeadership)
			{
				bestLeader = member;
				bestLeadership = leadership;
			}
			member = member->NextTeamMember;
		}

		if (!bestLeader) {
			this->StepCompleted = true;
			return;
		}

		int enemyID = bestLeader->Owner->EnemyHouseIndex;
		if (enemyID == -1) {
			this->StepCompleted = true;
			return;
		}

		HouseClass* enemyHouse = HouseClass::Array->Items[enemyID];
		CoordStruct enemyBase {};
		enemyHouse->GetBaseCenterCoords(&enemyBase);

		if (enemyBase == CoordStruct::Empty) {
			this->StepCompleted = true;
			return;
		}

		// Get own base center, fallback to unit center if no base
		CoordStruct ownBase {};
		this->OwnerHouse->GetBaseCenterCoords(&ownBase);
		if (ownBase == CoordStruct::Empty) {
			bestLeader->Owner->GetBaseCenterCoords(&ownBase);
		}

		int dx = ownBase.X - enemyBase.X;
		int dy = ownBase.Y - enemyBase.Y;

		double angleRad = Math::atan2((double)dy, (double)dx) - Math::DEG90_AS_RAD;

		int safeDistance = (RulesClass::Instance->AISafeDistance + nNode->Argument) << 8;
		double distance = static_cast<double>(safeDistance);

		double targetX = enemyBase.X + Math::cos(angleRad) * distance;
		double targetY = enemyBase.Y - Math::sin(angleRad) * distance;

		CellStruct targetCell { (short)(static_cast<int>(targetX) / 256) ,  (short)(static_cast<int>(targetY) / 256) };

		TechnoTypeClass* leaderType = bestLeader->GetTechnoType();
		CellStruct finalCell = MapClass::Instance->NearByLocation(
			targetCell,
			leaderType->SpeedType,
			ZoneType::None, MovementZone::Normal,
			0, 3, 3, 0, 0, 0, 1, CellStruct::Empty, 0, 0
		);

		this->AssignMissionTarget(MapClass::Instance->GetCellAt(finalCell));

		Debug::LogInfo("[{}][{}] Team with Owner '{}' has chosen ({} , {}) for its GatherAtEnemy cell.",
			(void*)this, this->Type->ID, bestLeader->Owner ? bestLeader->Owner->get_ID() : GameStrings::NoneStrb(), finalCell.X, finalCell.Y);

	}

	this->CoordinateMove();
}

void FakeTeamClass::_TMission_ChangeHouse(ScriptActionNode* nNode, bool arg3)
{
	if (FootClass* Member = this->FirstUnit)
	{
		const auto pHouse = HouseClass::FindByCountryIndex(nNode->Argument);
		if (!pHouse) {
			const auto nonestr = GameStrings::NoneStr();
			Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]",
				this->OwnerHouse ? this->OwnerHouse->get_ID() : nonestr, this->OwnerHouse,
				this->get_ID(), this, nNode->Argument);
		} else{

			FootClass* nextTeam = nullptr;

			do
			{
				nextTeam = Member->NextTeamMember;
				Member->SetOwningHouse(pHouse, 1);
				Member = nextTeam;
			}
			while (nextTeam);

		}
	}

	this->StepCompleted = true;
}

static NOINLINE int GetStrayDistanceForMission(ScriptClass* script)
{
	auto [TMission, arg] = script->GetCurrentAction();

	if (TMission == TeamMissionType::Gather_at_base || TMission == TeamMissionType::Gather_at_enemy)
	{
		return RulesClass::Instance->RelaxedStray;
	}

	return RulesClass::Instance->Stray;
}

void FakeTeamClass::_CoordinateMove()
{
 //TODO ///
}

void FakeTeamClass::_AI()
{
	HouseExtContainer::HousesTeams[this->OwnerHouse].emplace(this);

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
		&& (!this->OwnerHouse->IsControlledByHuman() || !this->IsHasBeen))
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

	if (!v8)
	{
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

	switch (node.Action)
	{
	case TeamMissionType::Guard:
	{
		this->_TMission_Guard(&node, arg4);
		return;
	}
	case TeamMissionType::Change_house:
	{
		this->_TMission_ChangeHouse(&node, arg4);
		return;
	}
	case TeamMissionType::Gather_at_enemy:
	{
		this->_TMission_GatherAtEnemy(&node, arg4);
		return;
	}
	case TeamMissionType::Gather_at_base:
	{
		this->_TMission_GatherAtBase(&node, arg4);
		return;
	}
	default:

		if (AresScriptExt::Handle(this, &node, arg4) || ScriptExtData::ProcessScriptActions(this, &node, arg4))
			return;

		break;
	}

	this->ExecuteTMission(node.Action, &node, arg4);
}

#include <Misc/Hooks.Otamaa.h>

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
				if (FakeObjectClass::_GetDistanceOfObj(Member , discard_t() , this->Zone) <= RelaxedStrayDistance)
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
				if (FakeObjectClass::_GetDistanceOfObj(Member, discard_t(), this->Zone) <= RelaxedStrayDistance
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
DEFINE_FUNCTION_JUMP(CALL, 0x6ED7A2, FakeTeamClass::_CoordinateRegroup)

//DEFINE_FUNCTION_JUMP(LJMP, 0x6EBAD0 , FakeTeamClass::_CoordinateMove)
//
//DEFINE_FUNCTION_JUMP(CALL, 0x6EA3B5, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC75A, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC7BD, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC98B, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECE46, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EDB68, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE5AC, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE7EA, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EEB96, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EF9D2, FakeTeamClass::_CoordinateMove)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EFC62, FakeTeamClass::_CoordinateMove)
//

DEFINE_FUNCTION_JUMP(LJMP, 0x6E9050 , FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x4153F1, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x416EF5, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x417395, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6E93F4, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6E942A, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6E959F, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC3A5, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC753, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC7B6, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC889, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EC984, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECA05, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECA44, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECAE3, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECB22, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECD10, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECD3B, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECE22, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECE84, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECEDE, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ECEFC, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6ED167, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE383, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE3C2, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE591, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EE7C7, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EEA8F, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EEAA0, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EF9AB, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6EFC59, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6F0070, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6F0315, FakeTeamClass::_AssignMissionTarget)
//DEFINE_FUNCTION_JUMP(CALL, 0x6FF911, FakeTeamClass::_AssignMissionTarget)


//ASMJIT_PATCH(0x55B4F5, LogicClass_Update_Teams, 0x6)
//{
//	for (int i = 0; i < TeamClass::Array->Count; ++i)
//	{
//		TeamClass::Array->Items[i]->Update();
//	}
//
//	//if(Phobos::Otamaa::IsAdmin){
//	//	for (auto& [house, vec] : HouseExtContainer::HousesTeams) {
//	//		Debug::LogInfo("House {} - {} has {} valid Teams!", (void*)house, house->Type->ID, vec.size());
//	//	}
//	//}
//
//	return 0x55B5A1;
//}

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
StaticObjectPool<TeamExtData, 10000> TeamExtContainer::pools;
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
	HouseExtContainer::HousesTeams[pThis->OwnerHouse].erase(pThis);
	TeamExtContainer::Instance.Remove(pThis);
	return 0;
}

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