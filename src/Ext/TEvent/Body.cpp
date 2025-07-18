#include "Body.h"

#include <Utilities/SavegameDef.h>

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

#include <TeamTypeClass.h>

// =============================
// load / save

template <typename T>
void TEventExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		;
	//Stm;
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
		const char* eventTechno = this->AttachedToObject->String;
		TechnoTypeClass* pType = TechnoTypeClass::Find(eventTechno);

		if (!pType)
		{
			Debug::LogInfo("Event{}] with Team[{} - {}] references non-existing techno type \"%s\".",
				(void*)this->AttachedToObject,
				this->AttachedToObject->TeamType ? this->AttachedToObject->TeamType->ID : GameStrings::NoneStr(),
				(void*)this->AttachedToObject->TeamType,
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

	if (PhobosAEFunctions::HasAttachedEffects(pTechno, pTypeAttached, false, false, nullptr, nullptr, nullptr, nullptr))
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

// =============================
// container
TEventExtContainer TEventExtContainer::Instance;

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

#include <Misc/Hooks.Otamaa.h>

HRESULT __stdcall FakeTEventClass::_Load(IStream* pStm)
{

	TEventExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TEventClass::Load(pStm);

	if (SUCCEEDED(res))
		TEventExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeTEventClass::_Save(IStream* pStm, bool clearDirty)
{

	TEventExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TEventClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		TEventExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F558C, FakeTEventClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5590, FakeTEventClass::_Save)