#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Misc/Spawner/Main.h>
#include <Ext/Infantry/Body.h>

#include <Misc/Hooks.Otamaa.h>

#include <New/Type/GenericPrerequisite.h>
#include <New/Type/CrateTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Cast.h>

#include <ExtraHeaders/StackVector.h>

#include <ScenarioClass.h>

#pragma region defines
PhobosMap<TechnoClass*, KillMethod> HouseExtData::AutoDeathObjects;
HelperedVector<TechnoClass*> HouseExtData::LimboTechno;
PhobosMap<HouseClass*, VectorSet<TeamClass*>> HouseExtContainer::HousesTeams;

int HouseExtData::LastGrindingBlanceUnit;
int HouseExtData::LastGrindingBlanceInf;
int HouseExtData::LastHarvesterBalance;
int HouseExtData::LastSlaveBalance;

CDTimerClass HouseExtData::CloakEVASpeak;
CDTimerClass HouseExtData::SubTerraneanEVASpeak;

bool HouseExtData::IsAnyFirestormActive;

HouseClass* HouseExtContainer::Civilian = nullptr;
SideClass* HouseExtContainer::CivilianSide = nullptr;
HouseClass* HouseExtContainer::Special = nullptr;
HouseClass* HouseExtContainer::Neutral = nullptr;

#pragma endregion

void HouseExtData::InitializeTrackers(HouseClass* pHouse)
{
	//auto pExt = HouseExtContainer::Instance.Find(pHouse);
	//pExt->BuiltAircraftTypes.PopulateCounts(AircraftTypeClass::Array->Count);
	//pExt->BuiltInfantryTypes.PopulateCounts(InfantryTypeClass::Array->Count);
	//pExt->BuiltUnitTypes.PopulateCounts(UnitTypeClass::Array->Count);
	//pExt->BuiltBuildingTypes.PopulateCounts(BuildingTypeClass::Array->Count);
	//pExt->KilledAircraftTypes.PopulateCounts(AircraftTypeClass::Array->Count);
	//pExt->KilledInfantryTypes.PopulateCounts(InfantryTypeClass::Array->Count);
	//pExt->KilledUnitTypes.PopulateCounts(UnitTypeClass::Array->Count);
	//pExt->KilledBuildingTypes.PopulateCounts(BuildingTypeClass::Array->Count);
	//pExt->CapturedBuildings.PopulateCounts(BuildingTypeClass::Array->Count);
	//pExt->CollectedCrates.PopulateCounts(CrateTypeClass::Array.size());
}

// restored from TS
void FakeHouseClass::_GiveTiberium(float amount, int type)
{
	this->SiloMoney += int(amount * 5.0);

	if (SessionClass::Instance->GameMode == GameMode::Campaign || this->IsHumanPlayer)
	{
		// don't change, old values are needed for silo update
		const double lastStorage = (this->_GetExtData()->TiberiumStorage.GetAmounts());
		const auto lastTotalStorage = this->TotalStorage;
		const auto curStorage = (double)lastTotalStorage - lastStorage;
		double rest = 0.0;

		// this is the upper limit for stored tiberium
		if (amount > curStorage)
		{
			rest = amount - curStorage;
			amount = float(curStorage);
		}

		// go through all buildings and fill them up until all is in there
		for (auto const& pBuilding : this->Buildings)
		{
			if (amount <= 0.0)
			{
				break;
			}

			auto const storage = pBuilding->Type->Storage;
			if (pBuilding->IsOnMap && storage > 0)
			{
				auto storage_ = &TechnoExtContainer::Instance.Find(pBuilding)->TiberiumStorage;
				// put as much tiberium into this silo
				double freeSpace = (double)storage - storage_->GetAmounts();

				if (freeSpace > 0.0)
				{
					if (freeSpace > amount)
					{
						freeSpace = amount;
					}

					storage_->IncreaseAmount((float)freeSpace, type);
					this->_GetExtData()->TiberiumStorage.IncreaseAmount((float)freeSpace, type);
					amount -= (float)freeSpace;
				}
			}
		}

		if (RulesExtData::Instance()->GiveMoneyIfStorageFull)
		{
			amount += (float)rest;

			//no free space , just give the money ,..
			if (amount > 0.0)
			{
				auto const pTib = TiberiumClass::Array->Items[type];
				this->Balance += int(amount * pTib->Value * this->Type->IncomeMult);
			}
		}

		// redraw silos
		this->UpdateAllSilos((int)lastStorage, lastTotalStorage);
	}
	else
	{
		// just add the money. this is the only original YR logic
		auto const pTib = TiberiumClass::Array->Items[type];
		this->Balance += int(amount * pTib->Value * this->Type->IncomeMult);
	}
}

bool HouseExtData::IsMutualAllies(HouseClass const* pThis, HouseClass const* pHouse) {
	return pHouse == pThis
		|| (pThis->Allies.Contains(pHouse->ArrayIndex)
			&& pHouse->Allies.Contains(pThis->ArrayIndex));
}

float HouseExtData::GetRestrictedFactoryPlantMult(TechnoTypeClass* pTechnoType) const
{
	float mult = 1.0;
	auto const pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechnoType);

	for (auto const& pBuilding : this->RestrictedFactoryPlants)
	{
		auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

		if (!pTypeExt->FactoryPlant_AllowTypes.empty() && !pTypeExt->FactoryPlant_AllowTypes.Contains(pTechnoType))
			continue;

		if (!pTypeExt->FactoryPlant_DisallowTypes.empty()&& pTypeExt->FactoryPlant_DisallowTypes.Contains(pTechnoType))
			continue;

		float currentMult = 1.0f;

		switch (pTechnoType->WhatAmI())
		{
		case AbstractType::BuildingType:
			if (((BuildingTypeClass*)pTechnoType)->BuildCat == BuildCat::Combat)
				currentMult = pBuilding->Type->DefensesCostBonus;
			else
				currentMult = pBuilding->Type->BuildingsCostBonus;
			break;
		case AbstractType::AircraftType:
			currentMult = pBuilding->Type->AircraftCostBonus;
			break;
		case AbstractType::InfantryType:
			currentMult = pBuilding->Type->InfantryCostBonus;
			break;
		case AbstractType::UnitType:
			currentMult = pBuilding->Type->UnitsCostBonus;
			break;
		default:
			break;
		}

		mult *= currentMult;
	}

	return float(1.0f - ((1.0f - mult) * pTechnoTypeExt->FactoryPlant_Multiplier));
}

RequirementStatus HouseExtData::RequirementsMet(
	HouseClass* pHouse, TechnoTypeClass* pItem)
{
	const auto pData = TechnoTypeExtContainer::Instance.Find(pItem);
	const auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	const bool IsHuman = pHouse->IsControlledByHuman();
	// bool IsUnbuildable = pItem->Unbuildable || (IsHuman && pData->HumanUnbuildable);

	if (pItem->Unbuildable || (IsHuman && pData->HumanUnbuildable))
		return RequirementStatus::Forbidden;

	if(!(pData->Prerequisite_RequiredTheaters & (1 << static_cast<int>(ScenarioClass::Instance->Theater))))
		return RequirementStatus::Forbidden;

	if(Prereqs::HouseOwnsAny(pHouse, pData->Prerequisite_Negative.data(), pData->Prerequisite_Negative.size()))
		return RequirementStatus::Forbidden;

	for(auto pRever : pHouseExt->Reversed){
		if(pRever == pItem) {
			return RequirementStatus::Overridden;
		}
	}

	if (pData->RequiredStolenTech.any()) {
		if ((pHouseExt->StolenTech & pData->RequiredStolenTech) != pData->RequiredStolenTech) {
			return RequirementStatus::Incomplete;
		}
	}

	if (Prereqs::HouseOwnsAny(pHouse, pItem->PrerequisiteOverride))
		return RequirementStatus::Overridden;

	if (pHouse->HasFromSecretLab(pItem))
		return RequirementStatus::Overridden;

	if (IsHuman && pItem->TechLevel == -1)
		return RequirementStatus::Incomplete;

	if (!pHouse->HasAllStolenTech(pItem))
		return RequirementStatus::Incomplete;

	if (!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem))
		return RequirementStatus::Forbidden;

	if (!HouseExtData::CheckFactoryOwners(pHouse, pItem))
		return RequirementStatus::Incomplete;

	if (auto const pBldType = type_cast<BuildingTypeClass const*>(pItem)) {
		if (HouseExtData::IsDisabledFromShell(pHouse, pBldType)) {
			return RequirementStatus::Forbidden;
		}
	}

	if (pData->Prerequisite_Power.isset()) {
		if (pData->Prerequisite_Power <= 0) {
			if (-pData->Prerequisite_Power > pHouse->PowerOutput) {
				return RequirementStatus::Incomplete;
			}
			} else if (pData->Prerequisite_Power > pHouse->PowerOutput - pHouse->PowerDrain) {
				return RequirementStatus::Incomplete;
		}
	}

	return (pHouse->StaticData.TechLevel >= pItem->TechLevel) ?
		RequirementStatus::Complete : RequirementStatus::Incomplete;
}

std::pair<NewFactoryState, BuildingClass*> HouseExtData::HasFactory(
	HouseClass* pHouse,
	TechnoTypeClass* pType,
	bool bSkipAircraft,
	bool requirePower,
	bool bCheckCanBuild,
	bool b7)
{

	if (bCheckCanBuild && pHouse->CanBuild(pType, true, true) <= CanBuildResult::Unbuildable)
	{
		return { NewFactoryState::NoFactory  , nullptr };
	}

	auto const nWhat = pType->WhatAmI();
	auto const bitsOwners = pType->GetOwners();
	auto const isNaval = pType->Naval;
	//auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);
	BuildingClass* pNonPrimaryBuilding = nullptr;
	BuildingClass* pOfflineBuilding = nullptr;

	for (auto const& pBld : pHouse->Buildings)
	{

		if (pBld->InLimbo
			|| pBld->GetCurrentMission() == Mission::Selling
			|| pBld->QueuedMission == Mission::Selling
		)
		{
			continue;
		}

		auto const pBType = pBld->Type;

		if ((nWhat != pBType->Factory) || !pType->InOwners(bitsOwners))
			continue;

		if (!bSkipAircraft && (nWhat == AbstractType::AircraftType) && pBld->HasAnyLink())
		{
			if (!pBld->HasFreeLink())
			{
				continue;
			}
		}
		else if ((nWhat == AbstractType::UnitType) && (pBType->Naval != isNaval))
		{
			continue;
		}

		if (TechnoTypeExtData::CanBeBuiltAt(pType, pBType))
		{
			if (requirePower && (!pBld->HasPower || pBld->Deactivated))
			{
				pOfflineBuilding = pBld;
			}
			else
			{
				pNonPrimaryBuilding = pBld;
				if (pBld->IsPrimaryFactory)
					return { NewFactoryState::Available_Primary , pNonPrimaryBuilding };

				//do only single loop and use the pOfflineBuildingResult
				if (b7)
				{
					pOfflineBuilding = pBld;
					break;
				}
			}
		}
	}

	if (pNonPrimaryBuilding)
	{
		return { NewFactoryState::Available_Alternative  , pNonPrimaryBuilding };
	}
	else if (pOfflineBuilding)
	{
		return { NewFactoryState::Unpowered  , pOfflineBuilding };
	}

	return { NewFactoryState::NotFound , nullptr };
}

int HouseExtData::BuildBuildingLimitRemaining(HouseClass* pHouse, BuildingTypeClass* pItem)
{
	auto const BuildLimit = pItem->BuildLimit;

	if (BuildLimit >= 0)
		return BuildLimit - BuildingTypeExtData::GetUpgradesAmount(const_cast<BuildingTypeClass*>(pItem), const_cast<HouseClass*>(pHouse));
	else
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
}

int HouseExtData::CheckBuildingBuildLimit(HouseClass* pHouse, BuildingTypeClass* pItem, bool const includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	int BuildLimit = pItem->BuildLimit;
	int Remaining = HouseExtData::BuildBuildingLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem)) ? NotReached : ReachedPermanently;

	return Remaining > 0 ? NotReached : ReachedTemporarily;

}

CanBuildResult HouseExtData::PrereqValidate(
	HouseClass* pHouse, TechnoTypeClass* pItem,
	bool buildLimitOnly, bool includeQueued)
{

	//	auto canBuiltresult = CanBuildResult::Unbuildable;
	//	bool resultRetrieved = false;
	//	std::string TemporarilyResult {};

	const bool IsHuman = pHouse->IsControlledByHuman();
	//const bool debug = CRT::strcmpi(pItem->ID, "GAOREP") == 0;

	if (!buildLimitOnly)
	{
		const RequirementStatus ReqsMet = HouseExtData::RequirementsMet(pHouse, pItem);
		//const auto pItemExt = TechnoTypeExtContainer::Instance.Find(pItem);

		if (ReqsMet <= RequirementStatus::Incomplete)
		{

			//if (ReqsMet == RequirementStatus::Incomplete  &&
			//	!pItemExt->Prerequisite_Display.empty())
			//{
			//	if (Prereqs::HouseOwnsAll(pHouse,
			//		pItemExt->Prerequisite_Display.data(),
			//		pItemExt->Prerequisite_Display.size())
			//	)
			//	{
			//		return CanBuildResult::TemporarilyUnbuildable;
			//	}
			//}

			return CanBuildResult::Unbuildable;
		}

		if (IsHuman && ReqsMet == RequirementStatus::Complete)
		{
			if (!HouseExtData::PrerequisitesMet(pHouse, pItem))
			{
				return CanBuildResult::Unbuildable;
			}
		}

		const auto factoryresult = HouseExtData::HasFactory(pHouse, pItem, true, true, false, true).first;
		if (factoryresult == NewFactoryState::NotFound || factoryresult == NewFactoryState::NoFactory)
			return CanBuildResult::Unbuildable;

		if (factoryresult == NewFactoryState::Unpowered)
			return CanBuildResult::TemporarilyUnbuildable;
	}

	if (!IsHuman && RulesExtData::Instance()->AllowBypassBuildLimit[pHouse->GetAIDifficultyIndex()]) {
		return CanBuildResult::Buildable;
	}

	const auto builtLimitResult = static_cast<CanBuildResult>(HouseExtData::CheckBuildLimit(pHouse, pItem, includeQueued));

	if (builtLimitResult == CanBuildResult::Buildable
		&& pItem->WhatAmI() == BuildingTypeClass::AbsID
		&& !BuildingTypeExtContainer::Instance.Find((BuildingTypeClass*)pItem)->PowersUp_Buildings.empty()) {
		return static_cast<CanBuildResult>(HouseExtData::CheckBuildingBuildLimit(pHouse, (BuildingTypeClass*)pItem, includeQueued));
	}

	return builtLimitResult;
}

// true : continue check
// false : RequirementStatus::Incomplete
bool HouseExtData::CheckFactoryOwners(HouseClass* pHouse, TechnoTypeClass* pItem)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pItem);

	if (pExt->FactoryOwners.empty() && pExt->FactoryOwners_Forbidden.empty())
		return true;// no check needed

	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);
	bool isAvaible = false; // assiume it is not available

	for (auto& gather : pHouseExt->FactoryOwners_GatheredPlansOf) {

		for(auto& f_Owner : pExt->FactoryOwners){
			if (f_Owner == gather) {
				isAvaible = true; // one check pass
				break;
			}
		}

		for (auto& f_forbiddenOwner : pExt->FactoryOwners_Forbidden) {
			if (f_forbiddenOwner == gather) {
				return false; // it is forbidden , dont allow
			}

		}
	}

	if (!isAvaible)
	{ //cant found avaible plan

		const auto whatItem = pItem->WhatAmI();
		for (auto const& pBld : pHouse->Buildings)
		{
			auto pBldExt = TechnoExtContainer::Instance.Find(pBld);

			for (auto& f_Owner : pExt->FactoryOwners) {
				if (f_Owner == pBldExt->OriginalHouseType) {
					isAvaible = true; // one check pass
					break;
				}
			}

			for (auto& f_forbiddenOwner : pExt->FactoryOwners_Forbidden) {
				if (f_forbiddenOwner == pBldExt->OriginalHouseType) {
					return false;  // it is forbidden , dont allow
				}
			}

			if (isAvaible) { // further check


				//found one factory that avaible for the item , or the building type HasAllPlans
				if (pBld->Type->Factory == whatItem || BuildingTypeExtContainer::Instance.Find(pBld->Type)->Type->FactoryOwners_HasAllPlans) {
					return true;
				}
			}
		}
	}

	//FactoryOwners empty , so it should return true
	// otherwise we use isAvaible to check if we found any factory
	return pExt->FactoryOwners.empty() ? true : isAvaible;
}

void HouseExtData::UpdateAcademy(HouseClass* pHouse, BuildingClass* pAcademy, bool added)
{
	HouseExtContainer::Instance.Find(pHouse)->UpdateAcademy(pAcademy, added);
}

void HouseExtData::UpdateAcademy(BuildingClass* pAcademy, bool added)
{
	// check if added and there already, or removed and not there
	auto it = this->Academies.find(pAcademy);
	if (added == (it != this->Academies.end()))
	{
		return;
	}

	// now this can be unconditional
	if (added)
	{
		//using `emplace` because it already check above,..
		this->Academies.emplace(pAcademy);
	}
	else
	{
		this->Academies.erase(it);
	}
}

void HouseExtData::ApplyAcademy(HouseClass* pHouse, TechnoClass* pTechno, AbstractType considerAs)
{
	HouseExtContainer::Instance.Find(pHouse)->ApplyAcademy(pTechno, considerAs);
}

void HouseExtData::ApplyAcademyWithoutMutexCheck(
	TechnoClass* const pTechno, AbstractType const considerAs) const
{
	auto const pType = pTechno->GetTechnoType();
	if (pType->Trainable)
	{
		// get the academy data for this type
		Valueable<double> BuildingTypeExtData::* pmBonus = nullptr;
		switch (considerAs)
		{
		case AbstractType::Infantry:
		case AbstractType::InfantryType:
			pmBonus = &BuildingTypeExtData::AcademyInfantry;
			break;
		case AbstractType::AircraftType:
		case AbstractType::Aircraft:
			pmBonus = &BuildingTypeExtData::AcademyAircraft;
			break;
		case AbstractType::UnitType:
		case AbstractType::Unit:
			pmBonus = &BuildingTypeExtData::AcademyVehicle;
			break;
		default:
			pmBonus = &BuildingTypeExtData::AcademyBuilding;
			break;
		}

		auto veterancyBonus = 0.0;

		// aggregate the bonuses
		for (auto const& pBld : this->Academies)
		{
			if (!pBld)
				continue;

			auto const pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

			auto const isWhitelisted = pExt->AcademyWhitelist.empty()
				|| pExt->AcademyWhitelist.Contains(pType);

			if (isWhitelisted && !pExt->AcademyBlacklist.Contains(pType))
			{
				const auto& data = pExt->*pmBonus;
				veterancyBonus = MaxImpl(veterancyBonus, data.Get());
			}
		}

		// apply the bonus
		auto& value = pTechno->Veterancy.Veterancy;
		if (veterancyBonus > value)
		{
			value = static_cast<float>(MinImpl(
				veterancyBonus, RulesClass::Instance->VeteranCap));
		}
	}
}

void HouseExtData::ApplyAcademy(
	TechnoClass* const pTechno, AbstractType const considerAs) const
{
	// mutex in effect, ignore academies to fix preplaced order issues.
	// also triggered in game for certain "conversions" like deploy
	// Otamaa : added IsTethered check , so techno form WF wont get ignored !
	//
	if (Unsorted::ScenarioInit) {
		return;
	}

	auto const pType = pTechno->GetTechnoType();
	if (pType->Trainable)
	{
		// get the academy data for this type
		Valueable<double> BuildingTypeExtData::* pmBonus = nullptr;
		switch (considerAs)
		{
		case AbstractType::Infantry:
			pmBonus = &BuildingTypeExtData::AcademyInfantry;
			break;
		case AbstractType::Aircraft:
			pmBonus = &BuildingTypeExtData::AcademyAircraft;
			break;
		case AbstractType::Unit:
			pmBonus = &BuildingTypeExtData::AcademyVehicle;
			break;
		default:
			pmBonus = &BuildingTypeExtData::AcademyBuilding;
			break;
		}

		auto veterancyBonus = 0.0;

		// aggregate the bonuses
		for (auto const& pBld : this->Academies)
		{
			if (!pBld)
				continue;

			auto const pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

			auto const isWhitelisted = pExt->AcademyWhitelist.empty()
				|| pExt->AcademyWhitelist.Contains(pType);

			if (isWhitelisted && !pExt->AcademyBlacklist.Contains(pType))
			{
				const auto& data = pExt->*pmBonus;
				veterancyBonus = MaxImpl(veterancyBonus, data.Get());
			}
		}

		// apply the bonus
		auto& value = pTechno->Veterancy.Veterancy;
		if (veterancyBonus > value)
		{
			value = static_cast<float>(MinImpl(
				veterancyBonus, RulesClass::Instance->VeteranCap));
		}
	}
}

void HouseExtData::UpdateFactoryPlans(BuildingClass* pBld)
{
	auto Types = pBld->GetTypes();
	auto Types_c = Types.begin();

	while (!*Types_c || !TechnoTypeExtContainer::Instance.Find(*Types_c)->FactoryOwners_HaveAllPlans)
	{
		if (++Types_c == Types.end())
			return;
	}

	HouseExtContainer::Instance.Find(pBld->Owner)->FactoryOwners_GatheredPlansOf
		.insert(TechnoExtContainer::Instance.Find(pBld)->OriginalHouseType);
}

bool HouseExtData::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem)
{
	for (auto& prereq : TechnoTypeExtContainer::Instance.Find(pItem)->Prerequisites)
	{
		if (Prereqs::HouseOwnsAll(pThis, prereq.data(), prereq.size()))
		{
			return true;
		}
	}

	return false;
}

bool HouseExtData::PrerequisitesMet(HouseClass* pThis, int* items, int size)
{
	return Prereqs::HouseOwnsAll(pThis, items, size);
}

bool HouseExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pSection = This()->PlainName;

	if (!pINI->GetSection(pSection))
		return false;

	INI_EX exINI(pINI);
	bool readBaseNodeRepairInfo[3];
	size_t var = exINI.Read3Bool(pSection, "RepairBaseNodes", readBaseNodeRepairInfo);
	if (var <= 3) {
		for (size_t i = 0; i < var; i++) {
			this->RepairBaseNodes[i] = readBaseNodeRepairInfo[i];
		}
	}

	this->Degrades.Read(exINI, pSection, "Degrades");
	return true;
}

TunnelData* HouseExtData::GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx)
{
	if (!pHouse || nTunnelIdx >= TunnelTypeClass::Array.size())
		return nullptr;

	auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	while (pHouseExt->Tunnels.size() < TunnelTypeClass::Array.size())
	{
		pHouseExt->Tunnels.emplace_back(TunnelTypeClass::Array[nTunnelIdx]->Passengers);
	}

	return pHouseExt->Tunnels.data() + nTunnelIdx;
}

TunnelData* HouseExtData::GetTunnelVector(BuildingTypeClass* pBld, HouseClass* pHouse)
{
	return HouseExtData::GetTunnelVector(pHouse, BuildingTypeExtContainer::Instance.Find(pBld)->TunnelType);
}

void HouseExtData::UpdateShotCount(SuperWeaponTypeClass* pFor)
{
	this->LaunchDatas.resize(SuperWeaponTypeClass::Array->Count);
	this->LaunchDatas[pFor->ArrayIndex].Update();
}

void HouseExtData::UpdateShotCountB(SuperWeaponTypeClass* pFor)
{
	this->LaunchDatas.resize(SuperWeaponTypeClass::Array->Count);

	auto& nData = this->LaunchDatas[pFor->ArrayIndex];

	if ((nData.LastFrame & 0x80000000) != 0)
		nData.LastFrame = Unsorted::CurrentFrame();
}

SuperClass* HouseExtData::IsSuperAvail(int nIdx, HouseClass* pHouse)
{
	if (const auto pSW = pHouse->Supers.GetItemOrDefault(nIdx))
	{
		if (SWTypeExtContainer::Instance.Find(pSW->Type)->IsAvailable(pHouse))
		{
			return pSW;
		}
	}

	return nullptr;
}

int HouseExtData::GetSurvivorDivisor(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && (pTypeExt->SurvivorDivisor.Get() > 0))
		return pTypeExt->SurvivorDivisor;

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExtData::GetCrew(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Crew)
		return pTypeExt->Crew;

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExtData::GetEngineer(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Engineer)
		return pTypeExt->Engineer;

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExtData::GetTechnician(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Technician)
		return pTypeExt->Technician;

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExtData::GetDisguise(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Disguise)
		return pTypeExt->Disguise;

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetDisguise();
	}

	return nullptr;
}

AircraftTypeClass* HouseExtData::GetParadropPlane(HouseClass* pHouse)
{
	// tries to get the house's default plane and falls back to
	// the sides default plane.
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	AircraftTypeClass* pRest = nullptr;

	if (pTypeExt && pTypeExt->ParaDropPlane)
	{
		pRest = pTypeExt->ParaDropPlane;
	}

	if (!pRest)
	{
		int iPlane = -1;
		if (const auto pSide = HouseExtData::GetSide(pHouse))
		{
			iPlane = SideExtContainer::Instance.Find(pSide)->ParaDropPlane;
		}

		// didn't help. default to the PDPlane like the game does.

		pRest =
			AircraftTypeClass::Array->GetItemOrDefault(iPlane, RulesExtData::Instance()->DefaultParaPlane);
	}

	if (!pRest)
		Debug::FatalError("Invalid Paradrop Plane");

	return pRest;
}

AircraftTypeClass* HouseExtData::GetSpyPlane(HouseClass* pHouse)
{
	AircraftTypeClass* pRest = nullptr;

	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->SpyPlane)
	{
		pRest = pTypeExt->SpyPlane;
	}

	if (!pRest) {
		if (const auto pSide = HouseExtData::GetSide(pHouse)) {
			pRest = SideExtContainer::Instance.Find(pSide)->SpyPlane;
		}
	}

	if (!pRest)
		pRest = AircraftTypeClass::Find(GameStrings::SPYP);

	if (pRest && pRest->Strength == 0)
		Debug::FatalError("Invalid Spy Plane[%s]", pRest->ID);
	else if (!pRest)
		Debug::FatalError("Invalid Spy Plane");

	return pRest;
}

UnitTypeClass* HouseExtData::GetHunterSeeker(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->HunterSeeker)
	{
		return pTypeExt->HunterSeeker;
	}

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		return SideExtContainer::Instance.Find(pSide)->GetHunterSeeker();
	}

	return nullptr;
}

AnimTypeClass* HouseExtData::GetParachuteAnim(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->ParachuteAnim)
	{
		return pTypeExt->ParachuteAnim;
	}

	if (const auto pSide = HouseExtData::GetSide(pHouse))
	{
		if (auto pAnim = SideExtContainer::Instance.Find(pSide)->ParachuteAnim.Get(RulesClass::Instance->Parachute))
			return pAnim;

		Debug::LogInfo(
			"[GetParachuteAnim] House {} and its side have no valid parachute defined. Rules fallback failed.",
			pHouse->get_ID());
	}

	return AnimTypeClass::Find("PARACH");
}

bool HouseExtData::GetParadropContent(HouseClass* pHouse, Iterator<TechnoTypeClass*>& Types, Iterator<int>& Num)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	// tries to get the house's default contents and falls back to
	// the sides default contents.
	if (pTypeExt && !pTypeExt->ParaDropTypes.empty())
	{
		Types = pTypeExt->ParaDropTypes;
		Num = pTypeExt->ParaDropNum;
	}

	// fall back to side specific para drop
	if (!Types)
	{
		if (const auto pSide = HouseExtData::GetSide(pHouse))
		{
			SideExtData* pData = SideExtContainer::Instance.Find(pSide);

			Types = pData->GetParaDropTypes();
			Num = pData->GetParaDropNum();
		}
	}

	return (Types && Num);
}

#include <Ext/Team/Body.h>

NOINLINE void GetRemainingTaskForceMembers(TeamClass* pTeam, std::vector<TechnoTypeClass*>& missings)
{
	const auto pType = pTeam->Type;
	const auto pTaskForce = pType->TaskForce;

	for (int a = 0; a < pTaskForce->CountEntries; ++a)
	{
		for (int i = 0; i < pTaskForce->Entries[a].Amount; ++i)
		{
			if (auto pTaskType = pTaskForce->Entries[a].Type)
			{
				missings.emplace_back(pTaskType);
			}
		}
	}

	//remove first finded similarity
	for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
	{
		auto it = std::find_if(missings.begin(), missings.end(), [&](TechnoTypeClass* pMissType)
 {
	 return pMember->GetTechnoType() == pMissType || TeamExtData::IsEligible(pMember, pMissType);
		});

		if (it != missings.end())
			missings.erase(it);
	}
}

void HouseExtData::GetUnitTypeToProduce()
{
	auto pThis = This();
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	auto& creationFrames = this->Productions[0].CreationFrames;
	auto& values = this->Productions[0].Values;
	auto& bestChoices = this->Productions[0].BestChoices;
	auto& bestChoicesNaval = this->BestChoicesNaval;

	auto count = static_cast<size_t>(UnitTypeClass::Array->Count);
	creationFrames.assign(count, 0x7FFFFFFF);
	values.assign(count, 0);
	std::vector<TechnoTypeClass*> taskForceMembers;
	taskForceMembers.reserve(UnitTypeClass::Array->Count);

	for (auto& currentTeam : HouseExtContainer::HousesTeams[pThis])
	{
		taskForceMembers.clear();
		int teamCreationFrame = currentTeam->CreationFrame;

		if ((!currentTeam->Type->Reinforce || currentTeam->IsFullStrength)
			&& (currentTeam->IsForcedActive || currentTeam->IsHasBeen))
		{
			continue;
		}

		GetRemainingTaskForceMembers(currentTeam, taskForceMembers);

		for (auto& currentMember : taskForceMembers)
		{
			const auto what = currentMember->WhatAmI();

			if (what != UnitTypeClass::AbsID ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			const auto index = static_cast<size_t>(((UnitTypeClass*)currentMember)->ArrayIndex);
			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	for (int i = 0; i < UnitClass::Array->Count; ++i)
	{
		const auto pUnit = UnitClass::Array->Items[i];

		if (values[pUnit->Type->ArrayIndex] > 0 && pUnit->CanBeRecruited(pThis))
			--values[pUnit->Type->ArrayIndex];
	}

	bestChoices.clear();
	bestChoicesNaval.clear();

	int bestValue = -1;
	int bestValueNaval = -1;
	int earliestTypenameIndex = -1;
	int earliestTypenameIndexNaval = -1;
	int earliestFrame = 0x7FFFFFFF;
	int earliestFrameNaval = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		int currentValue = values[i];

		if (currentValue <= 0)
			continue;

		const auto buildableResult = pThis->CanBuild(type, false, false);

		if (buildableResult == CanBuildResult::Unbuildable
			|| type->GetActualCost(pThis) > pThis->Available_Money())
		{
			continue;
		}

		bool isNaval = type->Naval;
		int* cBestValue = !isNaval ? &bestValue : &bestValueNaval;
		std::vector<int>* cBestChoices = !isNaval ? &bestChoices : &bestChoicesNaval;

		if (*cBestValue < currentValue || *cBestValue == -1)
		{
			*cBestValue = currentValue;
			cBestChoices->clear();
		}

		cBestChoices->push_back(static_cast<int>(i));

		int* cEarliestTypeNameIndex = !isNaval ? &earliestTypenameIndex : &earliestTypenameIndexNaval;
		int* cEarliestFrame = !isNaval ? &earliestFrame : &earliestFrameNaval;

		if (*cEarliestFrame > creationFrames[i] || *cEarliestTypeNameIndex == -1)
		{
			*cEarliestTypeNameIndex = static_cast<int>(i);
			*cEarliestFrame = creationFrames[i];
		}
	}

	if (!skipGround)
	{
		int result_ground = earliestTypenameIndex;
		if (ScenarioClass::Instance->Random.RandomFromMax(99) >= RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty])
		{
			if (!bestChoices.empty())
				result_ground = bestChoices[ScenarioClass::Instance->Random.RandomFromMax(int(bestChoices.size() - 1))];
			else
				result_ground = -1;
		}

		pThis->ProducingUnitTypeIndex = result_ground;
	}

	if (!skipNaval)
	{
		int result_naval = earliestTypenameIndexNaval;
		if (ScenarioClass::Instance->Random.RandomFromMax(99) >= RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty])
		{
			if (!bestChoicesNaval.empty())
				result_naval = bestChoicesNaval[ScenarioClass::Instance->Random.RandomFromMax(int(bestChoicesNaval.size() - 1))];
			else
				result_naval = -1;
		}

		this->ProducingNavalUnitTypeIndex = result_naval;
	}
}

int HouseExtData::GetAircraftTypeToProduce()
{
	auto& CreationFrames = this->Productions[1].CreationFrames;
	auto& Values = this->Productions[1].Values;
	auto& BestChoices = this->Productions[1].BestChoices;

	auto const count = static_cast<unsigned int>(AircraftTypeClass::Array->Count);
	CreationFrames.assign(count, 0x7FFFFFFF);
	Values.assign(count, 0);
	BestChoices.clear();
	std::vector<TechnoTypeClass*> taskForceMembers;
	taskForceMembers.reserve(AircraftTypeClass::Array->Count);

	//Debug::LogInfo(__FUNCTION__" Executing with Current TeamArrayCount[%d] for[%s][House %s - %x] ", TeamClass::Array->Count, AbstractClass::GetAbstractClassName(Ttype::AbsID), pHouse->get_ID() , pHouse);
	for (auto& CurrentTeam : HouseExtContainer::HousesTeams[This()])
	{
		taskForceMembers.clear();
		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if (CurrentTeam->Type->Reinforce && !CurrentTeam->IsFullStrength || !CurrentTeam->IsForcedActive && !CurrentTeam->IsHasBeen)
		{
			GetRemainingTaskForceMembers(CurrentTeam, taskForceMembers);

			for (auto& pMember : taskForceMembers)
			{
				if (pMember->WhatAmI() != AircraftTypeClass::AbsID)
				{
					continue;
				}

				auto const Idx = static_cast<unsigned int>(((AircraftTypeClass*)pMember)->ArrayIndex);

				++Values[Idx];
				if (TeamCreationFrame < CreationFrames[Idx])
				{
					CreationFrames[Idx] = TeamCreationFrame;
				}
			}
		}
	}

	for (auto classPos = AircraftClass::Array->begin(); classPos != AircraftClass::Array->end(); ++classPos)
	{
		auto const Idx = static_cast<unsigned int>((*classPos)->Type->ArrayIndex);
		if (Values[Idx] > 0 && (*classPos)->CanBeRecruited(This()))
		{
			--Values[Idx];
		}
	}

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto const TT = AircraftTypeClass::Array->Items[static_cast<int>(i)];

		int CurrentValue = Values[i];

		if (CurrentValue <= 0)
			continue;

		const auto buildableResult = This()->CanBuild(TT, false, false);

		if (buildableResult != CanBuildResult::Buildable || TT->GetActualCost(This()) > This()->Available_Money()) {
			continue;
		}

		//yes , we checked this fucking twice just to make sure
		const auto factoryresult = HouseExtData::HasFactory(This(), TT, false, true, false, true).first;

		if (factoryresult == NewFactoryState::NotFound || factoryresult == NewFactoryState::NoFactory) {
			continue;
		}

		if (BestValue < CurrentValue || BestValue == -1)
		{
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(static_cast<int>(i));
		if (EarliestFrame > CreationFrames[i] || EarliestTypenameIndex < 0)
		{
			EarliestTypenameIndex = static_cast<int>(i);
			EarliestFrame = CreationFrames[i];
		}
	}

	const auto AIDiff = static_cast<int>(This()->GetAIDifficultyIndex());

	if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesClass::Instance->FillEarliestTeamProbability[AIDiff])
		return EarliestTypenameIndex;

	if (!BestChoices.empty())
		return BestChoices[ScenarioClass::Instance->Random.RandomFromMax(int(BestChoices.size() - 1))];

	return -1;
}

int HouseExtData::GetInfantryTypeToProduce()
{
	auto& CreationFrames = this->Productions[2].CreationFrames;
	auto& Values = this->Productions[2].Values;
	auto& BestChoices = this->Productions[2].BestChoices;

	auto const count = static_cast<unsigned int>(InfantryTypeClass::Array->Count);
	CreationFrames.assign(count, 0x7FFFFFFF);
	Values.assign(count, 0);
	BestChoices.clear();
	std::vector<TechnoTypeClass*> taskForceMembers;
	taskForceMembers.reserve(InfantryTypeClass::Array->Count);

	//Debug::LogInfo(__FUNCTION__" Executing with Current TeamArrayCount[%d] for[%s][House %s - %x] ", TeamClass::Array->Count, AbstractClass::GetAbstractClassName(Ttype::AbsID), pHouse->get_ID() , pHouse);
	for (auto& CurrentTeam : HouseExtContainer::HousesTeams[This()])
	{
		taskForceMembers.clear();
		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if (CurrentTeam->Type->Reinforce && !CurrentTeam->IsFullStrength || !CurrentTeam->IsForcedActive && !CurrentTeam->IsHasBeen)
		{
			GetRemainingTaskForceMembers(CurrentTeam, taskForceMembers);

			for (auto& pMember : taskForceMembers)
			{
				if (pMember->WhatAmI() != InfantryTypeClass::AbsID)
				{
					continue;
				}

				auto const Idx = static_cast<unsigned int>(((InfantryTypeClass*)pMember)->ArrayIndex);

				++Values[Idx];
				if (TeamCreationFrame < CreationFrames[Idx])
				{
					CreationFrames[Idx] = TeamCreationFrame;
				}
			}
		}
	}

	for (auto classPos = InfantryClass::Array->begin(); classPos != InfantryClass::Array->end(); ++classPos)
	{
		auto const Idx = static_cast<unsigned int>((*classPos)->Type->ArrayIndex);
		if (Values[Idx] > 0 && (*classPos)->CanBeRecruited(This()))
		{
			--Values[Idx];
		}
	}

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto const TT = InfantryTypeClass::Array->Items[static_cast<int>(i)];

		int CurrentValue = Values[i];

		if (CurrentValue <= 0)
			continue;

		const auto buildableResult = This()->CanBuild(TT, false, false);

		if (buildableResult == CanBuildResult::Unbuildable
			|| TT->GetActualCost(This()) > This()->Available_Money())
		{
			continue;
		}

		if (BestValue < CurrentValue || BestValue == -1)
		{
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(static_cast<int>(i));
		if (EarliestFrame > CreationFrames[i] || EarliestTypenameIndex < 0)
		{
			EarliestTypenameIndex = static_cast<int>(i);
			EarliestFrame = CreationFrames[i];
		}
	}

	const auto AIDiff = static_cast<int>(This()->GetAIDifficultyIndex());

	if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesClass::Instance->FillEarliestTeamProbability[AIDiff])
		return EarliestTypenameIndex;

	if (!BestChoices.empty())
		return BestChoices[ScenarioClass::Instance->Random.RandomFromMax(int(BestChoices.size() - 1))];

	return -1;
}

TechTreeTypeClass* HouseExtData::GetTechTreeType() {

	if(!this->SideTechTree.isset()){
		TechTreeTypeClass* ret = nullptr;

		for (auto& pType : TechTreeTypeClass::Array) {
			if (pType->SideIndex == This()->SideIndex) {
				ret = pType.get();
			}
		}

		if(!ret){
			Debug::LogInfo("TechTreeTypeClass::GetForSide: Could not find tech tree for side {}, returning tech tree 0: {}",
				This()->SideIndex, TechTreeTypeClass::Array.begin()->get()->Name.data());
			ret = TechTreeTypeClass::Array.begin()->get();
		}

		this->SideTechTree = ret;
	}

	return this->SideTechTree.get();
}

void HouseExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if (ptr == nullptr)
		return;

	//AnnounceInvalidPointer(OwnedTechno, ptr , bRemoved);
	AnnounceInvalidPointer(Factory_BuildingType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_InfantryType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_VehicleType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_NavyType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_AircraftType, ptr, bRemoved);

	Academies.InvalidatePointer(ptr, bRemoved);
	TunnelsBuildings.InvalidatePointer(ptr, bRemoved);
	RestrictedFactoryPlants.InvalidatePointer(ptr, bRemoved);
	OwnedCountedHarvesters.InvalidatePointer(ptr, bRemoved);

	for (auto& nTun : Tunnels){
		AnnounceInvalidPointer<FootClass*>(nTun.Vector, ptr, bRemoved);
	}

	AnnounceInvalidPointer<SuperClass*>(Batteries, ptr);
}

int HouseExtData::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer()) return 0;

	auto pOwnerExt = HouseExtContainer::Instance.Find(pThis);

	int result =
		std::count_if(pOwnerExt->OwnedCountedHarvesters.begin(), pOwnerExt->OwnedCountedHarvesters.end(),
		[pThis](TechnoClass* techno)
		{
			if (!techno->IsAlive || techno->Health <= 0 || techno->IsCrashing || techno->IsSinking)
				return false;

			if (techno->WhatAmI() == UnitClass::AbsID && (static_cast<UnitClass*>(techno)->DeathFrameCounter > 0))
				return false;

			return TechnoExtData::IsHarvesting(techno);
		});

	return result;
}


int HouseExtData::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer() || pThis->Defeated) return 0;

	int result = 0;
	auto pOwnerExt = HouseExtContainer::Instance.Find(pThis);

	std::for_each(pOwnerExt->OwnedCountedHarvesters.begin(), pOwnerExt->OwnedCountedHarvesters.end(), [&result, pThis](TechnoClass* techno) {
		result += !techno->InLimbo && techno->IsAlive && techno->Health > 0;
	});

	return result;
}

// This basically gets same cell that AI script action 53 Gather at Enemy Base uses, and code for that (0x6EF700) was used as reference here.
CellClass* HouseExtData::GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, const CoordStruct& defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance)
{
	if (!pTargetHouse || !pCurrentHouse)
		return nullptr;

	const auto targetBaseCoords = CellClass::Cell2Coord(pTargetHouse->GetBaseCenter());

	if (targetBaseCoords == CoordStruct::Empty)
		return nullptr;

	auto currentCoords = CellClass::Cell2Coord(pCurrentHouse->GetBaseCenter());

	if (currentCoords == CoordStruct::Empty)
		currentCoords = defaultCurrentCoords;

	//const int deltaX = currentCoords.X - targetBaseCoords.X;
	//const int deltaY = targetBaseCoords.Y - currentCoords.Y;
	const int distance = (RulesClass::Instance->AISafeDistance + extraDistance) * Unsorted::LeptonsPerCell;
	auto newCoords = GeneralUtils::CalculateCoordsFromDistance(currentCoords, targetBaseCoords, distance);
	auto cellStruct = CellClass::Coord2Cell(newCoords);
	cellStruct = MapClass::Instance->NearByLocation(cellStruct, speedTypeZone, ZoneType::None, MovementZone::Normal, false, 3, 3, false, false, false, true, cellStruct, false, false);

	return MapClass::Instance->TryGetCellAt(cellStruct);
}

HouseClass* HouseExtData::FindFirstCivilianHouse()
{
	if (!HouseExtContainer::Civilian) {

		auto idx = SideClass::FindIndexById(GameStrings::Civilian);

		if (RulesExtData::Instance()->CivilianSideIndex == -1 || RulesExtData::Instance()->CivilianSideIndex != idx)
			RulesExtData::Instance()->CivilianSideIndex = idx;

		if (!HouseExtContainer::Civilian) {
			HouseExtContainer::CivilianSide = SideClass::Array->Items[idx];

			for (auto pHouse : *HouseClass::Array) {
				if (pHouse->Type->SideIndex == idx) {
					HouseExtContainer::Civilian = pHouse;
					break;
				}
			}
		}

		if (!HouseExtContainer::Civilian) {
			Debug::FatalErrorAndExit("Failed to find Civilian House !!");
		}
	}

	return HouseExtContainer::Civilian;
}

HouseClass* HouseExtData::FindSpecial()
{
	if (!HouseExtContainer::Special) {

		auto idx = HouseTypeClass::FindIndexByIdAndName(GameStrings::Special);

		if (RulesExtData::Instance()->SpecialCountryIndex == -1 || RulesExtData::Instance()->SpecialCountryIndex != idx)
			RulesExtData::Instance()->SpecialCountryIndex = idx;

		if (!HouseExtContainer::Special) {
			for (auto pHouse : *HouseClass::Array) {
				if (pHouse->Type->ArrayIndex == idx|| pHouse->Type->ParentIdx == idx) {
					HouseExtContainer::Special = pHouse;
					break;
				}
			}
		}

		if (!HouseExtContainer::Special) {
			//HouseExtContainer::Special = GameCreate<HouseClass>(HouseTypeClass::Array->Items[idx]);
			Debug::FatalErrorAndExit("Cannot Find House with Special Country !");
		}
	}

	return HouseExtContainer::Special;
}

HouseClass* HouseExtData::FindNeutral()
{
	if(!HouseExtContainer::Neutral){
		auto idx = HouseTypeClass::FindIndexByIdAndName(GameStrings::Neutral);

		if (RulesExtData::Instance()->NeutralCountryIndex == -1 || RulesExtData::Instance()->NeutralCountryIndex != idx)
			RulesExtData::Instance()->NeutralCountryIndex = idx;

		if (!HouseExtContainer::Neutral) {
			for (auto pHouse : *HouseClass::Array) {
				//Debug::LogInfo("House [{} - {}/{}] , side {} country {} parent {}/{}", (void*)pHouse , pHouse->Type->ID , pHouse->Type->Name
				 ///,	pHouse->Type->SideIndex , pHouse->Type->ArrayIndex, pHouse->Type->ParentIdx , pHouse->Type->ParentCountry.data()
				//);

				if (pHouse->Type->ArrayIndex == idx || pHouse->Type->ParentIdx == idx) {
					HouseExtContainer::Neutral = pHouse;
					break;
				}
			}
		}

		if (!HouseExtContainer::Neutral) {
			//HouseExtContainer::Neutral = GameCreate<HouseClass>(HouseTypeClass::Array->Items[idx]);
			Debug::FatalErrorAndExit("Cannot Find House with Neutral Country !");
		}
	}

	return HouseExtContainer::Neutral;
}

void HouseExtData::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode = -1)
{
	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis);

	if (mode < 0 || mode > 2)
		mode = -1;

	enum { ForceFalse = 0, ForceTrue = 1, ForceRandom = 2, UseDefault = -1 };

	pHouseExt->ForceOnlyTargetHouseEnemyMode = mode;

	switch (mode)
	{
	case ForceFalse:
		pHouseExt->m_ForceOnlyTargetHouseEnemy = false;
		break;

	case ForceTrue:
		pHouseExt->m_ForceOnlyTargetHouseEnemy = true;
		break;

	case ForceRandom:
		pHouseExt->m_ForceOnlyTargetHouseEnemy = ScenarioClass::Instance->Random.RandomBool();
		break;

	default:
		pHouseExt->m_ForceOnlyTargetHouseEnemy = false;
		break;
	}
}

// Ares
HouseClass* HouseExtData::GetHouseKind(OwnerHouseKind const& kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind)
	{
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseExtData::FindFirstCivilianHouse();// HouseClass::FindFirstCivilianHouse();
	case OwnerHouseKind::Special:
		return HouseExtData::FindSpecial();//  HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseExtData::FindNeutral();//  HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom)
		{
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->Items[
				Random.RandomFromMax(HouseClass::Array->Count - 1)];
		}
		else
		{
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

HouseClass* HouseExtData::GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* const pKiller, HouseClass* const pVictim)
{
	switch (kind)
	{
	case SlaveReturnTo::Killer:
		return pKiller;
	case SlaveReturnTo::Master:
		return pVictim;
	case SlaveReturnTo::Civilian:
		return HouseExtData::FindFirstCivilianHouse();
	case SlaveReturnTo::Special:
		return HouseExtData::FindSpecial();
	case SlaveReturnTo::Neutral:
		return HouseExtData::FindNeutral();
	case SlaveReturnTo::Random:
		auto& Random = ScenarioClass::Instance->Random;
		return HouseClass::Array->Items[
			Random.RandomFromMax(HouseClass::Array->Count - 1)];
	}

	return pKiller;
}

bool HouseExtData::IsObserverPlayer()
{
	auto const pCur = HouseClass::CurrentPlayer();

	if (!pCur)
		return false;

	if (pCur == HouseClass::Observer)
		return true;

	return false;
}

bool HouseExtData::IsObserverPlayer(HouseClass* pCur)
{
	if (!pCur)
		return false;

	if (pCur == HouseClass::Observer())
		return true;

	return false;
}

int HouseExtData::GetHouseIndex(int param, TeamClass* pTeam = nullptr, TActionClass* pTAction = nullptr)
{
	// Special case that returns the house index of the TeamClass object or the Trigger Action
	if (param == 8997)
	{
		if (pTAction)
			return (pTeam ? pTeam->OwnerHouse->ArrayIndex : pTAction->TeamType->Owner->ArrayIndex);
		else
			return -1;
	}

	if (param < 0)
	{
		StackVector<HouseClass* , 20> housesListIdx {};

		switch (param)
		{
		case -1:
		{
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array)
			{
				if (!pHouse->Defeated
					&& !HouseExtData::IsObserverPlayer(pHouse)
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx->push_back(pHouse);
				}
			}

			return housesListIdx->empty() ?
				-1 :
				housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx->size() - 1)]->ArrayIndex;
		}
		case -2:
		{
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					return pHouseNeutral->ArrayIndex;
				}
			}

			return -1;
		}
		case -3:
		{
			// Random Human Player
			for (auto pHouse : *HouseClass::Array)
			{
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !HouseExtData::IsObserverPlayer(pHouse))
				{
					housesListIdx->push_back(pHouse);
				}
			}

			return housesListIdx->empty() ?
				-1 :
				housesListIdx[(ScenarioClass::Instance->Random.RandomFromMax(housesListIdx->size() - 1))]
				->ArrayIndex;
		}
		default:
			return -1;
		}
	}

	// Transtale the Multiplayer index into a valid index for the HouseClass array
	if (HouseClass::Index_IsMP(param))
	{
		if (HouseClass* pHouse = HouseClass::FindByIndex(param))
		{
			if (!pHouse->Defeated
				&& !pHouse->IsObserver()
				&& !pHouse->Type->MultiplayPassive)
			{
				return pHouse->ArrayIndex;
			}
		}

		return -1;
	}


	// Positive index values check. Includes any kind of House
	if (HouseClass* pHouse = HouseClass::FindByCountryIndex(param))
	{
		if (!pHouse->Defeated && !pHouse->IsObserver())
		{
			return pHouse->ArrayIndex;
		}

		return -1;
	}


	return -1;
}

bool HouseExtData::UpdateHarvesterProduction()
{
	auto pThis = This();
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	const auto idxParentCountry = pThis->Type->FindParentCountryIndex();
	const auto pHarvesterUnit = HouseExtData::FindOwned(pThis, idxParentCountry, make_iterator(RulesClass::Instance->HarvesterUnit));

	if (pHarvesterUnit)
	{
		const auto harvesters = pThis->CountResourceGatherers;
		const auto maxHarvesters = HouseExtData::FindBuildable(
			pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery))
			? RulesClass::Instance->HarvestersPerRefinery[AIDifficulty] * pThis->CountResourceDestinations
			: RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

		if (pThis->IQLevel2 >= RulesClass::Instance->Harvester && !pThis->IsTiberiumShort
			&& !pThis->IsControlledByHuman() && harvesters < maxHarvesters
			&& pThis->StaticData.TechLevel >= pHarvesterUnit->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvesterUnit->ArrayIndex;
			return true;
		}
	}
	else
	{
		if (pThis->CountResourceGatherers < RulesClass::Instance->AISlaveMinerNumber[AIDifficulty])
		{
			if (const auto pRefinery = HouseExtData::FindBuildable(
				pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery)))
			{
				if (auto const pSlaveMiner = pRefinery->UndeploysInto)
				{
					if (pSlaveMiner->ResourceDestination)
					{
						pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
						return true;
					}
				}
			}
		}
	}

	return false;
}

size_t HouseExtData::FindOwnedIndex(
	HouseClass*, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	auto const bitOwner = 1u << idxParentCountry;

	for (size_t i = start; i < items.size(); ++i)
	{
		if (items[i]->InOwners(bitOwner))
		{
			return i;
		}
	}

	return items.size();
}

bool HouseExtData::IsDisabledFromShell(
	HouseClass* pHouse, BuildingTypeClass const* const pItem)
{
	// SWAllowed does not apply to campaigns any more
	if (SessionClass::Instance->GameMode == GameMode::Campaign
		|| GameModeOptionsClass::Instance->SWAllowed)
	{
		return false;
	}

	if (pItem->SuperWeapon != -1)
	{
		// allow SWs only if not disableable from shell
		if (!RulesClass::Instance->BuildTech.Contains(const_cast<BuildingTypeClass*>(pItem)))
		{
			if (pHouse->Supers[pItem->SuperWeapon]->Type->DisableableFromShell)
			{
				return true;
			}
		}
	}

	return false;
}

void HouseExtData::UpdateAutoDeathObjects()
{
	 HouseExtData::AutoDeathObjects.erase_all_if([](auto& item) {
		if (!item.first || item.second == KillMethod::None || !item.first->IsAlive) {
			return true;
		}

		auto const pExt = TechnoExtContainer::Instance.Find(item.first);

		if (!item.first->IsInLogic && pExt->CheckDeathConditions()) {
			return true;
		}
		return false;
	});
}

int HouseExtData::CountOwnedIncludeDeploy(const HouseClass* pThis, const TechnoTypeClass* pItem)
{
	int count = pThis->CountOwnedNow(pItem);
	count += pItem->DeploysInto ? pThis->CountOwnedNow(pItem->DeploysInto) : 0;
	count += pItem->UndeploysInto ? pThis->CountOwnedNow(pItem->UndeploysInto) : 0;
	return count;
}

std::vector<int> HouseExtData::GetBuildLimitGroupLimits(HouseClass* pHouse, TechnoTypeClass* pType)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	std::vector<int> limits = pTypeExt->BuildLimitGroup_Nums;

	if (!pTypeExt->BuildLimitGroup_ExtraLimit_Types.empty() && !pTypeExt->BuildLimitGroup_ExtraLimit_Nums.empty()) {
		for (size_t i = 0; i < pTypeExt->BuildLimitGroup_ExtraLimit_Types.size(); i++) {

			int count = 0;
			auto pTmpType = pTypeExt->BuildLimitGroup_ExtraLimit_Types[i];
			auto const pBuildingType = type_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType &&
				(BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				count = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				count = pHouse->CountOwnedNow(pTmpType);

			if (i < pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount.size() && pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i] > 0)
				count = MinImpl(count, pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i]);

			for (auto& limit : limits) {
				if (i < pTypeExt->BuildLimitGroup_ExtraLimit_Nums.size() && pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i] > 0)
					limit += count * pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i];

				if (pTypeExt->BuildLimitGroup_ExtraLimit_MaxNum > 0)
					limit = MinImpl(limit, pTypeExt->BuildLimitGroup_ExtraLimit_MaxNum);
			}
		}
	}

	return limits;
}

int HouseExtData::QueuedNum(const HouseClass* pHouse, const TechnoTypeClass* pType)
{
	const AbstractType absType = pType->WhatAmI();

	int queued = 0;

	if (const FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare)){
		queued = pFactory->CountTotal(pType);

		if (const auto pObject = pFactory->Object) {
			if (pObject->GetType() == pType)
				--queued;
		}
	}

	return queued;
}

void HouseExtData::RemoveProduction(const HouseClass* pHouse, const TechnoTypeClass* pType, int num)
{
	const AbstractType absType = pType->WhatAmI();

	if (FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare)) {

		int queued = pFactory->CountTotal(pType);
		if (num >= 0)
			queued = MinImpl(num, queued);

		for (int i = 0; i < queued; i++) {
			pFactory->RemoveOneFromQueue(pType);
		}
	}
}

bool HouseExtData::ReachedBuildLimit(HouseClass* pHouse,TechnoTypeClass* pType, bool ignoreQueued)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pTypeExt->BuildLimitGroup_Types.empty() || pTypeExt->BuildLimitGroup_Nums.empty())
		return false;

	const std::vector<int> limits = HouseExtData::GetBuildLimitGroupLimits(pHouse , pType);

	if (limits.size() == 1)
	{
		int count = 0;
		int queued = 0;
		bool inside = false;

		for (auto& pTmpType: pTypeExt->BuildLimitGroup_Types) {
			const auto pTmpTypeExt = TechnoTypeExtContainer::Instance.Find(pTmpType);

			if (!ignoreQueued)
				queued += QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;

			int owned = 0;
			const auto pBuildingType = type_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType && (BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				owned = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				owned = pHouse->CountOwnedNow(pTmpType);

			count += owned * pTmpTypeExt->BuildLimitGroup_Factor;

			if (pTmpType == pType)
				inside = true;
		}

		int num = count - limits.back();

		if (num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
		{
			if (inside)
				RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);
			else if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}
	else
	{
		bool reached = true;
		bool realReached = true;

		for (size_t i = 0; i < limits.size(); i++)
		{
			TechnoTypeClass* pTmpType = pTypeExt->BuildLimitGroup_Types[i];
			const auto pTmpTypeExt = TechnoTypeExtContainer::Instance.Find(pTmpType);
			int queued = ignoreQueued ? 0 : QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;
			int num = 0;
			const auto pBuildingType = type_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType && (BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				num = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				num = pHouse->CountOwnedNow(pTmpType);

			num *= pTmpTypeExt->BuildLimitGroup_Factor - limits[i];

			if (pType == pTmpType && num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);

					return true;
				}
				else if (num < 1 - pTypeExt->BuildLimitGroup_Factor)
				{
					realReached = false;
				}
			}
			else if (pType != pTmpType && num + queued >= 0)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 0 || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, -1);

					return true;
				}
				else if (num < 0)
				{
					realReached = false;
				}
			}
			else
			{
				reached = false;
			}
		}

		if (reached)
		{
			if (realReached || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}

	return false;
}

bool HouseExtData::ShouldDisableCameo(HouseClass* pThis, TechnoTypeClass* pType)
{
	auto ret = false;
	if (pType)
	{
		//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		// there is some another stupid bug
		// where if the building already queueed and paused
		// then you remove the prereq thing , the timer wont restart
		// it make it odd
		// also doing this check every frame is kind a weird ,..
		//if(!pTypeExt->Prerequisite_Display.empty()
		//	&& HouseExtData::PrereqValidate(pThis, pType , false , false ) == CanBuildResult::TemporarilyUnbuildable)
		//{
		//	R->EAX(true);
		//	return 0x50B669;
		//}

		auto const abs = pType->WhatAmI();
		auto const pFactory = pThis->GetPrimaryFactory(
			abs, pType->Naval, BuildCat::DontCare);

		// special logic for AirportBound
		if (abs == AbstractType::AircraftType)
		{
			auto const pAType = static_cast<AircraftTypeClass const*>(pType);
			if (pAType->AirportBound)
			{
				auto ownedAircraft = 0;
				auto queuedAircraft = 0;

				for (auto const& pAircraft : RulesClass::Instance->PadAircraft)
				{
					ownedAircraft += pThis->CountOwnedAndPresent(pAircraft);
					if (pFactory)
					{
						queuedAircraft += pFactory->CountTotal(pAircraft);
					}
				}

				// #896082: also check BuildLimit, and not always return the
				// result of this comparison directly. originally, it would
				// return false here, too, allowing more units than the
				// BuildLimit permitted.
				if (ownedAircraft + queuedAircraft >= pThis->AirportDocks)
				{
					return true;
				}
			}
		}

		auto queued = 0;
		if (pFactory)
		{
			queued = pFactory->CountTotal(pType);

			// #1286800: build limit > 1 and queues
			// the object in production is counted twice: it appears in this
			// factory queue, and it is already counted in the house's counters.
			// this only affects positive build limits, for negative ones
			// players could queue up one more than BuildLimit.
			if (auto const pObject = pFactory->Object)
			{
				if (pObject->GetType() == pType && pType->BuildLimit > 0)
				{
					--queued;
				}
			}
		}

		// #1521738: to stay consistent, use the new method to calculate this
		if (HouseExtData::BuildLimitRemaining(pThis, pType) - queued <= 0)
		{ ret = true; }
		else
		{
			const auto state = HouseExtData::HasFactory(pThis, pType, true, true, false, true);
			ret = (state.first < NewFactoryState::Available_Alternative);
		}
	}

	return ret;
}

CanBuildResult HouseExtData::BuildLimitGroupCheck(HouseClass* pThis,TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued)
{
	auto pItemExt = TechnoTypeExtContainer::Instance.Find(pItem);

	if (pItemExt->BuildLimitGroup_Types.empty()){
		return CanBuildResult::Buildable;
	}

	const std::vector<int> limits = HouseExtData::GetBuildLimitGroupLimits(pThis , pItem);

	if (pItemExt->BuildLimitGroup_ContentIfAnyMatch.Get()) {
		bool reachedLimit = false;

		for (size_t i = 0; i < MinImpl(pItemExt->BuildLimitGroup_Types.size(), pItemExt->BuildLimitGroup_Nums.size()); i++) {
			TechnoTypeClass* pType = pItemExt->BuildLimitGroup_Types[i];
			const auto pBuildingType = type_cast<BuildingTypeClass*>(pType);
			//const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
			int ownedNow = 0;

			if (pBuildingType && (BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				ownedNow = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
			else
				ownedNow = CountOwnedIncludeDeploy(pThis, pType);

			if (ownedNow >= limits[i] + 1 - pItemExt->BuildLimitGroup_Factor)
				reachedLimit |= !(includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType));
		}

		return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
	}
	else
	{
		if (limits.size() == 1U)
		{
			int sum = 0;
			bool reachedLimit = false;

			for (auto& pType : pItemExt->BuildLimitGroup_Types)
			{
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
				const auto pBuildingType = type_cast<BuildingTypeClass*>(pType);
				int owned = 0;

				if (pBuildingType && (BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
					owned = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
				else
					owned = CountOwnedIncludeDeploy(pThis, pType);

				sum += owned * pTypeExt->BuildLimitGroup_Factor;
			}

			if (sum >= limits[0] + 1 - pItemExt->BuildLimitGroup_Factor) {
				for (auto& pType : pItemExt->BuildLimitGroup_Types) {
					reachedLimit |= !(includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType));
				}
			}

			return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
		}
		else
		{
			for (size_t i = 0; i < MinImpl(pItemExt->BuildLimitGroup_Types.size(), limits.size()); i++)
			{
				TechnoTypeClass* pType = pItemExt->BuildLimitGroup_Types[i];
				const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
				const auto pBuildingType = type_cast<BuildingTypeClass*>(pType);
				int ownedNow = 0;

				if (pBuildingType && (BuildingTypeExtContainer::Instance.Find(pBuildingType)->PowersUp_Buildings.size() > 0
				|| BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
					ownedNow = BuildingTypeExtData::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
				else
					ownedNow = CountOwnedIncludeDeploy(pThis, pType);

				ownedNow *= pTypeExt->BuildLimitGroup_Factor;

				if ((pItem == pType && ownedNow < limits[i] + 1 - pItemExt->BuildLimitGroup_Factor) || includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					return CanBuildResult::Buildable;

				if ((pItem != pType && ownedNow < limits[i]) || includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					return CanBuildResult::Buildable;
			}

			return CanBuildResult::TemporarilyUnbuildable;
		}
	}
}

BuildLimitStatus HouseExtData::CheckBuildLimit(
	HouseClass const* const pHouse, TechnoTypeClass* pItem,
	bool const includeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int remaining  = HouseExtData::BuildLimitRemaining(pHouse , pItem);

	if (BuildLimit > 0 && remaining <= 0) {
		return !includeQueued || !pHouse->GetFactoryProducing(pItem)
			? BuildLimitStatus::ReachedPermanently
			: BuildLimitStatus::NotReached;
	}

	return (remaining > 0)
		? BuildLimitStatus::NotReached
		: BuildLimitStatus::ReachedTemporarily
		;
}

signed int HouseExtData::BuildLimitRemaining(
	HouseClass const* const pHouse, TechnoTypeClass* pItem)
{
	int BuildLimit = pItem->BuildLimit;

	if (BuildLimit < 0)
	{
		return -(BuildLimit + pHouse->CountOwnedEver(pItem));
	}
	else
	{
		const auto cur = HouseExtData::CountOwnedNowTotal(pHouse, pItem);

		if (cur < 0)
			Debug::FatalError("%s for [%s - %x] CountOwned return less than 0 when counted", pItem->ID, pHouse->Type->ID, pHouse);

		return BuildLimit - cur;
	}
}

int HouseExtData::CountOwnedNowTotal(
	HouseClass const* const pHouse, TechnoTypeClass* pItem)
{
	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
	{
		const BuildingTypeClass* pBType = static_cast<BuildingTypeClass const*>(pItem);
		const char* pPowersUp = pBType->PowersUpBuilding;
		int sum = 0;

		if (pPowersUp[0])
		{
			if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			{
				for (auto const& pBld : pHouse->Buildings)
				{
					if (pBld->Type == pTPowersUp)
					{
						for (auto const& pUpgrade : pBld->Upgrades)
						{
							if (pUpgrade == pBType)
							{
								++sum;
							}
						}
					}
				}
			}
		}
		else
		{
			sum = pHouse->CountOwnedNow(pBType);
			if (auto const pUndeploy = pBType->UndeploysInto)
			{
				sum += pHouse->CountOwnedNow(pUndeploy);
			}
		}

		return sum;
	}
	case AbstractType::UnitType:
	{
		const UnitTypeClass* pUType = static_cast<UnitTypeClass const*>(pItem);
		auto sum = pHouse->CountOwnedNow(pUType);

		if (auto const pDeploy = pUType->DeploysInto)
		{
			sum += pHouse->CountOwnedNow(pDeploy);
		}

		return sum;
	}
	case AbstractType::InfantryType:
	{
		const InfantryTypeClass* pIType = static_cast<InfantryTypeClass const*>(pItem);

		auto sum = pHouse->CountOwnedNow(pIType);

		if (pIType->VehicleThief)
		{
			auto index = pIType->ArrayIndex;

			for (auto pUnit : *UnitClass::Array)
			{
				if (pUnit->HijackerInfantryType == index
					&& pUnit->Owner == pHouse)
				{
					++sum;
				}
			}
		}

		return sum;
	}

	case AbstractType::AircraftType:
	{
		return  pHouse->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));
	}
	default:
		break;
	}

	return 0;
}

void HouseExtData::UpdateTransportReloaders()
{
	HouseExtData::LimboTechno.remove_all_if([](TechnoClass* pTech) {
		if (!pTech || !pTech->IsAlive)
			return true;

		auto vtable = VTable::Get(pTech);

		if (vtable != UnitClass::vtable
			&& vtable != InfantryClass::vtable
			&& vtable != AircraftClass::vtable
			&& vtable != BuildingClass::vtable)
			return true;

		if (pTech->Transporter && pTech->Transporter->IsAlive && pTech->Transporter->IsInLogic) {
			if (TechnoTypeExtContainer::Instance.Find(pTech->GetTechnoType())->ReloadInTransport) {
				pTech->Reload();
			}
		}

		return false;
	});

}

void HouseExtData::UpdateNonMFBFactoryCounts(AbstractType rtti, bool remove, bool isNaval)
{
	int* count = nullptr;

	switch (rtti)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		count = &this->NumAirpads_NonMFB;
		break;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		count = &this->NumConYards_NonMFB;
		break;
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		count = &this->NumBarracks_NonMFB;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		count = isNaval ? &this->NumShipyards_NonMFB : &this->NumWarFactories_NonMFB;
		break;
	default:
		break;
	}

	if (count)
		*count += remove ? -1 : 1;
}

int HouseExtData::GetFactoryCountWithoutNonMFB(AbstractType rtti, bool isNaval)
{
	int count = 0;
	auto pThis = This();

	switch (rtti)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		count = pThis->NumAirpads - this->NumAirpads_NonMFB;
		break;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		count = pThis->NumConYards - this->NumConYards_NonMFB;
		break;
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		count = pThis->NumBarracks - this->NumBarracks_NonMFB;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		if (isNaval)
			count = pThis->NumShipyards - this->NumShipyards_NonMFB;
		else
			count = pThis->NumWarFactories - this->NumWarFactories_NonMFB;
		break;
	default:
		break;
	}

	return std::max(count, 0);
}

int HouseExtData::GetForceEnemyIndex()
{
	return this->ForceEnemyIndex;
}

void HouseExtData::SetForceEnemy(int EnemyIndex)
{
	if (EnemyIndex < 0 && EnemyIndex != -2)
		this->ForceEnemyIndex = -1;
	else
		this->ForceEnemyIndex = EnemyIndex;
}

void HouseExtData::UpdateBattlePoints(int modifier)
{
	this->BattlePoints += modifier;
	this->BattlePoints = this->BattlePoints < 0 ? 0 : this->BattlePoints;
}

bool HouseExtData::AreBattlePointsEnabled()
{
	const auto pThis = This();
	const auto pOwnerTypeExt = HouseTypeExtContainer::Instance.Find(pThis->Type);

		// Structures can enable this logic overwriting the house's setting
	for (auto& collect : this->BattlePointsCollectors) {
		if (collect.second > 0) // just bail on one structure found to avoid performance drop
			return true;
	}

	// Global setting
	if (RulesExtData::Instance()->BattlePoints)
		return true;

	// House specific setting
	if (pOwnerTypeExt->BattlePoints)
		return true;


	return false;
}

bool HouseExtData::CanTransactBattlePoints(int amount) {
	return (amount > 0) || this->BattlePoints >= -amount;
}

int HouseExtData::CalculateBattlePoints(TechnoClass* pTechno)
{
	return pTechno ? CalculateBattlePoints(pTechno->GetTechnoType(), pTechno->Owner) : 0;
}

int HouseExtData::CalculateBattlePoints(TechnoTypeClass* pTechno, HouseClass* pOwner)
{
	const auto pThis = This();
	const auto pThisTypeExt = HouseTypeExtContainer::Instance.Find(pThis->Type);
	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pTechno);

	if (pTechnoTypeExt->BattlePoints.isset() && pTechnoTypeExt->BattlePoints.Get() != 0)
		return pTechnoTypeExt->BattlePoints.Get();
	else if(!pTechnoTypeExt->BattlePoints.isset()){

		const int Points = RulesExtData::Instance()->BattlePoints_DefaultFriendlyValue.isset() && pThis->IsAlliedWith(pOwner) ?
			RulesExtData::Instance()->BattlePoints_DefaultFriendlyValue.Get() :  RulesExtData::Instance()->BattlePoints_DefaultValue;

		if(Points != 0)
			return Points;
	}

	return !pThisTypeExt->BattlePoints_CanUseStandardPoints ? 0 : pTechno->Points;
}

bool HouseExtData::ReverseEngineer(TechnoClass* Victim) {
	auto VictimType = Victim->GetTechnoType();
	auto pVictimData = TechnoTypeExtContainer::Instance.Find(VictimType);

	if (!pVictimData->CanBeReversed)
		return false;

	auto VictimAs = pVictimData->ReversedAs.Get(VictimType);

	if (!VictimAs)
		return false;

	if (HouseExtData::PrereqValidate(This(), VictimType, false, true) != CanBuildResult::Buildable) {
		this->Reversed.emplace(VictimAs);
		if (HouseExtData::RequirementsMet(This(), VictimType) != RequirementStatus::Forbidden) {
			This()->RecheckTechTree = true;
			return true;
		}
	}

	return false;
}

// =============================
// load / save

template <typename T>
void HouseExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Degrades)
		.Process(this->PowerPlantEnhancerBuildings)
		.Process(this->Building_BuildSpeedBonusCounter)
		.Process(this->Building_OrePurifiersCounter)
		.Process(this->BattlePointsCollectors)
		.Process(this->m_ForceOnlyTargetHouseEnemy)
		.Process(this->ForceOnlyTargetHouseEnemyMode)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->AllRepairEventTriggered)
		.Process(this->LastBuildingTypeArrayIdx);

		for (auto& node : this->RepairBaseNodes)
			Stm.Process(node);
		Stm
			.Process(this->LastBuiltNavalVehicleType)
			.Process(this->ProducingNavalUnitTypeIndex)
			.Process(this->LaunchDatas)
			.Process(this->CaptureObjectExecuted)
			.Process(this->DiscoverEvaDelay)
			.Process(this->Tunnels)
			.Process(this->SWLastIndex)
			.Process(this->Batteries)
			.Process(this->AvaibleDocks)
			.Process(this->StolenTech)
			.Process(this->RadarPersist)
			.Process(this->FactoryOwners_GatheredPlansOf)
			.Process(this->Academies)
			.Process(this->TunnelsBuildings)
			.Process(this->Reversed);

		//Debug::LogInfo("Before doing OwnedCountedHarvesters");
   Stm
			.Process(this->OwnedCountedHarvesters);

		//Debug::LogInfo("After doing OwnedCountedHarvesters for");
	Stm
		.Process(this->Is_NavalYardSpied)
		.Process(this->Is_AirfieldSpied)
		.Process(this->Is_ConstructionYardSpied)
		.Process(this->AuxPower)
		.Process(this->KeepAliveCount)
		.Process(this->KeepAliveBuildingCount)
		.Process(this->TiberiumStorage)
		.Process(this->SideTechTree)
		.Process(this->CombatAlertTimer)
		.Process(this->RestrictedFactoryPlants)
		.Process(this->AISellAllDelayTimer)
		.Process(this->OwnedDeployingUnits)
		.Process(this->Common)
		.Process(this->Combat)
		.Process(this->AISuperWeaponDelayTimer)
		.Process(this->NumAirpads_NonMFB)
		.Process(this->NumBarracks_NonMFB)
		.Process(this->NumWarFactories_NonMFB)
		.Process(this->NumConYards_NonMFB)
		.Process(this->NumShipyards_NonMFB)
		.Process(this->SuspendedEMPulseSWs)
		.Process(this->ForceEnemyIndex)
		.Process(this->BattlePoints)
		.Process(this->Productions)
		.Process(this->BestChoicesNaval)
		;
}

bool HouseExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(HouseExtData::LimboTechno)
		.Process(HouseExtData::AutoDeathObjects)
		.Process(HouseExtData::LastGrindingBlanceUnit)
		.Process(HouseExtData::LastGrindingBlanceInf)
		.Process(HouseExtData::LastHarvesterBalance)
		.Process(HouseExtData::LastSlaveBalance)
		.Process(HouseExtData::IsAnyFirestormActive)
		.Process(HouseExtData::CloakEVASpeak)
		.Process(HouseExtData::SubTerraneanEVASpeak)

		.Process(HouseExtContainer::HousesTeams)
		.Process(HouseExtContainer::Civilian)
		.Process(HouseExtContainer::Special)
		.Process(HouseExtContainer::Neutral)
		.Process(HouseExtContainer::CivilianSide)

		.Success();
}

bool HouseExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(HouseExtData::LimboTechno)
		.Process(HouseExtData::AutoDeathObjects)
		.Process(HouseExtData::LastGrindingBlanceUnit)
		.Process(HouseExtData::LastGrindingBlanceInf)
		.Process(HouseExtData::LastHarvesterBalance)
		.Process(HouseExtData::LastSlaveBalance)
		.Process(HouseExtData::IsAnyFirestormActive)
		.Process(HouseExtData::CloakEVASpeak)
		.Process(HouseExtData::SubTerraneanEVASpeak)

		.Process(HouseExtContainer::HousesTeams)
		.Process(HouseExtContainer::Civilian)
		.Process(HouseExtContainer::Special)
		.Process(HouseExtContainer::Neutral)
		.Process(HouseExtContainer::CivilianSide)

		.Success();
}

// =============================
// container

HouseExtContainer HouseExtContainer::Instance;
std::vector<HouseExtData*> Container<HouseExtData>::Array;

void HouseExtContainer::Clear()
{
	HouseExtData::LastGrindingBlanceUnit = 0;
	HouseExtData::LastGrindingBlanceInf = 0;
	HouseExtData::LastHarvesterBalance = 0;
	HouseExtData::LastSlaveBalance = 0;

	HouseExtData::CloakEVASpeak.Stop();
	HouseExtData::SubTerraneanEVASpeak.Stop();

	Civilian = 0;
	Special = 0;
	Neutral = 0;
	CivilianSide = 0;

	HouseExtData::LimboTechno.clear();
	HouseExtData::AutoDeathObjects.clear();
	HouseExtContainer::HousesTeams.clear();
}


/**
 *  Handles expert AI processing.
 *
 *  @author: 09/29/1995 JLB - Created.
 *           10/11/2024 ZivDero - Adjustments for Tiberian Sun
 * *         10/11/2024 Otamaa - Adjustments for YR + Ares + Phobos
 */

int FakeHouseClass::_Expert_AI()
{
	/**
	 *  If there is no enemy assigned to this house, then assign one now. The
	 *  enemy that is closest is picked. However, don't pick an enemy if the
	 *  base has not been established yet.
	 */
	if (this->ExpertAITimer.Expired()) {
		if (RulesExtData::Instance()->AIBiasSpawnCell && !SessionClass::IsCampaign()) {
			if (const auto count = this->ConYards.Count) {
				const auto wayPoint = this->GetSpawnPosition();

				if (wayPoint != -1) {
					const auto center = ScenarioClass::Instance->GetWaypointCoords(wayPoint);
					auto newCenter = center;
					double distanceSquared = 131072.0;

					for (int i = 0; i < count; ++i) {
						if (const auto pBuilding = this->ConYards.GetItem(i)) {
							if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo) {
								if(BuildingExtContainer::Instance.Find(pBuilding)->LimboID < 0){
									const auto newDistanceSquared = pBuilding->GetMapCoords().DistanceFromSquared(center);

									if (newDistanceSquared < distanceSquared) {
										distanceSquared = newDistanceSquared;
										newCenter = pBuilding->GetMapCoords();
									}
								}
							}
						}
					}

					if (newCenter != center) {
						this->BaseSpawnCell = newCenter;
						this->Base.Center = newCenter;
					}
				}
			}
		}

		if (this->EnemyHouseIndex == -1
			&& SessionClass::Instance->GameMode != GameMode::Campaign
			&& !this->Type->MultiplayPassive) {
			auto center = this->BaseCenter.IsValid() ? &this->BaseCenter : &this->BaseSpawnCell;

			if (center->IsValid()) {
				int close = INT_MAX;
				HouseClass* enemy = nullptr;
				for (int i = 0; i < HouseClass::Array->Count; i++) {
					HouseClass* house = HouseClass::Array->Items[i];
					if (house != this && !house->Type->MultiplayPassive && !house->Defeated && !this->IsObserver()) {
						if(!RulesExtData::Instance()->AIAngerOnAlly && this->IsAlliedWith(house))
							continue;

						/**
						 *  Determine a priority value based on distance to the center of the
						 *  candidate base. The higher the value, the better the candidate house
						 *  is to becoming the preferred enemy for this house.
						 */
						auto enemycenter = house->BaseCenter.IsValid() ? house->BaseCenter : house->BaseSpawnCell;
						const int value = (int)center->DistanceFrom(enemycenter);

						/**
						 *  Compare the calculated value for this candidate house and if it is
						 *  greater than the previously recorded maximum, record this house as
						 *  the prime candidate for enemy.
						 */
						if (value < close) {
							close = value;
							enemy = house;
						}
					}
				}
				/**
				 *  Record this closest enemy base as the first enemy to attack.
				 */
				if (enemy) {
					this->UpdateAngerNodes(1, enemy);
				}
			}
		}
	}

	/**
	 *  If the current enemy no longer has a base or is defeated, then don't consider
	 *  that house a threat anymore. Clear out the enemy record and then try
	 *  to find a new enemy.
	 */
	if (this->EnemyHouseIndex != -1) {
		HouseClass* h = HouseClass::Array->Items[this->EnemyHouseIndex];

		if (h->Defeated || this->IsAlliedWith(h) || this->IsObserver()) {
			this->RemoveFromAngerNodes(h);
			this->_GetExtData()->SetForceEnemy(-1);
			this->UpdateAngerNodes(0u, nullptr);
		}
	}

	/**
	 *  Use any ready super weapons.
	 */

	if (!RulesExtData::Instance()->AISuperWeaponDelay.isset()
		&& (!SessionClass::IsCampaign() || this->IQLevel2 >= RulesClass::Instance->SuperWeapons)) {
		this->_AITryFireSW();
	}

	/**
	 *  House state transition check occurs here. Transitions that occur here are ones
	 *  that relate to general base condition rather than specific combat events.
	 *  Typically, this is limited to transitions between normal buildup mode and
	 *  broke mode.
	 */
	if (this->AIMode == AIMode::SellAll) {
		Fire_Sale();
		All_To_Hunt();
	} else {
		if (this->AIMode == AIMode::General) {
			if (this->Available_Money() < 25) {
				this->AIMode = AIMode::LowOnCash;
			}
		}

		if (this->AIMode == AIMode::LowOnCash) {
			if (this->Available_Money() >= 25) {
				this->AIMode = AIMode::General;
			}
		}

		if (this->AIMode == AIMode::BuildBase && this->LATime + 900 < Unsorted::CurrentFrame) {
			this->AIMode = AIMode::General;
		}

		if (this->AIMode != AIMode::BuildBase && this->LATime + 900 > Unsorted::CurrentFrame) {
			this->AIMode = AIMode::BuildBase;
		}
	}

	if (SpawnerMain::GetGameConfigs()->SpawnerHackMPNodes || SessionClass::Instance->GameMode != GameMode::Campaign) {

		const std::array<UrgencyType,2u> urgency = {
			this->AIMode == AIMode::BuildBase ? UrgencyType::None  : this->Check_Fire_Sale()
			,
			this->Check_Raise_Money()
		};

        // Process strategies by priority from 4 down to 1
        for (int priority = (int)UrgencyType::Critical; priority >= (int)UrgencyType::Low; --priority) {
            for (int strat = 0; strat < 2; ++strat) {
                if (urgency[strat] == (UrgencyType)priority) {
                    if (strat == 1) {
                        this->AI_Raise_Money((UrgencyType)priority);
                    } else {
                        this->AI_Fire_Sale((UrgencyType)priority);
                    }
                }
            }
        }
	}

	return ScenarioClass::Instance->Random.RandomRanged(1, 7) + 105;
}

DEFINE_FUNCTION_JUMP(CALL ,0x4F9017, FakeHouseClass::_Expert_AI)
DEFINE_FUNCTION_JUMP(LJMP ,0x4FD500, FakeHouseClass::_Expert_AI)

void FakeHouseClass::_UpdateAngerNodes(int score_add, HouseClass* pHouse)
{
	if (score_add != 0 &&
		pHouse && (SessionClass::Instance->GameMode == GameMode::Campaign || !pHouse->Type->MultiplayPassive)
		) {
		for (int i = 0; i < this->AngerNodes.Count; ++i) {
			AngerStruct* pAnger = &this->AngerNodes.Items[i];
			if (pAnger->House == pHouse) {
				pAnger->AngerLevel += score_add;
			}
		}
	}

	const int forceIndex = this->_GetExtData()->GetForceEnemyIndex();

	if (forceIndex >= 0 || forceIndex == -2) {
		this->EnemyHouseIndex = forceIndex == -2 ? -1 : forceIndex;
		return;
	}

	int _scores = 0;
	HouseClass* pSelected = nullptr;

	for (int a = 0; a < this->AngerNodes.Count; ++a) {
		AngerStruct* pAnger = this->AngerNodes.Items + a;
		if (pAnger->AngerLevel > _scores && !pAnger->House->Defeated && !this->IsAlliedWith(pAnger->House)) {
			_scores = pAnger->AngerLevel;
			pSelected = pAnger->House;
		}
	}

	this->EnemyHouseIndex = pSelected ? pSelected->ArrayIndex : -1;
}

void FakeHouseClass::_AITryFireSW() {
	//if (!pThis->Supers.IsAllocated && !pThis->Supers.IsInitialized)
	//	return;

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	const bool humanControlled = this->IsControlledByHuman();

	for (const auto& pSuper : this->Supers) {
		//Debug::LogInfo("House[%s - %x] Trying To Fire SW[%s - %x]" , pThis->get_ID() , pThis, pSuper->Type->ID , pSuper);
		if (pSuper->IsCharged && pSuper->ChargeDrainState != ChargeDrainState::Draining) {
			if (!humanControlled || SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_AutoFire) {
				SWTypeExtData::TryFire(pSuper, false);
			}
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x504790, FakeHouseClass::_UpdateAngerNodes)

void FakeHouseClass::_BlowUpAll() {
	//safer way
	std::set<TechnoClass*> toBlowUp;

	for (int i = 0; i < TechnoClass::Array->Count; ++i) {

		TechnoClass* techno = TechnoClass::Array->Items[i];

		if (!techno->IsAlive || techno->IsCrashing || techno->IsSinking) {
			continue;
		}

		const auto nUnit = cast_to<UnitClass*, false>(techno);
		if (nUnit && nUnit->DeathFrameCounter > 0) {
			continue;
		}

		HouseClass* myOwner = techno->GetOriginalOwner();
		HouseClass* currentHouse = this;

		const bool isNotOwnedByCurrentHouse = (myOwner != techno->Owner);
		const bool hasOriginalOwner = (techno->MindControlledBy != nullptr);
		const bool shouldSetToCivilian = hasOriginalOwner && techno->MindControlledBy->CaptureManager->SetOriginalOwnerToCivilian(techno);

		if (isNotOwnedByCurrentHouse &&
			(myOwner != currentHouse || shouldSetToCivilian)
			|| myOwner != currentHouse) {
			continue;
		}

		toBlowUp.emplace(techno);
	}

	// Blow them up afterwards
	for (TechnoClass* techno : toBlowUp) {

		if (!techno->IsAlive || techno->IsCrashing || techno->IsSinking) {
			continue;
		}

		const auto nUnit = cast_to<UnitClass*, false>(techno);
		if (nUnit && nUnit->DeathFrameCounter > 0) {
			continue;
		}

		if (TemporalClass* temporal = techno->TemporalTargetingMe) {
			temporal->JustLetGo();
		}

		bool skipDoingDamage = false;

		if (auto pBld = cast_to<BuildingClass* , false>(techno)) {
			// do not return structures in campaigns
			if (!SessionClass::Instance->IsCampaign()) {
				// was the building owned by a neutral country?
				auto pInitialOwner = pBld->InitialOwner;

				if (!pInitialOwner || pInitialOwner->Type->MultiplayPassive) {
					auto pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

					auto occupants = pBld->GetOccupantCount();
					auto canReturn = (pInitialOwner != this) || occupants > 0;

					if (canReturn && pExt->Returnable.Get(RulesExtData::Instance()->ReturnStructures)) {
						// this may change owner
						if (occupants) {
							pBld->KillOccupants(nullptr);
						}

						// don't do this when killing occupants already changed owner
						if (pBld->GetOwningHouse() == this)
						{
							// fallback to first civilian side house, same logic SlaveManager uses
							if (!pInitialOwner)
							{
								pInitialOwner = HouseClass::FindCivilianSide();
							}

							// give to other house and disable
							if (pInitialOwner && pBld->SetOwningHouse(pInitialOwner, false))
							{
								pBld->Guard();

								if (pBld->Type->NeedsEngineer)
								{
									pBld->HasEngineer = false;
									pBld->DisableStuff();
								}

								skipDoingDamage = true;
							}
						}
					}
				}
			}
		}

		if (!skipDoingDamage) {
			int damage = techno->GetTechnoType()->Strength;
			techno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
		}
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x4F87FA, FakeHouseClass::_BlowUpAll)
DEFINE_FUNCTION_JUMP(CALL, 0x4F8F7B, FakeHouseClass::_BlowUpAll)
DEFINE_FUNCTION_JUMP(CALL, 0x6E31C8, FakeHouseClass::_BlowUpAll)
DEFINE_FUNCTION_JUMP(LJMP, 0x4FC6D0, FakeHouseClass::_BlowUpAll)

void FakeHouseClass::_BlowUpAllBuildings() {
	//safer way
	std::set<BuildingClass*> toBlowUp;

	for (int i = 0; i < BuildingClass::Array->Count; ++i)
	{
		BuildingClass* techno = BuildingClass::Array->Items[i];

		if (!techno->IsAlive) {
			continue;
		}

		HouseClass* myOwner = techno->GetOriginalOwner();
		HouseClass* currentHouse = this;

		const bool isNotOwnedByCurrentHouse = (myOwner != techno->Owner);
		const bool hasOriginalOwner = (techno->MindControlledBy != nullptr);
		const bool shouldSetToCivilian = hasOriginalOwner && techno->MindControlledBy->CaptureManager->SetOriginalOwnerToCivilian(techno);

		if (isNotOwnedByCurrentHouse &&
			(myOwner != currentHouse || shouldSetToCivilian)
			|| myOwner != currentHouse)
		{
			continue;
		}

		toBlowUp.emplace(techno);
	}

	// Blow them up afterwards
	for (BuildingClass* pBld : toBlowUp) {

		if (!pBld->IsAlive) {
			continue;
		}

		if (TemporalClass* temporal = pBld->TemporalTargetingMe) {
			temporal->JustLetGo();
		}

		bool skipDoingDamage = false;

		{
			// do not return structures in campaigns
			if (!SessionClass::Instance->IsCampaign())
			{
				// was the building owned by a neutral country?
				auto pInitialOwner = pBld->InitialOwner;

				if (!pInitialOwner || pInitialOwner->Type->MultiplayPassive)
				{
					auto pExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

					auto occupants = pBld->GetOccupantCount();
					auto canReturn = (pInitialOwner != this) || occupants > 0;

					if (canReturn && pExt->Returnable.Get(RulesExtData::Instance()->ReturnStructures))
					{
						// this may change owner
						if (occupants)
						{
							pBld->KillOccupants(nullptr);
						}

						// don't do this when killing occupants already changed owner
						if (pBld->GetOwningHouse() == this)
						{
							// fallback to first civilian side house, same logic SlaveManager uses
							if (!pInitialOwner)
							{
								pInitialOwner = HouseClass::FindCivilianSide();
							}

							// give to other house and disable
							if (pInitialOwner && pBld->SetOwningHouse(pInitialOwner, false))
							{
								pBld->Guard();

								if (pBld->Type->NeedsEngineer)
								{
									pBld->HasEngineer = false;
									pBld->DisableStuff();
								}

								skipDoingDamage = true;
							}
						}
					}
				}
			}
		}

		if (!skipDoingDamage)
		{
			int damage = pBld->GetTechnoType()->Strength;
			pBld->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
		}
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x6E3228, FakeHouseClass::_BlowUpAllBuildings)
DEFINE_FUNCTION_JUMP(LJMP, 0x4FC790, FakeHouseClass::_BlowUpAllBuildings)

void FakeHouseClass::_UpdateRadar() {
	bool radarAvailable = ScenarioClass::Instance->FreeRadar || !this->_GetExtData()->Batteries.empty();
    this->RecheckRadar = 0;

	if (this != HouseClass::CurrentPlayer()) {
    	return;
    }

    // If blackout still has time remaining,
	// just update tactical map availability and exit
    if (this->RadarBlackoutTimer.GetTimeLeft() > 0) {
        if (RadarClass::Instance->IsAvailableNow != 0) {
            RadarClass::Instance->UpdateRadarStatus(0);
        }
        return;
    }

	if(!radarAvailable){
		int power = this->PowerOutput;
        int drain = this->PowerDrain;

        if (power >= drain || !drain || (power > 0 && (double)power / (double)drain >= 1.0)) {

			const bool campaignAI = this->IsControlledByHuman();

            for (int i = 0; i < this->Buildings.Count; ++i) {

                FakeBuildingClass *building = (FakeBuildingClass*)this->Buildings.Items[i];

				if (!building) {
                    continue;
                }

				if (!building->IsAlive) continue;
				if (building->InLimbo) continue;
				if (!building->IsOnMap) continue;
				if (TechnoExtContainer::Instance.Find(building)->AE.DisableRadar) continue;

				const auto pExt = building->_GetExtData();

				if (!pExt->RegisteredJammers.empty()) continue;
				if (building->EMPLockRemaining > 0) continue;
				if (building->IsBeingWarpedOut()) continue;
				if (building->CurrentMission == Mission::Selling) continue;
				if (building->QueuedMission == Mission::Selling) continue;

				BuildingTypeClass* pRadar = nullptr;

				const auto pTypes = building->GetTypes(); // building types include upgrades

				for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin) {

					if (!(*begin)->Radar)
						continue;

					const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(*begin);
					if(!pTypeExt->Radar_RequirePower || ((*begin)->Powered && building->HasPower)){
						pRadar = (*begin);
						break;
					}
				}

				if (pRadar) {

					if	(pExt->LimboID != -1) {
						radarAvailable = true;
						break;
					}

					// Extra campaign/player checks
					const bool discoveredOrNonCampaign = building->DiscoveredByCurrentPlayer
								|| SessionClass::Instance->GameMode != GameMode::Campaign;

					if (!(campaignAI || discoveredOrNonCampaign)) continue;

					radarAvailable = true;
					break; // Found a valid radar
				}
            }
        }
	}

	if (RadarClass::Instance->IsAvailableNow != radarAvailable) {
		RadarClass::Instance->UpdateRadarStatus(radarAvailable);
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x4F8505, FakeHouseClass::_UpdateRadar)
DEFINE_FUNCTION_JUMP(LJMP, 0x508DF0, FakeHouseClass::_UpdateRadar)

void FakeHouseClass::_UpdateSpySat()
{
	const bool IsCurrentPlayer = this->ControlledByCurrentPlayer();
	const bool ItIsCurrentPlayer = this == HouseClass::CurrentPlayer();
	const bool IsCampaign = SessionClass::Instance->GameMode == GameMode::Campaign;
	const bool IsSpysatActulallyAllowed = !IsCampaign ? ItIsCurrentPlayer : IsCurrentPlayer;

	//===============reset all
	this->CostDefensesMult = 1.0;
	this->CostUnitsMult = 1.0;
	this->CostInfantryMult = 1.0;
	this->CostBuildingsMult = 1.0;
	this->CostAircraftMult = 1.0;
	BuildingClass* Spysat = nullptr;

	const auto pHouseExt = this->_GetExtData();

	pHouseExt->Building_BuildSpeedBonusCounter.clear();
	pHouseExt->Building_OrePurifiersCounter.clear();
	pHouseExt->RestrictedFactoryPlants.clear();
	pHouseExt->BattlePointsCollectors.clear();

	this->RecheckRadar = 0;

	int activeCount = this->Buildings.Count;

	if (activeCount <= 0)
	{
		// No buildings, remove shroud if active
		if (this->SpySatActive) {
			MapClass::Instance->Reshroud(this);
			this->SpySatActive = 0;

			if (ItIsCurrentPlayer) {
				VocClass::PlayGlobal(RulesClass::Instance->SpySatDeactivationSound, Panning::Center, 1.0, 0);
			}
		}

		return;
	}

	for (auto const& pBld : this->Buildings)
	{
		if (pBld && pBld->IsAlive && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto pExt = BuildingExtContainer::Instance.Find(pBld);
			const bool IsLimboDelivered = pExt->LimboID != -1;

			if (pBld->GetCurrentMission() == Mission::Selling || pBld->QueuedMission == Mission::Selling)
				continue;

			if (pBld->TemporalTargetingMe
				|| pExt->AboutToChronoshift
				|| pBld->IsBeingWarpedOut())
				continue;

			//const bool Online = pBld->IsPowerOnline(); // check power
			const auto pTypes = pBld->GetTypes(); // building types include upgrades
			const bool Jammered = !pExt->RegisteredJammers.empty();  // is this building jammed

			for (auto begin = pTypes.begin(); begin != pTypes.end() && *begin; ++begin)
			{
				const auto pTypeExt = BuildingTypeExtContainer::Instance.Find(*begin);
				//const auto Powered_ = pBld->IsOverpowered || (!PowerDown && !((*begin)->PowerDrain && LowpOwerHouse));

				const bool IsBattlePointsCollectorPowered = !pTypeExt->BattlePointsCollector_RequirePower || ((*begin)->Powered && pBld->HasPower);
				if (pTypeExt->BattlePointsCollector && IsBattlePointsCollectorPowered)
				{
					++pHouseExt->BattlePointsCollectors[(*begin)];
				}

				const bool IsFactoryPowered = !pTypeExt->FactoryPlant_RequirePower || ((*begin)->Powered && pBld->HasPower);

				//recalculate the multiplier
				if ((*begin)->FactoryPlant && IsFactoryPowered)
				{
					if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
					{
						pHouseExt->RestrictedFactoryPlants.emplace(pBld);
					}

					this->CostDefensesMult *= (*begin)->DefensesCostBonus;
					this->CostUnitsMult *= (*begin)->UnitsCostBonus;
					this->CostInfantryMult *= (*begin)->InfantryCostBonus;
					this->CostBuildingsMult *= (*begin)->BuildingsCostBonus;
					this->CostAircraftMult *= (*begin)->AircraftCostBonus;
				}

				if (IsSpysatActulallyAllowed && !Spysat)
				{
					//only pick avaible spysat
					if (!TechnoExtContainer::Instance.Find(pBld)->AE.DisableSpySat)
					{
						const bool IsSpySatPowered = !pTypeExt->SpySat_RequirePower || ((*begin)->Powered && pBld->HasPower);
						if ((*begin)->SpySat && !Jammered && IsSpySatPowered)
						{
							if (IsLimboDelivered || !IsCampaign || pBld->DiscoveredByCurrentPlayer)
							{
								Spysat = pBld;
							}
						}
					}
				}

				// add eligible building
				if (pTypeExt->SpeedBonus.Enabled && pBld->HasPower)
					++pHouseExt->Building_BuildSpeedBonusCounter[(*begin)];

				const bool IsPurifierRequirePower = !pTypeExt->PurifierBonus_RequirePower || ((*begin)->Powered && pBld->HasPower);
				// add eligible purifier
				if ((*begin)->OrePurifier && IsPurifierRequirePower)
					++pHouseExt->Building_OrePurifiersCounter[(*begin)];
			}
		}
	}

	//count them
	for (auto& purifier : pHouseExt->Building_OrePurifiersCounter)
		this->NumOrePurifiers += purifier.second;

	// If no valid spy sat found, turn off
	if (!Spysat) {

		if (this->SpySatActive) {
			MapClass::Instance->Reshroud(this);
			this->SpySatActive = 0;

			if (ItIsCurrentPlayer) {
				VocClass::PlayGlobal(RulesClass::Instance->SpySatDeactivationSound, Panning::Center, 1.0, 0);
			}
		}

		return;
	}

	// If valid spy sat found and shroud not yet cleared
	if (!this->SpySatActive) {
		MapClass::Instance->Reveal(this);
		this->SpySatActive = 1;

		if (ItIsCurrentPlayer) {
			VocClass::PlayGlobal(RulesClass::Instance->SpySatActivationSound, Panning::Center, 1.0, 0);
		}
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x4F850C, FakeHouseClass::_UpdateSpySat)
DEFINE_FUNCTION_JUMP(LJMP, 0x508F60, FakeHouseClass::_UpdateSpySat)

bool FakeHouseClass::_IsIonCannonEligibleTarget(TechnoClass* pTechno) const
{
	if (!pTechno->IsAlive)
		return false;

	bool allowed = true;
	if (pTechno->InLimbo) {
		if ((pTechno->Transporter && pTechno->Transporter->IsAlive) || (pTechno->BunkerLinkedItem && pTechno->BunkerLinkedItem->IsAlive) || TechnoExtContainer::Instance.Find(pTechno)->GarrisonedIn)
			allowed = true;
		else  if (pTechno->WhatAmI() == AbstractType::Aircraft && ((AircraftClass*)(pTechno))->DockedTo )
			allowed = true;
		else
			allowed = false;
	}

	if (!allowed)
		return false;

	//the fuck ?
	//always target ground
	if (pTechno->InWhichLayer() == Layer::Ground) {
		return true;
	}

	//otherwise consider the factory if the techno still in production
	// hard difficulty shoots the tank in the factory
	if (this->AIDifficulty == AIDifficulty::Hard)
	{
		for (const auto* pFactory : *FactoryClass::Array)
		{
			if (pFactory->Object == pTechno
				&& pFactory->Production.Timer.Rate
				&& !pFactory->IsSuspended)
			{
				return true;
			}
		}
	}

	return false;
}

// =============================
// container hooks

ASMJIT_PATCH(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	if (RulesExtData::Instance()->EnablePowerSurplus)
		pItem->PowerSurplus = RulesClass::Instance->PowerSurplus;

	HouseExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x4F5480, HouseClass_CTOR_NoInt, 0x7)
{
	GET(HouseClass*, pItem, ESI);
	HouseExtContainer::Instance.AllocateNoInit(pItem);
	return 0;
}

ASMJIT_PATCH(0x4F7186, HouseClass_DTOR, 0x8)
{
	GET(HouseClass*, pItem, ESI);
	HouseExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeHouseClass::_Detach(AbstractClass* target, bool all) {
	HouseExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->HouseClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EA8C8,  FakeHouseClass::_Detach)