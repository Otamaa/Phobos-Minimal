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

#include <Phobos.SaveGame.h>

template<typename Func, typename... Args>
concept ReturnsBool = std::same_as<std::invoke_result_t<Func, Args...>, bool>;


// Helper function for comparator operations
static NOINLINE COMPILETIMEEVAL bool EvaluateComparator(int counter, AITriggerConditionComparator comp, bool ret)
{
	switch (comp.ComparatorOperand)
	{
	case 0:
		return counter < comp.ComparatorType;
	case 1:
		return counter <= comp.ComparatorType;
	case 2:
		return counter == comp.ComparatorType;
	case 3:
		return counter >= comp.ComparatorType;
	case 4:
		return counter > comp.ComparatorType;
	case 5:
		return counter != comp.ComparatorType;
	default:
		return ret;
	}
}

template<typename Func>
void LoopThruMembers(TeamClass* pTeam, Func&& act)
{
	FootClass* pCur = nullptr;
	if (auto pFirst = pTeam->FirstUnit)
	{
		do
		{
			//we fetch next team member early for these specific usage
			auto pNext = pFirst->NextTeamMember;

			if constexpr (ReturnsBool<Func, FootClass*>) {
				if (act(pFirst))
					return;// break from function with return true
			} else {
				act(pFirst);
			}

			pFirst = pNext;
		}
		while (pFirst);
	}
}

#pragma region ExtFuncs

bool TeamExtData::IsEligible(TechnoTypeClass* pGoing, TechnoTypeClass* reinfocement){

	if (!pGoing || !reinfocement)
		return false;

	if(pGoing == reinfocement)
		return true;

	if (TechnoTypeExtContainer::Instance.Find(pGoing)->RecuitedAs.isset()
					&& TechnoTypeExtContainer::Instance.Find(pGoing)->RecuitedAs == reinfocement)
		return true;

	if (TechnoTypeExtContainer::Instance.Find(reinfocement)->RecuitedAs.isset()
		&& TechnoTypeExtContainer::Instance.Find(reinfocement)->RecuitedAs == pGoing)
		return true;

	if (!TechnoTypeExtContainer::Instance.Find(pGoing)->TeamMember_ConsideredAs.empty() && TechnoTypeExtContainer::Instance.Find(pGoing)->TeamMember_ConsideredAs.Contains(reinfocement))
		return true;

	if (!TechnoTypeExtContainer::Instance.Find(reinfocement)->TeamMember_ConsideredAs.empty() && TechnoTypeExtContainer::Instance.Find(reinfocement)->TeamMember_ConsideredAs.Contains(pGoing))
		return true;

	return false;
}

bool TeamExtData::IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement)
{
	return TeamExtData::IsEligible(GET_TECHNOTYPE(pGoing), reinfocement);
}

void TeamExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(TeamLeader, ptr, bRemoved);
	AnnounceInvalidPointer(LastFoundSW, ptr);
	AnnounceInvalidPointer(PreviousScript, ptr);
}

bool TeamExtData::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list)
{
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
				&& TeamExtData::IsEligible(GET_TECHNOTYPE(pObject), pItem))
			{
				counter++;
			}
		}
	}

	return EvaluateComparator(counter, pThis->Conditions[0] , false);
}

bool TeamExtData::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list)
{
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
				&& (!pEnemy || !pHouse->IsAlliedWith(pEnemy))
				&& !pObject->Owner->Type->MultiplayPassive
				&& TeamExtData::IsEligible(GET_TECHNOTYPE(pObject), pItem))
			{
				counter++;
			}
		}
	}

	return  EvaluateComparator(counter, pThis->Conditions[0], false);
}

bool TeamExtData::NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list)
{
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
					&& GET_TECHNOTYPE(pObject) == pItem)
				{
					counter++;
				}
			}
		}
	}

	return EvaluateComparator(counter, pThis->Conditions[0], false);
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

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject))
				continue;

			if (pObject->Owner == pHouse &&
				GET_TECHNOTYPE(pObject) == pItem)
			{
				counter++;
			}
		}

		result = EvaluateComparator(counter, pThis->Conditions[0], true);
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

		for (auto pObject : *TechnoClass::Array)
		{
			if (!TechnoExtData::IsAlive(pObject) || !pObject->Owner)
				continue;

			if (pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& TeamExtData::IsEligible(GET_TECHNOTYPE(pObject), pItem))
			{
				counter++;
			}
		}
		result = EvaluateComparator(counter, pThis->Conditions[0], true);
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
					TeamExtData::IsEligible(GET_TECHNOTYPE(pObject), pItem))
				{
					counter++;
				}
			}

			foundAll = EvaluateComparator(counter, pThis->Conditions[0], true);
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

	if (!this->GuardAreaTimer.HasTimeLeft())
		this->StepCompleted = true;

	//Debug::LogInfo("Script {} GuardAreaTimer Left {}", this->CurrentScript->Type->ID, );
}

void FakeTeamClass::_TMission_GatherAtBase(ScriptActionNode* nNode, bool arg3)
{
	// ------------------------------------------------------------
   // Early exit - just coordinate movement
   // ------------------------------------------------------------
	if (!arg3) {
		this->_CoordinateMove();
		return;
	}

	// ------------------------------------------------------------
	// Find team leader (highest leadership rating)
	// ------------------------------------------------------------
	TechnoClass* pLeader = this->_Fetch_A_Leader();

	if (!pLeader) {
		this->StepCompleted = true;
		return;
	}

	// ------------------------------------------------------------
	// Get enemy house (optional - may be null)
	// ------------------------------------------------------------
	HouseClass* pEnemyHouse = nullptr;

	int enemyIndex = pLeader->Owner->EnemyHouseIndex;
	if (enemyIndex != -1) {
		pEnemyHouse = HouseClass::Array->Items[enemyIndex];
	}

	// ------------------------------------------------------------
	// Get our base center
	// ------------------------------------------------------------
	CoordStruct ourBase;
	pLeader->Owner->GetBaseCenterCoords(&ourBase);

	// ------------------------------------------------------------
	// Calculate gather direction
	// ------------------------------------------------------------
	short binaryAngle;

	if (pEnemyHouse)
	{
		// Direction away from enemy base
		CoordStruct enemyBase;
		pEnemyHouse->GetBaseCenterCoords(&enemyBase);

		int deltaX = enemyBase.X - ourBase.X;
		int deltaY = enemyBase.Y - ourBase.Y;

		double angleRadians = Math::atan2(double(-deltaY), double(deltaX));
			   angleRadians -= Math::DEG90_AS_RAD;

		binaryAngle = static_cast<short>(angleRadians * Math::BINARY_ANGLE_MAGIC);
	}
	else
	{
		// Random direction
		binaryAngle = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
	}

	// ------------------------------------------------------------
	// Calculate offset position from our base
	// ------------------------------------------------------------
	int adjustedAngle = static_cast<short>(binaryAngle) - 0x3FFF;
	double radians = adjustedAngle * Math::DIRECTION_FIXED_MAGIC;

	int safeDistance = (RulesExtData::Instance()->AIFriendlyDistance.Get(RulesClass::Instance->AISafeDistance) + nNode->Argument) << 8;

	int targetY = static_cast<int>(ourBase.Y - Math::sin(radians) * safeDistance);
	int targetX = static_cast<int>(ourBase.X + Math::cos(radians) * safeDistance);

	// ------------------------------------------------------------
	// Convert to cell coordinates (signed division with rounding)
	// ------------------------------------------------------------
	CoordStruct result_coord(targetX, targetY, 0);
	CellStruct targetCell = CellClass::Coord2Cell(result_coord);

	// ------------------------------------------------------------
	// Find nearby valid location
	// ------------------------------------------------------------
	SpeedType speed = GET_TECHNOTYPE(pLeader)->SpeedType;
	CellStruct searchParams = { 0, 0 };

	CellStruct resulttargetCell = MapClass::Instance->NearByLocation(targetCell,
		speed,
		ZoneType::None,
		MovementZone::Normal,
		false,
		3,
		3,
		false,
		false,
		false,
		true,
		targetCell,
		false,
		false
	);

	// ------------------------------------------------------------
	// Assign target
	// ------------------------------------------------------------
	this->_AssignMissionTarget(MapClass::Instance->GetCellAt(resulttargetCell));
		Debug::Log(
		"A %s Team has chosen ( %d, %d) for its GatherAtBase cell.\n",
		this->Type->Name,
		resulttargetCell.X,
		resulttargetCell.Y
	);

	// ------------------------------------------------------------
	// Execute coordinated movement
	// ------------------------------------------------------------
	this->_CoordinateMove();
}

void FakeTeamClass::_TMission_GatherAtEnemy(ScriptActionNode* nNode, bool arg3)
{
	// ------------------------------------------------------------
// Early exit - just coordinate movement
// ------------------------------------------------------------
	if (!arg3) {
		this->_CoordinateMove();
		return;
	}

	// ------------------------------------------------------------
	// Find team leader (highest leadership rating)
	// ------------------------------------------------------------
	TechnoClass* pLeader = this->_Fetch_A_Leader();

	// ------------------------------------------------------------
	// Validate leader exists
	// ------------------------------------------------------------
	if (!pLeader)
	{
		this->StepCompleted = true;
		return;
	}

	// ------------------------------------------------------------
	// Get and validate enemy house
	// ------------------------------------------------------------
	int enemyIndex = pLeader->Owner->EnemyHouseIndex;
	if (enemyIndex == -1)
	{
		this->StepCompleted = true;
		return;
	}

	HouseClass* pEnemyHouse = HouseClass::Array->Items[enemyIndex];
	if (!pEnemyHouse)
	{
		this->StepCompleted = true;
		return;
	}

	// ------------------------------------------------------------
	// Get and validate enemy base center
	// ------------------------------------------------------------
	CoordStruct enemyBase;
	pEnemyHouse->GetBaseCenterCoords(&enemyBase);

	if (!enemyBase.IsValid()) {
		this->StepCompleted = true;
		return;
	}

	// ------------------------------------------------------------
	// Get our base center (fallback to leader position if invalid)
	// ------------------------------------------------------------
	CoordStruct ourBase;
	pLeader->Owner->GetBaseCenterCoords(&ourBase);

	if (!ourBase.IsValid()) {
		ourBase = pLeader->GetCoords();
	}

	// ------------------------------------------------------------
	// Calculate direction from enemy base toward our position
	// ------------------------------------------------------------
	int deltaX = ourBase.X - enemyBase.X;
	int deltaY = enemyBase.Y - ourBase.Y;

	double angleRadians = Math::atan2(double(-deltaY), double(deltaX));
	angleRadians -= Math::DEG90_AS_RAD;

	short binaryAngle = static_cast<short>(angleRadians * Math::BINARY_ANGLE_MAGIC);

	// ------------------------------------------------------------
	// Calculate offset position from enemy base
	// ------------------------------------------------------------
	int adjustedAngle = static_cast<short>(binaryAngle) - 0x3FFF;
	double radians = adjustedAngle * Math::DIRECTION_FIXED_MAGIC;

	int safeDistance = (RulesClass::Instance->AISafeDistance + nNode->Argument) << 8;

	int rawY = static_cast<int>(enemyBase.Y - Math::sin(radians) * safeDistance);
	int rawX = static_cast<int>(Math::cos(radians) * safeDistance + enemyBase.X);

	// ------------------------------------------------------------
	// Apply coordinate transformation
	// ------------------------------------------------------------
	CoordStruct transformedCoord(rawX , rawY , enemyBase.Z);

	// ------------------------------------------------------------
	// Convert to cell coordinates (signed division with rounding)
	// ------------------------------------------------------------

	CellStruct targetCell = CellClass::Coord2Cell(transformedCoord);

	// ------------------------------------------------------------
	// Find nearby valid location
	// ------------------------------------------------------------
	SpeedType speed = GET_TECHNOTYPE(pLeader)->SpeedType;
	CellStruct searchParams = { 0, 0 };

	CellStruct _resulttargetCell = MapClass::Instance->NearByLocation(
		targetCell,
		speed,
		ZoneType::None,
		MovementZone::Normal,
		false,
		3,
		3,
		false,
		false,
		false,
		true,
		searchParams,
		false,
		false
	);

	// ------------------------------------------------------------
	// Assign target and log
	// ------------------------------------------------------------
	CellClass* pTargetCell = MapClass::Instance->GetCellAt(_resulttargetCell);
	this->_AssignMissionTarget(pTargetCell);

	Debug::Log(
		"A %s Team has chosen ( %d, %d) for its GatherAtEnemy cell.\n",
		this->Type->Name,
		_resulttargetCell.X,
		_resulttargetCell.Y
	);

	// ------------------------------------------------------------
	// Execute coordinated movement
	// ------------------------------------------------------------
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
		GET_TECHNOTYPE(member)->Passengers > 0;

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
	TechnoTypeClass* unitType = GET_TECHNOTYPE(unit);
	TaskForceClass* taskForce = team->Type->TaskForce;

	for (int i = 0; i < taskForce->CountEntries; i++)
	{
		if (TeamExtData::IsEligible(taskForce->Entries[i].Type,unitType))
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

	//if (!unit->IsInPlayfield)
	//	return false;

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

	auto pExt = TechnoExtContainer::Instance.Find(unit);

	if (pExt->Is_DriverKilled)
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
	obj->RecruitableB = true;
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
				if (this->Type->TaskForce->Entries[typeindex].Type == GET_TECHNOTYPE(obj))
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
	//if (ShouldRetaliateAgainstAttacker(this, attacker))
	//{
	//	// Potentially change target (implementation seems incomplete in original)
	//	//attacker->a.vftable->t.r.m.o.a.Kind_Of(&attacker->a);
	//	Debug::FatalError("TamClass::Took_Damage function is seems incomplete calling this may result in wasted calculation !");
	//}

	if (RulesExtData::Instance()->TeamRetaliate)
	{
		auto pFocus = flag_cast_to<TechnoClass*>(this->ArchiveTarget);
		auto SpawnCell = this->Zone;

		if (!pFocus
		  || !pFocus->IsArmed()
		  || !SpawnCell
		  || pFocus->IsCloseEnoughToAttackCoords(SpawnCell->GetCoords()))
		{
			if (attacker->WhatAmI() != AircraftClass::AbsID)
			{
				auto pAttackerTechno = flag_cast_to<TechnoClass*, false>(attacker);

				auto Owner = this->OwnerHouse;
				if (pAttackerTechno && Owner->IsAlliedWith(pAttackerTechno->GetOwningHouse()))
				{
					return;
				}

				if (auto pAttackerFoot = flag_cast_to<FootClass*, false>(attacker))
				{
					if (pAttackerFoot->InLimbo
					|| GET_TECHNOTYPE(pAttackerFoot)->ConsideredAircraft)
					{
						return;
					}
				}

				this->ArchiveTarget = attacker;
			}
		}
	}

#ifdef CUSTOM
	// get ot if global option is off
	if (!RulesExtData::Instance()->TeamRetaliate)
	{
		return 0x6EB47A;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pThis->ArchiveTarget);
	auto pSpawn = pThis->SpawnCell;

	if (!pFocus || !pFocus->IsArmed() || !pSpawn || pFocus->IsCloseEnoughToAttackCoords(pSpawn->GetCoords()))
	{
		// disallow aircraft, or units considered as aircraft, or stuff not on map like parasites
		if (pAttacker->WhatAmI() != AircraftClass::AbsID)
		{
			if (pFocus)
			{
				if (auto pFocusOwner = pFocus->GetOwningHouse())
				{
					if (pFocusOwner->IsAlliedWith(pAttacker))
						return 0x6EB47A;
				}
			}

			if (auto pAttackerFoot = abstract_cast<FootClass*>(pAttacker))
			{
				auto IsInTransporter = pAttackerFoot->Transporter && pAttackerFoot->Transporter->GetTechnoType()->OpenTopped;

				if (pAttackerFoot->InLimbo && !IsInTransporter)
				{
					return 0x6EB47A;
				}

				if (IsInTransporter)
					pAttacker = pAttackerFoot->Transporter;

				if (((TechnoClass*)pAttacker)->GetTechnoType()->ConsideredAircraft || pAttacker->WhatAmI() == AircraftClass::AbsID)
					return 0x6EB47A;

				auto first = pThis->FirstUnit;
				if (first)
				{
					auto next = first->NextTeamMember;
					while (!first->IsAlive
						|| !first->Health
						|| !first->IsArmed()
						|| !first->IsTeamLeader && first->WhatAmI() != AircraftClass::AbsID
					)
					{
						first = next;
						if (!next)
							return 0x6EB47A;

						next = next->NextTeamMember;
					}

					pThis->AssignMissionTarget(pAttacker);
				}
			}
		}
	}
#endif
}

void FakeTeamClass::_Coordinate_Attack() {

	if (!this->ArchiveTarget) {
		this->ArchiveTarget = this->QueuedFocus;
	}

	FootClass* teamLeader = this->_Fetch_A_Leader();
	if (!teamLeader) {
		this->StepCompleted = 1;
		return;
	}

	// If target is a cell and team has non-aircraft members, try to find actual object in cell
	CellClass* targetCell = cast_to<CellClass*>(this->ArchiveTarget);

	if (targetCell
		&& this->FirstUnit
		&& teamLeader->WhatAmI() != AircraftClass::AbsID)
	{

		if (auto cellObject = targetCell->GetSomeObject(Point2D::Empty, false)) {
			this->ArchiveTarget = cellObject;
		}
	}

	// Check if team leader can fire at target (every 8 frames, on frame 4)
	if (Unsorted::CurrentFrame % 8 == 4) {
		const int weaponIndex = teamLeader->SelectWeapon(this->ArchiveTarget);

		if (teamLeader->GetFireError(this->ArchiveTarget, weaponIndex, true) == FireError::ILLEGAL) {
			this->TargetNotAssigned = 1;
		}
	}

	// Early exit if no target
	if (!this->ArchiveTarget)
	{
		this->StepCompleted = 1;
		return;
	}

	// Get current mission and prepare for team coordination
	const auto& [curMission, value] = this->CurrentScript->GetCurrentAction();
	FootClass* unitToProcess = this->FirstUnit;
	bool hasValidUnits = false;

	if (!unitToProcess)
	{
		this->StepCompleted = 1;
		return;
	}

	// Process each team member
	do
	{
		// Skip inactive units
		if (!unitToProcess->IsAlive)
		{
			// Special case: droppod units in limbo are still considered valid
			if (this->Type->DropPod && unitToProcess->InLimbo)
			{
				hasValidUnits = true;
			}
			unitToProcess = unitToProcess->NextTeamMember;
			continue;
		}

		// Handle unit initialization (bringing units to formation zone)
		if (unitToProcess->Health
			&& (Unsorted::ScenarioInit || !unitToProcess->InLimbo)
			&& !unitToProcess->IsTeamLeader)
		{
			int allowedStrayDistance = this->_Get_Stray();

			// Check if unit is within allowed distance of formation zone
			if (unitToProcess->DistanceFrom(this->Zone) <= allowedStrayDistance)
			{
				unitToProcess->IsTeamLeader = 1;
			}
			else if (!unitToProcess->Destination)
			{
				// Move unit to formation zone
				unitToProcess->QueueMission(Mission::Move, 0);
				unitToProcess->SetTarget(0);
				unitToProcess->SetDestination(this->Zone, 1);
			}
		}

		// Process active and initiated units for attack coordination
		if (unitToProcess->IsAlive
			&& unitToProcess->Health
			&& (Unsorted::ScenarioInit || !unitToProcess->InLimbo)
			&& (unitToProcess->IsTeamLeader || unitToProcess->WhatAmI() == AircraftClass::AbsID))
		{
			const auto currentMission = unitToProcess->GetCurrentMission();

			// Special handling for infantry capture missions
			if (curMission ==TeamMissionType::Spy
				&& unitToProcess->WhatAmI() == InfantryClass::AbsID
				&& ((InfantryClass*)unitToProcess)->Type->Infiltrate)
			{
				unitToProcess->QueueMission(Mission::Capture, 0);
				unitToProcess->SetTarget(this->ArchiveTarget);
			}
			// Assign attack mission if unit is not already on a critical mission
			else if (currentMission != Mission::Attack
					&& currentMission != Mission::Enter
					&& currentMission != Mission::Capture
					&& currentMission != Mission::Sabotage
					&& (currentMission != Mission::Unload
						|| !unitToProcess->CanDeployNow()))
			{
				unitToProcess->SendToEachLink(RadioCommand::NotifyUnlink);
				unitToProcess->QueueMission(Mission::Attack, 0);
				unitToProcess->SetTarget(0);
				unitToProcess->SetDestination(0, 1);
			}

			// Update unit's target if it doesn't have one or has wrong target

			if (unitToProcess->Target != this->ArchiveTarget && !unitToProcess->Target)
			{
				unitToProcess->SetTarget(this->ArchiveTarget);
			}

			// Check if unit is ready to attack (not aircraft, or has weapon and ammo)
			if (unitToProcess->WhatAmI() != AircraftClass::AbsID
				|| !unitToProcess->IsArmed()
				|| unitToProcess->Ammo > 0)
			{
				hasValidUnits = true;
			}
		}
		else
		{
			// Handle droppod special case for units in limbo
			if (this->Type->DropPod && unitToProcess->InLimbo)
			{
				hasValidUnits = true;
			}
		}

		unitToProcess = unitToProcess->NextTeamMember;
	}
	while (unitToProcess);

	// If no valid units remain, move to next mission
	if (!hasValidUnits)
	{
		this->StepCompleted = 1;
	}
}

void FakeTeamClass::_CoordinateMove() {
	bool finished = true;
	FootClass* pUnit = this->FirstUnit;
	bool found = false;

	if (!pUnit)
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

	// Process each unit in the team
	while (pUnit)
	{
		// ========================================================
		// PHASE 1: Process uninitiated units
		// ========================================================
		if (pUnit->IsAlive &&
			pUnit->Health &&
			(Unsorted::ScenarioInit || !pUnit->InLimbo) &&
			!pUnit->IsTeamLeader)
		{
			const int strayDistance = this->_Get_Stray();

			if (pUnit->DistanceFrom(this->Zone) <= strayDistance)
			{
				pUnit->IsTeamLeader = true;
			}
			else
			{
				if (!pUnit->Destination)
				{
					pUnit->QueueMission(Mission::Move, false);
					pUnit->SetTarget(nullptr);
					pUnit->SetDestination(this->Zone, true);
				}

				finished = false;
			}
		}

		// ========================================================
		// PHASE 2: Check if unloading
		// ========================================================
		if (pUnit->GetCurrentMission() == Mission::Unload ||
			pUnit->QueuedMission == Mission::Unload)
		{
			finished = false;
		}

		// ========================================================
		// PHASE 3: Check if unit should be processed for movement
		// ========================================================
		const bool shouldProcessMovement =
			pUnit->IsAlive &&
			pUnit->Health &&
			(Unsorted::ScenarioInit || !pUnit->InLimbo) &&
			(pUnit->IsTeamLeader || pUnit->WhatAmI() == AbstractType::Aircraft) &&
			pUnit->GetCurrentMission() != Mission::Unload &&
			pUnit->QueuedMission != Mission::Unload;

		if (!shouldProcessMovement)
		{
			pUnit = pUnit->NextTeamMember;
			continue;
		}

		// ========================================================
		// PHASE 4: Calculate stray distance
		// ========================================================
		int strayDistance = this->_Get_Stray();

		if (pUnit->IsInAir())
		{
			strayDistance *= 2;
		}

		found = true;
		int distanceToTarget = pUnit->DistanceFrom(this->ArchiveTarget);

		// ========================================================
		// PHASE 5: Check if unit has arrived at target
		// ========================================================
		bool hasArrived = false;

		if (distanceToTarget <= strayDistance)
		{
			// Check height first (short-circuit evaluation)
			if (pUnit->GetHeight() >= 0)
			{
				// Height OK - check aircraft special case
				if (pUnit->WhatAmI() != AbstractType::Aircraft)
				{
					hasArrived = true;
				}
				else
				{
					// Aircraft - check if landed
					if (pUnit->GetZ() <= 0)
					{
						hasArrived = true;
					}
					else
					{
						// Airborne aircraft - check if at target cell
						CoordStruct unitCoord = pUnit->GetCoords();
						CellClass* pUnitCell = MapClass::Instance->GetCellAt(unitCoord);

						if (pUnitCell == this->ArchiveTarget)
						{
							hasArrived = true;
						}
						else
						{
							// Check if next mission is MOVE
							const auto nextMission = this->CurrentScript->GetNextAction().Action;
							if (nextMission == TeamMissionType::Move)
							{
								hasArrived = true;
							}
						}
					}
				}
			}
			else
			{
				// Height < 0 - only count as arrived if next mission is MOVE
				const auto nextMission = this->CurrentScript->GetNextAction().Action;
				if (nextMission == TeamMissionType::Move)
				{
					hasArrived = true;
				}
			}
		}

		// ========================================================
		// PHASE 6: Handle arrival or continued movement
		// ========================================================
		if (hasArrived)
		{
			// ----------------------------------------------------
			// Unit has arrived
			// ----------------------------------------------------
			if (pUnit->GetCurrentMission() == Mission::Move)
			{
				if (!pUnit->Destination)
				{
					// No destination, idle if not engaged
					if (!pUnit->Target)
					{
						pUnit->SetDestination(nullptr, true);
						pUnit->EnterIdleMode(false, true);
					}
				}
				else if (pUnit->DistanceFrom(pUnit->Destination) <= RulesClass::Instance->CloseEnough)
				{
					if (!pUnit->Locomotor->Is_Really_Moving_Now())
					{
						if (!pUnit->Target)
						{
							pUnit->SetDestination(nullptr, true);
							pUnit->EnterIdleMode(false, true);
						}
					}
				}
			}
		}
		else
		{
			// ----------------------------------------------------
			// Unit hasn't arrived - handle movement
			// ----------------------------------------------------
			bool skipMovement = false;

			// Handle aggressive teams engaging threats
			if (this->Type->Aggressive && pUnit->Target)
			{
				if (pUnit->ShouldLoseTargetNow)
				{
					// Clear target and continue to movement
					pUnit->SetTarget(nullptr);
				}
				else
				{
					// Unit is engaged with target, skip movement processing
					skipMovement = true;
				}
			}

			if (!skipMovement)
			{
				// Ensure MOVE mission
				if (pUnit->GetCurrentMission() != Mission::Move)
				{
					pUnit->QueueMission(Mission::Move, false);

					if (pUnit->ReadyToNextMission())
					{
						pUnit->NextMission();
					}
				}

				// Set destination if needed
				if (!pUnit->Destination)
				{
					pUnit->SetDestination(this->ArchiveTarget, true);
				}

				// Update destination for special cases
				AbstractClass* pNavCom = pUnit->Destination;

				if (pNavCom != this->ArchiveTarget)
				{
					bool shouldReassign = false;

					TechnoTypeClass* pTechnoType = GET_TECHNOTYPE(pUnit);

					if (pTechnoType->BalloonHover)
					{
						shouldReassign = true;
					}
					else if (pNavCom != this->ArchiveTarget &&
							 pUnit->WhatAmI() == AbstractType::Aircraft &&
							 pNavCom == pUnit->GetCell())
					{
						shouldReassign = true;
					}

					if (shouldReassign)
					{
						pUnit->SetDestination(this->ArchiveTarget, true);
					}
				}

				finished = false;
			}
		}

		// ========================================================
		// PHASE 7: Final movement state check
		// (Runs for both arrived and not-arrived units)
		// ========================================================
		bool isAircraftAtCell = false;
		if (pUnit->WhatAmI() == AbstractType::Aircraft)
		{
			if (pUnit->Destination == pUnit->GetCell())
			{
				isAircraftAtCell = true;
			}
		}

		bool isBalloonSettled = GET_TECHNOTYPE(pUnit)->BalloonHover && distanceToTarget < strayDistance;

		if (pUnit->Destination && !isAircraftAtCell && !isBalloonSettled)
		{
			finished = false;
		}

		// Move to next unit
		pUnit = pUnit->NextTeamMember;
	}

	// ============================================================
	// All units processed - check if mission complete
	// ============================================================
	if (found && finished && this->IsMoving)
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
		if (team && team->OwnerHouse == house && team->Type->Priority < priority) {
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
		if (this->CurrentScript->HasMissionsRemaining()) {
			auto const&[Current_Mission , val] = this->CurrentScript->GetCurrentAction();
			if (Current_Mission == TeamMissionType::Move) {
				CellStruct  Waypoint_Location = ScenarioClass::Instance->GetWaypointCoords(val);
				if (!MapClass::Instance->IsWithinUsableArea(Waypoint_Location, 1)) {
					return true;
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
			if (TaskForce->Entries[i].Type->WhatAmI() == AircraftTypeClass::AbsID && TaskForce->Entries[i].Type->Passengers > 0) {
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
		int rating = GET_TECHNOTYPE(member)->LeadershipRating;
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

#ifdef _old
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
			return EligibleToRecruit(pMissType , pMember->GetTechnoType())pMember->GetTechnoType();
		});

		if (it != std::ranges::end(missings))
			missings.erase(it);
	}
}
#else
void FakeTeamClass::_GetTaskForceMissingMemberTypes(std::vector<TechnoTypeClass*>& missings)
{
	const auto pTaskForce = this->Type->TaskForce;

	// Build a map of required units: Type -> Count
	std::unordered_map<TechnoTypeClass*, int> required;

	for (int i = 0; i < pTaskForce->CountEntries; ++i) {
		TechnoTypeClass* technoType = pTaskForce->Entries[i].Type;
		if (technoType) {
			required[technoType] = pTaskForce->Entries[i].Amount;
		}
	}

	// Subtract existing team members
	for (auto pMember = this->FirstUnit; pMember; pMember = pMember->NextTeamMember) {
		TechnoTypeClass* memberType = GET_TECHNOTYPE(pMember);

		// Try exact match first
		if (required.count(memberType) && required[memberType] > 0) {
			required[memberType]--;
		} else {
			// Check if eligible for any required type
			for (auto& [reqType, count] : required) {
				if (count > 0 && TeamExtData::IsEligible(reqType, memberType)) {
					required[reqType]--;
					break;
				}
			}
		}
	}

	// Build final missing list
	missings.clear();
	for (const auto& [technoType, count] : required) {
		missings.insert(missings.end(), count, technoType);
	}
}
#endif

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

	while (GET_TECHNOTYPE(Member)->Ammo > 0 && Member->Ammo <= 0)
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
		if (unit->Owner == team->OwnerHouse && TeamExtData::IsEligible(team->Type->TaskForce->Entries[memberIndex].Type , unit->Type) &&
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
	if (origin.IsValid()) {
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
				this->_Add2(cargo, false);

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
	this->IsMoving = true;
	this->IsHasBeen = true;
	this->IsUnderStrength = false;

	for (FootClass* Member = this->FirstUnit; Member; Member = Member->NextTeamMember) {
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

	if (new_target && new_target != this->QueuedFocus) {
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

bool FakeTeamClass::_Recalculate() {
    bool was_IsUnderStrength = this->IsUnderStrength;
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

		if (was_IsUnderStrength != this->IsUnderStrength) {
            this->IsReforming = 1;
        }

        return 1;
    }

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
    if (!this->IsHasBeen) {
        if (was_IsUnderStrength != this->IsUnderStrength){
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
		((TeamClass*)this)->_scalar_dtor(1);
	}

    return 0;
}

void StopScript(FakeTeamClass* pTeam) {
	if (pTeam->IsFullStrength || pTeam->IsForcedActive) {
		pTeam->_TeamClass_6EA080();
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

template<typename T>
void NOINLINE SearchThruArray(DynamicVectorClass<T>* arr, TeamClass* pTeam, int& minDistance , FootClass*& closestAlly) {
	for (int i = 0; i < arr->Count; i++) {
		T unit = arr->operator[](i);

		if (!IsTechnoMemberEligible(unit, pTeam))
			continue;

		// Calculate distance between unit and first member
		const CoordStruct unitCoord = unit->GetCoords();
		const CoordStruct memberCoord = pTeam->FirstUnit->GetCoords();
		const CoordStruct diff = memberCoord - unitCoord;
		const int distance = (int)diff.Length();

		if (minDistance == -1 || distance < minDistance) {
			minDistance = distance;
			closestAlly = unit;
		}
	}
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
		SearchThruArray(UnitClass::Array(), this, minDistance, closestAlly);

		// Search through all infantry
		SearchThruArray(InfantryClass::Array(), this, minDistance, closestAlly);

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
			TechnoTypeClass* technoType = GET_TECHNOTYPE(member);
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

		int distance = (int)diff.Length();

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
			FootClass* leader = this->_Fetch_A_Leader();

			// Find a safe point near the building for the leader
			CellStruct zoneCell = CellClass::Coord2Cell(
				this->Zone->GetCoords()
			);

			CellStruct safetyPoint = leader->SafetyPoint(zoneCell, buildingCell, 2, 4);

			// Only use this destination if a valid safety point was found
			if (safetyPoint.IsValid())
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

#include <ThemeClass.h>
#include <Ion.h>
enum class BuildingFindType
{
	FIND_SAFEST = 0,        // Lowest threat
	FIND_MOST_THREAT = 1,   // Highest threat
	FIND_CLOSEST = 2,       // Closest (inverted distance)
	FIND_FARTHEST = 3       // Farthest (direct distance)
};

// Calculate scoring for building selection based on criteria
// NOTE: Shared by both Find_Enemy_Building and Find_Own_Building
int CalculateBuildingScore(BuildingClass* building, TechnoClass* searcher, BuildingFindType findType)
{
	switch (findType)
	{
	case BuildingFindType::FIND_SAFEST:
	{
		// Find building in safest area (lowest threat)
		CoordStruct buildingCoord = building->GetCoords();
		CellStruct buildingCell = CellClass::Coord2Cell(buildingCoord);

		int threat = MapClass::Instance->GetThreatPosed(buildingCell, searcher->Owner);
		return 0x7FFFFFFF - threat;  // Invert: lower threat = higher score
	}

	case BuildingFindType::FIND_MOST_THREAT:
	{
		// Find building in most dangerous area (highest threat)
		CoordStruct buildingCoord = building->GetCoords();
		CellStruct buildingCell = CellClass::Coord2Cell(buildingCoord);

		int threat = MapClass::Instance->GetThreatPosed(buildingCell, searcher->Owner);
		return threat;  // Higher threat = higher score
	}

	case BuildingFindType::FIND_FARTHEST:
	{
		// Find building farthest from searcher
		CoordStruct searcherPos = searcher->Location;
		CoordStruct buildingPos = building->Location;
		CoordStruct diff = buildingPos - searcherPos;

		int distance = (int)diff.Length();
		return 0x7FFFFFFF - distance;
	}

	case BuildingFindType::FIND_CLOSEST:
	{
		// Find building closest to searcher
		CoordStruct searcherPos = searcher->Location;
		CoordStruct buildingPos = building->Location;
		CoordStruct diff = buildingPos - searcherPos;;

		int distance = (int)diff.Length();
		return distance;
	}

	default:
		return -1;
	}
}

BuildingClass* Find_Enemy_Building(
		int buildingidx,
		HouseClass* house,
		TechnoClass* attacker,
		BuildingFindType find_type,
		bool onlyTargetHouseEnemy)
{
	if (buildingidx >= BuildingTypeClass::Array->Count) {
		Debug::FatalError("Find_Enemy_Building BuildingType Index is too big(%d of %d) !",
			buildingidx, BuildingTypeClass::Array->Count);
	}

	if (BuildingClass::Array->Count <= 0)
		return nullptr;

	BuildingTypeClass* buildingType = BuildingTypeClass::Array->operator[](buildingidx);
	BuildingClass* bestFriendlyBuilding = nullptr;
	BuildingClass* bestEnemyBuilding = nullptr;
	int bestFriendlyScore = -1;
	int bestEnemyScore = -1;

	// Search through all buildings
	for (int i = 0; i < BuildingClass::Array->Count; i++)
	{
		BuildingClass* building = BuildingClass::Array->Items[i];

		// Must match the requested building type
		if (building->Type != buildingType)
			continue;

		HouseClass* buildingOwner = building->Owner;
		bool isFriendly = (buildingOwner == house);

		// Skip allied buildings unless they're passive multiplayer
		if (!isFriendly)
		{
			if (attacker->Owner->IsAlliedWith(buildingOwner) &&
				!buildingOwner->Type->MultiplayPassive)
			{
				continue;
			}
		}

		// Calculate score based on find type
		int score = CalculateBuildingScore(building, attacker, find_type);

		// Track best friendly and enemy buildings separately
		if (isFriendly)
		{
			if (score > bestFriendlyScore)
			{
				bestFriendlyBuilding = building;
				bestFriendlyScore = score;
			}
		}

		if (score > bestEnemyScore)
		{
			bestEnemyBuilding = building;
			bestEnemyScore = score;
		}
	}

	// Return logic:
	// 1. If only targeting enemies, always return enemy building
	// 2. If friendly building has better score, prefer it
	// 3. Otherwise return enemy building
	if (onlyTargetHouseEnemy)
		return bestEnemyBuilding;

	if (bestFriendlyBuilding && bestFriendlyScore > bestEnemyScore)
		return bestFriendlyBuilding;

	return bestEnemyBuilding;
}

// Find a building of specific type owned by the same house
BuildingClass* Find_Own_Building(
	int buildingidx,
	FootClass* unused,
	TechnoClass* searcher,
	BuildingFindType findType)
{
	HouseClass* house = searcher->Owner;

	if (buildingidx >= BuildingTypeClass::Array->Count)
	{
		Debug::FatalError("Find_Own_Building of [%x - %s] BuildingType Index is too big(%d of %d) !",
			house , house->Type->ID , buildingidx, BuildingTypeClass::Array->Count);
	}

	if (house->Buildings.Count <= 0)
		return nullptr;

	BuildingTypeClass* buildingType = BuildingTypeClass::Array->operator[](buildingidx);

	BuildingClass* bestBuilding = nullptr;
	int bestScore = -1;

	// Search through all buildings owned by this house
	for (int i = 0; i < house->Buildings.Count; i++)
	{
		BuildingClass* building = house->Buildings.Items[i];

		// Must match the requested building type
		if (building->Type != buildingType)
			continue;

		// Calculate score based on find type
		int score = CalculateBuildingScore(building, searcher, findType);

		// Track building with highest score
		if (score > bestScore)
		{
			bestBuilding = building;
			bestScore = score;
		}
	}

	return bestBuilding;
}

void CheckSuperweaponReady(TeamClass* team, SuperClass* super)
{
	int timeLeft = super->RechargeTimer.GetTimeLeft();
	int rechargeTime = super->GetRechargeTime();

	if (super->Granted)
	{
		double percentReady = 1.0 - ((double)timeLeft / (double)rechargeTime);
		if (percentReady >= (1.0 - RulesClass::Instance->AIMinorSuperReadyPercent))
		{
			team->StepCompleted = true;
		}
	}
	else
	{
		team->StepCompleted = true;
	}
}

#include <Ext/SWType/Body.h>

bool NOINLINE ShouldFindNearbyLocation(TeamClass* team)
{
	// Don't search for nearby if any of these conditions are true:

	// 1. Team is not moving yet
	if (!team->IsMoving)
		return true;

	// 2. Script has no more missions
	if (!team->CurrentScript->HasMissionsRemaining())
		return true;

	// 3. Current mission is not MOVE
	const auto& [mission, val] = team->CurrentScript->GetCurrentAction();

	if (mission != TeamMissionType::Move)
		return true;

	// 4. Next waypoint in script is visible on radar
	CellStruct nextWaypoint = ScenarioClass::Instance->GetWaypointCoords(val);

	if (MapClass::Instance->IsWithinUsableArea(nextWaypoint, true)) {
		return true;
	}

	// None of the conditions met, don't search for nearby
	return false;
}

// Helper function (reused from previous artifacts)
bool ProcessMemberInitiation(FakeTeamClass* team, FootClass* member)
{
	if (!member->IsAlive || !member->Health)
		return false;

	if (!Unsorted::ScenarioInit && member->InLimbo)
		return false;

	if (member->IsTeamLeader)
		return false;

	int strayDistance = team->_Get_Stray();

	// Check if close enough to zone to be initiated
	if (member->DistanceFrom(team->Zone) <= strayDistance)
	{
		member->IsTeamLeader = true;
	}
	else if (!member->Destination)
	{
		// Send member to zone
		member->QueueMission(Mission::Move, 0);
		member->SetTarget(nullptr);
		member->SetDestination(team->Zone, 1);
	}

	return true;
}

// Process transport units after unloading
void ProcessTransports(FakeTeamClass* team, TeamMissionType nextMission, bool hasAircraftInTaskForce)
{
	FootClass* member = team->FirstUnit;

	while (member)
	{
		FootClass* next = member->NextTeamMember;
		TechnoTypeClass* technoType = GET_TECHNOTYPE(member);
		bool shouldRemove = false;

		// Skip non-transport units
		if (technoType->Passengers <= 0)
		{
			shouldRemove = true;
		}

		// If task force has aircraft, only process aircraft transports
		else if (hasAircraftInTaskForce)
		{
			if (member->WhatAmI() != AircraftClass::AbsID)
			{
				shouldRemove = true;
			}
		}

		if (shouldRemove)
		{
			if (nextMission == TeamMissionType::Move ||
				 nextMission == TeamMissionType::Att_waypt)
			{
				team->_Remove(member, -1, false);
				member->SetTarget(nullptr);
				member->SetDestination(nullptr, 1);
			}
		}
		else
		{
			if (technoType->Passengers > 0)
			{
				// Handle transport based on team settings
				if (team->Type->TransportsReturnOnUnload)
				{
					// Transport returns to archived target (usually home base)
					team->_Remove(member, -1, false);
					member->SetTarget(nullptr);
					member->SetDestination(member->ArchiveTarget, 1);
					member->QueueMission(Mission::Move, 0);

					if (member->ReadyToNextMission())
					{
						member->NextMission();
					}

					member->ArchiveTarget = nullptr;
				}

				else if (nextMission == TeamMissionType::Move ||
				nextMission == TeamMissionType::Go_bezerk)
				{
					team->_Remove(member, -1, false);
					member->SetTarget(nullptr);
					member->SetDestination(nullptr, 1);
				}
			}
		}

		member = next;
	}
}

void FakeTeamClass::_TMission_Unload(ScriptActionNode* nNode, bool arg3)
{
	FootClass* member = this->FirstUnit;

	if (!member) {
		this->StepCompleted = true;
		return;
	}

	bool allUnloaded = true;

	// First pass: Process each member for unloading
	FootClass* current = member;
	while (current) {
		FootClass* next = current->NextTeamMember;

		if (current->IsAlive) {
			// Initialize uninitiated members
			ProcessMemberInitiation(this, current);

			// Process initiated members with cargo
			if (current->IsAlive && current->Health &&
				(Unsorted::ScenarioInit || !current->InLimbo) &&
				(current->IsTeamLeader || current->WhatAmI() == AircraftClass::AbsID)) {
				// Check if member has cargo
				if (current->Passengers.NumPassengers)
				{
					allUnloaded = false;

					// Get member's current cell
					CellClass* memberCell = current->GetCell();

					// Only unload if not in a building and not already unloading
					if (!memberCell->GetBuilding() &&
						current->GetCurrentMission() != Mission::Unload) {
						// Start unloading
						current->SetDestination(nullptr, 1);
						current->SetTarget(nullptr);
						current->QueueMission(Mission::Unload, 0);
					}
				}
			}
		}

		current = next;
	}

	// If all members have unloaded, process transports
	if (!allUnloaded) {
		return;
	}

	bool hasAircraftInTaskForce = this->_has_aircraft();

	// Process transport units
	ProcessTransports(this, nNode->Action, hasAircraftInTaskForce);

	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Load(ScriptActionNode* nNode, bool arg3)
{
	// Check if task force has aircraft with cargo capacity
	bool hasAircraftTransport = false;
	TaskForceClass* taskForce = this->Type->TaskForce;

	for (int i = 0; i < taskForce->CountEntries; i++)
	{
		TechnoTypeClass* technoType = taskForce->Entries[i].Type;

		if (technoType->WhatAmI() == AircraftTypeClass::AbsID &&
			technoType->Passengers > 0)
		{
			hasAircraftTransport = true;
			break;
		}
	}

	FootClass* member = this->FirstUnit;
	if (!member)
	{
		this->StepCompleted = true;
		return;
	}

	// Find a transport that needs loading
	FootClass* transport = nullptr;
	while (member)
	{
		TechnoTypeClass* memberType = GET_TECHNOTYPE(member);

		// Must be a transport (has passengers)
		if (memberType->Passengers <= 0)
		{
			member = member->NextTeamMember;
			continue;
		}

		// Must not be full
		if (member->Passengers.NumPassengers >= memberType->Passengers)
		{
			member = member->NextTeamMember;
			continue;
		}

		// If task force has aircraft, only use aircraft transports
		if (hasAircraftTransport)
		{
			if (member->WhatAmI() != AircraftClass::AbsID)
			{
				member = member->NextTeamMember;
				continue;
			}
		}
		else
		{
			// Only use ground units for ground transport
			if (member->WhatAmI() != UnitClass::AbsID)
			{
				member = member->NextTeamMember;
				continue;
			}
		}

		// Found a valid transport
		transport = member;
		break;
	}

	if (!transport)
	{
		this->StepCompleted = true;
		return;
	}

	// If transport is in radio contact, wait
	if (transport->HasAnyLink())
	{
		return;
	}

	// Load all other team members into the transport
	bool allLoaded = true;
	FootClass* unit = this->FirstUnit;

	while (unit)
	{
		// Skip if only one member in team (the transport itself)
		if (this->TotalObjects <= 1)
			break;

		if (!unit->IsAlive)
		{
			unit = unit->NextTeamMember;
			continue;
		}

		// Initialize uninitiated members
		ProcessMemberInitiation(this, unit);

		// Process initiated members
		if (unit->IsAlive &&
			unit->Health &&
			(Unsorted::ScenarioInit || !unit->InLimbo) &&
			(unit->IsTeamLeader || unit->WhatAmI() == AircraftClass::AbsID))
		{
			// Don't load the transport into itself
			if (unit != transport)
			{
				allLoaded = false;

				// Assign ENTER mission if not already entering
				if (unit->GetCurrentMission() != Mission::Enter)
				{
					unit->QueueMission(Mission::Enter, 0);
					unit->SetTarget(nullptr);
					unit->SetDestination(transport, 1);
					return; // Wait for this unit to enter
				}
			}
		}

		unit = unit->NextTeamMember;
	}

	// All units loaded
	if (allLoaded)
	{
		// If transports should return home, save current location
		if (this->Type->TransportsReturnOnUnload)
		{
			transport->SetArchiveTarget(transport->GetCell());
		}

		this->StepCompleted = true;
	}
}

void FakeTeamClass::_TMission_Deploy(ScriptActionNode* nNode, bool arg3)
{
	FootClass* member = this->FirstUnit;

	if (!member)
	{
		this->StepCompleted = true;
		return;
	}

	bool allDeployed = true;

	do
	{
		if (!member->IsAlive)
		{
			member = member->NextTeamMember;
			continue;
		}

		// Initialize uninitiated members
		ProcessMemberInitiation(this, member);

		// Process initiated members for deployment
		if (member->Health && (Unsorted::ScenarioInit || !member->InLimbo) &&
			(member->IsTeamLeader || member->WhatAmI() == AircraftClass::AbsID))
		{
			bool canDeploy = false;
			bool isUnit = (member->WhatAmI() == UnitClass::AbsID);

			// Check for MCV deployment (unit that deploys into building)
			if (isUnit)
			{
				UnitClass* unit = (UnitClass*)member;
				if (unit->Type->DeploysInto)
				{
					canDeploy = true;
					allDeployed = false;

					if (member->GetCurrentMission() != Mission::Unload)
					{
						// Check if placement is legal
						member->Mark(MarkType::Up);
						CellStruct unitCell = member->GetMapCoords();

						if (!unit->Type->DeploysInto->CanCreateHere(unitCell, member->Owner))
						{
							// Not legal placement, try to flush area
							CellStruct currentCell = member->GetMapCoords();
							unit->Type->DeploysInto->FlushPlacement(&currentCell, member->Owner);
							member->Mark(MarkType::Down);
							// Handle searching for free space in Hunt mission handler
							member->ForceMission(Mission::Hunt);
							member->MissionStatus = 2; // Tells UnitClass::Mission_Hunt to omit certain checks.

						}
						else
						{
							member->Mark(MarkType::Down);
							// Legal placement, start deployment
							member->SetDestination(nullptr, 1);
							member->SetTarget(nullptr);
							member->QueueMission(Mission::Unload, 0);
						}
					}
				}
			}

			// Check for simple deployer (like siege choppers)
			bool isSimpleDeployer = isUnit && ((UnitClass*)member)->Type->IsSimpleDeployer;

			// Check for engineer/spy deployment
			bool isInfantryDeployer = false;
			if (auto pUnit = cast_to<InfantryClass*>(member))
				isInfantryDeployer = pUnit->Type->Deployer;

			if ((isSimpleDeployer || isInfantryDeployer) && !canDeploy)
			{
				if (member->GetCurrentMission() != Mission::Unload)
				{
					member->QueueMission(Mission::Unload, 0);
					allDeployed = false;
				}
			}
		}

		member = member->NextTeamMember;
	}
	while (member);

	if (allDeployed) {
		this->StepCompleted = true;
	}
}

void FakeTeamClass::_TMission_Scout(ScriptActionNode* nNode, bool arg3)
{
	bool shouldContinue = true;

		if (this->TargetHouse)
		{
			// Already scouting a house, check if movement is done
			FootClass* leader = this->_Fetch_A_Leader();

			if (leader &&
				leader->CurrentMission != Mission::Move &&
				leader->QueuedMission != Mission::Move)
			{
				// Movement complete, mark house as scouted
				this->OwnerHouse->UpdateScoutNodes(this->TargetHouse);
				this->TargetHouse = nullptr;
			}
		}
		else
		{
			// Find an unscouted house
			HouseClass* house = this->OwnerHouse;
			int unscoutedCount = 0;

			// Count unscouted nodes
			for (int i = 0; i < house->ScoutNodes.Count; i++) {
				if (!house->ScoutNodes.Items[i].IsPreferred) // Check scouted flag
				{
					unscoutedCount++;
				}
			}

			if (unscoutedCount <= 0)
			{
				shouldContinue = false;
			}
			else
			{
				// Pick random unscouted house
				int randomPick = ScenarioClass::Instance->Random.RandomRanged(0, unscoutedCount - 1);

				for (int i = 0; i < house->ScoutNodes.Count; i++) {
					if (!house->ScoutNodes.Items[i].IsPreferred) {
						if (randomPick-- == 0) {
							this->TargetHouse = house->ScoutNodes.Items[i].House;
							break;
						}
					}
				}

				auto IsValidTarget = [](BuildingClass* pBld) -> bool {
					if (!pBld->IsAlive)
						return false;

					const auto pExt = BuildingExtContainer::Instance.Find(pBld);
					if (pExt->LimboID != -1)
						return false;

					if (pBld->Type->InvisibleInGame)
						return false;

					if (pExt->Type->IsDummy)
						return false;

					return true;
				};

				// Find a building owned by the scout target house
				std::vector<BuildingClass*> targetBuildings;
				targetBuildings.reserve(this->TargetHouse->Buildings.Count);

				for (int i = 0; i < this->TargetHouse->Buildings.Count; i++) {
					if (IsValidTarget(this->TargetHouse->Buildings.Items[i])) {
						targetBuildings.push_back(this->TargetHouse->Buildings.Items[i]);
					}
				}
					
				if (targetBuildings.empty()) {
					// No buildings found, mark as scouted
					this->OwnerHouse->UpdateScoutNodes(this->TargetHouse);
					this->TargetHouse = nullptr;
				} else {
					// Pick random building
					const BuildingClass* target = targetBuildings[ScenarioClass::Instance->Random.RandomRanged(0, targetBuildings.size() - 1)];

					// Find leader and nearby location
					if (FootClass* leader = this->_Fetch_A_Leader())
					{
						CoordStruct targetCoord = target->GetCoords();
						CellStruct targetCell = CellClass::Coord2Cell(targetCoord);

						TechnoTypeClass* leaderType = GET_TECHNOTYPE(leader);

						CellStruct result = MapClass::Instance->NearByLocation(
							targetCell,
							leaderType->SpeedType,
							ZoneType::None,
							MovementZone::Normal,
							false, 1, 1, 0, 0, 0, 1,
							CellStruct::Empty,
							0, 0
						);

						if (result.IsValid()) {
							this->_AssignMissionTarget(MapClass::Instance->GetCellAt(result));
						} else {
							this->_AssignMissionTarget(nullptr);
						}
					}
				}
			}
		}

		this->_CoordinateMove();
		this->StepCompleted = shouldContinue ? false : true;
}

void FakeTeamClass::_TMission_Move_To_Own_Building(ScriptActionNode* nNode, bool arg3)
{
	if (arg3) {
		if (!this->QueuedFocus) {
			FootClass* member = this->FirstUnit;

			if (!member) {
				this->StepCompleted = true;
				return;
			}

			// Unpack argument
			int packedArg = nNode->Argument;
			int buildingTypeIndex = packedArg & 0xFFFF;
			int findMode = packedArg >> 16;

			// Get enemy house
			HouseClass* enemyHouse = nullptr;
			int enemyIndex = member->Owner->EnemyHouseIndex;
			if (enemyIndex != -1) {
				enemyHouse = HouseClass::Array->Items[enemyIndex];
			}

			// Find enemy building
			if (BuildingClass* targetBuilding = Find_Own_Building(
				buildingTypeIndex,
				0,
				member,
				(BuildingFindType)findMode
			)) {
				// Find nearby accessible location
				CoordStruct buildingCoord = targetBuilding->Location;
				CellStruct buildingCell = CellClass::Coord2Cell(buildingCoord);

				CoordStruct memberCoord = member->GetCoords();
				CellStruct memberCell = CellClass::Coord2Cell(memberCoord);

				TechnoTypeClass* memberType = GET_TECHNOTYPE(member);

				ZoneType memberZone = MapClass::Instance->GetMovementZoneType(
					&memberCell,
					memberType->MovementZone,
					member->OnBridge
				);

				const CellStruct nearbyCell = MapClass::Instance->NearByLocation(
					buildingCell,
					memberType->SpeedType,
					memberZone,
					memberType->MovementZone,
					false, 1, 1, 0, 0, 0, 1,
					CellStruct::Empty,
					0, 0
				);

				if (nearbyCell.IsValid()) {
					this->_AssignMissionTarget(MapClass::Instance->GetCellAt(nearbyCell));
				} else {
					this->_AssignMissionTarget(nullptr);
				}
			}
		} else {
			this->_CoordinateMove();
			return;
		}
	}

	if (!this->QueuedFocus) {
		this->StepCompleted = true;
		return;
	}

	this->_CoordinateMove();
}

void FakeTeamClass::_TMission_Attack_Enemy_Building(ScriptActionNode* nNode, bool arg3)
{
	if (arg3)
	{
		if (this->QueuedFocus)
		{
			if (!this->QueuedFocus || !this->_Does_Any_Member_Have_Ammo())
			{
				this->_AssignMissionTarget(nullptr);
				this->StepCompleted = true;
				return;
			}

			this->_Coordinate_Attack();
			return;
		}

		FootClass* member = this->FirstUnit;
		if (!member) {
			this->_AssignMissionTarget(nullptr);
			this->StepCompleted = true;
			return;
		}

		// Unpack argument: lower 16 bits = building type, upper 16 bits = find mode
		int packedArg = nNode->Argument;
		int buildingTypeIndex = packedArg & 0xFFFF;
		int findMode = packedArg >> 16;

		// Validate building type index
		if (buildingTypeIndex >= BuildingTypeClass::Array->Count)
		{
			Debug::FatalError("Team [%s] TMission_Attack_Enemy_Building: Invalid building type index %d (max: %d)",
				this->Type->ID, buildingTypeIndex, BuildingTypeClass::Array->Count - 1);
			this->_AssignMissionTarget(nullptr);
			this->StepCompleted = true;
			return;
		}

		// Get enemy house
		HouseClass* enemyHouse = nullptr;
		int enemyIndex = member->Owner->EnemyHouseIndex;
		if (enemyIndex != -1) {
			enemyHouse = HouseClass::Array->Items[enemyIndex];
		}

		// Find enemy building
		if (BuildingClass* targetBuilding = Find_Enemy_Building(
			buildingTypeIndex,
			enemyHouse,
			member,
			(BuildingFindType)findMode,
			this->Type->OnlyTargetHouseEnemy
		)) {
			this->_AssignMissionTarget(targetBuilding);
		}
	}

	if (!this->QueuedFocus || !this->_Does_Any_Member_Have_Ammo()) {
		this->_AssignMissionTarget(nullptr);
		this->StepCompleted = true;
		return;
	}

	this->_Coordinate_Attack();
}

void FakeTeamClass::_TMission_Chrono_prep_for_abwp(ScriptActionNode* nNode, bool arg3)
{
	FootClass* leader = this->_Fetch_A_Leader();
	if (!leader) {
		this->StepCompleted = true;
		return;
	}

	HouseClass* house = leader->Owner;

	// Find Chronosphere and Chronoshift superweapons
	SuperClass* chronosphere = nullptr;
	SuperClass* chronoshift = nullptr;

	for (int i = 0; i < house->Supers.Count; i++) {

		SuperClass* super = house->Supers.Items[i];
		SWTypeExtData* pExt = SWTypeExtContainer::Instance.Find(super->Type);

		if (!pExt->IsAvailable(house))
			continue;

		if (super->Type->Type == SuperWeaponType::ChronoSphere) // Chronosphere
			chronosphere = super;
		
		if (super->Type->Type == SuperWeaponType::ChronoWarp) // Chronoshift
			chronoshift = super;
	}

	if (!chronosphere || !chronoshift) {
		this->StepCompleted = true;
		return;
	}

	// Check if ready
	if (chronosphere->IsCharged && house->GetPowerPercentage() >= 1.0)
	{
		// Find enemy building target
		HouseClass* enemyHouse = nullptr;
		int enemyIndex = leader->Owner->EnemyHouseIndex;
		if (enemyIndex != -1) {
			enemyHouse = HouseClass::Array->Items[enemyIndex];
		}

		int packedArg = nNode->Argument;
		int buildingTypeIndex = packedArg & 0xFFFF;
		int findMode = packedArg >> 16;

		// Validate building type index
		if (buildingTypeIndex >= BuildingTypeClass::Array->Count) {
			Debug::FatalError("Team [%s] TMission_CHRONO_PREP_FOR_ABWP: Invalid building type index %d (max: %d)",
				this->Type->ID, buildingTypeIndex, BuildingTypeClass::Array->Count - 1);
			this->StepCompleted = true;
			return;
		}

		if (BuildingClass* targetBuilding = Find_Enemy_Building(
			buildingTypeIndex,
			enemyHouse,
			leader,
			(BuildingFindType)findMode,
			this->Type->OnlyTargetHouseEnemy
		)) {
			// Fire Chronosphere at team zone
			CoordStruct zoneCoord = this->Zone->GetCoords();
			CellStruct zoneCell = CellClass::Coord2Cell(zoneCoord);
			house->Fire_SW(chronosphere->Type->ArrayIndex, zoneCell);

			// Fire Chronoshift at target building
			CoordStruct targetCoord = targetBuilding->GetCoords();
			CellStruct targetCell = CellClass::Coord2Cell(targetCoord);
			house->Fire_SW(chronoshift->Type->ArrayIndex, targetCell);
			this->_AssignMissionTarget(targetBuilding);
		}

		this->StepCompleted = true;
	}
	else {
		CheckSuperweaponReady(this, chronosphere);
	}
}

void FakeTeamClass::_TMission_Play_Animation(ScriptActionNode* nNode, bool arg3)
{
	if (!arg3) {
		this->StepCompleted = true;
		return;
	}

	int packedArg = nNode->Argument;
	int animIndex = packedArg & 0xFFFF;
	int loopCount = packedArg >> 16;

	// Validate anim index
	if (animIndex >= AnimTypeClass::Array->Count) {
		Debug::FatalErrorAndExit("Team [%s] TMission_Play_Animation: Invalid anim type index %d (max: %d)",
			this->Type->ID, animIndex, AnimTypeClass::Array->Count - 1);

		this->StepCompleted = true;
		return;
	}

	AnimTypeClass* animType = AnimTypeClass::Array->Items[animIndex];
	FootClass* member = this->FirstUnit;

	if (!member || !animType) {
		this->StepCompleted = true;
		return;
	}

	// Create animation on each team member
	do
	{
		{
			CoordStruct memberCoord = member->GetCoords();
			if (AnimClass* created = GameCreate<AnimClass>(animType,
				memberCoord,
				0,
				loopCount,
				AnimFlag::AnimFlag_600,
				0,
				0
			)) {
				created->SetOwnerObject(member);
			}
		}

		member = member->NextTeamMember;
	}
	while (member);

	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Iron_Curtain_Me(ScriptActionNode* nNode, bool arg3)
{
	const auto pLeader = this->_Fetch_A_Leader();

	if (!pLeader)
	{
		this->StepCompleted = true;
		return;
	}
	const auto pOwner = this->OwnerHouse;

	if (pOwner->Supers.Count <= 0)
	{
		this->StepCompleted = true;
		return;
	}

	const bool havePower = pOwner->HasFullPower();
	SuperClass* obtain = nullptr;
	bool found = false;

	for (const auto& pSuper : pOwner->Supers)
	{
		const auto pExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

		if (!found && pExt->SW_AITargetingMode == SuperWeaponAITargetingMode::IronCurtain && pExt->SW_Group == nNode->Argument)
		{
			if (!pExt->IsAvailable(pOwner))
				continue;

			// found SW that already charged , just use it and return
			if (pSuper->IsCharged && (havePower || !pSuper->IsPowered()))
			{
				obtain = pSuper;
				found = true;

				continue;
			}

			if (!obtain && pSuper->Granted)
			{
				double rechargeTime = (double)pSuper->GetRechargeTime();
				double timeLeft = (double)pSuper->RechargeTimer.GetTimeLeft();

				if ((1.0 - RulesClass::Instance->AIMinorSuperReadyPercent) < (timeLeft / rechargeTime))
				{
					obtain = pSuper;
					found = false;
					continue;
				}
			}
		}
	}

	if (found)
	{
		auto nCoord = this->Zone->GetCoords();
		pOwner->Fire_SW(obtain->Type->ArrayIndex, CellClass::Coord2Cell(nCoord));
	}

	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Chrono_prep_for_aq(ScriptActionNode* nNode, bool arg3)
{
	FootClass* leader = _Fetch_A_Leader();
	if (!leader)
	{
		this->StepCompleted = true;
		return;
	}

	HouseClass* house = leader->Owner;

	// Find superweapons
	SuperClass* chronosphere = nullptr;
	SuperClass* chronoshift = nullptr;

	for (int i = 0; i < house->Supers.Count; i++)
	{
		SuperClass* super = (SuperClass*)house->Supers.Items[i];
		SWTypeExtData* pExt = SWTypeExtContainer::Instance.Find(super->Type);

		if (!pExt->IsAvailable(house))
			continue;

		if (super->Type->Type == SuperWeaponType::ChronoSphere) chronosphere = super;
		if (super->Type->Type == SuperWeaponType::ChronoWarp) chronoshift = super;
	}

	if (!chronosphere || !chronoshift)
	{
		this->StepCompleted = true;
		return;
	}

	if (chronosphere->IsCharged && house->GetPowerPercentage() >= 1.0)
	{
		// Find threat target
		ThreatType threatType = TeamClass::ThreatFromQuarry((QuarryType)nNode->Argument);
		CoordStruct leaderCoord = leader->Location;

		if (AbstractClass* threat = leader->GreatestThreat(
			threatType,
			&leaderCoord,
			this->Type->OnlyTargetHouseEnemy
		))
		{
			// Fire Chronosphere at zone
			CoordStruct zoneCoord = this->Zone->GetCoords();
			CellStruct zoneCell = CellClass::Coord2Cell(zoneCoord);
			house->Fire_SW(chronosphere->Type->ArrayIndex, zoneCell);

			// Fire Chronoshift at threat
			CoordStruct threatCoord = threat->GetCoords();
			CellStruct threatCell = CellClass::Coord2Cell(threatCoord);
			house->Fire_SW(chronoshift->Type->ArrayIndex, threatCell);
			this->_AssignMissionTarget(threat);
		}

		this->StepCompleted = true;
	}
	else
	{
		CheckSuperweaponReady(this, chronosphere);
	}

}

void FakeTeamClass::_TMission_Enter_Grinder(ScriptActionNode* nNode, bool arg3)
{
	FootClass* Member = this->FirstUnit;
	FootClass* NextMember = nullptr;
	if (Member)
	{
		do
		{
			NextMember = Member->NextTeamMember;
			if (Member->EnterGrinder())
			{
				this->_Remove(Member, -1, true);
			}
			Member = NextMember;
		}
		while (NextMember);
	}

	this->StepCompleted = 1;
}

void FakeTeamClass::_TMission_Enter_Bio_Reactor(ScriptActionNode* nNode, bool arg3)
{
	FootClass* Member = this->FirstUnit;
	FootClass* NextMember = nullptr;
	if (Member)
	{
		do
		{
			NextMember = Member->NextTeamMember;
			if (Member->EnterBioReactor())
			{
				this->_Remove(Member, -1, true);
			}
			Member = NextMember;
		}
		while (NextMember);
	}

	this->StepCompleted = 1;
}

void FakeTeamClass::_TMission_Occupy_Battle_Bunker(ScriptActionNode* nNode, bool arg3)
{
	FootClass* Member = this->FirstUnit;
	FootClass* NextMember = nullptr;
	if (Member)
	{
		do
		{
			NextMember = Member->NextTeamMember;
			if (Member->EnterBattleBunker())
			{
				this->_Remove(Member, -1, true);
			}
			Member = NextMember;
		}
		while (NextMember);
	}

	this->StepCompleted = 1;
}

void FakeTeamClass::_TMission_Occupy_Tank_Bunker(ScriptActionNode* nNode, bool arg3)
{
	FootClass* Member = this->FirstUnit;
	FootClass* NextMember = nullptr;
	if (Member)
	{
		do
		{
			NextMember = Member->NextTeamMember;
			if (Member->EnterTankBunker())
			{
				this->_Remove(Member, -1, true);
			}
			Member = NextMember;
		}
		while (NextMember);
	}

	this->StepCompleted = 1;
}

void FakeTeamClass::_TMission_Attack(ScriptActionNode* nNode, bool arg3)
{
	if (!this->QueuedFocus) {
		if (FootClass* pLeader = this->_Fetch_A_Leader()) {
			const ThreatType tt = ThreatFromQuarry((QuarryType)nNode->Argument);
			this->_AssignMissionTarget(pLeader->GreatestThreat(tt, &pLeader->Location, this->Type->OnlyTargetHouseEnemy));
		}
	}

	if (!this->QueuedFocus || !this->_Does_Any_Member_Have_Ammo()) {
		this->StepCompleted = true;
		return;
	}

	this->_Coordinate_Attack();
}

void FakeTeamClass::_TMission_Attack_Waypoint(ScriptActionNode* nNode, bool arg3)
{
	if (arg3) {
		CellClass* pWaypCell = ScenarioClass::Instance->GetWaypointCell(nNode->Argument);
		AbstractClass* pTarget = nullptr;

		if (pWaypCell && pWaypCell->WhatAmI() == CellClass::AbsID) {
			bool revealed = (pWaypCell->Flags & CellFlags::CenterRevealed) != CellFlags::Empty;
			if (auto pObj = pWaypCell->GetSomeObject(Point2D::Empty, revealed)) {
				pTarget = pObj;
			} else {
				pTarget = pWaypCell;
			}
		}

		this->_AssignMissionTarget(pTarget);
	}

	if (this->QueuedFocus && this->_Does_Any_Member_Have_Ammo()) {
		this->_Coordinate_Attack();
	} else {
		this->_AssignMissionTarget(nullptr);
		this->StepCompleted = true;
	}
}

void FakeTeamClass::_TMission_Move_To_Cell(ScriptActionNode* nNode, bool arg3)
{
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;
	CellStruct toCell { 
		short(nNode->Argument % nDivisor),
		short(nNode->Argument / nDivisor)
	};
	this->_AssignMissionTarget(MapClass::Instance->GetCellAt(toCell));
	this->_CoordinateMove();
}

void FakeTeamClass::_TMission_Loop(ScriptActionNode* nNode, bool arg3)
{
	this->CurrentScript->SetMission(nNode->Argument - 2); //????
	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Player_wins(ScriptActionNode* nNode, bool arg3)
{
	HouseClass::CurrentPlayer->Win(false);
	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Player_loses(ScriptActionNode* nNode, bool arg3)
{
	HouseClass::CurrentPlayer->Lose(false);
	this->StepCompleted = true;
}

void FakeTeamClass::_TMission_Talk_bubble(ScriptActionNode* nNode, bool arg3)
{
	if(arg3){
		if (this->FirstUnit)
			this->FirstUnit->CreateTalkBubble(nNode->Argument);

		this->StepCompleted = true;
	}
}

void FakeTeamClass::_AI()
{
	//HouseExtContainer::HousesTeams[this->OwnerHouse].emplace(this);
#pragma region UpdateFuncs

	if (this->IsSuspended) {
		if(!this->SuspendTimer.HasTimeLeft()) {
			this->IsSuspended = false;
		} else {
			return;
		}
	}

	if (this->NeedsToDisappear && !this->_Recalculate())
	{
		return;
	}

	if (this->IsMoving)
	{
		if (this->IsUnderStrength)
		{
			this->_Regroup();
		}
	}

	if (!this->IsMoving)
	{
		StopScript(this);
	}

	if (this->IsReforming || this->IsMoving || !this->Zone || !this->ClosestMember)
	{
		this->_Calc_Center(&this->Zone, &this->ClosestMember);
	}

	const bool shouldRecruit = (!this->IsMoving || (!this->IsFullStrength && this->Type->Reinforce));
	const bool playerCheck = (!this->OwnerHouse->IsControlledByHuman() || !this->IsHasBeen);

	if (shouldRecruit && playerCheck)
	{
		TaskForceClass* pTaskForce = this->Type->TaskForce;

		for (int i = 0; i < pTaskForce->CountEntries; ++i) {
			if (this->CountObjects[i] < pTaskForce->Entries[i].Amount)
			{
				this->_Recruit(i);
			}
			pTaskForce = this->Type->TaskForce;  // Refresh in case it changed
		}
	}

	FootClass* pFirstUnit = this->FirstUnit;

	if (!pFirstUnit)
	{
		bool shouldDissolve = this->IsHasBeen;
		if (!shouldDissolve && SessionClass::Instance->GameMode != GameMode::Campaign) {
			int elapsed = Unsorted::CurrentFrame - this->CreationFrame;
			if (elapsed > RulesClass::Instance->DissolveUnfilledTeamDelay) {
				shouldDissolve = true;
			}
		}

		if (shouldDissolve)
		{
			// Trigger leave map events if applicable
			if (this->IsLeavingMap) {
				for (int i = 0; i < TagClass::ActiveTags->Count; ++i) {
					if (TagClass::ActiveTags->Items[i]->SpringEvent(TriggerEvent::TeamLeavesMap, nullptr, CellStruct::Empty)) {
						--i;
						if (TagClass::ActiveTags->Count == 0) {
							break;
						}
					}
				}
			}

			// Destroy the team
			((TeamClass*)this)->_scalar_dtor(1);
			return;
		}

		// Team has no members but shouldn't dissolve yet
		// Fall through to coordinate move
		this->_CoordinateMove();
		return;
	}

	bool isBeingWarped = pFirstUnit->IsBeingWarpedOut();

	if (!this->IsMoving)
	{
		this->_CoordinateMove();
		return;
	}

	if (this->IsReforming || this->IsUnderStrength || !isBeingWarped)
	{
		this->IsReforming = !this->_CoordinateRegroup();
		return;
	}

	bool missionChanged = false;

	if (this->StepCompleted)
	{
		missionChanged = true;

		this->StepCompleted = false;
		this->CurrentScript->NextMission();

		for (FootClass* j = this->FirstUnit; j; j = j->NextTeamMember)
		{
			if (this->Type->TransportsReturnOnUnload && GET_TECHNOTYPE(j)->Passengers > 0) {
				if (j->ArchiveTarget) {
					Debug::LogInfo("[{}][{}] Transport just recieved orders to go home after unloading ", (void*)this, this->get_ID());
				}
			}
			else
			{
				j->SetArchiveTarget(nullptr);
			}
		}

		if (!this->CurrentScript->HasMissionsRemaining())
		{
			((TeamClass*)this)->_scalar_dtor(1);
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
		missionChanged = true;
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
#pragma endregion

	ScriptActionNode node = this->CurrentScript->GetCurrentAction();

	//dont prematurely finish the `Script` ,...
	//bailout the script if the `Action` already -1
	//this will free the Member and allow them to be recuited
	//if (node.Action == TeamMissionType::none || (TeamMissionType)node.Action >= TeamMissionType::count && 
	//	(AresScripts)node.Action >= AresScripts::count && 
	//	(PhobosScripts)node.Action >= PhobosScripts::count
	//) {
	//	// Unknown action. This action finished
	//	this->StepCompleted = true;
	//	this->NeedsToDisappear = true;
	//
	//	auto const pAction = this->CurrentScript->GetCurrentAction();
	//	Debug::LogInfo("AI Scripts : [{}] Team [{}][{}]  ( {} CurrentScript {} / {} line {}): Unknown Script Action: {} (action={}, arg={})",
	//		(void*)this,
	//		this->Type->ID,
	//		this->Type->Name,
	//
	//		(void*)this->CurrentScript,
	//		this->CurrentScript->Type->ID,
	//		this->CurrentScript->Type->Name,
	//		this->CurrentScript->CurrentMission,
	//
	//		(int)pAction.Action, (int)node.Action, node.Argument);
	//
	//	((TeamClass*)this)->_scalar_dtor(1);
	//	return;
	//}

	std::string first = GameStrings::NoneStr();
	if (this->FirstUnit)
		first = this->FirstUnit->get_ID();

	//Debug::Log("Team [%x - %s] with FirstUnit [%x - %s] , executing [%d arg %d] \n", this, this->Type->ID, this->FirstUnit, first.c_str(), (int)node.Action ,node.Argument);

	switch (node.Action)
	{
	case TeamMissionType::Unload:
	{
		this->_TMission_Unload(&node, missionChanged);
		return;
	}
	case TeamMissionType::Load:
	{
		this->_TMission_Load(&node, missionChanged);
		return;
	}
	case TeamMissionType::Deploy:
	{
		this->_TMission_Deploy(&node, missionChanged);
		return;
	}
	case TeamMissionType::Scout:
	{
		this->_TMission_Scout(&node, missionChanged);
		return;
	}
	case TeamMissionType::Move_to_own_building:
	{
		this->_TMission_Move_To_Own_Building(&node, missionChanged);
		return;
	}
	case TeamMissionType::Attack_enemy_building:
	{
		this->_TMission_Attack_Enemy_Building(&node, missionChanged);
		return;
	}
	case TeamMissionType::Chrono_prep_for_abwp:
	{
		this->_TMission_Chrono_prep_for_abwp(&node, missionChanged);
		return;
	}
	case TeamMissionType::Chrono_prep_for_aq:
	{
		this->_TMission_Chrono_prep_for_aq(&node, missionChanged);
		return;
	}
	case TeamMissionType::Play_anim:
	{
		this->_TMission_Play_Animation(&node, missionChanged);
		return;
	}
	case TeamMissionType::Iron_curtain_me:
	{
		this->_TMission_Iron_Curtain_Me(&node, missionChanged);
		return;
	}
	case TeamMissionType::Enter_grinder:
	{
		this->_TMission_Enter_Grinder(&node, missionChanged);
		return;
	}
	case TeamMissionType::Enter_bio_reactor: 
	{
		this->_TMission_Enter_Bio_Reactor(&node, missionChanged);
		return;
	}
	case TeamMissionType::Occupy_battle_bunker:
	{
		this->_TMission_Occupy_Battle_Bunker(&node, missionChanged);
		return;
	}
	case TeamMissionType::Occupy_tank_bunker:
	{
		this->_TMission_Occupy_Tank_Bunker(&node, missionChanged);
		return;
	}
	case TeamMissionType::Attack:
	{
		this->_TMission_Attack(&node, missionChanged);
		return;
	}
	case TeamMissionType::Att_waypt:
	{
		this->_TMission_Attack_Waypoint(&node, missionChanged);
		return;
	}
	case TeamMissionType::Movecell:
	{
		this->_TMission_Move_To_Cell(&node, missionChanged);
		return;
	}
	case TeamMissionType::Loop:
	{
		this->_TMission_Loop(&node, missionChanged);
		return;
	}
	case TeamMissionType::Player_wins:
	{
		this->_TMission_Player_wins(&node, missionChanged);
		return;
	}
	case TeamMissionType::Player_loses:
	{
		this->_TMission_Player_loses(&node, missionChanged);
		return;
	}
	case TeamMissionType::Talk_bubble:
	{
		this->_TMission_Talk_bubble(&node, missionChanged);
		return;
	}
	case TeamMissionType::Success:
	{
		this->AchievedGreatSuccess = true;
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Flash:
	{
		for (FootClass* f = this->FirstUnit; f; f = f->NextTeamMember) {
			if (f->Health > 0 && f->IsAlive && !f->IsCrashing && !f->IsSinking && !f->InLimbo) {
				f->Flashing.DurationRemaining = node.Argument;
			}
		}
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Truck_unload:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = this->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				auto pFirstType = GET_TECHNOTYPE(pFirst);
				if (IS_SAME_STR_(pFirstType->ID, "TRUCKB")) {
					TechnoExt_ExtData::ConvertToType(pFirst, UnitTypeClass::Find("TRUCKA"));
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Truck_load:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = this->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				auto pFirstType = GET_TECHNOTYPE(pFirst);
				if (IS_SAME_STR_(pFirstType->ID, "TRUCKA")) {
					TechnoExt_ExtData::ConvertToType(pFirst, UnitTypeClass::Find("TRUCKB"));
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Wait_till_fully_loaded:
	{
		if(!this->FirstUnit)
			this->StepCompleted = true;

		return;
	}
	case TeamMissionType::Force_facing:
	{
		FootClass* member = this->FirstUnit;

		if (!member) {
			this->StepCompleted = true;
			return;
		}

		// Target direction (0-7 facing) converted to internal facing units
		// Multiply by 8192 (0x2000) to convert 8-direction facing to 16-bit facing (65536 / 8 = 8192)
		DirType targetFacing = (DirType)(node.Argument << 13);

		bool allMembersFacing = true;

		// Process each team member
		while (member)
		{
			// Check if member is currently moving
			if (member->Locomotor->Is_Moving()) {
				// Member is moving, can't complete facing yet
				allMembersFacing = false;
			} else {
				// Get member's current facing
				DirStruct currentDir = member->PrimaryFacing.Current();

				// Check if already facing the correct direction
				if (currentDir.GetDir() != targetFacing) {
					// Not facing correct direction, turn the unit
					member->Locomotor->Do_Turn(DirStruct(targetFacing));
					allMembersFacing = false;
				}
			}

			member = member->NextTeamMember;
		}

		// Mission complete when all members are facing the target direction
		if (allMembersFacing) {
			this->StepCompleted = true;
		}

		return;
	}
	case TeamMissionType::Clear_global:
	{
		ScenarioClass::Instance->GlobalVarChange(node.Argument, false);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Set_global:
	{
		ScenarioClass::Instance->GlobalVarChange(node.Argument, true);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Clear_local:
	{
		ScenarioClass::Instance->LocalVarChange(node.Argument, false);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Set_local:
	{
		ScenarioClass::Instance->LocalVarChange(node.Argument, true);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Delete_team_members:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = this->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{

				if (pFirst->Health > 0
					&& pFirst->IsAlive
					&& !pFirst->IsCrashing
					&& !pFirst->IsSinking
					&& !pFirst->InLimbo) {
					pFirst->Limbo();
					pFirst->UnInit();
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Reveal_map:
	{
		MapClass::Instance->Reveal(nullptr);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Reshroud_map:
	{
		MapClass::Instance->Reshroud(nullptr);
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Ion_storm_start_in:
	{
		if (!LightningStorm::IsActive()) {
			LightningStorm::Start(
				node.Argument,
				RulesClass::Instance->LightningDeferment,
				CellStruct::Empty, nullptr
				);
		}
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Ion_storn_end:
	{
		if (LightningStorm::IsActive()) {
			LightningStorm::RequestStop();
		}
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Center_view_on_team:
	{
		if (this->Zone) {
			CoordStruct place = this->Zone->GetCoords();
			auto pCell = MapClass::Instance->GetCellAt(place);
			place.Z = pCell->ContainsBridgeEx() ? place.Z + CellClass::BridgeHeight : 0;

			TacticalClass::Instance->FocusOn(&place, node.Argument);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Self_destruct:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = this->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				if (pFirst->Health > 0
					&& pFirst->IsAlive
					&& !pFirst->IsCrashing
					&& !pFirst->IsSinking
					&& !pFirst->InLimbo
					)
				{
					int damage = pFirst->Health;
					pFirst->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}

				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Begin_production:
	{
		this->OwnerHouse->Production = true;
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Fire_sale:
	{
		this->OwnerHouse->AIMode = AIMode::SellAll;
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Reduce_tiberium:
	{
		if (this->FirstUnit) {
			this->FirstUnit->GetCell()->ReduceTiberiumWithinCircularArea();
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Goto_nearby_shroud:
	{
		if (missionChanged) {
			AbstractClass* pTarget = this->Zone;
			if (!this->Zone)
				pTarget = this->FirstUnit;

			this->_AssignMissionTarget(MapClass::Instance->MapClass_findnearbyshroud_580BC0(pTarget));
		}
		this->_CoordinateMove();
		return;
	}
	case TeamMissionType::Scatter:
	{
		FootClass* pCur = nullptr;
		if (auto pFirst = this->FirstUnit)
		{
			auto pNext = pFirst->NextTeamMember;
			do
			{
				pFirst->Scatter(CoordStruct::Empty, true, false);
				pCur = pNext;

				if (pNext)
					pNext = pNext->NextTeamMember;

				pFirst = pCur;

			}
			while (pCur);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Panic:
	{
		for (FootClass* f = this->FirstUnit; f; f = f->NextTeamMember) {
			f->Panic();
		}
		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Unpanic:
	{
		for (FootClass* f = this->FirstUnit; f; f = f->NextTeamMember) {
			f->UnPanic();
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Hound_dog:
	{
		this->_Calc_Center(&this->Zone, &this->ClosestMember);
		this->ArchiveTarget = this->Zone;
		this->_CoordinateMove();

		if (auto pTechno = flag_cast_to<TechnoClass*>(this->Zone))
		{
			for (auto i = this->FirstUnit; i; i = i->NextTeamMember)
			{
				if (i->IsAlive
				  && i->Health
				  && (Unsorted::ScenarioInit || !i->InLimbo)
				  && (i->IsTeamLeader || i->WhatAmI() == AircraftClass::AbsID))
				{

					if (i->Target && i->Target != pTechno->Target) {
						i->SetTarget(0);
					}

					if (pTechno->Target)
					{
						if (!i->Locomotor->Is_Moving()
						  && i->IsArmed()
						  && i->GetCurrentMission() == Mission::Guard)
						{
							i->QueueMission(Mission::Attack, 0);
							i->SetTarget(pTechno->Target);
						}
					}
				}
			}
		}
		return;
	}
	case TeamMissionType::Guard:
	{
		this->_TMission_Guard(&node, missionChanged);
		return;
	}
	case TeamMissionType::Idle_anim:
	{
		for (auto f = this->FirstUnit; f; f = f->NextTeamMember) {
			f->PlayIdleAnim(node.Argument);
		}

		this->StepCompleted = true;
		return;
	}
	case TeamMissionType::Change_house:
	{
		this->_TMission_ChangeHouse(&node, missionChanged);
		return;
	}
	case TeamMissionType::Change_script:
	{
		if (node.Argument < 0) {
			this->StepCompleted = true;
			return;
		}

		if (this->CurrentScript) {
			GameDelete<true, false>(this->CurrentScript);
		}

		this->CurrentScript = GameCreate<ScriptClass>(ScriptTypeClass::Array->operator[](node.Argument));
		if (this->CurrentScript)
			this->CurrentScript->ClearMission();

		return;
	}
	case TeamMissionType::Change_team:
	{
		if (node.Argument < 0 || !this->FirstUnit) {
			this->StepCompleted = true;
			return;
		}

		auto pNewTeam = GameCreate<TeamClass>(TeamTypeClass::Array->operator[](node.Argument), this->OwnerHouse, 0);
		for (FootClass* f = this->FirstUnit; f; f = f->NextTeamMember) {
			this->_Remove(f, -1, false);
			pNewTeam->AddMember(f);
		}

		((TeamClass*)this)->_scalar_dtor(1);
		return;
	}
	case TeamMissionType::Gather_at_enemy:
	{
		this->_TMission_GatherAtEnemy(&node, missionChanged);
		return;
	}
	case TeamMissionType::Gather_at_base:
	{
		this->_TMission_GatherAtBase(&node, missionChanged);
		return;
	}
	case TeamMissionType::Play_speech:{
		ScriptExtData::PlaySpeech(this);
		return;
	}
	case TeamMissionType::Play_sound:
	{
		VocClass::PlayGlobal(node.Argument, Panning::Center, 1.0, 0);
		this->StepCompleted = 1;
		return;
	}
	case TeamMissionType::Play_movie:
	{
		Game::PlayMovie(node.Argument, -1,1, 1, 0);
		this->StepCompleted = 1;
		return;
	}
	case TeamMissionType::Play_music:
	{
		ThemeClass::Instance->Queue(node.Argument);
		this->StepCompleted = 1;
		return;
	}
	case TeamMissionType::Do:
	{
		this->_Coordinate_Do(&node,CellStruct::Empty);
		return;
	}
	case TeamMissionType::Patrol:
	{
		// Get patrol waypoint
		const int waypointIndex = node.Argument;
		CellClass* waypointCell = ScenarioClass::Instance->GetWaypointCell(waypointIndex);

		// On first call, set waypoint as initial target
		if (missionChanged) {
			this->_AssignMissionTarget(waypointCell);
		}

		// If no target, check mission data and assign waypoint
		if (!this->ArchiveTarget)
		{
			auto const& [miss, value] = this->CurrentScript->GetCurrentAction();

			// Mission data < 702 is some threshold check (possibly frame-based)
			if (value < 702) {
				this->_AssignMissionTarget(waypointCell);
			}
		}

		// Periodically scan for threats (every PatrolTime minutes)
		int patrolInterval = (int)(RulesClass::Instance->PatrolScan * TICKS_PER_MINUTE);
		if ((Unsorted::CurrentFrame % patrolInterval) == 0)
		{
			// Find team leader
			FootClass* leader = this->_Fetch_A_Leader();

			if (leader)
			{
				// Search for threats near leader
				CoordStruct leaderCoord = leader->Location;

				AbstractClass* threat = leader->GreatestThreat(
					ThreatType::Range,
					&leaderCoord,
					false  // Don't filter by house enemy
				);

				if (threat)
				{
					// Found a threat, attack it
					this->_AssignMissionTarget(threat);

				}
				else if (this->ArchiveTarget != waypointCell)
				{
					// No threat and not at waypoint, clear target to return to patrol
					this->_AssignMissionTarget(nullptr);
				}
			}
		}

		// Coordinate action based on target type
		if (flag_cast_to<TechnoClass*>(this->ArchiveTarget))
		{
			// Target is a techno (unit/building/aircraft), attack it
			this->_Coordinate_Attack();
		}
		else
		{
			// Target is a cell/location, move to it
			this->_CoordinateMove();
		}

		return;
	}
	case TeamMissionType::Move:
	{
		if (missionChanged) {
			if (FootClass* pLeader = this->_Fetch_A_Leader()) {

				CellStruct wp = ScenarioClass::Instance->GetWaypointCoords(node.Argument);
				CellClass* pCell = MapClass::Instance->GetCellAt(wp);
				this->_AssignMissionTarget(pCell);

				//first assigment is failed , lets try to check nearby location
				if (ShouldFindNearbyLocation(this))
				{
					//can the leader move here ?
					if (pLeader->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) > Move::OK)
					{
						TechnoTypeClass* pLeaderType = GET_TECHNOTYPE(pLeader);
						wp = MapClass::Instance->NearByLocation(wp,
							pLeaderType->SpeedType,
							ZoneType::None,
							MovementZone::Normal,
							false,
							1,
							1,
							false,
							false,
							false,
							true,
							CellStruct::Empty,
							false,
							false);
					}
				}

				this->_AssignMissionTarget(wp.IsValid() ? MapClass::Instance->TryGetCellAt(wp) : nullptr);
			}
		}

		this->_CoordinateMove();
		return;
	}
	case TeamMissionType::Moveto_enemy_building:
	{
		if (!this->QueuedFocus)
		{
			FootClass* member = this->FirstUnit;

			if (member)
			{
				// Unpack argument
				int packedArg = node.Argument;
				int buildingTypeIndex = packedArg & 0xFFFF;
				int findMode = packedArg >> 16;

				// Get enemy house
				HouseClass* enemyHouse = nullptr;
				int enemyIndex = member->Owner->EnemyHouseIndex;
				if (enemyIndex != -1) {
					enemyHouse = HouseClass::Array->Items[enemyIndex];
				}

				// Find enemy building
				if (BuildingClass* targetBuilding = Find_Enemy_Building(
					buildingTypeIndex,
					enemyHouse,
					member,
					(BuildingFindType)findMode,
					this->Type->OnlyTargetHouseEnemy
				))
				{
					TechnoTypeClass* memberType = GET_TECHNOTYPE(member);
					// Find nearby accessible location
					CoordStruct buildingCoord = targetBuilding->Location;
					CellStruct buildingCell = CellClass::Coord2Cell(buildingCoord);

					CoordStruct memberCoord = member->GetCoords();
					CellStruct memberCell = CellClass::Coord2Cell(memberCoord);

					ZoneType memberZone = MapClass::Instance->GetMovementZoneType(
						&memberCell,
						memberType->MovementZone,
						member->OnBridge
					);

					const CellStruct nearbyCell = MapClass::Instance->NearByLocation(
						buildingCell,
						memberType->SpeedType,
						memberZone,
						memberType->MovementZone,
						false, 1, 1, 0, 0, 0, 1,
						CellStruct::Empty,
						0, 0
					);

					if (nearbyCell.IsValid()) {
						this->_AssignMissionTarget(MapClass::Instance->GetCellAt(nearbyCell));
					} else {
						this->_AssignMissionTarget(nullptr);
					}
				}
			}
		}

		if (!this->QueuedFocus) {
			this->StepCompleted = true;
			return;
		}

		this->_CoordinateMove();
		return;
	}
	case TeamMissionType::Spy:
	{
		if (missionChanged) {
			CellClass* pWaypCell = ScenarioClass::Instance->GetWaypointCell(node.Argument);
			this->_AssignMissionTarget(pWaypCell);
		}

		AbstractClass* pTarget = nullptr;

		if (this->QueuedFocus && this->QueuedFocus->WhatAmI() == CellClass::AbsID) {
			if (auto pObj = ((CellClass*)this->QueuedFocus)->GetSomeObject(Point2D::Empty, false)) {
				pTarget = pObj;
			}

			this->_AssignMissionTarget(pTarget);
			this->_Coordinate_Attack();
		} else {
			if (this->QueuedFocus) {
				this->_Coordinate_Attack();
			}

			this->_AssignMissionTarget(nullptr);
			this->StepCompleted = true;
		}

		return;
	}
	default:

		if (AresScriptExt::Handle(this, &node, missionChanged) || ScriptExtData::ProcessScriptActions(this, &node, missionChanged))
			return;

		break;
	}

	//this->ExecuteTMission(node.Action, &node, arg4);
}

bool FakeTeamClass::_CoordinateRegroup()
{
	bool allMembersRegrouped = true;
	FootClass* Member = this->FirstUnit;

	// If no members exist, mark as regrouped and return true
	if (!Member) {
		this->NeedsReGrouping = 0;
		return true;
	}

	int stray = this->_Get_Stray();

	// Process each member in the team
	do {
		if (Member->IsAlive) {
			// Check if member is valid and ready for regrouping
			if (Member->Health && (Unsorted::ScenarioInit || !Member->InLimbo) && !Member->IsTeamLeader) {
				// Check if member is close enough to initiate
				if (FakeObjectClass::_GetDistanceOfObj(Member , discard_t() , this->Zone) <= stray) {
					Member->IsTeamLeader = true;
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
		.Process(this->Name)
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

bool TeamExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(TeamExtContainer::ClassName))
	{
		auto& container = root[TeamExtContainer::ClassName];

		for (auto& entry : container[TeamExtData::ClassName])
		{
			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, TeamExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;

}

bool TeamExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[TeamExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : TeamExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer);

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[TeamExtData::ClassName] = std::move(_extRoot);

	return true;
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
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	this->TeamClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4758, FakeTeamClass::_Detach)
