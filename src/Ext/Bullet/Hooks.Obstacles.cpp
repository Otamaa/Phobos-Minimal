#include "Body.h"

#include <Ext/BulletType/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Ares/Hooks/AresTrajectoryHelper.h>

// Hooks

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x6F7248, TechnoClass_InRange_Additionals, 0x6)
{
	enum { ContinueCheck = 0x6F72E3, RetTrue = 0x6F7256, RetFalse = 0x6F7655 };

	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (!pTarget || !pWeapon)
		return RetFalse;

	int range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);

	if (range == -512)
		return RetTrue;

	const auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if(pThisTypeExt->NavalRangeBonus.isset()){
		if (auto const pFoot = abstract_cast<FootClass* const>(pTarget)) {
			if (pThisTypeExt->AttachedToObject->Naval) {
				const auto pFootCell = pFoot->GetCell();
				if (pFootCell->LandType == LandType::Water && !pFootCell->ContainsBridge())
					range += pThisTypeExt->NavalRangeBonus.Get();
			}
		}
	}

	if (pTarget->IsInAir())
		range += pThisTypeExt->AttachedToObject->AirRangeBonus;

	if (pThis->BunkerLinkedItem && pThis->BunkerLinkedItem->WhatAmI() != AbstractType::Building)
		range += RulesClass::Instance->BunkerWeaponRangeBonus * Unsorted::LeptonsPerCell;

	if (pThis->Transporter)
	{
		range += TechnoTypeExtContainer::Instance.Find(pThis->Transporter->GetTechnoType())
			->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus) * Unsorted::LeptonsPerCell;
	}

	R->EDI(range);
	return ContinueCheck;
}

DEFINE_HOOK(0x6FC3A1, TechnoClass_CanFire_InBunkerRangeCheck, 0x5)
{
	enum { ContinueChecks = 0x6FC3C5, CannotFire = 0x6FC86A };

	GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);


	if (pTarget->WhatAmI() == AbstractType::Unit && WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) < 384.0)
		return CannotFire;

	return ContinueChecks;
}

DEFINE_HOOK(0x70CF6F, TechnoClass_ThreatCoefficients_WeaponRange, 0x6)
{
	enum { SkipGameCode = 0x70CF75 };

	GET(TechnoClass*, pThis, EDI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	R->EAX(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

DEFINE_HOOK(0x41810F, AircraftClass_MissionAttack_WeaponRangeCheck1, 0x6)
{
	enum { WithinDistance = 0x418117, NotWithinDistance = 0x418131 };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, distance, EAX);

	int range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);

	if (distance < range)
		return WithinDistance;

	return NotWithinDistance;
}

DEFINE_HOOK(0x418BA8, AircraftClass_MissionAttack_WeaponRangeCheck2, 0x6)
{
	enum { SkipGameCode = 0x418BAE };

	GET(AircraftClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EAX);

	R->EAX(WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis));

	return SkipGameCode;
}

// Skip a forced detonation check for Level=true projectiles that is now handled in Hooks.Obstacles.cpp.
DEFINE_JUMP(LJMP, 0x468D08, 0x468D2F);
//DEFINE_SKIP_HOOK(0x468D08 , BulletClass_IsForceToExplode_SkipLevelCheck , 0x6 , 468D2F);