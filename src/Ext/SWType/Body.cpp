#include "Body.h"

#include <SuperWeaponTypeClass.h>

#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <Misc/AresData.h>

#include <Notifications.h>

#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

#include "NewSuperWeaponType/NewSWType.h"
#include "NewSuperWeaponType/NuclearMissile.h"
#include "NewSuperWeaponType/LightningStorm.h"
#include "NewSuperWeaponType/Dominator.h"

//#include <ExtraHeaders/DiscreteSelectionClass_s.h>
//#include <ExtraHeaders/DiscreteDistributionClass_s.h>

#include <DiscreteSelectionClass.h>
#include <DiscreteDistributionClass.h>

bool SWTypeExt::Handled = false;
SuperClass* SWTypeExt::TempSuper = nullptr;
SuperClass* SWTypeExt::LauchData = nullptr;

//TODO re-evaluate these , since the default array seems not contains what the documentation table says,..
std::array<const AITargetingModeInfo, (size_t)SuperWeaponAITargetingMode::count> SWTypeExt::AITargetingModes =
{
{
	{SuperWeaponAITargetingMode::None, SuperWeaponTarget::None, AffectedHouse::None , TargetingConstraint::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Nuke, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies , TargetingConstraint::Enemy, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::LightningStorm, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraint::Enemy | TargetingConstraint::LighningStormInactive, TargetingPreference::Offensive},
	{SuperWeaponAITargetingMode::PsychicDominator, SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit, AffectedHouse::All, TargetingConstraint::Enemy | TargetingConstraint::DominatorInactive | TargetingConstraint::OffensiveCellClear , TargetingPreference::None},
	{SuperWeaponAITargetingMode::ParaDrop, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::None, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::GeneticMutator, SuperWeaponTarget::Infantry, AffectedHouse::All, TargetingConstraint::OffensiveCellClear , TargetingPreference::None},
	{SuperWeaponAITargetingMode::ForceShield, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::Enemy , TargetingPreference::Devensive},
	{SuperWeaponAITargetingMode::NoTarget, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Offensive, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraint::Enemy , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Stealth, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraint::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::Self, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Base, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::MultiMissile, SuperWeaponTarget::Building, AffectedHouse::None, TargetingConstraint::Enemy, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::HunterSeeker, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::Enemy, TargetingPreference::None },
	{SuperWeaponAITargetingMode::EnemyBase, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::Enemy, TargetingPreference::None },
	{SuperWeaponAITargetingMode::IronCurtain, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::Attack, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraint::Enemy, TargetingPreference::Offensive},
	{SuperWeaponAITargetingMode::LowPower, SuperWeaponTarget::None, AffectedHouse::Owner, TargetingConstraint::LowPower, TargetingPreference::None},
	{SuperWeaponAITargetingMode::LowPowerAttack, SuperWeaponTarget::Building, AffectedHouse::Owner, TargetingConstraint::Attacked | TargetingConstraint::LowPower, TargetingPreference::None },
	{SuperWeaponAITargetingMode::Droppod, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraint::Enemy , TargetingPreference::None},
	{SuperWeaponAITargetingMode::LighningRandom, SuperWeaponTarget::AllCells, AffectedHouse::All, TargetingConstraint::Enemy, TargetingPreference::None },
}
};

bool SWTypeExt::ExtData::IsTypeRedirected() const
{
	return this->HandledType > SuperWeaponType::Invalid;
}

bool SWTypeExt::ExtData::IsOriginalType() const
{
	return NewSWType::IsOriginalType(this->Get()->Type);
}

NewSWType* SWTypeExt::ExtData::GetNewSWType() const
{
	return NewSWType::GetNewSWType(this);
}

void SWTypeExt::ExtData::Initialize()
{
	this->Text_Ready = GameStrings::TXT_READY();
	this->Text_Hold = GameStrings::TXT_HOLD();
	this->Text_Charging = GameStrings::TXT_CHARGING();
	this->Text_Active = GameStrings::TXT_FIRESTORM_ON();
	this->Message_CannotFire = "MSG:CannotFire";

	this->EVA_InsufficientFunds = VoxClass::FindIndexById(GameStrings::EVA_InsufficientFunds);
	this->EVA_SelectTarget = VoxClass::FindIndexById(GameStrings::EVA_SelectTarget);

	if (auto pNewSWType = NewSWType::GetNewSWType(this))
	{
		this->Get()->Action = Action(AresNewActionType::SuperWeaponAllowed);
		pNewSWType->Initialize(this);
	}

	this->LastAction = this->Get()->Action;
}

Action NOINLINE SWTypeExt::ExtData::GetAction(SuperWeaponTypeClass* pSuper, CellStruct* pTarget)
{
	if ((AresNewActionType)pSuper->Action != AresNewActionType::SuperWeaponAllowed)
		return Action::None;

	const auto pExt = SWTypeExt::ExtMap.Find(pSuper);

	auto result = AresNewActionType::SuperWeaponAllowed;

	// prevent firing into shroud
	if (!pExt->SW_FireToShroud)
	{
		CellClass* pCell = MapClass::Instance->GetCellAt(pTarget);
		CoordStruct Crd = pCell->GetCoords();

		if (MapClass::Instance->IsLocationShrouded(Crd))
		{
			result = AresNewActionType::SuperWeaponDisallowed;
		}
	}

	if (result == AresNewActionType::SuperWeaponAllowed)
	{
		if (auto pNewType = NewSWType::GetNewSWType(pExt))
		{
			// new SW types have to check whether the coordinates are valid.
			if (!pNewType->CanFireAt(pExt, HouseClass::CurrentPlayer, *pTarget, true))
			{
				result = AresNewActionType::SuperWeaponDisallowed;
			}
		}
	}

	const bool bCanFire = result == AresNewActionType::SuperWeaponAllowed;
	int Cursor = -1;

	if (!bCanFire)
	{
		Ares_CurrentSWType = nullptr;
		Cursor = pExt->NoCursorType;
	}
	else
	{
		Ares_CurrentSWType = pSuper;
		Cursor = pExt->CursorType;
	}


	AresData::SetSWMouseCursorAction(Cursor, pExt->SW_FireToShroud, -1);
	return (Action)result;
}

SuperWeaponTarget SWTypeExt::ExtData::GetAIRequiredTarget() const
{
	if (this->SW_AIRequiresTarget.isset())
	{
		return this->SW_AIRequiresTarget;
	}

	auto index = static_cast<unsigned int>(this->SW_AITargetingMode.Get());

	if (index < AITargetingModes.size())
	{
		return AITargetingModes[index].Target;
	}

	return SuperWeaponTarget::None;
}

AffectedHouse SWTypeExt::ExtData::GetAIRequiredHouse() const
{
	if (this->SW_AIRequiresHouse.isset())
	{
		return this->SW_AIRequiresHouse;
	}

	auto index = static_cast<unsigned int>(this->SW_AITargetingMode.Get());

	if (index < AITargetingModes.size())
	{
		return AITargetingModes[index].House;
	}

	return AffectedHouse::None;
}

std::pair<TargetingConstraint, bool> SWTypeExt::ExtData::GetAITargetingConstraint() const
{
	if (this->SW_AITargetingConstrain.isset())
		return { this->SW_AITargetingConstrain.Get(), true };
	else
	{
		const auto index = static_cast<unsigned int>(this->SW_AITargetingMode.Get());

		if (index < AITargetingModes.size()) {
			return { AITargetingModes[(int)index].Constrain , true };
		}
	}

	return { TargetingConstraint::None , false };
}

TargetingPreference SWTypeExt::ExtData::GetAITargetingPreference() const
{
	auto nFlag = TargetingPreference::None;
	if (this->SW_AITargetingPreference.isset())
		nFlag = this->SW_AITargetingPreference.Get();
	else
	{
		auto index = static_cast<unsigned int>(this->SW_AITargetingMode.Get());
		if (index < AITargetingModes.size())
		{
			nFlag = AITargetingModes[(int)index].Preference;
		}
	}

	return nFlag;
}

bool SWTypeExt::ExtData::IsCellEligible(CellClass* pCell, SuperWeaponTarget allowed)
{
	if (allowed & SuperWeaponTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water && !pCell->ContainsBridge())
		{
			// check whether it supports water
			return (allowed & SuperWeaponTarget::Water) != SuperWeaponTarget::None;
		}
		else
		{
			// check whether it supports non-water
			return (allowed & SuperWeaponTarget::Land) != SuperWeaponTarget::None;
		}
	}
	return true;
}

bool SWTypeExt::ExtData::IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget allowed)
{
	if (allowed & SuperWeaponTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				return (allowed & SuperWeaponTarget::Infantry) != SuperWeaponTarget::None;
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return (allowed & SuperWeaponTarget::Unit) != SuperWeaponTarget::None;
			case AbstractType::Building:
				return (allowed & SuperWeaponTarget::Building) != SuperWeaponTarget::None;
			}
		}
		else
		{
			// is the target cell allowed to be empty?
			return (allowed & SuperWeaponTarget::Empty) != SuperWeaponTarget::None;
		}
	}
	return true;
}

bool SWTypeExt::ExtData::IsTechnoAffected(TechnoClass* pTechno)
{
	// check land and water cells
	if (!this->IsCellEligible(pTechno->GetCell(), this->SW_AffectsTarget))
	{
		return false;
	}

	// check for specific techno type
	if (!this->IsTechnoEligible(pTechno, this->SW_AffectsTarget))
	{
		return false;
	}

	// no restriction
	return true;
}

bool SWTypeExt::ExtData::CanFireAt(HouseClass* pOwner, const CellStruct& coords, bool manual)
{
	auto pCell = MapClass::Instance->GetCellAt(coords);

	// check cell type
	auto const AllowedTarget = !manual ? this->GetAIRequiredTarget() : this->SW_RequiresTarget;

	if (!IsCellEligible(pCell, AllowedTarget)) {
		return false;
	}

	// check for techno type match
	auto const pTechno = abstract_cast<TechnoClass*>(pCell->GetContent());
	auto const AllowedHouse = !manual ? this->GetAIRequiredHouse() : this->SW_RequiresHouse;
	if (pTechno && AllowedHouse != AffectedHouse::None)
	{
		if (!this->IsHouseAffected(pOwner, pTechno->Owner, AllowedHouse))
		{
			return false;
		}
	}

	if (!this->IsTechnoEligible(pTechno, AllowedTarget))
	{
		return false;
	}

	// no restriction
	return true;
}

bool SWTypeExt::ExtData::IsTargetConstraintEligible(SuperClass* pThis, bool IsPlayer)
{
	const auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);
	auto pOwner = pThis->Owner;
	auto const& [nFlag , IsDefault] = pExt->GetAITargetingConstraint();

	auto valid = [](const CellStruct& nVal) { return nVal.X || nVal.Y; };

	if (!IsDefault) {
		if (((nFlag & TargetingConstraint::OffensiveCellClear) != TargetingConstraint::None) && valid(pOwner->PreferredTargetCell))
			return false;
	}

	if (((nFlag & TargetingConstraint::OffensiveCellSet) != TargetingConstraint::None) && !valid(pOwner->PreferredTargetCell))
		return false;

	if (((nFlag & TargetingConstraint::DefensifeCellClear) != TargetingConstraint::None) && valid(pOwner->PreferredDefensiveCell2))
		return false;

	if (((nFlag & TargetingConstraint::DefensiveCellSet) != TargetingConstraint::None) && !valid(pOwner->PreferredDefensiveCell2))
		return false;

	if (((nFlag & TargetingConstraint::Enemy) != TargetingConstraint::None) && pOwner->EnemyHouseIndex < 0)
		return false;

	if (!IsPlayer)
	{
		if (((nFlag & TargetingConstraint::LighningStormInactive) != TargetingConstraint::None) && LightningStorm::Active())
			return false;

		if (((nFlag & TargetingConstraint::DominatorInactive) != TargetingConstraint::None) && PsyDom::Active())
			return false;

		if (((nFlag & TargetingConstraint::Attacked) != TargetingConstraint::None) && (pOwner->LATime && (pOwner->LATime + 75) < Unsorted::CurrentFrame))
			return false;

		if (((nFlag & TargetingConstraint::LowPower) != TargetingConstraint::None) && pOwner->HasFullPower())
			return false;
	}

	return true;
}

bool SWTypeExt::ExtData::TryFire(SuperClass* pThis, bool IsPlayer)
{
	const auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);

	// don't try to fire if we obviously haven't enough money
	if (pThis->Owner->CanTransactMoney(pExt->Money_Amount.Get()))
	{
		if (SWTypeExt::ExtData::IsTargetConstraintEligible(pThis, IsPlayer))
		{
			auto const& [Cell, Flag] = SWTypeExt::ExtData::PickSuperWeaponTarget(pThis);
			if (Flag != SWTargetFlags::DisallowEmpty)
			{
				return pThis->Owner->Fire_SW(pThis->Type->ArrayIndex, Cell);
			}
		}
	}

	return false;

}

struct TargetingInfo
{
	TargetingInfo(SuperClass* pSuper) :
		Super(pSuper),
		Owner(pSuper->Owner),
		TypeExt(SWTypeExt::ExtMap.Find(pSuper->Type)),
		NewType(NewSWType::GetNewSWType(SWTypeExt::ExtMap.Find(pSuper->Type)))
	{
	}

	bool CanFireAt(const CellStruct& cell, bool manual = false) const
	{
		return this->NewType->CanFireAt(*this->Data, cell, manual);
	}

	void GetData() const
	{
		this->Data = this->NewType->GetTargetingData(this->TypeExt, this->Owner);
	}

public:
	SuperClass* Super;
	HouseClass* Owner;
	SWTypeExt::ExtData* TypeExt;
	NewSWType* NewType;
	std::unique_ptr<const TargetingData> mutable Data;
};

struct TargetingFuncs
{
#pragma region Helpers
	template<typename It, typename Valuator>
	static ObjectClass* GetTargetFirstMax(It first, It last, Valuator value)
	{
		ObjectClass* pTarget = nullptr;
		int maxValue = 0;

		for (auto it = first; it < last; ++it)
		{
			if (auto pItem = *it)
			{
				auto curValue = value(pItem, maxValue);

				if (curValue > maxValue)
				{
					pTarget = pItem;
					maxValue = curValue;
				}
			}
		}

		return pTarget;
	}

	template<typename It, typename Valuator>
	static ObjectClass* GetTargetAnyMax(It first, It last, Valuator value)
	{
		DiscreteSelectionClass<ObjectClass*> targets;

		for (auto it = first; it < last; ++it)
		{
			if (auto pItem = *it)
			{
				auto curValue = value(pItem, targets.GetRating());
				targets.Add(pItem, curValue);
			}
		}

		return targets.Select(ScenarioClass::Instance->Random);
	}

	template<typename It, typename Valuator>
	static ObjectClass* GetTargetShareAny(It first, It last, Valuator value)
	{
		DiscreteDistributionClass<ObjectClass*> targets;

		for (auto it = first; it < last; ++it)
		{
			if (auto pItem = *it)
			{
				auto curValue = value(pItem);
				targets.Add(pItem, curValue);
			}
		}

		return targets.Select(ScenarioClass::Instance->Random);
	}

	static bool IsTargetAllowed(TechnoClass* pTechno)
	{
		return !pTechno->InLimbo;
	}

	static bool IgnoreThis(TechnoClass* pTechno)
	{
		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		if (pTechnoTypeExt->IsDummy)
			return true;

		if (auto pBld = specific_cast<BuildingClass*>(pTechno))
		{
			const auto pBldExt = BuildingExt::ExtMap.Find(pBld);

			if (pBldExt->LimboID != -1)
				return true;
		}

		return false;
	}

	static TargetResult NoTarget()
	{
		return { CellStruct::Empty , SWTargetFlags::AllowEmpty };
	}
#pragma endregion

#pragma region MainTargeting
	static TargetResult GetIonCannonTarget(const TargetingInfo& info, HouseClass* pEnemy, CloakHandling cloak)
	{
		const auto it = info.TypeExt->GetPotentialAITargets(pEnemy);

		const auto pResult = GetTargetAnyMax(it.begin(), it.end(),
			[=, &info](TechnoClass* pTechno, int curMax) {
				if(TargetingFuncs::IgnoreThis(pTechno))
					return -1;

				// original game code only compares owner and doesn't support nullptr
				auto const passedFilter = (!pEnemy || pTechno->Owner == pEnemy);

				if (passedFilter && info.Owner->IsIonCannonEligibleTarget(pTechno))
			{
				auto const cell = CellClass::Coord2Cell(pTechno->GetCoords());

				if (!MapClass::Instance->IsWithinUsableArea(cell, true)) { return -1; }

				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

				int value = !pTypeExt->AIIonCannonValue.HasValue() ?
					pTechno->GetIonCannonValue(info.Owner->AIDifficulty) :
					make_iterator(pTypeExt->AIIonCannonValue).at((int)info.Owner->AIDifficulty);

				// cloak options
				if (cloak != CloakHandling::AgnosticToCloak)
				{
					bool cloaked = pTechno->IsCloaked();

					if (cloak == CloakHandling::RandomizeCloaked)
					{
						// original behavior
						if (cloaked)
						{
							value = ScenarioClass::Instance->Random.RandomFromMax(curMax + 10);
						}
					}
					else if (cloak == CloakHandling::IgnoreCloaked)
					{
						// this prevents the 'targeting cloaked units bug'
						if (cloaked)
						{
							return -1;
						}
					}
					else if (cloak == CloakHandling::RequireCloaked)
					{
						if (!cloaked)
						{
							return -1;
						}
					}
				}

				// do not do heavy lifting on objects that
				// would not be chosen anyhow
				if (value >= curMax && info.CanFireAt(cell)) { return value; }
			}

				return -1;
			});


		if (pResult) {
			return { CellClass::Coord2Cell(pResult->GetCoords()) , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult PickByHouseType(HouseClass* pThis, TargetType type)
	{
		const auto nTarget = pThis->PickTargetByType(type);

		if (nTarget.IsValid()) {
			return { nTarget , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetDominatorTarget(const TargetingInfo& info)
	{
		auto it = info.TypeExt->GetPotentialAITargets();
		const auto pTarget = GetTargetFirstMax(it.begin(), it.end(), [&info](TechnoClass* pTechno, int curMax) {

		 if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
		 {
			return -1;
		 }

	  auto cell = pTechno->GetCell()->MapCoords;

	  int value = 0;
	  for (size_t i = 0; i < CellSpread::NumCells(3); ++i)
	  {
		  auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

		  for (NextObject j(pCell->FirstObject); j && abstract_cast<FootClass*>(*j); ++j)
		  {
			  auto pFoot = static_cast<FootClass*>(*j);
			  if (!info.Owner->IsAlliedWith_(pFoot) && !pFoot->IsInAir())
			  {
				  // original game does not consider cloak
				  if (pFoot->CanBePermaMindControlled() && (pFoot->CloakState != CloakState::Cloaked))
				  {
					  ++value;
				  }
			  }
		  }
	  }

	  // new check
	  if (value <= curMax || !info.CanFireAt(cell)) { return -1; }

	  return value;
		});

		if (pTarget)
		{
			return{ CellClass::Coord2Cell(pTarget->GetCoords())  , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetParadropTarget(const TargetingInfo& info)
	{
		auto pOwner = info.Owner;
		static const int SpaceSize = 5;

		auto target = CellStruct::Empty;
		if (pOwner->PreferredTargetType == TargetType::Anything)
		{
			// if no enemy yet, reinforce own base
			auto pTargetPlayer = HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex, info.Owner);

			target = MapClass::Instance->NearByLocation(
				pTargetPlayer->GetBaseCenter(), SpeedType::Foot, -1,
				MovementZone::Normal, false, SpaceSize, SpaceSize, false,
				false, false, true, CellStruct::Empty, false, false);

			if (target != CellStruct::Empty)
			{
				target += CellStruct{SpaceSize / 2, SpaceSize / 2};
			}

		}
		else
		{
			target = pOwner->PickTargetByType(pOwner->PreferredTargetType);
		}

		if (!target.IsValid() || !info.CanFireAt(target))
		{
			return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
		}

		return  { target , SWTargetFlags::AllowEmpty };
	}

	static TargetResult GetMutatorTarget(const TargetingInfo& info)
	{
		//specific implementation for GeneticMutatorTargetSelector for
		auto it = info.TypeExt->GetPotentialAITargets();
		const auto pResult = GetTargetFirstMax(it.begin(), it.end(), [&info](TechnoClass* pTechno, int curMax)
 {

	 if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
	 {
		 return -1;
	 }

	 auto cell = pTechno->GetCell()->MapCoords;
	 int value = 0;

	 for (size_t i = 0; i < CellSpread::NumCells(1); ++i)
	 {
		 auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

		 for (NextObject j(pCell->GetInfantry(pTechno->OnBridge)); j && abstract_cast<InfantryClass*>(*j); ++j)
		 {
			 auto pInf = static_cast<InfantryClass*>(*j);

			 if (!info.Owner->IsAlliedWith_(pInf) && !pInf->IsInAir())
			 {
				 // original game does not consider cloak
				 if (pInf->CloakState != CloakState::Cloaked)
				 {
					 ++value;
				 }
			 }
		 }

		 // new check
		 if (value <= curMax || !info.CanFireAt(cell))
		 {
			 return -1;
		 }
	 }

	 return value;
		});

		if (pResult)
		{
			return { CellClass::Coord2Cell(pResult->GetCoords()), SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetForceShieldTarget(const TargetingInfo& info)
	{
		const auto pOwner = info.Owner;

		if (pOwner->PreferredDefensiveCell.IsValid()
			&& (RulesClass::Instance->AISuperDefenseFrames + pOwner->PreferredDefensiveCellStartTime) > Unsorted::CurrentFrame
			&& info.CanFireAt(pOwner->PreferredDefensiveCell))
		{
			return { pOwner->PreferredDefensiveCell , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetOffensiveTarget(const TargetingInfo& info)
	{
		return GetIonCannonTarget(info, HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex), CloakHandling::IgnoreCloaked);
	}

	static TargetResult GetNukeAndLighningTarget(const TargetingInfo& info)
	{
		auto pOwner = info.Owner;

		if (pOwner->PreferredTargetType == TargetType::Anything) {
			return TargetingFuncs::GetIonCannonTarget(info,
				HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex),
				CloakHandling::IgnoreCloaked);
		}

		return TargetingFuncs::PickByHouseType(pOwner, pOwner->PreferredTargetType);
	}

	static TargetResult GetStealthTarget(const TargetingInfo& info)
	{
		return TargetingFuncs::GetIonCannonTarget(info, nullptr, CloakHandling::RequireCloaked);
	}

	static TargetResult GetDroppodTarget(const TargetingInfo& info)
	{
		auto nRandom = ScenarioClass::Instance->Random.RandomRanged(1, 4);
		auto nCell = info.Owner->RandomCellInZone((ZoneType)nRandom);
		auto nNearby = MapClass::Instance->NearByLocation(nCell,
			SpeedType::Foot, -1, MovementZone::Normal, false, 1, 1, false,
			false, false, true, CellStruct::Empty, false, false);

		if (nNearby.IsValid() && info.CanFireAt(nNearby))
		{
			return { nNearby  , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetLighningRandomTarget(const TargetingInfo& info)
	{
		CellStruct nBuffer;
		for (int i = 0; i < 5; ++i)
		{
			auto& nRand = ScenarioClass::Instance->Random;
			if (!MapClass::Instance->CoordinatesLegal(nBuffer))
			{
				do
				{
					nBuffer.X = (short)nRand.RandomFromMax(MapClass::MapCellWidth);
					nBuffer.Y = (short)nRand.RandomFromMax(MapClass::MapCellHeight);

				}
				while (!MapClass::Instance->CoordinatesLegal(nBuffer));
			}


			if (nBuffer.IsValid() && info.CanFireAt(nBuffer))
				return { nBuffer , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetSelfTarget(const TargetingInfo& info)
	{
		// find the first building providing super
		auto index = info.Super->Type->ArrayIndex;
		const auto& buildings = info.Owner->Buildings;
		// Ares < 0.9 didn't check power
		const auto it = std::find_if(buildings.begin(),
			buildings.end(), [index, &info](BuildingClass* pBld)
		{
			auto const pExt = BuildingExt::ExtMap.Find(pBld);

			if ( /*pExt->HasSuperWeapon(index, true)*/
				info.NewType->IsLaunchSite(info.TypeExt, pBld)
				&& pBld->IsPowerOnline())
			{
				auto cell = CellClass::Coord2Cell(pBld->GetCoords());

				if (info.CanFireAt(cell))
				{
					return true;
				}
			}

			return false;
		});

		if (it != buildings.end()) {
			return { CellClass::Coord2Cell((*it)->GetCoords()) ,SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetBaseTarget(const TargetingInfo& info)
	{
		// fire at the SW's owner's base cell
		CellStruct cell = info.Owner->GetBaseCenter();

		if (cell.IsValid() && info.CanFireAt(cell))
		{
			return { cell, SWTargetFlags::AllowEmpty };
		}

		return{ CellStruct::Empty, SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetMultiMissileTarget(const TargetingInfo& info)
	{
		auto pOwner = info.Owner;
		auto it = info.TypeExt->GetPotentialAITargets(HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex));

		const auto pResult = GetTargetFirstMax(it.begin(), it.end(), [pOwner, &info](TechnoClass* pTechno, int curMax)
		{
			if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
			{
				return -1;
			}

			auto cell = CellClass::Coord2Cell(pTechno->GetCoords());

			auto const value = pTechno->IsCloaked()
				? ScenarioClass::Instance->Random.RandomFromMax(100)
				: MapClass::Instance->GetThreatPosed(cell, pOwner);

			if (value <= curMax || !info.CanFireAt(cell)) { return -1; }

			return value;
		});

		if (pResult)
		{
			return{
				CellClass::Coord2Cell(pResult->GetCoords()),
				SWTargetFlags::AllowEmpty
			};
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetEnemyBaseTarget(const TargetingInfo& info)
	{
		if (auto pEnemy = HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex))
		{
			CellStruct cell = pEnemy->GetBaseCenter();

			if (info.CanFireAt(cell))
			{
				return { cell , SWTargetFlags::AllowEmpty };
			}
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

#pragma endregion
};

TargetResult SWTypeExt::ExtData::PickSuperWeaponTarget(SuperClass* pSuper)
{
	TargetingInfo info(pSuper);

	if (!info.Data)
		info.GetData();

	switch (info.TypeExt->GetAITargetingPreference())
	{
	case TargetingPreference::Offensive:
		if (!pSuper->Owner->PreferredTargetCell.IsValid())
			break;

		return { pSuper->Owner->PreferredTargetCell ,SWTargetFlags::AllowEmpty };
	case TargetingPreference::Devensive:

		if (!pSuper->Owner->PreferredDefensiveCell2.IsValid())
			break;

		return { pSuper->Owner->PreferredDefensiveCell2 ,SWTargetFlags::AllowEmpty };
	default:
		break;
	}

	// all the different AI targeting modes
	switch (info.TypeExt->SW_AITargetingMode)
	{
	case SuperWeaponAITargetingMode::Nuke:
	case SuperWeaponAITargetingMode::LightningStorm:
	{
		return TargetingFuncs::GetNukeAndLighningTarget(info);
	}
	case SuperWeaponAITargetingMode::PsychicDominator:
	{
		return TargetingFuncs::GetDominatorTarget(info);
	}
	case SuperWeaponAITargetingMode::ParaDrop:
	{
		return TargetingFuncs::GetParadropTarget(info);
	}
	case SuperWeaponAITargetingMode::GeneticMutator:
	{
		return TargetingFuncs::GetMutatorTarget(info);
	}
	case SuperWeaponAITargetingMode::ForceShield:
	{
		return TargetingFuncs::GetForceShieldTarget(info);
	}
	case SuperWeaponAITargetingMode::NoTarget:
	case SuperWeaponAITargetingMode::HunterSeeker:
	case SuperWeaponAITargetingMode::Attack:
	case SuperWeaponAITargetingMode::LowPower:
	case SuperWeaponAITargetingMode::LowPowerAttack:
	{
		return TargetingFuncs::NoTarget();
	}
	case SuperWeaponAITargetingMode::Offensive:
	{
		return TargetingFuncs::GetOffensiveTarget(info);
	}
	case SuperWeaponAITargetingMode::Stealth:
	{
		return TargetingFuncs::GetStealthTarget(info);
	}
	case SuperWeaponAITargetingMode::Self:
	{
		return TargetingFuncs::GetSelfTarget(info);
	}
	case SuperWeaponAITargetingMode::Base:
	{
		return TargetingFuncs::GetBaseTarget(info);
	}
	case SuperWeaponAITargetingMode::MultiMissile:
	{
		return TargetingFuncs::GetMultiMissileTarget(info);
	}
	case SuperWeaponAITargetingMode::EnemyBase:
	{
		return TargetingFuncs::GetEnemyBaseTarget(info);
	}
	case SuperWeaponAITargetingMode::Droppod:
	{
		return TargetingFuncs::GetDroppodTarget(info);
	}
	case SuperWeaponAITargetingMode::LighningRandom:
	{
		return TargetingFuncs::GetLighningRandomTarget(info);
	}
	}

	return{ CellStruct::Empty, SWTargetFlags::DisallowEmpty };
}

double SWTypeExt::ExtData::GetChargeToDrainRatio() const
{
	return this->SW_ChargeToDrainRatio.Get(RulesClass::Instance->ChargeToDrainRatio);
}

const char* SWTypeExt::ExtData::get_ID()
{
	return this->Get()->ID;
}

bool SWTypeExt::ExtData::CanFire(HouseClass* pOwner)
{
	const int nAmount = this->SW_Shots;

	if (nAmount < 0)
		return true;

	return HouseExt::ExtMap.Find(pOwner)->GetShotCount(this->Get()).Count < nAmount;
}

// can i see the animation of pFirer's SW?
bool SWTypeExt::ExtData::IsAnimVisible(HouseClass* pFirer)
{
	// auto relation = SWTypeExt::ExtData::GetRelation(pFirer, HouseClass::CurrentPlayer);
	// const auto nRelationResult = (this->SW_AnimVisibility & relation);
	// const bool IsVisible = nRelationResult == relation;
	return EnumFunctions::CanTargetHouse(this->SW_AnimVisibility , pFirer , HouseClass::CurrentPlayer());
}

// does pFirer's SW affects object belonging to pHouse?
bool SWTypeExt::ExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse)
{
	return this->IsHouseAffected(pFirer, pHouse, this->SW_AffectsHouse);
}

bool SWTypeExt::ExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, AffectedHouse value)
{
	// auto relation = SWTypeExt::ExtData::GetRelation(pFirer, pHouse);
	// const auto nRelationResult = (value & relation);
	// const bool IsVisible = nRelationResult == relation;
	return EnumFunctions::CanTargetHouse(value , pFirer ,pHouse);
}

AffectedHouse SWTypeExt::ExtData::GetRelation(HouseClass* pFirer, HouseClass* pHouse)
{
	// that's me!
	if (pFirer == pHouse)
	{
		return AffectedHouse::Owner;
	}

	if (pFirer->IsAlliedWith(pHouse))
	{
		// only friendly houses
		return AffectedHouse::Allies;
	}

	// the bad guys
	return AffectedHouse::Enemies;
}

void SWTypeExt::ExtData::PrintMessage(const CSFText& message, HouseClass* pFirer)
{
	if (message.empty())
	{
		return;
	}

	int color = ColorScheme::FindIndex("Gold");
	if (this->Message_FirerColor)
	{
		// firer color
		if (pFirer)
		{
			color = pFirer->ColorSchemeIndex;
		}
	}
	else
	{
		if (this->Message_ColorScheme >= 0 && this->Message_ColorScheme < ColorScheme::Array->Count)
		{
			// user defined color
			color = this->Message_ColorScheme;
		}
		else if (HouseClass::CurrentPlayer)
		{
			// default way: the current player's color
			color = HouseClass::CurrentPlayer->ColorSchemeIndex;
		}
	}

	// print the message
	MessageListClass::Instance->PrintMessage(message, RulesClass::Instance->MessageDelay, color);
}

Iterator<TechnoClass*> SWTypeExt::ExtData::GetPotentialAITargets(HouseClass* pTarget) const
{
	const auto require = this->GetAIRequiredTarget() & SuperWeaponTarget::AllTechnos;

	if (require == SuperWeaponTarget::None || require & SuperWeaponTarget::Building)
	{
		// either buildings only or it includes buildings
		if (require == SuperWeaponTarget::Building)
		{
			// only buildings from here, either all or of a particular house
			if (pTarget) {
				return make_iterator(pTarget->Buildings);
			}

			return make_iterator(*BuildingClass::Array);
		}

		return make_iterator(*TechnoClass::Array);
	}

	if (require == SuperWeaponTarget::Infantry) {
		return make_iterator(*InfantryClass::Array);
	}

	// it's techno stuff, but not buildings
	return make_iterator(*FootClass::Array);
}

bool SWTypeExt::ExtData::Launch(NewSWType* pNewType, SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	const auto nResult = pNewType->Activate(pSuper, cell, isPlayer);

	if (!nResult)
		return false;

	const auto pOwner = pSuper->Owner;
	auto pHouseExt = HouseExt::ExtMap.Find(pOwner);

	pHouseExt->UpdateShotCount(pSuper->Type);
	const auto pCurrentSWTypeData = SWTypeExt::ExtMap.Find(pSuper->Type); //previous data
	const auto flags = pNewType->Flags(pCurrentSWTypeData);

	if ((flags & SuperWeaponFlags::PostClick))
	{
		// use the properties of the originally fired SW
		if (pHouseExt->SWLastIndex >= 0)
		{
			pSuper = pOwner->Supers[pHouseExt->SWLastIndex];
		}
	}

	const auto pData = SWTypeExt::ExtMap.Find(pSuper->Type); //newer data

	if (pSuper->Granted || pData->CanFire(pOwner))
		pOwner->RecheckTechTree = true;

	const auto curSuperIdx = pOwner->Supers.FindItemIndex(pSuper);
	if (!(flags & SuperWeaponFlags::PostClick) && !pData->SW_AutoFire)
		pHouseExt->SWLastIndex = curSuperIdx;

	if ((flags & SuperWeaponFlags::NoEVA) == SuperWeaponFlags::None && !Unsorted::MuteSWLaunches)
	{
		if (pData->EVA_Activated >= 0)
			VoxClass::PlayIndex(pData->EVA_Activated);
	}

	if ((flags & SuperWeaponFlags::NoMoney) == SuperWeaponFlags::None)
	{
		pOwner->TransactMoney(pData->Money_Amount);
	}

	if ((flags & SuperWeaponFlags::NoPower) == SuperWeaponFlags::None)
	{
		if (pData->SW_Power.isset() && pData->SW_Power.Get() != 0)
		{
			AuxPower(pOwner) += pData->SW_Power.Get();
			pOwner->RecheckPower = true;
		}
	}

	const bool bPlayAnim = (flags & SuperWeaponFlags::NoAnim) == SuperWeaponFlags::None;
	const bool bPlaySound = (flags & SuperWeaponFlags::NoSound) == SuperWeaponFlags::None;

	if (bPlayAnim || bPlaySound)
	{
		auto pCell = MapClass::Instance->GetCellAt(cell);
		auto nCoord = pCell->GetCoordsWithBridge();

		if (bPlayAnim)
		{
			if (auto pAnim = pNewType->GetAnim(pData))
			{
				nCoord.Z += pData->SW_AnimHeight;
				if (AnimClass* placeholder = GameCreate<AnimClass>(pAnim, nCoord))
				{
					placeholder->SetHouse(pOwner);
					placeholder->Invisible = !pData->IsAnimVisible(pOwner);
				}
			}
		}

		if (bPlaySound)
		{
			const int sound = pNewType->GetSound(pData);

			if (sound >= 0)
				VocClass::PlayAt(sound, nCoord, nullptr);
		}
	}

	if ((flags & SuperWeaponFlags::NoEvent) == SuperWeaponFlags::None && pData->SW_RadarEvent)
	{
		RadarEventClass::Create(RadarEventType::SuperweaponActivated, cell);
	}

	if ((flags & SuperWeaponFlags::NoMessage) == SuperWeaponFlags::None)
	{
		pData->PrintMessage(pData->Message_Launch, pOwner);
	}

	// this sw has been fired. clean up.
	if (isPlayer && (flags & SuperWeaponFlags::NoCleanup) == SuperWeaponFlags::None)
	{
		// what's this? we reset the selected SW only for the player on this
		// computer, so others don't deselect it when firing simultaneously.
		// and we only do this, if this type's index is the current one, because
		// auto-firing might happen while the player still selects a target.
		// PostClick SWs do have a different type index, so they need to be
		// special cased, but they can't auto-fire anyhow.
		if (pOwner->IsCurrentPlayer())
		{
			if (curSuperIdx == Unsorted::CurrentSWType || (flags & SuperWeaponFlags::PostClick))
			{
				Unsorted::CurrentSWType = -1;
			}
		}

		// do not play ready sound. this thing just got off.
		if (pData->EVA_Ready >= 0)
		{
			VoxClass::SilenceIndex(pData->EVA_Ready);
		}
	}

	if (!pData->SW_ResetType.empty()) {
		for (auto& pHouseSuper : pOwner->Supers) {
			if (pData->SW_ResetType.Contains(pHouseSuper->Type->ArrayIndex))
				pHouseSuper->Reset();
		}
	}

	return true;
}

bool SWTypeExt::ExtData::Activate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	auto const pOwner = pSuper->Owner;
	if (!pOwner) // the game will crash later anyway , just put some log to give an hint
	{
		Debug::Log("Trying To Activate Super[%s] Without House Owner ! \n", pSuper->Type->ID);
		return false;
	}

	const auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);
	const auto pNewType = NewSWType::GetNewSWType(pExt);

	if (!pNewType || !pExt->Launch(pNewType, pSuper, cell, isPlayer))
		return false;

	for (int i = 0; i < pOwner->RelatedTags.Count; ++i)
	{
		if (auto pTag = pOwner->RelatedTags.GetItem(i))
		{
			std::pair<SuperClass*, CellStruct> nPass { pSuper, cell };

			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::SuperNearWaypoint), nullptr, CellStruct::Empty, false, &nPass);
		}
	}

	for (int a = 0; a < pOwner->RelatedTags.Count; ++a)
	{
		if (auto pTag = pOwner->RelatedTags.GetItem(a))
			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::SuperActivated), nullptr, CellStruct::Empty, false, pSuper);
	}

	return true;

}

bool SWTypeExt::ExtData::Deactivate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	const auto pData = SWTypeExt::ExtMap.Find(pSuper->Type);

	if (auto pNewSWType = NewSWType::GetNewSWType(pData))
	{
		pNewSWType->Deactivate(pSuper, cell, isPlayer);

		const auto flags = pNewSWType->Flags(pData);

		if ((flags & SuperWeaponFlags::NoPower) == SuperWeaponFlags::None)
		{
			if (pData->SW_Power.isset() && pData->SW_Power.Get() != 0)
			{
				AuxPower(pSuper->Owner) -= pData->SW_Power.Get();
				pSuper->Owner->RecheckPower = true;
			}
		}

		for (auto pTag : pSuper->Owner->RelatedTags)
		{
			if (pTag)
				pTag->SpringEvent(TriggerEvent(AresTriggerEvents::SuperDeactivated), nullptr, CellStruct::Empty, false, pSuper);
		}

		return true;
	}

	return false;
}

void SWTypeExt::ExtData::LoadFromRulesFile(CCINIClass* pINI)
{
	//auto pThis = this->Get();
	//const char* pSection = pThis->ID;

	//INI_EX exINI(pINI);
}

void SWTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->EVA_Activated.Read(exINI, pSection, "EVA.Activated");
	this->EVA_Ready.Read(exINI, pSection, "EVA.Ready");
	this->EVA_Detected.Read(exINI, pSection, "EVA.Detected");

	this->Message_Launch.Read(exINI, pSection, "Message.Launch");
	this->Message_FirerColor.Read(exINI, pSection, "Message.FirerColor");

	this->SW_RadarEvent.Read(exINI, pSection, "SW.CreateRadarEvent");

	this->Money_Amount.Read(exINI, pSection, "Money.Amount");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");
	this->LimboDelivery_Types.Read(exINI, pSection, "LimboDelivery.Types");
	this->LimboDelivery_IDs.Read(exINI, pSection, "LimboDelivery.IDs");
	this->LimboDelivery_RollChances.Read(exINI, pSection, "LimboDelivery.RollChances");

	char tempBuffer[0x30];
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights;
		IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "LimboDelivery.RandomWeights%d", i);
		weights.Read(exINI, pSection, tempBuffer);

		if (!weights.size())
			break;

		this->LimboDelivery_RandomWeightsData.push_back(weights);
	}

	ValueableVector<int> weights;
	weights.Read(exINI, pSection, "LimboDelivery.RandomWeights");
	if (weights.size())
		this->LimboDelivery_RandomWeightsData[0] = weights;

	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");
	// inhibitor related
	this->SW_Inhibitors.Read(exINI, pSection, "SW.Inhibitors");
	this->SW_AnyInhibitor.Read(exINI, pSection, "SW.AnyInhibitor");
	this->SW_Designators.Read(exINI, pSection, "SW.Designators");
	this->SW_AnyDesignator.Read(exINI, pSection, "SW.AnyDesignator");

	//Enemy Inhibitors
	this->SW_Suppressors.Read(exINI, pSection, "SW.Suppressors");
	this->SW_AnySuppressor.Read(exINI, pSection, "SW.AnySuppressor");

	//Enemy Designator
	this->SW_Attractors.Read(exINI, pSection, "SW.Attractors");
	this->SW_AnyAttractor.Read(exINI, pSection, "SW.AnyAttractor");

	this->SW_RangeMinimum.Read(exINI, pSection, "SW.RangeMinimum");
	this->SW_RangeMaximum.Read(exINI, pSection, "SW.RangeMaximum");
	this->SW_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SW.RequiredHouses", this->SW_RequiredHouses);
	this->SW_ForbiddenHouses = pINI->ReadHouseTypesList(pSection, "SW.ForbiddenHouses", this->SW_ForbiddenHouses);
	this->SW_AuxBuildings.Read(exINI, pSection, "SW.AuxBuildings");
	this->SW_NegBuildings.Read(exINI, pSection, "SW.NegBuildings");
	this->SW_InitialReady.Read(exINI, pSection, "SW.InitialReady");

	this->SW_AITargetingConstrain.Read(exINI, pSection, "SW.AITargeting.Constraints");;
	this->SW_AIRequiresTarget.Read(exINI, pSection, "SW.AIRequiresTarget");
	this->SW_AIRequiresHouse.Read(exINI, pSection, "SW.AIRequiresHouse");
	this->SW_AITargetingPreference.Read(exINI, pSection, "SW.AITargeting.Preference");
	this->SW_FireToShroud.Read(exINI, pSection, "SW.FireIntoShroud");
	this->SW_UseAITargeting.Read(exINI, pSection, "SW.UseAITargeting");
	this->Message_CannotFire.Read(exINI, pSection, "Message.CannotFire");
	this->SW_RequiresTarget.Read(exINI, pSection, "SW.RequiresTarget");
	this->SW_RequiresHouse.Read(exINI, pSection, "SW.RequiresHouse");

	this->Detonate_Warhead.Read(exINI, pSection, "Detonate.Warhead", true);
	this->Detonate_Weapon.Read(exINI, pSection, "Detonate.Weapon", true);
	this->Detonate_Damage.Read(exINI, pSection, "Detonate.Damage");
	this->Detonate_AtFirer.Read(exINI, pSection, "Detonate.AtFirer");

#pragma region Otamaa
	this->GClock_Shape.Read(exINI, pSection, "GClock.Shape");
	this->GClock_Transculency.Read(exINI, pSection, "GClock.Transculency");
	this->GClock_Palette.Read(exINI, pSection, "GClock.Palette");

	// code disabled , unfinished
	this->ChargeTimer.Read(exINI, pSection, "Timer.ChargeMode");
	this->ChargeTimer_Backwards.Read(exINI, pSection, "Timer.ChargeModeBackwards");
	//
#pragma endregion

	this->SW_Priority.Read(exINI, pSection, "SW.Priority");
	this->SW_Damage.Read(exINI, pSection, "SW.Damage");

	this->CursorType.Read(exINI, pSection, "Cursor");
	this->NoCursorType.Read(exINI, pSection, "NoCursor");
	this->SW_Range.Read(exINI, pSection, "SW.Range");
	this->Message_ColorScheme.Read(exINI, pSection, "Message.ColorScheme");
	this->SW_AITargetingMode.Read(exINI, pSection, "SW.AITargeting");
	this->SW_Group.Read(exINI, pSection, "SW.Group");
	this->SW_Shots.Read(exINI, pSection, "SW.Shots");
	this->SW_AutoFire.Read(exINI, pSection, "SW.AutoFire");
	this->SW_AllowPlayer.Read(exINI, pSection, "SW.AllowPlayer");
	this->SW_AllowAI.Read(exINI, pSection, "SW.AllowAI");
	this->SW_AffectsHouse.Read(exINI, pSection, "SW.AffectsHouse");
	this->SW_AnimVisibility.Read(exINI, pSection, "SW.AnimationVisibility");
	this->SW_AnimHeight.Read(exINI, pSection, "SW.AnimationHeight");
	this->SW_ChargeToDrainRatio.Read(exINI, pSection, "SW.ChargeToDrainRatio");

	//
	this->Converts.Read(exINI, pSection, "Converts");
	this->ConvertsPair.Read(exINI, pSection, "ConvertsPair");
	this->Convert_SucceededAnim.Read(exINI, pSection, "ConvertsAnim");

	//
	this->SW_Warhead.Read(exINI, pSection, "SW.Warhead");
	this->SW_Anim.Read(exINI, pSection, "SW.Animation", true);
	this->SW_Sound.Read(exINI, pSection, "SW.Sound");
	this->SW_ActivationSound.Read(exINI, pSection, "SW.ActivationSound");

	this->SW_Power.Read(exINI, pSection, "SW.Power");

	this->SW_AffectsTarget.Read(exINI, pSection, "SW.AffectsTarget");
	this->SW_PostDependent.Read(exINI.GetINI(), pSection, "SW.PostDependent");

	this->SW_Deferment.Read(exINI, pSection, "SW.Deferment");

	this->Message_Activate.Read(exINI, pSection, "Message.Activate");
	this->Message_Abort.Read(exINI, pSection, "Message.Abort");

	this->SW_MaxCount.Read(exINI, pSection, "SW.MaxCount");

	ValueableIdx<VoxClass> EVA_Impatient { -1 };
	EVA_Impatient.Read(exINI, pSection, "EVA.Impatient");
	pThis->ImpatientVoice = EVA_Impatient.Get();

	this->EVA_InsufficientFunds.Read(exINI, pSection, "EVA.InsufficientFunds");
	this->EVA_SelectTarget.Read(exINI, pSection, "EVA.SelectTarget");

	this->Message_Detected.Read(exINI, pSection, "Message.Detected");
	this->Message_Ready.Read(exINI, pSection, "Message.Ready");
	this->Message_Launch.Read(exINI, pSection, "Message.Launch");
	this->Message_CannotFire.Read(exINI, pSection, "Message.CannotFire");
	this->Message_InsufficientFunds.Read(exINI, pSection, "Message.InsufficientFunds");

	this->Text_Preparing.Read(exINI, pSection, "Text.Preparing");
	this->Text_Ready.Read(exINI, pSection, "Text.Ready");
	this->Text_Hold.Read(exINI, pSection, "Text.Hold");
	this->Text_Charging.Read(exINI, pSection, "Text.Charging");
	this->Text_Active.Read(exINI, pSection, "Text.Active");

	this->SW_ShowCameo.Read(exINI, pSection, "SW.ShowCameo");
	this->SW_VirtualCharge.Read(exINI, pSection, "SW.VirtualCharge");
	this->SW_TimerVisibility.Read(exINI, pSection, "SW.TimerVisibility");
	this->Money_DrainAmount.Read(exINI, pSection, "Money.DrainAmount");
	this->Money_DrainDelay.Read(exINI, pSection, "Money.DrainDelay");

	this->SW_ManualFire.Read(exINI, pSection, "SW.ManualFire");
	this->SW_Unstoppable.Read(exINI, pSection, "SW.Unstoppable");

	// lighting
	this->Lighting_Enabled.Read(exINI, pSection, "Light.Enabled");
	this->Lighting_Ambient.Read(exINI, pSection, "Light.Ambient");
	this->Lighting_Red.Read(exINI, pSection, "Light.Red");
	this->Lighting_Green.Read(exINI, pSection, "Light.Green");
	this->Lighting_Blue.Read(exINI, pSection, "Light.Blue");

	this->SW_AlwaysGranted.Read(exINI, pSection, "SW.AlwaysGranted");

	this->SidebarPalette.Read(exINI, pSection, "SidebarPalette");
	this->SidebarPCX.Read(exINI.GetINI(), pSection, "SidebarPCX");
	this->SW_ResetType.Read(exINI, pSection, "SW.ResetTypes");
	this->SW_Require.Read(exINI, pSection, "SW.RequireBuildings");
	this->Aux_Techno.Read(exINI, pSection, "SW.AuxTechnos");
	this->SW_Lauchsites.Read(exINI, pSection, "SW.LaunchSites");

	// initialize the NewSWType that handles this SWType.
	if (auto pNewSWType = NewSWType::GetNewSWType(this))
	{
		pThis->Action = this->LastAction;
		pNewSWType->LoadFromINI(this, pINI);
		this->LastAction = pThis->Action;

		// whatever the user does, we take care of the stupid tags.
		// there is no need to have them not hardcoded.
		auto const flags = pNewSWType->Flags(this);
		pThis->PreClick = static_cast<bool>(flags & SuperWeaponFlags::PreClick);
		pThis->PostClick = static_cast<bool>(flags & SuperWeaponFlags::PostClick);
		pThis->PreDependent = SuperWeaponType::Invalid;
	}
}

// Universal handler of the rolls-weights system
void SWTypeExt::WeightedRollsHandler(std::vector<int>& nResult, Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls.size();
	size_t weightsSize = weights.size();
	int index = 0;
	std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && RandomBuffer > abs(rolls[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = MinImpl(i, weightsSize - 1);
		index = GeneralUtils::ChooseOneWeighted(RandomBuffer, weights[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			nResult.push_back(index);
	}
}

void SWTypeExt::ExtData::WeightedRollsHandler(std::vector<int>& nResult, std::vector<float>* rolls, std::vector<std::vector<int>>* weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls->size();
	size_t weightsSize = weights->size();
	int index = 0;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && this->RandomBuffer > abs((*rolls)[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = MinImpl(i, weightsSize - 1);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, (*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			nResult.push_back(index);
	}
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		std::vector<int> results;
		this->WeightedRollsHandler(results, &this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());

		for (const size_t& result : results)
		{
			if (result < idsSize)
				id = this->LimboDelivery_IDs[result];

			LimboDeliver(this->LimboDelivery_Types[result], pHouse, id);
		}
	}
	// no randomness mode
	else
	{
		int id = -1;
		size_t ids = this->LimboDelivery_IDs.size();

		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < ids)
				id = this->LimboDelivery_IDs[i];

			LimboDeliver(this->LimboDelivery_Types[i], pHouse, id);
		}
	}
}

void SWTypeExt::ExtData::ApplyLimboKill(HouseClass* pHouse)
{
	if (this->LimboKill_IDs.empty())
		return;

	for (HouseClass* pTargetHouse : *HouseClass::Array())
	{
		if (pTargetHouse->Type->MultiplayPassive)
			continue;

		BuildingExt::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTargetHouse, pHouse);
	}
}

void SWTypeExt::ExtData::ApplyDetonation(SuperClass* pSW, HouseClass* pHouse, const CellStruct& cell)
{
	if (!this->Detonate_Weapon.isset() && !this->Detonate_Warhead.isset())
		return;

	const auto pCell = MapClass::Instance->GetCellAt(cell);
	TechnoClass* pFirer = this->GetNewSWType()->GetFirer(pSW, cell, true);

	CoordStruct nDest = CoordStruct::Empty;
	AbstractClass* pTarget = nullptr;

	if (this->Detonate_AtFirer)
	{
		if (!pFirer)
			return;

		pTarget = pFirer;
		nDest = pFirer->GetCenterCoords();
	}
	else
	{
		pTarget = pCell;
		nDest = pCell->GetCoords();
	}

	if (!MapClass::Instance->IsWithinUsableArea(nDest))
		Debug::Log("SW[%s] Lauch Outside Usable Map Area [%d . %d]! \n", this->Get()->ID , nDest.X , nDest.Y);

	if (!pFirer)
		Debug::Log("SW[%s] ApplyDetonate without Firer!\n", this->Get()->ID);

	if (const auto pWeapon = this->Detonate_Weapon.Get())
		WeaponTypeExt::DetonateAt(pWeapon, nDest, pFirer, this->Detonate_Damage.Get(pWeapon->Damage), true);
	else
	{
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), pTarget, nDest, pFirer, this->Detonate_Damage.Get(this->SW_Damage.Get(0)));
	}
}

void SWTypeExt::ExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// random mode
	if (this->SW_Next_RandomWeightsData.size())
	{
		std::vector<int> results;
		this->WeightedRollsHandler(results, &this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (const int& result : results)
		{
			SWTypeExt::Launch(pSW, pSW->Owner, this, this->SW_Next[result], cell);
		}
	}
	// no randomness mode
	else
	{
		for (const auto& pSWType : this->SW_Next)
			SWTypeExt::Launch(pSW ,pSW->Owner, this, pSWType, cell);
	}
}

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer)
{
	if (!this->LimboDelivery_Types.empty())
		ApplyLimboDelivery(pHouse);

	if (!this->LimboKill_IDs.empty())
		ApplyLimboKill(pHouse);

	if (this->Detonate_Warhead.isset() || this->Detonate_Weapon.isset())
		this->ApplyDetonation(pSW , pSW->Owner, *pCell);

	if (!this->SW_Next.empty())
		this->ApplySWNext(pSW, *pCell);

	if (this->Converts)
	{
		for (const auto pTargetFoot : *FootClass::Array)
			TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTargetFoot , this->Convert_SucceededAnim);
	}
}

bool SWTypeExt::ExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno)
{
	return this->GetNewSWType()->IsInhibitor(this, pOwner, pTechno);
}

bool SWTypeExt::ExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& Coords)
{
	return this->GetNewSWType()->HasInhibitor(this, pOwner, Coords);
}

bool SWTypeExt::ExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno)
{
	return this->GetNewSWType()->IsInhibitorEligible(this, pOwner, Coords, pTechno);
}

bool SWTypeExt::ExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsDesignator(this, pOwner, pTechno);
}

bool SWTypeExt::ExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	return this->GetNewSWType()->HasDesignator(this, pOwner, coords);
}

bool SWTypeExt::ExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsDesignatorEligible(this, pOwner, coords, pTechno);
}

bool SWTypeExt::ExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange)
{
	return this->GetNewSWType()->IsLaunchSiteEligible(this, Coords, pBuilding, ignoreRange);
}

bool SWTypeExt::ExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	return this->GetNewSWType()->IsLaunchSite(this, pBuilding);
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return this->GetNewSWType()->GetLaunchSiteRange(this, pBuilding);
}

bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse)
{
	const auto pThis = this->Get();

	if (!this->CanFire(pHouse))
		return false;

	const bool IsCurrentPlayer = pHouse->IsControlledByCurrentPlayer();

	if (IsCurrentPlayer ? this->SW_AllowPlayer == 0 : this->SW_AllowAI == 0)
		return false;

	if (!this->SW_Require.empty()) {
		for (auto const& pBld : this->SW_Require) {
			if (pBld && !pHouse->CountOwnedAndPresent(pBld))
				return false;
		}
	}

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
		return false;

	// allow only certain houses, disallow forbidden houses
	if (!this->SW_RequiredHouses.Contains(pHouse->Type) || this->SW_ForbiddenHouses.Contains(pHouse->Type))
		return false;

	// check that any aux building exist and no neg building
	auto IsBuildingPresent = [pHouse](BuildingTypeClass* pType) {
		return pType && pHouse->CountOwnedAndPresent(pType) > 0;
	};

	const auto& Aux = this->SW_AuxBuildings;
	// If building Not Exist
	if (!Aux.empty() && std::none_of(Aux.begin(), Aux.end(), IsBuildingPresent)) {
		return false;
	}

	const auto& Neg = this->SW_NegBuildings;
	// If building Exist
	if (!Neg.empty() && std::any_of(Neg.begin(), Neg.end(), IsBuildingPresent)) {
		return false;
	}

	const auto& AuxT = this->Aux_Techno;
	if (!AuxT.empty() && std::none_of(AuxT.begin(), AuxT.end(),
		[pHouse](TechnoTypeClass* pType) { return pType && pHouse->CountOwnedAndPresent(pType) > 0; })) {
		return false;
	}

	return true;
}

void SWTypeExt::ClearChronoAnim(SuperClass* pThis)
{
	if (pThis->Animation)
	{
		pThis->Animation->RemainingIterations = 0;
		pThis->Animation = nullptr;
		PointerExpiredNotification::NotifyInvalidAnim->Remove(pThis);
	}

	if (pThis->AnimationGotInvalid)
	{
		PointerExpiredNotification::NotifyInvalidAnim->Remove(pThis);
		pThis->AnimationGotInvalid = false;
	}
}

void SWTypeExt::CreateChronoAnim(SuperClass* const pThis, const CoordStruct& Coords, AnimTypeClass* const pAnimType)
{
	SWTypeExt::ClearChronoAnim(pThis);

	if (pAnimType)
	{
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, Coords))
		{
			auto const pData = SWTypeExt::ExtMap.Find(pThis->Type);
			pAnim->Invisible = !pData->IsAnimVisible(pThis->Owner);
			pAnim->SetHouse(pThis->Owner);
			pThis->Animation = pAnim;
			PointerExpiredNotification::NotifyInvalidAnim->Add(pThis);
		}
	}
}

LightingColor SWTypeExt::GetLightingColor(SuperWeaponTypeClass* pCustom)
{
	SuperWeaponTypeClass* pType = nullptr;
	LightingColor ret {};
	auto scen = ScenarioClass::Instance();

	if (NukeFlash::IsFadingIn() || ChronoScreenEffect::Status)
	{
		// nuke flash
		ret.Ambient = scen->NukeAmbient;
		ret.Red = scen->NukeLighting.Tint.Red * 10;
		ret.Green = scen->NukeLighting.Tint.Green * 10;
		ret.Blue = scen->NukeLighting.Tint.Blue * 10;
		ret.HasValue = true; //default

		if (SuperClass* pSuper = SW_NuclearMissile::CurrentNukeType)
		{
			pType = pSuper->Type;
		}
	}
	else if (LightningStorm::Active)
	{
		// lightning storm
		ret.Ambient = scen->IonAmbient;
		ret.Red = scen->IonLighting.Tint.Red * 10;
		ret.Green = scen->IonLighting.Tint.Green * 10;
		ret.Blue = scen->IonLighting.Tint.Blue * 10;

		ret.HasValue = true;

		if (SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm)
		{
			pType = pSuper->Type;
		}
	}
	else if (PsyDom::Status != PsychicDominatorStatus::Inactive && PsyDom::Status != PsychicDominatorStatus::Over)
	{
		// psychic dominator
		ret.Ambient = scen->DominatorAmbient;
		ret.Red = scen->DominatorLighting.Tint.Red * 10;
		ret.Green = scen->DominatorLighting.Tint.Green * 10;
		ret.Blue = scen->DominatorLighting.Tint.Blue * 10;
		ret.HasValue = true;

		if (SuperClass* pSuper = SW_PsychicDominator::CurrentPsyDom)
		{
			pType = pSuper->Type;
		}
	}
	else
	{
		// no special lightning
		ret.Ambient = scen->AmbientOriginal;
		ret.Red = scen->NormalLighting.Tint.Red * 10;
		ret.Green = scen->NormalLighting.Tint.Green * 10;
		ret.Blue = scen->NormalLighting.Tint.Blue * 10;
		ret.HasValue = false; //this is the default value
	}

	if (auto const pSW = pCustom ? pCustom : pType) {
		SWTypeExt::ExtMap.Find(pSW)->UpdateLightingColor(ret);
	}

	return ret;
}

bool SWTypeExt::ExtData::UpdateLightingColor(LightingColor& Lighting) const
{
	auto UpdateValue = [](const Nullable<int>& from, int& into, int factor) {
		int value = from.Get(-1);
		if (value >= 0) { into = factor * value; }
	};

	UpdateValue(this->Lighting_Ambient, Lighting.Ambient, 1);
	UpdateValue(this->Lighting_Red, Lighting.Red, 10);
	UpdateValue(this->Lighting_Green, Lighting.Green, 10);
	UpdateValue(this->Lighting_Blue, Lighting.Blue, 10);

	if (this->Lighting_Enabled.isset())
		Lighting.HasValue = this->Lighting_Enabled.Get();

	return true;
}

bool SWTypeExt::ChangeLighting(SuperWeaponTypeClass* pCustom)
{
	auto lighting = SWTypeExt::GetLightingColor(pCustom);

	if (lighting.HasValue)
	{
		auto scen = ScenarioClass::Instance();
		scen->AmbientTarget = lighting.Ambient;
		scen->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, 1);
	}

	return lighting.HasValue;
}

void SWTypeExt::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	BuildingExt::LimboDeliver(pType, pOwner, ID);
}

// SW.Next proper launching mechanic
void SWTypeExt::Launch(SuperClass* pFired, HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell)
{
	const auto pSuper = pHouse->Supers.GetItemOrDefault(pLaunchedType);

	if (!pSuper)
		return;

	const auto pSuperTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
	if (!pLauncherTypeExt->SW_Next_RealLaunch ||
		(pSuperTypeExt && pSuper->IsCharged && pHouse->CanTransactMoney(pSuperTypeExt->Money_Amount)))
	{

		if (pLauncherTypeExt->SW_Next_IgnoreInhibitors || !pSuperTypeExt->HasInhibitor(pHouse, cell)
			&& (pLauncherTypeExt->SW_Next_IgnoreDesignators || pSuperTypeExt->HasDesignator(pHouse, cell)))
		{
			// Forcibly fire
			//SuperExt::ExtMap.Find(pSuper)->Firer = SuperExt::ExtMap.Find(pFired)->Firer;

			pSuper->Launch(cell, true);
			if (pLauncherTypeExt->SW_Next_RealLaunch)
				pSuper->Reset();
		}
	}
}

template <typename T>
void SWTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->EVA_Activated)
		.Process(this->EVA_Ready)
		.Process(this->EVA_Detected)
		.Process(this->Message_Launch)
		.Process(this->Message_FirerColor)
		.Process(this->SW_RadarEvent)
		.Process(this->Money_Amount)
		.Process(this->UIDescription)
		.Process(this->CameoPriority)
		.Process(this->LimboDelivery_Types)
		.Process(this->LimboDelivery_IDs)
		.Process(this->LimboDelivery_RandomWeightsData)
		.Process(this->LimboDelivery_RollChances)
		.Process(this->LimboKill_Affected)
		.Process(this->LimboKill_IDs)
		.Process(this->RandomBuffer)
		.Process(this->SW_Inhibitors)
		.Process(this->SW_AnyInhibitor)
		.Process(this->SW_Designators)
		.Process(this->SW_AnyDesignator)
		.Process(this->SW_RangeMinimum)
		.Process(this->SW_RangeMaximum)
		.Process(this->SW_RequiredHouses)
		.Process(this->SW_ForbiddenHouses)
		.Process(this->SW_AuxBuildings)
		.Process(this->SW_NegBuildings)
		.Process(this->SW_InitialReady)

		.Process(this->SW_AITargetingConstrain)
		.Process(this->SW_AIRequiresTarget)
		.Process(this->SW_AIRequiresHouse)
		.Process(this->SW_AITargetingPreference)
		.Process(this->SW_FireToShroud)
		.Process(this->SW_UseAITargeting)
		.Process(this->Message_CannotFire)
		.Process(this->SW_RequiresTarget)
		.Process(this->SW_RequiresHouse)

		.Process(this->GClock_Shape)
		.Process(this->GClock_Transculency)
		.Process(this->GClock_Palette)
		.Process(this->Detonate_Warhead)
		.Process(this->Detonate_Weapon)
		.Process(this->Detonate_Damage)
		.Process(this->Detonate_AtFirer)
		.Process(this->ChargeTimer)
		.Process(this->ChargeTimer_Backwards)
		.Process(this->SW_Priority)
		.Process(this->SW_Damage)
		.Process(this->CursorType)
		.Process(this->NoCursorType)
		.Process(this->SW_Range)
		.Process(this->Message_ColorScheme)
		.Process(this->SW_AITargetingMode)
		.Process(this->SW_Group)
		.Process(this->SW_Shots)
		.Process(this->SW_AutoFire)
		.Process(this->SW_AllowPlayer)
		.Process(this->SW_AllowAI)
		.Process(this->SW_AffectsHouse)
		.Process(this->SW_AnimVisibility)
		.Process(this->SW_AnimHeight)
		.Process(this->SW_ChargeToDrainRatio)
		.Process(this->HandledType)
		.Process(this->LastAction)
		.Process(this->Converts)
		.Process(this->ConvertsPair)
		.Process(this->Convert_SucceededAnim)
		.Process(this->SW_Warhead)
		.Process(this->SW_Anim)
		.Process(this->SW_Sound)
		.Process(this->SW_ActivationSound)

		.Process(this->SpyPlanes_TypeIndex)
		.Process(this->SpyPlanes_Count)
		.Process(this->SpyPlanes_Mission)
		.Process(this->SpyPlanes_Rank)

		.Process(this->SW_Power)

		.Process(this->Battery_Overpower)
		.Process(this->Battery_KeepOnline)

		.Process(this->SW_AffectsTarget)
		.Process(this->SW_PostDependent)

		.Process(this->Chronosphere_BlastSrc)
		.Process(this->Chronosphere_BlastDest)
		.Process(this->Chronosphere_KillOrganic)
		.Process(this->Chronosphere_KillTeleporters)
		.Process(this->Chronosphere_AffectUndeployable)
		.Process(this->Chronosphere_AffectBuildings)
		.Process(this->Chronosphere_AffectIronCurtain)
		.Process(this->Chronosphere_BlowUnplaceable)
		.Process(this->Chronosphere_ReconsiderBuildings)
		.Process(this->Chronosphere_Delay)

		.Process(this->SW_Deferment)

#pragma region Psychic Dominator
		.Process(this->Dominator_Capture)
		.Process(this->Dominator_FireAtPercentage)
		.Process(this->Dominator_FirstAnimHeight)
		.Process(this->Dominator_SecondAnimHeight)
		.Process(this->Dominator_FirstAnim)
		.Process(this->Dominator_SecondAnim)
		.Process(this->Dominator_ControlAnim)
		.Process(this->Dominator_Ripple)
		.Process(this->Dominator_CaptureMindControlled)
		.Process(this->Dominator_CapturePermaMindControlled)
		.Process(this->Dominator_CaptureImmuneToPsionics)
		.Process(this->Dominator_PermanentCapture)
#pragma endregion

		.Process(this->Message_Activate)
		.Process(this->Message_Abort)

		.Process(this->DropPod_Minimum)
		.Process(this->DropPod_Maximum)
		.Process(this->DropPod_Veterancy)
		.Process(this->DropPod_Types)
		.Process(this->Droppod_Duration)

		.Process(this->EMPField_Duration)
		.Process(this->SW_MaxCount)

		.Process(this->EMPulse_Linked)
		.Process(this->EMPulse_TargetSelf)
		.Process(this->EMPulse_PulseDelay)
		.Process(this->EMPulse_PulseBall)
		.Process(this->EMPulse_Cannons)

		.Process(this->Mutate_Explosion)
		.Process(this->Mutate_IgnoreCyborg)
		.Process(this->Mutate_IgnoreNotHuman)
		.Process(this->Mutate_KillNatural)

		.Process(this->Text_Preparing)
		.Process(this->Text_Hold)
		.Process(this->Text_Ready)
		.Process(this->Text_Charging)
		.Process(this->Text_Active)
#pragma endregion

		.Process(this->Get()->ImpatientVoice)
		.Process(this->EVA_InsufficientFunds)
		.Process(this->EVA_SelectTarget)

		.Process(this->Message_InsufficientFunds)
		.Process(this->Message_Detected)
		.Process(this->Message_Ready)

		.Process(this->HunterSeeker_Type)
		.Process(this->HunterSeeker_RandomOnly)
		.Process(this->HunterSeeker_Buildings)
		.Process(this->HunterSeeker_AllowAttachedBuildingAsFallback)

		.Process(this->Weather_Duration)
		.Process(this->Weather_HitDelay)
		.Process(this->Weather_ScatterDelay)
		.Process(this->Weather_ScatterCount)
		.Process(this->Weather_Separation)
		.Process(this->Weather_CloudHeight)
		.Process(this->Weather_RadarOutage)
		.Process(this->Weather_DebrisMin)
		.Process(this->Weather_DebrisMax)
		.Process(this->Weather_PrintText)
		.Process(this->Weather_IgnoreLightningRod)
		.Process(this->Weather_BoltExplosion)
		.Process(this->Weather_Clouds)
		.Process(this->Weather_Bolts)
		.Process(this->Weather_Debris)
		.Process(this->Weather_Sounds)
		.Process(this->Weather_RadarOutageAffects)
		.Process(this->Weather_UseSeparateState)
		.Process(this->Weather_LightningRodTypes)

		.Process(this->ParaDropDatas)

		.Process(this->Protect_Duration)
		.Process(this->Protect_PlayFadeSoundTime)
		.Process(this->Protect_PowerOutageDuration)
		.Process(this->Protect_IsForceShield)

		.Process(this->Sonar_Delay)

		.Process(this->SW_Deliverables)
		.Process(this->SW_DeliverBuildups)
		.Process(this->SW_OwnerHouse)

		.Process(this->SW_ShowCameo)

		.Process(this->Lighting_Enabled)
		.Process(this->Lighting_Ambient)
		.Process(this->Lighting_Green)
		.Process(this->Lighting_Blue)
		.Process(this->Lighting_Red)

		.Process(this->SW_VirtualCharge)
		.Process(this->SW_TimerVisibility)
		.Process(this->Money_DrainAmount)
		.Process(this->Money_DrainDelay)
		.Process(this->SW_ManualFire)
		.Process(this->SW_Unstoppable)

		//Enemy Inhibitors
		.Process(this->SW_Suppressors)
		.Process(this->SW_AnySuppressor)

		//Enemy Designator
		.Process(this->SW_Attractors)
		.Process(this->SW_AnyAttractor)

		.Process(this->SW_AlwaysGranted)

		.Process(this->SidebarPalette)
		.Process(this->SidebarPCX)
		.Process(this->SW_ResetType)
		.Process(this->SW_Require)
		.Process(this->Aux_Techno)
		.Process(this->SW_Lauchsites)

		.Process(this->MeteorCounts)
		.Process(this->MeteorImactCounts)
		.Process(this->MeteorAddImpactChance)
		.Process(this->MeteorKindChance)
		.Process(this->MeteorImpactKindChance)

		.Process(this->MeteorSmall)
		.Process(this->MeteorLarge)

		.Process(this->MeteorImpactSmall)
		.Process(this->MeteorImpactLarge)

		;

}

// =============================
// container

SWTypeExt::ExtContainer SWTypeExt::ExtMap;

void SWTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(SWTypeExt::TempSuper, ptr);
	AnnounceInvalidPointer(SWTypeExt::LauchData, ptr);
}

bool SWTypeExt::ExtContainer::InvalidateIgnorable(void* ptr)
{
	switch (GetVtableAddr(ptr))
	{
	case SuperClass::vtable:
		return false;
	}

	return true;
}

bool SWTypeExt::ExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	const bool First = Stm
		.Process(SWTypeExt::TempSuper)
		.Process(SWTypeExt::Handled)
		.Process(SWTypeExt::LauchData)
		.Success();

	return First && NewSWType::LoadGlobals(Stm);
}

bool SWTypeExt::ExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	const bool First = Stm
		.Process(SWTypeExt::TempSuper)
		.Process(SWTypeExt::Handled)
		.Process(SWTypeExt::LauchData)
		.Success();

	return First && NewSWType::SaveGlobals(Stm);
}

void SWTypeExt::ExtContainer::Clear()
{
	SWTypeExt::LauchData = nullptr;
	SWTypeExt::TempSuper = nullptr;
}

SWTypeExt::ExtContainer::ExtContainer() : Container("SuperWeaponTypeClass") { }
SWTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6CE6F6, SuperWeaponTypeClass_CTOR, 0x5)
{
	GET(SuperWeaponTypeClass*, pItem, EAX);

	NewSWType::Init();
	SWTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);
	SWTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8D0, SuperWeaponTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6CE800, SuperWeaponTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(SuperWeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SWTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CE8BE, SuperWeaponTypeClass_Load_Suffix, 0x7)
{
	SWTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CE8EA, SuperWeaponTypeClass_Save_Suffix, 0x3)
{
	SWTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CEE50, SuperWeaponTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x6CEE43, SuperWeaponTypeClass_LoadFromINI, 0xA)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x3FC);

	SWTypeExt::ExtMap.LoadFromINI(pItem, pINI, R->Origin() == 0x6CEE50);
	return 0;
}
