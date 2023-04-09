#include "Body.h"

#include <Utilities/SavegameDef.h>

#include <Ext/Scenario/Body.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>

#include <New/Entity/ShieldClass.h>

TEventExt::ExtContainer TEventExt::ExtMap;

 void TEventExt::ExtData::InitializeConstants() { }

// =============================
// load / save

template <typename T>
void TEventExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TEventExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TEventClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TEventExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TEventClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// helper struct
namespace std {
	template <class _Ty = void>
	struct and_with {
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty _FIRST_ARGUMENT_TYPE_NAME;
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef _Ty _SECOND_ARGUMENT_TYPE_NAME;
		_CXX17_DEPRECATE_ADAPTOR_TYPEDEFS typedef bool _RESULT_TYPE_NAME;

		_NODISCARD constexpr bool operator()(const _Ty& _Left, const _Ty& _Right) const {
			return _Left & _Right;
		}
	};
	//struct and_with { bool operator()(int a, int b) { return a & b; } };
}

bool TEventExt::Occured(TEventClass* pThis, EventArgs const& args , bool& bReturn)
{
	//int iEvent = args.EventType;
	//HouseClass* pHouse = args.Owner;
	//ObjectClass* pObject = args.Object;
	//CDTimerClass* pTimer = args.ActivationFrame;
	//bool* isPersitant = args.isRepeating;
	//AbstractClass* pSource = args.Source;

	switch (static_cast<PhobosTriggerEvent>(pThis->EventKind))
	{
	case PhobosTriggerEvent::LocalVariableGreaterThan:
		bReturn = TEventExt::VariableCheck<false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThan:
		bReturn = TEventExt::VariableCheck<false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsTo:
		bReturn = TEventExt::VariableCheck<false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsTo:
		bReturn = TEventExt::VariableCheck<false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsTo:
		bReturn = TEventExt::VariableCheck<false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrue:
		bReturn = TEventExt::VariableCheck<false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThan:
		bReturn = TEventExt::VariableCheck<true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThan:
		bReturn = TEventExt::VariableCheck<true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsTo:
		bReturn = TEventExt::VariableCheck<true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsTo:
		bReturn = TEventExt::VariableCheck<true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsTo:
		bReturn = TEventExt::VariableCheck<true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrue:
		bReturn = TEventExt::VariableCheck<true, std::and_with<int>>(pThis);
		break;

	case PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable:
		bReturn = TEventExt::VariableCheckBinary<false, true, std::and_with<int>>(pThis);
		break;

	case PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableGreaterThanOrEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableLessThanOrEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, false, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::greater<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::less<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::equal_to<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableGreaterThanOrEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::greater_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableLessThanOrEqualsToGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::less_equal<int>>(pThis);
		break;
	case PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable:
		bReturn = TEventExt::VariableCheckBinary<true, true, std::and_with<int>>(pThis);
		break;
	case PhobosTriggerEvent::ShieldBroken:
		bReturn = ShieldClass::TEventIsShieldBroken(args.Object);
		break;
	default:
		bReturn = false;
		return false;
	};

	return true;
}

template<bool IsGlobal, class _Pr>
bool TEventExt::VariableCheck(TEventClass* pThis)
{
	auto const& nVar = ScenarioExt::GetVariables(IsGlobal);
	auto itr = nVar.find(pThis->Value);

	if (itr != nVar.end())
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
	auto const& nVar = ScenarioExt::GetVariables(IsGlobal);
	auto itr = nVar.find(pThis->Value);

	if (itr != nVar.end())
	{
		// We uses TechnoName for our src variable index
		int nSrcVariable = atoi(pThis->String);
		auto itrsrc = nVar.find(nSrcVariable);

		if (itrsrc != nVar.end())
			return _Pr()(itr->second.Value, itrsrc->second.Value);
	}

	return false;
}

// =============================
// container

TEventExt::ExtContainer::ExtContainer() : Container("TEventClass") { }
TEventExt::ExtContainer::~ExtContainer() = default;

bool TEventExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TEventExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container hooks
//

//TODO : ReadFrom INI

// Value0 is not instantiated yet !
// Ares hook at the end of the function
//DEFINE_HOOK(0x71E752, TEventClass_CTOR, 0x5)
//{
//	GET(TEventClass*, pItem, ESI);
//	pItem->Value = 0;
//	TEventExt::ExtMap.Allocate(pItem);
//	return 0;
//}

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

DEFINE_HOOK(0x71F91B, TEventClass_Load_Suffix, 0x9)
{
	GET(TEventClass*, pItem, ESI);

	SwizzleManagerClass::Instance->Swizzle((void**)&pItem->House);
	TEventExt::ExtMap.LoadStatic();
	
	return 0x71F929;
}

DEFINE_HOOK(0x71F944, TEventClass_Save_Suffix, 0x6)
{
	GET(HRESULT, nRest, EAX);

	if(SUCCEEDED(nRest)) {
		Debug::Log("%s Executed !  HRES : %d \n", __FUNCTION__,nRest);
		TEventExt::ExtMap.SaveStatic();
		return 0x71F948;
	}

	return 0x71F94A;
}

DEFINE_HOOK(0x71F811, TEventClass_Detach, 0x5)
{
	GET(TEventClass*, pThis, ECX);
	GET(void*, pTarget, EDX);
	GET_STACK(bool, bRemoved, 0x8);

	if (pThis->TeamType == pTarget)
	{
		pThis->TeamType = nullptr;
	}

	if (auto pExt = TEventExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(pTarget, bRemoved);

	return 0x71F81D;
}