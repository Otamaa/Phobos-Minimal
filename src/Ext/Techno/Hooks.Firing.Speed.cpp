#include "Body.h"

DEFINE_HOOK(0x415F5C, AircraftClass_FireAt_SpeedModifiers, 0xA)
{
	GET(AircraftClass*, pThis, EDI);

	if (pThis->Type->Locomotor == LocomotionClass::CLSIDs::Fly)
	{
		if (const auto pLocomotor = reinterpret_cast<FlyLocomotionClass*>(pThis->Locomotor.get()))
		{
			const double currentSpeed = pThis->GetTechnoType()->Speed * pLocomotor->CurrentSpeed *
				TechnoExt::GetCurrentSpeedMultiplier(pThis);

			R->EAX(Game::F2I(currentSpeed));
		}
	}

	return 0;
}
