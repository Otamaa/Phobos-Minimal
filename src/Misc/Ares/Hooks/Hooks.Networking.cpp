#include <Ext/Building/Body.h>

#include <HouseClass.h>

#include <Misc/AresData.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include "Header.h"
#include "AresNetEvent.h"

#include <Ares_TechnoExt.h>

#include <EventClass.h>

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

void AresNetEvent::TrenchRedirectClick::Raise(BuildingClass *Source, CellStruct *Target) {
	EventClass Event {};

	if(Source->Owner->ArrayIndex >= 0){
		Event.Type = EventType(AresNetEvent::Events::TrenchRedirectClick);
		Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	}

	TrenchRedirectClick Datas { Target , Source };
	memcpy(&Event.Data.SpaceGap, &Datas, TrenchRedirectClick::size());

	//the data is an array containing 2 stuffs
	EventClass::AddEvent(Event);
}

bool NOINLINE IsSameTrech(BuildingClass* currentBuilding , BuildingClass* targetBuilding)
{
	auto pThisTypeExt = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	if(pThisTypeExt->IsTrench <= 0) {
		return false;
	}

	return pThisTypeExt->IsTrench == BuildingTypeExt::ExtMap.Find(targetBuilding->Type)->IsTrench;
}

bool NOINLINE canTraverseTo(BuildingClass* currentBuilding , BuildingClass* targetBuilding) {
	if(targetBuilding != currentBuilding) {
		BuildingTypeClass* pTargetType = targetBuilding->Type;
		if (pTargetType->CanBeOccupied && targetBuilding->Occupants.Count < pTargetType->MaxNumberOccupants) {
			if(currentBuilding->Occupants.Count && IsSameTrech(currentBuilding , targetBuilding)) {
				if(targetBuilding->Location.DistanceFrom(currentBuilding->Location) <= 256.0)
					return true;
			}
		}
	}

	return false;
}

void NOINLINE doTraverseTo(BuildingClass* currentBuilding ,BuildingClass* targetBuilding) {
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	// depending on Westwood's handling, this could explode when Size > 1 units are involved...but don't tell the users that
	while(currentBuilding->Occupants.Count && (targetBuilding->Occupants.Count < targetBuildingType->MaxNumberOccupants)) {
		targetBuilding->Occupants.AddItem(currentBuilding->Occupants.GetItem(0));
		currentBuilding->Occupants.RemoveAt(0); // maybe switch Add/Remove if the game gets pissy about multiple of them walking around
	}

	// fix up firing index, as decrementing the source occupants can invalidate it
	if(currentBuilding->FiringOccupantIndex >= currentBuilding->GetOccupantCount()) {
		currentBuilding->FiringOccupantIndex = 0;
	}

	TechnoExt_ExtData::EvalRaidStatus(currentBuilding); // if the traversal emptied the current building, it'll have to be returned to its owner
}

void AresNetEvent::TrenchRedirectClick::Respond(EventClass *Event) {
	TargetClass *ID = reinterpret_cast<TargetClass*>(Event->Data.SpaceGap.Data);
	if(CellClass * pTargetCell = ID->As_Cell()) {
		++ID;
		if(BuildingClass * pSourceBuilding = ID->As_Building()) {
			/*
				pSourceBuilding == selected building the soldiers are in
				pTargetCell == cell the user clicked on; event fires only on buildings which showed the enter cursor
			*/
			BuildingClass* targetBuilding = pTargetCell->GetBuilding();
			if(canTraverseTo(pSourceBuilding, targetBuilding)) // check has happened before the enter cursor appeared
				doTraverseTo(pSourceBuilding , targetBuilding);
		}
	}
}

void AresNetEvent::FirewallToggle::Raise(HouseClass *Source) {
	EventClass Event;

	Event.Type = static_cast<EventType>(AresNetEvent::Events::FirewallToggle);
	Event.HouseIndex = byte(Source->ArrayIndex);

	EventClass::AddEvent(Event);
}

void AresNetEvent::FirewallToggle::Respond(EventClass*Event) {
	if(HouseClass * pSourceHouse = HouseClass::Array->GetItem(Event->HouseIndex)) {
		AresData::AresNetEvent_Handlers_RespondToFirewallToggle(pSourceHouse , !pSourceHouse->FirestormActive);
	}
}

/* just for compile test */
void AresNetEvent::ResponseTime2::Raise()
{
	if (SessionClass::IsSingleplayer())
		return;

	int currentFrame = Unsorted::CurrentFrame;

	EventClass Event {};
	Event.Type = AsEventType();
	Event.HouseIndex = (char)HouseClass::CurrentPlayer->ArrayIndex;
	Event.Frame = currentFrame + Game::Network::MaxAhead;
	ResponseTime2 dataToSend { 10 ,20 };
	memcpy(&Event.Data.SpaceGap, &dataToSend, size());

	if (EventClass::AddEvent(Event))
		Debug::Log("test");
}

void AresNetEvent::ResponseTime2::Respond(EventClass* Event)
{
	if (SessionClass::IsSingleplayer())
		return;

	char MaxAhead = Event->Data.SpaceGap.Data[0];
	uint8_t LatencyLevel = Event->Data.SpaceGap.Data[1];

	if (MaxAhead == 0)
	{
		Debug::Log("[Spawner] Returning because event->MaxAhead == 0\n");
		return;
	}

	static int32_t PlayerMaxAheads[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static uint8_t PlayerLatencyMode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static int32_t PlayerLastTimingFrame[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	int32_t houseIndex = Event->HouseIndex;
	PlayerMaxAheads[houseIndex] = (int32_t)MaxAhead;
	PlayerLatencyMode[houseIndex] = LatencyLevel;
	PlayerLastTimingFrame[houseIndex] = Event->Frame;

}


// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_OVERRIDE_HOOK(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = specific_cast<BuildingClass*>(T)) {
		if(canTraverseTo(pThis ,targetBuilding)) {
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
