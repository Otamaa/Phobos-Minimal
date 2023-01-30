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
	auto const pOwnerExt = HouseExt::ExtMap.Find<true>(pOwner);

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

	if (ID != -1){
		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
		auto const pTechnoExt = TechnoExt::ExtMap.Find(pBuilding);
		auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoExt->Type);

		pBuildingExt->LimboID = ID;
#ifdef COMPILE_PORTED_DP_FEATURES
		pTechnoExt->PaintBallState.release();
#endif
		const bool peacefulDeath = pTechnoTypeExt->Death_Peaceful.Get();
		const auto nKillMethod = peacefulDeath ? KillMethod::Vanish : pTechnoTypeExt->Death_Method.Get();

		if (nKillMethod != KillMethod::None && pTechnoTypeExt->Death_Countdown > 0)
		{
			pTechnoExt->Death_Countdown.Start(pTechnoTypeExt->Death_Countdown);
			pOwnerExt->AutoDeathObjects.push_back(pBuilding);
		}
	}
}

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer)
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
		this->ApplyDetonation(pSW->Owner, *pCell);

	if (this->SW_Next.size() > 0)
		this->ApplySWNext(pSW, *pCell);
}

// Universal handler of the rolls-weights system
std::vector<int> SWTypeExt::ExtData::WeightedRollsHandler(ValueableVector<float>* rolls, ValueableVector<ValueableVector<int>>* weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls->size();
	size_t weightsSize = weights->size();
	int index;
	std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && this->RandomBuffer > (*rolls)[i])
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = std::min(weightsSize - 1, i);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, (*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			indices.push_back(index);
	}
	return indices;
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		auto results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());
		for (size_t result : results)
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

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	const auto pCell = MapClass::Instance->GetCellAt(cell);
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pHouse->Buildings) {
		if (this->IsLaunchSiteEligible(cell, pBld, false)) {
			pFirer = pBld;
			break;
		}
	}

	const auto pWeapon = this->Detonate_Weapon.isset() ? this->Detonate_Weapon.Get() : nullptr;

	if (pWeapon)
		WeaponTypeExt::DetonateAt(pWeapon, pCell->GetCoords(), pFirer, this->Detonate_Damage.Get(pWeapon->Damage));
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), pCell->GetCoords(), pFirer, this->Detonate_Damage.Get(0));
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

// SW.Next proper launching mechanic
void Launch(HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, SuperWeaponTypeClass* pLaunchedType, const CellStruct& cell)
{
	const auto pSuper = pHouse->Supers.GetItem(SuperWeaponTypeClass::Array->FindItemIndex(pLaunchedType));

	if (!pSuper)
		return;

	const auto pSuperTypeExt = SWTypeExt::ExtMap.Find(pLaunchedType);
	if (!pLauncherTypeExt->SW_Next_RealLaunch || (pSuperTypeExt && pSuper->IsCharged && pHouse->CanTransactMoney(pSuperTypeExt->Money_Amount)))
	{

		if (pLauncherTypeExt->SW_Next_IgnoreInhibitors || !pSuperTypeExt->HasInhibitor(pHouse, cell)
			&& (pLauncherTypeExt->SW_Next_IgnoreDesignators || pSuperTypeExt->HasDesignator(pHouse, cell)))
		{
			// Forcibly fire
			pSuper->Launch(cell, true);
			if (pLauncherTypeExt->SW_Next_RealLaunch)
				pSuper->Reset();
		}

	}
}

void SWTypeExt::ExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// random mode
	if (this->SW_Next_RandomWeightsData.size())
	{
		auto results = this->WeightedRollsHandler(&this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (int result : results)
			Launch(pSW->Owner, this, this->SW_Next[result], cell);
	}
	// no randomness mode
	else
	{
		for (const auto pSWType : this->SW_Next)
			Launch(pSW->Owner, this, pSWType, cell);
	}
}