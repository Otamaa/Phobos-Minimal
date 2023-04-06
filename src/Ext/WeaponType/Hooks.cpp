#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

#pragma region Otamaa
//Ares 3.0p1 completely change the function , no reference to this anymore !
//DEFINE_HOOK(0x71AB47, TemporalClass_GetHelperDamage_AfterAres, 0x5)
//{
//	GET(WeaponStruct* const, Weapon, EAX);
//	GET(TemporalClass*, pTemp, ESI);
//
//	if (auto const TemporalWeapon = Weapon->WeaponType) {
//		TemporalExt::ExtMap.Find(pTemp)->Weapon = TemporalWeapon;
//	}
//
//	return 0;
//}
//


#pragma endregion