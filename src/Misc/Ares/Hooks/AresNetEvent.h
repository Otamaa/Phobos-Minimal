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
		FirewallToggle = 0x61,

		First = TrenchRedirectClick,
		Last = FirewallToggle
	};

	template<bool timestamp , class T , typename... ArgTypes>
	static void AddToEvent(EventClass& event, ArgTypes... args) {
		T type { args... };
		memcpy(&event.Data.nothing, &type, T::size());

		if constexpr (timestamp)
			EventClass::AddEventWithTimeStamp(&event);
		else
			EventClass::AddEvent(&event);
	}

	struct TrenchRedirectClick
	{
		TrenchRedirectClick(CellStruct* target, BuildingClass* source);

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

	static constexpr size_t GetDataSize(EventType type)
	{
		if (type <= EventType::ABANDON_ALL) // default event
			return EventClass::EventLength[(uint8_t)type];

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

	static constexpr bool IsValidType(Events type)
	{
		return (type >= Events::First && type <= Events::Last);
	}
};