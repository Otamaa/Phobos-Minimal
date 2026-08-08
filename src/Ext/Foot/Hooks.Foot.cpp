#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/House/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/Aircraft/Body.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <New/Type/ArmorTypeClass.h>
#include <New/Interfaces/AdvancedDriveLocomotionClass.h>

#include <InfantryClass.h>
#include <RadarEventClass.h>

#include <Locomotor/LocomotionClass.h>
#include <Interface/IPiggyback.h>

//ElectricAssultTemp
void ElectrictAssaultCheck(FootClass* pThis, bool updateIdleAction)
{
	if (pThis->Target)
		return;

	auto pWeapon = pThis->GetWeapon(1);

	if (pWeapon && pWeapon->WeaponType && pWeapon->WeaponType->Warhead->ElectricAssault)
	{

		auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->WeaponType->Warhead);
		auto myLoc = pThis->GetMapCoords();

		for (int i = 0; i < 8; ++i)
		{
			if (auto pBld = MapClass::Instance->GetCellAt(myLoc + CellSpread::AdjacentCell[i])->GetBuilding())
			{
				if (pBld->Type->Overpowerable && pBld->Owner->IsAlliedWith(pThis->Owner))
				{

					if (pWHExt->ElectricAssault_Requireverses && pWHExt->GetVerses(TechnoExtData::GetTechnoArmor(pBld, pWeapon->WeaponType->Warhead))
					.Verses < 0.001)
						continue;

					pThis->SetTarget(pBld);
					pThis->__AssignNewThreat = true;
					pThis->QueueMission(Mission::Attack, false);
					return;
				}
			}
		}

	}
	else if (updateIdleAction)
	{
		pThis->UpdateIdleAction();
	}
}

ASMJIT_PATCH(0x4D6F38, FootClass_MI_AreaGuard_ElectrictAssault, 0x6)
{
	GET(FootClass*, pThis, ESI);
	ElectrictAssaultCheck(pThis, false);
	return 0x4D7025;
}

ASMJIT_PATCH(0x4D50E1, FootClass_MI_Guard_ElectrictAssault, 0xA)
{
	GET(FootClass*, pThis, ESI);
	ElectrictAssaultCheck(pThis, true);
	return 0x4D5225;
}

// https://bugs.launchpad.net/ares/+bug/895893
ASMJIT_PATCH(0x4DB37C, FootClass_Limbo_ClearCellJumpjet, 0x6)
{
	GET(FootClass*, pThis, EDI);
	auto pCell = pThis->GetCell();

	if (GET_TECHNOTYPE(pThis)->JumpJet)
	{
		if (pCell->Jumpjet == pThis)
		{
			pCell->TryAssignJumpjet(nullptr);
		}
	}

	//FootClass_Remove_Airspace_ares
	return pCell->MapCoords.IsValid() ? 0x4DB3A4 : 0x4DB3AF;
}

ASMJIT_PATCH(0x4DB1A0, FootClass_GetMovementSpeed_SpeedMult, 0x6)
{
	GET(FootClass*, pThis, ECX);

	const auto maxSpeed = pThis->GetDefaultSpeed();
	int speedResult = int(maxSpeed * TechnoExtData::GetCurrentSpeedMultiplier(pThis));

	if (pThis->WhatAmI() == UnitClass::AbsID && ((UnitClass*)pThis)->FlagHouseIndex != -1)
	{
		speedResult /= 2;
	}

	R->EAX((int)speedResult);
	return 0x4DB245;
}

ASMJIT_PATCH(0x4DBF01, FootClass_SetOwningHouse_FixArgs, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	GET_STACK(HouseClass* const, pNewOwner, 0xC + 0x4);
	GET_STACK(bool const, bAnnounce, 0xC + 0x8);

	//Debug::LogInfo("SetOwningHouse for [%s] announce [%s - %d]", pNewOwner->get_ID(), bAnnounce ? "True" : "False" , bAnnounce);
	bool result = false;
	if (pThis->TechnoClass::SetOwningHouse(pNewOwner, bAnnounce))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pThis);

		for (auto& trail : pExt->LaserTrails)
		{
			if (trail->Type->IsHouseColor)
			{
				trail->CurrentColor = pThis->Owner->LaserColor;
			}
		}

		if (pThis->Owner->IsControlledByHuman())
		{
			// This is not limited to mind control, could possibly affect many map triggers
			// This is still not even correct, but let's see how far this can help us

			pThis->ShouldScanForTarget = false;
			pThis->ShouldEnterAbsorber = false;
			pThis->ShouldEnterOccupiable = false;
			pThis->ShouldLoseTargetNow = false;
			pThis->ShouldGarrisonStructure = false;
			pThis->CurrentTargets.clear();
			auto pThisType = GET_TECHNOTYPE(pThis);

			if (pThis->HasAnyLink() || pThisType->ResourceGatherer) // Don't want miners to stop
				return 0x4DBF13;

			switch (pThis->GetCurrentMission())
			{
			case Mission::Harvest:
			case Mission::Sleep:
			case Mission::Harmless:
			case Mission::Repair:
				return 0x4DBF13;
			}

			pThis->Override_Mission(pThisType->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, nullptr, nullptr); // I don't even know what this is, just clear the target and destination for me
		}

		result = true;
	}

	R->AL(result);
	return 0x4DBF0F;
}

ASMJIT_PATCH(0x4DFE00, FootClass_GarrisonStructure_TakeVehicle, 6)
{
	GET(FootClass*, pThis, ECX);

	if (!TechnoExtContainer::Instance.Find(pThis)->TakeVehicleMode)
		return 0x0;

	R->EAX(TechnoExtData::FindAndTakeVehicle(pThis));
	return 0x4DFF3E;
}

ASMJIT_PATCH(0x4D718C, FootClass_Put_InitialPayload, 6)
{
	GET(FootClass* const, pThis, ESI);

	TechnoExtContainer::Instance.Find(pThis)->CreateInitialPayload();

	return 0;
}

ASMJIT_PATCH(0x4D98C0, FootClass_Destroyed_PlayEvent, 0xA)
{
	enum { Skip = 0x4D9918 };
	GET(FootClass*, pThis, ECX);
	//GET_STACK(ObjectClass*, pKiller, 0x4);

	const auto pType = GET_TECHNOTYPE(pThis);
	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->SupressEVALost
		|| pType->DontScore
		|| pType->Insignificant
		|| pType->Spawned
		|| !pThis->Owner
		|| !pThis->Owner->ControlledByCurrentPlayer()
	)
	{
		return Skip;
	}

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (RadarEventClass::Create(RadarEventType::UnitLost, pThis->GetMapCoords()))
		VoxClass::PlayIndex(pTypeExt->EVA_UnitLost);

	return Skip;
}

ASMJIT_PATCH(0x4D5776, FootClass_ApproachTarget_Passive, 0x6)
{
	GET(FootClass* const, pThis, EBX);
	GET_STACK(bool, bSomething, 0x12);

	if (pThis->BunkerLinkedItem || pThis->ShouldLoseTargetNow || pThis->InOpenToppedTransport)
		R->AL(0);

	return (!bSomething)
		? 0x4D5796 : 0x4D57EA;
}

ASMJIT_PATCH(0x4D9EE1, FootClass_CanBeSold_Dock, 0x6)
{
	GET(BuildingClass* const, pBld, EAX);
	GET(CoordStruct*, pBuffer, ECX);
	GET(TechnoClass* const, pDocker, ESI);
	R->EAX(pBld->GetDockCoords(pBuffer, pDocker));
	return 0x4D9EE7;
}

// replace Is_Moving_Now, because it doesn't check the
// current speed in case the unit is turning.
ASMJIT_PATCH(0x4DBDD4, FootClass_IsCloakable_CloakStop, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	R->AL(pThis->Locomotor.GetInterfacePtr()->Is_Moving());
	return 0x4DBDE3;
}

// support Occupier and VehicleThief on one type. if this is not done
// the Occupier handling will leave a dangling Destination pointer.
ASMJIT_PATCH(0x4D9A83, FootClass_PointerGotInvalid_OccupierVehicleThief, 0x6)
{
	GET(InfantryClass* const, pInfantry, ESI);
	GET(InfantryTypeClass* const, pType, EAX);

	if (pType->VehicleThief
		&& pInfantry->Destination
		&& (pInfantry->Destination->AbstractFlags & AbstractFlags::Foot) )
	{
		return 0x4D9AB9;
	}

	return 0;
}

#include <Locomotor/Cast.h>

// update parasite coords along with the host
ASMJIT_PATCH(0x4DB874, FootClass_SetLocation_Extra, 0xA)
{
	enum { SkipGameCode = 0x4DB88F };

	GET(FootClass*, pThis, ESI);
	const auto pParasite = pThis->ParasiteEatingMe;

	// Fix Ares's bug that parasite always on victim's location
	if (pParasite && pParasite->InLimbo)
		pParasite->SetLocation(pThis->Location);

	// Restore overriden instructions
	if (pThis->GetTechnoType()->OpenTopped)
		pThis->UpdatePassengerCoords();

	return SkipGameCode;
}

ASMJIT_PATCH(0x4D8D95, FootClass_UpdatePosition_HunterSeeker, 0xA)
{
	GET(FootClass* const, pThis, ESI);

	// ensure the target won't get away
	if (GET_TECHNOTYPE(pThis)->HunterSeeker) {
		if (auto const pTarget = flag_cast_to<TechnoClass*>(pThis->Target)) {

			const auto pWpS = pThis->GetWeapon(0);

			if(pWpS && pWpS->WeaponType)
			{
				auto damage = pWpS->WeaponType->Damage;
				pTarget->ReceiveDamage(&damage, 0, pWpS->WeaponType->Warhead, pThis, true, true, pThis->Owner);
			}
			else
			{
				auto damage = FakeRulesClass::Instance()->HunterSeeker_Damage.Get();
				pTarget->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, pThis, true, true, pThis->Owner);
			}
		}
	}

	return 0;
}

// stops movement sound from being played while unit is being pulled by a magnetron (see terror drone)
ASMJIT_PATCH(0x7101CF, FootClass_ImbueLocomotor, 0x7)
{
	GET(FootClass* const, pThis, ESI);
	pThis->MoveSoundAudioController.AudioEventHandleEndLooping();
	return 0;
}

ASMJIT_PATCH(0x4D7524, FootClass_ActionOnObject_Allow, 9)
{
	//overwrote the ja, need to replicate it
	GET(Action, CursorIndex, EBP);

	if (CursorIndex == Action::None || CursorIndex > Action::Airstrike) {
		return CursorIndex == Action(127) || CursorIndex == Action(126) ? 0x4D769F : 0x4D7CC0;
	}

	return 0x4D752D;
}

ASMJIT_PATCH(0x4D9920, FootClass_SelectAutoTarget_Cloaked, 9)
{
	GET(FootClass* const, pThis, ECX);

	if (pThis->Owner->IsControlledByHuman()
		&& pThis->GetCurrentMission() == Mission::Guard)
	{
		auto const pType = GET_TECHNOTYPE(pThis);
		auto const pExt = TechnoTypeExtContainer::Instance.Find(pType);

		auto allowAquire = true;

		if (!pExt->CanPassiveAcquire_Guard)
		{
			// we are in guard mode
			allowAquire = false;
		}
		else if (!pExt->CanPassiveAcquire_Cloak)
		{
			// passive acquire is disallowed when guarding and cloakable
			if (pThis->IsCloakable() || pThis->HasAbility(AbilityType::Cloak))
			{
				allowAquire = false;
			}
		}

		if (!allowAquire)
		{
			R->EAX(static_cast<TechnoClass*>(nullptr));
			return 0x4D995C;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4D9EBD, FootClass_CanBeSold_SellUnit, 6)
{
	GET(BuildingClass*, pBld, EAX);
	GET(TechnoClass*, pDocker, ESI);

	const auto nUnitRepair = BuildingTypeExtContainer::Instance.Find(pBld->Type)->UnitSell.Get(pBld->Type->UnitRepair);
	const auto nSellable = GET_TECHNOTYPEEXT(pDocker)->Unsellable.Get(FakeRulesClass::Instance()->Units_UnSellable);

	if (!nUnitRepair || !nSellable)
	{
		R->CL(false);
	}
	else
	{
		R->CL(true);
	}

	return 0x4D9EC9;
}

#include <Locomotor/LocomotionClass.h>

 //rotation when crashing made optional
ASMJIT_PATCH(0x4DECAE, FootClass_Crash_Spin, 5)
{
	GET(FootClass*, pThis, ESI);
	return GET_TECHNOTYPEEXT(pThis)->CrashSpin ? 0u : 0x4DED4Bu;
}

#include <Ext/Cell/Body.h>

ASMJIT_PATCH(0x4D85E4, FootClass_UpdatePosition_TiberiumDamage, 9)
{
	GET(FootClass*, pThis, ESI);

	if (!pThis->IsAlive)
		return 0x0;

	int damage = 0;
	WarheadTypeClass* pWarhead = nullptr;
	int transmogrify = RulesClass::Instance->TiberiumTransmogrify;

	if (FakeRulesClass::Instance()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		TechnoTypeClass* pType = GET_TECHNOTYPE(pThis);
		TechnoTypeExtData* pExt = TechnoTypeExtContainer::Instance.Find(pType);

		// default is: infantry can be damaged, others cannot
		const bool enabled = (pThis->WhatAmI() != InfantryClass::AbsID);

		if (!pExt->TiberiumProof.Get(enabled) && !pThis->HasAbility(AbilityType::TiberiumProof))
		{
			if (pThis->Health > 0)
			{
				auto pCell = (FakeCellClass*)pThis->GetCell();
				if (auto pTiberium = TiberiumClass::Array->get_or_default(pCell->_GetTiberiumType()))
				{
					auto pTibExt = TiberiumExtContainer::Instance.Find(pTiberium);

					pWarhead = pTibExt->GetWarhead();
					damage = pTibExt->GetDamage();

					transmogrify = pExt->TiberiumTransmogrify.Get(transmogrify);
				}
			}
		}
	}

	if (damage != 0 && pWarhead) {
		CoordStruct crd = pThis->GetCoords();

		if (pThis->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead) {
			TechnoExtData::SpawnVisceroid(crd, RulesClass::Instance->SmallVisceroid, transmogrify, ScenarioClass::Instance->TiberiumDeathToVisceroid , HouseExtData::FindNeutral());
			return 0x4D8F29;
		}
	}

	return 0;
}

#ifdef _OldHooks
ASMJIT_PATCH(0x4DAA68, FootClass_Update_MoveSound, 0x6)
{
	GET(FootClass* const, pThis, ESI);

	const auto pType = GET_TECHNOTYPE(pThis);

	if(pType->IdleRate && TechnoTypeExtContainer::Instance.Find(pType)->NoIdleSound) {
		if(!pThis->Locomotor->Is_Moving_Now()) {
			pThis->MoveSoundDelay = 0;
			return 0x4DAB3C;
		}
	}

	if (pThis->IsMoveSoundPlaying ) {
		return 0x4DAAEE;
	}

	if (pThis->LocomotorSource) {
		pThis->MoveSoundAudioController.AudioEventHandleEndLooping();
		return 0x4DAAEE;
	}

	return 0x4DAA70;
}

ASMJIT_PATCH(0x4DA9C9, FootClass_Update_DeployToLandSound, 0xA)
{
	GET(TechnoTypeClass* const, pType, EAX);
	GET(FootClass* const, pThis, ESI);

	return !pType->JumpJet || pThis->GetHeight() <= 0 ? 0x4DAA01 : 0x4DA9D7;
}

// DeployToLand units increment WalkingFramesSoFar on every frame, on hover units this causes weird behaviour with move sounds etc.
ASMJIT_PATCH(0x4DA9F3, FootClass_AI_DeployToLand, 0x6)
{
	enum { SkipGameCode = 0x4DAA01 };

	GET(FootClass*, pThis, ESI);

	if (GET_TECHNOTYPE(pThis)->Locomotor == HoverLocomotionClass::ClassGUID())
		return SkipGameCode;

	return 0;
}

// Fix unit will play crash voice when crashing after attacked by locomotor warhead
// Author : NetsuNegi
ASMJIT_PATCH(0x4DACDD, FootClass__AI_CrashingVoice, 0x6)
{
	GET(FootClass*, pThis, ESI);

	if (pThis->IsCrashing != pThis->WasCrashingAlready)
	{
		if (pThis->IsCrashing)
		{
			pThis->MoveSoundAudioController.ShutUp();
			auto const nCoord = pThis->GetCoords();

			if (!pThis->IsAttackedByLocomotor)
			{
				const auto pType = GET_TECHNOTYPE(pThis);

				if (pThis->Owner->IsControlledByHuman())
					VocClass::SafeImmedietelyPlayAt(pType->VoiceCrashing, &nCoord);

				VocClass::SafeImmedietelyPlayAt(pType->CrashingSound, &nCoord, &pThis->MoveSoundAudioController);

			}
			else
			{
				VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->ScoldSound, &nCoord, &pThis->MoveSoundAudioController);
			}
		}
		else if (pThis->IsMoveSoundPlaying ) // done playing
			pThis->MoveSoundAudioController.ShutUp();

		pThis->WasCrashingAlready = pThis->IsCrashing;
	}

	return 0x4DADC8;
}

ASMJIT_PATCH(0x4DAD06, FootClass_AI_IsCrashing_VoiceAndSound, 0xA)
{
	enum { SkipVoiceAndSound = 0x4DADBC, ContinueAfter = 0x4DAD10 };

	GET(FootClass*, pThis, ESI);

	if (pThis->IsAttackedByLocomotor)
		return SkipVoiceAndSound;

	// Restore overriden instructions
	R->EAX(pThis->GetTechnoType());
	return ContinueAfter;
}

ASMJIT_PATCH(0x4DA9FB, FootClass_Update_WalkedFrames, 0x6)
{
	enum { SkipGameCode = 0x4DAA01 };

	GET(FootClass* const, pThis, ESI);

	if (AdvancedDriveLocomotionClass::IsReversing(pThis))
	{
		--pThis->WalkedFramesSoFar;
		return SkipGameCode;
	}

	return 0; // ++pThis->WalkedFramesSoFar;
}

ASMJIT_PATCH(0x4DA54E, FootClass_Update_AresAddition, 6)
{
	enum {
		SkipEverything = 0x4DAF00,
		SightChecking = 0x4DA677
	};

	GET(FootClass* , pThis, ESI);

	pThis->isidle_6B3 = false;
	auto const pType = GET_TECHNOTYPE(pThis);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	pExt->UpdateWarpInDelay();
	pExt->UpdateTiberiumEater();

	if(!pThis->IsAlive)
		return SkipEverything;

	pExt->AmmoAutoConvertActions();

	if(!pThis->IsAlive)
		return SkipEverything;

	pExt->DeployConvertAction();

	if(!pThis->IsAlive)
		return SkipEverything;

	pExt->ImmolateVictim();

	if(!pThis->IsAlive)
		return SkipEverything;

	pExt->UpdateTiberiumHeal();

	 if(!pThis->IsAlive)
	 	return SkipEverything;

	auto pAir = cast_to<AircraftClass*, false>(pThis);

	const bool IsMissisleSpawn = (RulesClass::Instance->V3Rocket.Type == pType ||
	 pType == RulesClass::Instance->DMisl.Type || pType == RulesClass::Instance->CMisl.Type
	 || (pAir && AircraftTypeExtContainer::Instance.Find(pAir->Type)->IsCustomMissile));

	if (pThis->SpawnOwner && !IsMissisleSpawn
		)
	{
		//auto pSpawnTechnoType = GET_TECHNOTYPE(pThis->SpawnOwner);
		//auto pSpawnTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pSpawnTechnoType);

		if (const auto pTargetTech = flag_cast_to<TechnoClass*>(pThis->Target))
		{
			//Spawnee trying to chase Aircraft that go out of map until it reset
			//fix this , so reset immedietely if target is not on map
			if (!MapClass::Instance->IsValid(pTargetTech->Location)
				|| pTargetTech->TemporalTargetingMe
				)
			{
				if (pThis->SpawnOwner->Target == pThis->Target)
					pThis->SpawnOwner->SetTarget(nullptr);

				pThis->SpawnOwner->SpawnManager->ResetTarget();
			}

		}
	}

	//skip together radiation damaging it is now direclt applyed on undate of the RadSiteClass itself
	//without this the sight wont properly updated
	if(!pThis->IsInPlayfield && (pType->BalloonHover || pType->JumpJet)) {
		if(MapClass::Instance->IsWithinUsableArea(pThis->GetCell(), true))
			pThis->IsInPlayfield = true;
	}


	return	pThis->IsAlive ? SightChecking : SkipEverything;
}

// move to the next hva frame, even if this unit isn't moving
ASMJIT_PATCH(0x4DA8B2, FootClass_Update_AnimRate, 6)
{
	GET(FootClass*, pThis, ESI);
	auto pType = GET_TECHNOTYPE(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->ResetLocomotor)
	{
		// Reinstalling Locomotor can avoid various issues such as teleportation, ignoring commands, and automatic return
		while (LocomotionClass::End_Piggyback(pThis->Locomotor));

		if (const auto pNewLoco = LocomotionClass::CreateInstance(pType->Locomotor)) {
			pThis->Locomotor = std::move(pNewLoco);
			pThis->Locomotor->Link_To_Object(pThis);
		}

		pExt->ResetLocomotor = false;
	}

	// Update laser trails after locomotor process, to ensure that the updated position is not the previous frame's position
	pExt->UpdateLaserTrails();
	TechnoExtData::Fastenteraction(pThis);

	enum { Undecided = 0u,
			NoChange = 0x4DAA01u,
			Advance = 0x4DA9FBu,
			Checks = 0x4DA8B2u
		};

	if (!pThis->InLimbo) {
		if (pThis->InWhichLayer() != pThis->LastLayer) {
			DisplayClass::Instance->SubmitObject(pThis);
		}
	}

	// any of these prevents the animation to advance to the next frame
	if (pThis->IsBeingWarpedOut() || pThis->IsWarpingIn() || pThis->IsAttackedByLocomotor) {
		return NoChange;
	}

	// animate unit whenever in air
	if (pTypeExt->AirRate && pThis->GetHeight() > 0)
	{
		return (Unsorted::CurrentFrame.get() % pTypeExt->AirRate) ? NoChange : Advance;
	}

	return Undecided;
}


#endif

#include <SpawnManagerClass.h>

void __fastcall FakeFootClass::_AI(FootClass* pThis)
{
    // ── 1. Base class ─────────────────────────────────────────────────────────
	FakeTechnoClass::__AI(pThis);

    if (!pThis->IsAlive)
        return;
 
    // ── 2. Extension pre-processing + early exits ─────────────────────────────
    // EXTENSION_HOOK: FootClass_Update_AresAddition @ 0x4DA54E (size 0x6)
    // Fires immediately after IsActive check, before vanilla radiation block.
    // Replaces vanilla sections 2 (radiation) and 3 (BalloonHover/IsLocked):
    //   - Radiation is now applied directly by RadSiteClass::Update — skipped here.
    //   - BalloonHover IsLocked is skipped — hook jumps past it to section 4.
    {
        pThis->isidle_6B3 = false;
 
        auto* pType = pThis->GetTechnoType();
        auto* pExt  = TechnoExtContainer::Instance.Find(pThis);
 
        pExt->UpdateWarpInDelay();
        pExt->UpdateTiberiumEater();
        if (!pThis->IsAlive) return;
 
        pExt->AmmoAutoConvertActions();
        if (!pThis->IsAlive) return;
 
		pExt->HealthAutoConvertActions();
		if (!pThis->IsAlive) return;
 
        pExt->ImmolateVictim();
        if (!pThis->IsAlive) return;
 
        pExt->UpdateTiberiumHeal();
        if (!pThis->IsAlive) return;
 
        // Missile-spawn target validation
        // Spawned missiles that chase aircraft leaving the map must reset immediately.
        auto* pAir = cast_to<AircraftClass*>(pThis); // nullptr if not aircraft

        const bool isMissileSpawn =
            (RulesClass::Instance->V3Rocket.Type == pType)
            || (pType == RulesClass::Instance->DMisl.Type)
            || (pType == RulesClass::Instance->CMisl.Type)
            || (pAir && AircraftTypeExtContainer::Instance.Find(pAir->Type)->IsCustomMissile);
 
        if (pThis->SpawnOwner && !isMissileSpawn)
        {
            if (auto* pTargetTech = flag_cast_to<TechnoClass*>(pThis->Target))
            {
                if (!MapClass::Instance->IsValid(pTargetTech->Location)
                    || pTargetTech->TemporalTargetingMe)
                {
                    if (pThis->SpawnOwner->Target == pThis->Target)
                        pThis->SpawnOwner->SetTarget(nullptr);
 
                    pThis->SpawnOwner->SpawnManager->ResetTarget();
                }
            }
        }
 
        // IsInPlayfield fix: BalloonHover / JumpJet units that re-enter usable area
        if (!pThis->IsInPlayfield && (pType->BalloonHover || pType->JumpJet))
        {
            if (MapClass::Instance->IsWithinUsableArea(pThis->GetCell(), true))
                pThis->IsInPlayfield = true;
        }
 
        if (!pThis->IsAlive) return;
    }
    // Vanilla sections 2 (radiation) and 3 (BalloonHover IsLocked) are intentionally omitted:
    //   - Radiation: handled by RadSiteClass::Update directly (per hook comment).
    //   - BalloonHover IsLocked: hook jumps straight to 0x4DA677 (section 4).
 
    // ── 4. Sight timer reveal (moving + in air + allied) ─────────────────────
    // Assembly: 0x4DA677 – 0x4DA7AF
    // Reveal fires when:
    //   a) Started != -1 and elapsed >= AccumDelay
    //   b) Started == -1 and AccumDelay == 0
    {
        if (pThis->Locomotor->Is_Moving_Now()) {

            bool inAir = pThis->IsInAir();

            if (inAir && pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer())) {

                int timerStarted = pThis->SightTimer.StartTime;
                int timerDelay   = pThis->SightTimer.TimeLeft;
 
               const bool doReveal = (timerStarted != -1)
                    ? (Unsorted::CurrentFrame() - timerStarted) >= timerDelay
                    : (timerDelay == 0);
 
                if (doReveal) {
                    pThis->vt_entry_48C(0, 0, 0, 0);
                    pThis->UpdateSight(0, 0, 0, 0, 0);
 
                    CoordStruct origin = pThis->Location;
                    int range = pThis->LastSightRange;
 
                    if (pThis->LastSightHeight == pThis->GetHeight())
                        MapClass::Instance->RevealArea3(&origin, range - 3, range + 3, 0);
                    else
                        MapClass::Instance->RevealArea3(&origin, 0, range + 3, 0);
 
                    pThis->LastSightHeight = pThis->GetHeight();
					pThis->SightTimer.Start(15);
                }
            }
        }
    }
 
    // ── 5. Tag spring: entered/overflown (every 16 frames, in air, has tag) ──
    // Assembly: 0x4DA7B0 – 0x4DA805
    if ((Unsorted::CurrentFrame() & 0xF) == 0) {
        if (pThis->IsInAir()) {
            CellClass* pCell = pThis->GetCell();
            if (pCell->AttachedTag) {
				pCell->AttachedTag->SpringEvent(TriggerEvent::EnteredOrOverflownBy,pThis ,pThis->GetMapCoords(), false);
            }
        }
    }
 
    // ── 6. Locomotion Process + WalkedFramesSoFar ────────────────────────────
    // Assembly: 0x4DA806 – 0x4DA9FA
    {
        int prevWalked = pThis->WalkedFramesSoFar;
 
        const bool skipProcess = pThis->IsSinking
            || pThis->IsFallingDown
            || (pThis->DirectRockerLinkedUnit
                && !pThis->GetTechnoType()->Pushy)
            || pThis->InLimbo;
 
        if (!skipProcess) {
            pThis->Locomotor->Process();
 
            if (!pThis->IsAlive)
                return;
 
            UnitClass* pUnit = cast_to<UnitClass*, false>(pThis);
 
            // EXTENSION_HOOK: FootClass_Update_AnimRate @ 0x4DA8B2 (size 0x6)
            // Fires after Process() + IsActive check, before the vanilla Is_Moving_Now / WalkRate logic.
            // Returns:
            //   Undecided (0)        → fall through to vanilla walk-counter logic below
            //   NoChange (0x4DAA01)  → skip all increment paths; go straight to sound comparison
            //   Advance  (0x4DA9FB)  → force-increment WalkedFramesSoFar and continue
            // (Checks enum value 0x4DA8B2 is defined but never returned by the hook body — ignored)
            bool skipWalkLogic  = false;
            bool forceIncrement = false;
            {
                auto* pExt     = TechnoExtContainer::Instance.Find(pThis);
                auto* pType    = pThis->GetTechnoType();
                auto* pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
 
                // Reinstall locomotor if flagged (avoids teleport / ignore-command bugs)
                if (pExt->ResetLocomotor)
                {
                    while (LocomotionClass::End_Piggyback(pThis->Locomotor));
 
                    if (auto pNewLoco = LocomotionClass::CreateInstance(pType->Locomotor))
                    {
                        pThis->Locomotor = std::move(pNewLoco);
                        pThis->Locomotor->Link_To_Object(pThis);
                    }
                    pExt->ResetLocomotor = false;
                }
 
                // Post-locomotor-process updates
                pExt->UpdateLaserTrails();
                TechnoExtData::Fastenteraction(pThis);
 
                // Layer change notification (not in limbo)
                if (!pThis->InLimbo && pThis->InWhichLayer() != pThis->LastLayer)
                    DisplayClass::Instance->SubmitObject(pThis);
 
                // Warp / locomotor-warhead: suppress walk counter entirely this frame
                if (pThis->IsBeingWarpedOut() || pThis->IsWarpingIn() || pThis->IsAttackedByLocomotor)
                {
                    skipWalkLogic = true; // → NoChange
                }
                // AirRate: drive walk counter by height + AirRate instead of WalkRate
                else if (pTypeExt->AirRate && pThis->GetHeight() > 0)
                {
                    if (Unsorted::CurrentFrame.get() % pTypeExt->AirRate)
                        skipWalkLogic = true;  // → NoChange (not this frame)
                    else
                        forceIncrement = true; // → Advance (this frame)
                }
                // else Undecided: fall through to vanilla condA logic
            }
 
            if (skipWalkLogic) {
                // NoChange: skip to walked-count comparison without touching WalkedFramesSoFar
                // (fall through to section 7 with prevWalked == WalkedFramesSoFar)
            }
            else if (forceIncrement) {
                // Advance: skip vanilla condA, directly increment
                ++pThis->WalkedFramesSoFar;
            } else {
                // Undecided: vanilla WalkRate / IdleRate / DeployToLand logic
                auto* pType     = pThis->GetTechnoType();
                bool isMoving   = pThis->Locomotor->Is_Moving_Now();
                bool isTrans50F = pThis->IsWarpingIn();
                bool isTrans50E = pThis->IsBeingWarpedOut();
                bool pathBlock  = (pThis->IsAttackedByLocomotor != 0);
 
                bool condA = (isMoving && !isTrans50F)
                    || (pThis->TarCom && pType->HoverAttack && pUnit && !pUnit->Deployed);
 
                bool doWalkIncrement = false;
 
                if (condA && !(Unsorted::CurrentFrame() % pType->WalkRate) && !isTrans50E && !isTrans50F && !pathBlock) {
                    doWalkIncrement = true;
                } else if (pType->IdleRate) {
                    // EXTENSION_HOOK: FootClass_Update_DeployToLandSound @ 0x4DA9C9 (size 0xA)
                    // JumpJet units hovering in air bypass the Is_Moving_Now gate for idle-rate increments.
                    bool suppressIdleIncrement;
                    if (pType->JumpJet && pThis->GetHeight() > 0)
                        suppressIdleIncrement = false;
                    else
                        suppressIdleIncrement = pThis->Locomotor->Is_Moving_Now();
 
                    if (!suppressIdleIncrement && !(Unsorted::CurrentFrame() % pType->IdleRate) && !isTrans50E && !isTrans50F && !pathBlock)
                        doWalkIncrement = true;
                }
 
                if (!doWalkIncrement
                    && pType->DeployToLand
                    && pThis->GetHeight() > 0
                    && !isTrans50E && !isTrans50F && !pathBlock) {
                    // EXTENSION_HOOK: FootClass_AI_DeployToLand @ 0x4DA9F3 (size 0x6)
                    // HoverLocomotion units must not increment via the DeployToLand path.
                    if (pType->Locomotor != HoverLocomotionClass::ClassGUID())
                        doWalkIncrement = true;
                }
 
                // EXTENSION_HOOK: FootClass_Update_WalkedFrames @ 0x4DA9FB (size 0x6)
                if (doWalkIncrement){
					if (AdvancedDriveLocomotionClass::IsReversing(pThis)) {
						--pThis->WalkedFramesSoFar;
					} else {
                    	++pThis->WalkedFramesSoFar;
					}
				}

            }
        }
 
        // ── 7. Moving sound: start or stop ────────────────────────────────────
        // Assembly: 0x4DAA01 – 0x4DAB3B
        bool walkedUnchanged = (prevWalked == pThis->WalkedFramesSoFar);
        bool isMovingNow = false;
 
        if (walkedUnchanged) {
            isMovingNow = pThis->Locomotor->Is_Moving_Now();
        }
 
        const bool forceStop =
            pThis->IsFallingDown
            || pThis->IsCrashing
            || (pThis->DirectRockerLinkedUnit
                && pThis->GetTechnoType()->Pushy);
 
        const bool stopSoundPath = forceStop || (walkedUnchanged && !isMovingNow);
 
        if (stopSoundPath) {
            // Stop moving sound (LABEL_136 / 0x4DAAFA)
            if (pThis->IsMoveSoundPlaying) {
                int delay = pThis->MoveSoundDelay;
                if (!delay || pThis->IsFallingDown || pThis->IsCrashing) {
                    pThis->MoveSoundAudioController.ShutUp();
                    pThis->IsMoveSoundPlaying = false;
                    pThis->MoveSoundDelay     = 0;
                } else {
                    pThis->MoveSoundDelay = delay - 1;
                }
            }
        } else {
            // Start/maintain moving sound (0x4DAA32 – 0x4DAAF8)
            // EXTENSION_HOOK: FootClass_Update_MoveSound @ 0x4DAA68 (size 0x6)
            auto* pType    = pThis->GetTechnoType();
            auto* pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
 
            if (pType->IdleRate && pTypeExt->NoIdleSound
                && !pThis->Locomotor->Is_Moving_Now()) {
                // NoIdleSound suppression: clear delay, skip play entirely
                pThis->MoveSoundDelay = 0;
            } else if (pThis->IsMoveSoundPlaying) {
                // Already playing: just reset countdown
                pThis->MoveSoundDelay = 3;
            } else if (pThis->LocomotorSource) {
                // Locomotor source active: stop then reset
                pThis->MoveSoundAudioController.ShutUp();
                pThis->MoveSoundDelay = 3;
            } else {
                // Vanilla: play if sound list has entries
                if (pType->MoveSound.Count > 0)  {
                    pThis->MoveSoundAudioController.ShutUp();
                    CoordStruct coord = pThis->Location;
                    unsigned int rnd = Random2Class::NonCriticalRandomNumber->Random();
                    VocClass::ImmedietelyPlayAt(pType->MoveSound[rnd % pType->MoveSound.Count], &coord, &pThis->MoveSoundAudioController);
                    pThis->IsMoveSoundPlaying = true;
                }

                pThis->MoveSoundDelay = 3;
            }
        }
    } // end WalkedFrames / sound block
 
    // ── 8. IsFalling state change ─────────────────────────────────────────────
    // Assembly: 0x4DAB3C – 0x4DABC6
    {
        bool falling = pThis->IsFallingDown;
        if (falling != pThis->WasFallingDown) {
            auto* pType = pThis->GetTechnoType();
            if (falling && pType->VoiceFalling != -1 && pThis->IsABomb) {
                VocClass::ImmedietelyPlayAt(pType->VoiceFalling, &pThis->Location, nullptr);
            }

            pThis->WasFallingDown = falling;
        }
    }
 
    // ── 9. IsSinking state change ─────────────────────────────────────────────
    // Assembly: 0x4DABC7 – 0x4DACD0
    {
        bool sinking = pThis->IsSinking;
        if (sinking != pThis->WasSinkingAlready) {
            if (sinking) {
                auto* pType = pThis->GetTechnoType();

                if (pType->VoiceSinking != -1) {
                    CoordStruct coord = pThis->Location;
                    VocClass::SafeImmedietelyPlayAt(pType->VoiceSinking, &coord, nullptr);
                }

                if (pType->SinkingSound == -1) {
                     VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->SinkingSound, &pThis->Location, &pThis->MoveSoundAudioController);
                } else {
                    VocClass::SafeImmedietelyPlayAt(pType->SinkingSound, & pThis->Location, &pThis->MoveSoundAudioController); // VERIFY: +0x548
                }
            } else if (!pThis->IsMoveSoundPlaying) {
                pThis->MoveSoundAudioController.ShutUp();
            }

            pThis->WasSinkingAlready = sinking;
        }
    }
 
    // ── 10. IsCrashing state change ───────────────────────────────────────────
    // Assembly: 0x4DACDD – 0x4DADC7
    // EXTENSION_HOOK: FootClass__AI_CrashingVoice @ 0x4DACDD (size 0x6) — replaces entire vanilla block.
    // EXTENSION_HOOK: FootClass_AI_IsCrashing_VoiceAndSound @ 0x4DAD06 (size 0xA) — absorbed below.
    {
        bool crashing = pThis->IsCrashing;

        if (crashing != pThis->WasCrashingAlready) {
            if (crashing)  {
                pThis->MoveSoundAudioController.ShutUp();
                CoordStruct coord = pThis->GetCoords();
 
                if (!pThis->IsAttackedByLocomotor)  {
                    auto* pType = pThis->GetTechnoType();

                    if (pThis->Owner->IsControlledByHuman())
                        VocClass::SafeImmedietelyPlayAt(pType->VoiceCrashing, &coord);

                    VocClass::SafeImmedietelyPlayAt(pType->CrashingSound, &coord, &pThis->MoveSoundAudioController);
                } else {
                    VocClass::SafeImmedietelyPlayAt(RulesClass::Instance->ScoldSound, &coord, &pThis->MoveSoundAudioController);
                }
            } else if (pThis->IsMoveSoundPlaying) {
                pThis->MoveSoundAudioController.ShutUp();
            }

            pThis->WasCrashingAlready = crashing;
        }
    }
 
    // ── 11. Positional audio range update ─────────────────────────────────────
    // Assembly: 0x4DADC8 – 0x4DADF4
    {
        CoordStruct coord = pThis->Location;
        VocClass::PlayIfInRange(coord, &pThis->MoveSoundAudioController);
    }
 
    // ── 12. Idle scatter ──────────────────────────────────────────────────────
    // Assembly: 0x4DADFE – 0x4DAE59
    if ((Unsorted::CurrentFrame() & 63) == 63
        && !pThis->NavCom
        && !pThis->OnBridge
        && !pThis->GetCell()->OverlayData
        && pThis->vt_entry_2B0()
        && !pThis->GetHeight())
    {
        pThis->Scatter(CoordStruct::Empty,true ,false);
    }
 
    // ── 13. IPiggyback locomotion swap ────────────────────────────────────────
    // Assembly: 0x4DAE5F – 0x4DAEC5
    IPiggyback* pPiggy = nullptr;
    {
        if (ILocomotion* pLoco = pThis->Locomotor) {

            const HRESULT hr = pLoco->QueryInterface(IPiggyback::_CLSID, (void**)&pPiggy);

            if (hr < 0 && hr != E_NOINTERFACE){
                _com_issue_error(hr);
				pPiggy = nullptr;
			}
 
            if (pPiggy && pPiggy->Is_Ok_To_End()) {
				ILocomotion** ppLoco = &pThis->Locomotor;

                if (*ppLoco)
                    (*ppLoco)->Release();

                *ppLoco = nullptr;
                pPiggy->End_Piggyback(ppLoco);
            }
        }
    }
 
    // ── 14. Reset height subtract + TechnoClass linked update ─────────────────
    // Assembly: 0x4DAEC6 – 0x4DAEE0
    pThis->height_subtract_6B4 = 0;
    if (!pThis->IsWarpingIn()) {
        pThis->DoOnLinked();
 	}

    // ── 15. Parasite AI ───────────────────────────────────────────────────────
    // Assembly: 0x4DAEE1 – 0x4DAEF5
    if (auto pParasite = pThis->ParasiteEatingMe)  {
        pParasite->ParasiteImUsing->Update();
    }
 
    // ── 16. Release IPiggyback reference ──────────────────────────────────────
    // Assembly: 0x4DAEF6 – 0x4DAEFD
    if (pPiggy)
        pPiggy->Release();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x4DA530 , FakeFootClass::_AI)
//DEFINE_FUNCTION_JUMP(CALL, 0x51BC9F	, FakeFootClass::_AI)
//DEFINE_FUNCTION_JUMP(CALL, 0x73647B , FakeFootClass::_AI)