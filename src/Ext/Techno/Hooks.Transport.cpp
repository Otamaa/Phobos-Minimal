#include "Body.h"
#include <Ext/TechnoType/Body.h>

#ifndef DEBUG_CODE

#define SET_THREATEVALS(addr , techreg , name ,size , ret)\
ASMJIT_PATCH(addr, name, size) {\
GET(TechnoClass* , pThis , techreg);\
	return TechnoTypeExtContainer::Instance.Find(pThis->Transporter->GetTechnoType())->Passengers_SyncOwner.Get() ?  ret : 0; }

SET_THREATEVALS(0x6FA33C, ESI, TechnoClass_AI_ThreatEvals_OpenToppedOwner, 0x6, 0x6FA37A) //
SET_THREATEVALS(0x6F89F4, ESI, TechnoClass_EvaluateCell_ThreatEvals_OpenToppedOwner, 0x6, 0x6F8A0F)
SET_THREATEVALS(0x6F8FD7, ESI, TechnoClass_Greatest_Threat_ThreatEvals_OpenToppedOwner, 0x5, 0x6F8FDC)
SET_THREATEVALS(0x6F7EC2, EDI, TechnoClass_EvaluateObject_ThreatEvals_OpenToppedOwner, 0x6, 0x6F7EDA)

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
