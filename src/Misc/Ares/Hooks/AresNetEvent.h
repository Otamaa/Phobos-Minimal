#pragma once
#include <EventClass.h>

class HouseClass;
class BuildingClass;
class CellStruct;
class AresNetEvent {
public:

	enum class Events : uint8_t {
		TrenchRedirectClick = 0x60,
		FirewallToggle = 0x61,
		RespondTime2 = 0x62,

		First = TrenchRedirectClick,
		Last = RespondTime2
	};

	struct ResponseTime2 {

		static inline constexpr size_t size() { return sizeof(ResponseTime2); }
		static inline constexpr EventType AsEventType() {
			return (EventType)Events::RespondTime2;
		}

		static void Raise();
		static void Respond(EventClass* Event);

		char MaxAhead;
		uint8_t LatencyLevel;
	};

	struct TrenchRedirectClick
	{
		TrenchRedirectClick(CellStruct* target , BuildingClass* source)
			: TargetCell { target } , Source { source }
		{ }

		static inline constexpr size_t size() { return sizeof(TrenchRedirectClick); }
		static inline constexpr EventType AsEventType() {
			return (EventType)Events::TrenchRedirectClick;
		}

		static void Raise(BuildingClass* Source, CellStruct* Target);
		static void Respond(EventClass* Event);

		TargetClass TargetCell;
		TargetClass Source;
	};

	struct FirewallToggle
	{
		static inline constexpr size_t size() { return sizeof(FirewallToggle); }
		static inline constexpr EventType AsEventType() {
			return (EventType)Events::FirewallToggle;
		}

		static void Raise(HouseClass* Source);
		static void Respond(EventClass* Event);

		TargetClass dummy; //not really used actually
	};

	static size_t GetDataSize(uint8_t type)
	{
		if ((EventType)type <= EventType::ABANDON_ALL) // default event
			return EventClass::EventLength[type];

		switch ((Events)type)
		{
		case Events::TrenchRedirectClick:
			return TrenchRedirectClick::size();
		case Events::FirewallToggle:
			return FirewallToggle::size();
		default :
			return 0;
		}
	}

	static void RespondEvent(EventClass* pEvent , Events type) {
		switch (type)
		{
		case AresNetEvent::Events::TrenchRedirectClick: {
			AresNetEvent::TrenchRedirectClick::Respond(pEvent);
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

	static bool IsValidType(Events type)
	{
		return (type >= Events::First && type <= Events::Last);
	}
};