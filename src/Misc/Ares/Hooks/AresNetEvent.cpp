#include "AresNetEvent.h"

#include "Header.h"

#include <Ext/Building/Body.h>

#include <WWKeyboardClass.h>

#include <Misc/Spawner/ProtocolZero.h>
#include <IPXManagerClass.h>

EventExt::TrenchRedirectClick::TrenchRedirectClick(CellStruct* target, BuildingClass* source)
	: TargetCell { target }, Source { source }
{
}

void EventExt::TrenchRedirectClick::Raise(BuildingClass* Source, CellStruct* Target)
{
	EventClass Event {};

	if (Source->Owner->ArrayIndex >= 0)
	{
		Event.Type = EventType(EventExt::Events::TrenchRedirectClick);
		Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	}

	EventExt::AddToEvent<true , TrenchRedirectClick>(Event, Target, Source);
}

void EventExt::TrenchRedirectClick::Respond(EventClass* Event)
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

bool EventExt::ProtocolZero::Enable = false;
int EventExt::ProtocolZero::WorstMaxAhead = 24;
unsigned char EventExt::ProtocolZero::MaxLatencyLevel = 0xff;

EventExt::ProtocolZero::ProtocolZero(char maxahead, uint8_t latencylevel)
	: MaxAhead { maxahead } , LatencyLevel { latencylevel }
{
}

void EventExt::ProtocolZero::Raise()
{
	if (SessionClass::IsSingleplayer())
		return;

	static int NextSendFrame = 6 * SendResponseTimeInterval;
	int currentFrame = Unsorted::CurrentFrame;

	if (NextSendFrame >= currentFrame)
		return;

	const int ipxResponseTime = IPXManagerClass::Instance->ResponseTime();
	if (ipxResponseTime <= -1)
		return;

	EventClass event {};
	event.Type = ProtocolZero::AsEventType();
	event.HouseIndex = (char)HouseClass::CurrentPlayer->ArrayIndex;
	event.Frame = currentFrame + Game::Network::MaxAhead;
	const auto maxAhead = char((int8_t)ipxResponseTime + 1);
	const auto latencyLevel = (uint8_t)LatencyLevel::FromResponseTime((uint8_t)ipxResponseTime);
	ProtocolZero type { maxAhead , latencyLevel };
	memcpy(&event.Data.nothing, &type, ProtocolZero::size());

	if (EventClass::AddEvent(reinterpret_cast<EventClass*>(&event)))
	{
		NextSendFrame = currentFrame + SendResponseTimeInterval;
		Debug::Log("[Spawner] Player %d sending response time of %d, LatencyMode = %d, Frame = %d\n"
			, event.HouseIndex
			, maxAhead
			, latencyLevel
			, currentFrame
		);
	}
	else
	{
		++NextSendFrame;
	}
}

void EventExt::ProtocolZero::Respond(EventClass* Event)
{
	if (ProtocolZero::Enable == false || SessionClass::IsSingleplayer())
		return;

	const ProtocolZero* netData = reinterpret_cast<ProtocolZero*>(Event->Data.nothing.Data);

	if (netData->MaxAhead == 0)
	{
		Debug::Log("[Spawner] Returning because event->MaxAhead == 0\n");
		return;
	}

	static int32_t PlayerMaxAheads[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static uint8_t PlayerLatencyMode[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static int32_t PlayerLastTimingFrame[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	int32_t houseIndex = Event->HouseIndex;
	PlayerMaxAheads[houseIndex] = (int32_t)netData->MaxAhead;
	PlayerLatencyMode[houseIndex] = netData->LatencyLevel;
	PlayerLastTimingFrame[houseIndex] = Event->Frame;

	uint8_t setLatencyMode = 0;
	int maxMaxAheads = 0;

	for (char i = 0; i < (char)std::size(PlayerMaxAheads); ++i)
	{
		if (Unsorted::CurrentFrame >= (PlayerLastTimingFrame[i] + (SendResponseTimeInterval * 4)))
		{
			PlayerMaxAheads[i] = 0;
			PlayerLatencyMode[i] = 0;
		}
		else
		{
			maxMaxAheads = PlayerMaxAheads[i] > maxMaxAheads ? PlayerMaxAheads[i] : maxMaxAheads;
			if (PlayerLatencyMode[i] > setLatencyMode)
				setLatencyMode = PlayerLatencyMode[i];
		}
	}

	ProtocolZero::WorstMaxAhead = maxMaxAheads;
	LatencyLevel::Apply(setLatencyMode);

}

void EventExt::FirewallToggle::Raise(HouseClass* Source)
{
	EventClass Event;

	Event.Type = static_cast<EventType>(EventExt::Events::FirewallToggle);
	Event.HouseIndex = byte(Source->ArrayIndex);

	EventClass::AddEvent(&Event);
}

void EventExt::FirewallToggle::Respond(EventClass* Event)
{
	if (HouseClass* pSourceHouse = HouseClass::Array->GetItemOrDefault(Event->HouseIndex))
	{
		AresHouseExt::SetFirestormState(pSourceHouse, !pSourceHouse->FirestormActive);
	}
}
