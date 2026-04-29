#include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>

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
