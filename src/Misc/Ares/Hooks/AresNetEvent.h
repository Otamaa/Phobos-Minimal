#pragma once

struct NetworkEvent;
class HouseClass;
class BuildingClass;
class CellStruct;
class AresNetEvent
{
public:
	enum class Events : uint8_t
	{
		TrenchRedirectClick = 0x60,
		FirewallToggle = 0x61,
		Revealmap = 0x62,
		SetDriverKilledStatusToTrue = 0x63,

		First = TrenchRedirectClick,
		Last = Revealmap
	};

	class Handlers
	{
	public:
		static void RaiseTrenchRedirectClick(BuildingClass* Source, CellStruct* Target);
		static void RespondToTrenchRedirectClick(NetworkEvent* Event);

		static void RaiseFirewallToggle(HouseClass* Source);
		static void RespondToFirewallToggle(NetworkEvent* Event);

		static void RaiseRevealMap(HouseClass* pSource);
		static void RespondRevealMap(NetworkEvent* Event);

		static void RaiseSetDriverKilledStatusToTrue(TechnoClass* Current);
		static void ResponseToSetDriverKilledStatusToTrue(NetworkEvent* Event);
	};
};