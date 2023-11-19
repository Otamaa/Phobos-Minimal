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
#include <Ext/Bullet/Body.h>
#include <Ext/Side/Body.h>

#include <TerrainTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
//#include <Lib/gcem/gcem.hpp>

#include <Notifications.h>

#include "Header.h"

DEFINE_DISABLE_HOOK(0x6E232E, ActionClass_PlayAnimAt_ares)

DEFINE_DISABLE_HOOK(0x6DD176, TActionClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x6E4761, TActionClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6E3E30, TActionClass_Save_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6E3DB0, TActionClass_Load_Prefix_ares)
DEFINE_DISABLE_HOOK(0x6E3E29, TActionClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x6E3E4A, TActionClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71e7f8, TEventClass_CTOR_ares)
DEFINE_DISABLE_HOOK(0x71f8c0, TEventClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x71f92b, TEventClass_Load_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71f930, TEventClass_SaveLoad_Prefix_ares)
DEFINE_DISABLE_HOOK(0x71f94a, TEventClass_Save_Suffix_ares)
DEFINE_DISABLE_HOOK(0x71faa6, TEventClass_SDDTOR_ares)
DEFINE_DISABLE_HOOK(0x6e1780, TActionClass_PlayAudioAtRandomWP_ares)

DEFINE_OVERRIDE_HOOK(0x6E20D8, TActionClass_DestroyAttached_Loop, 0x5)
{
	GET(int, nLoopVal, EAX);
	return nLoopVal < 4 ? 0x6E20E0 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6DE0D3, TActionClass_Execute_MessageColor, 6)
{
	int idxColor = 0;
	if (SideClass* pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		if (SideExtData* pExt = SideExtContainer::Instance.Find(pSide)) {
			idxColor = pExt->MessageTextColorIndex;
		}
	}

	R->EAX(idxColor);
	return 0x6DE0DE;
}

DEFINE_OVERRIDE_HOOK(0x41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0xA)
{
	GET(HouseClass*, House, EDI);
	GET(int, triggerSide, EAX);

	enum { Eligible = 0x41E8D7, NotEligible = 0x41E8A1 };

	if (!triggerSide) {
		return Eligible;
	}

	return((triggerSide - 1) == House->SideIndex)
		? Eligible
		: NotEligible
		;
}

DEFINE_OVERRIDE_HOOK(0x6E3EE0, TActionClass_GetFlags, 5)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = AresTActionExt::GetFlag(nAction);

	if (!Handled)
		return 0;

	R->EAX(SomeFlag);
	return 0x6E3EFE;
}

DEFINE_OVERRIDE_HOOK(0x6E3B60, TActionClass_GetMode, 8)
{
	GET(AresNewTriggerAction, nAction, ECX);

	auto const& [SomeFlag, Handled] = AresTActionExt::GetMode(nAction);
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
	if (AresTActionExt::Execute(
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

DEFINE_OVERRIDE_HOOK(0x71F9C0, TEventClass_Persistable_AresNewTriggerEvents, 6)
{
	GET(TEventClass*, pThis, ECX);
	auto const& [Flag, Handled] =
		AresTEventExt::GetPersistableFlag((AresTriggerEvents)pThis->EventKind);
	if (!Handled)
		return 0x0;

	R->EAX(Flag);
	return 0x71F9DF;
}

DEFINE_OVERRIDE_HOOK(0x71F39B, TEventClass_SaveToINI, 5)
{
	GET(AresTriggerEvents, nAction, EDX);
	const auto& [Logic, handled] = AresTEventExt::GetLogicNeed(nAction);

	if (!handled)
		return (int)nAction > 61 ? 0x71F3FC : 0x71F3A0;

	R->EAX(Logic);
	return 0x71F3FE;
}

DEFINE_OVERRIDE_HOOK(0x71f683, TEventClass_GetFlags_Ares, 5)
{
	GET(AresTriggerEvents, nAction, ECX);

	const auto& [handled, result] = AresTEventExt::GetAttachFlags(nAction);
	if (handled)
	{
		R->EAX(result);
		return 0x71F6F6;
	}

	return (int)nAction > 59 ? 0x71F69C : 0x71F688;
}

// the general events requiring a house
DEFINE_OVERRIDE_HOOK(0x71F06C, EventClass_HasOccured_PlayerAtX1, 5)
{
	GET(int const, param, ECX);

	auto const pHouse = AresTEventExt::ResolveHouseParam(param);
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
	R->EAX(AresTEventExt::ResolveHouseParam(param));
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
		auto const pPlayer = AresTEventExt::ResolveHouseParam(param);
		param = pPlayer ? pPlayer->ArrayIndex : -1;
	}

	return (pHouse->ArrayIndex == param) ? 0x71EE82u : 0x71F163u;
}

DEFINE_OVERRIDE_HOOK(0x71E949, TEventClass_HasOccured_Ares, 7)
{

	GET(TEventClass*, pThis, EBP);
	GET_STACK(EventArgs const, args, (0x2C + 0x4));
	enum { return_true = 0x71F1B1, return_false = 0x71F163 };

	bool result = false;
	if (AresTEventExt::HasOccured(pThis, args, result))
	{
		return result ? return_true : return_false;
	}

	return 0;
}
