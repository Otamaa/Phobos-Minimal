#include "Body.h"

#include <Ext/BulletType/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Ares/Hooks/AresTrajectoryHelper.h>

// Hooks
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <InfantryClass.h>
#include <AircraftClass.h>

namespace InRangeTemp
{
	TechnoClass* Techno = nullptr;
}

ASMJIT_PATCH(0x6F737F, TechnoClass_InRange_WeaponMinimumRange, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDX);

	auto pTechno = InRangeTemp::Techno;

	if (const auto keepRange = WeaponTypeExtData::GetTechnoKeepRange(pWeapon, pTechno, true))
		R->ECX(keepRange);

	InRangeTemp::Techno = nullptr;
	return 0;
}

ASMJIT_PATCH(0x6F7248, TechnoClass_InRange_Additionals, 0x6)
{
	enum { ContinueCheck = 0x6F72E3, RetTrue = 0x6F7256, RetFalse = 0x6F7655 };

	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);
	GET(WeaponTypeClass*, pWeapon, EBX);

	InRangeTemp::Techno = pThis;

	int range = 0;
	if (const auto keepRange = WeaponTypeExtData::GetTechnoKeepRange(pWeapon, pThis, false))
	range = keepRange;
	else
	range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);

	if (range == -512)
		return RetTrue;

	const auto pThisTypeExt = GET_TECHNOTYPEEXT(pThis);

	if(pThisTypeExt->NavalRangeBonus.isset()){
		if (auto const pFoot = flag_cast_to<FootClass* const>(pTarget)) {
			if (pThisTypeExt->This()->Naval) {
				const auto pFootCell = pFoot->GetCell();
				if (pFootCell->LandType == LandType::Water && !pFootCell->ContainsBridge())
					range += pThisTypeExt->NavalRangeBonus.Get();
			}
		}
	}

	if (pTarget->IsInAir())
		range += pThisTypeExt->This()->AirRangeBonus;

	if (pThis->BunkerLinkedItem) {
		const auto vtable = VTable::Get(pThis->BunkerLinkedItem);

		bool clear = false;
		if ((vtable != BuildingClass::vtable) && (vtable != InfantryClass::vtable)
			&& (vtable != AircraftClass::vtable)
			&& (vtable != UnitClass::vtable))
		{
			Debug::LogInfo("TechnoClass_InRange Techno[{}] bunker linked item is broken pointer[{}] !", pThisTypeExt->This()->ID , (void*)pThis->BunkerLinkedItem);
			clear = true;
			pThis->BunkerLinkedItem = nullptr;
		}

		if(!clear && vtable != BuildingClass::vtable) {
			range += RulesClass::Instance->BunkerWeaponRangeBonus * Unsorted::LeptonsPerCell;
		}
	}


	if (pThis->InOpenToppedTransport) {
		int OpetoppedBonus = pThisTypeExt->OpenTransport_RangeBonus;

		if(auto pTrans = pThis->Transporter){
			OpetoppedBonus += GET_TECHNOTYPEEXT(pTrans)
				->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus);
		} else{
		  OpetoppedBonus += RulesClass::Instance->OpenToppedRangeBonus;
		}

		range += int(OpetoppedBonus * Unsorted::LeptonsPerCell);
	}

	R->EDI(range);
	return ContinueCheck;
}

ASMJIT_PATCH(0x6FC3A1, TechnoClass_CanFire_InBunkerRangeCheck, 0x5)
{
	enum { ContinueChecks = 0x6FC3C5, CannotFire = 0x6FC86A };

	GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	if (pTarget->WhatAmI() == AbstractType::Unit && WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) < 384.0)
		return CannotFire;

	return ContinueChecks;
}

ASMJIT_PATCH(0x70CF6F, TechnoClass_ThreatCoefficients_WeaponRange, 0x6)
{
	enum { SkipGameCode = 0x70CF75 };

	GET(TechnoClass*, pThis, EDI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	R->EAX(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

ASMJIT_PATCH(0x41810F, AircraftClass_MissionAttack_WeaponRangeCheck1, 0x6)
{
	enum { WithinDistance = 0x418117, NotWithinDistance = 0x418131 };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, distance, EAX);

	int range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, (TechnoClass*)pThis);

	if (distance < range)
		return WithinDistance;

	return NotWithinDistance;
}

ASMJIT_PATCH(0x418BA8, AircraftClass_MissionAttack_WeaponRangeCheck2, 0x6)
{
	enum { SkipGameCode = 0x418BAE };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EAX);

	R->EAX(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, (TechnoClass*)pThis));

	return SkipGameCode;
}

// Skip a forced detonation check for Level=true projectiles that is now handled in Hooks.Obstacles.cpp.
DEFINE_JUMP(LJMP, 0x468D08, 0x468D2F);
//DEFINE_SKIP_HOOK(0x468D08 , BulletClass_IsForceToExplode_SkipLevelCheck , 0x6 , 468D2F);