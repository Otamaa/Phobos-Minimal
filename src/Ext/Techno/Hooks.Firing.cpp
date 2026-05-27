#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/InfantryType/Body.h>

#include <Locomotor/Cast.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <TerrainClass.h>

#ifndef FireAt_Backport
// TechnoClass_FireAt.cpp
// Backport : TechnoClass::Fire_At  [0x6FDD50 – 0x6FF94A]
// Phobos   : all ASMJIT_PATCH / DEFINE_FUNCTION_JUMP hooks integrated inline.
//
// Fixes vs previous version:
//   - velocity is declared at function scope; passed to HandlePostBulletLogic
//     so LimboLaunch Unlimbo re-uses the same vector as the main shot (matches asm)
//   - all WeaponTypeExtContainer / TechnoTypeExt / BulletTypeExt lookups hoisted
//     to one call-site each; inner blocks use the cached pointers
//   - artificial delta {} scope removed; everything at function level
//
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ⚠ WARNING SUMMARY
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//  W1 [0x6FDDC0] TechnoClass_FireAt_Early skips vanilla §SUICIDE (dead code).
//  W2 [0x6FDFA8] TechnoClass_FireAt_SprayOffsets skips §SPRAY (dead code).
//  W3 [0x6FE31C] TechnoClass_FireAt_AllowDamage replaces vanilla IsSonic/UFP zero.
//  W4 [0x6FE337] TechnoClass_FireAt_DamageMult jumps past §VET_BONUS (dead code).
//  W5 [0x6FE3E3] TechnoClass_FireAt_OccupyDamageBonus replaces §DMG_MULTS + §DISK.
//  W6 [0x6FF0DD] TechnoClass_FireAt_TurretRecoil skips §RECOIL (dead code).
//  W7 [0x6FF15F] TechnoClass_FireAt_Additionals_Start skips §PARTICLES/ROF/ANIM/SOUND.
//  W8 [0x6FF48D] TechnoClass_FireAt_IsLaser:  true→skips all beam effects; false→continues.
//  W9 [0x6FF5F5] TechnoClass_FireAt_Additionals_End skips §MAGBEAM (dead code).
//  W11 POTENTIAL CRASH [0x6FDE05]: pWeapon may be null on null-weapon path; null-guarded.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

//extern "C" int     __cdecl   Direction_Legal(bool, int, int, int, double, DirStruct*);
//extern "C" double  __cdecl   combat_Get_Floater_Gravity();
//extern "C" void    __stdcall MapClass_visibility_567DA0(MapClass*, CoordStruct*, int, int, int);
//extern "C" CoordStruct* __fastcall TechnoClass_get_Coord_70BCB0(TechnoClass*, CoordStruct*);
//extern CoordStruct techno_defaulcoord; // VERIFY: 0xB0EA90
//static void __cdecl sub_6FF950() {}   // atexit no-op @ 0x6FF950

// ─── Shared state between hooks ───────────────────────────────────────────────
namespace FireAtTemp
{
	BulletClass* FireBullet = nullptr;
	CoordStruct    originalTargetCoords = {};
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
	AbstractClass* pWaveOwnerTarget = nullptr;
}
namespace UnlimboDetonateFireTemp
{
	BulletClass* Bullet = nullptr;
	bool         InSelected = false;
	bool         InLimbo = false;
}

// ─── Helper: KillSelf on every return  (hook 0x6FDE05 / 0x6FF933) ─────────────
static void RunEndHook(TechnoClass* pThis, WeaponTypeClass* pWeapon)
{
	if (!pWeapon) return; // W11: null on null-weapon early exit
	auto* pWExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	if (pWExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);
}

// ─── Helper: Made_A_Kill + visibility reveal  (0x6FF660 in binary) ────────────
static void HandleAfterEffects(
	TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	// Only perform vanilla ammo deduction if the weapon does not explicitly opt out.
	// Ammo=0 or Ammo<0 on the weapon type means no ammo should be consumed; the
	// GetAmmo loop in TechnoExtData::DecreaseAmmo already handles positive values.
	if (WeaponTypeExtContainer::Instance.Find(pWeapon)->Ammo > 0)
		pThis->DecreaseAmmo();
	pThis->Mark(MarkType::Change);

	CoordStruct center = pThis->GetCoords();

	const bool shooterVisible =
		(pThis->IsOwnedByCurrentPlayer || pThis->DiscoveredByCurrentPlayer)
		&& (MapClass::Instance->IsLocationShrouded(center) || MapClass::Instance->IsLocationFogged(center))
		&& (pThis->WhatAmI() != AbstractType::Aircraft || !pThis->IsOwnedByCurrentPlayer);

	auto pObjTarget = flag_cast_to<ObjectClass*, true>(pTarget);

	if (shooterVisible && pObjTarget)
	{
		auto* tgtHouse = pObjTarget->GetOwningHouse();

		if (tgtHouse->ControlledByCurrentPlayer() && pWeapon->RevealOnFire)
		{
			MapClass::Instance->RevealArea1(&center, 3, tgtHouse, 0, 0, 0, 1, 0);
			MapClass::Instance->RevealArea3(&center, 0, 4, 0);
		}
	}
	pThis->LastFireBulletFrame = Unsorted::CurrentFrame();

	// Reset temp state for this shot
	FireAtTemp::FireBullet = nullptr;
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;
}

// ─── Helper: LimboLaunch / DistributedFire / FireOnce  (post-bullet) ──────────
// velocity: the same vector computed for the main bullet (needed by LimboLaunch
//           parasite re-unlimbo to match original asm behaviour).
static BulletClass* HandlePostBulletLogic(
	TechnoClass* pThis,
	TechnoTypeClass* pType,
	WeaponTypeClass* pWeapon,
	WeaponTypeExtData* pWeaponExt,
	BulletClass* pBullet,
	AbstractClass* pTarget,
	const CoordStruct& a3,
	const CoordStruct& coord1,
	int                     finalDamage,
	int                     which,
	const VelocityClass& velocity)   // ← fixed: original a1 reused in LimboLaunch
{
	// ── §LIMBO_LAUNCH ─────────────────────────────────────────────────────────
	if (pWeapon->LimboLaunch && pBullet)
	{
		if (pType->ReselectIfLimboed && pThis->IsSelected
			&& pThis->WhatAmI() != AbstractType::Building) {
			auto pThisOwner = pThis->GetOwningHouse();

			if (pThisOwner->ControlledByCurrentPlayer()
			 && pTarget->WhatAmI() == AbstractType::Infantry)
				pThis->ShouldBeReselectOnUnlimbo = 1;
		}

		auto pThisFoot = flag_cast_to<FootClass*, false>(pThis);

		if (pType->RejoinTeamIfLimboed
		 && pThisFoot
		 && pThisFoot->Team && pTarget->WhatAmI() == AbstractType::Infantry)
			pThisFoot->OldTeam = pThisFoot->Team;

		pThis->Limbo();

		if (pWeapon->Warhead->Parasite)
		{
			if (auto pTargetFoot = flag_cast_to<FootClass*>(pTarget))
				pTargetFoot->LastBeParasitedStartFrame = Unsorted::CurrentFrame() + 20;

			pBullet->Limbo();

			// ╔═══ HOOK: TechnoClass_FireAt_UnlimboDetonate @ 0x6FF7FF ════════╗
			{
				auto* pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

				if (pThis->IsAlive && pThis->Health > 0 && pBullet
				 && !UnlimboDetonateFireTemp::InLimbo
				 && !pWeapon->Warhead->Parasite && pWHExt->UnlimboDetonate)
				{
					if (pWHExt->UnlimboDetonate_KeepSelected)
					{
						TechnoExtContainer::Instance.Find(pThis)->IsSelected
							= UnlimboDetonateFireTemp::InSelected;
						ScenarioExtData::Instance()->LimboLaunchers.emplace(pThis);
					}
					pBullet->Owner = pThis;
				}
			}
			// ╚═══════════════════════════════════════════════════════════════╝

			pBullet->Construct(pWeapon->Projectile, pTarget, pThis,
				finalDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);

			// Reuse the main-shot velocity – matches original asm (a1 reused)
			pBullet->MoveTo(a3, velocity); // hook 0x6FF860

			// ╔═══ HOOK: TechnoClass_FireAt_FSW @ 0x6FF860 ════════════════════╗
			if (HouseExtContainer::Instance.IsAnyFirestormActive
			 && !pBullet->Type->IgnoresFirestorm)
			{
				CoordStruct crdSrc = a3, crdTgt = coord1;
				auto crd = MapClass::Instance->FindFirstFirestorm(
					crdSrc, crdTgt, pBullet->Owner->Owner);

				if (crd.IsValid())
				{
					pBullet->Target = MapClass::Instance->GetCellAt(crd)->GetContent();
					pBullet->Owner->ShouldLoseTargetNow = 1;
				}
			}
			// ╚═══════════════════════════════════════════════════════════════╝
		}
	}

	// ── §DISTRIBUTED_FIRE ─────────────────────────────────────────────────────
	if (pType->DistributedFire
	 && (pThis->CurrentMission != Mission::Attack
		 || pThis->WhatAmI() == AbstractType::Building)) {
		pThis->AttackedTargets.emplace_back(pTarget);
		pThis->SetTarget(nullptr);
	}

	// ── §FIRE_ONCE ────────────────────────────────────────────────────────────
	if (pWeapon->FireOnce)
	{
		// ╔═══ HOOK: TechnoClass_FireAt_FireOnce_A @ 0x6FF905 ══════════════════╗
		if (auto* pInf = cast_to<InfantryClass*, false>(pThis))
		{
			if (!pWeaponExt->FireOnce_ResetSequence)
				InfantryExtContainer::Instance.Find(pInf)
				->SkipTargetChangeResetSequence = true;
		}
		// ╚═══════════════════════════════════════════════════════════════════╝

		// ╔═══ HOOK: TechnoClass_FireAt_FireOnce_B @ 0x6FF923 ══════════════════╗
		pThis->SetTarget(nullptr);
		if (auto* pUnit = cast_to<UnitClass*, false>(pThis))
		{
			if (pUnit->Type->DeployFire && !pUnit->Type->IsSimpleDeployer
			 && !pUnit->Deployed && pThis->CurrentMission == Mission::Unload)
				TechnoExtContainer::Instance.Find(pUnit)
				->DeployFireTimer.Start(pWeapon->ROF);
		}
		// ╚═══════════════════════════════════════════════════════════════════╝
	}

	// ╔═══ HOOK: TechnoClass_FireAt_End @ 0x6FF933 ════════════════════════════╗
	RunEndHook(pThis, pWeapon);
	// ╚═══════════════════════════════════════════════════════════════════════╝
	return pBullet;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
//  Main function
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
BulletClass* __fastcall FakeTechnoClass::__Fire_At(
	TechnoClass* pThis, discard_t,
	AbstractClass* pTarget, int which)
{
	auto* pExt = TechnoExtContainer::Instance.Find(pThis);

	// ╔═══ HOOK: TechnoClass_FireAt_PreFire @ 0x6FDD50 ════════════════════════╗
	//pExt->CurrentWeaponIdx = which;
	// ╚═══════════════════════════════════════════════════════════════════════╝

	// ╔═══ HOOK: TechnoClass_FireAt_GetWeapon DEFINE_FUNCTION_JUMP @ 0x6FDD69 ═╗
	// Redirected to always use CurrentWeaponIdx instead of raw `which`.
	// ╚═══════════════════════════════════════════════════════════════════════╝
	auto* pWeapon = pThis->GetWeapon(which)->WeaponType; // hook 0x6FDD69

	if (!pWeapon) { return nullptr; } // W11

	// ─── Hoist all extension lookups (one call each) ──────────────────────────
	auto* pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto* pBulletType = pWeapon->Projectile;
	auto* pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBulletType);
	auto* pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
	auto* pTechnoType = GET_TECHNOTYPE(pThis);
	auto* pTechnoTypeExt = GET_TECHNOTYPEEXT(pThis);

	auto pThisFoot = flag_cast_to<FootClass*, false>(pThis);
	auto* pTargetFoot = flag_cast_to<FootClass*>(pTarget);
	auto* pTargetTechno = flag_cast_to<TechnoClass*>(pTarget);

	const auto rtti = pThis->WhatAmI();

	// ╔═══ HOOK: TechnoClass_FireAt_UpdateWeaponType @ 0x6FDD7D ═══════════════╗
	{
		if (pWeapon->LimboLaunch && !pWeapon->Warhead->Parasite && pWHExt->UnlimboDetonate)
		{
			if (pThisFoot) {
				if (pThisFoot->Locomotor->Is_Really_Moving_Now())
				{ RunEndHook(pThis, pWeapon); return nullptr; }
			}
		}

		if (pThis->CurrentBurstIndex && pWeapon != pExt->LastWeaponType
		 && pTechnoTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst))
		{
			if (pExt->LastWeaponType && pExt->LastWeaponType->Burst)
			{
				const double ratio = (double)pThis->CurrentBurstIndex
					/ pExt->LastWeaponType->Burst;
				const int rof = (int)(ratio * pExt->LastWeaponType->ROF
								* pExt->AE.ROFMultiplier)
					- (Unsorted::CurrentFrame.get() - pThis->LastFireBulletFrame);
				if (rof > 0)
				{
					pThis->ROF = rof;
					pThis->RearmTimer.Start(rof);
					pThis->CurrentBurstIndex = 0;
					pExt->LastWeaponType = pWeapon;
					RunEndHook(pThis, pWeapon); return nullptr;
				}
			}
			pThis->CurrentBurstIndex = 0;
		}
		pExt->LastWeaponType = pWeapon;
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝

	if (Unsorted::MAP_DEBUG_MODE() || !pTarget || (pTargetFoot && pTargetFoot->InLimbo))
	{ RunEndHook(pThis, pWeapon); return nullptr; }

	// ╔═══ HOOK: TechnoClass_FireAt_Early @ 0x6FDDC0 ══════════════════════════╗
	// W1: Replaces §SUICIDE.  Handles AE discardables, DelayedFire, Suicide,
	//     OnlyAttacker, AttachEffect.  Returns 0x6FDE0E or 0x6FDE03.
	{
		if (pExt->AE.flags.HasOnFireDiscardables)
			for (auto& ae : pExt->PhobosAE)
				if (ae && !ae->ShouldBeDiscarded) ae->DiscardOnFire();

		{
			auto& timer = pExt->DelayedFireTimer;
			if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != which)
				pExt->ResetDelayedFireTimer();

			if (pWeaponExt->DelayedFire_Duration.isset()
			 && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
			{
				if (pWeaponExt->DelayedFire_PauseFiringSequence
				 && (rtti == AbstractType::Infantry
					 || (rtti == AbstractType::Unit && !pThis->HasTurret()
						 && !pTechnoType->Voxel)))
				{
					RunEndHook(pThis, pWeapon); return nullptr;
				}

				if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst
				 || pThis->CurrentBurstIndex == 0)
				{
					if (timer.InProgress())
					{ RunEndHook(pThis, pWeapon); return nullptr; }
					if (!timer.HasStarted())
					{
						pExt->DelayedFireWeaponIndex = which;
						timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(
							pWeaponExt->DelayedFire_Duration), 0));
						// DelayedFire anim creation omitted – see hook source
						RunEndHook(pThis, pWeapon); return nullptr;
					}
					else pExt->ResetDelayedFireTimer();
				}
			}

			if (pTargetTechno) {
				auto* pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
				if (pWeaponExt->NoRepeatFire > 0)
					pTargetExt->LastBeLockedFrame = Unsorted::CurrentFrame;

				if (pWeaponExt->AttachEffect_Enable) {
					auto* info = &pWeaponExt->AttachEffects;
					PhobosAttachEffectClass::Attach(pTargetTechno, pThis->Owner,
						pThis, pWeapon->Warhead, info);
					PhobosAttachEffectClass::Detach(pTargetTechno, info);
					PhobosAttachEffectClass::DetachByGroups(pTargetTechno, info);
				}
			}

			if (pWeapon->Suicide && pThis->IsAlive) {
				RunEndHook(pThis, pWeapon);
				if (pThis->IsAlive) {
					int dmg = pThis->Health;
					pThis->ReceiveDamage(&dmg, 0, RulesClass::Instance->C4Warhead,
						nullptr, false, true, nullptr);
				}
				return nullptr;
			}

			if (pWeaponExt->OnlyAttacker.Get() && pTarget == pThis->Target && pTargetTechno)
				TechnoExtContainer::Instance.Find(pTargetTechno)->AddFirer(pWeapon, pThis);
		}
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝

#if 0 // ⚠ §SUICIDE  DEAD CODE – W1
	if (pWeapon->Suicide) { /* vanilla: TakeDamage on self */ }
#endif

	// Particle / wave guard
	if ((pWeapon->UseFireParticles && pThis->Sys.Fire)
	 || (pWeapon->IsRailgun && pThis->Sys.Railgun)
	 || (pWeapon->UseSparkParticles && pThis->Sys.Spark)
	 || (pWeapon->IsSonic && pThis->Wave))
	{
		RunEndHook(pThis, pWeapon); return nullptr;
	}

	// Spawner
	if (pWeapon->Spawner && pThis->SpawnManager)
	{
		pThis->SpawnManager->SetTarget(pTarget);
		CoordStruct c = pThis->GetCoords();

		if (pThis->IsOwnedByCurrentPlayer || pThis->DiscoveredByCurrentPlayer) {

			if (!MapClass::Instance->IsLocationShrouded(c) && !MapClass::Instance->IsLocationFogged(c))
			{ RunEndHook(pThis, pWeapon); return nullptr; }

			if (rtti == AbstractType::Aircraft && pThis->IsOwnedByCurrentPlayer)
			{ RunEndHook(pThis, pWeapon); return nullptr; }
		}

		if (!pTargetFoot) { RunEndHook(pThis, pWeapon); return nullptr; }

		auto* pOwner = pTargetFoot->GetOwningHouse();

		if (pOwner->ControlledByCurrentPlayer() && pWeapon->RevealOnFire) {
			MapClass::Instance->RevealArea1(&c, 3, pOwner, 0, 0, 0, 1, 0);
		}

		MapClass::Instance->RevealArea3(&c, 0, 4, 0);
		RunEndHook(pThis, pWeapon); return nullptr;
	}

	// Drain weapon
	if (pWeapon->DrainWeapon && pTargetTechno)
	{
		bool drainable = pTargetTechno->GetTechnoType()->Drainable;
		if (!drainable) { RunEndHook(pThis, pWeapon); return nullptr; }
		pThis->SetDrainTarget(pTargetTechno);
		pThis->SetTarget(nullptr);
		RunEndHook(pThis, pWeapon); return nullptr;
	}

	// ╔═══ HOOK: TechnoClass_FireAt_SprayOffsets @ 0x6FDFA8 ════════════════════╗
	// W2: §SPRAY is dead code. Coord calc uses ext SprayOffsets table.
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §SPRAY  DEAD CODE – W2
	// vanilla SprayOffsets init table (static array at 0xB0EAA8)
#endif

	// ── Target coordinate ─────────────────────────────────────────────────────
	CoordStruct    coord1 {};
	AbstractClass* pEffectiveTarget = pTarget;

	if (pTechnoType->SprayAttack)
	{
		// Uses ext table – matches hook 0x6FDFA8 behaviour
		auto& extOffsets = pTechnoTypeExt->SprayOffsets;
		const int sz = (int)extOffsets.size();
		if (sz > 0)
		{
			pThis->SprayOffsetIndex = pThis->CurrentBurstIndex
				? (sz / pWeapon->Burst + pThis->SprayOffsetIndex) % sz
				: ScenarioClass::Instance->Random.RandomRanged(0, sz - 1);
			const CoordStruct& off = extOffsets[pThis->SprayOffsetIndex];
			coord1 = { pThis->Location.X + off.X,
					   pThis->Location.Y + off.Y,
					   pThis->Location.Z + off.Z };
		}
		// LABEL_51: SprayAttack always re-fetches target
		pEffectiveTarget = MapClass::Instance->GetCellAt(coord1);
	}
	else if (pWeapon->AreaFire) // hook 0x6FE140
	{
		auto c = pThis->GetCoords();
		// ???????
		CellStruct cell = CellClass::Coord2Cell(c);
		CellClass* refCell = MapClass::Instance->GetCellAt(cell);
		coord1 = refCell->GetCoords();
		CellClass* pCell = MapClass::Instance->GetCellAt(coord1);

		// ╔═══ HOOK: TechnoClass_FireAt_AreaFire @ 0x6FE19A ════════════════════╗
		switch (TechnoExtData::ApplyAreaFire(pThis, pCell, pWeapon))
		{
		case AreaFireReturnFlag::DoNotFire:
			RunEndHook(pThis, pWeapon); return nullptr;
		case AreaFireReturnFlag::SkipSetTarget:
			pEffectiveTarget = pThis; break;
		default: // Continue
			pEffectiveTarget = pCell; break;
		}
		// ╚═══════════════════════════════════════════════════════════════════╝
	}
	else
	{
		coord1 = pTargetFoot
			? pTargetFoot->GetTargetCoords()
			: pTarget->GetCoords();
	}

	// ── Fire origin (fireOrigin) ──────────────────────────────────────────────
	CoordStruct fireOrigin = pThis->GetFLH(which, 0, 0, 0); // fire origin (FLH)

	const bool bulletHasROTorVertical =
		pBulletType->ROT > 0 || pBulletType->Vertical;

	if (bulletHasROTorVertical)
	{
		//unused
		//DirStruct fd = pThis->GetRealFacing();

		if (pBulletType->Vertical) { 
			fireOrigin = pThis->GetCoords();
		}
	}

	// ── Damage ────────────────────────────────────────────────────────────────
	int finalDamage = pWeapon->Damage;

	// ╔═══ HOOK: TechnoClass_FireAt_AllowDamage @ 0x6FE31C ═════════════════════╗
	// W3: Replaces IsSonic/UseFireParticles zero-damage check.
	{
		const bool applyDamage = pWeaponExt->ApplyDamage.Get(
			!pWeapon->IsSonic && !pWeapon->UseFireParticles);

		if (!applyDamage)
			finalDamage = 0;
		else if (finalDamage > 0)
		{
			// ╔═══ HOOK: TechnoClass_FireAt_DamageMult @ 0x6FE337 ══════════════╗
			// W4: Replaces FP*FP*dmg + vet bonus. §VET_BONUS is dead code.
			finalDamage = (int)TechnoExtData::GetDamageMult(pThis, (double)finalDamage);
			// ╚══════════════════════════════════════════════════════════════════╝
		}
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §VET_BONUS  DEAD CODE – W4
#endif

	// ╔═══ HOOK: TechnoClass_FireAt_OccupyDamageBonus @ 0x6FE3E3 ═══════════════╗
	// W5: Replaces OccupyDamage + BunkerDamage + OpenTopped + DiskLaser.
	{
		if (pThis->CanOccupyFire())
		{
			float mult = (rtti == AbstractType::Building)
				? BuildingTypeExtContainer::Instance.Find(
					static_cast<BuildingClass*>(pThis)->Type)
				->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)
				: RulesClass::Instance->OccupyDamageMultiplier;

			finalDamage = (int)(finalDamage * mult);
		}

		if (pThis->InOpenToppedTransport)
		{
			finalDamage = (int)(finalDamage * pTechnoTypeExt->OpenTransport_DamageMultiplier);
			finalDamage = (int)(finalDamage * (pThis->Transporter
				? GET_TECHNOTYPEEXT(pThis->Transporter)->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier)
				: RulesClass::Instance->OpenToppedDamageMultiplier));
		}

		if (pWeapon->DiskLaser)
		{
			auto* pDisk = GameCreate<DiskLaserClass>();
			++pThis->CurrentBurstIndex;
			int rearm = pThis->GetROF(which);
			TechnoExtData::SetChargeTurretDelay(pThis, rearm, pWeapon);
			pThis->RearmTimer.Start(rearm);
			pThis->CurrentBurstIndex %= pWeapon->Burst;
			DiskLaserExtContainer::Instance.Find(pDisk)->WeaponIdx = which;
			static_cast<FakeDiskLaserClass*>(pDisk)->__Fire(pThis, pTarget, pWeapon, finalDamage);

			RunEndHook(pThis, pWeapon); return nullptr; // 0x6FE4E7
		}
	}

	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §DMG_MULTS + §DISK  DEAD CODE – W5
#endif

	// ── Bullet creation ───────────────────────────────────────────────────────
	// ╔═══ HOOK: TechnoClass_FireAt_CreateBullet @ 0x6FE53F ════════════════════╗
	const int dist2d = (int)Math::sqrt(
		(double)((coord1.Y - fireOrigin.Y) * (coord1.Y - fireOrigin.Y)
			+ (coord1.X - fireOrigin.X) * (coord1.X - fireOrigin.X)));

	int bulletSpeed = pWeapon->GetWeaponSpeed(dist2d);
	
	auto* pBullet = pBulletTypeExt->CreateBullet(
		pEffectiveTarget, pThis, finalDamage, pWeapon->Warhead,
		bulletSpeed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	UnlimboDetonateFireTemp::Bullet = pBullet;
	UnlimboDetonateFireTemp::InSelected = pThis->IsSelected;
	UnlimboDetonateFireTemp::InLimbo = pThis->InLimbo;
	// ╚═══════════════════════════════════════════════════════════════════════╝

	// velocity declared here so it is in scope for both the main shot
	// and HandlePostBulletLogic (LimboLaunch reuses it – matches asm)
	VelocityClass velocity {};

	if (!pBullet)
		return HandlePostBulletLogic(pThis, pTechnoType, pWeapon, pWeaponExt, nullptr,
			pTarget, fireOrigin, coord1, finalDamage, which, velocity);

	// ── Bullet initial setup ──────────────────────────────────────────────────
	pBullet->WeaponType = pWeapon;
	pBullet->Limbo();

	if (pThisFoot) {
		if (pThisFoot->Locomotor->Is_Moving() && !pTechnoType->JumpJet)
			pBullet->IsInaccurate = true;
	}

	if (!pBullet->IsInaccurate && !pBulletType->Inaccurate) {
		if (auto pTTarCom = flag_cast_to<TechnoClass*>(pThis->TarCom)) {
			pTTarCom->EstimatedHealth -= FakeTechnoClass::__AdjustDamage(pThis, discard_t(), pTTarCom, pWeapon);
		}
	}

	// ── Delta / scatter / direction ───────────────────────────────────────────
	CoordStruct pTgtCoord;
	pThis->GetMovingTargetCoords(&pTgtCoord);
	CoordStruct delta { pTgtCoord.X - fireOrigin.X, pTgtCoord.Y - fireOrigin.Y, pTgtCoord.Z - fireOrigin.Z };
	CoordStruct scatterDelta = delta;

	if (pBulletType->FlakScatter && pBulletType->Inviso)
	{
		// ╔═══ HOOK: TechnoClass_FireAt_BallisticScatter1/2 @ 0x6FE709 / 0x6FE7FE ╗
		if (*(bool*)((char*)pBulletType + 0x2A3) && !*(bool*)((char*)pBulletType + 0x29E))
		{
			// Mode A: 2D polar (hook 0x6FE709)
			const int min_s = pBulletTypeExt->BallisticScatterMin.Get(Leptons(0));
			const int max_s = pBulletTypeExt->BallisticScatterMax.Get(
				Leptons(RulesClass::Instance->BallisticScatter));
			const int sd = ScenarioClass::Instance->Random.RandomRanged(min_s, max_s);
			const double rd = ScenarioClass::Instance->Random.RandomDouble() * Math::GAME_TWOPI - Math::DEG90_AS_RAD;
			const double ra = (double)((int16_t)(int)(rd * Math::BINARY_ANGLE_MAGIC) - 0x3FFF)
				* Math::DIRECTION_FIXED_MAGIC;
			scatterDelta.Y = (int)((double)delta.Y - Math::sin(ra) * sd);
			scatterDelta.X = (int)(Math::cos(ra) * sd + (double)delta.X);
		}
		else
		{
			// Mode B: proportional 3D (hook 0x6FE7FE)
			const int min_s = pBulletTypeExt->BallisticScatterMin.Get(
				Leptons(RulesClass::Instance->BallisticScatter / 2));
			const int max_s = pBulletTypeExt->BallisticScatterMax.Get(
				Leptons(RulesClass::Instance->BallisticScatter));

			const double d3 = Math::sqrt((double)delta.X * delta.X
										   + (double)delta.Y * delta.Y
										   + (double)delta.Z * delta.Z);

			const int   rng = pThis->GetWeaponRange(which);
			const double pr = rng > 0 ? d3 * ScenarioClass::Instance->Random.RandomRanged(min_s, max_s) / (double)rng : 0.0;
			const double rd = ScenarioClass::Instance->Random.RandomDouble() * Math::GAME_TWOPI - Math::DEG90_AS_RAD;
			const double ra = (double)((int16_t)(int)(rd * Math::BINARY_ANGLE_MAGIC) - 0x3FFF)
				* Math::DIRECTION_FIXED_MAGIC;
			scatterDelta.Y = (int)((double)delta.Y - Math::sin(ra) * pr);
			scatterDelta.X = (int)(Math::cos(ra) * pr + (double)delta.X);
		}
		// ╚═══════════════════════════════════════════════════════════════════╝
		delta.X = scatterDelta.X;
		delta.Y = scatterDelta.Y;
	}

	// Horizontal direction (use DirStruct helpers instead of raw math)
	DirStruct horzDirDS;
	if (bulletHasROTorVertical)
	{
		DirStruct fd;
		pThis->GetRealFacing(&fd);

		horzDirDS = fd;

		if (pBulletType->Dropping)
		{ fireOrigin = pThis->GetCoords(); }
	}
	else
	{
		const double at = Math::atan2(-(double)delta.Y, (double)scatterDelta.X);
		horzDirDS = DirStruct((int16_t)(int)((at - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC));
	}

	// Speed clamp to half 3D distance
	const double dist3d = Math::sqrt(
		(double)scatterDelta.X * scatterDelta.X
	  + (double)delta.Y * delta.Y
	  + (double)delta.Z * delta.Z);

	if ((double)bulletSpeed > dist3d / 2.0) bulletSpeed = (int)(dist3d / 2.0);

	const int radialSegs = pTechnoType->RadialFireSegments;
	if (pBulletType->ROT > 0 || pBulletType->Vertical) {
		if (!radialSegs) bulletSpeed = 1;
		pBullet->Speed = pWeapon->Speed;
	}

	// Build velocity vector
	velocity.X = (double)bulletSpeed;
	velocity.Y = 0.0;
	velocity.Z = 0.0;

	if (radialSegs <= 0) {
		velocity.SetIfZeroXY();
		const double xyLen = velocity.LengthXY();
		const double radH = horzDirDS.GetRadian<32>();
		velocity.X = Math::cos(radH) * xyLen;
		velocity.Y = -Math::sin(radH) * xyLen;
	} else {
		const double radOffset = Math::GAME_PI / (double)radialSegs
			* (double)pThis->__RadialFireCounter_43C - Math::DEG90_AS_RAD;
		auto pFacing = pThis->PrimaryFacing.Current();
		const double faceRad = (double)((int16_t)pFacing.Raw - 0x3FFF)
			* Math::DIRECTION_FIXED_MAGIC;
		const DirStruct radDirDS = DirStruct((int16_t)(int)(
			(faceRad + radOffset - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC));

		velocity.SetIfZeroXY();

		const double xyLen2 = velocity.LengthXY();
		const double radH2 = radDirDS.GetRadian<32>();

		velocity.X = Math::cos(radH2) * xyLen2;
		velocity.Y = -Math::sin(radH2) * xyLen2;

		if (++pThis->__RadialFireCounter_43C >= radialSegs) pThis->__RadialFireCounter_43C = 0;
	}

	// Elevation direction (elevDir)
	DirStruct elevDir;
	elevDir.Raw = 0x3FFF;
	bool directionLegal = true;

	if (pBulletType->Arcing)
	{
		// ╔═══ HOOK: TechnoClass_FireAt_ApplyGravity @ 0x6FECB2 ════════════════╗
		double gravity = BulletTypeExtData::GetAdjustedGravity(pBulletType);
		// ╚═══════════════════════════════════════════════════════════════════╝

		const double xyDist = Math::sqrt(
			(double)delta.Y * delta.Y + (double)scatterDelta.X * scatterDelta.X);
		directionLegal = (bool)Game::func_48A8D0_Legal(
			pThis->CanReachTarget(which),
			bulletSpeed, (int)xyDist, delta.Z, gravity, &elevDir);
	}
	else if (pBulletTypeExt->VerticalInitialFacing.Get(pBulletType->Voxel || pBulletType->Vertical))
	{
		// ╔═══ HOOK: TechnoClass_FireAt_VerticalInitialFacing @ 0x6FED2F ════════╗
			const int tZ = pEffectiveTarget->GetCoords().Z;
			const int sZ = pThis->GetCoords().Z;
			elevDir.Raw = (tZ >= sZ) ? (int16_t)0x4000 : (int16_t)0x8000;
		// ╚═══════════════════════════════════════════════════════════════════╝
	}
	else
	{
		int absZ = Math::abs(delta.Z);
		if (absZ > 200)
		{
			double zOffset = 0.0;

			if (auto pBldTarcom = cast_to<BuildingClass*>(pThis->TarCom)) {
				CoordStruct wc;
				pThis->vt_entry_300(&wc, which);
				const int newDZ = 200 * pBldTarcom->Type->Height - wc.Z;
				absZ = Math::abs(newDZ); 
				delta.Z = newDZ;
				if ((double)absZ < 20.0) zOffset = 0.0;
			}

			const double xyFlat = MaxImpl(Math::sqrt(
				(double)delta.Y * delta.Y + (double)scatterDelta.X * scatterDelta.X), 0.05);

			double elev = Math::atan2((float)(absZ - zOffset), (float)xyFlat);

			if (delta.Z < 0) elev = -elev;
				elevDir.Raw = (int16_t)(int)((elev - (float)Math::DEG90_AS_RAD) * (float)Math::BINARY_ANGLE_MAGIC);
		}
	}

	// Pitch correction then apply elevDir elevation to velocity
	const double xySpeed3d = velocity.LengthXY();
	const double pitchRaw = Math::atan2(velocity.Z, xySpeed3d) - Math::DEG90_AS_RAD;
	const double pitchRad = (double)((int16_t)(int)(pitchRaw * Math::BINARY_ANGLE_MAGIC) - 0x3FFF)
		* Math::DIRECTION_FIXED_MAGIC;
	const double speed3d = velocity.Length();

	if (pitchRad != 0.0) {
		const double cosPitch = Math::cos(pitchRad);
		if (cosPitch != 0.0) { velocity.X /= cosPitch; velocity.Y/= cosPitch; }
	}

	const double elevRad = elevDir.GetRadian<32>();
	const float cos_elevRad = Math::cos(elevRad);
	velocity.X *= cos_elevRad;
	velocity.Y *= cos_elevRad;
	velocity.Z = Math::sin(elevRad) * speed3d;

	// ── Unlimbo ───────────────────────────────────────────────────────────────
	if (!directionLegal || !pBullet->MoveTo(fireOrigin, velocity))
	{
		pBullet->Release();
		pBullet = nullptr;
		return HandlePostBulletLogic(pThis, pTechnoType, pWeapon, pWeaponExt, nullptr,
			pTarget, fireOrigin, coord1, finalDamage, which, velocity);
	}

	// ── Unlimbo success ───────────────────────────────────────────────────────

	// ╔═══ HOOK: TechnoClass_FireAt_FSW @ 0x6FF008 ════════════════════════════╗
	// Zero-gravity arcing fix + firestorm wall check.
	{
		auto* pBulletExt = BulletExtContainer::Instance.Find(pBullet);

		if (!(pBulletExt->Trajectory &&
			pBulletExt->Trajectory->Flag != TrajectoryFlag::Invalid))
		{
			if (pBullet->Type->Arcing
			 && BulletTypeExtContainer::Instance.Find(pBullet->Type)->GetAdjustedGravity() == 0.0)
			{
				pBullet->Velocity *= -1;
				auto* pBTExtG = BulletTypeExtContainer::Instance.Find(pBullet->Type);
				if (pBTExtG->Gravity_HeightFix)
				{
					const double spd = pBullet->Velocity.Length();
					pBullet->Velocity.X = pBullet->TargetCoords.X - pBullet->SourceCoords.X;
					pBullet->Velocity.Y = pBullet->TargetCoords.Y - pBullet->SourceCoords.Y;
					pBullet->Velocity.Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
					const double mag = pBullet->Velocity.Length();
					pBullet->Velocity *= spd / mag;
				}
			}
		}
		if (HouseExtContainer::Instance.IsAnyFirestormActive && !pBullet->Type->IgnoresFirestorm)
		{
			const CoordStruct crdSrc = fireOrigin, crdTgt = coord1;
			const auto crd = MapClass::Instance->FindFirstFirestorm(
				crdSrc, crdTgt, pBullet->Owner->Owner);
			if (crd.IsValid())
			{
				pBullet->Target = MapClass::Instance->GetCellAt(crd)->GetContent();
				pBullet->Owner->ShouldLoseTargetNow = 1;
			}
		}
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝

	if (pBulletType->Inviso && pTargetFoot && pTargetFoot->OnBridge)
		pBullet->OnBridge = true;

	if (pThis->CanOccupyFire() && rtti == AbstractType::Building)
	{
		auto* pBldg = static_cast<BuildingClass*>(pThis);

		++pBldg->FiringOccupantIndex;
		pBldg->FiringOccupantIndex %= pBldg->GetOccupantCount();
	}

	// ╔═══ HOOK: TechnoClass_FireAt_RecordBullet @ 0x6FF08B ════════════════════╗
	FireAtTemp::FireBullet = pBullet;
	// ╚═══════════════════════════════════════════════════════════════════════╝

	// ╔═══ HOOK: TechnoClass_FireAt_TurretRecoil @ 0x6FF0DD ════════════════════╗
	// W6: §RECOIL is dead code.
	if (!pWeaponExt->TurretRecoil_Suppress)
		TechnoExtContainer::Instance.Find(pThis)->RecordRecoilData();
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §RECOIL  DEAD CODE – W6
#endif

	// ╔═══ HOOK: TechnoClass_FireAt_Additionals_Start @ 0x6FF15F ═══════════════╗
	// W7: §PARTICLES / §ROF / §ANIM / §SOUND are dead code.
	{
		FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(
			fireOrigin, coord1, pWeapon->Projectile, pThis->Owner);
		pExt->FiringObstacleCell = FireAtTemp::pObstacleCell;
		AbstractClass* pObstacleOrTarget = FireAtTemp::pObstacleCell
			? FireAtTemp::pObstacleCell : pTarget;

		if (pWeapon->UseFireParticles && !pThis->Sys.Fire && pWeapon->AttachedParticleSystem)
			pThis->Sys.Fire = GameCreate<ParticleSystemClass>(
				pWeapon->AttachedParticleSystem, fireOrigin, pObstacleOrTarget, pThis);

		if (pWeapon->UseSparkParticles && !pThis->Sys.Spark && pWeapon->AttachedParticleSystem)
			pThis->Sys.Spark = GameCreate<ParticleSystemClass>(
				pWeapon->AttachedParticleSystem, fireOrigin, pObstacleOrTarget, pThis);

		if (pWeapon->AttachedParticleSystem
		 && (pWeaponExt->IsDetachedRailgun || (pWeapon->IsRailgun && !pThis->Sys.Railgun)))
		{
			CoordStruct railPt;
			auto* coord2 = pThis->DealthParticleDamage(&railPt, &fireOrigin, pTarget, pWeapon);
			auto* pRG = GameCreate<ParticleSystemClass>(
				pWeapon->AttachedParticleSystem, &fireOrigin, nullptr, pThis, coord2, nullptr);
			if (!pWeaponExt->IsDetachedRailgun) pThis->Sys.Railgun = pRG;
		}

		++pThis->CurrentBurstIndex;
		int ROF = pThis->GetROF(which);
		if (pThis->Berzerk) {
			ROF = (int)(ROF * pTechnoTypeExt->BerserkROFMultiplier
				.Get(RulesExtData::Instance()->BerserkROFMultiplier));
		}
		TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);
		pThis->RearmTimer.Start(ROF);

		// Anim index (temporarily revert burst index for Ares compat)
		--pThis->CurrentBurstIndex;
		
		AnimTypeClass* pFiringAnim = nullptr;

		if (pThis->CanOccupyFire())
		{
			if (pWeaponExt->OccupantAnim_UseMultiple.Get() && !pWeaponExt->OccupantAnims.empty())
			{
				if (pWeaponExt->OccupantAnims.size() == 1)
				{
					pFiringAnim = pWeaponExt->OccupantAnims[0];
				}
				else
				{
					pFiringAnim = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
				}
			}
			else
			{
				pFiringAnim = pWeapon->OccupantAnim;
			}
		}
		else
		{
			if (pWeapon->Anim.Count > 0)
			{
				int nIdx = -1;

				if (pWeapon->Anim.Count == 1)
					nIdx = 0;
				else
				{
					DirStruct facing {};
					pThis->GetRealFacing(&facing);

					if (pWeapon->Anim.Count == 8)
					{
						nIdx = (facing.GetFacing<8>() + 8 / 8) % 8;
					}
					else if (pWeapon->Anim.Count == 16)
					{
						nIdx = (facing.GetFacing<16>() + 16 / 8) % 16;
					}
					else if (pWeapon->Anim.Count == 32)
					{
						nIdx = (facing.GetFacing<32>() + 32 / 8) % 32;
					}
					else if (pWeapon->Anim.Count == 64)
					{
						nIdx = (facing.GetFacing<64>() + 64 / 8) % 64;
					}
					else
					{
						//only execute if the anim count is more than 1
						const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

						// 2^highest is the frame count, 3 means 8 frames
						if (highest >= 3)
						{
							nIdx = facing.GetValue(highest, 1u << (highest - 3));
						}

						nIdx %= pWeapon->Anim.Count;
					}
				}

				pFiringAnim = pWeapon->Anim.Items[nIdx];
			}
		}

		if (pWeapon->Report.Count > 0 && !pTechnoType->IsGattling)
		{
			const int idx = pWeapon->Report.Count == 1
				? 0 : (pThis->weapon_sound_randomnumber_3C8 % pWeapon->Report.Count);
			VocClass::SafeImmedietelyPlayAt(pWeapon->Report[idx], &fireOrigin, nullptr);
		}

		auto TryFeedbackWeapon = [&](WeaponTypeClass* fbWpn) -> bool
			{
				if (!pThis->InOpenToppedTransport || fbWpn->FireInTransport)
				{
					WeaponTypeExtData::DetonateAt1(fbWpn, pThis, pThis, true, nullptr);
					if (!pThis->IsAlive) return false;
				}
				return true;
			};

		if (auto* fbWpn = pWeaponExt->FeedbackWeapon.Get()){
			if (!TryFeedbackWeapon(fbWpn))
				return HandlePostBulletLogic(pThis,pTechnoType, pWeapon, pWeaponExt, pBullet,
					pTarget, fireOrigin, coord1, finalDamage, which, velocity);
		}

		if (pExt->AE.flags.HasFeedbackWeapon)
		{
			for (auto const& pAE : pExt->PhobosAE)
			{
				if (!pAE || !pAE->IsActive()) continue;
				if (auto wfb = pAE->GetType()->FeedbackWeapon) {
					if (pThis->InOpenToppedTransport && !wfb->FireInTransport) continue; 
					else WeaponTypeExtData::DetonateAt1(wfb, pThis, pThis, true, nullptr);
				}
			}
			if (!pThis->IsAlive)
				return HandlePostBulletLogic(pThis, pTechnoType, pWeapon, pWeaponExt, pBullet,
					pTarget, fireOrigin, coord1, finalDamage, which, velocity);
		}

		if (pFiringAnim)
		{
			auto* pAn = GameCreate<AnimClass>(pFiringAnim, fireOrigin, 0, 1,
				AnimFlag::AnimFlag_600, 0, 0);
			AnimExtData::SetAnimOwnerHouseKind(pAn, pThis->GetOwningHouse(),
				pThis->Target ? pThis->Target->GetOwningHouse() : nullptr,
				pThis, false, false);
			const auto rend = pThis->GetRenderCoords();
			pAn->ZAdjust = pThis->GetOccupantCount() > 0
				? -200 : ((fireOrigin.Y - rend.Y) / -4 >= 0 ? 0 : (fireOrigin.Y - rend.Y) / -4);
		}

		// Redirect to obstacle cell if any
		FireAtTemp::originalTargetCoords = coord1;
		FireAtTemp::pOriginalTarget = pTarget;
		if (FireAtTemp::pObstacleCell)
		{
			coord1 = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
			pTarget = FireAtTemp::pObstacleCell;
		}
	}
	// Resumes at 0x6FF48A (TargetLaser + IsLaser)
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §PARTICLES / §ROF / §ANIM / §SOUND  DEAD CODE – W7
#endif

	// ╔═══ HOOK: TechnoClass_FireAt_IsLaser @ 0x6FF48D ═════════════════════════╗
	// W8: Handles TargetLaser timer and replaces laser drawing.
	{
		if (pTechnoType->TargetLaser && pThis->Owner->ControlledByCurrentPlayer())
		{
			if (pTechnoTypeExt->TargetLaser_WeaponIdx.empty()
			 || pTechnoTypeExt->TargetLaser_WeaponIdx.Contains(which))
				pThis->TargetLaserTimer.Start(pTechnoTypeExt->TargetLaser_Time.Get());
		}

		if (pWeapon->IsLaser)
		{
			const int thickness = pWeaponExt->Laser_Thickness;
			if (auto* pBld = cast_to<BuildingClass*, false>(pThis))
			{
				auto* pTW = pBld->GetPrimaryWeapon()->WeaponType;
				if (auto* pLaser = pBld->CreateLaser(
					pTarget,
					which, pTW, CoordStruct::Empty))
				{
					pLaser->Thickness = (thickness == -1) ? 3 : thickness;
					auto* pBTD = BuildingTypeExtContainer::Instance.Find(pBld->Type);
					if (pBTD->PrismForwarding.CanAttack() && pBld->SupportingPrisms > 0)
					{
						if (pBTD->PrismForwarding.Intensity < 0)
							pLaser->Thickness -= pBTD->PrismForwarding.Intensity;
						else if (pBTD->PrismForwarding.Intensity > 0)
							pLaser->Thickness += pBTD->PrismForwarding.Intensity
							* pBld->SupportingPrisms;
						pLaser->IsSupported = true;
					}
				}
			}
			else if (auto* pLaser = pThis->CreateLaser(
				pTarget,
				which, pWeapon, CoordStruct::Empty))
			{
				if (thickness == -1) pLaser->Thickness = 2;
				else { pLaser->Thickness = thickness; pLaser->IsSupported = (thickness > 3); }
			}
			// IsLaser path: jumps to 0x6FF656 (Made_A_Kill), skips all beam effects
			HandleAfterEffects(pThis, pWeapon, pTarget);
			return HandlePostBulletLogic(pThis, pTechnoType, pWeapon, pWeaponExt, pBullet,
				pTarget, fireOrigin, coord1, finalDamage, which, velocity);
		}
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §LASER  DEAD CODE – W8
#endif

	// Not IsLaser → 0x6FF57D: ElectricBolt / RadBeam / RadEruption
	if (pWeapon->IsElectricBolt)
		pThis->FireEBolt(pTarget);
	else if (pWeapon->IsRadBeam)
		FakeTechnoClass::__FireBeam(pThis, which, pWeapon, pTarget,
			(pWeapon->Warhead && pWeapon->Warhead->Temporal) ? RadBeamType::Temporal : RadBeamType::RadBeam);
	else if (pWeapon->IsRadEruption)
		FakeTechnoClass::__FireRadEruption(pThis,pWeapon , pWeapon->Warhead->CellSpread);

	// ╔═══ HOOK: TechnoClass_FireAt_Additionals_End @ 0x6FF5F5 ════════════════╗
	// W9: §MAGBEAM is dead code.  Handles wave, ammo, burst, interceptor, shake.
	{
		if (pWeaponExt->IsWave() && !pThis->Wave)
		{
			WaveType nType = pWeapon->IsMagBeam ? WaveType::Magnetron
				: (pWeaponExt->Wave_IsBigLaser ? WaveType::BigLaser : WaveType::Laser);
			pThis->Wave = WaveExtData::Create(fireOrigin, coord1, pThis, nType, pTarget, pWeapon, which);
		}
		TechnoExtData::DecreaseAmmo(pThis, pWeapon);

		// Restore original target / coords
		coord1 = FireAtTemp::originalTargetCoords;
		pTarget = FireAtTemp::pOriginalTarget;

		++pThis->CurrentBurstIndex;
		pThis->CurrentBurstIndex %= pWeapon->Burst;
		if (pExt->ForceFullRearmDelay)
		{
			pExt->ForceFullRearmDelay = false;
			pThis->CurrentBurstIndex = 0;
		}

		if (auto* pBT = cast_to<BulletClass* const, false>(pTarget))
		{
			if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor())
			{
				auto* pBExt2 = BulletExtContainer::Instance.Find(pBT);
				auto* pBTExt3 = BulletTypeExtContainer::Instance.Find(pBT->Type);
				if (!pBTExt3->Armor.isset())
					pBExt2->InterceptedStatus |= InterceptedStatus::Locked;
				auto* pFireExt = BulletExtContainer::Instance.Find(pBullet);
				pFireExt->InterceptorTechnoType = pTechnoType;
				pFireExt->InterceptedStatus |= InterceptedStatus::Targeted;
				if (!pTechnoTypeExt->Interceptor_ApplyFirepowerMult)
					pBullet->Health = pWeapon->Damage;
			}
		}

		if (!Phobos::Config::HideShakeEffects && (!pWeaponExt->ShakeLocal.Get() || pThis->IsOnMyView()))
		{
			if (pWeaponExt->Xhi || pWeaponExt->Xlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX,
					ScenarioClass::Instance->Random.RandomRanged(pWeaponExt->Xlo, pWeaponExt->Xhi));
			if (pWeaponExt->Yhi || pWeaponExt->Ylo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY,
					ScenarioClass::Instance->Random.RandomRanged(pWeaponExt->Ylo, pWeaponExt->Yhi));
		}
	}
	// ╚═══════════════════════════════════════════════════════════════════════╝
#if 0 // ⚠ §MAGBEAM  DEAD CODE – W9
#endif

	// 0x6FF660: Made_A_Kill / visibility / cleanup
	HandleAfterEffects(pThis, pWeapon, pTarget);

	return HandlePostBulletLogic(pThis,pTechnoType, pWeapon, pWeaponExt, pBullet,
		pTarget, fireOrigin, coord1, finalDamage, which, velocity);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6FDD50, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(CALL, 0x415F12, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(CALL, 0x51DF77, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(CALL, 0x7413CE, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4288, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E9060, FakeTechnoClass::__Fire_At)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4D2C, FakeTechnoClass::__Fire_At)

#else

ASMJIT_PATCH(0x6FDE05, TechnoClass_FireAt_End, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0;
} ASMJIT_PATCH_AGAIN(0x6FF933, TechnoClass_FireAt_End, 0x5);

ASMJIT_PATCH(0x6FDD50, TechnoClass_FireAt_PreFire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	//GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(const int, nWeapon, 0x8);
	//GET(AbstractClass*, pTarget, EDI);

	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->CurrentWeaponIdx = nWeapon;

	return 0x0;
}

ASMJIT_PATCH(0x6FF905, TechnoClass_FireAt_FireOnce_A, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pInf = cast_to<InfantryClass*, false>(pThis))
	{
		GET(WeaponTypeClass*, pWeapon, EBX);

		if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->FireOnce_ResetSequence)
			InfantryExtContainer::Instance.Find(pInf)->SkipTargetChangeResetSequence = true;
	}

	return 0;
}

ASMJIT_PATCH(0x6FE337, TechnoClass_FireAt_DamageMult, 0x6)
{
	GET(int, damage, EDI);
	GET(TechnoClass*, pThis, ESI);

	int _damage = (int)TechnoExtData::GetDamageMult(pThis, (double)damage);
	R->Stack(0x28, GET_TECHNOTYPE(pThis));
	R->EDI(_damage);
	R->EAX(_damage);
	return 0x6FE3DF;
}

ASMJIT_PATCH(0x6FED2F, TechnoClass_FireAt_VerticalInitialFacing, 0x6)
{
	enum { Continue = 0x6FED39, SkipGameCode = 0x6FED8F };

	GET(FakeBulletTypeClass*, pBulletType, EAX);

	if (pBulletType->_GetExtData()->VerticalInitialFacing.Get(pBulletType->Voxel || pBulletType->Vertical))
		return Continue;

	return SkipGameCode;
}

ASMJIT_PATCH(0x6FF0DD, TechnoClass_FireAt_TurretRecoil, 0x6)
{
	enum { SkipGameCode = 0x6FF15B };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFSET(0xB0, -0x70));

	if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->TurretRecoil_Suppress)
	{
		GET(TechnoClass* const, pThis, ESI);
		TechnoExtContainer::Instance.Find(pThis)->RecordRecoilData();
	}

	return SkipGameCode;
}

static WeaponStruct* __fastcall TechnoClass_FireAt_GetWeapon_(TechnoClass* pTech, void*, int idx)
{
	return pTech->GetWeapon(TechnoExtContainer::Instance.Find(pTech)->CurrentWeaponIdx);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x6FDD69, TechnoClass_FireAt_GetWeapon_);

ASMJIT_PATCH(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	switch (TechnoExtData::ApplyAreaFire(pThis, pCell, pWeaponType))
	{
	case AreaFireReturnFlag::Continue:
	{
		R->EAX(pCell);
		return Continue;
	}
	case AreaFireReturnFlag::DoNotFire:
	{
		return DoNotFire;
	}
	case AreaFireReturnFlag::SkipSetTarget:
	{
		R->EAX(pThis);
		return SkipSetTarget;
	}
	default:
		return Continue;
		break;
	}
}

ASMJIT_PATCH(0x6FDDC0, TechnoClass_FireAt_Early, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AE.flags.HasOnFireDiscardables)
	{
		for (auto& attachEffect : pExt->PhobosAE)
		{
			if (!attachEffect || attachEffect->ShouldBeDiscarded)
				continue;

			attachEffect->DiscardOnFire();
		}
	}

	// if (pThis->Passengers.FirstPassenger)
	// {
	// 	// TODO : implement this for UnitClass
	// 	pThis->DropOffParadropCargo();
	// }

	if (pWeapon)
	{
		auto pWeaponExt = pWeapon->_GetExtData();

		auto& timer = pExt->DelayedFireTimer;
		if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
			pExt->ResetDelayedFireTimer();

		if (pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
		{
			auto const rtti = pThis->WhatAmI();

			if (pWeaponExt->DelayedFire_PauseFiringSequence && (rtti == AbstractType::Infantry
				|| (rtti == AbstractType::Unit && !pThis->HasTurret() && !GET_TECHNOTYPE(pThis)->Voxel)))
			{
				return 0;
			}

			if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
			{
				if (timer.InProgress())
					return 0x6FDE03;

				if (!timer.HasStarted())
				{
					pExt->DelayedFireWeaponIndex = weaponIndex;
					timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
					auto pAnimType = pWeaponExt->DelayedFire_Animation;

					if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
						pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation.Get();

					auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

					if (pWeaponExt->DelayedFire_AnimOffset.isset())
						firingCoords = pWeaponExt->DelayedFire_AnimOffset;

					pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

					if (pWeaponExt->DelayedFire_InitialBurstSymmetrical)
						pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
							pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, { firingCoords.X, -firingCoords.Y, firingCoords.Z });

					return 0x6FDE03;
				}
				else
				{
					pExt->ResetDelayedFireTimer();
				}
			}
		}

		const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget);

		if (pTargetTechno)
		{
			auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			if (pWeaponExt->NoRepeatFire > 0)
			{
				pTargetExt->LastBeLockedFrame = Unsorted::CurrentFrame;
			}

			if (pWeaponExt->AttachEffect_Enable)
			{
				auto const info = &pWeaponExt->AttachEffects;
				PhobosAttachEffectClass::Attach(pTargetTechno, pThis->Owner, pThis, pWeapon->Warhead, info);
				PhobosAttachEffectClass::Detach(pTargetTechno, info);
				PhobosAttachEffectClass::DetachByGroups(pTargetTechno, info);
			}
		}

		if (pWeapon->Suicide && pThis->IsAlive)
		{
			int scdamage = pThis->Health;
			pThis->ReceiveDamage(&scdamage, 0, RulesClass::Instance->C4Warhead, nullptr, false, true, nullptr);
			return 0x6FDE03;
		}

		if (pWeaponExt->OnlyAttacker.Get() && pTarget == pThis->Target && pTargetTechno)
		{
			const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			pTargetExt->AddFirer(pWeapon, pThis);
		}
	}

	return 0x6FDE0E;
}

ASMJIT_PATCH(0x6FDD7D, TechnoClass_FireAt_UpdateWeaponType, 0x5)
{

	enum { CanNotFire = 0x6FDE03 };

	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(TechnoClass*, pThis, ESI);

	const auto pWH = pWeapon->Warhead;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
	auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWeapon->LimboLaunch)
	{
		if (!pWH->Parasite && pWHExt->UnlimboDetonate)
		{
			if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis))
			{
				if (pFoot->Locomotor->Is_Really_Moving_Now())
					return CanNotFire;
			}
		}
	}

	{
		if (pThis->CurrentBurstIndex && pWeapon != pExt->LastWeaponType && pTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst))
		{
			if (pExt->LastWeaponType && pExt->LastWeaponType->Burst)
			{

				const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pExt->LastWeaponType->Burst;
				const auto rof = static_cast<int>(ratio * pExt->LastWeaponType->ROF * pExt->AE.ROFMultiplier) - (Unsorted::CurrentFrame.get() - pThis->LastFireBulletFrame);

				if (rof > 0)
				{
					pThis->ROF = rof;
					pThis->RearmTimer.Start(rof);
					pThis->CurrentBurstIndex = 0;
					pExt->LastWeaponType = pWeapon;

					return CanNotFire;
				}
			}

			pThis->CurrentBurstIndex = 0;

		}

		pExt->LastWeaponType = pWeapon;
	}

	return 0;
}

namespace UnlimboDetonateFireTemp
{
	BulletClass* Bullet;
	bool InSelected;
	bool InLimbo;
}

ASMJIT_PATCH(0x6FE53F, TechnoClass_FireAt_CreateBullet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);


	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto pBulletExt = BulletTypeExtContainer::Instance.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	const auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright, false);

	UnlimboDetonateFireTemp::Bullet = ret;
	UnlimboDetonateFireTemp::InSelected = pThis->IsSelected;
	UnlimboDetonateFireTemp::InLimbo = pThis->InLimbo;
	R->EAX(ret);
	return 0x6FE562;
}

ASMJIT_PATCH(0x6FF7FF, TechnoClass_FireAt_UnlimboDetonate, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WarheadTypeClass* const, pWH, EAX);

	const auto pBullet = UnlimboDetonateFireTemp::Bullet;
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (pThis->IsAlive && pThis->Health > 0 && pBullet
		&& !UnlimboDetonateFireTemp::InLimbo && !pWH->Parasite && pWHExt->UnlimboDetonate)
	{
		if (pWHExt->UnlimboDetonate_KeepSelected)
		{
			TechnoExtContainer::Instance.Find(pThis)->IsSelected = UnlimboDetonateFireTemp::InSelected;
			ScenarioExtData::Instance()->LimboLaunchers.emplace(pThis);
		}

		pBullet->Owner = pThis;
	}

	return 0;
}

ASMJIT_PATCH(0x6FE3E3, TechnoClass_FireAt_OccupyDamageBonus, 0xA) //B
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_STACK(int, nDamage, 0x2C);
	GET_BASE(int, weapon_idx, 0xC);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	auto pExtType = GET_TECHNOTYPEEXT(pThis);

	if (pThis->CanOccupyFire())
	{
		if (auto const Building = cast_to<BuildingClass*, false>(pThis))
		{
			nDamage = int(nDamage * BuildingTypeExtContainer::Instance.Find(Building->Type)->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier));
		}
		else
		{
			nDamage = int(nDamage * RulesClass::Instance->OccupyDamageMultiplier);
		}
	}

	if (pThis->InOpenToppedTransport)
	{
		nDamage = int(nDamage * pExtType->OpenTransport_DamageMultiplier);

		if (auto const  pTransport = pThis->Transporter)
		{
			float nDamageMult = GET_TECHNOTYPEEXT(pTransport)->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
			nDamage = int(nDamage * nDamageMult);
		}
		else
		{
			nDamage = int(nDamage * RulesClass::Instance->OpenToppedDamageMultiplier);
		}
	}

	if (!pWeapon->DiskLaser)
	{
		R->EDI(nDamage);
		R->Stack(0x2C, nDamage);
		return 0x6FE4F6; // continue check
	}

	auto pDiskLaser = GameCreate<DiskLaserClass>();

	++pThis->CurrentBurstIndex;
	int rearm = pThis->GetROF(weapon_idx);
	TechnoExtData::SetChargeTurretDelay(pThis, rearm, pWeapon);
	pThis->RearmTimer.Start(rearm);
	pThis->CurrentBurstIndex %= pWeapon->Burst;
	((FakeDiskLaserClass*)pDiskLaser)->__Fire(pThis, pTarget, pWeapon, nDamage);
	//pDiskLaser->Fire(pThis, pTarget, pWeapon, nDamage);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	//this may crash the game , since the object got deleted after killself ,..
	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
		TechnoExtData::KillSelf(pThis, KillMethod::Explode);

	return 0x6FE4E7; //end of func
}

ASMJIT_PATCH(0x6FDFA8, TechnoClass_FireAt_SprayOffsets, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	LEA_STACK(CoordStruct*, pCoord, 0xB0 - 0x28);

	auto pType = GET_TECHNOTYPE(pThis);
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pType->SprayAttack)
	{
		if (pThis->CurrentBurstIndex)
		{
			pThis->SprayOffsetIndex = (pExt->SprayOffsets.size() / pWeapon->Burst + pThis->SprayOffsetIndex) % pExt->SprayOffsets.size();
		}
		else
		{
			pThis->SprayOffsetIndex = ScenarioClass::Instance->Random.RandomRanged(0, pExt->SprayOffsets.size() - 1);
		}

		auto& Coord = pExt->SprayOffsets[pThis->SprayOffsetIndex];
		pCoord->X = (pThis->Location.X + Coord->X);//X
		pCoord->Y = (pThis->Location.Y + Coord->Y);//Y
		R->EAX(pThis->Location.Z + Coord->Z); //Z
		return 0x6FE218;
	}

	return 0x6FE140;
}

ASMJIT_PATCH(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

ASMJIT_PATCH(0x6FF031, TechnoClass_FireAt_ReverseVelocityWhileGravityIsZero, 0xA)
{
	GET(BulletClass*, pBullet, EBX);
	//GET(TechnoClass*, pThis, ESI);

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (pBulletExt->Trajectory &&
		pBulletExt->Trajectory->Flag != TrajectoryFlag::Invalid)
		return 0x0;

	if (pBullet->Type->Arcing && pBulletTypeExt->GetAdjustedGravity() == 0.0)
	{
		pBullet->Velocity *= -1;
		if (pBulletTypeExt->Gravity_HeightFix)
		{
			const auto speed = pBullet->Velocity.Length();

			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
			pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);

			const auto magnitude = pBullet->Velocity.Length();
			pBullet->Velocity *= speed / magnitude;
		}
	}

	return 0;
}

namespace FireAtTemp
{
	BulletClass* FireBullet = nullptr;
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
	AbstractClass* pWaveOwnerTarget = nullptr;
}

ASMJIT_PATCH(0x6FF08B, TechnoClass_FireAt_RecordBullet, 0x6)
{
	GET(BulletClass*, pBullet, EBX);
	FireAtTemp::FireBullet = pBullet;
	return 0;
}

ASMJIT_PATCH(0x6FF15F, TechnoClass_FireAt_Additionals_Start, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);

	REF_STACK(CoordStruct, crdSrc, 0xB0 - 0x6C);
	REF_STACK(CoordStruct, crdTgt, 0xB0 - 0x28);
	REF_STACK(CoordStruct, railgunCrrd_1, 0xB0 - 0x1C);

	GET_BASE(AbstractClass*, pOriginalTarget, 0x8);

	GET_BASE(int, weaponIdx, 0xC);

	auto coords = pOriginalTarget->GetCenterCoords();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pBuilding = cast_to<BuildingClass*, false>(pOriginalTarget))
		coords = pBuilding->GetTargetCoords();

	// This is set to a temp variable as well, as accessing it everywhere needed from TechnoExt would be more complicated.
	FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(crdSrc, coords, pWeapon->Projectile, pThis->Owner);
	pExt->FiringObstacleCell = FireAtTemp::pObstacleCell;

	R->Stack(0x10, &crdSrc);

	if (pWeapon->UseFireParticles && !pThis->Sys.Fire && pWeapon->AttachedParticleSystem)
	{
		pThis->Sys.Fire = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->UseSparkParticles && !pThis->Sys.Spark && pWeapon->AttachedParticleSystem)
	{
		pThis->Sys.Spark = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->AttachedParticleSystem && (pWeapon->_GetExtData()->IsDetachedRailgun || (pWeapon->IsRailgun && !pThis->Sys.Railgun)))
	{
		auto coord = pThis->DealthParticleDamage(&railgunCrrd_1, &crdSrc, pOriginalTarget, pWeapon);
		auto pRailgun = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, &crdSrc,
			nullptr, pThis, coord, nullptr);

		if (!pWeapon->_GetExtData()->IsDetachedRailgun)
			pThis->Sys.Railgun = pRailgun;
	}

	++pThis->CurrentBurstIndex;
	int ROF = pThis->GetROF(weaponIdx);

	if (pThis->Berzerk)
	{
		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
		const double multiplier = pTypeExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
		ROF = static_cast<int>(ROF * multiplier);
	}

	TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);

	pThis->RearmTimer.Start(ROF);

	// Issue #46: Laser is mirrored relative to FireFLH
	// Author: Starkku
	--pThis->CurrentBurstIndex;

	AnimTypeClass* pFiringAnim = nullptr;
	auto pWeaponExt = pWeapon->_GetExtData();

	if (pThis->CanOccupyFire())
	{
		if (pWeaponExt->OccupantAnim_UseMultiple.Get() && !pWeaponExt->OccupantAnims.empty())
		{
			if (pWeaponExt->OccupantAnims.size() == 1)
			{
				pFiringAnim = pWeaponExt->OccupantAnims[0];
			}
			else
			{
				pFiringAnim = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
			}

		}
		else
		{
			pFiringAnim = pWeapon->OccupantAnim;
		}

	}
	else
	{

		if (pWeapon->Anim.Count > 0)
		{
			int nIdx = -1;

			if (pWeapon->Anim.Count == 1)
				nIdx = 0;
			else
			{

				DirStruct facing {};
				pThis->GetRealFacing(&facing);

				if (pWeapon->Anim.Count == 8)
				{
					nIdx = (facing.GetFacing<8>() + 8 / 8) % 8;
				}
				else if (pWeapon->Anim.Count == 16)
				{
					nIdx = (facing.GetFacing<16>() + 16 / 8) % 16;
				}
				else if (pWeapon->Anim.Count == 32)
				{
					nIdx = (facing.GetFacing<32>() + 32 / 8) % 32;
				}
				else if (pWeapon->Anim.Count == 64)
				{
					nIdx = (facing.GetFacing<64>() + 64 / 8) % 64;
				}
				else
				{
					//only execute if the anim count is more than 1
					const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

					// 2^highest is the frame count, 3 means 8 frames
					if (highest >= 3)
					{
						nIdx = facing.GetValue(highest, 1u << (highest - 3));
					}

					nIdx %= pWeapon->Anim.Count;
				}
			}

			pFiringAnim = pWeapon->Anim.Items[nIdx];
		}
	}

	if (pWeapon->Report.Count > 0 && !GET_TECHNOTYPE(pThis)->IsGattling)
	{
		if (pWeapon->Report.Count == 1)
		{
			VocClass::SafeImmedietelyPlayAt(pWeapon->Report[0], &crdSrc, nullptr);
		}
		else
		{
			auto v116 = pThis->weapon_sound_randomnumber_3C8 % pWeapon->Report.Count;
			VocClass::SafeImmedietelyPlayAt(pWeapon->Report[v116], &crdSrc, nullptr);
		}
	}

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
	{
		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? crdSrc : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
		{
			auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord);
			AnimExtData::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false, false);

			if (pThis->WhatAmI() != BuildingClass::AbsID)
			{
				pFeedBackAnim->SetOwnerObject(pThis);
			}
			else
			{
				if (pThis->GetOccupantCount() > 0)
				{
					pFeedBackAnim->ZAdjust = -200;
				}
				else
				{
					auto rend = pThis->GetRenderCoords();
					pFeedBackAnim->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
				}
			}
		}
	}

	if (!pThis->IsAlive)
	{
		return 0x6FF92F;
	}

	if (pFiringAnim)
	{
		auto pFiring = GameCreate<AnimClass>(pFiringAnim, crdSrc, 0, 1, AnimFlag::AnimFlag_600, 0, 0);
		AnimExtData::SetAnimOwnerHouseKind(pFiring, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false, false);
		auto pAnimExt = AnimExtContainer::Instance.Find(pFiring);

		if (pWeapon->_GetExtData()->Anim_Update.Get(RulesExtData::Instance()->FiringAnim_Update))
		{
			pAnimExt->FromWeapon = pWeapon;
			pAnimExt->FromWeaponIdx = weaponIdx;
			pAnimExt->FromBurstIdx = pThis->CurrentBurstIndex;
		}
		// if (pThis->WhatAmI() != BuildingClass::AbsID)
		// {
		// 	pFiring->SetOwnerObject(pThis);
		// } else
		{
			if (pThis->GetOccupantCount() > 0)
			{
				pFiring->ZAdjust = -200;
			}
			else
			{
				auto rend = pThis->GetRenderCoords();
				pFiring->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
			}
		}
	}

	if (!pThis->IsAlive)
	{
		return 0x6FF92F;
	}

#ifndef PERFORMANCE_HEAVY
	//TargetSet
	FireAtTemp::originalTargetCoords = crdTgt;
	FireAtTemp::pOriginalTarget = pOriginalTarget;

	if (FireAtTemp::pObstacleCell)
	{
		crdTgt = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
		R->Base(8, FireAtTemp::pObstacleCell); // Replace original target so it gets used by Ares sonic wave stuff etc. as well.
		pOriginalTarget = FireAtTemp::pObstacleCell;
	}
#endif

	//FeedbackWeapon
	if (auto fbWeapon = pWeaponExt->FeedbackWeapon.Get())
	{
		if (!pThis->InOpenToppedTransport || fbWeapon->FireInTransport)
		{
			WeaponTypeExtData::DetonateAt1(fbWeapon, pThis, pThis, true, nullptr);
			//pThis techno was die after after getting affect of FeedbackWeapon
			//if the function not bail out , it will crash the game because the vtable is already invalid
			if (!pThis->IsAlive)
			{
				return 0x6FF92F;
			}
		}
	}

	if (pExt->AE.flags.HasFeedbackWeapon)
	{
		for (auto const& pAE : pExt->PhobosAE)
		{

			if (!pAE || !pAE->IsActive())
				continue;

			if (auto const pWeaponFeedback = pAE->GetType()->FeedbackWeapon)
			{
				if (pThis->InOpenToppedTransport && !pWeaponFeedback->FireInTransport)
					return 0;

				WeaponTypeExtData::DetonateAt1(pWeaponFeedback, pThis, pThis, true, nullptr);
			}
		}

		//pThis techno was die after after getting affect of FeedbackWeapon
		//if the function not bail out , it will crash the game because the vtable is already invalid
		if (!pThis->IsAlive)
		{
			return 0x6FF92F;
		}
	}

	// if(pWeapon->IsSonic){
	// 	pThis->Wave = WaveExtData::Create(crdSrc, crdTgt, pThis, WaveType::Sonic, pOriginalTarget, pWeapon);
	// }

	return 0x6FF48A;
}

ASMJIT_PATCH(0x6FF5F5, TechnoClass_FireAt_Additionals_End, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass*, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));
	GET_BASE(int, weaponIndex, 0xC);
	REF_STACK(CoordStruct const, crdSrc, 0x44);
	REF_STACK(CoordStruct, pTargetCoords, 0x88);

	auto const pData = WeaponTypeExtContainer::Instance.Find(pWeaponType);
	//TechnoClass_Fire_OtherWaves
	if (pData->IsWave() && !pThis->Wave)
	{
		WaveType nType = WaveType::Sonic;
		if (pWeaponType->IsMagBeam)
			nType = WaveType::Magnetron;
		else
			nType = pData->Wave_IsBigLaser
			? WaveType::BigLaser : WaveType::Laser;

		pThis->Wave = WaveExtData::Create(crdSrc, pTargetCoords, pThis, nType, pTarget, pWeaponType);
	}

	//remove ammo rounds depending on weapon
	TechnoExtData::DecreaseAmmo(pThis, pWeaponType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	auto const pTypeExt = GET_TECHNOTYPEEXT(pThis);

#ifndef PERFORMANCE_HEAVY
	// Restore original target & coords
	pTargetCoords = FireAtTemp::originalTargetCoords;
	R->Base(8, FireAtTemp::pOriginalTarget);
	pTarget = FireAtTemp::pOriginalTarget;
	R->EDI(FireAtTemp::pOriginalTarget);

	// Reset temp values
	FireAtTemp::originalTargetCoords = CoordStruct::Empty;
	FireAtTemp::FireBullet = nullptr;
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;
#endif

	//TechnoClass_FireAt_ToggleLaserWeaponIndex
	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeaponType->IsLaser)
	{
		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	//TechnoClass_FireAt_BurstOffsetFix_2
	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pWeaponType->Burst;

	if (pExt->ForceFullRearmDelay)
	{
		pExt->ForceFullRearmDelay = false;
		pThis->CurrentBurstIndex = 0;
	}

	if (auto const pTargetObject = cast_to<BulletClass* const, false>(pTarget))
	{
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor())
		{
			auto pBulletTargetExt = BulletExtContainer::Instance.Find(pTargetObject);
			auto pBulletTypeTargetExt = BulletTypeExtContainer::Instance.Find(pTargetObject->Type);

			if (!pBulletTypeTargetExt->Armor.isset())
				pBulletTargetExt->InterceptedStatus |= InterceptedStatus::Locked;

			const auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

			pBulletExt->InterceptorTechnoType = GET_TECHNOTYPE(pThis);
			pBulletExt->InterceptedStatus |= InterceptedStatus::Targeted;

			if (!pTypeExt->Interceptor_ApplyFirepowerMult)
				pBullet->Health = pWeaponType->Damage;
		}
	}

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);

	if (!Phobos::Config::HideShakeEffects)
	{
		if (!pWeaponExt->ShakeLocal.Get() || pThis->IsOnMyView())
		{
			if (pWeaponExt->Xhi || pWeaponExt->Xlo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

			if (pWeaponExt->Yhi || pWeaponExt->Ylo)
				GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));
		}
	}
	return 0x6FF660;
}

ASMJIT_PATCH(0x6FF48D, TechnoClass_FireAt_IsLaser, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EDI);
	GET(WeaponTypeClass* const, pFiringWeaponType, EBX);
	GET_BASE(int, idxWeapon, 0xC);// don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	auto pType = GET_TECHNOTYPE(pThis);
	if (pType->TargetLaser && pThis->Owner->ControlledByCurrentPlayer())
	{

		const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

		if (pTypeExt->TargetLaser_WeaponIdx.empty()
			|| pTypeExt->TargetLaser_WeaponIdx.Contains(idxWeapon))
		{
			pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
		}
	}

	if (pFiringWeaponType->IsLaser)
	{
		auto const pData = WeaponTypeExtContainer::Instance.Find(pFiringWeaponType);
		int const Thickness = pData->Laser_Thickness;

		if (auto const pBld = cast_to<BuildingClass*, false>(pThis))
		{	//ToggleLaserWeaponIndex

			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = idxWeapon;
			else
				pExt->CurrentLaserWeaponIndex.clear();

			auto const pTWeapon = pBld->GetPrimaryWeapon()->WeaponType;

			if (auto const pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty))
			{

				//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
				pLaser->Thickness = Thickness == -1 ? 3 : Thickness;
				auto const pBldTypeData = BuildingTypeExtContainer::Instance.Find(pBld->Type);

				if (pBldTypeData->PrismForwarding.CanAttack())
				{
					//is a prism tower

					if (pBld->SupportingPrisms > 0)
					{ //Ares sets this to the longest backward chain
						//is being supported... so increase beam intensity
						if (pBldTypeData->PrismForwarding.Intensity < 0)
						{
							pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
						}
						else if (pBldTypeData->PrismForwarding.Intensity > 0)
						{
							pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
						}

						// always supporting
						pLaser->IsSupported = true;
					}
				}
			}
		}
		else
		{
			if (auto const pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, CoordStruct::Empty))
			{
				if (Thickness == -1)
				{
					pLaser->Thickness = 2;
				}
				else
				{
					pLaser->Thickness = Thickness;

					// required for larger Thickness to work right
					pLaser->IsSupported = (Thickness > 3);
				}
			}
		}

		// skip all default handling
		return 0x6FF656;
	}

	//other affects
	return 0x6FF57D;
}

ASMJIT_PATCH(0x6FF008, TechnoClass_FireAt_FSW, 8)
{
	REF_STACK(CoordStruct const, src, 0x44);
	REF_STACK(CoordStruct const, tgt, 0x88);

	const DWORD origin = R->Origin();
	auto const Bullet = origin == 0x6FF860
		? R->EDI<FakeBulletClass*>()
		: R->EBX<FakeBulletClass*>()
		;

	if (origin != 0x6FF860)
	{

		GET_STACK(CoordStruct, crdOffset, STACK_OFFSET(0xB0, -0x1C));
		GET_STACK(CoordStruct, fireCoords, STACK_OFFSET(0xB0, -0x6C));

		const auto crdTgt = crdOffset + fireCoords;
		if (Bullet->Type->Arcing && !Bullet->_GetTypeExtData()->Arcing_AllowElevationInaccuracy)
		{
			REF_STACK(VelocityClass, velocity, STACK_OFFSET(0xB0, -0x60));
			REF_STACK(CoordStruct, crdSrc, STACK_OFFSET(0xB0, -0x6C));

			Bullet->_GetExtData()->ApplyArcingFix(crdSrc, crdTgt, velocity);
		}
	}

	if (!HouseExtContainer::Instance.IsAnyFirestormActive || !Bullet->Type->IgnoresFirestorm)
	{
		return 0;
	}

	auto const crd = MapClass::Instance->FindFirstFirestorm(src, tgt, Bullet->Owner->Owner);

	if (crd.IsValid())
	{
		Bullet->Target = MapClass::Instance->GetCellAt(crd)->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
	}

	return 0;
}ASMJIT_PATCH_AGAIN(0x6FF860, TechnoClass_FireAt_FSW, 8)

ASMJIT_PATCH(0x6FF923, TechnoClass_FireAt_FireOnce_B, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	pThis->SetTarget(nullptr);
	if (auto pUnit = cast_to<UnitClass*, false>(pThis))
	{
		if (pUnit->Type->DeployFire
			&& !pUnit->Type->IsSimpleDeployer
			&& !pUnit->Deployed
			&& pThis->CurrentMission == Mission::Unload
		)
		{
			TechnoExtContainer::Instance.Find(pUnit)->DeployFireTimer.Start(pWeapon->ROF);
		}
	}

	return 0x6FF92F;
}

ASMJIT_PATCH(0x6FE31C, TechnoClass_FireAt_AllowDamage, 8)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	// whether conventional damage should be used
	const bool applyDamage =
		WeaponTypeExtContainer::Instance.Find(pWeapon)->ApplyDamage.Get(!pWeapon->IsSonic && !pWeapon->UseFireParticles);

	if (!applyDamage)
	{
		// clear damage
		R->EDI(0);
		return 0x6FE3DFu;
	}

	return 0x6FE32Fu;
}

ASMJIT_PATCH(0x6FE709, TechnoClass_FireAt_BallisticScatter1, 6)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for FlakScatter && !Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(0));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE71C;
}

ASMJIT_PATCH(0x6FE7FE, TechnoClass_FireAt_BallisticScatter2, 5)
{
	GET_STACK(BulletTypeClass*, pProjectile, 0x68);
	auto pExt = BulletTypeExtContainer::Instance.Find(pProjectile);

	// defaults for !FlakScatter || Inviso
	int min = pExt->BallisticScatterMin.Get(Leptons(RulesClass::Instance->BallisticScatter / 2));
	int max = pExt->BallisticScatterMax.Get(Leptons(RulesClass::Instance->BallisticScatter));
	int scatter = ScenarioClass::Instance->Random.RandomRanged(min, max);

	R->EAX(scatter);
	return 0x6FE821;
}

#endif

//=======================================================================================================================
// An example for quick tilting test
ASMJIT_PATCH(0x7413DD, UnitClass_FireAt_RecoilForce, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	if (!pThis->IsVoxel())
		return 0;

	GET(BulletClass* const, pTraj, EDI);

	const auto& force = WeaponTypeExtContainer::Instance.Find(pTraj->WeaponType)->RecoilForce;

	if (!force.isset() || Math::abs(force.Get()) < 0.005)
		return 0x0;

	double force_result = force / MaxImpl(pThis->Type->Weight, 1.);

	if (Math::abs(force.Get()) < 0.002)
		return 0;

	const double theta = pThis->GetRealFacing().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>();

	pThis->RockingForwardsPerFrame = (float)(-force_result * Math::cos(theta));
	pThis->RockingSidewaysPerFrame = (float)(force_result * Math::sin(theta) * Math::pow(pThis->Type->VoxelScaleX / pThis->Type->VoxelScaleY, 2.0));

	return 0;
}

ASMJIT_PATCH(0x51B20E, InfantryClass_AssignTarget_FireOnce, 0x6)
{
	enum { SkipGameCode = 0x51B255 };

	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);

	auto const pExt = InfantryExtContainer::Instance.Find(pThis);

	if (!pTarget && pExt->SkipTargetChangeResetSequence)
	{
		pThis->IsFiring = false;
		pExt->SkipTargetChangeResetSequence = false;
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x772AA2, WeaponTypeClass_AllowedThreats_AAOnly, 0x5)
{
	GET(BulletTypeClass* const, pType, ECX);

	if (BulletTypeExtContainer::Instance.Find(pType)->AAOnly) {
		R->EAX(4);
		return 0x772AB3;
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
ASMJIT_PATCH(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = GET_TECHNOTYPE(pThis);

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);

		if (pThisType->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = GET_TECHNOTYPE(pPassenger)->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else //if (pPassenger->HasTurret())
					//tWeaponIndex = pPassenger->CurrentWeaponNumber;
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				const auto pTWeapon = pPassenger->GetWeapon(tWeaponIndex);

				if (pTWeapon->WeaponType && pTWeapon->WeaponType->FireInTransport)
				{
					int TWeaponRange = WeaponTypeExtData::GetRangeWithModifiers(pTWeapon->WeaponType, pPassenger);

					if (TWeaponRange < smallestRange) {
						smallestRange = TWeaponRange;
					}
				}

				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	if (result == 0 && pThis->WhatAmI() == AircraftClass::AbsID && pThisType->OpenTopped)
	{
		result = pThisType->GuardRange;
		if (result == 0)
			Debug::LogInfo("Warning ! , range of Aircraft[{}] return 0 result will cause Aircraft to stuck ! ", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

#pragma region WallWeaponStuff

ASMJIT_PATCH(0x51C1F1, InfantryClass_CanEnterCell_WallWeapon, 0x5)
{
	enum { SkipGameCode = 0x51C1FE };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x73F495, UnitClass_CanEnterCell_WallWeapon, 0x6)
{
	enum { SkipGameCode = 0x73F4A1 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x70095A, TechnoClass_WhatAction_WallWeapon, 0x6) {
	GET_STACK(OverlayTypeClass*, pOverlayTypeClass, STACK_OFFSET(0x2C, -0x18));
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));
	return 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F8C10, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9572, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F971F, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9904, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9AB8, FakeTechnoClass::_EvaluateJustCell);
#pragma endregion

ASMJIT_PATCH(0x6FCDD2, TechnoClass_AssignTarget_Changed, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pNewTarget, EDI);

	if (!pNewTarget)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		pExt->ResetDelayedFireTimer();
	}

	return 0;
}

//===========================================================================================================================
// https://github.com/Phobos-developers/Phobos/pull/825
// Todo :  Otamaa : massive FPS drops !
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Adjust target coordinates for laser drawing.
ASMJIT_PATCH(0x6FD38D, TechnoClass_DrawSth_Coords, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	const auto pBullet = FireAtTemp::FireBullet;

	if (pBullet) {
		*pTargetCoords = BulletExtData::GetTargetCoords(pBullet);

	} else if (FireAtTemp::pObstacleCell) {
		*pTargetCoords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
	}

	R->EAX(pTargetCoords);
	return 0;
}
ASMJIT_PATCH_AGAIN(0x6FD514, TechnoClass_DrawSth_Coords, 0x7) // CreateEBolt

// Cut railgun logic off at obstacle coordinates.
ASMJIT_PATCH(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { Continue = 0x70CA79, Stop = 0x70CAD8 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	if (MapClass::Instance->GetCellAt(coords) == FireAtTemp::pObstacleCell)
		return Stop;

	return Continue;
}

// WaveClass requires the firer's target and wave's target to match so it needs bit of extra handling here for obstacle cell targets.
ASMJIT_PATCH(0x762AFF, WaveClass_WaveAI_TargetSet, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (pThis->Target && pThis->Owner)
	{
		auto const pObstacleCell = TechnoExtContainer::Instance.Find(pThis->Owner)->FiringObstacleCell;

		if (pObstacleCell == pThis->Target && pThis->Owner->Target)
		{
			FireAtTemp::pWaveOwnerTarget = pThis->Owner->Target;
			pThis->Owner->Target = pThis->Target;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x762D57, WaveClass_Wave_AI_TargetUnset, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (FireAtTemp::pWaveOwnerTarget)
	{
		if (pThis->Owner->Target)
			pThis->Owner->Target = FireAtTemp::pWaveOwnerTarget;

		FireAtTemp::pWaveOwnerTarget = nullptr;
	}

	return 0;
}
//===========================================================================================================================
