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

	static std::array<const char*, (size_t)FullMapDetonateResult::count> FullMapDetonateResult_ToStrings;
	static std::array<const char*, (size_t)PhobosAbilityType::count> PhobosAbilityType_ToStrings;
	static std::array<const char* const, 3u> Rank_ToStrings;

	static bool CanTargetHouse(AffectedHouse const &flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget const&  allowed, bool explicitEmptyCells = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget  const& allowed, bool considerAircraftSeparately = false);
	static bool IsTechnoEligibleB(TechnoClass* const pTechno, AffectedTarget const& allowed);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget  const& allowed, AffectedHouse  const& allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false);
	static BlitterFlags GetTranslucentLevel(int const& nInt);
	static TextPrintType CastAlignToFlags(HorizontalPosition const& pos);
	static IronCurtainFlag GetICFlagResult(IronCurtainFlag const& Input);
};