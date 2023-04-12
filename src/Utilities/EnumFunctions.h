#pragma once

#include <HouseClass.h>
#include <CellClass.h>

#include "Enum.h"

class EnumFunctions final
{
	EnumFunctions() = delete;
	EnumFunctions(const EnumFunctions&) = delete;
	EnumFunctions(EnumFunctions&&) = delete;
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
	static std::array<const char* const, 12u> LandType_ToStrings;
	static std::array<std::pair<const char* const, const char* const>, 11u> LocomotorPairs_ToStrings;
	static std::array<const char* const, 5u> LayerType_ToStrings;
	static std::array<const char* const, 4u> TextAlign_ToStrings;
	static std::array<const char* const, 3u> AreaFireTarget_ToStrings;
	static std::array<const char* const, 5u> AttachedAnimFlag_ToStrings;
	static std::array<const char* const, 10u> AffectedHouse_ToStrings;
	static std::array<const char* const, 15u> AffectedTarget_ToStrings;
	static std::array<const char* const, 15u> SuperWeaponAITargetingMode_ToStrings;
	static std::array<const char* const, 8u> OwnerHouseKind_ToStrings;
	static std::array<const char* const, 5u> IronCurtainFlag_ToStrings;
	static std::array<const char* const, 6u> KillMethod_ToStrings;
	static std::array<const char* const, 9u> SlaveReturnTo_ToStrings;
	static std::array<const char* const, 3u> VerticalPosition_ToStrings;
	static std::array<const char* const, 5u> BannerNumberType_ToStrings;

	static bool CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget const&  allowed, bool explicitEmptyCells = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget  const& allowed, bool considerAircraftSeparately = false);
	static bool IsTechnoEligibleB(TechnoClass* const pTechno, AffectedTarget const& allowed);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse  const& allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false);
	static BlitterFlags GetTranslucentLevel(int const& nInt);
	static TextPrintType CastAlignToFlags(HorizontalPosition const& pos);
	static IronCurtainFlag GetICFlagResult(IronCurtainFlag const& Input);
};