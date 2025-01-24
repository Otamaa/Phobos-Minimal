#include "Body.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <WWKeyboardClass.h>

#include <Misc/Spawner/ProtocolZero.h>
#include <IPXManagerClass.h>

EventExt::ManualReload::ManualReload(TechnoClass* pTechno) : Who { pTechno }
{ }

void EventExt::ManualReload::Raise(TechnoClass* pTechno)
{
	EventClass Event {};

	if (pTechno->Owner->ArrayIndex >= 0)
	{
		Event.Type = AsEventType();
		Event.HouseIndex = byte(pTechno->Owner->ArrayIndex);
	}

	EventExt::AddToEvent<true, ManualReload>(Event, pTechno);
	Debug::Log("Adding event MANUAL_RELOAD\n");
}

void EventExt::ManualReload::Respond(EventClass* Event)
{
	ManualReload* ID = Event->Data.nothing.As<ManualReload>();

	if (const auto pTechno = ID->Who.As_Techno())
	{
		if (pTechno->Ammo > 0 && pTechno->IsAlive && !pTechno->Berzerk)
		{
			const auto pType = pTechno->GetTechnoType();
			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

			if (pTechno->Ammo != pType->Ammo && pTypeExt->CanManualReload)
			{
				if (pTypeExt->CanManualReload_DetonateWarhead && pTypeExt->CanManualReload_DetonateConsume <= pTechno->Ammo)
					WarheadTypeExtData::DetonateAt(pTypeExt->CanManualReload_DetonateWarhead, pTechno->Target, pTechno->GetCoords(), pTechno, 1, pTechno->Owner);

				if (pTypeExt->CanManualReload_ResetROF)
					pTechno->DiskLaserTimer.Stop();

				pTechno->Ammo = 0;

				if (pTechno->WhatAmI() != AbstractType::Aircraft)
					pTechno->StartReloading();
			}
		}
	}
}

EventExt::TrenchRedirectClick::TrenchRedirectClick(CellStruct* target, BuildingClass* source)
	: TargetCell { target }, Source { source }
{ }

void EventExt::TrenchRedirectClick::Raise(BuildingClass* Source, CellStruct* Target)
{
	EventClass Event {};

	if (Source->Owner->ArrayIndex >= 0)
	{
		Event.Type = AsEventType();
		Event.HouseIndex = byte(Source->Owner->ArrayIndex);
	}

	EventExt::AddToEvent<true, TrenchRedirectClick>(Event, Target, Source);
}

void EventExt::TrenchRedirectClick::Respond(EventClass* Event)
{
	TrenchRedirectClick* ID = Event->Data.nothing.As<TrenchRedirectClick>();
	if (CellClass* pTargetCell = ID->TargetCell.As_Cell())
	{
		if (BuildingClass* pSourceBuilding = ID->Source.As_Building())
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
	: MaxAhead { maxahead }, LatencyLevel { latencylevel }
{ }

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
	event.Data.nothing.Set<ProtocolZero>(&type);

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

	const ProtocolZero* netData = Event->Data.nothing.As<ProtocolZero>();

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
	EventClass Event {};

	Event.Type = AsEventType();
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
