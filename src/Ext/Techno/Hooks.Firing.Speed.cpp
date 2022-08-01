#include "Body.h"

DEFINE_HOOK(0x415F5C, AircraftClass_FireAt_SpeedModifiers, 0xA)
{
	GET(const BulletClass*, pBullet, ESI);
	GET(AircraftClass*, pThis, EDI);

	CLSID nDummy = pThis->Type->Locomotor;
	if (auto const pCurLoco = pThis->Locomotor.get())
		static_cast<LocomotionClass*>(pCurLoco)->GetClassID(&nDummy);

	if (nDummy == LocomotionClass::CLSIDs::Fly && !pBullet->Type->Cluster) {
		if (const auto pLocomotor = static_cast<FlyLocomotionClass*>(pThis->Locomotor.get())) {
			const double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
				TechnoExt::GetCurrentSpeedMultiplier(pThis);

			R->EAX(Game::F2I(currentSpeed));
		}
	}

	return 0;
}
