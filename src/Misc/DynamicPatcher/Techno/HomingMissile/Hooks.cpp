#ifdef ENABLE_HOMING_MISSILE
#include <Utilities/Macro.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

ASMJIT_PATCH(0x6B7A18, SpawnManagerClass_AI_Add_Missile_Target, 0x6)
{
	GET(SpawnManagerClass*, pManager, ESI);
	GET(AircraftClass*, pRocket, ECX);

	if (pRocket->Type->MissileSpawn) {
		if (const auto pTarget = pManager->Target) {
			if (const auto pExt = TechnoExtContainer::Instance.Find(pRocket)) {
				if (const auto pTracker = pExt->MissileTargetTracker)
				{
					pTracker->Target = pTarget;
					pTracker->AI();
				}
			}
		}

		return 0x6B7A2B;
	}

	return 0x6B7AD4;
}

ASMJIT_PATCH(0x54E42B, KamikazeTrackerClass_Add_Missile_Has_Target, 0x6)
{
	GET(TechnoClass*, pRocket, ESI);
	//GET(AbstractClass*, pTarget, ECX);

	if (const auto pExt = TechnoExtContainer::Instance.Find(pRocket)) {
		if (auto const pTracker = pExt->MissileTargetTracker) {
			pTracker->AI();
			if(auto const pRedirect = pTracker->AsAbstract()){
				R->EAX(pRedirect);
				return 0x54E475;
			}
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x6622C0, RocketLocomotionClass_Process, 0x6)
{
	GET(FootClass*, pFoot, ESI);

	if (auto const pExt = TechnoTypeExtContainer::Instance.Find(pFoot->GetTechnoType())) {
		if (pExt->MissileHoming.Get()) {
			const auto pLocomotor = static_cast<RocketLocomotionClass*>(pFoot->Locomotor.get());
			if (pLocomotor->MissionState == 0)
				pLocomotor->MissionState = 1;
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x662CAC, RocketLocomotionClass_Process_Step5_To_Lazy_4, 0x6)
{
	GET(ILocomotion*, pILoco, ESI);

	//It is an offset to ILocomotion pointer -AlexB
	if (const auto pLoco = static_cast<RocketLocomotionClass*>(pILoco)) {
		if (const auto pResult = pLoco->LinkedTo) {
			if (const auto pExt = TechnoTypeExtContainer::Instance.Find(pResult->GetTechnoType())) {
				if (pExt->MissileHoming.Get())
					return 0x662A32;
			}
		}
	}

	return 0x0;
}

#endif
