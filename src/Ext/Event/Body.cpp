#include "Body.h"

#include <Misc/Ares/Hooks/Header.h>

#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <WWKeyboardClass.h>

#include <Misc/Spawner/ProtocolZero.h>
#include <IPXManagerClass.h>

EventExt::TogglePassiveAcquireMode::TogglePassiveAcquireMode(TechnoClass* pTechno, PassiveAcquireMode mode) : Who { pTechno } , Mode { mode }
{ }

void EventExt::TogglePassiveAcquireMode::Raise(TechnoClass* pTechno, PassiveAcquireMode mode)
{
	EventClass eventExt {};
	eventExt.Type = AsEventType();
	eventExt.HouseIndex = byte(pTechno->Owner->ArrayIndex);
	EventExt::AddToEvent<true, true, TogglePassiveAcquireMode>(eventExt ,pTechno, mode);
}

void EventExt::TogglePassiveAcquireMode::Respond(EventClass* Event)
{
	TogglePassiveAcquireMode* ID = Event->Data.nothing.As<TogglePassiveAcquireMode>();

	if (const auto pTechno = ID->Who.As_Techno()) {
		if (pTechno->IsAlive && !pTechno->Berzerk) {
			const auto pTechnoExt = TechnoExtContainer::Instance.Find(pTechno);

			if (pTechnoExt->CanTogglePassiveAcquireMode())
				pTechnoExt->TogglePassiveAcquireMode(ID->Mode);
		}
	}
}

EventExt::ManualReload::ManualReload(TechnoClass* pTechno) : Who { pTechno }
{ }

void EventExt::ManualReload::Raise(TechnoClass* pTechno)
{
	EventClass Event {};

	if (pTechno->Owner->ArrayIndex >= 0) {
		Event.Type = AsEventType();
		Event.HouseIndex = byte(pTechno->Owner->ArrayIndex);
	}

	EventExt::AddToEvent<true, true, ManualReload>(Event, pTechno);
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
					pTechno->RearmTimer.Stop();

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

	EventExt::AddToEvent<true, true, TrenchRedirectClick>(Event, Target, Source);
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
int EventExt::ProtocolZero::NextSendFrame = -1;
int EventExt::ProtocolZero::WorstMaxAhead = LatencyLevel::GetMaxAhead(LatencyLevelEnum::LATENCY_LEVEL_6);
unsigned char EventExt::ProtocolZero::MaxLatencyLevel = std::numeric_limits<unsigned char>::max();

EventExt::ProtocolZero::ProtocolZero(char maxahead, uint8_t latencylevel)
	: MaxAhead { maxahead }, LatencyLevel { latencylevel }
{ }

void EventExt::ProtocolZero::Raise()
{
	if (SessionClass::IsSingleplayer())
		return;

	int currentFrame = Unsorted::CurrentFrame;

	if (ProtocolZero::NextSendFrame < 0) {
		ProtocolZero::NextSendFrame = currentFrame + Game::Network::FrameSendRate + ProtocolZero::SendResponseTimeFrame;
		return;
	}

	if (ProtocolZero::NextSendFrame >= currentFrame)
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

	if (EventExt::AddToEvent<false , true , ProtocolZero>(event , maxAhead, latencyLevel))
	{
		ProtocolZero::NextSendFrame = currentFrame + ProtocolZero::SendResponseTimeInterval;
		Debug::LogInfo("[Spawner] Player {} sending response time of {}, LatencyMode = {}, Frame = {}"
			, event.HouseIndex
			, maxAhead
			, latencyLevel
			, currentFrame
		);
	}
	else
	{
		++ProtocolZero::NextSendFrame;
	}
}

void EventExt::ProtocolZero::Respond(EventClass* Event)
{
	if (!ProtocolZero::Enable || SessionClass::IsSingleplayer())
		return;

	const ProtocolZero* netData = Event->Data.nothing.As<ProtocolZero>();

	if (netData->MaxAhead == 0)
	{
		Debug::LogInfo("[Spawner] Returning because event->MaxAhead == 0");
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
		if (Unsorted::CurrentFrame >= (PlayerLastTimingFrame[i] + (ProtocolZero::SendResponseTimeFrame / 2)))
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

	EventExt::AddToEvent<false , false, FirewallToggle>(Event);
}

void EventExt::FirewallToggle::Respond(EventClass* Event)
{
	if (HouseClass* pSourceHouse = HouseClass::Array->GetItemOrDefault(Event->HouseIndex))
	{
		AresHouseExt::SetFirestormState(pSourceHouse, !pSourceHouse->FirestormActive);
	}
}
