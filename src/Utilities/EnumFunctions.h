#pragma once

#include <Phobos.Defines.h>

#include "Enum.h"
#include "Interpolation.h"

#include <MouseClass.h>

class TechnoClass;
class HouseClass;
class CellClass;
class LocomotionClass;
class EnumFunctions final
{
	NO_CONSTRUCT_CLASS(EnumFunctions)
public:

	static constexpr auto AttachedAnimPosition_ToStrings = magic_enum::enum_entries<AttachedAnimPosition>();
	static constexpr auto LaserTrailDrawType_ToStrings = magic_enum::enum_entries<LaserTrailDrawType>();
	static constexpr auto OwnerHouseKind_ToStrings = magic_enum::enum_entries<OwnerHouseKind>();
	static std::array<const char* const, (size_t)FullMapDetonateResult::count> FullMapDetonateResult_ToStrings;
	static std::array<const char* const, (size_t)PhobosAbilityType::count> PhobosAbilityType_ToStrings;
	static std::array<const char* const, 3u> Rank_ToStrings;
	static std::array<const char* const, (size_t)TargetZoneScanType::count> TargetZoneScanType_ToStrings;
	static std::array<const char* const, (size_t)DefaultColorList::count> DefaultGameColor_ToStrings;
	static std::array<const char* const, (size_t)DamageDelayTargetFlag::count> DamageDelayTargetFlag_ToStrings;
	static std::array<const char* const, 6u> MouseCursorData_ToStrings;
	static std::array<const char* const, 3u> HealthCondition_ToStrings;
	static std::array<const char* const, 21u> TileType_ToStrings;
	static std::array<std::pair<const char*, const char*>, 11u> LocomotorPairs_ToStrings;
	static std::array<std::pair<wchar_t*, wchar_t*>, 11u> LocomotorPairs_ToWideStrings;
	static std::array<std::pair<const char* const, HorizontalPosition>, 3u> HorizontalPosition_ToStrings;
	static std::array<std::pair<const char* const, TextAlign>, 4u> TextAlign_ToStrings;

	static std::array<const char* const, 3u> AreaFireTarget_ToStrings;
	static std::array<std::pair<const char* const, AttachedAnimFlag>, 5u> AttachedAnimFlag_ToStrings;
	static std::array<std::pair<const char* const, AffectedHouse>, 11u> AffectedHouse_ToStrings;
	static std::array<std::pair<const char* const, AffectedTarget>, 15u> AffectedTarget_ToStrings;
	static std::array<std::pair<const char* const, SuperWeaponAITargetingMode>, 24u> SuperWeaponAITargetingMode_ToStrings;
	static std::array<std::pair<const char* const, IronCurtainFlag>, 6u> IronCurtainFlag_ToStrings;
	static std::array<std::pair<const char* const, KillMethod>, 6u> KillMethod_ToStrings;
	static std::array<std::pair<const char* const, SlaveReturnTo>, 9u> SlaveReturnTo_ToStrings;
	static std::array<const char* const, 3u> VerticalPosition_ToStrings;
	static std::array<std::pair<const char* const, BannerNumberType>, 4u> BannerNumberType_ToStrings;
	static std::array<std::pair<const char* const , TargetingConstraints> , 10u> TargetingConstraints_ToStrings;
	static std::array<const char* const, 3u> TargetingPreference_ToStrings;
	static std::array<const char* const, 3u> SelfHealGainType_ToStrings;
	static std::array<std::pair<const char* const, ChronoSparkleDisplayPosition>, 5u> ChronoSparkleDisplayPosition_ToStrings;
	static std::array<std::pair<const char* const, SpotlightFlags>, 5u> SpotlightFlags_ToStrings;
	static std::array<std::pair<const char* ,AffectPlayerType>, 5u> AffectPlayerType_ToStrings;
	static std::array<const char*, 3u> SpotlightAttachment_ToStrings;
	static std::array<const char*, 3u> ShowTimerType_ToStrings;
	static std::array<const char*, (size_t)BountyValueOption::count> BountyValueOption_ToStrings;

	static std::array<const char*, (size_t)BuildingSelectBracketPosition::count> BuildingSelectBracketPosition_ToStrings;
	static std::array<const char*, (size_t)DisplayInfoType::count> DisplayInfoType_ToStrings;
	static std::array<const char*, (size_t)NewCrateType::count> NewCrateType_ToStrings;

	static std::array<const char*, 6u> DamageState_to_strings;
	static std::array<const char*, 8u> FacingType_to_strings;

	static std::array<const char*, (size_t)TrajectoryCheckReturnType::count> TrajectoryCheckReturnType_to_strings;

	static std::array<const char*, (size_t)DiscardCondition::count> DiscardCondition_to_strings;
	static std::array<const char*, 6u> ExpireWeaponCondition_to_strings;

	static std::array<std::pair<const char* const, MouseHotSpotX>, 3u> MouseHotSpotX_ToStrings;
	static std::array<std::pair<const char* const, MouseHotSpotY>, 3u> MouseHotSpotY_ToStrings;

	static std::array<std::pair<const char* const, InterpolationMode>, 2u> InterpolationMode_ToStrings;

	static bool CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget const&  allowed, bool explicitEmptyCells = false, bool considerBridgesLand = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget  const& allowed, bool considerAircraftSeparately = false);
	static bool IsTechnoEligibleB(TechnoClass* const pTechno, AffectedTarget const& allowed);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse  const& allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false, bool allowBridges = false);
	static BlitterFlags GetTranslucentLevel(int const& nInt);
	static TextPrintType CastAlignToFlags(HorizontalPosition const& pos);
	static IronCurtainFlag GetICFlagResult(IronCurtainFlag const& Input);
	static std::pair<const char*, const char*>* locomotion_toSring(LocomotionClass* ptr);
	static bool IsPlayerTypeEligible(AffectPlayerType flags, HouseClass* pFor);

	COMPILETIMEEVAL OPTIONALINLINE bool IsLandTypeInFlags(LandTypeFlags flags, LandType type)
	{
		return (bool)((LandTypeFlags)(1 << (char)type) & flags);
	}
};

class MouseCursorHotSpotX
{
public:
	static OPTIONALINLINE bool Parse(char* key, MouseHotSpotX* value)
	{
		if (key && value)
		{
			for (const auto& arr : EnumFunctions::MouseHotSpotX_ToStrings)
			{
				if (IS_SAME_STR_(key, arr.first))
				{
					*value = arr.second;
					return true;
				}
			}

			if (IS_SAME_STR_(key, "centre")) {
				*value = MouseHotSpotX::Center;
				return true;
			}
		}
		return false;
	}
};

class MouseCursorHotSpotY
{
public:
	static OPTIONALINLINE bool Parse(char* key, MouseHotSpotY* value)
	{
		if (key && value)
		{
			for (const auto& arr : EnumFunctions::MouseHotSpotY_ToStrings)
			{
				if (IS_SAME_STR_(key, arr.first))
				{
					*value = arr.second;
					return true;
				}
			}
		}

		return false;
	}
};
