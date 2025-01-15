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

		First = TrenchRedirectClick,
		Last = FirewallToggle
	};

	template<bool timestamp , class T , typename... ArgTypes>
	static bool AddToEvent(EventClass& event, ArgTypes... args) {
		T type { args... };
		memcpy(&event.Data.nothing, &type, T::size());

		if COMPILETIMEEVAL (timestamp)
			return EventClass::AddEventWithTimeStamp(&event);
		else
			return EventClass::AddEvent(&event);
	}

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

		switch ((Events)type)
		{
		case Events::TrenchRedirectClick:
			return TrenchRedirectClick::size();
		case Events::ProtocolZero:
			return ProtocolZero::size();
		case Events::FirewallToggle:
			return FirewallToggle::size();
		default :
			return 0;
		}
	}

	static COMPILETIMEEVAL const char* GetEventNames(Events type)
	{
		switch (type)
		{
		case Events::TrenchRedirectClick:
			return TrenchRedirectClick::name();
		case Events::ProtocolZero:
			return ProtocolZero::name();
		case Events::FirewallToggle:
			return FirewallToggle::name();
		default:
			return "Unknown";
		}
	}

	static void RespondEvent(EventClass* pEvent , Events type) {
		switch (type)
		{
		case EventExt::Events::TrenchRedirectClick: {
			EventExt::TrenchRedirectClick::Respond(pEvent);
			break;
		}
		case EventExt::Events::ProtocolZero: {
			EventExt::ProtocolZero::Respond(pEvent);
			break;
		}
		case EventExt::Events::FirewallToggle: {
			EventExt::FirewallToggle::Respond(pEvent);
			break;
		}
		default:
			break;
		}
	}

	static COMPILETIMEEVAL bool IsValidType(Events type)
	{
		return (type >= Events::First && type <= Events::Last);
	}
};