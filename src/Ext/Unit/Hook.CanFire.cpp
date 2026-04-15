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
//
// ============================================================================
// STRUCTURE
// ============================================================================
// The original's goto chain is replaced with helper functions that each
// return either a FireError to propagate or a kContinue sentinel to signal
// "run the next helper." The orchestrator is a flat sequence of calls.
//
//   1. CheckPreflightLocks      — passenger/spawn/deploy + Techno::Can_Fire
//   2. CheckBuildableAndRadio   — DeployToFire hook + buildable cell check
//   3. CheckDockedToBuilding    — tether check + Tethered hook
//   4. GetCurrentWeapon         — Get_Weapon or Sync hook (DISABLED)
//   5. CheckWeaponGates         — weapon flags / reload / Temporal hook
//   6. CheckFacing              — angular target check
//   7. CheckAmmoFinal           — AmmoClass COM delegation
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

	// Sentinel used by helpers to signal "continue to next phase"
	constexpr FireError kContinue = FireError::NONE;

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
												  FireError& out_cached_error)
	{
		{
			if (pThis->DeathFrameCounter >= 0)
				return FireError::CANT;
		}

		// ORIG 0x740FF2 — SpawnManager "no spawns available"
		if (pThis->SpawnManager && pThis->SpawnManager->CountLaunchingSpawns() == 0)
			return FireError::BUSY;

		{
			if (pThis->DirectRockerLinkedUnit)
				return FireError::ILLEGAL;
		}

		// ORIG 0x74102D — Delegate to TechnoClass::Can_Fire
		out_cached_error = FakeTechnoClass::__CanFireMod(pThis, target, which, false, false);

		return kContinue;
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

		auto pLink = pThis->GetRadioContact(0);

		// Hook 0x7410D6 — null guard
		if (!pLink)
			return DockedCheckResult::Continue;

		if (pLink->WhatAmI() == AbstractType::Building)
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
		return pThis->_GetExtData()->CanFireWeaponType;
#if 0
		// -----------------------------------------------------------------------
		// VANILLA FALLBACK — use Get_Weapon(which)
		// -----------------------------------------------------------------------
		auto pWeaponStruct = pThis->GetWeapon(which);
		if (!pWeaponStruct || !pWeaponStruct->WeaponType)
			return nullptr;
		return pWeaponStruct->WeaponType;
#endif
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
		auto pTypeData = pThis->Type;

		// ORIG 0x7410F9 — Combat_Damage(-1) + low-health retreat check
		{
			const int damage = pThis->CombatDamage(-1);

			if (damage < 0) {
				if (auto pTargetTechno = flag_cast_to<TechnoClass*>(target)) {
					if (pTargetTechno->IsStrange())
					{
						const double ratio = pThis->GetHealthPercentage();
						if (ratio < RulesClass::Instance->ConditionYellow)
							return FireError::ILLEGAL;
					}
				}
			}
		}

		// ORIG 0x741149 — UnitType +0x6AE + reload state
		{
			if (!pTypeData->MobileFire && pThis->Destination)
				return FireError::MOVING;
		}

		// ORIG 0x741172 — Weapon +0x141 bypass flag (THE CRASH SITE)
		// pWeapon is guaranteed non-null by the orchestrator

		if (!pWeapon->FireWhileMoving)
		{
			auto pTypeData = *reinterpret_cast<unsigned char**>(
				reinterpret_cast<char*>(pThis) + 0x6C4);
			const bool type_flag_d6a = pTypeData[0xD6A] != 0;

			if (!type_flag_d6a)
			{
				const int reload_state_2 = *reinterpret_cast<int*>(
					reinterpret_cast<char*>(pThis) + 0x5A4);
				if (reload_state_2 != 0)
					return FireError::RELOADING;
			}
			else
			{
				// ORIG 0x7411A3 — AmmoClass COM check
				auto pAmmoObj = *reinterpret_cast<void**>(
					reinterpret_cast<char*>(pThis) + 0x674);
				if (!pAmmoObj)
					return FireError::RELOADING;

				using AmmoCheckFn = bool(__thiscall*)(void*);
				const auto ammo_vt = *reinterpret_cast<AmmoCheckFn**>(pAmmoObj);
				const auto ammo_check = reinterpret_cast<AmmoCheckFn>(
					ammo_vt[0x80 / 4]);
				if (ammo_check(pAmmoObj))
					return FireError::RELOADING;
			}
		}

		// ORIG 0x7411D9 — Weapon +0x12A / +0x129 + reload state
		{
			const bool weapon_flag_12a = *(
				reinterpret_cast<unsigned char*>(pWeapon) + 0x12A) != 0;
			const bool weapon_flag_129 = *(
				reinterpret_cast<unsigned char*>(pWeapon) + 0x129) != 0;

			if (weapon_flag_12a || weapon_flag_129)
			{
				const int reload_state_3 = *reinterpret_cast<int*>(
					reinterpret_cast<char*>(pThis) + 0x5A4);
				if (reload_state_3 != 0)
					return FireError::RELOADING;
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
			auto pType = pThis->Type;
			const bool has_turret_and_not_gattling =
				pType->TurretCount && !pType->IsGattling;

			if (has_turret_and_not_gattling)
			{
				const auto pSelectedWeaponStruct = pThis->GetWeapon(
					pThis->SelectWeapon(nullptr));

				const bool is_temporal =
					pSelectedWeaponStruct
					&& pSelectedWeaponStruct->WeaponType
					&& pSelectedWeaponStruct->WeaponType->Warhead
					&& pSelectedWeaponStruct->WeaponType->Warhead->Temporal;

				if (is_temporal)
				{
					// ORIG 0x741210 — the +0x274 + +0x5A4 reload check
					const int state_274 = *reinterpret_cast<int*>(
						reinterpret_cast<char*>(pThis) + 0x274);
					const int reload_state_4 = *reinterpret_cast<int*>(
						reinterpret_cast<char*>(pThis) + 0x5A4);

					if (state_274 != 0 && reload_state_4 != 0)
						return FireError::RELOADING;
				}
			}
		}

		// ORIG 0x741229 — Non-homing weapon lockout
		// (jumpjet in air + non-homing weapon → can't fire)
		{
			const bool flag_68d = *(
				reinterpret_cast<unsigned char*>(pThis) + 0x68D) != 0;
			if (!flag_68d)
			{
				const bool flag_6af = *(
					reinterpret_cast<unsigned char*>(pThis) + 0x6AF) != 0;
				if (flag_6af)
				{
					auto pProjectile = *reinterpret_cast<BulletTypeClass**>(
						reinterpret_cast<char*>(pWeapon) + 0xA0);
					if (pProjectile)
					{
						const int rot = *reinterpret_cast<int*>(
							reinterpret_cast<char*>(pProjectile) + 0x2DC);
						if (rot == 0)
							return FireError::CANT;
					}
				}
			}
		}

		return kContinue;
	}

	// ===========================================================================
	// CheckFacing
	//
	// ORIG 0x74125C
	// Angular diff between unit facing and target direction; if within threshold
	// return ROTATING.
	// ===========================================================================
	OPTIONALINLINE FireError CheckFacing(FakeUnitClass* pThis,
										   AbstractClass* target,
										   WeaponTypeClass* pWeapon)
	{
		const bool weapon_12b = *(
			reinterpret_cast<unsigned char*>(pWeapon) + 0x12B) != 0;

		auto pTypeData = *reinterpret_cast<unsigned char**>(
			reinterpret_cast<char*>(pThis) + 0x6C4);
		const bool type_e19 = pTypeData[0xE19] != 0;
		const bool type_e18 = pTypeData[0xE18] != 0;

		if (weapon_12b || type_e19 || type_e18)
			return kContinue;

		const bool use_turret_facing = pTypeData[0xCA1] != 0;
		const int facing_offset = use_turret_facing ? 0x3A0 : 0x388;

		auto pFacing = reinterpret_cast<FacingClass*>(
			reinterpret_cast<char*>(pThis) + facing_offset);
		DirStruct current_facing = pFacing->Current();

		auto pProjectile = *reinterpret_cast<BulletTypeClass**>(
			reinterpret_cast<char*>(pWeapon) + 0xA0);
		if (!pProjectile)
			return kContinue;

		const int rot = *reinterpret_cast<int*>(
			reinterpret_cast<char*>(pProjectile) + 0x2DC);
		const int threshold = 8 + (rot != 0 ? 8 : 0);

		DirStruct target_dir = pThis->Direction(target);

		const short raw_diff = static_cast<short>(current_facing.Raw - target_dir.Raw);
		const int abs_diff = raw_diff < 0 ? -raw_diff : raw_diff;

		const int threshold_scaled = threshold << 8;

		if (abs_diff < threshold_scaled)
			return FireError::ROTATING;

		return kContinue;
	}

	// ===========================================================================
	// CheckAmmoFinal
	//
	// ORIG 0x741300 — Final AmmoClass COM delegation.
	// ===========================================================================
	OPTIONALINLINE FireError CheckAmmoFinal(FakeUnitClass* pThis)
	{
		auto pAmmoObj = *reinterpret_cast<void**>(
			reinterpret_cast<char*>(pThis) + 0x674);
		if (!pAmmoObj)
			return FireError::ILLEGAL;

		using AmmoFinalFn = int(__thiscall*)(void*, void*);
		const auto ammo_vt = *reinterpret_cast<AmmoFinalFn**>(pAmmoObj);
		const auto ammo_final = reinterpret_cast<AmmoFinalFn>(ammo_vt[0x8C / 4]);

		const int ammo_result = ammo_final(pAmmoObj, pAmmoObj);
		if (ammo_result != 0)
			return static_cast<FireError>(ammo_result);

		return kContinue;
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
		const auto preflight = CheckPreflightLocks(this, target, which, cached_error);
		if (preflight != kContinue)
			return preflight;
	}

	// ORIG 0x741049 — if TechnoClass::Can_Fire returned non-OK non-ROTATING,
	// skip the buildable check and go to the cached-error tail.
	const bool techno_error_skips_buildable =
		(cached_error != FireError::OK && cached_error != FireError::ROTATING);

	bool skip_weapon_gates = techno_error_skips_buildable;

	// ----- Phase 5: buildable cell + DeployToFire hook --------------------
	if (!techno_error_skips_buildable)
	{
		switch (CheckBuildableAndRadio(this))
		{
		case BuildableCheckResult::ReturnBuildingForbidden:
			return FireError::BUILDING_FORBIDDEN;

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
		if (gates != kContinue)
			return gates;
	}

	// ----- Phase 9: facing check ------------------------------------------
	{
		const auto facing = CheckFacing(this, target, pWeapon);
		if (facing != kContinue)
			return facing;
	}

	// ----- Phase 10: final AmmoClass delegation ---------------------------
	{
		const auto ammo = CheckAmmoFinal(this);
		if (ammo != kContinue)
			return ammo;
	}

	// ----- Tail: return cached error --------------------------------------
	return cached_error;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6030, FakeUnitClass::_Can_Fire);
DEFINE_FUNCTION_JUMP(LJMP, 0x740FD0, FakeUnitClass::_Can_Fire);
// ============================================================================
// DEBUGGING NOTES
// ============================================================================
//
// The original crash at 0x741172 was reading [ebx+141h] where ebx was null.
// In this backport the read is inside CheckWeaponGates, with pWeapon
// null-checked in the orchestrator's Phase 7.
//
// Hook 0x7410EC (UnitClass_CanFire_Sync) is DISABLED in GetCurrentWeapon.
// Testing path: vanilla Get_Weapon(which) + null-guard fallback. Re-enable
// by flipping the `#if 0` to `#if 1` when done debugging.
//
// If the null-guard in Phase 7 fires:
//   - set a breakpoint on the `return FireError::ILLEGAL` line
//   - inspect `which`, `this->Primary`, `this->Secondary`
//   - walk the call stack to find who passed the bad `which`
//
// Most likely suspects for bad `which`:
//   1. What_Weapon_Should_I_Use backport returning an invalid slot
//   2. SelectWeapon() returning a slot the unit doesn't have
//   3. A caller passing `which=1` to a unit with no Secondary weapon
// ============================================================================