#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>

#include <ScenarioClass.h>

#include <New/Type/GenericPrerequisite.h>

std::vector<int> HouseExt::AIProduction_CreationFrames;
std::vector<int> HouseExt::AIProduction_Values;
std::vector<int> HouseExt::AIProduction_BestChoices;
std::vector<int> HouseExt::AIProduction_BestChoicesNaval;

int HouseExt::LastGrindingBlanceUnit = 0;
int HouseExt::LastGrindingBlanceInf = 0;
int HouseExt::LastHarvesterBalance = 0;
int HouseExt::LastSlaveBalance = 0;

CDTimerClass HouseExt::CloakEVASpeak;
CDTimerClass HouseExt::SubTerraneanEVASpeak;

RequirementStatus HouseExt::RequirementsMet(
	HouseClass* pHouse, TechnoTypeClass* pItem)
{

	if (pItem->Unbuildable) {
		return RequirementStatus::Forbidden;
	}

	const auto pData = TechnoTypeExt::ExtMap.Find(pItem);
	auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (!(pData->Prerequisite_RequiredTheaters & (1 << static_cast<int>(ScenarioClass::Instance->Theater)))) {
		return RequirementStatus::Forbidden;
	}

	if (Prereqs::HouseOwnsAny(pHouse, pData->Prerequisite_Negative.data() , pData->Prerequisite_Negative.size())) {
		return RequirementStatus::Forbidden;
	}

	if (pHouseExt->Reversed.contains(pItem)) {
		return RequirementStatus::Overridden;
	}

	if (pData->RequiredStolenTech.any()) {
		if ((pHouseExt->StolenTech & pData->RequiredStolenTech) != pData->RequiredStolenTech) {
			return RequirementStatus::Incomplete;
		}
	}

	if (Prereqs::HouseOwnsAny(pHouse, pItem->PrerequisiteOverride)) {
		return RequirementStatus::Overridden;
	}

	if (pHouse->HasFromSecretLab(pItem)) {
		return RequirementStatus::Overridden;
	}

	if (pHouse->IsControlledByHuman_() && pItem->TechLevel == -1) {
		return RequirementStatus::Incomplete;
	}

	if (!pHouse->HasAllStolenTech(pItem)) {
		return RequirementStatus::Incomplete;
	}

	if (!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) {
		return RequirementStatus::Forbidden;
	}

	if (!HouseExt::CheckFactoryOwners(pHouse, pItem)) {
		return RequirementStatus::Incomplete;
	}

	if (auto const pBldType = specific_cast<BuildingTypeClass const*>(pItem)) {
		if (HouseExt::IsDisabledFromShell(pHouse, pBldType)) {
			return RequirementStatus::Forbidden;
		}
	}

	return (pHouse->TechLevel >= pItem->TechLevel) ?
		RequirementStatus::Complete : RequirementStatus::Incomplete;
}

std::pair<NewFactoryState, BuildingClass*> HouseExt::HasFactory(
	HouseClass* pHouse,
	TechnoTypeClass* pType,
	bool bSkipAircraft,
	bool requirePower,
	bool bCheckCanBuild,
	bool b7)
{

	if (bCheckCanBuild && pHouse->CanBuild(pType, true, true) <= CanBuildResult::Unbuildable) {
		return { NewFactoryState::NoFactory  , nullptr };
	}

	auto const nWhat = pType->WhatAmI();
	auto const bitsOwners = pType->GetOwners();
	auto const isNaval = pType->Naval;
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);
	BuildingClass* pNonPrimaryBuilding = nullptr;
	BuildingClass* pOfflineBuilding = nullptr;

	for (auto const& pBld : pHouse->Buildings)
	{

		if (pBld->InLimbo
			|| pBld->GetCurrentMission() == Mission::Selling
			|| pBld->QueuedMission == Mission::Selling)
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

		if (TechnoTypeExt::CanBeBuiltAt(pType ,pBType))
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

CanBuildResult HouseExt::PrereqValidate(
	HouseClass* pHouse, TechnoTypeClass* pItem,
	bool buildLimitOnly, bool includeQueued)
{
	const bool IsHuman = pHouse->IsControlledByHuman();

	if (!buildLimitOnly)
	{
		const RequirementStatus ReqsMet = HouseExt::RequirementsMet(pHouse, pItem);

		if (ReqsMet <= RequirementStatus::Incomplete){
			return CanBuildResult::Unbuildable;
		}

		if (IsHuman && (ReqsMet == RequirementStatus::Complete)) {
			if (!HouseExt::PrerequisitesMet(pHouse, pItem)) {
				return CanBuildResult::Unbuildable;
			}
		}

		const auto res = HouseExt::HasFactory(pHouse, pItem, true, true, false, true).first;

		if (res <= NewFactoryState::NotFound)
			return CanBuildResult::Unbuildable;

		if (res <= NewFactoryState::Unpowered)
			return CanBuildResult::TemporarilyUnbuildable;
	}

	if (!IsHuman && RulesExt::Global()->AllowBypassBuildLimit[pHouse->GetAIDifficultyIndex()]) {
		return CanBuildResult::Buildable;
	}

	return static_cast<CanBuildResult>(HouseExt::CheckBuildLimit(pHouse, pItem, includeQueued));
}

bool HouseExt::CheckFactoryOwners(HouseClass* pHouse, TechnoTypeClass* pItem)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (!pExt->FactoryOwners.empty() || !pExt->FactoryOwners_Forbidden.empty())
	{
		for (auto const& pOwner : pExt->FactoryOwners)
		{
			if (!pHouseExt->FactoryOwners_GatheredPlansOf.contains(pOwner))
				continue;

			if (pExt->FactoryOwners_Forbidden.empty() || !pExt->FactoryOwners_Forbidden.Contains(pOwner))
				return true;
		}

		for (auto const& pBld : pHouse->Buildings)
		{
			if (!pHouseExt->FactoryOwners_GatheredPlansOf.contains(pBld->align_154->OriginalHouseType))
				continue;

			if (pExt->FactoryOwners_Forbidden.empty() || !pExt->FactoryOwners_Forbidden.Contains(pBld->align_154->OriginalHouseType)) {
				const auto pBldExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

				if (pBld->Type->Factory == pItem->WhatAmI() || pBldExt->Type->FactoryOwners_HasAllPlans)
				{
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

void HouseExt::UpdateAcademy(HouseClass* pHouse , BuildingClass* pAcademy, bool added)
{
	HouseExt::ExtMap.Find(pHouse)->UpdateAcademy(pAcademy , added);
}

void HouseExt::ExtData::UpdateAcademy(BuildingClass* pAcademy, bool added)
{
	// check if added and there already, or removed and not there
	auto it = this->Academies.find(pAcademy);
	if (added == (it != this->Academies.end())) {
		return;
	}

	// now this can be unconditional
	if (added)
	{
		this->Academies.push_back(pAcademy);
	}
	else
	{
		this->Academies.erase(it);
	}
}

void HouseExt::ApplyAcademy(HouseClass* pHouse ,TechnoClass* pTechno, AbstractType considerAs)
{
	HouseExt::ExtMap.Find(pHouse)->ApplyAcademy(pTechno , considerAs);
}

void HouseExt::ExtData::ApplyAcademy(
	TechnoClass* const pTechno, AbstractType const considerAs) const
{
	// mutex in effect, ignore academies to fix preplaced order issues.
	// also triggered in game for certain "conversions" like deploy
	if (Unsorted::ScenarioInit) {
		return;
	}

	auto const pType = pTechno->GetTechnoType();

	// get the academy data for this type
	Valueable<double> BuildingTypeExt::ExtData::* pmBonus = nullptr;
	switch (considerAs)
	{
	case AbstractType::Infantry:
		pmBonus = &BuildingTypeExt::ExtData::AcademyInfantry;
		break;
	case AbstractType::Aircraft:
		pmBonus = &BuildingTypeExt::ExtData::AcademyAircraft;
		break;
	case AbstractType::Unit:
		pmBonus = &BuildingTypeExt::ExtData::AcademyVehicle;
		break;
	default:
		pmBonus = &BuildingTypeExt::ExtData::AcademyBuilding;
		break;
	}

	auto veterancyBonus = 0.0;

	// aggregate the bonuses
	for (auto const& pBld : this->Academies)
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

		auto const isWhitelisted = pExt->AcademyWhitelist.empty()
			|| pExt->AcademyWhitelist.Contains(pType);

		if (isWhitelisted && !pExt->AcademyBlacklist.Contains(pType))
		{
			const auto& data = pExt->*pmBonus;
			veterancyBonus = MaxImpl(veterancyBonus, data.Get());
		}
	}

	// apply the bonus
	if (pType->Trainable)
	{
		auto& value = pTechno->Veterancy.Veterancy;
		if (veterancyBonus > value)
		{
			value = static_cast<float>(MinImpl(
				veterancyBonus, RulesClass::Instance->VeteranCap));
		}
	}
}

void HouseExt::UpdateFactoryPlans(BuildingClass* pBld)
{
	auto Types = pBld->GetTypes();
	auto Types_c = Types.begin();
	while (!*Types_c || !TechnoTypeExt::ExtMap.Find(*Types_c)->FactoryOwners_HaveAllPlans)
	{
		if (++Types_c == Types.end())
			return;
	}

	auto pNewOwnerExt = HouseExt::ExtMap.Find(pBld->Owner);

	if(!pNewOwnerExt->FactoryOwners_GatheredPlansOf.contains(pBld->align_154->OriginalHouseType))
		pNewOwnerExt->FactoryOwners_GatheredPlansOf.push_back(pBld->align_154->OriginalHouseType);
}

bool HouseExt::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem)
{
	for (auto& prereq : TechnoTypeExt::ExtMap.Find(pItem)->Prerequisites) {
		if (Prereqs::HouseOwnsAll(pThis , prereq.data() , prereq.size())) {
			return true;
		}
	}

	return false;
}

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pSection = this->Get()->PlainName;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	exINI.Read3Bool(pSection, "RepairBaseNodes", this->RepairBaseNodes);
}

NOINLINE TunnelData* HouseExt::GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx)
{
	if (!pHouse || nTunnelIdx >= TunnelTypeClass::Array.size())
		return nullptr;

	auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

	while (pHouseExt->Tunnels.size() < TunnelTypeClass::Array.size()) {
		pHouseExt->Tunnels.emplace_back(TunnelData{});
		pHouseExt->Tunnels.back().MaxCap = TunnelTypeClass::Array[nTunnelIdx]->Passengers;
	}

	return &pHouseExt->Tunnels[nTunnelIdx];
}

TunnelData* HouseExt::GetTunnelVector(BuildingTypeClass* pBld, HouseClass* pHouse)
{
	return HouseExt::GetTunnelVector(pHouse, BuildingTypeExt::ExtMap.Find(pBld)->TunnelType);
}

void HouseExt::ExtData::UpdateShotCount(SuperWeaponTypeClass* pFor)
{
	this->LaunchDatas.resize(SuperWeaponTypeClass::Array->Count);
	this->LaunchDatas[pFor->ArrayIndex].Update();
}

void HouseExt::ExtData::UpdateShotCountB(SuperWeaponTypeClass* pFor)
{
	this->LaunchDatas.resize(SuperWeaponTypeClass::Array->Count);

	auto& nData = this->LaunchDatas[pFor->ArrayIndex];

	if ((nData.LastFrame & 0x80000000) != 0)
		nData.LastFrame = Unsorted::CurrentFrame();
}

LauchData HouseExt::ExtData::GetShotCount(SuperWeaponTypeClass* pFor)
{
	if (pFor->ArrayIndex >= (int)this->LaunchDatas.size()) {
		return {};
	}

	return this->LaunchDatas[pFor->ArrayIndex];
}

SuperClass* HouseExt::ExtData::IsSuperAvail(int nIdx, HouseClass* pHouse)
{
	if (nIdx < 0)
		return nullptr;

	const auto pSW = pHouse->Supers[nIdx];

	if (SWTypeExt::ExtMap.Find(pSW->Type)->IsAvailable(pHouse))
		return pSW;

	return nullptr;
}

int HouseExt::GetSurvivorDivisor(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && (pTypeExt->SurvivorDivisor.Get(-1) > 0))
		return pTypeExt->SurvivorDivisor;

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExt::GetCrew(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Crew.Get(nullptr))
		return pTypeExt->Crew;

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::GetEngineer(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Engineer.Get(nullptr))
		return pTypeExt->Engineer;

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExt::GetTechnician(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Technician.Get(nullptr))
		return pTypeExt->Technician;

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::GetDisguise(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Disguise.Get(nullptr))
		return pTypeExt->Disguise;

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetDisguise();
	}

	return nullptr;
}

AircraftTypeClass* HouseExt::GetParadropPlane(HouseClass* pHouse)
{
	// tries to get the house's default plane and falls back to
	// the sides default plane.
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->ParaDropPlane.Get(nullptr)) {
		return pTypeExt->ParaDropPlane;
	}

	int iPlane = -1;
	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		iPlane = SideExt::ExtMap.Find(pSide)->ParaDropPlane;
	}

	// didn't help. default to the PDPlane like the game does.

	return AircraftTypeClass::Array->GetItemOrDefault(iPlane , RulesExt::Global()->DefaultParaPlane);
}

AircraftTypeClass* HouseExt::GetSpyPlane(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->SpyPlane.Get(nullptr)) {
		return pTypeExt->SpyPlane;
	}

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		const auto pSideExt = SideExt::ExtMap.Find(pSide);

		if(pSideExt->SpyPlane.Get(nullptr))
			return pSideExt->SpyPlane;
	}

	return AircraftTypeClass::Find(GameStrings::SPYP);
}

UnitTypeClass* HouseExt::GetHunterSeeker(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->HunterSeeker.Get(nullptr)) {
		return pTypeExt->HunterSeeker;
	}

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetHunterSeeker();
	}

	return nullptr;
}

AnimTypeClass* HouseExt::GetParachuteAnim(HouseClass* pHouse) {
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->ParachuteAnim.Get(nullptr)) {
		return pTypeExt->ParachuteAnim;
	}

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		if (auto pAnim = SideExt::ExtMap.Find(pSide)->ParachuteAnim.Get(RulesClass::Instance->Parachute))
			return pAnim;

		Debug::Log(
			"[GetParachuteAnim] House %s and its side have no valid parachute defined. Rules fallback failed.\n",
			pHouse->get_ID());
	}

	return AnimTypeClass::Find("PARACH");
}

bool HouseExt::GetParadropContent(HouseClass* pHouse, Iterator<TechnoTypeClass*>& Types, Iterator<int>& Num)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	// tries to get the house's default contents and falls back to
	// the sides default contents.
	if (pTypeExt && pTypeExt->ParaDropTypes.size()) {
		Types = pTypeExt->ParaDropTypes;
		Num = pTypeExt->ParaDropNum;
	}

	// fall back to side specific para drop
	if (!Types) {
		if (const auto pSide = HouseExt::GetSide(pHouse)) {
			SideExt::ExtData* pData = SideExt::ExtMap.Find(pSide);

			Types = pData->GetParaDropTypes();
			Num = pData->GetParaDropNum();
		}
	}

	return (Types && Num);
}

bool HouseExt::ExtData::InvalidateIgnorable(AbstractClass* ptr)
{
	switch (VTable::Get(ptr))
	{
	case BuildingClass::vtable:
	case InfantryClass::vtable:
	case UnitClass::vtable:
	case AircraftClass::vtable:
	case TeamClass::vtable:
	case SuperClass::vtable:
		return false;
	}

	return true;

}

void HouseExt::ExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved)
{
	if (ptr == nullptr)
		return;

	//AnnounceInvalidPointer(OwnedTechno, ptr , bRemoved);
	AnnounceInvalidPointer(Factory_BuildingType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_InfantryType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_VehicleType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_NavyType, ptr, bRemoved);
	AnnounceInvalidPointer(Factory_AircraftType, ptr, bRemoved);
	AnnounceInvalidPointer(ActiveTeams, ptr);
	AnnounceInvalidPointer<TechnoClass*>(LimboTechno, ptr, bRemoved);
	AnnounceInvalidPointer<BuildingClass*>(Academies, ptr, bRemoved);
	AnnounceInvalidPointer<TechnoClass*>(OwnedTransportReloaders, ptr, bRemoved);

	if(bRemoved)
		AutoDeathObjects.erase((TechnoClass*)ptr);

	for (auto& nTun : Tunnels)
		AnnounceInvalidPointer(nTun.Vector , ptr , bRemoved);

	AnnounceInvalidPointer<SuperClass*>(Batteries, ptr);
}

int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer()) return 0;

	int result =
	std::count_if(TechnoClass::Array->begin(), TechnoClass::Array->end(),
	[pThis](TechnoClass* techno)
	{
		if (TechnoTypeExt::ExtMap.Find(techno->GetTechnoType())->IsCountedAsHarvester()
			&& techno->Owner == pThis)
			return TechnoExt::IsHarvesting(techno);

		return false;
	});

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer() || pThis->Defeated) return 0;

	int result = 0;

	TechnoTypeClass::Array->for_each([&result, pThis](TechnoTypeClass* techno) {
		if (TechnoTypeExt::ExtMap.Find(techno)->IsCountedAsHarvester()) {
			result += pThis->CountOwnedAndPresent(techno);
		}
	});

	return result;
}

// This basically gets same cell that AI script action 53 Gather at Enemy Base uses, and code for that (0x6EF700) was used as reference here.
CellClass* HouseExt::GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, const CoordStruct& defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance)
{
	if (!pTargetHouse || !pCurrentHouse)
		return nullptr;

	const auto targetBaseCoords = CellClass::Cell2Coord(pTargetHouse->GetBaseCenter());

	if (targetBaseCoords == CoordStruct::Empty)
		return nullptr;

	auto currentCoords = CellClass::Cell2Coord(pCurrentHouse->GetBaseCenter());

	if (currentCoords == CoordStruct::Empty)
		currentCoords = defaultCurrentCoords;

	const int deltaX = currentCoords.X - targetBaseCoords.X;
	const int deltaY = targetBaseCoords.Y - currentCoords.Y;
	const int distance = (RulesClass::Instance->AISafeDistance + extraDistance) * Unsorted::LeptonsPerCell;

	const double atan = Math::atan2((double)deltaY, (double)deltaX);
	const double radians = (((atan - Math::HalfPi) * (1.0 / Math::BinaryAngleMagic)) - 0X3FFF) * Math::BinaryAngleMagic;
	const int x = static_cast<int>(targetBaseCoords.X + Math::cos(radians) * distance);
	const int y = static_cast<int>(targetBaseCoords.Y - Math::sin(radians) * distance);

	auto newCoords = CoordStruct { x, y, targetBaseCoords.Z };
	auto cellStruct = CellClass::Coord2Cell(newCoords);
	cellStruct = MapClass::Instance->NearByLocation(cellStruct, speedTypeZone, -1, MovementZone::Normal, false, 3, 3, false, false, false, true, cellStruct, false, false);

	return MapClass::Instance->TryGetCellAt(cellStruct);
}

HouseClass* HouseExt::FindCivilianSide()
{
	return HouseClass::FindBySideIndex(RulesExt::Global()->CivilianSideIndex);
}

HouseClass* HouseExt::FindSpecial()
{
	return HouseClass::FindByCountryIndex(RulesExt::Global()->SpecialCountryIndex);
}

HouseClass* HouseExt::FindNeutral()
{
	return HouseClass::FindByCountryIndex(RulesExt::Global()->NeutralCountryIndex);
}

void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode = -1)
{
	const auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	if (mode < 0 || mode > 2)
		mode = -1;

	enum { ForceFalse = 0, ForceTrue = 1, ForceRandom = 2, UseDefault = -1 };

	pHouseExt->ForceOnlyTargetHouseEnemyMode = mode;

	switch (mode)
	{
	case ForceFalse:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;

	case ForceTrue:
		pHouseExt->ForceOnlyTargetHouseEnemy = true;
		break;

	case ForceRandom:
		pHouseExt->ForceOnlyTargetHouseEnemy = ScenarioClass::Instance->Random.RandomBool();
		break;

	default:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;
	}
}

// Ares
HouseClass* HouseExt::GetHouseKind(OwnerHouseKind const& kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind)
	{
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseExt::FindCivilianSide();// HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseExt::FindSpecial();//  HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseExt::FindNeutral();//  HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom)
		{
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomFromMax(HouseClass::Array->Count - 1));
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

HouseClass* HouseExt::GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* const pKiller, HouseClass* const pVictim)
{
	switch (kind)
	{
	case SlaveReturnTo::Killer:
		return pKiller;
	case SlaveReturnTo::Master:
		return pVictim;
	case SlaveReturnTo::Civilian:
		return HouseExt::FindCivilianSide();
	case SlaveReturnTo::Special:
		return HouseExt::FindSpecial();
	case SlaveReturnTo::Neutral:
		return HouseExt::FindNeutral();
	case SlaveReturnTo::Random:
		auto& Random = ScenarioClass::Instance->Random;
		return HouseClass::Array->GetItem(
			Random.RandomFromMax(HouseClass::Array->Count - 1));
	}

	return pKiller;
}

bool HouseExt::IsObserverPlayer()
{
	auto const pCur = HouseClass::CurrentPlayer();

	if (!pCur)
		return false;

	if (pCur == HouseClass::Observer)
		return true;

	return false;
}

bool HouseExt::IsObserverPlayer(HouseClass* pCur)
{
	if (!pCur)
		return false;

	if (pCur == HouseClass::Observer())
		return true;

	return false;
}

int HouseExt::GetHouseIndex(int param, TeamClass* pTeam = nullptr, TActionClass* pTAction = nullptr)
{
	// Special case that returns the house index of the TeamClass object or the Trigger Action
	if (param == 8997) {
		if(pTAction)
			return (pTeam ? pTeam->Owner->ArrayIndex : pTAction->TeamType->Owner->ArrayIndex);
		else
			return -1;
	}

	if (param < 0)
	{
		std::vector<HouseClass*> housesListIdx {};

		switch (param)
		{
		case -1:
		{
			// Random non-neutral
			for (auto pHouse : *HouseClass::Array) {
				if (!pHouse->Defeated
					&& !HouseExt::IsObserverPlayer(pHouse)
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.emplace_back(pHouse);
				}
			}

			return housesListIdx.empty() ?
				-1 :
				housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1)]->ArrayIndex;
		}
		case -2:
		{
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array) {
				if (pHouseNeutral->IsNeutral()) {
					return pHouseNeutral->ArrayIndex;
				}
			}

			return -1;
		}
		case -3:
		{
			// Random Human Player
			for (auto pHouse : *HouseClass::Array) {
				if (pHouse->IsControlledByHuman()
					&& !pHouse->Defeated
					&& !HouseExt::IsObserverPlayer(pHouse))
				{
					housesListIdx.emplace_back(pHouse);
				}
			}

			return housesListIdx.empty() ?
				-1 :
				housesListIdx[(ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1))]
				->ArrayIndex;
		}
		default:
			return -1;
		}
	}

	// Transtale the Multiplayer index into a valid index for the HouseClass array
	if (HouseClass::Index_IsMP(param)) {
		if(HouseClass* pHouse = HouseClass::FindByIndex(param)) {
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
	if (HouseClass* pHouse = HouseClass::FindByCountryIndex(param)) {
		if (!pHouse->Defeated && !pHouse->IsObserver())
		{
				return pHouse->ArrayIndex;
		}

		return -1;
	}


	return -1;
}

bool HouseExt::ExtData::UpdateHarvesterProduction()
{
	auto pThis = this->OwnerObject();
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	const auto idxParentCountry = pThis->Type->FindParentCountryIndex();
	const auto pHarvesterUnit = HouseExt::FindOwned(pThis, idxParentCountry, make_iterator(RulesClass::Instance->HarvesterUnit));

	if (pHarvesterUnit)
	{
		const auto harvesters = pThis->CountResourceGatherers;
		const auto maxHarvesters = HouseExt::FindBuildable(
			pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery))
			? RulesClass::Instance->HarvestersPerRefinery[AIDifficulty] * pThis->CountResourceDestinations
			: RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

		if (pThis->IQLevel2 >= RulesClass::Instance->Harvester && !pThis->IsTiberiumShort
			&& !pThis->IsControlledByHuman_() && harvesters < maxHarvesters
			&& pThis->TechLevel >= pHarvesterUnit->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvesterUnit->ArrayIndex;
			return true;
		}
	}
	else
	{
		if (pThis->CountResourceGatherers < RulesClass::Instance->AISlaveMinerNumber[AIDifficulty])
		{
			if (const auto pRefinery = HouseExt::FindBuildable(
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

size_t HouseExt::FindOwnedIndex(
	HouseClass* , int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	auto const bitOwner = 1u << idxParentCountry;

	for (auto i = start; i < items.size(); ++i)
	{
		auto const pItem = items[i];

		if (pItem->InOwners(bitOwner))
		{
			return i;
		}
	}

	return items.size();
}

bool HouseExt::IsDisabledFromShell(
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
		auto const pItem2 = const_cast<BuildingTypeClass*>(pItem);
		auto const& BuildTech = RulesClass::Instance->BuildTech;
		if (BuildTech.FindItemIndex(pItem2) == -1)
		{
			auto const pSuper = pHouse->Supers[pItem->SuperWeapon];
			if (pSuper->Type->DisableableFromShell)
			{
				return true;
			}
		}
	}

	return false;
}

size_t HouseExt::FindBuildableIndex(
	HouseClass* pHouse, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	for (auto i = start; i < items.size(); ++i)
	{
		auto const pItem = items[i];

		if (pHouse->CanExpectToBuild(pItem, idxParentCountry))
		{
			auto const pBld = specific_cast<const BuildingTypeClass*>(pItem);
			if (pBld && HouseExt::IsDisabledFromShell(pHouse, pBld))
			{
				continue;
			}

			return i;
		}
	}

	return items.size();
}

void HouseExt::ExtData::UpdateAutoDeathObjects()
{
	if (this->AutoDeathObjects.empty())
		return;

	for (const auto& [pThis , nMethod] : this->AutoDeathObjects)
	{
		if (pThis->IsInLogic || !pThis->IsAlive || nMethod == KillMethod::None)
			continue;

		auto const pExt = TechnoExt::ExtMap.TryFind(pThis);
		if (!pExt)
		{
			Debug::Log("HouseExt::ExtData::UpdateAutoDeathObject -  Killing Techno Failed , No Extptr [%x - %s] ! \n", pThis, pThis->get_ID());
			continue;
		}


		if(!pExt->Death_Countdown.Completed())
			continue;

		Debug::Log("HouseExt::ExtData::UpdateAutoDeathObject -  Killing Techno[%x - %s] ! \n", pThis, pThis->get_ID());
		if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
			if (BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1) {
				//this->RemoveFromLimboTracking(pBuilding->Type);
				BuildingExt::LimboKill(pBuilding);
				continue;
			}
		}

		TechnoExt::KillSelf(pThis, nMethod, true , TechnoTypeExt::ExtMap.Find(pExt->Type)->AutoDeath_VanishAnimation);
	}
}

BuildLimitStatus HouseExt::CheckBuildLimit(
	HouseClass const* const pHouse, TechnoTypeClass* pItem,
	bool const includeQueued)
{
	if (pItem->BuildLimit < 0)
	{
		return (-(pItem->BuildLimit + pHouse->CountOwnedEver(pItem))) > 0
			? BuildLimitStatus::NotReached
			: BuildLimitStatus::ReachedTemporarily
			;
	}

	const int remaining = pItem->BuildLimit - HouseExt::CountOwnedNowTotal(pHouse, pItem);

	if (pItem->BuildLimit > 0 && remaining <= 0)
	{
		if (!includeQueued || pHouse->GetFactoryProducing(pItem))
		{
			return BuildLimitStatus::ReachedPermanently;
		}

		BuildLimitStatus::NotReached;
	}

	return remaining > 0 ?
		BuildLimitStatus::NotReached
		: BuildLimitStatus::ReachedTemporarily;
}

signed int HouseExt::BuildLimitRemaining(
	HouseClass const* const pHouse, TechnoTypeClass* pItem)
{
	auto const BuildLimit = pItem->BuildLimit;
	if (BuildLimit >= 0)
	{
		return BuildLimit - HouseExt::CountOwnedNowTotal(pHouse, pItem);
	}
	else
	{
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
	}
}

int HouseExt::CountOwnedNowTotal(
	HouseClass const* const pHouse, TechnoTypeClass* pItem)
{
	int index = -1;
	int sum = 0;
	const BuildingTypeClass* pBType = nullptr;
	const UnitTypeClass* pUType = nullptr;
	const InfantryTypeClass* pIType = nullptr;
	const char* pPowersUp = nullptr;

	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
		pBType = static_cast<BuildingTypeClass const*>(pItem);
		pPowersUp = pBType->PowersUpBuilding;
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
		break;

	case AbstractType::UnitType:
		pUType = static_cast<UnitTypeClass const*>(pItem);
		sum = pHouse->CountOwnedNow(pUType);
		if (auto const pDeploy = pUType->DeploysInto)
		{
			sum += pHouse->CountOwnedNow(pDeploy);
		}
		break;

	case AbstractType::InfantryType:
		pIType = static_cast<InfantryTypeClass const*>(pItem);
		sum = pHouse->CountOwnedNow(pIType);
		if (pIType->VehicleThief)
		{
			index = pIType->ArrayIndex;
			for (auto const& pUnit : *UnitClass::Array)
			{
				if (pUnit->HijackerInfantryType == index
					&& pUnit->Owner == pHouse)
				{
					++sum;
				}
			}
		}
		break;

	case AbstractType::AircraftType:
		sum = pHouse->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));
		break;

	default:
		break;
	}

	return sum;
}

void HouseExt::ExtData::UpdateTransportReloaders()
{
	for (auto const pTechno : this->OwnedTransportReloaders) {
		if (pTechno->Transporter)
			pTechno->Reload();
	}
}

//void HouseExt::ExtData::AddToLimboTracking(TechnoTypeClass* pTechnoType)
//{
//	if (pTechnoType)
//	{
//		int arrayIndex = pTechnoType->GetArrayIndex();
//
//		switch (pTechnoType->WhatAmI())
//			// I doubt those in LimboDelete being really necessary, they're gonna be updated either next frame or after uninit anyway
//		{
//		case AbstractType::AircraftType:
//			this->LimboAircraft.Increment(arrayIndex);
//			break;
//		case AbstractType::BuildingType:
//			this->LimboBuildings.Increment(arrayIndex);
//			break;
//		case AbstractType::InfantryType:
//			this->LimboInfantry.Increment(arrayIndex);
//			break;
//		case AbstractType::UnitType:
//			this->LimboVehicles.Increment(arrayIndex);
//			break;
//		default:
//			break;
//		}
//	}
//}
//
//void HouseExt::ExtData::RemoveFromLimboTracking(TechnoTypeClass* pTechnoType)
//{
//	if (pTechnoType)
//	{
//		int arrayIndex = pTechnoType->GetArrayIndex();
//
//		switch (pTechnoType->WhatAmI())
//		{
//		case AbstractType::AircraftType:
//			this->LimboAircraft.Decrement(arrayIndex);
//			break;
//		case AbstractType::BuildingType:
//			this->LimboBuildings.Decrement(arrayIndex);
//			break;
//		case AbstractType::InfantryType:
//			this->LimboInfantry.Decrement(arrayIndex);
//			break;
//		case AbstractType::UnitType:
//			this->LimboVehicles.Decrement(arrayIndex);
//			break;
//		default:
//			break;
//		}
//	}
//}
//
//int HouseExt::ExtData::CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType)
//{
//	int count = this->OwnerObject()->CountOwnedAndPresent(pTechnoType);
//	int arrayIndex = pTechnoType->GetArrayIndex();
//
//	switch (pTechnoType->WhatAmI())
//	{
//	case AbstractType::AircraftType:
//		count += this->LimboAircraft.GetItemCount(arrayIndex);
//		break;
//	case AbstractType::BuildingType:
//		count += this->LimboBuildings.GetItemCount(arrayIndex);
//		break;
//	case AbstractType::InfantryType:
//		count += this->LimboInfantry.GetItemCount(arrayIndex);
//		break;
//	case AbstractType::UnitType:
//		count += this->LimboVehicles.GetItemCount(arrayIndex);
//		break;
//	default:
//		break;
//	}
//
//	return count;
//}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->PowerPlantEnhancerBuildings)
		.Process(this->Building_BuildSpeedBonusCounter)
		.Process(this->ForceOnlyTargetHouseEnemy)
		.Process(this->ForceOnlyTargetHouseEnemyMode)
		//.Process(this->RandomNumber)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->AllRepairEventTriggered)
		.Process(this->LastBuildingTypeArrayIdx)
		.Process(this->RepairBaseNodes)
		.Process(this->ActiveTeams)
		.Process(this->LastBuiltNavalVehicleType)
		.Process(this->ProducingNavalUnitTypeIndex)

		.Process(this->AutoDeathObjects)
		.Process(this->LaunchDatas)
		.Process(this->CaptureObjectExecuted)
		.Process(this->DiscoverEvaDelay)

		.Process(this->Tunnels)
		.Process(this->Seed)

		.Process(this->SWLastIndex)
		.Process(this->Batteries)
		.Process(this->Factories_HouseTypes)
		.Process(this->LimboTechno)
		.Process(this->OwnedTransportReloaders)
		.Process(this->AvaibleDocks)

		.Process(this->StolenTech)
		.Process(this->RadarPersist)
		.Process(this->FactoryOwners_GatheredPlansOf)
		.Process(this->Academies)
		.Process(this->Reversed)


		.Process(this->Is_NavalYardSpied)
		.Process(this->Is_AirfieldSpied)
		.Process(this->Is_ConstructionYardSpied)
		.Process(this->AuxPower)
		;
}

bool HouseExt::ExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(HouseExt::LastGrindingBlanceUnit)
		.Process(HouseExt::LastGrindingBlanceInf)
		.Process(HouseExt::LastHarvesterBalance)
		.Process(HouseExt::LastSlaveBalance)
		.Success();
}

bool HouseExt::ExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(HouseExt::LastGrindingBlanceUnit)
		.Process(HouseExt::LastGrindingBlanceInf)
		.Process(HouseExt::LastHarvesterBalance)
		.Process(HouseExt::LastSlaveBalance)
		.Success();
}

// =============================
// container

HouseExt::ExtContainer HouseExt::ExtMap;
HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") { }
HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);
	HouseExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4F7186, HouseClass_DTOR, 0x8)
{
	GET(HouseClass*, pItem, ESI);
	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	HouseExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI , false);

	return 0;
}

//DEFINE_HOOK(0x4FB9B7, HouseClass_Detach, 0xA)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(void*, target, STACK_OFFSET(0xC, 0x4));
//	GET_STACK(bool, all, STACK_OFFSET(0xC, 0x8));
//
//	HouseExt::ExtMap.InvalidatePointerFor(pThis, target, all);
//
//	R->ESI(pThis);
//	R->EBX(0);
//	return pThis->ToCapture == target ?
//		0x4FB9C3 : 0x4FB9C9;
//}

void __fastcall HouseClass_Detach_Wrapper(HouseClass* pThis, DWORD, AbstractClass* target, bool all)
{
	HouseExt::ExtMap.InvalidatePointerFor(pThis, target, all);
	pThis->HouseClass::PointerExpired(target, all);
}
DEFINE_JUMP(VTABLE, 0x7EA8C8 , GET_OFFSET(HouseClass_Detach_Wrapper))