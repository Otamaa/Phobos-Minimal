#include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>

// Constants for better code readability
constexpr int INVALID_PRODUCTION_INDEX = -1;
constexpr int MAX_RANDOM_VALUE = 99;
constexpr int MAX_FRAME_VALUE = 0x7FFFFFFF;
constexpr size_t VECTOR_RESERVE_SIZE = 32;
constexpr size_t MAX_VECTOR_CAPACITY = 1024; // Prevent excessive memory usage

// Forward declarations
bool ShouldSkipParallelProduction(HouseClass* pOwner, HouseExtData* pHouseData, AbstractType factoryType, bool isNaval);

// Utility function to count positive values efficiently
inline int CountPositiveValues(const std::vector<int>& values) noexcept
{
	int count = 0;
	for (const int value : values)
	{
		if (value > 0) [[likely]]
			count += value;
	}
	return count;
}

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

// Final optimized team completion tracking
struct TeamCompletionInfo
{
	TeamClass* pTeam;
	int totalRequired;
	int missingUnits;
	int completionPercentage;
	bool isActiveOrRecruiting;
	bool needsProcessing; // Pre-computed flag to avoid redundant filtering

	TeamCompletionInfo(TeamClass* team) : pTeam(team), totalRequired(0), missingUnits(0),
		completionPercentage(0), isActiveOrRecruiting(false), needsProcessing(false)
	{
		if (!team || !team->Type || !team->Type->TaskForce)
			return;

		isActiveOrRecruiting = team->IsForcedActive || team->IsHasBeen;

		// Calculate total required efficiently - avoid potential overflow
		const int maxEntries = team->Type->TaskForce->CountEntries;
		for (int i = 0; i < maxEntries && totalRequired < 1000; ++i) // Cap to prevent overflow
		{
			const auto& entry = team->Type->TaskForce->Entries[i];
			if (entry.Amount > 0) // Safety check
				totalRequired += entry.Amount;
		}

		if (totalRequired > 0)
		{
			// Use static allocation to avoid repeated DynamicVectorClass creation
			static thread_local DynamicVectorClass<TechnoTypeClass*> missing;
			missing.Clear();
			team->GetTaskForceMissingMemberTypes(missing);
			missingUnits = missing.Count;

			// Clamp values to prevent overflow/underflow
			missingUnits = std::min(missingUnits, totalRequired);

			// Safe percentage calculation
			completionPercentage = ((totalRequired - missingUnits) * 100) / totalRequired;
			completionPercentage = std::clamp(completionPercentage, 0, 100);

			// Fix logic: needsProcessing should match original filtering logic exactly
			needsProcessing = !((!team->Type->Reinforce || team->IsFullStrength) &&
							   (team->IsForcedActive || team->IsHasBeen));
		}
		else
		{
			// Edge case: team with no requirements
			missingUnits = 0;
			completionPercentage = 100; // Consider as complete
			needsProcessing = false; // No processing needed
		}
	}
};

void HouseExtData::UpdateVehicleProduction()
{
	auto pHouse = this->AttachedToObject;
	const int aiDifficulty = static_cast<int>(pHouse->GetAIDifficultyIndex());
	const bool skipGround = pHouse->ProducingUnitTypeIndex != INVALID_PRODUCTION_INDEX;
	const bool skipNaval = this->ProducingNavalUnitTypeIndex != INVALID_PRODUCTION_INDEX;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	// Keep original vectors and logic structure
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

	// Optimized: Analyze team completion status with pre-filtering
	std::vector<TeamCompletionInfo> teamInfos;
	teamInfos.reserve(16); // Pre-allocate reasonable capacity

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pHouse || !currentTeam->Type)
			continue;

		TeamCompletionInfo info(currentTeam);
		// Only add teams that need units AND need processing
		if (info.missingUnits > 0 && info.needsProcessing)
			teamInfos.push_back(info);
	}

	// Early exit if no teams need attention
	if (teamInfos.empty()) [[unlikely]]
	{
		// Still need to process existing units to prevent early production
		ProcessExistingUnitsForProduction(pHouse, values);
		return;
	}

	// Sort by completion priority - teams closer to completion get higher priority
	std::sort(teamInfos.begin(), teamInfos.end(), [](const TeamCompletionInfo& a, const TeamCompletionInfo& b)
 {
	 // Prioritize active/recruiting teams first
	 if (a.isActiveOrRecruiting != b.isActiveOrRecruiting)
		 return a.isActiveOrRecruiting > b.isActiveOrRecruiting;
	 // Then prioritize teams closer to completion
	 if (a.completionPercentage != b.completionPercentage)
		 return a.completionPercentage > b.completionPercentage;
	 // Finally by creation frame (earlier first)
	 return a.pTeam->CreationFrame < b.pTeam->CreationFrame;
	});

	// Process teams with completion priority - no redundant filtering needed
	ProcessTeamsForCompletionFocus(pHouse, creationFrames, values, skipGround, skipNaval, teamInfos);

	// Keep original logic for existing units
	ProcessExistingUnitsForProduction(pHouse, values);

	// Keep original best choices logic
	int earliestTypeIndex = -1;
	int earliestTypeIndexNaval = -1;
	DetermineBestProductionChoices(pHouse, creationFrames, values, bestChoices, bestChoicesNaval,
		unitTypeCount, earliestTypeIndex, earliestTypeIndexNaval);

	// Keep original final selection logic
	SetFinalProductionIndices(pHouse, aiDifficulty, bestChoices, bestChoicesNaval,
		earliestTypeIndex, earliestTypeIndexNaval, skipGround, skipNaval);
}

void HouseExtData::ProcessTeamsForCompletionFocus(HouseClass* pHouse, std::vector<int>& creationFrames,
	std::vector<int>& values, bool skipGround, bool skipNaval, const std::vector<TeamCompletionInfo>& teamInfos)
{
	// Use static allocation to avoid repeated memory allocation
	static thread_local DynamicVectorClass<TechnoTypeClass*> taskForceMembers;
	taskForceMembers.Clear();
	taskForceMembers.Reserve(32);

	// Limit processing to prevent excessive CPU usage in extreme cases
	const size_t maxTeamsToProcess = std::min(teamInfos.size(), static_cast<size_t>(24));

	// Process teams in completion priority order - filtering already done
	for (size_t teamIdx = 0; teamIdx < maxTeamsToProcess; ++teamIdx)
	{
		const auto& teamInfo = teamInfos[teamIdx];
		auto currentTeam = teamInfo.pTeam;
		if (!currentTeam) [[unlikely]]
			continue;

			const int teamCreationFrame = currentTeam->CreationFrame;

			taskForceMembers.Clear();
			currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

			// Optimized completion bonus calculation
			int completionBonus = 1;

			// Active/recruiting teams get highest priority
			if (teamInfo.isActiveOrRecruiting) [[likely]]
				completionBonus += 4; // Increased from 3 for better focus

				// Teams closer to completion get progressive bonus
				if (teamInfo.completionPercentage >= 75) [[unlikely]]
					completionBonus += 3; // Nearly complete teams
				else if (teamInfo.completionPercentage >= 50) [[likely]]
					completionBonus += 2; // Half complete teams
				else if (teamInfo.completionPercentage >= 25) [[likely]]
					completionBonus += 1; // Quarter complete teams

					// Defense teams get additional priority
					if (currentTeam->Type->IsBaseDefense) [[unlikely]]
						completionBonus += 1;

						// Process with early termination for efficiency
						int unitsProcessed = 0;
						const int maxUnitsPerTeam = 16; // Prevent excessive processing

						for (auto currentMember : taskForceMembers)
						{
							if (++unitsProcessed > maxUnitsPerTeam) [[unlikely]]
								break; // Prevent excessive processing

								if (currentMember->WhatAmI() != UnitTypeClass::AbsID) [[unlikely]]
									continue;

									auto pUnitType = static_cast<UnitTypeClass*>(currentMember);

									// Filter unit types efficiently
									if ((skipGround && !pUnitType->Naval) || (skipNaval && pUnitType->Naval)) [[unlikely]]
										continue;

										const size_t index = static_cast<size_t>(pUnitType->ArrayIndex);

										if (index >= values.size()) [[unlikely]]
											continue;

											// Apply completion bonus to prioritize completing teams
											values[index] += completionBonus;

											if (teamCreationFrame < creationFrames[index]) [[likely]]
												creationFrames[index] = teamCreationFrame;
						}
	}
}

void HouseExtData::ProcessExistingUnitsForProduction(HouseClass* pHouse, std::vector<int>& values)
{
	const size_t maxTypeIndex = values.size();

	// Use utility function for better code reuse
	int totalUnitsNeeded = CountPositiveValues(values);

	if (totalUnitsNeeded == 0) [[unlikely]]
		return; // No units needed, skip processing

		// Add safety check for array validity
		if (!UnitClass::Array || UnitClass::Array->Count <= 0) [[unlikely]]
			return;

			for (int i = 0; i < UnitClass::Array->Count && totalUnitsNeeded > 0; ++i)
			{
				const auto pUnit = UnitClass::Array->Items[i];

				// Add critical null pointer check
				if (!pUnit || !pUnit->Type) [[unlikely]]
					continue;

					const size_t typeIndex = static_cast<size_t>(pUnit->Type->ArrayIndex);

					// Bounds check to prevent crash
					if (typeIndex >= maxTypeIndex) [[unlikely]]
						continue;

						if (values[typeIndex] > 0 && pUnit->CanBeRecruited(pHouse)) [[likely]]
						{
							--values[typeIndex];
							--totalUnitsNeeded; // More efficient than nested loop
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
		targetBestChoices.push_back(static_cast<int>(typeIndex));
	}
	else if (targetBestValue == currentValue)
	{
		targetBestChoices.push_back(static_cast<int>(typeIndex));
	}

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

	if (!pUnit || !pFactory || !pFactory->Owner)
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
	{
		const int producingIndex = pOwner->ProducingAircraftTypeIndex;
		if (producingIndex >= 0 && producingIndex < AircraftTypeClass::Array->Count)
		{
			auto pAircraftType = AircraftTypeClass::Array->Items[producingIndex];
			if (pAircraftType)
			{
				return TechnoTypeExtContainer::Instance.Find(pAircraftType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	}
	case InfantryTypeClass::AbsID:
	{
		const int producingIndex = pOwner->ProducingInfantryTypeIndex;
		if (producingIndex >= 0 && producingIndex < InfantryTypeClass::Array->Count)
		{
			auto pInfantryType = InfantryTypeClass::Array->Items[producingIndex];
			if (pInfantryType)
			{
				return TechnoTypeExtContainer::Instance.Find(pInfantryType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	}
	case BuildingTypeClass::AbsID:
	{
		const int producingIndex = pOwner->ProducingBuildingTypeIndex;
		if (producingIndex >= 0 && producingIndex < BuildingTypeClass::Array->Count)
		{
			auto pBuildingType = BuildingTypeClass::Array->Items[producingIndex];
			if (pBuildingType)
			{
				return TechnoTypeExtContainer::Instance.Find(pBuildingType)->ForbidParallelAIQueues.Get(false);
			}
		}
		break;
	}
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
		break;
	}
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
	// Use optimized team completion approach
	static thread_local std::vector<TeamCompletionInfo> teamInfos;
	static thread_local DynamicVectorClass<TechnoTypeClass*> taskForceMembers;

	teamInfos.clear();
	teamInfos.reserve(16);
	taskForceMembers.Reserve(32);

	// Analyze teams that need this type of unit - optimized
	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pHouse || !currentTeam->Type)
			continue;

		// Pre-create info to use cached needsProcessing flag
		TeamCompletionInfo info(currentTeam);
		if (!info.needsProcessing || info.missingUnits <= 0)
			continue;

		// Quick check if team needs this unit type
		taskForceMembers.Clear();
		currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

		bool hasRequiredUnitType = false;
		for (auto currentMember : taskForceMembers)
		{
			if (currentMember->WhatAmI() == TType::AbsID)
			{
				hasRequiredUnitType = true;
				break;
			}
		}

		if (hasRequiredUnitType)
			teamInfos.push_back(info);
	}

	if (teamInfos.empty())
		return INVALID_PRODUCTION_INDEX; // No teams need units, prevent early production

	// Sort by completion priority with improved logic
	std::sort(teamInfos.begin(), teamInfos.end(), [](const TeamCompletionInfo& a, const TeamCompletionInfo& b)
 {
	 // Prioritize active/recruiting teams first
	 if (a.isActiveOrRecruiting != b.isActiveOrRecruiting)
		 return a.isActiveOrRecruiting > b.isActiveOrRecruiting;
	 // Then prioritize teams closer to completion
	 if (a.completionPercentage != b.completionPercentage)
		 return a.completionPercentage > b.completionPercentage;
	 // Finally by creation frame (earlier teams first)
	 return a.pTeam->CreationFrame < b.pTeam->CreationFrame;
	});

	// Use original aggregation approach but with completion priority weighting
	const size_t typeCount = static_cast<size_t>(TType::Array->Count);
	std::vector<int> values(typeCount, 0);
	std::vector<int> creationFrames(typeCount, MAX_FRAME_VALUE);

	// Process teams in completion priority order
	for (const auto& teamInfo : teamInfos)
	{
		auto currentTeam = teamInfo.pTeam;
		if (!currentTeam)
			continue;

		const int teamCreationFrame = currentTeam->CreationFrame;

		DynamicVectorClass<TechnoTypeClass*> taskForceMembers;
		currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

		// Apply completion bonus
		int completionBonus = 1;
		if (teamInfo.isActiveOrRecruiting)
			completionBonus += 3;
		if (teamInfo.completionPercentage >= 50)
			completionBonus += 2;
		if (currentTeam->Type->IsBaseDefense)
			completionBonus += 1;

		for (auto currentMember : taskForceMembers)
		{
			if (currentMember->WhatAmI() != TType::AbsID)
				continue;

			const size_t index = static_cast<size_t>(static_cast<TType*>(currentMember)->ArrayIndex);
			if (index >= values.size())
				continue;

			values[index] += completionBonus;

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	// Account for existing units that can be recruited (keep original logic)
	int totalUnitsNeeded = CountPositiveValues(values);
	if (totalUnitsNeeded > 0 && T::Array && T::Array->Count > 0)
	{
		for (auto it = T::Array->begin(); it != T::Array->end() && totalUnitsNeeded > 0; ++it)
		{
			const auto pUnit = *it;
			if (!pUnit || !pUnit->Type)
				continue;

			const size_t typeIndex = static_cast<size_t>(pUnit->Type->ArrayIndex);
			if (typeIndex >= values.size())
				continue;

			if (values[typeIndex] > 0 && pUnit->CanBeRecruited(pHouse))
			{
				--values[typeIndex];
				--totalUnitsNeeded;
			}
		}
	}

	// Keep original best choices logic
	std::vector<int> bestChoices;
	bestChoices.reserve(32);
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
			bestChoices.push_back(static_cast<int>(i));
		}
		else if (bestValue == currentValue)
		{
			bestChoices.push_back(static_cast<int>(i));
		}

		if (earliestFrame > creationFrames[i] || earliestTypeIndex < 0)
		{
			earliestTypeIndex = static_cast<int>(i);
			earliestFrame = creationFrames[i];
		}
	}

	// Keep original final selection logic
	const int aiDifficulty = static_cast<int>(pHouse->GetAIDifficultyIndex());
	auto& random = ScenarioClass::Instance->Random;

	if (random.RandomFromMax(MAX_RANDOM_VALUE) < RulesClass::Instance->FillEarliestTeamProbability[aiDifficulty])
		return earliestTypeIndex;

	if (!bestChoices.empty())
	{
		const int choicesCount = static_cast<int>(bestChoices.size());
		if (choicesCount == 1)
			return bestChoices[0];
		else
			return bestChoices[random.RandomFromMax(choicesCount - 1)];
	}

	return INVALID_PRODUCTION_INDEX;
}

template <class TType>
inline bool CanProduceType(HouseClass* pHouse, TType* pType)
{
	const auto buildResult = pHouse->CanBuild(pType, false, false);

	if (buildResult == CanBuildResult::Unbuildable ||
		pType->GetActualCost(pHouse) > pHouse->Available_Money()) [[unlikely]]
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