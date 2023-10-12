#include <Ext/Building/Body.h>

#include <HouseClass.h>

#include <Misc/AresData.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include "Header.h"
#include "AresNetEvent.h"

#include "Header.h"

#include <EventClass.h>

DEFINE_OVERRIDE_HOOK(0x6ab773, SelectClass_ProcessInput_ProduceUnsuspended, 10)
{
	GET(EventClass*, pEvent, EAX);
	GET_STACK(DWORD, flag, 0xB8);

	for (int i = (4 * (flag & 1)) | 1); i > 0; --i)
	{
		if (EventClass::OutList->Count < 128)
		{
			BYTE v8[sizeof(EventClass)];
			std::memcpy(v8, pEvent, sizeof(EventClass));//make copy of currentEvent
			//put the event back onto list ?
			auto list = &EventClass::OutList->List[EventClass::OutList->Tail];
			std::memcpy(list, v8, 0x6Cu);
			//modify some data on it ?
			*reinterpret_cast<BYTE*>(list + 0x6Cu) = v8[94];
			*reinterpret_cast<BYTE*>(list + 0x6Cu + 2u) = v8[96];

			EventClass::OutList->Timings[EventClass::OutList->Tail] = static_cast<int>(Imports::TimeGetTime.get()());

			++EventClass::OutList->Count;
			EventClass::OutList->Tail = (EventClass::OutList->Tail + 1) & 127;
		}
	}

	return 0x6AB7CC;
}

DEFINE_OVERRIDE_HOOK(0x64C314, sub_64BDD0_PayloadSize2, 0x8)
{
	GET(uint8_t, nSize, ESI);

	const auto nFix = AresNetEvent::GetDataSize(nSize);

	R->ECX(nFix);
	R->EBP(nFix + (nSize == 4u));

	return 0x64C321;
}

DEFINE_OVERRIDE_HOOK(0x64BE83, sub_64BDD0_PayloadSize1, 0x8)
{
	GET(uint8_t, nSize, EDI);

	const auto nFix = AresNetEvent::GetDataSize(nSize);

	R->ECX(nFix);
	R->EBP(nFix);
	R->Stack(0x20, nFix);

	return nSize == 4 ? 0x64BF1A : 0x64BE97;
}

DEFINE_OVERRIDE_HOOK(0x64B704, sub_64B660_PayloadSize, 0x8)
{
	GET(uint8_t, nSize, EDI);

	const auto nFix = AresNetEvent::GetDataSize(nSize);

	R->EDX(nFix);
	R->EBP(nFix);

	return (nSize == 0x1Fu) ? 0x64B710 : 0x64B71D;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_OVERRIDE_HOOK(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
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

DEFINE_OVERRIDE_HOOK(0x443414, BuildingClass_ActionOnObject, 6)
{
	GET(Action, action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	// part of deactivation logic
	if(pThis->Deactivated) {
		R->EAX(1);
		return 0x44344D;
	}

	// trenches
	if(action == Action::Enter) {
		if(BuildingClass *pTargetBuilding = specific_cast<BuildingClass *>(pTarget)) {
			CoordStruct XYZ = pTargetBuilding->GetCoords();
			CellStruct tgt = CellClass::Coord2Cell(XYZ);
			AresNetEvent::TrenchRedirectClick::Raise(pThis, &tgt);
			R->EAX(1);
			return 0x44344D;
		}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4C6CCD, Networking_RespondToEvent, 0xA)
{
	GET(DWORD, EventKind, EAX);
	GET(EventClass *, Event, ESI);

	auto kind = static_cast<AresNetEvent::Events>(EventKind);
	if(kind >= AresNetEvent::Events::First) {
		// Received Ares event, do something about it
		AresNetEvent::RespondEvent(Event, kind);
	}

	--EventKind;
	R->EAX(EventKind);
	return (EventKind > 0x2D)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}
