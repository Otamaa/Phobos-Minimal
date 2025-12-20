#include "Body.h"
#include <Ext/TechnoType/Body.h>

#include <Misc/Ares/Hooks/Header.h>
#include <InfantryClass.h>

#ifndef DEBUG_CODE

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	return TechnoTypeExtContainer::Instance.Find(pThis->Transporter->GetTechnoType())->Passengers_SyncOwner.Get() ?  ret : 0; }

SET_THREATEVALS(0x6F89F4, ESI, TechnoClass_EvaluateCell_ThreatEvals_OpenToppedOwner, 0x6, 0x6F8A0F)
//SET_THREATEVALS(0x6F8FD7, ESI, TechnoClass_GreatestThreat_ThreatEvals_OpenToppedOwner, 0x5, 0x6F8FDC)
SET_THREATEVALS(0x6F7EC2, EDI, TechnoClass_EvaluateObject_ThreatEvals_OpenToppedOwner, 0x6, 0x6F7EDA)

#undef SET_THREATEVALS
#undef SET_THREATEVALSB
#else
ASMJIT_PATCH_AGAIN(0x6F89F4, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateCell
ASMJIT_PATCH_AGAIN(0x6F7EC2, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateObject
ASMJIT_PATCH(0x6F8FD7, TechnoClass_ThreatEvals_OpenToppedOwner, 0x5)       // TechnoClass::Greatest_Threat
{
	enum { SkipCheckOne = 0x6F8FDC, SkipCheckTwo = 0x6F7EDA, SkipCheckThree = 0x6F8A0F, SkipCheckFour = 0x6FA37A };

	TechnoClass* pThis = nullptr;
	auto returnAddress = SkipCheckOne;

	switch (R->Origin())
	{
	case 0x6F8FD7:
		pThis = R->ESI<TechnoClass*>();
		break;
	case 0x6F7EC2:
		pThis = R->EDI<TechnoClass*>();
		returnAddress = SkipCheckTwo;
		break;
	case 0x6F89F4:
		pThis = R->ESI<TechnoClass*>();
		returnAddress = SkipCheckThree;
	case 0x6FA33C:
		pThis = R->ESI<TechnoClass*>();
		returnAddress = SkipCheckFour;
	default:
		return 0;
	}

	if (auto pTransport = pThis->Transporter)
	{
		if (auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType()))
		{
			if (pTypeExt->Passengers_SyncOwner)
				return returnAddress;
		}
	}

	return 0;
}
#endif

ASMJIT_PATCH(0x71067B, TechnoClass_EnterTransport_ApplyChanges, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pPassenger)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pPassExt = TechnoExtContainer::Instance.Find(pPassenger);
		//auto const pPassTypeExt = TechnoTypeExtContainer::Instance.Find(pPassenger->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pPassExt->OriginalPassengerOwner = pPassenger->Owner;

		for (auto& pLaserTrail : pPassExt->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation.clear();
		}

		TrailsManager::Hide((TechnoClass*)pPassenger);
	}

	return 0;
}

ASMJIT_PATCH(0x4DE722, FootClass_LeaveTransport, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EAX);

	if (pPassenger)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pPassExt = TechnoExtContainer::Instance.Find(pPassenger);
		//auto const pPassTypeExt = TechnoTypeExtContainer::Instance.Find(pPassenger->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit &&
			pPassExt->OriginalPassengerOwner)
		{
			pPassenger->SetOwningHouse(pPassExt->OriginalPassengerOwner, false);
		}
	}

	return 0;
}

ASMJIT_PATCH(0x710552, TechnoClass_SetOpenTransportCargoTarget_ShareTarget, 0x6)
{
	enum { ReturnFromFunction = 0x71057F, Continue = 0x0 };

	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(AbstractClass* const, pTarget, STACK_OFFSET(0x8, 0x4));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	return pTarget && !pTypeExt->OpenTopped_ShareTransportTarget
		? ReturnFromFunction : Continue;
}

#include <CaptureManagerClass.h>
#include <Locomotor/Cast.h>
#include <VoxClass.h>
#include <VocClass.h>
#include <Utilities/Macro.h>

#pragma region NoQueueUpToEnterAndUnload

// Use a square range because it doesn't seem necessary to calculate the circular range
static FORCEDINLINE bool IsCloseEnoughToEnter(UnitClass* pTransport, FootClass* pPassenger)
{
	return (Math::abs(pPassenger->Location.X - pTransport->Location.X) < 384
		&& Math::abs(pPassenger->Location.Y - pTransport->Location.Y) < 384
		&& Math::abs(pPassenger->Location.Z - pTransport->Location.Z) < Unsorted::CellHeight);
}

// Rewrite from 0x73758A, replace send RadioCommand::QueryCanEnter
static FORCEDINLINE bool CanEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	if (!pTransport->Owner->IsAlliedWith(pPassenger) || pTransport->IsBeingWarpedOut())
		return false;

	// Added to prevent unexpected enter action
	if (pTransport->OnBridge || pPassenger->Deactivated || pPassenger->IsUnderEMP())
		return false;

	if (pPassenger->IsMindControlled() || pPassenger->ParasiteEatingMe)
		return false;

	const auto pManager = pPassenger->CaptureManager;

	if (pManager && pManager->IsControllingSomething())
		return false;

	const auto pTransportType = pTransport->Type;

	// Added to fit with AmphibiousEnter
	if (pTransport->GetCell()->LandType == LandType::Water
		&& !TechnoTypeExtContainer::Instance.Find(pTransportType)->AmphibiousEnter.Get(RulesExtData::Instance()->AmphibiousEnter))
		return false;

	const bool bySize = TechnoTypeExtContainer::Instance.Find(pTransportType)->Passengers_BySize;
	const int passengerSize = static_cast<int>(pPassenger->GetTechnoType()->Size);

	if (passengerSize > static_cast<int>(pTransportType->SizeLimit))
		return false;

	const int maxSize = pTransportType->Passengers;
	const int predictSize = bySize ? (pTransport->Passengers.GetTotalSize() + passengerSize) : (pTransport->Passengers.NumPassengers + 1);
	const auto pLink = flag_cast_to<FootClass*>(pTransport->GetNthLink());
	const bool needCalculate = pLink && pLink != pPassenger && pLink->Destination == pTransport;

	// When the most important passenger is close, need to prevent overlap
	if (needCalculate)
	{
		if (IsCloseEnoughToEnter(pTransport, pLink))
			return (predictSize <= (maxSize - (bySize ? static_cast<int>(pLink->GetTechnoType()->Size) : 1)));

		if (predictSize > (maxSize - (bySize ? static_cast<int>(pLink->GetTechnoType()->Size) : 1)))
		{
			pLink->QueueMission(Mission::None, false);
			pLink->SetDestination(nullptr, true);
			pLink->SendCommand(RadioCommand::NotifyUnlink, pTransport);
		}
	}

	return predictSize <= maxSize;
}

// Rewrite from 0x51A21B/0x73A6D1
static FORCEDINLINE void DoEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	// Vanilla only for infantry, but why
	if (const auto pTag = pTransport->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::EnteredBy, pPassenger, CellStruct::Empty);

	// Vanilla did not handle SpawnManager and SlaveManager, so I don't care about these here either
	pPassenger->SetArchiveTarget(nullptr);
	pPassenger->MissionAccumulateTime = 0;
	pPassenger->GattlingValue = 0;
	pPassenger->CurrentGattlingStage = 0;

	pPassenger->Limbo(); // Don't swap order casually
	pPassenger->OnBridge = false; // Don't swap order casually, important
	pPassenger->NextObject = nullptr; // Don't swap order casually, very important

	pPassenger->SetDestination(nullptr, true); // Added, to prevent passengers from return to board position when survive
	pPassenger->QueueUpToEnter = nullptr; // Added, to prevent passengers from wanting to get on after getting off
	pPassenger->FrozenStill = true; // Added, to prevent the vehicles from stacking together when unloading
	pPassenger->SetSpeedPercentage(0.0); // Added, to stop the passengers and let OpenTopped work normally

	const auto pPassengerType = pPassenger->GetTechnoType();

	pTransport->AddPassenger(pPassenger); // Don't swap order casually, very very important
	pPassenger->Transporter = pTransport;

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	if (pPassengerType->OpenTopped)
		pPassenger->SetTargetForPassengers(nullptr);

	pPassenger->Undiscover();
	TechnoExtContainer::Instance.Find(pPassenger)->ResetLocomotor = true;
}

void TechnoExtData::Fastenteraction(FootClass* pThis) {
	if (const auto pDest = cast_to<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->GetNthLink() : pThis->QueueUpToEnter))
	{
		const auto pType = pDest->Type;

		if (pType->Passengers > 0 && TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter))
		{
			if (IsCloseEnoughToEnter(pDest, pThis))
			{
				const auto absType = pThis->WhatAmI();

				if ((absType == AbstractType::Infantry || absType == AbstractType::Unit) && CanEnterNow(pDest, pThis))
					DoEnterNow(pDest, pThis);
			}
			else if (!pThis->Destination // Move to enter position, prevent other passengers from waiting for call and not moving early
				&& !pDest->OnBridge && !pDest->Destination)
			{
				auto cell = CellStruct::Empty;
				pThis->NearbyLocation(&cell, pDest);

				if (cell != CellStruct::Empty)
				{
					pThis->SetDestination(MapClass::Instance->GetCellAt(cell), true);
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}
			}
		}
	}
}

// Rewrite from 0x4835D5/0x74004B, replace check pThis->GetCell()->LandType != LandType::Water
static FORCEDINLINE bool CanUnloadNow(UnitClass* pTransport, FootClass* pPassenger)
{
	if (TechnoTypeExtContainer::Instance.Find(pTransport->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
		return GroundType::Array[static_cast<int>(pTransport->GetCell()->LandType)].Cost[static_cast<int>(pPassenger->GetTechnoType()->SpeedType)] != 0.0;

	return pTransport->GetCell()->LandType != LandType::Water;
}

namespace TransportUnloadTemp
{
	bool ShouldPlaySound = false;
}

// Interrupted due to insufficient location or other reasons
ASMJIT_PATCH(0x73DC9C, UnitClass_Mission_Unload_NoQueueUpToUnloadBreak, 0xA)
{
	enum { SkipGameCode = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	// Restore vanilla function
	pPassenger->Undiscover();

	// Clean up the unload space
	const bool alt = pThis->OnBridge;
	const auto pCell = pThis->GetCell();
	const auto coord = pCell->GetCoords();

	for (int i = 0; i < 8; ++i)
	{
		const auto pAdjCell = pCell->GetNeighbourCell(static_cast<FacingType>(i));
		const auto pTechno = pAdjCell->FindTechnoNearestTo(Point2D::Empty, alt, pThis);

		if (pTechno && pTechno->Owner->IsAlliedWith(pThis))
			pAdjCell->ScatterContent(coord, true, true, alt);
	}

	// Play the sound when interrupted
	if (TransportUnloadTemp::ShouldPlaySound) // Only when NoQueueUpToUnload enabled
	{
		TransportUnloadTemp::ShouldPlaySound = false;
		const auto pType = pThis->Type;

		if (TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
			VocClass::PlayIndexAtPos(pType->LeaveTransportSound, pThis->Location, false);
	}

	return SkipGameCode;
}

// Within a single frame, cycle to get off the car
ASMJIT_PATCH(0x73DC1E, UnitClass_Mission_Unload_NoQueueUpToUnloadLoop, 0xA)
{
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289, NoUnloadReturn = 0x73D8AA };

	GET(UnitClass* const, pThis, ESI);

	const auto pType = pThis->Type;
	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	if (TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
	{
		if (!pPassenger || pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			VocClass::PlayIndexAtPos(pType->LeaveTransportSound, pThis->Location , false);
			TransportUnloadTemp::ShouldPlaySound = false;
			return UnloadReturn;
		}
		else if (!CanUnloadNow(pThis, pPassenger))
		{
			VocClass::PlayIndexAtPos(pType->LeaveTransportSound, pThis->Location, false);
			TransportUnloadTemp::ShouldPlaySound = false;
			pThis->MissionStatus = 0; // Retry
			return NoUnloadReturn;
		}

		TransportUnloadTemp::ShouldPlaySound = true;
		R->EBX(0); // Reset
		return UnloadLoop;
	}

	// PlayAtPos has already handled the situation where Sound is less than 0 internally, so unnecessary checks will be skipped
	VocClass::PlayIndexAtPos(pType->LeaveTransportSound, pThis->Location, false);

	if (!pPassenger || CanUnloadNow(pThis, pPassenger))
		return UnloadReturn;

	pThis->MissionStatus = 0; // Retry
	return NoUnloadReturn;
}

#pragma endregion

#pragma region TransportFix

ASMJIT_PATCH(0x51D45B, InfantryClass_Scatter_NoProcess, 0x6)
{
	enum { SkipGameCode = 0x51D47B };

	REF_STACK(const int, addr, STACK_OFFSET(0x50, 0));
	// Skip process in InfantryClass::UpdatePosition which can create invisible barrier
	return (addr == 0x51A4B5) ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x4D92BF, FootClass_Mission_Enter_CheckLink, 0x5)
{
	enum { NextAction = 0x4D92ED, NotifyUnlink = 0x4D92CE, DoNothing = 0x4D946C };

	GET(UnitClass* const, pThis, ESI);
	GET(const RadioCommand, answer, EAX);
	// Restore vanilla check
	if (pThis->IsTethered || answer == RadioCommand::AnswerPositive)
		return NextAction;
	// The link should not be disconnected while the transporter is in motion (passengers waiting to enter),
	// as this will result in the first passenger not getting on board
	return answer == RadioCommand::RequestLoading ? DoNothing : NotifyUnlink;
}

ASMJIT_PATCH(0x70D842, FootClass_UpdateEnter_NoMoveToBridge, 0x5)
{
	enum { NoMove = 0x70D84F };

	GET(TechnoClass* const, pEnter, EDI);
	// If the transport vehicle is on the bridge, passengers should wait in place for the transport vehicle to arrive
	return pEnter->OnBridge && (pEnter->WhatAmI() == AbstractType::Unit && static_cast<UnitClass*>(pEnter)->Type->Passengers > 0) ? NoMove : 0;
}

ASMJIT_PATCH(0x70D910, FootClass_QueueEnter_NoMoveToBridge, 0x5)
{
	enum { NoMove = 0x70D977 };

	GET(TechnoClass* const, pEnter, EAX);
	// If the transport vehicle is on the bridge, passengers should wait in place for the transport vehicle to arrive
	return pEnter->OnBridge && (pEnter->WhatAmI() == AbstractType::Unit && static_cast<UnitClass*>(pEnter)->Type->Passengers > 0) ? NoMove : 0;
}

ASMJIT_PATCH(0x7196BB, TeleportLocomotionClass_Process_MarkDown, 0xA)
{
	enum { SkipGameCode = 0x7196C5 };

	GET(FootClass*, pLinkedTo, ECX);
	// When Teleport units board transport vehicles on the bridge, the lack of this repair can lead to numerous problems
	// An impassable invisible barrier will be generated on the bridge (the object linked list of the cell will leave it)
	// And the transport vehicle will board on the vehicle itself (BFRT Passenger:..., BFRT)
	// If any infantry attempts to pass through this position on the bridge later, it will cause the game to freeze
	auto shouldMarkDown = [pLinkedTo]()
		{
			if (pLinkedTo->GetCurrentMission() != Mission::Enter)
				return true;

			const auto pEnter = pLinkedTo->GetNthLink();

			return (!pEnter || pEnter->GetTechnoType()->Passengers <= 0);
		};

	if (shouldMarkDown())
		pLinkedTo->Mark(MarkType::Down);

	return SkipGameCode;
}

#pragma endregion

#pragma region AmphibiousEnterAndUnload

// Related fix
ASMJIT_PATCH(0x4B08EF, DriveLocomotionClass_Process_CheckUnload, 0x5)
{
	enum { SkipGameCode = 0x4B078C, ContinueProcess = 0x4B0903 };

	GET(ILocomotion* const, iloco, ESI);

	const auto pFoot = static_cast<LocomotionClass*>(iloco)->LinkedTo;

	if (pFoot->GetCurrentMission() != Mission::Unload)
		return ContinueProcess;

	return (pFoot->GetTechnoType()->Passengers > 0 && pFoot->Passengers.GetFirstPassenger()) ? ContinueProcess : SkipGameCode;
}

ASMJIT_PATCH(0x69FFB6, ShipLocomotionClass_Process_CheckUnload, 0x5)
{
	enum { SkipGameCode = 0x69FE39, ContinueProcess = 0x69FFCA };

	GET(ILocomotion* const, iloco, ESI);

	const auto pFoot = static_cast<LocomotionClass*>(iloco)->LinkedTo;

	if (pFoot->GetCurrentMission() != Mission::Unload)
		return ContinueProcess;

	return (pFoot->GetTechnoType()->Passengers > 0 && pFoot->Passengers.GetFirstPassenger()) ? ContinueProcess : SkipGameCode;
}

// Rewrite from 0x718505
ASMJIT_PATCH(0x718F1E, TeleportLocomotionClass_MovingTo_ReplaceMovementZone, 0x6)
{
	GET(TechnoTypeClass* const, pType, EAX);

	auto movementZone = pType->MovementZone;

	if (movementZone == MovementZone::Fly || movementZone == MovementZone::Destroyer)
		movementZone = MovementZone::Normal;
	else if (movementZone == MovementZone::AmphibiousDestroyer)
		movementZone = MovementZone::Amphibious;

	R->EBP(movementZone);
	return R->Origin() + 0x6;
}ASMJIT_PATCH_AGAIN(0x7190B0, TeleportLocomotionClass_MovingTo_ReplaceMovementZone, 0x6)


// Enter building
DEFINE_JUMP(LJMP, 0x43C38D, 0x43C3FF); // Skip amphibious and naval check if no Ares

// Enter unit
ASMJIT_PATCH(0x73796B, UnitClass_ReceiveCommand_AmphibiousEnter, 0x7)
{
	enum { ContinueCheck = 0x737990, MoveToPassenger = 0x737974 };

	GET(UnitClass* const, pThis, ESI);

	if (pThis->OnBridge)
		return MoveToPassenger;

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousEnter.Get(RulesExtData::Instance()->AmphibiousEnter))
		return ContinueCheck;

	GET(CellClass* const, pCell, EBP);

	return (pCell->LandType != LandType::Water) ? ContinueCheck : MoveToPassenger;
}

// Unit unload
ASMJIT_PATCH(0x7400B5, UnitClass_MouseOverObject_AmphibiousUnload, 0x7)
{
	enum { ContinueCheck = 0x7400C6, CannotUnload = 0x7400BE };

	GET(CellClass* const, pCell, EBX);

	if (pCell->LandType == LandType::Water) // I don't know why WW made a reverse judgment here? Because of the coast/beach?
		return ContinueCheck;

	GET(UnitClass* const, pThis, ESI);

	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload) ? ContinueCheck : CannotUnload;
}

ASMJIT_PATCH(0x70107A, TechnoClass_CanDeploySlashUnload_AmphibiousUnload, 0x7)
{
	enum { ContinueCheck = 0x701087, CannotUnload = 0x700DCE };

	GET(CellClass* const, pCell, EBP);

	if (pCell->LandType != LandType::Water)
		return ContinueCheck;

	GET(UnitClass* const, pThis, ESI);

	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload) ? ContinueCheck : CannotUnload;
}

ASMJIT_PATCH(0x73D769, UnitClass_Mission_Unload_AmphibiousUnload, 0x7)
{
	enum { MoveToLand = 0x73D772, UnloadCheck = 0x73D7E4 };

	GET(UnitClass* const, pThis, ESI);

	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	return (!pPassenger || CanUnloadNow(pThis, pPassenger)) ? UnloadCheck : MoveToLand;
}

ASMJIT_PATCH(0x73D7AB, UnitClass_Mission_Unload_FindUnloadPosition, 0x5)
{
	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
	{
		if (const auto pPassenger = pThis->Passengers.GetFirstPassenger())
		{
			REF_STACK(SpeedType, speedType, STACK_OFFSET(0xBC, -0xB4));
			REF_STACK(MovementZone, movementZone, STACK_OFFSET(0xBC, -0xAC));

			const auto pType = pPassenger->GetTechnoType();
			speedType = pType->SpeedType; // Replace hard code SpeedType::Wheel
			movementZone = pType->MovementZone; // Replace hard code MovementZone::Normal
		}
	}

	return 0;
}

ASMJIT_PATCH(0x73D7B7, UnitClass_Mission_Unload_CheckInvalidCell, 0x6)
{
	enum { CannotUnload = 0x73D87F };

	GET(const CellStruct, cell, EAX);

	return cell != CellStruct::Empty ? 0 : CannotUnload;
}

ASMJIT_PATCH(0x740C9C, UnitClass_GetUnloadDirection_CheckUnloadPosition, 0x7)
{
	GET(UnitClass* const, pThis, EDI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
	{
		if (const auto pPassenger = pThis->Passengers.GetFirstPassenger())
		{
			GET(const int, speedType, EDX);
			R->EDX(speedType + static_cast<int>(pPassenger->GetTechnoType()->SpeedType)); // Replace hard code SpeedType::Foot
		}
	}

	return 0;
}

ASMJIT_PATCH(0x73DAD8, UnitClass_Mission_Unload_PassengerLeavePosition, 0x5)
{
	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
	{
		GET(FootClass* const, pPassenger, EDI);
		REF_STACK(MovementZone, movementZone, STACK_OFFSET(0xBC, -0xAC));
		movementZone = pPassenger->GetTechnoType()->MovementZone; // Replace hard code MovementZone::Normal
	}

	return 0;
}

#pragma endregion


#pragma region BuildingEnterExtension

ASMJIT_PATCH(0x51EE36, InfantryClass_MouseOvetObject_NoQueueUpToEnter, 0x5)
{
	GET(ObjectClass*, pObject, ESI);
	enum { NewAction = 0x51EE3B };

	if (auto pBuilding = cast_to<BuildingClass*, false>(pObject))
	{
		const auto pRulesExt = RulesExtData::Instance();
		const auto pType = pBuilding->Type;

		if (pType->InfantryAbsorb
			&& TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToEnter.Get(
				pRulesExt->NoQueueUpToEnter_Buildings.Get(pRulesExt->NoQueueUpToEnter)))
		{
			R->EBP(Action::Repair);
			return NewAction;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x740375, UnitClass_MouseOvetObject_NoQueueUpToEnter, 0x5)
{
	GET(ObjectClass*, pObject, EDI);
	enum { NewAction = 0x74037A };

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		const auto pRulesExt = RulesExtData::Instance();
		const auto pType = static_cast<BuildingClass*>(pObject)->Type;

		if (pType->UnitAbsorb
			&& TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToEnter.Get(
				pRulesExt->NoQueueUpToEnter_Buildings.Get(pRulesExt->NoQueueUpToEnter)))
		{
			R->EBX(Action::Repair);
			return NewAction;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x73F63F, UnitClass_IsCellOccupied_NoQueueUpToEnter, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	enum { SkipGameCode = 0x73F64F };

	const auto pRulesExt = RulesExtData::Instance();
	const auto pType = pThis->Type;

	if (pType->UnitAbsorb
		&& TechnoTypeExtContainer::Instance.Find(pType)->NoQueueUpToEnter.Get(
			pRulesExt->NoQueueUpToEnter_Buildings.Get(pRulesExt->NoQueueUpToEnter)))
	{
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x4DFC83, FootClass_EnterBioReactor_NoQueueUpToUnload, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);
	enum { SkipGameCode = 0x4DFC91 };

	const auto RulesExt = RulesExtData::Instance();
	const Mission mission = TechnoTypeExtContainer::Instance.Find(pBuilding->Type)->NoQueueUpToEnter.Get(
		RulesExt->NoQueueUpToEnter_Buildings.Get(RulesExt->NoQueueUpToEnter))
		? Mission::Eaten : Mission::Enter;

	pThis->QueueMission(mission, false);
	return SkipGameCode;
}

ASMJIT_PATCH(0x44DCB1, BuildingClass_Mi_Unload_NoQueueUpToUnload, 0x7)
{
	GET(BuildingClass*, pThis, EBP);

	const auto pRulesExt = RulesExtData::Instance();

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoQueueUpToUnload.Get(
		pRulesExt->NoQueueUpToUnload_Buildings.Get(pRulesExt->NoQueueUpToUnload)))
	{
		R->EAX(0);
	}

	return 0;
}

ASMJIT_PATCH(0x519776, InfantryClass_UpdatePosition_NoQueueUpToEnter, 0x5)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);
	enum { EnterBuilding = 0x51A2AD, CannotEnter = 0x51A488 };

	const auto pType = pBuilding->Type;
	if (!pType->InfantryAbsorb)
		return 0;

	if (pType->Passengers > 0 || HouseExtData::GetTunnelVector(pBuilding->Type, pBuilding->Owner))
	{
		if (pThis->SendCommand(RadioCommand::QueryCanEnter, pBuilding) == RadioCommand::AnswerPositive) {
			if (const auto pTag = pBuilding->AttachedTag)
				pTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, CellStruct::Empty);

			R->EBP(0);
			R->EDI(pBuilding);
			return EnterBuilding;
		}

		R->EBP(0);
		return CannotEnter;
	}

	return 0;
}

ASMJIT_PATCH(0x739FA2, UnitClassClass_UpdatePosition_NoQueueUpToEnter, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);
	enum { EnterBuilding = 0x73A28A, CannotEnter = 0x73A796, SkipGameCode = 0x73A315 };

	const auto pType = pBuilding->Type;
	if (!pType->UnitAbsorb)
		return 0;

	auto pTunnel = HouseExtData::GetTunnelVector(pBuilding->Type, pBuilding->Owner);

	if (pType->Passengers > 0 || pTunnel) {
		if (pThis->SendCommand(RadioCommand::QueryCanEnter, pBuilding) == RadioCommand::AnswerPositive) {
			if (const auto pTag = pBuilding->AttachedTag)
				pTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, CellStruct::Empty);

			// This might fix a bug where hover vehicles enter tunnels.
			TechnoExtContainer::Instance.Find(pThis)->ResetLocomotor = true;

			if (pTunnel) 	{
				TunnelFuncs::EnterTunnel(&pTunnel->Vector, pBuilding, pThis);
				return SkipGameCode;
			}

			return EnterBuilding;
		}
		else
		{
			return CannotEnter;
		}
	}

	return 0;
}

#pragma endregion
