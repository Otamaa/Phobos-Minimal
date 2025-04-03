#include <Ext/Building/Body.h>

#include <HouseClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include "Header.h"
#include <Ext/Event/Body.h>

#include <EventClass.h>

ASMJIT_PATCH(0x6ab773, SelectClass_ProcessInput_ProduceUnsuspended, 0xA)
{
	GET(EventClass*, pEvent, EAX);
	GET_STACK(DWORD, flag, 0xB8);

	for (int i = ((4 * (flag & 1)) | 1); i > 0; --i) {
		EventClass::AddEvent(pEvent);
	}

	return 0x6AB7CC;
}

ASMJIT_PATCH(0x64C314, Breakup_Receive_Packet_PayloadSize2, 0x8)
{
	GET(EventType, eventType, ESI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize + (EventType::MEGAMISSION == eventType));

	return 0x64C321;
}

ASMJIT_PATCH(0x64BE83, Breakup_Receive_Packet_PayloadSize1, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize);
	R->Stack(0x20, eventDataSize);

	return (EventType::MEGAMISSION == eventType) ? 0x64BF1A : 0x64BE97;
}

ASMJIT_PATCH(0x64B704, Add_Compressed_Events_PayloadSize, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->EDX(eventDataSize);
	R->EBP(eventDataSize);

	return (EventType::ADDPLAYER == eventType) ? 0x64B710 : 0x64B71D;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
ASMJIT_PATCH(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = cast_to<BuildingClass*>(T)) {
		if(TechnoExt_ExtData::canTraverseTo(pThis ,targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> EventExt::Handlers::RespondToTrenchRedirectClick
			R->EAX(Action::Enter);
			return 0x447273;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x443414, BuildingClass_ActionOnObject, 6)
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
		EventExt::TrenchRedirectClick::Raise(pThis, &tgt);
		R->EAX(1);
		return 0x44344D;
	}

	return 0;
}

ASMJIT_PATCH(0x4C6CCD, EventClass_Execute, 0xA)
{
	GET(int, EventKind, EAX);
	GET(EventClass *, Event, ESI);

	const auto kind = static_cast<EventExt::Events>(EventKind);
	if(EventExt::IsValidType(kind)) {
		// Received Ares event, do something about it
		EventExt::RespondEvent(Event, kind);
	}

	--(EventKind);
	R->EAX(EventKind);
	return (EventKind > (int)EventType::ABANDON_ALL)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}

ASMJIT_PATCH(0x4C65EF, EventClass_CTOR_Log, 0x7)
{
	GET(int, events, EAX);

	const auto eventType = static_cast<EventExt::Events>(events);
	if (EventExt::IsValidType(eventType)) {
		// Received Ares event, send the names
		R->ECX(EventExt::GetEventNames(eventType));
		return 0x4C65F6;
	}

	return 0;
}

ASMJIT_PATCH(0x64C5C7, Execute_DoList_Log, 0x7)
{
	const auto eventType = static_cast<EventExt::Events>(R->AL());
	if (EventExt::IsValidType(eventType)) {
		// Received Ares event, send the names
		R->ECX(EventExt::GetEventNames(eventType));
		return 0x64C5CE;
	}

	return 0;
}
