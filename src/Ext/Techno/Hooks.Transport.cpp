#include "Body.h"

#define SET_THREATEVALS(addr , techAddr , name ,size , ret)\
DEFINE_HOOK(addr, name, size) {\
GET(TechnoClass* , pThis , techAddr);\
	if (auto pTransport = pThis->Transporter) {\
		if (auto const pTypeExt = TechnoTypeExt::GetExtData(pTransport->GetTechnoType())) {\
			if (pTypeExt->Passengers_SyncOwner) return ret; }} return 0; }

SET_THREATEVALS(0x6FA33C,ESI,TechnoClass_AI_ThreatEvals_OpenToppedOwner,0x6,0x6FA37A)
SET_THREATEVALS(0x6F89F4,ESI,TechnoClass_EvaluateCell_ThreatEvals_OpenToppedOwner,0x6,0x6F8A0F)
SET_THREATEVALS(0x6F8FD7,ESI,TechnoClass_Greatest_Threat_ThreatEvals_OpenToppedOwner,0x5,0x6F8FDC)
SET_THREATEVALS(0x6F7EC2,EDI,TechnoClass_EvaluateObject_ThreatEvals_OpenToppedOwner,0x6,0x6F7EDA)


#undef SET_THREATEVALS

DEFINE_HOOK(0x701881, TechnoClass_ChangeHouse_Passenger_SyncOwner, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType()))
	{
		if (pTypeExt->Passengers_SyncOwner && pThis->Passengers.NumPassengers > 0)
		{
			if (FootClass* pPassenger = pThis->Passengers.GetFirstPassenger())
			{
				pPassenger->SetOwningHouse(pThis->Owner, false);

				while (pPassenger->NextObject)
				{
					pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);

					if (pPassenger)
						pPassenger->SetOwningHouse(pThis->Owner, false);
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_SyncOwner, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pThis && pPassenger)
	{
		auto const pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType());
		auto pExt = TechnoExt::GetExtData(pPassenger);

		if (pExt && pTypeExt && pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
			pExt->OriginalPassengerOwner = pPassenger->Owner;
	}

	return 0;
}

DEFINE_HOOK(0x4DE67B, FootClass_LeaveTransport_SyncOwner, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EBX);

	if (pThis && pPassenger)
	{
		auto const pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType());
		auto pExt = TechnoExt::GetExtData(pPassenger);

		if (pExt && pTypeExt && pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit &&
			pExt->OriginalPassengerOwner)
		{
			pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
		}
	}

	return 0;
}

// Has to be done here, before Ares survivor hook to take effect.
DEFINE_HOOK(0x737F80, TechnoClass_ReceiveDamage_Cargo_SyncOwner, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis && pThis->Passengers.NumPassengers > 0)
	{
		auto const pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType());

		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
		{
			if(auto pPassenger = pThis->Passengers.GetFirstPassenger()){

			auto pExt = TechnoExt::GetExtData(pPassenger);

			if (pExt && pExt->OriginalPassengerOwner)
				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);

			while (pPassenger->NextObject)
			{
				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
				pExt = TechnoExt::GetExtData(pPassenger);

				if (pExt && pExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
			}
		 }
		}
	}

	return 0;
}