#include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>

// Constants for better code readability
constexpr int INVALID_PRODUCTION_INDEX = -1;
constexpr int MAX_RANDOM_VALUE = 99;
constexpr int MAX_FRAME_VALUE = 0x7FFFFFFF;

// Forward declarations
bool ShouldSkipParallelProduction(HouseClass* pOwner, HouseExtData* pHouseData, AbstractType factoryType, bool isNaval);

template <class TType>
void ProcessTeamsForTypeProduction(HouseClass* pHouse, std::vector<int>& creationFrames, std::vector<int>& values);

template <class T, class TType>
void ProcessExistingUnitsForTypeProduction(HouseClass* pHouse, std::vector<int>& values);

template <class TType>
int DetermineBestTypeToProduceNew(HouseClass* pHouse, const std::vector<int>& creationFrames,
	const std::vector<int>& values, std::vector<int>& bestChoices, size_t typeCount);

template <class TType>
inline bool CanProduceType(HouseClass* pHouse, TType* pType);

inline int SelectFinalTypeToProduceNew(HouseClass* pHouse, const std::vector<int>& bestChoices, int earliestTypeIndex);

std::tuple<BuildingClass**, bool, AbstractType> GetFactory(AbstractType absType, bool isNaval, HouseExtData* pHouseData)
{
	if (!pHouseData || !pHouseData->AttachedToObject)
	{
		return { nullptr, false, absType };
	}

	BuildingClass** currentFactory = nullptr;
	bool shouldBlock = false;
	auto pRules = RulesExtData::Instance();
	auto pHouse = pHouseData->AttachedToObject;

	// Additional safety check
	if (!pRules)
	{
		return { nullptr, false, absType };
	}

	switch (absType)
	{
	case AbstractType::BuildingType:
	{
		currentFactory = &pHouseData->Factory_BuildingType;
		shouldBlock = pRules->ForbidParallelAIQueues_Building.Get(!pRules->AllowParallelAIQueues);

		if (pHouse->ProducingBuildingTypeIndex >= 0 &&
			pHouse->ProducingBuildingTypeIndex < BuildingTypeClass::Array->Count)
		{
			auto pBuildingType = BuildingTypeClass::Array->Items[pHouse->ProducingBuildingTypeIndex];
			if (pBuildingType)
			{
				shouldBlock = TechnoTypeExtContainer::Instance.Find(pBuildingType)->ForbidParallelAIQueues.Get(shouldBlock);
			}
		}
		break;
	}
	case AbstractType::UnitType:
	{
		if (!isNaval)
		{
			currentFactory = &pHouseData->Factory_VehicleType;
			shouldBlock = pRules->ForbidParallelAIQueues_Vehicle.Get(!pRules->AllowParallelAIQueues);

			if (pHouse->ProducingUnitTypeIndex >= 0 &&
				pHouse->ProducingUnitTypeIndex < UnitTypeClass::Array->Count)
			{
				auto pUnitType = UnitTypeClass::Array->Items[pHouse->ProducingUnitTypeIndex];
				if (pUnitType)
				{
					shouldBlock = TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues.Get(shouldBlock);
				}
			}
		}
		else
		{
			currentFactory = &pHouseData->Factory_NavyType;
			shouldBlock = pRules->ForbidParallelAIQueues_Navy.Get(!pRules->AllowParallelAIQueues);

			if (pHouseData->ProducingNavalUnitTypeIndex >= 0 &&
				pHouseData->ProducingNavalUnitTypeIndex < UnitTypeClass::Array->Count)
			{
				auto pUnitType = UnitTypeClass::Array->Items[pHouseData->ProducingNavalUnitTypeIndex];
				if (pUnitType)
				{
					shouldBlock = TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues.Get(shouldBlock);
				}
			}
		}
		break;
	}
	case AbstractType::InfantryType:
	{
		currentFactory = &pHouseData->Factory_InfantryType;
		shouldBlock = pRules->ForbidParallelAIQueues_Infantry.Get(!pRules->AllowParallelAIQueues);

		if (pHouse->ProducingInfantryTypeIndex >= 0 &&
			pHouse->ProducingInfantryTypeIndex < InfantryTypeClass::Array->Count)
		{
			auto pInfantryType = InfantryTypeClass::Array->Items[pHouse->ProducingInfantryTypeIndex];
			if (pInfantryType)
			{
				shouldBlock = TechnoTypeExtContainer::Instance.Find(pInfantryType)->ForbidParallelAIQueues.Get(shouldBlock);
			}
		}
		break;
	}
	case AbstractType::AircraftType:
	{
		currentFactory = &pHouseData->Factory_AircraftType;
		shouldBlock = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);

		if (pHouse->ProducingAircraftTypeIndex >= 0 &&
			pHouse->ProducingAircraftTypeIndex < AircraftTypeClass::Array->Count)
		{
			auto pAircraftType = AircraftTypeClass::Array->Items[pHouse->ProducingAircraftTypeIndex];
			if (pAircraftType)
			{
				shouldBlock = TechnoTypeExtContainer::Instance.Find(pAircraftType)->ForbidParallelAIQueues.Get(shouldBlock);
			}
		}
		break;
	}
	default:
		break;
	}

	return { currentFactory, shouldBlock, absType };
}

void HouseExtData::UpdateVehicleProduction()
{
	auto pHouse = this->AttachedToObject;
	const int aiDifficulty = static_cast<int>(pHouse->GetAIDifficultyIndex());
	const bool skipGround = pHouse->ProducingUnitTypeIndex != INVALID_PRODUCTION_INDEX;
	const bool skipNaval = this->ProducingNavalUnitTypeIndex != INVALID_PRODUCTION_INDEX;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	auto& creationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& values = HouseExtData::AIProduction_Values;
	auto& bestChoices = HouseExtData::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExtData::AIProduction_BestChoicesNaval;

	const size_t unitTypeCount = static_cast<size_t>(UnitTypeClass::Array->Count);

	// Optimize vector operations - resize once, then fill
	if (creationFrames.size() != unitTypeCount)
	{
		creationFrames.resize(unitTypeCount);
		values.resize(unitTypeCount);
	}
	std::fill(creationFrames.begin(), creationFrames.end(), MAX_FRAME_VALUE);
	std::fill(values.begin(), values.end(), 0);

	// Process teams to determine production priorities
	ProcessTeamsForProduction(pHouse, creationFrames, values, skipGround, skipNaval);

	// Account for existing units that can be recruited
	ProcessExistingUnitsForProduction(pHouse, values);

	// Determine best production choices
	int earliestTypeIndex = -1;
	int earliestTypeIndexNaval = -1;
	DetermineBestProductionChoices(pHouse, creationFrames, values, bestChoices, bestChoicesNaval,
		unitTypeCount, earliestTypeIndex, earliestTypeIndexNaval);

	// Set production indices based on AI difficulty and randomization
	SetFinalProductionIndices(pHouse, aiDifficulty, bestChoices, bestChoicesNaval,
		earliestTypeIndex, earliestTypeIndexNaval, skipGround, skipNaval);
}

void HouseExtData::ProcessTeamsForProduction(HouseClass* pHouse, std::vector<int>& creationFrames,
	std::vector<int>& values, bool skipGround, bool skipNaval)
{
	// Move allocation outside the loop for better performance
	DynamicVectorClass<TechnoTypeClass*> taskForceMembers;
	taskForceMembers.Reserve(32); // Pre-allocate reasonable capacity

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pHouse)
			continue;

		const int teamCreationFrame = currentTeam->CreationFrame;

		// Original logic: Skip teams that (don't need reinforce OR are full) AND (are active OR have been active)
		// This logic processes teams that either need reinforcement or are not at full strength,
		// but only if they are not currently active or have never been active
		if ((!currentTeam->Type->Reinforce || currentTeam->IsFullStrength) &&
			(currentTeam->IsForcedActive || currentTeam->IsHasBeen))
		{
			continue;
		}

		taskForceMembers.Clear(); // Reuse existing allocation
		currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

		for (auto currentMember : taskForceMembers)
		{
			if (currentMember->WhatAmI() != UnitTypeClass::AbsID ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			const size_t index = static_cast<size_t>(static_cast<UnitTypeClass*>(currentMember)->ArrayIndex);

			// Add bounds check to prevent crash
			if (index >= values.size())
				continue;

			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}
}

void HouseExtData::ProcessExistingUnitsForProduction(HouseClass* pHouse, std::vector<int>& values)
{
	const size_t maxTypeIndex = values.size();

	for (int i = 0; i < UnitClass::Array->Count; ++i)
	{
		const auto pUnit = UnitClass::Array->Items[i];
		const size_t typeIndex = static_cast<size_t>(pUnit->Type->ArrayIndex);

		// Bounds check to prevent crash
		if (typeIndex >= maxTypeIndex)
			continue;

		if (values[typeIndex] > 0 && pUnit->CanBeRecruited(pHouse))
		{
			--values[typeIndex];
			// Early termination: if this type no longer needed, we can skip checking more units of this type
			// Note: We can't break entirely because there might be other types that still need units
		}
	}
}

void HouseExtData::DetermineBestProductionChoices(HouseClass* pHouse, const std::vector<int>& creationFrames,
	const std::vector<int>& values, std::vector<int>& bestChoices, std::vector<int>& bestChoicesNaval,
	size_t unitTypeCount, int& earliestTypeIndex, int& earliestTypeIndexNaval)
{
	bestChoices.clear();
	bestChoicesNaval.clear();

	// Reserve memory to prevent reallocations
	bestChoices.reserve(32);
	bestChoicesNaval.reserve(32);

	int bestValue = -1;
	int bestValueNaval = -1;
	earliestTypeIndex = -1;
	earliestTypeIndexNaval = -1;
	int earliestFrame = MAX_FRAME_VALUE;
	int earliestFrameNaval = MAX_FRAME_VALUE;

	for (size_t i = 0; i < unitTypeCount; ++i)
	{
		auto pUnitType = UnitTypeClass::Array->Items[static_cast<int>(i)];
		const int currentValue = values[i];

		if (currentValue <= 0)
			continue;

		const auto buildResult = pHouse->CanBuild(pUnitType, false, false);
		if (buildResult == CanBuildResult::Unbuildable ||
			pUnitType->GetActualCost(pHouse) > pHouse->Available_Money())
		{
			continue;
		}

		const bool isNaval = pUnitType->Naval;
		UpdateBestChoicesForType(i, currentValue, creationFrames[i], isNaval,
			bestValue, bestValueNaval, bestChoices, bestChoicesNaval,
			earliestTypeIndex, earliestTypeIndexNaval, earliestFrame, earliestFrameNaval);
	}
}

void HouseExtData::UpdateBestChoicesForType(size_t typeIndex, int currentValue, int creationFrame, bool isNaval,
	int& bestValue, int& bestValueNaval, std::vector<int>& bestChoices, std::vector<int>& bestChoicesNaval,
	int& earliestTypeIndex, int& earliestTypeIndexNaval, int& earliestFrame, int& earliestFrameNaval)
{
	int& targetBestValue = isNaval ? bestValueNaval : bestValue;
	std::vector<int>& targetBestChoices = isNaval ? bestChoicesNaval : bestChoices;
	int& targetEarliestIndex = isNaval ? earliestTypeIndexNaval : earliestTypeIndex;
	int& targetEarliestFrame = isNaval ? earliestFrameNaval : earliestFrame;

	if (targetBestValue < currentValue || targetBestValue == -1)
	{
		targetBestValue = currentValue;
		targetBestChoices.clear();
	}

	targetBestChoices.push_back(static_cast<int>(typeIndex));

	if (targetEarliestFrame > creationFrame || targetEarliestIndex == -1)
	{
		targetEarliestIndex = static_cast<int>(typeIndex);
		targetEarliestFrame = creationFrame;
	}
}

void HouseExtData::SetFinalProductionIndices(HouseClass* pHouse, int aiDifficulty,
	const std::vector<int>& bestChoices, const std::vector<int>& bestChoicesNaval,
	int earliestTypeIndex, int earliestTypeIndexNaval, bool skipGround, bool skipNaval)
{
	auto& random = ScenarioClass::Instance->Random;
	const int fillEarliestProbability = RulesClass::Instance->FillEarliestTeamProbability[aiDifficulty];

	if (!skipGround)
	{
		int result = earliestTypeIndex;
		if (random.RandomFromMax(MAX_RANDOM_VALUE) >= fillEarliestProbability)
		{
			if (!bestChoices.empty())
				result = bestChoices[random.RandomFromMax(static_cast<int>(bestChoices.size()) - 1)];
			else
				result = INVALID_PRODUCTION_INDEX;
		}
		pHouse->ProducingUnitTypeIndex = result;
	}

	if (!skipNaval)
	{
		int result = earliestTypeIndexNaval;
		if (random.RandomFromMax(MAX_RANDOM_VALUE) >= fillEarliestProbability)
		{
			if (!bestChoicesNaval.empty())
				result = bestChoicesNaval[random.RandomFromMax(static_cast<int>(bestChoicesNaval.size()) - 1)];
			else
				result = INVALID_PRODUCTION_INDEX;
		}
		this->ProducingNavalUnitTypeIndex = result;
	}
}

ASMJIT_PATCH(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6) //was C
{
	GET(BuildingClass*, pBuilding, ESI);

	if (!pBuilding->Owner || pBuilding->Owner->IsControlledByHuman() || pBuilding->Owner->IsNeutral())
		return 0x0;

	auto pRules = RulesExtData::Instance();

	bool ForbidParallelAIQueues_ = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);

	if (auto const pType = pBuilding->Owner->ProducingAircraftTypeIndex >= 0 ?
		AircraftTypeClass::Array()->Items[pBuilding->Owner->ProducingAircraftTypeIndex] : nullptr)
	{
		ForbidParallelAIQueues_ = TechnoTypeExtContainer::Instance.Find(pType)->ForbidParallelAIQueues.Get(ForbidParallelAIQueues_);
	}

	if (!ForbidParallelAIQueues_)
	{
		return 0;
	}

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExtData::HasFreeDocks(pBuilding))
		{
			BuildingExtData::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EDX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_AircraftType)
		pExt->Factory_AircraftType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_BuildingType)
		pExt->Factory_BuildingType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis, ESI);

	auto pExt = pHouse->_GetExtData();

	if (pThis == pExt->Factory_InfantryType)
		pExt->Factory_InfantryType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if (!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

ASMJIT_PATCH(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (HouseClass* pOwner = pFactory->Owner)
	{
		HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);

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
	}

	return 0;
}

ASMJIT_PATCH(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	enum { Skip = 0x4503CA };

	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;
	if (!pOwner || !pOwner->Production)
		return 0x0;

	HouseExtData* pHouseData = HouseExtContainer::Instance.Find(pOwner);
	const auto& [currentFactory, shouldBlock, factoryType] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pHouseData);

	if (!currentFactory)
	{
		_com_issue_error(E_POINTER);
		return 0x0;
	}

	if (!*currentFactory)
	{
		if (factoryType != AircraftTypeClass::AbsID && !pThis->IsPrimaryFactory)
			pThis->IsPrimaryFactory = true;

		*currentFactory = pThis;
		return 0;
	}

	if (*currentFactory != pThis)
	{
		if (ShouldSkipParallelProduction(pOwner, pHouseData, factoryType, pThis->Type->Naval))
			return Skip;

		return shouldBlock ? Skip : 0x0;
	}

	return 0x0;
}

bool ShouldSkipParallelProduction(HouseClass* pOwner, HouseExtData* pHouseData, AbstractType factoryType, bool isNaval)
{
	if (!pOwner || !pHouseData)
		return false;

	switch (factoryType)
	{
	case AircraftTypeClass::AbsID:
		if (pOwner->ProducingAircraftTypeIndex >= 0 &&
			pOwner->ProducingAircraftTypeIndex < AircraftTypeClass::Array->Count)
		{
			auto pAircraftType = AircraftTypeClass::Array->Items[pOwner->ProducingAircraftTypeIndex];
			if (pAircraftType)
			{
				return TechnoTypeExtContainer::Instance.Find(pAircraftType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	case InfantryTypeClass::AbsID:
		if (pOwner->ProducingInfantryTypeIndex >= 0 &&
			pOwner->ProducingInfantryTypeIndex < InfantryTypeClass::Array->Count)
		{
			auto pInfantryType = InfantryTypeClass::Array->Items[pOwner->ProducingInfantryTypeIndex];
			if (pInfantryType)
			{
				return TechnoTypeExtContainer::Instance.Find(pInfantryType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	case BuildingTypeClass::AbsID:
		if (pOwner->ProducingBuildingTypeIndex >= 0 &&
			pOwner->ProducingBuildingTypeIndex < BuildingTypeClass::Array->Count)
		{
			auto pBuildingType = BuildingTypeClass::Array->Items[pOwner->ProducingBuildingTypeIndex];
			if (pBuildingType)
			{
				return TechnoTypeExtContainer::Instance.Find(pBuildingType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	case UnitTypeClass::AbsID:
	{
		const int unitTypeIndex = isNaval ? pHouseData->ProducingNavalUnitTypeIndex : pOwner->ProducingUnitTypeIndex;
		if (unitTypeIndex >= 0 && unitTypeIndex < UnitTypeClass::Array->Count)
		{
			auto pUnitType = UnitTypeClass::Array->Items[unitTypeIndex];
			if (pUnitType)
			{
				return TechnoTypeExtContainer::Instance.Find(pUnitType)->ForbidParallelAIQueues.Get(false);
			}
		}
	}
	break;
	default:
		break;
	}
	return false;
}

ASMJIT_PATCH(0x4FEA60, HouseClass_AI_UnitProduction, 0x6)
{
	GET(HouseClass* const, pThis, ECX);

	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 15);

	HouseExtContainer::Instance.Find(pThis)->UpdateVehicleProduction();
	return ret();
}

#include <Ext/Team/Body.h>

template <class T, class TType>
int NOINLINE GetTypeToProduceNew(HouseClass* pHouse)
{
	auto& creationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& values = HouseExtData::AIProduction_Values;
	auto& bestChoices = HouseExtData::AIProduction_BestChoices;

	const size_t typeCount = static_cast<size_t>(TType::Array->Count);

	// Optimize vector operations - resize once, then fill
	if (creationFrames.size() != typeCount)
	{
		creationFrames.resize(typeCount);
		values.resize(typeCount);
	}
	std::fill(creationFrames.begin(), creationFrames.end(), MAX_FRAME_VALUE);
	std::fill(values.begin(), values.end(), 0);
	bestChoices.clear();

	// Process teams to find missing unit types
	ProcessTeamsForTypeProduction<TType>(pHouse, creationFrames, values);

	// Account for existing units that can be recruited
	ProcessExistingUnitsForTypeProduction<T, TType>(pHouse, values);

	// Find the best production choices
	return DetermineBestTypeToProduceNew<TType>(pHouse, creationFrames, values, bestChoices, typeCount);
}

template <class TType>
void ProcessTeamsForTypeProduction(HouseClass* pHouse, std::vector<int>& creationFrames, std::vector<int>& values)
{
	// Move allocation outside the loop for better performance
	DynamicVectorClass<TechnoTypeClass*> missingMembers;
	missingMembers.Reserve(32); // Pre-allocate reasonable capacity

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pHouse)
			continue;

		const int teamCreationFrame = currentTeam->CreationFrame;

		// Original logic: Skip teams that (don't need reinforce OR are full) AND (are active OR have been active)
		// This logic processes teams that either need reinforcement or are not at full strength,
		// but only if they are not currently active or have never been active
		if ((!currentTeam->Type->Reinforce || currentTeam->IsFullStrength) &&
			(currentTeam->IsForcedActive || currentTeam->IsHasBeen))
		{
			continue;
		}

		missingMembers.Clear(); // Reuse existing allocation
		currentTeam->GetTaskForceMissingMemberTypes(missingMembers);

		for (auto pMember : missingMembers)
		{
			if (pMember->WhatAmI() != TType::AbsID)
				continue;

			const size_t typeIndex = static_cast<size_t>(static_cast<TType*>(pMember)->ArrayIndex);

			// Add bounds check to prevent crash
			if (typeIndex >= values.size())
				continue;

			++values[typeIndex];

			if (teamCreationFrame < creationFrames[typeIndex])
				creationFrames[typeIndex] = teamCreationFrame;
		}
	}
}

template <class T, class TType>
void ProcessExistingUnitsForTypeProduction(HouseClass* pHouse, std::vector<int>& values)
{
	const size_t maxTypeIndex = values.size();

	for (auto it = T::Array->begin(); it != T::Array->end(); ++it)
	{
		const size_t typeIndex = static_cast<size_t>((*it)->Type->ArrayIndex);

		// Bounds check to prevent crash
		if (typeIndex >= maxTypeIndex)
			continue;

		if (values[typeIndex] > 0 && (*it)->CanBeRecruited(pHouse))
		{
			--values[typeIndex];
			// Early termination: if this type no longer needed, we can skip checking more units of this type
			// Note: We can't break entirely because there might be other types that still need units
		}
	}
}

template <class TType>
int DetermineBestTypeToProduceNew(HouseClass* pHouse, const std::vector<int>& creationFrames,
	const std::vector<int>& values, std::vector<int>& bestChoices, size_t typeCount)
{
	int bestValue = -1;
	int earliestTypeIndex = -1;
	int earliestFrame = MAX_FRAME_VALUE;

	for (size_t i = 0; i < typeCount; ++i)
	{
		auto pType = TType::Array->Items[static_cast<int>(i)];
		const int currentValue = values[i];

		if (currentValue <= 0)
			continue;

		if (!CanProduceType<TType>(pHouse, pType))
			continue;

		if (bestValue < currentValue || bestValue == -1)
		{
			bestValue = currentValue;
			bestChoices.clear();
		}
		bestChoices.push_back(static_cast<int>(i));

		if (earliestFrame > creationFrames[i] || earliestTypeIndex < 0)
		{
			earliestTypeIndex = static_cast<int>(i);
			earliestFrame = creationFrames[i];
		}
	}

	return SelectFinalTypeToProduceNew(pHouse, bestChoices, earliestTypeIndex);
}

template <class TType>
inline bool CanProduceType(HouseClass* pHouse, TType* pType)
{
	const auto buildResult = pHouse->CanBuild(pType, false, false);

	if (buildResult == CanBuildResult::Unbuildable ||
		pType->GetActualCost(pHouse) > pHouse->Available_Money())
	{
		return false;
	}

	// Special handling for aircraft
	if constexpr (std::is_same_v<TType, AircraftTypeClass>)
	{
		const auto factoryResult = HouseExtData::HasFactory(pHouse, pType, false, true, false, true).first;
		return factoryResult != NewFactoryState::NotFound && factoryResult != NewFactoryState::NoFactory;
	}

	return true;
}

inline int SelectFinalTypeToProduceNew(HouseClass* pHouse, const std::vector<int>& bestChoices, int earliestTypeIndex)
{
	const int aiDifficulty = static_cast<int>(pHouse->GetAIDifficultyIndex());
	auto& random = ScenarioClass::Instance->Random;

	if (random.RandomFromMax(MAX_RANDOM_VALUE) < RulesClass::Instance->FillEarliestTeamProbability[aiDifficulty])
		return earliestTypeIndex;

	if (!bestChoices.empty())
		return bestChoices[random.RandomFromMax(static_cast<int>(bestChoices.size()) - 1)];

	return INVALID_PRODUCTION_INDEX;
}

ASMJIT_PATCH(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingInfantryTypeIndex < 0)
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

	if (pThis->ProducingAircraftTypeIndex < 0)
	{
		const int result = GetTypeToProduceNew<AircraftClass, AircraftTypeClass>(pThis);
		if (result >= 0)
			pThis->ProducingAircraftTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF534;
}