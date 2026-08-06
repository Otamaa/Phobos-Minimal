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

	// Only raises `block`; never clears it (matches original !block short-circuit).
	// Templated so each *TypeClass::Array element type resolves at the call site.
	auto applyItemBlock = [pRules](auto* pArray, int idx) {
		if (idx >= 0 && TechnoTypeExtContainer::Instance.Find(pArray->Items[idx])->ForbidParallelAIQueues) {
			return true;
		}

		return !pRules->AllowParallelAIQueues.Get();
	};

	switch (AbsType)
	{
	case AbstractType::BuildingType: {
		currFactory = &pData->Factory_BuildingType;	;
		const bool block = pRules->ForbidParallelAIQueues_Building.Get(applyItemBlock(BuildingTypeClass::Array(), pThis->ProducingBuildingTypeIndex));
		return { currFactory, block, AbsType };
	}
	case AbstractType::UnitType: {
		if (naval) {
			currFactory = &pData->Factory_NavyType;
			const bool block = pRules->ForbidParallelAIQueues_Navy.Get(applyItemBlock(UnitTypeClass::Array(), pData->ProducingNavalUnitTypeIndex));
			return { currFactory, block, AbsType };
		} else {
			currFactory = &pData->Factory_VehicleType;
			const bool  block = pRules->ForbidParallelAIQueues_Vehicle.Get(applyItemBlock(UnitTypeClass::Array(), pThis->ProducingUnitTypeIndex));
			return { currFactory, block, AbsType };
		}

	}
	case AbstractType::InfantryType: {
		currFactory = &pData->Factory_InfantryType;
		const bool block = pRules->ForbidParallelAIQueues_Infantry.Get(applyItemBlock(InfantryTypeClass::Array(), pThis->ProducingInfantryTypeIndex));
		return { currFactory, block, AbsType };
	}
	case AbstractType::AircraftType: {
		currFactory = &pData->Factory_AircraftType;
		const bool block = pRules->ForbidParallelAIQueues_Aircraft.Get(applyItemBlock(AircraftTypeClass::Array(), pThis->ProducingAircraftTypeIndex));
		return { currFactory, block, AbsType };
	}
	default:
		return { currFactory, false, AbsType };
	}

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

	if(pType && TechnoTypeExtContainer::Instance.Find(pType)->ForbidParallelAIQueues){
		forbid = true;
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
		if(block)
		return Skip;
	}

	return 0x0;
}