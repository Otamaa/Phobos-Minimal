#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>
// Hooks that allow harvesters / weeders to work correctly with MovementZone=Subterannean (sic).

// Author : Otamaa
ASMJIT_PATCH(0x4D423A, FootClass_MissionMove_SubterraneanResourceGatherer, 0x6)
{
	GET(FootClass*, pThis, ESI);

	const auto pType = GET_TECHNOTYPE(pThis);

	if (pThis->WhatAmI() == UnitClass::AbsID && pType->ResourceGatherer) {
		//https://github.com/Phobos-developers/Phobos/issues/326
		if (pType->IsSubterranean || VTable::Get(((UnitClass*)pThis)->Locomotor.GetInterfacePtr()) == HoverLocomotionClass::vtable)
			pThis->QueueMission(Mission::Harvest, false);
	}
	pThis->EnterIdleMode(false, true);
	return 0x4D4248;
}

#include <Utilities/Cast.h>

//==================================================================== Author : Starkku
// Allow scanning for docks in all map zones.
ASMJIT_PATCH(0x4DEFC6, FootClass_FindDock_SubterraneanHarvester, 0x5)
{
	GET(TechnoTypeClass*, pTechnoType, EAX);

	if (auto const pUnitType = type_cast<UnitTypeClass*>(pTechnoType)) {
		if ((pUnitType->Harvester || pUnitType->Weeder) && pUnitType->MovementZone == MovementZone::Subterrannean)
			R->ECX(MovementZone::Fly);
	}

	return 0;
}

// Allow scanning for ore in all map zones.
ASMJIT_PATCH(0x4DCF86, FootClass_FindTiberium_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, ECX);

	if (mZone == MovementZone::Subterrannean)
		R->ECX(MovementZone::Fly);

	return 0;
}

// Allow scanning for weeds in all map zones.
ASMJIT_PATCH(0x4DDB23, FootClass_FindWeeds_SubterraneanHarvester, 0x5)
{
	enum { SkipGameCode = 0x4DCF9B };

	GET(MovementZone, mZone, EAX);

	if (mZone == MovementZone::Subterrannean)
		R->EAX(MovementZone::Fly);

	return 0;
}

// Force harvest mission and clear all destination info etc. upon reaching the rally point.
ASMJIT_PATCH(0x738A3E, UnitClass_EnterIdleMode_SubterraneanHarvester, 0x5)
{
	enum { ReturnFromFunction = 0x738D21 };

	GET(UnitClass*, pThis, ESI);

	auto const pType = pThis->Type;

	if ((pType->Harvester || pType->Weeder) && pType->MovementZone == MovementZone::Subterrannean)
	{
		auto const mission = pThis->CurrentMission;

		if (mission == Mission::Unload || mission == Mission::Harvest)
			return ReturnFromFunction;
	}

	return 0;
}

// Reset the rally point when it is not needed anymore.
ASMJIT_PATCH(0x73EDA1, UnitClass_Mission_Harvest_SubterraneanHarvester, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	if (pThis->Type->MovementZone == MovementZone::Subterrannean)
		TechnoExtContainer::Instance.Find(pThis)->SubterraneanHarvRallyPoint = nullptr;

	return 0;
}

// Apply same special rules on idle to player-owned subterranean harvesters as to Teleporter=yes ones.
DEFINE_HOOK(0x740949, UnitClass_Mission_Guard_SubterraneanHarvester, 0x6)
{
	enum { Continue = 0x740957 };

	GET(UnitTypeClass*, pType, ECX);

	if (pType->MovementZone == MovementZone::Subterrannean)
		return Continue;

	return 0;
}
