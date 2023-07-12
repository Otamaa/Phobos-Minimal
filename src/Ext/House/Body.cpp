#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>

#include <ScenarioClass.h>

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

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pSection = this->Get()->PlainName;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	exINI.Read3Bool(pSection, "RepairBaseNodes", this->RepairBaseNodes);
}

//TODO : remove NOINLINE
void NOINLINE HouseExt::ExtData::UpdateShotCount(SuperWeaponTypeClass* pFor)
{
	SuperExt::ExtMap.Find(this->Get()->Supers[pFor->ArrayIndex])->LauchDatas.Update();
}

//TODO : remove NOINLINE
void NOINLINE HouseExt::ExtData::UpdateShotCountB(SuperWeaponTypeClass* pFor)
{
	auto& nData = SuperExt::ExtMap.Find(this->Get()->Supers[pFor->ArrayIndex])->LauchDatas;

	if ((nData.LastFrame & 0x80000000) != 0)
		nData.LastFrame = Unsorted::CurrentFrame();
}

//TODO : remove NOINLINE
LauchData NOINLINE HouseExt::ExtData::GetShotCount(SuperWeaponTypeClass* pFor)
{
	return SuperExt::ExtMap.Find(this->Get()->Supers[pFor->ArrayIndex])->LauchDatas;
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

	if (pTypeExt && pTypeExt->SurvivorDivisor.isset())
		return pTypeExt->SurvivorDivisor.Get();

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExt::GetCrew(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Crew.isset())
		return pTypeExt->Crew.Get();

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::GetEngineer(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Engineer.isset())
		return pTypeExt->Engineer.Get();

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExt::GetTechnician(HouseClass* pHouse)
{
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->Technician.isset())
		return pTypeExt->Technician.Get();

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

AircraftTypeClass* HouseExt::GetParadropPlane(HouseClass* pHouse)
{
	// tries to get the house's default plane and falls back to
	// the sides default plane.
	const auto pTypeExt = HouseTypeExt::ExtMap.TryFind(pHouse->Type);

	if (pTypeExt && pTypeExt->ParaDropPlane) {
		return pTypeExt->ParaDropPlane;
	}

	int iPlane = -1;
	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		iPlane = SideExt::ExtMap.Find(pSide)->ParaDropPlane;
	}

	// didn't help. default to the PDPlane like the game does.
	if (iPlane < 0) {
		return RulesExt::Global()->DefaultParaPlane;
	}

	return AircraftTypeClass::Array->GetItemOrDefault(iPlane);
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
	if (pTypeExt && pTypeExt->HunterSeeker.isset()) {
		return pTypeExt->HunterSeeker;
	}

	if (const auto pSide = HouseExt::GetSide(pHouse)) {
		return SideExt::ExtMap.Find(pSide)->GetHunterSeeker();
	}

	return nullptr;
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

bool HouseExt::ExtData::InvalidateIgnorable(void* ptr) const
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

void HouseExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	if (ptr == nullptr)
		return;

	AnnounceInvalidPointer(HouseAirFactory, reinterpret_cast<BuildingClass*>(ptr));
	AnnounceInvalidPointer(Factory_BuildingType, ptr);
	AnnounceInvalidPointer(Factory_InfantryType, ptr);
	AnnounceInvalidPointer(Factory_VehicleType, ptr);
	AnnounceInvalidPointer(Factory_NavyType, ptr);
	AnnounceInvalidPointer(Factory_AircraftType, ptr);
	AnnounceInvalidPointer(ActiveTeams, ptr);

	if (!AutoDeathObjects.empty()) {
		AutoDeathObjects.erase(reinterpret_cast<TechnoClass*>(ptr));
	}


	for (auto& nTun : Tunnels)
		AnnounceInvalidPointer(nTun.Vector , ptr);

	AnnounceInvalidPointer(Batteries, ptr);
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

	std::for_each(TechnoTypeClass::Array->begin(), TechnoTypeClass::Array->end(),
	[&result, pThis](TechnoTypeClass* techno)
	{
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
	if ((pTeam && pTAction) || (param == 8997 && !pTeam && !pTAction))
		return -1;

	int houseIdx = -1;
	std::vector<int> housesListIdx;

	// Transtale the Multiplayer index into a valid index for the HouseClass array
	if (param >= HouseClass::PlayerAtA && param <= HouseClass::PlayerAtH)
	{
		switch (param)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		default:
			break;
		}

		if (houseIdx >= 0)
		{
			HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx);

			if (!pHouse->Defeated
				&& !pHouse->IsObserver()
				&& !pHouse->Type->MultiplayPassive)
			{
				return houseIdx;
			}
		}

		return -1;
	}

	// Special case that returns the house index of the TeamClass object or the Trigger Action
	if (param == 8997)
	{
		return (pTeam ? pTeam->Owner->ArrayIndex : pTAction->TeamType->Owner->ArrayIndex);
	}

	// Positive index values check. Includes any kind of House
	if (param >= 0)
	{
		if (param < HouseClass::Array->Count)
		{
			HouseClass* pHouse = HouseClass::Array->GetItem(param);

			if (!pHouse->Defeated
				&& !pHouse->IsObserver())
			{
				return houseIdx;
			}
		}

		return -1;
	}

	// Special cases
	switch (param)
	{
	case -1:
		// Random non-neutral
		for (auto pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated
				&& !pHouse->IsObserver()
				&& !pHouse->Type->MultiplayPassive)
			{
				housesListIdx.push_back(pHouse->ArrayIndex);
			}
		}

		if (!housesListIdx.empty())
			houseIdx = housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1)];
		else
			return -1;

		break;

	case -2:
		// Find first Neutral house
		for (auto pHouseNeutral : *HouseClass::Array)
		{
			if (pHouseNeutral->IsNeutral())
			{
				houseIdx = pHouseNeutral->ArrayIndex;
				break;
			}
		}

		break;

	case -3:
		// Random Human Player
		for (auto pHouse : *HouseClass::Array)
		{
			if (pHouse->IsControlledByHuman()
				&& !pHouse->Defeated
				&& !pHouse->IsObserver())
			{
				housesListIdx.push_back(pHouse->ArrayIndex);
			}
		}

		if (!housesListIdx.empty())
			houseIdx = housesListIdx[ScenarioClass::Instance->Random.RandomFromMax(housesListIdx.size() - 1)];
		else
			return -1;

		break;

	default:
		break;
	}

	return houseIdx;
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
			&& !pThis->IsControlledByHuman() && harvesters < maxHarvesters
			&& pThis->TechLevel >= pHarvesterUnit->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvesterUnit->ArrayIndex;
			return true;
		}
	}
	else
	{
		const auto maxHarvesters = RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

		if (pThis->CountResourceGatherers < maxHarvesters)
		{
			const auto pRefinery = HouseExt::FindBuildable(
				pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery));

			if (pRefinery)
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

// Based on Ares' rewrite of 0x4FEA60.
void HouseExt::ExtData::UpdateVehicleProduction()
{
	const auto pThis = this->Get();
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	const bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	const bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if (skipGround && skipNaval)
		return;

	if (!skipGround && this->UpdateHarvesterProduction())
		return;

	auto& creationFrames = HouseExt::AIProduction_CreationFrames;
	auto& values = HouseExt::AIProduction_Values;
	auto& bestChoices = HouseExt::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExt::AIProduction_BestChoicesNaval;

	const auto count = static_cast<size_t>(UnitTypeClass::Array->Count);
	creationFrames.assign(count, 0x7FFFFFFF);
	values.assign(count, 0);

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pThis)
			continue;

		int teamCreationFrame = currentTeam->CreationFrame;

		if ((!currentTeam->Type->Reinforce || currentTeam->IsFullStrength)
			&& (currentTeam->IsForcedActive || currentTeam->IsHasBeen))
		{
			continue;
		}

		DynamicVectorClass<TechnoTypeClass*> taskForceMembers;
		currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

		for (auto const& currentMember : taskForceMembers)
		{
			if (!currentMember)
				continue;

			if (Is_UnitType(currentMember) ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			const auto index = static_cast<size_t>(currentMember->GetArrayIndex());
			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	for (auto unit : *UnitClass::Array)
	{
		const auto index = static_cast<unsigned int>(unit->GetType()->GetArrayIndex());

		if (values[index] > 0 && unit->CanBeRecruited(pThis))
			--values[index];
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
		const auto type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		const int currentValue = values[i];

		if (currentValue <= 0 || pThis->CanBuild(type, false, false) == CanBuildResult::Unbuildable
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

	const int earliestOdds = RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty];

	if (!skipGround)
	{
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < earliestOdds)
		{
			pThis->ProducingUnitTypeIndex = earliestTypenameIndex;
		}
		else if (const auto size = static_cast<int>(bestChoices.size()))
		{
			pThis->ProducingUnitTypeIndex = bestChoices[static_cast<size_t>(ScenarioClass::Instance->Random.RandomFromMax(size - 1))];
		}
	}

	if (!skipNaval)
	{
		if (ScenarioClass::Instance->Random.RandomFromMax(99) < earliestOdds)
		{
			this->ProducingNavalUnitTypeIndex = earliestTypenameIndexNaval;
		}
		else if (const auto size = static_cast<int>(bestChoicesNaval.size()))
		{
			this->ProducingNavalUnitTypeIndex = bestChoicesNaval[static_cast<size_t>(ScenarioClass::Instance->Random.RandomFromMax(size - 1))];
		}
	}
}

size_t HouseExt::FindOwnedIndex(
	HouseClass const* const, int const idxParentCountry,
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
	HouseClass const* const pHouse, BuildingTypeClass const* const pItem)
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
	HouseClass const* const pHouse, int const idxParentCountry,
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

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);

		if(!pExt->Death_Countdown.Completed())
			continue;

		Debug::Log("HouseExt::ExtData::UpdateAutoDeathObject -  Killing Techno[%x - %s] ! \n", pThis, pThis->get_ID());
		if (auto const pBuilding = specific_cast<BuildingClass*>(pThis)) {
			auto const pBldExt = BuildingExt::ExtMap.Find(pBuilding);

			if (pBldExt && pBldExt->LimboID != -1) {
				BuildingExt::LimboKill(pBuilding);
				continue;
			}
		}

		TechnoExt::KillSelf(pThis, nMethod, true , pTypeExt->AutoDeath_VanishAnimation);
	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->PowerPlantEnhancerBuildings)
		.Process(this->Building_BuildSpeedBonusCounter)
		.Process(this->HouseAirFactory)
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
		//.Process(this->LaunchDatas)
		.Process(this->CaptureObjectExecuted)
		.Process(this->DiscoverEvaDelay)

		.Process(this->Tunnels)
		.Process(this->Seed)

		.Process(this->SWLastIndex)
		.Process(this->Batteries)
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

DEFINE_HOOK(0x4FB9B7, HouseClass_Detach, 0xA)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(void*, target, STACK_OFFSET(0xC, 0x4));
	GET_STACK(bool, all, STACK_OFFSET(0xC, 0x8));

	HouseExt::ExtMap.InvalidatePointerFor(pThis, target, all);

	R->ESI(pThis);
	R->EBX(0);
	return pThis->ToCapture == target ?
		0x4FB9C3 : 0x4FB9C9;
}