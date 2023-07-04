#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Techno/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>
#include <Misc/AresData.h>

DEFINE_OVERRIDE_HOOK(0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA)
{
	GET(HouseClass*, House, EDI);
	GET(int, triggerSide, EAX);

	enum { Eligible = 0x41E8D7, NotEligible = 0x41E8A1 };

	if (!triggerSide)
	{
		return Eligible;
	}

	--triggerSide;
	return(triggerSide == House->SideIndex)
		? Eligible
		: NotEligible
		;
}

std::pair<TriggerAttachType, bool> AresGetFlag(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
	case AresNewTriggerAction::SetEVAVoice:
		return { TriggerAttachType::None , true };
	case AresNewTriggerAction::KillDriversOf:
	case AresNewTriggerAction::SetGroup:
		return { TriggerAttachType::Object , true };
	default:
		return { TriggerAttachType::None , false };
	}
}

DEFINE_OVERRIDE_HOOK(0x6E3EE0, TActionClass_GetFlags, 5)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = AresGetFlag(nAction);

	if (!Handled)
		return 0;

	R->EAX(SomeFlag);
	return 0x6E3EFE;
}

std::pair<LogicNeedType, bool> GetMode(AresNewTriggerAction nAction)
{
	switch (nAction)
	{
	case AresNewTriggerAction::AuxiliaryPower:
		return { LogicNeedType::NumberNSuper  , true };
	case AresNewTriggerAction::KillDriversOf:
		return { LogicNeedType::None , true };
	case AresNewTriggerAction::SetEVAVoice:
	case AresNewTriggerAction::SetGroup:
		return { LogicNeedType::Number, true };
	default:
		return { LogicNeedType::None , false };
	}
}

DEFINE_OVERRIDE_HOOK(0x6E3B60, TActionClass_GetMode, 8)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = GetMode(nAction);
	if (Handled)
	{
		R->EAX(SomeFlag);
		return 0x6E3C4B;
	}
	else
	{
		R->EAX(((int)nAction) - 1);
		return ((int)nAction) > 0x8F ? 0x6E3C49 : 0x6E3B6E;
	}
}

enum class Persistable : unsigned int
{
	None = 0,
	unk_0x100 = 256,
	unk_0x101 = 257
};

// original game using between 0 - 2 ?
// why these were 256 257 ? , something not right ,..
std::pair<Persistable, bool> GetPersistableFlag(AresTriggerEvents nAction)
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
		return { Persistable::unk_0x100  , true };
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
		return { Persistable::unk_0x101  , true };
	default:
		return { Persistable::None  , false };
	}

}

DEFINE_OVERRIDE_HOOK(0x71F9C0, TEventClass_Persistable_AresNewTriggerEvents, 6)
{
	GET(TEventClass*, pThis, ECX);
	auto const& [Flag, Handled] =
		GetPersistableFlag((AresTriggerEvents)pThis->EventKind);
	if (!Handled)
		return 0x0;

	R->EAX(Flag);
	return 0x71F9DF;
}

std::pair<LogicNeedType, bool >  GetTEventLogcNeed(AresTriggerEvents nAction)
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

DEFINE_OVERRIDE_HOOK(0x71F39B, TEventClass_SaveToINI, 5)
{
	GET(AresTriggerEvents, nAction, EDX);
	const auto& [Logic, handled] = GetTEventLogcNeed(nAction);

	if (!handled)
		return (int)nAction > 61 ? 0x71F3FC : 0x71F3A0;

	R->EAX(Logic);
	return 0x71F3FE;
}

std::pair<bool, TriggerAttachType> GetTEventAttachFlags(AresTriggerEvents nEvent)
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
		return { true , TriggerAttachType::Object };
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
		return { true ,TriggerAttachType::House };
	}
	case AresTriggerEvents::TechnoTypeDoesntExistMoreThan:
	{
		return { true ,TriggerAttachType::Logic };
	}
	}


	return { false ,TriggerAttachType::None };
}

DEFINE_OVERRIDE_HOOK(0x71f683 , TEventClass_GetFlags_Ares, 5)
{
	GET(AresTriggerEvents, nAction, ECX);

	const auto& [handled , result] = GetTEventAttachFlags(nAction);
	if (handled) {
		R->EAX(result);
		return 0x71F6F6;
	}

	return (int)nAction > 59 ? 0x71F69C : 0x71F688;
}

// Resolves a param to a house.
HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse = nullptr)
{
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

// the general events requiring a house
DEFINE_OVERRIDE_HOOK(0x71F06C, EventClass_HasOccured_PlayerAtX1, 5)
{
	GET(int const, param, ECX);

	auto const pHouse = ResolveHouseParam(param);
	R->EAX(pHouse);

	// continue normally if a house was found or this isn't Player@X logic,
	// otherwise return false directly so events don't fire for non-existing
	// players.
	return (pHouse || !HouseClass::Index_IsMP(param)) ? 0x71F071u : 0x71F0D5u;
}

// validation for Spy as House, the Entered/Overflown Bys and the Crossed V/H Lines
DEFINE_OVERRIDE_HOOK_AGAIN(0x71ED33, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK_AGAIN(0x71F1C9, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK_AGAIN(0x71F1ED, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_OVERRIDE_HOOK(0x71ED01, EventClass_HasOccured_PlayerAtX2, 5)
{
	GET(int const, param, ECX);
	R->EAX(ResolveHouseParam(param));
	return R->Origin() + 5;
}

// param for Attacked by House is the array index
DEFINE_OVERRIDE_HOOK(0x71EE79, EventClass_HasOccured_PlayerAtX3, 9)
{
	GET(int, param, EAX);
	GET(HouseClass* const, pHouse, EDX);

	// convert Player @ X to real index
	if (HouseClass::Index_IsMP(param))
	{
		auto const pPlayer = ResolveHouseParam(param);
		param = pPlayer ? pPlayer->ArrayIndex : -1;
	}

	return (pHouse->ArrayIndex == param) ? 0x71EE82u : 0x71F163u;
}

#include <Ext/TEvent/Body.h>

namespace TEventExt_dummy
{
	bool FindTechnoType(TEventClass* pThis, int args, HouseClass* pWho)
	{
		const auto pType = TEventExt::ExtMap.Find(pThis)->GetTechnoType();
		if (!pType)
			return false;

		if (args <= 0)
			return false;

		if (!pType->Insignificant && !pType->DontScore)
		{
			HouseClass** arr = pWho ? &pWho : HouseClass::Array->Items;
			HouseClass** nEnd = &arr[pWho ? 1 : HouseClass::Array->Count];
			int i = args;

			for (HouseClass** nPos = &arr[0]; nPos != nEnd; ++nPos)
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
			switch (pType->WhatAmI())
			{
			case AbstractType::AircraftType:
			{
				for (auto pTechno : *AircraftClass::Array)
				{
					if (pWho && pWho != pTechno->Owner)
						continue;

					if (pTechno->GetTechnoType() == pType)
					{
						i--;

						if (i <= 0)
						{
							return true;
						}
					}
				}
				break;
			}
			case AbstractType::UnitType:
			{
				for (auto pTechno : *UnitClass::Array)
				{
					if (pWho && pWho != pTechno->Owner)
						continue;

					if (pTechno->GetTechnoType() == pType)
					{
						i--;

						if (i <= 0)
						{
							return true;
						}
					}
				}
				break;
			}
			case AbstractType::InfantryType:
			{
				for (auto pTechno : *InfantryClass::Array)
				{
					if (pWho && pWho != pTechno->Owner)
						continue;

					if (pTechno->GetTechnoType() == pType)
					{
						i--;

						if (i <= 0)
						{
							return true;
						}
					}
				}
				break;
			}
			case AbstractType::BuildingType:
			{
				for (auto pTechno : *BuildingClass::Array)
				{
					if (pWho && pWho != pTechno->Owner)
						continue;

					if (pTechno->GetTechnoType() == pType)
					{
						i--;

						if (i <= 0)
						{
							return true;
						}
					}
				}
				break;
			}
			}
		}

		return false;
	}

	// the function return is deciding if the case is handled or not
	// the bool result pointer is for the result of the Event itself
	bool NOINLINE HasOccured(TEventClass* pThis, EventArgs const Args, bool& result)
	{
		switch ((AresTriggerEvents)Args.EventType)
		{
		case AresTriggerEvents::UnderEMP:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& ((TechnoClass*)Args.Object)->EMPLockRemaining > 0;

			break;
		}
		case AresTriggerEvents::UnderEMP_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source && ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::RemoveEMP:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& ((TechnoClass*)Args.Object)->EMPLockRemaining <= 0;

			break;
		}
		case AresTriggerEvents::RemoveEMP_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source && ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::EnemyInSpotlightNow:
		{
			result = true;
			break;
		}
		case AresTriggerEvents::DriverKiller:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType;

			break;
		}
		case AresTriggerEvents::DriverKilled_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::VehicleTaken:
		{
			result = generic_cast<FootClass*>(Args.Object) 
				&& pThis->EventKind == Args.EventType;

			break;
		}
		case AresTriggerEvents::VehicleTaken_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::Abducted:
		case AresTriggerEvents::AbductSomething:
		{
			result = generic_cast<FootClass*>(Args.Object) 
				&& pThis->EventKind == Args.EventType;

			break;
		}
		case AresTriggerEvents::Abducted_ByHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::AbductSomething_OfHouse:
		{
			result = generic_cast<FootClass*>(Args.Object)
				&& pThis->EventKind == Args.EventType
				&& Args.Source
				&& Args.Source->WhatAmI() == AbstractType::House
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::SuperActivated:
		case AresTriggerEvents::SuperDeactivated:
		{
			result = pThis->EventKind == Args.EventType
				&& Args.Source
				&& Args.Source->WhatAmI() == AbstractType::Super
				&& ((HouseClass*)Args.Source)->ArrayIndex == pThis->Value;

			break;
		}
		case AresTriggerEvents::SuperNearWaypoint:
		{
			struct PackedDatas
			{
				SuperClass* Super;
				CellStruct Cell;
			};

			if ((pThis->EventKind == Args.EventType) && IS_SAME_STR_(((PackedDatas*)Args.Source)->Super->Type->ID, pThis->String))
			{
				const auto nCell = ScenarioClass::Instance->GetWaypointCoords(pThis->Value);
				CellStruct nDesired = { ((PackedDatas*)Args.Source)->Cell.X - nCell.X ,((PackedDatas*)Args.Source)->Cell.Y - nCell.Y };
				if (nDesired.MagnitudeSquared() <= 5.0)
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
				auto const& nVec = ReverseEngineeredTechnoType(Args.Owner);
				result = std::any_of(nVec.begin(), nVec.end(), [&](TechnoTypeClass* pTech)
					{ return pTech == TEventExt::ExtMap.Find(pThis)->GetTechnoType(); });
			}
			break;
		}
		case AresTriggerEvents::ReverseEngineerAnything:
		{
			result = (pThis->EventKind == Args.EventType);
			break;
		}
		case AresTriggerEvents::ReverseEngineerType:
		{
			result = ((TechnoClass*)Args.Source)->GetTechnoType() == TEventExt::ExtMap.Find(pThis)->GetTechnoType();
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
			result = (pThis->EventKind == Args.EventType);
			break;
		}
		case AresTriggerEvents::AttackedOrDestroyedByHouse:
		{
			result = (pThis->EventKind == Args.EventType)
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
			const HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && KeepAlivesCount(pHouse) <= 0;
			break;
		}
		case AresTriggerEvents::AllKeppAlivesBuildingDestroyed:
		{
			const HouseClass* pHouse = pThis->Value == 0x2325 ?
				nullptr : HouseClass::Index_IsMP(pThis->Value) ?
				HouseClass::FindByIndex(pThis->Value) : HouseClass::FindByCountryIndex(pThis->Value);

			result = pHouse && KeepAlivesBuildingCount(pHouse) <= 0;
			break;
		}
		default:
			switch (Args.EventType)
			{			
			case TriggerEvent::TechTypeExists:
			{
				//TechnoTypeExist
				result = FindTechnoType(pThis, pThis->Value, nullptr);
				break;
			}
			case TriggerEvent::TechTypeDoesntExist:
			{
				//TechnoTypeDoesntExist
				result = FindTechnoType(pThis, 1, nullptr);
				break;
			}
			}

			return false;
		}

		return true;
	}
}

DEFINE_OVERRIDE_HOOK(0x71E949, TEventClass_HasOccured_Ares , 7)
{

	GET(TEventClass*, pThis, EBP);
	GET_BASE(EventArgs const, args, STACK_OFFSET(0x2C, 0x4));
	enum { return_true = 0x71F1B1, return_false = 0x71F163 };
	bool result = false;
	if (TEventExt_dummy::HasOccured(pThis, args, result))
	{
		return result ? return_true : return_false;
	}

	return 0;
}

namespace TActionExt_dummy
{
	bool NOINLINE ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (pHouse->FirestormActive) {
			AresData::RespondToFirewall(pHouse, true);
		}
		return true;
	}

	bool NOINLINE DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (pHouse->FirestormActive) {
			AresData::RespondToFirewall(pHouse, false);
		}
		return true;
	}

	bool NOINLINE AuxiliaryPower(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		const auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);

		if (!pDecidedHouse)
			return false;

		AuxPower(pDecidedHouse) += pAction->Value2;
		pDecidedHouse->RecheckPower = true;
		return true;
	}

	bool NOINLINE KillDriversOf(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);
		if (!pDecidedHouse)
			pDecidedHouse = HouseExt::FindSpecial();

		for (auto pUnit : *FootClass::Array)
		{
			if (pUnit->Health > 0 && pUnit->IsAlive && pUnit->IsOnMap && !pUnit->InLimbo)
			{
				if (auto pTag = pUnit->AttachedTag)
				{
					if (pTag->ContainsTrigger(pTrigger))
					{
						if (!Is_DriverKilled(pUnit) && AresData::IsDriverKillable(pUnit, 1.0))
						{
							AresData::KillDriverCore(pUnit, pDecidedHouse, nullptr, false);
						}
					}
				}
			}
		}

		return true;
	}

	bool NOINLINE SetEVAVoice(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		auto nValue = pAction->Value;
		const auto& nEva= EvaTypes;
		if ((size_t)nValue >= nEva.size()) {
			return false;
		}

		if (nValue < -1)
			nValue = -1;

		VoxClass::EVAIndex = nValue;
		return true;
	}

	bool NOINLINE SetGroup(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (auto pTech = generic_cast<TechnoClass*>(pObject)) {
			pTech->Group = pAction->Value;

			return true;
		}

		return false;
	}

	bool NOINLINE Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& ret)
	{
		switch ((AresNewTriggerAction)pAction->ActionKind)
		{
		case AresNewTriggerAction::AuxiliaryPower:
		{
			ret = AuxiliaryPower(pAction, pHouse, pObject, pTrigger, location);
			break;
		}
		case AresNewTriggerAction::KillDriversOf:
		{
			ret = AuxiliaryPower(pAction, pHouse, pObject, pTrigger, location);
			break;
		}
		case AresNewTriggerAction::SetEVAVoice:
		{
			ret = SetEVAVoice(pAction, pHouse, pObject, pTrigger, location);
			break;
		}
		case AresNewTriggerAction::SetGroup:
		{
			ret = SetGroup(pAction, pHouse, pObject, pTrigger, location);
			break;
		}
		default:
			switch (pAction->ActionKind)
			{
			case TriggerAction::ActivateFirestorm:
				ret = ActivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
				return true;

			case TriggerAction::DeactivateFirestorm:
				ret = DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
				return true;
			}

			return false;
		}

		return true;
	}
}

DEFINE_OVERRIDE_HOOK(0x6DD8D7, TActionClass_Execute_Ares, 0xA)
{
	GET(TActionClass* const, pAction, ESI);
	GET(ObjectClass* const, pObject, ECX);

	GET_STACK(HouseClass* const, pHouse, 0x254);
	GET_STACK(TriggerClass* const, pTrigger, 0x25C);
	GET_STACK(CellStruct const*, pLocation, 0x260);

	enum { Handled = 0x6DFDDD, Default = 0x6DD8E7u };

	// check for actions handled in Ares.
	auto ret = false;
	if (TActionExt_dummy::Execute(
		pAction, pHouse, pObject, pTrigger, *pLocation, ret))
	{
		// returns true or false
		R->AL(ret);
		return Handled;
	}

	// replicate the original instructions, using underflow
	auto const value = static_cast<unsigned int>(pAction->ActionKind) - 1;
	R->EDX(value);
	return (value > 144u) ? Handled : Default;
}