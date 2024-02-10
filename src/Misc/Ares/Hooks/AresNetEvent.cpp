#include "AresNetEvent.h"

#include "Header.h"

#include <Ext/Building/Body.h>

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
	memcpy(&Event.Data.nothing, &Datas, TrenchRedirectClick::size());

	//the data is an array containing 2 stuffs
	EventClass::AddEventWithTimeStamp(&Event);
}

void AresNetEvent::TrenchRedirectClick::Respond(EventClass* Event)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(Event->Data.nothing.Data);
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

	EventClass::AddEvent(&Event);
}

void AresNetEvent::FirewallToggle::Respond(EventClass* Event)
{
	if (HouseClass* pSourceHouse = HouseClass::Array->GetItemOrDefault(Event->HouseIndex))
	{
		AresHouseExt::SetFirestormState(pSourceHouse, !pSourceHouse->FirestormActive);
	}
}