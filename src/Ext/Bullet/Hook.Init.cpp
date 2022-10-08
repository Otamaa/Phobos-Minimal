#include "Body.h"

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_Phobos, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (const auto pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->Owner = pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr;

		if (pThis->Type)
		{
			const auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
			{
				pExt->TypeExt = pTypeExt;
				pExt->CurrentStrength = pTypeExt->Health.Get();
				if (pTypeExt->LaserTrail_Types.size() > 0)
					pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

				pExt->InitializeLaserTrails(pTypeExt);
			}
		}

		//LineTrailExt::ConstructLineTrails(pThis);
	}


#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Construct(pThis);
#endif

	return 0;
}