 #include "Body.h"

#include <Ext/House/Body.h>
#ifndef aaa
std::tuple<BuildingClass**, bool, AbstractType> GetFactory(AbstractType AbsType, bool naval, HouseExtData* pData)
{
	BuildingClass** currFactory = nullptr;
	bool block = false;
	auto pRules = RulesExtData::Instance();

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

//#include <ostream>

static std::map<void*, std::string> MappedCaller {};
//
//DEFINE_HOOK(0x7353C0, UnitClass_CTOR_RecordCaller , 0x7) {
//
//	GET(UnitClass*, pThis, ECX);
//	GET_STACK(UnitTypeClass*, pType, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//	MappedCaller[pThis] = std::make_pair(pType , std::to_string(caller));
//	return 0x0;
//}

//DEFINE_HOOK(0x7C8E17, Game_OperatorNew_Map, 0x6)
//{
//	GET_STACK(int, size, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	const auto pTr = YRMemory::__mh_malloc(size , 1);
//	MappedCaller[pTr] = std::to_string(caller);
//	R->EAX(pTr);
//	return 0x7C8E24;
//}
//
//DEFINE_HOOK(0x7C9430, Game_MAlloc_Map, 0x6)
//{
//	GET_STACK(int, size, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	const auto pTr = YRMemory::__mh_malloc(size, CRT::AllocatorMode());
//	MappedCaller[pTr] = std::to_string(caller);
//	R->EAX(pTr);
//	return 0x7C9441;
//}
//
//DEFINE_HOOK(0x7C8B3D, Game_OperatorDelete_UnMap, 0x9)
//{
//	GET_STACK(void*, ptr, 0x4);
//	MappedCaller.erase(ptr);
//	YRMemory::__free(ptr);
//	return 0x7C8B47;
//}

void HouseExtData::UpdateVehicleProduction()
{
	auto pThis = this->AttachedToObject;
	const auto AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if ((skipGround && skipNaval) || (!skipGround && this->UpdateHarvesterProduction()))
		return;

	auto& creationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& values = HouseExtData::AIProduction_Values;
	auto& bestChoices = HouseExtData::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExtData::AIProduction_BestChoicesNaval;

	auto count = static_cast<size_t>(UnitTypeClass::Array->Count);
	creationFrames.assign(count, 0x7FFFFFFF);
	values.assign(count, 0);

//	std::vector<TeamClass*> Teams;

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pThis)
			continue;

//		if (IS_SAME_STR_(currentTeam->Type->ID, "0100003I-G"))
//			Debug::Log("HereIam\n");

//		Teams.push_back(currentTeam);
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
			const auto what = currentMember->WhatAmI();

			if (what != UnitTypeClass::AbsID ||
				(skipGround && !currentMember->Naval) ||
				(skipNaval && currentMember->Naval))
				continue;

			const auto index = static_cast<size_t>(currentMember->GetArrayIndex());
			++values[index];

//			if (IS_SAME_STR_(currentTeam->Type->ID, "0100003I-G")) {
//				Debug::Log("0100003I Unit %s  idx %d AddedValueResult %d\n", currentMember->ID, index, values[index]);
//			}

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

//	for (int i = 0; i < (int)Teams.size(); ++i) {
//		Debug::Log("House [%s] Have [%d] Teams %s.\n", pThis->get_ID(), i, Teams[i]->get_ID());
//	}

	//std::vector<int> Toremove {};
	for (int i = 0; i < UnitClass::Array->Count; ++i) {
		const auto pUnit = UnitClass::Array->Items[i];

		//if (VTable::Get(pUnit) != UnitClass::vtable){

		//	const char* Caller = "unk";
		//	//const char* Type = "unk";
		//	if (MappedCaller.contains(pUnit)) {
		//		Caller = MappedCaller[pUnit].c_str();
		//	}

		//	Debug::Log("UpdateVehicleProduction for [%s] UnitClass Array(%d) at [%d] contains broken pointer[%x allocated from %s] WTF ???\n", pThis->get_ID() , UnitClass::Array->Count , i, pUnit , Caller);
		//	Toremove.push_back(i);
		//	continue;
		//}

		const auto index = static_cast<unsigned int>(pUnit->Type->GetArrayIndex());

		if (values[index] > 0 && pUnit->CanBeRecruited(pThis))
			--values[index];
	}

	//for (auto ToRemoveIdx : Toremove) { 
	//	UnitClass::Array->RemoveAt(ToRemoveIdx);
	//}

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

	const double earliestOdds = 0.01 * RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty];

	if (!skipGround)
	{
		auto result = -1;
		if (earliestOdds <= ScenarioClass::Instance->Random.RandomDouble())
		{
			result = earliestTypenameIndex;
		}
		else if (const auto size = int(bestChoices.size()))
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
		else if (const auto size = int(bestChoicesNaval.size()))
		{
			result = bestChoicesNaval[static_cast<size_t>(ScenarioClass::Instance->Random.RandomFromMax(size - 1))];
		}


		this->ProducingNavalUnitTypeIndex = result;
	}
}

//DEFINE_OVERRIDE_HOOK(0x7258D0, AnnounceInvalidPointer_PhobosGlobal_Mapped, 0x6)
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
//DEFINE_HOOK(0x685659, Scenario_ClearClasses_PhobosGlobal_Mapped, 0xA)
//{ 
//	MappedCaller.clear();
//	return 0x0;
//}

DEFINE_HOOK(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6) //was C
{
	GET(BuildingClass*, pBuilding, ESI);

	auto pRules = RulesExtData::Instance();

	if (!pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues))
		return 0;

	if (!pBuilding->Owner || pBuilding->Owner->IsNeutral() || pBuilding->Owner->IsControlledByHuman())
		return 0;

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

//DEFINE_HOOK(0x04500FA, BuildingClass_AI_Factory_SkipNoneForComputer, 0x6)
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

DEFINE_OVERRIDE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EDX);
	HouseExtContainer::Instance.Find(pHouse)->Factory_AircraftType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EAX);
	HouseExtContainer::Instance.Find(pHouse)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	HouseExtContainer::Instance.Find(pHouse)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);

	if (pUnit->Type->Naval)
		pHouseExt->Factory_NavyType = nullptr;
	else
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if(HouseClass* pOwner = pFactory->Owner) {
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

DEFINE_OVERRIDE_HOOK(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	enum { Skip = 0x4503CA };

	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;
	if (!pOwner || !pOwner->Production)
		return 0x0;

	auto pRules = RulesExtData::Instance();
	HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);
	const auto&[curFactory , block , type] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pData);

	if (!curFactory) {
		Game::RaiseError(E_POINTER);
	}
	else if (!*curFactory)
	{
		if(type != AircraftTypeClass::AbsID){
			if (!pThis->IsPrimaryFactory)
				pThis->IsPrimaryFactory = true;
		}

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

#ifdef sss
	if (pThis->ProducingUnitTypeIndex != -1)
	{
		return ret();
	}

	auto const pRules = RulesClass::Instance();
	auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());
	auto const idxParentCountry = pThis->Type->FindParentCountryIndex();
	auto const pHarvester = HouseExtData::FindOwned(
		pThis, idxParentCountry, make_iterator(pRules->HarvesterUnit));

	if (pHarvester)
	{
		//Buildable harvester found
		auto const harvesters = pThis->CountResourceGatherers;

		auto maxHarvesters = HouseExtData::FindBuildable(
			pThis, idxParentCountry, make_iterator(pRules->BuildRefinery))
			? pRules->HarvestersPerRefinery[AIDiff] * pThis->CountResourceDestinations
			: pRules->AISlaveMinerNumber[AIDiff];

		if (pThis->IQLevel2 >= pRules->Harvester && !pThis->IsTiberiumShort
			&& !pThis->IsControlledByHuman() && harvesters < maxHarvesters
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
			auto const pRefinery = HouseExtData::FindBuildable(
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
	HouseExtContainer::Instance.Find(pThis)->UpdateVehicleProduction();
#endif
	return ret();
}

DEFINE_OVERRIDE_HOOK(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingInfantryTypeIndex == -1)
	{
		auto& CreationFrames = HouseExtData::AIProduction_CreationFrames;
		auto& Values = HouseExtData::AIProduction_Values;
		auto& BestChoices = HouseExtData::AIProduction_BestChoices;

		auto const count = static_cast<unsigned int>(InfantryTypeClass::Array->Count);
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
				if (CurrentMember->WhatAmI() != InfantryTypeClass::AbsID)
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

		for (auto T : *InfantryClass::Array)
		{
			//if (VTable::Get(T) != InfantryClass::vtable)
			//	Debug::FatalErrorAndExit("InfantryClass Array contains broken pointer WTF ???\n");

			auto const Idx = static_cast<unsigned int>(T->Type->GetArrayIndex());
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
			auto const TT = InfantryTypeClass::Array->Items[static_cast<int>(i)];
			int CurrentValue = Values[i];
			if (CurrentValue <= 0)
				continue;

			const auto buildableResult = pThis->CanBuild(TT, false, false);

			if (buildableResult == CanBuildResult::Unbuildable
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
			pThis->ProducingInfantryTypeIndex = EarliestTypenameIndex;
		}
		else if (auto const size = int(BestChoices.size()))
		{
			int RandomChoice = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			pThis->ProducingInfantryTypeIndex = BestChoices[static_cast<unsigned int>(RandomChoice)];
		}
		else
			pThis->ProducingInfantryTypeIndex = -1;
	}

	R->EAX(15);
	return 0x4FF204;
}

DEFINE_OVERRIDE_HOOK(0x4FF210, HouseClass_AI_AircraftProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingAircraftTypeIndex == -1)
	{
		auto& CreationFrames = HouseExtData::AIProduction_CreationFrames;
		auto& Values = HouseExtData::AIProduction_Values;
		auto& BestChoices = HouseExtData::AIProduction_BestChoices;

		auto const count = static_cast<unsigned int>(AircraftTypeClass::Array->Count);
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
				if (CurrentMember->WhatAmI() != AircraftTypeClass::AbsID)
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

		for (auto T : *AircraftClass::Array)
		{
			//if (VTable::Get(T) != AircraftClass::vtable)
			//	Debug::FatalErrorAndExit("AircraftClass Array contains broken pointer WTF ???\n");

			auto const Idx = static_cast<unsigned int>(T->Type->GetArrayIndex());
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
			auto const TT = AircraftTypeClass::Array->Items[static_cast<int>(i)];
			int CurrentValue = Values[i];

			if (CurrentValue <= 0)
				continue;

			const auto buildableResult = pThis->CanBuild(TT, false, false);

			if(buildableResult == CanBuildResult::Unbuildable
				||TT->GetActualCost(pThis) > pThis->Available_Money())
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

		const int Dockercount = std::count_if(pThis->Buildings.begin(), pThis->Buildings.end(), [](BuildingClass* pBld)
		{
			const auto pExt = BuildingExtContainer::Instance.Find(pBld);
			if (pExt->AboutToChronoshift || pExt->LimboID >= 0)
				return false;

			const bool forbidden = (!pBld->IsAlive
				|| pBld->GetCurrentMission() == Mission::Selling
				|| pBld->QueuedMission == Mission::Selling
				|| pBld->Health <= 0
				|| pBld->InLimbo
				);

			if (forbidden)
				return false;

			if (pBld->Type->Factory == AircraftTypeClass::AbsID) {
				if (BuildingExtData::HasFreeDocks(pBld))
					return true;
			}

			return false;
		});

		if (!Dockercount) {
			pThis->ProducingAircraftTypeIndex = -1;
			R->EAX(15);
			return 0x4FF534;
		}

		auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());
		auto EarliestOdds = 0.01 * RulesClass::Instance->FillEarliestTeamProbability[AIDiff];
		if (EarliestOdds <= ScenarioClass::Instance->Random.RandomDouble())
		{
			pThis->ProducingAircraftTypeIndex = EarliestTypenameIndex;
		}
		else if (auto const size = int(BestChoices.size()))
		{
			int RandomChoice = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			pThis->ProducingAircraftTypeIndex = BestChoices[static_cast<unsigned int>(RandomChoice)];
		}
		else
			pThis->ProducingAircraftTypeIndex = -1;

	}

	R->EAX(15);
	return 0x4FF534;
}
#endif