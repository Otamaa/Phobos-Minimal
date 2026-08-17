#include "Body.h"

#include <Base/Always.h>

#include <Helpers/Macro.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/AircraftType/Body.h>

#include <SpawnManagerClass.h>

void FakeAircraftClass::_AI()
{
	// -----------------------------------------------------------------------
		// Reset spy counter for non-relevant missions
		// -----------------------------------------------------------------------
	switch (this->CurrentMission)
	{
	case Mission::Attack:
	case Mission::ParadropOverfly:
	case Mission::SpyplaneApproach:
	case Mission::SpyplaneOverfly:
		break;
	default:
		this->DoingOverfly = 0;
		break;
	}

	// -----------------------------------------------------------------------
	// Tick temporal targeting object if present
	// -----------------------------------------------------------------------
	TechnoExtData::IsTechnoShouldBeAliveAfterTemporal(this);

	// -----------------------------------------------------------------------
	// ChronoSparkle anim every 24 frames
	// -----------------------------------------------------------------------
	const bool isWarpedOut = this->IsBeingWarpedOut();
	const bool isWarpingIn = this->IsWarpingIn();

	if ((isWarpedOut || isWarpingIn)) {
		TechnoExtData::PlayChronoSparkleAnim(this, &this->Location, 0, FakeRulesClass::Instance()->ChronoSparkleDisplayDelay);
	}

	const bool needsLocoProcess = isWarpingIn
		|| (isWarpedOut && this->IsImmobilized);

	if (needsLocoProcess) {
		this->Locomotor->Process();
	}

	if (this->IsAlive) {
		// Clear-targets condition (original LABEL_69 outer if)
		if ((isWarpingIn && this->TemporalTargetingMe) || isWarpedOut) {
			if (this->TarCom)
				this->SetTarget(0);

			if (this->NavCom)
				this->SetDestination(0, 1);

			return; // no further processing in chrono/temporal state
		}

		// --------------------------------------------------------------------
		// Normal else path: target validity
		// --------------------------------------------------------------------
		if (auto pTargetTechno = flag_cast_to<TechnoClass*>(this->TarCom)) {
			if (!this->Owner->IsAlliedWith(pTargetTechno)
				&& pTargetTechno->CloakState == CloakState::Cloaked
				&& !pTargetTechno->IsSensorVisibleToHouse(this->Owner)) {
				this->SetTarget(0);
			}
		}

		//auto pExt = TechnoExtContainer::Instance.Find(this);
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(this->Type);

		//pExt->UpdateAircraftOpentopped();
		//AircraftPutDataFunctional::AI(pExt, pTypeExt);
		//AircraftDiveFunctional::AI(pExt, pTypeExt);
		//FighterAreaGuardFunctional::AI(pExt, pTypeExt);

		//if (pThis->IsAlive && pThis->SpawnOwner != nullptr)
		//{
		//
		//	/**
		//	 *  If we are close enough to our owner, delete us and return true
		//	 *  to signal to the challer that we were deleted.
		//	 */
		//	if (Spawned_Check_Destruction(pThis))
		//	{
		//		pThis->UnInit();
		//		return 0x414F99;
		//	}
		//}

		FakeFootClass::_AI(this);

		if (this->IsAlive && this->Type->AirportBound && !this->Airstrike && !this->Spawned) {
			bool extendedMissions = AircraftTypeExtData::ExtendedAircraftMissionsEnabled(this);

			if (extendedMissions) {
				if (const auto pArchive = this->ArchiveTarget) {
					if (this->Target && !this->IsFiring && !this->DoingOverfly
						&& this->DistanceFromSquared(pArchive) > static_cast<int>(this->GetGuardRange(1) * 1.1)) {
						this->SetTarget(nullptr);
						this->SetDestination(pArchive, true);
					}
				}

				this->FindDockingBayInVector(reinterpret_cast<TypeList<TechnoTypeClass*>*>(&this->Type->Dock), 0, 0);
			}

			if (this->DockedTo) {
				if (this->GetCurrentMission() == Mission::Area_Guard && this->MissionStatus) {
					this->SetArchiveTarget(nullptr);
					this->EnterIdleMode(false, true);
				}
			} else if (this->IsInAir()) {
				int damage = AircraftTypeExtContainer::Instance.Find(this->Type)
					->ExtendedAircraftMissions_UnlandDamage
					.Get(FakeRulesClass::Instance()->ExtendedAircraftMissions_UnlandDamage);

				if (damage > 0) {
					if (!extendedMissions
						&& !this->IsCrushingSomething
						&& this->FindDockingBayInVector(reinterpret_cast<TypeList<TechnoTypeClass*>*>(&this->Type->Dock), 0, 0))
						return;

					// Injury every four frames
					if (!((Unsorted::CurrentFrame.get() - this->LastFireBulletFrame + this->UniqueID) & 0x3))
						this->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				} else if (damage < 0) {
					// Avoid using circular movement paths to prevent the aircraft from crashing
					if (extendedMissions)
						this->Crash(nullptr);
				}
			}
		}

		if (!this->IsAlive)
			return;

		// --------------------------------------------------------------------
		// Sinking logic
		// --------------------------------------------------------------------
		if (this->IsSinking) {
			CoordStruct v29 = this->Location;
			v29.Z -= 5;

			this->SetLocation(v29);
			const int height = this->GetHeight();

			if (height < -400) {
				this->RegisterKill(0);
				this->UnInit();
				return;
			}

			if (auto pWakeType = TechnoTypeExtData::GetSinkAnim(this)) {
				if ((Unsorted::CurrentFrame() & 3) == 0) {

					CoordStruct coord = this->Location;
					coord.X += ScenarioClass::Instance->Random.RandomRanged(-170, 170);
					coord.Y += ScenarioClass::Instance->Random.RandomRanged(-170, 170);
					coord.Z -= height;

					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pWakeType, coord),
						this->GetOwningHouse(),
						nullptr,
						this,
						false, false
					);
				}
			}
		}

		// --------------------------------------------------------------------
		// Trailer anim
		// --------------------------------------------------------------------
		{
			CoordStruct v30 = this->Location;

			if (this->Type->Trailer && !(Unsorted::CurrentFrame() % this->Type->SpawnDelay)) {
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(this->Type->Trailer, v30, 1, 1),
					this->GetOwningHouse(),
					nullptr,
					this,
					false, false
				);
			}
		}

		// --------------------------------------------------------------------
		// Map bounds / FlyBy / FlyBack check
		// SUSPECT: v19 and v27 are IDA decompiler stack artifacts.
		// VERIFY: Grand_Opening_SOMETHINGELSETOO second overload signature —
		//         the (this, v27) call with out-char v19 is unclear; preserved verbatim.
		// --------------------------------------------------------------------
		{
			CellStruct cellBuf = this->GetMapCoords();

			// -- Check 1: radar/flyby guard --
			if (!this->Type->FlyBy
				&& !this->Type->FlyBack
				&& !MapClass::Instance->IsWithinUsableArea(cellBuf, 1)
				&& this->IsLeavingMap()) {
				this->UnInit();
				return;
			}

			// -- Check 2: map cell guard --
			if (!MapClass::Instance->CoordinatesLegal(cellBuf) && this->IsLeavingMap()) {
				this->UnInit();
				return;
			}
		}

		// --------------------------------------------------------------------
		// WorkingCell sentinel handling
		// --------------------------------------------------------------------
		if (this->TarCom == CellClass::Instance() || this->NavCom == CellClass::Instance()) {
			this->SetDestination(0, 1);
			this->SetTarget(0);
			this->_Enter_Idle_Mode(0, 1);
		}

		// --------------------------------------------------------------------
		// Ready to commence
		// --------------------------------------------------------------------
		if (this->ReadyToNextMission())
			this->NextMission();

		// --------------------------------------------------------------------
		// Ammo loss outside MISSION_ATTACK
		// --------------------------------------------------------------------
		if (this->loseammo_6c8 && this->CurrentMission != Mission::Attack) {
			int v20 = this->Ammo;
			this->loseammo_6c8 = 0;
			this->Ammo = v20 - 1;
		}

		// --------------------------------------------------------------------
		// Damage smoke anim
		// --------------------------------------------------------------------

		if (AnimTypeClass* pType = pTypeExt->SmokeAnim.Get(FakeRulesClass::Instance()->DefaultAircraftDamagedSmoke)) {
			const int chance = (this->Health > 0) ?
				pTypeExt->SmokeChanceRed.Get(10) : pTypeExt->SmokeChanceDead.Get(80);

			if (chance > 0) {
				if (this->GetHealthRatio() < RulesClass::Instance->ConditionRed) {
					if (this->GetHeight() > 0) {
						if (ScenarioClass::Instance->Random.RandomFromMax(99) < chance) {
							AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, this->Location),
								this->Owner,
								nullptr,
								nullptr,
								false, false
							);
						}
					}
				}
			}
		}

		// --------------------------------------------------------------------
		// Carryall cargo sync
		// --------------------------------------------------------------------
		if (this->Passengers.FirstPassenger && this->Type->Carryall) {
			FootClass* attached = this->Passengers.FirstPassenger;
			attached->PrimaryFacing = this->SecondaryFacing;
			attached->SecondaryFacing = this->SecondaryFacing;
			attached->SetLocation(this->Location);
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x414BB0, FakeAircraftClass::_AI)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2300, FakeAircraftClass::_AI)

ASMJIT_PATCH(0x41949F, AircraftClass_ReceivedRadioCommand_SpecificPassengers, 6)
{
	GET(AircraftClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSender, 0x14);

	enum { Allowed = 0x41945Fu, Disallowed = 0x41951Fu };

	auto const pType = pThis->Type;

	if (pThis->Passengers.NumPassengers >= pType->Passengers)
	{
		return Disallowed;
	}

	auto const pSenderType = GET_TECHNOTYPE(pSender);

	return TechnoTypeExtData::PassangersAllowed(pType, pSenderType) ? Allowed : Disallowed;
}

ASMJIT_PATCH(0x41946B, AircraftClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(AircraftClass*, pThis, ESI);
	return (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x4190DDu : 0u);
}

ASMJIT_PATCH(0x416CF4, AircraftClass_Carryall_Unload_Guard, 0x5)
{
	GET(FootClass*, pCargo, ESI);

	pCargo->Transporter = 0;
	pCargo->QueueMission(Mission::Guard, true);

	if (auto pTeam = pCargo->Team)
		pTeam->AddMember(pCargo, false);

	return 0;
}

ASMJIT_PATCH(0x416C94, AircraftClass_Carryall_Unload_UpdateCargo, 0x6)
{
	GET(UnitClass*, pCargo, ESI);

	pCargo->UpdatePosition(PCPType::End);

	if (pCargo->Deactivated && pCargo->Locomotor.GetInterfacePtr()->Is_Powered())
	{
		pCargo->Locomotor.GetInterfacePtr()->Power_Off();
	}

	return 0;
}

ASMJIT_PATCH(0x413FA3, AircraftClass_Init_Cloakable, 0x5)
{
	GET(AircraftClass*, Item, ESI);

	if (Item->Type->Cloakable) {
		Item->Cloakable = true;
	}

	return 0;
}

ASMJIT_PATCH(0x415533, AircraftClass_Mi_Unload_Blocked, 0x5)
{
	GET(AircraftClass*, pThis, ESI);
	GET(FootClass*, pCargo, EDI);

	if (pThis->KickOutUnit(pCargo, CellStruct::Empty) != KickOutResult::Failed) {
		pCargo->Transporter = nullptr;
		pCargo->IsOnCarryall = false;
	} else {
		pThis->AddPassenger(pCargo);
		pThis->EnterIdleMode(false, 1);
	}

	return 0x41554F;
}

// #1232: fix for dropping units out of flying Carryalls
ASMJIT_PATCH(0x415DF6, AircraftClass_Paradrop_Carryall, 0x6)
{
	GET(FootClass*, pTechno, ESI);
	pTechno->Transporter = nullptr;
	pTechno->IsOnCarryall = false;
	return 0;
}

// fix for vehicle paradrop alignment
ASMJIT_PATCH(0x415CA6, AircraftClass_Paradrop_Units, 0x6)
{
	GET(AircraftClass*, A, EDI);
	GET(FootClass*, P, ESI);

	if (P->WhatAmI() == UnitClass::AbsID)
	{
		const CoordStruct SrcXYZ = A->GetCoords();
		LEA_STACK(CoordStruct*, XYZ, 0x20);
		*XYZ = SrcXYZ;
		XYZ->Snap();
		XYZ->Z = SrcXYZ.Z - 1;
		R->ECX(XYZ);
		return 0x415DE3;
	}

	return 0;
}

// flying aircraft carriers
// allow spawned units to spawn above ground
ASMJIT_PATCH(0x414338, AircraftClass_Put_SpawnHigh, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, ECX);

	return pType->MissileSpawn || pThis->SpawnOwner ? 0x41438F : 0x414342;
}

// aim for the cell for flying carriers
ASMJIT_PATCH(0x6B7838, SpawnManagerClass_Update_SpawnHigh, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(TechnoClass*, pSpawnee , EDI);

	pSpawnee->SetDestination(pThis->Owner->GetHeight() > 0
		? (AbstractClass*)pThis->Owner->GetCell() : (AbstractClass*)pThis->Owner , true);

	return 0x6B7848;
}

/* #1354 - Aircraft and empty SovParaDropInf list */
ASMJIT_PATCH(0x41D887, AirstrikeClass_Fire, 0x6)
{
	if (!RulesClass::Instance->SovParaDropInf.Count) {
		R->ECX(-1);
		return 0x41D895;
	}

	return 0x0;
}
