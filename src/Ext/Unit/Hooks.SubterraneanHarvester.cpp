#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>
// Hooks that allow harvesters / weeders to work correctly with MovementZone=Subterannean (sic).

// Author : Otamaa
DEFINE_HOOK(0x4D423A, FootClass_MissionMove_SubterraneanResourceGatherer, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pType = pThis->GetTechnoType();
	if (pThis->WhatAmI() == UnitClass::AbsID && pType->ResourceGatherer) {
		//https://github.com/Phobos-developers/Phobos/issues/326
		if (pType->IsSubterranean || VTable::Get(((UnitClass*)pThis)->Locomotor.GetInterfacePtr()) == HoverLocomotionClass::vtable)
			pThis->QueueMission(Mission::Harvest, false);
	}
	pThis->EnterIdleMode(false, true);
	return 0x4D4248;
}

/*==================================================================== Author : Starkku
// Allow scanning for docks in all map zones.
DEFINE_HOOK(0x4DEFC6, FootClass_FindDock_SubterraneanHarvester, 0x5)
{
	GET(TechnoTypeClass*, pTechnoType, EAX);

	if (auto const pUnitType = specific_cast<UnitTypeClass*>(pTechnoType)) {
		if ((pUnitType->Harvester || pUnitType->Weeder) && pUnitType->MovementZone == MovementZone::Subterrannean)
			R->ECX(MovementZone::Fly);
	}

	return 0;
}

// Allow scanning for ore in all map zones.
DEFINE_HOOK(0x4DCF86, FootClass_FindTiberium_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, ECX);

	if (mZone == MovementZone::Subterrannean)
		R->ECX(MovementZone::Fly);

	return 0;
}

// Allow scanning for weeds in all map zones.
DEFINE_HOOK(0x4DDB23, FootClass_FindWeeds_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, EAX);

	if (mZone == MovementZone::Subterrannean)
		R->EAX(MovementZone::Fly);

	return 0;
}

// Force harvest mission and clear all destination info etc. upon reaching the rally point.
DEFINE_HOOK(0x738A3E, UnitClass_EnterIdleMode_SubterraneanHarvester, 0x5)
{
	enum { ReturnFromFunction = 0x738D21 };

	GET(UnitClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if ((pType->Harvester || pType->Weeder) && pType->MovementZone == MovementZone::Subterrannean)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (pExt->SubterraneanHarvRallyPoint && pThis->GetMapCoords() == pExt->SubterraneanHarvRallyPoint->MapCoords)
		{
			pExt->SubterraneanHarvRallyPoint = nullptr;
			pThis->SetDestination(nullptr, false);
			pThis->ClearNavQueue();
			pThis->SetArchiveTarget(nullptr);
			pThis->ForceMission(Mission::Harvest);
			return ReturnFromFunction;
		}
	}

	return 0;
}

// Set rally point.
DEFINE_HOOK(0x44459A, BuildingClass_ExitObject_SubterraneanHarvester, 0x5)
{
	GET(TechnoClass*, pThis, EDI);

	if (auto const pUnit = specific_cast<UnitClass*>(pThis))
	{
		auto const pType = pUnit->Type;

		if ((pType->Harvester || pType->Weeder) && pType->MovementZone == MovementZone::Subterrannean) {
			TechnoExtContainer::Instance.Find(pThis)->SubterraneanHarvRallyPoint = abstract_cast<CellClass*>(pThis->ArchiveTarget);
		}
	}

	return 0;
}

// Reset the rally point when it is not needed anymore.
DEFINE_HOOK(0x73EDA1, UnitClass_Mission_Harvest_SubterraneanHarvester, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	if (pThis->Type->MovementZone == MovementZone::Subterrannean)
		TechnoExtContainer::Instance.Find(pThis)->SubterraneanHarvRallyPoint = nullptr;

	return 0;
}*/