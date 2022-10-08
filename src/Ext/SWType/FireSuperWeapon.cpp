#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include "Ext/Building/Body.h"
#include "Ext/House/Body.h"
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

// Too big to be kept in ApplyLimboDelivery
void LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);

	// BuildLimit check goes before creation
	if (pType->BuildLimit > 0)
	{
		int sum = pOwner->CountOwnedNow(pType);

		// copy Ares' deployable units x build limit fix
		if (auto const pUndeploy = pType->UndeploysInto)
			sum += pOwner->CountOwnedNow(pUndeploy);

		if (sum >= pType->BuildLimit)
			return;
	}

	BuildingClass* pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner));

	// All of these are mandatory
	pBuilding->InLimbo = false;
	pBuilding->IsAlive = true;
	pBuilding->IsOnMap = true;
	pOwner->RegisterGain(pBuilding, false);
	pOwner->UpdatePower();
	pOwner->RecheckTechTree = true;
	pOwner->RecheckPower = true;
	pOwner->RecheckRadar = true;
	pOwner->Buildings.AddItem(pBuilding);

	// increment limbo build count
	if (pOwnerExt)
		pOwnerExt->OwnedLimboBuildingTypes.Increment(pType->ArrayIndex);

	// Different types of building logics
	if (pType->ConstructionYard)
		pOwner->ConYards.AddItem(pBuilding); // why would you do that????

	if (pType->SecretLab)
		pOwner->SecretLabs.AddItem(pBuilding);

	if (pType->FactoryPlant)
	{
		pOwner->FactoryPlants.AddItem(pBuilding);
		pOwner->CalculateCostMultipliers();
	}

	if (pType->OrePurifier)
		pOwner->NumOrePurifiers++;

	// BuildingClass::Place is where Ares hooks secret lab expansion
	// pTechnoBuilding->Place(false);
	// even with it no bueno yet, plus new issues
	// probably should just port it from Ares 0.A and be done

	// LimboKill init
	auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
	auto const pTechnoExt = TechnoExt::ExtMap.Find(pBuilding);

	if (ID != -1){
		pBuildingExt->LimboID = ID;
#ifdef COMPILE_PORTED_DP_FEATURES
		pTechnoExt->PaintBallState.release();
#endif
		pBuildingExt->IsInLimboDelivery = true;
	}
}

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse,const CoordStruct& coords , bool IsCurrentPlayer)
{
	if(!pHouse) {
		Debug::Log("SW[%x] Trying To execute %s with nullptr HouseOwner ! \n" , pSW , "FireSuperWeapon");
		return;
	}

	if (this->LimboDelivery_Types.size())
		ApplyLimboDelivery(pHouse);

	if (this->LimboKill_IDs.size())
		ApplyLimboKill(pHouse);

	if (this->Detonate_Warhead.isset() || this->Detonate_Weapon.isset())
		this->ApplyDetonation(pSW->Owner, coords);
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		bool rollOnce = false;
		int id = -1;
		size_t rolls = this->LimboDelivery_RollChances.size();
		size_t weights = this->LimboDelivery_RandomWeightsData.size();
		size_t ids = this->LimboDelivery_IDs.size();
		size_t index;
		size_t j;

		// if no RollChances are supplied, do only one roll
		if (rolls == 0)
		{
			rolls = 1;
			rollOnce = true;
		}

		for (size_t i = 0; i < rolls; i++)
		{
			this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
			if (!rollOnce && this->RandomBuffer > this->LimboDelivery_RollChances[i])
				continue;

			j = rolls > weights ? weights: i;
			index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, this->LimboDelivery_RandomWeightsData[j]);

			// extra weights are bound to automatically fail
			if (index >= this->LimboDelivery_Types.size())
				index = size_t(-1);

			if (index != -1)
			{
				if (index < ids)
					id = this->LimboDelivery_IDs[index];

				LimboDeliver(this->LimboDelivery_Types[index], pHouse, id);
			}
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
	for (unsigned int i = 0; i < this->LimboKill_IDs.size(); i++)
	{
		for (HouseClass* pTargetHouse : *HouseClass::Array())
		{
			if (EnumFunctions::CanTargetHouse(this->LimboKill_Affected, pHouse, pTargetHouse))
			{
				for (const auto& pBuilding : pTargetHouse->Buildings)
				{
					const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
					if (pBuildingExt->LimboID == this->LimboKill_IDs[i])
					{
						BuildingTypeClass* pType = static_cast<BuildingTypeClass*>(pBuilding->Type);

						// Mandatory
						pBuilding->InLimbo = true;
						pBuilding->IsAlive = false;
						pBuilding->IsOnMap = false;
						pTargetHouse->RegisterLoss(pBuilding, false);
						pTargetHouse->UpdatePower();
						//pTargetHouse->RecheckTechTree = true;
						pTargetHouse->RecheckPower = true;
						pTargetHouse->RecheckRadar = true;
						pTargetHouse->Buildings.Remove(pBuilding);

						// Building logics
						if (pType->ConstructionYard)
							pTargetHouse->ConYards.Remove(pBuilding);

						if (pType->SecretLab)
							pTargetHouse->SecretLabs.Remove(pBuilding);

						if (pType->FactoryPlant)
						{
							pTargetHouse->FactoryPlants.Remove(pBuilding);
							pTargetHouse->CalculateCostMultipliers();
						}

						if (pType->OrePurifier)
							pTargetHouse->NumOrePurifiers--;

						// Remove completely
						TechnoExt::HandleRemove(pBuilding);
						//pBuilding->UnInit();
					}
				}
			}
		}
	}
}

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CoordStruct& coords)
{
	const auto pCell = MapClass::Instance->GetCellAt(coords);
	const auto cell = pCell->MapCoords;
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pHouse->Buildings) {
		if (this->IsLaunchSiteEligible(cell, pBld, false)) {
			pFirer = pBld;
			break;
		}
	}

	const auto pWeapon = this->Detonate_Weapon.isset() ? this->Detonate_Weapon.Get() : nullptr;

	if (pWeapon)
		WeaponTypeExt::DetonateAt(pWeapon, coords, pFirer, this->Detonate_Damage.Get(pWeapon->Damage));
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), coords, pFirer, this->Detonate_Damage.Get(0));
}

bool SWTypeExt::ExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pBuilding))
	{
		return false;
	}

	if (ignoreRange)
	{
		return true;
	}

	// get the range for this building
	auto range = this->GetLaunchSiteRange(pBuilding);
	const auto& minRange = range.first;
	const auto& maxRange = range.second;

	const auto center = CellClass::Coord2Cell(BuildingExt::GetCenterCoords(pBuilding));
	const auto distance = Coords.DistanceFrom(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

bool SWTypeExt::ExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		return BuildingExt::ExtMap.Find(pBuilding)->HasSuperWeapon(this->Get()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return std::make_pair(this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get());
}

bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse) const
{
	const auto pThis = this->Get();

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
	{
		return false;
	}

	// allow only certain houses, disallow forbidden houses
	const auto OwnerBits = 1u << pHouse->Type->ArrayIndex;
	if (!(this->SW_RequiredHouses & OwnerBits) || (this->SW_ForbiddenHouses & OwnerBits))
	{
		return false;
	}

	// check that any aux building exist and no neg building
	auto IsBuildingPresent = [pHouse](BuildingTypeClass* pType)
	{
		return pType && pHouse->CountOwnedAndPresent(pType) > 0;
	};

	const auto& Aux = this->SW_AuxBuildings;
	if (!Aux.empty() && std::none_of(Aux.begin(), Aux.end(), IsBuildingPresent))
	{
		return false;
	}

	const auto& Neg = this->SW_NegBuildings;
	if (std::any_of(Neg.begin(), Neg.end(), IsBuildingPresent))
	{
		return false;
	}

	return true;
}