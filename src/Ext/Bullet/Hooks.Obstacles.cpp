#include "Body.h"

#include <Ext/BulletType/Body.h>

#include <Utilities/Macro.h>

#include <Misc/Ares/Hooks/AresTrajectoryHelper.h>

// Hooks
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Building/Body.h>

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

static bool IsChasing(TechnoClass* pThis, AbstractClass* pTarget)
{
	if ((pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
		return false;

	auto pFootTarget = flag_cast_to<FootClass*>(pTarget);
	if (!pFootTarget)
		return false;

	if (!pFootTarget->Locomotor.GetInterfacePtr()->Is_Moving_Now())
		return false;

	return true;
}

static bool IsPrefiring(TechnoClass* pThis, WeaponTypeClass* pWeapon)
{
	auto pTypeExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	bool includeBurst = pTypeExt->PrefiringExtraRange_IncludeBurst.Get(RulesExtData::Instance()->PrefiringExtraRange_IncludeBurst);

	if (includeBurst && pThis->CurrentBurstIndex % pWeapon->Burst != 0)
		return true;

	auto pTechnoExt = TechnoExtContainer::Instance.Find(pThis);

	if (pTechnoExt->DelayedFireTimer.InProgress())
		return true;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Unit:
	{
		auto pUnit = static_cast<UnitClass*>(pThis);
		auto currentBurst = pThis->CurrentBurstIndex % pWeapon->Burst;
		auto syncFrame = -1;

		if (currentBurst == 0)
			syncFrame = pUnit->Type->FiringSyncFrame0;
		else if (currentBurst == 1)
			syncFrame = pUnit->Type->FiringSyncFrame1;

		if (syncFrame == -1)
			return false;

		return pUnit->CurrentFiringFrame >= syncFrame;
	}
	case AbstractType::Aircraft:
	{
		auto pAircraft = static_cast<AircraftClass*>(pThis);
		auto status = (AirAttackStatus)pAircraft->MissionStatus;
		return status == AirAttackStatus::FireAtTarget
			|| status == AirAttackStatus::FireAtTarget2
			|| status == AirAttackStatus::FireAtTarget2_Strafe
			|| status == AirAttackStatus::FireAtTarget3_Strafe
			|| status == AirAttackStatus::FireAtTarget4_Strafe
			|| status == AirAttackStatus::FireAtTarget5_Strafe;
	}
	case AbstractType::Building:
	{
		auto pBuilding = static_cast<BuildingClass*>(pThis);
		auto pExt = BuildingExtContainer::Instance.Find(pBuilding);
		return pBuilding->DelayBeforeFiring || pExt->IsFiringNow;
	}
	case AbstractType::Infantry:
	{
		auto pInfantry = static_cast<InfantryClass*>(pThis);
		return pInfantry->IsFiring;
	}
	default:
		return false;
	}
}

ASMJIT_PATCH(0x6F7248, TechnoClass_InRange_Additionals, 0x6)
{
	enum { ContinueCheck = 0x6F72E3, RetTrue = 0x6F7256, RetFalse = 0x6F7655 };

	GET_BASE(AbstractClass*, pTarget, 0xC);
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	InRangeTemp::Techno = pThis;

	int range = 0;
	if (const auto keepRange = WeaponTypeExtData::GetTechnoKeepRange(pWeapon, pThis, false))
		range = keepRange;
	else {	
		range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);

		if (range != -512)
		{
			auto pRulesExt = RulesExtData::Instance();
			auto pTypeExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			auto prefiringExtraRange = pTypeExt->PrefiringExtraRange.Get(pRulesExt->PrefiringExtraRange);

			if (prefiringExtraRange
				&& IsPrefiring(pThis, pWeapon))
			{
				range += prefiringExtraRange;
			}

			auto chasingExtraRange = pTypeExt->ChasingExtraRange.Get(pThis->GetTechnoType()->CloseRange || !pRulesExt->ChasingExtraRange_CloseRangeOnly ? pRulesExt->ChasingExtraRange : Leptons(0));

			if (chasingExtraRange
				&& IsChasing(pThis, pTarget))
			{
				range += chasingExtraRange;
			}
		}
	}

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