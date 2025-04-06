#include "Body.h"

#include <Ext/TechnoType/Body.h>

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

// Rewrite from 0x51A21B
void OPTIONALINLINE InfantryEnterNow(UnitClass* pTransport, InfantryClass* pPassenger)
{
	if (const auto pTag = pTransport->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::EnteredBy, pPassenger, CellStruct::Empty);

	pPassenger->ArchiveTarget = nullptr;
	pPassenger->OnBridge = false;
	pPassenger->MissionAccumulateTime = 0;
	pPassenger->GattlingValue = 0;
	pPassenger->CurrentGattlingStage = 0;

	/* Have checked in CanEnterNow
	if (const auto pMind = pPassenger->MindControlledBy)
	{
		if (const auto pManager = pMind->CaptureManager)
			pManager->FreeUnit(pPassenger);
	}
	*/

	pPassenger->Limbo();

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pTransport;
	pTransport->AddPassenger(pPassenger);
	pPassenger->Undiscover();

	// Added, to prevent passengers from wanting to get on after getting off
	pPassenger->QueueUpToEnter = nullptr;

	// Added, to stop the passengers and let OpenTopped work normally
	pPassenger->SetSpeedPercentage(0.0);

	// Added, to stop hover unit's meaningless behavior
	if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pPassenger->Locomotor))
		pHover->__Height = 0;
}

// Rewrite from 0x73A6D1
void OPTIONALINLINE UnitEnterNow(UnitClass* pTransport, UnitClass* pPassenger)
{
	// I don't know why units have no trigger

	pPassenger->ArchiveTarget = nullptr;
	pPassenger->OnBridge = false;
	pPassenger->MissionAccumulateTime = 0;
	pPassenger->GattlingValue = 0;
	pPassenger->CurrentGattlingStage = 0;

	/* Have checked in CanEnterNow
	if (const auto pMind = pPassenger->MindControlledBy)
	{
		if (const auto pManager = pMind->CaptureManager)
			pManager->FreeUnit(pPassenger);
	}
	*/

	pPassenger->Limbo();
	pTransport->AddPassenger(pPassenger);

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pTransport;

	if (pPassenger->Type->OpenTopped)
		pPassenger->SetTargetForPassengers(nullptr);

	pPassenger->Undiscover();

	// Added, to prevent passengers from wanting to get on after getting off
	pPassenger->QueueUpToEnter = nullptr;

	// Added, to stop the passengers and let OpenTopped work normally
	pPassenger->SetSpeedPercentage(0.0);

	// Added, to stop hover unit's meaningless behavior
	if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pPassenger->Locomotor))
		pHover->__Height = 0;
}

template<typename T>
bool OPTIONALINLINE Entered(T* pThis)
{
	if (const auto pDest = cast_to<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		if (pDest->Type->Passengers > 0 
			&& TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter))
		{
			const auto delta = pThis->GetCoords() - pDest->GetCoords();

			if (abs(delta.X) <= 384 && abs(delta.Y) <= 384)
			{
				if (CanEnterNow(pDest, pThis))
				{
					if constexpr (T::AbsID == AbstractType::Infantry)
						InfantryEnterNow(pDest, pThis);
					else
						UnitEnterNow(pDest, pThis);

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
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289, NoUnloadReturn = 0x73D8AA };

	GET(UnitClass* const, pThis, ESI);

	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
	{
		if (!pPassenger || pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			PlayUnitLeaveTransportSound(pThis);
			TransportUnloadTemp::ShouldPlaySound = false;
			pThis->MissionStatus = 4; // Then guard
			return UnloadReturn;
		}
		else if (!CanUnloadNow(pThis, pPassenger))
		{
			PlayUnitLeaveTransportSound(pThis);
			TransportUnloadTemp::ShouldPlaySound = false;
			pThis->MissionStatus = 0; // Retry
			return NoUnloadReturn;
		}

		TransportUnloadTemp::ShouldPlaySound = false;
		R->EBX(0); // Reset
		return UnloadLoop;
	}

	// PlayAtPos has already handled the situation where Sound is less than 0 internally, so unnecessary checks will be skipped
	PlayUnitLeaveTransportSound(pThis);

	if (!pPassenger || CanUnloadNow(pThis, pPassenger))
		return UnloadReturn;

	pThis->MissionStatus = 0; // Retry
	return NoUnloadReturn;
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
		const auto absType = pThis->WhatAmI();
		// When the distance is very close, the passengers may not move, which can cause UpdatePosition to not be called
		// So special handling is needed here, avoid not being able to trigger quick boarding without moving
		if (absType == AbstractType::Infantry)
		{
			if (IsCloseEnoughToEnter(pDest, pThis) && CanEnterNow(pDest, pThis))
				InfantryEnterNow(pDest, static_cast<InfantryClass*>(pThis));
		}
		else if (absType == AbstractType::Unit)
		{
			if (IsCloseEnoughToEnter(pDest, pThis) && CanEnterNow(pDest, pThis))
				UnitEnterNow(pDest, static_cast<UnitClass*>(pThis));
		}
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

ASMJIT_PATCH(0x70D894, FootClass_UpdateEnter_UpdateEnterPosition, 0x7)
{
	GET(FootClass* const, pThis, ESI);
	GET(UnitClass* const, pDest, EDI); // Is techno not unit, only for convenience

	if (pDest->WhatAmI() == AbstractType::Unit 
	&& TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter)
	&& !pThis->Deactivated && !pThis->IsUnderEMP() && !pDest->Locomotor->Is_Moving())
	{
		if (IsCloseEnoughToEnter(pDest, pThis))
		{
			if (!pThis->Locomotor->Is_Moving()) // Entering while moving can cause many problems)
			{
				{
					const auto absType = pThis->WhatAmI();

					if (absType == AbstractType::Infantry)
					{
						if (CanEnterNow(pDest, pThis))
							InfantryEnterNow(pDest, static_cast<InfantryClass*>(pThis));
					}
					else if (absType == AbstractType::Unit)
					{
						if (CanEnterNow(pDest, pThis))
							UnitEnterNow(pDest, static_cast<UnitClass*>(pThis));
					}
				}
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

	return 0;
}

ASMJIT_PATCH(0x737945, UnitClass_ReceiveCommand_MoveTransporter, 0x7)
{
	enum { SkipGameCode = 0x737952 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	// Move to the vicinity of the passenger
	auto cell = CellStruct::Empty;
	pThis->NearbyLocation(&cell , pPassenger);
	pThis->SetDestination((cell != CellStruct::Empty ? static_cast<AbstractClass*>(MapClass::Instance->GetCellAt(cell)) : pPassenger), true);

	return SkipGameCode;
}
