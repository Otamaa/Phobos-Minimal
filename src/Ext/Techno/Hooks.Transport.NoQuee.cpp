#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Macro.h>

#include <FootClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <CaptureManagerClass.h>

#include <Locomotor/HoverLocomotionClass.h>
#include <Locomotor/Cast.h>

namespace TransportUnloadTemp
{
	bool ShouldPlaySound = false;
}

static OPTIONALINLINE void PlayUnitLeaveTransportSound(UnitClass* pThis) {
	if (pThis->Type->LeaveTransportSound != -1)
		VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
}

static OPTIONALINLINE COMPILETIMEEVAL bool IsCloseEnoughToEnter(UnitClass* pTransport, FootClass* pPassenger) {
	return (Math::abs(pPassenger->Location.X - pTransport->Location.X) <= 384 
		    && Math::abs(pPassenger->Location.Y - pTransport->Location.Y) <= 384);
}

// TODO : is these check sufficient ???
static OPTIONALINLINE bool CanEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	const auto pOwner = pTransport->Owner;

	if (!pOwner || !pOwner->IsAlliedWith(pPassenger) || pTransport->IsBeingWarpedOut())
		return false;

	// Added to prevent unexpected enter action
	if (pPassenger->Deactivated || pPassenger->IsUnderEMP())
		return false;

	if (pPassenger->IsMindControlled() || pPassenger->ParasiteEatingMe)
		return false;

	const auto pManager = pPassenger->CaptureManager;

	if (pManager && pManager->IsControllingSomething())
		return false;

	const auto pTransportType = pTransport->Type;
	const auto bySize = TechnoTypeExtContainer::Instance.Find(pTransportType)->Passengers_BySize;
	const auto passengerSize = bySize ? int(pPassenger->GetTechnoType()->Size) : 1;
	
	if (passengerSize > int(pTransportType->SizeLimit))
		return false;

	const auto maxSize = pTransportType->Passengers;
	const auto predictSize = (bySize ? pTransport->Passengers.GetTotalSize() : pTransport->Passengers.NumPassengers) + passengerSize;
	const auto pLink = static_cast<FootClass*>(pTransport->GetNthLink());
	const auto needCalculate = pLink && pLink != pPassenger && pLink->Destination == pTransport;

	if (needCalculate) {
		// When the most important passenger is close, need to prevent overlap
		if (IsCloseEnoughToEnter(pTransport,pLink))
			return (predictSize <= (maxSize - int(pLink->GetTechnoType()->Size)));
	}

	return predictSize < maxSize;
}

static inline void DoEnterNow(UnitClass* pTransport, FootClass* pPassenger)
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

	const auto pPassengerType = pPassenger->GetTechnoType();

	if (auto const pCapturer = pPassenger->MindControlledBy) {
		if (const auto pCmanager = pCapturer->CaptureManager) {
			pCmanager->FreeUnit(pPassenger);
		}
	}

	if (pPassenger->GetCurrentMission() == Mission::Hunt)
		pPassenger->AbortMotion();

	pTransport->AddPassenger(pPassenger); // Don't swap order casually, very very important
	pPassenger->Transporter = pTransport;

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	if (pPassengerType->OpenTopped)
		pPassenger->SetTargetForPassengers(nullptr);

	pPassenger->Undiscover();

	pPassenger->QueueUpToEnter = nullptr; // Added, to prevent passengers from wanting to get on after getting off
	pPassenger->FrozenStill = true; // Added, to prevent the vehicles from stacking together when unloading
	pPassenger->SetSpeedPercentage(0.0); // Added, to stop the passengers and let OpenTopped work normally
}

bool OPTIONALINLINE Entered(FootClass* pThis)
{
	if (const auto pDest = cast_to<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		if (pDest->Type->Passengers > 0
			&& TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter))
		{
			if (IsCloseEnoughToEnter(pDest , pThis)) {
				if (CanEnterNow(pDest, pThis)) {
					DoEnterNow(pDest , pThis);
					return true;
				}
			}
		}
	}

	return false;
}

// Unit unload
static OPTIONALINLINE bool CanUnloadLater(UnitClass* pTransport)
{
	return pTransport->Passengers.GetFirstPassenger()
		&& (TechnoTypeExtContainer::Instance.Find(pTransport->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload)
			|| pTransport->GetCell()->LandType != LandType::Water);
}


// Rewrite from 0x4835D5/0x74004B, replace check pThis->GetCell()->LandType != LandType::Water
static COMPILETIMEEVAL OPTIONALINLINE bool CanUnloadNow(UnitClass* pTransport, FootClass* pPassenger)
{
	if (TechnoTypeExtContainer::Instance.Find(pTransport->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
		return GroundType::Array[static_cast<int>(pTransport->GetCell()->LandType)].Cost[static_cast<int>(pPassenger->GetTechnoType()->SpeedType)] != 0.0;

	return pTransport->GetCell()->LandType != LandType::Water;
}

ASMJIT_PATCH(0x73A5EA, UnitClass_UpdatePosition_NoQueueUpToEnter, 0x5)
{
	enum { EnteredThenReturn = 0x73A78C };

	GET(UnitClass* , pThis, EBP);

	return Entered(pThis) ? 0x73A78C : 0x0;
}

ASMJIT_PATCH(0x51A0D4, InfantryClass_UpdatePosition_NoQueueUpToEnter, 0x6)
{
	enum { EnteredThenReturn = 0x51A47E };

	GET(InfantryClass* , pThis, ESI);

	return Entered(pThis) ? 0x51A47E : 0x0;
}

ASMJIT_PATCH(0x73DC1E, UnitClass_Mission_Unload_NoQueueUpToUnloadLoop, 0xA)
{
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);

	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
	{
		if (!pPassenger || pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			PlayUnitLeaveTransportSound(pThis);
			TransportUnloadTemp::ShouldPlaySound = false;
			return UnloadReturn;
		}

		TransportUnloadTemp::ShouldPlaySound = true;
		R->EBX(0); // Reset
		return UnloadLoop;
	}

	// PlayAtPos has already handled the situation where Sound is less than 0 internally, so unnecessary checks will be skipped
	PlayUnitLeaveTransportSound(pThis);
	return UnloadReturn;
}

ASMJIT_PATCH(0x70D965, FootClass_QueueEnter_ForceEnter, 0x7)
{
	GET(FootClass* const, pThis, ESI);

	const auto pDest = cast_to<UnitClass*>(pThis->QueueUpToEnter);

	if (pDest
	&& TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter)
	&& !pThis->Deactivated 
	&& !pThis->IsUnderEMP()
	&& !pThis->Locomotor->Is_Moving()) // Entering while moving can cause many problems
	{
		if (IsCloseEnoughToEnter(pDest, pThis) && CanEnterNow(pDest, pThis))
				DoEnterNow(pDest,pThis);
	}

	return 0;
}

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

	// Play the sound when interrupted for some reason
	if (TransportUnloadTemp::ShouldPlaySound)
	{
		TransportUnloadTemp::ShouldPlaySound = false;
		if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
			PlayUnitLeaveTransportSound(pThis);
	}

	return SkipGameCode;
}

//Enter unit
ASMJIT_PATCH(0x73796B, UnitClass_ReceiveCommand_AmphibiousEnter, 0x7)
{
	enum { ContinueCheck = 0x737990, MoveToPassenger = 0x737974 };

	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousEnter.Get(RulesExtData::Instance()->AmphibiousEnter))
	return ContinueCheck;

	if (pThis->OnBridge)
		return MoveToPassenger;

	GET(CellClass* const, pCell, EBP);
	return (pCell->LandType != LandType::Water) ? ContinueCheck : MoveToPassenger;
}

ASMJIT_PATCH(0x74004B, UnitClass_MouseOverObject_AmphibiousUnload, 0x6)
{
	enum { ContinueCheck = 0x7400C6, CannotUnload = 0x7400BE };

	GET(UnitClass* const, pThis, ESI);

	return CanUnloadLater(pThis) ? ContinueCheck : CannotUnload;
}

ASMJIT_PATCH(0x70106A, TechnoClass_CanDeploySlashUnload_AmphibiousUnload, 0x6)
{
	enum { ContinueCheck = 0x701087, CannotUnload = 0x700DCE };

	GET(UnitClass* const, pThis, ESI);

	return !pThis->Owner->IsHumanPlayer || CanUnloadLater(pThis) ? ContinueCheck : CannotUnload;
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

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload
			.Get(RulesExtData::Instance()->AmphibiousUnload))
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

ASMJIT_PATCH(0x740C9C, UnitClass_GetUnloadDirection_CheckUnloadPosition, 0x7)
{
	GET(UnitClass* const, pThis, EDI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->AmphibiousUnload.Get(RulesExtData::Instance()->AmphibiousUnload))
	{
		if (const auto pPassenger = pThis->Passengers.GetFirstPassenger())
		{
			GET(const int, speedType, EDX);
			R->EDX(speedType 
				+ static_cast<int>(pPassenger->GetTechnoType()->SpeedType)); // Replace hard code SpeedType::Foot
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

ASMJIT_PATCH(0x4DA8A0, FootClass_Update_FastEnter, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	if (const auto pDest = cast_to<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter)){
		if(TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter)
		&& !pThis->Deactivated && !pThis->IsUnderEMP() && !pDest->Locomotor->Is_Moving())
		{
			if (IsCloseEnoughToEnter(pDest, pThis))
			{
				if (!pThis->Locomotor->Is_Moving()) // Entering while moving can cause many problems)
				{
					const auto absType = pThis->WhatAmI();

					if ((absType == AbstractType::Infantry || absType == AbstractType::Unit) && CanEnterNow(pDest, pThis))
						DoEnterNow(pDest, pThis);
				}
			}
			else if (!pThis->Destination) // Move to enter position, prevent other passengers from waiting for call and not moving early
			{
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

	return 0;
}
