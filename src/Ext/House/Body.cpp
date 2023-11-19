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

std::vector<int> HouseExtData::AIProduction_CreationFrames;
std::vector<int> HouseExtData::AIProduction_Values;
std::vector<int> HouseExtData::AIProduction_BestChoices;
std::vector<int> HouseExtData::AIProduction_BestChoicesNaval;

int HouseExtData::LastGrindingBlanceUnit = 0;
int HouseExtData::LastGrindingBlanceInf = 0;
int HouseExtData::LastHarvesterBalance = 0;
int HouseExtData::LastSlaveBalance = 0;

bool HouseExtData::IsAnyFirestormActive = false;

CDTimerClass HouseExtData::CloakEVASpeak;
CDTimerClass HouseExtData::SubTerraneanEVASpeak;

RequirementStatus HouseExtData::RequirementsMet(
	HouseClass* pHouse, TechnoTypeClass* pItem)
{

	const auto pData = TechnoTypeExtContainer::Instance.Find(pItem);
	auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (pItem->Unbuildable) {
		return RequirementStatus::Forbidden;
	}

	bool IsHuman = false;

	if (pHouse->IsControlledByHuman_()) {
		IsHuman = true;
		if(pData->HumanUnbuildable || pItem->TechLevel == -1)
			return RequirementStatus::Forbidden;
	}

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

	if (!pHouse->HasAllStolenTech(pItem)) {
		return RequirementStatus::Incomplete;
	}

	if (!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) {
		return RequirementStatus::Forbidden;
	}

	if (!HouseExtData::CheckFactoryOwners(pHouse, pItem)) {
		return RequirementStatus::Incomplete;
	}

	if (auto const pBldType = specific_cast<BuildingTypeClass const*>(pItem)) {
		if (HouseExtData::IsDisabledFromShell(pHouse, pBldType)) {
			return RequirementStatus::Forbidden;
		}
	}

	//if(IsHuman && !HouseExtData::PrerequisitesMet(pHouse, pItem))
	//	return RequirementStatus::Incomplete;

	if (pData->Prerequisite_Power.isset())
	{
		if (pData->Prerequisite_Power <= 0) {
			if (-pData->Prerequisite_Power > pHouse->PowerOutput)
				return RequirementStatus::Incomplete;
		}
		else if (pData->Prerequisite_Power > pHouse->PowerOutput - pHouse->PowerDrain)
		{
			return RequirementStatus::Incomplete;
		}
	}

	return (pHouse->TechLevel >= pItem->TechLevel) ?
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

	if (bCheckCanBuild && pHouse->CanBuild(pType, true, true) <= CanBuildResult::Unbuildable) {
		return { NewFactoryState::NoFactory  , nullptr };
	}

	auto const nWhat = pType->WhatAmI();
	auto const bitsOwners = pType->GetOwners();
	auto const isNaval = pType->Naval;
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);
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

		if (TechnoTypeExtData::CanBeBuiltAt(pType ,pBType))
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

CanBuildResult HouseExtData::PrereqValidate(
	HouseClass* pHouse, TechnoTypeClass* pItem,
	bool buildLimitOnly, bool includeQueued)
{
	const bool IsHuman = pHouse->IsControlledByHuman_();
	//const bool debug = CRT::strcmpi(pItem->ID, "GAOREP") == 0;

	if (!buildLimitOnly)
	{
		const RequirementStatus ReqsMet = HouseExtData::RequirementsMet(pHouse, pItem);
		const auto pItemExt = TechnoTypeExtContainer::Instance.Find(pItem);

		if (ReqsMet <= RequirementStatus::Incomplete) {

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

		if (IsHuman && ReqsMet == RequirementStatus::Complete) {
			if (!HouseExtData::PrerequisitesMet(pHouse, pItem)) {
				return CanBuildResult::Unbuildable;
			}
		}

		const auto res = HouseExtData::HasFactory(pHouse, pItem, true, true, false, true).first;

		if (res <= NewFactoryState::NotFound)
			return CanBuildResult::Unbuildable;

		if (res <= NewFactoryState::Unpowered)
			return CanBuildResult::TemporarilyUnbuildable;
	}

	if (!IsHuman && RulesExtData::Instance()->AllowBypassBuildLimit[pHouse->GetAIDifficultyIndex()]) {
		return CanBuildResult::Buildable;
	}

	if (pItem->WhatAmI() == BuildingTypeClass::AbsID && !BuildingTypeExtContainer::Instance.Find((BuildingTypeClass*)pItem)->PowersUp_Buildings.empty())
		return static_cast<CanBuildResult>(BuildingTypeExtData::CheckBuildLimit(pHouse, (BuildingTypeClass*)pItem, includeQueued));

	return static_cast<CanBuildResult>(HouseExtData::CheckBuildLimit(pHouse, pItem, includeQueued));
}

bool HouseExtData::CheckFactoryOwners(HouseClass* pHouse, TechnoTypeClass* pItem)
{
	auto const pExt = TechnoTypeExtContainer::Instance.Find(pItem);
	auto const pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	if (!pExt->FactoryOwners.empty() || !pExt->FactoryOwners_Forbidden.empty())
	{
		if(!pHouseExt->FactoryOwners_GatheredPlansOf.empty()) {
			for (auto const& pOwner : pExt->FactoryOwners)
			{
				if (!pHouseExt->FactoryOwners_GatheredPlansOf.contains(pOwner))
					continue;

				if (pExt->FactoryOwners_Forbidden.empty() || !pExt->FactoryOwners_Forbidden.Contains(pOwner))
					return true;
			}
		}

		const auto whatItem = pItem->WhatAmI();
		for (auto const& pBld : pHouse->Buildings)
		{
			auto pBldExt = TechnoExtContainer::Instance.Find(pBld);

			if (!pExt->FactoryOwners.Contains(pBldExt->OriginalHouseType))
				continue;

			if (pExt->FactoryOwners_Forbidden.empty() || !pExt->FactoryOwners_Forbidden.Contains(pBldExt->OriginalHouseType)) {
				const auto pBldExt = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				if (pBld->Type->Factory == whatItem || pBldExt->Type->FactoryOwners_HasAllPlans)
				{
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

void HouseExtData::UpdateAcademy(HouseClass* pHouse , BuildingClass* pAcademy, bool added)
{
	HouseExtContainer::Instance.Find(pHouse)->UpdateAcademy(pAcademy , added);
}

void HouseExtData::UpdateAcademy(BuildingClass* pAcademy, bool added)
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

void HouseExtData::ApplyAcademy(HouseClass* pHouse ,TechnoClass* pTechno, AbstractType considerAs)
{
	HouseExtContainer::Instance.Find(pHouse)->ApplyAcademy(pTechno , considerAs);
}

void HouseExtData::ApplyAcademy(
	TechnoClass* const pTechno, AbstractType const considerAs) const
{
	// mutex in effect, ignore academies to fix preplaced order issues.
	// also triggered in game for certain "conversions" like deploy
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
		.push_back_unique(TechnoExtContainer::Instance.Find(pBld)->OriginalHouseType);
}

bool HouseExtData::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem)
{
	for (auto& prereq : TechnoTypeExtContainer::Instance.Find(pItem)->Prerequisites) {
		if (Prereqs::HouseOwnsAll(pThis , prereq.data() , prereq.size())) {
			return true;
		}
	}

	return false;
}

void HouseExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pSection = this->AttachedToObject->PlainName;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	exINI.Read3Bool(pSection, "RepairBaseNodes", this->RepairBaseNodes);

	this->Degrades.Read(exINI, pSection, "Degrades");
}

TunnelData* HouseExtData::GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx)
{
	if (!pHouse || nTunnelIdx >= TunnelTypeClass::Array.size())
		return nullptr;

	auto pHouseExt = HouseExtContainer::Instance.Find(pHouse);

	while (pHouseExt->Tunnels.size() < TunnelTypeClass::Array.size()) {
		pHouseExt->Tunnels.emplace_back().MaxCap = TunnelTypeClass::Array[nTunnelIdx]->Passengers;
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

LauchData HouseExtData::GetShotCount(SuperWeaponTypeClass* pFor)
{
	if((size_t)pFor->ArrayIndex < this->LaunchDatas.size())
		return this->LaunchDatas[pFor->ArrayIndex];

	return {};
}

SuperClass* HouseExtData::IsSuperAvail(int nIdx, HouseClass* pHouse)
{
	if(const auto pSW = pHouse->Supers.GetItemOrDefault(nIdx)){
		if (SWTypeExtContainer::Instance.Find(pSW->Type)->IsAvailable(pHouse)) {
			return pSW;
		}
	}

	return nullptr;
}

int HouseExtData::GetSurvivorDivisor(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && (pTypeExt->SurvivorDivisor.Get(-1) > 0))
		return pTypeExt->SurvivorDivisor;

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		return SideExtContainer::Instance.Find(pSide)->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExtData::GetCrew(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Crew.Get(nullptr))
		return pTypeExt->Crew;

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		return SideExtContainer::Instance.Find(pSide)->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExtData::GetEngineer(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Engineer.Get(nullptr))
		return pTypeExt->Engineer;

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		return SideExtContainer::Instance.Find(pSide)->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExtData::GetTechnician(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Technician.Get(nullptr))
		return pTypeExt->Technician;

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		return SideExtContainer::Instance.Find(pSide)->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExtData::GetDisguise(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Disguise.Get(nullptr))
		return pTypeExt->Disguise;

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
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

	if (pTypeExt && pTypeExt->ParaDropPlane.Get(nullptr)) {
		pRest =  pTypeExt->ParaDropPlane;
	}

	if(!pRest) {
		int iPlane = -1;
		if (const auto pSide = HouseExtData::GetSide(pHouse)) {
			iPlane = SideExtContainer::Instance.Find(pSide)->ParaDropPlane;
		}

		// didn't help. default to the PDPlane like the game does.

		pRest =
		AircraftTypeClass::Array->GetItemOrDefault(iPlane , RulesExtData::Instance()->DefaultParaPlane);
	}

	if(pRest && pRest->Strength == 0 )
		Debug::FatalError("Invalid Paradrop Plane[%s]" , pRest->ID);
	else if (!pRest)
		Debug::FatalError("Invalid Paradrop Plane");

	return pRest;
}

AircraftTypeClass* HouseExtData::GetSpyPlane(HouseClass* pHouse)
{
	AircraftTypeClass* pRest = nullptr;

	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->SpyPlane.Get(nullptr)) {
		pRest = pTypeExt->SpyPlane;
	}

	if(!pRest) {
		if (const auto pSide = HouseExtData::GetSide(pHouse)) {
			const auto pSideExt = SideExtContainer::Instance.Find(pSide);

			if(pSideExt->SpyPlane.Get(nullptr))
				pRest = pSideExt->SpyPlane;
		}
	}

	if(!pRest)
		pRest = AircraftTypeClass::Find(GameStrings::SPYP);

	if(pRest && pRest->Strength == 0 )
		Debug::FatalError("Invalid Spy Plane[%s]" , pRest->ID);
	else if (!pRest)
		Debug::FatalError("Invalid Spy Plane");

	return pRest;
}

UnitTypeClass* HouseExtData::GetHunterSeeker(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->HunterSeeker.Get(nullptr)) {
		return pTypeExt->HunterSeeker;
	}

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		return SideExtContainer::Instance.Find(pSide)->GetHunterSeeker();
	}

	return nullptr;
}

AnimTypeClass* HouseExtData::GetParachuteAnim(HouseClass* pHouse) {
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);
	if (pTypeExt && pTypeExt->ParachuteAnim.Get(nullptr)) {
		return pTypeExt->ParachuteAnim;
	}

	if (const auto pSide = HouseExtData::GetSide(pHouse)) {
		if (auto pAnim = SideExtContainer::Instance.Find(pSide)->ParachuteAnim.Get(RulesClass::Instance->Parachute))
			return pAnim;

		Debug::Log(
			"[GetParachuteAnim] House %s and its side have no valid parachute defined. Rules fallback failed.\n",
			pHouse->get_ID());
	}

	return AnimTypeClass::Find("PARACH");
}

bool HouseExtData::GetParadropContent(HouseClass* pHouse, Iterator<TechnoTypeClass*>& Types, Iterator<int>& Num)
{
	const auto pTypeExt = HouseTypeExtContainer::Instance.TryFind(pHouse->Type);

	// tries to get the house's default contents and falls back to
	// the sides default contents.
	if (pTypeExt && !pTypeExt->ParaDropTypes.empty()) {
		Types = pTypeExt->ParaDropTypes;
		Num = pTypeExt->ParaDropNum;
	}

	// fall back to side specific para drop
	if (!Types) {
		if (const auto pSide = HouseExtData::GetSide(pHouse)) {
			SideExtData* pData = SideExtContainer::Instance.Find(pSide);

			Types = pData->GetParaDropTypes();
			Num = pData->GetParaDropNum();
		}
	}

	return (Types && Num);
}

bool HouseExtData::InvalidateIgnorable(AbstractClass* ptr)
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
	AnnounceInvalidPointer(ActiveTeams, ptr);
	AnnounceInvalidPointer<TechnoClass*>(LimboTechno, ptr, bRemoved);
	AnnounceInvalidPointer<BuildingClass*>(Academies, ptr, bRemoved);

	if(bRemoved)
		AutoDeathObjects.erase((TechnoClass*)ptr);

	for (auto& nTun : Tunnels)
		AnnounceInvalidPointer(nTun.Vector , ptr , bRemoved);

	AnnounceInvalidPointer<SuperClass*>(Batteries, ptr);
}

int HouseExtData::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer()) return 0;

	int result =
	std::count_if(TechnoClass::Array->begin(), TechnoClass::Array->end(),
	[pThis](TechnoClass* techno)
	{
		if (TechnoTypeExtContainer::Instance.Find(techno->GetTechnoType())->IsCountedAsHarvester()
			&& techno->Owner == pThis)
			return TechnoExtData::IsHarvesting(techno);

		return false;
	});

	return result;
}

int HouseExtData::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer() || pThis->Defeated) return 0;

	int result = 0;

	TechnoTypeClass::Array->for_each([&result, pThis](TechnoTypeClass* techno) {
		if (TechnoTypeExtContainer::Instance.Find(techno)->IsCountedAsHarvester()) {
			result += pThis->CountOwnedAndPresent(techno);
		}
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

	const int deltaX = currentCoords.X - targetBaseCoords.X;
	const int deltaY = targetBaseCoords.Y - currentCoords.Y;
	const int distance = (RulesClass::Instance->AISafeDistance + extraDistance) * Unsorted::LeptonsPerCell;
	auto newCoords = GeneralUtils::CalculateCoordsFromDistance(currentCoords, targetBaseCoords, distance);
	auto cellStruct = CellClass::Coord2Cell(newCoords);
	cellStruct = MapClass::Instance->NearByLocation(cellStruct, speedTypeZone, -1, MovementZone::Normal, false, 3, 3, false, false, false, true, cellStruct, false, false);

	return MapClass::Instance->TryGetCellAt(cellStruct);
}

HouseClass* HouseExtData::FindCivilianSide()
{
	return HouseClass::FindBySideIndex(RulesExtData::Instance()->CivilianSideIndex);
}

HouseClass* HouseExtData::FindSpecial()
{
	return HouseClass::FindByCountryIndex(RulesExtData::Instance()->SpecialCountryIndex);
}

HouseClass* HouseExtData::FindNeutral()
{
	return HouseClass::FindByCountryIndex(RulesExtData::Instance()->NeutralCountryIndex);
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
		return HouseExtData::FindCivilianSide();// HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseExtData::FindSpecial();//  HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseExtData::FindNeutral();//  HouseClass::FindNeutral();
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

HouseClass* HouseExtData::GetSlaveHouse(SlaveReturnTo const& kind, HouseClass* const pKiller, HouseClass* const pVictim)
{
	switch (kind)
	{
	case SlaveReturnTo::Killer:
		return pKiller;
	case SlaveReturnTo::Master:
		return pVictim;
	case SlaveReturnTo::Civilian:
		return HouseExtData::FindCivilianSide();
	case SlaveReturnTo::Special:
		return HouseExtData::FindSpecial();
	case SlaveReturnTo::Neutral:
		return HouseExtData::FindNeutral();
	case SlaveReturnTo::Random:
		auto& Random = ScenarioClass::Instance->Random;
		return HouseClass::Array->GetItem(
			Random.RandomFromMax(HouseClass::Array->Count - 1));
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
					&& !HouseExtData::IsObserverPlayer(pHouse)
					&& !pHouse->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pHouse);
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
					&& !HouseExtData::IsObserverPlayer(pHouse))
				{
					housesListIdx.push_back(pHouse);
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

bool HouseExtData::UpdateHarvesterProduction()
{
	auto pThis = this->AttachedToObject;
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
	HouseClass* , int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	auto const bitOwner = 1u << idxParentCountry;

	for (size_t i = start; i < items.size(); ++i) {
		if (items[i]->InOwners(bitOwner)) {
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
		if (!RulesClass::Instance->BuildTech.Contains(const_cast<BuildingTypeClass*>(pItem))) {
			if (pHouse->Supers[pItem->SuperWeapon]->Type->DisableableFromShell) {
				return true;
			}
		}
	}

	return false;
}

void HouseExtData::UpdateAutoDeathObjects()
{
	if (this->AutoDeathObjects.empty())
		return;

	for (const auto& [pThis , nMethod] : this->AutoDeathObjects)
	{
		if (pThis->IsInLogic || !pThis->IsAlive || nMethod == KillMethod::None)
			continue;

		auto const pExt = TechnoExtContainer::Instance.TryFind(pThis);
		if (!pExt)
		{
			Debug::Log("HouseExtData::UpdateAutoDeathObject -  Killing Techno Failed , No Extptr [%x - %s] ! \n", pThis, pThis->get_ID());
			continue;
		}

		if(!pExt->Death_Countdown.Completed())
			continue;

		Debug::Log("HouseExtData::UpdateAutoDeathObject -  Killing Techno[%x - %s] ! \n", pThis, pThis->get_ID());
		if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
			if (BuildingExtContainer::Instance.Find(pBuilding)->LimboID != -1) {
				//this->RemoveFromLimboTracking(pBuilding->Type);
				BuildingExtData::LimboKill(pBuilding);
				continue;
			}
		}

		TechnoExtData::KillSelf(pThis, nMethod, true , TechnoTypeExtContainer::Instance.Find(pExt->Type)->AutoDeath_VanishAnimation);
	}
}

BuildLimitStatus HouseExtData::CheckBuildLimit(
	HouseClass const* const pHouse, TechnoTypeClass* pItem,
	bool const includeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int Remaining = 0;

	if (BuildLimit < 0)
	{
		Remaining = -(BuildLimit + pHouse->CountOwnedEver(pItem));
	}
	else
	{
		Remaining = BuildLimit - HouseExtData::CountOwnedNowTotal(pHouse, pItem);

		if (BuildLimit > 0 && Remaining <= 0) {
			return !includeQueued || !pHouse->GetFactoryProducing(pItem) ?
				BuildLimitStatus::ReachedPermanently : BuildLimitStatus::NotReached;
		}
	}

	return (Remaining > 0)
		? BuildLimitStatus::NotReached
		: BuildLimitStatus::ReachedTemporarily
		;
}

signed int HouseExtData::BuildLimitRemaining(
	HouseClass const* const pHouse, TechnoTypeClass* pItem)
{
	auto const BuildLimit = pItem->BuildLimit;

	if (BuildLimit < 0) {
		return -(BuildLimit + pHouse->CountOwnedEver(pItem));
	} else {
		return BuildLimit - HouseExtData::CountOwnedNowTotal(pHouse, pItem);
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

		if (auto const pDeploy = pUType->DeploysInto) {
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

			for (auto pUnit : *UnitClass::Array) {
				if (pUnit->HijackerInfantryType == index
					&& pUnit->Owner == pHouse) {
					++sum;
				}
			}
		}

		return sum;
	}

	case AbstractType::AircraftType: {
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
	for(auto& pTech : this->LimboTechno) {

		if(pTech->IsAlive
			&& pTech->WhatAmI() != AircraftClass::AbsID
			&& pTech->WhatAmI() != BuildingClass::AbsID
			&& pTech->Transporter) {
				const auto pType = pTech->GetTechnoType();
			if (pType->Ammo > 0 && TechnoTypeExtContainer::Instance.Find(pTech->GetTechnoType())->ReloadInTransport) {
				pTech->Reload();
			}
		}
	}
}

//void HouseExtData::AddToLimboTracking(TechnoTypeClass* pTechnoType)
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
//void HouseExtData::RemoveFromLimboTracking(TechnoTypeClass* pTechnoType)
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
//int HouseExtData::CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType)
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
void HouseExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Degrades)
		.Process(this->PowerPlantEnhancerBuildings)
		.Process(this->Building_BuildSpeedBonusCounter)
		.Process(this->Building_OrePurifiersCounter)
		.Process(this->m_ForceOnlyTargetHouseEnemy)
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
		.Process(this->KeepAliveCount)
		.Process(this->KeepAliveBuildingCount)
		;
}

bool HouseExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(HouseExtData::LastGrindingBlanceUnit)
		.Process(HouseExtData::LastGrindingBlanceInf)
		.Process(HouseExtData::LastHarvesterBalance)
		.Process(HouseExtData::LastSlaveBalance)
		.Process(HouseExtData::IsAnyFirestormActive)
		.Success();
}

bool HouseExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(HouseExtData::LastGrindingBlanceUnit)
		.Process(HouseExtData::LastGrindingBlanceInf)
		.Process(HouseExtData::LastHarvesterBalance)
		.Process(HouseExtData::LastSlaveBalance)
		.Process(HouseExtData::IsAnyFirestormActive)
		.Success();
}

// =============================
// container

HouseExtContainer HouseExtContainer::Instance;

void HouseExtContainer::Clear()
{
	HouseExtData::AIProduction_CreationFrames.clear();
	HouseExtData::AIProduction_Values.clear();
	HouseExtData::AIProduction_BestChoices.clear();
	HouseExtData::AIProduction_BestChoicesNaval.clear();

	HouseExtData::LastGrindingBlanceUnit = 0;
	HouseExtData::LastGrindingBlanceInf = 0;
	HouseExtData::LastHarvesterBalance = 0;
	HouseExtData::LastSlaveBalance = 0;

	HouseExtData::CloakEVASpeak.Stop();
	HouseExtData::SubTerraneanEVASpeak.Stop();
}
// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);
	HouseExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4F7186, HouseClass_DTOR, 0x8)
{
	GET(HouseClass*, pItem, ESI);
	HouseExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	HouseExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExtContainer::Instance.LoadFromINI(pThis, pINI , false);

	return 0;
}

//DEFINE_HOOK(0x4FB9B7, HouseClass_Detach, 0xA)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(void*, target, STACK_OFFSET(0xC, 0x4));
//	GET_STACK(bool, all, STACK_OFFSET(0xC, 0x8));
//
//	HouseExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
//
//	R->ESI(pThis);
//	R->EBX(0);
//	return pThis->ToCapture == target ?
//		0x4FB9C3 : 0x4FB9C9;
//}

void __fastcall HouseClass_Detach_Wrapper(HouseClass* pThis, DWORD, AbstractClass* target, bool all)
{
	HouseExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	pThis->HouseClass::PointerExpired(target, all);
}
DEFINE_JUMP(VTABLE, 0x7EA8C8 , GET_OFFSET(HouseClass_Detach_Wrapper))