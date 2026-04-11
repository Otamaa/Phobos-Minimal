#include "Body.h"

#include <Ext/Bullet/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/BulletType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

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
			auto const pShield = pTargetExt->GetShield();
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
