#include "Body.h"

#include <AircraftClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x417FE9, AircraftClass_Mission_Attack_StrafeShots, 0x7)
{
	GET(AircraftClass* const, pThis, ECX);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus > (int)AirAttackStatus::FireAtTarget5_Strafe) {
		return 0;
	}

	const int weaponIndex = pThis->SelectWeapon(pThis->Target);
	const auto pWeaponStr = pThis->GetWeapon(weaponIndex);

	if (pWeaponStr) {
		if (pWeaponStr->WeaponType) {
			if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponStr->WeaponType)) {
				int fireCount = pThis->MissionStatus - 4;
				if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount)
				{
					if (!pThis->Ammo)
						pThis->__DoingOverfly = false;

					pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
				}
			}
		}
	}

	return 0;
}

#define Hook_AircraftBurstFix(addr , Mode , ret)\
DEFINE_HOOK(addr , AircraftClass_Mission_Attack_##Mode##_Strafe_BurstFix,0x8){ \
GET(AircraftClass* const, pThis, ESI); \
AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::##Mode##); return ret; }

Hook_AircraftBurstFix(0x4186B6,FireAt,0x4186D7)
Hook_AircraftBurstFix(0x418805,Strafe2,0x418826)
Hook_AircraftBurstFix(0x418914,Strafe3,0x418935)
Hook_AircraftBurstFix(0x418A23,Strafe4,0x418A44)
Hook_AircraftBurstFix(0x418B1F,Strafe5,0x418B40)

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	pThis->unknown_bool_6C8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt);

	return 0x418478;
}

/*
DEFINE_HOOK(0x4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt);

	return 0x4186D7;
}

DEFINE_HOOK(0x418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe2);

	return 0x418826;
}

DEFINE_HOOK(0x418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe3);

	return 0x418935;
}

DEFINE_HOOK(0x418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe4);

	return 0x418A44;
}

DEFINE_HOOK(0x418B1F, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::Strafe5);

	return 0x418B40;
}
*/

#undef Hook_AircraftBurstFix

DEFINE_HOOK(0x414F21, AircraftClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0xC));

	GameConstruct(pAnim, pThis->Type->Trailer, nCoord, 1, 1, 0x600, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x414F47;
}