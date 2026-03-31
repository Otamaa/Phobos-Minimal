#include "Body.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/BulletType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Locomotor/Cast.h>

#include <AircraftClass.h>

// 0x6F3330 — Full reimplementation, all Phobos/Ares hooks integrated
int __fastcall FakeTechnoClass::__WhatWeaponShouldIUse(TechnoClass* pThis, discard_t , AbstractClass* pTarget)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	// ===== Interceptor targeting bullets (hook 0x6F3339) =====
	if (pTarget
		&& TechnoExtContainer::Instance.Find(pThis)->IsInterceptor()
		&& pTarget->WhatAmI() == AbstractType::Bullet)
	{
		const int iw = pTypeExt->Interceptor_Weapon.Get();
		return (iw < 0) ? 0 : iw;
	}

	// ===== Turret weapon slot / MultiWeapon (hook 0x6F3339 + vanilla 0x6F3343) =====
	if (pType->TurretCount > 0 && !pType->IsGattling)
	{
		// MultiWeapon non-Gunner units skip the turret slot and fall through
		if (!pTypeExt->MultiWeapon
			|| (pThis->WhatAmI() == AbstractType::Unit && pType->Gunner))
		{
			const int slot = pThis->CurrentWeaponNumber; // +0x138
			return (slot != -1) ? slot : 0;
		}
	}

	// ===== Is_Occupied → primary (vanilla 0x6F3379) =====
	if (pThis->CanOccupyFire()) // vtable +0x400
		return 0;

	// ===== Weapon null checks (vanilla 0x6F338B–0x6F33C7) =====
	auto const pSecondary = pThis->GetWeapon(1)->WeaponType;
	if (!pSecondary)
		return 0;

	auto const pPrimary = pThis->GetWeapon(0)->WeaponType;
	if (!pPrimary || pSecondary->NeverUse)
		return 0;

	if (!pTarget)
		return 0;

	// ===== ForceFire on cells (hook 0x6F33CD) =====
	if (const auto pCell = cast_to<CellClass*>(pTarget))
	{
		auto const pPrimaryExt = WeaponTypeExtContainer::Instance.Find(pPrimary);

		if (!pPrimaryExt->SkipWeaponPicking
			&& (!EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true, true)
				|| (pPrimaryExt->AttachEffect_CheckOnFirer
					&& !pPrimaryExt->HasRequiredAttachedEffects(pThis, pThis)))
			&& (!pTypeExt->NoSecondaryWeaponFallback
				|| TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)))
		{
			return 1;
		}
		else if (pCell->OverlayTypeIndex != -1)
		{
			auto const pOverlayType = OverlayTypeClass::Array()->Items[pCell->OverlayTypeIndex];

			if (pOverlayType->Wall
				&& (pCell->OverlayData >> 4) != pOverlayType->DamageLevels)
			{
				return TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayType);
			}
		}
	}

	// ===== OpenTransportWeapon (vanilla 0x6F33D9) =====
	if (pThis->InOpenToppedTransport) // +0x82
	{
		const int otw = pType->OpenTransportWeapon; // TechnoTypeClass+0xD50
		if (otw != -1)
			return otw;
	}

	// ===== NoAmmoWeapon (hook 0x6F3410) =====
	if (pType->Ammo >= 0
		&& pTypeExt->NoAmmoWeapon >= 0
		&& pThis->Ammo <= pTypeExt->NoAmmoAmount)
	{
		return pTypeExt->NoAmmoWeapon;
	}

	// ===== Resolve target to TechnoClass* (vanilla 0x6F3410 AbstractFlags bit check) =====
	auto const pTargetTechno = flag_cast_to<TechnoClass*>(pTarget);

	// ===== ForceWeapon / SelectMultiWeapon (hook 0x6F3428) =====
	const int phobosWeapon = pTypeExt->SelectMultiWeapon(pThis, pTarget);
	if (phobosWeapon >= 0)
		return phobosWeapon;

	// =========================================================================
	// Gattling path (hook 0x6F3432 — full replacement)
	// =========================================================================
	if (pType->IsGattling)
	{
		const int oddIdx = 2 * pThis->CurrentGattlingStage; // +0x140
		const int evenIdx = oddIdx + 1;

		const int eligibleIdx = TechnoExtData::PickWeaponIndex(
			pThis, pTargetTechno, pTarget, oddIdx, evenIdx, true, true);

		if (eligibleIdx != -1)
			return eligibleIdx;

		int chosenIdx = oddIdx;

		if (pTargetTechno)
		{
			auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			auto const pWeaponEven = pThis->GetWeapon(evenIdx)->WeaponType;
			auto const pShield = pTargetExt->Shield.get();
			auto const armor = pTargetTechno->GetTechnoType()->Armor;
			const bool inAir = pTargetTechno->IsInAir();
			const bool isUnderground = pTargetTechno->InWhichLayer() == Layer::Underground;

			auto isWeaponValid = [&](WeaponTypeClass* pWeapon) -> bool
				{
					if (inAir && !pWeapon->Projectile->AA)
						return false;
					if (isUnderground && !BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AU)
						return false;
					if (pShield && pShield->IsActive() && !pShield->CanBeTargeted(pWeapon))
						return false;
					if (GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, armor) == 0.0)
						return false;
					return true;
				};

			// Even weapon (secondary-side) invalid → stick with odd (primary-side)
			if (!isWeaponValid(pWeaponEven))
				return chosenIdx;

			// Naval targeting
			if (!pTargetTechno->OnBridge && !inAir)
			{
				const auto landType = pTargetTechno->GetCell()->LandType;
				if (landType == LandType::Water || landType == LandType::Beach)
				{
					if (pThis->SelectNavalTargeting(pTargetTechno)
						== NavalTargetingType::Underwater_secondary)
					{
						chosenIdx = evenIdx;
					}
					return chosenIdx;
				}
			}

			// Odd weapon (primary-side) invalid or LandTargeting override
			auto const pWeaponOdd = pThis->GetWeapon(oddIdx)->WeaponType;
			if (!isWeaponValid(pWeaponOdd)
				|| pType->LandTargeting == LandTargetingType::Land_secondary)
			{
				chosenIdx = evenIdx;
			}
		}

		return chosenIdx;
	}

	// =========================================================================
	// Non-gattling path
	// =========================================================================
	auto const pSecondaryWH = pSecondary->Warhead;

	// ===== Airstrike (hook 0x6F348F — full replacement) =====
	if (pSecondaryWH->Airstrike)
	{
		if (!pTargetTechno)
			return 0;

		auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pSecondaryWH);

		if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets, false))
			return 0;

		auto const pTargetType = pTargetTechno->GetTechnoType();
		auto const pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);

		if (pTargetTechno->AbstractFlags & AbstractFlags::Foot)
			return pTargetTypeExt->AllowAirstrike.Get(true) ? 1 : 0;

		// Building path — only non-Foot TechnoClass subtype is BuildingClass
		auto const pBldgType = static_cast<BuildingTypeClass*>(pTargetType);
		return (pTargetTypeExt->AllowAirstrike.Get(pBldgType->CanC4)
			&& (!pBldgType->ResourceDestination || !pBldgType->ResourceGatherer))
			? 1 : 0;
	}

	// ===== IsLocomotor — both weapons (hook 0x6F3528) =====
	if (pTargetTechno && pTargetTechno->WhatAmI() == AbstractType::Building)
	{
		if (pPrimary->Warhead->IsLocomotor)
			return 1;

		if (pSecondary->Warhead->IsLocomotor)
			return 0;
	}

	// ===== DrainWeapon (hook 0x6F357F extends vanilla with DrainingMe) =====
	if (pSecondary->DrainWeapon
		&& pTargetTechno
		&& pTargetTechno->GetTechnoType()->Drainable
		&& !pThis->DrainTarget       // +0x1CC
		&& !pTargetTechno->DrainingMe // Phobos addition
		&& !pThis->Owner->IsAlliedWith(pTargetTechno))
	{
		return 1;
	}

	// ===== AreaFire during Unload (vanilla 0x6F35A8) =====
	if (pSecondary->AreaFire
		&& pThis->GetCurrentMission() == Mission::Unload) // vtable +0x184 == 0x10
	{
		return 1;
	}

	// ===== Self is Building — flag at +0x661 (vanilla 0x6F35D4) =====
	// VERIFY: BuildingClass+0x661 — IDA shows BYTE1(SightTimer.Timer), likely a
	//         byte flag embedded at that offset (e.g. BuildingClass::IsCharging?)
	if (pThis->WhatAmI() == AbstractType::Building
		&& static_cast<BuildingClass*>(pThis)->IsOverpowered)
	{
		return 1;
	}

	// ===== ElectricAssault on allied building (vanilla 0x6F35F9) =====
	if (pTarget->WhatAmI() == AbstractType::Building
		&& pThis->Owner->IsAlliedWith(pTarget)
		&& pSecondaryWH->ElectricAssault)
	{
		auto const pBldg = static_cast<BuildingClass*>(pTarget);
		// VERIFY: [BuildingClass+0x520] → [+0x1575] — possibly BuildingTypeClass::HasPower
		//         or similar flag accessed via an indirection pointer
		if (pBldg->Type->Overpowerable)
		{
			return 1;
		}
	}

	// ===== Self is Unit — flag at +0x6CA (vanilla 0x6F3648) =====
	// VERIFY: UnitClass+0x6CA — IDA shows BYTE2(FollowerCar), likely a byte flag
	//         (e.g. UnitClass::HasDeployFireWeapon?)
	if (pThis->WhatAmI() == AbstractType::Aircraft
		&& static_cast<AircraftClass*>(pThis)->IsKamikaze)
	{
		return 1;
	}

	// ===== Cell targeting on dry land (vanilla 0x6F3671) =====
	if (pTarget->WhatAmI() == AbstractType::Cell)
	{
		auto const pCell = static_cast<CellClass*>(pTarget);

		const bool bDryLandOrNaval =
			(pCell->LandType != LandType::Water && pTarget->IsOnFloor())  // vtable +0x50
			|| ((pCell->UINTAltFlags & 0x100) && pType->Naval);              // CellClass+0x140 bit 8

		if (bDryLandOrNaval
			&& !pTarget->IsInAir()                                        // vtable +0x54
			&& pType->LandTargeting == LandTargetingType::Land_secondary) // +0x604 == 2
		{
			return 1;
		}
	}

	// =========================================================================
	// Extended weapon selection — shields, armor, naval, AA+AU
	// (hooks 0x6F36DB + vanilla 0x6F3754 + hook 0x6F37EB)
	// =========================================================================
	if (pTargetTechno)
	{
		const bool allowFallback = !pTypeExt->NoSecondaryWeaponFallback;
		const bool allowAAFallback = allowFallback || pTypeExt->NoSecondaryWeaponFallback_AllowAA;

		// --- Phobos PickWeaponIndex (hook 0x6F36DB entry) ---
		const int pickResult = TechnoExtData::PickWeaponIndex(
			pThis, pTargetTechno, pTarget, 0, 1, allowFallback, allowAAFallback);

		if (pickResult != -1)
			return pickResult;

		if (!pTargetTechno->IsAlive)
			return 0;

		// --- Shield check (hook 0x6F36DB middle) ---
		auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
		const auto pShield = pTargetExt->GetShield();

		if (pShield && pShield->IsActive())
		{
			const bool secondaryIsAA =
				pTargetTechno->IsInAir() && pSecondary->Projectile->AA;

			const bool canUseSecondary = allowFallback
				|| (allowAAFallback && secondaryIsAA)
				|| (pTargetTechno->InWhichLayer() == Layer::Underground
					&& BulletTypeExtContainer::Instance.Find(pSecondary->Projectile)->AU)
				|| TechnoExtData::CanFireNoAmmoWeapon(pThis, 1);

			if (!canUseSecondary)
				return 0;

			if (!pShield->CanBeTargeted(pPrimary))
				return 1;

			// Shield active but primary CAN target it — fall through to naval/AA tiebreakers
		}
		else
		{
			// --- Armor/verses check (hook 0x6F36DB tail — extended armor types) ---
			const int nArmor = static_cast<int>(TechnoExtData::GetArmor(pTargetTechno));

			const auto vsSecondary =
				&WarheadTypeExtContainer::Instance.Find(pSecondaryWH)->Verses[nArmor];

			if (vsSecondary->Verses == 0.0)
				return 0;

			const auto vsPrimary =
				&WarheadTypeExtContainer::Instance.Find(pPrimary->Warhead)->Verses[nArmor];

			if (vsPrimary->Verses == 0.0)
				return 1;

			// Both weapons can damage — fall through to naval/AA tiebreakers
		}

		// --- Naval / bridge tiebreaker (vanilla 0x6F3754) ---
		{
			auto const pTargetCell = pTargetTechno->GetCell();
			bool bOnWater = (pTargetCell->LandType == LandType::Water
						  || pTargetCell->LandType == LandType::Beach);

			if (pTargetTechno->IsInAir())
				bOnWater = false;

			if (!pTargetTechno->OnBridge && bOnWater)
			{
				const auto navalResult = pThis->SelectNavalTargeting(pTarget);
				// ??????
				return int((navalResult != NavalTargetingType::NotAvaible) ? navalResult : NavalTargetingType::Underwater_never);
			}
		}

		// --- LandTargeting == Land_secondary (vanilla 0x6F37B9) ---
		if (!pTargetTechno->IsInAir()
			&& pType->LandTargeting == LandTargetingType::Land_secondary)
		{
			return 1;
		}

		// --- Anti-Air + Anti-Underground (hook 0x6F37EB) ---
		{
			auto const pPrimaryProj = pPrimary->Projectile;
			auto const pSecondaryProj = pSecondary->Projectile;

			if (!pPrimaryProj->AA && pSecondaryProj->AA
				&& pTargetTechno->IsInAir())
			{
				return 1;
			}

			if (BulletTypeExtContainer::Instance.Find(pSecondaryProj)->AU
				&& !BulletTypeExtContainer::Instance.Find(pPrimaryProj)->AU
				&& pTargetTechno->InWhichLayer() == Layer::Underground)
			{
				return 1;
			}
		}
	}

	return 0;
}

//idk who calling this or replacing this thru vtable , so keep this simple like this
DEFINE_FUNCTION_JUMP(LJMP, 0x6F3330, FakeTechnoClass::__WhatWeaponShouldIUse);

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
ASMJIT_PATCH(0x70E1A0, TechnoClass_GetTurretWeapon_LaserWeapon, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(DWORD, caller, 0x0);

	if (!pThis)
		Debug::FatalError("Caller %u ", caller);

	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (!pExt->CurrentLaserWeaponIndex.empty())
		{
			R->EAX(pThis->GetWeapon(pExt->CurrentLaserWeaponIndex));
			return 0x70E1C8;
		}
	}

	return 0;
}

#ifdef _old

ASMJIT_PATCH(0x6F3528, TechnoClass_WhatWeaponShouldIUse_IsLocomotor, 0x6)
{
	enum { ContinueAfter = 0x6F3558, Primary = 0x6F37AD, Secondary = 0x6F3549 };

	GET(TechnoClass*, pTargetTechno, EBP);

	if (pTargetTechno && pTargetTechno->WhatAmI() == AbstractType::Building)
	{
		GET(WeaponTypeClass*, pPrimary, EBX);

		if (pPrimary->Warhead->IsLocomotor)
			return Secondary;

		GET_STACK(WeaponTypeClass*, pSecondary, STACK_OFFSET(0x18, -0x8));

		if (pSecondary->Warhead->IsLocomotor)
			return Primary;
	}

	return ContinueAfter;
}

// Weapon Selection
// TODO : check
ASMJIT_PATCH(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum {
		ReturnGameCode = 0x6F3341,
		ReturnHandled = 0x6F3406 ,
		CheckOccupy = 0x6F3379,
		SetWeaponSlot = 0x6F3360
	};

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(const AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTarget) {
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor() && pTarget->WhatAmI() == BulletClass::AbsID) {
			R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
			return ReturnHandled;
		}
	}

	if(pTypeExt->This()->TurretCount > 0 && !pTypeExt->This()->IsGattling) {
		if (pTypeExt->MultiWeapon && (pThis->WhatAmI() != AbstractType::Unit || !pTypeExt->This()->Gunner)) {
			return CheckOccupy;
		}

		return SetWeaponSlot;
	}

	return CheckOccupy;
}

ASMJIT_PATCH(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745, UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = cast_to<CellClass*>(pTarget))
	{
		auto const pWeaponPrimary = pThis->GetWeapon(0)->WeaponType;
		auto const pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;
		const auto pPrimaryExt = WeaponTypeExtContainer::Instance.Find(pWeaponPrimary);

		if (pWeaponSecondary && !pPrimaryExt->SkipWeaponPicking
			&& (!EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true, true)
			|| (pPrimaryExt->AttachEffect_CheckOnFirer && !pPrimaryExt->HasRequiredAttachedEffects(pThis, pThis)))
			&& (!GET_TECHNOTYPEEXT(pThis)->NoSecondaryWeaponFallback
			|| TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)))
		{
			R->EAX(1);
			return UseWeaponIndex;
		}
		else if (pCell->OverlayTypeIndex != -1)
		{
			auto const pOverlayType = OverlayTypeClass::Array()->Items[pCell->OverlayTypeIndex];

			if (pOverlayType->Wall && pCell->OverlayData >> 4 != pOverlayType->DamageLevels)
			{
				R->EAX(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayType));
				return UseWeaponIndex;
			}
		}
	}

	return 0;
}

//this hook disregard everything and return weapon index
ASMJIT_PATCH(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	GET(TechnoTypeClass*, pThisTechnoType, EAX);
	//GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pAbsTarget, STACK_OFFSET(0x18, 0x4));
	//GET(WeaponTypeClass* , pSecondary , EDI);
	//GET(WeaponTypeClass* , pSecondary , EBX);

	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pThisTechnoType);
	const int phobosWeapon = pTechnoTypeExt->SelectMultiWeapon(pThis, pAbsTarget);

	if (phobosWeapon >= 0) {
 		R->EAX(phobosWeapon);
 		return 0x6F37AF;
	}

	return 0;
}

#include <Ext/BulletType/Body.h>

ASMJIT_PATCH(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, 0x18 + 0x4);
	GET_STACK(WeaponTypeClass*, pSecondary, 0x10); //secondary
	GET_STACK(WeaponTypeClass*, pPrimary, 0x14); //primary

	enum
	{
		Primary = 0x6F37AD,
		Secondary = 0x6F3745,
		Secondary_b = 0x6F3807,
		FurtherCheck = 0x6F3754,
		OriginalCheck = 0x6F36E3
	};

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	bool allowFallback = !pTypeExt->NoSecondaryWeaponFallback;
	bool allowAAFallback = allowFallback ? true : pTypeExt->NoSecondaryWeaponFallback_AllowAA;
	const int weaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, allowFallback, allowAAFallback);

	if (weaponIndex != -1)
		return weaponIndex == 1 ? Secondary : Primary;

	if (!pTargetTechno || !pTargetTechno->IsAlive)
		return Primary;

	//select weapon is executed with dead target ?
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);

	//if (!pTargetExt) {
	//	Debug::LogInfo("Caller[%x] Techno[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]", calleraddr ,  pThis->get_ID(), pTargetTechno , pTargetTechno->align_154->OriginalHouseType->ID);
	//	calleraddr = -1;
	//	return OriginalCheck;
	//}

	if (const auto pShield = pTargetExt->GetShield())
	{
		if (pShield->IsActive())
		{
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pSecondary && pSecondary->Projectile->AA;

			if (pSecondary && (allowFallback || ((allowAAFallback && secondaryIsAA)
											|| (pTargetTechno->InWhichLayer() == Layer::Underground && BulletTypeExtContainer::Instance.Find(pSecondary->Projectile)->AU))

											|| TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)))
			{
				if (!pShield->CanBeTargeted(pPrimary))
					return Secondary;
				else
					return FurtherCheck;
			}

			return Primary;
		}
	}

	const int nArmor = (int)TechnoExtData::GetArmor(pTargetTechno);
	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::LogInfo(__FUNCTION__" Armor is more that avaible ArmorTypeClass ");

	const auto vsData_Secondary = &WarheadTypeExtContainer::Instance.Find(pSecondary->Warhead)->Verses[nArmor];

	if (vsData_Secondary->Verses == 0.0)
		return Primary;

	const auto vsData_Primary = &WarheadTypeExtContainer::Instance.Find(pPrimary->Warhead)->Verses[nArmor];

	return vsData_Primary->Verses != 0.0 ? FurtherCheck : Secondary;
}

ASMJIT_PATCH(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	//GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x18, 0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	if(pTargetTechno){

		const auto pPrimaryProj = pWeapon->Projectile;
		const auto pSecondaryProj = pSecWeapon->Projectile;

		if (!pPrimaryProj->AA && pSecondaryProj->AA) {
			if (pTargetTechno->IsInAir())
				return Secondary;
		}

		if (BulletTypeExtContainer::Instance.Find(pSecondaryProj)->AU && !BulletTypeExtContainer::Instance.Find(pPrimaryProj)->AU) {
			if (pTargetTechno->InWhichLayer() == Layer::Underground)
				return Secondary;
		}
	}

	return Primary;
}

ASMJIT_PATCH(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { ReturnValue = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	int oddWeaponIndex = 2 * pThis->CurrentGattlingStage;
	int evenWeaponIndex = oddWeaponIndex + 1;
	int eligibleWeaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true,true);

	if (eligibleWeaponIndex != -1)
	{
		R->EAX(eligibleWeaponIndex);
		return ReturnValue;
	}
	
	int chosenWeaponIndex = oddWeaponIndex;

	if (pTargetTechno)
	{
		auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
		auto const pWeaponEven = pThis->GetWeapon(evenWeaponIndex)->WeaponType;
		auto const pShield = pTargetExt->Shield.get();
		auto const armor = pTargetTechno->GetTechnoType()->Armor;
		const bool inAir = pTargetTechno->IsInAir();
		const bool isUnderground = pTargetTechno->InWhichLayer() == Layer::Underground;

		auto isWeaponValid = [&](WeaponTypeClass* pWeapon)
			{
				if (inAir && !pWeapon->Projectile->AA)
					return false;
				if (isUnderground && !BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AU)
					return false;
				if (pShield && pShield->IsActive() && !pShield->CanBeTargeted(pWeapon))
					return false;
				if (GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, armor) == 0.0)
					return false;
				return true;
			};

		// check even weapon first

		if (!isWeaponValid(pWeaponEven))
		{
			R->EAX(chosenWeaponIndex);
			return ReturnValue;
		}

		// handle naval targeting

		if (!pTargetTechno->OnBridge && !inAir)
		{
			auto const landType = pTargetTechno->GetCell()->LandType;

			if (landType == LandType::Water || landType == LandType::Beach)
			{
				if (pThis->SelectNavalTargeting(pTargetTechno) == NavalTargetingType::Underwater_secondary)
					chosenWeaponIndex = evenWeaponIndex;

				R->EAX(chosenWeaponIndex);
				return ReturnValue;
			}
		}

		// check odd weapon

		auto const pWeaponOdd = pThis->GetWeapon(oddWeaponIndex)->WeaponType;

		if (!isWeaponValid(pWeaponOdd) || pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_secondary)
			chosenWeaponIndex = evenWeaponIndex;
	}

	R->EAX(chosenWeaponIndex);
	return ReturnValue;
}

ASMJIT_PATCH(0x6F357F, TechnoClass_WhatWeaponShouldIUse_DrainWeaponTarget, 0x6)
{
	enum { CheckAlly = 0x6F3589, ContinueCheck = 0x6F35A8, RetPrimary = 0x6F37AD };

	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EBP);

	const bool IsTargetEligible = !pThis->DrainTarget && !pTarget->DrainingMe;
	return IsTargetEligible ?
		CheckAlly : ContinueCheck;
}

ASMJIT_PATCH(0x6F3410, TechnoClass_WhatWeaponShouldIUse_NoAmmoWeapon, 5)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = GET_TECHNOTYPE(pThis);

	if (pType->Ammo < 0)
		return 0x0;

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->NoAmmoWeapon < 0 || pThis->Ammo > pExt->NoAmmoAmount)
		return 0x0;

	R->EAX(pExt->NoAmmoWeapon);
	return 0x6F3406;
}

ASMJIT_PATCH(0x6F348F, TechnoClass_WhatWeaponShouldIUse_Airstrike, 0x7)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET(TechnoClass*, pTargetTechno, EBP);

	if (!pTargetTechno)
		return Primary;

	GET(WarheadTypeClass*, pSecondaryWH, ECX);
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pSecondaryWH);

	if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets, false))
		return Primary;

	const auto pTargetType = GET_TECHNOTYPE(pTargetTechno);

	if (pTargetTechno->AbstractFlags & AbstractFlags::Foot)
	{
		const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
		return pTargetTypeExt->AllowAirstrike.Get(true) ? Secondary : Primary;
	}

	const auto pTargetTypeExt = TechnoTypeExtContainer::Instance.Find(pTargetType);
	return pTargetTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTargetType)->CanC4) && (!pTargetType->ResourceDestination || !pTargetType->ResourceGatherer) ? Secondary : Primary;
}
#endif

#pragma region unused

// ASMJIT_PATCH(0x6F34B7, TechnoClass_WhatWeaponShouldIUse_AllowAirstrike, 0x6)
// {
// 	enum { SkipGameCode = 0x6F34BD };
// 	GET(BuildingTypeClass*, pThis, ECX);
//
// 	if (pThis)
// 	{
// 		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
// 		return SkipGameCode;
// 	}

// 	return 0x0;
// }
//
// ASMJIT_PATCH(0x51EAF2, TechnoClass_WhatAction_AllowAirstrike, 0x6)
// {
// 	enum { SkipGameCode = 0x51EAF8 };
// 	GET(BuildingTypeClass*, pThis, ESI);
//
// 	if (pThis)
// 	{
// 		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
// 		return SkipGameCode;
// 	}
//
// 	return 0x0;
// }

// ASMJIT_PATCH(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(WeaponTypeClass* const, pWeapon, EBX);
// 	GET_BASE(int, weaponIndex, 0xC);
//
// 	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeapon->IsLaser)
// 	{
// 		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
//
// 		if (pExt->CurrentLaserWeaponIndex.empty())
// 			pExt->CurrentLaserWeaponIndex = weaponIndex;
// 		else
// 			pExt->CurrentLaserWeaponIndex.clear();
// 	}
//
// 	return 0;
// }
//broke spawner building ?
//ASMJIT_PATCH(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8) //7
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(TechnoClass*, pTargetTechno, EBP);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));
//
//	enum { Primary = 0x6F37AD,
//		Secondary = 0x6F3745,
//		FurtherCheck = 0x6F3754,
//		OriginalCheck = 0x6F36E3
//	};
//
//	if (const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())) {
//		int weaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, !pTypeExt->NoSecondaryWeaponFallback);
//		return (weaponIndex != -1) && weaponIndex == 1 ? Secondary : Primary;
//	}
//
//	return OriginalCheck;
//}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
//int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
//{
//	CellClass* pTargetCell = nullptr;
//
//	// Ignore target cell for airborne target technos.
//	if (!pTargetTechno || !pTargetTechno->IsInAir())
//	{
//		if (auto const pCell = cast_to<CellClass*>(pTarget))
//			pTargetCell = pCell;
//		else if (auto const pObject = flag_cast_to<ObjectClass*>(pTarget))
//			pTargetCell = pObject->GetCell();
//	}
//
//	auto const pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
//	auto const pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);
//
//	if (!pWeaponStructOne && !pWeaponStructTwo)
//		return -1;
//	else if (!pWeaponStructTwo)
//		return weaponIndexOne;
//	else if (!pWeaponStructOne)
//		return weaponIndexTwo;
//
//	auto const pWeaponOne = pWeaponStructOne->WeaponType;
//	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;
//
//	if (auto const pSecondExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponTwo))
//	{
//		if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true , true)) ||
//			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
//				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
//		{
//			return weaponIndexOne;
//		}
//		else if (auto const pFirstExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponOne))
//		{
//			bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
//
//			if (!allowFallback && (!allowAAFallback || !secondaryIsAA) && !TechnoExtData::CanFireNoAmmoWeapon(pThis, 1))
//				return weaponIndexOne;
//
//			if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true , true)) ||
//				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
//					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
//			{
//				return weaponIndexTwo;
//			}
//		}
//	}
//
//	auto const pType = pThis->GetTechnoType();
//
//	// Handle special case with NavalTargeting / LandTargeting.
//	if (!pTargetTechno && pTargetCell && (pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary)
//		&& pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
//	{
//		return weaponIndexTwo;
//	}
//
//	return -1;
//}
//
//#include <Ares_TechnoExt.h>

//#pragma optimize("", off )
//static uintptr_t calleraddr = -1;
//ASMJIT_PATCH(0x5218E0, InfantryClass_SelectWeapon_IsTargetTechnoAlive, 0x9)
//{
//	GET(InfantryClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	if(pTarget && Is_Techno(pTarget) && !static_cast<TechnoClass*>(pTarget)->IsAlive)
//		Debug::LogInfo("Caller[%x] InfantryClass_SelectWeapon[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]", callerAddress, pThis->get_ID(), static_cast<TechnoClass*>(pTarget), static_cast<TechnoClass*>(pTarget)->align_154->OriginalHouseType->ID);
//
//	return 0x0;
//}
//

//ASMJIT_PATCH(0x6F3330, TechnoClass_WhatWeaponShouldIUse_IsTechnoTargetAlive, 5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
//		if(!pObj->IsAlive) {
//			Debug::LogInfo("[{}] {} {} Attempt to target death Object of {}!"
//				, callerAddress ,(void*)pThis , pThis->get_ID() , (void*)pTarget);
//
//			R->Stack(0x4 , nullptr);
//		}
//	}
//
//	return 0x0;
//}
//#pragma optimize("", on )
#pragma endregion