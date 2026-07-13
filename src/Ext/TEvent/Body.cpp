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

// the function return is deciding if the case is handled or not
// the bool result pointer is for the result of the Event itself
bool TEventExtData::HasOccured(TEventClass* pThis, EventArgs& Args, bool& result)
{
	const AresTriggerEvents TEventKind = (AresTriggerEvents)pThis->EventKind;
	const AresTriggerEvents ExecutedKind = (AresTriggerEvents)Args.EventType;
	// They must be the same, but for other triggers to take effect normally, this cannot be judged outside case.
	const auto isSameEvent = [&]() { return TEventKind == ExecutedKind; };

	{
		switch (TEventKind)
		{
		case AresTriggerEvents::UnderEMP:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent() && pTechno->EMPLockRemaining > 0;
			return true;
		}
		case AresTriggerEvents::UnderEMP_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value
				&& pTechno->EMPLockRemaining > 0;

			return true;
		}
		case AresTriggerEvents::RemoveEMP:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent() && pTechno->EMPLockRemaining <= 0;
			return true;
		}
		case AresTriggerEvents::RemoveEMP_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value
				&& pTechno->EMPLockRemaining <= 0;
		}
		case AresTriggerEvents::EnemyInSpotlightNow:
		{
			result = true;
			return true;
		}
		case AresTriggerEvents::DriverKiller:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent();

			return true;
		}
		case AresTriggerEvents::DriverKilled_ByHouse:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::VehicleTaken:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent();

			return true;
		}
		case AresTriggerEvents::VehicleTaken_ByHouse:
		{
			result = flag_cast_to<FootClass*>(Args.Object)
				&& isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::Abducted:
		case AresTriggerEvents::AbductSomething:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno && isSameEvent();
			return true;
		}
		case AresTriggerEvents::Abducted_ByHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& (flag_cast_to<TechnoClass*>(Args.Source)
				&& ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value);

			return true;
		}
		case AresTriggerEvents::AbductSomething_OfHouse:
		{
			const auto pTechno = flag_cast_to<TechnoClass*>(Args.Object);
			result = pTechno
				&& isSameEvent()
				&& (cast_to<HouseClass*>(Args.Source)
				&& ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value);

			return true;

		}
		case AresTriggerEvents::SuperActivated:
		case AresTriggerEvents::SuperDeactivated:
		{
			result = isSameEvent()
				&& Args.Source
				&& Args.Source->WhatAmI() == AbstractType::Super
				&& ((SuperClass*)Args.Source)->Type->ArrayIndex == pThis->Value;

			return true;
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
					return true;
				}
			}

			result = false;
			return true;
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

			return true;
		}
		case AresTriggerEvents::ReverseEngineerAnything:
		{
			result = isSameEvent();
			return true;
		}
		case AresTriggerEvents::ReverseEngineerType:
		{
			result = GET_TECHNOTYPE(((TechnoClass*)Args.Source)) == TEventExtContainer::Instance.Find(pThis)->GetTechnoType();
			return true;
		}
		case AresTriggerEvents::HouseOwnTechnoType:
		{
			result = FindTechnoType(pThis, pThis->Value, Args.Owner);
			return true;
		}
		case AresTriggerEvents::HouseDoesntOwnTechnoType:
		{
			result = !FindTechnoType(pThis, pThis->Value + 1, Args.Owner);
			return true;
		}
		case AresTriggerEvents::AttackedOrDestroyedByAnybody:
		{
			result = isSameEvent();
			return true;
		}
		case AresTriggerEvents::AttackedOrDestroyedByHouse:
		{
			result = isSameEvent()
				&& Args.Source
				&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::DestroyedByHouse:
		{
			result = isSameEvent()
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			return true;
		}
		case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
		{
			result = FindTechnoType(pThis, pThis->Value + 1, nullptr);
			return true;
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
			return true;
		}
		default:
			break;
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

	if (RulesExtData::Instance()->AITargetTypesLists.empty()
		|| size_t(desiredListIdx) >= RulesExtData::Instance()->AITargetTypesLists.size()
		|| RulesExtData::Instance()->AITargetTypesLists[desiredListIdx].empty())
		return false;

	bool found = false;

	if (auto const pTechno = flag_cast_to<TechnoClass*, false>(pObject)) {
		auto const pTechnoType = GET_TECHNOTYPE(pTechno);

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
		if (GET_TECHNOTYPE(techno) == targetType) {
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

	if (!obj) {
		return false;
	}

	if (evt->Value != -1) {
		auto country = TEventExtData::ResolveHouseParam(evt->Value);

		if (!country) {
			return false;
		}

		if (obj->GetOwningHouseIndex() != country->ArrayIndex) {
			return false;
		}
	}

	*bool1 = 1;
	evt->House = obj->GetOwningHouse();
	return true;
}

// accept 53 and 54
// 0x71ECE1, TriggerClass_SpyAsInfantryOrHouse, 0x8
bool HandleSpyAsHouse(TEventClass* evt, TriggerEvent event, ObjectClass* obj, bool* bool1)
{
	if ((event != TriggerEvent::SpyAsInfantry && event != TriggerEvent::SpyAsHouse) || !obj) {
		return false;
	}

	auto country = TEventExtData::ResolveHouseParam(evt->Value);

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

// accept 53 and 54
// 0x71ED5E, TriggerClass_SpyAsInfantryOrHouse, 0x8
bool HandleSpyAsInfantry(TEventClass* evt, TriggerEvent event, ObjectClass* obj, bool* bool1)
{
	if ((event != TriggerEvent::SpyAsInfantry && event != TriggerEvent::SpyAsHouse) || !obj) {
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
		if (building && 
			!building->InLimbo &&
			 building->Type->Factory != AbstractType::None)
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
	{
		auto pHouseExt = HouseExtContainer::Instance.Find(house);

		if (house->LastBuiltVehicleType == evt->Value 
			|| pHouseExt->LastBuiltNavalVehicleType == evt->Value)
		{
			*bool1 = 1;
			return true;
		}

		break;
	}
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
		break;
	}

	return false;
}

bool HandleValue2HouseEvents(TEventClass* evt)
{
	HouseClass* targetHouse = TEventExtData::ResolveHouseParam(evt->Value);

	// continue normally if a house was found or this isn't Player@X logic,
	// otherwise return false directly so events don't fire for non-existing
	// players.
	if(targetHouse){
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
	}else if (HouseClass::Index_IsMP(evt->Value))
	{
		return false;
	}

	return true;
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
	//53 and 54 are modified to accept both between
	static constexpr bool RequiresEventMatch[static_cast<int>(TriggerEvent::count)] = {
		false, true,  true,  true,  true,  false, true,  true,  false, false, // 0-9
		false, false, false, false, false, false, false, false, true,  false, // 10-19
		false, false, false, true,  true,  true,  true,  false, false, true,  // 20-29
		false, true,  false, true,  true,  true,  false, false, true,  true,  // 30-39
		true,  true,  true,  true,  true,  false, false, false, true,  true,  // 40-49
		true,  false, false, false,  false,  false, false, false, false, true,  // 50-59
		false, false                                                           // 60-61
	};

	const int typeIndex = static_cast<int>(evt->EventKind);
	if (typeIndex >= 0 && typeIndex < static_cast<int>(TriggerEvent::count)) {
		if (RequiresEventMatch[typeIndex] && event != evt->EventKind && !Unsorted::MAP_DEBUG_MODE()) {
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
			auto const pPlayer = TEventExtData::ResolveHouseParam( evt->Value);
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
		switch (evt->EventKind) {
			case TriggerEvent::CreditsExceed:
			case TriggerEvent::DestroyedBuildingsNum:
			case TriggerEvent::DestroyedUnitsNum:
			case TriggerEvent::NoFactoriesLeft:
			case TriggerEvent::CiviliansEvacuated:
			case TriggerEvent::BuildBuildingType:
			case TriggerEvent::BuildUnitType:
			case TriggerEvent::BuildInfantryType:
			case TriggerEvent::BuildAircraftType:
			case TriggerEvent::TeamLeavesMap:
			case TriggerEvent::BuildingExists:
			case TriggerEvent::CreditsBelow:
			case TriggerEvent::BuildingDoesNotExist:
				if (!HandleHouseEvents(evt, house, bool1))
					return false;
				break;
			default:
				break;
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

	if (TEventExtData::HasOccured(this, args, result)) {
		return result;
	}

	switch (this->EventKind)
	{
	case TriggerEvent::ElapsedTime:
	case TriggerEvent::RandomDelay:
	{
		// Pseudocode: if Started==-1 return (DelayTime==0); else return (Frame-Started >= DelayTime)
		// Expired() returns true for any non-ticking timer regardless of TimeLeft, which is wrong
		// when the timer hasn't started but still has time remaining.
		const int started = td->StartTime;
		const int delayTime = td->TimeLeft;

		if (started == -1)
			return delayTime == 0;

		const int elapsed = Unsorted::CurrentFrame() - started;
		if (elapsed < delayTime)
			return (delayTime - elapsed) == 0;

		return true;
	}
	case TriggerEvent::MissionTimerExpired:
	{
		// Pseudocode: if Started==-1 return false; else return (Frame-Started >= DelayTime)
		// Expired() returns true when not ticking, but original returns false when not started.
		auto& mt = ScenarioClass::Instance->MissionTimer;
		if (mt.StartTime == -1)
			return false;

		return (Unsorted::CurrentFrame() - mt.StartTime) >= mt.TimeLeft;

	}

	case TriggerEvent::GlobalSet:
		ScenarioClass::Instance->GetGlobalVarValue_ptr(this->Value, bool1);
		return *bool1;

	case TriggerEvent::GlobalCleared:
		ScenarioClass::Instance->GetGlobalVarValue_ptr(this->Value, bool1);
		return !*bool1;

	case TriggerEvent::LocalSet:
		ScenarioClass::Instance->GetLocalVarValue_ptr(this->Value, bool1);
		return *bool1;

	case TriggerEvent::LocalCleared:
		ScenarioClass::Instance->GetLocalVarValue_ptr(this->Value, bool1);
		return !*bool1;

	case TriggerEvent::AmbientLightBelow:
		return ScenarioClass::Instance->AmbientCurrent <= this->Value;

	case TriggerEvent::AmbientLightAbove:
		return ScenarioClass::Instance->AmbientCurrent >= this->Value;

	case TriggerEvent::ElapsedScenarioTime:
		return this->Value <= Unsorted::CurrentFrame() / 15;

	case TriggerEvent::TechTypeExists:
	{
		return TEventExtData::FindTechnoType(this, this->Value, nullptr);
	}
	case TriggerEvent::TechTypeDoesntExist:
	{
		if (!TEventExtContainer::Instance.Find(this)->GetTechnoType())
			return false; // type not defined in game -> never "doesn't exist" event

		return !TEventExtData::FindTechnoType(this, 1, nullptr);
	}
	default:
		return HandleDefaultEvents(this, event, house, obj, bool1, source);
	}
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