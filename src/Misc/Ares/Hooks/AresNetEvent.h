#pragma once
#include <EventClass.h>
#include <TargetClass.h>

class HouseClass;
class BuildingClass;
class CellStruct;
class AresNetEvent {
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

		if constexpr (timestamp)
			return EventClass::AddEventWithTimeStamp(&event);
		else
			return EventClass::AddEvent(&event);
	}

	struct TrenchRedirectClick
	{
		TrenchRedirectClick(CellStruct* target, BuildingClass* source);

		static inline constexpr size_t size() { return sizeof(TrenchRedirectClick); }
		static inline constexpr EventType AsEventType() {
			return (EventType)Events::TrenchRedirectClick;
		}

		static inline constexpr const char* name() { return "TrenchRedirectClick"; }

		static void Raise(BuildingClass* Source, CellStruct* Target);
		static void Respond(EventClass* Event);

		TargetClass TargetCell;
		TargetClass Source;
	};

	struct ProtocolZero
	{
		ProtocolZero(char maxahead, uint8_t latencylevel);

		static inline constexpr size_t size() { return sizeof(ProtocolZero); }
		static inline constexpr EventType AsEventType()
		{
			return (EventType)Events::ProtocolZero;
		}

		static inline constexpr const char* name() { return "ProtocolZero"; }

		static void Raise();
		static void Respond(EventClass* Event);

		static constexpr int SendResponseTimeInterval = 30;

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
		static inline constexpr size_t size() { return sizeof(FirewallToggle); }
		static inline constexpr EventType AsEventType() {
			return (EventType)Events::FirewallToggle;
		}

		static inline constexpr const char* name() { return "FirewallToggle"; }

		static void Raise(HouseClass* Source);
		static void Respond(EventClass* Event);

		TargetClass dummy; //not really used actually
	};

	static constexpr size_t GetDataSize(EventType type)
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

	static constexpr const char* GetEventNames(Events type)
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
		case AresNetEvent::Events::TrenchRedirectClick: {
			AresNetEvent::TrenchRedirectClick::Respond(pEvent);
			break;
		}
		case AresNetEvent::Events::ProtocolZero: {
			AresNetEvent::ProtocolZero::Respond(pEvent);
			break;
		}
		case AresNetEvent::Events::FirewallToggle: {
			AresNetEvent::FirewallToggle::Respond(pEvent);
			break;
		}
		default:
			break;
		}
	}

	static constexpr bool IsValidType(Events type)
	{
		return (type >= Events::First && type <= Events::Last);
	}
};