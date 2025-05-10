#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>

ASMJIT_PATCH(0x468B72, BulletClass_Unlimbo_ArcingFix, 0x5)
{
	GET(FakeBulletClass*, pThis, EBX);

	if (pThis->Type->Arcing)
		pThis->_GetExtData()->ApplyArcingFix();

	return 0;
}

// ASMJIT_PATCH(0x44D23C, BuildingClass_Mission_Missile_ArcingFix, 0x7)
// {
// 	GET(FakeWeaponTypeClass*, pWeapon, EBP);
// 	GET(int, targetHeight, EBX);
// 	GET(int, fireHeight, EAX);
//
// 	auto const pBulletType = pWeapon->Projectile;
// 	auto const pBulletTypeExt = pWeapon->_GetBulletTypeExtData();
//
// 	if (pBulletType->Arcing && targetHeight > fireHeight && !pBulletTypeExt->Arcing_AllowElevationInaccuracy) {
// 		R->EAX(targetHeight);
// 	}
//
// 	return 0;
// }