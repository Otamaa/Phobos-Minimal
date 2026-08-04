#include "Body.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/Macro.h>

#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Super/Body.h>

#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

#include <New/Entity/ShieldClass.h>
#include <New/PhobosAttachedAffect/PhobosAttachEffectClass.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include <New/TextBox/Types/TextBoxTypeClass.h>
#include <New/TextBox/Entities/Derived/TechnoTextBoxClass.h>
#include <New/TextBox/Entities/Derived/WaypointTextBoxClass.h>

#include <New/ChoiceBox/Types/ChoiceBoxTypeClass.h>
#include <New/ChoiceBox/Entities/Derived/ScreenChoiceBoxClass.h>
#include <New/ChoiceBox/Entities/Derived/WaypointChoiceBoxClass.h>

#include <TeamTypeClass.h>

// =============================
// load / save

template <typename T>
void TEventExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TechnoType)
		;
}

// helper struct
namespace std
{
	template <class _Ty = void>
	struct and_with
	{
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty _FIRST_ARGUMENT_TYPE_NAME;
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty _SECOND_ARGUMENT_TYPE_NAME;
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool _RESULT_TYPE_NAME;

		_NODISCARD COMPILETIMEEVAL bool operator()(const _Ty& _Left, const _Ty& _Right) const
		{
			return _Left & _Right;
		}
	};
	//struct and_with { bool operator()(int a, int b) { return a & b; } };
}

NOINLINE HouseClass* TEventExtData::ResolveHouseParam(int const param, HouseClass* const pOwnerHouse)
{
	if (param == -1)
		return nullptr;

	if (param == 8997)
	{
		return pOwnerHouse;
	}

	if (HouseClass::Index_IsMP(param))
	{
		return HouseClass::FindByIndex(param);
	}

	return HouseClass::FindByCountryIndex(param);
}

std::pair<bool, bool> TEventExtData::GetPersistableFlag(PhobosTriggerEvent nAction)
{
	return { true , true };
}

std::pair<bool, bool> TEventExtData::GetPersistableFlag(AresTriggerEvents nAction)
{
	switch (nAction)
	{
	case AresTriggerEvents::UnderEMP:
	case AresTriggerEvents::UnderEMP_ByHouse:
	case AresTriggerEvents::RemoveEMP:
	case AresTriggerEvents::RemoveEMP_ByHouse:
	case AresTriggerEvents::EnemyInSpotlightNow:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::AttackedOrDestroyedByAnybody:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		return { false , true };
	case AresTriggerEvents::DriverKiller:
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::SuperActivated:
	case AresTriggerEvents::SuperDeactivated:
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineerAnything:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		return { true  , true };
	default:
		return { false  , false };
	}
}

std::pair<LogicNeedType, bool> TEventExtData::GetLogicNeed(PhobosTriggerEvent nAction)
{
	switch (nAction)
	{
	case PhobosTriggerEvent::LocalVariableGreaterThan:
	case PhobosTriggerEvent::LocalVariableLessThan:
	case PhobosTriggerEvent::LocalVariableEqualsTo:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::GlobalVariableGreaterThan:
	case PhobosTriggerEvent::GlobalVariableLessThan:
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::TechnoTypeOfHouseNearWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseAllLeavesWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseExistsAtWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseNotExistsAtWaypoint:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::ElapsedTimeFrames:
	case PhobosTriggerEvent::MissionTimerGreater:
	case PhobosTriggerEvent::MissionTimerLess:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::ChoiceBoxButtonClicked:
	case PhobosTriggerEvent::ChoiceBoxAnyButtonClicked:
	case PhobosTriggerEvent::ChoiceBoxTimedOut:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::HousePowerOutputMuch:
	case PhobosTriggerEvent::HousePowerOutputLess:
	case PhobosTriggerEvent::HousePowerDrainMuch:
	case PhobosTriggerEvent::HousePowerDrainLess:
	case PhobosTriggerEvent::HousePowerSurplusMuch:
	case PhobosTriggerEvent::HousePowerSurplusLess:
		return { LogicNeedType::NumberNTech , true };

	case PhobosTriggerEvent::HouseOwnsTechnoType:
	case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
	case PhobosTriggerEvent::HousesDestroyed:
		return { LogicNeedType::House , true };

	case PhobosTriggerEvent::CellHasTechnoType:
	case PhobosTriggerEvent::CellHasAnyTechnoTypeFromList:
		return { LogicNeedType::Cell , true };

	case PhobosTriggerEvent::AttachedIsUnderAttachedEffect:
		return { LogicNeedType::None , true };

	case PhobosTriggerEvent::ShieldBroken:
		return { LogicNeedType::None , true };

	default:
		return { LogicNeedType::None , false };
	}
}

std::pair<LogicNeedType, bool > TEventExtData::GetLogicNeed(AresTriggerEvents nAction)
{
	switch (nAction)
	{
	case AresTriggerEvents::UnderEMP://
	case AresTriggerEvents::RemoveEMP: //
	case AresTriggerEvents::EnemyInSpotlightNow://
	case AresTriggerEvents::DriverKiller://
	case AresTriggerEvents::VehicleTaken://
	case AresTriggerEvents::Abducted://
	case AresTriggerEvents::AbductSomething://
	case AresTriggerEvents::SuperActivated://
	case AresTriggerEvents::SuperDeactivated://
	case AresTriggerEvents::ReverseEngineerAnything://
	case AresTriggerEvents::AttackedOrDestroyedByAnybody://
		return { LogicNeedType::None  , true };
	case AresTriggerEvents::UnderEMP_ByHouse://
	case AresTriggerEvents::RemoveEMP_ByHouse://
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		return { LogicNeedType::House  , true };
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		return { LogicNeedType::NumberNTech  , true };
	default:
		return { LogicNeedType::None  , false };
	}
}

std::pair<TriggerAttachType, bool> TEventExtData::GetTriggetAttach(PhobosTriggerEvent nAction)
{
	switch (nAction)
	{
	case PhobosTriggerEvent::LocalVariableGreaterThan:
	case PhobosTriggerEvent::LocalVariableLessThan:
	case PhobosTriggerEvent::LocalVariableEqualsTo:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
	case PhobosTriggerEvent::GlobalVariableLessThan:
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		return { TriggerAttachType::Logic , true };

	case PhobosTriggerEvent::TechnoTypeOfHouseNearWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseAllLeavesWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseExistsAtWaypoint:
	case PhobosTriggerEvent::TechnoTypeOfHouseNotExistsAtWaypoint:
		return { TriggerAttachType::Logic , true };

	case PhobosTriggerEvent::ElapsedTimeFrames:
	case PhobosTriggerEvent::MissionTimerGreater:
	case PhobosTriggerEvent::MissionTimerLess:
		return { TriggerAttachType::Logic , true };

	case PhobosTriggerEvent::ChoiceBoxButtonClicked:
	case PhobosTriggerEvent::ChoiceBoxAnyButtonClicked:
	case PhobosTriggerEvent::ChoiceBoxTimedOut:
		return { TriggerAttachType::Logic , true };

	case PhobosTriggerEvent::HousePowerOutputMuch:
	case PhobosTriggerEvent::HousePowerOutputLess:
	case PhobosTriggerEvent::HousePowerDrainMuch:
	case PhobosTriggerEvent::HousePowerDrainLess:
	case PhobosTriggerEvent::HousePowerSurplusMuch:
	case PhobosTriggerEvent::HousePowerSurplusLess:
		return { TriggerAttachType::Logic , true };

	case PhobosTriggerEvent::HouseOwnsTechnoType:
	case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
	case PhobosTriggerEvent::HousesDestroyed:
		return { TriggerAttachType::House , true };
	case PhobosTriggerEvent::CellHasTechnoType:
	case PhobosTriggerEvent::CellHasAnyTechnoTypeFromList:
	case PhobosTriggerEvent::DestroyedOnly:
	case PhobosTriggerEvent::AttachedIsUnderAttachedEffect:
		return { TriggerAttachType::Object , true };
	case PhobosTriggerEvent::ShieldBroken:
		return { TriggerAttachType::None , true };
	default:
		return { TriggerAttachType::None , false };
	}
}

std::pair<TriggerAttachType, bool> TEventExtData::GetAttachFlags(AresTriggerEvents nEvent)
{
	switch (nEvent)
	{
	case AresTriggerEvents::UnderEMP:
	case AresTriggerEvents::UnderEMP_ByHouse:
	case AresTriggerEvents::RemoveEMP:
	case AresTriggerEvents::RemoveEMP_ByHouse:
	case AresTriggerEvents::EnemyInSpotlightNow:
	case AresTriggerEvents::DriverKiller:
	case AresTriggerEvents::DriverKilled_ByHouse:
	case AresTriggerEvents::VehicleTaken:
	case AresTriggerEvents::VehicleTaken_ByHouse:
	case AresTriggerEvents::Abducted:
	case AresTriggerEvents::Abducted_ByHouse:
	case AresTriggerEvents::AbductSomething:
	case AresTriggerEvents::AbductSomething_OfHouse:
	case AresTriggerEvents::ReverseEngineerAnything:
	case AresTriggerEvents::ReverseEngineerType:
	case AresTriggerEvents::AttackedOrDestroyedByAnybody:
	case AresTriggerEvents::AttackedOrDestroyedByHouse:
	{
		return { TriggerAttachType::Object , true };
	}
	case AresTriggerEvents::SuperActivated:
	case AresTriggerEvents::SuperDeactivated:
	case AresTriggerEvents::SuperNearWaypoint:
	case AresTriggerEvents::ReverseEngineered:
	case AresTriggerEvents::HouseOwnTechnoType:
	case AresTriggerEvents::HouseDoesntOwnTechnoType:
	case AresTriggerEvents::DestroyedByHouse:
	case AresTriggerEvents::AllKeepAlivesDestroyed:
	case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
	{
		return { TriggerAttachType::House , true };
	}
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
	{
		return { TriggerAttachType::Logic , true };
	}
	}

	return { TriggerAttachType::None , false };
}

bool TEventExtData::FindTechnoType(TEventClass* pThis, int args, HouseClass* pWho)
{
	const auto pType = TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
	if (!pType)
		return false;

	if (args <= 0)
		return true;

	if (!pType->Insignificant && !pType->DontScore)
	{
		HouseClass** const arr = pWho ? &pWho : HouseClass::Array->Items;
		HouseClass** const nEnd = arr + (pWho ? 1 : HouseClass::Array->Count);

		int i = args;

		for (HouseClass** nPos = arr; nPos != nEnd; ++nPos)
		{
			i -= (*nPos)->CountOwnedNow(pType);

			if (i <= 0)
			{
				return true;
			}
		}
	}
	else
	{
		int i = args;
		TechnoClass** arrayItems = nullptr;
		int arrayCount = 0;

		switch (pType->WhatAmI())
		{
		case AbstractType::AircraftType:
		{
			arrayItems = (TechnoClass**)AircraftClass::Array->Items;
			arrayCount = AircraftClass::Array->Count;
			break;
		}
		case AbstractType::UnitType:
		{
			arrayItems = (TechnoClass**)UnitClass::Array->Items;
			arrayCount = UnitClass::Array->Count;
			break;
		}
		case AbstractType::InfantryType:
		{
			arrayItems = (TechnoClass**)InfantryClass::Array->Items;
			arrayCount = InfantryClass::Array->Count;
			break;
		}
		case AbstractType::BuildingType:
		{
			arrayItems = (TechnoClass**)BuildingClass::Array->Items;
			arrayCount = BuildingClass::Array->Count;
			break;
		}
		default:
			break;
		}

		if (arrayCount > 0 && arrayItems)
		{
			const auto arrayItemsEnd = arrayItems + arrayCount;
			for (auto walk = arrayItems; walk != arrayItemsEnd; ++walk)
			{
				if (pWho && pWho != (*walk)->Owner)
					continue;

				if (GET_TECHNOTYPE((*walk)) == pType)
				{
					i--;

					if (i <= 0)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

// Gets the TechnoType pointed to by the event's TechnoName field.
/*!
	Resolves the TechnoName to a TechnoTypeClass and caches it. This function
	is an O(n) operation for the first call, every subsequent call is O(1).

	\returns The TechnoTypeClass TechnoName points to, nullptr if not set or invalid.

	\date 2012-05-09, 2013-02-09
*/
TechnoTypeClass* TEventExtData::GetTechnoType()
{
	if (this->TechnoType.empty())
	{
		const char* eventTechno = This()->String;
		TechnoTypeClass* pType = TechnoTypeClass::Find(eventTechno);

		if (!pType)
		{
			Debug::LogInfo("Event{}] with Team[{} - {}] references non-existing techno type \"%s\".",
				(void*)This(),
				This()->TeamType ? This()->TeamType->ID : GameStrings::NoneStr(),
				(void*)This()->TeamType,
				eventTechno
			);
		}

		this->TechnoType = pType;
	}

	return this->TechnoType;
}

bool TEventExtData::AttachedIsUnderAttachedEffectTEvent(TEventClass* pThis, ObjectClass* pObject)
{
	if (!pObject)
		return false;

	const auto pTypeAttached = PhobosAttachEffectTypeClass::Find(pThis->String);

	if (!pTypeAttached)
	{
		Debug::Log("Error in event %d. The parameter 2 '%s' isn't a valid AttachEffect ID\n", static_cast<PhobosTriggerEvent>(pThis->EventKind), pThis->String);
		return false;
	}

	auto const pTechno = flag_cast_to<TechnoClass* , false>(pObject);

	if (!pTechno)
		return false;

	std::vector<PhobosAttachEffectTypeClass*> attach { pTypeAttached };

	if (PhobosAEFunctions::HasAttachedEffects(pTechno, attach, false, false, nullptr, nullptr, nullptr, nullptr))
		return true;

	return false;
}

HouseClass* TEventExtData::GetHouse(int TEvetValue, HouseClass* pEventHouse)
{
	if (TEvetValue <= -2)
		return pEventHouse;
	else if (TEvetValue >= 0)
		return HouseClass::Index_IsMP(TEvetValue) ? HouseClass::FindByIndex(TEvetValue) : HouseClass::FindByCountryIndex(TEvetValue);

	return nullptr;
}

bool TEventExtData::CellHasAnyTechnoTypeFromListTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pEventHouse)
{

	if (!pObject)
		return false;

	int desiredListIdx = -1;
	if (sscanf_s(pThis->String, "%d", &desiredListIdx) <= 0 || desiredListIdx < 0) {
		Debug::LogInfo("Error in event {}. The parameter 2 '{}' isn't a valid index value for [AITargetTypes]",
			static_cast<int>(pThis->EventKind),
			pThis->String
		);

		return false;
	}

	if (FakeRulesClass::Instance()->AITargetTypesLists.empty()
		|| size_t(desiredListIdx) >= FakeRulesClass::Instance()->AITargetTypesLists.size()
		|| FakeRulesClass::Instance()->AITargetTypesLists[desiredListIdx].empty())
		return false;

	bool found = false;

	if (auto const pTechno = flag_cast_to<TechnoClass*, false>(pObject)) {
		auto const pTechnoType = GET_TECHNOTYPE(pTechno);

		for (const auto& pDesiredItem : FakeRulesClass::Instance()->AITargetTypesLists[desiredListIdx]) {
			if (pDesiredItem == pTechnoType) {
				HouseClass* pHouse = GetHouse(pThis->Value, pEventHouse);

				if (pHouse && pTechno->Owner != pHouse)
					break;

				found = true;
				break;
			}
		}
	}

	return found;
}

bool TEventExtData::CellHasTechnoTypeTEvent(TEventClass* pThis, ObjectClass* pObject, HouseClass* pEventHouse)
{
	if (pObject) {

		const auto pTypeAttached = TechnoTypeClass::Find(pThis->String);

		if (!pTypeAttached) {
			Debug::LogInfo("Error in event {}. The parameter 2 '{}' isn't a valid Techno ID",
				static_cast<int>(pThis->EventKind),
				pThis->String
			);
			return false;
		}

		if (auto const pTechno = flag_cast_to<TechnoClass*, false>(pObject)) {
			auto const pTechnoType = GET_TECHNOTYPE(pTechno);

			if (pTypeAttached == pTechnoType) {
				if (HouseClass* pHouse = GetHouse(pThis->Value, pEventHouse)) {
					return pTechno->Owner == pHouse;
				}

				return true;
			}
		}
	}

	return false;
}

template<bool IsGlobal, class _Pr>
bool TEventExtData::VariableCheck(TEventClass* pThis)
{
	const auto nVar = ScenarioExtData::GetVariables(IsGlobal);

	if (auto itr = nVar->tryfind(pThis->Value)) {
		// We uses TechnoName for our operator number
		int nOpt = atoi(pThis->String);
		return _Pr()(itr->Value, nOpt);
	}

	return false;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
bool TEventExtData::VariableCheckBinary(TEventClass* pThis)
{
	const auto nVar = ScenarioExtData::GetVariables(IsGlobal);

	if (const auto itr = nVar->tryfind(pThis->Value))
	{
		// We uses TechnoName for our src variable index
		if (auto itrsrc = nVar->tryfind(atoi(pThis->String)))
			return _Pr()(itr->Value, itrsrc->Value);
	}

	return false;
}

bool TEventExtData::HouseOwnsTechnoTypeTEvent(TEventClass* pThis)
{
	auto pType = TechnoTypeClass::Find(pThis->String);
	if (!pType)
		return false;

	auto pHouse = HouseClass::Index_IsMP(pThis->Value)
	? HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value) ;

	if (!pHouse)
		return false;

	if (pType->WhatAmI() == AbstractType::BuildingType)
	{
		for (auto pBuilding : pHouse->Buildings) {
			if (pBuilding->Type != pType)
				continue;

			if (pBuilding->IsAlive && pBuilding->Health > 0 && !pBuilding->InLimbo)
				return true;
		}

		return false;
	} else {
		return pHouse->CountOwnedNow(pType) > 0;
	}
}

bool TEventExtData::HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis)
{
	return !TEventExtData::HouseOwnsTechnoTypeTEvent(pThis);
}

bool TEventExtData::HousesAreDestroyedTEvent(TEventClass* pThis)
{
	const int nIdxVariable = pThis->Value; //atoi(pThis->String);

	const auto& nHouseList = FakeRulesClass::Instance()->AIHousesLists;

	if ((size_t)nIdxVariable >= nHouseList.size())
	{
		Debug::LogInfo("Map event {}: [AIHousesList] is empty. This event can't continue.", (int)pThis->EventKind);
		return false;
	}

	const auto housesList = Iterator(nHouseList[nIdxVariable]);

	if (housesList.empty())
	{
		Debug::LogInfo("Map event {}: [AIHousesList]({}) is empty. This event can't continue.", (int)pThis->EventKind, nIdxVariable);
		return false;
	}

	for (auto pTechno : *TechnoClass::Array)
	{
		if (ScriptExtData::IsUnitAvailable(pTechno, false))
		{
			if (pTechno->Owner && housesList.contains(pTechno->Owner->Type))
				return false;
		}
	}

	return true;
}

int FindTechnoTypeByName(const char* name)
{
	for (int i = TechnoTypeClass::Array->Count - 1; i >= 0; --i) {
		if (strcmp(TechnoTypeClass::Array->Items[i]->Name, name) == 0) {
			return i;
		}
	}

	return -1;
}

bool CheckTechTypeExists(TEventClass* evt, bool shouldExist)
{
	int typeIndex = FindTechnoTypeByName(evt->String);
	int typeCount = TechnoTypeClass::Array->Count;

	if (typeIndex < 0 || typeIndex >= typeCount) {
		return false;
	}

	TechnoTypeClass* targetType = TechnoTypeClass::Array->Items[typeIndex];
	int foundCount = 0;
	int technoCount = TechnoClass::Array->Count;

	for (int i = technoCount - 1; i >= 0; --i) {
		TechnoClass* techno = TechnoClass::Array->Items[i];
		if (GET_TECHNOTYPE(techno) == targetType) {
			foundCount++;
			if (shouldExist && foundCount >= evt->Value) {
				return true;
			}
		}
	}

	return shouldExist ? false : (foundCount == 0);
}

bool TEventExtData::TechnoTypeOfHouseNearWaypoint(TEventClass* pThis, HouseClass* pHouse)
{
	int range = pThis->Value;
	int wayPointIndex = std::stoi(pThis->String);

	CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(wayPointIndex);

	for (auto pTechno : *TechnoClass::Array) {
		if (pTechno && pTechno->Owner == pHouse) {
			if (GeneralUtils::IsTechnoNearCell(pTechno, cell, range)) {
				return true;
			}
		}
	}

	return false;
}

bool TEventExtData::TechnoTypeOfHouseExistsAtWaypoint(TEventClass* pThis, HouseClass* pHouse)
{
	int wayPointIndex = pThis->Value;
	const char* technoID = pThis->String;

	CellStruct cell = ScenarioClass::Instance->GetWaypointCoords(wayPointIndex);

	for (TechnoClass* pTechno : *TechnoClass::Array) {

		if (pTechno
			&& pTechno->Owner == pHouse
			&& strcmp(pTechno->get_ID(), technoID) == 0)
		{
			if (pTechno->WhatAmI() == AbstractType::Building) {

				if (BuildingClass* pBuilding = cast_to<BuildingClass*>(pTechno)) {
					if (GeneralUtils::IsCellInBuildingFoundation(pBuilding, cell)) {
						return true;
					}
				}
			}
			else {
				if (CellClass::Coord2Cell(pTechno->GetCoords()) == cell) {
					return true;
				}
			}
		}
	}

	return false;
}

bool TEventExtData::ElapsedTimeFramesFunc(TEventClass* pThis)
{
	static std::map<const TEventClass*, int> StartFrames;

	int waitFrames = pThis->Value;

	auto it = StartFrames.find(pThis);
	if (it == StartFrames.end()) {
		StartFrames[pThis] = Unsorted::CurrentFrame();
	}

	int startFrame = StartFrames[pThis];
	int elapsed = Unsorted::CurrentFrame() - startFrame;
	bool result = elapsed >= waitFrames;

	return result;
}

bool TEventExtData::MissionTimerGreaterFunc(TEventClass* pThis)
{
	auto const pTimer = &ScenarioClass::Instance->MissionTimer;
	int thresholdFrames = pThis->Value * 15;

	return pTimer->GetTimeLeft() > thresholdFrames;
}

bool TEventExtData::MissionTimerLessFunc(TEventClass* pThis)
{
	auto const pTimer = &ScenarioClass::Instance->MissionTimer;
	int thresholdFrames = pThis->Value * 15;

	return pTimer->GetTimeLeft() < thresholdFrames;
}

bool TEventExtData::ChoiceBoxButtonClickedFunc(TEventClass* pThis, HouseClass* pHouse)
{
	int targetID = std::atoi(pThis->String);
	int targetButtonIndex = pThis->Value - 1;

	auto* pBox = MapChoiceBoxClass::FindByID(targetID);
	if (!pBox || pBox->ClickedConsumed)
		return false;

	if (pBox->ClickedIndex == targetButtonIndex)
	{
		pBox->ClickedConsumed = true;
		return true;
	}
	return false;
}

bool TEventExtData::ChoiceBoxAnyButtonClickedFunc(TEventClass* pThis, HouseClass* pHouse)
{
	int targetID = pThis->Value;;

	auto* pBox = MapChoiceBoxClass::FindByID(targetID);
	if (!pBox || pBox->ClickedConsumed)
		return false;

	if (pBox->ClickedIndex >= 0)
	{
		pBox->ClickedConsumed = true;
		return true;
	}
	return false;
}

bool TEventExtData::ChoiceBoxTimedOutFunc(TEventClass* pThis, HouseClass* pHouse)
{
	int targetID = pThis->Value;

	auto* pBox = MapChoiceBoxClass::FindByID(targetID);
	if (!pBox)
		return false;

	return pBox->IsExpired;
}

bool TEventExtData::PowerHander(TEventClass* pThis, HouseClass* pHouse, PowerEventMode mode, bool isMuch)
{
	if (!pHouse) return false;
	if (Unsorted::CurrentFrame() == 0) return false;

	int val = pThis->Value;
	int target = 0;

	switch (mode)
	{
	case PowerEventMode::Output:
		target = pHouse->PowerOutput;
		break;
	case PowerEventMode::Drain:
		target = pHouse->PowerDrain;
		break;
	case PowerEventMode::Surplus:
		target = pHouse->PowerOutput - pHouse->PowerDrain;
		break;
	default:
		break;
	}

	return isMuch ? (val < target) : (val > target);
}

bool TEventExtData::VanillaTriggerEventOccured(TEventClass* pThis, EventArgs& Args, bool& result)
{
	// --- shared helpers --------------------------------------------------------------

  // ORIG: Debug_Map_DEBUGDEBUG @ 0xA8ED6B
	auto guarded = [&]() -> bool
		{
			return Args.RequestedEventType == pThis->EventKind && !Unsorted::MAP_DEBUG_MODE();
		};

	auto tgt = [&]() -> HouseClass*
		{
			return TEventExtData::ResolveHouseParam(pThis->Value);
		};

	// ORIG: Frame @ 0xA8ED84 -> Unsorted::CurrentFrame
	auto timer_expired = [](int started, int delay) -> bool
		{
			if (started == -1)
				return delay == 0;
			return (Unsorted::CurrentFrame() - started) >= delay;
		};

	switch (pThis->EventKind)
	{
	case TriggerEvent::AnyEvent:                    // 0x08  (unconditional -- not guarded)
		return true;

		// ---------------------------------------------------------------------------------
		//  timers
		// ---------------------------------------------------------------------------------
	case TriggerEvent::ElapsedTime:                 // 0x0D
	case TriggerEvent::RandomDelay:                 // 0x33
	{
		result = timer_expired(Args.ActivationFrame->StartTime, Args.ActivationFrame->TimeLeft);
		break;
	}
	case TriggerEvent::MissionTimerExpired:         // 0x0E
	{
		auto& mt = ScenarioClass::Instance->MissionTimer;

		if (mt.StartTime == -1)
			result =  false;
		else 
			result = (Unsorted::CurrentFrame() - mt.StartTime) >= mt.TimeLeft;

		break;
	}

		// ---------------------------------------------------------------------------------
		//  scenario global / local state
		// ---------------------------------------------------------------------------------
	case TriggerEvent::GlobalSet:
	{                 // 0x1B
		ScenarioClass::Instance->GetGlobalVarValue_ptr(pThis->Value, Args.isRepeating);
		result = *Args.isRepeating;
		break;
	}
	case TriggerEvent::GlobalCleared:
	{             // 0x1C
		ScenarioClass::Instance->GetGlobalVarValue_ptr(pThis->Value, Args.isRepeating);
		result = !*Args.isRepeating;
		break;
	}
	case TriggerEvent::LocalSet:
	{                   // 0x24
		ScenarioClass::Instance->GetLocalVarValue_ptr(pThis->Value, Args.isRepeating);
		result = *Args.isRepeating;
		break;
	}
	case TriggerEvent::LocalCleared:
	{               // 0x25
		ScenarioClass::Instance->GetLocalVarValue_ptr(pThis->Value, Args.isRepeating);
		result = !*Args.isRepeating;
		break;
	}

	// ---------------------------------------------------------------------------------
	//  ambient light / scenario time  (Scen +0x352C = AmbientCurrent)
	// ---------------------------------------------------------------------------------
	case TriggerEvent::AmbientLightBelow:           // 0x2D
	{
		result = ScenarioClass::Instance->AmbientCurrent <= pThis->Value;
		break;
	}
	case TriggerEvent::AmbientLightAbove:           // 0x2E
	{
		result = ScenarioClass::Instance->AmbientCurrent >= pThis->Value;
		break;
	}
	case TriggerEvent::ElapsedScenarioTime:         // 0x2F  (imul 0x88888889 == signed /15)
	{
		result = pThis->Value <= Unsorted::CurrentFrame() / 15;
		break;
	}

		// ---------------------------------------------------------------------------------
		//  techtype existence (count TechnoClass::Array by IniName)
		// ---------------------------------------------------------------------------------
	case TriggerEvent::TechTypeExists:
	{            // 0x3C
		if (!TEventExtContainer::Instance.Find(pThis)->GetTechnoType()
			&& !TEventExtData::FindTechnoType(pThis, 1, nullptr)
			)
			result = false;

		return true;
	}
	case TriggerEvent::TechTypeDoesntExist:
	{       // 0x3D

		if (TEventExtContainer::Instance.Find(pThis)->GetTechnoType()
			|| TEventExtData::FindTechnoType(pThis, 1, nullptr)
			)
			result = false;

		return true;
	}

	// ---------------------------------------------------------------------------------
	//  location / spy / waypoint  (all guarded, self-contained)
	// ---------------------------------------------------------------------------------
	case TriggerEvent::EnteredBy:                   // 0x01
	case TriggerEvent::ZoneEntryBy:                 // 0x18
	case TriggerEvent::CrossesHorizontalLine:       // 0x19
	case TriggerEvent::CrossesVerticalLine:         // 0x1A
	case TriggerEvent::EnteredOrOverflownBy:        // 0x3B
	{
		result = false;
		if (guarded() && Args.Object) {
			if (pThis->Value != -1) {
				auto country = TEventExtData::ResolveHouseParam(pThis->Value);

				if (!country) {
					result = false;
					return true;
				}

				if (Args.Object->GetOwningHouseIndex() != country->ArrayIndex)
				{
					result = false;
					return true;
				}
			}

			result = true;
			*Args.isRepeating = true;
			pThis->House = Args.Object->GetOwningHouse();
		}

		break;
	}

	case TriggerEvent::SpyAsHouse:
	{                // 0x35
		result = false;

		if ((Args.RequestedEventType == pThis->EventKind || Args.RequestedEventType == TriggerEvent::SpyAsInfantry) && !Unsorted::MAP_DEBUG_MODE() && Args.Object) {
			if (auto country = TEventExtData::ResolveHouseParam(pThis->Value)) {
				if (HouseClass* showAsHouse = Args.Object->GetDisguiseHouse(true)) {
					if (showAsHouse->ArrayIndex == country->ArrayIndex) {
						*Args.isRepeating = 1;
						result = true;
					}
				}
			}
		}

		break;
	}
	case TriggerEvent::SpyAsInfantry:
	{             // 0x36

		result = false;

		if ((Args.RequestedEventType == pThis->EventKind || Args.RequestedEventType == TriggerEvent::SpyAsHouse) && !Unsorted::MAP_DEBUG_MODE() && Args.Object) {
			if (auto* showAs = Args.Object->GetDisguise(true)) {
				if (showAs->WhatAmI() != InfantryTypeClass::AbsID){
					if (pThis->Value != -1 && showAs->GetArrayIndex() == pThis->Value) {
						*Args.isRepeating = 1;
						result = true;
					}
				}
			}
		}

		break;
	}
	case TriggerEvent::ComesNearWaypoint:
	{         // 0x22
		result = false;

		if (guarded()){
			CoordStruct waypointCoord;
			ScenarioClass::Instance->GetWaypointCoordinate(&waypointCoord, pThis->Value);
			result = (int)((Args.Object->GetCoords() - waypointCoord).Length()) <= 1280;
		}

		break;
	}

	// ---------------------------------------------------------------------------------
	//  house-counter events  (check only when 'house' present, else -> true)
	// ---------------------------------------------------------------------------------
	case TriggerEvent::CreditsExceed:               // 0x0C
	{
		if(Args.Owner)
			result = Args.Owner->Available_Money() >= pThis->Value;
		break;
	}
	case TriggerEvent::CreditsBelow:                // 0x34
	{
		if (Args.Owner)
			result = Args.Owner && Args.Owner->Available_Money() <= pThis->Value;
		break;
	}
	case TriggerEvent::DestroyedBuildingsNum:       // 0x0F
	{
		if (Args.Owner)
			result = Args.Owner && Args.Owner->TotalKilledBuildings >= pThis->Value;
		break;
	}
	case TriggerEvent::DestroyedUnitsNum:           // 0x10
	{
		if (Args.Owner)
			result = Args.Owner && Args.Owner->TotalKilledUnits >= pThis->Value;
		break;
	}
	case TriggerEvent::NoFactoriesLeft:             // 0x11
	{
		if (Args.Owner && Args.Owner->OwnedBuildings > 0) {
			for (int i = 0; i < Args.Owner->Buildings.Count; ++i) {
				BuildingClass* building = Args.Owner->Buildings.Items[i];
				if (building &&
					!building->InLimbo &&
					 building->Type->Factory != AbstractType::None)
				{
					result = false;
					return true;
				}
			}
		}

		break;
	}
	case TriggerEvent::CiviliansEvacuated:          // 0x12  (guarded)
	{
		if (Args.Owner)
			result = guarded() && Args.Owner && Args.Owner->CiviliansEvacuated;
		break;
	}
	case TriggerEvent::BuildBuildingType:           // 0x13  (build events skip guard)
	{
		if (Args.Owner) {
			if (Args.Owner->LastBuiltBuildingType != pThis->Value)
				result = false;
			else
				*Args.isRepeating = true;
		}

		break;
	}
	case TriggerEvent::BuildUnitType:               // 0x14
	{
		if (Args.Owner) {
			auto pHouseExt = HouseExtContainer::Instance.Find(Args.Owner);

			if (Args.Owner->LastBuiltVehicleType != pThis->Value
			&& pHouseExt->LastBuiltNavalVehicleType != pThis->Value)
				result = false;
			else
				*Args.isRepeating = true;
		}

		break;
	}
	case TriggerEvent::BuildInfantryType:           // 0x15
	{
		if (Args.Owner) {
			if (Args.Owner->LastBuiltInfantryType != pThis->Value)
				result = false;
			else
				*Args.isRepeating = true;
		}

		break;
	}
	case TriggerEvent::BuildAircraftType:           // 0x16
	{
		if (Args.Owner) {
			if (Args.Owner->LastBuiltAircraftType != pThis->Value)
				result = false;
			else
				*Args.isRepeating = true;
		}

		return true;
	}
	case TriggerEvent::TeamLeavesMap:               // 0x17  (guarded)
	{
		if (guarded()) {
			if (Args.Owner) {

				bool found = false;

				for (int i = 0; i < TeamClass::Array->Count; ++i) {
					TeamClass* team = TeamClass::Array->Items[i];
					if (team->Type == pThis->TeamType && !team->FirstUnit && team->IsLeavingMap) { 
						found = true;
						break;
					}
				}

				if (!found){
					result = false;
				} else {
					*Args.isRepeating = true;
				}			
			}
		} else  { result = false; }

		break;
	}
	case TriggerEvent::BuildingExists:              // 0x20
	{
		if (Args.Owner) {
			if(Args.Owner->ActiveBuildingTypes.get_count(pThis->Value))
			*Args.isRepeating = 1;
			else result = false;
		}

		break;
	}
	case TriggerEvent::BuildingDoesNotExist:        // 0x39
	{
		if (Args.Owner) {
			if(!Args.Owner->ActiveBuildingTypes.get_count(pThis->Value))
				*Args.isRepeating = 1;
			else result = false;
		}

		break;
	}

		// ---------------------------------------------------------------------------------
		//  target-house events  (check only when As_Pointer(Value2) != null, else -> true)
		// ---------------------------------------------------------------------------------
	case TriggerEvent::ThievedBy:                   // 0x03  (guarded)
	{
		result = false;

		if (guarded()) {
			if (HouseClass* t = tgt()) {
				result = t->HasBeenThieved;
			} else {
				result = !HouseClass::Index_IsMP(pThis->Value);
			}
		}
		break;
	}
	case TriggerEvent::HouseDiscovered:             // 0x05
	{
		result = false;

		if (HouseClass* t = tgt()) {
			result = t->DiscoveredByPlayer;
		} else {
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}
	case TriggerEvent::DestroyedUnitsAll:           // 0x09
	{
		result = false;

		if (HouseClass* targetHouse = tgt())
		{
			result = targetHouse->ActiveUnitTypes.total() <= 0 &&
				targetHouse->ActiveInfantryTypes.total() <= 0;
		}
		else
		{
			result = !HouseClass::Index_IsMP(pThis->Value);
		}

		break;
	}
	case TriggerEvent::DestroyedBuildingsAll:       // 0x0A
	{
		result = false;

		if (HouseClass* targetHouse = tgt()) {
			result = targetHouse->OwnedBuildings <= 0;
		} else {
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}
	case TriggerEvent::DestroyedAll:                // 0x0B
	{
		result = true;

		if (HouseClass* targetHouse = tgt()) {
	
				if (targetHouse->ActiveInfantryTypes.total() <= 0) {
					for (auto& bld : targetHouse->Buildings) {
						if (bld->Type->CanBeOccupied && bld->Occupants.Count > 0) {
							result = false;
							break;
						}
					}
				}

				if (SessionClass::IsCampaign()) {

					if (result && targetHouse->ActiveAircraftTypes.total() > 0)
						result = false;

					if (result && targetHouse->ActiveInfantryTypes.total() > 0)
						result = false;

					if(result){
						for (auto pItem : *InfantryClass::Array) {
							if (pItem->InLimbo && targetHouse == pItem->GetOwningHouse() && targetHouse->IsAlliedWith(pItem->Transporter)){
								result = false;
								break;
							}
						}
					}
				}

			} else {
				result = !HouseClass::Index_IsMP(pThis->Value);
		}

		break;
	}
	case TriggerEvent::LowPower:                     // 0x1E
	{
		result = false;

		if (HouseClass* targetHouse = tgt())
		{
			result = targetHouse->GetPowerPercentage() < 1.0;
		}
		else
		{
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}
	case TriggerEvent::PowerFull:                    // 0x3A
	{
		result = false;

		if (HouseClass* targetHouse = tgt())
		{
			result = targetHouse->GetPowerPercentage() >= 1.0;
		}
		else
		{
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}
	case TriggerEvent::DestroyedUnitsNaval:         // 0x37
	{
		result = false;

		if (HouseClass* targetHouse = tgt())
		{
			result = targetHouse->OwnedNavy <= 0;
		}
		else
		{
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}
	case TriggerEvent::DestroyedUnitsLand:          // 0x38
	{
		result = false;

		if (HouseClass* targetHouse = tgt())
		{
			result = (targetHouse->OwnedUnits - targetHouse->OwnedNavy) <= 0 &&
				targetHouse->OwnedInfantry <= 0;
		}
		else
		{
			result = !HouseClass::Index_IsMP(pThis->Value);
		}
		break;
	}

		// ---------------------------------------------------------------------------------
		//  attacked-by-specific-house  (guarded + source house match)
		// ---------------------------------------------------------------------------------
	case TriggerEvent::AttackedByHouse:             // 0x2C
	{
		result = false;

		if (guarded() && Args.Source) {
			int param = pThis->Value;
			// convert Player @ X to real index
			if (HouseClass::Index_IsMP(pThis->Value)) {
				auto const pPlayer = TEventExtData::ResolveHouseParam(pThis->Value);
				param = pPlayer ? pPlayer->ArrayIndex : -1;
			}

			result = param == Args.Source->GetOwningHouse()->ArrayIndex;
		}

		break;
	}
	
		// ---------------------------------------------------------------------------------
		//  guard-only events: fire iff the incoming event matches; no further condition.
		//  (== return guarded())
		// ---------------------------------------------------------------------------------
	case TriggerEvent::SpiedBy:                      // 0x02
	case TriggerEvent::DiscoveredByPlayer:           // 0x04
	case TriggerEvent::AttackedByAnybody:            // 0x06
	case TriggerEvent::DestroyedByAnybody:           // 0x07
	case TriggerEvent::DestroyedFakesAll:            // 0x1D
	case TriggerEvent::AllBridgesDestroyed:          // 0x1F
	case TriggerEvent::SelectedByPlayer:             // 0x21
	case TriggerEvent::EnemyInSpotlight:             // 0x23
	case TriggerEvent::FirstDamaged_combatonly:      // 0x26
	case TriggerEvent::HalfHealth_combatonly:        // 0x27
	case TriggerEvent::QuarterHealth_combatonly:     // 0x28
	case TriggerEvent::FirstDamaged_anysource:       // 0x29
	case TriggerEvent::HalfHealth_anysource:         // 0x2A
	case TriggerEvent::QuarterHealth_anysource:      // 0x2B
	case TriggerEvent::DestroyedByAnything:          // 0x30
	case TriggerEvent::PickupCrate:                  // 0x31
	case TriggerEvent::PickupCrate_any:              // 0x32
	{
		result = guarded();
		break;
	}
	default:
		return false;
	}

	return true;
}

// the function return is deciding if the case is handled or not
// the bool result pointer is for the result of the Event itself
bool TEventExtData::AresTriggerEventOccured(TEventClass* pThis, EventArgs& Args, bool& result)
{
	const AresTriggerEvents TEventKind = (AresTriggerEvents)pThis->EventKind;
	const AresTriggerEvents ExecutedKind = (AresTriggerEvents)Args.RequestedEventType;
	// They must be the same, but for other triggers to take effect normally, this cannot be judged outside case.
	const auto isSameEvent = [&]() { return TEventKind == ExecutedKind; };

	{
		switch (TEventKind)
		{
		case AresTriggerEvents::UnderEMP:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent() && pTechno->EMPLockRemaining > 0;
			break;
		}
		case AresTriggerEvents::UnderEMP_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value
				&& pTechno->EMPLockRemaining > 0;

			break;
		}
		case AresTriggerEvents::RemoveEMP:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent() && pTechno->EMPLockRemaining <= 0;
			break;
		}
		case AresTriggerEvents::RemoveEMP_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value
				&& pTechno->EMPLockRemaining <= 0;
			break;
		}
		case AresTriggerEvents::EnemyInSpotlightNow:
		{
			result = true;
			break;
		}
		case AresTriggerEvents::DriverKiller:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent();
			break;
		}
		case AresTriggerEvents::DriverKilled_ByHouse:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;
			break;
		}
		case AresTriggerEvents::VehicleTaken:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent();
			break;
		}
		case AresTriggerEvents::VehicleTaken_ByHouse:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;
			break;
		}
		case AresTriggerEvents::Abducted:
		case AresTriggerEvents::AbductSomething:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent();
			break;
		}
		case AresTriggerEvents::Abducted_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& (flag_cast_to<TechnoClass*>(Args.Source)
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value);
			break;
		}
		case AresTriggerEvents::AbductSomething_OfHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& (cast_to<HouseClass*>(Args.Source)
				&& ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value);
			break;
		}
		case AresTriggerEvents::SuperActivated:
		case AresTriggerEvents::SuperDeactivated:
		{
			result = isSameEvent()
				&& Args.Source
				&& Args.Source->WhatAmI() == AbstractType::Super
				&& ((SuperClass*)Args.Source)->Type->ArrayIndex == pThis->Value;
			break;
		}
		case AresTriggerEvents::SuperNearWaypoint:
		{
			struct PackedDatas
			{
				SuperClass* Super;
				CellStruct Cell;
			};

			if (isSameEvent() && IS_SAME_STR_(((PackedDatas*)Args.Source)->Super->Type->ID, pThis->String))
			{
				const auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Value);
				CellStruct nDesired = { ((PackedDatas*)Args.Source)->Cell.X - nCell.X ,((PackedDatas*)Args.Source)->Cell.Y - nCell.Y };
				if (nDesired.pow() <= 5.0)
				{
					result = true;
					break;
				}
			}

			result = false;
			break;
		}
		case AresTriggerEvents::ReverseEngineered:
		{
			if (!Args.Owner)
				result = false;
			else
			{
				if (!HouseExtContainer::Instance.Find(Args.Owner)->Reversed.empty())
				{
					auto TEvetType = TEventExtContainer::Instance.Find(pThis)->GetTechnoType();

					for (auto pTechR : HouseExtContainer::Instance.Find(Args.Owner)->Reversed)
					{
						if (pTechR == TEvetType)
						{
							result = true;
							break;
						}
					}

				}
			}
			break;
		}
		case AresTriggerEvents::ReverseEngineerAnything:
		{
			result = isSameEvent();
			break;
		}
		case AresTriggerEvents::ReverseEngineerType:
		{
			result = GET_TECHNOTYPE(((TechnoClass*)Args.Source)) == TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
			break;
		}
		case AresTriggerEvents::HouseOwnTechnoType:
		{
			result = FindTechnoType(pThis, pThis->Value, Args.Owner);
			break;
		}
		case AresTriggerEvents::HouseDoesntOwnTechnoType:
		{
			result = !FindTechnoType(pThis, pThis->Value + 1, Args.Owner);
			break;
		}
		case AresTriggerEvents::AttackedOrDestroyedByAnybody:
		{
			result = isSameEvent();
			break;
		}
		case AresTriggerEvents::AttackedOrDestroyedByHouse:
		{
			result = isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;
			break;
		}
		case AresTriggerEvents::DestroyedByHouse:
		{
			result = isSameEvent()
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;
			break;
		}
		case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		{
			result = FindTechnoType(pThis, pThis->Value + 1, nullptr);
			break;
		}
		case AresTriggerEvents::AllKeepAlivesDestroyed:
		{
			HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && HouseExtContainer::Instance.Find(pHouse)->KeepAliveCount <= 0;
			return true;
		}
		case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		{
			HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && HouseExtContainer::Instance.Find(pHouse)->KeepAliveBuildingCount <= 0;
			break;
		}
		default:
			return false;
		}
	}

	return true;
}

bool TEventExtData::PhobosTriggerEventOccured(TEventClass* pThis, EventArgs const& args, bool& result)
{
	//int iEvent = args.EventType; // not used here ,.. ares using it compare
	HouseClass* pHouse = args.Owner;
	ObjectClass* pObject = args.Object;
	const PhobosTriggerEvent TEventKind = (PhobosTriggerEvent)pThis->EventKind;
	const PhobosTriggerEvent ExtcutedEventKind = (PhobosTriggerEvent)args.RequestedEventType;

	//CDTimerClass* pTimer = args.ActivationFrame;
	//bool* isPersitant = args.isRepeating;
	//AbstractClass* pSource = args.Source;

	// They must be the same, but for other triggers to take effect normally, this cannot be judged outside case.
	const auto isSameEvent = [&]() { return TEventKind == ExtcutedEventKind; };

	switch (TEventKind)
	{

#pragma region LovalVariableManipulation
	case PhobosTriggerEvent::LocalVariableGreaterThan:
		result = TEventExtData::VariableCheck<false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThan:
		result = TEventExtData::VariableCheck<false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsTo:
		result = TEventExtData::VariableCheck<false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		result = TEventExtData::VariableCheck<false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		result = TEventExtData::VariableCheck<false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		result = TEventExtData::VariableCheck<false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
		result = TEventExtData::VariableCheck<true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThan:
		result = TEventExtData::VariableCheck<true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
		result = TEventExtData::VariableCheck<true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		result = TEventExtData::VariableCheck<true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		result = TEventExtData::VariableCheck<true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		result = TEventExtData::VariableCheck<true, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		result = TEventExtData::VariableCheckBinary<false, true, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		result = TEventExtData::VariableCheckBinary<true, true, std::and_with<int>>(pThis);
		break;
#pragma endregion

		//TODO compare agains like vanilla does ?
#pragma region PhobosEvent
	case PhobosTriggerEvent::DestroyedOnly:
		result = isSameEvent();
		break;
		/*
		*	- PersistableFlag ?
		*	- LogcNeed ?
		*   - AttachFlags ?
		*/
	case PhobosTriggerEvent::ShieldBroken:
		result = isSameEvent() && ShieldClass::TEventIsShieldBroken(pObject);
		break;
	case PhobosTriggerEvent::HouseOwnsTechnoType:
		result = TEventExtData::HouseOwnsTechnoTypeTEvent(pThis);
		break;
	case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
		result = TEventExtData::HouseDoesntOwnTechnoTypeTEvent(pThis);
		break;
	case PhobosTriggerEvent::HousesDestroyed:
		result = TEventExtData::HousesAreDestroyedTEvent(pThis);
		break;
	case PhobosTriggerEvent::CellHasTechnoType:
		result = TEventExtData::CellHasTechnoTypeTEvent(pThis, pObject, pHouse);
		break;
	case PhobosTriggerEvent::CellHasAnyTechnoTypeFromList:
		result = TEventExtData::CellHasAnyTechnoTypeFromListTEvent(pThis, pObject, pHouse);
		break;
	case PhobosTriggerEvent::AttachedIsUnderAttachedEffect:
		result = TEventExtData::AttachedIsUnderAttachedEffectTEvent(pThis, pObject);
		break;
		{//https://github.com/Chang-zhi/PhobosExt_Changzhi

	case PhobosTriggerEvent::TechnoTypeOfHouseNearWaypoint:
		result = TEventExtData::TechnoTypeOfHouseNearWaypoint(pThis, pHouse); break;
	case PhobosTriggerEvent::TechnoTypeOfHouseAllLeavesWaypoint:
		result = !TEventExtData::TechnoTypeOfHouseNearWaypoint(pThis, pHouse); break;
	case PhobosTriggerEvent::TechnoTypeOfHouseExistsAtWaypoint:
		result = TEventExtData::TechnoTypeOfHouseExistsAtWaypoint(pThis, pHouse); break;
	case PhobosTriggerEvent::TechnoTypeOfHouseNotExistsAtWaypoint:
		result = !TEventExtData::TechnoTypeOfHouseExistsAtWaypoint(pThis, pHouse); break;
	case PhobosTriggerEvent::ElapsedTimeFrames:
		result = TEventExtData::ElapsedTimeFramesFunc(pThis); break;

	case PhobosTriggerEvent::MissionTimerGreater:
		result = TEventExtData::MissionTimerGreaterFunc(pThis); break;
	case PhobosTriggerEvent::MissionTimerLess:
		result = TEventExtData::MissionTimerLessFunc(pThis); break;

	case PhobosTriggerEvent::ChoiceBoxButtonClicked:
		result = TEventExtData::ChoiceBoxButtonClickedFunc(pThis, pHouse); break;
	case PhobosTriggerEvent::ChoiceBoxAnyButtonClicked:
		result = TEventExtData::ChoiceBoxAnyButtonClickedFunc(pThis, pHouse); break;
	case PhobosTriggerEvent::ChoiceBoxTimedOut:
		result = TEventExtData::ChoiceBoxTimedOutFunc(pThis, pHouse); break;

	case PhobosTriggerEvent::HousePowerOutputMuch:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Output, true); break;
	case PhobosTriggerEvent::HousePowerOutputLess:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Output, false); break;
	case PhobosTriggerEvent::HousePowerDrainMuch:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Drain, true); break;
	case PhobosTriggerEvent::HousePowerDrainLess:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Drain, false); break;
	case PhobosTriggerEvent::HousePowerSurplusMuch:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Surplus, true); break;
	case PhobosTriggerEvent::HousePowerSurplusLess:
		result = TEventExtData::PowerHander(pThis, pHouse, PowerEventMode::Surplus, false); break;

	}
#pragma endregion
	default:
		return false;
	};

	return true;
}

bool FakeTEventClass::_Occured(TriggerEvent requestedEvent, HouseClass* house, ObjectClass* obj, CDTimerClass* td, bool* isPresistent, AbstractClass* source)
{
	bool result = true;

	EventArgs args {
		requestedEvent, house, obj, td, isPresistent, source
	};

	if (TEventExtData::VanillaTriggerEventOccured(this, args, result)) {
		return result;
	}

	if (TEventExtData::AresTriggerEventOccured(this, args, result)) {
		return result;
	}

	if (TEventExtData::PhobosTriggerEventOccured(this, args, result)) {
		return result;
	}

	Debug::LogInfo("TEvent {} Trying to execute unknown event of {} with {}",(void*)this, (int)requestedEvent , (int)this->EventKind);
	return false;//empty
}

bool FakeTEventClass::_IsPresistable()
{
	switch (this->EventKind)
	{
		case TriggerEvent::SpiedBy:
		case TriggerEvent::SpyAsHouse:
		case TriggerEvent::SpyAsInfantry:
		case TriggerEvent::AttackedByAnybody:
		case TriggerEvent::AttackedByHouse:
		case TriggerEvent::EnteredOrOverflownBy:
		{
			return false;
		};
	}

		//todo : CHECK IF ARES AND PHOBOS STUFFS IS IN THIS AREA
	std::pair<bool, bool> result =
		TEventExtData::GetPersistableFlag((AresTriggerEvents)this->EventKind);

	if (!result.second)
		result = TEventExtData::GetPersistableFlag((PhobosTriggerEvent)this->EventKind);

	if (!result.second)
		return true; // default

	return result.first;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x71F9C0, FakeTEventClass::_IsPresistable);
DEFINE_FUNCTION_JUMP(CALL, 0x726579, FakeTEventClass::_IsPresistable);

bool FakeTEventClass::_IsTemporal()
{
	//todo : CHECK IF ARES AND PHOBOS STUFFS IS IN THIS AREA

	switch (this->EventKind)
	{
	case TriggerEvent::EnteredBy:
	case TriggerEvent::SpiedBy:
	case TriggerEvent::ThievedBy:
	case TriggerEvent::DiscoveredByPlayer:
	case TriggerEvent::AttackedByAnybody:
	case TriggerEvent::DestroyedByAnybody:
	case TriggerEvent::CiviliansEvacuated:
	case TriggerEvent::BuildBuildingType:
	case TriggerEvent::BuildUnitType:
	case TriggerEvent::BuildInfantryType:
	case TriggerEvent::BuildAircraftType:
	case TriggerEvent::TeamLeavesMap:
	case TriggerEvent::ZoneEntryBy:
	case TriggerEvent::CrossesHorizontalLine:
	case TriggerEvent::CrossesVerticalLine:
	case TriggerEvent::DestroyedFakesAll:
	case TriggerEvent::AllBridgesDestroyed:
	case TriggerEvent::SelectedByPlayer:
	case TriggerEvent::ComesNearWaypoint:
	case TriggerEvent::EnemyInSpotlight:
	case TriggerEvent::FirstDamaged_combatonly:
	case TriggerEvent::HalfHealth_combatonly:
	case TriggerEvent::QuarterHealth_combatonly:
	case TriggerEvent::FirstDamaged_anysource:
	case TriggerEvent::HalfHealth_anysource:
	case TriggerEvent::QuarterHealth_anysource:
	case TriggerEvent::AttackedByHouse:
	case TriggerEvent::DestroyedByAnything:
	case TriggerEvent::PickupCrate:
	case TriggerEvent::PickupCrate_any:
	case TriggerEvent::SpyAsHouse:
	case TriggerEvent::SpyAsInfantry:
	case TriggerEvent::EnteredOrOverflownBy:
		return 1;
	default:
		return 0;
	}
}
DEFINE_FUNCTION_JUMP(LJMP, 0x71F950, FakeTEventClass::_IsTemporal);
DEFINE_FUNCTION_JUMP(CALL, 0x72656E, FakeTEventClass::_IsTemporal);

TriggerAttachType __fastcall FakeTEventClass::AttachesTo(unsigned int nAction)
{
	std::pair<TriggerAttachType, bool> result = TEventExtData::GetAttachFlags((AresTriggerEvents)nAction);

	if (!result.second)
		result = TEventExtData::GetTriggetAttach((PhobosTriggerEvent)nAction);

	TriggerAttachType attach = TriggerAttachType::None;
	if (result.second) {
		attach = result.first;
	}
	else {
		switch ((TriggerEvent)nAction)
		{
		case TriggerEvent::None:
		case TriggerEvent::EnteredBy:
		case TriggerEvent::DiscoveredByPlayer:
		case TriggerEvent::AnyEvent:
		case TriggerEvent::ZoneEntryBy:
		case TriggerEvent::CrossesHorizontalLine:
		case TriggerEvent::CrossesVerticalLine:
		case TriggerEvent::AllBridgesDestroyed:
		case TriggerEvent::SpyAsHouse:
		case TriggerEvent::SpyAsInfantry:
		case TriggerEvent::EnteredOrOverflownBy:
			attach = TriggerAttachType::Global;
			break;
		default:
			break;
		}

		switch ((TriggerEvent)nAction)
		{
		case TriggerEvent::None:
		case TriggerEvent::EnteredBy:
		case TriggerEvent::SpiedBy:
		case TriggerEvent::DiscoveredByPlayer:
		case TriggerEvent::AttackedByAnybody:
		case TriggerEvent::DestroyedByAnybody:
		case TriggerEvent::AnyEvent:
		case TriggerEvent::DestroyedFakesAll:
		case TriggerEvent::SelectedByPlayer:
		case TriggerEvent::ComesNearWaypoint:
		case TriggerEvent::EnemyInSpotlight:
		case TriggerEvent::FirstDamaged_combatonly:
		case TriggerEvent::HalfHealth_combatonly:
		case TriggerEvent::QuarterHealth_combatonly:
		case TriggerEvent::FirstDamaged_anysource:
		case TriggerEvent::HalfHealth_anysource:
		case TriggerEvent::QuarterHealth_anysource:
		case TriggerEvent::AttackedByHouse:
		case TriggerEvent::DestroyedByAnything:
		case TriggerEvent::PickupCrate:
			attach |= TriggerAttachType::Object;
			break;
		default:
			break;
		}

		if ((TriggerEvent)nAction == TriggerEvent::AnyEvent
			|| (TriggerEvent)nAction == TriggerEvent::ZoneEntryBy)
		{
			attach |= TriggerAttachType::Map;
		}

		switch ((TriggerEvent)nAction)
		{
		case TriggerEvent::ThievedBy:
		case TriggerEvent::HouseDiscovered:
		case TriggerEvent::AnyEvent:
		case TriggerEvent::DestroyedUnitsAll:
		case TriggerEvent::DestroyedBuildingsAll:
		case TriggerEvent::DestroyedAll:
		case TriggerEvent::CreditsExceed:
		case TriggerEvent::DestroyedBuildingsNum:
		case TriggerEvent::DestroyedUnitsNum:
		case TriggerEvent::NoFactoriesLeft:
		case TriggerEvent::CiviliansEvacuated:
		case TriggerEvent::BuildBuildingType:
		case TriggerEvent::BuildUnitType:
		case TriggerEvent::BuildInfantryType:
		case TriggerEvent::BuildAircraftType:
		case TriggerEvent::LowPower:
		case TriggerEvent::BuildingExists:
		case TriggerEvent::CreditsBelow:
		case TriggerEvent::DestroyedUnitsNaval:
		case TriggerEvent::DestroyedUnitsLand:
		case TriggerEvent::BuildingDoesNotExist:
		case TriggerEvent::PowerFull:
			attach |= TriggerAttachType::House;
			break;
		default:
			break;
		}

		switch ((TriggerEvent)nAction)
		{
		case TriggerEvent::AnyEvent:
		case TriggerEvent::ElapsedTime:
		case TriggerEvent::MissionTimerExpired:
		case TriggerEvent::TeamLeavesMap:
		case TriggerEvent::GlobalSet:
		case TriggerEvent::GlobalCleared:
		case TriggerEvent::LocalSet:
		case TriggerEvent::LocalCleared:
		case TriggerEvent::AmbientLightBelow:
		case TriggerEvent::AmbientLightAbove:
		case TriggerEvent::ElapsedScenarioTime:
		case TriggerEvent::PickupCrate_any:
		case TriggerEvent::RandomDelay:
		case TriggerEvent::TechTypeExists:
		case TriggerEvent::TechTypeDoesntExist:
			attach |= TriggerAttachType::Logic;
			break;
		default:
			break;
		}
	}

	return attach;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x71F680 , FakeTEventClass::AttachesTo)

void FakeTEventClass::_ReadINI()
{
	
	this->Value = 0;
	this->EventKind = static_cast<TriggerEvent>(std::atoi(std::strtok(nullptr, ",")));

	const int   code = std::atoi(std::strtok(nullptr, ","));
	char* const text = std::strtok(nullptr, ",");
	const int   val = std::atoi(text);

	// --- token consumption pass ---
	char* fourth_arg = nullptr;
	char* fifth_arg = nullptr;

	switch (code)
	{
	case 4: // three numeric args: consume two more tokens
		fourth_arg = std::strtok(nullptr, ",");
		fifth_arg = std::strtok(nullptr, ",");
		break;

	case 2: // FALLTHROUGH
	case 3: // two args: consume one more token
		fourth_arg = std::strtok(nullptr, ",");
		break;
	default:
		break;
	}

	switch (code)
	{
	case 0:
		this->Value = val;
		break;

	case 1:
		this->TeamType = TeamTypeClass::Find(text);
		break;

	case 2:
	{
		this->Value = val;

		// BUGFIX: vanilla strncpy limit was 24, leaving String[25..27] untouched.
		 //         Use full 27 chars + null to utilize the complete char[28] field.
		std::strncpy(this->String, fourth_arg ? fourth_arg : "", sizeof(this->String) - 1);
		this->String[sizeof(this->String) - 1] = '\0';
		break;
	}

	default:
		// param1 >= 3: vanilla falls through silently — no operation.
		break;
	}

	//0x71F58B, TEventClass_ReadINI_MaskedTEvents, 0x7
	switch (static_cast<PhobosTriggerEvent>(this->EventKind))
	{
	case PhobosTriggerEvent::EnteredByByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::EnteredBy;
		break;
	case PhobosTriggerEvent::SpiedByByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::SpiedBy;
		break;
	case PhobosTriggerEvent::HouseDiscoveredByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::HouseDiscovered;
		break;
	case PhobosTriggerEvent::DestroyedUnitsAllByID:
		this->Value = UnitTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::DestroyedUnitsAll;
		break;
	case PhobosTriggerEvent::DestroyedBuildingsAllByID:
		this->Value = BuildingTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::DestroyedBuildingsAll;
		break;
	case PhobosTriggerEvent::DestroyedAllByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::DestroyedAll;
		break;
	case PhobosTriggerEvent::BuildBuildingTypeByID:
		this->Value = BuildingTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildBuildingType;
		break;
	case PhobosTriggerEvent::BuildUnitTypeByID:
		this->Value = UnitTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildUnitType;
		break;
	case PhobosTriggerEvent::BuildInfantryTypeByID:
		this->Value = InfantryTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildInfantryType;
		break;
	case PhobosTriggerEvent::BuildAircraftTypeByID:
		this->Value = AircraftTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildAircraftType;
		break;
	case PhobosTriggerEvent::ZoneEntryByByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::ZoneEntryBy;
		break;
	case PhobosTriggerEvent::CrossesHorizontalLineByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::CrossesHorizontalLine;
		break;
	case PhobosTriggerEvent::CrossesVerticalLineByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::CrossesVerticalLine;
		break;
	case PhobosTriggerEvent::LowPowerByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::LowPower;
		break;
	case PhobosTriggerEvent::BuildingExistsByID:
		this->Value = BuildingTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildingExists;
		break;
	case PhobosTriggerEvent::AttackedByHouseByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::AttackedByHouse;
		break;
	case PhobosTriggerEvent::SpyAsHouseByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::SpyAsHouse;
		break;
	case PhobosTriggerEvent::SpyAsInfantryByID:
		this->Value = InfantryTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::SpyAsInfantry;
		break;
	case PhobosTriggerEvent::DestroyedUnitsNavalByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::DestroyedUnitsNaval;
		break;
	case PhobosTriggerEvent::DestroyedUnitsLandByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::DestroyedUnitsLand;
		break;
	case PhobosTriggerEvent::BuildingDoesNotExistByID:
		this->Value = BuildingTypeClass::FindIndexById(this->String);
		this->EventKind = TriggerEvent::BuildingDoesNotExist;
		break;
	case PhobosTriggerEvent::PowerFullByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::PowerFull;
		break;
	case PhobosTriggerEvent::EnteredOrOverflownByByID:
		this->Value = HouseTypeClass::FindIndexByIdAndName(this->String);
		this->EventKind = TriggerEvent::EnteredOrOverflownBy;
		break;

	default:
		break;
	}
}
DEFINE_FUNCTION_JUMP(LJMP, 0x71F4E0, FakeTEventClass::_ReadINI);
DEFINE_FUNCTION_JUMP(CALL, 0x7274FF, FakeTEventClass::_ReadINI);

LogicNeedType TEventExtData::ClassifyEvent(int event)
{
	// Ares extended events
	std::pair<LogicNeedType, bool> result = TEventExtData::GetLogicNeed(
		static_cast<AresTriggerEvents>(event));

	if (result.second)
		return result.first;

	// Phobos extended events
	result = TEventExtData::GetLogicNeed(static_cast<PhobosTriggerEvent>(event));

	if (result.second)
		return result.first;

	switch (static_cast<TriggerEvent>(event))
	{
	case TriggerEvent::EnteredBy:
	case TriggerEvent::ThievedBy:
	case TriggerEvent::HouseDiscovered:
	case TriggerEvent::DestroyedUnitsAll:
	case TriggerEvent::DestroyedBuildingsAll:
	case TriggerEvent::DestroyedAll:
	case TriggerEvent::ZoneEntryBy:
	case TriggerEvent::CrossesHorizontalLine:
	case TriggerEvent::CrossesVerticalLine:
	case TriggerEvent::LowPower:
	case TriggerEvent::AttackedByHouse:
	case TriggerEvent::SpyAsHouse:
	case TriggerEvent::DestroyedUnitsNaval:
	case TriggerEvent::DestroyedUnitsLand:
	case TriggerEvent::PowerFull:
	case TriggerEvent::EnteredOrOverflownBy:
		return LogicNeedType::House;

	case TriggerEvent::CreditsExceed:
	case TriggerEvent::ElapsedTime:
	case TriggerEvent::DestroyedBuildingsNum:
	case TriggerEvent::DestroyedUnitsNum:
	case TriggerEvent::AmbientLightBelow:
	case TriggerEvent::AmbientLightAbove:
	case TriggerEvent::ElapsedScenarioTime:
	case TriggerEvent::RandomDelay:
	case TriggerEvent::CreditsBelow:
		return LogicNeedType::Number;

	case TriggerEvent::BuildBuildingType:
	case TriggerEvent::BuildingExists:
	case TriggerEvent::BuildingDoesNotExist:
		return LogicNeedType::Structure;

	case TriggerEvent::BuildUnitType:
		return LogicNeedType::Unit;

	case TriggerEvent::BuildInfantryType:
	case TriggerEvent::SpyAsInfantry:
		return LogicNeedType::Infantry;

	case TriggerEvent::BuildAircraftType:
		return LogicNeedType::Aircraft;

	case TriggerEvent::TeamLeavesMap:
		return LogicNeedType::Team;

	case TriggerEvent::GlobalSet:
	case TriggerEvent::GlobalCleared:
		return LogicNeedType::Global;

	case TriggerEvent::ComesNearWaypoint:
		return LogicNeedType::Waypoint;

	case TriggerEvent::LocalSet:
	case TriggerEvent::LocalCleared:
		return LogicNeedType::Local;

	case TriggerEvent::TechTypeExists:
	case TriggerEvent::TechTypeDoesntExist:
		return LogicNeedType::NumberNTech;

	default:
		return LogicNeedType::None;
	}
}

std::string FakeTEventClass::_BuildINIEntry()
{
	const int           event = this->Event;
	const int           value2 = this->Value;
	const LogicNeedType need = TEventExtData::ClassifyEvent(event);

	if (this->TeamType)
		return fmt::format("{},{},{}", event, 1, this->TeamType->ID);

	if (need == LogicNeedType::NumberNTech)
		return fmt::format("{},{},{},{}", event, 2, value2, this->String);

	return fmt::format("{},{},{}", event, 0, value2);
}

DEFINE_FUNCTION_JUMP(CALL , 0x726540, FakeTEventClass::_Occured)

// =============================
// container
TEventExtContainer TEventExtContainer::Instance;
// =============================
// container hooks
//

ASMJIT_PATCH(0x71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
	TEventExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x71E856, TEventClass_SDDTOR, 0x6)
{
	GET(TEventClass*, pItem, ESI);
	TEventExtContainer::Instance.Remove(pItem);
	return 0;
}
ASMJIT_PATCH_AGAIN(0x71FAA6, TEventClass_SDDTOR, 0x6) // Factory

HRESULT __stdcall FakeTEventClass::__Load(IStream* pStm)
{
	HRESULT hr = this->TEventClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!TEventExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F558C, FakeTEventClass::__Load)

HRESULT __stdcall FakeTEventClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->TEventClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!TEventExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5590, FakeTEventClass::__Save)