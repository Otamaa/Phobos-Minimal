#include "Body.h"

#include <Ext/BulletType/Body.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_Phobos, 0x6)
{
	GET(FakeBulletClass*, pThis, ECX);

	const auto pExt = pThis->_GetExtData();

	pExt->Owner = pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr;

	auto const pTypeExt = pThis->_GetTypeExtData();
	pExt->CurrentStrength = pTypeExt->Health.Get();
	if (!pTypeExt->LaserTrail_Types.empty()) {
		pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());
		pExt->InitializeLaserTrails();
	}

	TrailsManager::Construct(pThis->_AsBullet());

	return 0;
}