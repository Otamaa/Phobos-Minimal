#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>


FootExtContainer FootExtContainer::Instance;

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