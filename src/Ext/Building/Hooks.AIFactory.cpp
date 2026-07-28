#include "Body.h"

#include <Ext/House/Body.h>

#include <TeamTypeClass.h>
#include <InfantryClass.h>
#include <AircraftClass.h>

std::tuple<BuildingClass**, bool, AbstractType> GetFactory(AbstractType AbsType, bool naval, HouseExtData* pData)
{
	auto pRules = FakeRulesClass::Instance();
	auto pThis = pData->This();
	BuildingClass** currFactory = nullptr;
	bool block = !pRules->AllowParallelAIQueues;
	// Only raises `block`; never clears it (matches original !block short-circuit).
	// Templated so each *TypeClass::Array element type resolves at the call site.
	auto applyItemBlock = [&](auto* pArray, int idx) {
			if (idx >= 0) {
				block |= TechnoTypeExtContainer::Instance.Find(pArray->Items[idx])->ForbidParallelAIQueues;
			}
		};

	switch (AbsType)
	{
	case AbstractType::BuildingType:
	{
		currFactory = &pData->Factory_BuildingType;
		applyItemBlock(BuildingTypeClass::Array(), pThis->ProducingBuildingTypeIndex);
		block = pRules->ForbidParallelAIQueues_Building.Get(block);
		break;
	}
	case AbstractType::UnitType:
	{
		if (naval)
		{
			currFactory = &pData->Factory_NavyType;
			// VERIFY: naval reads the index off the EXT (pData), not off This() — preserved verbatim.
			applyItemBlock(UnitTypeClass::Array(), pData->ProducingNavalUnitTypeIndex);
			block = pRules->ForbidParallelAIQueues_Vehicle.Get(block);
		}
		else
		{
			currFactory = &pData->Factory_VehicleType;
			applyItemBlock(UnitTypeClass::Array(), pThis->ProducingUnitTypeIndex);
			block = pRules->ForbidParallelAIQueues_Navy.Get(block);
		}

		break;
	}
	case AbstractType::InfantryType:
	{
		currFactory = &pData->Factory_InfantryType;
		applyItemBlock(InfantryTypeClass::Array(), pThis->ProducingInfantryTypeIndex);
		block = pRules->ForbidParallelAIQueues_Infantry.Get(block);
		break;
	}
	case AbstractType::AircraftType:
	{
		currFactory = &pData->Factory_AircraftType;
		applyItemBlock(AircraftTypeClass::Array(), pThis->ProducingAircraftTypeIndex);
		block = pRules->ForbidParallelAIQueues_Aircraft.Get(block);
		break;
	}
	default:
		break;
	}

	return { currFactory, block, AbsType };
}

ASMJIT_PATCH(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->IsUnderEMP())
		return 0x4401D2;

	auto const pOwner = pBuilding->Owner;
	const int index = pOwner->ProducingAircraftTypeIndex;
	auto const pType = AircraftTypeClass::Array->get_or_default(index);
	const auto pRules = FakeRulesClass::Instance();
	bool forbid = !pRules->AllowParallelAIQueues;

	if(pType){
		forbid |= TechnoTypeExtContainer::Instance.Find(pType)->ForbidParallelAIQueues;
	}

	if(!pRules->ForbidParallelAIQueues_Aircraft.Get(forbid))
		return 0x0;

	if (pOwner->Type->MultiplayPassive
		|| pOwner->IsCurrentPlayer()
		|| pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType) {
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

	HouseExtData* pData = HouseExtContainer::Instance.Find(pOwner);
	const auto& [curFactory, block, type] = GetFactory(pThis->Type->Factory, pThis->Type->Naval, pData);

	if (!curFactory)
	{
		_com_issue_error(E_POINTER); // throws; never returns normally
	}
	else if (!*curFactory)
	{
		// claim the empty factory slot
		if (type != AircraftTypeClass::AbsID) {
			pThis->IsPrimaryFactory = true; // was guarded by `if (!IsPrimaryFactory)` — redundant, plain bool set
		}

		*curFactory = pThis; // last check
		return 0x0;
	}
	else if (*curFactory != pThis)
	{
		// A different building already owns this factory slot.
		// GetFactory already folded the currently-producing item's ForbidParallelAIQueues
		// into `block` using the SAME indices (aircraft/infantry/building + naval-aware unit),
		// so the original per-type switch here was 100% redundant with `block`.
		// Both original exit conditions (item-forbids `Skip`, and `block ? Skip`) collapse to this.
		return block ? Skip : 0x0;
	}

	return 0x0;
}
