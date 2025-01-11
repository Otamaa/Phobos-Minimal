#include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <FootClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <CaptureManagerClass.h>

static FORCEINLINE void PlayUnitLeaveTransportSound(UnitClass* pThis) {
	if (pThis->Type->LeaveTransportSound != -1)
		VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
}

// TODO : is these check sufficient ???
static bool CanEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	const auto pOwner = pTransport->Owner;

	if (!pOwner || !pOwner->IsAlliedWith(pPassenger) || pTransport->IsBeingWarpedOut())
		return false;

	if (pPassenger->IsMindControlled() || pPassenger->ParasiteEatingMe)
		return false;

	const auto pManager = pPassenger->CaptureManager;

	if (pManager && pManager->IsControllingSomething())
		return false;

	const auto passengerSize = pPassenger->GetTechnoType()->Size;
	const auto pTransportType = pTransport->Type;

	if (passengerSize > pTransportType->SizeLimit)
		return false;

	const auto maxSize = pTransportType->Passengers;
	const auto predictSize = pTransport->Passengers.GetTotalSize() + static_cast<int>(passengerSize);
	const auto pLink = pTransport->GetNthLink();
	const auto needCalculate = pLink && pLink != pPassenger;

	if (needCalculate)
	{
		const auto linkCell = pLink->GetCoords();
		const auto tranCell = pTransport->GetCoords();

		// When the most important passenger is close, need to prevent overlap
		if (Math::abs(linkCell.X - tranCell.X) <= 384 && Math::abs(linkCell.Y - tranCell.Y) <= 384)
			return (predictSize <= (maxSize - pLink->GetTechnoType()->Size));
	}

	const auto remain = maxSize - predictSize;

	if (remain < 0)
		return false;

	if (needCalculate && remain < static_cast<int>(pLink->GetTechnoType()->Size))
	{
		// Avoid passenger moving forward, resulting in overlap with transport and create invisible barrier
		pLink->SendToFirstLink(RadioCommand::NotifyUnlink);
		pLink->EnterIdleMode(false, true);
	}

	return true;
}

static bool Entered(FootClass* pThis)
{
	if (const auto pDest = cast_to<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		if (pDest->Type->Passengers > 0 && TechnoTypeExtContainer::Instance.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExtData::Instance()->NoQueueUpToEnter))
		{
			const auto thisCell = pThis->GetCoords();
			const auto destCell = pDest->GetCoords();

			if (Math::abs(thisCell.X - destCell.X) <= 384 && Math::abs(thisCell.Y - destCell.Y) <= 384)
			{
				if (CanEnterNow(pDest, pThis)) // Replace send radio command: QueryCanEnter
				{
					// I don't know why units have no trigger

					pThis->ArchiveTarget = nullptr;
					pThis->OnBridge = false;
					pThis->MissionAccumulateTime = 0;
					pThis->GattlingValue = 0;
					pThis->CurrentGattlingStage = 0;

					if (const auto pMind = pThis->MindControlledBy)
					{
						if (const auto pManager = pMind->CaptureManager)
							pManager->FreeUnit(pThis);
					}

					pThis->Limbo();
					pDest->AddPassenger(pThis);

					if (pDest->Type->OpenTopped)
						pDest->EnteredOpenTopped(pThis);

					pThis->Transporter = pDest;

					if (pThis->GetTechnoType()->OpenTopped)
						pThis->SetTargetForPassengers(nullptr);

					pThis->Undiscover();

					pThis->QueueUpToEnter = nullptr; // Added, to prevent passengers from wanting to get on after getting off
					pThis->SetSpeedPercentage(0.0); // Added, to stop the passengers and let OpenTopped work normally

					return true;
				}
			}
		}
	}

	return false;
}

DEFINE_HOOK(0x73A5EA, UnitClass_UpdatePosition_NoQueueUpToEnter, 0x5)
{
	enum { EnteredThenReturn = 0x73A78C };

	GET(UnitClass* const, pThis, EBP);

	return Entered(pThis) ? 0x73A78C : 0x0;
}

DEFINE_HOOK(0x51A0D4, InfantryClass_UpdatePosition_NoQueueUpToEnter, 0x6)
{
	enum { EnteredThenReturn = 0x51A47E };

	GET(InfantryClass* const, pThis, ESI);

	return Entered(pThis) ? 0x51A47E : 0x0;
}

DEFINE_HOOK(0x73DC1E, UnitClass_Mission_Unload_NoQueueUpToUnloadLoop, 0xA)
{
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExtData::Instance()->NoQueueUpToUnload))
	{
		if (pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			PlayUnitLeaveTransportSound(pThis);
			pThis->MissionStatus = 4;
			return UnloadReturn;
		}

		R->EBX(0); // Reset
		return UnloadLoop;
	}

	PlayUnitLeaveTransportSound(pThis);
	return UnloadReturn;
}