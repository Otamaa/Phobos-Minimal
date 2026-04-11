#include "Body.h"
#include <RadarClass.h>
#include <RadarEventClass.h>
#include <VoxClass.h>

#include <Locomotor/Cast.h>
#include <Locomotor/HoverLocomotionClass.h>
#include <Locomotor/DropPodLocomotionClass.h>
#include <Locomotor/ShipLocomotionClass.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

int FakeBuildingClass::_Mission_Repair()
{
	auto const pThis = this;
	auto const pType = pThis->Type;
	auto const pExt = pThis->_GetExtData();
	auto const pTypeExt = pThis->_GetTypeExtData();

	// --- Bunker side-effect (executes regardless of class) ---
	if (pType->Bunker)
	{
		pThis->UpdateBunker();
	}

	// --- Common tail: return (mission_control->Rate * TICKS_PER_MINUTE) ---
	auto const return_mission_rate = [pThis]() -> int
		{
			auto const pMC = pThis->GetCurrentMissionControl();
			return int(pMC->Rate * TICKS_PER_MINUTE);
		};

	// --- Health-driven anim selector helper ---
	auto const play_health_anim = [pThis](int slot, BuildingAnimSlot destAnim)
		{
			bool const damaged =
				pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

			auto pAnimSet = damaged
				? pThis->Type->BuildingAnim[slot].Damaged
				: pThis->Type->BuildingAnim[slot].Anim;

			if (pAnimSet && pAnimSet[0])
			{
				pThis->PlayAnim(pAnimSet, destAnim, damaged, false, false);
			}
		};

	// ==================================================================
	// === ConstructionYard ============================================
	// ==================================================================
	if (pType->ConstructionYard)
	{
		switch (pThis->MissionStatus)
		{
		case 0:
			pThis->BeginMode(BStateType::Active);
			play_health_anim(7, BuildingAnimSlot::PreProduction);
			pThis->MissionStatus = 2;
			break;

		case 2:
			if (!pThis->HasAnyLink())
			{
				pThis->QueueMission(Mission::Guard, false);
				pThis->DestroyNthAnim(BuildingAnimSlot::PreProduction);
			}
			break;

		default:
			break;
		}
		return 1;
	}

	// ==================================================================
	// === Hospital ====================================================
	// ==================================================================
	if (pType->Hospital)
	{
		if (pThis->MissionStatus == 0)
		{
			pThis->MissionStatus = 2;
			pThis->IsReadyToCommence = false;
			pThis->RepairProgress.Stage = 0;
			pThis->RepairProgress.Step = 1;
			pThis->RepairProgress.Timer.Start(1);

			if (pThis->Type->Ammo != -1)
			{
				int ammo = pThis->Ammo - 1;
				pThis->Ammo = (ammo <= 0) ? 0 : ammo;
			}
			return return_mission_rate();
		}

		if (pThis->MissionStatus == 2)
		{
			pThis->RepairProgress.Update();

			// Hook 0x44B8F1: Units_RepairRate override (IRepairRate default)
			double const repairRate =
				pTypeExt->Units_RepairRate.Get(RulesClass::Instance->IRepairRate);

			if (repairRate * TICKS_PER_MINUTE <= pThis->RepairProgress.Stage)
			{
				pThis->IsReadyToCommence = false;
				pThis->RepairProgress.Stage = 0;

				auto const msg = pThis->SendToFirstLink(RadioCommand::RequestRepair);

				// Vanilla dispatch (assembly: -10, -22, -1):
				//   AnswerNegative (10) -> shared Exit+Guard fallthrough
				//   AnswerBlocked  (32) -> Detach+Exit, return 1 (no Guard)
				//   QueryDone      (33) -> EVA + sound, then shared fallthrough
				//   anything else       -> return 1
				if (msg == RadioCommand::AnswerNegative)  // FIX 1: was AnswerPositive
				{
					// fall through to Exit_Object + guard
				}
				else if (msg == RadioCommand::AnswerBlocked)
				{
					pThis->__ExitObject(pThis->Passengers.RemoveFirstPassenger(), CellStruct::Empty);
					return 1;
				}
				else if (msg == RadioCommand::QueryDone)
				{
					if (pThis->Owner->ControlledByCurrentPlayer())
					{
						auto const cell = pThis->GetCell()->MapCoords;

						if (RadarEventClass::Create(RadarEventType::UnitRepaired, cell))
						{
							VoxClass::Play(GameStrings::EVA_UnitRepaired());
						}

						VocClass::ImmedietelyPlayAt(RulesClass::Instance->HealCrateSound, &pThis->Location, 0);
					}
				}
				else
				{
					return 1;
				}

				// Shared Exit + return to guard (msg == AnswerNegative or QueryDone)
				pThis->__ExitObject(pThis->Passengers.RemoveFirstPassenger(), CellStruct::Empty);
				pThis->QueueMission(Mission::Guard, false);
			}
			return 1;
		}

		return return_mission_rate();
	}

	// ==================================================================
	// === Armory ======================================================
	// ==================================================================
	if (pType->Armory)
	{
		if (pThis->MissionStatus == 0)
		{
			pThis->MissionStatus = 2;
			pThis->IsReadyToCommence = false;
			pThis->RepairProgress.Stage = 0;
			pThis->RepairProgress.Step = 1;
			pThis->RepairProgress.Timer.Start(1);

			int ammo = pThis->Ammo - 1;
			pThis->Ammo = (ammo <= 0) ? 0 : ammo;
		}
		else if (pThis->MissionStatus == 2)
		{
			pThis->RepairProgress.Update();

			double const repairRate =
				pTypeExt->Units_RepairRate.Get(RulesClass::Instance->IRepairRate);

			if (repairRate * TICKS_PER_MINUTE
				<= pThis->RepairProgress.Stage)
			{
				// ------------------------------------------------------------------
				// Hook 0x44BB1B: BuildingClass_Mission_Repair_Promote.
				// for yet unestablished reasons a unit might not be present.
				// maybe something triggered the KickOutHospitalArmory
				// ------------------------------------------------------------------
				if(auto* pContact = pThis->GetRadioContact(0)){
					auto& vet = pContact->Veterancy;

					if (vet.IsRookie())
					{
						vet.SetVeteran();
					}
					else
					{
						vet.SetElite();
					}
				}

				pThis->__ExitObject(pThis->Passengers.RemoveFirstPassenger(), CellStruct::Empty);
				pThis->QueueMission(Mission::Guard, false);
			}
		}

		return return_mission_rate();
	}

	// ==================================================================
	// === UnitReload (service depot reload path) =====================
	// ==================================================================
	if (!pType->UnitRepair)
	{
		if (!pType->UnitReload)
		{
			return 15;
		}

		// ------------------------------------------------------------------
		// Hook 0x44C836: Units_RepairRate separate periodic repair pass.
		// Runs alongside the vanilla reload loop below — purely additive.
		// ------------------------------------------------------------------
		if (pTypeExt->Units_RepairRate.isset())
		{
			double const repairRate = pTypeExt->Units_RepairRate.Get();
			if (repairRate >= 0.0)
			{
				int const rate = static_cast<int>(
					MaxImpl(repairRate * 900.0, 1.0));

				if ((Unsorted::CurrentFrame % rate) == 0)
				{
					pExt->SeparateRepair = true;

					for (int i = 0; i < pThis->RadioLinks.Capacity; ++i)
					{
						auto* const pLink = pThis->GetRadioContact(i);
						if (!pLink)
						{
							continue;
						}

						auto* const pLinkType = pLink->GetTechnoType();
						if (!pLink->IsInAir()
							&& pLink->Health < pLinkType->Strength
							&& pThis->SendCommand(RadioCommand::QueryMoving, pLink)
								   == RadioCommand::AnswerPositive)
						{
							pThis->SendCommand(RadioCommand::RequestRepair, pLink);
						}
					}

					pExt->SeparateRepair = false;
				}
			}
		}
#ifdef _Vanilla
		// --- vanilla UnitReload loop ---
		bool any_in_service = false;
		int const radio_slots = pThis->RadioLinks.Capacity;

		for (int i = 0; i < radio_slots; ++i)
		{
			auto* pClient = pThis->GetRadioContact(i);
			if (!pClient)
			{
				continue;
			}

			if (pThis->SendCommand(RadioCommand::QueryReadiness, pClient) == RadioCommand::AnswerPositive
				&& pClient->Health == pClient->GetTechnoType()->Strength)
			{
				pClient->EnterIdleMode(false, true);
				pClient->QueueMission(Mission::Guard, false);
				pClient->ProceedToNextPlanningWaypoint();
				continue;
			}

			if (pClient->GetMission() == Mission::Enter
				|| pThis->SendCommand(RadioCommand::QueryMoving, pClient) != RadioCommand::AnswerPositive)
			{
				continue;
			}

			any_in_service = true;

			if (pClient->GetMission() != Mission::Sleep)
			{
				pClient->QueueMission(Mission::Sleep, false);
				continue;
			}

			if (pThis->SendCommand(RadioCommand::RequestReload, pClient) == RadioCommand::AnswerPositive
				|| pThis->SendCommand(RadioCommand::RequestRepair, pClient) == RadioCommand::AnswerPositive)
			{
				continue;
			}

			pClient->EnterIdleMode(false, true);
			pClient->QueueMission(Mission::Guard, false);
			pClient->ProceedToNextPlanningWaypoint();
		}

		if (any_in_service)
		{
			return int(RulesClass::Instance->ReloadRate * TICKS_PER_MINUTE);
		}

#else
		// ------------------------------------------------------------------
		// Hook 0x44C844: BuildingClass_Mission_Repair_Reload.
		// replaces the UnitReload handling and makes each docker independent of all
		// others. this means planes don't have to wait one more ReloadDelay because
		// the first docker triggered repair mission while the other dockers arrive
		// too late and need to be put to sleep first.
		// ------------------------------------------------------------------
		// ensure there are enough slots
		pExt->DockReloadTimers.resize(pThis->RadioLinks.Capacity, -1);

		// update all dockers, check if there's
		// at least one needing more attention
		bool keep_reloading = false;
		for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pThis->GetNthLink(i))
			{
				auto const SendCommand = [=](RadioCommand command)
					{
						return pThis->SendCommand(command, pLink) == RadioCommand::AnswerPositive;
					};

				// check if reloaded and repaired already
				auto const pLinkType = GET_TECHNOTYPE(pLink);
				auto done = SendCommand(RadioCommand::QueryReadiness)
					&& pLink->Health == pLinkType->Strength;

				if (!done)
				{
					// check if docked
					auto const miss = pLink->GetCurrentMission();
					if (miss == Mission::Enter
						|| !SendCommand(RadioCommand::QueryMoving))
					{
						continue;
					}

					keep_reloading = true;

					// make the unit sleep first
					if (miss != Mission::Sleep)
					{
						pLink->QueueMission(Mission::Sleep, false);
						continue;
					}

					// check whether the timer completed
					auto const last_timer = pExt->DockReloadTimers[i];
					if (last_timer > Unsorted::CurrentFrame)
					{
						continue;
					}

					// set the next frame
					auto const pLinkExt = TechnoTypeExtContainer::Instance.Find(pLinkType);
					auto const defaultRate = RulesClass::Instance->ReloadRate;
					auto const rate = pLinkExt->ReloadRate.Get(defaultRate);
					auto const frames = static_cast<int>(rate * 900);
					pExt->DockReloadTimers[i] = Unsorted::CurrentFrame + frames;

					// only reload if the timer was not outdated
					if (last_timer != Unsorted::CurrentFrame)
					{
						continue;
					}

					// reload and repair, return true if both failed
					done = !SendCommand(RadioCommand::RequestReload)
						&& !SendCommand(RadioCommand::RequestRepair);
				}

				if (done)
				{
					pLink->EnterIdleMode(false, 1);
					pLink->ForceMission(Mission::Guard);
					pLink->ProceedToNextPlanningWaypoint();

					pExt->DockReloadTimers[i] = -1;
				}
			}
		}

		if (keep_reloading)
		{
			// update each frame
			return 1;
		}
#endif
		pThis->QueueMission(Mission::Guard, false);


		return 3;
	}

	// ==================================================================
	// === UnitRepair (service depot repair path) =====================
	// ==================================================================
	switch (pThis->MissionStatus)
	{

		// ----- Status 0: waiting for a client to dock -----
	case 0:
		// FIX 3: original kicks back to MISSION_GUARD entirely, does NOT set status=1
		if (!pThis->HasAnyLink())
		{
			pThis->DestroyNthAnim(BuildingAnimSlot::Production);
			pThis->DestroyNthAnim(BuildingAnimSlot::SpecialTwo);
			play_health_anim(12, BuildingAnimSlot::SpecialThree);
			play_health_anim(3, BuildingAnimSlot::Active);
			pThis->QueueMission(Mission::Guard, true);
			return 1;
		}
		{
			pThis->IsReadyToCommence = false;

			int min_distance = 100;
			FootClass* pClient = (FootClass*)pThis->GetRadioContact(0);

			if (locomotion_cast<HoverLocomotionClass*>(pClient->Locomotor)
				|| locomotion_cast<ShipLocomotionClass*>(pClient->Locomotor))
			{
				min_distance = 200;
			}

			if (pThis->SendToFirstLink(RadioCommand::QueryMoving) == RadioCommand::AnswerPositive
				&& pThis->DistanceFrom(pClient) < min_distance)
			{
				pThis->MissionStatus = 1;
				return 3;
			}

			// FIX 6: original asm uses pThis->Owner ([ebp+21Ch] = building's house), not pClient->Owner
			if (!pThis->Owner->IonSensitivesShouldBeOffline())
			{
				pClient->Locomotor->Power_On();
			}
		}
		return return_mission_rate();

		// ----- Status 1: radio established, waiting to repair -----
	case 1:
	{
		// FIX 4: anim destroy/play gated on (Production || SpecialTwo);
		//        Guard reassign gated on (!SpecialThree) and flips IsReadyToCommence
		if (!pThis->HasAnyLink())
		{
			if (pThis->Anims[(int)BuildingAnimSlot::Production]
				|| pThis->Anims[(int)BuildingAnimSlot::SpecialTwo])
			{
				pThis->DestroyNthAnim(BuildingAnimSlot::Production);
				pThis->DestroyNthAnim(BuildingAnimSlot::SpecialTwo);
				play_health_anim(12, BuildingAnimSlot::SpecialThree);
				play_health_anim(3, BuildingAnimSlot::Active);
			}
			if (!pThis->Anims[(int)BuildingAnimSlot::SpecialThree])
			{
				pThis->IsReadyToCommence = true;
				pThis->QueueMission(Mission::Guard, true);
			}
			return 1;
		}

		// FIX 2: original checks Anims[8] == BuildingAnimSlot::Production, not Special
		if (pThis->Anims[(int)BuildingAnimSlot::Production])
		{
			return 1;
		}

		// Docking proximity check — pull client in if too far
		{
			auto* pClient = (FootClass*)pThis->GetRadioContact(0);
			int const dist = pThis->DistanceFrom(pClient);
			if (dist < 200)
			{
				// MINOR: rename for clarity — variable now matches its meaning
				bool const is_powered_on = pClient->Locomotor->Is_Powered();

				if (is_powered_on)
				{
					if (!pClient->Locomotor->Is_Moving())
					{
						pClient->Locomotor->Power_Off();
					}
					return 1;
				}

				if (pClient->Destination)
				{
					pClient->AbortMotion();
				}
			}
		}

		// Ask "do you need to move?"
		auto const move_msg = pThis->SendToFirstLink(RadioCommand::QueryMoving);
		if (move_msg != RadioCommand::AnswerPositive)
		{
			if (!pThis->Owner->IonSensitivesShouldBeOffline())
			{
				auto* pClient = (FootClass*)pThis->GetRadioContact(0);
				if (!pClient->Locomotor->Is_Powered())
				{
					pClient->Locomotor->Power_On();
				}
			}
			return return_mission_rate();
		}

		// Positive — evaluate repair vs sell-back
		auto* pClient = (FootClass*)pThis->GetRadioContact(0);
		bool const needs_repair =
			pClient->GetHealthPercentage() < RulesClass::Instance->ConditionGreen;
		bool const needs_reload = pClient->GetTechnoType()->ManualReload;

		auto const repair_msg = pThis->SendToFirstLink(RadioCommand::RequestRepair);
		bool const repair_ok = (repair_msg == RadioCommand::AnswerPositive);
		bool const repair_waiting = (repair_msg == RadioCommand::QueryDone);

		if ((needs_repair || needs_reload) && (repair_ok || repair_waiting))
		{
			if (pClient->IsUseless && !pClient->Owner->IsControlledByHuman())
			{
				pClient->Sell(1);
				pThis->MissionStatus = 0;
				pThis->IsReadyToCommence = true;
				return return_mission_rate();
			}

			if (pThis->IsOwnedByCurrentPlayer)
			{
				VoxClass::Play(GameStrings::EVA_Repairing);
			}

			pThis->MissionStatus = 2;
			play_health_anim(10, BuildingAnimSlot::Special);
			pThis->DestroyNthAnim(BuildingAnimSlot::Active);
			pThis->DestroyNthAnim(BuildingAnimSlot::Idle);

			pThis->IsReadyToCommence = false;
			pThis->RepairProgress.Stage = 0;
			pThis->RepairProgress.Step = 1;
			pThis->RepairProgress.Timer.Start(1);
			return return_mission_rate();
		}

		// Client is full-health and doesn't need reload -> release it
		if (pClient->GetHealthPercentage() == RulesClass::Instance->ConditionGreen)
		{
			if (!pClient->Locomotor->Is_Powered())
			{
				pClient->Locomotor->Power_On();

				if (pClient->ArchiveTarget && !pClient->Owner->IsControlledByHuman())
				{
					pClient->QueueMission(Mission::Move, false);
					pClient->SetDestination(pClient->ArchiveTarget, true);
					pClient->SetArchiveTarget(nullptr);
					pClient->QueueUpToEnter = nullptr;
					pThis->SendToEachLink(RadioCommand::NotifyUnlink);
				}
				else
				{
					CellStruct exit_cell = pThis->FindExitCell(pClient, CellStruct::Empty);

					if (pThis->ArchiveTarget)
					{
						exit_cell = CellClass::Coord2Cell(pThis->ArchiveTarget->GetCoords());
					}

					if (exit_cell.IsValid())
					{
						pClient->QueueMission(Mission::Move, false);
						pClient->SetDestination(MapClass::Instance->TryGetCellAt(exit_cell), true);
						pClient->SetArchiveTarget(nullptr);
						pThis->SendToEachLink(RadioCommand::NotifyUnlink);
						pClient->QueueUpToEnter = nullptr;
					}
				}
			}
		}
		return return_mission_rate();
	}

	// ----- Status 2: actively repairing -----
	case 2:
	{
		// FIX 5: no Transmit_Message here; just degrade to status 1
		if (!pThis->HasAnyLink())
		{
			pThis->DestroyNthAnim(BuildingAnimSlot::Production);
			pThis->DestroyNthAnim(BuildingAnimSlot::SpecialTwo);
			play_health_anim(12, BuildingAnimSlot::SpecialThree);
			play_health_anim(3, BuildingAnimSlot::Active);
			pThis->MissionStatus = 1;
			return 1;
		}

		if (pThis->RepairProgress.Step == 0)
		{
			pThis->RepairProgress.Step = 1;
		}
	
		pThis->RepairProgress.Update();

		// Hook 0x44BD38: Units_RepairRate override (URepairRate default)
		double const repairRate =
			pTypeExt->Units_RepairRate.Get(RulesClass::Instance->URepairRate);

		if (repairRate * TICKS_PER_MINUTE > pThis->RepairProgress.Stage)
		{
			return 1;
		}

		if (pThis->SendToFirstLink(RadioCommand::QueryMoving) != RadioCommand::AnswerPositive)
		{
			return 1;
		}

		pThis->IsReadyToCommence = false;
		pThis->RepairProgress.Stage = 0;

		auto const repair_msg = pThis->SendToFirstLink(RadioCommand::RequestRepair);

		if (repair_msg == RadioCommand::AnswerPositive)
		{
			return 1;
		}

		bool const out_of_money = (repair_msg == RadioCommand::AnswerBlocked);

		if (out_of_money)
		{
			if (pThis->IsOwnedByCurrentPlayer && !pThis->Owner->Available_Money())
			{
				VoxClass::Play(GameStrings::EVA_InsufficientFunds);
			}
			pThis->DestroyNthAnim(BuildingAnimSlot::Production);
			pThis->DestroyNthAnim(BuildingAnimSlot::SpecialTwo);
			play_health_anim(12, BuildingAnimSlot::SpecialThree);
			play_health_anim(3, BuildingAnimSlot::Active);
			pThis->MissionStatus = 1;
			return 1;
		}

		// Fully repaired path
		if (pThis->IsOwnedByCurrentPlayer)
		{
			auto const cell = pThis->GetCell()->MapCoords;
			if (RadarEventClass::Create(RadarEventType::UnitRepaired, cell))
			{
				VoxClass::Play(GameStrings::EVA_UnitRepaired);
			}
		}

		pThis->DestroyNthAnim(BuildingAnimSlot::Production);
		pThis->DestroyNthAnim(BuildingAnimSlot::SpecialTwo);
		play_health_anim(12, BuildingAnimSlot::SpecialThree);
		play_health_anim(3, BuildingAnimSlot::Active);
		pThis->MissionStatus = 1;

		// Dispatch finished client
		auto pRaw = pThis->GetRadioContact(0);
		auto pClient = (pRaw && (pRaw->AbstractFlags & AbstractFlags::Foot)) ? pRaw : nullptr;

		if (pClient && pClient->ArchiveTarget
			&& !pClient->Owner->IsControlledByHuman())
		{
			pClient->QueueMission(Mission::Move, false);
			pClient->SetDestination(pClient->ArchiveTarget, true);
			pClient->SetArchiveTarget(nullptr);
			pThis->SendToEachLink(RadioCommand::NotifyUnlink);
			pClient->QueueUpToEnter = nullptr;
			return 1;
		}

		CellStruct exit_cell = pThis->FindExitCell(pRaw, CellStruct::Empty);

		if (pThis->ArchiveTarget)
		{
			exit_cell = CellClass::Coord2Cell(pThis->ArchiveTarget->GetCoords());
		}

		if (pClient && exit_cell.IsValid())
		{
			pClient->QueueMission(Mission::Move, false);
			pClient->SetDestination(MapClass::Instance->TryGetCellAt(exit_cell), true);
			pThis->SendToEachLink(RadioCommand::NotifyUnlink);
			pClient->QueueUpToEnter = nullptr;
		}

		return 1;
	}

	default:
		return return_mission_rate();
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x44B780, FakeBuildingClass::_Mission_Repair);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4108, FakeBuildingClass::_Mission_Repair);

//ASMJIT_PATCH(0x44B8F1, BuildingClass_Mission_Repair_Hospital, 0x6)
//{
//	enum { SkipGameCode = 0x44B8F7 };
//
//	GET(FakeBuildingClass*, pThis, EBP);
//	double repairRate = pThis->_GetTypeExtData()->Units_RepairRate.Get(RulesClass::Instance->IRepairRate);
//	__asm { fld repairRate }
//	return SkipGameCode;
//}
//
//ASMJIT_PATCH(0x44BD38, BuildingClass_Mission_Repair_UnitRepair, 0x6)
//{
//	enum { SkipGameCode = 0x44BD3E };
//
//	GET(FakeBuildingClass*, pThis, EBP);
//
//	double repairRate = pThis->_GetTypeExtData()->Units_RepairRate.Get(RulesClass::Instance->URepairRate);
//	__asm { fld repairRate }
//
//	return SkipGameCode;
//}
//
//ASMJIT_PATCH(0x44C836, BuildingClass_Mission_Repair_UnitReload, 0x6)
//{
//	GET(FakeBuildingClass*, pThis, EBP);
//
//	if (pThis->Type->UnitReload)
//	{
//		auto const pTypeExt = pThis->_GetTypeExtData();
//
//		if (pTypeExt->Units_RepairRate.isset())
//		{
//			double repairRate = pTypeExt->Units_RepairRate.Get();
//
//			if (repairRate < 0.0)
//				return 0;
//
//			int rate = static_cast<int>(std::max(repairRate * 900, 1.0));
//
//			auto pExt = pThis->_GetExtData();
//
//			if (!(Unsorted::CurrentFrame % rate))
//			{
//				pExt->SeparateRepair = true;
//
//				for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
//				{
//					if (auto const pLink = pThis->GetNthLink(i))
//					{
//						if (!pLink->IsInAir()  && pLink->Health < GET_TECHNOTYPE(pLink)->Strength && pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
//							pThis->SendCommand(RadioCommand::RequestRepair, pLink);
//					}
//				}
//
//				pExt->SeparateRepair = false;
//			}
//		}
//	}
//
//	return 0;
//}

ASMJIT_PATCH(0x6F4CF0, TechnoClass_ReceiveCommand_Repair, 0x5)
{
	enum { AnswerNegative = 0x6F4CB4 , Continue = 0x0 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pFrom, STACK_OFFSET(0x18, 0x4));

	auto const pType = GET_TECHNOTYPE(pThis);
	int repairStep = pType->GetRepairStep();
	int repairCost = pType->GetRepairStepCost();

	if (auto const pBuilding = cast_to<BuildingClass* , false>(pFrom))
	{
		auto const pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);
		auto pbldExt = BuildingExtContainer::Instance.Find(pBuilding);

		if (pBuilding->Type->UnitReload && pTypeExt->Units_RepairRate.isset() && !pbldExt->SeparateRepair)
			return AnswerNegative;

		repairStep = pTypeExt->Units_RepairStep.Get(repairStep);
		double repairPercent = pTypeExt->Units_RepairPercent.Get(RulesClass::Instance->RepairPercent);

		if (pTypeExt->Units_UseRepairCost.Get(pThis->WhatAmI() != AbstractType::Infantry)) {

			repairCost = static_cast<int>((pType->GetCost() / (pType->Strength / static_cast<double>(repairStep))) * repairPercent);

			if (repairCost < 1)
				repairCost = 1;

		} else {
			repairCost = 0;
		}
	}

	if (repairStep < 1)
		repairStep = 1;

	R->EDI(repairStep);
	R->EBX(repairCost);

	return 0x6F4D26;
}

// Fixes docks not repairing docked aircraft unless they enter the dock first e.g just built ones.
// Also potential edge cases with unusual docking offsets, original had a distance check for 64 leptons which is replaced with IsInAir here.
ASMJIT_PATCH(0x44985B, BuildingClass_Mission_Guard_UnitReload, 0x6)
{
	enum { AssignRepairMission = 0x449942 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pLink, EDI);

	if (pThis->Type->UnitReload && pLink->WhatAmI() == AbstractType::Aircraft && !pLink->IsInAir()
		&& pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
	{
		return AssignRepairMission;
	}

	return 0;
}