#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

#include <WWKeyboardClass.h>

#include <Misc/Spawner/ProtocolZero.h>
#include <IPXManagerClass.h>

#include <New/ChoiceBox/Entities/Base/MapChoiceBoxClass.h>
#include <New/TextBox/Entities/Base/MapTextBoxClass.h>

#include <New/TextBox/Types/TextBoxTypeClass.h>
#include <New/ChoiceBox/Types/ChoiceBoxTypeClass.h>

EventExt::ApproachObject::ApproachObject(FootClass* pThis, ObjectClass* pObject) :
	Whom { pThis }, Target { pObject } { }

void EventExt::ApproachObject::Raise(FootClass* pThis, ObjectClass* pObject)
{
	EventClass Event {};
	if (pThis->Owner->ArrayIndex >= 0) {
		Event.Type = AsEventType();
		Event.HouseIndex = byte(pThis->Owner->ArrayIndex);
	}
	EventExt::AddToEvent<true, true, ApproachObject>(Event, pThis, pObject);
}

#include <SlaveManagerClass.h>

void EventExt::ApproachObject::Respond(EventClass* Event)
{

	const auto pSource = Event->Data.nothing.As<ApproachObject>()->Whom.As_Foot();

	if (!pSource || static_cast<char>(pSource->Owner->ArrayIndex) != Event->HouseIndex)
		return;

	pSource->ClearPlanningTokens(nullptr);

	if (!pSource->IsAlive || pSource->Health <= 0 || pSource->InLimbo)
		return;

	if (pSource->IsTethered)
	{
		const auto pLink = cast_to<BuildingClass*>(pSource->GetNthLink());

		if (pLink && pLink->IsAlive && pLink->Type->DockUnload)
		{
			pSource->SendToFirstLink(RadioCommand::NotifyUnlink);
			pSource->IsTethered = false;
		}
	}
	else
	{
		pSource->SendToFirstLink(RadioCommand::NotifyUnlink);
	}

	pSource->QueueUpToEnter = nullptr;
	pSource->LastDestination = nullptr;

	if (const auto pManager = pSource->SlaveManager)
		pManager->AllGuard();

	pSource->ClearNavQueue();
	pSource->SetDestination(nullptr, true);
	pSource->SetTarget(nullptr);
	pSource->SetArchiveTarget(nullptr);

	const auto pObject = Event->Data.nothing.As<ApproachObject>()->Target.As_Object();

	if (!pObject)
		return;

	const auto pOriginalTarget = std::exchange(pSource->Target, pObject);
	pSource->ApproachTarget(0);
	pSource->Target = pOriginalTarget;
}

EventExt::TogglePassiveAcquireMode::TogglePassiveAcquireMode(TechnoClass* pTechno, PassiveAcquireModes mode) : Who { pTechno } , Mode { mode }
{ }

void EventExt::TogglePassiveAcquireMode::Raise(TechnoClass* pTechno, PassiveAcquireModes mode)
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
			const auto pType = GET_TECHNOTYPE(pTechno);
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
			if (TechnoExtData::canTraverseTo(pSourceBuilding, targetBuilding)) // check has happened before the enter cursor appeared
				TechnoExtData::doTraverseTo(pSourceBuilding, targetBuilding);
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

	int currentFrame = Unsorted::CurrentFrame.get();

	if (ProtocolZero::NextSendFrame < 0) {
		ProtocolZero::NextSendFrame = currentFrame + Game::Network::FrameSendRate.get() + ProtocolZero::SendResponseTimeFrame;
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
	event.Frame = currentFrame + Game::Network::MaxAhead.get();
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
		if (Unsorted::CurrentFrame.get() >= (PlayerLastTimingFrame[i] + (ProtocolZero::SendResponseTimeFrame / 2)))
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
	if (HouseClass* pSourceHouse = HouseClass::Array->get_or_default(Event->HouseIndex))
	{
		HouseExtData::SetFirestormState(pSourceHouse, !pSourceHouse->FirestormActive);
	}
}

void EventExt::TogglePlayerAutoRepair::Raise()
{
	EventClass Event {};

	Event.Type = AsEventType();
	Event.HouseIndex = byte(HouseClass::CurrentPlayer->ArrayIndex);

	EventExt::AddToEvent<true, false, TogglePlayerAutoRepair>(Event);
}

void EventExt::TogglePlayerAutoRepair::Respond(EventClass* Event)
{
	if (HouseClass* pSourceHouse = HouseClass::Array->get_or_default(Event->HouseIndex))
	{
		auto pExt = HouseExtContainer::Instance.Find(pSourceHouse);

		pExt->PlayerAutoRepair = !pExt->PlayerAutoRepair;

		if (HouseClass::CurrentPlayer.get() == pSourceHouse && !Phobos::Config::TogglePowerInsteadOfRepair) {
			SidebarClass::Instance->SidebarNeedsRedraw = true;
			auto pButton = &Make_Global<ShapeButtonClass>(0xB0B3A0);

			if (pExt->PlayerAutoRepair)
				pButton->TurnOn();
			else
				pButton->TurnOff();
		}
	}
}

EventExt::ChoiceBoxClick::ChoiceBoxClick(int boxID, int buttonIndex)
	: BoxID { boxID }, ButtonIndex { buttonIndex }
{}


// ============================================================================
// Raise - clicking machine only
// ============================================================================

void EventExt::ChoiceBoxClick::Raise(int boxID, int buttonIndex)
{
	// Observers and spectators have no valid house and cannot answer. Matches
	// the ArrayIndex >= 0 guard used by ApproachObject and ManualReload.
	const auto pPlayer = HouseClass::CurrentPlayer.get();

	if (!pPlayer || pPlayer->ArrayIndex < 0)
		return;

	// Boxes created without an explicit ID carry -1 and are unaddressable by
	// FindByID, so an answer could never be routed back to them.
	if (boxID < 0)
		return;

	EventClass Event {};
	Event.Type = AsEventType();
	Event.HouseIndex = byte(pPlayer->ArrayIndex);

	// timestamp = true: AddEventWithTimeStamp stamps Frame with the frame the
	// click was MADE, not the frame it executes on. Respond() needs exactly
	// that to judge the answer against the deadline fairly - command delay must
	// not be able to invalidate an otherwise legal click.
	//
	// Contrast ProtocolZero, which passes timestamp = false and sets Frame to
	// the intended EXECUTION frame. Opposite need, opposite flag.
	EventExt::AddToEvent<true, true, ChoiceBoxClick>(Event, boxID, buttonIndex);
}

// ============================================================================
// Respond - every machine, same frame
// ============================================================================

void EventExt::ChoiceBoxClick::Respond(EventClass* Event)
{
	const auto pData = Event->Data.nothing.As<ChoiceBoxClick>();

	if (!pData)
		return;

	// ---- box still exists? -------------------------------------------------
	// Guaranteed by the delayed timeout flip in TickDuration(): the box is not
	// flipped to expired until past DeadlineFrame + NetworkGrace().
	const auto pBox = MapChoiceBoxClass::FindByID(pData->BoxID);

	if (!pBox)
		return;

	// ---- sending house valid? ----------------------------------------------
	// HouseIndex is a signed char and EventClass documents -1 as "not a valid
	// house". Never trust it unchecked. Same shape as FirewallToggle.
	if (!HouseClass::Array->get_or_default(Event->HouseIndex))
		return;

	// NOTE: no per-house filter. TEvent 557/558/559 all take HouseClass* pHouse
	// and none of them use it, so gating who may ANSWER while any house's
	// trigger can still CONSUME the answer would read as enforced when it is
	// not. Add both halves together or neither - and note that adding them
	// changes behaviour for existing missions.

	// ---- first answer wins -------------------------------------------------
	// DoList order is identical on every machine, so "first" is deterministic
	// with no tiebreak needed. Later answers in the same round are dropped.
	//
	// Tested via AnsweredBy rather than ClickedIndex, which cannot distinguish
	// "unanswered" from "answered with button 0" (-2 already means timed out).
	if (pBox->AnsweredBy >= 0)
		return;

	// ---- already timed out? ------------------------------------------------
	// Should be unreachable while the grace window holds, since the flip is
	// delayed past it. Kept as a hard guard: if this ever logs, the grace
	// window is too short for the current latency configuration.
	if (pBox->IsExpired)
	{
		Debug::LogInfo("[ChoiceBoxClick] Answer for box {} from house {} arrived after the"
			" timeout flip - NetworkGraceFrames ({}) is too short.",
			pData->BoxID, static_cast<int>(Event->HouseIndex), NetworkGrace());
		return;
	}

	// ---- was the click made in time? ---------------------------------------
	// Judged against the frame the click was MADE (Event->Frame), not the frame
	// this executes on. Both come from the packet or lockstep state, so every
	// machine reaches the same verdict.
	//
	// DeadlineFrame < 0 means no timeout - the box accepts indefinitely.
	if (pBox->DeadlineFrame >= 0 && static_cast<int>(Event->Frame) > pBox->DeadlineFrame)
		return;

	// ---- button index in range? --------------------------------------------
	// Never trust an index off the wire as a subscript.
	// ValueableVector<T> derives from std::vector<T>, so size() is direct.
	const auto pType = pBox->Type;

	if (!pType)
		return;

	if (pData->ButtonIndex < 0
		|| pData->ButtonIndex >= static_cast<int>(pType->Buttons.size()))
	{
		Debug::LogInfo("[ChoiceBoxClick] House {} sent out-of-range button {} for box {}"
			" ({} buttons)",
			static_cast<int>(Event->HouseIndex), pData->ButtonIndex, pData->BoxID,
			static_cast<int>(pType->Buttons.size()));
		return;
	}

	// ========================================================================
	// commit
	//
	// This is the only place ClickedIndex is written to a button value. The
	// only other writer is TickDuration(), which writes the -2 timeout
	// sentinel. If you find a third, that writer is a desync.
	// ========================================================================
	pBox->ClickedIndex = pData->ButtonIndex;
	pBox->AnsweredBy = static_cast<int>(Event->HouseIndex);

	// Carried over from what phase one used to do immediately after
	// CheckMouseClick() returned true. It now happens here, on the frame the
	// answer resolves, on every machine.
	pBox->ClickedConsumed = false;

	// 回弹模式：点击即启动隐藏期倒计时（不依赖 Duration 耗尽）
	if (pType->Button_Mode == static_cast<int>(ChoiceBoxButtonMode::Bounce))
	{
		pBox->ClickExpireCounter = MapChoiceBoxClass::CLICK_EXPIRE_FRAMES;
	}
	else
	{
		// Non-bounce: stop the visual countdown and open the hidden period so
		// TEvent 557/558 have a stable window to observe ClickedIndex in.
		pBox->RemainingFrames = 0;

		if (pBox->ClickExpireCounter < 0)
			pBox->ClickExpireCounter = MapChoiceBoxClass::CLICK_EXPIRE_FRAMES;
	}

	// Deliberately NOT touched:
	//
	//   IsExpired - means "timed out UNANSWERED". TEvent 559 polls it with no
	//               consumption latch, so setting it here would fire the
	//               timeout trigger for a box that was answered correctly.
	//
	//   ClickedConsumed is set false above, not true: it is the trigger
	//   system's one-fire-per-click latch and only a TEvent may claim it.
}

void FakeEventClass::_Execute()
{

}
