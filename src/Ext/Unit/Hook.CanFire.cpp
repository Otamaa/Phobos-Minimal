#include "Body.h"

// ============================================================================
// FakeUnitClass::_Can_Fire — backport of UnitClass::Can_Fire (0x740FD0)
// with all existing Phobos hooks merged in.
//
// ============================================================================
// HOOK INTEGRATION TABLE
// ============================================================================
//   Address  Sz   Hook name                                  Helper
//   -------  --   ---------------------------------------    --------------------
//   0x741050  6   UnitClass_CanFire_DeployToFire             CheckBuildableAndRadio
//   0x7410D6  7   UnitClass_CanFire_Tethered                 CheckDockedToBuilding
//   0x7410EC  5   UnitClass_CanFire_Sync                     GetCurrentWeapon (DISABLED)
//   0x741206  6   UnitClass_CanFire                          CheckWeaponGates (Temporal)
//   0x741288  6   UnitClass_CanFire_DeployFire_DoNotErrorFacing  CheckFacing (absorbed, dead — LJMP 0x740FD0 makes this unreachable)
//
// ============================================================================
// STRUCTURE
// ============================================================================
// The original's goto chain is replaced with helper functions that each
// return either a `FireError` to propagate or `FireError::NONE` to signal
// "run the next helper." The orchestrator is a flat sequence of calls.
//
//   1. CheckPreflightLocks      — passenger/spawn/deploy + Techno::Can_Fire
//   2. CheckBuildableAndRadio   — DeployToFire hook + buildable cell check
//   3. CheckDockedToBuilding    — tether check + Tethered hook
//   4. GetCurrentWeapon         — Get_Weapon or Sync hook (DISABLED)
//   5. CheckWeaponGates         — weapon flags / reload / Temporal hook
//   6. CheckFacing              — angular target check
//   7. CheckLocomotorCanFire   — Locomotor COM delegation
// ============================================================================

#include <UnitClass.h>
#include <TechnoClass.h>
#include <WeaponTypeClass.h>
#include <BulletTypeClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <RulesClass.h>
#include <SpawnManagerClass.h>
#include <HouseClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

namespace
{
	// ===========================================================================
	// CheckPreflightLocks
	//
	// ORIG 0x740FD9..0x74104E
	// Passenger lockout, spawn manager check, deploy-state lockout, and the
	// initial TechnoClass::Can_Fire call. The cached error is written to the
	// out parameter so the orchestrator can reuse it for the tail return.
	// ===========================================================================
	OPTIONALINLINE FireError CheckPreflightLocks(FakeUnitClass* pThis,
													  AbstractClass* target,
													  int which,
													  bool check_fire_coord,
													  FireError& outCachedError)
	{
		{
			if (pThis->DeathFrameCounter >= 0)
				return FireError::CANT;
		}

		// ORIG 0x740FF2 — SpawnManager "no spawns available"
		if (pThis->SpawnManager && pThis->SpawnManager->CountLaunchingSpawns() != 0)
			return FireError::BUSY;

		{
			if (pThis->DirectRockerLinkedUnit)
				return FireError::ILLEGAL;
		}

		// ORIG 0x74102D — Delegate to TechnoClass::Can_Fire
		outCachedError = FakeTechnoClass::__CanFireMod(pThis, target, which, check_fire_coord, false);

		//0x74132B, UnitClass_CanFire_DisallowMoving, 0x7
		if(outCachedError == FireError::RANGE && TechnoExtData::CannotMove(pThis))
			return FireError::ILLEGAL;

		return FireError::NONE;
	}

	// ===========================================================================
	// CheckBuildableAndRadio
	//
	// ORIG 0x741050 + HOOK 0x741050 (UnitClass_CanFire_DeployToFire)
	//
	// The hook completely replaces the vanilla buildable-cell + radio-contact
	// check. Returns one of three outcomes which the orchestrator translates
	// into either an early return or a skip-to-tail.
	// ===========================================================================
	enum class BuildableCheckResult
	{
		Continue,                   // run next helper
		ReturnBuildingForbidden,    // return BUILDING_FORBIDDEN
		SkipToTail,                 // skip weapon gates, return cached_error
	};

	OPTIONALINLINE BuildableCheckResult CheckBuildableAndRadio(FakeUnitClass* pThis)
	{
		// -----------------------------------------------------------------------
		// Hook 0x741050 — UnitClass_CanFire_DeployToFire
		// -----------------------------------------------------------------------
		if (pThis->Type->DeployToFire
			&& pThis->CanDeployNow()
			&& !TechnoExtData::CanDeployIntoBuilding(pThis, true))
		{
			return BuildableCheckResult::ReturnBuildingForbidden;
		}

		auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

		if (!pTypeExt->NoTurret_TrackTarget.Get(
			RulesExtData::Instance()->NoTurret_TrackTarget))
		{
			// Hook's NoNeedToCheck path — jump directly to cached-error tail
			return BuildableCheckResult::SkipToTail;
		}

		// Hook's SkipGameCode path — fall through to next helper
		return BuildableCheckResult::Continue;
	}

	// ===========================================================================
	// CheckDockedToBuilding
	//
	// ORIG 0x7410C3..0x7410E9 + HOOK 0x7410D6 (UnitClass_CanFire_Tethered)
	//
	// If the unit is tethered (+0x418) AND in radio contact with a building,
	// approve the fire immediately. The hook adds a null-check on the radio
	// link that vanilla omitted.
	// ===========================================================================
	enum class DockedCheckResult
	{
		Continue,
		ReturnOK,
	};

	OPTIONALINLINE DockedCheckResult CheckDockedToBuilding(FakeUnitClass* pThis)
	{
		// VERIFY: +0x418 — IsTethered or InTransit
		if (!pThis->IsTethered)
			return DockedCheckResult::Continue;

		auto pRadioLink = pThis->GetRadioContact(0);

		// Hook 0x7410D6 — null guard
		if (!pRadioLink)
			return DockedCheckResult::Continue;

		if (pRadioLink->WhatAmI() == AbstractType::Building)
			return DockedCheckResult::ReturnOK;

		return DockedCheckResult::Continue;
	}

	// ===========================================================================
	// GetCurrentWeapon
	//
	// ORIG 0x7410EC..0x7410F7 + HOOK 0x7410EC (UnitClass_CanFire_Sync)
	//
	// The hook replaces the vanilla Get_Weapon vtable call with a cached pointer
	// on the unit's ext data. DISABLED FOR TESTING — re-enable the #if 1 block
	// when the Can_Fire crash is diagnosed.
	// ===========================================================================
		OPTIONALINLINE WeaponTypeClass* GetCurrentWeapon(FakeUnitClass* pThis, int which)
	{
		// DISABLED FOR TESTING — Hook 0x7410EC (UnitClass_CanFire_Sync)
		// -----------------------------------------------------------------------
		// Hook 0x7410EC — UnitClass_CanFire_Sync
		// -----------------------------------------------------------------------
		// Replaces the vanilla Get_Weapon vtable call with a direct read from
		// the unit's ext data. Bypasses weapon-slot lookup entirely.
		//
		// If CanFireWeaponType is null, this will null-propagate into the
		// downstream weapon flag reads. The hook is presumably set only after
		// something else has cached the right weapon. Disabling it forces the
		// vanilla Get_Weapon path, which lets us isolate whether the hook or
		// the vanilla logic is responsible for the null-weapon crash.
		// -----------------------------------------------------------------------
		if (auto pWeapon = pThis->_GetExtData()->CanFireWeaponType)
			return pWeapon;

		// -----------------------------------------------------------------------
		// VANILLA FALLBACK — use Get_Weapon(which)
		// -----------------------------------------------------------------------
		auto pWeaponSlot = pThis->GetWeapon(which);

		if (!pWeaponSlot || !pWeaponSlot->WeaponType)
			return nullptr;

		return pWeaponSlot->WeaponType;
	}

	// ===========================================================================
	// CheckWeaponGates
	//
	// ORIG 0x7410F9..0x741203 + HOOK 0x741206 (UnitClass_CanFire)
	//
	// Chain of weapon-flag / reload-state checks. The original crash was at
	// +0x141 read; the orchestrator null-checks pWeapon before calling in.
	// ===========================================================================
		OPTIONALINLINE FireError CheckWeaponGates(FakeUnitClass* pThis,
													AbstractClass* target,
													WeaponTypeClass* pWeapon)
	{
			auto pUnitType = pThis->Type;

		// ORIG 0x7410F9 — Combat_Damage(-1) + low-health retreat check
		{
			if (TechnoExtData::IsHealer(pThis))
			{
				if (auto pTargetTechno = flag_cast_to<TechnoClass*>(target))
				{
					//0x741113, UnitClass_CanFire_Heal, 0xA
					if(pTargetTechno->IsIronCurtained()
						|| !TechnoExtData::FiringAllowed(pThis, pTargetTechno, pWeapon, true))
						return FireError::ILLEGAL;

					if (pTargetTechno->IsStrange())
					{
						const double ratio = pThis->GetHealthRatio();
						if (ratio < RulesClass::Instance->ConditionYellow)
							return FireError::ILLEGAL;
					}
				}
			}
		}

		// ORIG 0x741149 — UnitType +0x6AE + reload state
		{
			if (!pUnitType->MobileFire && pThis->Destination)
				return FireError::MOVING;
		}

		// ORIG 0x741172 — Weapon +0x141 bypass flag (THE CRASH SITE)
		// pWeapon is guaranteed non-null by the orchestrator

		if (!pWeapon->FireWhileMoving)
		{
			if (!pThis->Type->BalloonHover)
			{
				if (pThis->NavCom != 0)
					return FireError::MOVING;
			}
			else
			{
				// ORIG 0x7411A3 — Locomotor COM check; null ammo → skip (jz 0x7411D9)
				if (auto pLoco = pThis->Locomotor) {
					if (pLoco->Is_Moving_Now())
						return FireError::MOVING;
				}
			}
		}

		// ORIG 0x7411D9 — Weapon +0x12A / +0x129 + reload state
		{
			const bool useSparkParticles = pWeapon->UseSparkParticles != 0;
			const bool useFireParticles = pWeapon->UseFireParticles != 0;

			if (useSparkParticles || useFireParticles) {
				if (pThis->NavCom != 0)
					return FireError::MOVING;
			}
		}

		// -----------------------------------------------------------------------
		// Hook 0x741206 — UnitClass_CanFire (Turret/Gattling/Temporal)
		// -----------------------------------------------------------------------
		// Replaces vanilla +0x274 check with:
		//   if (!TurretCount || IsGattling)       → skip
		//   else if (selected weapon is Temporal) → run the +0x274+0x5A4 reload check
		//   else                                  → skip
		// -----------------------------------------------------------------------
		{
			auto pUnitType2 = pThis->Type;
			const bool has_turret_and_not_gattling =
				pUnitType2->TurretCount && !pUnitType2->IsGattling;

			if (has_turret_and_not_gattling)
			{
				const auto pSelectedWeaponSlot = pThis->GetWeapon(
					pThis->SelectWeapon(nullptr));

				const bool is_temporal =
					pSelectedWeaponSlot
					&& pSelectedWeaponSlot->WeaponType
					&& pSelectedWeaponSlot->WeaponType->Warhead
					&& pSelectedWeaponSlot->WeaponType->Warhead->Temporal;

				if (is_temporal)
				{
					// ORIG 0x741210 — the +0x274 + +0x5A4 reload check
					const auto temporal_state = pThis->TemporalImUsing;
					const auto navcom_state = pThis->NavCom;

					if (temporal_state != 0 && navcom_state != 0)
						return FireError::MOVING;
				}
			}
		}

		// ORIG 0x741229 — Non-homing weapon lockout
		// (jumpjet in air + non-homing weapon → can't fire)
		{
			const bool isFiring = pThis->IsFiring != 0;
			if (!isFiring) {
				if (pThis->IsRotating != 0)
				{
					if (auto pProjectile = pWeapon->Projectile)
					{
						if (pProjectile->ROT == 0)
							return FireError::ROTATING;
					}
				}
			}
		}

		return FireError::NONE;
	}

	// ===========================================================================
	// CheckFacing
	//
	// ORIG 0x74125C
	// Angular diff between unit facing and target direction.
	// abs_diff < threshold_scaled → unit is well-aimed → return `FireError::NONE`.
	// DeployFire units bypass the check entirely (return `FireError::NONE`).
	// Otherwise → FACING.
	// (ASMJIT_PATCH 0x741288 logic absorbed here; dead due to LJMP at 0x740FD0.)
	// ===========================================================================
	OPTIONALINLINE FireError CheckFacing(FakeUnitClass* pThis,
										   AbstractClass* target,
										   WeaponTypeClass* pWeapon)
	{
		const bool isOmniFire =
			pWeapon->OmniFire != 0;

		auto pTypeData = pThis->Type;
		const bool IsLargeVisceroid = pTypeData->LargeVisceroid != 0;
		const bool IsSmallVisceroid = pTypeData->SmallVisceroid != 0;

		if (isOmniFire || IsLargeVisceroid || IsSmallVisceroid)
			return FireError::NONE;

		// Prefer the instance-level turret state when available; some units
		// can have per-instance turretness that differs from the type flag.
		const bool use_turret_facing = pThis->HasTurret();
		DirStruct current_facing = (use_turret_facing ? &pThis->SecondaryFacing : &pThis->PrimaryFacing)->Current();

		auto pProjectile = pWeapon->Projectile;

		if (!pProjectile)	
			return FireError::NONE;

		const int rot = pProjectile->ROT;
		const int threshold = 8 + (rot != 0 ? 8 : 0);

		DirStruct target_dir = pThis->GetDirectionOverObject(target);

		// Compute the wrapped angular delta using DirStruct subtraction so
		// callers don't deal with Raw arithmetic directly. Widen to `int`
		// after casting to `short` to preserve the game's signed-16
		// subtraction semantics while avoiding UB.
		const DirStruct delta = current_facing - target_dir;
		const int raw_diff = static_cast<int>(static_cast<short>(delta.Raw));
		const int abs_diff = raw_diff < 0 ? -raw_diff : raw_diff;

		const int threshold_scaled = threshold << 8;

		// ORIG 0x741288 — abs_diff < threshold: unit is aimed at target, can fire
		//
		// Fix: use inclusive compare `<=` to avoid off-by-one directions
		// being classified as `FACING`. The original game's `<` check is
		// preserved behind the `PHOBOS_USE_ORIGINAL_FACING` macro for
		// compatibility/testing.
	#ifndef PHOBOS_USE_ORIGINAL_FACING
		if (abs_diff <= threshold_scaled)
		    return FireError::NONE;
	#else
		if (abs_diff < threshold_scaled)
		    return FireError::NONE;
	#endif

		// ASMJIT_PATCH 0x741288 — DeployFire bypass (absorbed from dead hook)
		// dont return FACING if the unit can deploy to fire and isn't already deployed
		if (pThis->Type->DeployFire
			&& !pThis->Type->IsSimpleDeployer
			&& !pThis->Deployed
			&& pThis->CurrentMission == Mission::Unload)
		{
			return FireError::NONE;
		}

		return FireError::FACING;
	}

	// ===========================================================================
	// CheckLocomotorCanFire
	//
	// ORIG 0x741300 — Final Locomotor COM delegation.
	// ===========================================================================
	OPTIONALINLINE FireError CheckLocomotorCanFire(FakeUnitClass* pThis)
	{
		auto pLocomotor = pThis->Locomotor;
		if (!pLocomotor)
			return FireError::ILLEGAL;

		const FireError locoResult = pLocomotor->Can_Fire();
		if (locoResult != FireError::NONE)
			return locoResult;

		return FireError::NONE;
	}

}  // anonymous namespace

// ===========================================================================
// FakeUnitClass::_Can_Fire — orchestrator
// ===========================================================================
FireError FakeUnitClass::_Can_Fire(AbstractClass* target, int which, bool check_fire_coord)
{
	// ----- Phase 1..4: preflight + TechnoClass::Can_Fire ------------------
	FireError cached_error = FireError::OK;
	{
		const auto preflight = CheckPreflightLocks(this, target, which, check_fire_coord, cached_error);
		if (preflight != FireError::NONE)
			return preflight;
	}

	// ORIG 0x741049 — if TechnoClass::Can_Fire returned non-OK non-ROTATING,
	// skip the buildable check and go to the cached-error tail.
	const bool techno_error_skips_buildable =
		(cached_error != FireError::OK && cached_error != FireError::FACING);

	bool skip_weapon_gates = techno_error_skips_buildable;

	// ----- Phase 5: buildable cell + DeployToFire hook --------------------
	if (!techno_error_skips_buildable)
	{
		switch (CheckBuildableAndRadio(this))
		{
		case BuildableCheckResult::ReturnBuildingForbidden:
			return FireError::MUST_DEPLOY;

		case BuildableCheckResult::SkipToTail:
			skip_weapon_gates = true;
			break;

		case BuildableCheckResult::Continue:
			break;
		}
	}

	// ----- Cached-error tail (skip weapon gates if requested) -------------
	if (skip_weapon_gates)
	{
		// If cached error is set, return it. Otherwise we approved via the
		// hook's NoNeedToCheck path — return OK.
		return cached_error;
	}

	// ----- Phase 6: docked-to-building approval ---------------------------
	if (CheckDockedToBuilding(this) == DockedCheckResult::ReturnOK)
		return FireError::OK;

	// ----- Phase 7: resolve current weapon --------------------------------
	WeaponTypeClass* pWeapon = GetCurrentWeapon(this, which);

	// Null guard — the original crash at 0x741172 landed here because
	// vanilla never checked. Returning ILLEGAL lets you set a breakpoint
	// and inspect `which` / `pThis->Primary` / `pThis->Secondary`.
	if (!pWeapon)
	{
		// Debug::Log("Can_Fire: null weapon for slot %d\n", which);
		return FireError::ILLEGAL;
	}

	// ----- Phase 8: weapon-flag / reload-state gates ----------------------
	{
		const auto gates = CheckWeaponGates(this, target, pWeapon);
		if (gates != FireError::NONE)
			return gates;
	}

	// ----- Phase 9: facing check ------------------------------------------
	{
		const auto facing = CheckFacing(this, target, pWeapon);
		if (facing != FireError::NONE)
			return facing;
	}

	// ----- Phase 10: final Locomotor delegation ---------------------------
	{
		const auto locoCheck = CheckLocomotorCanFire(this);
		if (locoCheck != FireError::NONE)
			return locoCheck;
	}

	// ----- Tail: return cached error --------------------------------------
	return cached_error;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6030, FakeUnitClass::_Can_Fire);
DEFINE_FUNCTION_JUMP(LJMP, 0x740FD0, FakeUnitClass::_Can_Fire);