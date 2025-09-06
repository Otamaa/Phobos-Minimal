#include "Body.h"

#include <SuperWeaponTypeClass.h>

#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Super/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <Misc/Ares/Hooks/Header.h>

#include <Misc/PhobosToolTip.h>

#include <Notifications.h>

#include <SuperWeaponTypeClass.h>
#include <StringTable.h>

#include "NewSuperWeaponType/NewSWType.h"
#include "NewSuperWeaponType/NuclearMissile.h"
#include "NewSuperWeaponType/LightningStorm.h"
#include "NewSuperWeaponType/Dominator.h"
#include <New/Type/GenericPrerequisite.h>
//#include <ExtraHeaders/DiscreteSelectionClass_s.h>
//#include <ExtraHeaders/DiscreteDistributionClass_s.h>

#include <DiscreteSelectionClass.h>
#include <DiscreteDistributionClass.h>
#include <EventClass.h>

//TODO re-evaluate these , since the default array seems not contains what the documentation table says,..
std::array<const AITargetingModeInfo, (size_t)SuperWeaponAITargetingMode::count> SWTypeExtData::AITargetingModes =
{
{
	{SuperWeaponAITargetingMode::None, SuperWeaponTarget::None, AffectedHouse::None , TargetingConstraints::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Nuke, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies , TargetingConstraints::Enemy, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::LightningStorm, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraints::Enemy | TargetingConstraints::LighningStormInactive, TargetingPreference::Offensive},
	{SuperWeaponAITargetingMode::PsychicDominator, SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit, AffectedHouse::All, TargetingConstraints::Enemy | TargetingConstraints::DominatorInactive | TargetingConstraints::OffensiveCellClear , TargetingPreference::None},
	{SuperWeaponAITargetingMode::ParaDrop, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::GeneticMutator, SuperWeaponTarget::Infantry, AffectedHouse::All, TargetingConstraints::OffensiveCellClear , TargetingPreference::None},
	{SuperWeaponAITargetingMode::ForceShield, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::Enemy , TargetingPreference::Devensive},
	{SuperWeaponAITargetingMode::NoTarget, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Offensive, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraints::Enemy , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Stealth, SuperWeaponTarget::AllTechnos, AffectedHouse::Enemies, TargetingConstraints::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::Self, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::Base, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::MultiMissile, SuperWeaponTarget::Building, AffectedHouse::None, TargetingConstraints::Enemy, TargetingPreference::Offensive },
	{SuperWeaponAITargetingMode::HunterSeeker, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::Enemy, TargetingPreference::None },
	{SuperWeaponAITargetingMode::EnemyBase, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::Enemy, TargetingPreference::None },
	{SuperWeaponAITargetingMode::IronCurtain, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::Attack, SuperWeaponTarget::Building, AffectedHouse::None, TargetingConstraints::Attacked, TargetingPreference::None},
	{SuperWeaponAITargetingMode::LowPower, SuperWeaponTarget::None, AffectedHouse::Owner, TargetingConstraints::LowPower, TargetingPreference::None},
	{SuperWeaponAITargetingMode::LowPowerAttack, SuperWeaponTarget::None, AffectedHouse::Owner, TargetingConstraints::Attacked | TargetingConstraints::LowPower, TargetingPreference::None },
	{SuperWeaponAITargetingMode::DropPod, SuperWeaponTarget::None, AffectedHouse::None, TargetingConstraints::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::LightningRandom, SuperWeaponTarget::AllCells, AffectedHouse::All, TargetingConstraints::None, TargetingPreference::None },
	{SuperWeaponAITargetingMode::LauchSite, SuperWeaponTarget::Building, AffectedHouse::None, TargetingConstraints::None , TargetingPreference::None},
	{SuperWeaponAITargetingMode::FindAuxTechno , SuperWeaponTarget::AllTechnos , AffectedHouse::Owner , TargetingConstraints::None , TargetingPreference::None }

}
};

SWTypeExtData::~SWTypeExtData() noexcept
{
	SuperWeaponTypeClass* pCopy = SWTypeExtData::CurrentSWType;
	if (This() == SWTypeExtData::CurrentSWType)
		pCopy = nullptr;

	SWTypeExtData::CurrentSWType = pCopy;
};

bool SWTypeExtData::IsTypeRedirected() const
{
	return this->HandledType > SuperWeaponType::Invalid;
}

bool SWTypeExtData::IsOriginalType() const
{
	return NewSWType::IsOriginalType(This()->Type);
}

NewSWType* SWTypeExtData::GetNewSWType() const
{
	return NewSWType::GetNewSWType(this);
}

void SWTypeExtData::Initialize()
{
	this->Text_Ready = GameStrings::TXT_READY();
	this->Text_Hold = GameStrings::TXT_HOLD();
	this->Text_Charging = GameStrings::TXT_CHARGING();
	this->Text_Active = GameStrings::TXT_FIRESTORM_ON();
	this->Message_CannotFire = "MSG:CannotFire";

	this->EVA_InsufficientFunds = VoxClass::FindIndexById(GameStrings::EVA_InsufficientFunds);
	this->EVA_SelectTarget = VoxClass::FindIndexById(GameStrings::EVA_SelectTarget);

	if (auto pNewSWType = NewSWType::GetNewSWType(this)) {
		pNewSWType->Initialize(this);

		if(This()->Action != Action::None)
			This()->Action = Action(AresNewActionType::SuperWeaponAllowed);
	}

	this->LastAction = This()->Action;

	this->Music_Theme = -1;
	this->Music_Duration = 0;
}

Action SWTypeExtData::GetAction(SuperWeaponTypeClass* pSuper, CellStruct* pTarget)
{
	if ((AresNewActionType)pSuper->Action != AresNewActionType::SuperWeaponAllowed)
		return Action::None;

	const auto pExt = SWTypeExtContainer::Instance.Find(pSuper);

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
		SWTypeExtData::CurrentSWType = nullptr;
		Cursor = pExt->NoCursorType;
	}
	else
	{
		SWTypeExtData::CurrentSWType = pSuper;
		Cursor = pExt->CursorType;
	}

	MouseCursorFuncs::SetSuperWeaponCursorAction(Cursor, (Action)result , pExt->SW_FireToShroud);
	return (Action)result;
}

SuperWeaponTarget SWTypeExtData::GetAIRequiredTarget() const
{
	if (this->SW_AIRequiresTarget.isset())
	{
		return this->SW_AIRequiresTarget;
	}

	const size_t index = static_cast<size_t>(this->SW_AITargetingMode.Get());

	if (index < AITargetingModes.size())
	{
		return AITargetingModes[index].Target;
	}

	return SuperWeaponTarget::None;
}

AffectedHouse SWTypeExtData::GetAIRequiredHouse() const
{
	if (this->SW_AIRequiresHouse.isset())
	{
		return this->SW_AIRequiresHouse;
	}

	const size_t index = static_cast<size_t>(this->SW_AITargetingMode.Get());

	if (index < AITargetingModes.size())
	{
		return AITargetingModes[index].House;
	}

	return AffectedHouse::None;
}

std::pair<TargetingConstraints, bool> SWTypeExtData::GetAITargetingConstraints() const
{
	if (this->SW_AITargetingConstrain.isset())
		return { this->SW_AITargetingConstrain.Get(), false };
	else
	{
		const size_t index = static_cast<size_t>(this->SW_AITargetingMode.Get());

		if (index < AITargetingModes.size()) {
			return { AITargetingModes[index].Constraints , false };
		}
	}

	return { TargetingConstraints::None , true };
}

TargetingPreference SWTypeExtData::GetAITargetingPreference() const
{
	if (this->SW_AITargetingPreference.isset())
		return this->SW_AITargetingPreference.Get();
	else
	{
		const size_t index = static_cast<size_t>(this->SW_AITargetingMode.Get());

		if (index < AITargetingModes.size())
		{
			return AITargetingModes[index].Preference;
		}
	}

	return TargetingPreference::None;
}

bool SWTypeExtData::IsCellEligible(CellClass* pCell, SuperWeaponTarget allowed)
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

bool SWTypeExtData::IsTechnoEligible(TechnoClass* pTechno, SuperWeaponTarget allowed)
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

bool SWTypeExtData::IsTechnoAffected(TechnoClass* pTechno)
{
	// check land and water cells
	if (!this->IsCellEligible(pTechno->GetCell(), this->SW_AffectsTarget)) {
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

bool SWTypeExtData::CanFireAt(HouseClass* pOwner, const CellStruct& coords, bool manual)
{
	auto pCell = MapClass::Instance->GetCellAt(coords);

	// check cell type
	auto const AllowedTarget = !manual ? this->GetAIRequiredTarget() : this->SW_RequiresTarget;

	if (!IsCellEligible(pCell, AllowedTarget)) {
		return false;
	}

	// check for techno type match
	auto const pTechno = flag_cast_to<TechnoClass*>(pCell->GetContent());
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

bool SWTypeExtData::LauchSuper(SuperClass* pSuper)
{
	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	const auto pHouseExt = HouseExtContainer::Instance.Find(pSuper->Owner);
	const auto pCurrent = pSuper->Owner;
	// if this SW is only auto-firable, discard any clicks.
	// if AutoFire is off, the sw would not be firable at all,
	// thus we ignore the setting in that case.
	const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
	const bool unstoppable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
		&& pSWExt->SW_Unstoppable;

	// play impatient voice, if this isn't charged yet
	if (!manual && !pSuper->CanFire())
	{
		pSWExt->UneableToFireAtTheMoment(pCurrent);
		return false;
	}

	if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
	{
		pSWExt->UneableToTransactMoney(pCurrent);
		return false;
	}

	if (!pHouseExt->CanTransactBattlePoints(pSWExt->BattlePoints_Amount))
	{
		pSWExt->UneableToTransactBattlePoints(pCurrent);
		return false;
	}

	if (!pSWExt->SW_UseAITargeting || SWTypeExtData::IsTargetConstraintsEligible(pSuper, true))
	{
		if (!manual && !unstoppable)
		{
			const auto swIndex = pSuper->Type->ArrayIndex;

			if (pSuper->Type->Action == Action::None || pSWExt->SW_UseAITargeting)
			{
				EventClass Event { pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty };
				EventClass::AddEvent(&Event);
			}
			else
			{
				DisplayClass::Instance->CurrentBuilding = nullptr;
				DisplayClass::Instance->CurrentBuildingType = nullptr;
				DisplayClass::Instance->CurrentBuildingOwnerArrayIndex = -1;
				DisplayClass::Instance->SetActiveFoundation(nullptr);
				MapClass::Instance->SetRepairMode(0);
				MapClass::Instance->SetSellMode(0);
				DisplayClass::Instance->PowerToggleMode = false;
				DisplayClass::Instance->PlanningMode = false;
				DisplayClass::Instance->PlaceBeaconMode = false;
				DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
				MapClass::Instance->UnselectAll();
				VoxClass::PlayIndex(pSWExt->EVA_SelectTarget);
			}

			return true;
		}
	}
	else
	{
		//const auto pHouseID = pCurrent->get_ID();
		//Debug::LogInfo("[{} - {}] SW [{} - {}] CannotFire", pHouseID, (void*)pCurrent, pSuper->Type->ID, (void*)pSuper);
		pSWExt->PrintMessage(pSWExt->Message_CannotFire, pCurrent);
	}

	return false;
}

bool SWTypeExtData::DrawDarken(SuperClass* pSuper)
{
	const auto pSWExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	const auto pHouseExt = HouseExtContainer::Instance.Find(pSuper->Owner);
	const auto pCurrent = pSuper->Owner;

	if (pSuper->CanFire())
	{
		if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
			return true;

		if (!pHouseExt->CanTransactBattlePoints(pSWExt->BattlePoints_Amount))
			return true;
	}

	if (pSWExt->SW_UseAITargeting && !SWTypeExtData::IsTargetConstraintsEligible(pSuper, true))
		return true;

	return false;
}

bool SWTypeExtData::IsTargetConstraintsEligible(SuperClass* pThis, bool IsPlayer)
{
	const auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);
	auto pOwner = pThis->Owner;
	auto const& [nFlag , SkipOffensiveClear] = pExt->GetAITargetingConstraints();

	auto valid = [](const CellStruct& nVal) { return nVal.X || nVal.Y; };

	if (!SkipOffensiveClear) {
		if (((nFlag & TargetingConstraints::OffensiveCellClear) != TargetingConstraints::None) && valid(pOwner->PreferredTargetCell))
			return false;
	}

	if (((nFlag & TargetingConstraints::OffensiveCellSet) != TargetingConstraints::None) && !valid(pOwner->PreferredTargetCell))
		return false;

	if (((nFlag & TargetingConstraints::DefensifeCellClear) != TargetingConstraints::None) && valid(pOwner->PreferredDefensiveCell2))
		return false;

	if (((nFlag & TargetingConstraints::DefensiveCellSet) != TargetingConstraints::None) && !valid(pOwner->PreferredDefensiveCell2))
		return false;

	if (((nFlag & TargetingConstraints::Enemy) != TargetingConstraints::None) && pOwner->EnemyHouseIndex < 0)
		return false;

	if (!IsPlayer)
	{
		if (((nFlag & TargetingConstraints::LighningStormInactive) != TargetingConstraints::None) && LightningStorm::IsActive())
			return false;

		if (((nFlag & TargetingConstraints::DominatorInactive) != TargetingConstraints::None) && PsyDom::IsActive())
			return false;

		if (((nFlag & TargetingConstraints::Attacked) != TargetingConstraints::None) && (pOwner->LATime && ((pOwner->LATime + 75) < Unsorted::CurrentFrame)))
			return false;

		if (((nFlag & TargetingConstraints::LowPower) != TargetingConstraints::None) && pOwner->HasFullPower())
			return false;
	}

	return true;
}

bool SWTypeExtData::TryFire(SuperClass* pThis, bool IsPlayer)
{
	const auto pExt = SWTypeExtContainer::Instance.Find(pThis->Type);

	// don't try to fire if we obviously haven't enough money
	if (SWTypeExtData::IsResourceAvailable(pThis)) {

		if (pExt->SW_AutoFire_CheckAvail && !pExt->IsAvailable(pThis->Owner))
			return false;

		if (SWTypeExtData::IsTargetConstraintsEligible(pThis, IsPlayer)) {

			const auto pNewType = pExt->GetNewSWType();
			if (!pNewType) {
				Debug::FatalErrorAndExit("Trying to fire SW [%s] with invalid Type[%d]", pThis->Type->ID, (int)pThis->Type->Type);
			}

			auto pTargeting = pNewType->GetTargetingData(pExt, pThis->Owner);
			const auto& [Cell, Flag] = SWTypeExtData::PickSuperWeaponTarget(pNewType , pTargeting.get() , pThis);

			if (Flag == SWTargetFlags::AllowEmpty) {
				 if(pThis->Owner->IsControlledByHuman() && !pExt->SW_AutoFire && pExt->SW_ManualFire) {
				 	Unsorted::CurrentBuilding = nullptr;
				 	Unsorted::CurrentBuildingType = nullptr;
				 	Unsorted::unknown_11AC = static_cast<DWORD>(-1);
				 	DisplayClass::Instance->SetRepairMode(0);
				 	DisplayClass::Instance->SetSellMode(0);
				 	Unsorted::PowerToggleMode = false;
				 	Unsorted::PlanningMode = false;
				 	Unsorted::PlaceBeaconMode = false;
				 	MapClass::UnselectAll();
				 }

				return pThis->Owner->Fire_SW(pThis->Type->ArrayIndex, Cell);
			}
		}
	}

	return false;

}

//struct TargetingInfo
//{
//	TargetingInfo(SuperClass* pSuper) :
//		Super(pSuper),
//		Owner(pSuper->Owner),
//		TypeExt(nullptr),
//		NewType(nullptr)
//	{
//		this->TypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
//		this->NewType = NewSWType::GetNewSWType(this->TypeExt);
//		this->Data = this->NewType->GetTargetingData(this->TypeExt, pSuper->Owner);
//	}
//
//	~TargetingInfo() = default;
//
//	bool CanFireAt(const CellStruct& cell, bool manual = false) const
//	{
//		return this->NewType->CanFireAt(*this->Data, cell, manual);
//	}
//
//	// void GetData() const
//	// {
//	// 	this->Data = this->NewType->GetTargetingData(this->TypeExt, this->Owner);
//	// }
//
//public:
//	SuperClass* Super;
//	HouseClass* Owner;
//	SWTypeExtData* TypeExt;
//	NewSWType* NewType;
//	std::unique_ptr<const TargetingData> mutable Data;
//
//private:
//	TargetingInfo(const TargetingInfo&) = default;
//	TargetingInfo(TargetingInfo&&) = default;
//	TargetingInfo&operator=(const TargetingInfo& other) = default;
//};

struct TargetingFuncs
{
#pragma region Helpers
	template<typename It, typename Valuator>
	static COMPILETIMEEVAL ObjectClass* GetTargetFirstMax(It first, It last, Valuator value)
	{
		ObjectClass* pTarget = nullptr;
		int maxValue = 0;

		for (auto it = first; it < last; ++it) {
			auto curValue = value(*it, maxValue);

			if (curValue > maxValue) {
				pTarget = *it;
				maxValue = curValue;
			}
		}

		return pTarget;
	}

	template<typename It, typename Valuator>
	static ObjectClass* GetTargetAnyMax(It first, It last, Valuator value)
	{
		if (first == last) { return nullptr; }

		DiscreteSelectionClass<ObjectClass*> targets {};

		for (auto it = first; it < last; ++it) {
			int val = value(*it, targets.GetRating());
			targets.Add(*it, val);
		}

		return targets.Select(ScenarioClass::Instance->Random);
	}

	template<typename It, typename Valuator>
	static ObjectClass* GetTargetShareAny(It first, It last, Valuator value)
	{
		if (first == last) { return nullptr; }

		DiscreteDistributionClass<ObjectClass*> targets {};

		for (auto it = first; it < last; ++it) {
			int val = value(*it);
			targets.Add(*it, val);
		}

		return targets.Select(ScenarioClass::Instance->Random);
	}

	static COMPILETIMEEVAL bool IsTargetAllowed(TechnoClass* pTechno)
	{
		return !pTechno->InLimbo && pTechno->IsAlive;
	}

	static bool IgnoreThis(TechnoClass* pTechno)
	{
		if (const auto pBld = cast_to<BuildingClass*>(pTechno)) {
			if (BuildingExtContainer::Instance.Find(pBld)->LimboID != -1)
				return true;
		}

		return false;
	}

	static COMPILETIMEEVAL TargetResult NoTarget()
	{
		return { CellStruct::Empty , SWTargetFlags::AllowEmpty };
	}
#pragma endregion

#pragma region MainTargeting
	static TargetResult GetIonCannonTarget(NewSWType* pNewType, const TargetingData* pTargeting, HouseClass* pEnemy, CloakHandling cloak)
	{
		std::vector<TechnoClass*> targets {};
		const auto it = pTargeting->TypeExt->GetPotentialAITargets(pEnemy , targets);
		const auto pResult = GetTargetAnyMax(it.begin(), it.end(),
			[=](TechnoClass* pTechno, int curMax) {

				// original game code only compares owner and doesn't support nullptr
				auto const passedFilter = (!pEnemy || pTechno->Owner == pEnemy);

				if (passedFilter && ((FakeHouseClass*)pTargeting->Owner)->_IsIonCannonEligibleTarget(pTechno))
				{
					if (TargetingFuncs::IgnoreThis(pTechno))
						return -1;

					auto const cell = CellClass::Coord2Cell(pTechno->GetCoords());

					if (!MapClass::Instance->IsWithinUsableArea(cell, true)) { return -1; }

					const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno->GetTechnoType());

					int value = 0;
					bool RandomiedCloaked = false; // avoid the early AIIonCannnonValue

					// cloak options
					if (cloak != CloakHandling::AgnosticToCloak)
					{
						bool cloaked = pTechno->IsCloaked();

						if (cloak == CloakHandling::RandomizeCloaked)
						{
							// original behavior
							if (cloaked) {
								RandomiedCloaked = true;
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

					if(!RandomiedCloaked){
						value = pTypeExt->AIIonCannonValue.isset() ?
							pTypeExt->AIIonCannonValue->at(pTargeting->Owner->GetAIDifficultyIndex())
							:
							pTechno->GetIonCannonValue(pTargeting->Owner->AIDifficulty);
					}

					// do not do heavy lifting on objects that
					// would not be chosen anyhow
					if (value >= curMax) {
							if(pNewType->CanTargetingFireAt(pTargeting, cell, false)){
								return value;
						}
					}
				}

				return -1;
			});

		return pResult ?
		TargetResult{ CellClass::Coord2Cell(pResult->GetCoords()) , SWTargetFlags::AllowEmpty } :
		TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult PickByHouseType(HouseClass* pThis, QuarryType type)
	{
		const auto nTarget = pThis->PickTargetByType(type);
		const SWTargetFlags flag = nTarget.IsValid() ? SWTargetFlags::AllowEmpty : SWTargetFlags::DisallowEmpty;
		return { nTarget , flag };
	}

	static TargetResult GetDominatorTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		std::vector<TechnoClass*> targets {};
		const auto it = pTargeting->TypeExt->GetPotentialAITargets(nullptr, targets);
		const auto pTarget = GetTargetFirstMax(it.begin(), it.end(), [pTargeting, pNewType](TechnoClass* pTechno, int curMax) {

			if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno)) {
				return -1;
			}

			const auto cell = pTechno->GetCell()->MapCoords;
			int value = 0;

			for (size_t i = 0; i < CellSpread::NumCells(3); ++i)
			{
				 auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));
				 for (NextObject j(pCell->FirstObject); flag_cast_to<FootClass*>(*j); ++j)
				 {
					 const auto pFoot = static_cast<FootClass*>(*j);

					 if (!pTargeting->Owner->IsAlliedWith(pFoot) && !pFoot->IsInAir())
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
			 return (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting, cell , false)) ? -1 : value;
		});

		return pTarget ?
		TargetResult{ CellClass::Coord2Cell(pTarget->GetCoords())  , SWTargetFlags::AllowEmpty }:
		TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetParadropTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		static const int SpaceSize = 5;
		auto target = CellStruct::Empty;

		if (pTargeting->Owner->PreferredTargetType == QuarryType::Anything)
		{
			// if no enemy yet, reinforce own base
			const auto pTargetPlayer = HouseClass::Array->GetItemOrDefault(pTargeting->Owner->EnemyHouseIndex, pTargeting->Owner);

			target = MapClass::Instance->NearByLocation(
				pTargetPlayer->GetBaseCenter(), SpeedType::Foot, ZoneType::None,
				MovementZone::Normal, false, SpaceSize, SpaceSize, false,
				false, false, true, CellStruct::Empty, false, false);

			if (target != CellStruct::Empty) {
				target += CellStruct{ short(SpaceSize / 2), short(SpaceSize / 2) };
			}
		}
		else
		{
			target = pTargeting->Owner->PickTargetByType(pTargeting->Owner->PreferredTargetType);
		}

		return  (!target.IsValid() || !pNewType->CanTargetingFireAt(pTargeting , target , false))  ?
		TargetResult{ CellStruct::Empty, SWTargetFlags::DisallowEmpty } :
		TargetResult{ target , SWTargetFlags::AllowEmpty };
	}

	static TargetResult GetMutatorTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		//specific implementation for GeneticMutatorTargetSelector for
		std::vector<TechnoClass*> targets {};
		const auto it = pTargeting->TypeExt->GetPotentialAITargets(nullptr, targets);
		const auto pResult = GetTargetFirstMax(it.begin(), it.end(), [pTargeting , pNewType](TechnoClass* pTechno, int curMax) {

			if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno)) {
			  return -1;
			}

			if(pTargeting->TypeExt->This()->Type == SuperWeaponType::GeneticMutator && pTechno->WhatAmI() == InfantryClass::AbsID) {
				const auto pInfantryType = ((InfantryClass*)pTechno)->Type;

				if (pInfantryType->Cyborg && pTargeting->TypeExt->Mutate_IgnoreCyborg) {
					return -1;
				}
				if (pInfantryType->NotHuman && pTargeting->TypeExt->Mutate_IgnoreNotHuman) {
					return -1;
				}
			}

			auto cell = pTechno->GetCell()->MapCoords;
			int value = 0;

			for (size_t i = 0; i < CellSpread::NumCells(1); ++i)
			{
				 const auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

				 for (NextObject j(pCell->GetInfantry(pTechno->OnBridge)); cast_to<InfantryClass*>(*j); ++j)
				 {
					const auto pInf = static_cast<InfantryClass*>(*j);

					 if (!pTargeting->Owner->IsAlliedWith(pInf) && !pInf->IsInAir())
					 {
						 // original game does not consider cloak
						 if (pInf->CloakState != CloakState::Cloaked)
						 {
							 ++value;
						 }
					 }
				 }

				 if (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting , cell , false)) {
					 return -1;
				 }
			}

		   return value;
		});

		return pResult ?
		TargetResult{ CellClass::Coord2Cell(pResult->GetCoords()), SWTargetFlags::AllowEmpty }:
		TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetForceShieldTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		if (pTargeting->Owner->PreferredDefensiveCell.IsValid()
			&& (RulesClass::Instance->AISuperDefenseFrames + pTargeting->Owner->PreferredDefensiveCellStartTime) > Unsorted::CurrentFrame
			&& pNewType->CanTargetingFireAt(pTargeting , pTargeting->Owner->PreferredDefensiveCell , false))
		{
			return { pTargeting->Owner->PreferredDefensiveCell , SWTargetFlags::AllowEmpty };
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetOffensiveTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		return GetIonCannonTarget(pNewType ,
			pTargeting,
			HouseClass::Array->GetItemOrDefault(pTargeting->Owner->EnemyHouseIndex),
			CloakHandling::IgnoreCloaked);
	}

	static TargetResult GetNukeAndLighningTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		if (pTargeting->Owner->PreferredTargetType == QuarryType::Anything) {
			return TargetingFuncs::GetIonCannonTarget(pNewType ,pTargeting,
				HouseClass::Array->GetItemOrDefault(pTargeting->Owner->EnemyHouseIndex),
				CloakHandling::IgnoreCloaked);
		}

		return TargetingFuncs::PickByHouseType(pTargeting->Owner, pTargeting->Owner->PreferredTargetType);
	}

	static TargetResult GetStealthTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		return TargetingFuncs::GetIonCannonTarget(pNewType, pTargeting, nullptr, CloakHandling::RequireCloaked);
	}

	static TargetResult GetDroppodTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		const auto nRandom = ScenarioClass::Instance->Random.RandomRangedSpecific(ZoneType::North, ZoneType::West);
		const auto nCell = pTargeting->Owner->RandomCellInZone(nRandom);
		const auto nNearby = MapClass::Instance->NearByLocation(nCell,
			SpeedType::Foot, ZoneType::None, MovementZone::Normal, false, 1, 1, false,
			false, false, true, CellStruct::Empty, false, false);

		return (nNearby.IsValid() && pNewType->CanTargetingFireAt(pTargeting, nNearby, false)) ?
		TargetResult{ nNearby, SWTargetFlags::AllowEmpty }:
		TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetLighningRandomTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		CellStruct nBuffer {};
		for (int i = 0; i < 5; ++i)
		{
			auto& nRand = ScenarioClass::Instance->Random;
			if (!MapClass::Instance->CoordinatesLegal(nBuffer))
			{
				do
				{
					nBuffer.X = (short)nRand.RandomFromMax(MapClass::MapCellDimension->Width);
					nBuffer.Y = (short)nRand.RandomFromMax(MapClass::MapCellDimension->Height);

				}
				while (!MapClass::Instance->CoordinatesLegal(nBuffer));
			}

			if (pNewType->CanTargetingFireAt(pTargeting, nBuffer , false)) {
				return { nBuffer , SWTargetFlags::AllowEmpty };
			}
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetOwnerBuildingAsTarget(NewSWType* pNewType, const TargetingData* pTargeting, bool checkLauchsite = false)
	{
		// find the first building providing super
		auto index = pTargeting->TypeExt->This()->ArrayIndex;
		const auto& buildings = pTargeting->Owner->Buildings;
		// Ares < 0.9 didn't check power
		const auto it = buildings.find_if([index, pTargeting, checkLauchsite , pNewType](BuildingClass* pBld)
			{
				auto const pExt = BuildingExtContainer::Instance.Find(pBld);
				const bool IsEligibleBuilding = !checkLauchsite ?
					pNewType->IsLaunchSite(pTargeting->TypeExt, pBld):
					pExt->HasSuperWeapon(index, true);

				if (IsEligibleBuilding && pBld->IsPowerOnline())
				{
					auto cell = CellClass::Coord2Cell(pBld->GetCoords());

					if (pNewType->CanTargetingFireAt(pTargeting, cell , false))
					{
						return true;
					}
				}

				return false;
			});

		return (it != buildings.end()) ?
			TargetResult{ CellClass::Coord2Cell((*it)->GetCoords()), SWTargetFlags::AllowEmpty } :
			TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetBaseTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		// fire at the SW's owner's base cell
		CellStruct cell = pTargeting->Owner->GetBaseCenter();

		return cell.IsValid() && pNewType->CanTargetingFireAt(pTargeting, cell , false) ?
			TargetResult{ cell, SWTargetFlags::AllowEmpty }:
			TargetResult{ CellStruct::Empty, SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetMultiMissileTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		std::vector<TechnoClass*> targets {};
		const auto it = pTargeting->TypeExt->GetPotentialAITargets(HouseClass::Array->GetItemOrDefault(pTargeting->Owner->EnemyHouseIndex), targets);
		const auto pResult = GetTargetFirstMax(it.begin(), it.end(), [pTargeting, pNewType](TechnoClass* pTechno, int curMax)
		{
			if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
			{
				return -1;
			}

			auto cell = CellClass::Coord2Cell(pTechno->GetCoords());

			auto const value = pTechno->IsCloaked()
				? ScenarioClass::Instance->Random.RandomFromMax(100)
				: MapClass::Instance->GetThreatPosed(cell, pTargeting->Owner);

			if (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting, cell , false)) { return -1; }

			return value;
		});

		return pResult ?
			TargetResult{ CellClass::Coord2Cell(pResult->GetCoords()), SWTargetFlags::AllowEmpty } :
			TargetResult{ CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetEnemyBaseTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		if (auto pEnemy = HouseClass::Array->GetItemOrDefault(pTargeting->Owner->EnemyHouseIndex))
		{
			CellStruct cell = pEnemy->GetBaseCenter();

			if (pNewType->CanTargetingFireAt(pTargeting, cell , false))
			{
				return { cell , SWTargetFlags::AllowEmpty };
			}
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}

	static TargetResult GetAuxTechnoTarget(NewSWType* pNewType, const TargetingData* pTargeting)
	{
		if (!pTargeting->TypeExt->Aux_Techno.empty())
		{
			//const auto TargetOwner = info.TypeExt->GetAIRequiredHouse();
			//HouseClass* TargetHouse = nullptr;
			//if ((TargetOwner & AffectedHouse::Owner) != AffectedHouse::None)
			//	TargetHouse = info.Owner;
			//else if ((TargetOwner & AffectedHouse::Enemies) != AffectedHouse::None)
			//	TargetHouse = HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex);
			//else if ((TargetOwner & AffectedHouse::Allies) != AffectedHouse::None){
			//	const auto It = std::find_if(HouseClass::Array->begin(), HouseClass::Array->end(),
			//		[&](HouseClass* pHouse) {
			//			 if (pHouse->Defeated || pHouse->IsObserver())
			//				 return false;
			//
			//			return info.Owner->Allies.Contains(pHouse);
			//		});
			//
			//	TargetHouse = It != HouseClass::Array->end() ? *It : nullptr;
			//}
			//
			//if (!TargetHouse || TargetHouse->Defeated || TargetHouse->IsObserver())
			//	return { CellStruct::Empty ,SWTargetFlags::DisallowEmpty };
			std::vector<TechnoClass*> targets {};

			for (auto pTech : pTargeting->TypeExt->GetPotentialAITargets(nullptr, targets)) {
				if (TechnoExtData::IsAlive(pTech, false, false, false) && !TargetingFuncs::IgnoreThis(pTech)) {

					auto nLoc = pTech->GetCoords();
					auto nLocCell = CellClass::Coord2Cell(nLoc);

					if (nLoc == CoordStruct::Empty || nLocCell == CellStruct::Empty)
						continue;

					if (pTargeting->TypeExt->Aux_Techno.Contains(pTech->GetTechnoType())) {
						if (pNewType->CanTargetingFireAt(pTargeting ,nLocCell , false)) {
							return { nLocCell , SWTargetFlags::AllowEmpty };
						}
					}
				}
			}
		} else {
			Debug::LogInfo("Uneable to fire SW [{} - {}] , AuxTechno is empty!", pTargeting->TypeExt->Name(), pTargeting->Owner->Type->ID);
		}

		return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
	}
#pragma endregion

};

TargetResult SWTypeExtData::PickSuperWeaponTarget(NewSWType* pNewType , const TargetingData* pTargeting, const SuperClass* pSuper)
{
	switch (pTargeting->TypeExt->GetAITargetingPreference())
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
	switch (pTargeting->TypeExt->SW_AITargetingMode)
	{
	case SuperWeaponAITargetingMode::FindAuxTechno:
	{
		return TargetingFuncs::GetAuxTechnoTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::Nuke:
	case SuperWeaponAITargetingMode::LightningStorm:
	case SuperWeaponAITargetingMode::IonCannon:
	{
		return TargetingFuncs::GetNukeAndLighningTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::PsychicDominator:
	{
		return TargetingFuncs::GetDominatorTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::ParaDrop:
	{
		return TargetingFuncs::GetParadropTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::GeneticMutator:
	{
		return TargetingFuncs::GetMutatorTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::ForceShield:
	{
		return TargetingFuncs::GetForceShieldTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::NoTarget:
	case SuperWeaponAITargetingMode::HunterSeeker: // the documentation say the house need to pick `Favorite` enemy , but infact the dll code just like thse
	case SuperWeaponAITargetingMode::Attack:
	case SuperWeaponAITargetingMode::LowPower:
	case SuperWeaponAITargetingMode::LowPowerAttack:
	{
		return TargetingFuncs::NoTarget();
	}
	case SuperWeaponAITargetingMode::Offensive:
	{
		return TargetingFuncs::GetOffensiveTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::Stealth:
	{
		return TargetingFuncs::GetStealthTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::Self:
	{
		return TargetingFuncs::GetOwnerBuildingAsTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::LauchSite:
	{
		return TargetingFuncs::GetOwnerBuildingAsTarget(pNewType, pTargeting, true);
	}
	case SuperWeaponAITargetingMode::Base:
	{
		return TargetingFuncs::GetBaseTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::MultiMissile:
	{
		return TargetingFuncs::GetMultiMissileTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::EnemyBase:
	{
		return TargetingFuncs::GetEnemyBaseTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::DropPod:
	{
		return TargetingFuncs::GetDroppodTarget(pNewType, pTargeting);
	}
	case SuperWeaponAITargetingMode::LightningRandom:
	{
		return TargetingFuncs::GetLighningRandomTarget(pNewType, pTargeting);
	}
	default:
		break;
	}

	return{ CellStruct::Empty, SWTargetFlags::DisallowEmpty };
}

 bool SWTypeExtData::CanFire(HouseClass* pOwner) const
 {
 	const int nAmount = this->SW_Shots;

 	if (nAmount < 0)
 		return true;

 	return
		HouseExtContainer::Instance.Find(pOwner)->GetShotCount(This()).Count <
		nAmount;
 }

// can i see the animation of pFirer's SW?
bool SWTypeExtData::IsAnimVisible(HouseClass* pFirer) const
{
	// auto relation = SWTypeExtData::GetRelation(pFirer, HouseClass::CurrentPlayer);
	// const auto nRelationResult = (this->SW_AnimVisibility & relation);
	// const bool IsVisible = nRelationResult == relation;
	return EnumFunctions::CanTargetHouse(this->SW_AnimVisibility , pFirer , HouseClass::CurrentPlayer());
}

// does pFirer's SW affects object belonging to pHouse?
bool SWTypeExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse)
{
	return this->IsHouseAffected(pFirer, pHouse, this->SW_AffectsHouse);
}

bool SWTypeExtData::IsHouseAffected(HouseClass* pFirer, HouseClass* pHouse, AffectedHouse value)
{
	// auto relation = SWTypeExtData::GetRelation(pFirer, pHouse);
	// const auto nRelationResult = (value & relation);
	// const bool IsVisible = nRelationResult == relation;
	return EnumFunctions::CanTargetHouse(value , pFirer ,pHouse);
}

AffectedHouse SWTypeExtData::GetRelation(HouseClass* pFirer, HouseClass* pHouse)
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

void SWTypeExtData::PrintMessage(const CSFText& message, HouseClass* pFirer)
{
	if (message.empty()) {
		return;
	}

	int color = -1;

	if (this->Message_FirerColor && pFirer) {
		// firer color
		color = pFirer->ColorSchemeIndex;
	}
	else
	{
		if ((size_t)this->Message_ColorScheme < ColorScheme::Array->size())
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

	if (color == -1)
		color =  ColorScheme::FindIndex("Gold");

	// print the message
	message.PrintAsMessage<false>(color);
}

Iterator<TechnoClass*> SWTypeExtData::GetPotentialAITargets(HouseClass* pTarget, std::vector<TechnoClass*>& outVec) const
{
	const auto require = this->GetAIRequiredTarget();

	if (require == SuperWeaponTarget::None
		|| require == SuperWeaponTarget::AllTechnos
		|| require == SuperWeaponTarget::All
		|| require == SuperWeaponTarget::AllContents
		|| (require & SuperWeaponTarget::Building && require & SuperWeaponTarget::Infantry && require & SuperWeaponTarget::Unit)) {

		return make_iterator(*TechnoClass::Array);
	}

	if (require == SuperWeaponTarget::Building) {
		if (pTarget) {
			return make_iterator(pTarget->Buildings);
		}
		else {
			return make_iterator(*BuildingClass::Array);
		}
	}
	else if (require == SuperWeaponTarget::Infantry) {
		return make_iterator(*InfantryClass::Array);

		//default behaviours
	} else if (require == SuperWeaponTarget::Unit
		|| require == SuperWeaponTarget::Land
		|| require == SuperWeaponTarget::Water
		|| require == SuperWeaponTarget::Empty
		|| require == SuperWeaponTarget::AllCells
		) {
		return make_iterator(*FootClass::Array);
	}

	// part below is more expensive targeting
	outVec.clear();
	outVec.reserve(TechnoClass::Array->Count);

	if (require & SuperWeaponTarget::Building) {
		if (pTarget) {
			std::copy(pTarget->Buildings.begin(), pTarget->Buildings.end(), std::back_inserter(outVec));
		}else {
			std::copy(BuildingClass::Array->begin(), BuildingClass::Array->end(), std::back_inserter(outVec));
		}
	}

	if(require & SuperWeaponTarget::Infantry)
		std::copy(InfantryClass::Array->begin(), InfantryClass::Array->end(), std::back_inserter(outVec));

	if (require & SuperWeaponTarget::Unit){
		std::copy(UnitClass::Array->begin(), UnitClass::Array->end(), std::back_inserter(outVec));
		std::copy(AircraftClass::Array->begin(), AircraftClass::Array->end(), std::back_inserter(outVec));
	}

	return make_iterator(outVec);
}

bool SWTypeExtData::Launch(NewSWType* pNewType, SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	const auto nResult = pNewType->Activate(pSuper, cell, isPlayer);

	if (!nResult)
		return false;

	const auto pOwner = pSuper->Owner;
	auto pHouseExt = HouseExtContainer::Instance.Find(pOwner);

	pHouseExt->UpdateShotCount(pSuper->Type);
	const auto pCurrentSWTypeData = SWTypeExtContainer::Instance.Find(pSuper->Type); //previous data
	const auto flags = pNewType->Flags(pCurrentSWTypeData);

	if ((flags & SuperWeaponFlags::PostClick))
	{
		// use the properties of the originally fired SW
		if (pHouseExt->SWLastIndex >= 0)
		{
			pSuper = pOwner->Supers[pHouseExt->SWLastIndex];
		}
	}

	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type); //newer data

	if (pSuper->OneTime || (pCurrentSWTypeData->SW_Shots >= 0 && HouseExtContainer::Instance.Find(pOwner)->GetShotCount(pSuper->Type).Count >= pCurrentSWTypeData->SW_Shots))
		pOwner->RecheckTechTree = true;

	const auto curSuperIdx = pOwner->Supers.FindItemIndex(pSuper);
	if (!(flags & SuperWeaponFlags::PostClick) && !pData->SW_AutoFire) {
		pHouseExt->SWLastIndex = curSuperIdx;
	}

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
			pHouseExt->AuxPower += pData->SW_Power.Get();
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
				AnimClass* placeholder = GameCreate<AnimClass>(pAnim, nCoord);
				placeholder->SetHouse(pOwner);
				placeholder->Invisible = !pData->IsAnimVisible(pOwner);
			}
		}

		if (bPlaySound)
		{
			VocClass::SafeImmedietelyPlayAt(pNewType->GetSound(pData), & nCoord, nullptr);
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
			auto& curSw = Unsorted::CurrentSWType();
			if (curSuperIdx == curSw || (flags & SuperWeaponFlags::PostClick))
			{
				curSw = -1;
			}
		}

		// do not play ready sound. this thing just got off.
		if (pData->EVA_Ready >= 0)
		{
			VoxClass::SilenceIndex(pData->EVA_Ready);
		}
	}

	//if (!pData->SW_ResetType.empty()) {
	//	for (auto& pHouseSuper : pOwner->Supers) {
	//		if (pHouseSuper == pSuper)
	//			continue;

	//		if (pData->SW_ResetType.Contains(pHouseSuper->Type->ArrayIndex))
	//		{
	//			pHouseSuper->Reset();
	//			//auto pTypeExt = SWTypeExtContainer::Instance.Find(pHouseSuper->Type);

	//			//if(pTypeExt->SW_VirtualCharge)

	//		}

	//	}
	//}

	if ((AresNewSuperType)This()->Type == AresNewSuperType::EMPulse)
	{
		//why this fuckery even exist ,..
		//the SW can be state can be exploited at some point , smh
		if (this->EMPulse_SuspendOthers)
		{
			auto iter = pHouseExt->SuspendedEMPulseSWs.get_key_iterator(pSuper);

			if (iter != pHouseExt->SuspendedEMPulseSWs.end()) {
				for (auto const& ppSuper : iter->second) {
					ppSuper->IsOnHold = false;
				}

				pHouseExt->SuspendedEMPulseSWs.erase(iter);
			}
		}
	}

	for (auto& sw_ : pData->SW_ResetType) {

		if (size_t(sw_) >= pOwner->Supers.size())
			continue;

		const auto pHouseSuper = pOwner->Supers[sw_];

		if (pHouseSuper == pSuper)
			continue;

		pHouseSuper->Reset();
	}

	return true;
}

bool SWTypeExtData::Activate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	auto const pOwner = pSuper->Owner;
	if (!pOwner) // the game will crash later anyway , just put some log to give an hint
	{
		Debug::LogInfo("Trying To Activate Super[{}] Without House Owner ! ", pSuper->Type->ID);
		return false;
	}

	const auto pExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	const auto pNewType = NewSWType::GetNewSWType(pExt);

	if (!pNewType || !pExt->Launch(pNewType, pSuper, cell, isPlayer))
		return false;

	//Debug::LogInfo(__FUNCTION__" for [%s] - Owner[%s] AfterSWLauch ", pExt->get_ID(), pOwner->get_ID());
	std::pair<SuperClass*, CellStruct> nPass { pSuper, cell };

	for (int i = 0; i < pOwner->RelatedTags.Count; ++i)
	{
		if (auto pTag = pOwner->RelatedTags.Items[i])
		{
			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::SuperNearWaypoint), nullptr, CellStruct::Empty, false, &nPass);
		}
	}

	//Debug::LogInfo(__FUNCTION__" for [%s] - Owner[%s] After SuperNearWaypoint  ", pExt->get_ID(), pOwner->get_ID());
	for (int a = 0; a < pOwner->RelatedTags.Count; ++a)
	{
		if (auto pTag = pOwner->RelatedTags.Items[a])
			pTag->RaiseEvent(TriggerEvent(AresTriggerEvents::SuperActivated), nullptr, CellStruct::Empty, false, pSuper);
	}

	//Debug::LogInfo(__FUNCTION__" for [%s] - Owner[%s] After SuperActivated  ", pExt->get_ID(), pOwner->get_ID());

	return true;
}

bool SWTypeExtData::Deactivate(SuperClass* pSuper, CellStruct const cell, bool const isPlayer)
{
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);

	if (auto pNewSWType = NewSWType::GetNewSWType(pData))
	{
		pNewSWType->Deactivate(pSuper, cell, isPlayer);

		const auto flags = pNewSWType->Flags(pData);

		if ((flags & SuperWeaponFlags::NoPower) == SuperWeaponFlags::None)
		{
			if (pData->SW_Power.isset() && pData->SW_Power.Get() != 0)
			{
				HouseExtContainer::Instance.Find(pSuper->Owner)->AuxPower -= pData->SW_Power.Get();
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

void SWTypeExtData::LoadFromRulesFile(CCINIClass* pINI)
{
	//auto pThis = this->Get();
	//const char* pSection = pThis->ID;

	//INI_EX exINI(pINI);
}

bool SWTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = This();
	const char* pSection = pThis->ID;

	if (parseFailAddr)
		return false;

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
	this->LimboDelivery_RandomWeightsData.clear();

	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights {};
		weights.Read(exINI, pSection, (std::string("LimboDelivery.RandomWeights") + std::to_string(i)).c_str());

		if (weights.empty())
			break;

		this->LimboDelivery_RandomWeightsData.push_back(std::move(weights));
	}

	std::vector<int> weights {};
	detail::ReadVectors(weights, exINI, pSection, "LimboDelivery.RandomWeights");
	if (!weights.empty()) {
		if (this->LimboDelivery_RandomWeightsData.size())
			this->LimboDelivery_RandomWeightsData[0] = std::move(weights);
		else
			this->LimboDelivery_RandomWeightsData.push_back(std::move(weights));
	}

	this->LimboKill_Affected.Read(exINI, pSection, "LimboKill.Affected");
	this->LimboKill_IDs.Read(exINI, pSection, "LimboKill.IDs");

	// inhibitor related
	this->SW_Inhibitors.Read(exINI, pSection, "SW.Inhibitors");
	this->SW_AnyInhibitor.Read(exINI, pSection, "SW.AnyInhibitor");
	this->SW_Designators.Read(exINI, pSection, "SW.Designators");
	this->SW_AnyDesignator.Read(exINI, pSection, "SW.AnyDesignator");
	this->ShowDesignatorRange.Read(exINI, pSection, "ShowDesignatorRange");

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

	this->SW_AITargetingConstrain.Read(exINI, pSection, "SW.AITargeting.Constraints");
	this->SW_AIRequiresTarget.Read(exINI, pSection, "SW.AIRequiresTarget");
	this->SW_AIRequiresHouse.Read(exINI, pSection, "SW.AIRequiresHouse");
	this->SW_AITargetingPreference.Read(exINI, pSection, "SW.AITargeting.Preference");
	this->SW_FireToShroud.Read(exINI, pSection, "SW.FireIntoShroud");
	this->SW_UseAITargeting.Read(exINI, pSection, "SW.UseAITargeting");
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
	this->SW_AutoFire_CheckAvail.Read(exINI, pSection, "SW.AutoFire.CheckAvail");
	this->SW_AllowPlayer.Read(exINI, pSection, "SW.AllowPlayer");
	this->SW_AllowAI.Read(exINI, pSection, "SW.AllowAI");
	this->SW_AffectsHouse.Read(exINI, pSection, "SW.AffectsHouse");
	this->SW_AnimVisibility.Read(exINI, pSection, "SW.AnimationVisibility");
	this->SW_AnimHeight.Read(exINI, pSection, "SW.AnimationHeight");
	this->SW_ChargeToDrainRatio.Read(exINI, pSection, "SW.ChargeToDrainRatio");

	this->SW_Next.Read(exINI, pSection, "SW.Next");
	this->SW_Next_RealLaunch.Read(exINI, pSection, "SW.Next.RealLaunch");
	this->SW_Next_IgnoreInhibitors.Read(exINI, pSection, "SW.Next.IgnoreInhibitors");
	this->SW_Next_IgnoreDesignators.Read(exINI, pSection, "SW.Next.IgnoreDesignators");
	this->SW_Next_RollChances.Read(exINI, pSection, "SW.Next.RollChances");
	this->SW_Next_RandomWeightsData.clear();

	std::string basetag = "SW.Next.RandomWeights";
	for (size_t i = 0; ; ++i) {
		ValueableVector<int> weights2;
		weights2.Read(exINI, pSection, (basetag + std::to_string(i)).c_str());

		if (!weights2.size())
			break;

		this->SW_Next_RandomWeightsData.push_back(weights2);
	}

	ValueableVector<int> weights2;
	weights2.Read(exINI, pSection, basetag.c_str());
	if (weights2.size()) {
		if (this->SW_Next_RandomWeightsData.size())
			this->SW_Next_RandomWeightsData[0] = std::move(weights2);
		else
			this->SW_Next_RandomWeightsData.push_back(std::move(weights2));
	}

	//
	this->Converts_UseSWRange.Read(exINI, pSection, "Converts.UseSWRange");
	TechnoTypeConvertData::Parse(Phobos::Otamaa::CompatibilityMode, this->ConvertsPair, exINI, pSection, "ConvertsPair");
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

	IndexFinder<VoxClass>::getindex(pThis->ImpatientVoice , exINI, pSection, "EVA.Impatient");
	this->EVA_InsufficientFunds.Read(exINI, pSection, "EVA.InsufficientFunds");
	this->EVA_InsufficientBattlePoints.Read(exINI, pSection, "EVA.InsufficientBattlePoints");
	this->EVA_SelectTarget.Read(exINI, pSection, "EVA.SelectTarget");

	this->Message_Detected.Read(exINI, pSection, "Message.Detected");
	this->Message_Ready.Read(exINI, pSection, "Message.Ready");
	this->Message_Launch.Read(exINI, pSection, "Message.Launch");
	this->Message_CannotFire.Read(exINI, pSection, "Message.CannotFire");
	this->Message_InsufficientFunds.Read(exINI, pSection, "Message.InsufficientFunds");
	this->Message_InsufficientBattlePoints.Read(exINI, pSection, "Message.InsufficientBattlePoints");
	this->Message_Impatient.Read(exINI, pSection, "Message.Impatient");

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
	GenericPrerequisite::Parse(pINI, pSection, "SW.RequireBuildings", this->SW_Require);
	this->Aux_Techno.Read(exINI, pSection, "SW.AuxTechnos");
	this->SW_Lauchsites.Read(exINI, pSection, "SW.LaunchSites");

	this->UseWeeds.Read(exINI, pSection, "UseWeeds");

	if (this->UseWeeds)
		This()->ShowTimer = false;

	this->UseWeeds_Amount.Read(exINI, pSection, "UseWeeds.Amount");
	this->UseWeeds_StorageTimer.Read(exINI, pSection, "UseWeeds.StorageTimer");
	this->UseWeeds_ReadinessAnimationPercentage.Read(exINI, pSection, "UseWeeds.ReadinessAnimationPercentage");

	this->CrateGoodies.Read(exINI, pSection, "CrateGoodies");
	this->SuperWeaponSidebar_Allow.Read(exINI, pSection, "SuperWeaponSidebar.Allow");
	this->SuperWeaponSidebar_PriorityHouses = pINI->ReadHouseTypesList(pSection, "SuperWeaponSidebar.PriorityHouses", this->SuperWeaponSidebar_PriorityHouses);
	this->SuperWeaponSidebar_RequiredHouses = pINI->ReadHouseTypesList(pSection, "SuperWeaponSidebar.RequiredHouses", this->SuperWeaponSidebar_RequiredHouses);
	this->TabIndex.Read(exINI, pSection, "TabIndex");

	this->BattlePoints_Amount.Read(exINI, pSection, "BattlePoints.Amount");
	this->BattlePoints_DrainAmount.Read(exINI, pSection, "BattlePoints.DrainAmount");
	this->BattlePoints_DrainDelay.Read(exINI, pSection, "BattlePoints.DrainDelay");

	this->SuperWeaponSidebar_Significance.Read(exINI, pSection, "SuperWeaponSidebar.Significance");

	this->SW_Link.Read(exINI, pSection, "SW.Link");
	this->SW_Link_Grant.Read(exINI, pSection, "SW.Link.Grant");
	this->SW_Link_Ready.Read(exINI, pSection, "SW.Link.Ready");
	this->SW_Link_Reset.Read(exINI, pSection, "SW.Link.Reset");
	this->Message_LinkedSWAcquired.Read(exINI, pSection, "Message.LinkedSWAcquired");
	this->EVA_LinkedSWAcquired.Read(exINI, pSection, "EVA.LinkedSWAcquired");
	this->SW_Link_RollChances.Read(exINI, pSection, "SW.Link.RollChances");

	std::string _tag("SW.Link.RandomWeights");
	for (size_t i = 0; ; ++i)
	{
		ValueableVector<int> weights3;
		weights3.Read(exINI, pSection, (_tag + std::to_string(i)).c_str());

		if (!weights3.size())
			break;

		this->SW_Link_RandomWeightsData.push_back(std::move(weights3));
	}
	ValueableVector<int> weights3;
	weights3.Read(exINI, pSection, "SW.Link.RandomWeights");
	if (weights3.size())
	{
		if (this->SW_Link_RandomWeightsData.size())
			this->SW_Link_RandomWeightsData[0] = std::move(weights3);
		else
			this->SW_Link_RandomWeightsData.push_back(std::move(weights3));
	}

	this->Music_Theme = pINI->ReadTheme(pSection, "Music.Theme", this->Music_Theme);
	this->Music_Duration.Read(exINI, pSection, "Music.Duration");
	this->Music_AffectedHouses.Read(exINI, pSection, "Music.AffectedHouses");

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

	return true;
}

// Universal handler of the rolls-weights system
void SWTypeExtData::WeightedRollsHandler(std::vector<int>& nResult, Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls.size();
	size_t weightsSize = weights.size();
	int index = 0;
	//std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && RandomBuffer > Math::abs(rolls[i]))
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

std::vector<int> SWTypeExtData::WeightedRollsHandler(std::vector<float>* rolls, std::vector<std::vector<int>>* weights, size_t size)
{
	std::vector<int> nResult {};
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
		if (!rollOnce && this->RandomBuffer > Math::abs((*rolls)[i]))
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = MinImpl(i, weightsSize - 1);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, (*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			nResult.push_back(index);
	}

	return nResult;
}

void SWTypeExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	if (!pHouse || pHouse->Type->MultiplayPassive)
		return;

	// random mode
	if (!this->LimboDelivery_RandomWeightsData.empty())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		std::vector<int> results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());

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

void SWTypeExtData::ApplyLimboKill(HouseClass* pHouse)
{
	if (this->LimboKill_IDs.empty())
		return;

	if(this->LimboKill_Affected == AffectedHouse::Owner && !pHouse->Type->MultiplayPassive) {
		BuildingExtData::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pHouse, pHouse);
	} else if (this->LimboKill_Affected != AffectedHouse::None){
		for (HouseClass* pTargetHouse : *HouseClass::Array()) {
			if (pTargetHouse->Type->MultiplayPassive)
				continue;

			BuildingExtData::ApplyLimboKill(this->LimboKill_IDs, this->LimboKill_Affected, pTargetHouse, pHouse);
		}
	}
}

void SWTypeExtData::ApplyDetonation(SuperClass* pSW, HouseClass* pHouse, const CellStruct& cell)
{
	if (!this->Detonate_Weapon && !this->Detonate_Warhead)
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
		Debug::LogInfo("SW[{}] Lauch Outside Usable Map Area [{} . {}]! ", This()->ID , nDest.X , nDest.Y);

	if (!pFirer)
		Debug::LogInfo("SW[{}] ApplyDetonate without Firer!", This()->ID);

	if (const auto pWeapon = this->Detonate_Weapon.Get())
		WeaponTypeExtData::DetonateAt(pWeapon, nDest, pFirer, this->Detonate_Damage.Get(pWeapon->Damage), true , pSW->Owner);
	else
	{
		WarheadTypeExtData::DetonateAt(this->Detonate_Warhead.Get(), pTarget, nDest, pFirer, this->Detonate_Damage.Get(0), pSW->Owner);
	}
}

void SWTypeExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell, bool IsPlayer)
{
	// random mode
	if (!this->SW_Next_RandomWeightsData.empty()) {
		for (const int& result :
			this->WeightedRollsHandler(
				&this->SW_Next_RollChances,
				&this->SW_Next_RandomWeightsData,
				this->SW_Next.size())
			)
		{
			SWTypeExtData::Launch(pSW, pSW->Owner, this, this->SW_Next[result], cell , IsPlayer);
		}
	}
	// no randomness mode
	else
	{
		for (const auto& pSWType : this->SW_Next)
			SWTypeExtData::Launch(pSW ,pSW->Owner, this, pSWType, cell, IsPlayer);
	}
}

#include <ThemeClass.h>

void SWTypeExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer)
{
	//Debug::LogInfo("Applying additional functionalities for sw[%s]", pSW->get_ID());

	// Music: play theme and start timer if configured
	auto pSWExt = SuperExtContainer::Instance.Find(pSW);

	if (this->Music_Theme.Get() >= 0)
	{
		const auto affected = this->Music_AffectedHouses.Get();
		if (EnumFunctions::CanTargetHouse(affected, pHouse, HouseClass::CurrentPlayer))
		{
			ThemeClass::Instance->Play(this->Music_Theme);
		}

		const int duration = this->Music_Duration.Get();
		if (duration > 0)
		{
			pSWExt->MusicTimer.Start(duration);
			pSWExt->MusicActive = true;
		}
	}

	if (!this->LimboDelivery_Types.empty())
		ApplyLimboDelivery(pHouse);

	if (!this->LimboKill_IDs.empty())
		ApplyLimboKill(pHouse);

	if (this->Detonate_Warhead || this->Detonate_Weapon)
		this->ApplyDetonation(pSW , pSW->Owner, *pCell);

	if (!this->SW_Next.empty())
		this->ApplySWNext(pSW, *pCell , IsCurrentPlayer);

	if (this->SW_Link.size() > 0)
		this->ApplyLinkedSW(pSW);

	if (this->BattlePoints_Amount != 0)
		SWTypeExtData::ApplyBattlePoints(pSW);

	if (!this->ConvertsPair.empty())
	{
		if(!this->Converts_UseSWRange){
			for (const auto pTargetFoot : *FootClass::Array) {
				if (pTargetFoot->Health <= 0 || !pTargetFoot->IsAlive || pTargetFoot->IsCrashing || pTargetFoot->IsSinking)
					continue;

				if (pTargetFoot->WhatAmI() == UnitClass::AbsID) {
					if (static_cast<const UnitClass*>(pTargetFoot)->DeathFrameCounter > 0) {
						continue;
					}
				}

				TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTargetFoot , this->Convert_SucceededAnim);
			}
		}
		else
		{
			const auto pCellptr = MapClass::Instance->GetCellAt(*pCell);
			const auto range = this->GetNewSWType()->GetRange(this);
			Helpers::Alex::ApplyFuncToCellSpreadItems<FootClass>(pCellptr->GetCoordsWithBridge(), range.WidthOrRange ,
			[=](FootClass* pTarget) {
				TechnoTypeConvertData::ApplyConvert(this->ConvertsPair, pHouse, pTarget, this->Convert_SucceededAnim);
			 }, true);
		}
	}
}

bool SWTypeExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsInhibitor(this, pOwner, pTechno);
}

bool SWTypeExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& Coords) const
{
	return this->GetNewSWType()->HasInhibitor(this, pOwner, Coords);
}

bool SWTypeExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsInhibitorEligible(this, pOwner, Coords, pTechno);
}

bool SWTypeExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsDesignator(this, pOwner, pTechno);
}

bool SWTypeExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	return this->GetNewSWType()->HasDesignator(this, pOwner, coords);
}

bool SWTypeExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	return this->GetNewSWType()->IsDesignatorEligible(this, pOwner, coords, pTechno);
}

bool SWTypeExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange)
{
	return this->GetNewSWType()->IsLaunchSiteEligible(this, Coords, pBuilding, ignoreRange);
}

bool SWTypeExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	return this->GetNewSWType()->IsLaunchSite(this, pBuilding);
}

std::pair<double, double> SWTypeExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return this->GetNewSWType()->GetLaunchSiteRange(this, pBuilding);
}

bool SWTypeExtData::IsAvailable(HouseClass* pHouse)
{
	const auto pThis = This();

	if (this->SW_Shots >= 0 && HouseExtContainer::Instance.Find(pHouse)->GetShotCount(pThis).Count >= this->SW_Shots)
		return false;

	if (pHouse->IsControlledByHuman() ? (!this->SW_AllowPlayer) : (!this->SW_AllowAI))
		return false;

	if (!this->SW_Require.empty()) {
		if (!Prereqs::HouseOwnsAll(pHouse, this->SW_Require.data(), (int)this->SW_Require.size()))
			return false;
	}
	// check that any aux building exist and no neg building
	const auto IsBuildingPresent = [pHouse](BuildingTypeClass* pType) {

		if(pHouse->CountOwnedAndPresent(pType) <= 0){
			auto pAuxExt = BuildingTypeExtContainer::Instance.Find(pType);

			if(!pAuxExt->PowersUp_Buildings.empty() || BuildingTypeClass::Find(pType->PowersUpBuilding))
				 return BuildingTypeExtData::GetUpgradesAmount(pType, pHouse) > 0;

			return false;
		}

		return true;

	};

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && !IsBuildingPresent(pThis->AuxBuilding)){
		return false;
	}

	// allow only certain houses, disallow forbidden houses
	if (!((this->SW_RequiredHouses.data & (1u << pHouse->Type->ArrayIndex)) != 0u)
			|| ((this->SW_ForbiddenHouses.data & (1u << pHouse->Type->ArrayIndex)) != 0u))
		return false;

	const auto& Aux = this->SW_AuxBuildings;
	// If building Not Exist
	if (!Aux.empty() && Aux.None_Of(IsBuildingPresent)) {
		return false;
	}

	const auto& Neg = this->SW_NegBuildings;
	// If building Exist
	if (!Neg.empty() && Neg.Any_Of(IsBuildingPresent)) {
		return false;
	}

	const auto& AuxT = this->Aux_Techno;
	int count = 0;
	if (!AuxT.empty() && AuxT.None_Of([pHouse, &count](TechnoTypeClass* pType)
		{
			if (pType)
			{
				count = pHouse->CountOwnedAndPresent(pType);
			}

			return count > 0;
		}))
	{
		return false;
	}

	return true;
}

void SWTypeExtData::UneableToTransactMoney(HouseClass* pHouse)
{
	VoxClass::PlayIndex(this->EVA_InsufficientFunds);
	this->PrintMessage(this->Message_InsufficientFunds, pHouse);
}

void SWTypeExtData::UneableToTransactBattlePoints(HouseClass* pHouse)
{
	VoxClass::PlayIndex(this->EVA_InsufficientBattlePoints);
	this->PrintMessage(this->Message_InsufficientBattlePoints, pHouse);
}

void SWTypeExtData::UneableToFireAtTheMoment(HouseClass* pHouse) {
	VoxClass::PlayIndex(This()->ImpatientVoice);
	this->PrintMessage(this->Message_Impatient, pHouse);
}

bool SWTypeExtData::ApplyDrainMoney(int timeLeft, HouseClass* pHouse) {
	const int money = this->Money_DrainAmount;

	if (money != 0 && this->Money_DrainDelay > 0)
	{
		if (!(timeLeft % this->Money_DrainDelay))
		{
			// only abort if SW drains money and there is none
			if (!pHouse->CanTransactMoney(money))
			{
				if (pHouse->IsControlledByHuman())
				{
					this->UneableToTransactMoney(pHouse);
				}

				return false;
			}

			// apply drain money
			pHouse->TransactMoney(money);
		}
	}

	return true;
}

bool SWTypeExtData::ApplyDrainBattlePoint(int timeLeft, HouseClass* pHouse) {
	const int money = this->BattlePoints_DrainAmount;
	auto pExt = HouseExtContainer::Instance.Find(pHouse);

	if (money != 0 && this->BattlePoints_DrainDelay > 0)
	{
		if (!(timeLeft % this->BattlePoints_DrainDelay))
		{
			// only abort if SW drains and there is none
			if (!pExt->CanTransactBattlePoints(money))
			{
				if (pHouse->IsControlledByHuman())
				{
					this->UneableToTransactBattlePoints(pHouse);
				}

				return false;
			}

			// apply drain
			pExt->UpdateBattlePoints(money);
		}
	}

	return true;
}

void SWTypeExtData::ClearChronoAnim(SuperClass* pThis)
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

void SWTypeExtData::CreateChronoAnim(SuperClass* const pThis, const CoordStruct& Coords, AnimTypeClass* const pAnimType)
{
	SWTypeExtData::ClearChronoAnim(pThis);

	if (pAnimType)
	{
		auto pAnim = GameCreate<AnimClass>(pAnimType, Coords);
		auto const pData = SWTypeExtContainer::Instance.Find(pThis->Type);
		pAnim->Invisible = !pData->IsAnimVisible(pThis->Owner);
		pAnim->SetHouse(pThis->Owner);
		pThis->Animation = pAnim;
		PointerExpiredNotification::NotifyInvalidAnim->Add(pThis);
	}
}

LightingColor SWTypeExtData::GetLightingColor(SuperWeaponTypeClass* pCustom)
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

		if (auto  pSuper = SW_NuclearMissile::CurrentNukeType)
		{
			pType = pSuper;
		}
	}
	else if (LightningStorm::IsActive)
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
		SWTypeExtContainer::Instance.Find(pSW)->UpdateLightingColor(ret);
	}

	return ret;
}

bool SWTypeExtData::UpdateLightingColor(LightingColor& Lighting) const
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

bool SWTypeExtData::ChangeLighting(SuperWeaponTypeClass* pCustom)
{
	auto lighting = SWTypeExtData::GetLightingColor(pCustom);

	if (lighting.HasValue)
	{
		auto scen = ScenarioClass::Instance();
		scen->AmbientTarget = lighting.Ambient;
		scen->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, 1);
		return true;
	}

	return false;
}

void SWTypeExtData::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	BuildingExtData::LimboDeliver(pType, pOwner, ID);
}

// SW.Next proper launching mechanic
void SWTypeExtData::Launch(SuperClass* pFired, HouseClass* pHouse, SWTypeExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell, bool IsPlayer)
{
	const auto pSuper = pHouse->Supers.GetItemOrDefault(pLaunchedType);

	if (!pSuper || !pSuper->Type)
		return;

	const auto pSuperTypeExt = SWTypeExtContainer::Instance.Find(pSuper->Type);

	bool can_fire = true;

	if (pLauncherTypeExt->SW_Next_RealLaunch) {
		if (!pSuper->CanFire() || !SWTypeExtData::IsResourceAvailable(pSuper))
			can_fire = false;
	}

	if (can_fire)
	{
		if ((pLauncherTypeExt->SW_Next_IgnoreInhibitors || !pSuperTypeExt->HasInhibitor(pHouse, cell))
			&& (pLauncherTypeExt->SW_Next_IgnoreDesignators || pSuperTypeExt->HasDesignator(pHouse, cell)))
		{
			int oldstart = pSuper->RechargeTimer.StartTime;
			int oldleft = pSuper->RechargeTimer.TimeLeft;
			pSuper->SetReadiness(true);
			pSuper->Launch(cell, IsPlayer);
			pSuper->Reset();

			if (!pLauncherTypeExt->SW_Next_RealLaunch) {
				pSuper->RechargeTimer.StartTime = oldstart;
				pSuper->RechargeTimer.TimeLeft = oldleft;
			}
		}
	}
}

template <typename T>
void SWTypeExtData::Serialize(T& Stm)
{
	Stm
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

		.Process(this->SW_Next)
		.Process(this->SW_Next_RealLaunch)
		.Process(this->SW_Next_IgnoreInhibitors)
		.Process(this->SW_Next_IgnoreDesignators)
		.Process(this->SW_Next_RollChances)
		.Process(this->SW_Next_RandomWeightsData)

		.Process(this->SW_Inhibitors)
		.Process(this->SW_AnyInhibitor)
		.Process(this->SW_Designators)
		.Process(this->SW_AnyDesignator)
		.Process(this->ShowDesignatorRange)
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
		.Process(this->SW_AutoFire_CheckAvail)
		.Process(this->SW_AllowPlayer)
		.Process(this->SW_AllowAI)
		.Process(this->SW_AffectsHouse)
		.Process(this->SW_AnimVisibility)
		.Process(this->SW_AnimHeight)
		.Process(this->SW_ChargeToDrainRatio)
		.Process(this->HandledType)
		.Process(this->LastAction)
		.Process(this->Converts_UseSWRange)
		.Process(this->ConvertsPair)
		.Process(this->Convert_SucceededAnim)
		.Process(this->Nuke_Payload)
		.Process(this->Nuke_PsiWarning)
		.Process(this->Nuke_TakeOff)
		.Process(this->Nuke_SiloLaunch)
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
		.Process(this->Chronosphere_AffectUnwarpable)
		.Process(this->Chronosphere_AffectIronCurtain)
		.Process(this->Chronosphere_BlowUnplaceable)
		.Process(this->Chronosphere_ReconsiderBuildings)
		.Process(this->Chronosphere_Delay)
		.Process(this->Chronosphere_KillCargo)

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
		.Process(this->Droppod_RetryCount)
		.Process(this->DroppodProp)

		.Process(this->EMPField_Duration)
		.Process(this->SW_MaxCount)

		.Process(this->EMPulse_Linked)
		.Process(this->EMPulse_TargetSelf)
		.Process(this->EMPulse_PulseDelay)
		.Process(this->EMPulse_PulseBall)
		.Process(this->EMPulse_Cannons)
		.Process(this->EMPulse_SuspendOthers)
		.Process(this->EMPulse_WeaponIndex)

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

		.Process(This()->ImpatientVoice)
		.Process(this->EVA_InsufficientFunds)
		.Process(this->EVA_InsufficientBattlePoints)
		.Process(this->EVA_SelectTarget)

		.Process(this->Message_InsufficientFunds)
		.Process(this->Message_InsufficientBattlePoints)
		.Process(this->Message_Impatient)
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
		.Process(this->SW_DeliverableCounts)
		.Process(this->SW_Deliverables_Facing)
		.Process(this->SW_DeliverBuildups)
		.Process(this->SW_BaseNormal)
		.Process(this->SW_OwnerHouse)
		.Process(this->SW_DeliverableScatter)

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

		.Process(this->IonCannon_Ripple)
		.Process(this->IonCannon_Blast)
		.Process(this->IonCannon_Beam)
		.Process(this->IonCannon_BlastHeight)
		.Process(this->IonCannon_BeamHeight)
		.Process(this->IonCannon_FireAtPercentage)

#pragma region LaserStrike
		.Process(this->LaserStrikeDuration)
		.Process(this->LaserStrikeRadius)
		.Process(this->LaserStrikeMax)
		.Process(this->LaserStrikeMin)
		.Process(this->LaserStrikeMaxRadius)
		.Process(this->LaserStrikeMinRadius)
		.Process(this->LaserStrikeRadiusReduce)
		.Process(this->LaserStrikeRadiusReduceAcceleration)
		.Process(this->LaserStrikeRadiusReduceMax)
		.Process(this->LaserStrikeRadiusReduceMin)
		.Process(this->LaserStrikeROF)
		.Process(this->LaserStrikeScatter_Max)
		.Process(this->LaserStrikeScatter_Min)
		.Process(this->LaserStrikeScatter_Max_IncreaseMax)
		.Process(this->LaserStrikeScatter_Max_IncreaseMin)
		.Process(this->LaserStrikeScatter_Max_Increase)
		.Process(this->LaserStrikeScatter_Min_IncreaseMax)
		.Process(this->LaserStrikeScatter_Min_IncreaseMin)
		.Process(this->LaserStrikeScatter_Min_Increase)
		.Process(this->LaserStrikeLines)
		.Process(this->LaserStrikeAngle)
		.Process(this->LaserStrikeAngleAcceleration)
		.Process(this->LaserStrikeAngleMax)
		.Process(this->LaserStrikeAngleMin)
		.Process(this->LaserStrikeZeroRadius_Weapon)
		.Process(this->LaserStrikeInnerColor)
		.Process(this->LaserStrikeOuterColor)
		.Process(this->LaserStrikeOuterSpread)
		.Process(this->LaserStrikeLaserDuration)
		.Process(this->LaserStrikeLaserHeight)
		.Process(this->LaserStrikeThickness)
		.Process(this->LaserStrikeRate)
#pragma endregion

#pragma region GenericWarheadSW
		.Process(this->Generic_Warhead_Detonate)
#pragma endregion

		.Process(this->UseWeeds)
		.Process(this->UseWeeds_Amount)
		.Process(this->UseWeeds_StorageTimer)
		.Process(this->UseWeeds_ReadinessAnimationPercentage)

		.Process(this->CrateGoodies)
		.Process(this->SuperWeaponSidebar_Allow)
		.Process(this->SuperWeaponSidebar_PriorityHouses)
		.Process(this->SuperWeaponSidebar_RequiredHouses)
		.Process(this->TabIndex)
		.Process(this->BattlePoints_Amount)
		.Process(this->BattlePoints_DrainAmount)
		.Process(this->BattlePoints_DrainDelay)
		.Process(this->SuperWeaponSidebar_Significance)

		.Process(this->SW_Link)
		.Process(this->SW_Link_Grant)
		.Process(this->SW_Link_Ready)
		.Process(this->SW_Link_Reset)
		.Process(this->SW_Link_RandomWeightsData)
		.Process(this->SW_Link_RollChances)
		.Process(this->Message_LinkedSWAcquired)
		.Process(this->EVA_LinkedSWAcquired)

		.Process(this->Music_Theme)
		.Process(this->Music_Duration)
		.Process(this->Music_AffectedHouses)
		;

}

void SWTypeExtData::ApplyBattlePoints(SuperClass* pSW)
{
	HouseExtContainer::Instance.Find(pSW->Owner)->UpdateBattlePoints
		(SWTypeExtContainer::Instance.Find(pSW->Type)->BattlePoints_Amount);
}

bool SWTypeExtData::IsResourceAvailable(SuperClass* pSuper)
{
	const auto pExt = SWTypeExtContainer::Instance.Find(pSuper->Type);
	const auto pHouseExt = HouseExtContainer::Instance.Find(pSuper->Owner);

	if(!pSuper->Owner->CanTransactMoney(pExt->Money_Amount.Get()))
		return false;

	if (!pHouseExt->CanTransactBattlePoints(pExt->BattlePoints_Amount)) {
		return false;
	}

	return true;
}

void SWTypeExtData::ApplyLinkedSW(SuperClass* pSW)
{
	const auto pHouse = pSW->Owner;
	const bool notObserver = !pHouse->IsObserver() || !pHouse->IsCurrentPlayerObserver();

	if (pHouse->Defeated || !notObserver)
		return;

	auto linkedSW = [=](const int swIdxToAdd)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(swIdxToAdd))
			{
				const bool granted = this->SW_Link_Grant && !pSuper->Granted && pSuper->Grant(true, false, false);
				bool isActive = granted;

				if (pSuper->Granted)
				{
					// check SW.Link.Reset first
					if (this->SW_Link_Reset)
					{
						pSuper->Reset();
						isActive = true;
					}
					// check SW.Link.Ready, which will default to SW.InitialReady for granted superweapon
					else if (this->SW_Link_Ready || (granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_InitialReady))
					{
						pSuper->RechargeTimer.TimeLeft = 0;
						pSuper->SetReadiness(true);
						isActive = true;
					}
					// reset granted superweapon if it doesn't meet above conditions
					else if (granted)
					{
						pSuper->Reset();
					}
				}

				if (granted && notObserver && pHouse->IsCurrentPlayer())
				{
					if (MouseClass::Instance->AddCameo(AbstractType::Special, swIdxToAdd))
						MouseClass::Instance->RepaintSidebar(1);
				}

				return isActive;
			}

			return false;
		};

	bool isActive = false;

	// random mode
	if (this->SW_Link_RandomWeightsData.size())
	{
		const auto results = this->WeightedRollsHandler(&this->SW_Link_RollChances, &this->SW_Link_RandomWeightsData, this->SW_Link.size());

		for (const int &result : results)
		{
			if (linkedSW(this->SW_Link[result]))
				isActive = true;
		}
	}

	// no randomness mode
	else
	{
		for (const auto swType : this->SW_Link)
		{
			if (linkedSW(swType))
				isActive = true;
		}
	}

	if (isActive && notObserver && pHouse->IsCurrentPlayer())
	{
		if (this->EVA_LinkedSWAcquired.isset())
			VoxClass::PlayIndex(this->EVA_LinkedSWAcquired.Get(), VoxType::none, VoxPriority::none);

		this->PrintMessage(this->Message_LinkedSWAcquired, HouseClass::CurrentPlayer());
	}
}
// =============================
// container

SWTypeExtContainer SWTypeExtContainer::Instance;
std::vector<SWTypeExtData*> Container<SWTypeExtData>::Array;
SuperWeaponTypeClass* SWTypeExtData::CurrentSWType;
SuperClass* SWTypeExtData::TempSuper;
bool SWTypeExtData::Handled;
SuperClass* SWTypeExtData::LauchData;

void SWTypeExtContainer::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(SWTypeExtData::TempSuper, ptr);
	AnnounceInvalidPointer(SWTypeExtData::LauchData, ptr);
}

bool SWTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new SWTypeExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "SWTypeExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}


	const bool First = Stm
		.Process(SWTypeExtData::CurrentSWType)
		.Process(SWTypeExtData::TempSuper)
		.Process(SWTypeExtData::Handled)
		.Process(SWTypeExtData::LauchData)
		.Success();

	return First && NewSWType::LoadGlobals(Stm);
}

bool SWTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array)
	{
		// write old pointer and name, then delegate
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	const bool First = Stm
		.Process(SWTypeExtData::CurrentSWType)
		.Process(SWTypeExtData::TempSuper)
		.Process(SWTypeExtData::Handled)
		.Process(SWTypeExtData::LauchData)
		.Success();

	return First && NewSWType::SaveGlobals(Stm);
}

void SWTypeExtContainer::Clear()
{
	Array.clear();
	SWTypeExtData::LauchData = nullptr;
	SWTypeExtData::TempSuper = nullptr;
}

// =============================
// container hooks

ASMJIT_PATCH(0x6CE6F2, SuperWeaponTypeClass_CTOR, 0x5)
{
	GET(SuperWeaponTypeClass*, pItem, EBP);

	NewSWType::Init();
	SWTypeExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x6CEFE0, SuperWeaponTypeClass_SDDTOR, 0x8)
{
	GET(SuperWeaponTypeClass*, pItem, ECX);
	SWTypeExtContainer::Instance.Remove(pItem);
	return 0;
}

bool FakeSuperWeaponTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->SuperWeaponTypeClass::LoadFromINI(pINI);
	SWTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F40F4, FakeSuperWeaponTypeClass::_ReadFromINI)

HRESULT __stdcall FakeSuperWeaponTypeClass::_Load(IStream* pStm)
{
	auto hr = this->SuperWeaponTypeClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = SWTypeExtContainer::Instance.ReadDataFromTheByteStream(this,
			SWTypeExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeSuperWeaponTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->SuperWeaponTypeClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = SWTypeExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F40A4, FakeSuperWeaponTypeClass::_Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7F40A8, FakeSuperWeaponTypeClass::_Save)