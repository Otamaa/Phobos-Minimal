#include "Body.h"
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

DEFINE_HOOK(0x6F7481, TechnoClass_Targeting_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EDX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6F74A4;
}


DEFINE_HOOK_AGAIN(0x44D2AE, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
DEFINE_HOOK_AGAIN(0x44D264, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
DEFINE_HOOK(0x44D074, BuildingClass_Mission_Missile_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EBP);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	switch (R->Origin())
	{
	case 0x44D074:
		return 0x44D07A;
		break;
	case 0x44D264:
		return 0x44D26A;
		break;
	case 0x44D2AE:
		return 0x44D2B4;
		break;
	}
}

DEFINE_HOOK(0x6FDAA6, TechnoClass_FireAngle_6FDA00_ApplyGravity, 0x5)
{
	GET(WeaponTypeClass* const, pWeaponType, EDI);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6FDACE;
}

DEFINE_HOOK(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

DEFINE_HOOK(0x6FF031, TechnoClass_FireAt_ReverseVelocityWhileGravityIsZero, 0xA)
{
	GET(BulletClass*, pBullet, EBX);
	//GET(TechnoClass*, pThis, ESI);

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (pBulletExt->Trajectory &&
		pBulletExt->Trajectory->Flag != TrajectoryFlag::Invalid)
		return 0x0;

	if (pBullet->Type->Arcing && pBulletTypeExt->GetAdjustedGravity() == 0.0)
	{
		pBullet->Velocity *= -1;
		if (pBulletTypeExt->Gravity_HeightFix)
		{
			const auto speed = pBullet->Velocity.Length();

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

			const auto magnitude = pBullet->Velocity.Length();
			pBullet->Velocity *= speed / magnitude;
		}
	}

	//auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	//if (!pExt->Aircraft_DecreaseAmmo.Get() && pThis->WhatAmI() == AbstractType::Aircraft)
	//{
	//	auto nAmmo = --pThis->Ammo;

	//	if (nAmmo < 0)
	//		nAmmo = 0;

	//	pThis->Ammo = nAmmo;
	//}

	return 0;
}