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

	//simple enums
	static COMPILETIMEEVAL OPTIONALINLINE auto PassiveAcquireMode_ToStrings = magic_enum::enum_entries<PassiveAcquireMode>();
	static COMPILETIMEEVAL OPTIONALINLINE auto AffectedTechno_ToStrings = magic_enum::enum_entries<AffectedTechno>();
	static COMPILETIMEEVAL OPTIONALINLINE auto AttachedAnimPosition_ToStrings = magic_enum::enum_entries<AttachedAnimPosition>();
	static COMPILETIMEEVAL OPTIONALINLINE auto LaserTrailDrawType_ToStrings = magic_enum::enum_entries<LaserTrailDrawType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto OwnerHouseKind_ToStrings = magic_enum::enum_entries<OwnerHouseKind>();
	static COMPILETIMEEVAL OPTIONALINLINE auto AttachmentYSortPosition_ToStrings = magic_enum::enum_entries<AttachmentYSortPosition>();
	static COMPILETIMEEVAL OPTIONALINLINE auto InterpolationMode_ToStrings = magic_enum::enum_entries<InterpolationMode>();
	static COMPILETIMEEVAL OPTIONALINLINE auto SpotlightAttachment_ToStrings = magic_enum::enum_entries<SpotlightAttachment>();
	static COMPILETIMEEVAL OPTIONALINLINE auto ShowTimerType_ToStrings = magic_enum::enum_entries<ShowTimerType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto BountyValueOption_ToStrings = magic_enum::enum_entries<BountyValueOption>();
	static COMPILETIMEEVAL OPTIONALINLINE auto BuildingSelectBracketPosition_ToStrings = magic_enum::enum_entries<BuildingSelectBracketPosition>();
	static COMPILETIMEEVAL OPTIONALINLINE auto DisplayInfoType_ToStrings = magic_enum::enum_entries<DisplayInfoType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto FullMapDetonateResult_ToStrings = magic_enum::enum_entries<FullMapDetonateResult>();
	static COMPILETIMEEVAL OPTIONALINLINE auto Rank_ToStrings = magic_enum::enum_entries<Rank>();
	static COMPILETIMEEVAL OPTIONALINLINE auto TargetZoneScanType_ToStrings = magic_enum::enum_entries<TargetZoneScanType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto DefaultGameColor_ToStrings = magic_enum::enum_entries<DefaultColorList>();
	static COMPILETIMEEVAL OPTIONALINLINE auto DamageDelayTargetFlag_ToStrings = magic_enum::enum_entries<DamageDelayTargetFlag>();
	static COMPILETIMEEVAL OPTIONALINLINE auto SelfHealGainType_ToStrings = magic_enum::enum_entries<SelfHealGainType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto TargetingPreference_ToStrings = magic_enum::enum_entries<TargetingPreference>();
	static COMPILETIMEEVAL OPTIONALINLINE auto BannerNumberType_ToStrings = magic_enum::enum_entries<BannerNumberType>();
	static COMPILETIMEEVAL OPTIONALINLINE auto VerticalPosition_ToStrings = magic_enum::enum_entries<VerticalPosition>();
	static COMPILETIMEEVAL OPTIONALINLINE auto SlaveReturnTo_ToStrings = magic_enum::enum_entries<SlaveReturnTo>();
	static COMPILETIMEEVAL OPTIONALINLINE auto KillMethod_ToStrings = magic_enum::enum_entries<KillMethod>();
	static COMPILETIMEEVAL OPTIONALINLINE auto IronCurtainFlag_ToStrings = magic_enum::enum_entries<IronCurtainFlag>();
	static COMPILETIMEEVAL OPTIONALINLINE auto SuperWeaponAITargetingMode_ToStrings = magic_enum::enum_entries<SuperWeaponAITargetingMode>();
	static COMPILETIMEEVAL OPTIONALINLINE auto AreaFireTarget_ToStrings = magic_enum::enum_entries<AreaFireTarget>();
	static COMPILETIMEEVAL OPTIONALINLINE auto HorizontalPosition_ToStrings = magic_enum::enum_entries<HorizontalPosition>();
	static COMPILETIMEEVAL OPTIONALINLINE auto HealthCondition_ToStrings = magic_enum::enum_entries<HealthCondition>();

	static std::array<std::pair<const char*, const char*>, 11u> LocomotorPairs_ToStrings;
	static std::array<std::pair<const wchar_t*, const wchar_t*>, 11u> LocomotorPairs_ToWideStrings;
	
	static std::array<const char*, (size_t)PhobosAbilityType::count> PhobosAbilityType_ToStrings;
	
	static std::array<const char*, 6u>  DisplayShowType_ToStrings;
	static std::array<const char*, 6u> MouseCursorData_ToStrings;
	static std::array<const char*, 21u> TileType_ToStrings;
	static std::array<const char*, 6u> DamageState_to_strings;
	static std::array<const char*, 8u> FacingType_to_strings;
	static std::array<const char*, 6u> ExpireWeaponCondition_to_strings;

	static std::array<std::pair<const char*, AffectedVeterancy>, 5u> AffectedVeterancy_ToStrings;
	static std::array<std::pair<const char*, TextAlign>, 4u> TextAlign_ToStrings;
	static std::array<std::pair<const char*, AttachedAnimFlag>, 5u> AttachedAnimFlag_ToStrings;
	static std::array<std::pair<const char*, AffectedHouse>, 11u> AffectedHouse_ToStrings;
	static std::array<std::pair<const char*, AffectedTarget>, 15u> AffectedTarget_ToStrings;
	static std::array<std::pair<const char*, SpotlightFlags>, 5u> SpotlightFlags_ToStrings;
	static std::array<std::pair<const char*, ChronoSparkleDisplayPosition>, 5u> ChronoSparkleDisplayPosition_ToStrings;
	static std::array<std::pair<const char* , TargetingConstraints> , 10u> TargetingConstraints_ToStrings;
	static std::array<std::pair<const char*, AffectPlayerType>, 5u> AffectPlayerType_ToStrings;
	static std::array<std::pair<const char*, MouseHotSpotX>, 3u> MouseHotSpotX_ToStrings;
	static std::array<std::pair<const char*, MouseHotSpotY>, 3u> MouseHotSpotY_ToStrings;
	static std::array<const char*, (size_t)NewCrateType::count> NewCrateType_ToStrings;
	static std::array<const char*, (size_t)TrajectoryCheckReturnType::count> TrajectoryCheckReturnType_to_strings;
	static std::array<const char*, (size_t)DiscardCondition::count> DiscardCondition_to_strings;


public:

	static bool CanTargetVeterancy(AffectedVeterancy flags, TechnoClass* pTechno);
	static bool CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells = false, bool considerBridgesLand = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately = false);
	static bool IsTechnoEligibleB(TechnoClass* const pTechno, AffectedTarget allowed);
	static bool CanAffectTechnoResult(AbstractType type, AffectedTechno allowed);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false, bool allowBridges = false);
	static BlitterFlags GetTranslucentLevel(int nInt);
	static TextPrintType CastAlignToFlags(HorizontalPosition pos);
	static IronCurtainFlag GetICFlagResult(IronCurtainFlag Input);
	static std::pair<const char*, const char*>* locomotion_toSring(LocomotionClass* ptr);
	static bool IsPlayerTypeEligible(AffectPlayerType flags, HouseClass* pFor);

	COMPILETIMEEVAL OPTIONALINLINE bool IsLandTypeInFlags(LandTypeFlags flags, LandType type) {
		return (bool)((LandTypeFlags)(1 << (char)type) & flags);
	}
};

class MouseCursorHotSpotX
{
public:
	static OPTIONALINLINE bool Parse(const char* key, MouseHotSpotX* value)
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
	static OPTIONALINLINE bool Parse(const char* key, MouseHotSpotY* value)
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
