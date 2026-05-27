
#include <Ext/Techno/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/House/Body.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <New/Type/ArmorTypeClass.h>

#include <InfantryClass.h>
#include <RadarEventClass.h>

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
					.Verses <= 0.0)
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

ASMJIT_PATCH(0x4DC0E4, FootClass_DrawActionLines_Attack, 0x8)
{
	enum { Skip = 0x4DC1A0, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->CommandLine_Attack_Color.isset())
	{
		GET(CoordStruct*, pMovingDestCoord, EAX);
		GET(int, nFLH_X, EBP);
		GET(int, nFLH_Y, EBX);
		GET_STACK(int, nFLH_Z, STACK_OFFS(0x34, 0x10));

		if (pTypeExt->CommandLine_Attack_Color.Get() != ColorStruct::Empty)
		{
			Drawing::Draw_action_lines_7049C0(nFLH_X, nFLH_Y, nFLH_Z, pMovingDestCoord->X, pMovingDestCoord->Y, pMovingDestCoord->Z,
				pTypeExt->CommandLine_Attack_Color->ToInit(), false, false);
		}

		return Skip;
	}

	return Continue;
}

ASMJIT_PATCH(0x4DC280, FootClass_DrawActionLines_Move, 0x5)
{
	enum { Skip = 0x4DC328, Continue = 0x0 };

	GET(FootClass* const, pThis, ESI);

	const auto pTypeExt = GET_TECHNOTYPEEXT(pThis);

	if (pTypeExt->CommandLine_Move_Color.isset())
	{
		GET_STACK(CoordStruct, nCooordDest, STACK_OFFS(0x34, 0x24));
		GET(int, nCoordDest_Adjusted_Z, EDI);
		GET(int, nLoc_X, EBP);
		GET(int, nLoc_Y, EBX);
		GET_STACK(int, nLoc_Z, STACK_OFFS(0x34, 0x10));
		GET_STACK(bool, barg3, STACK_OFFSET(0x34, 0x8));

		if (pTypeExt->CommandLine_Move_Color.Get() != ColorStruct::Empty)
		{
			Drawing::Draw_action_lines_7049C0(nLoc_X, nLoc_Y, nLoc_Z, nCooordDest.X, nCooordDest.Y, nCoordDest_Adjusted_Z,
				pTypeExt->CommandLine_Move_Color->ToInit(), barg3, false);
		}

		return Skip;
	}

	return Continue;
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

	if (pThis->WhatAmI() != AbstractType::Infantry)
	{
		TechnoExtContainer::Instance.Find(pThis)->CreateInitialPayload();
	}

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

ASMJIT_PATCH(0x4DEC7F, FootClass_Crash_FallingDownFix, 0x7)
{
	GET(FootClass*, pThis, ESI);

	if (pThis->IsFallingDown && !pThis->IsABomb && pThis->Locomotor)
	{
		if (const auto pJumpjet = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			pJumpjet->NextState = JumpjetLocomotionClass::State::Crashing;
	}

	return 0;
}

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
				auto damage = RulesExtData::Instance()->HunterSeeker_Damage.Get();
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
	const auto nSellable = GET_TECHNOTYPEEXT(pDocker)->Unsellable.Get(RulesExtData::Instance()->Units_UnSellable);

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

	if (RulesExtData::Instance()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
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

	if (damage != 0 && pWarhead)
	{
		CoordStruct crd = pThis->GetCoords();

		if (pThis->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead)
		{
			TechnoExtData::SpawnVisceroid(crd, RulesClass::Instance->SmallVisceroid, transmogrify, ScenarioClass::Instance->TiberiumDeathToVisceroid , HouseExtData::FindNeutral());
			return 0x4D8F29;
		}
	}

	return 0;
}
