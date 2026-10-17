#include "Body.h"
#include "Trajectories/PhobosTrajectory.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/NuclearMissile.h>
#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <RadarEventClass.h>
#include <Ion.h>		
enum class BouncePhaseResult { ContinueVanilla, KeepFlying, ForceDetonate };

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
		DirStruct currentPitch = pVeloc->GetDirectionFromXY();
		DirStruct newPitch = currentPitch;
		DirStruct pitchTarget { 0x2000 };    // ORIG: literal 0x2000 @ 0x5B2182
		newPitch.Func_5B29C0(pitchTarget, *pInputDir);

		const double totalSpeed = pVeloc->Length();
		const double currentPitchRad = currentPitch.GetRad();

		if (currentPitchRad != 0.0)          // ORIG: fcomp FLOAT_0_0 @ 0x5B21FF
		{
			const double c = Math::cos(currentPitchRad);
			pVeloc->X /= c;
			pVeloc->Y /= c;
		}

		const double newPitchRad = newPitch.GetRad();
		const double cosNew = Math::cos(newPitchRad);
		const double sinNew = Math::sin(newPitchRad);

		pVeloc->X *= cosNew;
		pVeloc->Y *= cosNew;
		pVeloc->Z = sinNew * totalSpeed;

		pCoordTo->operator+=(pVeloc->ToCoord());

		return static_cast<int>(pVeloc->Length());
	}

	// ── BRANCH B: homing toward pCoordFrom ─────────────────────────────
	pCoordTo->operator+=(pVeloc->ToCoord());

	const int distance3D = static_cast<int>((pCoordTo->operator-(*pCoordFrom)).Length());
	const Point2D pTo2D { pCoordTo->X,   pCoordTo->Y };
	const Point2D pFrom2D { pCoordFrom->X, pCoordFrom->Y };
	const int distance2D = static_cast<int>((pTo2D - pFrom2D).Length());

	CoordStruct deltaToTarget = pCoordFrom ->operator-(*pCoordTo);

	VelocityClass vToTarget = VelocityClass::CoordToVel(deltaToTarget);

	// ── Yaw correction ────────────────────────────────────────────────
	DirStruct currentHeading { -pVeloc->Y,    pVeloc->X };
	DirStruct targetHeading { -vToTarget.Y, vToTarget.X };

	DirStruct newHeading = currentHeading;
	newHeading.Func_5B29C0(targetHeading, *pInputDir);

	pVeloc->SetXIfZeroXY();

	const double horizontalSpeed = pVeloc->LengthXY();
	const double newHeadingRad = newHeading.GetRad();

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
	pVeloc->SetPitch(&newPitch);

	// ORIG @ 0x5B28F8-0x5B290F: airburst → Z=0, else sar 2 (signed /4)
	if (airburst)
		deltaToTarget.Z = 0;
	else
		deltaToTarget.Z /= 4;

	// ORIG: CoordStruct__Distance @ 0x5B2917
	return static_cast<int>(deltaToTarget.Length());
}

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

//#pragma optimize("", off )


#define CXYZ(c) (c).X, (c).Y, (c).Z

void FakeBulletClass::_AI()
{
	this->ObjectClass::Update();

	if (!this->IsAlive)
		return;

	auto pExt = this->_GetExtData();
	auto pTypeExt = this->_GetTypeExtData();

	// ------------------------------------------------------------------------
	//  Debug helper: dump every coordinate flavour of the current target
	// ------------------------------------------------------------------------
	auto DumpTarget = [this](const char* tag)
		{
			if (!this->Target)
			{
				Debug::LogInfo("[B{:X} f{}]  {} target=null TargetCoords=({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame() , tag, CXYZ(this->TargetCoords));
				return;
			}

			const CoordStruct getCoords = this->Target->GetCoords();
			const CoordStruct center = this->Target->GetCenterCoords();
			CoordStruct moving = getCoords;
			if (auto pObj = flag_cast_to<ObjectClass*>(this->Target))
				moving = pObj->GetTargetCoords();

			Debug::LogInfo("[B{:X} f{}]  {} target={:X} what={} TargetCoords=({},{},{}) GetCoords=({},{},{}) Center=({},{},{}) TargetCoordsFn=({},{},{})",
				reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				tag, reinterpret_cast<std::uintptr_t>(this->Target),
				static_cast<int>(this->Target->WhatAmI()),
				CXYZ(this->TargetCoords), CXYZ(getCoords), CXYZ(center), CXYZ(moving));
		};

	auto OnEarlyTrajectoryUpdate = [this, pExt]()
		{
			bool detonate = false;

			if (pExt->Trajectory)
			{
				detonate = pExt->Trajectory->OnAI();
			}

			return detonate && !this->SpawnNextAnim;
		};

	// ========================================================================
	//  Detonation tail  (0x467E53)
	// ========================================================================
	auto ApplyNukeWarhead = [this, pExt, DumpTarget](const char* tag, bool destroy, const CoordStruct& coord)
		{
			//Debug::LogInfo("[B{:X} f{}]  DETONATE via={} destroy={} working=({},{},{}) Location=({},{},{}) height={} speed={:.2f} distTraveled={:.2f} range={}",
			//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	tag, destroy, CXYZ(coord), CXYZ(this->Location), this->GetHeight(),
			//	this->Velocity.Length(), pExt->DistanceTraveled, this->Range);
			//DumpTarget("DETONATE target");

			//if (this->Target)
			//{
				//const CoordStruct center = this->Target->GetCenterCoords();
				//Debug::LogInfo("[B{:X} f{}]  DETONATE miss: Location->Center={:.1f} working->Center={:.1f}",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	this->Location.DistanceFrom(center), coord.DistanceFrom(center));
			//}

			// Hook 0x467E53 — PreDetonation_Trajectories
			if (auto& pTraj = pExt->Trajectory)
				pTraj->OnAIPreDetonate();

			// Hook 0x467E59 — NukeBall
			auto pWarheadExt = WarheadTypeExtContainer::Instance.Find(this->WH);
			bool allowFlash = true;

			if (pExt->NukeSW && !this->WH->NukeMaker)
			{
				SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

				if (this->GetHeight() < 0)
					this->SetHeight(0);

				auto pSWTypeExt = SWTypeExtContainer::Instance.Find(pExt->NukeSW);

				if (pSWTypeExt->SW_RadarEvent)
				{
					auto coords = this->GetMapCoords();
					RadarEventClass::Create(RadarEventType::SuperweaponActivated, coords);
				}

				if (pSWTypeExt->Lighting_Enabled.isset())
					allowFlash = pSWTypeExt->Lighting_Enabled.Fetch();
			}

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

			if (auto pPreImpact = pWarheadExt->PreImpactAnim.Get())
			{
				//Debug::LogInfo("[B{:X} f{}]  DETONATE PreImpactAnim present -> bullet will park at Location=({},{},{})",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	CXYZ(this->Location));

				CoordStruct anim_coord = this->Location;
				int zadjust = -15;

				AnimClass* pNewAnim = GameCreate<AnimClass>(
					pPreImpact, &anim_coord,
					/*timedelay*/ 0, /*loop*/ 1,
					/*animflags*/ AnimFlag::AnimFlag_2000
								| AnimFlag::AnimFlag_400
								| AnimFlag::AnimFlag_200,
					/*zadjust*/ zadjust, /*reverse*/ 0);

				if (pNewAnim)
				{
					AnimExtData::SetAnimOwnerHouseKind(pNewAnim, pExt->Owner, nullptr, false);
					this->NextAnim = pNewAnim;
					this->SpawnNextAnim = true;
					PointerExpiredNotification::NotifyInvalidAnim->Array.push_back(this);
					this->LastMapCoords = CellClass::Coord2Cell(coord);
					return;
				}
				// Fall through to normal explode if anim creation failed
			}

			this->Explode(destroy);
			this->UnInit();
			this->LastMapCoords = CellClass::Coord2Cell(coord);
		};

	// ========================================================================
	//  Shared tail  (0x467B7A .. 0x467FBA)
	// ========================================================================
	auto MarkAndContinue = [this, pExt, ApplyNukeWarhead, DumpTarget]
	(const char* tag, CoordStruct& coord, bool& destroy, int& markType, bool& snapToTarget)
		{
			//Debug::LogInfo("[B{:X} f{}]  TAIL enter tag={} coord=({},{},{}) Location=({},{},{}) destroy={} markType={} snapTo={}",
			//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	tag, CXYZ(coord), CXYZ(this->Location), destroy, markType, snapToTarget);

			if (markType == 2)
			{
				//Debug::LogInfo("[B{:X} f{}]  TAIL markType==2 -> silent UnInit (left map)", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				this->Mark(MarkType::Change);
				this->UnInit();
				this->LastMapCoords = CellClass::Coord2Cell(coord);
				return;
			}

			this->Mark(MarkType::Change);

			// Hook 0x467B94 — Ranged + Firestorm
			if (this->Type->Ranged)
			{
				CoordStruct crdOld = this->GetCoords();

				int maxRange = this->Range;
				bool useAresBehavior = true;

				if (this->WeaponType)
				{
					auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(this->WeaponType);

					useAresBehavior = pWeaponExt->ProjectileRange_UseAresBehaviour;

					if (!useAresBehavior && maxRange > 0 && this->Owner &&
						pWeaponExt->ProjectileRange_ApplyModifiers
						.Get(FakeRulesClass::Instance->ProjectileRange_ApplyModifiers))
					{
						maxRange = WeaponTypeExtData::GetRangeWithModifiers(this->WeaponType, this->Owner, maxRange);
					}
				}

				auto distance = coord.DistanceFrom(crdOld);
				const bool destroyBefore = destroy;

				if (!useAresBehavior)
				{
					pExt->DistanceTraveled += distance;
					destroy |= pExt->DistanceTraveled >= (double)maxRange;
				}
				else
				{
					this->Range -= static_cast<int>(distance);
					if (this->Range <= 0)
						destroy = true;
				}

				//Debug::LogInfo("[B{:X} f{}]  TAIL ranged: aresMode={} maxRange={} step={:.2f} distTraveled={:.2f} Range={} destroy {}->{}",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	useAresBehavior, maxRange, distance, pExt->DistanceTraveled, this->Range, destroyBefore, destroy);
			}

			this->SetLocation(coord);

			if (HouseExtContainer::Instance.IsAnyFirestormActive && !this->Type->IgnoresFirestorm)
			{
				auto pCell = MapClass::Instance->GetCellAt(coord);

				if (auto pBld = pCell->GetBuilding())
				{
					HouseClass* pOwner = this->Owner
						? this->Owner->Owner
						: pExt->Owner;

					if (WarheadTypeExtContainer::Instance
						.Find(FakeRulesClass::Instance()->FirestormWarhead.Get(
							RulesClass::Instance->C4Warhead))
						->CanAffectHouse(pBld->Owner, pOwner))
					{
						pOwner = nullptr;
					}

					if (BuildingExtData::IsActiveFirestormWall(pBld, pOwner))
					{
						//Debug::LogInfo("[B{:X} f{}]  TAIL firestorm wall hit -> removed", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
						BuildingExtData::ImmolateVictim(pBld, this, false);
						BulletExtData::HandleBulletRemove(this,
							ScenarioClass::Instance->Random.RandomBool(), true);

						this->LastMapCoords = CellClass::Coord2Cell(coord);
						return;
					}
				}
			}

			bool ObstacleHandled = false;

			if (auto& pTraj = pExt->Trajectory)
			{
				if (pTraj->OnHandleObstacleCheck())
					ObstacleHandled = true;
			}

			if (!ObstacleHandled)
			{
				if (!destroy)
				{
					CoordStruct probe = this->Location;
					destroy = this->IsForceToExplode(probe);
					//Debug::LogInfo("[B{:X} f{}]  TAIL IsForceToExplode -> {} probe=({},{},{}) Location=({},{},{})",
					//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
					//	destroy, CXYZ(probe), CXYZ(this->Location));
					this->SetLocation(probe);
				}

				if (destroy && this->GetHeight() < 0)
				{
					//Debug::LogInfo("[B{:X} f{}]  TAIL height<0 -> SetHeight(0)", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
					this->SetHeight(0);
				}
			}

			// Hook 0x467C2E — FuseCheck / Hook 0x467C1C — UnknownTimer
			Fuse fuse_result = Fuse::DontIgnite;

			const bool fuseRan = (!pExt->Trajectory || !pExt->Trajectory->ShouldSkipRangedCheck())
				&& (this->Type->ROT > 0 || this->Type->Ranged || this->Type->Inviso);

			if (fuseRan)
			{
				if (this->Target)
				{
					//Debug::LogInfo("[B{:X} f{}]  FUSE pre: coord->Center={:.1f} coord->GetCoords={:.1f} Arm={} ",
					//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
					//	coord.DistanceFrom(this->Target->GetCenterCoords()),
					//	coord.DistanceFrom(this->Target->GetCoords()),
					//	this->Type->Arm);
				}

				fuse_result = static_cast<Fuse>(
					BulletExtData::FuseCheckup(this, &coord));
			}

			//Debug::LogInfo("[B{:X} f{}]  FUSE ran={} result={} (0=DontIgnite 1=Ignite 2=DistFactor) coord=({},{},{})",
			//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	fuseRan, static_cast<int>(fuse_result), CXYZ(coord));

			if (this->Owner
				&& this->Owner->GetTechnoType()->JumpJet
				&& (fuse_result == Fuse::Ignite_DistaceFactor))
			{
				//Debug::LogInfo("[B{:X} f{}]  FUSE jumpjet override 2->1", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				fuse_result = Fuse::Ignite;
			}

			if (!destroy && (this->Type->Dropping || fuse_result == Fuse::DontIgnite))
			{
				//Debug::LogInfo("[B{:X} f{}]  END: still flying (Dropping={} fuse={}) coord=({},{},{})",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	this->Type->Dropping, static_cast<int>(fuse_result), CXYZ(coord));

				if (!this->Type->Inviso && this->Type->Degenerates && this->Health > 5)
					--this->Health;

				this->LastMapCoords = CellClass::Coord2Cell(coord);
				return;
			}

			//Debug::LogInfo("[B{:X} f{}]  TAIL proceeding to detonation: destroy={} fuse={} snapTo={}",
			//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	destroy, static_cast<int>(fuse_result), snapToTarget);

			// Hook 0x467CCA — TargetSnapChecks
			if (this->Target && (fuse_result == Fuse::Ignite || snapToTarget))
			{
				bool snap_allowed = !this->Type->Airburst;

				if (this->Type->Inviso)
				{
					snap_allowed = !this->Type->Inaccurate;
				}
				else if (pExt->Trajectory)
				{
					snap_allowed = PhobosTrajectory::CanSnap(pExt->Trajectory);
				}

				//Debug::LogInfo("[B{:X} f{}]  SNAP check: snap_allowed={} Inaccurate={} Airburst={}",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	snap_allowed, this->Type->Inaccurate, this->Type->Airburst);

				if (snap_allowed && !this->Type->Inaccurate)
				{
					const CoordStruct target_coord = this->Target->GetCenterCoords();
					const CoordStruct delta {
						coord.X - target_coord.X,
						coord.Y - target_coord.Y,
						(target_coord.Z + coord.Z) / 2 - target_coord.Z
					};

					const int dist = static_cast<int>(delta.Length());
					const int dist_threshold = snapToTarget ? (dist / 3) : dist;
					const double speed_mag = this->Velocity.Length();
					const double snap_radius = (speed_mag * 2.0 >= 128.0)
						? speed_mag * 2.0
						: 128.0;

					const bool doSnap = (fuse_result == Fuse::Ignite || dist_threshold <= snap_radius);

					//Debug::LogInfo("[B{:X} f{}]  SNAP calc: distTarget(+Center)=({},{},{}) delta=({},{},{}) dist={} thr={} radius={:.1f} -> doSnap={}",
						//reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
					//	CXYZ(target_coord), CXYZ(delta), dist, dist_threshold, snap_radius, doSnap);

					if (doSnap)
					{
						CoordStruct center = this->Target->GetCoords();
						//Debug::LogInfo("[B{:X} f{}]  SNAP applying SetLocation(GetCoords)=({},{},{}) (Location was ({},{},{}))",
						//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
						//	CXYZ(center), CXYZ(this->Location));
						this->SetLocation(center);
					}
				}
			}
			else
			{
				//Debug::LogInfo("[B{:X} f{}]  SNAP skipped: Target={} fuse={} snapTo={}",
				//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	this->Target != nullptr, static_cast<int>(fuse_result), snapToTarget);
			}

			//Debug::LogInfo("[B{:X} f{}]  TAIL final Location=({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), CXYZ(this->Location));
			ApplyNukeWarhead(tag, destroy, coord);
		};

	if (!OnEarlyTrajectoryUpdate())
	{
		// Targeted -> Locked status promotion (intercept book-keeping)
		if (pExt->InterceptedStatus & InterceptedStatus::Targeted)
		{
			if (auto pTargetBullet = cast_to<BulletClass*>(this->Target))
			{
				auto pTgtTypeExt = BulletTypeExtContainer::Instance.Find(pTargetBullet->Type);
				auto pTgtExt = BulletExtContainer::Instance.Find(pTargetBullet);
				if (!pTgtTypeExt->Armor.isset())
					pTgtExt->InterceptedStatus |= InterceptedStatus::Locked;
			}
		}

		if (pExt->InterceptedStatus & InterceptedStatus::Intercepted)
		{
			if (auto pTargetBullet = cast_to<BulletClass*>(this->Target))
				BulletExtContainer::Instance.Find(pTargetBullet)->InterceptedStatus
				&= ~InterceptedStatus::Locked;

			if (BulletExtData::HandleBulletRemove(this, pExt->DetonateOnInterception, true))
			{
				//Debug::LogInfo("[B{:X} f{}]  END: removed by interception", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				return;
			}
		}

		if (this->HasParachute)
		{
			int fallRate = pExt->ParabombFallRate - pTypeExt->Parachuted_FallRate;
			int maxFallRate = pTypeExt->Parachuted_MaxFallRate.Get(RulesClass::Instance->ParachuteMaxFallRate);

			if (fallRate < maxFallRate)
				fallRate = maxFallRate;

			pExt->ParabombFallRate = fallRate;
			this->FallRate = fallRate;
		}

		// Hook 0x466705 — BulletClass_AI
		auto pBulletCurOwner = this->GetOwningHouse();
		bool ownerChanged = false;

		if (this->Owner && pBulletCurOwner && pBulletCurOwner != pExt->Owner)
		{
			ownerChanged = true;
			pExt->Owner = pBulletCurOwner;
		}

		if (this->WeaponType && this->WH)
		{
			if (!pExt->BrightCheckDone)
			{
				this->Bright = this->WeaponType->Bright || this->WH->Bright;
				pExt->BrightCheckDone = true;
			}
		}

		if (pTypeExt->PreExplodeRange.isset())
		{
			const auto thisCoord = this->GetCoords();
			const auto targetCoord = this->GetBulletTargetCoords();

			//Debug::LogInfo("[B{:X} f{}]  PreExplodeRange dist={:.1f} limit={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), thisCoord.DistanceFrom(targetCoord),
			//	pTypeExt->PreExplodeRange.Fetch() * 256);

			if (Math::abs(thisCoord.DistanceFrom(targetCoord))
				<= pTypeExt->PreExplodeRange.Fetch() * 256)
			{
				if (BulletExtData::HandleBulletRemove(this, true, true))
				{
					//Debug::LogInfo("[B{:X} f{}]  END: PreExplodeRange removal", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
					return;
				}
			}
		}

		// Non-trajectory LaserTrails path
		{
			if (!pExt->LaserTrails.empty())
			{
				const CoordStruct& location = this->Location;
				const VelocityClass& velocity = this->Velocity;

				CoordStruct drawnCoords {
					static_cast<int>(location.X + velocity.X),
					static_cast<int>(location.Y + velocity.Y),
					static_cast<int>(location.Z + velocity.Z)
				};

				for (auto& trail : pExt->LaserTrails)
				{
					if (!trail->LastLocation.isset())
						trail->LastLocation = location;

					if (trail->Type->IsHouseColor.Get() && ownerChanged && pExt->Owner)
						trail->CurrentColor = pExt->Owner->LaserColor;

					trail->Update(drawnCoords);
				}
			}
		}

		bool destroyed = false;
		bool snapToTarget = false;

		if (this->SpawnNextAnim)
		{
			if (this->NextAnim)
			{
				auto pWHExt = WarheadTypeExtContainer::Instance.Find(this->WH);

				if (pWHExt->PreImpact_Moves.Get())
				{
					auto coords = this->NextAnim->GetCoords();
					this->Location = coords;
					this->Target = MapClass::Instance->TryGetCellAt(coords);
				}
				//Debug::LogInfo("[B{:X} f{}]  END: parked on PreImpact anim, Location=({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), CXYZ(this->Location));
				return;  // bullet sleeps another tick
			}

			//Debug::LogInfo("[B{:X} f{}]  pending anim finished -> Explode(false) at Location=({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), CXYZ(this->Location));
			PointerExpiredNotification::NotifyInvalidAnim->Array.erase(this);
			this->SpawnNextAnim = false;
			this->Explode(false);
			this->UnInit();
			return;
		}

		// ====================================================================
		//  Normal flight
		// ====================================================================
		if (this->Type->Dropping && !this->IsFallingDown)
			destroyed = true;

		if (this->Type->AnimLow || this->Type->AnimHigh)
		{
			const auto decreased = --this->AnimRateCounter;

			if (!decreased)
			{
				this->AnimRateCounter = this->Type->AnimRate;
				if (++this->AnimFrame > this->Type->AnimHigh)
					this->AnimFrame = this->Type->AnimLow;
			}
		}

		if (this->Type->Trailer)
		{
			// Hook 0x466834 — BulletClass_AI_TrailerAnim
			const int delay = this->Type->ScaledSpawnDelay ?
				this->Type->ScaledSpawnDelay : this->Type->SpawnDelay;

			if (delay > 0)
			{
				if ((Unsorted::CurrentFrame.get() % delay) == 0)
				{
					auto pAnim = GameCreate<AnimClass>(
						this->Type->Trailer,
						this->Location,
						/*timedelay*/ 1,
						/*loop*/ 1,
						/*animflags*/ AnimFlag::AnimFlag_600,
						/*zadjust*/ 0,
						/*reverse*/ false);

					AnimExtData::SetAnimOwnerHouseKind(
						pAnim,
						this->Owner ? this->Owner->GetOwningHouse()
									  : (pExt->Owner ? pExt->Owner : nullptr),
						this->Target ? this->Target->GetOwningHouse() : nullptr,
						this->Owner,
						false,
						false);
				}
			}
		}

		CoordStruct _thisCoord = this->Location;
		int markType = 0;

		//Debug::LogInfo("[B{:X} f{}]  FRAME Location=({},{},{}) Vel=({:.3f},{:.3f},{:.3f}) |Vel|={:.2f} Speed={} height={} ROT={} Vertical={} Ranged={} Inviso={} Airburst={} VeryHigh={} Level={} CourseLock={} Dropping={}",
		//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
		//	CXYZ(this->Location), this->Velocity.X, this->Velocity.Y, this->Velocity.Z,
		//	this->Velocity.Length(), this->Speed, this->GetHeight(),
		//	this->Type->ROT, this->Type->Vertical, this->Type->Ranged, this->Type->Inviso,
		//	this->Type->Airburst, this->Type->VeryHigh, this->Type->Level,
		//	this->CourseLock, this->Type->Dropping);
		//DumpTarget("FRAME");

		// ====================================================================
		//  Homing (ROT > 0)
		// ====================================================================
		if (this->Type->ROT > 0)
		{
			// Hook 0x4668BD — Inviso interceptors detonate immediately
			if (this->Type->Inviso && this->_GetExtData()->InterceptorTechnoType)
			{
				//Debug::LogInfo("[B{:X} f{}]  END: inviso interceptor detonation", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				this->Explode(destroyed);
				this->UnInit();
				this->LastMapCoords = CellClass::Coord2Cell(_thisCoord);
				return;
			}

			// ---------- (a) course-lock gate -------------------------------
			double current_speed = this->Velocity.Length();
			const double speed_target = static_cast<double>(this->Speed);
			const bool lockBefore = this->CourseLock;

			if (this->Type->CourseLockDuration > 0)
			{
				if (this->CourseLockCounter < this->Type->CourseLockDuration)
				{
					if (++this->CourseLockCounter >= this->Type->CourseLockDuration)
						this->CourseLock = false;
				}
			}
			else if (this->Speed >= 40 || speed_target <= current_speed + 0.5)
			{
				this->CourseLock = false;
			}

			//Debug::LogInfo("[B{:X} f{}]  ROT(a) courseLock {}->{} lockDur={} lockCounter={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	lockBefore, this->CourseLock, this->Type->CourseLockDuration, this->CourseLockCounter);

			// ---------- (b) acceleration value -----------------------------
			int accel_value = this->Type->Acceleration;
			if (this->CourseLock && this->Type->CourseLockDuration == 0)
				accel_value = (Unsorted::CurrentFrame() % 2 == 0) ? 1 : 0;

			// ---------- (c) speed adjust + rescale -------------------------
			auto RescaleVelocity = [this](double speed)
				{
					this->Velocity.SetXIfZeroXYZ();
					const double k = speed / this->Velocity.Length();
					this->Velocity *= k;
				};

			const double speedBefore = current_speed;

			if (current_speed < speed_target)
			{
				current_speed += accel_value;
				if (current_speed >= speed_target)
					current_speed = speed_target;

				RescaleVelocity(current_speed);
			}
			else if (current_speed > speed_target)
			{
				current_speed -= static_cast<double>(accel_value / 2);
				if (current_speed <= 0.0)
					current_speed = 0.0;

				RescaleVelocity(current_speed);
			}

			//Debug::LogInfo("[B{:X} f{}]  ROT(c) accel={} speed {:.2f}->{:.2f} target={:.1f} |Vel|now={:.2f}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	accel_value, speedBefore, current_speed, speed_target, this->Velocity.Length());

			// ---------- (d) homing turn ------------------------------------
			auto GetTargetCoords = [this, pTypeExt]() -> CoordStruct
				{
					if (pTypeExt->MissileKeepTargetCoord.Get(FakeRulesClass::Instance()->MissileKeepTargetCoord))
					{
						if (!this->Target)
						{
							return this->TargetCoords;
						}
					}

					if (auto pObject = flag_cast_to<ObjectClass*>(this->Target))
						return pObject->GetTargetCoords();

					return this->Target ? this->Target->GetCenterCoords() : CoordStruct::Empty;
				};

			CoordStruct pCoordTarget = GetTargetCoords();
			const bool aimIsDefault = (pCoordTarget == CoordStruct::Empty);

			//Debug::LogInfo("[B{:X} f{}]  ROT(d) aim=({},{},{}) aimIsDefault={} keepTargetCoord={} isObjectFlag={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	CXYZ(pCoordTarget), aimIsDefault,
			//	pTypeExt->MissileKeepTargetCoord.Get(FakeRulesClass::Instance()->MissileKeepTargetCoord),
			//	flag_cast_to<ObjectClass*>(this->Target) != nullptr);

			// Hook 0x466BAF — MissileROTVar (type-ext override)
			const int    nFrame = (Unsorted::CurrentFrame.get() + this->Fetch_ID()) % 15;
			const double nMissileROTVar = pTypeExt->MissileROTVar.Get(RulesClass::Instance->MissileROTVar);

			int rot_amount = static_cast<int>(
				(Math::sin(static_cast<double>(nFrame)
					* Math::ONE_FIFTEENTH
					* Math::GAME_TWOPI) * nMissileROTVar
					+ nMissileROTVar + 1.0)
				* static_cast<double>(this->Type->ROT));

			const CoordStruct pCenter = this->GetCoords();
			const CoordStruct homing_offset = pCenter - pCoordTarget;
			const int homingDist = static_cast<int>(homing_offset.Length());

			if (homingDist < 256)
				rot_amount = static_cast<int>(rot_amount * 1.5);

			const CoordStruct pre_motion_coord = _thisCoord;

			const bool target_is_aircraft = cast_to<AircraftClass*>(this->Target) != nullptr;

			DirStruct dir { };
			if (!this->CourseLock)
				dir.Raw = static_cast<unsigned short>((rot_amount & 0xFF) << 8);

			//Debug::LogInfo("[B{:X} f{}]  ROT(d) nFrame={} rotVar={:.3f} rot={} (x1.5 if <256) center=({},{},{}) homingDist={} aircraftTarget={} dirRaw={}",
			//	reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	nFrame, nMissileROTVar, rot_amount, CXYZ(pCenter), homingDist, target_is_aircraft, dir.Raw);

			const int new_height = missile_maths(&_thisCoord, &this->Velocity, &pCoordTarget, &dir,
				target_is_aircraft,
				this->Type->Airburst,
				this->Type->VeryHigh,
				this->Type->Level);

			//Debug::LogInfo("[B{:X} f{}]  ROT(d) missile_maths ret={} coord ({},{},{})->({},{},{}) Vel=({:.3f},{:.3f},{:.3f}) aimAfter=({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	new_height, CXYZ(pre_motion_coord), CXYZ(_thisCoord),
			//	this->Velocity.X, this->Velocity.Y, this->Velocity.Z, CXYZ(pCoordTarget));

			auto pCellAfter = MapClass::Instance->GetCellAt(_thisCoord);

			// ---------- (e) altitude safety overrides ----------------------
			const double speed_after = this->Velocity.Length();
			const int cur_height = this->GetHeight();

			const bool condA = (speed_after * 0.5 >= new_height);
			const bool condB = (cur_height <= 0);

			//Debug::LogInfo("[B{:X} f{}]  ROT(e) speedAfter={:.2f} half={:.2f} new_height={} condA(half>=new_height)={} cur_height={} condB(height<=0)={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	speed_after, speed_after * 0.5, new_height, condA, cur_height, condB);

			if (condA || condB)
			{
				destroyed = true;
				markType = 1;

				//Debug::LogInfo("[B{:X} f{}]  ROT(e) IMPACT FLAGGED by {}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), condA ? "condA (speed/2 >= motion)" : "condB (height<=0)");

				if (cur_height > 0
					&& !this->Type->Airburst
					&& !aimIsDefault)
				{
					//Debug::LogInfo("[B{:X} f{}]  ROT(e) coord snapped to aim ({},{},{})", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), CXYZ(pCoordTarget));
					_thisCoord = pCoordTarget;
				}
			}

			// Hook 0x466E9F — MissileSafetyAltitude (type-ext override)
			if (aimIsDefault)
			{
				//Debug::LogInfo("[B{:X} f{}]  ROT(e) aim is default; safetyAlt={} cur_height={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
				//	pTypeExt->GetMissileSaveAltitude(RulesClass::Instance()), cur_height);

				if (cur_height >= pTypeExt->GetMissileSaveAltitude(RulesClass::Instance()))
				{
					//Debug::LogInfo("[B{:X} f{}]  ROT(e) IMPACT FLAGGED by safety altitude", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
					destroyed = true;
					markType = 1;
				}
			}

			// ---------- (f) smoothed "no progress" trigger -----------------
			const int dist_pre = static_cast<int>((pre_motion_coord - pCoordTarget).Length());
			const int dist_post = static_cast<int>((_thisCoord - pCoordTarget).Length());
			const int dist_delta = dist_pre - dist_post;

			//Debug::LogInfo("[B{:X} f{}]  ROT(f) dist_pre={} dist_post={} delta={} CourseLock={} trendCount={} smoothed={:.2f}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	dist_pre, dist_post, dist_delta, this->CourseLock, this->TrendSampleCount,
			//	this->SmoothedDistanceDelta);

			if (!this->CourseLock)
			{
				if (this->TrendSampleCount < 60)
				{
					++this->TrendSampleCount;
					this->SmoothedDistanceDelta += dist_delta;
				}
				else
				{
					this->SmoothedDistanceDelta = this->SmoothedDistanceDelta
						* Math::ProximityFuseDecayFactor + dist_delta;

					//Debug::LogInfo("[B{:X} f{}]  ROT(f) smoothed now {:.2f}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), this->SmoothedDistanceDelta);

					if (this->SmoothedDistanceDelta >= 0.0 && this->SmoothedDistanceDelta < 60.0)
					{
						if (!this->Type->Airburst && !this->Type->VeryHigh)
						{
							//Debug::LogInfo("[B{:X} f{}]  ROT(f) STALL TRIGGER", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
							destroyed = true;
							markType = 1;
							MarkAndContinue("ROT-stall", _thisCoord, destroyed, markType, snapToTarget);
							return;
						}
					}
				}
			}

			// ---------- (g) wall-top Z crossing ----------------------------
			if (markType == 0
				&& (pCellAfter->ContainsBridgeHead()
					|| MapClass::Instance->GetCellAt(pre_motion_coord)->ContainsBridgeHead()))
			{
				const int wall_top = MapClass::Instance->GetCellFloorHeight(_thisCoord) + CellClass::BridgeHeight;

				//Debug::LogInfo("[B{:X} f{}]  ROT(g) bridge cell: wall_top={} z {}->{}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), wall_top, pre_motion_coord.Z, _thisCoord.Z);

				if ((_thisCoord.Z > wall_top && pre_motion_coord.Z < wall_top)
					|| (_thisCoord.Z < wall_top && pre_motion_coord.Z > wall_top))
				{
					//Debug::LogInfo("[B{:X} f{}]  ROT(g) IMPACT FLAGGED by wall-top crossing", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
					markType = 1;
					_thisCoord.Z = wall_top;
					destroyed = true;
				}
			}

			MarkAndContinue("ROT-end", _thisCoord, destroyed, markType, snapToTarget);
			return; // end ROT projectile update
		}

		// ====================================================================
		//  Non-homing: shared target / techno proximity (0x4677D3)
		// ====================================================================
		VelocityClass vel_copy = this->Velocity;
		const double vel_len = vel_copy.Length();

		auto CheckBuildingCell = [this, pExt, &destroyed, &markType, &snapToTarget, MarkAndContinue, ApplyNukeWarhead]
		(CoordStruct& coords, VelocityClass& veloc)
			{
				//Debug::LogInfo("[B{:X} f{}]  CBC enter coords=({},{},{}) destroyed={} markType={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), CXYZ(coords), destroyed, markType);

				// Hook 0x4677D3 — TargetCoordCheck_Trajectories
				bool skip_building_check = false;
				if (auto& pTraj = pExt->Trajectory)
				{
					switch (pTraj->OnAITargetCoordCheck(coords))
					{
					case TrajectoryCheckReturnType::SkipGameCheck: //0x4678F8
					{
						skip_building_check = true;
						break;
					}
					case TrajectoryCheckReturnType::SatisfyGameCheck: //0x467879
					{
						markType = 1;
						destroyed = true;
						snapToTarget = true;
						MarkAndContinue("CBC-traj-satisfy", coords, destroyed, markType, snapToTarget);
						return;
					}
					case TrajectoryCheckReturnType::Detonate: //0x467E53
					{
						ApplyNukeWarhead("CBC-traj-detonate", destroyed, coords);
						return;
					}
					default:
						break;
					}
				}

				// ---------- (a) target cell / building footprint -------------
				if (!skip_building_check)
				{
					const CellStruct cell_now = CellClass::Coord2Cell(coords);
					const CellStruct cell_target = CellClass::Coord2Cell(this->TargetCoords);

					const bool target_cell_match = (cell_now.X == cell_target.X)
						&& (cell_now.Y == cell_target.Y)
						&& !this->Type->Vertical
						&& this->GetHeight() < 2 * Unsorted::LevelHeight;

					//Debug::LogInfo("[B{:X} f{}]  CBC(a) cellNow=({},{}) cellTarget=({},{}) height={} cap={} match={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
					//	cell_now.X, cell_now.Y, cell_target.X, cell_target.Y,
					//	this->GetHeight(), 2 * Unsorted::LevelHeight, target_cell_match);

					if (target_cell_match)
					{
						markType = 1;
						destroyed = true;
						snapToTarget = true;
						MarkAndContinue("CBC-target-cell", coords, destroyed, markType, snapToTarget);
						return;
					}

					auto pBldNow = MapClass::Instance->GetCellAt(cell_now)->GetBuilding();
					auto pBldTarget = MapClass::Instance->GetCellAt(cell_target)->GetBuilding();

					const bool building_footprint_hit = pBldNow
						&& !this->Type->Vertical
						&& pBldNow == pBldTarget
						&& this->GetHeight() < 2 * Unsorted::LevelHeight;

					if (building_footprint_hit)
					{
						markType = 1;
						destroyed = true;
						snapToTarget = true;
						MarkAndContinue("CBC-building-footprint", coords, destroyed, markType, snapToTarget);
						return;
					}
				}

				// ---------- (b) techno-on-cell / in-map / ground-hug ---------
				auto FinishFlight = [this, &markType, &destroyed, &snapToTarget, MarkAndContinue]
				(CoordStruct& c, VelocityClass& v)
					{
						if (!MapClass::Instance->IsValid(c))
						{
							//Debug::LogInfo("[B{:X} f{}]  CBC(b) coord outside map -> markType=2", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
							markType = 2;
							destroyed = true;
							c = this->Location;
							MarkAndContinue("CBC-out-of-map", c, destroyed, markType, snapToTarget);
							return;
						}

						// Hook 0x467AB2 — Parabomb
						if (!this->Type->Vertical && !this->HasParachute)
							this->Velocity = v;

						//Debug::LogInfo("[B{:X} f{}]  CBC(b) stored Vel=({:.3f},{:.3f},{:.3f}) |Vel|={:.2f} height={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
						//	this->Velocity.X, this->Velocity.Y, this->Velocity.Z,
						//	this->Velocity.Length(), this->GetHeight());

						if (this->Velocity.Length() < 10.0 && this->GetHeight() < 10)
						{
							//Debug::LogInfo("[B{:X} f{}]  CBC(b) IMPACT FLAGGED: slow + low", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
							markType = 1;
							destroyed = true;
						}

						MarkAndContinue("CBC-finish", c, destroyed, markType, snapToTarget);
					};

				auto pCellAtCoord = MapClass::Instance->GetCellAt(coords);
				auto pTechnoOnCell = pCellAtCoord->FindTechnoNearestTo(Point2D::Empty, false, nullptr);

				//Debug::LogInfo("[B{:X} f{}]  CBC(b) technoOnCell={:X}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), reinterpret_cast<std::uintptr_t>(pTechnoOnCell));

				// Hook 0x467927 — trajectory techno check
				if (auto& pTraj = pExt->Trajectory)
				{
					switch (pTraj->OnAITechnoCheck(pTechnoOnCell))
					{
					case TrajectoryCheckReturnType::SkipGameCheck: //0x467A2B
					{
						FinishFlight(coords, veloc);
						return;
					}
					case TrajectoryCheckReturnType::SatisfyGameCheck://0x4679EB
					{
						markType = 1;
						destroyed = true;
						if (!this->Type->Inaccurate)
							coords = pTechnoOnCell->Location;
						MarkAndContinue("CBC-traj-techno-satisfy", coords, destroyed, markType, snapToTarget);
						return;
					}
					case TrajectoryCheckReturnType::Detonate://0x467E53
					{
						ApplyNukeWarhead("CBC-traj-techno-detonate", destroyed, coords);
						return;
					}
					default:
						break;
					}
				}

				const auto pPayback = this->Owner;
				const bool is_self_hit = pTechnoOnCell && pPayback && (pTechnoOnCell == pPayback);
				const bool is_ally_hit = pTechnoOnCell && pPayback && pPayback->Owner->IsAlliedWith(pTechnoOnCell);
				const bool techno_close_enough = pTechnoOnCell
					&& (coords - pTechnoOnCell->Location).Length() < 128.0;

				//Debug::LogInfo("[B{:X} f{}]  CBC(b) self={} ally={} close={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), is_self_hit, is_ally_hit, techno_close_enough);

				if (!is_self_hit && !is_ally_hit && techno_close_enough)
				{
					markType = 1;
					destroyed = true;

					if (!this->Type->Inaccurate)
						coords = pTechnoOnCell->Location;

					MarkAndContinue("CBC-techno-hit", coords, destroyed, markType, snapToTarget);
					return;
				}

				FinishFlight(coords, veloc);
			};

		// ====================================================================
		//  Non-homing: vertical
		// ====================================================================
		if (this->Type->Vertical)
		{
			if (static_cast<int>(vel_len) < this->Speed)
			{
				const double new_speed = static_cast<double>(static_cast<int>(vel_len) + this->Type->Acceleration);

				this->Velocity.SetXIfZeroXYZ();
				const double cur_mag = this->Velocity.Length();

				this->Velocity *= (new_speed / cur_mag);
				//Debug::LogInfo("[B{:X} f{}]  VERT accel: new_speed={:.1f}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), new_speed);
			}

			const CoordStruct pre = _thisCoord;
			_thisCoord.operator+=(this->Velocity.ToCoord());

			//Debug::LogInfo("[B{:X} f{}]  VERT coord ({},{},{})->({},{},{}) detAlt={} height={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
			//	CXYZ(pre), CXYZ(_thisCoord), this->Type->DetonationAltitude, this->GetHeight());

			if (_thisCoord.Z > this->Type->DetonationAltitude)
			{
				//Debug::LogInfo("[B{:X} f{}]  VERT IMPACT: above detonation altitude", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				markType = 1;
				destroyed = true;
			}
			else if (this->GetHeight() < 0)
			{
				//Debug::LogInfo("[B{:X} f{}]  VERT IMPACT: height<0", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
				markType = 1;
				destroyed = true;
			}
			else
			{
				const int wall_top = MapClass::Instance->GetCellFloorHeight(_thisCoord) + CellClass::BridgeHeight;
				auto pCellNew = MapClass::Instance->GetCellAt(_thisCoord);
				auto pCellPre = MapClass::Instance->GetCellAt(pre);

				if (pCellNew->ContainsBridgeHead() || pCellPre->ContainsBridgeHead())
				{
					const bool crossed = (_thisCoord.Z < wall_top)
						? (pre.Z >= wall_top)
						: (pre.Z < wall_top);

					//Debug::LogInfo("[B{:X} f{}]  VERT bridge wall_top={} crossed={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), wall_top, crossed);

					if (crossed)
					{
						markType = 1;
						destroyed = true;
					}
				}
			}

			CheckBuildingCell(_thisCoord, vel_copy);
			return;
		}

		// ====================================================================
		//  Non-homing: ballistic
		// ====================================================================
		VelocityClass newVel = this->Velocity;

		if (vel_len < 8.0)
			markType = 1;

		Vector3D<double> pos_ {
			static_cast<double>(_thisCoord.X),
			static_cast<double>(_thisCoord.Y),
			static_cast<double>(_thisCoord.Z)
		};

		// Hook 0x4671B9 — ApplyGravity
		const double gravity = pTypeExt->GetAdjustedGravity();
		newVel.Z -= gravity;

		//Debug::LogInfo("[B{:X} f{}]  BALL start pos=({:.1f},{:.1f},{:.1f}) vel=({:.3f},{:.3f},{:.3f}) gravity={:.3f} (Rules={} Floater={}) slow(<8)={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
		//	pos_.X, pos_.Y, pos_.Z, newVel.X, newVel.Y, newVel.Z, gravity,
		//	RulesClass::Instance->Gravity, this->Type->Floater, vel_len < 8.0);

		const CoordStruct prevCoord = _thisCoord;

		// Hook 0x46745C — Position_Trajectories (Phobos)
		if (pExt->Trajectory)
		{
			VelocityClass position_as_velocity { pos_.X, pos_.Y, pos_.Z };

			pExt->Trajectory->OnAIVelocity(&this->Velocity, &position_as_velocity);

			pos_.X = position_as_velocity.X;
			pos_.Y = position_as_velocity.Y;
			pos_.Z = position_as_velocity.Z;

			if (!pExt->LaserTrails.empty())
			{
				CoordStruct futureCoords = (newVel + position_as_velocity).ToCoord();

				for (auto& trail : pExt->LaserTrails)
				{
					if (!trail->LastLocation.isset())
						trail->LastLocation = this->Location;
					trail->Update(futureCoords);
				}
			}
		}

		pos_ += (*reinterpret_cast<Vector3D<double>*>(&newVel));

		CoordStruct newCoord = VelocityClass::Vec3DToVel(pos_).ToCoord();

		const int groundZ = MapClass::Instance->GetCellFloorHeight(newCoord);
		const int groundBias = groundZ + CellClass::BridgeHeight;
		CellClass* pCell = MapClass::Instance->GetCellAt(newCoord);

		bool fellThrough = false;
		bool risenThrough = false;

		if (pCell->ContainsBridgeHead() || MapClass::Instance->GetCellAt(prevCoord)->ContainsBridgeHead())
		{
			if (newCoord.Z < groundBias)
				fellThrough = prevCoord.Z >= groundBias;
			else
				risenThrough = prevCoord.Z < groundBias;
		}

		auto BallisticHitsBuilding = [](BulletClass* pBullet, CellClass* pHitCell)
			{
				BuildingClass* pBuilding = pHitCell->GetBuilding();
				if (!pBuilding)
					return pHitCell->ConnectsToOverlay(-1, -1); // wall check

				TechnoClass* pPayback = pBullet->Owner;
				bool hit = true;

				if (static_cast<TechnoClass*>(pBuilding) == pPayback
					|| (pBuilding->Type->LaserFence && pBuilding->LaserFenceFrame >= 8))
					hit = false;

				if (pBuilding->IsStrange())
					hit = false;

				if (pPayback && pPayback->Owner->IsAlliedWith(pBuilding))
					hit = false;

				return hit;
			};

		bool hitBuilding = false;
		if (!fellThrough && !risenThrough)
		{
			const double dGround = static_cast<double>(groundZ);
			if (pos_.Z >= dGround && pos_.Z - 150.0 < dGround)
				hitBuilding = BallisticHitsBuilding(this, pCell);
		}

		const bool belowGround = pos_.Z < static_cast<double>(groundZ);

		//Debug::LogInfo("[B{:X} f{}]  BALL new pos=({:.1f},{:.1f},{:.1f}) newCoord=({},{},{}) groundZ={} bias={} fell={} risen={} hitBuilding={} belowGround={}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(),
		//	pos_.X, pos_.Y, pos_.Z, CXYZ(newCoord), groundZ, groundBias,
		//	fellThrough, risenThrough, hitBuilding, belowGround);

		if (!belowGround && !fellThrough && !risenThrough && !hitBuilding)
		{
			_thisCoord = VelocityClass::Vec3DToVel(pos_).ToCoord();
		}
		else
		{
			bool impact = true;

			if (this->Owner)
			{
				if (fellThrough)
					pos_.Z = static_cast<double>(groundBias);
				else if (risenThrough)
					pos_.Z = static_cast<double>(groundBias - 20);
				else
					impact = false;
			}
			else
			{
				if (fellThrough)
					pos_.Z = static_cast<double>(groundBias);
				else if (risenThrough)
					pos_.Z = static_cast<double>(groundBias - 20);
				else if (static_cast<double>(groundZ - 100) < pos_.Z)
					pos_.Z = static_cast<double>(groundZ);

				auto BounceOffRamp = [](CoordStruct& impactCoord, VelocityClass& vel, double elasticity)
					{
						Matrix3D tmpRamp { };
						const Matrix3D ramp = *Game::GetRampMtx(&tmpRamp, TacticalClass::Instance->GetRamp(&impactCoord));
						const Matrix3D inv = ramp.InvertRigid();

						Vector3D<float> in {
							static_cast<float>(vel.X),
							static_cast<float>(-vel.Y),
							static_cast<float>(vel.Z)
						};

						Vector3D<float> rotated = inv.RotateVector(in);

						const float e = static_cast<float>(elasticity);
						rotated.X *= e;
						rotated.Y *= e;
						rotated.Z *= e;
						rotated.Z = -rotated.Z;

						Vector3D<float> out = ramp.RotateVector(rotated);

						vel.X = static_cast<double>(out.X);
						vel.Y = static_cast<double>(-out.Y);
						vel.Z = static_cast<double>(out.Z);
					};

				BounceOffRamp(newCoord, newVel, this->Type->Elasticity);
			}

			//Debug::LogInfo("[B{:X} f{}]  BALL collision: hasOwner={} impact={} finalPosZ={:.1f}", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame(), this->Owner != nullptr, impact, pos_.Z);

			if (impact)
			{
				markType = 1;
				destroyed = true;
			}

			_thisCoord = VelocityClass::Vec3DToVel(pos_).ToCoord();
		}

		CheckBuildingCell(_thisCoord, newVel);
	}
	else
	{
		//Debug::LogInfo("[B{:X} f{}]  END: early trajectory detonation", reinterpret_cast<std::uintptr_t>(this), Unsorted::CurrentFrame());
		ApplyNukeWarhead("early-trajectory", false, this->Location);
		return;
	}
}

#undef CXYZ

//#pragma optimize("", on )

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4740, FakeBulletClass::_AI);
DEFINE_FUNCTION_JUMP(LJMP, 0x4666E0, FakeBulletClass::_AI);