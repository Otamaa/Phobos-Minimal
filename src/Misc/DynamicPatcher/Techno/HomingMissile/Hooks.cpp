#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/Macro.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x6B7A18, SpawnManagerClass_AI_Add_Missile_Target, 0xD)
{
	GET(SpawnManagerClass*, pManager, ESI);
	GET(AircraftClass*, pRocket, ECX);

	if (pRocket->Type->MissileSpawn) {
		if (auto pTarget = pManager->Target) {
			if (pTarget->IsInAir()) {
				if (auto pExt = TechnoExt::GetExtData(pRocket)) {
					pExt->IsMissileHoming = true;
				}
			}
		}

		return 0x6B7A2B;
	}

	return 0x6B7AD4;
}

DEFINE_HOOK(0x54E42B, KamikazeTrackerClass_Add_Missile_Has_Target, 0x6)
{
	GET(TechnoClass*, pRocket, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pRocket->GetTechnoType())) {
		if (pExt->MissileHoming.Get()) {
			R->EAX(R->ECX());
			return 0x54E475;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x6622C0, RocketLocomotionClass_Process, 0x6)
{
	GET(FootClass*, pFoot, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType())) {
		if (pExt->MissileHoming.Get()) {
			auto pLocomotor = static_cast<RocketLocomotionClass*>(pFoot->Locomotor.get());
			if (pLocomotor->MissionState == 0)
				pLocomotor->MissionState = 1;
		}
	}

	return 0x0;
}

#ifndef __cpp_lib_bit_cast
#include <Base/stdlib.h>
#endif

DEFINE_HOOK(0x662CAC, RocketLocomotionClass_Process_Step5_To_Lazy_4, 0x6)
{
	GET(ILocomotion*, pILoco, ESI);

	//It is an offset to ILocomotion pointer -AlexB
	if (const auto pLoco = static_cast<RocketLocomotionClass*>(pILoco)) {
		if (auto pResult = pLoco->LinkedTo) {
			if (auto pExt = TechnoTypeExt::ExtMap.Find(pResult->GetTechnoType())) {
				if (pExt->MissileHoming.Get())
					return 0x662A32;
			}
		}
	}

	return 0x0;
}

/*
static bool __fastcall RocketLoco_6620F0(RocketLocomotionClass* pThis, void* _, DWORD pDW) {
	return pThis->Func_6620F0(pDW);
}

DEFINE_POINTER_CALL(0x662CB2, &RocketLoco_6620F0);*/
#endif