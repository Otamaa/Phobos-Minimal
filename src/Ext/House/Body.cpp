#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <ScenarioClass.h>

//Static init
HouseExt::ExtContainer HouseExt::ExtMap;
int HouseExt::LastHarvesterBalance = 0;
int HouseExt::LastSlaveBalance = 0;
int HouseExt::LastGrindingBlanceUnit = 0;
int HouseExt::LastGrindingBlanceInf = 0;

void HouseExt::ExtData::InitializeConstants()
{
	//for (auto pSWType : *SuperWeaponTypeClass::Array())
	//{
	//	auto const pSW = GameCreate<SuperClass>(pSWType, this->Get());
	//	SecondarySWType.push_back(pSW);
	//}
}

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	const char* pSection = this->Get()->PlainName;

	INI_EX exINI(pINI);
	exINI.Read3Bool(pSection, "RepairBaseNodes", this->RepairBaseNodes);
}

void HouseExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(HouseAirFactory, reinterpret_cast<BuildingClass*>(ptr));
	AnnounceInvalidPointer(Factory_BuildingType, ptr);
	AnnounceInvalidPointer(Factory_InfantryType, ptr);
	AnnounceInvalidPointer(Factory_VehicleType, ptr);
	AnnounceInvalidPointer(Factory_NavyType, ptr);
	AnnounceInvalidPointer(Factory_AircraftType, ptr);
	AnnounceInvalidPointer(ActiveTeams, ptr);
	AnnounceInvalidPointer(AutoDeathObjects, ptr);
}

int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer()) return 0;

	int result =
		std::count_if(TechnoClass::Array->begin(), TechnoClass::Array->end(), [pThis](TechnoClass* techno)
		{
				if (TechnoTypeExt::ExtMap.Find(techno->GetTechnoType())->IsCountedAsHarvester() && techno->Owner == pThis)
				return TechnoExt::IsHarvesting(techno);

	return false;
		});

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis || !pThis->IsCurrentPlayer() || pThis->Defeated) return 0;

	int result = 0;

	std::for_each(TechnoTypeClass::Array->begin(), TechnoTypeClass::Array->end(), [&result, pThis](TechnoTypeClass* techno)
	{
		if (TechnoTypeExt::ExtMap.Find(techno)->IsCountedAsHarvester())
		{
			result += pThis->CountOwnedAndPresent(techno);
		}
	});

	return result;
}

int HouseExt::CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem)
{
	return HouseExt::ExtMap.Find(pThis)->OwnedLimboBuildingTypes.GetItemCount(pItem->ArrayIndex);
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
	return  HouseClass::FindByCountryIndex(RulesExt::Global()->NeutralCountryIndex);
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

	if (IS_SAME_STR_(pCur->get_ID(), "Observer"))
		return true;

	return false;
}

bool HouseExt::IsObserverPlayer(HouseClass* pCur)
{
	if (!pCur)
		return false;

	if (pCur == HouseClass::Observer())
		return true;

	if (IS_SAME_STR_(pCur->get_ID(), "Observer"))
		return true;

	return false;
}

bool HouseExt::PrerequisitesMet(HouseClass* const pThis, TechnoTypeClass* const pItem, const Iterator<BuildingTypeClass*>& ownedBuildingTypes)
{
	if (!pThis || !pItem)
		return false;

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	if (!pHouseExt)
		return false;

	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);
	if (!pItemExt)
		return false;

	// Prerequisite.RequiredTheaters check
	if (pItemExt->Prerequisite_RequiredTheaters.size() > 0)
	{
		int currentTheaterIndex = (int)ScenarioClass::Instance->Theater;
		if (pItemExt->Prerequisite_RequiredTheaters.IndexOf(currentTheaterIndex) < 0)
			return false;
	}

	// TechLevel check
	if (pThis->TechLevel < pItem->TechLevel)
		return false;

	// BuildLimit checks
	int nInstances = 0;

	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis
			&& pTechno->GetTechnoType() == pItem
			&& pTechno->IsAlive && pTechno->Health > 0)
		{
			nInstances++;
		}
	}

	if (nInstances >= pItem->BuildLimit)
		return false;

	bool prerequisiteNegativeMet = false; // Only one coincidence is needed

	// Ares Prerequisite.Negative list
	if (pItemExt->Prerequisite_Negative.size() > 0)
	{
		for (int const& idx : pItemExt->Prerequisite_Negative)
		{
			if (prerequisiteNegativeMet)
				return false;

			if (idx < 0) // Can be used generic prerequisites in this Ares tag? I have to investigate it but for now we support it...
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteNegativeMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				for (auto const& pObject : ownedBuildingTypes)
				{
					if (prerequisiteNegativeMet)
						break;

					if (idx == pObject->ArrayIndex)
						prerequisiteNegativeMet = true;
				}
			}
		}
	}

	auto const& prerequisiteOverride = pItem->PrerequisiteOverride;

	bool prerequisiteMet = false; // All buildings must appear in the buildings list owner by the house
	bool prerequisiteOverrideMet = false; // This tag uses an OR comparator: Only one coincidence is needed

	if (prerequisiteOverride.Count > 0)
	{
		for (int const& idx : prerequisiteOverride)
		{
			if (prerequisiteOverrideMet)
				break;

			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				prerequisiteOverrideMet = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				for (auto const& pObject : ownedBuildingTypes)
				{
					if (prerequisiteOverrideMet)
						break;

					if (idx == pObject->ArrayIndex)
						prerequisiteOverrideMet = true;
				}
			}
		}
	}

	if (pItemExt->Prerequisite.size() > 0)
	{
		bool found = false;

		for (int const& idx : pItemExt->Prerequisite)
		{
			found = false;

			if (idx < 0)
			{
				// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
				found = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
			}
			else
			{
				for (auto const& pObject : ownedBuildingTypes)
				{
					if (found)
						break;

					if (idx == pObject->ArrayIndex)
						found = true;
				}
			}

			if (!found)
				break;
		}

		prerequisiteMet = found;
	}
	else
	{
		// No prerequisites list means that always is buildable
		prerequisiteMet = true;
	}

	bool prerequisiteListsMet = false;

	// Ares Prerequisite lists
	if (pItemExt->Prerequisite_Lists.Get() > 0)
	{
		bool found = false;

		for (auto const& list : pItemExt->Prerequisite_ListVector)
		{
			if (found)
				break;

			for (int const& idx : list)
			{
				if (idx < 0)
				{
					// Default prerequisites like POWER, PROC, BARRACKS, FACTORY, ...
					found = HouseExt::HasGenericPrerequisite(idx, ownedBuildingTypes);
				}
				else
				{
					found = false;

					for (auto pObject : ownedBuildingTypes)
					{
						if (idx == pObject->ArrayIndex)
							found = true;

						if (found)
							break;
					}
				}

				if (!found)
					break;
			}
		}

		prerequisiteListsMet = found;
	}

	return prerequisiteMet || prerequisiteListsMet || prerequisiteOverrideMet;
}

bool HouseExt::HasGenericPrerequisite(int idx, const Iterator<BuildingTypeClass*>& ownedBuildingTypes)
{
	if (idx >= 0)
		return false;

	if (RulesExt::Global()->GenericPrerequisitesData.empty())
		return false;

	auto const& nSelected = RulesExt::Global()->GenericPrerequisitesData.at(abs(idx));

	if (nSelected.second.empty())
		return false;

	bool found = false;

	for (auto const& idxItem : nSelected.second)
	{
		if (found)
			break;

		for (auto const& pObject : ownedBuildingTypes)
		{
			if (found)
				break;

			if (idxItem == pObject->ArrayIndex)
				found = true;
		}
	}

	return found;
}

int HouseExt::FindGenericPrerequisite(const char* id)
{
	if (TechnoTypeClass::FindIndexById(id) >= 0)
		return 0;

	auto const& nPrereq = RulesExt::Global()->GenericPrerequisitesData;
	if (nPrereq.empty())
		RulesExt::FillDefaultPrerequisites(CCINIClass::INI_Rules()); // needed!

	int i = 0;
	for (auto const& str : nPrereq)
	{
		if (IS_SAME_STR_(id, str.first.c_str()))
			return (-1 * i);

		++i;
	}

	return 0;
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

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
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

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
		else
			return -1;

		break;

	default:
		break;
	}

	return houseIdx;
}


std::vector<int> HouseExt::AIProduction_CreationFrames;
std::vector<int> HouseExt::AIProduction_Values;
std::vector<int> HouseExt::AIProduction_BestChoices;
std::vector<int> HouseExt::AIProduction_BestChoicesNaval;

// Based on Ares' rewrite of 0x4FEA60.
void HouseExt::ExtData::UpdateVehicleProduction()
{
	auto pThis = this->Get();
	auto const AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if (skipGround && skipNaval)
		return;

	if (!skipGround)
	{
		auto const idxParentCountry = pThis->Type->FindParentCountryIndex();
		auto const pHarvester = HouseExt::FindOwned(pThis, idxParentCountry, make_iterator(RulesClass::Instance->HarvesterUnit));

		if (pHarvester)
		{
			//Buildable harvester found
			auto const harvesters = pThis->CountResourceGatherers;

			auto maxHarvesters = HouseExt::FindBuildable(
				pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery))
				? RulesClass::Instance->HarvestersPerRefinery[AIDifficulty] * pThis->CountResourceDestinations
				: RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

			if (pThis->IQLevel2 >= RulesClass::Instance->Harvester && !pThis->IsTiberiumShort
				&& !pThis->IsControlledByHuman() && harvesters < maxHarvesters
				&& pThis->TechLevel >= pHarvester->TechLevel)
			{
				pThis->ProducingUnitTypeIndex = pHarvester->ArrayIndex;
				return;
			}
		}
		else
		{
			//No buildable harvester found
			auto const maxHarvesters = RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

			if (pThis->CountResourceGatherers < maxHarvesters)
			{
				auto const pRefinery = HouseExt::FindBuildable(
					pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery));

				if (pRefinery)
				{
					//awesome way to find out whether this building is a slave miner, isn't it? ...
					if (auto const pSlaveMiner = pRefinery->UndeploysInto)
					{
						pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
						return;
					}
				}
			}
		}
	}

	auto& creationFrames = HouseExt::AIProduction_CreationFrames;
	auto& values = HouseExt::AIProduction_Values;
	auto& bestChoices = HouseExt::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExt::AIProduction_BestChoicesNaval;

	auto const count = static_cast<unsigned int>(UnitTypeClass::Array->Count);
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

		for (auto currentMember : taskForceMembers)
		{
			if (currentMember->WhatAmI() != UnitTypeClass::AbsID ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			auto const index = static_cast<unsigned int>(currentMember->GetArrayIndex());
			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	for (auto unit : *UnitClass::Array)
	{
		auto const index = static_cast<unsigned int>(unit->GetType()->GetArrayIndex());

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
		auto const type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		int currentValue = values[i];

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

	int earliestOdds = RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty];

	if (!skipGround)
	{
		if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < earliestOdds)
		{
			pThis->ProducingUnitTypeIndex = earliestTypenameIndex;
		}
		else if (auto const size = static_cast<int>(bestChoices.size()))
		{
			int randomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
			pThis->ProducingUnitTypeIndex = bestChoices[static_cast<unsigned int>(randomChoice)];
		}
	}

	if (!skipNaval)
	{
		if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < earliestOdds)
		{
			this->ProducingNavalUnitTypeIndex = earliestTypenameIndexNaval;
		}
		else if (auto const size = static_cast<int>(bestChoicesNaval.size()))
		{
			int randomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
			this->ProducingNavalUnitTypeIndex = bestChoicesNaval[static_cast<unsigned int>(randomChoice)];
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
			auto const pBld = abstract_cast<const BuildingTypeClass*>(pItem);
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
	for (const auto& pThis : this->AutoDeathObjects)
	{
		if (pThis->IsInLogic || !pThis->IsAlive)
			continue;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		const bool peacefulDeath = pTypeExt->Death_Peaceful.Get();
		const auto nKillMethod = peacefulDeath ? KillMethod::Vanish : pTypeExt->Death_Method.Get();
	
		if (pTypeExt->Death_Method != KillMethod::None && pExt->Death_Countdown.Completed())
			TechnoExt::KillSelf(pThis, nKillMethod);

	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->BuildingCounter)
		.Process(this->OwnedLimboBuildingTypes)
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
		;
}

void HouseExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(HouseExt::LastGrindingBlanceUnit)
		.Process(HouseExt::LastGrindingBlanceInf)
		.Process(HouseExt::LastHarvesterBalance)
		.Process(HouseExt::LastSlaveBalance)
		.Success();
}

bool HouseExt::SaveGlobals(PhobosStreamWriter& Stm)
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

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") { }
HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);
#ifndef ENABLE_NEWHOOKS
	HouseExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	HouseExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
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

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}

DEFINE_HOOK(0x4FB9B7, HouseClass_Detach, 0xA)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(void*, target, STACK_OFFSET(0xC, 0x4));
	GET_STACK(bool, all, STACK_OFFSET(0xC, 0x8));

	if (auto pExt = HouseExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return 0x0;
}