#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Script/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Hooks.Otamaa.h>
#include <Misc/Ares/Hooks/Header.h>

#include <AITriggerTypeClass.h>
#include <TaskForceClass.h>
#include <TubeClass.h>

#pragma region ExtFuncs

bool TeamExtData::IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement)
{
#ifdef _Use
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pGoing->GetTechnoType());

	if (pTypeExt->RecuitedAs.isset() && pTypeExt->RecuitedAs == reinfocement)
		return true;

	if (TechnoExtContainer::Instance.Find(pGoing)->Type == reinfocement)
		return true;
#endif

	return false;
}

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
		if (pHouse->Type->SideIndex == RulesExtData::Instance()->CivilianSideIndex)
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

		if (pHouse->Type->SideIndex == RulesExtData::Instance()->CivilianSideIndex)
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

#pragma endregion

#pragma region Backports

void FakeTeamClass::_TMission_Guard(ScriptActionNode* nNode, bool arg3)
{
	if (arg3)
	{
		this->GuardAreaTimer.Start(nNode->Argument * 15);
	}

	this->_CoordinateRegroup();

	if (!this->GuardAreaTimer.IsTicking() || this->GuardAreaTimer.GetTimeLeft() <= 0)
		this->StepCompleted = true;

	//Debug::LogInfo("Script {} GuardAreaTimer Left {}", this->CurrentScript->Type->ID, );
}

void FakeTeamClass::_TMission_GatherAtBase(ScriptActionNode* nNode, bool arg3)
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

		if (!bestLeader)
		{
			this->StepCompleted = true;
			return;
		}

		HouseClass* enemyHouse = nullptr;
		if (bestLeader->Owner->EnemyHouseIndex != -1)
		{
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

		this->_AssignMissionTarget(MapClass::Instance->GetCellAt(finalCell));
	}
	this->_CoordinateMove();
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

		if (!bestLeader)
		{
			this->StepCompleted = true;
			return;
		}

		int enemyID = bestLeader->Owner->EnemyHouseIndex;
		if (enemyID == -1)
		{
			this->StepCompleted = true;
			return;
		}

		HouseClass* enemyHouse = HouseClass::Array->Items[enemyID];
		CoordStruct enemyBase {};
		enemyHouse->GetBaseCenterCoords(&enemyBase);

		if (enemyBase == CoordStruct::Empty)
		{
			this->StepCompleted = true;
			return;
		}

		// Get own base center, fallback to unit center if no base
		CoordStruct ownBase {};
		this->OwnerHouse->GetBaseCenterCoords(&ownBase);
		if (ownBase == CoordStruct::Empty)
		{
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

		this->_AssignMissionTarget(MapClass::Instance->GetCellAt(finalCell));

		Debug::LogInfo("[{}][{}] Team with Owner '{}' has chosen ({} , {}) for its GatherAtEnemy cell.",
			(void*)this, this->Type->ID, bestLeader->Owner ? bestLeader->Owner->get_ID() : GameStrings::NoneStrb(), finalCell.X, finalCell.Y);

	}

	this->_CoordinateMove();
}

void FakeTeamClass::_TMission_ChangeHouse(ScriptActionNode* nNode, bool arg3)
{
	if (FootClass* Member = this->FirstUnit)
	{
		const auto pHouse = HouseClass::FindByCountryIndex(nNode->Argument);
		if (!pHouse)
		{
			const auto nonestr = GameStrings::NoneStr();
			Debug::FatalErrorAndExit("[%s - %x] Team [%s - %x] ChangeHouse cannot find House by country idx [%d]",
				this->OwnerHouse ? this->OwnerHouse->get_ID() : nonestr, this->OwnerHouse,
				this->get_ID(), this, nNode->Argument);
		}
		else
		{

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

bool FakeTeamClass::_Add(FootClass* obj){
	return this->_Add2(obj, 1);
}

bool FakeTeamClass::_Add2(FootClass* member, bool ignoreQuantity) {
	if (!member)
		return false;

	// Check if unit can be added (reuses outTypeIndex as storage)
	int typeIndex;
	if (!this->_Can_Add(member, &typeIndex, ignoreQuantity))
		return false;

	// Remove from previous team if needed
	if (FakeTeamClass* pTeam = (FakeTeamClass*)member->Team) {
		pTeam->_Remove(member, -1, false);
	}

	// Update quantity count (unless ignoring quantity limits)
	if (!ignoreQuantity)
	{
		++this->CountObjects[typeIndex];
	}

	// Add to front of member linked list
	// First member becomes captain (initiated)
	member->IsTeamLeader = (this->FirstUnit == nullptr);
	member->NextTeamMember = this->FirstUnit;
	this->FirstUnit = member;
	member->Team = this;

	// Set group ID
	member->Group = this->Type->GetGroup();

	// Attach team trigger to member
	if (this->Tag)
	{
		// Only attach if not OnTransOnly, or if unit can carry passengers
		bool shouldAttachTrigger = !this->Type->OnTransOnly ||
			member->GetTechnoType()->Passengers > 0;

		if (shouldAttachTrigger) {
			member->AttachTrigger(this->Tag);
		}
	}

	// Update team statistics
	++this->TotalObjects;
	int memberRisk = member->GetThreatValue();
	this->TotalThreatValue += memberRisk;

	// Calculate team center if not set
	if (!this->Zone) {
		this->_Calc_Center(&this->Zone, &this->ClosestMember);
	}

	// Set member's AI recruitability based on team type
	member->RecruitableB = this->Type->AreTeamMembersRecruitable;

	// Mark team as altered for AI updates
	this->JustDisappeared = true;
	this->NeedsToDisappear = true;

	return true;
}

// Find the index of unit's type in the task force
int NOINLINE FindUnitTypeInTaskForce(TeamClass* team, FootClass* unit)
{
	TechnoTypeClass* unitType = unit->GetTechnoType();
	TaskForceClass* taskForce = team->Type->TaskForce;

	for (int i = 0; i < taskForce->CountEntries; i++)
	{
		if (taskForce->Entries[i].Type == unitType)
			return i;
	}

	return taskForce->CountEntries; // Not found
}

// Check detailed recruitability conditions
bool NOINLINE CanRecruitUnit(TeamClass* team, FootClass* unit, int typeIndex, bool ignoreQuantity, bool alwaysRecruit)
{
	// Must be AI recruitable for autocreate teams (unless always recruit)
	if (!unit->RecruitableB && team->Type->Autocreate && !alwaysRecruit)
		return false;

	auto pExt = TechnoExtContainer::Instance.Find(unit);

	// Cannot recruit if unit is trying something
	if (unit->To_Try_Something() || pExt->Is_DriverKilled)
		return false;

	// Cannot recruit if unit has drain target (mind control, etc)
	if (unit->DrainTarget)
		return false;

	// Cannot recruit if unit is in a bunker
	if (unit->BunkerLinkedItem)
		return false;

	// Check recruit priority - can steal from lower priority teams
	if (auto pUnitTeam = unit->Team)
	{
		if (pUnitTeam->Type->Priority >= team->Type->Priority)
			return false;
	}

	// Aircraft with weapons must have ammo
	AbstractType unitKind = unit->WhatAmI();
	if (unitKind == AbstractType::Aircraft) {
		if (unit->IsArmed() && unit->Ammo <= 0) {
			return false;
		}
	}

	// Check if we need more of this type (unless ignoring quantity)
	if (!ignoreQuantity && team->CountObjects[typeIndex] >= team->Type->TaskForce->Entries[typeIndex].Amount) {
			return false;
	}

	// Cannot recruit if unit has locomotor source (being hijacked/mind controlled)
	if (unit->LocomotorSource)
		return false;

	return true;
}

bool FakeTeamClass::_Can_Add(FootClass* unit, int* outTypeIndex, bool ignoreQuantity) {

	// Basic validation checks
	if (!unit || !unit->IsAlive || !unit->Health || unit->IsCrashing || unit->IsSinking)
		return false;

	if (!Unsorted::ScenarioInit && unit->InLimbo)
		return false;

	// Unit already in this team
	if (this == unit->Team)
		return false;

	// Must be same house
	if (unit->Owner != this->OwnerHouse)
		return false;

	// Check if unit is in radio contact
	if (unit->HasAnyLink()) {
		// Exception: Crate goodie aircraft can be recruited even when in radio contact
		if (unit->WhatAmI() != AbstractType::Aircraft || !((AircraftClass*)unit)->Type->AirportBound)
			return false;
	}

	// Find the type index in the task force
	*outTypeIndex = FindUnitTypeInTaskForce(this, unit);

	// If type not found and we're strict about it, reject
	if (*outTypeIndex == this->Type->TaskForce->CountEntries && !ignoreQuantity)
		return false;

	// Check if unit's mission is recruitable
	Mission currentMission = unit->GetCurrentMission();
	if (currentMission != Mission::None && !MissionClass::IsRecruitableMission(currentMission))
		return false;

	// Check recruitability flags
	if (!unit->RecruitableA && !this->Type->Autocreate)
		return false;

	// Special cases that always allow recruitment
	bool alwaysRecruit = false;

	// UNLOAD mission teams can recruit anything
	if (auto pScript = this->CurrentScript)
	{
		auto const& [mission, val] = pScript->GetCurrentAction();
		if (mission == TeamMissionType::Unload)
			alwaysRecruit = true;
	}

	// Aircraft are always recruitable
	if (unit->WhatAmI() == AircraftClass::AbsID)
		alwaysRecruit = true;

	// Complex recruitability check
	if (!CanRecruitUnit(this, unit, *outTypeIndex, ignoreQuantity, alwaysRecruit))
		return false;

	return true;
}

// Remove member from the linked list and return whether any initiated member remains
bool NOINLINE RemoveMemberFromChain(TeamClass* team, FootClass* memberToRemove)
{
	FootClass* current = team->FirstUnit;
	FootClass* previous = nullptr;
	bool HasTeamLeader = false;
	bool memberFound = false;

	while (current)
	{
		if (current == memberToRemove)
		{
			// Unlink the member from the chain
			if (previous)
			{
				previous->NextTeamMember = current->NextTeamMember;
			}
			else
			{
				team->FirstUnit = current->NextTeamMember;
			}

			// Clear member's team data
			FootClass* nextMember = current->NextTeamMember;
			current->NextTeamMember = nullptr;
			current->Team = nullptr;
			current->SuspendedMission = Mission::None;
			current->LastDestination = nullptr;
			current->LastTarget = nullptr;

			// Update team statistics
			--team->TotalObjects;
			int memberRisk = memberToRemove->GetThreatValue();
			team->TotalThreatValue -= memberRisk;

			memberFound = true;
			current = nextMember;
		}
		else
		{
			// Check if this member is initiated
			if (current->IsTeamLeader)
			{
				HasTeamLeader = true;
			}

			previous = current;
			current = current->NextTeamMember;
		}

		// Early exit if we found the member and confirmed there's an initiated member
		if (memberFound && HasTeamLeader)
			break;
	}

	return HasTeamLeader;
}

bool FakeTeamClass::_Remove(FootClass* obj, int typeindex, bool enterIdleMode) {
	TeamClass* Team = obj->Team;
	obj->removed = 1;
	// /*
	// **   Make sure that the object is in fact a member of this team. If not, then it can't
	// **   be removed. Return success because the end result is the same.
	// */
	if (this != Team) {
		return 1;
	}

	// /*
	// **   Detach the common trigger for this team type. Only current and active members of the
	// **   team have that trigger attached. The exception is for player team members that
	// **   get removed from a reinforcement team.
	// */
	if (obj->AttachedTag == this->Tag) {
		if (auto House = obj->Owner) {
			if (!House->IsControlledByHuman()) {
				obj->AttachTrigger(nullptr);
			}
		}
	}
	// /*
	// **   If the proper team index was not provided, then find it in the type type class. The
	// **   team type class will not be set if the appropriate type could not be found
	// **   for this object. This indicates that the object was illegally added. Continue to
	// **   process however, since removing this object from the team is a good idea.
	// */

	if(this->Type->TaskForce){
		if (typeindex == -1) {
			for (typeindex = 0; typeindex < this->Type->TaskForce->CountEntries; ++typeindex) {
				if (this->Type->TaskForce->Entries[typeindex].Type == obj->GetTechnoType())
					break;
			}
		}

		if (typeindex >= 0 && typeindex < this->Type->TaskForce->CountEntries) {
			--this->CountObjects[typeindex];
		}
	}

	// /*
	// **   Actually remove the object from the team. Scan through the team members
	// **   looking for the one that matches the one specified. If it is found, it
	// **   is unlinked from the member chain. During this scan, a check is made to
	// **   ensure that at least one remaining member is still initiated. If not, then
	// **   a new team captain must be chosen.
	// */
	bool hasInitiatedMember = RemoveMemberFromChain(this, obj);

	// Clear member's team reference if still set
	if (obj->Team) {
		obj->Team = nullptr;
	}

	// A unit that breaks off of a team will enter idle mode
	if (!Unsorted::ScenarioInit &&
		obj->IsAlive &&
		!obj->IsCrashing &&
		!obj->InLimbo &&
		!enterIdleMode)
	{
		obj->EnterIdleMode(0, 1);
	}

	// /*
	// **   If, after removing the team member, there are no initiated members left
	// **   in the team, then just make the first remaining member of the team the
	// **   team captain. Mark the center location of the team as invalid so that
	// **   it will be centered around the captain.
	// */
	if (!hasInitiatedMember)
	{
		if (FootClass* firstMember = this->FirstUnit) {
			firstMember->IsTeamLeader = 1;
			this->Zone = 0;
		}
	}
	// /*
	// **   Must record that the team composition has changed. At the next opportunity,
	// **   the team members will be counted and appropriate AI adjustments made.
	// */
	this->JustDisappeared = 1;
	this->NeedsToDisappear = 1;
	return 1;
}

// Determine if team should retaliate against attacker
bool NOINLINE ShouldRetaliateAgainstAttacker(TeamClass* team, ObjectClass* attacker)
{
	// If no current target, should retaliate
	if (!team->ArchiveTarget)
		return true;

	// Check if current target is a techno object
	TechnoClass* currentTarget = flag_cast_to<TechnoClass*>(team->ArchiveTarget);
	if (!currentTarget)
		return true;

	// If current target is unarmed, should retaliate
	if (!currentTarget->IsArmed())
		return true;

	// If team has no zone, should retaliate
	if (!team->Zone)
		return true;

	// Check if current target is in threat range of zone
	CoordStruct zoneCoord = team->Zone->GetCoords();
	if (!currentTarget->IsCloseEnoughToAttackCoords(zoneCoord))
	{
		// Current target is not threatening zone, should retaliate
		return true;
	}

	// Current target is threatening zone, keep current target
	return false;
}

void FakeTeamClass::_Took_Damage(FootClass* attacker, DamageState result, ObjectClass* source) {
	// Ignore if no attacker, no damage, or team is suicidal
	if (!attacker || result == DamageState::Unaffected || this->Type->Suicide)
		return;

	// Team is not moving - trigger immediate regrouping
	if (!this->IsMoving)
	{
		this->Zone = nullptr;
		this->NeedsReGrouping = true;
		this->IsReforming = true;
		return;
	}

	// Team is moving - check if attacker is a team member (friendly fire)
	if (this->_Is_A_Member(attacker))
		return;

	// Check if first member can retaliate
	FootClass* firstMember = this->FirstUnit;
	if (!firstMember)
		return;

	// Aircraft don't react to damage while moving
	if (firstMember->WhatAmI() == AircraftClass::AbsID)
		return;

	// Only armed units can retaliate
	if (!firstMember->IsArmed())
		return;

	// Don't change target if already attacking the attacker
	if (this->ArchiveTarget == attacker)
		return;

	// For annoyance teams, trigger regrouping when attacked
	if (this->Type->Annoyance)
	{
		this->Zone = nullptr;
		this->NeedsReGrouping = true;
		this->IsReforming = true;
	}

	// Consider changing target to the attacker
	if (ShouldRetaliateAgainstAttacker(this, attacker))
	{
		// Potentially change target (implementation seems incomplete in original)
		//attacker->a.vftable->t.r.m.o.a.Kind_Of(&attacker->a);
		Debug::FatalError("TamClass::Took_Damage function is seems incomplete calling this may result in wasted calculation !");
	}
}

// If target is a cell and leader is not aircraft, find an object in the cell
void NOINLINE RefineTargetIfCell(TeamClass* team, AbstractClass* leader)
{
	if (auto pTargetcell = cast_to<CellClass*>(team->ArchiveTarget)) {
		// Only refine for non-aircraft
		if (leader && leader->WhatAmI() == AircraftClass::AbsID)
			return;

		// Try to find an object in the cell
		if (ObjectClass* cellObject = pTargetcell->GetSomeObject(Point2D::Empty, 0)) {
			team->ArchiveTarget = cellObject;
		}
	}
}

// Check if leader can fire at target
void NOINLINE CheckLeaderCanFire(TeamClass* team, FootClass* leader)
{
	if (!leader)
		return;

	int weaponIndex = leader->SelectWeapon(team->ArchiveTarget);
	FireError fireError = leader->GetFireError(team->ArchiveTarget, weaponIndex , true);

	// FIRE_CANT (5) means unit cannot fire at target
	if (fireError == FireError::CANT)
	{
		team->TargetNotAssigned = true;
	}
}

// Initialize uninitiated member (get them to the zone)
void NOINLINE ProcessMemberInitiation(TeamClass* team, FootClass* member)
{
	if (!member->IsAlive || !member->Health)
		return;
	if (!Unsorted::ScenarioInit && member->InLimbo)
		return;
	if (member->IsTeamLeader)
		return;

	// Get stray distance based on mission type
	int strayDistance = ((FakeTeamClass*)team)->_Get_Stray();

	// Check if close enough to zone to be initiated
	if (member->DistanceFrom(team->Zone) <= strayDistance) {
		member->IsTeamLeader = true;
	}
	else if (!member->Destination) {
		// Send member to zone
		member->QueueMission(Mission::Move, 0);
		member->SetTarget(nullptr);
		member->SetDestination(team->Zone, 1);
	}
}

// Process member attack - returns true if member is active
bool NOINLINE ProcessMemberAttack(TeamClass* team, FootClass* member, ScriptActionNode* currentMission)
{
	// Check if member is valid
	if (!member->IsAlive)
	{
		// Special case: droppod teams count limbo units as active
		if (team->Type->DropPod && member->InLimbo)
			return true;

		return false;
	}

	if (!member->Health)
		return false;
	if (!Unsorted::ScenarioInit && member->InLimbo)
		return false;

	bool isAircraft = (member->WhatAmI() == AircraftClass::AbsID);
	if (!member->IsTeamLeader && !isAircraft)
		return false;

	// Check if this is an engineer capture mission
	bool isInfiltrate = (currentMission->Action == TeamMissionType::Spy) &&
		(member->WhatAmI() == InfantryClass::AbsID) &&
		((InfantryClass*)member)->Type->Infiltrate;

	if (isInfiltrate)
	{
		// Assign capture mission to engineer
		member->QueueMission(Mission::Capture, 0);
		member->SetTarget(team->ArchiveTarget);
	}
	else
	{
		// Check if member needs attack mission assigned
		Mission currentMissionType = member->GetCurrentMission();

		bool shouldAssignAttack = (currentMissionType != Mission::Attack &&
								   currentMissionType != Mission::Enter &&
								   currentMissionType != Mission::Capture &&
								   currentMissionType != Mission::Sabotage);

		// Exception: if unloading and has passengers, don't interrupt
		if (currentMissionType == Mission::Unload)
		{
			if (member->CanDeployNow())
				shouldAssignAttack = false;
		}

		if (shouldAssignAttack)
		{
			member->SendToEachLink(RadioCommand::NotifyUnlink);
			member->QueueMission(Mission::Attack, 0);
			member->SetTarget(nullptr);
			member->SetDestination(nullptr, 1);
		}

		// Assign target if member doesn't have one
		if (member->Target != team->ArchiveTarget && !member->Target){
			member->SetTarget(team->ArchiveTarget);
		}
	}

	// Check if member is active and combat-ready
	if (isAircraft)
	{
		// Aircraft must have weapon and ammo to be considered active
		if (!member->IsArmed())
			return false;
		if (member->Ammo <= 0)
			return false;
	}

	return true;
}

void FakeTeamClass::_Coordinate_Attack() {
	// Ensure target is set
	if (!this->ArchiveTarget) {
		this->ArchiveTarget = this->QueuedFocus;
	}

	// Find team leader (member with highest leadership rating)
	FootClass* leader = this->_Fetch_A_Leader();

	// If targeting a cell and leader is not aircraft, try to find object in cell
	RefineTargetIfCell(this, leader);

	// Periodically check if leader can fire at target
	if (Unsorted::CurrentFrame % 8 == 4) {
		CheckLeaderCanFire(this, leader);
	}

	// Process all team members
	if (!this->ArchiveTarget || !this->FirstUnit) {
		this->StepCompleted = true;
		return;
	}

	auto mission = this->CurrentScript->GetCurrentAction();
	bool hasActiveMembers = false;
	for (FootClass* member = this->FirstUnit; member; member = member->NextTeamMember)
	{
		// Initialize uninitiated members
		ProcessMemberInitiation(this, member);

		// Assign attack missions to initiated members
		if (ProcessMemberAttack(this, member, &mission))
		{
			hasActiveMembers = true;
		}
	}

	// If no active members remain, advance to next mission
	if (!hasActiveMembers)
	{
		this->StepCompleted = true;
	}
}

bool FakeTeamClass::_Coordinate_Conscript(FootClass* a2) {
	if (!a2 || !a2->IsAlive || !a2->Health || !Unsorted::ScenarioInit && a2->InLimbo || a2->IsTeamLeader)
	{
		return 0;
	}

	int strayDistance = this->_Get_Stray();
	if (a2->DistanceFrom(this->Zone) <= strayDistance)
	{
		// /*
		// **   This unit has gotten close enough to the team center so that it is
		// **   now considered initiated. An initiated unit is considered when calculating
		// **   the center of the team.
		// */
		a2->IsTeamLeader = 1;
		return 0;
	}
	if (!a2->Destination)
	{
		a2->QueueMission( Mission::Move, 0);
		a2->SetTarget(0);
		a2->SetDestination(this->Zone, 1);
	}
	return 1;
}

void FakeTeamClass::_Coordinate_Do(ScriptActionNode* pNode, CellStruct unused) {
	auto const& [miss, value] = *pNode;

	for (auto i = this->FirstUnit; i; i = i->NextTeamMember) {
		if (i->IsAlive)
		{
			int stray = this->_Get_Stray();

			if (i->Health && (Unsorted::ScenarioInit || !i->InLimbo) && !i->IsTeamLeader)
			{
				if (i->DistanceFrom(this->Zone) <= stray)
				{
					i->IsTeamLeader = 1;
				}
				else if (!i->Destination)
				{
					i->QueueMission(Mission::Move, 0);
					i->SetTarget(0);
					i->SetDestination(this->Zone, 1);
				}
			}
			if (i->IsAlive && i->Health && (Unsorted::ScenarioInit || !i->InLimbo))
			{
				if (i->IsTeamLeader || (i->WhatAmI() == AbstractType::Aircraft))
				{
					if (!i->Target)
					{
						if (i->Destination
						  || (i->DistanceFrom(this->Zone) <= 2 * stray || value == (int)Mission::Area_Guard))
						{
							if (!i->Target
							  && !i->Destination
							  && i->GetCurrentMission() != (Mission)value
							  && ((Mission)value != Mission::Guard || i->GetCurrentMission() != Mission::Unload))
							{
								i->ArchiveTarget = nullptr;
								i->QueueMission((Mission)value,false);
								i->SetTarget(nullptr);
								i->SetDestination(nullptr, true);
							}
						}
						else
						{
							i->QueueMission(Mission::Move, false);
							i->SetDestination(this->Zone, true);
							i->QueueMission(Mission::Move, false);
							CoordStruct zoneC = this->Zone->GetCoords();
							i->SetDestination(MapClass::Instance->GetCellAt(zoneC), true);
						}
					}
				}
			}
		}
	}
}

bool FakeTeamClass::_Is_A_Member(FootClass* member) {

	FootClass* v2 = this->FirstUnit;

	if (!v2) {
		return 0;
	}

	while (v2 != member) {
		v2 = v2->NextTeamMember;
		if (!v2) {
			return 0;
		}
	}

	return 1;
}

void _fastcall FakeTeamClass::_Suspend_Teams(int priority, HouseClass* house) {
	for (auto& team : *TeamClass::Array) {
		if (team->OwnerHouse == house && team->Type->Priority < priority) {
			for (auto i = team->FirstUnit; i; i = team->FirstUnit) {
				((FakeTeamClass*)team)->_Remove(i,-1, false);

				team->JustDisappeared = 1;
				team->NeedsToDisappear = 1;
				team->IsSuspended = true;
				team->SuspendTimer.Start(RulesClass::Instance->SuspendDelay * TICKS_PER_MINUTE);
			}
		}
	}
}

bool FakeTeamClass::_Is_Leaving_Map() {
	if (this->IsMoving) {
		if (this->CurrentScript->HasMissionsRemaining())
		{
			auto const&[Current_Mission , val] = this->CurrentScript->GetCurrentAction();
			if (Current_Mission == TeamMissionType::Move)
			{
				CellStruct  Waypoint_Location = ScenarioClass::Instance->GetWaypointCoords(val);
				if (!MapClass::Instance->IsWithinUsableArea(Waypoint_Location, 1))
				{
					return 1;
				}
			}
		}
	}
	return false;
}

bool FakeTeamClass::_Has_Entered_Map() {

	FootClass*  Member = this->FirstUnit;
	bool result = 1;
	if (Member)
	{
		while (Member->IsInPlayfield)
		{
			Member = Member->NextTeamMember;
			if (!Member)
			{
				return result;
			}
		}
		return 0;
	}
	return result;
}

bool FakeTeamClass::_has_aircraft() {

	TaskForceClass* TaskForce = this->Type->TaskForce;

	for (int i = 0; i < TaskForce->CountEntries; ++i) {
		if (TaskForce->Entries[i].Type && TaskForce->Entries[i].Amount > 0) {
			if (TaskForce->Entries[i].Type->WhatAmI() == AircraftTypeClass::AbsID) {
				return true;
			}
		}
	}

	return 0;
}

void FakeTeamClass::_Scan_Limit() {
	this->_AssignMissionTarget(0);
	for (auto i = this->FirstUnit; i; i = i->NextTeamMember) {
		i->SetTarget(0);
		i->IsScanLimited = 1;
	}
}

FootClass* FakeTeamClass::_Fetch_A_Leader() {

	FootClass* member = this->FirstUnit;
	int last_rating = -1;

	FootClass* last_leader = member;

	if (!member) {
		return 0;
	}

	do
	{
		int rating = member->GetTechnoType()->LeadershipRating;
		if (member->IsAlive
		  && member->Health
		  && (Unsorted::ScenarioInit || !member->InLimbo)
		  && (member->IsTeamLeader || member->WhatAmI() == AircraftClass::AbsID)
		  && rating > last_rating)
		{
			last_leader = member;
			last_rating = rating;
		}
		member = member->NextTeamMember;
	}
	while (member);

	return last_leader;
}

void FakeTeamClass::_GetTaskForceMissingMemberTypes(std::vector<TechnoTypeClass*>& missings) {
	const auto pType = this->Type;
	const auto pTaskForce = pType->TaskForce;
	int amount = 0;
	for (auto& ent : pTaskForce->Entries) {
		amount += ent.Amount;
	}

	missings.reserve(amount);

	for (int a = 0; a < pTaskForce->CountEntries; ++a) {
		for (int i = 0; i < pTaskForce->Entries[a].Amount; ++i) {
			if (auto pTaskType = pTaskForce->Entries[a].Type) {
				missings.emplace_back(pTaskType);
			}
		}
	}

	//remove first finded similarity
	for (auto pMember = this->FirstUnit; pMember; pMember = pMember->NextTeamMember)
	{
		auto it = std::ranges::find_if(missings, [&](TechnoTypeClass* pMissType) {
			return pMember->GetTechnoType() == pMissType || TeamExtData::IsEligible(pMember, pMissType);
		});

		if (it != std::ranges::end(missings))
			missings.erase(it);
	}
}

void FakeTeamClass::_Flash_For(int a2) {
	for (auto i = this->FirstUnit; i; i = i->NextTeamMember) {
		i->Flashing.DurationRemaining = a2;
	}
}

int FakeTeamClass::_Get_Stray() {
	auto const& [TMission, arg] = this->CurrentScript->GetCurrentAction();

	if (TMission == TeamMissionType::Gather_at_base || TMission == TeamMissionType::Gather_at_enemy) {
		return RulesClass::Instance->RelaxedStray;
	}

	return RulesClass::Instance->Stray;
}

bool FakeTeamClass::_Does_Any_Member_Have_Ammo() {
	FootClass*  Member = this->FirstUnit;

	if (!Member) {
		return 0;
	}

	while (Member->GetTechnoType()->Ammo > 0 && Member->Ammo <= 0)
	{
		Member = Member->NextTeamMember;
		if (!Member) {
			return 0;
		}
	}

	return 1;

}

// Find closest infantry that matches criteria
FootClass* FindClosestInfantry(TeamClass* team, int memberIndex, const CoordStruct& location, int targetGroup)
{
	InfantryClass* closestInfantry = nullptr;
	int minDistance = -1;

	for (int i = 0; i < InfantryClass::Array->Count; i++)
	{
		InfantryClass* infantry = InfantryClass::Array->Items[i];

		// Check group membership
		if (targetGroup != -2 && infantry->Group != targetGroup && !team->Type->Recruiter)
			continue;

		// Calculate distance with penalty for wrong group
		int distance = infantry->DistanceFromSquared(&location);
		if (infantry->Group != targetGroup)
			distance += 12800; // Penalty for wrong group

		// Check if this is a better candidate
		if ((minDistance == -1 || distance < minDistance) &&
			((FakeTeamClass*)team)->_Can_Add(infantry, &memberIndex, 0))
		{
			closestInfantry = infantry;
			minDistance = distance;
		}
	}

	return closestInfantry;
}

// Find closest aircraft that matches criteria
FootClass* FindClosestAircraft(TeamClass* team, int memberIndex, const CoordStruct& location, int targetGroup)
{
	AircraftClass* closestAircraft = nullptr;
	int minDistance = -1;

	for (int i = 0; i < AircraftClass::Array->Count; i++)
	{
		AircraftClass* aircraft = AircraftClass::Array->Items[i];

		// Check group membership
		if (targetGroup != -2 && aircraft->Group != targetGroup && !team->Type->Recruiter)
			continue;

		// Calculate distance with penalty for wrong group
		int distance = aircraft->DistanceFromSquared(&location);
		if (aircraft->Group != targetGroup)
			distance += 12800; // Penalty for wrong group

		// Check if this is a better candidate
		if ((minDistance == -1 || distance < minDistance) &&
			((FakeTeamClass*)team)->_Can_Add(aircraft, &memberIndex, 0))
		{
			closestAircraft = aircraft;
			minDistance = distance;
		}
	}

	return closestAircraft;
}

// Find closest unit that matches criteria
FootClass* FindClosestUnit(TeamClass* team, int memberIndex, const CoordStruct& location, int targetGroup)
{
	UnitClass* closestUnit = nullptr;
	int minDistance = -1;

	for (int i = 0; i < UnitClass::Array->Count; i++)
	{
		UnitClass* unit = UnitClass::Array->Items[i];

		// Check group membership
		if (targetGroup != -2 && unit->Group != targetGroup && !team->Type->Recruiter)
			continue;

		// Calculate distance with penalty for wrong group
		int distance = unit->DistanceFromSquared(&location);
		if (unit->Group != targetGroup)
			distance += 12800; // Penalty for wrong group

		// Check if this unit matches requirements
		if (unit->Owner == team->OwnerHouse &&
			unit->Type == (UnitTypeClass*)team->Type->TaskForce->Entries[memberIndex].Type &&
			(minDistance == -1 || distance < minDistance) &&
			((FakeTeamClass*)team)->_Can_Add(unit, &memberIndex, 0))
		{
			closestUnit = unit;
			minDistance = distance;
		}
	}

	return closestUnit;
}

bool FakeTeamClass::_Recruit(int memberIndex) {
	// Check if we already have enough units of this type
	TaskForceClass* pTaskForce = this->Type->TaskForce;

	if (!pTaskForce || memberIndex >= 6)
		Debug::FatalError("Team [%s - %x] missing taskforces !", this->Type->ID, this);

	if (pTaskForce->Entries[memberIndex].Amount <= this->CountObjects[memberIndex])
		return false;

	// Determine recruitment location
	CoordStruct recruitLocation = this->Zone ? this->Zone->GetCoords() : CoordStruct::Empty;

	// Override with origin location if specified
	CellStruct origin;
	this->Type->GetWaypoint(&origin);
	if (origin.IsValid()) // Not default cell
	{
		recruitLocation = CellClass::Cell2Coord(origin);
	}

	// Get recruitment group (-2 means any, -1 means unassigned)
	int targetGroup = this->Type->GetGroup();

	// Get the type of unit we're looking for
	auto unitKind = pTaskForce->Entries[memberIndex].Type->WhatAmI();
	// Search for appropriate unit based on type
	FootClass* recruitedUnit = nullptr;

	switch (unitKind)
	{
	case InfantryTypeClass::AbsID:
		recruitedUnit = FindClosestInfantry(this, memberIndex, recruitLocation, targetGroup);
		break;
	case AircraftTypeClass::AbsID:
		recruitedUnit = FindClosestAircraft(this, memberIndex, recruitLocation, targetGroup);
		break;
	case UnitTypeClass::AbsID:
		recruitedUnit = FindClosestUnit(this, memberIndex, recruitLocation, targetGroup);
		break;
	default:
		return false;
	}

	// Add recruited unit to team
	if (recruitedUnit)
	{
		recruitedUnit->SetTarget(nullptr);
		this->_Add2(recruitedUnit, false);

		// For units with cargo, also add attached objects
		if (unitKind == UnitTypeClass::AbsID)
		{
			FootClass* cargo = recruitedUnit->Passengers.GetFirstPassenger();
			while (cargo)
			{
				this->_Add2(recruitedUnit, false);

				// Check if next cargo item is still attached (bit 2 of TargetBitfield)
				if ((cargo->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
					break;

				cargo = (FootClass*)cargo->NextObject;
			}
		}

		return true;
	}

	return false;

}

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

void FakeTeamClass::_AssignMissionTarget(AbstractClass* new_target)
{
	// If the new target is different than current mission target

	if (new_target != this->QueuedFocus && this->QueuedFocus) {
		FootClass* unit = this->FirstUnit;

		while (unit)
		{
			const bool navMatch = (unit->Destination == this->QueuedFocus);
			const bool tarMatch = (unit->Target == this->QueuedFocus);

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

	// If Target was linked to previous QueuedFocus or is null, update Target as well
	if (this->ArchiveTarget == this->QueuedFocus || !this->ArchiveTarget) {
		this->ArchiveTarget = new_target;
	}

	this->QueuedFocus = new_target;

	// If new target is a CellClass (special case for map cells)
	if (auto pCellTarget = cast_to<CellClass*>(new_target))
	{
		if (MapClass::Instance->IsWithinUsableArea(pCellTarget, true)) {
			this->IsLeavingMap = false;
		} else {
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

bool FakeTeamClass::_Lagging_Units()
{
	bool result = 0;
	FootClass* unit = this->FirstUnit;
	bool lag = 0;

	// /*
	// ** If the IsLagging bit is not set, then obviously there are no lagging
	// ** units.
	// */
	if (this->IsLagging)
	{
		// /*
		// **   Scan through all of the units, searching for units who are having
		// ** trouble keeping up with the pack.
		// */
		if (unit)
		{
			do
			{
				if (unit->IsAlive
				  && unit->Health
				  && (Unsorted::ScenarioInit || !unit->InLimbo)
				  && (unit->IsTeamLeader || unit->WhatAmI() == AircraftClass::AbsID))
				{

					int stray = this->_Get_Stray();
					if (unit->WhatAmI() == AircraftClass::AbsID)
					{
						stray *= 3;
					}
					if (this->Type->GuardSlower && !unit->vt_entry_4E0())
					{
						stray /= 3;
					}

					CoordStruct v6 = this->ClosestMember->GetCoords();
					CoordStruct v7 = unit->GetCoords();
					CoordStruct diff = v7 - v6;
					// /*
					// ** If we find a unit who has fallen too far away from the center of
					// ** the pack, then we need to order that unit to catch up with the
					// ** first unit.
					// */
					if ((int)diff.Length() <= stray)
					{
						if (unit->GetCurrentMission() != Mission::Guard)
						{
							// /*
							// ** We need to order all of the other units to hold their
							// ** position until all lagging units catch up.
							// */
							unit->QueueMission(Mission::Guard, 0);
							unit->SetDestination(0, 1);
						}
					}
					else
					{
						if (!unit->Destination)
						{
							unit->QueueMission(Mission::Move, 0);
							unit->SetDestination(this->ClosestMember, 1);
						}
						lag = 1;
					}
				}
				unit = unit->NextTeamMember;
			}
			while (unit);
			result = lag;
		}
		this->IsLagging = result;
	}
	return result;
}

// Check if unit should continue moving
bool NOINLINE ShouldUnitKeepMoving(FootClass* unit, int distanceToTarget, int strayDistance)
{
	bool isAircraft = unit->WhatAmI() == AircraftClass::AbsID;
	CellClass* unitCell = unit->GetCell();

	// Check if aircraft is hovering at its current cell
	bool isAircraftAtCell = isAircraft && unit->Destination == unitCell;

	// Check if balloon hover unit is close enough and grounded
	bool isBalloonSettled = false;
	if ((unit->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		TechnoTypeClass* technoType = unit->GetTechnoType();
		if (technoType->BalloonHover && distanceToTarget < strayDistance)
		{
			isBalloonSettled = true;
		}
	}

	// Unit should keep moving if it has a destination and isn't at a stopping condition
	if (unit->Destination && !isAircraftAtCell && !isBalloonSettled)
	{
		return true;
	}

	return false;
}

// Move unit toward target
void NOINLINE MoveUnitToTarget(TeamClass* team, FootClass* unit, int distanceToTarget, int strayDistance, bool* allFinished)
{
	// Handle aggressive teams that may engage threats
	if (team->Type->Aggressive && unit->Target)
	{
		if (unit->__AssignNewThreat)
		{
			unit->SetTarget(nullptr);
		}
		// If engaged with target, don't force movement
		// Fall through to movement code
	}

	// Ensure unit has MOVE mission
	if (unit->GetCurrentMission() != Mission::Move)
	{
		unit->QueueMission(Mission::Move, 0);

		if (unit->ReadyToNextMission())
		{
			unit->NextMission();
		}
	}

	// Set destination if not already set
	if (!unit->Destination)
	{
		unit->SetDestination(team->ArchiveTarget, 1);
	}

	// Check if destination needs updating
	AbstractClass* navCom = unit->Destination;

	if (navCom != team->ArchiveTarget)
	{
		TechnoTypeClass* technoType = unit->GetTechnoType();
		bool isAircraft = unit->WhatAmI() == AircraftClass::AbsID;

		// Update destination for balloon hover units or aircraft at wrong cell
		if (technoType->BalloonHover ||
			(isAircraft && navCom == unit->GetCell()))
		{
			unit->SetDestination(team->ArchiveTarget, 1);
		}
	}

	// Check if unit is still moving
	if (ShouldUnitKeepMoving(unit, distanceToTarget, strayDistance))
	{
		*allFinished = false;
	}
}

// Stop unit and enter idle mode if not engaged
void NOINLINE StopUnitAndIdle(FootClass* unit)
{
	if (!unit->Target) // Not engaged with a target
	{
		unit->SetDestination(nullptr, 1);
		unit->EnterIdleMode(0, 1);
	}
}

// Handle unit arrival at target
void NOINLINE HandleUnitArrival(TeamClass* team, FootClass* unit)
{
	// Check if unit has reached its navigation destination
	if (unit->GetCurrentMission() == Mission::Move)
	{
		if (!unit->Destination)
		{
			// No destination, enter idle if not engaged
			StopUnitAndIdle(unit);
		}
		else
		{
			// Check if close enough to destination
			int distToNav = unit->DistanceFrom(unit->Destination);
			if (distToNav <= RulesClass::Instance->CloseEnough) {
				// Check if actually stopped moving
				if (!unit->Locomotor->Is_Moving()) {
					StopUnitAndIdle(unit);
				}
			}
		}
	}
}

// Process unit movement toward target - returns true if unit has arrived
bool NOINLINE ProcessUnitMovement(TeamClass * team, FootClass * unit, bool* anyFound, bool* allFinished)
{
	// Check if unit is valid for movement
	if (!unit->IsAlive || !unit->Health)
		return false;
	if (!Unsorted::ScenarioInit && unit->InLimbo)
		return false;

	bool isAircraft = (unit->WhatAmI() == AircraftClass::AbsID);
	if (!unit->IsTeamLeader && !isAircraft)
		return false;

	// Skip if unloading
	if (unit->GetCurrentMission() == Mission::Unload ||
		unit->QueuedMission == Mission::Unload)
	{
		*allFinished = false;
		return false;
	}

	// Get stray distance
	int strayDistance = ((FakeTeamClass*)team)->_Get_Stray();

	// Double stray distance for airborne units
	if (unit->IsInAir())
	{
		strayDistance *= 2;
	}

	*anyFound = true;
	int distanceToTarget = unit->DistanceFrom(team->ArchiveTarget);

	// Check if unit has arrived at target
	if (distanceToTarget <= strayDistance)
	{
		int height = unit->GetHeight();

		auto const& [nextMissionType, value] = team->CurrentScript->GetNextAction();

		if (height >= 0 || nextMissionType == TeamMissionType::Move)
		{
			// For aircraft, check if landed or next mission is MOVE
			if (isAircraft)
			{
				int zCoord = unit->GetZ();
				if (zCoord > 0) // Still flying
				{
					CoordStruct unitCoord;
					unit->GetCoords(&unitCoord);
					CellClass* unitCell = MapClass::Instance->GetCellAt(unitCoord);

					// Not arrived unless at target cell or next mission is MOVE
					if (unitCell != team->ArchiveTarget && nextMissionType != TeamMissionType::Move)
					{
						MoveUnitToTarget(team, unit, distanceToTarget, strayDistance, allFinished);
						return false;
					}
				}
			}

			// Unit has arrived
			return true;
		}
	}

	// Unit hasn't arrived yet, continue moving
	MoveUnitToTarget(team, unit, distanceToTarget, strayDistance, allFinished);
	return false;
}

// Check if unit should be initiated (start moving toward zone)
void NOINLINE ProcessUnitInitiation(TeamClass* team, FootClass* unit, bool* allFinished)
{
	// Only process uninitiated, active units
	if (!unit->IsAlive|| !unit->Health)
		return;
	if (!Unsorted::ScenarioInit && unit->InLimbo)
		return;
	if (unit->IsTeamLeader)
		return;

	// Get stray distance based on mission type
	int strayDistance = ((FakeTeamClass*)team)->_Get_Stray();

	// Check if unit is close enough to zone to be considered initiated
	if (unit->DistanceFrom(team->Zone) <= strayDistance)
	{
		unit->IsTeamLeader = true;
	}
	else
	{
		// Send unit to zone if not already moving there
		if (!unit->Destination)
		{
			unit->QueueMission(Mission::Move, 0);
			unit->SetTarget(nullptr);
			unit->SetDestination(team->Zone, 1);
		}
		*allFinished = false;
	}

	// Check if unit is unloading
	if (unit->GetCurrentMission() == Mission::Unload ||
		unit->QueuedMission == Mission::Unload)
	{
		*allFinished = false;
	}
}

void FakeTeamClass::_CoordinateMove()
{
	FootClass* unit = this->FirstUnit;
	if (!unit)
		return;

	// Ensure target is set
	if (!this->ArchiveTarget)
	{
		this->ArchiveTarget = this->QueuedFocus;
		if (!this->ArchiveTarget)
			return;
	}

	// Check if team has lagging units that need to catch up
	if (this->_Lagging_Units())
		return;

	bool anyUnitFound = false;
	bool allUnitsFinished = true;

	// Process each unit in the team
	while (unit)
	{
		ProcessUnitInitiation(this, unit, &allUnitsFinished);

		if (ProcessUnitMovement(this, unit, &anyUnitFound, &allUnitsFinished))
		{
			// Unit has reached target and is ready
			HandleUnitArrival(this, unit);
		}

		unit = unit->NextTeamMember;
	}

	// If all units have arrived and are ready, advance to next mission
	if (anyUnitFound && allUnitsFinished && this->IsMoving)
	{
		this->StepCompleted = true;
	}
}

bool FakeTeamClass::_Recalculate() {
    bool IsUnderStrength = this->IsUnderStrength;
	TeamTypeClass* pType = this->Type;
    int desired = pType->TaskForce->Required();
    int total = this->TotalObjects;

    if (total > 0 ) {
        this->IsFullStrength = total == desired;

        if ( total == desired ) {
            this->IsHasBeen = 1;
        }

        // /*
        // **   Reinforceable teams will revert (or snap out of) the under strength
        // **   mode when the members transition the magic 1/3 strength threshold.
        // */
        if (pType->Reinforce) {
            this->IsUnderStrength = desired <= 2  ? total < desired : (total <= desired / 3);

        } else {
            // /*
            // **   Teams that are not flagged as reinforceable are never considered under
            // **   strength if the team has already started its main mission. This
            // **   ensures that once the team has started, it won't dally to pick up
            // **   new members.
            // */
            this->IsUnderStrength = this->IsHasBeen == 0;
        }

        if (pType->GuardSlower) {
			this->GuardSlowerIsNotUnderStrength = !this->IsUnderStrength;
		}

        this->JustDisappeared = 0;
        this->NeedsToDisappear = 0;

		if (IsUnderStrength != this->IsUnderStrength) {
            this->IsReforming = 1;
        }

        return 1;
    }

    IsHasBeen = this->IsHasBeen;
    this->GuardSlowerIsNotUnderStrength = 0;
    this->IsUnderStrength = 1;
    this->IsFullStrength = 0;
    this->Zone = 0;

    // /*
    // **   A team that exists on the player's side is automatically destroyed
    // **   when there are no team members left. This team was created as a
    // **   result of reinforcement logic and no longer needs to exist when there
    // **   are no more team members.
    // */
    if (!IsHasBeen) {
        if (IsUnderStrength != this->IsUnderStrength){
            this->IsReforming = 1;
        }

        return 1;
    }

    // /*
    // **   If this team had no members (i.e., the team object wasn't terminated by some
    // **   outside means), then pass through the logic triggers to see if one that
    // **   depends on this team leaving the map should be sprung.
    // */
    if (this->IsLeavingMap) {
		for(int i = TagClass::Array->Count -1; i >= 0; --i) {
			TagClass::Array->operator[](i)->SpringEvent(TriggerEvent::TeamLeavesMap,
				nullptr ,
				CellStruct::Empty,
				false ,
				nullptr);
		}
    }

	if (this) {
		((TeamClass*)this)->~TeamClass();
	}

    return 0;
}

void NOINLINE StopScript(TeamClass* pTeam) {
	if (pTeam->IsFullStrength || pTeam->IsForcedActive) {
		pTeam->IsMoving = 1;
		pTeam->IsHasBeen = 1;
		pTeam->IsUnderStrength = 0;
		for (FootClass* Member = pTeam->FirstUnit; Member; Member = Member->NextTeamMember){
			if (pTeam->IsReforming || pTeam->IsForcedActive) {
				Member->IsTeamLeader = 1;
			}
		}

		pTeam->CurrentScript->ClearMission();
		pTeam->StepCompleted = 1;
	}
}

bool NOINLINE IsTechnoMemberEligible(FootClass* pTech, TeamClass* pTeam)
{
	if (!pTech || !pTech->IsAlive || !pTech->Health)
		return false;

	if (!Unsorted::ScenarioInit && pTech->InLimbo)
		return false;

	if (!pTech->Owner->IsAlliedWith(pTeam->OwnerHouse))
		return false;

	if (pTeam == pTech->Team)
		return false;

	return true;
}

void FakeTeamClass::_Calc_Center(AbstractClass** outCell, FootClass** outClosestMember)
{
	*outCell = nullptr;
	*outClosestMember = nullptr;

	FootClass* member = this->FirstUnit;

	if (!member)
		return;

	auto const& [mission, args] = this->CurrentScript->GetCurrentAction();

	if (mission == TeamMissionType::Hound_dog)
	{
		// Hound Dog mission: Find closest ally unit/infantry
		FootClass* closestAlly = nullptr;
		int minDistance = -1;

		// Search through all units
		for (int i = 0; i < UnitClass::Array->Count; i++)
		{
			UnitClass* unit = UnitClass::Array->operator[](i);

			if (!IsTechnoMemberEligible(unit, this))
				continue;

			// Calculate distance between unit and first member
			CoordStruct unitCoord = unit->GetCoords();
			CoordStruct memberCoord = member->GetCoords();
			CoordStruct diff = memberCoord - unitCoord;

			int distance = diff.Length();

			if (minDistance == -1 || distance < minDistance)
			{
				minDistance = distance;
				closestAlly = unit;
			}
		}

		// Search through all infantry
		for (int i = 0; i < InfantryClass::Array->Count; i++)
		{
			InfantryClass* infantry = InfantryClass::Array->operator[](i);
			if (!IsTechnoMemberEligible(infantry, this))
				continue;

			// Calculate distance between infantry and first member
			CoordStruct infantryCoord = infantry->GetCoords();
			CoordStruct memberCoord = member->GetCoords();
			CoordStruct diff = memberCoord - infantryCoord;

			int distance = diff.Length();
			if (minDistance == -1 || distance < minDistance)
			{
				minDistance = distance;
				closestAlly = infantry;
			}
		}

		// If found closest ally, set output parameters
		if (closestAlly)
		{
			// Check if ally is in tunnel
			if (closestAlly->TubeIndex >= 0)
			{
				// Get tunnel exit cell
				CellStruct exitCell = TubeClass::Array->operator[](closestAlly->TubeIndex)->ExitCell;
				*outCell = MapClass::Instance->GetCellAt(exitCell);
			}
			else
			{
				*outCell = closestAlly;
			}
			*outClosestMember = this->FirstUnit;
		}
	}
	else
	{
		// Normal mission: Calculate center of team members
		CoordStruct tatalXYZ {};
		int memberCount = 0;

		FootClass* closestToTarget = nullptr;
		int minDistanceToTarget = 0;

		// Iterate through all team members
		while (member)
		{
			if (!member->IsAlive || !member->Health)
			{
				member = member->NextTeamMember;
				continue;
			}
			if (!Unsorted::ScenarioInit && member->InLimbo)
			{
				member = member->NextTeamMember;
				continue;
			}

			// Check if member is initiated or is aircraft
			bool isAircraft = (member->WhatAmI() == AircraftClass::AbsID);
			if (!member->IsTeamLeader && !isAircraft)
			{
				member = member->NextTeamMember;
				continue;
			}
			if (!member->IsInPlayfield)
			{
				member = member->NextTeamMember;
				continue;
			}

			// Add member coordinates to total
			CoordStruct coord = member->Location;
			tatalXYZ += coord;
			memberCount++;

			// If team guards slower units, count this member twice
			if (this->Type->GuardSlower && member->vt_entry_4E0())
			{
				tatalXYZ += coord;
				memberCount++;
			}

			// Find member closest to target (exclude transports)
			TechnoTypeClass* technoType = member->GetTechnoType();
			bool isTransport = (technoType->Passengers > 0 && technoType->Naval);

			if (!isTransport)
			{
				int distToTarget = member->DistanceFrom(this->ArchiveTarget);
				if (!closestToTarget || distToTarget < minDistanceToTarget)
				{
					minDistanceToTarget = distToTarget;
					closestToTarget = member;
				}
			}

			member = member->NextTeamMember;
		}

		// Calculate average position
		if (memberCount > 0)
		{
			CoordStruct centerCoord = tatalXYZ / memberCount;
			CellClass* centerCell = MapClass::Instance->GetCellAt(centerCoord);
			*outCell = centerCell;

			// Use first member if no closest member found
			if (!closestToTarget)
				closestToTarget = this->FirstUnit;

			// Verify closest member can enter the center cell
			if (closestToTarget->IsCellOccupied(centerCell, FacingType::None, -1, 0, 1) != Move::OK)
			{
				*outCell = closestToTarget;
			}
		}

		if (closestToTarget)
		{
			*outClosestMember = closestToTarget;
		}
	}
}

#include <Ext/Building/Body.h>

void FakeTeamClass::_Regroup()
{
	// Stop current script and movement
	this->IsMoving = false;
	this->CurrentScript->ClearMission();

	if (this->TotalObjects <= 0) {
		this->Zone = nullptr;
		return;
	}

	// Calculate team center position
	this->_Calc_Center(&this->Zone, &this->ClosestMember);

	if (!this->Zone)
	{
		if (!this->ClosestMember)
			return;
		this->Zone = (CellClass*)this->ClosestMember;
	}

	/*
	 * When a team is badly damaged and needs to regroup, it should
	 * pick a friendly building to go and regroup at. Its first preference
	 * should be somewhere near a repair factory. If it cannot find a repair
	 * factory, then it should pick another structure that is friendly to
	 * its side.
	 */

	CoordStruct zoneCoord = this->Zone->GetCoords();
	CellStruct destCell = CellClass::Coord2Cell(zoneCoord);

	int minWeightedDistance = 0x7FFFFFFF;

	// Search through all friendly buildings
	for (int i = 0; i < this->OwnerHouse->Buildings.Count; i++)
	{
		BuildingClass* building = this->OwnerHouse->Buildings.Items[i];
		if (!building)
			continue;

		BuildingExtData* pExt = BuildingExtContainer::Instance.Find(building);

		// Skip invalid or limbo buildings, or armed buildings
		if (pExt->LimboID != -1 || building->InLimbo || !building->IsAlive)
			continue;

		if (building->IsArmed())
			continue;

		// Get building's cell position
		CoordStruct buildingCoord = building->GetCoords();
		CellStruct buildingCell = CellClass::Coord2Cell(buildingCoord);

		// Calculate distance from zone to building
		CoordStruct diff = buildingCoord - zoneCoord;

		int distance = diff.Length();

		// Weight distance by cell threat (avoid dangerous areas)
		int cellThreat = MapClass::Instance->GetThreatPosed(buildingCell, this->OwnerHouse);
		int weightedDistance = distance * (cellThreat + 1);

		// Prefer repair facilities (half the weighted distance)
		if (building->Type->UnitRepair)
		{
			weightedDistance /= 2;
		}

		if (weightedDistance < minWeightedDistance)
		{
			// Find team leader (member with highest leadership rating)
			FootClass* leader = this->FirstUnit;
			int maxLeadership = -1;

			for (FootClass* member = this->FirstUnit; member; member = member->NextTeamMember)
			{
				if (!member->IsAlive || !member->Health)
					continue;

				if (!Unsorted::ScenarioInit && member->InLimbo)
					continue;

				bool isAircraft = (member->WhatAmI() == AircraftClass::AbsID);
				if (!member->IsTeamLeader && !isAircraft)
					continue;

				int leadershipRating = member->GetTechnoType()->LeadershipRating;
				if (leadershipRating > maxLeadership)
				{
					leader = member;
					maxLeadership = leadershipRating;
				}
			}

			// Find a safe point near the building for the leader
			CellStruct zoneCell = CellClass::Coord2Cell(
				this->Zone->GetCoords()
			);

			CellStruct safetyPoint;
			leader->SafetyPoint(&safetyPoint, &zoneCell, &buildingCell, 2, 4);

			// Only use this destination if a valid safety point was found
			if (safetyPoint.X != -1 && safetyPoint.Y != -1) // Assuming default_cell is (-1, -1)
			{
				minWeightedDistance = weightedDistance;
				destCell = safetyPoint;
			}
		}
	}

	// Set target to the chosen destination and coordinate team movement
	this->ArchiveTarget = MapClass::Instance->GetCellAt(destCell);
	this->_CoordinateMove();
}

void FakeTeamClass::_AI()
{
	//HouseExtContainer::HousesTeams[this->OwnerHouse].emplace(this);

	if (this->IsSuspended) {
		const int suspend_timeleft = this->SuspendTimer.GetTimeLeft();
		if(suspend_timeleft == 0) {
			this->IsSuspended = false;
		} else {
			return;
		}
	}

	if (this->NeedsToDisappear && !this->_Recalculate())
	{
		return;
	}

	if (!this->IsMoving)
	{
		StopScript(this);
	}

	if (this->IsUnderStrength)
	{
		this->_Regroup();
	}

	if (!this->IsMoving)
	{
		StopScript(this);
	}

	if (this->IsReforming || this->IsMoving || !this->Zone || !this->ClosestMember)
	{
		this->_Calc_Center(&this->Zone, &this->ClosestMember);
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
					this->_Recruit(v5);
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
		this->_CoordinateMove();
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

		this->_AssignMissionTarget(nullptr);
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
		this->_AssignMissionTarget(nullptr);
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

	ScriptActionNode node = this->CurrentScript->GetCurrentAction();

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
	case TeamMissionType::Play_speech:{
		ScriptExtData::PlaySpeech(this);
		return;
	}
	default:

		if (AresScriptExt::Handle(this, &node, arg4) || ScriptExtData::ProcessScriptActions(this, &node, arg4))
			return;

		break;
	}

	this->ExecuteTMission(node.Action, &node, arg4);
}

bool FakeTeamClass::_CoordinateRegroup()
{
	bool allMembersRegrouped = true;
	FootClass* Member = this->FirstUnit;
	bool hasTeamLeader = false;  // Track if we have a captain

	// If no members exist, mark as regrouped and return true
	if (!Member)
	{
		this->NeedsReGrouping = 0;
		return true;
	}

	int stray = this->_Get_Stray();

	// First pass: Check if we already have a team leader
	for (FootClass* checkMember = Member; checkMember; checkMember = checkMember->NextTeamMember) {
		if (checkMember->IsTeamLeader && checkMember->IsAlive && checkMember->Health) {
			hasTeamLeader = true;
			break;
		}
	}

	// Process each member in the team
	do
	{
		if (Member->IsAlive)
		{
			// Check if member is valid and ready for regrouping
			if (Member->Health && (Unsorted::ScenarioInit || !Member->InLimbo) && !Member->IsTeamLeader) {
				// Check if member is close enough to initiate
				if (FakeObjectClass::_GetDistanceOfObj(Member , discard_t() , this->Zone) <= stray)
				{
					if (!hasTeamLeader) {
						Member->IsTeamLeader = true;
						hasTeamLeader = true;
					}
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
				if (FakeObjectClass::_GetDistanceOfObj(Member, discard_t(), this->Zone) <= stray
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
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA610, FakeTeamClass::_Can_Add)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA870, FakeTeamClass::_Remove)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EAA90, FakeTeamClass::_Recruit)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EB380, FakeTeamClass::_Took_Damage)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA4F0, FakeTeamClass::_Add)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA500, FakeTeamClass::_Add2)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA3E0, FakeTeamClass::_Recalculate)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EB490, FakeTeamClass::_Coordinate_Attack)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC130, FakeTeamClass::_Coordinate_Conscript)
DEFINE_FUNCTION_JUMP(LJMP, 0x6ED7E0, FakeTeamClass::_Coordinate_Do)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EB870, FakeTeamClass::_CoordinateRegroup)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EBAD0, FakeTeamClass::_CoordinateMove)
DEFINE_FUNCTION_JUMP(LJMP, 0x6E9050, FakeTeamClass::_AssignMissionTarget)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA0D0, FakeTeamClass::_Regroup)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EAEE0, FakeTeamClass::_Calc_Center)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EBF50, FakeTeamClass::_Lagging_Units)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EA080, FakeTeamClass::_TeamClass_6EA080)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC220, FakeTeamClass::_Is_A_Member)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC250, FakeTeamClass::_Suspend_Teams)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC300, FakeTeamClass::_Is_Leaving_Map)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC370, FakeTeamClass::_Has_Entered_Map)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EC3A0, FakeTeamClass::_Scan_Limit)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EF470, FakeTeamClass::_has_aircraft)
DEFINE_FUNCTION_JUMP(LJMP, 0x6EF5F0, FakeTeamClass::_Flash_For)
DEFINE_FUNCTION_JUMP(LJMP, 0x6F03B0, FakeTeamClass::_Get_Stray)
DEFINE_FUNCTION_JUMP(LJMP, 0x6F03F0, FakeTeamClass::_Does_Any_Member_Have_Ammo)
#pragma endregion
//ASMJIT_PATCH(0x55B4F5, LogicClass_Update_Teams, 0x6)
//{
//	for (int i = 0; i < TeamClass::Array->Count; ++i) {
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
std::vector<TeamExtData*> Container<TeamExtData>::Array;

void Container<TeamExtData>::Clear()
{
	Array.clear();
}

bool TeamExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool TeamExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

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
	//HouseExtContainer::HousesTeams[pThis->OwnerHouse].erase(pThis);
	TeamExtContainer::Instance.Remove(pThis);
	return 0;
}

void FakeTeamClass::_Detach(AbstractClass* target, bool all)
{
	TeamExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->TeamClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4758, FakeTeamClass::_Detach)

HRESULT __stdcall FakeTeamClass::_Load(IStream* pStm)
{
	HRESULT hr = this->TeamClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = TeamExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeTeamClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->TeamClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = TeamExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4744, FakeTeamClass::_Load)
// DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4748, FakeTeamClass::_Save)
