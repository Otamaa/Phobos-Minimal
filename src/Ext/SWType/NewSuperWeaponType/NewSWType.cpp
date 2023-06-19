#include "NewSWType.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include "SonarPulse.h"
#include "UnitDelivery.h"
#include "GenericWarhead.h"
#include "Firewall.h"
#include "Protect.h"
#include "Reveal.h"
#include "ParaDrop.h"
#include "SpyPlane.h"
#include "ChronoSphere.h"
#include "ChronoWarp.h"
#include "GeneticMutator.h"
#include "Dominator.h"
#include "LightningStorm.h"
#include "NuclearMissile.h"
#include "HunterSeeker.h"
#include "DropPod.h"
#include "EMPulse.h"
#include "Battery.h"
#include "EMPField.h"
#include "IonCannon.h"

#include <Misc/AresData.h>

//#include "ChemLauncher.h"
//#include "MultiLauncher.h"

std::array<std::unique_ptr<NewSWType>, (size_t)AresNewSuperType::count> NewSWType::Array;

bool NewSWType::CanFireAt(TargetingData const& data, CellStruct const& cell, bool manual) const
{
	if (!data.TypeExt->CanFireAt(data.Owner, cell, manual)) {
		return false;
	}

	if (data.NeedsLaunchSite && std::none_of(data.LaunchSites.begin(),
		data.LaunchSites.end(), [cell](TargetingData::LaunchSite const& site)
		{
			auto const distance = cell.DistanceFrom(site.Center);

			// negative range values just pass the test
			return (site.MinRange < 0.0 || distance >= site.MinRange)
				&& (site.MaxRange < 0.0 || distance <= site.MaxRange);
		}))
	{
		return false;
	}

	if (data.NeedsDesignator && std::none_of(data.Designators.begin(),
		data.Designators.end(), [cell](TargetingData::RangedItem const& site)
		{
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	if (std::any_of(data.Inhibitors.begin(), data.Inhibitors.end(),
		[cell](TargetingData::RangedItem const& site)
		{
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	return true;
}

bool NewSWType::IsDesignator(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated) {
		if (pTechno->GetOwningHouse() == pOwner) {
			return pData->SW_AnyDesignator
				|| pData->SW_Designators.Contains(pTechno->GetTechnoType());
		}
	}

	return false;
}

bool NewSWType::HasDesignator(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not require designators
	if (pData->SW_Designators.empty() && !pData->SW_AnyDesignator) {
		return true;
	}

	// a single designator in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &Coords](TechnoClass* pTechno) {
		return IsDesignatorEligible(pData, pOwner, Coords, pTechno);
	});
}

bool NewSWType::IsDesignatorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const {
	if (IsDesignator(pData, pOwner, pTechno)) {
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno)) {
			center = BuildingExt::GetCenterCoords(pBuilding);
		}

		// has to be closer than the designator range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool NewSWType::IsInhibitor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated) {
		if (!pOwner->IsAlliedWith(pTechno)) {
			if (auto pBld = abstract_cast<BuildingClass*>(pTechno)) {
				if (!pBld->IsPowerOnline()) {
					return false;
				}
			}

			return pData->SW_AnyInhibitor
				|| pData->SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}

	return false;
}

bool NewSWType::HasInhibitor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not allow inhibitors
	if (pData->SW_Inhibitors.empty() && !pData->SW_AnyInhibitor) {
		return false;
	}

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &Coords](TechnoClass* pTechno) {
		return IsInhibitorEligible(pData, pOwner, Coords, pTechno);
		});
}

bool NewSWType::IsInhibitorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const {
	if (IsInhibitor(pData, pOwner, pTechno)) {
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno)) {
			center = BuildingExt::GetCenterCoords(pBuilding);
		}

		// has to be closer than the inhibitor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

SWRange NewSWType::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range;
}

WarheadTypeClass* NewSWType::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Warhead.Get(nullptr);
}

AnimTypeClass* NewSWType::GetAnim(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Anim.Get(nullptr);
}

int NewSWType::GetSound(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Sound.Get(-1);
}

int NewSWType::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(0);
}

bool NewSWType::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline()) {

		if (pBuilding->TemporalTargetingMe || pBuilding->IsBeingWarpedOut())
			return false;

		return BuildingExt::ExtMap.Find(pBuilding)->HasSuperWeapon(pData->OwnerObject()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> NewSWType::GetLaunchSiteRange(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	return { pData->SW_RangeMinimum.Get(), pData->SW_RangeMaximum.Get() };
}

static bool NewSWTypeInited = false;

std::unique_ptr<const TargetingData> NewSWType::GetTargetingData(SWTypeExt::ExtData* pData, HouseClass* pOwner) const
{
	auto data = std::make_unique<TargetingData>(pData, pOwner);

	// get launchsite data
	auto const&[minRange , MaxRange] = this->GetLaunchSiteRange(pData);

	if (minRange >= 0.0 || minRange >= 0.0) {
		data->NeedsLaunchSite = true;

		for (auto const& pBld : pOwner->Buildings) {
			if (this->IsLaunchSite(pData, pBld)) {
				auto const range = this->GetLaunchSiteRange(pData, pBld);
				auto const center = CellClass::Coord2Cell(BuildingExt::GetCenterCoords(pBld));

				data->LaunchSites.emplace_back(TargetingData::LaunchSite{
					pBld, center, range.first, range.second });
			}
		}
	}

	// get designator data
	if (!pData->SW_Designators.empty() || pData->SW_AnyDesignator) {
		data->NeedsDesignator = true;

		for (auto const& pTechno : *TechnoClass::Array) {
			if (this->IsDesignator(pData, pOwner, pTechno)) {
				// get the designator's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno)) {
					center = BuildingExt::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExt::ExtMap.Find(pType);
				auto const range = pExt->DesignatorRange.Get(pType->Sight);

				if (range > 0) {
					data->Designators.emplace_back(TargetingData::RangedItem{
						range* range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	// get inhibitor data
	if (!pData->SW_Inhibitors.empty() || pData->SW_AnyInhibitor) {
		for (auto const& pTechno : *TechnoClass::Array) {
			if (this->IsInhibitor(pData, pOwner, pTechno)) {
				// get the inhibitor's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno)) {
					center = BuildingExt::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExt::ExtMap.Find(pType);
				auto const range = pExt->InhibitorRange.Get(pType->Sight);

				if (range > 0) {
					data->Inhibitors.emplace_back(TargetingData::RangedItem{
						range* range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	return std::unique_ptr<const TargetingData>(std::move(data));
}

bool NewSWType::CanFireAt(SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const
{
	auto const data = this->GetTargetingData(pData, pOwner);
	return this->CanFireAt(*data, cell, manual);
}

void NewSWType::Init()
{
	if (NewSWTypeInited)
		return;

	SW_Firewall_Type = SuperWeaponType((int)AresNewSuperType::Firestorm + (int)SuperWeaponType::count);

	NewSWTypeInited = true;
#define RegSW(name ,type) Register(std::make_unique<name>(), type);

	RegSW(SW_SonarPulse , AresNewSuperType::SonarPulse)
	RegSW(SW_UnitDelivery, AresNewSuperType::UnitDelivery)
	RegSW(SW_GenericWarhead, AresNewSuperType::GenericWarhead)
	RegSW(SW_Firewall, AresNewSuperType::Firestorm)
	RegSW(SW_Protect, AresNewSuperType::Protect)
	RegSW(SW_Reveal, AresNewSuperType::Reveal)
	RegSW(SW_ParaDrop, AresNewSuperType::ParaDrop)
	RegSW(SW_SpyPlane, AresNewSuperType::SpyPlane)
	RegSW(SW_ChronoSphere, AresNewSuperType::ChronoSphere)
	RegSW(SW_ChronoWarp, AresNewSuperType::ChronoWarp)
	RegSW(SW_GeneticMutator, AresNewSuperType::GeneticMutator)
	RegSW(SW_PsychicDominator, AresNewSuperType::PsychicDominator)
	RegSW(SW_LightningStorm, AresNewSuperType::LightningStorm)
	RegSW(SW_NuclearMissile, AresNewSuperType::NuclearMissile)
	RegSW(SW_HunterSeeker, AresNewSuperType::HunterSeeker)
	RegSW(SW_DropPod, AresNewSuperType::DropPod)
	RegSW(SW_EMPulse, AresNewSuperType::EMPulse)
	RegSW(SW_Battery, AresNewSuperType::Battery)
	//Adding new SW Type is not advisable atm, since we need to handle absolutely everyhing 
	//otherwise the game will crash with out of bound array ,..
	//RegSW(SW_EMPField, AresNewSuperType::EMPField)
	//RegSW(SW_IonCannon, AresNewSuperType::IonCannon)
	//RegSW(SW_ChemLauncher, AresNewSuperType::ChemLauncher)
	//RegSW(SW_MultiLauncher, AresNewSuperType::MultiLauncher)
#undef RegSW
}

bool NewSWType::IsOriginalType(SuperWeaponType nType)
{
	return nType < SuperWeaponType::count;
}

bool NewSWType::LoadGlobals(PhobosStreamReader& Stm)
{
	for (const auto& ptr : Array) {
		Stm.RegisterChange(ptr.get());
	}

	return Stm.Success();
}

bool NewSWType::SaveGlobals(PhobosStreamWriter& Stm)
{
	for (const auto& ptr : Array) {
		Stm.Save(ptr.get());
	}

	return Stm.Success();
}

NewSWType* NewSWType::GetNthItem(SuperWeaponType i)
{
	return Array[static_cast<size_t>(i) - (size_t)SuperWeaponType::count].get();
}

SuperWeaponType NewSWType::GetHandledType(SuperWeaponType nType)
{
	auto It = std::find_if(Array.begin(), Array.end(), [&](const auto& Item) {
		return Item->HandleThisType(nType);
	});

	if (It != Array.end())
		return SuperWeaponType((int)SuperWeaponType::count + (int)(*It)->TypeIndex);

	return SuperWeaponType::Invalid;
}

NewSWType* NewSWType::GetNewSWType(const SWTypeExt::ExtData* pData)
{
	SuperWeaponType iDx = pData->HandledType != SuperWeaponType::Invalid ? pData->HandledType : pData->OwnerObject()->Type;
	return iDx >= SuperWeaponType::count ? NewSWType::GetNthItem(iDx) : nullptr;
}

SuperWeaponType NewSWType::FindFromTypeID(const char* pType)
{
	auto It = std::find_if(Array.begin(), Array.end(), [pType](const std::unique_ptr<NewSWType>& item) {
		const auto& pIDs = item->GetTypeString();

		if (!pIDs.empty()) {
			for (auto& Id : pIDs) {
				//ares usin strcmp , so i just follow it here
				if (!strcmp(Id, pType)) {
					return true;
				}
			}
		}

		return false;
	});

	if (It != Array.end()) {
		return static_cast<SuperWeaponType>(
			(size_t)SuperWeaponType::count + (size_t)(*It)->TypeIndex);
	}

	return SuperWeaponType::Invalid;
}

bool NewSWType::IsLaunchSiteEligible(SWTypeExt::ExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pSWType, pBuilding)) {
		return false;
	}

	if (ignoreRange) {
		return true;
	}

	// get the range for this building
	auto const& [minRange, maxRange] = this->GetLaunchSiteRange(pSWType, pBuilding);
	const auto center = CellClass::Coord2Cell(BuildingExt::GetCenterCoords(pBuilding));
	const auto distance = Coords.DistanceFrom(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

bool NewSWType::HasLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const
{
	// the quick way out: no range restriction at all
	auto const&[minRange, maxRange] = this->GetLaunchSiteRange(pSWType);

	if (minRange < 0.0 && maxRange < 0.0) {
		return true;
	}

	return std::any_of(pOwner->Buildings.begin(), pOwner->Buildings.end(), [=, &Coords](BuildingClass* pBld) {
	 return this->IsLaunchSiteEligible(pSWType, Coords, pBld, false);
	});
}
