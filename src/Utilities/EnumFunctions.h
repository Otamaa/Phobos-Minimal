#pragma once

#include <Phobos.h>

#include "Enum.h"

class MouseCursorHotSpotX
{
public:
	typedef MouseHotSpotX Value;

	static inline bool Parse(char* key, Value* value)
	{
		if (key && value)
		{
			if (IS_SAME_STR_(key, "left"))
			{
				*value = MouseHotSpotX::Left;
			}
			else if (IS_SAME_STR_(key, "right"))
			{
				*value = MouseHotSpotX::Right;
			}
			else if (IS_SAME_STR_(key, "center"))
			{
				*value = MouseHotSpotX::Center;
			}
			else
			{
				return false;
			}
			return true;
		}
		return false;
	}
};

class MouseCursorHotSpotY
{
public:
	typedef MouseHotSpotY Value;

	static inline bool Parse(char* key, Value* value)
	{
		if (key && value)
		{
			if (IS_SAME_STR_(key, "top"))
			{
				*value = MouseHotSpotY::Top;
			}
			else if (IS_SAME_STR_(key, "bottom"))
			{
				*value = MouseHotSpotY::Bottom;
			}
			else if (IS_SAME_STR_(key, "middle"))
			{
				*value = MouseHotSpotY::Middle;
			}
			else
			{
				return false;
			}
			return true;
		}
		return false;
	}
};

class HouseClass;
class CellClass;
class LocomotionClass;
class EnumFunctions final
{
	NO_CONSTRUCT_CLASS(EnumFunctions)
public:

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
	static std::array<std::pair<const char* const, HorizontalPosition>, 4u> HorizontalPosition_ToStrings;
	static std::array<std::pair<const char* const, TextAlign>, 4u> TextAlign_ToStrings;

	static std::array<const char* const, 3u> AreaFireTarget_ToStrings;
	static std::array<std::pair<const char* const, AttachedAnimFlag>, 5u> AttachedAnimFlag_ToStrings;
	static std::array<std::pair<const char* const, AffectedHouse>, 11u> AffectedHouse_ToStrings;
	static std::array<std::pair<const char* const, AffectedTarget>, 15u> AffectedTarget_ToStrings;
	static std::array<std::pair<const char* const, SuperWeaponAITargetingMode>, 23u> SuperWeaponAITargetingMode_ToStrings;
	static std::array<const char* const, 8u> OwnerHouseKind_ToStrings;
	static std::array<std::pair<const char* const, IronCurtainFlag>, 6u> IronCurtainFlag_ToStrings;
	static std::array<std::pair<const char* const, KillMethod>, 6u> KillMethod_ToStrings;
	static std::array<std::pair<const char* const, SlaveReturnTo>, 9u> SlaveReturnTo_ToStrings;
	static std::array<const char* const, 3u> VerticalPosition_ToStrings;
	static std::array<std::pair<const char* const, BannerNumberType>, 5u> BannerNumberType_ToStrings;
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
};