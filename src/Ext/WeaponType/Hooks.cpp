#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x7729F0, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, ECX);

	const auto pBullet = pWeapon->Projectile;
	if (pBullet && !pBullet->ROT) {
		const auto gravity = BulletTypeExtData::GetAdjustedGravity(pBullet);
		pWeapon->Speed = Game::AdjustRangeWithGravity(pWeapon->Range, gravity);
	}

	return 0x772A4C;
}

DEFINE_HOOK(0x773070, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, ECX);

	if (!pWeapon->Projectile || pWeapon->Projectile->ROT) {
		R->EAX(pWeapon->Speed);
		return 0x7730C9;
	}

	const auto  gravity = BulletTypeExtData::GetAdjustedGravity(pWeapon->Projectile);
	R->EAX(Game::AdjustRangeWithGravity(pWeapon->Range, gravity));
	return 0x7730C9;
}
