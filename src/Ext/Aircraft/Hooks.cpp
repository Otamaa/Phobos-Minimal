#include "Body.h"

#include <AircraftClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

#ifndef SecondMode
DEFINE_HOOK(0x417FE9, AircraftClass_Mission_Attack_StrafeShots, 0x7)
{
	GET(AircraftClass* const, pThis, ECX);

	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe 
		|| !pExt->Aircraft_DecreaseAmmo.Get())
	{
		return 0;
	}

	const int weaponIndex = pThis->SelectWeapon(pThis->Target);
	const auto pWeaponStr = pThis->GetWeapon(weaponIndex);

	if (pWeaponStr) {
		if (pWeaponStr->WeaponType) {
			int fireCount = pThis->MissionStatus - 4;
			if (fireCount > 1 && 
				WeaponTypeExt::ExtMap.Find(pWeaponStr->WeaponType)->Strafing_Shots < fireCount) {

					if (!pThis->Ammo)
						pThis->__DoingOverfly = false;

					pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
			}
		}
	}

	return 0;
}

//was 8
#define Hook_AircraftBurstFix(addr , Mode , ret)\
DEFINE_HOOK(addr , AircraftClass_Mission_Attack_##Mode##_Strafe_BurstFix,0x6){ \
GET(AircraftClass* const, pThis, ESI); \
AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::##Mode##); return ret; }


	Hook_AircraftBurstFix(0x4186B6, FireAt, 0x4186D7)
	Hook_AircraftBurstFix(0x418805, Strafe2, 0x418826)
	Hook_AircraftBurstFix(0x418914, Strafe3, 0x418935)
	Hook_AircraftBurstFix(0x418A23, Strafe4, 0x418A44)
	Hook_AircraftBurstFix(0x418B1F, Strafe5, 0x418B40)

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x6) //8
{
	GET(AircraftClass*, pThis, ESI);

	pThis->unknown_bool_6C8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, AircraftFireMode::FireAt);

	return 0x418478;
}

#undef Hook_AircraftBurstFix
#endif

DEFINE_HOOK(0x414F21, AircraftClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x40, 0xC));

	GameConstruct(pAnim, pThis->Type->Trailer, nCoord, 1, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false);
	AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);

	return 0x414F47;
}