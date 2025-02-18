 #include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>

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
		if (pData->AttachedToObject->ProducingBuildingTypeIndex >= 0) {
			block = TechnoTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items
				[pData->AttachedToObject->ProducingBuildingTypeIndex])->ForbidParallelAIQueues.Get(block);
		}
		break;
	}
	case AbstractType::UnitType:
	{
		if (!naval)
		{
			block = pRules->ForbidParallelAIQueues_Vehicle.Get(!pRules->AllowParallelAIQueues);
			if (pData->AttachedToObject->ProducingUnitTypeIndex >= 0) {
				block = TechnoTypeExtContainer::Instance.Find(UnitTypeClass::Array->Items
				[pData->AttachedToObject->ProducingUnitTypeIndex])->ForbidParallelAIQueues.Get(block);
			}
			currFactory = &pData->Factory_VehicleType;
		}
		else
		{
			block = pRules->ForbidParallelAIQueues_Navy.Get(!pRules->AllowParallelAIQueues);
			if (pData->ProducingNavalUnitTypeIndex >= 0) {
				block = TechnoTypeExtContainer::Instance.Find(UnitTypeClass::Array->Items
				[pData->ProducingNavalUnitTypeIndex])->ForbidParallelAIQueues.Get(block);
			}
			currFactory = &pData->Factory_NavyType;
		}

		break;
	}
	case AbstractType::InfantryType:
	{
		block = pRules->ForbidParallelAIQueues_Infantry.Get(!pRules->AllowParallelAIQueues);
		if (pData->AttachedToObject->ProducingInfantryTypeIndex >= 0) {
			block = TechnoTypeExtContainer::Instance.Find(InfantryTypeClass::Array->Items
			[pData->AttachedToObject->ProducingInfantryTypeIndex])->ForbidParallelAIQueues.Get(block);
		}
		currFactory = &pData->Factory_InfantryType;
		break;
	}
	case AbstractType::AircraftType:
	{
		currFactory = &pData->Factory_AircraftType;
		block = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);
		if (pData->AttachedToObject->ProducingAircraftTypeIndex >= 0) {
			block = TechnoTypeExtContainer::Instance.Find(AircraftTypeClass::Array->Items
				[pData->AttachedToObject->ProducingAircraftTypeIndex])->ForbidParallelAIQueues.Get(block);
		}

		break;
	}
	default:
		break;
	}

	return { currFactory  , block ,AbsType };
}

//#include <ostream>

//static std::map<void*, std::string> MappedCaller {};
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
//	const auto pTr = __mh_malloc(size , 1);
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
//	const auto pTr = __mh_malloc(size, CRT::AllocatorMode());
//	MappedCaller[pTr] = std::to_string(caller);
//	R->EAX(pTr);
//	return 0x7C9441;
//}
//
//DEFINE_HOOK(0x7C8B3D, Game_OperatorDelete_UnMap, 0x9)
//{
//	GET_STACK(void*, ptr, 0x4);
//	MappedCaller.erase(ptr);
//	__free(ptr);
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
//			Debug::LogInfo("HereIam");

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

			const auto index = static_cast<size_t>(((UnitTypeClass*)currentMember)->ArrayIndex);
			++values[index];

//			if (IS_SAME_STR_(currentTeam->Type->ID, "0100003I-G")) {
//				Debug::LogInfo("0100003I Unit %s  idx %d AddedValueResult %d", currentMember->ID, index, values[index]);
//			}

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

//	for (int i = 0; i < (int)Teams.size(); ++i) {
//		Debug::LogInfo("House [%s] Have [%d] Teams %s.", pThis->get_ID(), i, Teams[i]->get_ID());
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

		//	Debug::LogInfo("UpdateVehicleProduction for [%s] UnitClass Array(%d) at [%d] contains broken pointer[%x allocated from %s] WTF ???", pThis->get_ID() , UnitClass::Array->Count , i, pUnit , Caller);
		//	Toremove.push_back(i);
		//	continue;
		//}

		if (values[pUnit->Type->ArrayIndex] > 0 && pUnit->CanBeRecruited(pThis))
			--values[pUnit->Type->ArrayIndex];
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

	if (!skipGround)
	{
		int result_ground = earliestTypenameIndex;
		if (ScenarioClass::Instance->Random.RandomFromMax(99) >= RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty]) {
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

//DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer_PhobosGlobal_Mapped, 0x6)
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

	if (!pBuilding->Owner || pBuilding->Owner->IsControlledByHuman() || pBuilding->Owner->IsNeutral())
		return 0x0;

	auto pRules = RulesExtData::Instance();

	bool ForbidParallelAIQueues_ = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);

	if (auto const pType = pBuilding->Owner->ProducingAircraftTypeIndex >= 0 ?
		AircraftTypeClass::Array()->Items[pBuilding->Owner->ProducingAircraftTypeIndex] : nullptr) {
		ForbidParallelAIQueues_ = TechnoTypeExtContainer::Instance.Find(pType)->ForbidParallelAIQueues.Get(ForbidParallelAIQueues_);
	}

	if (!ForbidParallelAIQueues_) {
		return 0;
	}

	if (pBuilding->Type->Factory == AbstractType::AircraftType) {
		if (pBuilding->Factory
			&& !BuildingExtData::HasFreeDocks(pBuilding)) {
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

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EDX);
	GET(BuildingClass*, pThis , ESI);

	auto pExt = pHouse->_GetExtData();

	if(pThis == pExt->Factory_AircraftType)
		pExt->Factory_AircraftType = nullptr;

	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis , ESI);

	auto pExt = pHouse->_GetExtData();

	if(pThis == pExt->Factory_BuildingType)
		pExt->Factory_BuildingType = nullptr;

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(FakeHouseClass*, pHouse, EAX);
	GET(BuildingClass*, pThis , ESI);

	auto pExt = pHouse->_GetExtData();

	if(pThis == pExt->Factory_InfantryType)
		pExt->Factory_InfantryType = nullptr;

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pHouseExt = HouseExtContainer::Instance.Find(pFactory->Owner);

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if(!pUnit->Type->Naval && pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
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

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory, 0x6)
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
		_com_issue_error(E_POINTER);
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
		switch (type)
		{
		case AircraftTypeClass::AbsID:
		{
			if (pOwner->ProducingAircraftTypeIndex >= 0)
			{
				if (TechnoTypeExtContainer::Instance.Find(AircraftTypeClass::Array->Items
					[pOwner->ProducingAircraftTypeIndex])->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case InfantryTypeClass::AbsID:
		{
			if (pOwner->ProducingInfantryTypeIndex >= 0)
			{
				if (TechnoTypeExtContainer::Instance.Find(InfantryTypeClass::Array->Items
					[pOwner->ProducingInfantryTypeIndex])->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case BuildingTypeClass::AbsID:
		{
			if (pOwner->ProducingBuildingTypeIndex >= 0)
			{
				if (TechnoTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items
					[pOwner->ProducingBuildingTypeIndex])->ForbidParallelAIQueues)
				{
					return Skip;
				}
			}
			break;
		}
		case UnitTypeClass::AbsID:
		{
			const int idx = pThis->Type->Naval ? pData->ProducingNavalUnitTypeIndex : pOwner->ProducingUnitTypeIndex;
			if (idx >= 0)
			{
				if (TechnoTypeExtContainer::Instance.Find(UnitTypeClass::Array->Items
					[idx])->ForbidParallelAIQueues)
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

DEFINE_HOOK(0x4FEA60, HouseClass_AI_UnitProduction, 0x6)
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

#include <Ext/Team/Body.h>
//#pragma optimize("", off )
template <class T, class Ttype >
int NOINLINE GetTypeToProduceNew(HouseClass* pHouse) {

	auto& CreationFrames = HouseExtData::AIProduction_CreationFrames;
	auto& Values = HouseExtData::AIProduction_Values;
	auto& BestChoices = HouseExtData::AIProduction_BestChoices;

	auto const count = static_cast<unsigned int>(Ttype::Array->Count);
	CreationFrames.assign(count, 0x7FFFFFFF);
	Values.assign(count, 0);
	BestChoices.clear();

	//Debug::LogInfo(__FUNCTION__" Executing with Current TeamArrayCount[%d] for[%s][House %s - %x] ", TeamClass::Array->Count, AbstractClass::GetAbstractClassName(Ttype::AbsID), pHouse->get_ID() , pHouse);
	for (auto CurrentTeam : *TeamClass::Array)
	{
		if (!CurrentTeam || CurrentTeam->Owner != pHouse)
		{
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if (CurrentTeam->Type->Reinforce && !CurrentTeam->IsFullStrength || !CurrentTeam->IsForcedActive && !CurrentTeam->IsHasBeen) {
			DynamicVectorClass<TechnoTypeClass*> arr;
			CurrentTeam->GetTaskForceMissingMemberTypes(arr);

			for (auto pMember : arr) {

				if (pMember->WhatAmI() != Ttype::AbsID)
				{
					continue;
				}

				auto const Idx = static_cast<unsigned int>(((Ttype*)pMember)->ArrayIndex);

				++Values[Idx];
				if (TeamCreationFrame < CreationFrames[Idx])
				{
					CreationFrames[Idx] = TeamCreationFrame;
				}
			}
		}
	}

	for (auto classPos = T::Array->begin(); classPos != T::Array->end(); ++classPos) {
		auto const Idx = static_cast<unsigned int>((*classPos)->Type->ArrayIndex);
		if (Values[Idx] > 0 && (*classPos)->CanBeRecruited(pHouse)) {
			--Values[Idx];
		}
	}

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto const TT = Ttype::Array->Items[static_cast<int>(i)];

		int CurrentValue = Values[i];

		if (CurrentValue <= 0)
			continue;

		const auto buildableResult = pHouse->CanBuild(TT, false, false);

		// Aircraft has it own handling
		if COMPILETIMEEVAL (Ttype::AbsID == AbstractType::AircraftType) {
			//Debug::LogInfo("Aircraft [%s][%s] return result [%d] for can build");

			if (buildableResult != CanBuildResult::Buildable || TT->GetActualCost(pHouse) > pHouse->Available_Money()) {
				continue;
			}

			//yes , we checked this fucking twice just to make sure
			const auto factoryresult = HouseExtData::HasFactory(pHouse, TT, false, true, false, true).first;

			if (factoryresult == NewFactoryState::NotFound || factoryresult == NewFactoryState::NoFactory) {
				continue;
			}

		} else {
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

//#pragma optimize("", on )
DEFINE_HOOK(0x6EF4D0, TeamClass_GetRemainingTaskForceMembers, 0x8)
{
	GET(TeamClass*, pThis, ECX);
	GET_STACK(DynamicVectorClass<TechnoTypeClass*>*, pVec, 0x4);

	const auto pType = pThis->Type;
	const auto pTaskForce = pType->TaskForce;

	for (int a = 0; a < pTaskForce->CountEntries; ++a) {
		for (int i = 0; i < pTaskForce->Entries[a].Amount; ++i) {
			if(auto pType = pTaskForce->Entries[a].Type) {
				pVec->AddItem(pType);
			}
		}
	}

	//remove first finded similarity
	for (auto pMember = pThis->FirstUnit; pMember; pMember = pMember->NextTeamMember) {
		for (auto pMemberNeeded : *pVec) {
			if ((pMemberNeeded == pMember->GetTechnoType()
				|| TechnoExtContainer::Instance.Find(pMember)->Type == pMemberNeeded
				//|| TeamExtData::GroupAllowed(pMemberNeeded, pMember->GetTechnoType())
				//|| TeamExtData::GroupAllowed(pMemberNeeded, TechnoExtContainer::Instance.Find(pMember)->Type)

				)) {

				pVec->Remove<true>(pMemberNeeded);
				break;
			}
		}
	}

	return 0x6EF5B2;
}
//#pragma optimize("", off )

DEFINE_HOOK(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if (pThis->ProducingInfantryTypeIndex < 0) {

		const int result = GetTypeToProduceNew<InfantryClass , InfantryTypeClass>(pThis);
		if (result >= 0)
			pThis->ProducingInfantryTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF204;
}

DEFINE_HOOK(0x4FF210, HouseClass_AI_AircraftProduction, 6)
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