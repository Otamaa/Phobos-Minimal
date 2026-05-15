#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <Ext/Building/Body.h>

#include <InfantryClass.h>
#include <SlaveManagerClass.h>

FootExtContainer FootExtContainer::Instance;

// FootClass::Mission_Hunt — backported from game (0x4D5350–0x4D55B6)
// Ported from gamemd.exe, address range 0x4D5350 to 0x4D55B6
int __fastcall FakeFootClass::_Mission_Hunt(FootClass* pThis)
{
	CoordStruct loc = pThis->GetCoords();
	const bool hasTarget = !pThis->GetTechnoType()->StupidHunt
		&& FakeTechnoClass::__TargetSomethingNearby(pThis, discard_t(), &loc, ThreatType::None);

	if (hasTarget)
	{
		if (auto pInfantry = cast_to<InfantryClass*,false>(pThis))
		{
			InfantryTypeClass* pType = pInfantry->Type;

			if (pType->Engineer && !pType->C4 && !pThis->HasAbility(AbilityType::C4))
			{
				// Engineer: capture the target
				pThis->SetDestination(pThis->Target, true);
				pThis->QueueMission(Mission::Capture, false);
				if (pThis->ReadyToNextMission())
					pThis->NextMission();
			}
			else if ((pType->C4 || pThis->HasAbility(AbilityType::C4))
			 && pThis->Target != nullptr
			 && pThis->Target->WhatAmI() == AbstractType::Building)
			{
				// C4 / bomber infantry vs a building: sabotage
				pThis->SetDestination(pThis->Target, true);
				pThis->QueueMission(Mission::Sabotage, false);
				if (pThis->ReadyToNextMission())
					pThis->NextMission();
			}
			else if (pType->VehicleThief)
			{
				// Vehicle thief: capture the target
				pThis->SetDestination(pThis->Target, true);
				pThis->QueueMission(Mission::Capture, false);
				if (pThis->ReadyToNextMission())
					pThis->NextMission();
			}
			else
			{
				pThis->SetDestination(nullptr, true);
				pThis->ApproachTarget(false);
			}
		}
		else
		{
			pThis->ApproachTarget(false);
		}
	}
	else
	{
		if (pThis->Owner->IsControlledByHuman() || SessionClass::Instance->GameMode != GameMode::Campaign)
		{
			pThis->UpdateIdleAction();
		}
		else
		{
			// AI in campaign: move toward the player's base center
			const CellStruct& baseCenter = HouseClass::CurrentPlayer->GetBaseCenter();
			if (baseCenter.IsValid())
			{
				if (pThis->SlaveManager)
					pThis->SlaveManager->Guard();

				CellClass* pCell = MapClass::Instance->GetCellAt(baseCenter);
				pThis->SetDestination(pCell, true);
				pThis->QueueMission(Mission::Move, true);
			}
		}
	}

	auto* pControl = pThis->GetCurrentMissionControl();
	return pControl->NormalDelay() + ScenarioClass::Instance->Random.RandomRanged(0, 2);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4D5350, FakeFootClass::_Mission_Hunt)
DEFINE_FUNCTION_JUMP(CALL, 0x51F60C, FakeFootClass::_Mission_Hunt)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8EBC, FakeFootClass::_Mission_Hunt)

bool __fastcall FakeFootClass::__Try_Grinding(FootClass* pFoot)
{
	BuildingClass* bestGrinder = nullptr;
	int shortestDistance = MIN_VAL(int);

	int grinderCount = pFoot->Owner->Grinders.Count;

	if (grinderCount <= 0) {
		return false;
	}

	// Iterate through all grinders to find the closest one
	for (int i = grinderCount - 1; i >= 0; --i) {
		auto currentGrinder = pFoot->Owner->Grinders[i];

		if (!BuildingExtData::CanGrindTechno(currentGrinder, pFoot)) {
			continue;
		}

		auto grinderCoords = currentGrinder->GetCoords();
		auto footCoords = pFoot->GetCoords();

		int currentDistance = static_cast<int>(footCoords.DistanceFrom(grinderCoords));

		if (currentDistance < shortestDistance) {
			shortestDistance = currentDistance;
			bestGrinder = currentGrinder;
		}
	}

	if (!bestGrinder) {
		return false;
	}

	// Command the unit to move to the grinder and trigger the 'Eaten' mission
	pFoot->SetDestination(bestGrinder, true);
	pFoot->QueueMission(Mission::Eaten, false);

	return true;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E25E0, FakeFootClass::__Try_Grinding)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E8FD0, FakeFootClass::__Try_Grinding)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB394, FakeFootClass::__Try_Grinding)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5FAC, FakeFootClass::__Try_Grinding)

bool __fastcall FakeFootClass::_IsRecruitable(FootClass* pThis, discard_t, HouseClass* pHouse)
{
	if (!pThis || !pHouse || !pThis->IsAlive) return false;

	const bool inTeam = pThis->Team != nullptr;
	const bool available = pThis->Health > 0 && !pThis->InLimbo;
	const bool wrongOwner = pThis->Owner != pHouse;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (inTeam || !available || wrongOwner || pExt->Is_DriverKilled)
		return false;

	if (auto pUnit = cast_to<UnitClass*, false>(pThis)) {
		if (pUnit->DeathFrameCounter >= 0)
			return false;
	}

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	const bool canRecruit = pThis->RecruitableA && pThis->RecruitableB;
	if (!canRecruit)
		return false;

	const Mission mission = pThis->GetCurrentMission();

	if (!MissionClass::IsRecruitableMission(mission))
		return false;

	const bool validState =
		!(pThis->ShouldEnterAbsorber || pThis->ShouldEnterOccupiable || pThis->ShouldGarrisonStructure) &&
		pThis->DrainTarget == nullptr &&
		!pThis->BunkerLinkedItem &&
		pThis->LocomotorSource == nullptr;

	return validState;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4DA230 , FakeFootClass::_IsRecruitable)