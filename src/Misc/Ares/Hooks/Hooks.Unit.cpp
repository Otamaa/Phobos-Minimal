#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <SlaveManagerClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/BuildingType/Body.h>

#include <WWKeyboardClass.h>
#include <Conversions.h>
#include <Locomotor/TunnelLocomotionClass.h>

#include <New/Entity/FlyingStrings.h>

#include "Header.h"

DEFINE_HOOK(0x73D219, UnitClass_Draw_OreGatherAnim, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	// disabled ore gatherers should not appear working.
	return (pTechno->IsWarpingIn() || pTechno->IsUnderEMP()) ?
		0x73D28E : 0x73D223;
}

DEFINE_HOOK(0x7461C5, UnitClass_BallooonHoverExplode_OverrideCheck, 0x6)
{
	GET(UnitClass*, pThis, EDI);
	GET(UnitTypeClass*, pType, EAX);

	R->CL(pType->BalloonHover || pType->Explodes || pThis->HasAbility(AbilityType::Explodes));
	return 0x7461CB;
}

DEFINE_HOOK(0x73F7DD, UnitClass_IsCellOccupied_Bib, 0x8)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(UnitClass*, pThis, EBX);

	return pThis && pBuilding->Owner->IsAlliedWith(pThis) ? 0x0 : 0x73F823;
}

// #1171643: keep the last passenger if this is a gunner, not just
// when it has multiple turrets. gattling and charge turret is no
// longer affected by this.

DEFINE_HOOK(0x73D81C, UnitClass_Mi_Unload_LastPassenger, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	R->EAX(pThis->GetTechnoType()->Gunner);
	return 0x73D823;
}

//UnitClass_ActionOnObject_IvanBombs
DEFINE_JUMP(LJMP, 0x7388EB, 0x7388FD);

//UnitClass_DrawSHP_SkipTurretedShadow
DEFINE_JUMP(LJMP, 0x73C733, 0x73C7AC);

DEFINE_HOOK(0x741206, UnitClass_CanFire, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto Type = pThis->Type;

	if (!Type->TurretCount || Type->IsGattling)
	{
		return 0x741229;
	}

	const auto W = pThis->GetWeapon(pThis->SelectWeapon(nullptr));
	return (W->WeaponType && W->WeaponType->Warhead->Temporal)
		? 0x741210u
		: 0x741229u
		;
}

DEFINE_HOOK(0x73C613, UnitClass_DrawSHP_FacingsA, 0x7)
{
	GET(UnitClass*, pThis, EBP);

	unsigned int ret = 0;

	if (pThis->Type->Facings > 0)
	{
		auto highest = Conversions::Int2Highest(pThis->Type->Facings);

		// 2^highest is the frame count, 3 means 8 frames
		if (highest >= 3 && !pThis->IsDisguised())
		{
			ret = pThis->PrimaryFacing.Current().GetValue(highest, 1u << (highest - 3));
		}
	}

	R->EBX(ret);
	return 0x73C64B;
}

DEFINE_HOOK(0x73CD01, UnitClass_DrawSHP_FacingsB, 0x5)
{
	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);
	GET(int, facing, EAX);

	R->ECX(pThis);
	R->EAX(facing + pType->WalkFrames * pType->Facings);

	return 0x73CD06;
}

DEFINE_HOOK(0x739ADA, UnitClass_SimpleDeploy_Height, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Deployed)
		return 0x739CBF;

	if (!pThis->InAir && pThis->Type->DeployToLand && pThis->GetHeight() > 0)
		pThis->InAir = 1;

	R->EAX(true);
	return 0x739B14;
}

DEFINE_HOOK(0x736E8E, UnitClass_UpdateFiringState_Heal, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pTargetTechno = generic_cast<TechnoClass*>(pThis->Target);

	if (!pTargetTechno || pTargetTechno->GetHealthPercentage() <= RulesClass::Instance()->ConditionGreen)
		pThis->SetTarget(nullptr);

	return 0x737063;
}

DEFINE_HOOK(0x7440BD, UnitClass_Remove, 0x6)
{
	GET(UnitClass*, U, ESI);

	if (auto Bld = specific_cast<BuildingClass*>(U->BunkerLinkedItem))
	{
		Bld->ClearBunker();
	}

	return 0;
}

DEFINE_HOOK(0x739B8A, UnitClass_SimpleDeploy_Facing, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	auto const pType = pThis->Type;

	if (pType->DeployingAnim)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
		// not sure what is the bitfrom or bitto so it generate this result
		// yes iam dum , iam sorry - otamaa
		const auto nRulesDeployDir = ((((RulesClass::Instance->DeployDir) >> 4) + 1) >> 1) & 7;
		const FacingType nRaw = pTypeExt->DeployDir.isset() ? pTypeExt->DeployDir.Get() : (FacingType)nRulesDeployDir;
		const auto nCurrent = (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 7);

		if (nCurrent != (int)nRaw)
		{
			if (const auto pLoco = pThis->Locomotor.GetInterfacePtr())
			{
				if (!pLoco->Is_Moving_Now())
				{
					pLoco->Do_Turn(DirStruct { nRaw });
				}

				return 0x739C70;
			}
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x74642C, UnitClass_ReceiveGunner, 6)
{
	GET(UnitClass*, Unit, ESI);

	const auto pTemp = std::exchange(Unit->TemporalImUsing , nullptr);

	if (pTemp)
		pTemp->LetGo();

	TechnoExtContainer::Instance.Find(Unit)->MyOriginalTemporal = pTemp;
	return 0;
}

DEFINE_HOOK(0x74653C, UnitClass_RemoveGunner, 0xA)
{
	GET(UnitClass*, Unit, EDI);
	Unit->TemporalImUsing = std::exchange(TechnoExtContainer::Instance.Find(Unit)->MyOriginalTemporal, nullptr);
	return 0x746546;
}

//DEFINE_HOOK(0x746420, UnitClass_ReceiveGunner, 5)
//{
//	GET(UnitClass*, pThis, ECX);
//	GET_STACK(FootClass*, Gunner, 0x4);
//
//	if(Gunner) {
//		if (Gunner->TemporalImUsing) {
//
//			const auto pTemp = std::exchange(pThis->TemporalImUsing, nullptr);
//
//			if (pTemp)
//				pTemp->LetGo();
//
//			TechnoExtContainer::Instance.Find(pThis)->MyOriginalTemporal = pTemp;
//			pThis->TemporalImUsing = std::exchange(Gunner->TemporalImUsing, nullptr);
//			pThis->TemporalImUsing->Owner = pThis;
//			//pThis->SwitchTurretWeapon(7);
//		}
//
//		if (Gunner->DiskLaserTimer.HasTimeLeft()) {
//			++pThis->CurrentBurstIndex;
//			pThis->DiskLaserTimer.Start(pThis->GetROF(7)); //hardcoded it seems
//		}
//
//		pThis->SwitchTurretWeapon(Gunner->GetTechnoType()->IFVMode);
//	}
//
//
//}
//
//DEFINE_HOOK(0x7464E0 , UnitClass_RemoveGunner, 5)
//{
//	GET(UnitClass*, pThis, ECX);
//	GET_STACK(FootClass* , Gunner , 0x4);
//
//	if(Gunner) {
//
//		if(!pThis->DiskLaserTimer.GetTimeLeft()) {
//			++pThis->CurrentBurstIndex;
//			pThis->DiskLaserTimer.Start(pThis->GetROF(0));
//		}
//
//		if (pThis->TemporalImUsing)
//		{
//			Gunner->TemporalImUsing = pThis->TemporalImUsing;
//			pThis->TemporalImUsing->Owner = Gunner;
//			auto pData = TechnoExtContainer::Instance.Find(pThis);
//			pThis->TemporalImUsing = std::exchange(pData->MyOriginalTemporal, nullptr);
//
//			if (Gunner->TemporalImUsing && Gunner->TemporalImUsing->Target)
//			{
//				Gunner->TemporalImUsing->LetGo();
//			}
//		}
//	}
//
//	pThis->SwitchTurretWeapon(0);
//	 return 0x7465A1;
//}

DEFINE_HOOK(0x73769E, UnitClass_ReceivedRadioCommand_SpecificPassengers, 8)
{
	GET(UnitClass* const, pThis, ESI);
	GET(TechnoClass const* const, pSender, EDI);

	auto const pType = pThis->GetTechnoType();
	auto const pSenderType = pSender->GetTechnoType();

	return TechnoTypeExtData::PassangersAllowed(pType, pSenderType) ? 0u : 0x73780Fu;
}

DEFINE_HOOK(0x73762B, UnitClass_ReceivedRadioCommand_BySize1, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x737677 : 0x73780F;
}

DEFINE_HOOK(0x73778F, UnitClass_ReceivedRadioCommand_BySize2, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers == pType->Passengers ?
		0x7377AA : 0x7377C9;
}

DEFINE_HOOK(0x73782F, UnitClass_ReceivedRadioCommand_BySize3, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x737877 : 0x73780F;
}

DEFINE_HOOK(0x737994, UnitClass_ReceivedRadioCommand_BySize4, 6)
{
	GET(UnitClass*, pThis, ESI);

	auto pType = pThis->Type;
	auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->Passengers_BySize.Get())
		return 0;

	return pThis->Passengers.NumPassengers < pType->Passengers ?
		0x7379E8 : 0x737AFC;
}

DEFINE_HOOK(0x6FC0D3, TechnoClass_CanFire_DisableWeapons, 8)
{
	enum { FireRange = 0x6FC0DF, ContinueCheck = 0x0 };
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if(pExt->DisableWeaponTimer.InProgress())
		return FireRange;

	if(pExt->AE_DisableWeapons)
		return FireRange;

	return ContinueCheck;
}

// stop command would still affect units going berzerk
DEFINE_HOOK(0x730EE5, StopCommandClass_Execute_Berzerk, 6)
{
	GET(TechnoClass*, pTechno, ESI);
	return pTechno->Berzerk || TechnoExtContainer::Instance.Find(pTechno)->Is_DriverKilled ? 0x730EF7 : 0;
}

DEFINE_HOOK(0x6F3283, TechnoClass_CanScatter_KillDriver, 8)
{
	// prevent units with killed drivers from scattering when attacked.
	GET(TechnoClass*, pThis, ESI);
	return (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x6F32C5u : 0u);
}

DEFINE_HOOK(0x7091D6, TechnoClass_CanPassiveAquire_KillDriver, 6)
{
	// prevent units with killed drivers from looking for victims.
	GET(TechnoClass*, pThis, ESI);
	return (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x70927Du : 0u);
}

DEFINE_HOOK(0x6F6A58, TechnoClass_DrawHealthBar_HidePips_KillDriver, 6)
{
	// prevent player from seeing pips on transports with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	return HouseClass::IsCurrentPlayerObserver() ||
	TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x6F6AB6u : 0u;
}

DEFINE_HOOK(0x7087EB, TechnoClass_ShouldRetaliate_KillDriver, 6)
{
	// prevent units with killed drivers from retaliating.
	GET(TechnoClass*, pThis, ESI);
	return (TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x708B17u : 0u);
}

DEFINE_HOOK(0x73758A, UnitClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	return TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled ? 0x73761Fu : 0u;
}

DEFINE_HOOK(0x70DEBA, TechnoClass_UpdateGattling_Cycle, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, lastStageValue, EAX);
	GET_STACK(int, a2, 0x24);

	auto pType = pThis->GetTechnoType();

	if (pThis->GattlingValue < lastStageValue)
	{
		// just increase the value
		pThis->GattlingValue += a2 * pType->RateUp;
	}
	else
	{
		// if max or higher, reset cyclic gattlings
		if (TechnoTypeExtContainer::Instance.Find(pType)->GattlingCyclic)
		{
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			pThis->Audio4.AudioEventHandleStop();
			pThis->GattlingAudioPlayed = false;
		}
	}

	// recreate hooked instruction
	R->Stack<int>(0x10, pThis->GattlingValue);

	return 0x70DEEB;
}

// do not let deactivated teleporter units move, otherwise
// they could block a cell forever
DEFINE_HOOK(0x71810D, TeleportLocomotionClass_ILocomotion_MoveTo_Deactivated, 6)
{
	GET(FootClass*, pFoot, ECX);
	return (!pFoot->Deactivated && pFoot->Locomotor.GetInterfacePtr()->Is_Powered() && !TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled)
		? 0 : 0x71820F;
}

// sink stuff that simply cannot exist on water
DEFINE_HOOK(0x7188F2, TeleportLocomotionClass_Unwarp_SinkJumpJets, 7)
{
	GET(CellClass*, pCell, EAX);
	GET(TechnoClass**, pTechno, ESI);

	if (pCell->Tile_Is_Wet() && !pCell->ContainsBridge())
	{
		if (UnitClass* pUnit = specific_cast<UnitClass*>(pTechno[3]))
		{
			if (pUnit->Deactivated || TechnoExtContainer::Instance.Find(pUnit)->Is_DriverKilled)
			{
				// this thing does not float
				R->BL(0);
			}

			// manually sink it
			if (pUnit->Type->SpeedType == SpeedType::Hover && pUnit->Type->JumpJet)
			{
				return 0x718A66;
			}
		}
	}

	return 0;
}

// iron curtained units would crush themselves
DEFINE_HOOK(0x7187DA, TeleportLocomotionClass_Unwarp_PreventSelfCrush, 6)
{
	GET(TechnoClass*, pTeleporter, EDI);
	GET(TechnoClass*, pContent, ECX);
	return (pTeleporter == pContent) ? 0x71880A : 0;
}

// bug 897
DEFINE_HOOK(0x718871, TeleportLocomotionClass_UnfreezeObject_SinkOrSwim, 7)
{
	GET(TechnoTypeClass*, Type, EAX);

	switch (Type->MovementZone)
	{
	case MovementZone::Amphibious:
	case MovementZone::AmphibiousCrusher:
	case MovementZone::AmphibiousDestroyer:
	case MovementZone::WaterBeach:
		R->BL(1);
		return 0x7188B1;
	}
	if (Type->SpeedType == SpeedType::Hover)
	{
		// will set BL to 1 , unless this is a powered unit with no power centers <-- what if we have a powered unit that's not a hover?
		return 0x71887A;
	}
	return 0x7188B1;
}

#include <Ext/Building/Body.h>

// sanitize the power output
DEFINE_HOOK(0x508D4A, HouseClass_UpdatePower_LocalDrain2, 6)
{
	GET(HouseClass*, pThis, ESI);
	if(pThis->PowerOutput < 0) {
		pThis->PowerOutput = 0;
	}
	return 0;
}

DEFINE_HOOK(0x73DE90, UnitClass_Mi_Unload_SimpleDeployer, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pThis->Deployed
		&& pTypeExt->Convert_Deploy
		&& TechnoExt_ExtData::ConvertToType(pThis, pTypeExt->Convert_Deploy))
	{
		if(pTypeExt->Convert_Deploy_Delay > 0)
			TechnoExtContainer::Instance.Find(pThis)->Convert_Deploy_Delay
				//.Start(100);
				.Start(pTypeExt->Convert_Deploy_Delay);

		pThis->Deployed = false;
	}

	TechnoExtData::InitializeLaserTrail(pThis, true);
	TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);
	//LineTrailExt::DeallocateLineTrail(pUnit);
	//LineTrailExt::ConstructLineTrails(pUnit);

	return pThis->Locomotor.GetInterfacePtr()->Is_Moving_Now() ? 0x73E5B1 : 0x0;
}

#include <Ext/CaptureManager/Body.h>

// do not order deactivated units to move
DEFINE_HOOK(0x73DBF9, UnitClass_Mi_Unload_Decactivated, 5)
{
	GET(UnitClass*, pUnloadee, EDI);
	LEA_STACK(CellStruct**, ppCell, 0x0);
	LEA_STACK(CellStruct*, pPosition, 0x1C);

	const auto pLoco = pUnloadee->Locomotor.GetInterfacePtr();

	if (pUnloadee->Deactivated)
	{
		pLoco->Power_Off();
	}

	if (!pLoco->Is_Powered())
	{
		*ppCell = pPosition;
	}

	//auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pUnloadee->Type);
	//if(!pTypeExt->Operators.empty() || pTypeExt->Operator_Any)
	//{
	//	auto pCapturer = pUnloadee->MindControlledBy;
	//	if(pCapturer && pCapturer->CaptureManager)
	//	{
	//		if(auto pOriginal = pCapturer->CaptureManager->GetOriginalOwner(pUnloadee))
	//		{
	//			++Unsorted::ScenarioInit();
	//			CaptureExt::FreeUnit(pCapturer->CaptureManager,pUnloadee,true);
	//			pUnloadee->SetOwningHouse(pOriginal,false);
	//			CaptureExt::CaptureUnit(pCapturer->CaptureManager , pUnloadee ,false ,true , pUnloadee->MindControlRingAnim->Type);
	//			--Unsorted::ScenarioInit();
	//		}
	//	}
	//	else
	//	{
	//		//transfer the anim
	//	}
	//}

	return 0;
}

//UnitClass_Mi_Harvest_SkipDock
DEFINE_JUMP(LJMP, 0x73E66D, 0x73E6CF);

DEFINE_HOOK(0x6AF748, SlaveManagerClass_UpdateSlaves_SlaveScan, 6)
{
	GET(InfantryClass*, pSlave, ESI);
	//GET(SlaveManagerClass*, pThis, EDI);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSlave->Type);
	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerSlaveScan));
	return 0x6AF74E;
}

DEFINE_HOOK_AGAIN(0x6B026C, SlaveManagerClass_UpdateMiner_ShortScan, 6)
DEFINE_HOOK(0x6B006D, SlaveManagerClass_UpdateMiner_ShortScan, 6)
{
	GET(TechnoClass*, pSlaveOwner, ECX);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSlaveOwner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerShortScan));
	return R->Origin() + 0x6;
}

DEFINE_HOOK(0x6B01A3, SlaveManagerClass_UpdateMiner_ScanCorrection, 6)
{
	GET(SlaveManagerClass*, pThis, ESI);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_ScanCorrection.Get(RulesClass::Instance->SlaveMinerScanCorrection));
	return 0x6B01A9;
}

DEFINE_HOOK_AGAIN(0x6B02CC, SlaveManagerClass_UpdateMiner_LongScan, 6)
DEFINE_HOOK_AGAIN(0x6B00BD, SlaveManagerClass_UpdateMiner_LongScan, 6)
DEFINE_HOOK(0x6AFDFC, SlaveManagerClass_UpdateMiner_LongScan, 6)
{
	GET(TechnoClass*, pSlaveOwner, ECX);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSlaveOwner->GetTechnoType());

	R->EAX(pTypeExt->Harvester_LongScan.Get(RulesClass::Instance->SlaveMinerLongScan));
	return R->Origin() + 0x6;
}

DEFINE_HOOK(0x6B1065, SlaveManagerClass_ShouldWakeUp_ShortScan, 5)
{
	GET(SlaveManagerClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Owner->GetTechnoType());
	auto nKickFrameDelay = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	if (nKickFrameDelay < 0 || nKickFrameDelay + pThis->LastScanFrame >= Unsorted::CurrentFrame)
		return 0x6B10C6;

	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->SlaveMinerShortScan));
	return 0x6B1085;
}

DEFINE_HOOK(0x73EC0E, UnitClass_Mi_Harvest_TooFarDistance1, 6)
{
	GET(UnitClass*, pThis, EBP);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	R->EDX(pTypeExt->Harvester_TooFarDistance.Get(RulesClass::Instance->HarvesterTooFarDistance));
	return 0x73EC14;
}

DEFINE_HOOK(0x73EE40, UnitClass_Mi_Harvest_TooFarDistance2, 6)
{
	GET(UnitClass*, pThis, EBP);

	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	R->EDX(pTypeExt->Harvester_TooFarDistance.Get(RulesClass::Instance->ChronoHarvTooFarDistance));
	return 0x73EE46;
}

DEFINE_HOOK_AGAIN(0x73EAC6, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_HOOK_AGAIN(0x73EAA6, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_HOOK_AGAIN(0x73EA17, UnitClass_Mi_Harvest_ShortScan, 6)
DEFINE_HOOK(0x73E9F1, UnitClass_Mi_Harvest_ShortScan, 6)
{
	GET(UnitClass*, pThis, EBP);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	R->EAX(pTypeExt->Harvester_ShortScan.Get(RulesClass::Instance->TiberiumShortScan));
	return R->Origin() + 0x6;
}

DEFINE_HOOK_AGAIN(0x73E772, UnitClass_Mi_Harvest_LongScan, 6)
DEFINE_HOOK(0x73E851, UnitClass_Mi_Harvest_LongScan, 6)
{
	GET(UnitClass*, pThis, EBP);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	R->EAX(pTypeExt->Harvester_LongScan.Get(RulesClass::Instance->TiberiumLongScan));
	return R->Origin() + 0x6;
}

DEFINE_HOOK(0x74081F, UnitClass_Mi_Guard_KickFrameDelay, 5)
{
	GET(UnitClass*, pThis, ESI);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	return(nFrame < 0 || nFrame + pThis->CurrentMissionStartTime >= Unsorted::CurrentFrame) ?
		0x740854 : 0x74083B;
}

DEFINE_HOOK(0x74410D, UnitClass_Mi_AreaGuard_KickFrameDelay, 5)
{
	GET(UnitClass*, pThis, ESI);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
	auto nFrame = pTypeExt->Harvester_KickDelay.Get(RulesClass::Instance->SlaveMinerKickFrameDelay);

	return(nFrame < 0 || nFrame + pThis->CurrentMissionStartTime >= Unsorted::CurrentFrame) ?
		0x74416C : 0x744129;
}


DEFINE_HOOK_AGAIN(0x735678, UnitClass_Init_Academy, 6) // inlined in CTOR
DEFINE_HOOK(0x74689B, UnitClass_Init_Academy, 6)
{
	GET(UnitClass*, pThis, ESI);

	if (!pThis->Owner)
		return 0x0;

	const auto pType = pThis->Type;
	const auto pHouseExt = HouseExtContainer::Instance.Find(pThis->Owner);

	if (pType->Trainable && pType->Naval && pHouseExt->Is_NavalYardSpied)
	{
		pThis->Veterancy.SetVeteran();
	}

	AbstractType type = AbstractType::Unit;
	if(pType->ConsideredAircraft)
		type = AbstractType::Aircraft;
	else if (pType->Organic)
		type = AbstractType::Infantry;

	HouseExtData::ApplyAcademy(pThis->Owner, pThis, type);

	return 0;
}

// make the space between gunner name segment and ifv
// name smart. it disappears if one of them is empty,
// eliminating leading and trailing spaces.
DEFINE_HOOK(0x746C55, UnitClass_GetUIName_Space, 6)
{
	GET(UnitClass*, pThis, ESI);
	GET(wchar_t*, pGunnerName, EAX);

	const auto pName = pThis->Type->UIName;
	const auto pSpace = (pName && *pName && pGunnerName && *pGunnerName) ? L" " : L"";
	_snwprintf_s(pThis->ToolTipText, sizeof(pThis->ToolTipText), L"%s%s%s", pGunnerName, pSpace, pName);

	R->EAX(pThis->ToolTipText);
	return 0x746C76;
}

DEFINE_HOOK(0x740031, UnitClass_GetActionOnObject_NoManualUnload, 6)
{
	GET(UnitClass const* const, pThis, ESI);

	/*
	if(pExt->NoManualUnload)
		return 0x740115u;

	if(pThis->Passengers.NumPassengers)
		R->EBP(&pThis->Passengers);
	else
	{
		int nVal = Action::NoDeploy;
		if(pThis->Type->DeployFire)
		nVal = Action::Self;

		R->Stack(0x28 ,nVal);
		return 0x73FFFC;
	}
	*/
	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoManualUnload ? 0x740115u : 0u;
}

DEFINE_HOOK(0x417DD2, AircraftClass_GetActionOnObject_NoManualUnload, 6)
{
	enum { RetDefault = 0, NoUnload = 0x417DF4 };

	GET(AircraftClass const* const, pThis, ESI);
	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoManualUnload ? 0x417DF4u : 0u;
}

DEFINE_HOOK(0x700EEC, TechnoClass_CanDeploySlashUnload_NoManualUnload, 6)
{
	// this techno is known to be a unit
	GET(UnitClass const* const, pThis, ESI);

	return !TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoManualUnload || pThis->BunkerLinkedItem
		? 0u : 0x700DCEu;
}

DEFINE_HOOK(0x53C450, TechnoClass_CanBePermaMC, 5)
{
	// complete rewrite. used by psychic dominator, ai targeting, etc.
	GET(TechnoClass*, pThis, ECX);
	BYTE bDisaalow = 0;

	if (pThis && pThis->WhatAmI() != AbstractType::Building
		&& !pThis->IsIronCurtained() && !pThis->IsInAir())
	{

		const auto TechnoExt = TechnoExtContainer::Instance.Find(pThis);
		if (!TechnoExtData::IsPsionicsImmune(pThis) && !pThis->GetTechnoType()->BalloonHover)
		{
			// KillDriver check
			if (!TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled)
			{
				bDisaalow = 1;
			}
		}
	}

	R->AL(bDisaalow);
	return 0x53C4BA;
}

DEFINE_HOOK(0x700536, TechnoClass_GetActionOnObject_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualFire ? 0x70056Cu : 0u;
}

DEFINE_HOOK(0x7008D4, TechnoClass_GetCursorOverCell_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	return TechnoTypeExtContainer::Instance.Find(pType)->NoManualFire ? 0x700AB7u : 0u;
}

DEFINE_HOOK(0x74031A, UnitClass_GetActionOnObject_NoManualEnter, 6)
{
	//GET(UnitClass const* const, pThis, ESI);
	GET(TechnoTypeClass*, pTargetType, EAX);
	const bool enterable = pTargetType->Passengers > 0 &&
		!TechnoTypeExtContainer::Instance.Find(pTargetType)->NoManualEnter;
	return enterable ? 0x740324u : 0x74037Au;
}

//TechnoClass_CanAutoTargetObject_Heal
DEFINE_JUMP(LJMP, 0x6F7FC5, 0x6F7FDF);

//DEFINE_HOOK_AGAIN(0x6F8F1F, TechnoClass_FindTargetType_Heal, 6)
//DEFINE_HOOK(0x6F8EE3, TechnoClass_FindTargetType_Heal, 6)
//{
//	GET(unsigned int, nVal, EBX);
//
//	nVal |= 0x403Cu;
//
//	R->EBX(nVal);
//	return 0x6F8F25;
//}

DEFINE_PATCH(0x6F8F21, 0x3C);
DEFINE_PATCH(0x6F8EE5, 0x3C);

DEFINE_HOOK(0x51C913, InfantryClass_CanFire_Heal, 7)
{
	enum { retFireIllegal = 0x51C939 , retContinue = 0x51C947 };
	GET(InfantryClass*, pThis, EBX);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(int , nWeaponIdx , STACK_OFFSET(0x20 , 0x8));

	const auto pThatTechno = generic_cast<TechnoClass*>(pTarget);

	if (!pThatTechno || pThatTechno->IsIronCurtained()){
		return retFireIllegal;
	}

	return  TechnoExt_ExtData::FiringAllowed(pThis , pThatTechno, pThis->GetWeapon(nWeaponIdx)->WeaponType) ?
		retContinue : retFireIllegal;

}

DEFINE_HOOK(0x741113, UnitClass_CanFire_Heal, 0xA)
{
	enum { retFireIllegal = 0x74113A , retContinue = 0x741149 };
	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pThatTechno, EDI);
	GET_STACK(int , nWeaponIdx , STACK_OFFSET(0x1C , 0x8));

	return !pThatTechno->IsIronCurtained() && TechnoExt_ExtData::FiringAllowed(pThis, pThatTechno , pThis->GetWeapon(nWeaponIdx)->WeaponType ) ?
		retContinue : retFireIllegal ;
}

DEFINE_HOOK(0x6F7F4F, TechnoClass_EvalObject_NegativeDamage, 0x7)
{
	enum { SetHealthRatio = 0x6F7F56 , ContinueCheck = 0x6F7F6D , retFalse = 0x6F894F };
	GET(TechnoClass*, pThis, EDI);
	GET(ObjectClass*, pThat, ESI);

	const auto nRulesGreen = RulesClass::Instance->ConditionGreen;
	const auto pThatTechno = generic_cast<TechnoClass*>(pThat);

	if (!pThatTechno){
		return pThat->GetHealthPercentage_() >= nRulesGreen ?
			retFalse : ContinueCheck;
	}

	return !pThatTechno->IsIronCurtained() && TechnoExt_ExtData::FiringAllowed(pThis,pThatTechno ,pThis->GetWeapon(pThis->SelectWeapon(pThatTechno))->WeaponType) ?
		 ContinueCheck : retFalse;

}

template<bool CheckKeyPress>
std::pair<bool, int> HealActionProhibited(TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pThatTechnoExt = TechnoExtContainer::Instance.Find(pTarget);
	const auto pThatShield = pThatTechnoExt->GetShield();
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

	if constexpr (CheckKeyPress) {
		if (WWKeyboardClass::Instance->IsForceMoveKeyPressed())
			return { true , -1 };
	}

	if (pThatShield && pThatShield->IsActive())
	{
		const auto pShieldType = pThatShield->GetType();

		if (pWHExt->GetVerses(pShieldType->Armor).Verses <= 0.0) {
			return { true , -1 };
		}

		const auto pFoot = abstract_cast<FootClass*>(pTarget);

		if (!pThatShield->CanBePenetrated(pWeapon->Warhead)|| ((pFoot && pFoot->ParasiteEatingMe)))
		{
			if (pShieldType->CanBeHealed)
			{
				const bool IsFullHp = pThatShield->GetHealthRatio() >= RulesClass::Instance->ConditionGreen;

				if (!IsFullHp)
				{
					return { false ,  pShieldType->HealCursorType.Get(-1) };
				}
				else
				{

					if (pThatShield->GetType()->PassthruNegativeDamage)
						return { pTarget->IsFullHP() , -1 };
					else
						return { true , -1 };
				}
			}

			return { true , -1 };
		}
	}

	if (pWHExt->GetVerses(TechnoExtData::GetArmor(pTarget)).Verses <= 0.0)
		return { true , -1 };

	return { pTarget->IsFullHP() , -1 };
}

DEFINE_HOOK(0x51E710, InfantryClass_GetActionOnObject_Heal, 7)
{
	enum
	{
		ActionGuardArea = 0x51E748,
		NextCheck = 0x51E757,
		NextCheck2 = 0x51E7A6,
		DoActionHeal = 0x51E739
	};

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pThat, ESI);

	if (pThis == pThat)
		return TechnoTypeExtContainer::Instance.Find(pThis->Type)->NoSelfGuardArea ? NextCheck2 : ActionGuardArea;

	const auto pThatTechno = generic_cast<TechnoClass*>(pThat);
	if (!pThatTechno)
		return NextCheck;

	const auto pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThatTechno))->WeaponType;
	const auto pThatType = pThatTechno->GetTechnoType();
	const auto&[ret ,nCursorShield] = HealActionProhibited<true>(pThatTechno, pWeapon);

	if (ret) {
		if (const auto pBuilding = specific_cast<BuildingClass*>(pThatTechno)) {
			if (pBuilding->Type->Grinding) {
				return NextCheck2;
			}
		}

		return NextCheck;
	}

	const size_t nCursor = nCursorShield != -1 ? (size_t)nCursorShield :
		(pThatType->Organic || pThat->WhatAmI() == InfantryClass::AbsID) ? 90 : 91;

	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::Heal, false);
	return DoActionHeal;
}

DEFINE_HOOK(0x73FDBD, UnitClass_GetActionOnObject_Heal, 5)
{
	enum { ContinueCheck = 0x73FE48 , CheckIfTargetIsBuilding = 0x73FE2F, DoActionSelect = 0x73FE3B , DoActionGRepair = 0x73FE22 , CheckObjectHP = 0x73FE08 };
	GET(UnitClass*, pThis, ESI);
	GET(ObjectClass*, pThat, EDI);
	GET(Action, nAct, EBX);

	if (nAct == Action::GuardArea)
		return ContinueCheck;

	const auto pThatTechno = generic_cast<TechnoClass*>(pThat);
	if(WWKeyboardClass::Instance->IsForceMoveKeyPressed() ||
		pThis == pThat ||
		!pThatTechno ||
		!pThat->IsSurfaced()
	  )
		return DoActionSelect;

	if (auto const pAir = specific_cast<AircraftClass*>(pThat)) {
		if (pAir->GetCell()->GetBuilding()) {
			return ContinueCheck;
		}
	}

	auto pThatType = pThat->GetTechnoType();
	const auto& [ret, nCursorSelected] = HealActionProhibited<false>(pThatTechno,
			    pThis->GetWeapon(pThis->SelectWeapon(pThat))->WeaponType);

	if(ret)
		return DoActionSelect;

	size_t nCursor = nCursorSelected != -1 ? (size_t)nCursorSelected :
		(pThatType->Organic || pThat->WhatAmI() == InfantryClass::AbsID) ? 90 : 91;

	MouseCursorFuncs::SetMouseCursorAction(nCursor, Action::GRepair, false);

	return DoActionGRepair;//0x73FE08;
}

DEFINE_HOOK_AGAIN(0x73C69D, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_HOOK_AGAIN(0x73C702, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_HOOK(0x73C655, UnitClass_DrawSHP_ChangeType1, 6)
{
	GET(UnitClass*, U, EBP);

	if (UnitTypeClass* pCustomType = TechnoExt_ExtData::GetUnitTypeImage(U))
	{
		R->ECX<UnitTypeClass*>(pCustomType);
		return R->Origin() + 6;
	}

	return 0;
}

DEFINE_HOOK(0x73C5FC, UnitClass_DrawSHP_WaterType, 6)
{
	GET(UnitClass*, U, EBP);

	SHPStruct* Image = U->GetImage();

	if (UnitTypeClass* pCustomType = TechnoExt_ExtData::GetUnitTypeImage(U))
	{
		Image = pCustomType->GetImage();
	}

	if (Image)
	{
		R->EAX<SHPStruct*>(Image);
		return 0x73C602;
	}

	return 0x73CE00;
}

static bool ShadowAlreadyDrawn;

DEFINE_HOOK(0x73C725, UnitClass_DrawSHP_DrawShadowEarlier, 6)
{
	GET(UnitClass*, U, EBP);

	DWORD retAddr = (U->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
		? 0
		: 0x73CE0D
		;

	// TODO: other conditions where it would not make sense to draw shadow
	switch (U->VisualCharacter(VARIANT_FALSE, nullptr))
	{
	case VisualType::Normal:
	case VisualType::Indistinct:
		break;
	default:
		return retAddr;
	}

	if (U->CloakState != CloakState::Uncloaked ||
		U->Type->Underwater ||
		U->Type->SmallVisceroid || U->Type->LargeVisceroid
	)
	{
		return retAddr;
	}

	GET(SHPStruct*, Image, EDI);

	if (Image)
	{ // bug #960
		GET(int, FrameToDraw, EBX);
		GET_STACK(Point2D, coords, 0x12C);
		LEA_STACK(RectangleStruct*, BoundingRect, 0x134);

		if (U->IsOnCarryall)
		{
			coords.Y -= 14;
		}

		Point2D XYAdjust = U->Locomotor.GetInterfacePtr()->Shadow_Point();
		coords += XYAdjust;

		int ZAdjust = U->GetZAdjustment() - 2;

		FrameToDraw += Image->Frames / 2;

		DSurface::Hidden_2->DrawSHP(FileSystem::THEATER_PAL, Image, FrameToDraw, &coords, BoundingRect, BlitterFlags(0x2E01),
				0, ZAdjust, 0, 1000, 0, 0, 0, 0, 0);

		ShadowAlreadyDrawn = true;
	}

	return retAddr;
}

DEFINE_HOOK(0x705FF3, TechnoClass_Draw_A_SHP_File_SkipUnitShadow, 6)
{
	if (ShadowAlreadyDrawn)
	{
		ShadowAlreadyDrawn = false;
		return 0x706007;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x44A03C, DeploysInto_UndeploysInto_SyncStatuses, 0x6) //BuildingClass_Mi_Selling_SyncShieldStatus
DEFINE_HOOK(0x739956, DeploysInto_UndeploysInto_SyncStatuses, 0x6) //UnitClass_Deploy_SyncShieldStatus
{
	GET(TechnoClass*, pFrom, EBP);
	GET(TechnoClass*, pTo, EBX);

	// attahed bomb
	// ae
	// original owner
	TechnoExt_ExtData::TransferIvanBomb(pFrom , pTo);
	AresAE::TransferAttachedEffects(pFrom , pTo);
	TechnoExt_ExtData::TransferOriginalOwner(pFrom , pTo);
	//AresData::TechnoTransferAffects(pFrom, pTo);
	TechnoExtData::TransferMindControlOnDeploy(pFrom, pTo);
	ShieldClass::SyncShieldToAnother(pFrom, pTo);
	TechnoExtData::SyncIronCurtainStatus(pFrom, pTo);

	if (pFrom->AttachedTag)
		pTo->AttachTrigger(pFrom->AttachedTag);

	if (R->Origin() == 0x44A03C)
		pTo->QueueMission(Mission::Hunt, true);

	return 0;
}

DEFINE_HOOK(0x4140EB, AircraftClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, EDI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(0x517DF2, InfantryClass_DTOR_Prereqs, 6)
{
	GET(InfantryClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(0x7357F6, UnitClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExtContainer::Instance.Find(pThis->Type)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(0x4D7221, FootClass_Put_Prereqs, 6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (TechnoTypeExtContainer::Instance.Find(pType)->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6F4A37, TechnoClass_DiscoveredBy_Prereqs, 5)
DEFINE_HOOK(0x6F4A1D, TechnoClass_DiscoveredBy_Prereqs, 6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (pThis->WhatAmI() != BuildingClass::AbsID && TechnoTypeExtContainer::Instance.Find(pType)->IsGenericPrerequisite())
	{
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(0x741613, UnitClass_ApproachTarget_OmniCrusher, 6)
{
	GET(UnitClass* const, pThis, ESI);
	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->OmniCrusher_Aggressive
		? 0u : 0x741685u;
}

DEFINE_HOOK(0x7418E1, UnitClass_CrushCell_DeathWeapon, 0xA)
{
	GET(ObjectClass* const, pVictim, ESI);

	if (auto const pVictimTechno = abstract_cast<TechnoClass*>(pVictim))
	{
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pVictim->GetTechnoType());

		const auto nChance = abs(pExt->CrushFireDeathWeapon.Get(pVictimTechno));

		if (nChance == 0.0f)
			return 0x0;

		if (ScenarioClass::Instance->Random.RandomDouble() <= nChance)
			pVictimTechno->FireDeathWeapon(0);
	}

	return 0x0;
}

DEFINE_HOOK(0x74192E, UnitClass_CrushCell_CrushDecloak, 0x5)
{
	enum { Decloak = 0x0, DoNotDecloak = 0x741939 };
	GET(UnitClass* const, pThis, EDI);

	return TechnoTypeExtContainer::Instance.Find(pThis->Type)->CrusherDecloak ? Decloak : DoNotDecloak;
}

DEFINE_HOOK(0x7418A1, UnitClass_CrusCell_TiltWhenCrushSomething, 0x5)
{
	enum { DoNotTilt = 0x7418AA , Tilt = 0x7418A6 };
	GET(ObjectClass* const, pVictim, ESI);
	GET(UnitClass* const, pThis, EDI);
	GET(AbstractType, nWhat, EAX);

	if (!pThis->IsVoxel())
		return DoNotTilt;

	switch (nWhat)
	{
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::Terrain:
		return Tilt;
	case AbstractType::Infantry:
	{
		const auto pInf = static_cast<InfantryClass*>(pVictim);
		if (pInf->Type->Cyborg)
			return Tilt;

		break;
	}
	default:
		break;
	}

	return DoNotTilt;
}

DEFINE_HOOK(0x7418AA, UnitClass_CrushCell_CrushDamage, 6)
{
	GET(UnitClass* const, pThis, EDI);
	GET(ObjectClass* const, pVictim, ESI);

	if (auto const pVictimTechno = abstract_cast<TechnoClass*>(pVictim))
	{
		const auto pExt = TechnoExtContainer::Instance.Find(pVictimTechno);
		const auto pThisTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);
		const auto pVictimTypeExt = TechnoTypeExtContainer::Instance.Find(pVictim->GetTechnoType());
		auto damage = pVictimTypeExt->CrushDamage.Get(pVictimTechno);

		if (pThisTypeExt->Crusher_SupressLostEva)
			pExt->SupressEVALost = true;

		if (damage != 0)
		{
			const auto pWarhead = pVictimTypeExt->CrushDamageWarhead.Get(
				RulesClass::Instance->C4Warhead);

			pThis->ReceiveDamage(
				&damage, 0, pWarhead, nullptr, false, false, nullptr);
			if (pVictimTypeExt->CrushDamagePlayWHAnim) {
				auto loc = pVictim->GetCoords();
				if (auto pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pThis->GetCell()->LandType, loc)) {
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, loc),
						pThis->Owner, pVictim->GetOwningHouse(), pThis, false);
				}
			}
		}
	}

	return 0;// continue crush function
}

DEFINE_HOOK(0x735584, UnitClass_CTOR_TurretROT, 6)
{
	GET(UnitTypeClass*, pType, ECX);
	R->EDX(TechnoTypeExtContainer::Instance.Find(pType)->TurretRot.Get(pType->ROT));
	return 0x73558A;
}

DEFINE_HOOK(0x413ffa , AircraftClass_Init_TurretROT , 6)
{
	GET(AircraftTypeClass*, pType, EDX);
	R->EAX(TechnoTypeExtContainer::Instance.Find(pType)->TurretRot.Get(pType->ROT));
	return 0x414000;
}

DEFINE_HOOK(0x728EF0, TunnelLocomotionClass_ILocomotion_Process_Dig, 5)
{
	GET(FootClass*, pFoot, EAX);

	TechnoExt_ExtData::HandleTunnelLocoStuffs(pFoot, true, true);
	return 0x728F74;
}

DEFINE_HOOK(0x7292CF, TunnelLocomotionClass_sub_7291F0_Dig, 8)
{
	GET(RepeatableTimerStruct*, pTimer, EDX);
	GET(TunnelLocomotionClass*, pThis, ESI);
	GET(int, nTimeLeft, EAX);

	pTimer->Start(nTimeLeft);
	TechnoExt_ExtData::HandleTunnelLocoStuffs(pThis->LinkedTo, true, true);
	return 0x729365;
}

DEFINE_HOOK(0x7293DA, TunnelLocomotionClass_sub_729370_Dig, 6)
{
	GET(FootClass*, pFoot, ECX);

	TechnoExt_ExtData::HandleTunnelLocoStuffs(pFoot, true, true);
	return 0x72945E;
}

DEFINE_HOOK(0x7297C4, TunnelLocomotionClass_sub_729580_Dig, 6)
{
	GET(FootClass*, pFoot, EAX);

	TechnoExt_ExtData::HandleTunnelLocoStuffs(pFoot, false, false);
	return 0x7297F3;
}

DEFINE_HOOK(0x7299A9, TunnelLocomotionClass_sub_7298F0_Dig, 5)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	TechnoExt_ExtData::HandleTunnelLocoStuffs(pThis->LinkedTo, false, true);
	return 0x729A34;
}

DEFINE_HOOK(0x72920C, TunnelLocomotionClass_Turning, 9)
{
	GET(TunnelLocomotionClass*, pThis, ESI);

	if (pThis->_CoordsNow.IsValid())
		return 0;

	pThis->State = TunnelLocomotionClass::State::DUG_OUT;
	return 0x729369;
}

// select the most appropriate firing voice and also account
// for undefined flags, so you actually won't lose functionality
// when a unit becomes elite.
DEFINE_HOOK(0x7090A8, TechnoClass_SelectFiringVoice, 5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, ECX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoTypeExtData* pData = TechnoTypeExtContainer::Instance.Find(pType);

	int idxVoice = -1;

	int idxWeapon = pThis->SelectWeapon(pTarget);
	WeaponTypeClass* pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;

	// repair
	if (pWeapon && pWeapon->Damage < 0)
	{
		idxVoice = pData->VoiceRepair;
		if (idxVoice < 0)
		{
			if ((IS_SAME_STR_(pType->ID, GameStrings::FV())))
			{
				idxVoice = RulesClass::Instance->VoiceIFVRepair;
			}
		}
	}

	// don't mix them up, but fall back to rookie voice if there
	// is no elite voice.
	if (idxVoice < 0)
	{
		if (idxWeapon)
		{
			// secondary
			if (pThis->Veterancy.IsElite())
			{
				idxVoice = pType->VoiceSecondaryEliteWeaponAttack;
			}

			if (idxVoice < 0)
			{
				idxVoice = pType->VoiceSecondaryWeaponAttack;
			}
		}
		else
		{
			// primary
			if (pThis->Veterancy.IsElite())
			{
				idxVoice = pType->VoicePrimaryEliteWeaponAttack;
			}

			if (idxVoice < 0)
			{
				idxVoice = pType->VoicePrimaryWeaponAttack;
			}
		}
	}

	// generic attack voice
	if (idxVoice < 0 && pType->VoiceAttack.Count)
	{
		unsigned int idxRandom = Random2Class::Global->Random();
		idxVoice = pType->VoiceAttack.Items[idxRandom % pType->VoiceAttack.Count];
	}

	// play voice
	if (idxVoice > -1)
	{
		pThis->QueueVoice(idxVoice);
	}

	return 0x7091C5;
}

// #908369, #1100953: units are still deployable when warping or falling
DEFINE_HOOK(0x700E47, TechnoClass_CanDeploySlashUnload_Immobile, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	const CellClass* pCell = pThis->GetCell();
	const CoordStruct crd = pCell->GetCoordsWithBridge();

	// recreate replaced check, and also disallow if unit is still warping or dropping in.
	return TechnoExtContainer::Instance.Find(pThis)->Convert_Deploy_Delay.InProgress()
	|| pThis->IsUnderEMP()
	|| pThis->IsWarpingIn()
	|| pThis->IsFallingDown
	|| TechnoExtContainer::Instance.Find(pThis)->Is_DriverKilled
	 ? 0x700DCE : 0x700E59;
}

DEFINE_HOOK(0x7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x44, -0xC));
	return !WarheadTypeExtContainer::Instance.Find(pWH)->Malicious || WarheadTypeExtContainer::Instance.Find(pWH)->Nonprovocative ? 0x738535u : 0u;
}

DEFINE_HOOK(0x736135, UnitClass_Update_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);

	// don't sparkle on EMP, Operator, ....
	return TechnoExt_ExtData::IsPowered(pThis)
		? 0x7361A9 : 0;
}

// merge two small visceroids into one large visceroid
DEFINE_HOOK(0x739F21, UnitClass_UpdatePosition_Visceroid, 6)
{
	GET(UnitClass*, pThis, EBP);

	if (!pThis->Type->SmallVisceroid
		 || TechnoExtContainer::Instance.Find(pThis)->MergePreventionTimer.InProgress()
		 || !pThis->Destination
		 ||  pThis->Destination->WhatAmI() != UnitClass::AbsID)
		return 0x0;

	const auto pThis_Large = TechnoTypeExtContainer::Instance.Find(pThis->Type)->LargeVisceroid.Get(RulesClass::Instance->LargeVisceroid);

	if(!pThis_Large
		|| pThis_Large->Strength <= 0
		|| !TechnoExt_ExtData::IsUnitAlive(pThis))
		return 0x0;

	UnitClass* pDest = static_cast<UnitClass*>(pThis->Destination);
	if(!pDest->Type->SmallVisceroid || TechnoExtContainer::Instance.Find(pDest)->MergePreventionTimer.InProgress())
		return 0x0;

	if(pThis->Owner != pDest->Owner)
		return 0x0;

	const auto pDest_Large = TechnoTypeExtContainer::Instance.Find(pDest->Type)->LargeVisceroid.Get(RulesClass::Instance->LargeVisceroid);
	if(pDest_Large != pThis_Large)
		return 0x0;

	// fleshbag erotic
	if (TechnoExt_ExtData::IsUnitAlive(pDest))
	{
		// nice to meat you!
		if (CellClass::Coord2Cell(pThis->GetCoords()) == CellClass::Coord2Cell(pDest->GetCoords()))
		{
			// two become one
			TechnoExt_ExtData::ConvertToType(pDest , pThis_Large , false);
			pDest->IsSelected = pThis->IsSelected;
			pDest->Override_Mission(pThis->IsArmed() ? Mission::Hunt : Mission::Area_Guard);
			pThis->Limbo();
			CellClass* pCell = MapClass::Instance->GetCellAt(pDest->LastMapCoords);
			pDest->UpdateThreatInCell(pCell);
			Debug::Log(__FUNCTION__" Executed!\n");
			TechnoExtData::HandleRemove(pThis, nullptr, false, false);
			return 0x73B0A5;
		}
	}

	return 0;
}

DEFINE_HOOK(0x73C141, UnitClass_DrawVXL_Deactivated, 7)
{
	GET(UnitClass*, pThis, EBP);
	REF_STACK(int, Value, 0x1E0);

	const auto pRules = RulesExtData::Instance();
	double factor = 1.0;

	if (pThis->IsUnderEMP())
	{
		factor = pRules->DeactivateDim_EMP;
	}
	else if (pThis->IsDeactivated())
	{
		// use the operator check because it is more
		// efficient than the powered check.
		if (TechnoExt_ExtData::IsOperatedB(pThis))
		{
			factor = pRules->DeactivateDim_Powered;
		}
		else
		{
			factor = pRules->DeactivateDim_Operator;
		}
	}

	Value = int(Value * factor);

	return 0x73C15F;
}
