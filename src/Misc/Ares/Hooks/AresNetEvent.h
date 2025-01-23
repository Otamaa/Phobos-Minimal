#pragma once
#include <EventClass.h>
#include <TargetClass.h>

class HouseClass;
class BuildingClass;
class CellStruct;
class EventExt {
public:

	enum class Events : uint8_t {
		TrenchRedirectClick = 0x60,
		ProtocolZero = 0x61,
		FirewallToggle = 0x62,
		ManualReload = 0x63,

		First = TrenchRedirectClick,
		Last = FirewallToggle
	};

	template<bool timestamp , class T , typename... ArgTypes>
	static bool AddToEvent(EventClass& event, ArgTypes... args) {
		T type { args... };
		event.Data.nothing.Set<T>(&type);

		if COMPILETIMEEVAL (timestamp)
			return EventClass::AddEventWithTimeStamp(&event);
		else
			return EventClass::AddEvent(&event);
	}

	struct ManualReload
	{
		ManualReload(TechnoClass* source);

		static OPTIONALINLINE COMPILETIMEEVAL size_t size() { return sizeof(ManualReload); }
		static OPTIONALINLINE COMPILETIMEEVAL EventType AsEventType()
		{
			return (EventType)Events::ManualReload;
		}

		static OPTIONALINLINE COMPILETIMEEVAL const char* name() { return "ManualReload"; }

		static void Raise(TechnoClass* Source);
		static void Respond(EventClass* Event);

		TargetClass Who;
	};

	struct TrenchRedirectClick
	{
		TrenchRedirectClick(CellStruct* target, BuildingClass* source);

		static OPTIONALINLINE COMPILETIMEEVAL size_t size() { return sizeof(TrenchRedirectClick); }
		static OPTIONALINLINE COMPILETIMEEVAL EventType AsEventType() {
			return (EventType)Events::TrenchRedirectClick;
		}

		static OPTIONALINLINE COMPILETIMEEVAL const char* name() { return "TrenchRedirectClick"; }

		static void Raise(BuildingClass* Source, CellStruct* Target);
		static void Respond(EventClass* Event);

		TargetClass TargetCell;
		TargetClass Source;
	};

	struct ProtocolZero
	{
		ProtocolZero(char maxahead, uint8_t latencylevel);

		static OPTIONALINLINE COMPILETIMEEVAL size_t size() { return sizeof(ProtocolZero); }
		static OPTIONALINLINE COMPILETIMEEVAL EventType AsEventType()
		{
			return (EventType)Events::ProtocolZero;
		}

		static OPTIONALINLINE COMPILETIMEEVAL const char* name() { return "ProtocolZero"; }

		static void Raise();
		static void Respond(EventClass* Event);

		static COMPILETIMEEVAL int SendResponseTimeInterval = 30;

		static bool Enable;
		static unsigned char MaxLatencyLevel;
		static int WorstMaxAhead;

#pragma pack(push, 1)
		char MaxAhead;
		uint8_t LatencyLevel;
#pragma pack(pop)
	};

	struct FirewallToggle
	{
		static OPTIONALINLINE COMPILETIMEEVAL size_t size() { return sizeof(FirewallToggle); }
		static OPTIONALINLINE COMPILETIMEEVAL EventType AsEventType() {
			return (EventType)Events::FirewallToggle;
		}

		static OPTIONALINLINE COMPILETIMEEVAL const char* name() { return "FirewallToggle"; }

		static void Raise(HouseClass* Source);
		static void Respond(EventClass* Event);

		TargetClass dummy; //not really used actually
	};

	static COMPILETIMEEVAL size_t GetDataSize(EventType type)
	{
		if (type <= EventType::ABANDON_ALL) // default event
			return EventClass::EventLength[(uint8_t)type];

#define GET_SIZE_EV(ev) case Events::##ev##: return ev##::size();
		switch ((Events)type)
		{
		GET_SIZE_EV(TrenchRedirectClick)
		GET_SIZE_EV(ProtocolZero)
		GET_SIZE_EV(FirewallToggle)
		GET_SIZE_EV(ManualReload)
		default :
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
		default:
			return "Unknown";
		}
#undef GET_NAME_EV
	}

	static void RespondEvent(EventClass* pEvent , Events type) {
#define RESPOND_TO_EV(ev) case EventExt::Events::##ev## : { EventExt::##ev##::Respond(pEvent); break; }
		switch (type)
		{
		RESPOND_TO_EV(TrenchRedirectClick)
		RESPOND_TO_EV(ProtocolZero)
		RESPOND_TO_EV(FirewallToggle)
		RESPOND_TO_EV(ManualReload)
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