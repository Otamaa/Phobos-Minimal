#include "Body.h"

#include <Utilities/SavegameDef.h>

#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>

#include <Ext/Script/Body.h>

#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

#include <New/Entity/ShieldClass.h>

// =============================
// load / save

template <typename T>
void TEventExt::ExtData::Serialize(T& Stm)
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

		_NODISCARD constexpr bool operator()(const _Ty& _Left, const _Ty& _Right) const
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
TechnoTypeClass* TEventExt::ExtData::GetTechnoType()
{
	if (this->TechnoType.empty())
	{
		const char* eventTechno = this->OwnerObject()->String;
		TechnoTypeClass* pType = TechnoTypeClass::Find(eventTechno);

		if (!pType)
		{
			Debug::Log("Event references non-existing techno type \"%s\".", eventTechno);
		}

		this->TechnoType = pType;
	}

	return this->TechnoType;
}

bool TEventExt::Occured(TEventClass* pThis, EventArgs const& args, bool& result)
{
	//int iEvent = args.EventType; // not used here ,.. ares using it compare
	//HouseClass* pHouse = args.Owner;
	//ObjectClass* pObject = args.Object;
	//CDTimerClass* pTimer = args.ActivationFrame;
	//bool* isPersitant = args.isRepeating;
	//AbstractClass* pSource = args.Source;

	if ((PhobosTriggerEvent)pThis->EventKind < PhobosTriggerEvent::LocalVariableGreaterThan)
		return false;

	switch ((PhobosTriggerEvent)pThis->EventKind)
	{
#pragma region LovalVariableManipulation
	case PhobosTriggerEvent::LocalVariableGreaterThan:
		result = TEventExt::VariableCheck<false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThan:
		result = TEventExt::VariableCheck<false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsTo:
		result = TEventExt::VariableCheck<false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		result = TEventExt::VariableCheck<false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		result = TEventExt::VariableCheck<false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		result = TEventExt::VariableCheck<false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
		result = TEventExt::VariableCheck<true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThan:
		result = TEventExt::VariableCheck<true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
		result = TEventExt::VariableCheck<true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		result = TEventExt::VariableCheck<true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		result = TEventExt::VariableCheck<true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		result = TEventExt::VariableCheck<true, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		result = TEventExt::VariableCheckBinary<false, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		result = TEventExt::VariableCheckBinary<false, true, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		result = TEventExt::VariableCheckBinary<true, true, std::and_with<int>>(pThis);
		break;
#pragma endregion

	/*
	*	- PersistableFlag ?
	*	- LogcNeed ?
	*   - AttachFlags ?
	*/
	case PhobosTriggerEvent::ShieldBroken:
		result = ShieldClass::TEventIsShieldBroken(args.Object);
		break;
	case PhobosTriggerEvent::HouseOwnsTechnoType:
		result = TEventExt::HouseOwnsTechnoTypeTEvent(pThis);
		break;
	case PhobosTriggerEvent::HouseDoesntOwnTechnoType:
		result = TEventExt::HouseDoesntOwnTechnoTypeTEvent(pThis);
		break;
	case PhobosTriggerEvent::HousesDestroyed:
		result = TEventExt::HousesAreDestroyedTEvent(pThis);
		break;
	default:
		return false;
	};

	return true;
}

template<bool IsGlobal, class _Pr>
bool TEventExt::VariableCheck(TEventClass* pThis)
{
	const auto nVar = ScenarioExt::GetVariables(IsGlobal);
	auto itr = nVar->find(pThis->Value);

	if (itr != nVar->end())
	{
		// We uses TechnoName for our operator number
		int nOpt = atoi(pThis->String);
		return _Pr()(itr->second.Value, nOpt);
	}

	return false;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
bool TEventExt::VariableCheckBinary(TEventClass* pThis)
{
	const auto nVar = ScenarioExt::GetVariables(IsGlobal);
	const auto itr = nVar->find(pThis->Value);

	if (itr != nVar->end())
	{
		// We uses TechnoName for our src variable index
		int nSrcVariable = atoi(pThis->String);
		auto itrsrc = nVar->find(nSrcVariable);

		if (itrsrc != nVar->end())
			return _Pr()(itr->second.Value, itrsrc->second.Value);
	}

	return false;
}

bool TEventExt::HouseOwnsTechnoTypeTEvent(TEventClass* pThis)
{
	auto pType = TechnoTypeClass::Find(pThis->String);
	if (!pType)
		return false;

	auto pHouse = HouseClass::FindByIndex(pThis->Value);
	if (!pHouse)
		return false;

	return pHouse->CountOwnedNow(pType) > 0;
}

bool TEventExt::HouseDoesntOwnTechnoTypeTEvent(TEventClass* pThis)
{
	return !TEventExt::HouseOwnsTechnoTypeTEvent(pThis);
}

bool TEventExt::HousesAreDestroyedTEvent(TEventClass* pThis)
{
	const int nIdxVariable = pThis->Value; //atoi(pThis->String);

	const auto& nHouseList = RulesExt::Global()->AIHousesLists;

	if (nHouseList.empty() || (size_t)nIdxVariable >= nHouseList.size())
	{
		Debug::Log("Map event %d: [AIHousesList] is empty. This event can't continue.\n", (int)pThis->EventKind);
		return false;
	}

	const auto housesList = Iterator(nHouseList[nIdxVariable]);

	if (housesList.empty())
	{
		Debug::Log("Map event %d: [AIHousesList](%d) is empty. This event can't continue.\n", (int)pThis->EventKind, nIdxVariable);
		return false;
	}

	for (auto pTechno : *TechnoClass::Array)
	{
		if (ScriptExt::IsUnitAvailable(pTechno, false))
		{
			if (pTechno->Owner && housesList.contains(pTechno->Owner->Type))
				return false;
		}
	}

	return true;
}

// =============================
// container
TEventExt::ExtContainer TEventExt::ExtMap;

// =============================
// container hooks
//

DEFINE_HOOK(0x71E7F8, TEventClass_CTOR, 5)
{
	GET(TEventClass*, pItem, ESI);

	TEventExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71FAA6, TEventClass_SDDTOR, 0x6) // Factory
DEFINE_HOOK(0x71E856, TEventClass_SDDTOR, 0x6)
{
	GET(TEventClass*, pItem, ESI);
	TEventExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71F930, TEventClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x71F8C0, TEventClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TEventClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TEventExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71F92B, TEventClass_Load_Suffix, 0x5)
{
	TEventExt::ExtMap.LoadStatic();
	return 0x0;
}

DEFINE_HOOK(0x71F94A, TEventClass_Save_Suffix, 0x5)
{
	TEventExt::ExtMap.SaveStatic();
	return 0x0;
}

//DEFINE_HOOK(0x71F811, TEventClass_Detach, 0x5)
//{
//	GET(TEventClass*, pThis, ECX);
//	GET(void*, pTarget, EDX);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	if (pThis->TeamType == pTarget) {
//		pThis->TeamType = nullptr;
//	}
//
//	TEventExt::ExtMap.InvalidatePointerFor(pThis, pTarget, bRemoved);
//
//	return 0x71F81D;
//}