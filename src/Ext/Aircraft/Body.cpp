#include "Body.h"

#include <Ext/AircraftType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Cell/Body.h>

#include <AircraftClass.h>
#include <TeamClass.h>
#include <WaypointPathClass.h>

#include <Utilities/Macro.h>

#include <Misc/MapRevealer.h>

#include <Locomotor/FlyLocomotionClass.h>

AircraftExtData::AircraftExtData(AircraftClass* pObj) : FootExtData(pObj)
{
	this->Name = pObj->Type->ID;
	this->AbsType = AircraftClass::AbsID;
	this->CurrentType = pObj->Type;
	this->TypeExtData = AircraftTypeExtContainer::Instance.Find(pObj->Type);
}

COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon)
{
	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
		.Get(pWeapon->Projectile->ROT <= 1
			&& !pWeapon->Projectile->Inviso)
		&& !BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->TrajectoryType;
}

bool AircraftExtData::FireWeapon(AircraftClass* pAir, AbstractClass* pTarget)
{
	const auto pExt = AircraftExtContainer::Instance.Find(pAir);
	const int weaponIndex = pExt->CurrentAircraftWeaponIndex;
	bool Scatter = true;
	auto pDecideTarget = (pExt->Strafe_TargetCell ? pExt->Strafe_TargetCell : pTarget);

	if (const auto pWeaponStruct = pAir->GetWeapon(weaponIndex))
	{
		if (const auto weaponType = pWeaponStruct->WeaponType)
		{
			Scatter = pExt->GetTypeExtData()->FiringForceScatter.Get(weaponType->Damage > 0);

			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(weaponType);
			bool isStrafe = pAir->Is_Strafe();

			if (weaponType->Burst > 0)
			{
				for (int i = 0; i < weaponType->Burst; i++)
				{
					if (isStrafe && weaponType->Burst < 2 && pWeaponExt->Strafing_SimulateBurst)
						pAir->CurrentBurstIndex = pExt->Strafe_BombsDroppedThisRound % 2 == 0;

					pAir->Fire(pDecideTarget, weaponIndex);
				}

				if (isStrafe)
				{
					pExt->Strafe_BombsDroppedThisRound++;

					if (pWeaponExt->Strafing_UseAmmoPerShot)
					{
						pAir->Ammo--;
						pAir->loseammo_6c8 = false;

						if (!pAir->Ammo)
						{
							pAir->SetTarget(nullptr);
							pAir->SetDestination(nullptr, true);
						}
					}
				}
			}

		}
	}

	if (pDecideTarget && Scatter)
	{
		auto coord = pDecideTarget->GetCoords();

		if (auto pCell = MapClass::Instance->TryGetCellAt(coord)) {
			pCell->ScatterContent(coord, true, false, false);
		}

		return true;
	}

	return false;
}

int AircraftExtData::GetDelay(AircraftClass* pThis, bool isLastShot)
{
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);
	auto const pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	int delay = pWeapon->ROF;

	if (isLastShot || pExt->Strafe_BombsDroppedThisRound == pWeaponExt->Strafing_Shots.Get(5) || (pWeaponExt->Strafing_UseAmmoPerShot && !pThis->Ammo))
	{
		pExt->Strafe_TargetCell = nullptr;
		pThis->MissionStatus = (int)AirAttackStatus::FlyToPosition;
		delay = pWeaponExt->Strafing_EndDelay.Get((WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis) + 1024) / pThis->Type->Speed);
	}

	return delay;
}

long __stdcall AircraftClass_IFlyControl_IsStrafe(IFlyControl* ifly)
{
	auto pThis = static_cast<AircraftClass*>(ifly);
	WeaponTypeClass* pWeapon = nullptr;
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);

	pWeapon = pThis->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;

	if (pWeapon)
		return (long)AircraftCanStrafeWithWeapon(pWeapon);

	return false;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2268, AircraftClass_IFlyControl_IsStrafe);

Action FakeAircraftClass::_MouseOverCell(CellStruct const& cell, bool checkFog, bool ignoreForce)
{
	if (!this->Owner->ControlledByCurrentPlayer()) {
		return Action::None;
	}

	Action action = FootClass::MouseOverCell(cell, checkFog, ignoreForce);

	if (action == Action::Attack && !this->GetWeapon(0)->WeaponType) {
		return Action::None;
	}

	return action;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x417F80, FakeAircraftClass::_MouseOverCell);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2314, FakeAircraftClass::_MouseOverCell);

void FakeAircraftClass::_DropOffCarryallCargo()
{
	auto pFirstCargo = this->Passengers.RemoveFirstPassenger();

	CoordStruct loc = this->Location;
	if (MapClass::Instance->GetCellAt(loc)->ContainsBridge()) {
		loc.Z += MapClass::Instance->GetZPos(&this->Location);
		pFirstCargo->OnBridge = true;
	} else {
		pFirstCargo->OnBridge = false;
	}
	const auto pCargoType = GET_TECHNOTYPE(pFirstCargo);

	pFirstCargo->Locomotor.Release();
	auto NewLoco = LocomotionClass::CreateInstance(pCargoType->Locomotor);
	pFirstCargo->Locomotor = NewLoco;
	NewLoco->Link_To_Object(pFirstCargo);

	const auto nFacing = this->TurretFacing();
	//[0x416C3A - AircraftClass_Carryall_Unload_Facing]
	if (pFirstCargo->Unlimbo(loc, (DirType)(nFacing.GetFacing<256>()))) {
		const auto pCorgoTypeExt = TechnoTypeExtContainer::Instance.Find(pCargoType);
		const auto nRot = pCargoType->ROT;

		pFirstCargo->PrimaryFacing.Set_ROT(nRot);
		pFirstCargo->SecondaryFacing.Set_ROT(pCorgoTypeExt->TurretRot.Get(nRot));

		pFirstCargo->IsOnCarryall = false;
		pFirstCargo->vt_entry_48C(nullptr, 0, false, nullptr);
		pFirstCargo->UpdateSight(0, 0, 0, 0, 0);
		int lastSigt = pFirstCargo->LastSightRange;
		MapClass::Instance->RevealArea3(&this->Location, lastSigt - 3, lastSigt + 3, 0);
		CellStruct nearbyLanding;
		this->TechnoClass::NearbyLocation(&nearbyLanding, nullptr);
	
		if (this->GetNthLink() == pFirstCargo) {
			this->SendToFirstLink(RadioCommand::RequestUntether);
			this->SendToFirstLink(RadioCommand::NotifyUnlink);
		}

		if (!nearbyLanding.IsValid()) {
			this->SetDestination(nullptr, true);
		} else {
			this->SetDestination(MapClass::Instance->GetCellAt(nearbyLanding), true);
		}

		return;
	}

	//fail unlimbo 
	//destroy both cargo and the aircraft 
	//[0x416C4D - AircraftClass_Carryall_Unload_DestroyCargo]

	int Damage = pCargoType->Strength;
	pFirstCargo->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	Damage = this->Type->Strength;
	this->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x416AF0, FakeAircraftClass::_DropOffCarryallCargo);

Action  FakeAircraftClass::_MouseOverObject(ObjectClass* pObject, bool ignoreForce) const
{
	//[0x417CCB - AircraftClass_GetActionOnObject_Deactivated]
	if (this->Deactivated)
		return TechnoExtData::GetAction((TechnoClass*)this, pObject);

	// -----------------------------------------------------------------
   // LABEL_24 tail — lambda eliminates the forward jmp at 0x417D7A.
   // Entry (a): CanTote hook takes the Tote path → RunTail(Action::Tote).
   // Entry (b): normal fall-through after LABEL_18.
   // -----------------------------------------------------------------
	const auto RunTail = [&](Action act) -> Action {
			// 0x417DFC: ControlledByCurrentPlayer + action==Select → dock block
			if (this->Owner->ControlledByCurrentPlayer() && act == Action::Select) {
				// 0x417E0E: Kind_Of == RTTI_BUILDING
				if (auto pBldTarget = cast_to<BuildingClass*>(pObject)) {
	
					// -------------------------------------------------------
					// EXTENSION_Dock @ 0x417E16 (size 6)
					// Vanilla: loads [edi+520h] (pBuilding->Type via planningpath),
					//          reads [ebp+16A9h] (Helipad flag), then linear-scans
					//          this->Type->Dock vector.
					// Hook: replaces both the Helipad check and the vector scan.
					//   return 0x417E4B → can dock, proceed to radio/cargo checks.
					//   return 0x417E7D → cannot dock, skip to continuation.
					// -------------------------------------------------------
					if (this->Type->Dock.contains(pBldTarget->Type)) {
						// 0x417E4B path onwards
						// 0x417E4E: RadioClass::In_Radio_Contact(pBuilding->r, this->r)
						const bool inContact = pBldTarget->ContainsLink(this) != 0;
						// 0x417E57: [edi+118h] → pBuilding CargoHold
						// VERIFY: 0x118 == CargoHold on FootClass
						const bool targetFull = pBldTarget->Passengers.FirstPassenger;

						if (!inContact || targetFull)
						{
							act = Action::NoEnter; // LABEL_36: 0x1F
						}
						else
						{
							// 0x417E68: Transmit_Message(RADIO_CAN_LOAD=0x0F, pBuilding)
							// VERIFY: vtable 0x278 == Transmit_Message; 0x0F == RADIO_CAN_LOAD
							const auto reply = ((AircraftClass*)this)->SendCommand(RadioCommand::QueryCanEnter, pBldTarget);

							act = (reply == RadioCommand::AnswerPositive)
								? Action::Enter    // 0x417E71: act = 3
								: Action::NoEnter; // LABEL_36:  act = 0x1F
						}
					}
					// else: Dock.contains == false → hook returns 0x417E7D
					// act unchanged (Action::Select), falls to continuation below
				}
			}

			// 0x417E7D continuation
			// IsCarryall && action==Tote → land-spot vacancy check
			if (this->Type->Carryall && act == Action::Tote)
			{
				CellStruct pickupCell = pObject->GetMapCoords();
				if (pickupCell.IsValid()) {
					if (auto pBldCell = MapClass::Instance->GetCellAt(pickupCell)->GetBuilding()) {
						if (pBldCell->Type->WeaponsFactory)
							return Action::None; // 0x417EF0: early retn
					}
				}
				return act; // 0x417EFB: early retn with Tote
			}
			// 0x417EFE / 0x417F4A: mutually exclusive else-if chain
			else if (act == Action::NoMove) {
				if (pObject->IsDisguised()) {
					if (!pObject->GetDisguiseHouse(1)) {
						if (pObject->GetDisguise(1)->WhatAmI() == AbstractType::OverlayType)
							return Action::Move; // 0x417F39: early retn (1)
					}
				}
			} else if (act == Action::Attack) {
				if(pObject->GetType()->Immune){

					if(auto pTargetTech = flag_cast_to<TechnoClass*>(pObject)){
						const auto& [allow1, allow2, canBeDefused] = TechnoExtData::CanBeAffectedByFakeEngineer((AircraftClass*)this, pTargetTech, true, true, true);
					
						if (allow1 || allow2 || canBeDefused)
							return act;
					}

					act = Action::NoMove;
				}
			}

			return act; // 0x417F68
		};

	// -----------------------------------------------------------------
	// 0x417CD3: FootClass::What_Action
	// -----------------------------------------------------------------
	Action action = FootClass::MouseOverObject(pObject, ignoreForce);

	// 0x417CDA: ToggleSelect early return
	if (action == Action::ToggleSelect)
		return action;

	// 0x417CE9: GuardArea + AirportBound → None
	// VERIFY: 0x6C4 == Class ptr; 0xE0D == AirportBound on AircraftTypeClass
	if (action == Action::GuardArea && this->Type->AirportBound)
		action = Action::None;

	// -----------------------------------------------------------------
	// 0x417D00: IsCarryall + ControlledByCurrentPlayer gate
	// VERIFY: 0xDFC == IsCarryall on AircraftTypeClass
	// -----------------------------------------------------------------
	if (this->Type->Carryall && this->Owner->ControlledByCurrentPlayer()) {
		// 0x417D1F/0x417D24: jnz 0x417DCD when action != None && action != Select
		if (action == Action::None || action == Action::Select) {
			if (this->Owner->IsAlliedWith(pObject)) {
				bool passesOwnerCheck = true;
				if (pObject != nullptr) {
					if (auto pTechno = flag_cast_to<TechnoClass*, false>(pObject)) {
						if(auto pOrigOwner = pTechno->GetOriginalOwner()){
							passesOwnerCheck = (pOrigOwner->IsAlliedWith(this) != 0);
						}
					}
				}

				if (passesOwnerCheck) {
					UnitClass* targetIsUnit = cast_to<UnitClass*>(pObject);

					if ((this->Passengers.FirstPassenger == 0) && targetIsUnit) {
						if (TechnoTypeExtData::CarryallCanLift(this->Type, targetIsUnit))
							return RunTail(Action::Tote); // hook returned 0x417DF6
						// else hook returned 0: skip Tote, fall through to 0x417D7C
					}
				}
			}
		}
		// action != None && action != Select → fall through to LABEL_18
	}

	// -----------------------------------------------------------------
	// 0x417D7C: action == None → call What_Action1 via vt[70h]
	// Converges from: jz 0x417D3A, jz 0x417D5D, jz 0x417D67,
	//                 jnz 0x417D7E (action != 0 skips this block).
	// -----------------------------------------------------------------
	if (action == Action::None) {
		action = this->MouseOverCell(CellClass::Coord2Cell(pObject->GetCoords()), 0, ignoreForce);
	}

	// -----------------------------------------------------------------
	// LABEL_18 (0x417DCD): Self_Deploy / Attack edge cases
	// -----------------------------------------------------------------
	// 0x417DCD: cmp ebx, 4 (Action::Self_Deploy)
	if (action == Action::Self_Deploy) {
		// -----------------------------------------------------------
		// EXTENSION_NoManualUnload @ 0x417DD2 (size 6)
		// Vanilla: mov eax,[esi+114h] / test eax,eax / jnz LABEL_24
		//   → if Quantity != 0, keep Self_Deploy; else zero it.
		// Hook comment: skip the Quantity check entirely for UnitRepair
		//   compatibility; use NoManualUnload ext flag instead.
		//   return 0x417DF4 → prohibit unload (action = None, LABEL_23).
		//   return 0        → allow unload (keep Self_Deploy, fall to LABEL_24).
		// VERIFY: 0x114 == Cargo::Quantity on TechnoClass (vanilla check removed)
		// -----------------------------------------------------------
		if (TechnoTypeExtContainer::Instance.Find(this->Type)->NoManualUnload)
			action = Action::None; // mirrors hook returning 0x417DF4 (LABEL_23: xor ebx,ebx)
		// else: keep Self_Deploy, fall through to LABEL_24
	}
	// 0x417DDE: cmp ebx, 5 (Action::Attack)
	else if (action == Action::Attack) {	
		if (!this->GetWeapon(0)->WeaponType)
			action = Action::None;
	}

	// -----------------------------------------------------------------
	// LABEL_24 (0x417DF6)
	// -----------------------------------------------------------------
	return RunTail(action);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x417CC0, FakeAircraftClass::_MouseOverObject);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2318, FakeAircraftClass::_MouseOverObject);

// AircraftClass::Enter_Idle_Mode
// Vanilla address: 0x4176F0
// Goto-free cleanup — structure preserved 1:1 from ASM/pseudocode.
// Hooks integrated inline; each marked with vanilla address + macro name.

bool FakeAircraftClass::_Enter_Idle_Mode(bool initial, bool bool2)
{
	// ── tail helpers ─────────────────────────────────────────────────────────

	// LABEL_51: assign Good_LZ dest, mission = Move (2)
	const auto assign_good_lz_move = [&](Mission& missionOut)
		{
			CellClass* lz = this->GoodLandingZone_();
			this->SetDestination(lz, true);
			missionOut = Mission::Move; // 2
		};

	// LABEL_56: clear target/dest, mission = Retreat (4)
	const auto loaner_retire = [&](Mission& missionOut)
		{
			this->SetTarget(nullptr);
			this->SetDestination(nullptr, true);
			missionOut = Mission::Retreat; // 4
		};

	// loc_417B44: radio-contact override + QueueMission + NextMission.
	// DEFINE_JUMP(LJMP, 0x4179E2, 0x417B44)
	// Successful RADIO_ROGER dock path jumps straight here, bypassing loc_417AD4.
	// Reason: bay is confirmed — no need to re-run the ammo/rearm-dock guard below.
	const auto assign_and_commence_direct = [&](Mission mission, bool v6) -> bool
		{
			if (this->HasAnyLink())
				mission = Mission::Enter; // 7

			this->QueueMission(mission, false);

			if (this->ReadyToNextMission())
				this->NextMission();

			return v6;
		};

	// ── HOOK: 0x4179F7 / 0x417B82  AircraftClass_EnterIdleMode_NoCrash (size 6) ──
	// Replaces vanilla Crash()+return false at both AirportBound sites.
	// PATCH_AGAIN 0x417B82 is the ground docking-miss path; same logic applies.
	// Returns engaged value (skip game code) or nullopt (fall through to vanilla).
	const auto hook_NoCrash = [&](bool v6) -> std::optional<bool>
		{
			if (this->Airstrike || this->Spawned)
				return std::nullopt;

			if (AircraftTypeExtContainer::Instance.Find(this->Type)
					->ExtendedAircraftMissions_UnlandDamage
					.Get(RulesExtData::Instance()->ExtendedAircraftMissions_UnlandDamage) < 0)
				return std::nullopt;

			if (!this->Team
				&& (this->CurrentMission != Mission::Area_Guard || !this->ArchiveTarget))
			{
				const auto pCell = this->GoodLandingZone_();
				this->SetDestination(pCell, true);
				this->SetArchiveTarget(pCell);
				this->QueueMission(Mission::Area_Guard, true);
			}
			else if (!this->Destination)
			{
				const auto pCell = this->GoodLandingZone_();
				this->SetDestination(pCell, true);
			}

			return v6; // SkipGameCode → 0x417B69
		};

	// loc_417AD4 full tail: ammo/rearm-dock guard → loc_417B44.
	// All paths that did NOT just confirm a RADIO_ROGER bay converge here.
	// Checks ammo: if empty + armed + not already in radio contact → try rearm dock.
	const auto assign_and_commence = [&](Mission mission, bool v6) -> bool
		{
			if (!this->Ammo && this->IsArmed() && !this->HasAnyLink())
			{
				TechnoClass* rearmBay = this->FindDockingBayInVector(
					reinterpret_cast<DynamicVectorClass<TechnoTypeClass*>*>(&this->Type->Dock),
					false, false);

				if (rearmBay)
				{
					this->SetDestination(rearmBay, true);
					this->SetTarget(nullptr);
					mission = Mission::Enter; // 7
					// falls through to loc_417B44 below
				}
				else if (this->Type->AirportBound)
				{
					// ── HOOK: 0x4179F7 (rearm-dock-miss AirportBound path at loc_417AD4) ──
					// Same NoCrash handler — no free rearm bay and AirportBound set.
					if (auto r = hook_NoCrash(v6); r.has_value())
						return *r;

					this->Crash(nullptr);
					return false;
				}
			}

			return assign_and_commence_direct(mission, v6);
		};

	// ── 1. Mission_Overriden branch (0x4176F8) ───────────────────────────────
	if (this->MissionIsOverriden())
	{
		this->Mission_Revert();
		if (this->CurrentMission == Mission::Patrol) // 25
		{
			this->MissionStatus = 0;
			this->IsLocked = 0;
		}
		return false;
	}

	// ── 2. Switch: airstrike-mission guard (0x417733) ────────────────────────
	switch (this->CurrentMission)
	{
	case Mission::Retreat:          // 4
	case Mission::ParadropApproach: // 26
	case Mission::ParadropOverfly:  // 27
	case Mission::SpyplaneApproach: // 30
	case Mission::SpyplaneOverfly:  // 31
		if (!this->Airstrike)
			return false;
		break;
	default:
		break;
	}

	// ── 3. MissionQueue early Commence (0x41775E) ────────────────────────────
	{
		Mission q = this->QueuedMission;
		if (q == Mission::ParadropApproach || q == Mission::SpyplaneApproach) {
			this->NextMission();
			return false;
		}
	}

	// ── 4. FootClass base call + initial mission select (0x417782) ───────────
	bool v6 = FootClass::EnterIdleMode(initial, bool2);

	// ASM 0x4177AE: mov edi, 0Bh (11) on weapon-equipped AI branch.
	// 11 = Mission::Area_Guard — NOT Mission::Attack (1). Pseudocode label was misleading.
	Mission missionSelect;
	if (this->Owner->ControlledByCurrentPlayer()
		|| this->Team
		|| !this->IsArmed())
		missionSelect = Mission::Guard;      // 5
	else
		missionSelect = Mission::Area_Guard; // 11 — ASM: mov edi, 0Bh

	// ── 5. Flying / above-landing-altitude path ──────────────────────────────
	int landingAlt = this->Landing_Altitude();

	const bool aboveGround =
		this->InWhichLayer() != Layer::Ground
		&& this->GetHeight() > landingAlt
		&& !this->Type->MissileSpawn;

	// Hoisted outside both branches — used in aboveGround and ground paths.
	const bool isLoaner = this->Spawned;
	const bool hasCargo = this->Passengers.FirstPassenger != nullptr;

	if (aboveGround)
	{
		if (hasCargo)
		{
			if (isLoaner)
			{
				if (!this->Team)
				{
					// loaner + cargo + no team → Good_LZ, mission = Unload (16)
					CellClass* lz = this->GoodLandingZone_();
					this->SetDestination(lz, true);
					missionSelect = Mission::Unload; // 16 — ASM: mov edi, 10h; verified
				}
				else
				{
					missionSelect = Mission::Guard; // 5
				}
			}
			else
			{
				assign_good_lz_move(missionSelect);
			}
			return assign_and_commence(missionSelect, v6);
		}

		// no cargo ────────────────────────────────────────────────────────────
		{
			bool shouldRemoveFromTeam =
				(isLoaner && this->Owner->ControlledByCurrentPlayer())
				|| (!this->Owner->ControlledByCurrentPlayer() && !this->Type->Ammo);

			if (shouldRemoveFromTeam)
			{
				if (TeamClass* team = this->Team)
					if (team->HasEnteredMap())
						team->RemoveMember(this, -1, false);
			}

			if (this->GetWeapon(0)->WeaponType)
			{
				const int ammo = this->Ammo;

				if (isLoaner)
				{
					if (!ammo)
					{
						if (TeamClass* team = this->Team)
							team->RemoveMember(this, -1, false);

						loaner_retire(missionSelect);
						return assign_and_commence(missionSelect, v6);
					}

					if (!this->Team)
						missionSelect = Mission::Hunt; // 15 — ASM: mov edi, 0Fh; verified

					return assign_and_commence(missionSelect, v6);
				}
				else // !isLoaner
				{
					bool wantsAttack =
						(ammo && this->Target
						 && this->GetCurrentMission() == Mission::Attack)
						|| this->QueuedMission == Mission::Attack;

					if (wantsAttack)
					{
						missionSelect = Mission::Attack; // 1
						return assign_and_commence(missionSelect, v6);
					}

					// ── FIX: OpenTopped armed aircraft docking (replaces misplaced HOOK 0x417A2E) ──
					//
					// VANILLA BUG — armed OpenTopped aircraft loops forever on Mission::Move.
					//
					//   The original hook at 0x417A2E only fires in loc_417A1A (no-weapon path).
					//   Armed aircraft (WeaponType != null) never reach that label — they exit
					//   this !isLoaner branch via one of two paths below, both missed by the hook:
					//
					//   PATH A — Dock.Count > 0, all bays busy, !AirportBound:
					//     FindDockingBayInVector → null → assign_good_lz_move → Mission::Move
					//     → Enter_Idle_Mode next frame → same result → infinite loop.
					//
					//   PATH B — IsInAir() guard fails entirely (not airborne, NavCom+Enter,
					//     Count == 0, or IsLocked=false with a Team):
					//     Falls through the dock block with no action, assigns Guard/Area_Guard.
					//
					// WHY OpenTopped IS SPECIAL:
					//   Vanilla assumed OpenTopped aircraft are unarmed transports.
					//   Projectile interception breaks this: interceptors are OpenTopped AND
					//   carry a weapon (or gain one via passenger upgrade).  Vanilla's weapon
					//   branch only retries docking when TarCom or queued Attack exists.
					//   An idle armed interceptor has neither, so it always hits the Move-loop.
					//
					// FIX — opentopped_try_dock lambda, applied at both Path A and Path B:
					//   If OpenTopped + non-Spawned + no attack intent + dock slot exists
					//   → attempt RadioCommand::RequestLink to bay → Mission::Enter.
					//   If no bay accepts → QueueMission Area_Guard so the aircraft orbits
					//   instead of beelining to a random cell and looping on Mission::Move.
					// ─────────────────────────────────────────────────────────────────────

					const auto opentopped_try_dock = [&]() -> bool
						{
							if (this->Spawned || !this->Type->OpenTopped)
								return false;
							if (this->QueuedMission == Mission::Attack || this->Target)
								return false;
							if (this->Type->Dock.Count <= 0)
								return false;
							if (!this->IsLocked && this->Team)
								return false;

							TechnoClass* bay = this->FindDockingBayInVector(
								reinterpret_cast<DynamicVectorClass<TechnoTypeClass*>*>(&this->Type->Dock),
								false, false);
							this->SetDestination(nullptr, true);

							if (bay
								&& this->SendCommand(RadioCommand::RequestLink, bay)
								   == RadioCommand::AnswerPositive) {
								this->SetDestination(bay, true);
								missionSelect = Mission::Enter; // 7 — dock accepted
								// Bay confirmed — same DEFINE_JUMP logic: skip ammo/rearm guard.
								return true; // caller uses assign_and_commence_direct
							}

							// No bay free: orbit until one opens rather than Mission::Move loop.
							this->QueueMission(Mission::Area_Guard, true);
							missionSelect = Mission::Area_Guard; // 11
							return true;
						};

					if (this->IsInAir()
						&& (!this->NavCom || this->CurrentMission != Mission::Enter))
					{
						if (this->Type->Dock.Count > 0
							&& (this->IsLocked || !this->Team)) {
							TechnoClass* bay = this->FindDockingBayInVector(
								reinterpret_cast<DynamicVectorClass<TechnoTypeClass*>*>(&this->Type->Dock),
								false, false);
							this->SetDestination(nullptr, true);

							if (bay
								&& this->SendCommand(RadioCommand::RequestLink, bay)
								   == RadioCommand::AnswerPositive) {
								this->SetDestination(bay, true);
								missionSelect = Mission::Enter; // 7
								// DEFINE_JUMP(LJMP, 0x4179E2, 0x417B44)
								// Bay confirmed — skip loc_417AD4 ammo/rearm guard, jump to loc_417B44.
								return assign_and_commence_direct(missionSelect, v6);
							}

							if (this->Type->AirportBound) {
								// ── HOOK: 0x4179F7  AircraftClass_EnterIdleMode_NoCrash (size 6) ──
								// Vanilla: Crash(nullptr) + return false.
								// Extension: redirect to Area_Guard landing instead of crashing.
								if (auto r = hook_NoCrash(v6); r.has_value())
									return *r;

								this->Crash(nullptr);
								return false;
							}

							// PATH A: all bays busy, !AirportBound.
							// Fix fires before vanilla assign_good_lz_move → Move loop.
							// opentopped_try_dock sets Mission::Enter on ROGER (bay confirmed → _direct),
							// or Mission::Area_Guard on failure (no confirmed bay → full tail).
							if (opentopped_try_dock())
								return missionSelect == Mission::Enter
								? assign_and_commence_direct(missionSelect, v6)
								: assign_and_commence(missionSelect, v6);

							assign_good_lz_move(missionSelect);
							return assign_and_commence(missionSelect, v6);
						}
					}

					// PATH B: IsInAir() guard failed — vanilla exits with no dock attempt at all.
					// Fix gives OpenTopped interceptor a dock retry before the tail.
					if (opentopped_try_dock())
						return missionSelect == Mission::Enter
						? assign_and_commence_direct(missionSelect, v6)
						: assign_and_commence(missionSelect, v6);

					return assign_and_commence(missionSelect, v6);
				}
			}
			else // no weapon
			{
				if (!this->Team)
					assign_good_lz_move(missionSelect);

				return assign_and_commence(missionSelect, v6);
			}
		}
	}
	else // on ground / below landing alt
	{
		if (isLoaner)
		{
			if (hasCargo)
			{
				missionSelect = this->Team ? Mission::Guard : Mission::Unload; // 5 or 16
				return assign_and_commence(missionSelect, v6);
			}

			if (this->Team)
				return assign_and_commence(missionSelect, v6);

			loaner_retire(missionSelect);
			return assign_and_commence(missionSelect, v6);
		}

		// not loaner, on ground
		this->SetDestination(nullptr, true);
		this->SetTarget(nullptr);

		if (this->Owner->ControlledByCurrentPlayer() || this->Team)
		{
			missionSelect = Mission::Guard; // 5
		}
		else
		{
			missionSelect = Mission::Area_Guard; // 11 — ASM: mov edi, 0Bh
			if (!this->IsArmed())
				missionSelect = Mission::Guard; // 5
		}

		// loc_417B72: AirportBound check on ground docking-miss path.
		if (this->Type->AirportBound) {
			// ── HOOK: 0x417B82  ASMJIT_PATCH_AGAIN  AircraftClass_EnterIdleMode_NoCrash (size 6) ──
			// Vanilla: Crash(nullptr) + return false.
			// Same extension handler as 0x4179F7.
			if (auto r = hook_NoCrash(v6); r.has_value())
				return *r;

			this->Crash(nullptr);
			return false;
		}

		return assign_and_commence(missionSelect, v6);
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2728, FakeAircraftClass::_Enter_Idle_Mode);
DEFINE_FUNCTION_JUMP(LJMP, 0x4176F0, FakeAircraftClass::_Enter_Idle_Mode);

int FakeAircraftClass::_Mission_Attack()
{
	auto* pExt = AircraftExtContainer::Instance.Find(this);

	// 0x417FF1 - Top-of-function: weapon re-eval and strafe state bookkeeping
	{
		AirAttackStatus const state = static_cast<AirAttackStatus>(this->MissionStatus);

		if (state > AirAttackStatus::ValidateAZ && state < AirAttackStatus::FireAtTarget)
			pExt->CurrentAircraftWeaponIndex = MaxImpl(this->SelectWeapon(this->Target), 0);

		if (this->MissionStatus < static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe)
			|| this->MissionStatus > static_cast<int>(AirAttackStatus::FireAtTarget5_Strafe))
		{
			pExt->Strafe_BombsDroppedThisRound = 0;
		}

		if (pExt->Strafe_BombsDroppedThisRound)
		{
			auto const pWeapon = this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
			auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
			int const  count = pWeaponExt->Strafing_Shots.Get(5);

			if (count > 5
				&& this->MissionStatus == static_cast<int>(AirAttackStatus::FireAtTarget3_Strafe)
				&& (count - 3 - pExt->Strafe_BombsDroppedThisRound) > 0)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe);
			}
		}
	}

	// --- Helpers ---

	auto MissionRate = [this]() -> int
		{
			auto ctrl = this->GetCurrentMissionControl();
			return static_cast<int>(ctrl->Rate * TICKS_PER_MINUTE)
				+ ScenarioClass::Instance->Random.RandomRanged(0, 2);
		};

	auto CurleyShuffle = [this]() -> bool
		{
			return TechnoTypeExtContainer::Instance.Find(this->Type)
				->CurleyShuffle.Get(RulesClass::Instance->CurleyShuffle);
		};

	auto ReturnToBaseNow = [this]() -> int
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
			return 1;
		};

	auto HandleOutOfRange = [&]() -> int
		{
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				this->IsLocked = 0;
			}
			return 1;
		};

	auto SelectWeaponBeforeFiring = [&]() -> int
		{
			if (!pExt->Strafe_BombsDroppedThisRound)
				pExt->CurrentAircraftWeaponIndex = MaxImpl(this->SelectWeapon(this->Target), 0);
			return pExt->CurrentAircraftWeaponIndex;
		};

	// CurleyShuffle A/B/C/D: pick re-approach or continue attacking
	auto SetCurleyStatus = [&]()
		{
			this->MissionStatus = CurleyShuffle()
				? static_cast<int>(AirAttackStatus::PickAttackLocation)
				: static_cast<int>(AirAttackStatus::FireAtTarget);
		};

	// Used in default cases: if in range defer to CurleyShuffle, otherwise always re-approach
	auto SetRangeBasedStatus = [&]()
		{
			if (this->IsCloseEnoughToAttack(this->Target))
				SetCurleyStatus();
			else
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
		};

	// Shared logic for FACING and default in FireAtTarget2:
	//   checkStrafe45 = true  -> FACING case (also returns 45 if strafing)
	//   checkStrafe45 = false -> default case (no strafe-45 path)
	auto HandleCantFire = [&](bool checkStrafe45) -> int
		{
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				return MissionRate();
			}
			if (checkStrafe45 && (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe()))
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			}
			else if (!checkStrafe45)
			{
				SetRangeBasedStatus();
			}
			else
			{
				SetCurleyStatus();
			}
			if (checkStrafe45 && this->Is_Strafe())
				return 45;
			return MissionRate();
		};

	auto SetupReturnFlight = [&]() -> int
		{
			this->IsLocked = 0;
			CellStruct edgeCell = MapClass::Instance->PickCellOnEdge(
				this->Owner->GetCurrentEdge(),
				CellStruct::Empty, CellStruct::Empty,
				SpeedType::Winged, true, MovementZone::Normal);
			this->SetDestination(MapClass::Instance->GetCellAt(edgeCell), true);
			this->NumParadropsLeft = 0;
			if (this->Airstrike && this->Ammo > 0)
				this->QueueMission(Mission::Retreat, false);
			else
				this->EnterIdleMode(false, true);
			this->NumParadropsLeft = true;
			return 1;
		};

	// ---

	switch (static_cast<AirAttackStatus>(this->MissionStatus))
	{
	case AirAttackStatus::ValidateAZ:
	{
		this->IsLocked = 0;
		this->MissionStatus = this->Target
			? static_cast<int>(AirAttackStatus::PickAttackLocation)
			: static_cast<int>(AirAttackStatus::ReturnToBase);
		return 1;
	}

	case AirAttackStatus::PickAttackLocation:
	{
		this->IsLocked = 0;
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			this->Ammo--;
		}
		if (this->Target && this->Ammo)
		{

			// if (!this->Type->MissileSpawn && !this->Type->Fighter && !this->Is_Strafe())
			// {
			// 	AbstractClass* pTarget = this->Target;
			// 	int weaponIdx = this->SelectWeapon(pTarget);

			// 	if (this->IsCloseEnough(pTarget, weaponIdx))
			// 	{
			// 		this->IsLocked = true;
			// 		CoordStruct pos = this->GetCoords();
			// 		CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);
			// 		this->SetDestination(pCell, true);
			// 		this->MissionStatus = this->Destination
			// 			? static_cast<int>(AirAttackStatus::FlyToPosition)
			// 			: static_cast<int>(AirAttackStatus::ReturnToBase);
			// 	}
			// 	else
			// 	{
			// 		int dest = this->DistanceFrom(this->Target);
			// 		WeaponTypeClass* pWeapon = this->GetWeapon(weaponIdx)->WeaponType;
			// 		CoordStruct nextPos = CoordStruct::Empty;
			// 		if (dest < pWeapon->MinimumRange)
			// 		{
			// 			CoordStruct flh = CoordStruct::Empty;
			// 			flh.X = (int)(pWeapon->Range * 0.5);
			// 			nextPos = TechnoExtData::GetFLHAbsoluteCoords(this, flh, true);
			// 		}
			// 		else if (dest > pWeapon->Range) //TODO :Evaluate weapon range
			// 		{
			// 			int length = (int)(pWeapon->Range * 0.5);
			// 			int flipY = 1;
			// 			if (ScenarioClass::Instance->Random.RandomRanged(0, 1) == 1) {
			// 				flipY *= -1;
			// 			}
			// 			CoordStruct sourcePos = this->GetCoords();
			// 			int r = (dest - length) * Unsorted::LeptonsPerCell;
			// 			r = ScenarioClass::Instance->Random.RandomRanged(0, r);
			// 			CoordStruct flh{ 0, r * flipY, 0 };
			// 			CoordStruct targetPos = this->Target->GetCoords();
			// 			DirStruct dir = Point2Dir(sourcePos, targetPos);
			// 			sourcePos = GetFLHAbsoluteCoords(sourcePos, flh, dir);
			// 			sourcePos.Z = 0;
			// 			targetPos.Z = 0;

			// 			nextPos = GetForwardCoords(targetPos, sourcePos, length);
			// 		}
			// 		if (!nextPos.IsEmpty())
			// 		{
			// 			CellClass* pCell = MapClass::Instance->TryGetCellAt(nextPos);
			// 			this->SetDestination(pCell, true);
			// 			this->MissionStatus = this->Destination
			// 				? static_cast<int>(AirAttackStatus::FlyToPosition)
			// 				: static_cast<int>(AirAttackStatus::ReturnToBase);
			// 		}
			// 	}
			// }

			this->SetDestination(this->GoodTargetLoc_(this->Target), true);
			this->MissionStatus = this->Destination
				? static_cast<int>(AirAttackStatus::FlyToPosition)
				: static_cast<int>(AirAttackStatus::ReturnToBase);
		}
		else
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
		}
		return MissionRate();
	}

	case AirAttackStatus::FlyToPosition:
	{
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			this->Ammo--;
		}
		this->IsLocked = 0;

		if (!this->Target || !this->Ammo)
			return ReturnToBaseNow();

		if (this->Is_Strafe())
		{
			// 0x4180F4 - use CurrentAircraftWeaponIndex instead of slot 0 for range check
			auto* wt = this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType;
			if (this->DistanceFrom(this->Target) < WeaponTypeExtData::GetRangeWithModifiers(wt, this))
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				return 1;
			}
			this->SetDestination(this->Target, true);
		}
		else
		{
			// Skip fire twice,
			// IsLocked always is False, so the game will jump to MissionStatus=AIR_ATT_FIRE_AT_TARGET1, and fire weapon again.
			// this skip looks no effect for ROT=0 or Arcing.
			//DEFINE_JUMP(LJMP, 0x4184FC, 0x418506);
			// if (this->Is_Locked())
			// {
			// 	this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
			// 	return 1;
			// }

			if (!this->Locomotor.GetInterfacePtr()->Is_Moving_Now())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				return 1;
			}
		}

		if (!this->Destination)
		{
			this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			return 1;
		}

		int const dist = this->DistanceFrom(this->Destination);
		if (dist >= 512)
		{
			CoordStruct myPos;
			CoordStruct navCenter = this->Destination->GetCenterCoords();
			FakeTechnoClass::__Get_FLH(this, discard_t(), &myPos, 0, {});

			DirStruct dir;
			if (myPos.X == navCenter.X && myPos.Y == navCenter.Y)
			{
				dir.Raw = 0;
			}
			else
			{
				double angle = Math::atan2(float(myPos.Y - navCenter.Y), float(navCenter.X - myPos.X));
				dir.Raw = static_cast<short>((angle - Math::DEG90_AS_RAD) * Math::BINARY_ANGLE_MAGIC);
			}
			this->SecondaryFacing.Set_Desired(dir);
		}
		else
		{
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));

			if (dist < 16) {
				if(!this->Type->MissileSpawn && !this->Type->Fighter) {
					this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
					return 1;
				}

				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
				this->SetDestination(nullptr, true);
			}
		}

		return 1;
	}

	case AirAttackStatus::FireAtTarget:
	{
		if (!this->Target || !this->Ammo)
			return ReturnToBaseNow();

		if (!this->Is_Strafe())
		{
			this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
			this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
		}

		int const weapSlot = SelectWeaponBeforeFiring(); // 0x41831E

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		{
			this->loseammo_6c8 = true; // 0x418403
			bool const fired = AircraftExtData::FireWeapon(this, this->Target);

			if (this->Is_Strafe())
			{
				// 0x4184CC - Delay1A
				auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(
					this->GetWeapon(pExt->CurrentAircraftWeaponIndex)->WeaponType);
				if (pWeaponExt->Strafing_TargetCell && this->Target)
					pExt->Strafe_TargetCell = MapClass::Instance->GetCellAt(this->Target->GetCoords());

				// Set destination toward target so aircraft flies through it during end-delay,
				// matching STRAFE_FIRE_CASE behaviour and ensuring the aircraft leaves weapon range
				// before FlyToPosition re-evaluates (fixes Strafing.Shots < 5 looping in place)
				if (fired)
					this->SetDestination(this->Target, true);

				this->IsLocked = true;
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2_Strafe);
				return AircraftExtData::GetDelay(this, false);
			}

			//DEFINE_JUMP(LJMP, 0x4184FC, 0x418506);
			//if (!this->Is_Locked())
			//{
			//	this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2);
			//	return 1;
			//}
			
			// 0x418506 - Delay1B
			this->IsLocked = true;
			this->MissionStatus = this->Ammo > 0
				? static_cast<int>(AirAttackStatus::PickAttackLocation)
				: static_cast<int>(AirAttackStatus::ReturnToBase);
			return AircraftExtData::GetDelay(this, false);
		}
		case FireError::FACING:
		{
			if (!this->Ammo)
				return ReturnToBaseNow();

			if (!this->IsCloseEnoughToAttack(this->Target) || this->Is_Strafe())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
			}
			else if (this->Is_Locked())
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget);
			}
			else
			{
				SetCurleyStatus(); // 0x4183C3 CurleyShuffle_A
			}
			if (this->Is_Strafe())
				return 45;
			return 1;
		}
		case FireError::REARM:
			return 1;

		case FireError::CLOAKED:
			this->Uncloak(false);
			return 1;

		case FireError::RANGE:
			if (this->Is_Strafe()) // 0x418544 StrafingDestinationFix
				this->SetDestination(this->Target, true);
			[[fallthrough]];

		default:
			if (!this->Ammo)
				return ReturnToBaseNow();
			if (this->Is_Strafe())
				return 1;
			// 0x418572 — original: Status = FireAtTarget2 (5), try secondary fire state
			this->MissionStatus = static_cast<int>(AirAttackStatus::FireAtTarget2);
			return 1;
		}
	}

	case AirAttackStatus::FireAtTarget2:
	{
		if (!this->Target)
			return ReturnToBaseNow();

		this->PrimaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));
		this->SecondaryFacing.Set_Desired(this->GetDirectionOverObject(this->Target));

		int const weapSlot = SelectWeaponBeforeFiring(); // 0x4185F5

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		{
			AircraftExtData::FireWeapon(this, this->Target); // 0x4186B6
			if (!this->Ammo)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::ReturnToBase);
				return MissionRate();
			}
			SetCurleyStatus(); // 0x418733 CurleyShuffle_C
			return MissionRate();
		}
		case FireError::FACING:
			return HandleCantFire(true);  // 0x418671 CurleyShuffle_B, strafe-45 applies

		case FireError::REARM:
			return MissionRate();

		case FireError::CLOAKED:
			this->Uncloak(false);
			return MissionRate();

		case FireError::RANGE:
			if (this->Is_Strafe()) // 0x41874E StrafingDestinationFix
				this->SetDestination(this->Target, true);
			[[fallthrough]];

		default:
			return HandleCantFire(false); // 0x418782 CurleyShuffle_D, no strafe-45
		}
	}

	// Strafe states 2-4: SelectWeaponBeforeFiring -> Can_Fire -> FireWeapon
	// -> if fired: SetDestination -> GetDelay(false) to next strafe state
#define STRAFE_FIRE_CASE(CaseName, NextState)                               \
    case AirAttackStatus::CaseName:                                         \
    {                                                                       \
        if (!this->Target)                                                  \
            return ReturnToBaseNow();                                       \
        int const weapSlot = SelectWeaponBeforeFiring();                    \
        switch (this->GetFireError(this->Target, weapSlot, true))           \
        {                                                                   \
        case FireError::OK:                                                 \
        case FireError::FACING:                                             \
        case FireError::CLOAKED:                                            \
            break;                                                          \
        case FireError::RANGE:                                              \
            this->SetDestination(this->Target, true);                      \
            break;                                                          \
        default:                                                            \
            return HandleOutOfRange();                                      \
        }                                                                   \
        if (AircraftExtData::FireWeapon(this, this->Target))                                 \
            this->SetDestination(this->Target, true);                      \
        this->MissionStatus = static_cast<int>(AirAttackStatus::NextState);\
        return AircraftExtData::GetDelay(this, false);                                       \
    }

	STRAFE_FIRE_CASE(FireAtTarget2_Strafe, FireAtTarget3_Strafe) // 0x418805 + 0x418883
		STRAFE_FIRE_CASE(FireAtTarget3_Strafe, FireAtTarget4_Strafe) // 0x418914 + 0x418992
		STRAFE_FIRE_CASE(FireAtTarget4_Strafe, FireAtTarget5_Strafe) // 0x418A23 + 0x418AA1
#undef STRAFE_FIRE_CASE

	case AirAttackStatus::FireAtTarget5_Strafe:
	{
		if (!this->Target)
			return ReturnToBaseNow();

		int const weapSlot = SelectWeaponBeforeFiring();

		switch (this->GetFireError(this->Target, weapSlot, true))
		{
		case FireError::OK:
		case FireError::FACING:
		case FireError::RANGE:
		case FireError::CLOAKED:
			AircraftExtData::FireWeapon(this, this->Target); // 0x418B1F
			return AircraftExtData::GetDelay(this, true);    // 0x418B8A isLastShot=true
		default:
			return HandleOutOfRange();
		}
	}

	case AirAttackStatus::ReturnToBase:
	{
		this->IsLocked = 0;
		if (this->loseammo_6c8)
		{
			this->loseammo_6c8 = false;
			if (this->Ammo > 0)
				this->Ammo--;
		}

		if (this->Ammo)
		{
			// 0x418CD1 - ContinueFlyToDestination
			if (this->Target)
			{
				this->MissionStatus = static_cast<int>(AirAttackStatus::PickAttackLocation);
				return 1;
			}
			if (AircraftTypeExtData::ExtendedAircraftMissionsEnabled(this)
				&& this->MegaMissionIsAttackMove()
				&& this->MegaDestination)
			{
				this->SetDestination(reinterpret_cast<AbstractClass*>(this->MegaDestination), false);
				this->QueueMission(Mission::Move, true);
				this->QueueMission(Mission::Move, true);
				this->HaveAttackMoveTarget = false;
				return 1;
			}
			// No target and no attack-move: fly out even with ammo remaining
			return SetupReturnFlight();
		}

		if (this->Spawned || this->Owner->IsControlledByHuman())
			this->SetTarget(nullptr);

		return SetupReturnFlight();
	}

	default:
		return MissionRate();
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x417FE0, FakeAircraftClass::_Mission_Attack);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E24B4, FakeAircraftClass::_Mission_Attack);

AbstractClass* FakeAircraftClass::_GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy)
{
	if (AircraftTypeExtData::ExtendedAircraftMissionsEnabled(this) && !this->Team && this->Ammo && !this->Airstrike && !this->Spawned)
	{
		if (WeaponTypeClass* const pPrimaryWeapon = this->GetWeapon(0)->WeaponType)
			threatType |= pPrimaryWeapon->AllowedThreats();

		if (WeaponTypeClass* const pSecondaryWeapon = this->GetWeapon(1)->WeaponType)
			threatType |= pSecondaryWeapon->AllowedThreats();
	}

	return this->FootClass::GreatestThreat(threatType, pSelectCoords, onlyTargetHouseEnemy); // FootClass_GreatestThreat (Prevent circular calls)
}

// Sleep: return to airbase if in incorrect sleep status

int FakeAircraftClass::_Mission_Sleep()
{
	if (!this->Destination || this->Destination == this->DockedTo)
		return 450; // Vanilla MissionClass_Mission_Sleep value

	this->EnterIdleMode(false, true);
	return 1;
}

int FakeAircraftClass::_Mission_ParadropOverfly()
{
	auto pTarCom = this->Target;
	this->IsLocked = 1;

	if (!pTarCom || !this->Passengers.NumPassengers)
	{
		this->IsLocked = 0;
		this->SetTarget(0);
		this->SetDestination(0, 1);
		this->QueueMission(Mission::Retreat, 0);
		return 5;
	}

	const int distance = this->DistanceFrom(pTarCom);
	auto pTypeExt = AircraftTypeExtContainer::Instance.Find(this->Type);
	const int nRadius = AircraftTypeExtContainer::Instance.Find(this->Type)->ParadropRadius.Get(RulesClass::Instance->ParadropRadius);

	if (distance > nRadius) {
		auto paradrop_attempts = this->NumParadropsLeft;
		this->IsLocked = 0;

		if (paradrop_attempts > 0)
		{
			this->QueueMission(Mission::ParadropApproach, 0);
			return 5;
		}

		this->SetTarget(0);
		this->SetDestination(0, 1);
		this->QueueMission(Mission::Retreat, 0);
		return 5;
	}

	int delay = 5;

	if (MapClass::Instance->IsWithinUsableArea(this->Location)) {

		this->DropOffParadropCargo();
		if (this->Passengers.NumPassengers) {
			delay = pTypeExt->ParadropDelay.Get(RulesExtData::Instance()->ParadropDelay);
		} else {
			delay = pTypeExt->ParadropEndDelay.Get(RulesExtData::Instance()->ParadropEndDelay);

			if (delay < 0)
				delay = INT32_MAX;
		}
	}

	return delay;
}

int FakeAircraftClass::_Mission_ParadropApproach()
{
	if (auto pTarCom = this->Target)
	{
		if (auto pDest = this->Destination)
		{
			const int distance = this->DistanceFrom(pTarCom);
			const int nRadius = AircraftTypeExtContainer::Instance.Find(this->Type)->ParadropRadius.Get(RulesClass::Instance->ParadropRadius);

			if (distance <= nRadius)
			{
				this->QueueMission(Mission::ParadropOverfly, 0);
				--this->NumParadropsLeft;
			}

			return 3;
		}
		else
		{
			this->SetDestination(pTarCom, 1);
			return 3;
		}
	}
	else
	{
		this->SetDestination(0, 1);
		this->QueueMission(Mission::Retreat, 0);
		return 3;
	}
}

static FORCEDINLINE bool CheckSpyPlaneCameraCount(AircraftClass* pThis, WeaponTypeClass* pWeapon)
{
	auto const pExt = AircraftExtContainer::Instance.Find(pThis);

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!pWeaponExt->Strafing_Shots.isset())
		return true;

	if (pExt->Strafe_BombsDroppedThisRound >= pWeaponExt->Strafing_Shots)
		return false;

	pExt->Strafe_BombsDroppedThisRound++;
	return true;
}

int FakeAircraftClass::_Mission_SpyPlaneOverfly()
{
	int range = this->DistanceFrom(this->Target);

	const auto pPrimary = this->GetWeapon(0);

	if (range <= pPrimary->WeaponType->Range.value && CheckSpyPlaneCameraCount(this, pPrimary->WeaponType)) {

		this->vt_entry_48C(nullptr, 0u, false, nullptr);
		this->UpdateSight(false, 0, false, nullptr, pPrimary->WeaponType->Damage);

		MapRevealer const revealer(this->Location);
		revealer.UpdateShroud(0u, static_cast<size_t>(MaxImpl(this->LastSightRange + 3, 0)), false);
	}

	if (!this->NavCom) {
		auto edge = this->Owner->ResolveEdge();
		auto loc = MapClass::Instance->PickCellOnEdge(edge
			, CellStruct::Empty
			, CellStruct::Empty
			, SpeedType::Winged
			, true
			, MovementZone::Normal
		);

		if (loc.IsValid()) {
			this->SetDestination(MapClass::Instance->GetCellAt(loc), true);
		}
	}

	return 3;
}

int FakeAircraftClass::_Mission_SpyPlaneApproach()
{

	int range = this->DistanceFrom(this->Target);
	if (this->TarCom) {
		if(this->NavCom) {
			const auto pPrimary = this->GetWeapon(0);

			if (range <= pPrimary->WeaponType->Range.value && CheckSpyPlaneCameraCount(this, pPrimary->WeaponType)) {

				this->vt_entry_48C(nullptr, 0u, false, nullptr);
				this->UpdateSight(false, 0, false, nullptr, pPrimary->WeaponType->Damage);

				MapRevealer const revealer(this->Location);
				revealer.UpdateShroud(0u, static_cast<size_t>(MaxImpl(this->LastSightRange + 3, 0)), false);
			}
		} else {
			this->SetDestination(this->TarCom, true);
		}

	} else {
		this->SetDestination(0, 1);
		this->QueueMission(Mission::Retreat, 0);
	}

	if (range <= 768) {
		this->IsLocked = true;
		auto edge = this->Owner->ResolveEdge();
		auto loc = MapClass::Instance->PickCellOnEdge(edge
			, CellStruct::Empty
			, CellStruct::Empty
			, SpeedType::Winged
			, true
			, MovementZone::Normal
		);

		if (loc.IsValid()) {
			this->SetDestination(MapClass::Instance->GetCellAt(loc), true);
		}
	}

	return RulesClass::Instance->SpyPlaneCameraFrames;
}

int FakeAircraftClass::_Mission_Move_ForCarryAll()
{
	auto GetMissionDelay = [=]() -> int {
			const auto control = this->GetCurrentMissionControl();
			return static_cast<int>(control->Rate * TICKS_PER_MINUTE) +
				ScenarioClass::Instance->Random.RandomRanged(0, 2);
		};

	auto EnterIdleAndDelay = [=]() -> int {
			this->EnterIdleMode(0, 1);
			return GetMissionDelay();
		};

	auto FindLZ = [=]() -> int {

			AbstractClass* const pDest = AircraftExtData::IsValidLandingZone(this) ?
				this->Destination : this->NewLandingZone_(this->Destination);

			this->SetDestination(pDest, true);

			if (auto pTeam = this->Team) {
				pTeam->AssignMissionTarget(this->NavCom);
			}

			this->MissionStatus = 1;
			return GetMissionDelay();
		};

	//AircraftClass_MI_Move_Carryall_AllowWater_LZClear
	auto LZClear = [this](AbstractClass* navCom) {
			return AircraftExtData::IsValidLandingZone(this) || this->IsLandingZoneClear(navCom);
		};

	switch (this->MissionStatus)
	{
	case 0: // VALIDATE_LZ
	{
		auto* const pNavCom = this->NavCom;
		if (!pNavCom)
			return EnterIdleAndDelay();

		auto* const pTarget = flag_cast_to<TechnoClass*>(pNavCom);

		// Non-unit or ineligible target: fly directly to LZ without radio pickup
		if (!pTarget
			|| this->Passengers.NumPassengers
			|| !this->Owner->IsAlliedWith(pTarget)
			|| !pTarget->GetOwningHouse()->IsAlliedWith(this)
			|| pTarget->WhatAmI() != AbstractType::Unit)
		{
			return FindLZ();
		}

		// Extension: reject unit types this carryall cannot lift
		if (!TechnoTypeExtData::CarryallCanLift(this->Type, (UnitClass*)pTarget))
			return EnterIdleAndDelay();

		// Valid unit target: perform radio handshake
		if (this->GetRadioContact() != pTarget)
			this->SendToFirstLink(RadioCommand::NotifyUnlink);

		if (this->SendCommand(RadioCommand::RequestLink, pTarget) != RadioCommand::AnswerPositive) {
			this->SetDestination(nullptr, true);
			return EnterIdleAndDelay();
		}

		if (this->SendToFirstLink(RadioCommand::RequestTether) == RadioCommand::AnswerPositive) {
			this->SendToFirstLink(RadioCommand::NotifyBeginLoad);
			return FindLZ();
		} else {
			this->SendToFirstLink(RadioCommand::NotifyUnlink);
			return EnterIdleAndDelay();
		}
	}
	case 1: // Setup movement to LZ
	{
		auto* navCom = this->NavCom;

		if (!navCom) {
			this->EnterIdleMode(0, 1);
		} else {
			this->Locomotor->Move_To(navCom->GetCoords());
			this->MissionStatus = 2;
		}

		return 1;
	}
	case 2: // FLY_TO_LZ
	{
		auto* navCom = this->Destination;
		auto pCellDest = cast_to<CellClass*>(navCom);

		// Lost contact check
		if (!pCellDest && this->GetRadioContact() != navCom)
		{
			this->MissionStatus = 0;
			return 1;
		}

		// Cell-based target handling
		if (pCellDest && !LZClear(navCom))
		{
			this->MissionStatus = 0;
			return 1;
		}

		// Begin landing when arrived
		if (this->Locomotor->Get_Status() == 1)
		{			// Check if unit target moved
			if (auto pUnitNav = cast_to<UnitClass*>(this->NavCom))
			{
				const CellStruct ourCell = CellClass::Coord2Cell(this->Location);
				const CellStruct targetCellX = CellClass::Coord2Cell(pUnitNav->Location);

				if (ourCell.DifferTo(targetCellX)) {
					this->MissionStatus = 0;
					return 1;
				}
			}
			this->IsCarryallNotLanding = 0;
		}

		// Check if stopped moving
		if (!this->Locomotor->Is_Moving())
		{
			this->MissionStatus = 3;
		}
		return 1;
	}
	case 3: // LAND
	{

		if (this->Passengers.NumPassengers) {
			// Drop off cargo
			if (this->BunkerLinkedItem) {
				this->IsCarryallNotLanding = 1;
			} else {
				this->Mark(MarkType::Remove);
				this->_DropOffCarryallCargo();
				this->Mark(MarkType::Put);
				this->MissionStatus = 0;
			}
			return 1;
		}

		// Pick up cargo
		this->Mark(MarkType::Remove);

		const auto& coord = this->Location;
		CoordStruct cellCoord = { coord.X, coord.Y, coord.Z };

		auto* cell = MapClass::Instance->GetCellAt(cellCoord);
		auto* unit = cell->GetUnit(cell->UINTFlags & 1);

		bool pickupSuccess = false;
		if (unit && unit == this->GetRadioContact()) {
			if (this->SendToFirstLink(RadioCommand::QueryMoving) == RadioCommand::AnswerPositive) {

				unit->Limbo();
				unit->OnBridge = 0;
				unit->IsOnCarryall = 1;
				this->Passengers.AddPassenger(unit);
				pickupSuccess = true;
			}
		}

		if (!pickupSuccess)
			this->MissionStatus = 0;

		this->SendToFirstLink(RadioCommand::NotifyUnlink);
		this->Mark(MarkType::Put);
		this->EnterIdleMode(0, 1);
		this->IsCarryallNotLanding = 1;
		return 1;
	}

	default:
		return GetMissionDelay();
	}
}

enum class OverrideFlag : BYTE {
	Original , ContinueMoving , Idle
};

OverrideFlag OverrideMoving(AircraftClass* const pThis , CoordStruct* const pCoords) {
	const auto pType = pThis->Type;
	if (pThis->Team || pThis->Airstrike || pThis->Spawned || !pType->AirportBound){
		return OverrideFlag::Original;
	}

	const auto extendedMissions = AircraftTypeExtData::ExtendedAircraftMissionsEnabled(pThis);
	const auto pTypeExt = AircraftTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->ExtendedAircraftMissions_SmoothMoving.Get(extendedMissions)) {
		return OverrideFlag::Original;
	}

	const auto rotRadian = Math::abs(pThis->PrimaryFacing.ROT.Raw * (Math::GAME_TWOPI / 65536)); // GetRadian<65536>() is an incorrect methodw
	const auto turningRadius = rotRadian > 1e-10 ? static_cast<int>(pType->Speed / rotRadian) : 0;
	const int distance = int(Point2D { pCoords->X, pCoords->Y }.DistanceFrom(Point2D { pThis->Location.X, pThis->Location.Y }));

	if (distance > MaxImpl((pType->SlowdownDistance / 2), turningRadius)) {
		return OverrideFlag::ContinueMoving;
	}

	if (!extendedMissions || !pThis->TryNextPlanningTokenNode())
		pThis->EnterIdleMode(false, true);

	return OverrideFlag::Idle;
}

int FakeAircraftClass::_Mission_Retreat()
{
	// --- Branch A: NavCom already set ---
	if (this->NavCom) {
		// If already AT the nav destination, clear it
		CellClass* currentCell = this->GetCell();

		if (this->NavCom == currentCell) {
			this->SetDestination(nullptr, true);
		}
		return 3;
	}

	// --- Branch B: No NavCom — find a retreat cell ---
	auto edge = this->Owner->GetHouseEdge();

	// Calculate a retreat cell on the map edge
	CellStruct retreatCell = AircraftExtData::PickEdgeCellForPlane(this->Type, this	->GetMapCoords(), edge, true);

	// Only assign if we got a valid (non-default) cell back
	if (retreatCell.IsValid()) {
		this->SetDestination(MapClass::Instance->GetCellAt(retreatCell), true);
	}

	return 3;
}

int FakeAircraftClass::_Mission_Move()
{
	if (this->Type->Carryall) {
		return this->_Mission_Move_ForCarryAll();
	}

	switch (this->MissionStatus)
	{
	case 0:
	{
		if (auto pNavCom = this->NavCom) {
			this->SetDestination(this->NewLandingZone_(pNavCom), true);
			this->MissionStatus = 1;
		} else {
			this->EnterIdleMode(0, 1);
		}
		// Fall through to default case
	}
	// Intentional fall-through

	default:
	{
		auto Current_Mission_Control = this->GetCurrentMissionControl();
		return  int(Current_Mission_Control->Rate * TICKS_PER_MINUTE) + ScenarioClass::Instance->Random.RandomRanged(0, 2);
	}
	case 1:
	{
		if (auto pNavCom = this->NavCom) {
			this->Locomotor->Move_To(pNavCom->GetDestination(this));
			this->MissionStatus = 2;
			return 1;
		} else {
			this->EnterIdleMode(0, 1);
			return 1;
		}
	}
	case 2:
	{
		// Waypoint handling
		if (this->PlanningPathIdx != -1)
		{
			bool shouldHandleWaypoint = false;

			if (auto pNavCom = this->NavCom) {
				auto distanceCoord = this->Location - pNavCom->GetCoords();
					 distanceCoord.Z = 0;

				if ((int)distanceCoord.Length() < 0x100) {
					shouldHandleWaypoint = true;
				}
			} else {
				shouldHandleWaypoint = true;
			}

			if (shouldHandleWaypoint) {
				auto pPath = HouseClass::CurrentPlayer->PlanningPaths[this->PlanningPathIdx];
				this->FootClass_4DC8C0(pPath->GetWaypoint(this->WaypointIndex));
			}
		}

		if (!this->Locomotor->Is_Moving()) {
			this->MissionStatus = 3;
			return 1;
		}

		if (auto pNavCom = this->NavCom) { 
			bool ContinueMoving = false;
			auto _dest = pNavCom->GetDestination(this);

			switch (OverrideMoving(this, &_dest))
			{
			case OverrideFlag::Original:
			{
				ContinueMoving = (CellClass::Coord2Cell(pNavCom->GetDestination(this)) == this->GetMapCoords());
			}break;
			case OverrideFlag::ContinueMoving:
			{
				ContinueMoving = true;
			}break;
			default:
				return 1;
				break;
			}

			if (!ContinueMoving) {
				this->MissionStatus = 3;//enter idle mode
				return 1;
			}

			auto Dest = NavCom->GetDestination(this);

			if (this->CellSeemsOk_(CellClass::Coord2Cell(Dest), true))
			{
				this->MissionStatus = 4;
				return 1;
			}
			else
			{
				this->MissionStatus = 0;
				return 1;
			}
		} else {
			// NavCom cleared while flying: proceed to landing state
			this->MissionStatus = 4;
			return 1;
		}
	}
	case 3:
	{
		if (!this->Locomotor->Is_Moving()) {
			this->EnterIdleMode(0, 1);
		}
		return 1;
	}
	case 4:
	{
		bool ContinueMoving = true;

		if(this->Locomotor->Is_Moving()){
			if (auto pNav = this->NavCom) {
				auto _dest = pNav->GetDestination(this);

				switch (OverrideMoving(this, &_dest))
				{
				case OverrideFlag::Original: {
					ContinueMoving = (CellClass::Coord2Cell(pNav->GetDestination(this)) == this->GetMapCoords());
				}break;
				case OverrideFlag::ContinueMoving: {
					ContinueMoving = true;
				}break;
				default:
					return 1;
					break;
				}
			}
		} else {
			ContinueMoving = false;
		}

		if (!ContinueMoving) {
			this->MissionStatus = 3; //enter idle mode
		}
		else
		{
			if (auto pNav = this->NavCom)
			{
				auto destCell = CellClass::Coord2Cell(pNav->GetDestination(this));

				if (!this->CellSeemsOk_(destCell, 1)) {
					this->MissionStatus = 0;
					return 1;
				}
			}
		}
		return 1;
	}
	}
}

COMPILETIMEEVAL FORCEDINLINE bool IsFlyLoco(const ILocomotion* pLoco) {
	return (((DWORD*)pLoco)[0] == FlyLocomotionClass::ILoco_vtable);
}

//COMPILETIMEEVAL FORCEDINLINE bool AircraftCanStrafeWithWeapon(WeaponTypeClass* pWeapon) {
//	return pWeapon && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing
//		.Get(pWeapon->Projectile->ROT <= 1 && !pWeapon->Projectile->Inviso);
//}

NOINLINE void CalculateVelocity(AircraftClass* pThis , BulletClass* pBullet , AbstractClass* pTarget) {
	auto const pBulletTypeExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);

	if (pBullet->HasParachute ||(pBullet->Type->Vertical && pBulletTypeExt->Vertical_AircraftFix)) {
		return;
	}

	auto const pBulletExt = BulletExtContainer::Instance.Find(pBullet);
	const auto pLoco = pThis->Locomotor.GetInterfacePtr();

	if (pBullet->Type->ROT == 0 && !PhobosTrajectory::IgnoreAircraftROT0(pBulletExt->Trajectory))
	{
		const auto pLocomotor = static_cast<LocomotionClass*>(pLoco);
		double aircraftSpeed = !pBullet->Type->Cluster && IsFlyLoco(pLoco) ?
			pThis->Type->Speed * static_cast<FlyLocomotionClass*>(pLoco)->CurrentSpeed * TechnoExtData::GetCurrentSpeedMultiplier(pThis)
			: pLocomotor->Apparent_Speed();

		VelocityClass* velocity = &pBullet->Velocity;

		velocity->SetIfZeroXYZ();

		const double dist = velocity->Length();
		const double scale = aircraftSpeed / dist;

		velocity->X *= scale;
		velocity->Y *= scale;
		velocity->Z *= scale;

		DirStruct dir = velocity->GetDirectionFromXY();
		const int facingOffset = dir.Raw - Math::BINARY_ANGLE_MASK;
		const double yawRad = facingOffset * Math::DIRECTION_FIXED_MAGIC;
		const double mag = velocity->Length();

		if (yawRad != 0.0)
		{
			velocity->X /= Math::cos(yawRad);
			velocity->Y /= Math::cos(yawRad);
		}

		velocity->X *= Math::COS_DIRECTION_FIXED_MAGIC;
		velocity->Y *= Math::SIN_DIRECTION_FIXED_MAGIC;
		velocity->Z = Math::SIN_DIRECTION_FIXED_MAGIC * mag;

		const DirStruct newFacingDir = pThis->SecondaryFacing.Current();

		velocity->SetIfZeroXY();

		const double dist2D = velocity->LengthXY();
		const int newFacing = newFacingDir.Raw - Math::BINARY_ANGLE_MASK;
		const double newRad = newFacing * Math::DIRECTION_FIXED_MAGIC;

		velocity->X = Math::cos(newRad) * dist2D;
		velocity->Y = -Math::sin(newRad) * dist2D;

	} else if (pBullet->Type->ROT == 1)
		{

			// Homing weapon: calculate angle and scale
			CoordStruct src = pThis->GetCoords();
			CoordStruct tgt = pTarget->GetCoords();

			CoordStruct offset = tgt - src;

			// Copy offset components into double vector for math
			Vector3D aimVector = {
				static_cast<double>(offset.X),
				static_cast<double>(offset.Y),
				static_cast<double>(offset.Z)
			};

			// Calculate yaw angle to face the target in XY plane
			double yawRadians = Math::atan2(-aimVector.Y, aimVector.X) - Math::DEG90_AS_RAD;
			int yawBinaryAngle = static_cast<int>(yawRadians * Math::BINARY_ANGLE_MAGIC);
			int adjustedYaw = yawBinaryAngle - Math::BINARY_ANGLE_MASK;
			double adjustedYawRad = adjustedYaw * Math::DIRECTION_FIXED_MAGIC;

			// Prepare bullet velocity (set if all-zero)
			VelocityClass* velocity = &pBullet->Velocity;

			velocity->SetIfZeroXY();

			double originalSpeed2D = velocity->LengthXY();

			// Set initial XY velocity facing target yaw
			velocity->X = Math::cos(adjustedYawRad) * originalSpeed2D;
			velocity->Y = -Math::sin(adjustedYawRad) * originalSpeed2D;

			// Calculate pitch angle from aim vector
			double horizontalDistance = aimVector.LengthXY();
			double pitchRadians = Math::atan2(aimVector.Z, horizontalDistance) - Math::DEG90_AS_RAD;
			int pitchBinaryAngle = static_cast<int>(pitchRadians * Math::BINARY_ANGLE_MAGIC);
			int adjustedPitch = pitchBinaryAngle - Math::BINARY_ANGLE_MASK;
			double adjustedPitchRad = adjustedPitch * Math::DIRECTION_FIXED_MAGIC;

			// Re-calculate current yaw from bullet velocity
			DirStruct currentFacing = velocity->GetDirectionFromXY();
			int currentFacingOffset = currentFacing.Raw - Math::BINARY_ANGLE_MASK;
			double currentYawRad = currentFacingOffset * Math::DIRECTION_FIXED_MAGIC;

			double currentSpeed3D = velocity->Length();

			// If yaw was altered, rescale velocity
			if (currentYawRad != 0.0)
			{
				double cosYaw = Math::cos(currentYawRad);
				velocity->X /= cosYaw;
				velocity->Y /= Math::cos(currentYawRad); // redundant, matches original logic
			}

			// Apply pitch to Z velocity
			velocity->X *= Math::cos(adjustedPitchRad);
			velocity->Y *= Math::cos(adjustedPitchRad);
			velocity->Z = Math::sin(adjustedPitchRad) * currentSpeed3D;

			// Normalize speed to weapon's max speed
			WeaponTypeClass* weapon = pThis->GetPrimaryWeapon()->WeaponType;
			double maxBulletSpeed = static_cast<double>(weapon->Speed);

			velocity->SetIfZeroXYZ();

			double finalSpeed = velocity->Length();
			double speedScale = maxBulletSpeed / finalSpeed;

			velocity->X *= speedScale;
			velocity->Y *= speedScale;
			velocity->Z *= speedScale;
	}
}

BulletClass* FakeAircraftClass::_FireAt(AbstractClass* pTarget, int nWeaponIdx) {

	auto const pTypeExt = AircraftTypeExtContainer::Instance.Find(this->Type);
	bool DropPassengers = pTypeExt->Paradrop_DropPassangers;

	if (this->Passengers.FirstPassenger)
	{
		if (auto pWewapons = this->GetWeapon(nWeaponIdx))
		{
			if (pWewapons->WeaponType)
			{
				const auto pExt = WeaponTypeExtContainer::Instance.Find(pWewapons->WeaponType);
				if (pExt->KickOutPassenger.isset())
					DropPassengers = pExt->KickOutPassenger; //#1151
			}
		}

		if (DropPassengers)
		{
			this->DropOffParadropCargo();
			return nullptr;
		}
	}

	BulletClass* pBullet = this->TechnoClass::Fire(pTarget, nWeaponIdx);

	if(pBullet) {

		if (AircraftCanStrafeWithWeapon(pBullet->WeaponType))
		{
			AircraftExtContainer::Instance.Find(this)->Strafe_BombsDroppedThisRound++;

			if (WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType)->Strafing_UseAmmoPerShot)
			{
				this->loseammo_6c8 = false;
				this->Ammo--;
			}
		}

		if(!pTypeExt->Firing_IgnoreGravity)
			CalculateVelocity(this, pBullet, pTarget);
	}
	// Reveal map for attacking aircraft if controlled by player

	if (this->Owner->ControlledByCurrentPlayer())
	{
		CoordStruct coord = this->Location;

		if (!MapClass::Instance->IsLocationShrouded(coord)) {
			bool mapped = false;
			constexpr CoordStruct offsets[4] = {
				{512, 512 , 0}, {-512, -512 , 0}, {512, -512 , 0}, {-512, 512 , 0}
			};

			for (auto& off : offsets) {
				CoordStruct probe = off + coord;

				if (MapClass::Instance->IsLocationShrouded(probe)) {
					mapped = true;
					break;
				}
			}

			if (!mapped) {
				mapped = MapClass::Instance->IsLocationShrouded(pTarget->GetCoords());
			}

			if (mapped) {
				const int sightRange = AircraftTypeExtContainer::Instance.Find(this->Type)->AttackingAircraftSightRange.Get(RulesClass::Instance->AttackingAircraftSightRange);
				MapClass::Instance->RevealArea2(&coord, sightRange, this->Owner, 0, 0, 0, 1, 0);
				MapClass::Instance->RevealArea2(&coord, sightRange, this->Owner, 0, 0, 0, 1, 1);
			}
		}
	}

	if (this->IsKamikaze) {
		this->UnInit();
	}

	return pBullet;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x415EE0, FakeAircraftClass::_FireAt);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2670, FakeAircraftClass::_FireAt);

void FakeAircraftClass::_SetTarget(AbstractClass* pTarget)
{
	this->TechnoClass::SetTarget(pTarget);
	AircraftExtContainer::Instance.Find(this)->CurrentAircraftWeaponIndex = -1;
}

void FakeAircraftClass::_Destroyed(int mult)
{
	AircraftExtData::TriggerCrashWeapon(this, mult);
}

WeaponStruct* FakeAircraftClass::_GetWeapon(int weaponIndex)
{
	auto const pExt = AircraftExtContainer::Instance.Find(this);

	if (pExt->CurrentAircraftWeaponIndex >= 0)
		return this->TechnoClass::GetWeapon(pExt->CurrentAircraftWeaponIndex);
	else
		return this->TechnoClass::GetWeapon(this->SelectWeapon(this->Target));
}

// Spy plane, airstrike etc.
bool AircraftExtData::PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell)
{
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	auto coords = CellClass::Cell2Coord(edgeCell);
	AbstractClass* pTarget = pThis->Target ? pThis->Target : pThis->Destination;
	auto dir = DirType::North;

	if (pTarget) {
		auto const pTargetCoords = pTarget->GetCoords();

		if (pTypeExt->SpawnDistanceFromTarget.isset())
			coords = GeneralUtils::CalculateCoordsFromDistance(CellClass::Cell2Coord(edgeCell), pTargetCoords, pTypeExt->SpawnDistanceFromTarget.Get());
		
		dir = GeneralUtils::GetDirectionBetweenCoords(coords, pTargetCoords).GetDir();
	}

	++Unsorted::ScenarioInit;
	const bool result = pThis->Unlimbo(coords, dir);
	--Unsorted::ScenarioInit;

	pThis->SetHeight(pTypeExt->SpawnHeight.Get(pThis->Type->GetFlightLevel()));

	if (pTarget)
		pThis->PrimaryFacing.Set_Desired(pThis->GetDirectionOverObject(pTarget));

	return result;
}

CellStruct AircraftExtData::PickEdgeCellForPlane(AircraftTypeClass* pPlaneType, CellStruct destCell, Edge edge, bool isOnRetreat)
{
	auto const pTypeExt = AircraftTypeExtContainer::Instance.Find(pPlaneType);
	auto const edgeMode = !isOnRetreat ? pTypeExt->SpawnFromEdge : pTypeExt->RetreatToEdge;
	auto spawnEdge = edge;
	auto refCell = CellStruct::Empty;

	switch (edgeMode)
	{
	case EdgeType::Closest:
	{
		if (destCell != CellStruct::Empty)
		{
			spawnEdge = Edge::None;
			refCell = destCell;

			// Scatter the coords a bit to randomize spawn cell a little - otherwise multiple planes sent at same target
			// from same source might end up overlapping - still a possibility, just less likely.
			// The edge cell picking function itself will do no randomization on Edge::None + waypoint cell set mode.
			int const randomRange = 5;
			short const randomX = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-randomRange, randomRange));
			short const randomY = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-randomRange, randomRange));
			refCell += CellStruct { randomX, randomY };
		}
		break;
	}
	case EdgeType::Random:
	{
		int const min = static_cast<int>(Edge::North);
		int const max = static_cast<int>(Edge::West);
		spawnEdge = static_cast<Edge>(ScenarioClass::Instance->Random.RandomRanged(min, max));
		break;
	}
	default:
	{
		break;
	}
	}

	return MapClass::Instance->PickCellOnEdge(spawnEdge, refCell, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
}

void AircraftExtData::TriggerCrashWeapon(AircraftClass* pThis, int nMult)
{
	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pCrashWeapon = pTypeExt->CrashWeapon.Get(pThis);
	if (!pCrashWeapon)
		pCrashWeapon = pTypeExt->CrashWeapon_s.Get();

	if (!TechnoExtData::FireWeaponAtSelf(pThis, pCrashWeapon))
		pThis->FireDeathWeapon(nMult);

	AnimTypeExtData::ProcessDestroyAnims(pThis, nullptr);
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	if (!pTarget)
		return;

	AircraftExtData::FireBurst(pThis, pTarget, shotNumber, pThis->SelectWeapon(pTarget));
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	const auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	const auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	AircraftExtData::FireBurst(pThis , pTarget, shotNumber, WeaponIdx, weaponType);
}

void AircraftExtData::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
{
	if (!pWeapon->Burst)
		return;

	for (int i = 0; i < pWeapon->Burst; i++)
	{
		if (pWeapon->Burst < 2 && WeaponTypeExtContainer::Instance.Find(pWeapon)->Strafing_SimulateBurst)
			pThis->CurrentBurstIndex = (int)shotNumber;

		pThis->Fire(pTarget, WeaponIdx);
	}
}


bool AircraftExtData::IsValidLandingZone(AircraftClass* pThis)
{
	if (const auto pPassanger = pThis->Passengers.GetFirstPassenger())
	{
		if (const auto pDest = pThis->Destination)
		{
			const auto pDestCell = MapClass::Instance->GetCellAt(pDest->GetCoords());

			return pDestCell->IsClearToMove(GET_TECHNOTYPE(pPassanger)->SpeedType,
			false, false, ZoneType::None, GET_TECHNOTYPE(pPassanger)->MovementZone, -1, false)
				&& pDestCell->OverlayTypeIndex == -1;
		}
	}

	return false;

}

AircraftExtContainer AircraftExtContainer::Instance;

ASMJIT_PATCH(0x413DB1, AircraftClass_CTOR, 0x6)
{
	GET(AircraftClass*, pItem, ESI);
	if(!Phobos::Otamaa::DoingLoadGame)
	AircraftExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x41412A, AircraftClass_DTOR, 0x6)
{
	GET(AircraftClass*, pItem, EDI);
	AircraftExtContainer::Instance.Remove(pItem);
	return 0;
}

HRESULT __stdcall FakeAircraftClass::__Load(IStream* pStm)
{
	HRESULT hr = this->AircraftClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!AircraftExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E22B8, FakeAircraftClass::__Load)

HRESULT __stdcall FakeAircraftClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->AircraftClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!AircraftExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E22BC, FakeAircraftClass::__Save)

void FakeAircraftClass::_Detach(AbstractClass* target, bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all, target->WhatAmI());
	//will detach type pointer
	this->AircraftClass::PointerExpired(target, all);
}
DEFINE_FUNCTION_JUMP(VTABLE , 0x7E22CC , FakeAircraftClass::_Detach)
