#include "Body.h"

DEFINE_HOOK(0x415F5C, AircraftClass_FireAt_SpeedModifiers, 0xA)
{
	GET(const BulletClass*, pBullet, ESI);
	GET(AircraftClass*, pThis, EDI);

	auto const pLoco = static_cast<LocomotionClass*>(pThis->Locomotor.get());

	if ((((DWORD*)pLoco)[0] == FlyLocomotionClass::vtable) && !pBullet->Type->Cluster) {
		if (const auto pLocomotor = static_cast<FlyLocomotionClass*>(pLoco)) {
			const double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
				TechnoExt::GetCurrentSpeedMultiplier(pThis);

			R->EAX(Game::F2I(currentSpeed));
		}
	}

	return 0;
}
