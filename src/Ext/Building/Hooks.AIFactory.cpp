#include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>
#include <algorithm>

std::tuple<BuildingClass**, bool, AbstractType> GetFactory(AbstractType AbsType, bool naval, HouseExtData* pData)
{
	if (!pData || !pData->AttachedToObject)
		return { nullptr, false, AbsType };

	BuildingClass** currFactory = nullptr;
	bool block = false;
	auto pRules = RulesExtData::Instance();
	auto pHouse = pData->AttachedToObject;

	if (!pRules)
		return { nullptr, false, AbsType };

	switch (AbsType)
	{
	case AbstractType::BuildingType:
	{
		currFactory = &pData->Factory_BuildingType;
		block = pRules->ForbidParallelAIQueues_Building.Get(!pRules->AllowParallelAIQueues);
		if (pHouse->ProducingBuildingTypeIndex >= 0 &&
			pHouse->ProducingBuildingTypeIndex < BuildingTypeClass::Array->Count)
		{
			auto pBuildingType = BuildingTypeClass::Array->Items[pHouse->ProducingBuildingTypeIndex];
			if (pBuildingType)
			{
				block = TechnoTypeExtContainer::Instance.Find(pBuildingType)->ForbidParallelAIQueues.Get(block);
			}
		}
		break;
	}
	case AbstractType::UnitType:
	{
		if (!naval)
		{
			block = pRules->ForbidParallelAIQueues_Vehicle.Get(!pRules->AllowParallelAIQueues);
			if (pHouse->ProducingUnitTypeIndex >= 0 &&
				pHouse->ProducingUnitTypeIndex < UnitTypeClass::Array->Count)
			{
				auto pUnitType = UnitTypeClass::Array->Items[pHouse->ProducingUnitTypeIndex];
				if (pUnitType)
				{
					block = TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues.Get(block);
				}
			}
			currFactory = &pData->Factory_VehicleType;
		}
		else
		{
			block = pRules->ForbidParallelAIQueues_Navy.Get(!pRules->AllowParallelAIQueues);
			if (pData->ProducingNavalUnitTypeIndex >= 0 &&
				pData->ProducingNavalUnitTypeIndex < UnitTypeClass::Array->Count)
			{
				auto pUnitType = UnitTypeClass::Array->Items[pData->ProducingNavalUnitTypeIndex];
				if (pUnitType)
				{
					block = TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues.Get(block);
				}
			}
			currFactory = &pData->Factory_NavyType;
		}

		break;
	}
	case AbstractType::InfantryType:
	{
		block = pRules->ForbidParallelAIQueues_Infantry.Get(!pRules->AllowParallelAIQueues);
		if (pHouse->ProducingInfantryTypeIndex >= 0 &&
			pHouse->ProducingInfantryTypeIndex < InfantryTypeClass::Array->Count)
		{
			auto pInfantryType = InfantryTypeClass::Array->Items[pHouse->ProducingInfantryTypeIndex];
			if (pInfantryType)
			{
				block = TechnoTypeExtContainer::Instance.Find(pInfantryType)->ForbidParallelAIQueues.Get(block);
			}
		}
		currFactory = &pData->Factory_InfantryType;
		break;
	}
	case AbstractType::AircraftType:
	{
		currFactory = &pData->Factory_AircraftType;
		block = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);
		if (pHouse->ProducingAircraftTypeIndex >= 0 &&
			pHouse->ProducingAircraftTypeIndex < AircraftTypeClass::Array->Count)
		{
			auto pAircraftType = AircraftTypeClass::Array->Items[pHouse->ProducingAircraftTypeIndex];
			if (pAircraftType)
			{
				block = TechnoTypeExtContainer::Instance.Find(pAircraftType)->ForbidParallelAIQueues.Get(block);
			}
		}

		break;
	}
	default:
		break;
	}

	return { currFactory, block, AbsType };
}

//#include <ostream>

//static std::map<void*, std::string> MappedCaller {};
//
//ASMJIT_PATCH(0x7353C0, UnitClass_CTOR_RecordCaller , 0x7) {
//
//	GET(UnitClass*, pThis, ECX);
//	GET_STACK(UnitTypeClass*, pType, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//	MappedCaller[pThis] = std::make_pair(pType , std::to_string(caller));
//	return 0x0;
//}

//ASMJIT_PATCH(0x7C8E17, Game_OperatorNew_Map, 0x6)
//{
//	GET_STACK(int, size, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	const auto pTr = __mh_malloc(size , 1);
//	MappedCaller[pTr] = std::to_string(caller);
//	R->EAX(pTr);
//	return 0x7C8E24;
//}
//
//ASMJIT_PATCH(0x7C9430, Game_MAlloc_Map, 0x6)
//{
//	GET_STACK(int, size, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	const auto pTr = __mh_malloc(size, CRT::AllocatorMode());
//	MappedCaller[pTr] = std::to_string(caller);
//	R->EAX(pTr);
//	return 0x7C9441;
//}
//
//ASMJIT_PATCH(0x7C8B3D, Game_OperatorDelete_UnMap, 0x9)
//{
//	GET_STACK(void*, ptr, 0x4);
//	MappedCaller.erase(ptr);
//	__free(ptr);
//	return 0x7C8B47;
//}

#include <Ext/Team/Body.h>

bool TeamExtData::IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement)
{
	if (!pGoing || !reinfocement)
		return false;

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pGoing->GetTechnoType());

	if (pTypeExt->RecuitedAs.isset() && pTypeExt->RecuitedAs == reinfocement)
		return true;

	if (TechnoExtContainer::Instance.Find(pGoing)->Type == reinfocement)
		return true;

	return false;
}

NOINLINE void GetRemainingTaskForceMembers(TeamClass* pTeam, std::vector<TechnoTypeClass*>& missings)
{
	if (!pTeam || !pTeam->Type || !pTeam->Type->TaskForce)
		return;

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

thread_local std::vector<TechnoTypeClass*> taskForceMembers {};

// NEW: Helper function to check if cloning building is eligible (power, status, etc.)
bool IsCloningBuildingEligible(BuildingClass* pBuilding, bool requirePower)
{
	if (!pBuilding)
		return false;
		
	// Building must be alive, on map, not being warped, etc.
	if (pBuilding->InLimbo || !pBuilding->IsAlive || !pBuilding->IsOnMap ||
		pBuilding->TemporalTargetingMe || pBuilding->IsBeingWarpedOut())
	{
		return false;
	}
	
	// Power check: only if building type is powered AND requirePower is true
	if (pBuilding->Type->Powered && requirePower && !pBuilding->IsPowerOnline())
		return false;
	
	return true;
}

// NEW: Helper function to calculate effective production multiplier including cloning
int GetCloningMultiplier(HouseClass* pHouse, TechnoTypeClass* pType)
{
	if (!pHouse || !pType)
		return 1;

	int multiplier = 1; // Base production (the original unit)
	
	// Check if InitialStrength.Cloning might affect recruitment
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	double healthFactor = 1.0; // Assume clones will be healthy enough
	if (pTypeExt->InitialStrength_Cloning.isset())
	{
		auto strengthRange = pTypeExt->InitialStrength_Cloning.Get();
		double minStrength = strengthRange.X < strengthRange.Y ? strengthRange.X : strengthRange.Y;
		// If clones might be too weak (below 5% health), reduce expected multiplier
		if (minStrength < 0.05)
			healthFactor = 0.5; // Assume only half the clones will be recruitable
	}
	
	if (pType->WhatAmI() == AbstractType::InfantryType)
	{
		// Infantry cloning: standard CloningVats
		for (auto pCloningVat : pHouse->CloningVats)
		{
			auto pBuildingExt = BuildingTypeExtContainer::Instance.Find(pCloningVat->Type);
			if (IsCloningBuildingEligible(pCloningVat, pBuildingExt->Cloning_RequirePower))
			{
				multiplier++;
			}
		}
		
		// Infantry cloning: custom buildings with Cloning=yes
		for (auto pBuilding : pHouse->Buildings)
		{
			if (pBuilding && pBuilding->Type && pBuilding->Type->Cloning)
			{
				auto pBuildingExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
				if (IsCloningBuildingEligible(pBuilding, pBuildingExt->Cloning_RequirePower))
				{
					multiplier++;
				}
			}
		}
	}
	else if (pType->WhatAmI() == AbstractType::UnitType)
	{
		// Vehicle cloning: CloningFacility=yes buildings
		UnitTypeClass* pUnitType = static_cast<UnitTypeClass*>(pType);
		
		for (auto pBuilding : pHouse->Buildings)
		{
			if (pBuilding && pBuilding->Type)
			{
				auto pBuildingExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
				if (pBuildingExt && pBuildingExt->CloningFacility)
				{
					// Check if this cloning facility matches the unit type (naval vs ground)
					if (pBuilding->Type->Naval == pUnitType->Naval)
					{
						if (IsCloningBuildingEligible(pBuilding, pBuildingExt->Cloning_RequirePower))
						{
							multiplier++;
						}
					}
				}
			}
		}
	}
	
	// Apply health factor if clones might be too weak to recruit
	if (healthFactor < 1.0 && multiplier > 1)
	{
		multiplier = 1 + static_cast<int>((multiplier - 1) * healthFactor);
	}
	
	return multiplier;
}

void HouseExtData::UpdateVehicleProduction()
{
	auto pThis = this->AttachedToObject;
	if (!pThis)
		return;

	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	auto& creationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& values = HouseExtData::AIProduction_Values;
	auto& bestChoices = HouseExtData::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExtData::AIProduction_BestChoicesNaval;

	const auto count = static_cast<size_t>(UnitTypeClass::Array->Count);

	// Optimize: resize only if needed, then fill with values
	if (creationFrames.size() != count)
	{
		creationFrames.resize(count);
		values.resize(count);
	}
	std::fill(creationFrames.begin(), creationFrames.end(), 0x7FFFFFFF);
	std::fill(values.begin(), values.end(), 0);

	for (auto& currentTeam : HouseExtContainer::HousesTeams[pThis])
	{
		if (!currentTeam || !currentTeam->Type)
			continue;

		taskForceMembers.clear();
		int teamCreationFrame = currentTeam->CreationFrame;

		if ((currentTeam->Type->Reinforce && !currentTeam->IsFullStrength) ||
			(!currentTeam->IsForcedActive && !currentTeam->IsHasBeen))
		{
			GetRemainingTaskForceMembers(currentTeam, taskForceMembers);
		}
		else
		{
			continue;
		}

		for (auto& currentMember : taskForceMembers)
		{
			if (!currentMember)
				continue;

			const auto what = currentMember->WhatAmI();

			if (what != UnitTypeClass::AbsID ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			const auto arrayIndex = ((UnitTypeClass*)currentMember)->ArrayIndex;
			// Add bounds checking for safety
			if (arrayIndex < 0 || static_cast<size_t>(arrayIndex) >= count)
				continue;

			const auto index = static_cast<size_t>(arrayIndex);
			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	// Add safety check for UnitClass::Array
	if (UnitClass::Array && UnitClass::Array->Count > 0)
	{
		// Use consistent indexing type
		for (size_t i = 0; i < static_cast<size_t>(UnitClass::Array->Count); ++i)
		{
			const auto pUnit = UnitClass::Array->Items[static_cast<int>(i)];
			if (!pUnit || !pUnit->Type)
				continue;

			const auto typeIndex = pUnit->Type->ArrayIndex;

			// Add bounds checking
			if (typeIndex >= 0 && static_cast<size_t>(typeIndex) < count &&
				values[typeIndex] > 0 && pUnit->CanBeRecruited(pThis))
			{
				--values[typeIndex];
			}
		}
	}

	// NEW: Apply cloning-aware production adjustment
	for (size_t i = 0; i < count; ++i)
	{
		if (values[i] > 0)
		{
			auto pType = UnitTypeClass::Array->Items[static_cast<int>(i)];
			if (pType)
			{
				int cloningMultiplier = GetCloningMultiplier(pThis, pType);
				if (cloningMultiplier > 1)
				{
					// Adjust needed production: divide by multiplier, round up
					values[i] = (values[i] + cloningMultiplier - 1) / cloningMultiplier;
				}
			}
		}
	}

	bestChoices.clear();
	bestChoicesNaval.clear();

	int bestValue = -1;
	int bestValueNaval = -1;
	int earliestTypenameIndex = -1;
	int earliestTypenameIndexNaval = -1;
	int earliestFrame = 0x7FFFFFFF;
	int earliestFrameNaval = 0x7FFFFFFF;

	for (size_t i = 0; i < count; ++i)
	{
		auto type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		if (!type)
			continue;

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

//ASMJIT_PATCH(0x7258D0, AnnounceInvalidPointer_PhobosGlobal_Mapped, 0x6)
//{
//	GET(AbstractClass* const, pInvalid, ECX);
//	GET(bool const, removed, EDX);
//
//	if (Phobos::Otamaa::ExeTerminated)
//		return 0;
//
//	if (removed && MappedCaller.contains((UnitClass*)pInvalid)) {
//		MappedCaller.erase((UnitClass*)pInvalid);
//	}
//
//	return 0;
//}
//

//// Clear static data from respective classes
//ASMJIT_PATCH(0x685659, Scenario_ClearClasses_PhobosGlobal_Mapped, 0xA)
//{
//	MappedCaller.clear();
//	return 0x0;
//}

ASMJIT_PATCH(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6) //was C
{
	GET(BuildingClass*, pBuilding, ESI);

	if (!pBuilding || !pBuilding->Owner || pBuilding->Owner->IsControlledByHuman() || pBuilding->Owner->IsNeutral())
		return 0x0;

	auto pRules = RulesExtData::Instance();
	if (!pRules)
		return 0x0;

	bool ForbidParallelAIQueues_ = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);

	const int producingIndex = pBuilding->Owner->ProducingAircraftTypeIndex;
	if (producingIndex >= 0 && producingIndex < AircraftTypeClass::Array->Count)
	{
		auto pType = AircraftTypeClass::Array()->Items[producingIndex];
		if (pType)
		{
			ForbidParallelAIQueues_ = TechnoTypeExtContainer::Instance.Find(pType)->ForbidParallelAIQueues.Get(ForbidParallelAIQueues_);
		}
	}

	if (!ForbidParallelAIQueues_)
	{
		return 0;
	}

	if (pBuilding->Type && pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExtData::HasFreeDocks(pBuilding))
		{
			BuildingExtData::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

//ASMJIT_PATCH(0x04500FA, BuildingClass_AI_Factory_SkipNoneForComputer, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	auto pRules = RulesExtData::Instance();
//
//	if (pThis->Type->Factory == AbstractType::AircraftType && pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues))
//		if(!pThis->Factory && !pThis->IsPrimaryFactory && pThis->Owner && pThis->Owner->IsControlledByHuman())
//		return 0x4503CA;
//
//	return 0x0;
//}

ASMJIT_PATCH(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EDX);
	GET(BuildingClass*, pThis, ESI);

	if (!pHouse || !pThis)
		return 0;

	auto pExt = pHouse->_GetExtData();
	if (!pExt)
		return 0;

	if (pThis == pExt->Factory_AircraftType)
		pExt->Factory_AircraftType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	if (!pHouse || !pThis)
		return 0;

	auto pExt = pHouse->_GetExtData();
	if (!pExt)
		return 0;

	if (pThis == pExt->Factory_BuildingType)
		pExt->Factory_BuildingType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	if (!pHouse || !pThis)
		return 0;

	auto pExt = pHouse->_GetExtData();
	if (!pExt)
		return 0;

	if (pThis == pExt->Factory_InfantryType)
		pExt->Factory_InfantryType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	if (!pUnit || !pFactory || !pFactory->Owner || !pUnit->Type)
		return 0;

	auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);
	if (!pHouseExt)
		return 0;

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (!pFactory || !pFactory->Owner || !pFactory->Object)
		return 0;

	HouseExtData* pData = HouseExtContainer::Instance.Find(pFactory->Owner);
	if (!pData)
		return 0;

	switch (pFactory->Object->WhatAmI())
	{
	case BuildingClass::AbsID:
		pData->Factory_BuildingType = nullptr;
		break;
	case UnitClass::AbsID:
		if (!pFactory->Object->GetTechnoType()->Naval)
			pData->Factory_VehicleType = nullptr;
		else
			pData->Factory_NavyType = nullptr;
		break;
	case InfantryClass::AbsID:
		pData->Factory_InfantryType = nullptr;
		break;
	case AircraftClass::AbsID:
		pData->Factory_AircraftType = nullptr;
		break;
	default:
		break;
	}

	return 0;
}

ASMJIT_PATCH(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	enum { Skip = 0x4503CA };

	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;
	if (!pOwner || !pOwner->Production || !pThis->Type)
		return 0x0;

	HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);
	if (!pData)
		return 0x0;

	const auto& [curFactory, block, type] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pData);

	if (!curFactory)
	{
		_com_issue_error(E_POINTER);
		return 0x0;
	}
	else if (!*curFactory)
	{
		if (type != AircraftTypeClass::AbsID)
		{
			if (!pThis->IsPrimaryFactory)
				pThis->IsPrimaryFactory = true;
		}

		*curFactory = pThis; //last check
		return 0;
	}
	else if (*curFactory != pThis)
	{
		switch (type)
		{
		case AircraftTypeClass::AbsID:
		{
			if (pOwner->ProducingAircraftTypeIndex >= 0 &&
				pOwner->ProducingAircraftTypeIndex < AircraftTypeClass::Array->Count)
			{
				auto pAircraftType = AircraftTypeClass::Array->Items[pOwner->ProducingAircraftTypeIndex];
				if (pAircraftType && TechnoTypeExtContainer::Instance.Find(pAircraftType)->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case InfantryTypeClass::AbsID:
		{
			if (pOwner->ProducingInfantryTypeIndex >= 0 &&
				pOwner->ProducingInfantryTypeIndex < InfantryTypeClass::Array->Count)
			{
				auto pInfantryType = InfantryTypeClass::Array->Items[pOwner->ProducingInfantryTypeIndex];
				if (pInfantryType && TechnoTypeExtContainer::Instance.Find(pInfantryType)->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case BuildingTypeClass::AbsID:
		{
			if (pOwner->ProducingBuildingTypeIndex >= 0 &&
				pOwner->ProducingBuildingTypeIndex < BuildingTypeClass::Array->Count)
			{
				auto pBuildingType = BuildingTypeClass::Array->Items[pOwner->ProducingBuildingTypeIndex];
				if (pBuildingType && TechnoTypeExtContainer::Instance.Find(pBuildingType)->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case UnitTypeClass::AbsID:
		{
			const int idx = pThis->Type->Naval ? pData->ProducingNavalUnitTypeIndex : pOwner->ProducingUnitTypeIndex;
			if (idx >= 0 && idx < UnitTypeClass::Array->Count)
			{
				auto pUnitType = UnitTypeClass::Array->Items[idx];
				if (pUnitType && TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}

			break;
		}
		default:
			break;
		}

		return block ? Skip : 0x0;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4FEA60, HouseClass_AI_UnitProduction, 0x6)
{
	GET(HouseClass* const, pThis, ECX);

	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 15);

	if (!pThis)
		return ret();

	HouseExtContainer::Instance.Find(pThis)->UpdateVehicleProduction();
	return ret();
}

template <class T, class Ttype >
int NOINLINE GetTypeToProduceNew(HouseClass* pHouse)
{
	if (!pHouse)
		return -1;

	auto& CreationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& Values = HouseExtData::AIProduction_Values;
	auto& BestChoices = HouseExtData::AIProduction_BestChoices;

	const auto count = static_cast<size_t>(Ttype::Array->Count);

	// Optimize: resize only if needed, then fill with values
	if (CreationFrames.size() != count)
	{
		CreationFrames.resize(count);
		Values.resize(count);
	}
	std::fill(CreationFrames.begin(), CreationFrames.end(), 0x7FFFFFFF);
	std::fill(Values.begin(), Values.end(), 0);
	BestChoices.clear();

	//Debug::LogInfo(__FUNCTION__" Executing with Current TeamArrayCount[%d] for[%s][House %s - %x] ", TeamClass::Array->Count, AbstractClass::GetAbstractClassName(Ttype::AbsID), pHouse->get_ID() , pHouse);
	for (auto& CurrentTeam : HouseExtContainer::HousesTeams[pHouse])
	{
		if (!CurrentTeam || !CurrentTeam->Type)
			continue;

		taskForceMembers.clear();
		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if (CurrentTeam->Type->Reinforce && !CurrentTeam->IsFullStrength || !CurrentTeam->IsForcedActive && !CurrentTeam->IsHasBeen)
		{
			GetRemainingTaskForceMembers(CurrentTeam, taskForceMembers);

			for (auto& pMember : taskForceMembers)
			{
				if (!pMember || pMember->WhatAmI() != Ttype::AbsID)
				{
					continue;
				}

				const auto arrayIndex = ((Ttype*)pMember)->ArrayIndex;
				// Add bounds checking for safety
				if (arrayIndex < 0 || static_cast<size_t>(arrayIndex) >= count)
					continue;

				const auto Idx = static_cast<size_t>(arrayIndex);
				++Values[Idx];
				if (TeamCreationFrame < CreationFrames[Idx])
				{
					CreationFrames[Idx] = TeamCreationFrame;
				}
			}
		}
	}

	// Add safety check for T::Array
	if (T::Array && T::Array->Count > 0)
	{
		for (auto classPos = T::Array->begin(); classPos != T::Array->end(); ++classPos)
		{
			const auto pUnit = *classPos;
			if (!pUnit || !pUnit->Type)
				continue;

			const auto typeIndex = pUnit->Type->ArrayIndex;
			// Add bounds checking
			if (typeIndex >= 0 && static_cast<size_t>(typeIndex) < count &&
				Values[typeIndex] > 0 && pUnit->CanBeRecruited(pHouse))
			{
				--Values[typeIndex];
			}
		}
	}

	// NEW: Apply cloning-aware production adjustment
	if constexpr (std::is_same_v<Ttype, InfantryTypeClass> || std::is_same_v<Ttype, UnitTypeClass>)
	{
		for (size_t i = 0; i < count; ++i)
		{
			if (Values[i] > 0)
			{
				auto pType = Ttype::Array->Items[static_cast<int>(i)];
				if (pType)
				{
					int cloningMultiplier = GetCloningMultiplier(pHouse, pType);
					if (cloningMultiplier > 1)
					{
						// Adjust needed production: divide by multiplier, round up
						Values[i] = (Values[i] + cloningMultiplier - 1) / cloningMultiplier;
					}
				}
			}
		}
	}

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for (size_t i = 0; i < count; ++i)
	{
		auto const TT = Ttype::Array->Items[static_cast<int>(i)];
		if (!TT)
			continue;

		int CurrentValue = Values[i];

		if (CurrentValue <= 0)
			continue;

		const auto buildableResult = pHouse->CanBuild(TT, false, false);

		// Aircraft has it own handling
		if constexpr (std::is_same_v<Ttype, AircraftTypeClass>)
		{
			//Debug::LogInfo("Aircraft [%s][%s] return result [%d] for can build");

			if (buildableResult != CanBuildResult::Buildable || TT->GetActualCost(pHouse) > pHouse->Available_Money())
			{
				continue;
			}

			//yes , we checked this fucking twice just to make sure
			const auto factoryresult = HouseExtData::HasFactory(pHouse, TT, false, true, false, true).first;

			if (factoryresult == NewFactoryState::NotFound || factoryresult == NewFactoryState::NoFactory)
			{
				continue;
			}
		}
		else
		{
			if (buildableResult == CanBuildResult::Unbuildable
				|| TT->GetActualCost(pHouse) > pHouse->Available_Money())
			{
				continue;
			}
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

	const auto AIDiff = static_cast<int>(pHouse->GetAIDifficultyIndex());

	if (ScenarioClass::Instance->Random.RandomFromMax(99) < RulesClass::Instance->FillEarliestTeamProbability[AIDiff])
		return EarliestTypenameIndex;

	if (!BestChoices.empty())
		return BestChoices[ScenarioClass::Instance->Random.RandomFromMax(int(BestChoices.size() - 1))];

	return -1;
}

ASMJIT_PATCH(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis && pThis->ProducingInfantryTypeIndex < 0)
	{
		const int result = GetTypeToProduceNew<InfantryClass, InfantryTypeClass>(pThis);
		if (result >= 0)
			pThis->ProducingInfantryTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF204;
}

ASMJIT_PATCH(0x4FF210, HouseClass_AI_AircraftProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis && pThis->ProducingAircraftTypeIndex < 0)
	{
		const int result = GetTypeToProduceNew<AircraftClass, AircraftTypeClass>(pThis);
		if (result >= 0)
			pThis->ProducingAircraftTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF534;
}