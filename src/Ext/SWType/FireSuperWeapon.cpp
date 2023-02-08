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
void SWTypeExt::LimboDeliver(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	BuildingExt::LimboDeliver(pType, pOwner, ID);
}

std::vector<int> SWTypeExt::WeightedRollsHandler(Valueable<double>& RandomBuffer, const ValueableVector<float>& rolls, const ValueableVector<ValueableVector<int>>& weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls.size();
	size_t weightsSize = weights.size();
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
		RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && RandomBuffer > rolls[i])
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = std::min(weightsSize - 1, i);
		index = GeneralUtils::ChooseOneWeighted(RandomBuffer, weights[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			indices.push_back(index);
	}

	return indices;
}

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, const CellStruct* const pCell, bool IsCurrentPlayer)
{
	if (!pHouse)
	{
		Debug::Log("SW[%x] Trying To execute %s with nullptr HouseOwner ! \n", pSW, "FireSuperWeapon");
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
std::vector<int> SWTypeExt::ExtData::WeightedRollsHandler(std::vector<float>* rolls, std::vector<std::vector<int>>* weights, size_t size)
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
		const auto results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());
		for (size_t result : results)
		{
			if (result < idsSize)
				id = this->LimboDelivery_IDs.at(result);

			LimboDeliver(this->LimboDelivery_Types.at(result), pHouse, id);
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
				id = this->LimboDelivery_IDs.at(i);

			LimboDeliver(this->LimboDelivery_Types.at(i), pHouse, id);
		}
	}
}

void SWTypeExt::ExtData::ApplyLimboKill(HouseClass* pHouse)
{
	if (this->LimboKill_IDs.empty())
		return;

	for (HouseClass* pTargetHouse : *HouseClass::Array())
	{
		if (!EnumFunctions::CanTargetHouse(this->LimboKill_Affected, pHouse, pTargetHouse))
			continue;

		for (const auto& pBuilding : pTargetHouse->Buildings)
		{
			const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

			if (pBuildingExt->LimboID <= -1)
				continue;

			if (this->LimboKill_IDs.Contains(pBuildingExt->LimboID)) {
				BuildingExt::LimboKill(pBuilding);
			}
		}
	}
}

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	const auto pCell = Map[cell];
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pHouse->Buildings)
	{
		if (this->IsLaunchSiteEligible(cell, pBld, false))
		{
			pFirer = pBld;
			break;
		}
	}

	if (const auto pWeapon = this->Detonate_Weapon.Get(nullptr))
		WeaponTypeExt::DetonateAt(pWeapon, pCell->GetCoords(), pFirer, this->Detonate_Damage.Get(this->SW_Damage.Get(pWeapon->Damage)));
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), pCell->GetCoords(), pFirer, this->Detonate_Damage.Get(this->SW_Damage.Get(0)));
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
	const auto& [minRange, maxRange] = this->GetLaunchSiteRange(pBuilding);
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

		if (pBuilding->TemporalTargetingMe || pBuilding->IsBeingWarpedOut())
			return false;

		return BuildingExt::ExtMap.Find(pBuilding)->HasSuperWeapon(this->Get()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return { this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get() };
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
void Launch(HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, int pLaunchedType, const CellStruct& cell)
{
	const auto pSuper = pHouse->Supers.GetItemOrDefault(pLaunchedType);

	if (!pSuper)
		return;

	const auto pSuperTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
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
		const auto results = this->WeightedRollsHandler(&this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (const int& result : results)
		{
			Launch(pSW->Owner, this, this->SW_Next.at(result), cell);
		}
	}
	// no randomness mode
	else
	{
		for (const auto& pSWType : this->SW_Next)
			Launch(pSW->Owner, this, pSWType, cell);
	}
}