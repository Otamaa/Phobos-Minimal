#include "Body.h"

#include <Ext/BulletType/Body.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_Phobos, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (const auto pExt = BulletExtContainer::Instance.Find(pThis))
	{
		pExt->Owner = pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr;

		if (auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type))
		{
			pExt->CurrentStrength = pTypeExt->Health.Get();
			if (!pTypeExt->LaserTrail_Types.empty()) {
				pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());
				pExt->InitializeLaserTrails();
			}
		}
	}

	TrailsManager::Construct(pThis);

	return 0;
}