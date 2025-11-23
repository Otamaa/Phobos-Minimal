#include "Body.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/Macro.h>

#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Script/Body.h>

#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

#include <New/Entity/ShieldClass.h>
#include <New/PhobosAttachedAffect/PhobosAttachEffectClass.h>
#include <New/PhobosAttachedAffect/Functions.h>

#include <Misc/Ares/Hooks/Header.h>

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

bool TEventExtData::Occured(TEventClass* pThis, EventArgs const& args, bool& result)
{
	//int iEvent = args.EventType; // not used here ,.. ares using it compare
	HouseClass* pHouse = args.Owner;
	ObjectClass* pObject = args.Object;
	const PhobosTriggerEvent TEventKind = (PhobosTriggerEvent)pThis->EventKind;
	const PhobosTriggerEvent ExtcutedEventKind = (PhobosTriggerEvent)args.EventType;

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

#pragma endregion

	default:
		return false;
	};

	return true;
}

HouseClass* TEventExtData::GetHouse(int TEvetValue, HouseClass* pEventHouse)
{
	if (TEvetValue <= -2)
		return pEventHouse;
	else if (TEvetValue >= 0 && size_t(TEvetValue) < HouseClass::Array->size())
		return HouseClass::Array->Items[TEvetValue];

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

	if (RulesExtData::Instance()->AITargetTypesLists.empty()
		|| size_t(desiredListIdx) >= RulesExtData::Instance()->AITargetTypesLists.size()
		|| RulesExtData::Instance()->AITargetTypesLists[desiredListIdx].empty())
		return false;

	bool found = false;

	if (auto const pTechno = flag_cast_to<TechnoClass*, false>(pObject)) {
		auto const pTechnoType = pTechno->GetTechnoType();

		for (const auto& pDesiredItem : RulesExtData::Instance()->AITargetTypesLists[desiredListIdx]) {
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
			auto const pTechnoType = pTechno->GetTechnoType();

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

	auto pHouse = HouseClass::FindByIndex(pThis->Value);
	if (!pHouse)
		return false;

	return pHouse->CountOwnedNow(pType) > 0;
}

bool TEventExtData::HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis)
{
	return !TEventExtData::HouseOwnsTechnoTypeTEvent(pThis);
}

bool TEventExtData::HousesAreDestroyedTEvent(TEventClass* pThis)
{
	const int nIdxVariable = pThis->Value; //atoi(pThis->String);

	const auto& nHouseList = RulesExtData::Instance()->AIHousesLists;

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
		if (techno->GetTechnoType() == targetType) {
			foundCount++;
			if (shouldExist && foundCount >= evt->Value) {
				return true;
			}
		}
	}

	return shouldExist ? false : (foundCount == 0);
}

bool HandleEntryEvents(TEventClass* evt, TriggerEvent event, ObjectClass* obj, bool* bool1)
{
	if (event != evt->EventKind) {
		return false;
	}

	auto country = AresTEventExt::ResolveHouseParam(evt->Value);

	if (!country) {
		return false;
	}

	if (!obj) {
		return false;
	}

	if (evt->Value != -1) {
		if (obj->GetOwningHouseIndex() != country->ArrayIndex) {
			return false;
		}
	}

	*bool1 = 1;
	evt->House = obj->GetOwningHouse();
	return true;
}

bool HandleSpyAsHouse(TEventClass* evt, TriggerEvent event, ObjectClass* obj, bool* bool1)
{
	if (event != TriggerEvent::SpyAsHouse || !obj) {
		return false;
	}

	auto country = AresTEventExt::ResolveHouseParam(evt->Value);

	if (!country) {
		return false;
	}

	HouseClass* showAsHouse = obj->GetDisguiseHouse(true);
	if (!showAsHouse) {
		return false;
	}

	if (showAsHouse->ArrayIndex == country->ArrayIndex) {
		*bool1 = 1;
		return true;
	}

	return false;
}

bool HandleSpyAsInfantry(TEventClass* evt, TriggerEvent event, ObjectClass* obj, bool* bool1)
{
	if (event != TriggerEvent::SpyAsInfantry || !obj) {
		return false;
	}

	AbstractClass* showAsType = obj->GetDisguise(true);
	if (showAsType->WhatAmI() != InfantryTypeClass::AbsID) {
		return false;
	}

	if (evt->Value != -1 && showAsType->GetArrayIndex() == evt->Value) {
		*bool1 = 1;
		return true;
	}

	return false;
}

bool HandleNearWaypoint(TEventClass* evt, TriggerEvent event, ObjectClass* obj)
{
	if (event != TriggerEvent::ComesNearWaypoint) {
		return false;
	}

	CoordStruct waypointCoord;
	ScenarioClass::Instance->GetWaypointCoordinate(&waypointCoord, evt->Value);
	CoordStruct objCoord = obj->GetCoords();

	return (objCoord - waypointCoord).Length() <= 1280;
}

bool CheckNoFactories(HouseClass* house)
{
	if (house->OwnedBuildings <= 0)
	{
		return true;
	}

	for (int i = 0; i < house->Buildings.Count; ++i)
	{
		BuildingClass* building = house->Buildings.Items[i];
		if (building && !building->InLimbo && building->Type->Factory != AbstractType::None)
		{
			return false;
		}
	}

	return true;
}

bool CheckLeavesMap(TEventClass* evt, bool* bool1)
{
	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		TeamClass* team = TeamClass::Array->Items[i];
		if (team->Type == evt->TeamType && !team->FirstUnit && team->IsLeavingMap)
		{
			*bool1 = 1;
			return true;
		}
	}

	return false;
}

bool HandleHouseEvents(TEventClass* evt, HouseClass* house, bool* bool1)
{
	switch (evt->EventKind)
	{
	case TriggerEvent::CreditsExceed:
		return house->Available_Money() >= evt->Value;

	case TriggerEvent::DestroyedBuildingsNum:
		return house->TotalKilledBuildings >= evt->Value;

	case TriggerEvent::DestroyedUnitsNum:
		return house->TotalKilledUnits >= evt->Value;

	case TriggerEvent::NoFactoriesLeft:
		return CheckNoFactories(house);

	case TriggerEvent::CiviliansEvacuated:
		return house->CiviliansEvacuated;

	case TriggerEvent::BuildBuildingType:
		if (house->LastBuiltBuildingType == evt->Value)
		{
			*bool1 = 1;
			return true;
		}
		break;

	case TriggerEvent::BuildUnitType:
		if (house->LastBuiltVehicleType == evt->Value)
		{
			*bool1 = 1;
			return true;
		}
		break;

	case TriggerEvent::BuildInfantryType:
		if (house->LastBuiltInfantryType == evt->Value)
		{
			*bool1 = 1;
			return true;
		}
		break;

	case TriggerEvent::BuildAircraftType:
		if (house->LastBuiltAircraftType == evt->Value)
		{
			*bool1 = 1;
			return true;
		}
		break;

	case TriggerEvent::TeamLeavesMap:
		return CheckLeavesMap(evt, bool1);

	case TriggerEvent::BuildingExists:
		if (house->ActiveBuildingTypes.get_count(evt->Value))
		{
			*bool1 = 1;
			return true;
		}
		break;

	case TriggerEvent::CreditsBelow:
		return house->Available_Money() <= evt->Value;

	case TriggerEvent::BuildingDoesNotExist:
		if (!house->ActiveBuildingTypes.get_count(evt->Value)) {
			*bool1 = 1;
			return true;
		}
		break;
	default:
		return true;
	}

	return false;
}

bool HandleValue2HouseEvents(TEventClass* evt)
{
	HouseClass* targetHouse = AresTEventExt::ResolveHouseParam(evt->Value);

	// continue normally if a house was found or this isn't Player@X logic,
	// otherwise return false directly so events don't fire for non-existing
	// players.
	if(targetHouse || !HouseClass::Index_IsMP(evt->Value)){
		switch (evt->EventKind)
		{
		case TriggerEvent::ThievedBy:
			return targetHouse->HasBeenThieved;

		case TriggerEvent::HouseDiscovered:
			return targetHouse->DiscoveredByPlayer;

		case TriggerEvent::DestroyedUnitsAll:
			return targetHouse->ActiveUnitTypes.total() <= 0 &&
				targetHouse->ActiveInfantryTypes.total() <= 0;

		case TriggerEvent::DestroyedBuildingsAll:
			return targetHouse->OwnedBuildings <= 0;

		case TriggerEvent::DestroyedAll:
		{
			if (SessionClass::IsCampaign()) {
				if (targetHouse->ActiveInfantryTypes.total() <= 0) {
					for (auto& bld : targetHouse->Buildings) {
						if (bld->Type->CanBeOccupied && bld->Occupants.Count > 0)
							return false;
					}
				}

				if (targetHouse->ActiveAircraftTypes.total() > 0)
					return false;

				if (targetHouse->ActiveInfantryTypes.total() > 0)
					return false;

				for (auto pItem : *InfantryClass::Array) {
					if (pItem->InLimbo && targetHouse == pItem->GetOwningHouse() && targetHouse->IsAlliedWith(pItem->Transporter))
						return false;
				}

				return true;
			}

			return false;
		}
		case TriggerEvent::LowPower:
			return targetHouse->GetPowerPercentage() < 1.0;

		case TriggerEvent::DestroyedUnitsNaval:
			return targetHouse->OwnedNavy <= 0;

		case TriggerEvent::DestroyedUnitsLand:
			return (targetHouse->OwnedUnits - targetHouse->OwnedNavy) <= 0 &&
				targetHouse->OwnedInfantry <= 0;

		case TriggerEvent::PowerFull:
			return targetHouse->GetPowerPercentage() >= 1.0;

		default:
			break;
		}

		return true;
	}

	return false;
}

bool HandleDefaultEvents(
		TEventClass* evt,
		TriggerEvent event,
		HouseClass* house,
		ObjectClass* obj,
		bool* bool1,
		AbstractClass* source)
{

	// Constexpr lookup table for events that require exact event matching
	static constexpr bool RequiresEventMatch[static_cast<int>(TriggerEvent::count)] = {
		false, true,  true,  true,  true,  false, true,  true,  false, false, // 0-9
		false, false, false, false, false, false, false, false, true,  true,  // 10-19
		true,  true,  true,  true,  true,  true,  true,  false, false, true,  // 20-29
		false, true,  false, true,  true,  true,  false, false, true,  true,  // 30-39
		true,  true,  true,  true,  true,  false, false, false, true,  true,  // 40-49
		true,  false, false, true,  true,  false, false, false, false, true,  // 50-59
		false, false                                                           // 60-61
	};

	const int typeIndex = static_cast<int>(evt->EventKind);
	if (typeIndex >= 0 && typeIndex < static_cast<int>(TriggerEvent::count)) {
		if (RequiresEventMatch[typeIndex] && event != evt->EventKind && !Unsorted::ArmageddonMode()) {
			return false;
		}
	}

	// Handle specific event types
	switch (evt->EventKind)
	{
	case TriggerEvent::EnteredBy:
	case TriggerEvent::CrossesHorizontalLine:
	case TriggerEvent::CrossesVerticalLine:
	case TriggerEvent::ZoneEntryBy:
	case TriggerEvent::EnteredOrOverflownBy:
		return HandleEntryEvents(evt, event, obj, bool1);

	case TriggerEvent::SpyAsHouse:
		return HandleSpyAsHouse(evt, event, obj, bool1);

	case TriggerEvent::SpyAsInfantry:
		return HandleSpyAsInfantry(evt, event, obj, bool1);

	case TriggerEvent::ComesNearWaypoint:
		return HandleNearWaypoint(evt, event, obj);

	case TriggerEvent::AttackedByHouse:{
		if (event != TriggerEvent::AttackedByHouse || !source) {
			return false;
		}

		int param = evt->Value;
		// convert Player @ X to real index
		if (HouseClass::Index_IsMP( evt->Value)) {
			auto const pPlayer = AresTEventExt::ResolveHouseParam( evt->Value);
			param = pPlayer ? pPlayer->ArrayIndex : -1;
		}

		if (param != source->GetOwningHouse()->ArrayIndex){
			return false;
		}

		return true;
	}
	default:
		break;
	}

	// Handle house-specific events
	if (house)
	{
		if (HandleHouseEvents(evt, house, bool1))
		{
			return true;
		}
	}

	// Handle Value2 house events
	return HandleValue2HouseEvents(evt);
}

bool FakeTEventClass::_Occured(TriggerEvent event, HouseClass* house, ObjectClass* obj, CDTimerClass* td, bool* bool1, AbstractClass* source)
{
	if(this->EventKind == TriggerEvent::None)
		return false;

	bool result = false;

	EventArgs args {
		event, house, obj, td, bool1, source
	};

	if (TEventExtData::Occured(this, args, result)) {
		return result;
	}

	if (AresTEventExt::HasOccured(this, args, result)) {
		return result;
	}

	switch (this->EventKind)
	{
	case TriggerEvent::ElapsedTime:
	case TriggerEvent::RandomDelay:
		return td->Expired();

	case TriggerEvent::MissionTimerExpired:
		return ScenarioClass::Instance->MissionTimer.Expired();

	case TriggerEvent::GlobalSet:
		ScenarioClass::Instance->GetGlobalVarValue_ptr(this->Value, bool1);
		return bool1 != 0;

	case TriggerEvent::GlobalCleared:
		ScenarioClass::Instance->GetGlobalVarValue_ptr(this->Value, bool1);
		return bool1 == 0;

	case TriggerEvent::LocalSet:
		ScenarioClass::Instance->GetLocalVarValue_ptr(this->Value, bool1);
		return bool1 != 0;

	case TriggerEvent::LocalCleared:
		ScenarioClass::Instance->GetLocalVarValue_ptr(this->Value, bool1);
		return bool1 == 0;

	case TriggerEvent::AmbientLightBelow:
		return ScenarioClass::Instance->AmbientCurrent <= this->Value;

	case TriggerEvent::AmbientLightAbove:
		return ScenarioClass::Instance->AmbientCurrent >= this->Value;

	case TriggerEvent::ElapsedScenarioTime:
		return this->Value <= Unsorted::CurrentFrame() / 15;

	case TriggerEvent::TechTypeExists:
	{
		return AresTEventExt::FindTechnoType(this, this->Value, nullptr);
	}
	case TriggerEvent::TechTypeDoesntExist:
	{
		return !AresTEventExt::FindTechnoType(this, 1, nullptr);
	}
	default:
		return HandleDefaultEvents(this, event, house, obj, bool1, source);
	}

}

ASMJIT_PATCH(0x71F9C0, TEventClass_Persistable, 6)
{
	GET(TEventClass*, pThis, ECX);

	switch (pThis->EventKind) {
		case TriggerEvent::SpiedBy:
		case TriggerEvent::SpyAsHouse:
		case TriggerEvent::SpyAsInfantry:
		{
			R->EAX(false);
			return 0x71F9DF;
		};
	}

	std::pair<bool, bool> result =
		AresTEventExt::GetPersistableFlag((AresTriggerEvents)pThis->EventKind);

	if (!result.second)
		result = TEventExtData::GetPersistableFlag((PhobosTriggerEvent)pThis->EventKind);

	if (!result.second)
		return 0x0;

	R->EAX(result.first);
	return 0x71F9DF;
}

ASMJIT_PATCH(0x71F39B, TEventClass_SaveToINI, 5)
{
	GET(AresTriggerEvents, nAction, EDX);

	std::pair<LogicNeedType, bool > result = AresTEventExt::GetLogicNeed(nAction);

	if (!result.second)
		result = TEventExtData::GetLogicNeed((PhobosTriggerEvent)nAction);

	if (!result.second)
		return (int)nAction > 61 ? 0x71F3FC : 0x71F3A0;

	R->EAX(result.first);
	return 0x71F3FE;
}

ASMJIT_PATCH(0x71f683, TEventClass_GetFlags, 5)
{
	GET(AresTriggerEvents, nAction, ECX);

	std::pair<TriggerAttachType, bool> result = AresTEventExt::GetAttachFlags(nAction);

	if (!result.second)
		result = TEventExtData::GetTriggetAttach((PhobosTriggerEvent)nAction);

	if (result.second)
	{
		R->EAX(result.first);
		return 0x71F6F6;
	}

	return (int)nAction > 59 ? 0x71F69C : 0x71F688;
}

DEFINE_FUNCTION_JUMP(CALL , 0x726540, FakeTEventClass::_Occured)
// =============================
// container
TEventExtContainer TEventExtContainer::Instance;
std::vector<TEventExtData*> Container<TEventExtData>::Array;
void Container<TEventExtData>::Clear()
{
	Array.clear();
}

bool TEventExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool TEventExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks
//

ASMJIT_PATCH(0x71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);

	TEventExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x71E856, TEventClass_SDDTOR, 0x6)
{
	GET(TEventClass*, pItem, ESI);
	TEventExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x71FAA6, TEventClass_SDDTOR, 0x6) // Factory


ASMJIT_PATCH(0x71F58B, TEventClass_ReadINI_MaskedTEvents, 0x7)
{
	REF_STACK(TEventClass*, pThis, 0x4);

	switch (static_cast<PhobosTriggerEvent>(pThis->EventKind))
	{
	case PhobosTriggerEvent::EnteredByByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::EnteredBy;
		break;
	case PhobosTriggerEvent::SpiedByByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::SpiedBy;
		break;
	case PhobosTriggerEvent::HouseDiscoveredByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::HouseDiscovered;
		break;
	case PhobosTriggerEvent::DestroyedUnitsAllByID:
		pThis->Value = UnitTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsAll;
		break;
	case PhobosTriggerEvent::DestroyedBuildingsAllByID:
		pThis->Value = BuildingTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedBuildingsAll;
		break;
	case PhobosTriggerEvent::DestroyedAllByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedAll;
		break;
	case PhobosTriggerEvent::BuildBuildingTypeByID:
		pThis->Value = BuildingTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildBuildingType;
		break;
	case PhobosTriggerEvent::BuildUnitTypeByID:
		pThis->Value = UnitTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildUnitType;
		break;
	case PhobosTriggerEvent::BuildInfantryTypeByID:
		pThis->Value = InfantryTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildInfantryType;
		break;
	case PhobosTriggerEvent::BuildAircraftTypeByID:
		pThis->Value = AircraftTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildAircraftType;
		break;
	case PhobosTriggerEvent::ZoneEntryByByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::ZoneEntryBy;
		break;
	case PhobosTriggerEvent::CrossesHorizontalLineByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::CrossesHorizontalLine;
		break;
	case PhobosTriggerEvent::CrossesVerticalLineByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::CrossesVerticalLine;
		break;
	case PhobosTriggerEvent::LowPowerByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::LowPower;
		break;
	case PhobosTriggerEvent::BuildingExistsByID:
		pThis->Value = BuildingTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildingExists;
		break;
	case PhobosTriggerEvent::AttackedByHouseByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::AttackedByHouse;
		break;
	case PhobosTriggerEvent::SpyAsHouseByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::SpyAsHouse;
		break;
	case PhobosTriggerEvent::SpyAsInfantryByID:
		pThis->Value = InfantryTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::SpyAsInfantry;
		break;
	case PhobosTriggerEvent::DestroyedUnitsNavalByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsNaval;
		break;
	case PhobosTriggerEvent::DestroyedUnitsLandByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::DestroyedUnitsLand;
		break;
	case PhobosTriggerEvent::BuildingDoesNotExistByID:
		pThis->Value = BuildingTypeClass::FindIndexById(pThis->String);
		pThis->EventKind = TriggerEvent::BuildingDoesNotExist;
		break;
	case PhobosTriggerEvent::PowerFullByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::PowerFull;
		break;
	case PhobosTriggerEvent::EnteredOrOverflownByByID:
		pThis->Value = HouseTypeClass::FindIndexByIdAndName(pThis->String);
		pThis->EventKind = TriggerEvent::EnteredOrOverflownBy;
		break;

	default:
		break;
	}

	return 0;
}