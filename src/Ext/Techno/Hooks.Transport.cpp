#include "Body.h"
#include <Ext/TechnoType/Body.h>

#ifndef DEBUG_CODE

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
DEFINE_HOOK(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	if (auto const  pTransport = pThis->Transporter) {\
		if (TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->Passengers_SyncOwner.Get()) return ret; } return 0; }

#define SET_THREATEVALSB(addr , techreg , name ,size , ret)\
DEFINE_HOOK(addr, name, size) {\
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
DEFINE_HOOK_AGAIN(0x6FA33C, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::AI
DEFINE_HOOK_AGAIN(0x6F89F4, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateCell
DEFINE_HOOK_AGAIN(0x6F7EC2, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateObject
DEFINE_HOOK(0x6F8FD7, TechnoClass_ThreatEvals_OpenToppedOwner, 0x5)       // TechnoClass::Greatest_Threat
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

DEFINE_HOOK(0x701881, TechnoClass_ChangeHouse_Passenger_SyncOwner, 0x5)
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

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_ApplyChanges, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pThis && pPassenger)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pPassExt = TechnoExtContainer::Instance.Find(pPassenger);
		auto const pPassTypeExt = TechnoTypeExtContainer::Instance.Find(pPassenger->GetTechnoType());

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

DEFINE_HOOK(0x4DE722, FootClass_LeaveTransport, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EAX);

	if (pThis && pPassenger)
	{
		auto const pTransTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		auto pPassExt = TechnoExtContainer::Instance.Find(pPassenger);
		auto const pPassTypeExt = TechnoTypeExtContainer::Instance.Find(pPassenger->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit &&
			pPassExt->OriginalPassengerOwner)
		{
			pPassenger->SetOwningHouse(pPassExt->OriginalPassengerOwner, false);
		}
	}

	return 0;
}

// Has to be done here, before Ares survivor hook to take effect.
//DEFINE_HOOK(0x737F80, TechnoClass_ReceiveDamage_Cargo_SyncOwner, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis && pThis->Passengers.NumPassengers > 0)
//	{
//		auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
//
//		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
//		{
//			auto pPassenger = pThis->Passengers.GetFirstPassenger();
//			auto pExt = TechnoExtContainer::Instance.Find(pPassenger);
//
//			if (pExt->OriginalPassengerOwner)
//				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
//
//			while (pPassenger->NextObject)
//			{
//				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
//				pExt = TechnoExtContainer::Instance.Find(pPassenger);
//
//				if (pExt->OriginalPassengerOwner)
//					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
//			}
//		}
//	}
//
//	return 0;
//}

DEFINE_HOOK(0x710552, TechnoClass_SetOpenTransportCargoTarget_ShareTarget, 0x6)
{
	enum { ReturnFromFunction = 0x71057F , Continue = 0x0 };

	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(AbstractClass* const, pTarget, STACK_OFFSET(0x8, 0x4));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	return pTarget && !pTypeExt->OpenTopped_ShareTransportTarget
		? ReturnFromFunction : Continue;
}