#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <ExtraHeaders/WWCRCEngine.h>

//AircraftExt is executed after CTOR done , so this were not valid
DEFINE_HOOK(0x413F98, AircraftClass_Init, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pThisType, EDI);

	if (auto const pExt = TechnoTypeExtContainer::Instance.TryFind(pThisType)) {
		TechnoExtContainer::Instance.Find(pThis)->Attempt = pExt->Paradrop_MaxAttempt.Get();
		//pThis->___paradrop_attempts = static_cast<BYTE>(pExt->Paradrop_MaxAttempt.Get());
	}

	return 0x0;
}

DEFINE_HOOK(0x415E93, AircraftClass_DropCargo_ParaLeft, 0x7)
{
	GET(AircraftClass*, pThis, EDI);

	int nAttempt = 5;

	if (auto const pExt = TechnoTypeExtContainer::Instance.TryFind(pThis->Type)) {
		nAttempt = pExt->Paradrop_MaxAttempt.Get();
	}

	TechnoExtContainer::Instance.Find(pThis)->Attempt = nAttempt;
	//pThis->___paradrop_attempts = static_cast<BYTE>(nAttempt);
	return 0x415E9A;
}

DEFINE_HOOK(0x415950, AircraftClass_MI_Paradrop_Attempt, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	--TechnoExtContainer::Instance.Find(pThis)->Attempt;
	return 0x415956;
}

DEFINE_HOOK(0x41599F, AircraftClass_MI_Paradrop_Attempt_B, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	pThis->__DoingOverfly = false;
	return TechnoExtContainer::Instance.Find(pThis)->Attempt > 0 ? 0x4159B0 : 0x415A11;
}

//DEFINE_HOOK(0x41B657, AircraftClass_CalculateCRC_Add, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(WWCRCEngine*, pCRC, EDI);
//
//	if (auto pExt = TechnoExtContainer::Instance.TryFind(pThis))
//		pCRC->Add(pExt->Attempt);
//
//	return 0x0;
//}