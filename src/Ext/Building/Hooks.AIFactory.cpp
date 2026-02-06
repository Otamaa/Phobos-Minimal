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
		if (pData->This()->ProducingBuildingTypeIndex >= 0)
		{
			block = TechnoTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items
				[pData->This()->ProducingBuildingTypeIndex])->ForbidParallelAIQueues.Get(block);
		}
		break;
	}
	case AbstractType::UnitType:
	{
		if (!naval)
		{
			block = pRules->ForbidParallelAIQueues_Vehicle.Get(!pRules->AllowParallelAIQueues);
			if (pData->This()->ProducingUnitTypeIndex >= 0)
			{
				block = TechnoTypeExtContainer::Instance.Find(UnitTypeClass::Array->Items
				[pData->This()->ProducingUnitTypeIndex])->ForbidParallelAIQueues.Get(block);
			}
			currFactory = &pData->Factory_VehicleType;
		}
		else
		{
			block = pRules->ForbidParallelAIQueues_Navy.Get(!pRules->AllowParallelAIQueues);
			if (pData->ProducingNavalUnitTypeIndex >= 0)
			{
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
		if (pData->This()->ProducingInfantryTypeIndex >= 0)
		{
			block = TechnoTypeExtContainer::Instance.Find(InfantryTypeClass::Array->Items
			[pData->This()->ProducingInfantryTypeIndex])->ForbidParallelAIQueues.Get(block);
		}
		currFactory = &pData->Factory_InfantryType;
		break;
	}
	case AbstractType::AircraftType:
	{
		currFactory = &pData->Factory_AircraftType;
		block = pRules->ForbidParallelAIQueues_Aircraft.Get(!pRules->AllowParallelAIQueues);
		if (pData->This()->ProducingAircraftTypeIndex >= 0)
		{
			block = TechnoTypeExtContainer::Instance.Find(AircraftTypeClass::Array->Items
				[pData->This()->ProducingAircraftTypeIndex])->ForbidParallelAIQueues.Get(block);
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
			if (!((UnitClass*)pFactory->Object)->Type->Naval)
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

	//	auto pRules = RulesExtData::Instance();
	HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);
	const auto& [curFactory, block, type] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pData);

	if (!curFactory)
	{
		_com_issue_error(E_POINTER);
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
		// block is already correctly computed by GetFactory using ForbidParallelAIQueues.Get(default)
		return block ? Skip : 0x0;
	}

	return 0x0;
}

ASMJIT_PATCH(0x4FEA60, HouseClass_AI_UnitProduction, 0x6)
{
	GET(FakeHouseClass* const, pThis, ECX);

	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 15);
	pThis->_GetExtData()->GetUnitTypeToProduce();
	return ret();
}

ASMJIT_PATCH(0x4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(FakeHouseClass*, pThis, ECX);

	if (pThis->ProducingInfantryTypeIndex < 0) {
		const int result = pThis->_GetExtData()->GetInfantryTypeToProduce();
		if (result >= 0)
			pThis->ProducingInfantryTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF204;
}

ASMJIT_PATCH(0x4FF210, HouseClass_AI_AircraftProduction, 6)
{
	GET(FakeHouseClass*, pThis, ECX);

	if (pThis->ProducingAircraftTypeIndex < 0) {

		const int result = pThis->_GetExtData()->GetAircraftTypeToProduce();
		if (result >= 0)
			pThis->ProducingAircraftTypeIndex = result;
	}

	R->EAX(15);
	return 0x4FF534;
}