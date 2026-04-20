#include "Body.h"
#include "Trajectories/PhobosTrajectory.h"

#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Rules/Body.h>


#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

// missile_maths @ 0x5B20F0
//
// External symbols referenced (cross-reference with IDA):
//   stru_ABEF10  → CoordStruct empty sentinel
//   dword_ABEF44 → bridge Z addend       (UNRESOLVED — see VERIFY below)
//   dword_ABEF50 → leptons per cruise lvl (UNRESOLVED — see VERIFY below)
//   MouseClass Map @ 0x87F7E8 → global Map instance

int __fastcall missile_maths(
	CoordStruct* pCoordTo,
	VelocityClass* pVeloc,
	CoordStruct* pCoordFrom,
	DirStruct* pInputDir,
	bool           inair,
	bool           airburst,
	bool           veryhigh,
	bool           level)
{
	// ── BRANCH A: ballistic-only (pCoordFrom == empty sentinel) ───────
	// ORIG: cmp against stru_ABEF10.{X,Y,Z} @ 0x5B2106-0x5B2134
	if (!pCoordFrom->IsValid())   // ORIG: stru_ABEF10
	{
		const DirStruct currentPitch = pVeloc->GetDirectionFromXY();

		DirStruct newPitch = currentPitch;
		DirStruct pitchTarget { 0x2000 };    // ORIG: literal 0x2000 @ 0x5B2182
		newPitch.Func_5B29C0(pitchTarget, *pInputDir);

		const double totalSpeed = pVeloc->Length();
		const double currentPitchRad = currentPitch.GetRadian<65536>();

		if (currentPitchRad != 0.0)          // ORIG: fcomp FLOAT_0_0 @ 0x5B21FF
		{
			const double c = Math::cos(currentPitchRad);
			pVeloc->X /= c;
			pVeloc->Y /= c;
		}

		const double newPitchRad = newPitch.GetRadian<65536>();
		const double cosNew = Math::cos(newPitchRad);
		const double sinNew = Math::sin(newPitchRad);

		pVeloc->X *= cosNew;
		pVeloc->Y *= cosNew;
		pVeloc->Z = sinNew * totalSpeed;

		pCoordTo->X += static_cast<int>(pVeloc->X);
		pCoordTo->Y += static_cast<int>(pVeloc->Y);
		pCoordTo->Z += static_cast<int>(pVeloc->Z);

		return static_cast<int>(pVeloc->Length());
	}

	// ── BRANCH B: homing toward pCoordFrom ─────────────────────────────
	pCoordTo->X += static_cast<int>(pVeloc->X);
	pCoordTo->Y += static_cast<int>(pVeloc->Y);
	pCoordTo->Z += static_cast<int>(pVeloc->Z);

	const int distance3D = static_cast<int>((*pCoordTo - *pCoordFrom).Length());

	const Point2D pTo2D { pCoordTo->X,   pCoordTo->Y };
	const Point2D pFrom2D { pCoordFrom->X, pCoordFrom->Y };
	const int distance2D = (pTo2D - pFrom2D).Length();

	CoordStruct deltaToTarget {
		pCoordFrom->X - pCoordTo->X,
		pCoordFrom->Y - pCoordTo->Y,
		pCoordFrom->Z - pCoordTo->Z
	};

	VelocityClass vToTarget {
		static_cast<double>(deltaToTarget.X),
		static_cast<double>(deltaToTarget.Y),
		static_cast<double>(deltaToTarget.Z)
	};

	// ── Yaw correction ────────────────────────────────────────────────
	DirStruct currentHeading { -pVeloc->Y,    pVeloc->X };
	DirStruct targetHeading { -vToTarget.Y, vToTarget.X };

	DirStruct newHeading = currentHeading;
	newHeading.Func_5B29C0(targetHeading, *pInputDir);

	pVeloc->SetIfZeroXY();
	const double horizontalSpeed = pVeloc->LengthXY();
	const double newHeadingRad = newHeading.GetRadian<65536>();

	pVeloc->X = Math::cos(newHeadingRad) * horizontalSpeed;
	pVeloc->Y = -Math::sin(newHeadingRad) * horizontalSpeed;

	DirStruct newPitch = pVeloc->GetDirectionFromXY();

	// ── Lookahead skip test ───────────────────────────────────────────
	// ORIG @ 0x5B2604-0x5B2638:
	//   inair                                            (bool_1)
	//   OR (!airburst && a3.X <= (veryhigh?6:3)<<8)      (bool_2, bool_4)
	//   OR (((*pInputDir>>7)+1)>>1) <= 1                 — Pattern 9
	const int  closeThreshold = (veryhigh ? 6 : 3) << 8;          // ORIG: literals 6/3 @ 0x5B2619, shl 8 @ 0x5B261C
	const bool tinyTurnRate = pInputDir->GetFacing<256>() <= 1; // ORIG: shr 7, +1, shr 1, cmp 1 @ 0x5B2629-0x5B2635
	const bool skipLookahead = inair
		|| (!airburst && distance2D <= closeThreshold)
		|| tinyTurnRate;

	if (skipLookahead)
	{
		if (!level)
		{
			DirStruct halfStep = *pInputDir;
			halfStep += DirStruct { 0x100 }; // ORIG: literal 0x100 @ 0x5B28B6 (minimum turn floor)
			halfStep /= 2;

			DirStruct pitchToTarget = vToTarget.GetDirectionFromXY();
			newPitch.Func_5B29C0(pitchToTarget, halfStep);
		}
	}
	else
	{
		// ── Cruise lookahead ──────────────────────────────────────────
		// ORIG @ 0x5B267B-0x5B2690: 6 * vx, 6 * vy, 6 * vz via lea+shl
		const CoordStruct futurePos {
			pCoordTo->X + 6 * static_cast<int>(pVeloc->X),     // ORIG: literal 6 (lookahead ticks)
			pCoordTo->Y + 6 * static_cast<int>(pVeloc->Y),
			pCoordTo->Z + 6 * static_cast<int>(pVeloc->Z)
		};

		// ORIG: MouseClass Map @ 0x87F7E8, MapClass__Get_Z_Pos call @ 0x5B26DC
		int terrainZ = MapClass::Instance->GetCellFloorHeight(futurePos);

		// ORIG: MapClass::operator[] @ 0x5B26ED, test ch,1 on +0x140 byte
		CellClass* pCell = MapClass::Instance->GetCellAt(futurePos);
		if (pCell->ContainsBridgeHead())
		{
			// ─────────────────────────────────────────────────────────
			// !!! UNRESOLVED ENGINE GLOBAL #1 !!!
			// ORIG: dword_ABEF44 @ 0x5B26FD
			//   add edi, dword_ABEF44
			// Bridge Z addend. I named it BridgeHeight as a placeholder.
			// VERIFY: replace with the real symbol from your tree, or
			// import via #include & extern decl.
			// ─────────────────────────────────────────────────────────
			terrainZ += CellClass::BridgeHeight; // ORIG: dword_ABEF44
		}

		// ORIG @ 0x5B2703-0x5B272D: cdq/sar 8 = signed div by 256
		int cruiseLevel = distance3D / 256;
		if (airburst || veryhigh)  cruiseLevel = 10;  // ORIG: literal 10 @ 0x5B272D
		else if (cruiseLevel >= 5) cruiseLevel = 5;   // ORIG: literal 5  @ 0x5B2721, 0x5B2726

		if (!level)
		{
			// ─────────────────────────────────────────────────────────
			// !!! UNRESOLVED ENGINE GLOBAL #2 !!!
			// ORIG: dword_ABEF50 @ 0x5B273D, 0x5B2762, 0x5B27F1
			//   imul eax, dword_ABEF50          ← used as multiplier
			//   mov  edi, dword_ABEF50          ← then /2 for half-band
			// Leptons per cruise level. Used in THREE places below.
			// I named it LEPTONS_PER_LEVEL as a placeholder. Likely
			// candidates: Game::CellHeightInLeptons (=96 in vanilla),
			// RulesClass::Instance->MissileCruiseStep, or similar.
			// VERIFY: replace with the real symbol.
			// ─────────────────────────────────────────────────────────
			const int LEPTONS_PER_LEVEL = Unsorted::LevelHeight; // ORIG: dword_ABEF50

			const int currentZ = pCoordTo->Z;
			// ORIG: imul eax,dword_ABEF50 @ 0x5B273D
			const int aboveCruiseBy = currentZ
				- LEPTONS_PER_LEVEL * cruiseLevel
				- terrainZ;

			// Z dead-zone snap.  ORIG: cmp ecx,0FFFFFFECh / cmp 14h @ 0x5B274D-0x5B275A
			if (aboveCruiseBy < -20)        // ORIG: -20 (0xFFFFFFEC)
				pCoordTo->Z = currentZ + 18; // ORIG: +0x12 @ 0x5B2752
			else if (aboveCruiseBy > 20)     // ORIG: +20 (0x14)
				pCoordTo->Z = currentZ - 18; // ORIG: -0x12 (0xFFFFFFEE) @ 0x5B275C

			DirStruct halfStep = *pInputDir;
			halfStep /= 2;

			// ORIG @ 0x5B2762-0x5B276F: edi=dword_ABEF50; eax=edi; cdq;
			//   sub eax,edx; sar eax,1; neg eax → -(dword_ABEF50/2 rounded)
			// Then cmp v73,eax (jge → upper branch)
			if (aboveCruiseBy < -LEPTONS_PER_LEVEL / 2) // ORIG: dword_ABEF50/-2 @ 0x5B2762-0x5B276F
			{
				DirStruct pitchUp { 0x2000 };           // ORIG: literal 0x2000 @ 0x5B2778
				newPitch.Func_5B29C0(pitchUp, halfStep);
			}
			// ORIG @ 0x5B27F1-0x5B27F8: same dword_ABEF50/2 but positive
			else if (aboveCruiseBy > LEPTONS_PER_LEVEL / 2) // ORIG: dword_ABEF50/2 @ 0x5B27F1
			{
				DirStruct pitchDown { 0x4800 };         // ORIG: literal 0x4800 @ 0x5B27FF
				newPitch.Func_5B29C0(pitchDown, halfStep);
			}
			else
			{
				DirStruct pitchLevel { 0x4000 };        // ORIG: literal 0x4000 @ 0x5B2875
				newPitch.Func_5B29C0(pitchLevel, halfStep);
			}
		}
	}

	// ORIG: VelocityStruct_5B2A30 @ 0x5B28EC
	pVeloc->Func_5B2A30(&newPitch);

	// ORIG @ 0x5B28F8-0x5B290F: airburst → Z=0, else sar 2 (signed /4)
	if (airburst)
		deltaToTarget.Z = 0;
	else
		deltaToTarget.Z /= 4;

	// ORIG: CoordStruct__Distance @ 0x5B2917
	return static_cast<int>(deltaToTarget.Length());
}

// ============================================================================
// FakeBulletClass::_AI — integrated backport of BulletClass::AI (0x4666E0)
//
// All Phobos/Ares hooks listed below have been merged into the cleaned
// helpers from the previous pass. Original control flow is preserved.
//
// ============================================================================
// HOOK INTEGRATION TABLE
// ============================================================================
//   Address  Sz   Hook name                                  Helper
//   -------  --   ---------------------------------------    --------------------
//   0x4666F7  6   BulletClass_AI_Trajectories                RunEarlyTickHooks
//   0x466705  6   BulletClass_AI                             RunEarlyTickHooks
//   0x46670F  6   BulletClass_AI_PreImpactAnim               HandleSpawnedAnimFollowup
//   0x466834  6   BulletClass_AI_TrailerAnim                 SpawnTrailerIfDue
//   0x4668BD  6   BulletClass_AI_Interceptor_InvisoSkip      UpdateGuidedMotion (entry)
//   0x466BAF  6   BulletClass_AI_MissileROTVar               UpdateGuidedMotion (homing)
//   0x466E9F  6   BulletClass_AI_MissileSafetyAltitude       UpdateGuidedMotion (alt-safety)
//   0x4671B9  6   BulletClass_AI_ApplyGravity                UpdateBallisticMotion (gravity)
//   0x46745C  7   BulletClass_AI_Position_Trajectories       UpdateBallisticMotion (integrate)
//   0x4677D3  5   BulletClass_AI_TargetCoordCheck_Trajector  CheckUnguidedTargetCell (entry)
//   0x467927  5   BulletClass_AI_TechnoCheck_Trajectories    CheckUnguidedTargetCell (techno hit)
//   0x467AB2  7   BulletClass_AI_Parabomb                    CheckUnguidedTargetCell (velocity copyback)
//   0x467B94  7   BulletClass_AI_Ranged                      FinalizeBulletMotion (Set_Coord)
//   0x467C1C  6   BulletClass_AI_UnknownTimer                FinalizeBulletMotion (degenerate skip)
//   0x467C2E  7   BulletClass_AI_FuseCheck                   FinalizeBulletMotion (fuse)
//   0x467CCA  6   BulletClass_AI_TargetSnapChecks            FinalizeBulletMotion (target snap)
//   0x467E53  6   BulletClass_AI_PreDetonation_Trajectories  RunDetonation (entry)
//   0x467E59  5   BulletClass_AI_NukeBall                    RunDetonation (NUKE/preimpact)
//
// ============================================================================
// HOOK COLLISION ANALYSIS
// ============================================================================
// No byte overlaps detected. Sequential adjacencies (both fire in order):
//
//   1. Top of function (sequential, all three may fire on the same tick):
//        0x4666F7 [6] → 0x466705 [6] → 0x46670F [6]
//      0x4666F7 covers bytes 0x4666F7..0x4666FC (the IsAlive load)
//      0x466705 covers bytes 0x466705..0x46670A (the SpawnNextAnim load)
//      0x46670F covers bytes 0x46670F..0x466714 (the NextAnim load inside the
//                                                 SpawnNextAnim==true branch)
//      4-byte gap between 0x466705 hook and 0x46670F hook holds the
//      `test al,al / jz` that gates the SpawnNextAnim branch — preserved.
//
//   2. Pre-detonation pair (both fire when control reaches the detonation
//      phase, including via the trajectory short-circuit from 0x4666F7):
//        0x467E53 [6] → 0x467E59 [5]
//      0x467E53 covers bytes 0x467E53..0x467E58 (the warhead pointer load)
//      0x467E59 covers bytes 0x467E59..0x467E5D (the "NUKE" string push)
//
// ============================================================================
// POSSIBLE BUGS / VERIFY ITEMS
// ============================================================================
// 1. (carry-over from cleanup pass) Smoothed proximity-fuse boundary in
//    UpdateGuidedMotion uses `>= 0.0 && < 60.0`. The asm at 0x466FE8..0x466FF9
//    confirms this, but worth verifying the >= bound was not intended as > 0.0
//    strictly. The boundary affects the exact tick a stalling missile detonates.
//
// 2. (hook 0x466834) BulletClass_AI_TrailerAnim guards with `if (delay < 0)`
//    only — does not guard against `delay == 0`. If both ScaledSpawnDelay and
//    SpawnDelay are zero, the subsequent `Frame % delay` is a divide-by-zero.
//    The original vanilla code had the same flaw (it picked one branch or the
//    other but never zero-checked). Preserved verbatim.
//
// 3. (hook 0x466705) Reads `pThis->GetOwningHouse()` unconditionally. If
//    a bullet has no Owner and no fallback house in its ext, GetOwningHouse
//    may dereference null depending on its implementation. The hook does
//    null-check `pThis->Owner` before assigning to ext->Owner, but the
//    `pBulletCurOwner` evaluation runs first. Preserved.
//
// 4. (hooks 0x467E53 + 0x4666F7) Trajectory short-circuit interaction:
//    when 0x4666F7 returns Detonate (0x467E53), control jumps directly to
//    the PreDetonation hook. This means OnAIPreDetonate runs even though
//    OnAI never returned the "I handled it" path (it returned the Detonate
//    path explicitly). Confirm this is the intended ordering — i.e., that
//    trajectory plugins expect OnAIPreDetonate after OnAI returns true.
//
// 5. (hook 0x466705) The owner-tracking + LaserTrails block fires once per
//    AI tick INCLUDING ticks where SpawnNextAnim is set. This means trails
//    keep updating during the post-detonation NUKEBALL wait tick. Intentional?
//    The vanilla function had no LaserTrails so we have no baseline.
//
// 6. (hooks 0x4666F7 + 0x466705) Both hooks update LaserTrails — 0x4666F7
//    handles the trajectory-managed case, 0x466705 handles the non-trajectory
//    case (gated by `!pBulletExt->Trajectory || !BlockDrawTrail`). This
//    creates a path where BOTH update if Trajectory exists but BlockDrawTrail
//    returns false. Double-update is likely benign (idempotent Update calls)
//    but worth a closer look.
//
// 7. The original LABEL_157 (CheckUnguidedTargetCell entry) integer cell-coord
//    rounding uses `cdq + and edx, 0xFF + add` which is sign-aware right-shift
//    by 8. For negative coords this rounds toward zero rather than toward
//    -infinity. Preserved as `(coord.X + ((coord.X >> 31) & 0xFF)) >> 8`.
//
// ============================================================================

#include <CellClass.h>
#include <MapClass.h>
#include <RulesClass.h>
#include <FootClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <ScenarioClass.h>
#include <TacticalClass.h>
#include <Matrix3D.h>
#include <Unsorted.h>
#include <Notifications.h>

namespace
{

	// ===========================================================================
	// Per-tick context bundling the four pieces of state that flowed through the
	// original via the v136/v137/v138 stack triplet.
	// ===========================================================================
	struct BulletAITickContext
	{
		bool exploded = false;  // v136[0] in original
		int  markType = 0;      // v138 in original (0/1/2)
		bool snap_to_target = false;  // v137 in original
		bool early_return = false;  // hook returned 0x467FEE
		bool detonate_now = false;  // hook returned 0x467E53 (skip motion)
		bool change_owner = false;  // bullet's owning house changed this tick
	};

	// ---- forward decls --------------------------------------------------------
	OPTIONALINLINE void RunEarlyTickHooks(FakeBulletClass* pThis, BulletAITickContext& ctx);
	OPTIONALINLINE bool HandleSpawnedAnimFollowup(FakeBulletClass* pThis);
	OPTIONALINLINE void AdvanceAnimFrame(BulletClass* pThis);
	OPTIONALINLINE void SpawnTrailerIfDue(FakeBulletClass* pThis, CoordStruct const& coord);
	OPTIONALINLINE void UpdateGuidedMotion(FakeBulletClass* pThis, CoordStruct& coord,
										   BulletAITickContext& ctx);
	OPTIONALINLINE void UpdateVerticalMotion(FakeBulletClass* pThis, CoordStruct& coord,
											  VelocityClass& veloc, BulletAITickContext& ctx);
	OPTIONALINLINE void UpdateBallisticMotion(FakeBulletClass* pThis, CoordStruct& coord,
											   VelocityClass& veloc, BulletAITickContext& ctx);
	OPTIONALINLINE void CheckUnguidedTargetCell(FakeBulletClass* pThis, CoordStruct& coord,
												 VelocityClass& veloc, BulletAITickContext& ctx);
	OPTIONALINLINE void FinalizeBulletMotion(FakeBulletClass* pThis, CoordStruct& coord,
											  BulletAITickContext& ctx);
	OPTIONALINLINE void RunDetonation(FakeBulletClass* pThis, CoordStruct& coord,
									  BulletAITickContext& ctx);
	OPTIONALINLINE void UpdateLastMapCoords(BulletClass* pThis, CoordStruct const& coord);

	// Helper for the sign-aware /256 cell rounding the original asm uses.
	OPTIONALINLINE short CoordToCell(int leptons)
	{
		return static_cast<short>((leptons + ((leptons >> 31) & 0xFF)) >> 8);
	}

	// ===========================================================================
	// RunEarlyTickHooks
	//   Hooks 0x4666F7 (BulletClass_AI_Trajectories)
	//   Hooks 0x466705 (BulletClass_AI)
	//
	// These two hooks fire near the top of the function, before the SpawnNextAnim
	// branch. Together they handle:
	//   - Trajectory plugin dispatch (OnAI, intercepted, IsAlive, parachute)
	//   - Bullet owner tracking
	//   - Bright flag check (one-shot)
	//   - PreExplodeRange detonation trigger
	//   - LaserTrails update (two paths: trajectory-managed and not)
	//
	// The context flags are set as follows:
	//   - early_return  : trajectory says bullet is dead/intercepted
	//   - detonate_now  : trajectory's OnAI returned true (skip motion, detonate)
	//   - change_owner  : firer's house changed this tick (used by LaserTrails)
	// ===========================================================================
	OPTIONALINLINE void RunEarlyTickHooks(FakeBulletClass* pThis, BulletAITickContext& ctx)
	{
		auto pExt = pThis->_GetExtData();
		auto pTypeExt = pThis->_GetTypeExtData();

		// -----------------------------------------------------------------------
		// Hook 0x4666F7 — BulletClass_AI_Trajectories
		// -----------------------------------------------------------------------
		auto& pTraj = pExt->Trajectory;

		if (!pThis->SpawnNextAnim && pTraj)
		{
			if (pTraj->OnAI())
			{
				ctx.detonate_now = true;
				return;
			}
		}

		// Targeted -> Locked status promotion (intercept book-keeping)
		if (pExt->InterceptedStatus & InterceptedStatus::Targeted)
		{
			if (auto pTargetBullet = cast_to<BulletClass*>(pThis->Target))
			{
				auto pTgtTypeExt = BulletTypeExtContainer::Instance.Find(pTargetBullet->Type);
				auto pTgtExt = BulletExtContainer::Instance.Find(pTargetBullet);
				if (!pTgtTypeExt->Armor.isset())
					pTgtExt->InterceptedStatus |= InterceptedStatus::Locked;
			}
		}

		// Intercepted bullets either get removed or unlock their target
		if (pExt->InterceptedStatus & InterceptedStatus::Intercepted)
		{
			if (auto pTargetBullet = cast_to<BulletClass*>(pThis->Target))
				BulletExtContainer::Instance.Find(pTargetBullet)->InterceptedStatus
				&= ~InterceptedStatus::Locked;

			if (BulletExtData::HandleBulletRemove(pThis, pExt->DetonateOnInterception, true))
			{
				ctx.early_return = true;
				return;
			}
		}

		if (!pThis->IsAlive)
		{
			ctx.early_return = true;
			return;
		}

		// Trajectory-path LaserTrails (only when trajectory does NOT block trail draw)
		if (!PhobosTrajectory::BlockDrawTrail(pTraj))
		{
			if (!pExt->LaserTrails.empty())
			{
				CoordStruct futureCoords {
					pThis->Location.X + static_cast<int>(pThis->Velocity.X),
					pThis->Location.Y + static_cast<int>(pThis->Velocity.Y),
					pThis->Location.Z + static_cast<int>(pThis->Velocity.Z)
				};

				for (auto& trail : pExt->LaserTrails)
				{
					if (!trail->LastLocation.isset())
						trail->LastLocation = pThis->Location;
					trail->Update(futureCoords);
				}
			}
		}

		// Parachute fall rate update (parabomb)
		if (pThis->HasParachute)
		{
			int fallRate = pExt->ParabombFallRate - pTypeExt->Parachuted_FallRate;
			int maxFallRate = pTypeExt->Parachuted_MaxFallRate.Get(RulesClass::Instance->ParachuteMaxFallRate);

			if (fallRate < maxFallRate)
				fallRate = maxFallRate;

			pExt->ParabombFallRate = fallRate;
			pThis->FallRate = fallRate;
		}

		// -----------------------------------------------------------------------
		// Hook 0x466705 — BulletClass_AI
		// -----------------------------------------------------------------------
		auto pBulletCurOwner = pThis->GetOwningHouse();
		if (pThis->Owner && pBulletCurOwner && pBulletCurOwner != pExt->Owner)
		{
			ctx.change_owner = true;
			pExt->Owner = pBulletCurOwner;
		}

		// One-shot Bright flag check (weapon or warhead can override)
		if (pThis->WeaponType && pThis->WH)
		{
			if (!pExt->BrightCheckDone)
			{
				pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
				pExt->BrightCheckDone = true;
			}
		}

		// PreExplodeRange — early detonation when within N cells of target
		if (pTypeExt->PreExplodeRange.isset())
		{
			const auto thisCoord = pThis->GetCoords();
			const auto targetCoord = pThis->GetBulletTargetCoords();

			if (Math::abs(thisCoord.DistanceFrom(targetCoord))
				<= pTypeExt->PreExplodeRange.Get(0) * 256)
			{
				if (BulletExtData::HandleBulletRemove(pThis, true, true))
				{
					ctx.early_return = true;
					return;
				}
			}
		}

		// Non-trajectory LaserTrails path
		if (!pExt->Trajectory || !PhobosTrajectory::BlockDrawTrail(pExt->Trajectory))
		{
			if (!pExt->LaserTrails.empty())
			{
				const CoordStruct& location = pThis->Location;
				const VelocityClass& velocity = pThis->Velocity;

				// Adjust LaserTrails for the vanilla one-frame-ahead drawing bug
				CoordStruct drawnCoords {
					static_cast<int>(location.X + velocity.X),
					static_cast<int>(location.Y + velocity.Y),
					static_cast<int>(location.Z + velocity.Z)
				};

				for (auto& trail : pExt->LaserTrails)
				{
					if (!trail->LastLocation.isset())
						trail->LastLocation = location;

					if (trail->Type->IsHouseColor.Get() && ctx.change_owner && pExt->Owner)
						trail->CurrentColor = pExt->Owner->LaserColor;

					trail->Update(drawnCoords);
				}
			}
		}
	}

	// ===========================================================================
	// HandleSpawnedAnimFollowup
	//   Hook 0x46670F (BulletClass_AI_PreImpactAnim)
	//
	// Called when SpawnNextAnim is set (the post-detonation tick where the bullet
	// is sleeping until its NUKEBALL/PreImpact anim finishes spawning). Returns
	// true if the bullet was actually removed this tick.
	// ===========================================================================
	OPTIONALINLINE bool HandleSpawnedAnimFollowup(FakeBulletClass* pThis)
	{
		// -----------------------------------------------------------------------
		// Hook 0x46670F — BulletClass_AI_PreImpactAnim
		// -----------------------------------------------------------------------
		if (pThis->NextAnim)
		{
			auto pWHExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);
			if (pWHExt->PreImpact_Moves.Get())
			{
				// Track the spawned anim's coords — bullet rides the anim
				auto coords = pThis->NextAnim->GetCoords();
				pThis->Location = coords;
				pThis->Target = MapClass::Instance->TryGetCellAt(coords);
			}
			return false;  // bullet sleeps another tick
		}

		// NextAnim has finished — perform the deferred cleanup
		PointerExpiredNotification::NotifyInvalidAnim->Array.erase(pThis);
		pThis->SpawnNextAnim = false;
		pThis->Explode(false);
		pThis->UnInit();
		return true;
	}

	// ===========================================================================
	// AdvanceAnimFrame
	// ===========================================================================
	OPTIONALINLINE void AdvanceAnimFrame(BulletClass* pThis)
	{
		auto pType = pThis->Type;
		if (!pType->AnimLow && !pType->AnimHigh)
			return;

		if (--pThis->AnimRateCounter != 0)
			return;

		pThis->AnimRateCounter = pType->AnimRate;
		if (++pThis->AnimFrame > pType->AnimHigh)
			pThis->AnimFrame = pType->AnimLow;
	}

	// ===========================================================================
	// SpawnTrailerIfDue
	//   Hook 0x466834 (BulletClass_AI_TrailerAnim)
	//
	// The hook completely replaces the original trailer-spawn logic with one that
	// uses the unified ScaledSpawnDelay/SpawnDelay fallback and routes through
	// AnimExtData::SetAnimOwnerHouseKind for proper house ownership.
	// ===========================================================================
	OPTIONALINLINE void SpawnTrailerIfDue(FakeBulletClass* pThis, CoordStruct const& /*coord*/)
	{
		auto pType = pThis->Type;
		if (!pType->Trailer)
			return;

		// -----------------------------------------------------------------------
		// Hook 0x466834 — BulletClass_AI_TrailerAnim
		// -----------------------------------------------------------------------
		const int delay = pType->ScaledSpawnDelay ? pType->ScaledSpawnDelay : pType->SpawnDelay;

		// VERIFY: hook only guards `delay < 0`, not `delay == 0`. If both
		// ScaledSpawnDelay and SpawnDelay are 0 the modulo below crashes.
		// Original vanilla code had the same flaw — preserved.
		if (delay < 0)
			return;

		if ((Unsorted::CurrentFrame.get() % delay) != 0)
			return;

		auto pExt = pThis->_GetExtData();

		auto pAnim = GameCreate<AnimClass>(
			pType->Trailer,
			pThis->Location,
			/*timedelay*/ 1,
			/*loop*/ 1,
			/*animflags*/ AnimFlag::AnimFlag_600,
			/*zadjust*/ 0,
			/*reverse*/ false);

		AnimExtData::SetAnimOwnerHouseKind(
			pAnim,
			pThis->Owner ? pThis->Owner->GetOwningHouse()
						  : (pExt->Owner ? pExt->Owner : nullptr),
			pThis->Target ? pThis->Target->GetOwningHouse() : nullptr,
			pThis->Owner,
			false,
			false);
	}

	// ===========================================================================
		// UpdateGuidedMotion (Type->ROT > 0)
		//
		// Hooks integrated:
		//   0x4668BD — Inviso interceptor short-circuit at entry
		//   0x466BAF — MissileROTVar replacement (type-ext lookup)
		//   0x466E9F — MissileSafetyAltitude replacement (type-ext lookup)
		//
		// EXTENSION_ARCING: 0x466E18 — Subterranean missile height bypass
		// ===========================================================================
	OPTIONALINLINE void UpdateGuidedMotion(FakeBulletClass* pThis, CoordStruct& coord,
										   BulletAITickContext& ctx)
	{
		auto pType = pThis->Type;
		auto pTypeExt = pThis->_GetTypeExtData();
		auto pVel = &pThis->Velocity;
		auto pTarget = pThis->Target;

		// -----------------------------------------------------------------------
		// Hook 0x4668BD — Inviso interceptors detonate immediately on entry
		// -----------------------------------------------------------------------
		if (pType->Inviso && pThis->_GetExtData()->InterceptorTechnoType)
		{
			ctx.detonate_now = true;
			return;
		}

		// ---------- (a) course-lock gate ---------------------------------------
		double current_speed = Math::sqrt(pVel->X * pVel->X +
										   pVel->Y * pVel->Y +
										   pVel->Z * pVel->Z);
		double speed_target = static_cast<double>(pThis->Speed);

		if (pType->CourseLockDuration > 0)
		{
			if (pThis->CourseLockCounter < pType->CourseLockDuration)
			{
				if (++pThis->CourseLockCounter >= pType->CourseLockDuration)
					pThis->CourseLock = false;
			}
		}
		else if (pThis->Speed >= 40 || speed_target <= current_speed + 0.5)
		{
			pThis->CourseLock = false;
		}

		// ---------- (b) acceleration value -------------------------------------
		int accel_value = pType->Acceleration;
		if (pThis->CourseLock && pType->CourseLockDuration == 0)
		{
			accel_value = (Unsorted::CurrentFrame.get() % 2 == 0) ? 1 : 0;
		}

		// ---------- (c) speed adjust + normalize -------------------------------
		if (current_speed != speed_target)
		{
			if (current_speed > speed_target)
			{
				current_speed -= accel_value / 2;
				if (current_speed < 0.0)
					current_speed = 0.0;
			}
			else
			{
				current_speed += accel_value;
				if (current_speed >= speed_target)
					current_speed = speed_target;
			}

			if (pVel->X == 0.0 && pVel->Y == 0.0 && pVel->Z == 0.0)
				pVel->X = 100.0;

			double mag = Math::sqrt(pVel->X * pVel->X +
									 pVel->Y * pVel->Y +
									 pVel->Z * pVel->Z);
			double scale = current_speed / mag;
			pVel->X *= scale;
			pVel->Y *= scale;
			pVel->Z *= scale;
		}

		// ---------- (d) homing turn --------------------------------------------
		VelocityClass veloc = *pVel;
		CoordStruct pCoordTarget = CoordStruct::Empty;
		CoordStruct targetCoord = CoordStruct::Empty;

		if (auto pObject = flag_cast_to<ObjectClass*>(pTarget))
		{
			pCoordTarget = pObject->GetCenterCoords();
		}
		else if (pTarget)
		{
			targetCoord = pCoordTarget = pTarget->GetCoords();
		}

		// -----------------------------------------------------------------------
		// Hook 0x466BAF — MissileROTVar (type-ext override)
		// -----------------------------------------------------------------------
		const int    nFrame = (Unsorted::CurrentFrame.get() + pThis->Fetch_ID()) % 15;
		const double nMissileROTVar = pTypeExt->MissileROTVar.Get(RulesClass::Instance->MissileROTVar);

		int rot_amount = static_cast<int>(
			(Math::sin(static_cast<double>(nFrame)
				* Math::ONE_FIFTEENTH
				* Math::GAME_TWOPI) * nMissileROTVar
				+ nMissileROTVar + 1.0)
			* static_cast<double>(pType->ROT));

		auto pCenter = pThis->GetCoords();
		CoordStruct homing_offset = pCenter - pCoordTarget;

		int homing_dist = static_cast<int>(Math::sqrt(
			static_cast<double>(homing_offset.X) * homing_offset.X +
			static_cast<double>(homing_offset.Y) * homing_offset.Y +
			static_cast<double>(homing_offset.Z) * homing_offset.Z));

		if (homing_dist < 256)
			rot_amount = static_cast<int>(rot_amount * 1.5);

		CoordStruct pre_motion_coord = coord;

		bool target_is_aircraft = (pTarget &&
								   pTarget->WhatAmI() == AbstractType::Aircraft);

		DirStruct dir {};
		if (!pThis->CourseLock)
			dir.Raw = static_cast<unsigned short>((rot_amount & 0xFF) << 8);

		int new_height = missile_maths(&coord, &veloc, &pCoordTarget, &dir,
										   target_is_aircraft,
										   pType->Airburst,
										   pType->VeryHigh,
										   pType->Level);

		auto pCellAfter = MapClass::Instance->GetCellAt(coord);
		*pVel = veloc;

		// ---------- (e) Z/altitude safety overrides ----------------------------
		double speed_after = Math::sqrt(pVel->X * pVel->X +
										 pVel->Y * pVel->Y +
										 pVel->Z * pVel->Z);

		// EXTENSION_ARCING: 0x466E18 — Subterranean missile height bypass
		// ----------------------------------------------------------------------
		// Default behavior: any guided bullet with height <= 0 force-detonates
		// at the target snap. The extension lets bullets flagged
		// `!SubjectToGround` continue flying underground (straight-line
		// missiles tunneling under terrain).
		//
		// When enabled, the second disjunct of the height-termination check
		// becomes conditional on the per-bullet status flag. The first
		// disjunct (speed_after * 0.5 < new_height — i.e. the projectile is
		// arriving faster than it can correct) is unaffected.
		//
		// COLLISION NOTE: hook is at 0x466E18, which is *inside* the original
		// `Get_Height ; test eax, eax ; jle` sequence. Pure replacement,
		// no overlap with any other Phobos hook.
		// ----------------------------------------------------------------------
		bool height_termination = (pThis->GetHeight() <= 0);
#if 0  // EXTENSION_ARCING: pending BulletStatus save/load
		if (height_termination)
		{
			if (auto status = BulletExt::GetStatus(pThis))
			{
				if (!status->SubjectToGround)
					height_termination = false;
			}
		}
#endif

		if (speed_after * 0.5 >= new_height || height_termination)
		{
			ctx.exploded = true;
			ctx.markType = 1;

			if (pThis->GetHeight() > 0
				&& !pType->Airburst
				&& (pCoordTarget.IsValid()))
			{
				coord = pCoordTarget;
			}
		}

		// -----------------------------------------------------------------------
		// Hook 0x466E9F — MissileSafetyAltitude (type-ext override)
		// -----------------------------------------------------------------------
		if (!pCoordTarget.IsValid())
		{
			const int safety_alt = pTypeExt->GetMissileSaveAltitude(RulesClass::Instance());
			if (pThis->GetHeight() >= safety_alt)
			{
				ctx.exploded = true;
				ctx.markType = 1;
			}
		}

		// ---------- (f) smoothed proximity-fuse trigger ------------------------
		CoordStruct to_target_pre = pre_motion_coord - pCoordTarget;

		int dist_pre = static_cast<int>(Math::sqrt(
			static_cast<double>(to_target_pre.X) * to_target_pre.X +
			static_cast<double>(to_target_pre.Y) * to_target_pre.Y +
			static_cast<double>(to_target_pre.Z) * to_target_pre.Z));

		CoordStruct to_target_post = coord - pCoordTarget;

		int dist_post = static_cast<int>(Math::sqrt(
			static_cast<double>(to_target_post.X) * to_target_post.X +
			static_cast<double>(to_target_post.Y) * to_target_post.Y +
			static_cast<double>(to_target_post.Z) * to_target_post.Z));

		int dist_delta = dist_pre - dist_post;

		if (!pThis->CourseLock)
		{
			if (pThis->SomeIntIncrement_118 < 60)
			{
				++pThis->SomeIntIncrement_118;
				pThis->unknown_120 += dist_delta;
			}
			else
			{
				pThis->unknown_120 *= 0.9833333333333333 + dist_delta;

				if (pThis->unknown_120 >= 0.0
					&& pThis->unknown_120 < 60.0)
				{
					if (!pType->Airburst && !pType->VeryHigh)
					{
						ctx.exploded = true;
						ctx.markType = 1;
						return;
					}
				}
			}
		}

		if (ctx.markType != 0)
			return;

		// ---------- (g) wall-top Z crossing test -------------------------------
		if (pCellAfter->ContainsBridgeHead()
			|| MapClass::Instance->GetCellAt(pre_motion_coord)->ContainsBridgeHead())
		{
			int wall_top = MapClass::Instance->GetCellFloorHeight(coord) + CellClass::BridgeHeight;

			if (coord.Z > wall_top && pre_motion_coord.Z < wall_top)
			{
				ctx.markType = 1;
				coord.Z = wall_top;
				ctx.exploded = true;
				return;
			}
			if (coord.Z < wall_top && pre_motion_coord.Z > wall_top)
			{
				ctx.markType = 1;
				coord.Z = wall_top;
				ctx.exploded = true;
				return;
			}
		}
	}

	// ===========================================================================
	// UpdateVerticalMotion (Type->Vertical)
	// ===========================================================================
	OPTIONALINLINE void UpdateVerticalMotion(FakeBulletClass* pThis, CoordStruct& coord,
											  VelocityClass& veloc, BulletAITickContext& ctx)
	{
		auto pType = pThis->Type;
		auto pVel = &pThis->Velocity;

		veloc = *pVel;

		if (Math::sqrt(veloc.X * veloc.X +
			veloc.Y * veloc.Y +
			veloc.Z * veloc.Z) < 8.0)
		{
			ctx.markType = 1;
		}

		// Accelerate up to Speed
		double mag = Math::sqrt(pVel->X * pVel->X +
								 pVel->Y * pVel->Y +
								 pVel->Z * pVel->Z);

		if (mag < pThis->Speed)
		{
			double new_mag = mag + pType->Acceleration;

			if (pVel->X == 0.0 && pVel->Y == 0.0 && pVel->Z == 0.0)
				pVel->X = 100.0;

			double cur_mag = Math::sqrt(pVel->X * pVel->X +
										 pVel->Y * pVel->Y +
										 pVel->Z * pVel->Z);
			double scale = new_mag / cur_mag;
			pVel->X *= scale;
			pVel->Y *= scale;
			pVel->Z *= scale;
		}

		CoordStruct pre = coord;
		coord.X += static_cast<int>(pVel->X);
		coord.Y += static_cast<int>(pVel->Y);
		coord.Z += static_cast<int>(pVel->Z);

		if (coord.Z > pType->DetonationAltitude)
		{
			ctx.markType = 1;
			ctx.exploded = true;
			return;
		}
		if (pThis->GetHeight() < 0)
		{
			ctx.markType = 1;
			ctx.exploded = true;
			return;
		}

		// Wall-top crossing
		int wall_top = MapClass::Instance->GetCellFloorHeight(coord) + CellClass::BridgeHeight;
		auto pCellNew = MapClass::Instance->GetCellAt(coord);
		auto pCellPre = MapClass::Instance->GetCellAt(pre);

		if (pCellNew->ContainsBridgeHead() || pCellPre->ContainsBridgeHead())
		{
			if (coord.Z < wall_top)
			{
				if (pre.Z >= wall_top)
				{
					ctx.markType = 1;
					ctx.exploded = true;
				}
			}
			else if (pre.Z < wall_top)
			{
				ctx.markType = 1;
				ctx.exploded = true;
			}
		}
	}

	// ===========================================================================
		// UpdateBallisticMotion (gravity + bounce)
		//
		// Hooks integrated:
		//   0x4671B9 — ApplyGravity (type-ext override)
		//   0x46745C — Position_Trajectories (Phobos plugin velocity/position)
		//
		// EXTENSION_ARCING: 0x46745C — Arcing per-tick velocity refresh
		//                   (COLLIDES with Position_Trajectories — see note)
		// ===========================================================================
	OPTIONALINLINE void UpdateBallisticMotion(FakeBulletClass* pThis, CoordStruct& coord,
											   VelocityClass& veloc, BulletAITickContext& ctx)
	{
		auto pType = pThis->Type;
		auto pVel = &pThis->Velocity;
		auto pExt = pThis->_GetExtData();

		veloc = *pVel;

		if (Math::sqrt(veloc.X * veloc.X +
			veloc.Y * veloc.Y +
			veloc.Z * veloc.Z) < 8.0)
		{
			ctx.markType = 1;
		}

		Vector3D<double> pos_ {
			static_cast<double>(coord.X) ,
			static_cast<double>(coord.Y) ,
			static_cast<double>(coord.Z)
		};

		// -----------------------------------------------------------------------
		// Hook 0x4671B9 — ApplyGravity (BulletTypeExtData::GetAdjustedGravity)
		// -----------------------------------------------------------------------
		double gravity = BulletTypeExtData::GetAdjustedGravity(pType);

		auto pPayback = pThis->Owner;

		veloc.Z = veloc.Z - gravity;

		CoordStruct old_pos {
			static_cast<int>(pos_.X),
			static_cast<int>(pos_.Y),
			static_cast<int>(pos_.Z)
		};

		// -----------------------------------------------------------------------
		// Hook 0x46745C — Position_Trajectories (Phobos)
		// -----------------------------------------------------------------------
		if (pExt->Trajectory)
		{
			VelocityClass position_as_velocity {
				pos_.X, pos_.Y, pos_.Z
			};
			pExt->Trajectory->OnAIVelocity(&veloc, &position_as_velocity);
			pos_.X = position_as_velocity.X;
			pos_.Y = position_as_velocity.Y;
			pos_.Z = position_as_velocity.Z;

			if (!pExt->LaserTrails.empty())
			{
				CoordStruct futureCoords {
					static_cast<int>(veloc.X + position_as_velocity.X),
					static_cast<int>(veloc.Y + position_as_velocity.Y),
					static_cast<int>(veloc.Z + position_as_velocity.Z)
				};
				for (auto& trail : pExt->LaserTrails)
				{
					if (!trail->LastLocation.isset())
						trail->LastLocation = pThis->Location;
					trail->Update(futureCoords);
				}
			}
		}

		// EXTENSION_ARCING: 0x46745C — Arcing per-tick velocity refresh
		// ----------------------------------------------------------------------
		// HOOK COLLISION: this hook lives at the SAME address as Phobos's
		// Position_Trajectories. The two cannot stack as raw ASMJIT patches
		// — only one can win. They have different intents though, so the
		// merged C++ form runs both sequentially:
		//
		//   1. Phobos trajectory plugin gets first crack at velocity/position
		//   2. The Arcing extension overwrites velocity from pThis->Velocity
		//      directly (used when scripts mutate Velocity mid-flight and the
		//      bullet needs to pick up the new value)
		//   3. If LocationLocked is also set, Z velocity is forced to 0
		//      (the bullet hovers at its current altitude)
		//
		// ORDER RATIONALE: extension wins because LocationLocked is the more
		// aggressive constraint — a hovering bullet should hover regardless
		// of what a trajectory plugin computes. Flip the order if your
		// plugin authors disagree.
		//
		// PAIRED WITH: 0x4677C7 — that hook freezes the X/Y position too,
		// completing the "hover in place" mode.
		// ----------------------------------------------------------------------
#if 0  // EXTENSION_ARCING: pending BulletStatus save/load
		if (auto status = BulletExt::GetStatus(pThis))
		{
			if (status->IsArcing() && status->SpeedChanged)
			{
				veloc.X = pThis->Velocity.X;
				veloc.Y = pThis->Velocity.Y;
				veloc.Z = status->LocationLocked ? 0.0 : pThis->Velocity.Z;
			}
		}
#endif

		// Integrate velocity into double position
		pos_.X += veloc.X;
		pos_.Y += veloc.Y;
		pos_.Z += veloc.Z;

		CoordStruct new_pos {
			static_cast<int>(pos_.X),
			static_cast<int>(pos_.Y),
			static_cast<int>(pos_.Z)
		};

		int terrain_z = MapClass::Instance->GetCellFloorHeight(new_pos);
		int wall_top = terrain_z + CellClass::BridgeHeight;
		auto pCellNew = MapClass::Instance->GetCellAt(new_pos);

		bool crossed_top_descending = false;
		bool crossed_top_ascending = false;

		if (pCellNew->ContainsBridgeHead() || MapClass::Instance->GetCellAt(old_pos)->ContainsBridgeHead())
		{
			if (new_pos.Z < wall_top)
			{
				if (old_pos.Z >= wall_top)
					crossed_top_ascending = true;
			}
			else if (old_pos.Z < wall_top)
			{
				crossed_top_descending = true;
			}
		}

		bool blocked = false;

		bool inside_terrain_window = !crossed_top_ascending
			&& !crossed_top_descending
			&& (pos_.Z >= static_cast<double>(terrain_z))
			&& (pos_.Z - 150.0 < static_cast<double>(terrain_z));

		if (inside_terrain_window)
		{
			auto pBuilding = pCellNew->GetBuilding();
			if (!pBuilding && !pCellNew->ConnectsToOverlay(-1, -1))
			{
				// Nothing to bounce off
			}
			else
			{
				blocked = true;
				if (pBuilding)
				{
					if (pBuilding == pPayback)
						blocked = false;
					if (pBuilding->Type->LaserFence && pBuilding->LaserFenceFrame >= 8)
						blocked = false;
					if (pBuilding->IsStrange())
						blocked = false;

					if (pPayback && pPayback->Owner
						&& pPayback->Owner->IsAlliedWith(pBuilding))
					{
						blocked = false;
					}
				}
			}
		}

		if (!blocked && pos_.Z >= static_cast<double>(terrain_z))
		{
			coord.X = static_cast<int>(pos_.X);
			coord.Y = static_cast<int>(pos_.Y);
			coord.Z = static_cast<int>(pos_.Z);
			return;
		}

		if (pPayback)
		{
			if (crossed_top_ascending)
				pos_.Z = wall_top;
			else if (crossed_top_descending)
				pos_.Z = wall_top - 20;
		}
		else
		{
			if (crossed_top_ascending)
				pos_.Z = wall_top;
			else if (crossed_top_descending)
				pos_.Z = wall_top - 20;
			else
			{
				int floor_minus = wall_top - 100;
				if (floor_minus < pos_.Z)
					pos_.Z = static_cast<double>(terrain_z);
			}

			int ramp = TacticalClass::Instance->GetRamp(&new_pos);
			Matrix3D ramp_matrix;
			Game::GetRampMtx(&ramp_matrix, ramp);
			Matrix3D inv;
			Matrix3D::TransposeMatrix(&inv, &ramp_matrix);

			Vector3D<float> v_in(static_cast<float>(veloc.X),
								 static_cast<float>(-veloc.Y),
								 static_cast<float>(veloc.Z));

			Vector3D<float> v_local = inv.RotateVector(v_in);

			v_local *= static_cast<float>(pType->Elasticity);
			v_local.Z = -v_local.Z;

			Vector3D<float> v_out = ramp_matrix.RotateVector(v_local);

			veloc.X = v_out.X;
			veloc.Y = -v_out.Y;
			veloc.Z = v_out.Z;
		}

		ctx.markType = 1;
		ctx.exploded = true;

		coord.X = static_cast<int>(pos_.X);
		coord.Y = static_cast<int>(pos_.Y);
		coord.Z = static_cast<int>(pos_.Z);
	}

	// ===========================================================================
		// CheckUnguidedTargetCell (LABEL_157 in the original)
		//
		// Hooks integrated:
		//   0x4677D3 — TargetCoordCheck_Trajectories (full-phase override)
		//   0x467927 — TechnoCheck_Trajectories (techno-collision override)
		//   0x467AB2 — Parabomb (skip velocity copyback for parachute bullets)
		//
		// EXTENSION_ARCING: 0x4677C7 — LocationLocked position freeze
		// ===========================================================================
	OPTIONALINLINE void CheckUnguidedTargetCell(FakeBulletClass* pThis, CoordStruct& coord,
												 VelocityClass& veloc, BulletAITickContext& ctx)
	{
		auto pType = pThis->Type;

		// EXTENSION_ARCING: 0x4677C7 — LocationLocked position freeze
		// ----------------------------------------------------------------------
		// Sits at the very entry to LABEL_157 (the cell-rounding block). For
		// LocationLocked Arcing bullets, the post-motion `coord` is forcibly
		// reset to pThis->Location — i.e. the bullet didn't move this tick.
		//
		// PAIRED WITH: 0x46745C above (Z velocity zero). Together they create
		// a "hover in place" mode that still runs all subsequent fuse and
		// detonation logic, just without ballistic motion.
		//
		// PLACEMENT: must run BEFORE the cell_now computation, otherwise the
		// building footprint snap and techno-on-cell checks would use the
		// wrong (post-motion) cell coordinates.
		// ----------------------------------------------------------------------
#if 0  // EXTENSION_ARCING: pending BulletStatus save/load
		if (auto status = BulletExt::GetStatus(pThis))
		{
			if (status->IsArcing() && status->SpeedChanged && status->LocationLocked)
			{
				coord = pThis->Location;
			}
		}
#endif

		// -----------------------------------------------------------------------
		// Hook 0x4677D3 — TargetCoordCheck_Trajectories
		// -----------------------------------------------------------------------
		{
			const auto traj_result = PhobosTrajectory::OnAITargetCoordCheck(pThis, coord);
			if (traj_result != 0u)
			{
				return;
			}
		}

		// ---------- (a) building footprint snap --------------------------------
		CellStruct cell_now {
			CoordToCell(coord.X),
			CoordToCell(coord.Y)
		};
		CellStruct cell_target {
			CoordToCell(pThis->TargetCoords.X),
			CoordToCell(pThis->TargetCoords.Y)
		};

		bool target_cell_match = (cell_now.X == cell_target.X)
			&& (cell_now.Y == cell_target.Y)
			&& !pType->Vertical
			&& pThis->GetHeight() < 2 * Unsorted::LevelHeight;

		if (target_cell_match)
		{
			ctx.markType = 1;
			ctx.exploded = true;
			ctx.snap_to_target = true;
			return;
		}

		auto pCellNow = MapClass::Instance->GetCellAt(cell_now);
		auto pCellTarget = MapClass::Instance->GetCellAt(cell_target);
		auto pBldNow = pCellNow->GetBuilding();
		auto pBldTarget = pCellTarget->GetBuilding();

		bool building_footprint_hit = pBldNow
			&& !pType->Vertical
			&& pBldNow == pBldTarget
			&& pThis->GetHeight() < 2 * Unsorted::LevelHeight;
		if (building_footprint_hit)
		{
			ctx.markType = 1;
			ctx.exploded = true;
			ctx.snap_to_target = true;
			return;
		}

		// ---------- (b) techno-on-cell collision -------------------------------
		Point2D point_zero { 0, 0 };
		auto pCellAtCoord = MapClass::Instance->GetCellAt(coord);
		auto pTechnoOnCell = pCellAtCoord->FindTechnoNearestTo(point_zero, false, nullptr);

		if (pTechnoOnCell)
		{
			const auto techno_result = PhobosTrajectory::OnAITechnoCheck(pThis, pTechnoOnCell);
			if (techno_result != 0u)
			{
				pTechnoOnCell = nullptr;
			}
		}

		auto pPayback = pThis->Owner;
		bool techno_present = pTechnoOnCell != nullptr;
		bool payback_present = pPayback != nullptr;

		bool is_self_hit = techno_present && payback_present
			&& (pTechnoOnCell == pPayback);

		bool is_ally_hit = false;
		if (techno_present && payback_present)
			is_ally_hit = pPayback->Owner->IsAlliedWith(pTechnoOnCell);

		bool techno_close_enough = false;
		if (pTechnoOnCell)
		{
			CoordStruct delta {
				coord.X - pTechnoOnCell->Location.X,
				coord.Y - pTechnoOnCell->Location.Y,
				coord.Z - pTechnoOnCell->Location.Z
			};
			if (Math::sqrt(static_cast<double>(delta.X) * delta.X +
				static_cast<double>(delta.Y) * delta.Y +
				static_cast<double>(delta.Z) * delta.Z) < 128.0)
			{
				techno_close_enough = true;
			}
		}

		if (!is_self_hit && !is_ally_hit && techno_close_enough)
		{
			ctx.markType = 1;
			ctx.exploded = true;
			if (!pType->Inaccurate)
				coord = pTechnoOnCell->Location;
			return;
		}

		// ---------- (c) in-map bounds ------------------------------------------
		if (!MapClass::Instance->IsValid(coord))
		{
			ctx.markType = 2;
			ctx.exploded = true;
			coord = pThis->Location;
			return;
		}

		// -----------------------------------------------------------------------
		// Hook 0x467AB2 — Parabomb
		// -----------------------------------------------------------------------
		if (!pType->Vertical && !pThis->HasParachute)
		{
			pThis->Velocity = veloc;
		}

		// ---------- (d) terminate when hugging ground at near-zero velocity ----
		if (Math::sqrt(pThis->Velocity.X * pThis->Velocity.X +
			pThis->Velocity.Y * pThis->Velocity.Y +
			pThis->Velocity.Z * pThis->Velocity.Z) < 10.0
			&& pThis->GetHeight() < 10)
		{
			ctx.markType = 1;
			ctx.exploded = true;
		}
	}

	// ============================================================================
	// FinalizeBulletMotion — refactored to integrate BounceTrajectory cleanly
	//
	// The bounce intercept is structured as an early branch that decides whether
	// the bullet (a) keeps flying after a bounce, (b) detonates immediately due to
	// exhausted bounces, or (c) wasn't a bouncing trajectory and falls through to
	// vanilla logic. No gotos, no skipped initializers.
	//
	// Drop-in replacement for the existing FinalizeBulletMotion in the anonymous
	// namespace of FakeBulletClass_AI.cpp.
	// ============================================================================

	OPTIONALINLINE void FinalizeBulletMotion(FakeBulletClass* pThis, CoordStruct& coord,
											  BulletAITickContext& ctx)
	{
		auto pType = pThis->Type;
		auto pTarget = pThis->Target;

		// markType==2 short-circuit: mark only, no Set_Coord, no explode
		if (ctx.markType == 2)
		{
			pThis->Mark(MarkType::Up);
			UpdateLastMapCoords(pThis, coord);
			return;
		}

		pThis->Mark(MarkType::Up);

		// -----------------------------------------------------------------------
		// Hook 0x467B94 — Ranged + Firestorm
		// -----------------------------------------------------------------------
		{
			if (pType->Ranged)
			{
				CoordStruct crdOld = pThis->GetCoords();
				pThis->Range -= static_cast<int>(coord.DistanceFrom(crdOld));
				if (pThis->Range <= 0)
					ctx.exploded = true;
			}

			pThis->SetLocation(coord);

			if (HouseExtContainer::Instance.IsAnyFirestormActive && !pType->IgnoresFirestorm)
			{
				auto pCell = MapClass::Instance->GetCellAt(coord);
				if (auto pBld = pCell->GetBuilding())
				{
					HouseClass* pOwner = pThis->Owner
						? pThis->Owner->Owner
						: pThis->_GetExtData()->Owner;

					if (WarheadTypeExtContainer::Instance
							.Find(RulesExtData::Instance()->FirestormWarhead)
							->CanAffectHouse(pBld->Owner, pOwner))
					{
						pOwner = nullptr;
					}

					if (BuildingExtData::IsActiveFirestormWall(pBld, pOwner))
					{
						BuildingExtData::ImmolateVictim(pBld, pThis, false);
						BulletExtData::HandleBulletRemove(pThis,
							ScenarioClass::Instance->Random.RandomBool(), true);
						UpdateLastMapCoords(pThis, coord);
						return;
					}
				}
			}
		}

		// -----------------------------------------------------------------------
		// Bounce trajectory intercept
		//
		// Runs BEFORE Is_Forced_To_Explode so a bouncing bullet can refuse the
		// forced detonation. Returns one of:
		//
		//   ContinueVanilla — not a bouncing trajectory, fall through normally
		//   KeepFlying      — bullet bounced, skip the forced-explode check this
		//                     tick, fall through to fuse/snap/detonate which will
		//                     find no reason to detonate and let the bullet
		//                     continue on its new trajectory
		//   ForceDetonate   — bounces exhausted, detonate immediately (skip fuse
		//                     and target snap, go straight to RunDetonation)
		// -----------------------------------------------------------------------
		enum class BouncePhaseResult { ContinueVanilla, KeepFlying, ForceDetonate };

		auto bounce_phase = [&]() -> BouncePhaseResult
			{
				auto pExt = pThis->_GetExtData();
				if (!pExt->Trajectory)
					return BouncePhaseResult::ContinueVanilla;

				bool force_detonate_from_bounce = false;
				const auto bounce_result = pExt->Trajectory->OnBounceCheck(
					coord, force_detonate_from_bounce);

				switch (bounce_result)
				{
				case BounceCheckResult::NotHandled:
					return BouncePhaseResult::ContinueVanilla;

				case BounceCheckResult::BouncedKeepFlying:
					// Reflection happened. Velocity has been updated by OnBounceCheck;
					// re-commit the location to be safe and clear any prior explode
					// flag from upstream phases (Range decay, etc. don't apply when
					// a bounce keeps us alive).
					ctx.exploded = false;
					pThis->SetLocation(coord);
					return BouncePhaseResult::KeepFlying;

				case BounceCheckResult::BouncedDetonate:
					ctx.exploded = true;
					return force_detonate_from_bounce
						? BouncePhaseResult::ForceDetonate
						: BouncePhaseResult::ContinueVanilla;
				}

				return BouncePhaseResult::ContinueVanilla;
			};

		const auto bounce_outcome = bounce_phase();

		if (bounce_outcome == BouncePhaseResult::ForceDetonate)
		{
			// Skip fuse, target snap, and the still-flying check entirely.
			// Go straight to detonation.
			if (pThis->GetHeight() < 0)
				pThis->SetHeight(0);
			RunDetonation(pThis, coord, ctx);
			return;
		}

		// -----------------------------------------------------------------------
		// Is_Forced_To_Explode override (custom-trigger explode)
		//
		// Skipped when a bounce kept the bullet flying — the bullet has just
		// changed direction and shouldn't be auto-detonated by the same trigger
		// that caused the bounce.
		// -----------------------------------------------------------------------
		bool forced = false;
		if (bounce_outcome != BouncePhaseResult::KeepFlying && !ctx.exploded)
		{
			CoordStruct here = pThis->Location;
			forced = pThis->IsForceToExplode(here);
			pThis->SetLocation(here);
			if (forced)
				ctx.exploded = true;
		}

		if (ctx.exploded || forced)
		{
			if (pThis->GetHeight() < 0)
				pThis->SetHeight(0);
		}

		// -----------------------------------------------------------------------
		// Hook 0x467C2E — FuseCheck (extended fuse checkup)
		// -----------------------------------------------------------------------
		Fuse fuse_result = Fuse::DontIgnite;
		if (pType->ROT > 0 || pType->Ranged)
		{
			fuse_result = static_cast<Fuse>(
				BulletExtData::FuseCheckup(pThis, &coord));
		}

		// JumpJet target override
		auto pPayback = pThis->Owner;
		if (pPayback
			&& pPayback->GetTechnoType()->JumpJet
			&& fuse_result == Fuse::Ignite_DistaceFactor)
		{
			fuse_result = Fuse::Ignite;
		}

		// -----------------------------------------------------------------------
		// "Still flying" early-out
		// Hook 0x467C1C — Inviso bullets skip the IsDegenerate damage tick
		// -----------------------------------------------------------------------
		if (!ctx.exploded)
		{
			if (pType->Dropping || fuse_result == Fuse::DontIgnite)
			{
				if (!pType->Inviso && pType->Degenerates && pThis->Health > 5)
					--pThis->Health;

				UpdateLastMapCoords(pThis, coord);
				return;
			}
		}

		// -----------------------------------------------------------------------
		// Hook 0x467CCA — TargetSnapChecks
		//
		// Skipped on KeepFlying outcome — a bouncing bullet shouldn't snap to its
		// original target after deflection.
		// -----------------------------------------------------------------------
		if (bounce_outcome != BouncePhaseResult::KeepFlying
			&& pTarget
			&& (fuse_result == Fuse::Ignite || ctx.snap_to_target))
		{
			bool snap_allowed;
			if (pType->Inviso)
			{
				snap_allowed = !pType->Inaccurate;
			}
			else
			{
				auto pExt = pThis->_GetExtData();
				if (pExt->Trajectory && PhobosTrajectory::CanSnap(pExt->Trajectory))
					snap_allowed = !pType->Inaccurate;
				else
					snap_allowed = !pType->Airburst && !pType->Inaccurate;
			}

			if (snap_allowed)
			{
				CoordStruct target_coord;
				pTarget->GetCoords(&target_coord);

				CoordStruct delta {
					coord.X - target_coord.X,
					coord.Y - target_coord.Y,
					(target_coord.Z + coord.Z) / 2 - target_coord.Z
				};
				int dist = static_cast<int>(Math::sqrt(
					static_cast<double>(delta.X) * delta.X +
					static_cast<double>(delta.Z) * delta.Z +
					static_cast<double>(delta.Y) * delta.Y));

				int dist_threshold = ctx.snap_to_target ? (dist / 3) : dist;

				double speed_mag = Math::sqrt(
					pThis->Velocity.X * pThis->Velocity.X +
					pThis->Velocity.Y * pThis->Velocity.Y +
					pThis->Velocity.Z * pThis->Velocity.Z);
				double snap_radius = (speed_mag * 2.0 >= 128.0)
					? speed_mag * 2.0
					: 128.0;

				if (fuse_result == Fuse::Ignite
					|| dist_threshold <= snap_radius)
				{
					CoordStruct center;
					pTarget->GetCoords(&center);
					pThis->SetLocation(center);
				}
			}
		}

		// Detonation pipeline
		RunDetonation(pThis, coord, ctx);
	}

	// ===========================================================================
	// RunDetonation
	//
	// Hooks integrated:
	//   0x467E53 — PreDetonation_Trajectories (notify trajectory plugin)
	//   0x467E59 — NukeBall (replace hardcoded NUKE warhead check with WH-ext lookup)
	//
	// Also reachable directly via the trajectory short-circuit from RunEarlyTickHooks
	// (when ctx.detonate_now is true).
	// ===========================================================================
	OPTIONALINLINE void RunDetonation(FakeBulletClass* pThis, CoordStruct& coord,
									  BulletAITickContext& ctx)
	{
		auto pExt = pThis->_GetExtData();

		// -----------------------------------------------------------------------
		// Hook 0x467E53 — PreDetonation_Trajectories
		// -----------------------------------------------------------------------
		if (auto& pTraj = pExt->Trajectory)
			pTraj->OnAIPreDetonate();

		// -----------------------------------------------------------------------
		// Hook 0x467E59 — NukeBall (universal NukeFlash + PreImpact anim)
		//
		// Replaces the hardcoded `Warhead == "NUKE"` check with:
		//   - SW NuclearMissile bullet detection (gets a radar event)
		//   - NukeFlashDuration warhead-ext for the screen flash
		//   - PreImpactAnim warhead-ext for the deferred-detonation anim
		// -----------------------------------------------------------------------
		auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);
		bool allowFlash = true;

		// SW Nuclear missile bullet path
		if (pExt->NukeSW && !pThis->WH->NukeMaker)
		{
			SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

			if (pThis->GetHeight() < 0)
				pThis->SetHeight(0);

			auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pExt->NukeSW);

			if (pSWTypeExt->SW_RadarEvent)
			{
				auto coords = pThis->GetMapCoords();
				RadarEventClass::Create(RadarEventType::SuperweaponActivated, coords);
			}

			if (pSWTypeExt->Lighting_Enabled.isset())
				allowFlash = pSWTypeExt->Lighting_Enabled.Get();
		}

		// Optional screen flash (replaces NukeFlash::FadeIn call)
		const auto duration = pWarheadExt->NukeFlashDuration.Get();
		if (allowFlash && duration > 0)
		{
			NukeFlash::Status = NukeFlashStatus::FadeIn;
			ScenarioClass::Instance->AmbientTimer.Start(1);

			NukeFlash::StartTime = Unsorted::CurrentFrame.get();
			NukeFlash::Duration = duration;

			SWTypeExtData::ChangeLighting(pExt->NukeSW ? pExt->NukeSW : nullptr);
			MapClass::Instance->RedrawSidebar(1);
		}

		// PreImpact anim — defer detonation by spawning anim and parking the bullet
		if (auto pPreImpact = pWarheadExt->PreImpactAnim.Get())
		{
			AnimClass* pNewAnim = nullptr;

			CoordStruct anim_coord = pThis->Location;
			int zadjust = -15;

			pNewAnim = GameCreate<AnimClass>(
				pPreImpact, &anim_coord,
				/*timedelay*/ 0, /*loop*/ 1,
				/*animflags*/ AnimFlag::AnimFlag_2000
							| AnimFlag::AnimFlag_400
							| AnimFlag::AnimFlag_200,
				/*zadjust*/ zadjust, /*reverse*/ 0);

			if (pNewAnim)
			{
				pThis->NextAnim = pNewAnim;
				pThis->SpawnNextAnim = true;
				PointerExpiredNotification::NotifyInvalidAnim->Array.push_back(pThis);
				UpdateLastMapCoords(pThis, coord);
				return;
			}
			// Fall through to normal explode if anim creation failed
		}

		// Normal detonation
		pThis->Explode(ctx.exploded);
		pThis->UnInit();
		UpdateLastMapCoords(pThis, coord);
	}

	// ===========================================================================
	// UpdateLastMapCoords — packed cell coords for the bullet's "last seen at"
	// field. Original asm packs cy<<16 | cx via two `mov word ptr` writes.
	// ===========================================================================
	OPTIONALINLINE void UpdateLastMapCoords(BulletClass* pThis, CoordStruct const& coord)
	{
		pThis->LastMapCoords = CellClass::Coord2Cell(coord);
	}

}  // anonymous namespace

// ===========================================================================
// FakeBulletClass::_AI — orchestrator
// ===========================================================================
void FakeBulletClass::_AI()
{
	auto pThis = static_cast<BulletClass*>(this);

	pThis->ObjectClass::Update();

	BulletAITickContext ctx;

	// ---- Early hooks (0x4666F7 + 0x466705) --------------------------------
	RunEarlyTickHooks(this, ctx);

	if (ctx.early_return)
		return;

	if (ctx.detonate_now)
	{
		// Trajectory plugin's OnAI returned true → skip motion entirely and
		// jump straight to the detonation pipeline (matches the original
		// 0x4666F7 → 0x467E53 jump).
		CoordStruct coord = pThis->Location;
		RunDetonation(this, coord, ctx);
		return;
	}

	if (!pThis->IsAlive)
		return;

	// ---- SpawnNextAnim sleep tick (NUKEBALL/PreImpact follow-up) ----------
	if (pThis->SpawnNextAnim)
	{
		HandleSpawnedAnimFollowup(this);
		return;
	}

	auto pType = pThis->Type;

	// IsDropping bullets explode immediately the first frame they're not
	// falling (the "release from carrier" gate).
	if (pType->Dropping && !pThis->IsFallingDown)
		ctx.exploded = true;

	AdvanceAnimFrame(pThis);

	CoordStruct coord = pThis->Location;

	SpawnTrailerIfDue(this, coord);

	if (pType->ROT > 0)
	{
		// Guided / homing missile path
		UpdateGuidedMotion(this, coord, ctx);

		// Guided path can also short-circuit to detonate (Inviso interceptor)
		if (ctx.detonate_now)
		{
			RunDetonation(this, coord, ctx);
			return;
		}
	}
	else
	{
		VelocityClass veloc;

		if (pType->Vertical)
			UpdateVerticalMotion(this, coord, veloc, ctx);
		else
			UpdateBallisticMotion(this, coord, veloc, ctx);

		CheckUnguidedTargetCell(this, coord, veloc, ctx);
	}

	FinalizeBulletMotion(this, coord, ctx);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4740, FakeBulletClass::_AI);

#ifdef _old
ASMJIT_PATCH(0x46670F, BulletClass_AI_PreImpactAnim, 6)
{
	GET(BulletClass*, pThis, EBP);

	const auto pWarheadTypeExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

	if (!pThis->NextAnim)
		return 0x46671D;

	if (pWarheadTypeExt->PreImpact_Moves.Get())
	{
		auto coords = pThis->NextAnim->GetCoords();
		pThis->Location = coords;
		pThis->Target = MapClass::Instance->TryGetCellAt(coords);
	}

	return 0x467FEE;
}

ASMJIT_PATCH(0x467CCA, BulletClass_AI_TargetSnapChecks, 0x6) //was C
{
	enum { SkipAirburstCheck = 0x467CDE, SkipSnapFunc = 0x467E53 };

	GET(FakeBulletClass*, pThis, EBP);

	retfunc_fixed nRet(R, SkipAirburstCheck , pThis->Type);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		return nRet();
	}
	else
	{
		auto const pExt = pThis->_GetExtData();

		if (pExt->Trajectory && PhobosTrajectory::CanSnap(pExt->Trajectory))
		{
			return nRet();
		}
	}

	return 0;
}

ASMJIT_PATCH(0x466705, BulletClass_AI, 0x6) //8
{
	enum { retContunue = 0x0 , retDead = 0x466781 };
	GET(FakeBulletClass* const, pThis, EBP);

	const auto pBulletExt = pThis->_GetExtData();
	bool bChangeOwner = false;
	auto const pBulletCurOwner = pThis->GetOwningHouse();

	if (pThis->Owner && pBulletCurOwner && pBulletCurOwner != pBulletExt->Owner)
	{
		bChangeOwner = true;
		pBulletExt->Owner = pBulletCurOwner;
	}

	if (pThis->WeaponType && pThis->WH)
	{
		if (!pBulletExt->BrightCheckDone)
		{
			pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
			pBulletExt->BrightCheckDone = true;
		}
	}

	auto const pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->PreExplodeRange.isset())
	{
		const auto ThisCoord = pThis->GetCoords();
		const auto TargetCoords = pThis->GetBulletTargetCoords();

		if (Math::abs(ThisCoord.DistanceFrom(TargetCoords))
			<= pTypeExt->PreExplodeRange.Get(0) * 256)
			if (BulletExtData::HandleBulletRemove(pThis, true, true))
				return retDead;
	}

	if(!pBulletExt->Trajectory || !PhobosTrajectory::BlockDrawTrail(pBulletExt->Trajectory)){

		// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
		// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
		if ((!pBulletExt->LaserTrails.empty()))
		{
			const CoordStruct& location = pThis->Location;
			const VelocityClass& velocity = pThis->Velocity;

			// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
			// Pretty meh solution but works until we fix the bug - Kerbiter
			CoordStruct drawnCoords
			{
				(int)(location.X + velocity.X),
				(int)(location.Y + velocity.Y),
				(int)(location.Z + velocity.Z)
			};

			for (auto& trail : pBulletExt->LaserTrails)
			{
				// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
				// TODO move hack to BulletClass creation
				if (!trail->LastLocation.isset())
					trail->LastLocation = location;

				if (trail->Type->IsHouseColor.Get() && bChangeOwner && pBulletExt->Owner)
					trail->CurrentColor = pBulletExt->Owner->LaserColor;

				trail->Update(drawnCoords);
			}
		}

		/*TrailsManager::AI(pThis->_AsBullet());*/
	}
	//if (!pThis->Type->Inviso && pBulletExt->InitialBulletDir.has_value())
	//	pBulletExt->InitialBulletDir = DirStruct((-1) * std::atan2(pThis->Velocity.Y, pThis->Velocity.X));

	return 0;
}

ASMJIT_PATCH(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	//GET(BulletClass* const, pThis, EBP);
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x4671BF;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
ASMJIT_PATCH(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B, Continue = 0x0 };
	GET(FakeBulletClass*, pThis, EBP);
	return (pThis->Type->Inviso && pThis->_GetExtData()->InterceptorTechnoType)
		? DetonateBullet : Continue;
}


ASMJIT_PATCH(0x466BAF, BulletClass_AI_MissileROTVar, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);

	const auto nFrame = (Unsorted::CurrentFrame + pThis->Fetch_ID()) % 15;
	const double nMissileROTVar = pThis->_GetTypeExtData()->MissileROTVar.Get(RulesClass::Instance->MissileROTVar);

	R->EAX(int(Math::sin(static_cast<double>(nFrame) *
		Math::ONE_FIFTEENTH *
		Math::GAME_TWOPI) *
		nMissileROTVar + nMissileROTVar + 1.0) *
		static_cast<double>(pThis->Type->ROT)
	);

	return 0x466C14;
}

ASMJIT_PATCH(0x466E9F, BulletClass_AI_MissileSafetyAltitude, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);
	GET(int, comparator, EAX);
	return comparator >= pThis->_GetTypeExtData()->GetMissileSaveAltitude(RulesClass::Instance)
		? 0x466EAD : 0x466EB6;
}


ASMJIT_PATCH(0x4666F7, BulletClass_AI_Trajectories, 0x6)
{
	enum { Detonate = 0x467E53 };

	GET(FakeBulletClass*, pThis, EBP);

	auto pExt = pThis->_GetExtData();
	auto pTypeExt  = pThis->_GetTypeExtData();

	auto& pTraj = pExt->Trajectory;

	if (!pThis->SpawnNextAnim && pTraj) {
		return pTraj->OnAI() ? Detonate : 0x0;
	}

	if (pExt->InterceptedStatus & InterceptedStatus::Targeted) {
		if (const auto pTarget = cast_to<BulletClass*>(pThis->Target)) {
			const auto pTargetTypeExt = BulletTypeExtContainer::Instance.Find(pTarget->Type);
			const auto pTargetExt = BulletExtContainer::Instance.Find(pTarget);

			if (!pTargetTypeExt->Armor.isset())
				pTargetExt->InterceptedStatus |= InterceptedStatus::Locked;
		}
	}

	if (pExt->InterceptedStatus & InterceptedStatus::Intercepted)
	{
		if (const auto pTarget = cast_to<BulletClass*>(pThis->Target))
			BulletExtContainer::Instance.Find(pTarget)->InterceptedStatus &= ~InterceptedStatus::Locked;

		if (BulletExtData::HandleBulletRemove(pThis, pExt->DetonateOnInterception, true))
			return 0x467FEE;
	}

	if (!pThis->IsAlive) {
        return 0x467FEE;
    }

	if (!PhobosTrajectory::BlockDrawTrail(pTraj)) {

		if(!pExt->LaserTrails.empty()) {
			CoordStruct futureCoords
			{
				pThis->Location.X + static_cast<int>(pThis->Velocity.X),
				pThis->Location.Y + static_cast<int>(pThis->Velocity.Y),
				pThis->Location.Z + static_cast<int>(pThis->Velocity.Z)
			};

			for (auto& trail : pExt->LaserTrails)
			{
				if (!trail->LastLocation.isset())
					trail->LastLocation = pThis->Location;

				trail->Update(futureCoords);
			}
		}

		/*TrailsManager::AI(pThis->_AsBullet());*/
	}

	if (pThis->HasParachute)
	{
		int fallRate = pExt->ParabombFallRate - pTypeExt->Parachuted_FallRate;
		int maxFallRate = pTypeExt->Parachuted_MaxFallRate.Get(RulesClass::Instance->ParachuteMaxFallRate);

		if (fallRate < maxFallRate)
			fallRate = maxFallRate;

		pExt->ParabombFallRate = fallRate;
		pThis->FallRate = fallRate;
	}

	return 0;
}

ASMJIT_PATCH(0x467AB2, BulletClass_AI_Parabomb, 0x7)
{
	GET(BulletClass*, pThis, EBP);

	if (pThis->HasParachute)
		return 0x467B1A;

	return 0;
}

ASMJIT_PATCH(0x467E53, BulletClass_AI_PreDetonation_Trajectories, 0x6)
{
	GET(FakeBulletClass*, pThis, EBP);

	if (auto& pTraj = pThis->_GetExtData()->Trajectory)
		pTraj->OnAIPreDetonate();

	return 0;
}

ASMJIT_PATCH(0x46745C, BulletClass_AI_Position_Trajectories, 0x7)
{
	GET(FakeBulletClass*, pThis, EBP);
	LEA_STACK(VelocityClass*, pSpeed, STACK_OFFS(0x1AC, 0x11C));
	LEA_STACK(VelocityClass*, pPosition, STACK_OFFS(0x1AC, 0x144));

	auto pExt =  pThis->_GetExtData();

	if (auto& pTraj = pExt->Trajectory)
		pTraj->OnAIVelocity(pSpeed, pPosition);


	// Trajectory can use Velocity only for turning Image's direction
	// The true position in the next frame will be calculate after here
	if (pExt->Trajectory) {

		if(!pExt->LaserTrails.empty()){
			CoordStruct futureCoords
			{
				static_cast<int>(pSpeed->X + pPosition->X),
				static_cast<int>(pSpeed->Y + pPosition->Y),
				static_cast<int>(pSpeed->Z + pPosition->Z)
			};
			for (auto& trail : pExt->LaserTrails)
			{
				if (!trail->LastLocation.isset())
					trail->LastLocation = pThis->Location;
				trail->Update(futureCoords);
			}
		}

		/*TrailsManager::AI(pThis->_AsBullet());*/
	}

	return 0;
}

ASMJIT_PATCH(0x4677D3, BulletClass_AI_TargetCoordCheck_Trajectories, 0x5)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x1A8, 0x184));

	return PhobosTrajectory::OnAITargetCoordCheck(pThis, coords);
}

ASMJIT_PATCH(0x467927, BulletClass_AI_TechnoCheck_Trajectories, 0x5)
{

	GET(BulletClass*, pThis, EBP);
	GET(TechnoClass*, pTechno, ESI);

	return PhobosTrajectory::OnAITechnoCheck(pThis, pTechno);
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
ASMJIT_PATCH(0x467E59, BulletClass_AI_NukeBall, 5)
{
	// changed the hardcoded way to just do this if the warhead is called NUKE
		// to a more universal approach. every warhead can get this behavior.
	GET(BulletClass* const, pThis, EBP);

	auto const pExt = BulletExtContainer::Instance.Find(pThis);
	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);

	enum { Default = 0u, FireNow = 0x467F9Bu, PreImpact = 0x467ED0 };

	auto allowFlash = true;
	// flashDuration = 0;

	// this is a bullet launched by a super weapon
	if (pExt->NukeSW && !pThis->WH->NukeMaker)
	{
		SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

		if (pThis->GetHeight() < 0)
		{
			pThis->SetHeight(0);
		}

		// cause yet another radar event
		auto const pSWTypeExt = SWTypeExtContainer::Instance.Find(pExt->NukeSW);

		if (pSWTypeExt->SW_RadarEvent)
		{
			auto const coords = pThis->GetMapCoords();
			RadarEventClass::Create(
				RadarEventType::SuperweaponActivated, coords);
		}

		if (pSWTypeExt->Lighting_Enabled.isset())
			allowFlash = pSWTypeExt->Lighting_Enabled.Get();
	}

	// does this create a flash?
	auto const duration = pWarheadExt->NukeFlashDuration.Get();

	if (allowFlash && duration > 0)
	{
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = duration;

		SWTypeExtData::ChangeLighting(pExt->NukeSW ? pExt->NukeSW : nullptr);
		MapClass::Instance->RedrawSidebar(1);
	}

	if (auto pPreImpact = pWarheadExt->PreImpactAnim.Get())
	{
		R->EDI(pPreImpact);
		return PreImpact;
	}

	return FireNow;
}

ASMJIT_PATCH(0x467B94, BulletClass_AI_Ranged, 7)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(bool, Destroy, 0x18);
	REF_STACK(CoordStruct, CrdNew, 0x24);

	// range check
	if (pThis->Type->Ranged)
	{
		CoordStruct crdOld = pThis->GetCoords();

		pThis->Range -= int(CrdNew.DistanceFrom(crdOld));
		if (pThis->Range <= 0)
		{
			Destroy = true;
		}
	}

	// replicate replaced instruction
	pThis->SetLocation(CrdNew);

	// firestorm wall check
	if (HouseExtContainer::Instance.IsAnyFirestormActive && !pThis->Type->IgnoresFirestorm)
	{
		auto const pCell = MapClass::Instance->GetCellAt(CrdNew);

		if (auto const pBld = pCell->GetBuilding())
		{
			HouseClass* pOwner = pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner;
			if (WarheadTypeExtContainer::Instance.Find(RulesExtData::Instance()->FirestormWarhead)->CanAffectHouse(pBld->Owner, pOwner))
				pOwner =  nullptr; // clear the pointer if can affect the bullet owner

			if (BuildingExtData::IsActiveFirestormWall(pBld, pOwner))
			{
				BuildingExtData::ImmolateVictim(pBld , pThis, false);
				BulletExtData::HandleBulletRemove(pThis, ScenarioClass::Instance->Random.RandomBool(), true);
				return 0x467FBA;
			}
		}
	}

	return 0x467BA4;
}


ASMJIT_PATCH(0x467C1C, BulletClass_AI_UnknownTimer, 0x6)
{
	GET(BulletTypeClass*, projectile, EAX);
	return projectile->Inviso ? 0x467C2A : 0;
}

ASMJIT_PATCH(0x467C2E, BulletClass_AI_FuseCheck, 0x7)
{
	GET(BulletClass*, pThis, EBP);
	GET(CoordStruct*, pCoord, ECX);

	R->EAX(BulletExtData::FuseCheckup(pThis, pCoord));

	return 0x467C3A;
}


ASMJIT_PATCH(0x466834, BulletClass_AI_TrailerAnim, 0x6)
{
	GET(BulletClass* const, pThis, EBP);
	const int delay = pThis->Type->ScaledSpawnDelay ? pThis->Type->ScaledSpawnDelay : pThis->Type->SpawnDelay;

	if (delay < 0)
		return 0x4668BD;

	if (!(Unsorted::CurrentFrame % delay))
	{

		auto const pExt = BulletExtContainer::Instance.Find(pThis);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pThis->Type->Trailer, pThis->Location, 1, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner) ? pExt->Owner : nullptr,
			pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis->Owner,
			false,
			false
		);
	}

	return 0x4668BD;
}

#endif