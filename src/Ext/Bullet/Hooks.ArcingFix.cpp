#include "Body.h"
#include <Ext/BulletType/Body.h>

DEFINE_HOOK(0x6FE657, TechnoClass_FireAt_ArcingFix, 0x6)
{
	GET_STACK(BulletTypeClass*, pBulletType, STACK_OFFSET(0xB0, -0x48));
	GET(int, targetHeight, EDI);
	GET(int, fireHeight, EAX);

	if (pBulletType->Arcing && targetHeight > fireHeight)
	{
		auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);

		if (!pBulletTypeExt->Arcing_AllowElevationInaccuracy)
			R->EAX(targetHeight);
	}

	return 0;
}

DEFINE_HOOK(0x44D23C, BuildingClass_Mission_Missile_ArcingFix, 0x7)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET(int, targetHeight, EBX);
	GET(int, fireHeight, EAX);

	auto const pBulletType = pWeapon->Projectile;

	if (pBulletType->Arcing && targetHeight > fireHeight)
	{
		auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);

		if (!pBulletTypeExt->Arcing_AllowElevationInaccuracy)
			R->EAX(targetHeight);
	}

	return 0;
}