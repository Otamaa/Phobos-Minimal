#pragma once
#include <EventClass.h>
#include <TargetClass.h>
#include <Utilities/Enum.h>
#include <SessionClass.h>

#define SET_DEFAULT_PROP(c) \
static OPTIONALINLINE COMPILETIMEEVAL size_t size() { return sizeof(##c##); } \
static OPTIONALINLINE COMPILETIMEEVAL EventType AsEventType() { return (EventType)Events::##c##; }\
static OPTIONALINLINE COMPILETIMEEVAL const char* name() { return #c; }\
static void Respond(EventClass* Event);

class HouseClass;
class BuildingClass;
class CellStruct;
class EventExt
{
public:

	enum class Events : uint8_t
	{
		TrenchRedirectClick = 0x60,
		ProtocolZero = 0x61,
		FirewallToggle = 0x62,
		ManualReload = 0x63,
		TogglePassiveAcquireMode = 0x64,
		TogglePlayerAutoRepair = 0x65,
		ApproachObject = 0x66,
		ChoiceBoxClick = 0x67,

		First = TrenchRedirectClick,
		Last = ChoiceBoxClick
	};

	template<bool timestamp, bool setData, class T, typename... ArgTypes>
	static bool AddToEvent(EventClass& event, ArgTypes... args)
	{
		if COMPILETIMEEVAL(setData) {
			T type { args... };
			event.Data.nothing.Set<T>(&type);
		}

		if COMPILETIMEEVAL(timestamp)
			return EventClass::AddEventWithTimeStamp(&event);
		else
			return EventClass::AddEvent(&event);
	}

	struct TogglePassiveAcquireMode
	{
		TogglePassiveAcquireMode(TechnoClass* pTechno, PassiveAcquireModes mode);

		SET_DEFAULT_PROP(TogglePassiveAcquireMode)

		static void Raise(TechnoClass* pTechno, PassiveAcquireModes mode);

		TargetClass Who;
		PassiveAcquireModes Mode;
	};

	struct ApproachObject
	{
		ApproachObject(FootClass* pThis, ObjectClass* pObject);

		SET_DEFAULT_PROP(ApproachObject)

		static void Raise(FootClass* pThis, ObjectClass* pObject);

		TargetClass Whom;
		TargetClass Target;
	};

	struct ManualReload
	{
		ManualReload(TechnoClass* source);

		SET_DEFAULT_PROP(ManualReload)

			static void Raise(TechnoClass* Source);

		TargetClass Who;
	};


	struct TrenchRedirectClick
	{
		TrenchRedirectClick(CellStruct* target, BuildingClass* source);

		SET_DEFAULT_PROP(TrenchRedirectClick)

		static void Raise(BuildingClass* Source, CellStruct* Target);

		TargetClass TargetCell;
		TargetClass Source;
	};

	struct ProtocolZero
	{
		ProtocolZero(char maxahead, uint8_t latencylevel);

		SET_DEFAULT_PROP(ProtocolZero)

		static void Raise();

		static COMPILETIMEEVAL int SendResponseTimeInterval = 30;
		static COMPILETIMEEVAL int SendResponseTimeFrame = 8 * SendResponseTimeInterval;

		static bool Enable;
		static unsigned char MaxLatencyLevel;
		static int WorstMaxAhead;
		static int NextSendFrame;

		static void Init()
		{
			Enable = false;
			WorstMaxAhead = 24;
			NextSendFrame = -1;
			MaxLatencyLevel = 0xff;
		}

#pragma pack(push, 1)
		char MaxAhead;
		uint8_t LatencyLevel;
#pragma pack(pop)
	};

	struct FirewallToggle
	{
		SET_DEFAULT_PROP(FirewallToggle)

		static void Raise(HouseClass* Source);

		TargetClass dummy; //not really used actually
	};

	struct TogglePlayerAutoRepair
	{
		SET_DEFAULT_PROP(TogglePlayerAutoRepair)

		static void Raise();

	};

	// Carries a player's answer to an on-map choice dialog.
	//
	// The dialog is NOT networked. It is created by a TAction, and the trigger
	// system already runs in lockstep, so every machine builds the same box on
	// the same frame from the same INI data. Networking creation would only add
	// a race. Only the answer travels.
	//
	// INVARIANT: ClickedIndex is written by Respond() and by the timeout path
	// in DrawChoiceBoxList (which writes -2). It is never written from local
	// input. CheckMouseClick() only hit-tests and calls Raise().
	struct ChoiceBoxClick
	{
		ChoiceBoxClick(int boxID, int buttonIndex);

		SET_DEFAULT_PROP(ChoiceBoxClick)

		// Called from local UI hit-testing, on the clicking machine only.
		static void Raise(int boxID, int buttonIndex);

		// REV2: renamed from GraceFrames and re-scoped.
		//
		// How long past the visual deadline a box keeps ACCEPTING answers, so
		// that a click made before the deadline is not invalidated by network
		// flight time. This is a THIRD window, distinct from the two that
		// already exist:
		//
		//   1. NetworkGraceFrames  (this)  - late answers still judged
		//   2. CLICK_EXPIRE_FRAMES         - answered box stays findable so
		//                                    TEvent 557/558 can poll it
		//   3. post-expiry lifetime        - timed-out box stays findable so
		//                                    TEvent 559 can poll it
		//
		// Timeline:  deadline -> [1] -> timeout flip -> [2 or 3] -> removal
		//
		// MUST be 0 in singleplayer. There is no flight time, and a non-zero
		// value would delay the timeout flip and change existing campaign
		// behaviour. See NetworkGrace() below.
		//
		// VERIFY: 120 frames against your worst-case latency config. It has to
		// exceed the command delay, i.e. Game::Network::MaxAhead at its
		// largest. Deliberately a constant and not MaxAhead itself, since
		// LatencyLevel::Apply() mutates that at runtime and a value read at
		// different moments on different machines would not be deterministic.
		static COMPILETIMEEVAL int NetworkGraceFrames = 120;

		// SessionClass::IsSingleplayer() is identical on every machine in an MP
		// session, so branching on it here stays deterministic.
		static int NetworkGrace()
		{
			return SessionClass::IsSingleplayer() ? 0 : NetworkGraceFrames;
		}

		int BoxID;
		int ButtonIndex;
	};


	static COMPILETIMEEVAL size_t GetDataSize(EventType type)
	{
		if (type < (EventType)EventClass::EventLength.size())
			return EventClass::EventLength[(uint8_t)type];

#define GET_SIZE_EV(ev) case Events::##ev##: return ev##::size();
		switch ((Events)type)
		{
			GET_SIZE_EV(TrenchRedirectClick)
			GET_SIZE_EV(ProtocolZero)
			GET_SIZE_EV(FirewallToggle)
			GET_SIZE_EV(ManualReload)
			GET_SIZE_EV(TogglePassiveAcquireMode)
			GET_SIZE_EV(TogglePlayerAutoRepair)
			GET_SIZE_EV(ApproachObject)
			GET_SIZE_EV(ChoiceBoxClick)
		default:
			return 0;
		}
#undef GET_SIZE_EV
	}

	static COMPILETIMEEVAL const char* GetEventNames(Events type)
	{
#define GET_NAME_EV(ev) case Events::##ev##: return ev##::name();

		switch (type)
		{
			GET_NAME_EV(TrenchRedirectClick)
			GET_NAME_EV(ProtocolZero)
			GET_NAME_EV(FirewallToggle)
			GET_NAME_EV(ManualReload)
			GET_NAME_EV(TogglePassiveAcquireMode)
			GET_NAME_EV(TogglePlayerAutoRepair)
			GET_NAME_EV(ApproachObject)
			GET_NAME_EV(ChoiceBoxClick)
		default:
			return "Unknown";
		}
#undef GET_NAME_EV
	}

	static void RespondEvent(EventClass* pEvent, Events type)
	{
#define RESPOND_TO_EV(ev) case EventExt::Events::##ev## : { EventExt::##ev##::Respond(pEvent); break; }
		switch (type)
		{
			RESPOND_TO_EV(TrenchRedirectClick)
			RESPOND_TO_EV(ProtocolZero)
			RESPOND_TO_EV(FirewallToggle)
			RESPOND_TO_EV(ManualReload)
			RESPOND_TO_EV(TogglePassiveAcquireMode)
			RESPOND_TO_EV(TogglePlayerAutoRepair)
			RESPOND_TO_EV(ApproachObject)
			RESPOND_TO_EV(ChoiceBoxClick)
		default:
			break;
		}
#undef RESPOND_TO_EV
	}

	static COMPILETIMEEVAL bool IsValidType(Events type)
	{
		return (type >= Events::First && type <= Events::Last);
	}
};


class FakeEventClass : public EventClass
{
	void _Execute();
};

static_assert(sizeof(FakeEventClass) == sizeof(EventClass), "Size Missmatch !");

#undef SET_DEFAULT_PROP