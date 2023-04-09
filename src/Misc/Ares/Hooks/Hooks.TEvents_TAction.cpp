#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

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

namespace TEventExt_dummy
{
	// the function return is deciding if the case is handled or not
	// the bool result pointer is for the result of the Event itself
	bool NOINLINE HasOccured(TEventClass* pThis, EventArgs const Args, bool* result)
	{
		return false;
	}
}

//TODO : 
// otw
//DEFINE_OVERRIDE_HOOK(0x71E949 , TEventClass_HasOccured, 7)
//{
//
//	GET(TEventClass*, pThis, EBP);
//	GET_BASE(EventArgs const, args, STACK_OFFSET(0x2C,0x4));
//	enum { return_true = 0x71F1B1, return_false = 0x71F163};
//	bool result = false;
//	if (TEventExt_dummy::HasOccured(pThis, args, &result)) {
//		return result ? return_true : return_false;
//	}
//
//	return 0;
//}