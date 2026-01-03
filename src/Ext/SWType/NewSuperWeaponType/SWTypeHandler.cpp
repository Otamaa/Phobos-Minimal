#include "SWTypeHandler.h"

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

#include <SWRange.h>

TypeContainer TypeContainer::Instance;

bool SWTypeHandler::CanTargetingFireAt(const TargetingData* pTargeting, CellStruct const& cell, bool manual) const
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
			auto designatorDistance = cell.DistanceFromSquared(site.Center);
			return designatorDistance <= site.RangeSqr;
		}))
	{
		return false;
	}

	//Enemy Designator
	if (pTargeting->NeedsAttractors && pTargeting->Attractors.none_of
		([cell](TargetingData::RangedItem const& site) {
			return cell.DistanceFromSquared(site.Center) <= site.RangeSqr;
		}))
	{
		return false;
	}

	if (pTargeting->NeedsInhibitors && pTargeting->Inhibitors.any_of
		([cell](TargetingData::RangedItem const& site) {
			return cell.DistanceFromSquared(site.Center) <= site.RangeSqr;
		}))
	{
		return false;
	}

	//Enemy Inhibitors
	if (pTargeting->NeedsSupressors && pTargeting->Suppressors.any_of
		([cell](TargetingData::RangedItem const& site) {
			return cell.DistanceFromSquared(site.Center) <= site.RangeSqr;
		}))
	{
		return false;
	}

	return true;
}

bool SWTypeHandler::IsDesignator(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive
		&& pTechno->Health
		&& !pTechno->InLimbo
		&& !pTechno->Deactivated
		)
	{
		if(!EnumFunctions::CanTargetHouse(pData->SW_Designators_Houses, pOwner, pTechno->Owner))
			return false;

		return pData->SW_AnyDesignator
			|| pData->SW_Designators.Contains(GET_TECHNOTYPE(pTechno));
	}

	return false;
}

 bool SWTypeHandler::HasDesignator(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
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

bool SWTypeHandler::IsDesignatorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsDesignator(pData, pOwner, pTechno))
	{
		const auto pType = GET_TECHNOTYPE(pTechno);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno)) {
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the designator range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeHandler::IsInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (!EnumFunctions::CanTargetHouse(pData->SW_Inhibitors_Houses, pOwner, pTechno->Owner)) {
			return false;
		}

		if (auto pBld = cast_to<BuildingClass*, false>(pTechno)) {
			if (!pBld->IsPowerOnline()) {
				return false;
			}
		}

		return pData->SW_AnyInhibitor
				|| pData->SW_Inhibitors.Contains(GET_TECHNOTYPE(pTechno));

	}

	return false;
}

 bool SWTypeHandler::HasInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
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

bool SWTypeHandler::IsInhibitorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsInhibitor(pData, pOwner, pTechno)) {

		const auto pType = GET_TECHNOTYPE(pTechno);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno)) {
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the inhibitor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeHandler::IsAttractor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner != pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo)
	{
		return pData->SW_AnyAttractor
			|| pData->SW_Attractors.Contains(GET_TECHNOTYPE(pTechno));
	}

	return false;
}

 bool SWTypeHandler::HasAttractor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
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

bool SWTypeHandler::IsAttractorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsAttractor(pData, pOwner, pTechno))
	{
		const auto pType = GET_TECHNOTYPE(pTechno);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the Attractor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
		{
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the Attractor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->AttractorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeHandler::IsSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (pOwner && pOwner->IsAlliedWith(pTechno)) {
			return false;
		}

		if (auto pBld = cast_to<BuildingClass*, false>(pTechno)) {
			if (!pBld->IsPowerOnline())
			{ return false; }
		}

		return pData->SW_AnySuppressor
				|| pData->SW_Suppressors.Contains(GET_TECHNOTYPE(pTechno));
	}

	return false;
}

bool SWTypeHandler::IsDesignatorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType) const
{
	if (EnumFunctions::CanTargetHouse(pData->SW_Designators_Houses, pSWOwner, pTechnoOwner)) {
		return pData->SW_AnyDesignator
			|| pData->SW_Designators.Contains(pTechnoType);
	}

	return false;
}

bool SWTypeHandler::IsInhibitorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType, bool bIsPoweredOffline) const
{
	if(!bIsPoweredOffline) {
		if (!EnumFunctions::CanTargetHouse(pData->SW_Inhibitors_Houses, pSWOwner, pTechnoOwner)) {
			return false;
		}

		return pData->SW_AnyInhibitor
			|| pData->SW_Inhibitors.Contains(pTechnoType);
	}

	return false;
}

bool SWTypeHandler::IsAttractorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType) const
{
	if (pTechnoOwner != pSWOwner)
	{
		return pData->SW_AnyAttractor
			|| pData->SW_Attractors.Contains(pTechnoType);
	}

	return false;
}

bool SWTypeHandler::IsSuppressorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType, bool bIsPoweredOffline) const
{
	if (!pSWOwner->IsAlliedWith(pTechnoOwner) && !bIsPoweredOffline) {
		return pData->SW_AnySuppressor
			|| pData->SW_Suppressors.Contains(pTechnoType);
	}

	return false;
}

 bool SWTypeHandler::HasSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const
 {
 	// does not allow Suppressor
 	if (pData->SW_Suppressors.empty() && !pData->SW_AnySuppressor) {
 		return false;
 	}

 	// a single Suppressor in range suffices
 	return TechnoClass::Array->any_of([=, &Coords](TechnoClass* pTechno)
 		{ return IsSuppressorEligible(pData, pOwner, Coords, pTechno); });
 }

bool SWTypeHandler::IsSuppressorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const
{
	if (IsSuppressor(pData, pOwner, pTechno))
	{
		const auto pType = GET_TECHNOTYPE(pTechno);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// get the Suppressor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
		{
			center = BuildingExtData::GetCenterCoords(pBuilding);
		}

		// has to be closer than the Suppressor range (which defaults to Sight)
		auto distance = Coords.DistanceFrom(CellClass::Coord2Cell(center));
		return distance <= pExt->SuppressorRange.Get(pType->Sight);
	}

	return false;
}

SWRange SWTypeHandler::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range;
}

WarheadTypeClass* SWTypeHandler::GetWarhead(const SWTypeExtData* pData) const
{
	return pData->SW_Warhead.Get(nullptr);
}

AnimTypeClass* SWTypeHandler::GetAnim(const SWTypeExtData* pData) const
{
	return pData->SW_Anim.Get(nullptr);
}

int SWTypeHandler::GetSound(const SWTypeExtData* pData) const
{
	return pData->SW_Sound.Get(-1);
}

int SWTypeHandler::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(0);
}

bool SWTypeHandler::IsLaunchsiteAlive(BuildingClass* pBuilding) const
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

bool SWTypeHandler::IsSWTypeAttachedToThis(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	return  BuildingExtContainer::Instance.Find(pBuilding)->HasSuperWeapon(pData->This()->ArrayIndex, true);
}

void SWTypeHandler::PlayAnim(SuperClass* pSuper, CoordStruct& coord) {

	const auto pCurrentSWTypeData = SWTypeExtContainer::Instance.Find(pSuper->Type); //previous data
	const auto flags = this->Flags(pCurrentSWTypeData);
	const bool bPlayAnim = (flags & SuperWeaponFlags::NoAnim) == SuperWeaponFlags::None;
	const bool bPlaySound = (flags & SuperWeaponFlags::NoSound) == SuperWeaponFlags::None;

	if (bPlayAnim || bPlaySound)
	{
		auto pCell = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(coord));
		auto nCoord = pCell->GetCoordsWithBridge();

		if (bPlayAnim)
		{
			if (auto pAnim = this->GetAnim(pCurrentSWTypeData))
			{
				nCoord.Z += pCurrentSWTypeData->SW_AnimHeight;
				AnimClass* placeholder = GameCreate<AnimClass>(pAnim, nCoord);
				placeholder->SetHouse(pSuper->Owner);
				placeholder->Invisible = !pCurrentSWTypeData->IsAnimVisible(pSuper->Owner);
			}
		}

		if (bPlaySound)
		{
			VocClass::SafeImmedietelyPlayAt(this->GetSound(pCurrentSWTypeData), &nCoord, nullptr);
		}
	}
}

bool SWTypeHandler::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	return BuildingExtContainer::Instance.Find(pBuilding)->HasSuperWeapon(pData->This()->ArrayIndex, true);
}

std::pair<double, double> SWTypeHandler::GetLaunchSiteRange(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	return { pData->SW_RangeMinimum.Get(), pData->SW_RangeMaximum.Get() };
}

static bool NewSWTypeInited = false;

std::unique_ptr<TargetingData> SWTypeHandler::GetTargetingData(SWTypeExtData* pData, HouseClass* pOwner) const
{
	auto pResult = std::make_unique<TargetingData>(pData, pOwner);

	// get launchsite data
	auto const& [minRange, MaxRange] = std::make_pair(pData->SW_RangeMinimum.Get(), pData->SW_RangeMaximum.Get());

	if (minRange >= 0.0 || MaxRange >= 0.0)
	{
		pResult->NeedsLaunchSite = true;

		pOwner->Buildings.for_each([&](BuildingClass* pBld) {
			if (this->IsLaunchSite(pData, pBld)) {

				auto const range = this->GetLaunchSiteRange(pData, pBld);
				auto const center = CellClass::Coord2Cell(BuildingExtData::GetCenterCoords(pBld));

				pResult->LaunchSites.emplace_back(pBld, center, range.first, range.second);
			}
		});
	}
#ifdef _old
	pResult->NeedsDesignator = (!pData->SW_Designators.empty() || pData->SW_AnyDesignator);
	pResult->NeedsAttractors = (!pData->SW_Attractors.empty() || pData->SW_AnyAttractor);
	pResult->NeedsInhibitors = (!pData->SW_Inhibitors.empty() || pData->SW_AnyInhibitor);
	pResult->NeedsSupressors = (!pData->SW_Suppressors.empty() || pData->SW_AnySuppressor);

	 if(pResult->NeedsDesignator || pResult->NeedsAttractors || pResult->NeedsInhibitors || pResult->NeedsSupressors){
		TechnoClass::Array->for_each([this , pData , &pResult, pOwner](TechnoClass* pTechno) {
			if(pTechno && pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && !pTechno->Deactivated) {

				// get the designator's center
				auto center = pTechno->GetCoords();
				bool isPoweOffline = false;

				if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno)) {
					center = BuildingExtData::GetCenterCoords(pBuilding);
					isPoweOffline = !pBuilding->IsPowerOnline();
				}

				const auto pType = GET_TECHNOTYPE(pTechno);
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				const auto pTechnoOwner = pTechno->GetOwningHouse();

				auto thecenter = CellClass::Coord2Cell(center);

				if(pResult->NeedsDesignator && this->IsDesignatorSimple(pData, pOwner, pTechnoOwner, pType)) {
					auto const range = pExt->DesignatorRange.Get(pType->Sight);

					if (range > 0) {
						pResult->Designators.emplace_back(range * range, thecenter);
					}
				}

				if (pResult->NeedsAttractors && this->IsAttractorSimple(pData, pOwner, pTechnoOwner, pType)){
					auto const range = pExt->AttractorRange.Get(pType->Sight);

					if (range > 0) {
						pResult->Attractors.emplace_back(range * range, thecenter);
					}
				}

				if (pResult->NeedsInhibitors && this->IsInhibitorSimple(pData, pOwner, pTechnoOwner, pType , isPoweOffline)) {
					auto const range = pExt->InhibitorRange.Get(pType->Sight);

					if (range > 0) {
						pResult->Inhibitors.emplace_back(range * range, thecenter);
					}
				}

				if (pResult->NeedsSupressors && this->IsSuppressorSimple(pData, pOwner, pTechnoOwner, pType, isPoweOffline)) {
					auto const range = pExt->SuppressorRange.Get(pType->Sight);

					if (range > 0) {
						pResult->Suppressors.emplace_back(range * range, thecenter);
					}
				}
			}
		});
	 }

#else
	if ((!pData->SW_Designators.empty() || pData->SW_AnyDesignator))
	{
		pResult->NeedsDesignator = true;

		for (auto const& pTechno : *TechnoClass::Array) {
			if (this->IsDesignator(pData, pOwner, pTechno))
			{
				// get the designator's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = GET_TECHNOTYPE(pTechno);
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->DesignatorRange.Get(pType->Sight);

				if (range > 0)
				{
					pResult->Designators.emplace_back(range * range, CellClass::Coord2Cell(center));
				}
			}
		}
	}

	if ((!pData->SW_Attractors.empty() || pData->SW_AnyAttractor))
	{
		pResult->NeedsAttractors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsAttractor(pData, pOwner, pTechno))
			{
				// get the designator's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = GET_TECHNOTYPE(pTechno);
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->AttractorRange.Get(pType->Sight);

				if (range > 0)
				{
					pResult->Attractors.emplace_back(range * range, CellClass::Coord2Cell(center));
				}
			}
		}
	}

	if (!pData->SW_Inhibitors.empty() || pData->SW_AnyInhibitor)
	{
		pResult->NeedsInhibitors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsInhibitor(pData, pOwner, pTechno))
			{
				// get the inhibitor's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = GET_TECHNOTYPE(pTechno);
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->InhibitorRange.Get(pType->Sight);

				if (range > 0)
				{
					pResult->Inhibitors.emplace_back(range * range, CellClass::Coord2Cell(center));
				}
			}
		}
	}

	if (!pData->SW_Suppressors.empty() || pData->SW_AnySuppressor)
	{
		pResult->NeedsSupressors = true;

		for (auto const& pTechno : *TechnoClass::Array)
		{
			if (this->IsSuppressor(pData, pOwner, pTechno))
			{
				// get the inhibitor's center
				auto center = pTechno->GetCoords();
				if (auto pBuilding = cast_to<BuildingClass*, false>(pTechno))
				{
					center = BuildingExtData::GetCenterCoords(pBuilding);
				}

				const auto pType = GET_TECHNOTYPE(pTechno);
				const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);
				auto const range = pExt->SuppressorRange.Get(pType->Sight);

				if (range > 0)
				{
					pResult->Suppressors.emplace_back(range * range, CellClass::Coord2Cell(center));
				}
			}
		}
	}
#endif
	return std::move(pResult);
}

bool SWTypeHandler::CanFireAt(SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const
{
	auto ptr = this->GetTargetingData(pData, pOwner);
	return this->CanTargetingFireAt(ptr.get(), cell, manual);
}

#include <Ext/Super/Body.h>
TechnoClass* SWTypeHandler::GetFirer(SuperClass* pSW, const CellStruct& Coords, bool ignoreRange) const
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

bool SWTypeHandler::CanHaveLauchSite(SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	//switch (this->TypeIndex)
	//{
	//case NewSuperType::LightningStorm:
	//case NewSuperType::PsychicDominator:
	//case NewSuperType::SonarPulse:
	//case NewSuperType::UnitDelivery:
	//case NewSuperType::GenericWarhead:
	//case NewSuperType::Protect:
	//case NewSuperType::Reveal:
	//case NewSuperType::ParaDrop:
	//case NewSuperType::SpyPlane:
	//case NewSuperType::IonCannon:
	//case NewSuperType::MeteorShower:
	//case NewSuperType::EMPField:
	//	if (!pData->SW_Lauchsites.empty()
	//		&& pData->SW_Lauchsites.Contains(pBuilding->Type))
	//		return true;
	//}

	return false;
}

SWTypeHandler* SWTypeHandler::get_Handler(NewSuperType i) {
	if (i == NewSuperType::Invalid){
		return nullptr;
	}

	return TypeContainer::Instance.Array[static_cast<size_t>(i)].get();

}

std::pair<SuperWeaponType, NewSuperType> SWTypeHandler::FindFromTypeID(const char* pType)
{
	if(pType){

		const auto It = std::ranges::find_if(TypeContainer::Instance.Array,
			[pType](const std::unique_ptr<SWTypeHandler>& item) {

				if (*pType == '\0')
					return false;

				for (const auto& Id : item->TypeStrings) {
					if (IS_SAME_STR_(Id.c_str() ,pType)) {
						return true;
					}
				}

				return false;
			}
		);

		if (It != std::ranges::end(TypeContainer::Instance.Array)) {
			return  { (*It)->GetSWType() , (*It)->TypeIndex };
		}
	}

	return  { SuperWeaponType::Invalid , NewSuperType::Invalid };
}

bool SWTypeHandler::IsLaunchSiteEligible(SWTypeExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
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

bool SWTypeHandler::HasLaunchSite(SWTypeExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const
{
	// the quick way out: no range restriction at all
	auto const& [minRange, maxRange] = std::make_pair(pSWType->SW_RangeMinimum.Get(), pSWType->SW_RangeMaximum.Get());

	if (minRange < 0.0 && maxRange < 0.0)
	{
		return true;
	}

	return pOwner->Buildings.any_of
		([=, &Coords](BuildingClass* pBld)
		{ return this->IsLaunchSiteEligible(pSWType, Coords, pBld, false); }
	);
}

// TypeContainer is always avaible on run time
// so just initialize once
TypeContainer::TypeContainer()
{
	SW_Firewall::FirewallType = SuperWeaponType((int)NewSuperType::Firestorm + (int)SuperWeaponType::count);

	NewSWTypeInited = true;
#define RegSW(name ,type , str) Register(std::make_unique<name>(), type , str);

		RegSW(SW_NuclearMissile, NewSuperType::Nuke , "NewNuke , ChemLauncher , MultiLauncher, MultiMissile")
		RegSW(SW_IronCurtain, NewSuperType::IronCurtain, "Protect , IronCurtain")
		RegSW(SW_LightningStorm, NewSuperType::LightningStorm , "NewLS, LightningStorm")
		RegSW(SW_ChronoSphere, NewSuperType::ChronoSphere , "ChronoSphere")
		RegSW(SW_ChronoWarp, NewSuperType::ChronoWarp, "ChronoWarp")
		RegSW(SW_ParaDrop, NewSuperType::ParaDrop , "NewParaDrop, ParaDrop")
		RegSW(SW_AmericanParaDrop, NewSuperType::AmerParaDrop, "AmerParaDrop , AmericanParaDrop")
		RegSW(SW_PsychicDominator, NewSuperType::PsychicDominator , "NewDominator , PsychicDominator , PsyDom")
		RegSW(SW_SpyPlane, NewSuperType::SpyPlane , "Airstrike , SpyPlane")
		RegSW(SW_GeneticMutator, NewSuperType::GeneticMutator, "GeneticMutator , GeneticConverter")
		RegSW(SW_ForceShield, NewSuperType::ForceShield, "ForceShield")
		RegSW(SW_Reveal, NewSuperType::PsychicReveal, "Reveal , PsychicReveal , SpySat")

		RegSW(SW_SonarPulse, NewSuperType::SonarPulse, "SonarPulse")
		RegSW(SW_UnitDelivery, NewSuperType::UnitDelivery, "UnitDelivery")
		RegSW(SW_GenericWarhead, NewSuperType::GenericWarhead, "GenericWarhead , Upgrade , Money , Animation")
		RegSW(SW_Firewall, NewSuperType::Firestorm, "Firestorm , FireWall")
		RegSW(SW_HunterSeeker, NewSuperType::HunterSeeker, "HunterSeeker")
		RegSW(SW_DropPod, NewSuperType::DropPod, "DropPod")
		RegSW(SW_EMPulse, NewSuperType::EMPulse, "EMPulse, Fire")
		RegSW(SW_Battery, NewSuperType::Battery, "Battery")
		RegSW(SW_EMPField, NewSuperType::EMPField, "EMPField")
		RegSW(SW_IonCannon, NewSuperType::IonCannon, "IonCannon")
		RegSW(SW_MeteorShower, NewSuperType::MeteorShower, "MeteorShower")
		RegSW(SW_LaserStrike, NewSuperType::LaserStrike, "LaserStrike")

#undef RegSW

}

void TypeContainer::Register(std::unique_ptr<SWTypeHandler> pType, NewSuperType nType, std::string_view typeStrings)
{
	pType->TypeIndex = nType;
	auto [arr, count] = PhobosCRT::splits<8>(typeStrings);
	pType->TypeStrings.assign(arr.begin(), arr.begin() + count);
	Array[size_t(nType)] = (std::move(pType));
}