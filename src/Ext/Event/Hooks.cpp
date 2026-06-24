#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/House/Body.h>

#include <HouseClass.h>

#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <Ext/Event/Body.h>

#include <Locomotor/Cast.h>
#include <Locomotor/JumpjetLocomotionClass.h>

ASMJIT_PATCH(0x64C314, Breakup_Receive_Packet_PayloadSize2, 0x8)
{
	GET(EventType, eventType, ESI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize + (EventType::MEGAMISSION == eventType));

	return 0x64C321;
}

ASMJIT_PATCH(0x64BE83, Breakup_Receive_Packet_PayloadSize1, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->ECX(eventDataSize);
	R->EBP(eventDataSize);
	R->Stack(0x20, eventDataSize);

	return (EventType::MEGAMISSION == eventType) ? 0x64BF1A : 0x64BE97;
}

ASMJIT_PATCH(0x64B704, Add_Compressed_Events_PayloadSize, 0x8)
{
	GET(EventType, eventType, EDI);

	const auto eventDataSize = EventExt::GetDataSize(eventType);

	R->EDX(eventDataSize);
	R->EBP(eventDataSize);

	return (EventType::ADDPLAYER == eventType) ? 0x64B710 : 0x64B71D;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
ASMJIT_PATCH(0x44725F, BuildingClass_GetActionOnObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = cast_to<BuildingClass*>(T)) {
		if(TechnoExtData::canTraverseTo(pThis ,targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> EventExt::Handlers::RespondToTrenchRedirectClick
			R->EAX(Action::Enter);
			return 0x447273;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x443414, BuildingClass_ActionOnObject, 6)
{
	GET(Action, action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	if(action == Action::Detonate)
		return 0;

	// part of deactivation logic
	if(pThis->Deactivated) {
		R->EAX(1);
		return 0x44344D;
	}

	// trenches
	if(action == Action::Enter && pTarget->WhatAmI() == BuildingClass::AbsID) {
		CoordStruct XYZ = pTarget->GetCoords();
		CellStruct tgt = CellClass::Coord2Cell(XYZ);
		EventExt::TrenchRedirectClick::Raise(pThis, &tgt);
		R->EAX(1);
		return 0x44344D;
	}

	return 0;
}

//ASMJIT_PATCH(0x4C65EF, EventClass_Target_CTOR_Log, 0x7)
//{
//	GET(int, events, EAX);
//
//	const auto eventType = static_cast<EventExt::Events>(events);
//	if (EventExt::IsValidType(eventType)) {
//		// Received Ares event, send the names
//		R->ECX(EventExt::GetEventNames(eventType));
//		return 0x4C65F6;
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x64C5C7, Execute_DoList_Log, 0x7)
{
	const auto eventType = static_cast<EventExt::Events>(R->AL());
	if (EventExt::IsValidType(eventType))
	{
		// Received Ares event, send the names
		R->ECX(EventExt::GetEventNames(eventType));
		return 0x64C5CE;
	}

	return 0;
}

ASMJIT_PATCH(0x4C6CCD, EventClass_Execute, 0xA)
{
	GET(int, EventKind, EAX);
	GET(EventClass *, Event, ESI);

	const auto kind = static_cast<EventExt::Events>(EventKind);
	if(EventExt::IsValidType(kind)) {
		// Received Ares event, do something about it
		EventExt::RespondEvent(Event, kind);
	}

	--(EventKind);
	R->EAX(EventKind);
	return (EventKind > (int)EventType::ABANDON_ALL)
	 ? 0x4C8109
	 : 0x4C6CD7
	;
}

// Handle assigning area guard mission to aircraft.
ASMJIT_PATCH(0x4C7403, EventClass_Execute_AircraftAreaGuard, 0x6)
{
	enum { SkipGameCode = 0x4C7435 };

	GET(EventClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);

	if (pTechno->WhatAmI() == AbstractType::Aircraft && AircraftTypeExtData::ExtendedAircraftMissionsEnabled((AircraftClass*)pTechno))
	{
		// Skip assigning destination / target here.
		R->ESI(&pThis->Data.MegaMission.Target);
		return 0x4C7426;
	}

	return 0;
}

// Do not untether aircraft when assigning area guard mission by default.
ASMJIT_PATCH(0x4C72F2, EventClass_Execute_AircraftAreaGuard_Untether, 0x6)
{
	enum { SkipGameCode = 0x4C7349 };

	GET(EventClass* const, pThis, ESI);
	GET(TechnoClass* const, pTechno, EDI);

	if (pTechno->WhatAmI() == AbstractType::Aircraft && AircraftTypeExtData::ExtendedAircraftMissionsEnabled((AircraftClass*)pTechno)
		&& pThis->Data.MegaMission.Mission == (char)Mission::Area_Guard
		&& (pTechno->CurrentMission != Mission::Sleep || !pTechno->Ammo)
		)
	{
		// If we're on dock reloading but have ammo, untether from dock and try to scan for targets.
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x4C77E4, EventClass_Execute_UnitDeployFire, 0x6)
{
	enum { DoNotExecute = 0x4C8109 };

	GET(TechnoClass* const, pThis, ESI);

	auto const pUnit = cast_to<UnitClass*, false>(pThis);

	/// Do not execute deploy command if the vehicle has only just fired its once-firing deploy weapon.
	if (pUnit && pUnit->Type->DeployFire
		&& !pUnit->Type->IsSimpleDeployer
		&& TechnoExtContainer::Instance.Find(pThis)->DeployFireTimer.InProgress())
	{
		return DoNotExecute;
	}

	return 0x0;
}

// issue #112 Make FireOnce=yes work on other TechnoTypes
// Author: Starkku
ASMJIT_PATCH(0x4C7518, EventClass_Execute_StopUnitDeployFire, 0x9)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto const pUnit = cast_to<UnitClass*, false>(pThis))
	{

		if (pUnit->CurrentMission == Mission::Unload
		&& pUnit->Type->DeployFire
		&& !pUnit->Type->IsSimpleDeployer)
		{
			pUnit->SetTarget(nullptr);
			pUnit->QueueMission(Mission::Guard, true);
		}

		// Explicit stop command should reset subterranean harvester state machine.
		auto const pExt = TechnoExtContainer::Instance.Find(pUnit);
		pExt->CurrentSubterraneanHarvStatus = SubterraneanHarvStatus::None;
		pExt->SubterraneanHarvRallyPoint = nullptr;
	}

	// Restore overridden instructions
	GET(Mission, eax, EAX);
	return eax == Mission::Construction ? 0x4C8109 : 0x4C7521;
}

ASMJIT_PATCH(0x4C8011, EventClassExecute_ETiming__ProtocolZero_DisableGame, 0x8)
{
	return EventExt::ProtocolZero::Enable ? 0x4C8024 : 0;
}

// Do not explicitly reset target for KeepTargetOnMove vehicles when issued move command.
ASMJIT_PATCH(0x4C7462, EventClass_Execute_KeepTargetOnMove, 0x5)
{
	enum { SkipGameCode = 0x4C74C0 };

	GET(EventClass*, pThis, ESI);
	GET(TechnoClass*, pTechno, EDI);
	GET(AbstractClass*, pTarget, EBX);

	if (pTechno->WhatAmI() != AbstractType::Unit)
		return 0;

	auto const mission = static_cast<Mission>(pThis->Data.MegaMission.Mission);
	auto const pExt = TechnoExtContainer::Instance.Find(pTechno);
	auto const pTypeExt = GET_TECHNOTYPEEXT(pTechno);

	if ((mission == Mission::Move))
	{
		// Explicitly reset subterranean harvester state machine.
		pExt->CurrentSubterraneanHarvStatus = SubterraneanHarvStatus::None;
		pExt->SubterraneanHarvRallyPoint = nullptr;


		if (pTypeExt->KeepTargetOnMove && pTechno->Target && !pTarget && pTechno->IsCloseEnoughToAttack(pTechno->Target))
		{
			auto const pDestination = pThis->Data.MegaMission.Destination.As_Abstract();
			pTechno->SetDestination(pDestination, true);
			pExt->KeepTargetOnMove = true;
			return SkipGameCode;
		}
	}
	pExt->KeepTargetOnMove = false;
	return 0;
}

// Buildable-upon TechnoTypes Hook #10 -> sub_4C6CB0 - Stop deploy when get stop command
ASMJIT_PATCH(0x4C7665, EventClass_E_IDLE_RespondToEvent_StopDeployInIdleEvent, 0x6)
{
	if (RulesExtData::Instance()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(UnitClass*, pUnit, ESI);

		if (pUnit->Type->DeploysInto)
		{
			const auto mission = pUnit->CurrentMission;

			if (mission == Mission::Guard || mission == Mission::Unload)
			{
				if (const auto pHouseExt = HouseExtContainer::Instance.Find(pUnit->Owner))
				{
					auto& vec = pHouseExt->OwnedDeployingUnits;

					if (!vec.empty())
						vec.remove(pUnit);
				}
			}
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4C780A, EventClass_Execute_DeployEvent_NoVoiceFix, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	pThis->VoiceDeploy();
	return 0x0;
}

ASMJIT_PATCH(0x4C75DA, EventClass_E_IDLE_RespondToEvent_Stop, 0x6)
{
	enum { SkipGameCode = 0x4C762A };

	GET(TechnoClass* const, pTechno, ESI);

	// Check aircraft
	const auto pAircraft = cast_to<AircraftClass*, false>(pTechno);
	const bool commonAircraft = pAircraft && !pAircraft->Airstrike && !pAircraft->Spawned;
	const auto mission = pTechno->CurrentMission;

	// To avoid aircraft overlap by keep link if is returning or is in airport now.
	if (!commonAircraft || (mission != Mission::Sleep && mission != Mission::Guard && mission != Mission::Enter)
		|| !pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink()))
	{
		pTechno->SendToEachLink(RadioCommand::NotifyUnlink);
	}

	// To avoid technos being unable to stop in attack move mega mission
	if (pTechno->MegaMissionIsAttackMove())
		pTechno->ClearMegaMissionData();

	// Clearing the current target should still be necessary for all technos
	pTechno->SetTarget(nullptr);

	// Stop any enter action
	pTechno->QueueUpToEnter = nullptr;

	if (commonAircraft)
	{
		if (pAircraft->Type->AirportBound)
		{
			// To avoid `AirportBound=yes` aircraft with ammo at low altitudes cannot correctly receive stop command and queue Mission::Guard with a `Destination`.
			if (pAircraft->Ammo)
				pTechno->SetDestination(nullptr, true);

			// To avoid `AirportBound=yes` aircraft pausing in the air and let they returning to air base immediately.
			if (!pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink())) // If the aircraft have no valid dock, try to find a new one
				pAircraft->EnterIdleMode(false, true);
		}
		else if (pAircraft->Ammo)
		{
			// To avoid `AirportBound=no` aircraft ignoring the stop task or directly return to the airport.
			if (pAircraft->Destination && static_cast<int>(CellClass::Coord2Cell(pAircraft->Destination->GetCoords()).DistanceFromSquared(pAircraft->GetMapCoords())) > 2) // If the aircraft is moving, find the forward cell then stop in it
				pAircraft->SetDestination(pAircraft->GetCell()->GetNeighbourCell(static_cast<FacingType>(pAircraft->PrimaryFacing.Current().GetValue<3>())), true);
		}
		else if (!pAircraft->DockedTo || (pAircraft->DockedTo != pAircraft->GetNthLink()))
		{
			pAircraft->EnterIdleMode(false, true);
		}
		// Otherwise landing or idling normally without answering the stop command
	}
	else
	{
		const auto pFoot = flag_cast_to<FootClass*, false>(pTechno);

		// Clear archive target for infantries and vehicles like receive a mega mission
		if (pFoot && !pAircraft)
			pTechno->SetArchiveTarget(nullptr);

		// Only stop when it is not under the bridge (meeting the original conditions which has been skipped)
		if (!pTechno->vt_entry_2B0() || pTechno->OnBridge || pTechno->IsInAir() || pTechno->GetCell()->SlopeIndex)
		{
			// To avoid foots stuck in Mission::Area_Guard
			if (pTechno->CurrentMission == Mission::Area_Guard
					&& !GET_TECHNOTYPE(pTechno)->DefaultToGuardArea)
				pTechno->QueueMission(Mission::Guard, true);

			// Check Jumpjets
			const auto pJumpjetLoco = pFoot ? locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor) : nullptr;

			// To avoid jumpjets falling into a state of standing idly by
			if (!pJumpjetLoco) // If is not jumpjet, clear the destination is enough
				pTechno->SetDestination(nullptr, true);
			else if (!pFoot->Destination) // When in attack move and have had a target, the destination will be cleaned up, enter the guard mission can prevent the jumpjets stuck in a status of standing idly by
				pTechno->QueueMission(Mission::Guard, true);
			else if (static_cast<int>(CellClass::Coord2Cell(pFoot->Destination->GetCoords()).DistanceFromSquared(pTechno->GetMapCoords())) > 2) // If the jumpjet is moving, find the forward cell then stop in it
				pTechno->SetDestination(pTechno->GetCell()->GetNeighbourCell(static_cast<FacingType>(pJumpjetLoco->Facing.Current().GetValue<3>())), true);

			// Otherwise landing or idling normally without answering the stop command
		}
	}

	return SkipGameCode;
}


static bool inline CanBeSold(TechnoClass* pTechno, AbstractType rtti)
{
	if (rtti == AbstractType::Building || rtti == AbstractType::Unit || rtti == AbstractType::Aircraft)
		return pTechno->CanBeSold();

	return false;
}

// Verify if object can be sold at event level.
ASMJIT_PATCH(0x4C6F55, EventClass_Execute_Sell, 0x5)
{
	enum { SkipGameCode = 0x4C6FA8 };

	GET(TechnoClass*, pTechno, EDI);
	GET(AbstractType, rtti, EAX);

	if (CanBeSold(pTechno, rtti))
		pTechno->Sell(-1);

	return SkipGameCode;
}

ASMJIT_PATCH(0x4C7643, EventClass_RespondToEvent_StopTemporal, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	auto const pTemporal = pTechno->TemporalImUsing;

	if (pTemporal && pTemporal->Target)
		pTemporal->LetGo();

	return 0;
}

//EventClass_Execute_IDLE
DEFINE_JUMP(LJMP, 0x4C752A, 0x4C757D); // Skip cell under bridge check
