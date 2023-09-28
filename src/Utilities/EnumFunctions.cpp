#include "EnumFunctions.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <CellClass.h>

std::array<const char* const, (size_t)FullMapDetonateResult::count> EnumFunctions::FullMapDetonateResult_ToStrings {
 {
	{ "TargetNotDamageable" } ,
	{ "TargetNotEligible" } ,
	{ "TargetHouseNotEligible" } ,
	{ "TargetRestricted" } ,
	{ "TargetValid" }
 }
};

std::array<const char* const, (size_t)PhobosAbilityType::count> EnumFunctions::PhobosAbilityType_ToStrings {
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
	{ "WEBIMMUNE" }
 }
};

std::array<const char* const, 3u> EnumFunctions::Rank_ToStrings {
 {
	{ "Elite" } ,
	{ "Veteran" } ,
	{ "Rookie" }
 }
};

std::array<const char* const, (size_t)TargetZoneScanType::count> EnumFunctions::TargetZoneScanType_ToStrings
{
{
	{ "same" } ,
	{ "any" } ,
	{ "inrange" }
}
};

std::array<const char* const, (size_t)DefaultColorList::count> EnumFunctions::DefaultGameColor_ToStrings
{
{
	{ "Grey" } ,
	{ "Red" } ,
	{ "Green" } ,
	{ "Blue" } ,
	{ "Yellow" } ,
	{ "White" }
}
};

std::array<const char* const, (size_t)DamageDelayTargetFlag::count> EnumFunctions::DamageDelayTargetFlag_ToStrings
{
{
	{ "Cell" } ,
	{ "AttachedObject" } ,
	{ "Invoker" }
}
};

std::array<const char* const, 6u> EnumFunctions::MouseCursorData_ToStrings
{
{
	{ "%s.Frame" } ,
	{ "%s.Count" } ,
	{ "%s.Interval" } ,
	{ "%s.MiniFrame" } ,
	{ "%s.MiniCount" } ,
	{ "%s.MiniInterval" }
}
};

std::array<const char* const, 3u> EnumFunctions::HealthCondition_ToStrings
{
{
	{ "ConditionGreen" } ,
	{ "ConditionYellow" } ,
	{ "ConditionRed" }
}
};

std::array<const char* const, 21u> EnumFunctions::TileType_ToStrings
{
{
	{"Unknown"} ,
	{"Tunnel"} ,
	{"Water"} ,
	{"Ramp"} ,
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

std::array<std::pair<wchar_t*, wchar_t*>, 11u> EnumFunctions::LocomotorPairs_ToWideStrings
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

std::array<const char* const, 3u> EnumFunctions::VerticalPosition_ToStrings
{
{
	{ "top" },
	{ "center" },
	{ "bottom" }
}
};

std::array<std::pair<const char* const, BannerNumberType>, 5u> EnumFunctions::BannerNumberType_ToStrings
{
{
	{"none" , BannerNumberType::None} ,
	{"variable" , BannerNumberType::Variable } ,
	{"prefixed" , BannerNumberType::Prefixed } ,
	{"suffixed" , BannerNumberType::Suffixed } ,
	{"fraction" , BannerNumberType::Fraction }
}
};

std::array<std::pair<const char* const, SpotlightFlags>, 5u> EnumFunctions::SpotlightFlags_ToStrings
{
{
	{"none" , SpotlightFlags::None},
	{ "NoColor" , SpotlightFlags::NoColor },
	{ "NoRed" , SpotlightFlags::NoRed },
	{ "NoGreen" , SpotlightFlags::NoGreen },
	{ "NoBlue" , SpotlightFlags::NoBlue }
}
};

std::array<std::pair<const char* const, HorizontalPosition>, 4u> EnumFunctions::HorizontalPosition_ToStrings
{
{
	{ "left", HorizontalPosition::Left },
	{ "center", HorizontalPosition::Center },
	{ "right", HorizontalPosition::Right }
}
};

std::array<std::pair<const char* const, TextAlign>, 4u> EnumFunctions::TextAlign_ToStrings
{
{
	{"left" , TextAlign::Left} ,
	{"center", TextAlign::Center } ,
	{"right", TextAlign::Right }
}
};

std::array<const char* const, 3u> EnumFunctions::AreaFireTarget_ToStrings
{
{
	{"base"} ,
	{"self"} ,
	{"random"}
}
};

std::array<std::pair<const char* const, AttachedAnimFlag>, 5u>  EnumFunctions::AttachedAnimFlag_ToStrings
{
{
	{"none" , AttachedAnimFlag::None} ,
	{"hides" , AttachedAnimFlag::Hides } ,
	{"temporal" , AttachedAnimFlag::Temporal } ,
	{"paused" , AttachedAnimFlag::Paused },
	{"pausedtemporal" , AttachedAnimFlag::PausedTemporal }
}
};

std::array<std::pair<const char* const, AffectedHouse>, 11u> EnumFunctions::AffectedHouse_ToStrings
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

std::array<std::pair<const char* const, AffectedTarget>, 15u> EnumFunctions::AffectedTarget_ToStrings
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

std::array<std::pair<const char* const, SuperWeaponAITargetingMode>, 23u> EnumFunctions::SuperWeaponAITargetingMode_ToStrings
{
{
	{"none" , SuperWeaponAITargetingMode::None} ,
	{"nuke", SuperWeaponAITargetingMode::Nuke },
	{"lightningstorm", SuperWeaponAITargetingMode::LightningStorm },
	{"psychicdominator", SuperWeaponAITargetingMode::PsychicDominator },
	{"paradrop", SuperWeaponAITargetingMode::ParaDrop },
	{"geneticmutator", SuperWeaponAITargetingMode::GeneticMutator },
	{"forceshield", SuperWeaponAITargetingMode::ForceShield },
	{"notarget", SuperWeaponAITargetingMode::NoTarget },
	{"offensive", SuperWeaponAITargetingMode::Offensive },
	{"stealth", SuperWeaponAITargetingMode::Stealth },
	{"self", SuperWeaponAITargetingMode::Self },
	{"base", SuperWeaponAITargetingMode::Base },
	{"multimissile", SuperWeaponAITargetingMode::MultiMissile },
	{"hunterseeker", SuperWeaponAITargetingMode::HunterSeeker },
	{"enemybase", SuperWeaponAITargetingMode::EnemyBase } ,
	{"ironcurtain", SuperWeaponAITargetingMode::IronCurtain },
	{"attack", SuperWeaponAITargetingMode::Attack } ,
	{"lowpower", SuperWeaponAITargetingMode::LowPower },
	{"lowpowerattack", SuperWeaponAITargetingMode::LowPowerAttack } ,
	{"droppod", SuperWeaponAITargetingMode::DropPod } ,
	{"lightningrandom", SuperWeaponAITargetingMode::LightningRandom } ,
	{"launchsite", SuperWeaponAITargetingMode::LauchSite },
	{"findauxtechno", SuperWeaponAITargetingMode::FindAuxTechno }
}
};

std::array<const char* const, 8u> EnumFunctions::OwnerHouseKind_ToStrings
{
{
	{"default"} ,
	{"invoker"} ,
	{"killer"} ,
	{"victim"} ,
	{"civilian"} ,
	{"special"} ,
	{"neutral"} ,
	{"random"}
}
};

std::array<std::pair<const char* const, IronCurtainFlag>, 6u>  EnumFunctions::IronCurtainFlag_ToStrings
{
{
	{"none", IronCurtainFlag::Default},
	{"Default" , IronCurtainFlag::Default} ,
	{"Kill" , IronCurtainFlag::Kill } ,
	{"Invulnerable" , IronCurtainFlag::Invulnerable } ,
	{"Ignore" , IronCurtainFlag::Ignore } ,
	{"random" , IronCurtainFlag::Random }
}
};

std::array<std::pair<const char* const, KillMethod>, 6u> EnumFunctions::KillMethod_ToStrings
{
{
	{"none" , KillMethod::None} ,
	{"explode" , KillMethod::Explode } ,
	{"kill" , KillMethod::Explode } ,
	{"vanish" , KillMethod::Vanish } ,
	{"sell" , KillMethod::Sell } ,
	{"random" , KillMethod::Random }
}
};

std::array<std::pair<const char* const, SlaveReturnTo>, 9u>  EnumFunctions::SlaveReturnTo_ToStrings
{
{
	{"killer" , SlaveReturnTo::Killer} ,
	{"master", SlaveReturnTo::Master } ,
	{"suicide", SlaveReturnTo::Suicide } ,
	{"explode", SlaveReturnTo::Suicide } ,
	{"kill", SlaveReturnTo::Suicide } ,
	{"neutral", SlaveReturnTo::Neutral } ,
	{"civilian", SlaveReturnTo::Civilian } ,
	{"special", SlaveReturnTo::Special } ,
	{"random", SlaveReturnTo::Random }
}
};

std::array<std::pair<const char* const, TargetingConstraints>, 10u> EnumFunctions::TargetingConstraints_ToStrings
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

std::array<const char* const, 3u> EnumFunctions::TargetingPreference_ToStrings
{
{
	{"none"} ,
	{"defensive"} ,
	{"offensive"}
}
};

std::array<const char* const, 3u> EnumFunctions::SelfHealGainType_ToStrings
{
{
	{ "none" },
	{ "Infantry" },
	{ "Units" }
}
};

std::array<std::pair<const char* const, ChronoSparkleDisplayPosition>, 5u> EnumFunctions::ChronoSparkleDisplayPosition_ToStrings
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

std::array<const char*, 3u> EnumFunctions::SpotlightAttachment_ToStrings
{
{
	{ "Body" },
	{ "Turret" },
	{ "Barrel" }
}
};

std::array<const char*, 3u> EnumFunctions::ShowTimerType_ToStrings
{
{
	{ "Hour" },
	{ "Minute" },
	{ "Second" }
}
};

std::array<const char*, (size_t)BountyValueOption::count> EnumFunctions::BountyValueOption_ToStrings
{
{
	{ "value" },
	{ "cost" },
	{ "soylent" }
}
};

std::array<const char*, (size_t)BuildingSelectBracketPosition::count> EnumFunctions::BuildingSelectBracketPosition_ToStrings
{
{
	{ "top" },
	{ "lefttop" },
	{ "leftbottom" },
	{ "bottom" },
	{ "rightbottom" },
	{ "righttop" },
}
};

std::array<const char*, (size_t)DisplayInfoType::count> EnumFunctions::DisplayInfoType_ToStrings
{
{	{ "health" },
	{ "shield" },
	{ "ammo" },
	{ "mindcontrol" },
	{ "spawns" },
	{ "passengers" },
	{ "tiberium" },
	{ "experience" },
	{ "occupants" },
	{ "gattlingstage" },
	{ "ironcurtain" } ,
	{ "disableweapon" },
	{ "cloakdisable" }
}
};

std::array<const char*, (size_t)NewCrateType::count> EnumFunctions::NewCrateType_ToStrings
{{
	{ "Money" }, { "Super" }, { "Weapon" }, { "Units" }
 }};

std::array<const char*, 6u> EnumFunctions::DamageState_to_srings
{
{
	"Unaffected", "Unchanged", "NowYellow", "NowRed", "NowDead", "PostMortem"
}
};

std::array<const char*, 8u> EnumFunctions::FacingType_to_strings
{
{
	"N" , "NE", "E" , "SE", "S", "SW", "W", "NW",
}
};

bool EnumFunctions::CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if (flags == AffectedHouse::All)
		return true;

	if (ownerHouse && targetHouse) {

		if ((flags & AffectedHouse::Owner) && ownerHouse == targetHouse)
			return true;

		const auto IsAlly = ownerHouse->IsAlliedWith_(targetHouse);
		return (flags & AffectedHouse::Allies) && ownerHouse != targetHouse && IsAlly ||
			   (flags & AffectedHouse::Enemies) && ownerHouse != targetHouse && !IsAlly;
	}

	return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget const& allowed, bool explicitEmptyCells, bool considerBridgesLand)
{
	if (explicitEmptyCells)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pCell->GetContent());

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
				if(!pTechno->GetTechnoType()->ConsideredAircraft)
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

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse const&  allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately, bool allowBridges)
{
	if (!pCell)
		return false;

	auto object = pCell->FirstObject;
	bool eligible = EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells , allowBridges);

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

	const bool IsHumanControlled = pFor->IsControlledByHuman_();

	if (Comp && IsHumanControlled)
		return false;

	if (Player && !IsHumanControlled)
		return false;

	return true;
}
