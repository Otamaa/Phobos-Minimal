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
#include "MeteorShower.h"
#include "LaserStrike.h"

std::array<std::unique_ptr<NewSWType>, (size_t)AresNewSuperType::count> NewSWType::Array;

bool NewSWType::CanFireAt(const TargetingData* pTargeting, CellStruct const& cell, bool manual) const
{
	if (!pTargeting->TypeExt->CanFireAt(pTargeting->Owner, cell, manual)) {
		return false;
	}

	if (pTargeting->NeedsLaunchSite && pTargeting->LaunchSites.none_of
		([cell](TargetingData::LaunchSite const& site) {
			auto const distance = cell.DistanceFrom(site.Center);

			// negative range values just pass the test
			return (site.MinRange < 0.0 || distance >= site.MinRange)
				&& (site.MaxRange < 0.0 || distance <= site.MaxRange);
		}))
	{
		return false;
	}

	if (pTargeting->NeedsDesignator && pTargeting->Designators.none_of
		([cell](TargetingData::RangedItem const& site) {
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	//Enemy Designator
	if (pTargeting->NeedsAttractors && pTargeting->Attractors.none_of
		([cell](TargetingData::RangedItem const& site) {
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	if (pTargeting->NeedsInhibitors && pTargeting->Inhibitors.any_of
		([cell](TargetingData::RangedItem const& site) {
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	//Enemy Inhibitors
	if (pTargeting->NeedsSupressors && pTargeting->Suppressors.any_of
		([cell](TargetingData::RangedItem const& site) {
			auto const distance = cell.DistanceFromSquared(site.Center);
			return distance <= site.RangeSqr;
		}))
	{
		return false;
	}

	return true;
}

bool NewSWType::IsDesignator(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive
		&& pTechno->Health
		&& !pTechno->InLimbo
		&& !pTechno->Deactivated
		&& pTechno->GetOwningHouse() == pOwner
		)
	{
		return pData->SW_AnyDesignator
			|| pData->SW_Designators.Contains(pTechno->GetTechnoType());
	}

	return false;
}

bool NewSWType::HasDesignator(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not require designators
	if (pData->SW_Designators.empty() && !pData->SW_AnyDesignator)
	{
		return true;
	}

	// a single designator in range suffices
	return TechnoClass::Array->any_of([=, &Coords](TechnoClass* pTechno) {
		return IsDesignatorEligible(pData, pOwner, Coords, pTechno);
	});
}

bool NewSWType::IsDesignatorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsDesignator(pData, pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = specific_cast<BuildingClass*>(pTechno)) {
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the designator range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool NewSWType::IsInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (pOwner && pOwner->IsAlliedWith(pTechno)) {
			return false;
		}

		if (auto pBld = specific_cast<BuildingClass*>(pTechno)) {
			if (!pBld->IsPowerOnline()) {
				return false;
			}
		}

		return pData->SW_AnyInhibitor
				|| pData->SW_Inhibitors.Contains(pTechno->GetTechnoType());

	}

	return false;
}

bool NewSWType::HasInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not allow inhibitors
	if (pData->SW_Inhibitors.empty() && !pData->SW_AnyInhibitor) {
		return false;
	}

	// a single inhibitor in range suffices
	return TechnoClass::Array->any_of([=, &Coords](TechnoClass* pTechno) {
		 return IsInhibitorEligible(pData, pOwner, Coords, pTechno);
	});
}

bool NewSWType::IsInhibitorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsInhibitor(pData, pOwner, pTechno)) {

		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = specific_cast<BuildingClass*>(pTechno)) {
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the inhibitor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool NewSWType::IsAttractor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner != pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo)
	{
		return pData->SW_AnyAttractor
			|| pData->SW_Attractors.Contains(pTechno->GetTechnoType());
	}

	return false;
}

bool NewSWType::HasAttractor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not require Attractor
	if (pData->SW_Attractors.empty() && !pData->SW_AnyAttractor)
	{
		return true;
	}

	// a single Attractor in range suffices
	return TechnoClass::Array->any_of([=, &Coords](TechnoClass* pTechno) {
		return IsAttractorEligible(pData, pOwner, Coords, pTechno);
	});
}

bool NewSWType::IsAttractorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsAttractor(pData, pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the Attractor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
		{
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the Attractor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->AttractorRange.Get(pType->Sight);
	}

	return false;
}

bool NewSWType::IsSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (pOwner && pOwner->IsAlliedWith(pTechno)) {
			return false;
		}

		if (auto pBld = specific_cast<BuildingClass*>(pTechno)) {
			if (!pBld->IsPowerOnline())
			{ return false; }
		}

		return pData->SW_AnySuppressor
				|| pData->SW_Suppressors.Contains(pTechno->GetTechnoType());
	}

	return false;
}

bool NewSWType::HasSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
{
	// does not allow Suppressor
	if (pData->SW_Suppressors.empty() && !pData->SW_AnySuppressor) {
		return false;
	}

	// a single Suppressor in range suffices
	return TechnoClass::Array->any_of([=, &Coords](TechnoClass* pTechno)
		{ return IsSuppressorEligible(pData, pOwner, Coords, pTechno); });
}

bool NewSWType::IsSuppressorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsSuppressor(pData, pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the Suppressor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
		{
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the Suppressor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->SuppressorRange.Get(pType->Sight);
	}

	return false;
}

SWRange NewSWType::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range;
}

WarheadTypeClass* NewSWType::GetWarhead(const SWTypeExtData* pData) const
{
	return pData->SW_Warhead.Get(nullptr);
}

AnimTypeClass* NewSWType::GetAnim(const SWTypeExtData* pData) const
{
	return pData->SW_Anim.Get(nullptr);
}

int NewSWType::GetSound(const SWTypeExtData* pData) const
{
	return pData->SW_Sound.Get(-1);
}

int NewSWType::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(0);
}

bool NewSWType::IsLaunchsiteAlive(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		//const auto nMission = pBuilding->GetCurrentMission();

		//if (nMission == Mission::Selling || nMission == Mission::Construction)
		//	return false;

		if (pBuilding->TemporalTargetingMe || pBuilding->IsBeingWarpedOut())
			return false;

		return true;
	}

	return false;
}

bool NewSWType::IsSWTypeAttachedToThis(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	return  BuildingExtContainer::Instance.Find(pBuilding)->HasSuperWeapon(pData->AttachedToObject->ArrayIndex, true);;
}

bool NewSWType::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	return BuildingExtContainer::Instance.Find(pBuilding)->HasSuperWeapon(pData->AttachedToObject->ArrayIndex, true);
}

std::pair<double, double> NewSWType::GetLaunchSiteRange(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	return { pData->SW_RangeMinimum.Get(), pData->SW_RangeMaximum.Get() };
}

static bool NewSWTypeInited = false;

std::unique_ptr<const TargetingData> NewSWType::GetTargetingData(SWTypeExtData* pData, HouseClass* pOwner) const
{
	auto data = std::make_unique<TargetingData>(pData, pOwner);

	// get launchsite data
	auto const& [minRange, MaxRange] = this->GetLaunchSiteRange(pData);

	if (minRange >= 0.0 || MaxRange >= 0.0)
	{
		data->NeedsLaunchSite = true;

		pOwner->Buildings.for_each([&](BuildingClass* pBld) {
			if (this->IsLaunchSite(pData, pBld)) {

				auto const range = this->GetLaunchSiteRange(pData, pBld);
				auto const center = CellClass::Coord2Cell(BuildingExtData::GetCenterCoords(pBld));

				data->LaunchSites.emplace_back(pBld, center, range.first, range.second);
			}
		});
	}

	if ((!pData->SW_Designators.empty() || pData->SW_AnyDesignator))
	{
		data->NeedsDesignator = true;

		for (auto const& pTechno : *TechnoClass::Array) {
			if (this->IsDesignator(pData, pOwner, pTechno))
			{
				// get the designator's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->DesignatorRange.Get(pType->Sight);

				if (range > 0)
				{
					data->Designators.emplace_back(TargetingData::RangedItem {
						range * range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	if ((!pData->SW_Attractors.empty() || pData->SW_AnyAttractor))
	{
		data->NeedsAttractors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsAttractor(pData, pOwner, pTechno))
			{
				// get the designator's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->AttractorRange.Get(pType->Sight);

				if (range > 0)
				{
					data->Attractors.emplace_back(TargetingData::RangedItem {
						range * range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	if (!pData->SW_Inhibitors.empty() || pData->SW_AnyInhibitor)
	{
		data->NeedsInhibitors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsInhibitor(pData, pOwner, pTechno))
			{
				// get the inhibitor's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->InhibitorRange.Get(pType->Sight);

				if (range > 0)
				{
					data->Inhibitors.emplace_back(TargetingData::RangedItem {
						range * range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	if (!pData->SW_Suppressors.empty() || pData->SW_AnySuppressor)
	{
		data->NeedsSupressors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsSuppressor(pData, pOwner, pTechno))
			{
				// get the inhibitor's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = specific_cast<BuildingClass*>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = pTechno->GetTechnoType();
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->SuppressorRange.Get(pType->Sight);

				if (range > 0)
				{
					data->Suppressors.emplace_back(TargetingData::RangedItem {
						range * range, CellClass::Coord2Cell(center) });
				}
			}
		}
	}

	return std::unique_ptr<const TargetingData>(std::move(data));
}

bool NewSWType::CanFireAt(SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const
{
	const auto& data = this->GetTargetingData(pData, pOwner);
	return this->CanFireAt(data.get(), cell, manual);
}

#include <Ext/Super/Body.h>
TechnoClass* NewSWType::GetFirer(SuperClass* pSW, const CellStruct& Coords, bool ignoreRange) const
{
	TechnoClass* pFirer = nullptr;
	auto const pData = SWTypeExtContainer::Instance.Find(pSW->Type);
	//const auto pFirer_e = SuperExtContainer::Instance.Find(pSW)->Firer;

	for (auto const& pBld : pSW->Owner->Buildings)
	{
		if (this->IsLaunchSiteEligible(pData, Coords, pBld, false))
		{
			pFirer = pBld;
			break;
		}
	}

	//if (!pFirer && pFirer_e)
	//	pFirer = pFirer_e;

	return pFirer;
}

bool NOINLINE NewSWType::CanHaveLauchSite(SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	//switch (this->TypeIndex)
	//{
	//case AresNewSuperType::LightningStorm:
	//case AresNewSuperType::PsychicDominator:
	//case AresNewSuperType::SonarPulse:
	//case AresNewSuperType::UnitDelivery:
	//case AresNewSuperType::GenericWarhead:
	//case AresNewSuperType::Protect:
	//case AresNewSuperType::Reveal:
	//case AresNewSuperType::ParaDrop:
	//case AresNewSuperType::SpyPlane:
	//case AresNewSuperType::IonCannon:
	//case AresNewSuperType::MeteorShower:
	//case AresNewSuperType::EMPField:
	//	if (!pData->SW_Lauchsites.empty()
	//		&& pData->SW_Lauchsites.Contains(pBuilding->Type))
	//		return true;
	//}

	return false;
}

void NewSWType::Init()
{
	if (NewSWTypeInited)
		return;

	SW_Firewall::FirewallType = SuperWeaponType((int)AresNewSuperType::Firestorm + (int)SuperWeaponType::count);

	NewSWTypeInited = true;
#define RegSW(name ,type) Register(std::make_unique<name>(), type);

		RegSW(SW_SonarPulse, AresNewSuperType::SonarPulse)
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
		RegSW(SW_EMPField, AresNewSuperType::EMPField)
		RegSW(SW_IonCannon, AresNewSuperType::IonCannon)
		RegSW(SW_MeteorShower, AresNewSuperType::MeteorShower)
		RegSW(SW_LaserStrike , AresNewSuperType::LaserStrike)
#undef RegSW
}

bool NewSWType::IsOriginalType(SuperWeaponType nType)
{
	return nType < SuperWeaponType::count;
}

bool NewSWType::LoadGlobals(PhobosStreamReader& Stm)
{
	for (const auto& ptr : Array)
	{
		Stm.RegisterChange(ptr.get());
	}

	return Stm.Success();
}

bool NewSWType::SaveGlobals(PhobosStreamWriter& Stm)
{
	for (const auto& ptr : Array)
	{
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
	const auto It = std::find_if(Array.begin(), Array.end(),
		[&](const auto& Item) {
			return Item && Item->HandleThisType(nType);
		}
	);

	if (It != Array.end())
		return SuperWeaponType((int)SuperWeaponType::count + (int)(*It)->TypeIndex);

	return SuperWeaponType::Invalid;
}

NewSWType* NewSWType::GetNewSWType(const SWTypeExtData* pData)
{
	SuperWeaponType iDx = pData->HandledType != SuperWeaponType::Invalid ?
		pData->HandledType : pData->AttachedToObject->Type;

	return iDx >= SuperWeaponType::count ? NewSWType::GetNthItem(iDx) : nullptr;
}

NewSWType* NewSWType::GetNewSWType(const SuperClass* pSuper)
{
	return NewSWType::GetNewSWType(SWTypeExtContainer::Instance.Find(pSuper->Type));
}

SuperWeaponType NewSWType::FindFromTypeID(const char* pType)
{
	if(!*pType || !strlen(pType))
		return SuperWeaponType::Invalid;

	const auto It = std::find_if(Array.begin(), Array.end(),
		[pType](const std::unique_ptr<NewSWType>& item) {

			if (!item)
				return false;

			for (const auto& Id : item->GetTypeString()) {
				//ares usin strcmp , so i just follow it here
				if (IS_SAME_STR_N(Id, pType)) {
					return true;
				}
			}

			return false;
		}
	);

	if (It != Array.end()) {
		return static_cast<SuperWeaponType>(
			(size_t)SuperWeaponType::count + (size_t)(*It)->TypeIndex);
	}

	return SuperWeaponType::Invalid;
}

bool NOINLINE NewSWType::IsLaunchSiteEligible(SWTypeExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pSWType, pBuilding) /*&& !this->CanHaveLauchSite(pSWType, pBuilding)*/)
	{
		return false;
	}

	if (ignoreRange) {
		return true;
	}

	// get the range for this building
	auto const& [minRange, maxRange] = this->GetLaunchSiteRange(pSWType, pBuilding);
	const auto center = CellClass::Coord2Cell(BuildingExtData::GetCenterCoords(pBuilding));
	const auto distance = Coords.DistanceFrom(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

bool NewSWType::HasLaunchSite(SWTypeExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const
{
	// the quick way out: no range restriction at all
	auto const& [minRange, maxRange] = this->GetLaunchSiteRange(pSWType);

	if (minRange < 0.0 && maxRange < 0.0)
	{
		return true;
	}

	return pOwner->Buildings.any_of
		([=, &Coords](BuildingClass* pBld)
		{ return this->IsLaunchSiteEligible(pSWType, Coords, pBld, false); }
	);
}
