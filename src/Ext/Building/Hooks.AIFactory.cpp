 #include "Body.h"

#include <Ext/House/Body.h>


std::tuple<BuildingClass**, bool, AbstractType> GetFactory(AbstractType AbsType, bool naval, HouseExt::ExtData* pData)
{
	BuildingClass** currFactory = nullptr;
	bool block = false;
	auto pRules = RulesExt::Global();

	switch (AbsType)
	{
	case AbstractType::BuildingType:
	{
		currFactory = &pData->Factory_BuildingType;
		block = pRules->ForbidParallelAIQueues_Building.Get(!pRules->AllowParallelAIQueues);
		break;
	}
	case AbstractType::UnitType:
	{
		if (!naval)
		{
			block = pRules->ForbidParallelAIQueues_Vehicle.Get(!pRules->AllowParallelAIQueues);
			currFactory = &pData->Factory_VehicleType;
		}
		else
		{
			block = pRules->ForbidParallelAIQueues_Navy.Get(!pRules->AllowParallelAIQueues);
			currFactory = &pData->Factory_NavyType;
		}

		break;
	}
	case AbstractType::InfantryType:
	{
		block = pRules->ForbidParallelAIQueues_Infantry.Get(!pRules->AllowParallelAIQueues);
		currFactory = &pData->Factory_InfantryType;
		break;
	}
	case AbstractType::AircraftType:
	{
		currFactory = &pData->Factory_AircraftType;
		block = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);
		break;
	}
	default:
		break;
	}

	return { currFactory  , block ,AbsType };
}

template <class TClass, class TType>
void GetTypeToProduce(HouseClass* pThis, int& ProducingTypeIndex)
{
	auto& CreationFrames = HouseExt::AIProduction_CreationFrames;
	auto& Values = HouseExt::AIProduction_Values;
	auto& BestChoices = HouseExt::AIProduction_BestChoices;

	auto const count = static_cast<unsigned int>(TType::Array->Count);
	CreationFrames.assign(count, 0x7FFFFFFF);
	Values.assign(count, 0);

	for (auto CurrentTeam : *TeamClass::Array)
	{
		if (!CurrentTeam || CurrentTeam->Owner != pThis)
		{
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if ((!CurrentTeam->Type->Reinforce || CurrentTeam->IsFullStrength)
		  && (CurrentTeam->IsForcedActive || CurrentTeam->IsHasBeen))
		{
			continue;
		}

		DynamicVectorClass<TechnoTypeClass*> TaskForceMembers;
		CurrentTeam->GetTaskForceMissingMemberTypes(TaskForceMembers);
		for (auto CurrentMember : TaskForceMembers)
		{
			if (CurrentMember->WhatAmI() != TType::AbsID)
			{
				continue;
			}
			auto const Idx = static_cast<unsigned int>(CurrentMember->GetArrayIndex());
			++Values[Idx];
			if (TeamCreationFrame < CreationFrames[Idx])
			{
				CreationFrames[Idx] = TeamCreationFrame;
			}
		}
	}

	for (auto T : *TClass::Array)
	{
		auto const Idx = static_cast<unsigned int>(T->GetType()->GetArrayIndex());
		if (Values[Idx] > 0 && T->CanBeRecruited(pThis))
		{
			--Values[Idx];
		}
	}

	BestChoices.clear();

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto const TT = TType::Array->Items[static_cast<int>(i)];
		int CurrentValue = Values[i];
		if (CurrentValue <= 0 || pThis->CanBuild(TT, false, false) == CanBuildResult::Unbuildable
			|| TT->GetActualCost(pThis) > pThis->Available_Money())
		{
			continue;
		}

		if (BestValue < CurrentValue || BestValue == -1)
		{
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(static_cast<int>(i));
		if (EarliestFrame > CreationFrames[i] || EarliestTypenameIndex == -1)
		{
			EarliestTypenameIndex = static_cast<int>(i);
			EarliestFrame = CreationFrames[i];
		}
	}

	auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());
	auto EarliestOdds = 0.01 * RulesClass::Instance->FillEarliestTeamProbability[AIDiff];
	if (EarliestOdds <= ScenarioClass::Instance->Random.RandomDouble())
	{
		ProducingTypeIndex = EarliestTypenameIndex;
	}
	else if (auto const size = static_cast<int>(BestChoices.size()))
	{
		int RandomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
		ProducingTypeIndex = BestChoices[static_cast<unsigned int>(RandomChoice)];
	}
	else
		ProducingTypeIndex = -1;
}

void HouseExt::ExtData::UpdateVehicleProduction()
{
	auto pThis = this->Get();
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	auto& creationFrames = HouseExt::AIProduction_CreationFrames;
	auto& values = HouseExt::AIProduction_Values;
	auto& bestChoices = HouseExt::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExt::AIProduction_BestChoicesNaval;

	auto count = static_cast<size_t>(UnitTypeClass::Array->Count);
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
			if (!Is_UnitType(currentMember) ||
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
		auto type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		int currentValue = values[i];

		if (currentValue <= 0
			|| pThis->CanBuild(type, false, false) == CanBuildResult::Unbuildable
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

	const double earliestOdds = 0.01 * RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty];

	if (!skipGround)
	{
		auto result = -1;
		if (earliestOdds <= ScenarioClass::Instance->Random.RandomDouble())
		{
			result = earliestTypenameIndex;
		}
		else if (const auto size = static_cast<int>(bestChoices.size()))
		{
			result = bestChoices[static_cast<size_t>(ScenarioClass::Instance->Random.RandomFromMax(size - 1))];
		}

		pThis->ProducingUnitTypeIndex = result;
	}

	if (!skipNaval)
	{
		auto result = -1;
		if (earliestOdds <= ScenarioClass::Instance->Random.RandomDouble())
		{
			result = earliestTypenameIndexNaval;
		}
		else if (const auto size = static_cast<int>(bestChoicesNaval.size()))
		{
			result = bestChoicesNaval[static_cast<size_t>(ScenarioClass::Instance->Random.RandomFromMax(size - 1))];
		}


		this->ProducingNavalUnitTypeIndex = result;
	}
}

DEFINE_HOOK(0x4401BB, Factory_AI_PickWithFreeDocks, 0x6) //was C
{
	GET(BuildingClass*, pBuilding, ESI);

	auto pRules = RulesExt::Global();

	if (!pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues))
		return 0;

	if (!pBuilding || !pBuilding->Owner)
		return 0;

	if (pBuilding->Owner->IsControlledByHuman_() || pBuilding->Owner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			BuildingExt::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

//DEFINE_HOOK(0x04500FA, BuildingClass_AI_Factory_SkipNoneForComputer, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	auto pRules = RulesExt::Global();
//
//	if (pThis->Type->Factory == AbstractType::AircraftType && pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues))
//		if(!pThis->Factory && !pThis->IsPrimaryFactory && pThis->Owner && pThis->Owner->IsControlledByHuman_())
//		return 0x4503CA;
//
//	return 0x0;
//}

DEFINE_OVERRIDE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EDX);
	HouseExt::ExtMap.Find(pHouse)->Factory_AircraftType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EAX);
	HouseExt::ExtMap.Find(pHouse)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	HouseExt::ExtMap.Find(pHouse)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	if (pUnit->Type->Naval)
		pHouseExt->Factory_NavyType = nullptr;
	else
		pHouseExt->Factory_VehicleType = nullptr;


	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	auto pRules = RulesExt::Global();

	if(HouseClass* pOwner = pFactory->Owner) {
		HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
		TechnoClass* pTechno = pFactory->Object;

		switch ((((DWORD*)pTechno)[0]))
	{
	case BuildingClass::vtable:
			pData->Factory_BuildingType = nullptr;
		break;
	case UnitClass::vtable:
		if (!pTechno->GetTechnoType()->Naval)
				pData->Factory_VehicleType = nullptr;
		else
				pData->Factory_NavyType = nullptr;
		break;
	case InfantryClass::vtable:
			pData->Factory_InfantryType = nullptr;
		break;
	case AircraftClass::vtable:
			pData->Factory_AircraftType = nullptr;
		break;
	}
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	enum { Skip = 0x4503CA };

	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;
	if (!pOwner || !pOwner->Production)
		return 0x0;

	auto pRules = RulesExt::Global();
	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
	const  auto&[curFactory , block , type] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pData);

	if (!curFactory) {
		Game::RaiseError(E_POINTER);
	}
	else if (!*curFactory)
	{
		if (!pThis->IsPrimaryFactory)
			pThis->IsPrimaryFactory = true;

			*curFactory = pThis; //last check
		return 0;
	}
	else if (*curFactory != pThis)
	{
		return block ? Skip : 0x0;
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x4FEA60, HouseClass_AI_UnitProduction, 0x6)
{
	GET(HouseClass* const, pThis, ECX);

	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 15);

#ifdef aaa
	if (pThis->ProducingUnitTypeIndex != -1)
	{
		return ret();
	}

	auto const pRules = RulesClass::Instance();
	auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());
	auto const idxParentCountry = pThis->Type->FindParentCountryIndex();
	auto const pHarvester = HouseExt::FindOwned(
		pThis, idxParentCountry, make_iterator(pRules->HarvesterUnit));

	if (pHarvester)
	{
		//Buildable harvester found
		auto const harvesters = pThis->CountResourceGatherers;

		auto maxHarvesters = HouseExt::FindBuildable(
			pThis, idxParentCountry, make_iterator(pRules->BuildRefinery))
			? pRules->HarvestersPerRefinery[AIDiff] * pThis->CountResourceDestinations
			: pRules->AISlaveMinerNumber[AIDiff];

		if (pThis->IQLevel2 >= pRules->Harvester && !pThis->IsTiberiumShort
			&& !pThis->IsControlledByHuman_() && harvesters < maxHarvesters
			&& pThis->TechLevel >= pHarvester->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvester->ArrayIndex;
			return ret();
		}
	}
	else
	{
		//No buildable harvester found
		auto const maxHarvesters = pRules->AISlaveMinerNumber[AIDiff];

		if (pThis->CountResourceGatherers < maxHarvesters)
		{
			auto const pRefinery = HouseExt::FindBuildable(
				pThis, idxParentCountry, make_iterator(pRules->BuildRefinery));

			if (pRefinery)
			{
				//awesome way to find out whether this building is a slave miner, isn't it? ...
				if (auto const pSlaveMiner = pRefinery->UndeploysInto)
				{
					pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
					return ret();
				}
			}
		}
	}

	GetTypeToProduce<UnitClass, UnitTypeClass>(pThis, pThis->ProducingUnitTypeIndex);

#else
	HouseExt::ExtMap.Find(pThis)->UpdateVehicleProduction();
#endif
	return ret();
}

DEFINE_OVERRIDE_HOOK(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingInfantryTypeIndex == -1)
	{
		GetTypeToProduce<InfantryClass, InfantryTypeClass>(pThis, pThis->ProducingInfantryTypeIndex);
	}

	R->EAX(15);
	return 0x4FF204;
}

DEFINE_OVERRIDE_HOOK(0x4FF210, HouseClass_AI_AircraftProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingAircraftTypeIndex == -1)
	{
		GetTypeToProduce<AircraftClass, AircraftTypeClass>(pThis, pThis->ProducingAircraftTypeIndex);
	}

	R->EAX(15);
	return 0x4FF534;
}
