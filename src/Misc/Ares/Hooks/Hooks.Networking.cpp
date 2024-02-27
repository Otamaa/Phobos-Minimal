#include <Ext/Building/Body.h>

#include <HouseClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include "Header.h"
#include "AresNetEvent.h"

#include "Header.h"

#include <EventClass.h>

DEFINE_HOOK(0x6ab773, SelectClass_ProcessInput_ProduceUnsuspended, 0xA)
{
	GET(EventClass*, pEvent, EAX);
	GET_STACK(DWORD, flag, 0xB8);

	for (int i = ((4 * (flag & 1)) | 1); i > 0; --i) {
		EventClass::AddEvent(pEvent);
	}

	return 0x6AB7CC;
}

DEFINE_HOOK(0x64C314, sub_64BDD0_PayloadSize2, 0x8)
{
	GET(EventType, eventType, ESI);

	const auto eventDataSize = AresNetEvent::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize + (EventType::MEGAMISSION == eventType));

	return 0x64C321;
}

DEFINE_HOOK(0x64BE83, sub_64BDD0_PayloadSize1, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = AresNetEvent::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize);
	R->Stack(0x20, eventDataSize);

	return (EventType::MEGAMISSION == eventType) ? 0x64BF1A : 0x64BE97;
}

DEFINE_HOOK(0x64B704, sub_64B660_PayloadSize, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = AresNetEvent::GetDataSize(eventType);

	R->EDX(eventDataSize);
	R->EBP(eventDataSize);

	return (EventType::ADDPLAYER == eventType) ? 0x64B710 : 0x64B71D;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_HOOK(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = specific_cast<BuildingClass*>(T)) {
		if(TechnoExt_ExtData::canTraverseTo(pThis ,targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
			R->EAX(Action::Enter);
			return 0x447273;
		}
	}

	return 0;
}

DEFINE_HOOK(0x443414, BuildingClass_ActionOnObject, 6)
{
	GET(Action, action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	if(action == Action::Detonate)
		return 0;

	// part of deactivation logic
	if(pThis->Deactivated) {
		R->EAX(1);
		return 0x44344D;
	}

	// trenches
	if(action == Action::Enter && pTarget->WhatAmI() == BuildingClass::AbsID) {
		CoordStruct XYZ = pTarget->GetCoords();
		CellStruct tgt = CellClass::Coord2Cell(XYZ);
		AresNetEvent::TrenchRedirectClick::Raise(pThis, &tgt);
		R->EAX(1);
		return 0x44344D;
	}

	return 0;
}

DEFINE_HOOK(0x4C6CCD, Networking_RespondToEvent, 0xA)
{
	GET(int, EventKind, EAX);
	GET(EventClass *, Event, ESI);

	auto kind = static_cast<AresNetEvent::Events>(EventKind);
	if(kind >= AresNetEvent::Events::First) {
		// Received Ares event, do something about it
		AresNetEvent::RespondEvent(Event, kind);
	}

	--(EventKind);
	R->EAX(EventKind);
	return (EventKind > (int)EventType::ABANDON_ALL)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}
