#include "AresNetEvent.h"

#include "Header.h"

#include <Ext/Building/Body.h>

#include <Misc/AresData.h>

#include <WWKeyboardClass.h>

AresNetEvent::TrenchRedirectClick::TrenchRedirectClick(CellStruct* target, BuildingClass* source)
	: TargetCell { target }, Source { source }
{
}

void AresNetEvent::TrenchRedirectClick::Raise(BuildingClass* Source, CellStruct* Target)
{
	EventClass Event {};

	if (Source->Owner->ArrayIndex >= 0)
	{
		Event.Type = EventType(AresNetEvent::Events::TrenchRedirectClick);
		Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	}

	TrenchRedirectClick Datas { Target , Source };
	memcpy(&Event.Data.SpaceGap, &Datas, TrenchRedirectClick::size());

	//the data is an array containing 2 stuffs
	EventClass::AddEvent(Event);
}

void AresNetEvent::TrenchRedirectClick::Respond(EventClass* Event)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(Event->Data.SpaceGap.Data);
	if (CellClass* pTargetCell = ID->As_Cell())
	{
		++ID;
		if (BuildingClass* pSourceBuilding = ID->As_Building())
		{
			/*
				pSourceBuilding == selected building the soldiers are in
				pTargetCell == cell the user clicked on; event fires only on buildings which showed the enter cursor
			*/
			BuildingClass* targetBuilding = pTargetCell->GetBuilding();
			if (TechnoExt_ExtData::canTraverseTo(pSourceBuilding, targetBuilding)) // check has happened before the enter cursor appeared
				TechnoExt_ExtData::doTraverseTo(pSourceBuilding, targetBuilding);
		}
	}
}

void AresNetEvent::FirewallToggle::Raise(HouseClass* Source)
{
	EventClass Event;

	Event.Type = static_cast<EventType>(AresNetEvent::Events::FirewallToggle);
	Event.HouseIndex = byte(Source->ArrayIndex);

	EventClass::AddEvent(Event);
}

void AresNetEvent::FirewallToggle::Respond(EventClass* Event)
{
	if (HouseClass* pSourceHouse = HouseClass::Array->GetItem(Event->HouseIndex))
	{
		AresHouseExt::SetFirestormState(pSourceHouse, !pSourceHouse->FirestormActive);
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
