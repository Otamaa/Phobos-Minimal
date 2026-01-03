#include "EnumFunctions.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <CellClass.h>

#include <TechnoClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>
#include <GeneralDefinitions.h>

std::array<const char*, (size_t)DiscardCondition::count>  EnumFunctions::DiscardCondition_to_strings {
 {
	{ "none" } ,
	{ "entry" } ,
	{ "move" } ,
	{ "stationary" } ,
	{ "drain" } ,
	{ "inrange" } ,
	{ "outofrange" },
	{ "InvokerDeleted" },
	{ "firing"}
 }
};

std::array<const char*, 6u>  EnumFunctions::ExpireWeaponCondition_to_strings {
 {
	{ "none" } ,
	{ "expire" } ,
	{ "remove" } ,
	{ "death" } ,
	{ "discard" },
	{ "all" }
 }
};

std::array<const char*, 6u> EnumFunctions::DisplayShowType_ToStrings {
{
	{ "none" } ,
	{ "cursorhover" } ,
	{ "selected" } ,
	{ "idle" } ,
	{ "select" } ,
	{ "all" } ,
}
};

std::array<const char*, (size_t)PhobosAbilityType::count> EnumFunctions::PhobosAbilityType_ToStrings {
 {
	{ "INTERCEPTOR" } ,
	{ "CHRONODELAYIMMUNE" } ,
	{ "CRITIMMUNE" } ,
	{ "PSIONICSIMMUNE" } ,
	{ "CULLINGIMMUNE" } ,
	{ "EMPIMMUNE" } ,
	{ "RADIMMUNE" } ,
	{ "PROTECTED_DRIVER" } ,
	{ "UNWARPABLE" } ,
	{ "POISONIMMUNE" } ,
	{ "PSIONICSWEAPONIMMUNE" } ,
	{ "BERSERKIMMUNE" } ,
	{ "ABDUCTORIMMUNE" } ,
	{ "ASSAULTER" } ,
	{ "PARASITEIMMUNE" } ,
	{ "BOUNTYHUNTER" } ,
	{ "WEBIMMUNE" } ,
	{ "UNTRACKABLE" }
 }
};

std::array<const char*, 6u> EnumFunctions::MouseCursorData_ToStrings
{
{
	{ ".Frame" } ,
	{ ".Count" } ,
	{ ".Interval" } ,
	{ ".MiniFrame" } ,
	{ ".MiniCount" } ,
	{ ".MiniInterval" }
}
};

std::array<const char*, 21u> EnumFunctions::TileType_ToStrings
{
{
	{"Unknown"} ,
	{"Tunnel"} ,
	{"Water"} ,
	{"Ramp"} ,
	{"Cliff"},
	{"Blank"} ,
	{"Shore"} ,
	{"Wet"} ,
	{"MiscPave"} ,
	{"Pave"} ,
	{"DirtRoad"} ,
	{"PavedRoad"} ,
	{"PavedRoadEnd"} ,
	{"PavedRoadSlope"} ,
	{"Median"} ,
	{"Bridge"} ,
	{"WoodBridge"} ,
	{"ClearToSandLAT"} ,
	{"Green"} ,
	{"NotWater"} ,
	{"DestroyableCliff"}
}
};

std::array<std::pair<const char*, const char*>, 11u> EnumFunctions::LocomotorPairs_ToStrings
{
{
	{"Drive","{4A582741-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Jumpjet","{92612C46-F71F-11d1-AC9F-006008055BB5}"} ,
	{"Hover","{4A582742-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Rocket","{B7B49766-E576-11d3-9BD9-00104B972FE8}"} ,
	{"Tunnel","{4A582743-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Walk","{4A582744-9839-11d1-B709-00A024DDAFD1}"} ,
	{"DropPod","{4A582745-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Fly","{4A582746-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Teleport","{4A582747-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Mech","{55D141B8-DB94-11d1-AC98-006008055BB5}"} ,
	{"Ship","{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}"}
}
};

std::array<std::pair<const wchar_t*, const wchar_t*>, 11u> EnumFunctions::LocomotorPairs_ToWideStrings
{
{
	{ L"Drive", L"{4A582741-9839-11d1-B709-00A024DDAFD1}"},
	{ L"Jumpjet",L"{92612C46-F71F-11d1-AC9F-006008055BB5}" },
	{ L"Hover", L"{4A582742-9839-11d1-B709-00A024DDAFD1}" },
	{ L"Rocket", L"{B7B49766-E576-11d3-9BD9-00104B972FE8}" },
	{ L"Tunnel", L"{4A582743-9839-11d1-B709-00A024DDAFD1}" },
	{ L"Walk", L"{4A582744-9839-11d1-B709-00A024DDAFD1}" },
	{ L"DropPod", L"{4A582745-9839-11d1-B709-00A024DDAFD1}" },
	{ L"Fly", L"{4A582746-9839-11d1-B709-00A024DDAFD1}" },
	{ L"Teleport", L"{4A582747-9839-11d1-B709-00A024DDAFD1}" },
	{ L"Mech", L"{55D141B8-DB94-11d1-AC98-006008055BB5}" },
	{ L"Ship", L"{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}" }
}
};

std::array<std::pair<const char*, SpotlightFlags>, 5u> EnumFunctions::SpotlightFlags_ToStrings
{
{
	{"none" , SpotlightFlags::None},
	{ "NoColor" , SpotlightFlags::NoColor },
	{ "NoRed" , SpotlightFlags::NoRed },
	{ "NoGreen" , SpotlightFlags::NoGreen },
	{ "NoBlue" , SpotlightFlags::NoBlue }
}
};

std::array<std::pair<const char*, MouseHotSpotX>, 3u> EnumFunctions::MouseHotSpotX_ToStrings
{
{
	{ "left", MouseHotSpotX::Left },
	{ "center", MouseHotSpotX::Center },
	{ "right", MouseHotSpotX::Right }
}
};

std::array<std::pair<const char*, MouseHotSpotY>, 3u> EnumFunctions::MouseHotSpotY_ToStrings
{
{
	{ "top", MouseHotSpotY::Top },
	{ "bottom", MouseHotSpotY::Bottom },
	{ "middle", MouseHotSpotY::Middle }
}
};

std::array<std::pair<const char*, TextAlign>, 4u> EnumFunctions::TextAlign_ToStrings
{
{
	{"none" , TextAlign::None} ,
	{"left" , TextAlign::Left} ,
	{"center", TextAlign::Center } ,
	{"right", TextAlign::Right }
}
};

std::array<std::pair<const char*, AttachedAnimFlag>, 5u>  EnumFunctions::AttachedAnimFlag_ToStrings
{
{
	{"none" , AttachedAnimFlag::None} ,
	{"hides" , AttachedAnimFlag::Hides } ,
	{"temporal" , AttachedAnimFlag::Temporal } ,
	{"paused" , AttachedAnimFlag::Paused },
	{"pausedtemporal" , AttachedAnimFlag::PausedTemporal }
}
};

std::array<std::pair<const char*, AffectedHouse>, 11u> EnumFunctions::AffectedHouse_ToStrings
{
{
	{"none" , AffectedHouse::None} ,
	{"owner" , AffectedHouse::Owner } ,
	{"self" , AffectedHouse::Owner } ,
	{"allies" , AffectedHouse::Allies } ,
	{"ally" , AffectedHouse::Allies } ,
	{"enemies" , AffectedHouse::Enemies } ,
	{"enemy" , AffectedHouse::Enemies } ,
	{"team" , AffectedHouse::Team } ,
	{"others" , AffectedHouse::NotOwner } ,
	{"all" , AffectedHouse::All } ,
	{"trump" , AffectedHouse::NotAllies }
}
};

std::array<std::pair<const char*, AffectedTarget>, 15u> EnumFunctions::AffectedTarget_ToStrings
{
{
	{"none" , AffectedTarget::None} ,
	{"Land" , AffectedTarget::Land } ,
	{"Water" , AffectedTarget::Water } ,
	{"NoContent" , AffectedTarget::NoContent } ,
	{"Infantry" , AffectedTarget::Infantry } ,
	{"Unit" , AffectedTarget::Unit } ,
	{"Units" , AffectedTarget::Unit } ,
	{"Building" , AffectedTarget::Building } ,
	{"Buildings" , AffectedTarget::Building } ,
	{"Aircraft" , AffectedTarget::Aircraft } ,
	{"All" , AffectedTarget::All } ,
	{"AllCells" , AffectedTarget::AllCells } ,
	{"AllTechnos" , AffectedTarget::AllTechnos } ,
	{"AllContents" , AffectedTarget::AllContents } ,
	{"Empty" , AffectedTarget::NoContent }
}
};

std::array<std::pair<const char*, TargetingConstraints>, 10u> EnumFunctions::TargetingConstraints_ToStrings
{
{
	{"none" , TargetingConstraints::None } ,
	{"offensive_cell_clear", TargetingConstraints::OffensiveCellClear } ,
	{"defensive_cell_clear", TargetingConstraints::DefensifeCellClear } ,
	{"enemy", TargetingConstraints::Enemy } ,
	{"lightningstorm_inactive", TargetingConstraints::LighningStormInactive } ,
	{"dominator_inactive", TargetingConstraints::DominatorInactive } ,
	{"attacked", TargetingConstraints::Attacked } ,
	{"lowpower", TargetingConstraints::LowPower },
	{"offensive_cell_set", TargetingConstraints::OffensiveCellSet } ,
	{"defensive_cell_set", TargetingConstraints::DefensiveCellSet }
}
};

std::array<std::pair<const char*, ChronoSparkleDisplayPosition>, 5u> EnumFunctions::ChronoSparkleDisplayPosition_ToStrings
{
{
	{ "none" , ChronoSparkleDisplayPosition::None},
	{ "Building" , ChronoSparkleDisplayPosition::Building },
	{ "occupants" , ChronoSparkleDisplayPosition::Occupants },
	{ "occupantslots", ChronoSparkleDisplayPosition::OccupantSlots } ,
	{ "all" , ChronoSparkleDisplayPosition::All }
}
};

std::array<std::pair<const char* ,AffectPlayerType>, 5u> EnumFunctions::AffectPlayerType_ToStrings
{
{	{ "none" ,AffectPlayerType::None },
	{ "computer" ,AffectPlayerType::Computer },
	{ "player" ,AffectPlayerType::Player },
	{ "observer" ,AffectPlayerType::Observer },
	{ "all" , AffectPlayerType::Computer | AffectPlayerType::Player | AffectPlayerType::Observer }
}
};

std::array<const char*, (size_t)NewCrateType::count> EnumFunctions::NewCrateType_ToStrings
{{
	{ "Money" }, { "Super" }, { "Weapon" }, { "Units" }
 }};

std::array<const char*, 6u> EnumFunctions::DamageState_to_strings
{
{
	"Unaffected", "Unchanged", "NowYellow", "NowRed", "NowDead", "PostMortem"
}
};

std::array<const char*, (size_t)TrajectoryCheckReturnType::count>  EnumFunctions::TrajectoryCheckReturnType_to_strings
{
	"ExecuteGameCheck" , "SkipGameCheck" , "SatisfyGameCheck" , "Detonate"
};

std::array<const char*, 8u> EnumFunctions::FacingType_to_strings
{
{
	"N" , "NE", "E" , "SE", "S", "SW", "W", "NW",
}
};

bool EnumFunctions::CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if(flags != AffectedHouse::None) {

		if (flags == AffectedHouse::All)
			return true;

		if (ownerHouse && targetHouse) {

			if ((flags & AffectedHouse::Owner) && ownerHouse == targetHouse)
				return true;

			const auto IsAlly = ownerHouse->IsAlliedWith(targetHouse);
			return (flags & AffectedHouse::Allies) && ownerHouse != targetHouse && IsAlly ||
				(flags & AffectedHouse::Enemies) && ownerHouse != targetHouse && !IsAlly;
		}

		return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
	}

	return false;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget const& allowed, bool explicitEmptyCells, bool considerBridgesLand)
{
	if (allowed == AffectedTarget::All)
		return true;

	if (explicitEmptyCells)
	{
		const auto pTechno = flag_cast_to<TechnoClass*>(pCell->GetContent());

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	if (allowed & AffectedTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water && (!considerBridgesLand || !pCell->ContainsBridge())) // check whether it supports water
			return (allowed & AffectedTarget::Water) != AffectedTarget::None;
		else                                    // check whether it supports non-water
			return (allowed & AffectedTarget::Land) != AffectedTarget::None;
	}

	return allowed != AffectedTarget::None;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget  const& allowed, bool considerAircraftSeparately)
{
	if (allowed == AffectedTarget::All)
		return true;

	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case InfantryClass::AbsID:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case UnitClass::AbsID:
			case AircraftClass::AbsID:
				if (!considerAircraftSeparately)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case BuildingClass::AbsID:
				if (pTechno->IsStrange())
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Building) != AffectedTarget::None;
			}
		}
		else
		{
			// is the target cell allowed to be empty?
			return (allowed & AffectedTarget::NoContent) != AffectedTarget::None;
		}
	}

	return allowed != AffectedTarget::None;
}

bool EnumFunctions::IsTechnoEligibleB(TechnoClass* const pTechno, AffectedTarget const& allowed)
{
	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case InfantryClass::AbsID:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case UnitClass::AbsID:
			{
				if(!((UnitClass*)pTechno)->Type->ConsideredAircraft)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;

				return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			}
			case AircraftClass::AbsID:
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case BuildingClass::AbsID:
			{
				return ((allowed & AffectedTarget::Building) != AffectedTarget::None);
			}
			}
		}

		return true;
	}

	return false;
}

bool EnumFunctions::CanAffectTechnoResult(AbstractType type, AffectedTechno allowed) {

	if (allowed != AffectedTechno::None) {

		switch (type) {
		case AbstractType::Building:
			return (allowed & AffectedTechno::Building) != AffectedTechno::None;
		case AbstractType::Infantry:
			return (allowed & AffectedTechno::Infantry) != AffectedTechno::None;
		case AbstractType::Unit:
			return (allowed & AffectedTechno::Unit) != AffectedTechno::None;
		case AbstractType::Aircraft:
			return (allowed & AffectedTechno::Aircraft) != AffectedTechno::None;

		}
	}
	return false;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse const&  allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately, bool allowBridges)
{
	if(!EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells , allowBridges))
		return false;

	for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
	{
		if (auto pTechno = flag_cast_to<TechnoClass*, false>(pObject))
		{
			if (owner && !EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner)) {
				return false;
			}

			if(!EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately))
				return false;
		}
	}

	return true;
}

BlitterFlags EnumFunctions::GetTranslucentLevel(int const& nInt)
{
	switch (nInt)
	{
	case 1:
		return BlitterFlags::TransLucent25;
	case 2:
		return BlitterFlags::TransLucent50;
	case 3:
		return BlitterFlags::TransLucent75;
	}

	return BlitterFlags::None;
}

TextPrintType EnumFunctions::CastAlignToFlags(HorizontalPosition const& pos)
{
	if (pos == HorizontalPosition::Center)
		return TextPrintType::Center;
	if (pos == HorizontalPosition::Right)
		return TextPrintType::Right;

	return (TextPrintType)0x0; // TextPrintType::Left that doesn't exist and is assumed by default
}

IronCurtainFlag EnumFunctions::GetICFlagResult(IronCurtainFlag const& Input)
{
	if (Input == IronCurtainFlag::Random)
		return (IronCurtainFlag)ScenarioClass::Instance->Random.RandomRanged((int)IronCurtainFlag::Kill, (int)IronCurtainFlag::Ignore);

	return Input;
}

#include <Locomotor/DriveLocomotionClass.h>
#include <Locomotor/JumpjetLocomotionClass.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <Locomotor/RocketLocomotionClass.h>
#include <Locomotor/TunnelLocomotionClass.h>
#include <Locomotor/WalkLocomotionClass.h>
#include <Locomotor/DropPodLocomotionClass.h>
#include <Locomotor/FlyLocomotionClass.h>
#include <Locomotor/TeleportLocomotionClass.h>
#include <Locomotor/MechLocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>

static std::pair<const char*, const char*> UnkPair = std::make_pair("Unk", "unk");

std::pair<const char*, const char*>* EnumFunctions::locomotion_toSring(LocomotionClass* ptr)
{
	switch (VTable::Get(ptr))
	{
	case DriveLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[0];
	case JumpjetLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[1];
	case HoverLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[2];
	case RocketLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[3];
	case TunnelLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[4];
	case WalkLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[5];
	case DropPodLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[6];
	case FlyLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[7];
	case TeleportLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[8];
	case MechLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[9];
	case ShipLocomotionClass::vtable:
		return &LocomotorPairs_ToStrings[10];
	default:
		return &UnkPair;
	}
}

bool EnumFunctions::IsPlayerTypeEligible(AffectPlayerType flags, HouseClass* pFor)
{
	if (!pFor)
		return true;

	if (flags == AffectPlayerType::None)
		return false;

	const bool Obs = (flags & AffectPlayerType::Observer) != AffectPlayerType::None;
	const bool Comp = (flags & AffectPlayerType::Computer) != AffectPlayerType::None;
	const bool Player = (flags & AffectPlayerType::Player) != AffectPlayerType::None;

	if (Obs && Comp && Player)
		return true;

	if (Obs && !pFor->IsObserver())
		return false;

	const bool IsHumanControlled = pFor->IsControlledByHuman();

	if (Comp && IsHumanControlled)
		return false;

	if (Player && !IsHumanControlled)
		return false;

	return true;
}
