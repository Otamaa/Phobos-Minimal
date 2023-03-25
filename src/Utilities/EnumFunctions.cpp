#include "EnumFunctions.h"

#include <ScenarioClass.h>

std::array<const char*, (size_t)FullMapDetonateResult::count> EnumFunctions::FullMapDetonateResult_ToStrings {
 {
	{ "TargetNotDamageable" } , { "TargetNotEligible" } ,
	{ "TargetHouseNotEligible" } , { "TargetRestricted" } ,
	{ "TargetValid" }
 }
};

std::array<const char*, (size_t)PhobosAbilityType::count> EnumFunctions::PhobosAbilityType_ToStrings {
 {
	{ "INTERCEPTOR" } , { "CHRONODELAYIMMUNE" } , { "CRITIMMUNE" } ,
	{ "PSIONICSIMMUNE" } , { "CULLINGIMMUNE" } , { "EMPIMMUNE" } ,
	{ "RADIMMUNE" } , { "PROTECTED_DRIVER" } , { "UNWARPABLE" } , 
	{ "POISONIMMUNE" } , { "PSIONICSWEAPONIMMUNE" } , { "BERZERKIMMUNE" }
 }
};

std::array<const char* const, 3u> EnumFunctions::Rank_ToStrings {
 {
	{ "Elite" } , { "Veteran" } , { "Rookie" }
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
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				if (!considerAircraftSeparately)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case AbstractType::Building:
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
			case AbstractType::Infantry:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case AbstractType::Unit:
			{
				if(!pTechno->GetTechnoType()->ConsideredAircraft)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;

				return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			}
			case AbstractType::Aircraft:
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case AbstractType::Building:
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
