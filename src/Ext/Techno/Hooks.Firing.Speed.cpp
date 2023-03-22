#include "Body.h"

bool NOINLINE IsFlyLoco(const ILocomotion* pLoco) {
	return (((DWORD*)pLoco)[0] == FlyLocomotionClass::ILoco_vtable);
}

DEFINE_HOOK(0x415F4D, AircraftClass_FireAt_SpeedModifiers, 0x6)
{
	GET(const BulletClass*, pBullet, ESI);
	GET(AircraftClass*, pThis, EDI);

	if (IsFlyLoco(pThis->Locomotor.get())) {

		if (pBullet->Type->Cluster)
			return 0x0;

		const auto pLocomotor = static_cast<FlyLocomotionClass*>(pThis->Locomotor.get());
		const double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
			TechnoExt::GetCurrentSpeedMultiplier(pThis);

		R->EAX(Game::F2I(currentSpeed));
		return 0x415F5C;
	}

	return 0;
}
