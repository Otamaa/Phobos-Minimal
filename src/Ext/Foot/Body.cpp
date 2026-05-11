#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <Ext/Building/Body.h>

FootExtContainer FootExtContainer::Instance;

bool __fastcall FakeFootClass::__Try_Grinding(FootClass* pFoot)
{
	BuildingClass* bestGrinder = nullptr;
	int shortestDistance = MAX_VAL(int);

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