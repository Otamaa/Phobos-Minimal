#include "EnumFunctions.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <CellClass.h>

std::array<const char* const, (size_t)FullMapDetonateResult::count> EnumFunctions::FullMapDetonateResult_ToStrings {
 {
	{ "TargetNotDamageable" } , { "TargetNotEligible" } ,
	{ "TargetHouseNotEligible" } , { "TargetRestricted" } ,
	{ "TargetValid" }
 }
};

std::array<const char* const, (size_t)PhobosAbilityType::count> EnumFunctions::PhobosAbilityType_ToStrings {
 {
	{ "INTERCEPTOR" } , { "CHRONODELAYIMMUNE" } , { "CRITIMMUNE" } ,
	{ "PSIONICSIMMUNE" } , { "CULLINGIMMUNE" } , { "EMPIMMUNE" } ,
	{ "RADIMMUNE" } , { "PROTECTED_DRIVER" } , { "UNWARPABLE" } , 
	{ "POISONIMMUNE" } , { "PSIONICSWEAPONIMMUNE" } , { "BERZERKIMMUNE" } ,
	{ "ABDUCTORIMMUNE" }
 }
};

std::array<const char* const, 3u> EnumFunctions::Rank_ToStrings {
 {
	{ "Elite" } , { "Veteran" } , { "Rookie" }
 }
};

std::array<const char* const, (size_t)TargetZoneScanType::count> EnumFunctions::TargetZoneScanType_ToStrings
{
{
	{ "same" } , { "any" } , { "inrange" }
}
};

std::array<const char* const, (size_t)DefaultColorList::count> EnumFunctions::DefaultGameColor_ToStrings
{
{
	{ GameStrings::Grey() } , { GameStrings::Red() } , { GameStrings::Green() } ,
	{ "Blue" } , {  GameStrings::Yellow() } , { "White" }
}
};

std::array<const char* const, (size_t)DamageDelayTargetFlag::count> EnumFunctions::DamageDelayTargetFlag_ToStrings
{
{
	{ "Cell" } , { "AttachedObject" } , { "Invoker" }
}
};

std::array<const char* const, 6u> EnumFunctions::MouseCursorData_ToStrings
{
{
	{ "%s.Frame" } , { "%s.Count" } , { "%s.Interval" } , 
	{ "%s.MiniFrame" } , { "%s.MiniCount" } , { "%s.MiniInterval" }
}
};

std::array<const char* const, 3u> EnumFunctions::HealthCondition_ToStrings
{
{
	{ "ConditionGreen" } , { "ConditionYellow" } , { "ConditionRed" }
}
};

std::array<const char* const, 21u> EnumFunctions::TileType_ToStrings
{
{
	{ "Unknown" } , { "Tunnel" } , { "Water" } , 
	{ "Ramp" } , {"Blank"} , {"Shore"} ,
	{"Wet"} , {"MiscPave"} , {"Pave"} ,
	{"DirtRoad"} , {"PavedRoad"} , {"PavedRoadEnd"} ,
	{"PavedRoadSlope"} , {"Median"} , {"Bridge"} , 
	{"WoodBridge"} , {"ClearToSandLAT"} , {"Green"} ,
	{"NotWater"} , {"DestroyableCliff"}
}
};

std::array<std::pair<const char* const, const char* const>, 11u> EnumFunctions::LocomotorPairs_ToStrings
{
{
	{"Drive","{4A582741-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Jumpjet","{92612C46-F71F-11d1-AC9F-006008055BB5}"} ,
	{"Hover","{4A582742-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Rocket","{B7B49766-E576-11d3-9BD9-00104B972FE8}"} ,
	{"Tunnel","{4A582743-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Walk","{4A582744-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Droppod","{4A582745-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Fly","{4A582746-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Teleport","{4A582747-9839-11d1-B709-00A024DDAFD1}"} ,
	{"Mech","{55D141B8-DB94-11d1-AC98-006008055BB5}"} ,
	{"Ship","{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}"}
}
};

std::array<const char* const, 3u> EnumFunctions::VerticalPosition_ToStrings
{
{
	{ "top" }, { "center" }, { "bottom" }
}
};

std::array<const char* const, 5u> EnumFunctions::BannerNumberType_ToStrings
{
{
	{NONE_STR2} , {"variable"} , {"prefixed"} , {"suffixed"} , {"fraction"}
}
};

std::array<const char* const, 4u> EnumFunctions::TextAlign_ToStrings
{
{
	{NONE_STR2} , {"left"} , {"center"} , {"right"}
}
};

std::array<const char* const, 3u> EnumFunctions::AreaFireTarget_ToStrings
{
{ 	
	{"base"} , {"self"} , {"random"}
}
};

std::array<const char* const, 5u>  EnumFunctions::AttachedAnimFlag_ToStrings
{
{
	{NONE_STR2} , {"hides"} , {"temporal"} , {"paused"}, {"pausedtemporal"}
}
};

std::array<const char* const, 11u> EnumFunctions::AffectedHouse_ToStrings
{
{
	{NONE_STR2} , {"owner"} , {"self"} , {"allies"} ,
	{"ally"} , {"enemies"} , {"enemy"} , {"team"} ,
	{"others"} , {"all"} , {"trump"}
}
};

std::array<const char* const, 15u> EnumFunctions::AffectedTarget_ToStrings
{
{
	{NONE_STR2} , {"Land"} , {"Water"} , {"NoContent"} ,
	{"Infantry"} ,	{"Unit"} , {"Units"} , {"Building"} ,
	{"Buildings"} , {"Aircraft"} , {"All"} , {"AllCells"} , 
	{"AllTechnos"} , {"AllContents"} ,  {"Empty"}
}
};

std::array<const char* const, 21u> EnumFunctions::SuperWeaponAITargetingMode_ToStrings
{
{
	{NONE_STR2} , {"nuke"}, {"lightningstorm"}, {"psychicdominator"}, {"paradrop"},
	{"geneticmutator"}, {"forceshield"}, {"notarget"}, {"offensive"}, {"stealth"},
	{"self"}, {"base"}, {"multimissile"}, {"hunterseeker"}, {"enemybase"} , {"ironcurtain"}
	, {"attack"} ,{"lowpower"}, {"lowpowerattack"} , {"droppod"} , {"lightningrandom"}
}
};

std::array<const char* const, 8u> EnumFunctions::OwnerHouseKind_ToStrings
{
{
	{"default"} ,  {"invoker"} , {"killer"} , {"victim"} , {GameStrings::Civilian()} ,
	{GameStrings::Special()} ,  {GameStrings::Neutral()} ,  {"random"}
}
};

std::array<const char* const, 5u>  EnumFunctions::IronCurtainFlag_ToStrings
{
{
	{"Default"} , {"Kill"} , {"Invulnerable"} , {"Ignore"} , {"Random"}
}
};

std::array<const char* const, 6u> EnumFunctions::KillMethod_ToStrings
{
{
	{NONE_STR2} , {"explode"} , {"kill"} , {"vanish"} , {"sell"} , {"random"}
}
};

std::array<const char* const, 9u>  EnumFunctions::SlaveReturnTo_ToStrings
{
{
	{"killer"} ,  {"master"} , {"suicide"} , {"explode"} , 
	{"kill"} , {GameStrings::Neutral()} , {GameStrings::Civilian()} , 
	{GameStrings::Special()} , {"random"}
}
};

std::array<const char* const, 9u> EnumFunctions::TargetingConstraint_ToStrings
{
{
	{NONE_STR2} , {"offensive_cell_clear"} , {"defensive_cell_clear"} ,
	{"enemy"} , {"lightningstorm_inactive"} , {"dominator_inactive"} ,
	{ "attacked" } , {"offensive_cell_set"} , {"defensive_cell_set"}
}
};

std::array<const char* const, 3u> TargetingPreference_ToStrings
{
{
	{NONE_STR2} , {"defensive"} , {"offensive"}
}
};

bool EnumFunctions::CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if ((flags & AffectedHouse::All) != AffectedHouse::None)
		return true;

	if (ownerHouse && targetHouse) {
		return (flags & AffectedHouse::Owner) && ownerHouse == targetHouse ||
			   (flags & AffectedHouse::Allies) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
			   (flags & AffectedHouse::Enemies) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
	}

	return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget const& allowed, bool explicitEmptyCells)
{
	if (explicitEmptyCells)
	{
		const auto pTechno = pCell->FirstObject ? abstract_cast<TechnoClass*>(pCell->FirstObject) : nullptr;

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	if (allowed & AffectedTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water && !pCell->ContainsBridge()) // check whether it supports water
			return (allowed & AffectedTarget::Water) != AffectedTarget::None;
		else                                    // check whether it supports non-water
			return (allowed & AffectedTarget::Land) != AffectedTarget::None;
	}

	return allowed != AffectedTarget::None;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget  const& allowed, bool considerAircraftSeparately)
{
	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (GetVtableAddr(pTechno))
			{
			case InfantryClass::vtable:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case UnitClass::vtable:
			case AircraftClass::vtable:
				if (!considerAircraftSeparately)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case BuildingClass::vtable:
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
			switch (GetVtableAddr(pTechno))
			{
			case InfantryClass::vtable:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case UnitClass::vtable:
			{
				if(!pTechno->GetTechnoType()->ConsideredAircraft)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;

				return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			}
			case AircraftClass::vtable:
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case BuildingClass::vtable:
			{
				return ((allowed & AffectedTarget::Building) != AffectedTarget::None);
			}
			}
		}

		return true;
	}

	return false;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse const&  allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately)
{
	if (!pCell)
		return false;

	auto object = pCell->FirstObject;
	bool eligible = EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells);

	while (true)
	{
		if (!object || !eligible)
			break;

		if (auto pTechno = abstract_cast<TechnoClass*>(object))
		{
			if (owner)
			{
				eligible = EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner);

				if (!eligible)
					break;
			}

			eligible = EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately);
		}

		object = object->NextObject;
	}

	return eligible;
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
