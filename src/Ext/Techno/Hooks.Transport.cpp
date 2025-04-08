#include "Body.h"
#include <Ext/TechnoType/Body.h>

#ifndef DEBUG_CODE

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	if (auto const  pTransport = pThis->Transporter) {\
		if (TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->Passengers_SyncOwner.Get()) return ret; } return 0; }

#define SET_THREATEVALSB(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	if (auto const  pTransport = pThis->Transporter) {\
	  if (TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->Passengers_SyncOwner.Get()) return ret; } return 0; }

SET_THREATEVALS(0x6FA33C,ESI,TechnoClass_AI_ThreatEvals_OpenToppedOwner,0x6,0x6FA37A) //
SET_THREATEVALSB(0x6F89F4,ESI,TechnoClass_EvaluateCell_ThreatEvals_OpenToppedOwner,0x6,0x6F8A0F)
SET_THREATEVALS(0x6F8FD7,ESI,TechnoClass_Greatest_Threat_ThreatEvals_OpenToppedOwner,0x5,0x6F8FDC)
SET_THREATEVALS(0x6F7EC2,EDI,TechnoClass_EvaluateObject_ThreatEvals_OpenToppedOwner,0x6,0x6F7EDA)

#undef SET_THREATEVALS
#undef SET_THREATEVALSB
#else
ASMJIT_PATCH_AGAIN(0x6FA33C, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::AI
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

ASMJIT_PATCH(0x701881, TechnoClass_ChangeHouse_Passenger_SyncOwner, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->Passengers_SyncOwner && pThis->Passengers.NumPassengers > 0)
	{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pPassenger)
				pPassenger->SetOwningHouse(pThis->Owner, false);

			while (pPassenger && pPassenger->NextObject)
			{
				pPassenger = flag_cast_to<FootClass*>(pPassenger->NextObject);

				if (pPassenger)
					pPassenger->SetOwningHouse(pThis->Owner, false);
			}
	}

	return 0;
}

ASMJIT_PATCH(0x71067B, TechnoClass_EnterTransport_ApplyChanges, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pThis && pPassenger)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pPassExt = TechnoExtContainer::Instance.Find(pPassenger);
		//auto const pPassTypeExt = TechnoTypeExtContainer::Instance.Find(pPassenger->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pPassExt->OriginalPassengerOwner = pPassenger->Owner;

		if (!pPassExt->LaserTrails.empty())
		{
			for (auto& pLaserTrail : pPassExt->LaserTrails)
			{
				pLaserTrail.Visible = false;
				pLaserTrail.LastLocation.clear();
			}
		}

		TrailsManager::Hide((TechnoClass*)pPassenger);
	}

	return 0;
}

ASMJIT_PATCH(0x4DE722, FootClass_LeaveTransport, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EAX);

	if (pThis && pPassenger)
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
	enum { ReturnFromFunction = 0x71057F , Continue = 0x0 };

	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(AbstractClass* const, pTarget, STACK_OFFSET(0x8, 0x4));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	return pTarget && !pTypeExt->OpenTopped_ShareTransportTarget
		? ReturnFromFunction : Continue;
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

#include <Locomotor/LocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>

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