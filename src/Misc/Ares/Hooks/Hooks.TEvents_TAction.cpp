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
#include <Misc/AresData.h>
#include <Ares_TechnoExt.h>

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

DEFINE_OVERRIDE_HOOK(0x6DE0D3, TActionClass_Execute_MessageColor, 6)
{
	int idxColor = 0;
	if (SideClass* pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		if (SideExt::ExtData* pExt = SideExt::ExtMap.Find(pSide)) {
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

DEFINE_OVERRIDE_HOOK(0x71f683, TEventClass_GetFlags_Ares, 5)
{
	GET(AresTriggerEvents, nAction, ECX);

	const auto& [handled, result] = GetTEventAttachFlags(nAction);
	if (handled)
	{
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
			switch (pType->WhatAmI())
			{
			case AbstractType::AircraftType:
			{
				for (auto pTechno : *AircraftClass::Array)
				{
					if (pWho && pWho != pTechno->Owner)
						continue;

					if (pTechno->Type == pType)
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
		if (pThis->EventKind < TriggerEvent::count)
		{
			switch (pThis->EventKind)
			{
			case TriggerEvent::TechTypeExists:
			{
				//TechnoTypeExist
				result = FindTechnoType(pThis, pThis->Value, nullptr);
				return true;
			}
			case TriggerEvent::TechTypeDoesntExist:
			{
				//TechnoTypeDoesntExist
				result = !FindTechnoType(pThis, 1, nullptr);
				return true;
			}
			}
		}
		else
		{
			switch ((AresTriggerEvents)pThis->EventKind)
			{
			case AresTriggerEvents::UnderEMP:
			case AresTriggerEvents::UnderEMP_ByHouse:
			case AresTriggerEvents::RemoveEMP:
			case AresTriggerEvents::RemoveEMP_ByHouse:
			{
				const auto pTechno = generic_cast<FootClass*>(Args.Object);

				if (pTechno && pThis->EventKind == Args.EventType)
				{

					switch ((AresTriggerEvents)Args.EventType)
					{
					case AresTriggerEvents::UnderEMP:
					{
						result = pTechno->EMPLockRemaining > 0;
						return true;
					}
					case AresTriggerEvents::UnderEMP_ByHouse:
					{
						if (Args.Source && ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value)
						{
							result = pTechno->EMPLockRemaining > 0;
							return true;
						}
						break;
					}
					case AresTriggerEvents::RemoveEMP:
					{
						result = pTechno->EMPLockRemaining <= 0;
						return true;
					}
					case AresTriggerEvents::RemoveEMP_ByHouse:
					{
						if (Args.Source && ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value)
						{
							result = pTechno->EMPLockRemaining <= 0;
							return true;
						}
						break;
					}
					}
				}

				result = false;
				return true;
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
			case AresTriggerEvents::Abducted_ByHouse:
			case AresTriggerEvents::AbductSomething_OfHouse:
			{
				const auto pTechno = generic_cast<FootClass*>(Args.Object);

				if (pTechno && pThis->EventKind == Args.EventType)
				{
					switch ((AresTriggerEvents)Args.EventType)
					{
					case AresTriggerEvents::Abducted:
					case AresTriggerEvents::AbductSomething:
					{
						result = true;
						return true;
					}
					case AresTriggerEvents::Abducted_ByHouse:
					{
						if (generic_cast<TechnoClass*>(Args.Source) && ((TechnoClass*)(Args.Source))->Owner->ArrayIndex == pThis->Value)
						{
							result = true;
							return true;
						}

						break;
					}
					case AresTriggerEvents::AbductSomething_OfHouse:
					{
						if (specific_cast<HouseClass*>(Args.Source) && ((HouseClass*)(Args.Source))->ArrayIndex == pThis->Value)
						{
							result = true;
							return true;
						}

						break;
					}
					}
				}

				result = false;
				break;
			}
			case AresTriggerEvents::SuperActivated:
			case AresTriggerEvents::SuperDeactivated:
			{
				result = pThis->EventKind == Args.EventType
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
					auto const& nVec = HouseExt::ExtMap.Find(Args.Owner)->Reversed;

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
					&& ((TechnoClass*)Args.Source)->Owner->ArrayIndex == pThis->Value;

				break;
			}
			case AresTriggerEvents::DestroyedByHouse:
			{
				result = ((AresTriggerEvents)Args.EventType == AresTriggerEvents::DestroyedByHouse)
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
			}

			return true;
		}

		return false;
	}
}

DEFINE_OVERRIDE_HOOK(0x71E949, TEventClass_HasOccured_Ares, 7)
{

	GET(TEventClass*, pThis, EBP);
	GET_STACK(EventArgs const, args, (0x2C + 0x4));
	enum { return_true = 0x71F1B1, return_false = 0x71F163 };

	bool result = false;
	if (TEventExt_dummy::HasOccured(pThis, args, result))
	{
		return result ? return_true : return_false;
	}

	return 0;
}

#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Misc/Ares/EVAVoices.h>

namespace TActionExt_dummy
{
	bool NOINLINE ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (pHouse->FirestormActive)
		{
			AresData::RespondToFirewall(pHouse, true);
		}
		return true;
	}

	bool NOINLINE DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (pHouse->FirestormActive)
		{
			AresData::RespondToFirewall(pHouse, false);
		}
		return true;
	}

	bool NOINLINE AuxiliaryPower(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		const auto pDecidedHouse = pAction->FindHouseByIndex(pTrigger, pAction->Value);

		if (!pDecidedHouse)
			return false;

		HouseExt::ExtMap.Find(pDecidedHouse)->AuxPower += pAction->Value2;
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
				if (pUnit->AttachedTag && pUnit->AttachedTag->ContainsTrigger(pTrigger))
				{
					if (!pUnit->align_154->Is_DriverKilled && AresData::IsDriverKillable(pUnit, 1.0))
					{
						AresData::KillDriverCore(pUnit, pDecidedHouse, nullptr, false);
					}
				}
			}
		}

		return true;
	}

	bool NOINLINE SetEVAVoice(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		auto nValue = pAction->Value;
		const auto& nEvas = EVAVoices::Types;
		if ((size_t)nValue >= nEvas.size()) {
			return false;
		}

		if (nValue < -1)
			nValue = -1;

		VoxClass::EVAIndex = nValue;
		return true;
	}

	bool NOINLINE SetGroup(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (auto pTech = generic_cast<TechnoClass*>(pObject))
		{
			pTech->Group = pAction->Value;

			return true;
		}

		return false;
	}

	//TODO : re-eval
	bool NOINLINE LauchhNuke(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		const auto pFind = WeaponTypeClass::Find(GameStrings::NukePayload);
		if (!pFind)
			return false;

		const auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		auto nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

		if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
			nCoord.Z += CellClass::BridgeHeight;

		SW_NuclearMissile::DropNukeAt(nullptr, nCoord, nullptr, pHouse, pFind);

		//if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nCoord), nullptr, pFind->Damage, pFind->Warhead, 50, false))
		//{
		//	pBullet->SetWeaponType(pFind);
		//	VelocityClass nVel {};
		//
		//	double nSin = Math::sin(1.570748388432313);
		//	double nCos = Math::cos(1.570748388432313);
		//
		//	double nX = nCos * nCos * -100.0;
		//	double nY = nCos * nSin * -100.0;
		//	double nZ = nSin * -100.0;
		//
		//	BulletExt::ExtMap.Find(pBullet)->Owner = pHouse;
		//	pBullet->MoveTo({ nCoord.X , nCoord.Y , nCoord.Z + 20000 }, nVel);
		//	return true;
		//}

		return false;
	}

	//TODO : re-eval
	bool NOINLINE LauchhChemMissile(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		const auto pFind = WeaponTypeClass::Find(GameStrings::ChemLauncher);
		if (!pFind)
			return false;

		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

		if (auto pBullet = pFind->Projectile->CreateBullet(MapClass::Instance->GetCellAt(nLoc), nullptr, pFind->Damage, pFind->Warhead, 20, false))
		{
			pBullet->SetWeaponType(pFind);
			VelocityClass nVel {};

			double nSin = Math::sin(1.570748388432313);
			double nCos = Math::cos(-0.00009587672516830327);

			double nX = nCos * nCos * 100.0;
			double nY = nCos * nSin * 100.0;
			double nZ = nSin * 100.0;

			BulletExt::ExtMap.Find(pBullet)->Owner = pHouse;
			auto nCell = MapClass::Instance->Localsize_586AC0(&nLoc, false);

			pBullet->MoveTo({ nCell.X + 128 , nCell.Y + 128 , 0 }, nVel);
			return true;
		}

		return false;
	}

	bool NOINLINE LightstormStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);

		// get center of cell coords
		auto const pCell = MapClass::Instance->GetCellAt(nLoc);

		// create a cloud animation
		if (auto coords = pCell->GetCoordsWithBridge())
		{
			// select the anim
			auto const& itClouds = RulesClass::Instance->WeatherConClouds;
			auto const pAnimType = itClouds.GetItem(ScenarioClass::Instance->Random.RandomFromMax(itClouds.Count - 1));

			if (pAnimType)
			{
				coords.Z += GeneralUtils::GetLSAnimHeightFactor(pAnimType, pCell , true);

				if (coords.IsValid())
				{
					// create the cloud and do some book keeping.
					if (auto const pAnim = GameCreate<AnimClass>(pAnimType, coords))
					{
						pAnim->SetHouse(pHouse);
						LightningStorm::CloudsManifesting->AddItem(pAnim);
						LightningStorm::CloudsPresent->AddItem(pAnim);
					}
				}
			}
		}

		return true;
	}

	bool NOINLINE MeteorStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		static constexpr reference<int, 0x842AFC, 5u> MeteorAddAmount {};

		const auto pSmall = AnimTypeClass::Find(GameStrings::METSMALL);
		const auto pBig = AnimTypeClass::Find(GameStrings::METLARGE);

		if (!pSmall && !pBig)
			return false;

		auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
		CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
		nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

		if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
			nCoord.Z += CellClass::BridgeHeight;

		const auto amount = MeteorAddAmount[pAction->Value % MeteorAddAmount.size()] + ScenarioClass::Instance->Random.Random() % 3;
		if (amount <= 0)
			return true;

		const int nTotal = 70 * amount;

		for (int i = nTotal; i > 0; --i)
		{
			auto nRandX = ScenarioClass::Instance->Random.Random() % nTotal;
			auto nRandY = ScenarioClass::Instance->Random.Random() % nTotal;
			CoordStruct nLoc { nRandX + nCoord.X ,nRandY + nCoord.Y ,nCoord.Z };

			AnimTypeClass* pSelected = pBig;
			int nRandHere = abs(ScenarioClass::Instance->Random.Random()) & 0x80000001;
			bool v13 = nRandHere == 0;
			if (nRandHere < 0)
			{
				v13 = ((nRandHere - 1) | 0xFFFFFFFE) == -1;
			}

			if (v13)
			{
				pSelected = pSmall;
			}

			if (pSelected)
			{
				if (auto pAnim = GameCreate<AnimClass>(pSelected, nLoc, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
				{
					pAnim->Owner = pHouse;
				}
			}
		}

		return true;
	}

	bool NOINLINE PlayAnimAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
	{
		if (const auto pAnimType = AnimTypeClass::Array->GetItemOrDefault(pAction->Value))
		{
			auto nLoc = ScenarioClass::Instance->GetWaypointCoords(pAction->Waypoint);
			CoordStruct nCoord = CellClass::Cell2Coord(nLoc);
			nCoord.Z = MapClass::Instance->GetCellFloorHeight(nCoord);

			if (MapClass::Instance->GetCellAt(nCoord)->ContainsBridge())
				nCoord.Z += CellClass::BridgeHeight;

			if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0))
			{
				pAnim->IsPlaying = true;
				pAnim->Owner = pHouse;
			}
		}

		return true;
	}

	bool NOINLINE Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& ret)
	{
		switch (pAction->ActionKind)
		{
		case TriggerAction::PlayAnimAt:
			ret = PlayAnimAt(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::MeteorShower:
			ret = MeteorStrike(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::LightningStrike:
			ret = LightstormStrike(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::ActivateFirestorm:
			ret = ActivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::DeactivateFirestorm:
			ret = DeactivateFirestorm(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::NukeStrike:
			ret = LauchhNuke(pAction, pHouse, pObject, pTrigger, location);
			return true;
		case TriggerAction::ChemMissileStrike:
			ret = LauchhChemMissile(pAction, pHouse, pObject, pTrigger, location);
			return true;
		}

		if ((AresNewTriggerAction)pAction->ActionKind < AresNewTriggerAction::AuxiliaryPower)
			return false;

		switch ((AresNewTriggerAction)pAction->ActionKind)
		{
		case AresNewTriggerAction::AuxiliaryPower:
		{
			ret = AuxiliaryPower(pAction, pHouse, pObject, pTrigger, location);
			break;
		}
		case AresNewTriggerAction::KillDriversOf:
		{
			ret = KillDriversOf(pAction, pHouse, pObject, pTrigger, location);
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