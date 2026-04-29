#include "Body.h"

#include <New/Entity/FlyingStrings.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Infantry/Body.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <GameOptionsClass.h>
#include <BitFont.h>
#include <UnitClass.h>

ASMJIT_PATCH(0x441553, BuildingClass_Unlimbo_PowerPlantEnhancer, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	pExt->PowerPlantEnhancer.Register();

	return 0;
}

ASMJIT_PATCH(0x448A78, BuildingClass_SetOwningHouse_RemovePowerPlantEnhancer, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	//GET(HouseClass*, pOldOwner, EBX);
	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	// We need to get the new owner too — depends on where in the function we hook
	// For the Remove hook, we just unregister from old
	pExt->PowerPlantEnhancer.Unregister();

	return 0;
}

ASMJIT_PATCH(0x449197, BuildingClass_SetOwningHouse_AddPowerPlantEnhancer, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	//GET(HouseClass*, pNewOwner, EBP);
	auto pExt = BuildingExtContainer::Instance.Find(pThis);

	// Re-register (AttachedToObject->Owner should now be pNewOwner)
	pExt->PowerPlantEnhancer.Register();

	return 0;
}

ASMJIT_PATCH(0x44955D, BuildingClass_WeaponFactoryOutsideBusy_WeaponFactoryCell, 0x6)
{
	enum { NotBusy = 0x44969B };

	GET(BuildingClass* const, pThis, ESI);

	const auto pLink = pThis->GetNthLink();

	if (!pLink)
		return NotBusy;

	const auto pLinkType = GET_TECHNOTYPE(pLink);

	if (pLinkType->JumpJet && pLinkType->BalloonHover)
		return NotBusy;

	return 0;
}

ASMJIT_PATCH(0x445B62, BuildingClass_Limbo_WallTower_AdjacentWallDamage, 0x5)
{
	enum { SkipGameCode = 0x445B6E };

	GET(CellClass*, pThis, EDI);
	pThis->ReduceWall(200);

	if (pThis->OverlayTypeIndex == -1)
		TechnoClass::ClearWhoTargetingThis(pThis);

	return SkipGameCode;
}

ASMJIT_PATCH(0x4400F9, BuildingClass_AI_UpdateOverpower, 0x6)
{
	enum { SkipGameCode = 0x44019D };

	GET(FakeBuildingClass*, pThis, ESI);

	pThis->_GetExtData()->IsFiringNow = false;

	if (!pThis->Type->Overpowerable)
		return SkipGameCode;

	int overPower = 0;

	for (int idx = pThis->Overpowerers.Count - 1; idx >= 0; idx--)
	{
		const auto pCharger = pThis->Overpowerers[idx];

		if (pCharger->Target != pThis)
		{
			pThis->Overpowerers.erase_at(idx);
			continue;
		}

		const auto pWeapon = pCharger->GetWeapon(1)->WeaponType;

		if (!pWeapon || !pWeapon->Warhead || !pWeapon->Warhead->ElectricAssault)
		{
			pThis->Overpowerers.erase_at(idx);
			continue;
		}

		const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
		overPower += pWHExt->ElectricAssaultLevel;
	}

	const int charge = pThis->_GetTypeExtData()->Overpower_ChargeWeapon;

	pThis->IsOverpowered = overPower >= pThis->_GetTypeExtData()->Overpower_KeepOnline + charge
		|| (pThis->Owner->GetPowerPercentage() == 1.0 && pThis->HasPower && overPower >= charge);
	return SkipGameCode;
}

ASMJIT_PATCH(0x4555E4, BuildingClass_IsPowerOnline_Overpower, 0x6)
{
	enum { LowPower = 0x4556BE, Continue1 = 0x4555F0, Continue2 = 0x455643 };

	GET(FakeBuildingClass*, pThis, ESI);
	GET(const int, threshold, EDI);

		// Battery.KeepOnline activated
	if (!threshold)
		return R->Origin() == 0x4555E4 ? Continue1 : Continue2;

	if(pThis->_GetTypeExtData()->Overpower_KeepOnline < 0)
		return LowPower;

	int overPower = 0;

	for (const auto& pCharger : pThis->Overpowerers) {
		const auto pWeapon = pCharger->GetWeapon(1)->WeaponType;

		if (pWeapon && pWeapon->Warhead) {
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			overPower += pWHExt->ElectricAssaultLevel;
		}
	}

	return overPower < pThis->_GetTypeExtData()->Overpower_KeepOnline ? LowPower : (R->Origin() == 0x4555E4 ? Continue1 : Continue2);
}ASMJIT_PATCH_AGAIN(0x45563B, BuildingClass_IsPowerOnline_Overpower, 0x6)

ASMJIT_PATCH(0x43FBEF, BuildingClass_AI_PoweredKillSpawns, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	BuildingExtContainer::Instance.Find(pThis)->UpdatePoweredKillSpawns();

	return 0;
}

ASMJIT_PATCH(0x483D8E, CellClass_CheckPassability_DestroyableObstacle, 0x6)
{
	enum { IsBlockage = 0x483CD4 };

	GET(FakeBuildingClass*, pBuilding, ESI);

	if (pBuilding->_GetTypeExtData()->IsDestroyableObstacle)
		return IsBlockage;

	return 0;
}

ASMJIT_PATCH(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(FakeBuildingClass*, pThis, ESI);

	return pThis->_GetTypeExtData()->SellBuildupLength == pThis->Animation.Stage
		? Continue : Skip;
}


ASMJIT_PATCH(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	BuildingExtContainer::Instance.Find(pStructure)->DeployedTechno = true;

	return 0;
}

ASMJIT_PATCH(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x6) //was 0
{
	GET(FakeBuildingClass*, pThis, ESI);

	Mission nMission = Mission::Guard;
	if (pThis->_GetExtData()->DeployedTechno && pThis->LastTarget) {
		pThis->SetTarget(pThis->LastTarget);
		nMission = Mission::Attack;
	}

	pThis->QueueMission(nMission, false);
	return 0x449AE8;
}

ASMJIT_PATCH(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
    enum { ContinueCheck = 0x440B58, ShouldNotRebuild = 0x440B81 };
	GET(FakeBuildingClass* const, pThis, ESI);

	if (pThis->_GetTypeExtData()->NewEvaVoice)
		pThis->_GetExtData()->UpdateMainEvaVoice();

	if(SessionClass::IsCampaign())
	{
		if(!pThis->BeingProduced)
			return ShouldNotRebuild;

		// Preplaced structures are already managed before
		if (pThis->_GetExtData()->IsCreatedFromMapFile)
			return ShouldNotRebuild;

		if (!HouseExtContainer::Instance.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty].Get(RulesExtData::Instance()->RepairBaseNodes))
		return ShouldNotRebuild;
	}

	// Vanilla instruction: always repairable in other game modes
	return ContinueCheck;
}

ASMJIT_PATCH(0x465D40, BuildingTypeClass_IsUndeployable_ConsideredVehicle, 0x6)
{
	enum { ReturnFromFunction = 0x465D6A , Continue = 0x0 };

	GET(FakeBuildingTypeClass*, pThis, ECX);

	const auto pBldExt = pThis->_GetExtData();
	const bool IsCustomEligible = pThis->Foundation == BuildingTypeExtData::CustomFoundation
			&& pBldExt->CustomHeight == 1 && pBldExt->CustomWidth == 1;

	const bool FoundationEligible = IsCustomEligible || pThis->Foundation == Foundation::_1x1;

	R->EAX(pBldExt->ConsideredVehicle.Get(pThis->UndeploysInto && FoundationEligible));
	return ReturnFromFunction;
}

ASMJIT_PATCH(0x445FD6, BuildingTypeClass_GrandOpening_StorageActiveAnimations, 0x6)
{
	GET(FakeBuildingClass*, pBuilding, EBP);

	const auto pTypeExt = pBuilding->_GetTypeExtData();

	if (pTypeExt->Storage_ActiveAnimations.Get(pBuilding->Type->Refinery || pBuilding->Type->Weeder))
	{
		R->EAX(pBuilding->Type->Weeder ?
			int(4 * pBuilding->Owner->OwnedWeed.GetTotalAmount() / RulesClass::Instance->WeedCapacity) :
			int(4 * TechnoExtContainer::Instance.Find(pBuilding)->TiberiumStorage.GetAmounts() / pBuilding->Type->Storage)
		);
		return 0x446016;
	}

	return 0x446183;
}

#include <Ext/SWType/Body.h>

ASMJIT_PATCH(0x450D9C, BuildingClass_AI_Anims_IncludeWeeder_1, 0x6)
{
	GET(FakeBuildingClass*, pThis, ESI);

	const auto pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->Storage_ActiveAnimations.Get(pThis->Type->Refinery || pThis->Type->Weeder))
	{
		R->EAX(pThis->Type->Weeder ?
			int(4 * pThis->Owner->OwnedWeed.GetTotalAmount() / RulesClass::Instance->WeedCapacity) :
			int(4 * TechnoExtContainer::Instance.Find(pThis)->TiberiumStorage.GetAmounts() / pThis->Type->Storage)
		);

		return 0x450DDC;
	}

	const auto pSuper = BuildingExtData::GetFirstSuperWeapon(pThis);

	if (!pSuper)
		return 0x451145;

	const auto miss = pThis->GetCurrentMission();
	if (miss == Mission::Construction || miss == Mission::Selling || pThis->Type->ChargedAnimTime > 990.0)
		return 0x451145;

	R->EDI(pThis->Type);
	// Do not advance SuperAnim for buildings with superweapons if the recharge timer hasn't actually started at any point yet.
	if (pSuper->RechargeTimer.StartTime == 0
		&& pSuper->RechargeTimer.TimeLeft == 0
		&& !SWTypeExtContainer::Instance.Find(pSuper->Type)->SW_InitialReady)
		return 0x451048;


	R->EAX(pSuper);
	return 0x451030;
}

ASMJIT_PATCH(0x44EFD8, BuildingClass_FindExitCell_BarracksExitCell, 0x6)
{
	enum { SkipGameCode = 0x44F13B, ReturnFromFunction = 0x44F037 };

	GET(FakeBuildingClass*, pThis, EBX);
	GET(TechnoClass*, pTechno, ESI);
	REF_STACK(CellStruct, resultCell, STACK_OFFSET(0x30, -0x20));

	auto const pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->BarracksExitCell.isset()) {

		const auto exitCell = pThis->GetMapCoords() + CellStruct {
			(short)pTypeExt->BarracksExitCell->X, (short)pTypeExt->BarracksExitCell->Y
		};

		if (MapClass::Instance->CoordinatesLegal(exitCell))
		{
			if (pTechno->IsCellOccupied(MapClass::Instance->GetCellAt(exitCell),
				FacingType::None,
				-1,
				nullptr,
				true)
				== Move::OK) {
				resultCell = exitCell;
				return ReturnFromFunction;
			}

		}

		return SkipGameCode;
	}

	return 0;
}

#pragma region EnableBuildingProductionQueue

ASMJIT_PATCH(0x4FA520, HouseClass_BeginProduction_SkipBuilding, 0x5)
{
	enum { SkipGameCode = 0x4FA553 };
	return RulesExtData::Instance()->ExpandBuildingQueue ? SkipGameCode : 0;
}

ASMJIT_PATCH(0x4FAAD8, HouseClass_AbandonProduction_RewriteForBuilding, 0x8)
{
	enum { CheckSame = 0x4FAB3D, SkipCheck = 0x4FAB64, Return = 0x4FAC9B };

	GET_STACK(const bool, all, STACK_OFFSET(0x18, 0x10));
	GET(const int, index, EBX);
	GET(const BuildCat, buildCat, ECX);
	GET(const AbstractType, absType, EBP);
	GET(FactoryClass* const, pFactory, ESI);

	// After placing the building, the factory will be in this state
	if (buildCat != BuildCat::DontCare && !all && !pFactory->Object)
		return SkipCheck;

	const auto pType = TechnoTypeClass::GetByTypeAndIndex(absType, index);
	const auto firstRemoved = pFactory->RemoveOneFromQueue(pType);

	if (firstRemoved)
	{
		SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true; // Added, force redraw strip
		SidebarClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(absType, index, 0));

		if (all)
			while (pFactory->RemoveOneFromQueue(pType));
		else
			return Return;
	}

	return CheckSame;
}

ASMJIT_PATCH(0x4FA612, HouseClass_BeginProduction_ForceRedrawStrip, 0x5)
{
	SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true;
	return 0;
}

ASMJIT_PATCH(0x6AA88D, StripClass_RecheckCameo_FindFactoryDehardCode, 0x6)
{
	GET(TechnoTypeClass* const, pType, EBX);
	LEA_STACK(BuildCat*, pBuildCat, STACK_OFFSET(0x158, -0x158));

	if (const auto pBuildingType = type_cast<BuildingTypeClass*>(pType))
		*pBuildCat = pBuildingType->BuildCat;

	return 0;
}

#pragma endregion
#include <PlanningTokenClass.h>

ASMJIT_PATCH(0x4AE95E, DisplayClass_sub_4AE750_AntiStupid, 0x5)
{
	enum { Ret = 0x4AE982 };

	GET(ObjectClass*, pObject, ECX);
	//GET(int, address, ESP);
	LEA_STACK(CellStruct*, pCell, 0x28);

	auto action = pObject->MouseOverCell(*pCell);

	bool shouldSkip = PlanningNodeClass::PlanningModeActive.get() && pObject->WhatAmI() == AbstractType::Building && action != Action::Attack;

	if (!shouldSkip)
		pObject->CellClickedAction(action, pCell, pCell, false);

	return Ret;
}

ASMJIT_PATCH(0x44A541, BuildingClass_LeaveBioReactor, 0x7)
{
	GET(FootClass*, pFoot, ESI);
	pFoot->Transporter = nullptr;
	return 0;
}ASMJIT_PATCH_AGAIN(0x442F9B, BuildingClass_LeaveBioReactor, 0x6)

static void KickOutStuckUnits(BuildingClass* pThis)
{
	auto buffer = CoordStruct::Empty;
	pThis->GetExitCoords(&buffer, 0);

	auto cell = CellClass::Coord2Cell(buffer);

	const auto pType = pThis->Type;
	const short start = static_cast<short>(pThis->Location.X / Unsorted::LeptonsPerCell + pType->GetFoundationWidth() - 2); // door
	const short end = cell.X; // exit
	cell.X = start;
	auto pCell = MapClass::Instance->GetCellAt(cell);

	while (true)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pUnit = cast_to<UnitClass*, true>(pObject))
			{
				if (pThis->Owner != pUnit->Owner || pUnit->Locomotor->Destination() != CoordStruct::Empty)
					continue;

				const auto height = pUnit->GetHeight();

				if (height < 0 || height > Unsorted::CellHeight)
					continue;

				pThis->SendCommand(RadioCommand::RequestLink, pUnit);
				pThis->QueueMission(Mission::Unload, false);
				return; // one after another
			}
		}

		if (--cell.X < end)
			return; // no stuck

		pCell = MapClass::Instance->GetCellAt(cell);
	}
}

// Attempt to kick the stuck unit out again by setting the destination
ASMJIT_PATCH(0x44E202, BuildingClass_Mission_Unload_CheckStuck, 0x6)
{
	enum { Waiting = 0x44E267, NextStatus = 0x44E20C};

	GET(BuildingClass*, pThis, EBP);

	if (!pThis->IsTethered)
		return NextStatus;

	if (const auto pUnit = cast_to<UnitClass*>(pThis->GetNthLink()))
	{
		// Detecting movement status
		if (pUnit->Locomotor->Destination() == CoordStruct::Empty)
		{
			// Evacuate the congestion at the entrance
			pThis->ClearFactoryBib();
			const auto pType = pThis->Type;
			const auto cell = pThis->GetMapCoords() + pType->FoundationOutside[10];
			const auto door = cell - CellStruct { 1, 0 };
			const auto pDest = MapClass::Instance->GetCellAt(door);

			// Hover units may stop one cell behind their destination, should forcing them to advance one more cell
			pUnit->SetDestination((pUnit->Destination != pDest ? pDest : MapClass::Instance->GetCellAt(cell)), true);
		}
	}

	return Waiting;
}

ASMJIT_PATCH(0x73F5A7, UnitClass_IsCellOccupied_UnlimboDirection, 0x8)
{
	enum { NextObject = 0x73FA87, ContinueCheck = 0x73F5AF };

	GET(const bool, notPassable, EAX);

	if (notPassable)
		return ContinueCheck;

	GET(BuildingClass* const, pBuilding, ESI);

	const auto pType = pBuilding->Type;

	if (!pType->WeaponsFactory)
		return NextObject;

	GET(CellClass* const, pCell, EDI);

	return pCell->MapCoords.Y == pBuilding->Location.Y / Unsorted::LeptonsPerCell + pType->FoundationOutside[10].Y
		? NextObject : ContinueCheck;
}

// Check for any stuck units inside after successful unload each time. If there is, kick it out
 ASMJIT_PATCH(0x44E260, BuildingClass_Mission_Unload_KickOutStuckUnits, 0x7)
 {
 	GET(BuildingClass*, pThis, EBP);

 	KickOutStuckUnits(pThis);

 	return 0;
 }

ASMJIT_PATCH(0x449306, BuildingClass_SetOwningHouse_Sell, 0x6)
{
	enum { NoSell = 0x44936E };
	GET(FakeBuildingClass*, pThis, ESI);
	return pThis->_GetTypeExtData()->AISellCapturedBuilding
			.Get(RulesExtData::Instance()->AISellCapturedBuilding) ? 0 : NoSell;
}

ASMJIT_PATCH(0x4485DB, BuildingClass_SetOwningHouse_SyncLinkedOwner, 0x6)
{
	enum { SkipGameCode = 0x4486C8 };
	GET(FakeBuildingClass*, pThis, ESI);
	return pThis->_GetTypeExtData()->BuildingRadioLink_SyncOwner
			.Get(RulesExtData::Instance()->BuildingRadioLink_SyncOwner) ? 0 : SkipGameCode;
}

#pragma region PrefiringMark

ASMJIT_PATCH(0x440045, BuildingClass_UpdateDelayedFiring_PrefiringMark1, 0x6)
{
	GET(FakeBuildingClass*, pThis, ESI);
	pThis->_GetExtData()->IsFiringNow = (int)pThis->PrismStage && pThis->DelayBeforeFiring <= 1;
	return 0;
}

#pragma endregion

#pragma region TurretAnim

DEFINE_HOOK(0x451242, BuildingClass_AnimationAI_TurretAnim, 0xA)
{
	enum { SkipGameCode = 0x451296 };

	GET(FakeBuildingClass*, pThis, ESI);

	if (auto const pAnim = pThis->Anims[(int)BuildingAnimSlot::Turret]) {
		pAnim->Animation.Stage = BuildingExtData::GetTurretFrame(pThis);
		pAnim->Animation.Step = 0;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x44B6C7, BuildingClass_Mission_Attack_TurretAnim, 0x6)
{
	enum { SkipFiring = 0x44B6FE };

	GET(FakeBuildingClass*, pThis, ESI);

	if (pThis->HasTurret()) {
		if (auto const pAnim = pThis->Anims[(int)BuildingAnimSlot::Turret]) {
			auto pExt = pThis->_GetExtData();
			const auto pTypeExt = pExt->GetTypeExtData();
			const bool isLowPower = !pThis->StuffEnabled || !pThis->IsPowerOnline();
			const auto firingFrames = (isLowPower ? &pTypeExt->TurretAnim_LowPowerFiringFrames : &pTypeExt->TurretAnim_FiringFrames);

			if (firingFrames->Get() > 0 && pExt->TurretAnimFiringFrame == -1)
				pExt->TurretAnimFiringFrame = 0;
		}
	}

	return 0;
}

#pragma endregion

ASMJIT_PATCH(0x453E02, BuildingClass_Clear_Occupy_Spot_Skip, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(CellClass*, pCell, EAX);

	ObjectClass* pObject = pCell->FirstObject;

	do
	{
		if (pObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Building:
			{
				if (pObject != pTechno)
				{
					// skip change the OccFlag of this cell
					return 0x453E12;
				}
				break;
			}
			}
		}
	}
	while (pObject && (pObject = pObject->NextObject) != nullptr);

	return 0;
}

ASMJIT_PATCH(0x44DBBC, BuildingClass_Mission_Unload_Leave_Bio_Readtor_Sound, 0x7)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(FootClass* const, pPassenger, ESI);
	LEA_STACK(CoordStruct*, pBuffer, 0x40);

	int sound = GET_TECHNOTYPE(pPassenger)->LeaveBioReactorSound;

	if (sound == -1)
		sound = RulesClass::Instance->LeaveBioReactorSound;

	auto coord = pThis->GetCoords(pBuffer);
	VocClass::SafeImmedietelyPlayAt(sound, coord, 0);
	return 0x44DBDA;
}

ASMJIT_PATCH(0x447E90, BuildingClass_GetDestinationCoord_Helipad, 0x6)
{
	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(CoordStruct*, pCoord, 0x4);
	GET_STACK(TechnoClass* const, pDocker, 0x8);

	auto const pType = pThis->Type;
	if (pType->Helipad)
	{
		pThis->GetDockCoords(pCoord, pDocker);
		pCoord->Z = 0;
	}
	else if (pType->UnitRepair || pType->Bunker)
	{
		pThis->GetDockCoords(pCoord, pDocker);
	}
	else
	{
		pThis->GetCoords(pCoord);
	}

	R->EAX(pCoord);
	return 0x447F06;
}

ASMJIT_PATCH(0x442CCF, BuildingClass_Init_Sellable, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	pThis->IsAllowedToSell = !pThis->Type->Unsellable;
	return 0x0;
}

ASMJIT_PATCH(0x458291, BuildingClass_GarrisonAI_AbandonedSound, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	const auto nVal = pExt->AbandonedSound.Get(RulesClass::Instance->BuildingAbandonedSound);
	if (nVal >= 0)
	{
		VocClass::PlayGlobal(nVal, Panning::Center, 1.0, 0);
	}

	return 0x4582AE;
}

ASMJIT_PATCH(0x4431D3, BuildingClass_Destroyed_removeLog, 0x5)
{
	GET(InfantryClass*, pSurvivor, ESI);
	GET_STACK(int, nData, 0x8C - 0x70);
	Debug::Log("Survivor[(%x - %s) - %s] unlimbo OK\n", pSurvivor, pSurvivor->Type->ID, pSurvivor->Owner->Type->ID);

	R->EBP(--nData);
	R->EDX(pSurvivor->Type);
	return 0x4431EB;
}

ASMJIT_PATCH(0x443292, BuildingClass_Destroyed_CreateSmudge_A, 0x6)
{
	GET(BuildingClass*, pThis, EDI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4433F9;
}

ASMJIT_PATCH(0x44177E, BuildingClass_Destroyed_CreateSmudge_B, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->Destroyed_CreateSmudge
		? 0x0 : 0x4418EC;
}

ASMJIT_PATCH(0x447110, BuildingClass_Sell_Handled, 0x9)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(int, control, 0x4);

	// #754 - evict Hospital/Armory contents
	TechnoExtData::KickOutHospitalArmory(pThis);

	BuildingExtContainer::Instance.Find(pThis)->MyPrismForwarding->RemoveFromNetwork(true);

	if (pThis->HasBuildup)
	{

		switch (control)
		{
		case -1:
		{
			if (pThis->GetCurrentMission() != Mission::Selling)
			{

				pThis->QueueMission(Mission::Selling, false);
				pThis->NextMission();
			}

			break;
		}
		case 0:
		{
			if (pThis->GetCurrentMission() != Mission::Selling)
			{
				return 0x04471C2;
			}

			break;
		}
		case 1:
		{
			if (pThis->GetCurrentMission() != Mission::Selling && !pThis->IsGoingToBlow)
			{
				pThis->QueueMission(Mission::Selling, false);
				pThis->NextMission();
			}

			break;
		}
		default:
			break;
		}

		if (!BuildingExtContainer::Instance.Find(pThis)->Silent)
		{
			if (pThis->Owner->ControlledByCurrentPlayer())
			{
				VocClass::PlayGlobal(RulesClass::Instance->GenericClick, Panning::Center, 1.0, 0);
			}
		}

		return 0x04471C2;
	}
	else
	{
		if (pThis->Type->FirestormWall || BuildingTypeExtContainer::Instance.Find(pThis->Type)->Firestorm_Wall)
		{
			//if(const auto pBomb = pThis->AttachedBomb) {
			//	if (BombExtContainer::Instance.Find(pBomb)->Weapon->Ivan_DetonateOnSell.Get()){
			//		pBomb->Detonate();// Otamaa : detonate may kill the techno before this function
			//		// so this can possibly causing some weird crashes if that happening
			//	}
			//}

			pThis->Limbo();
			pThis->UnInit();
		}
	}

	return 0x04471C2;
}

ASMJIT_PATCH(0x449462, BuildingClass_IsCellOccupied_UndeploysInto, 0x6)
{
	enum { PlacingCheck = 0x449493, SkipGameCode = 0x449487 };

	GET(BuildingClass*, pThis, ECX);

	if (pThis->CurrentMission == Mission::None)
		return PlacingCheck;

	GET(BuildingTypeClass*, pType, EAX);
	LEA_STACK(CellStruct*, pDest, 0x4);

	const auto pUndeploysInto = pType->UndeploysInto;
	R->AL(MapClass::Instance->GetCellAt(pDest)
		->IsClearToMove(pUndeploysInto->SpeedType, 0, 0, ZoneType::None, pUndeploysInto->MovementZone, -1, 1)
	);

	return SkipGameCode;
}

ASMJIT_PATCH(0x449E8E, BuildingClass_Mission_Selling_UndeployLocationFix, 0x5)
{
	GET(BuildingClass*, pThis, EBP);
	CellStruct mapCoords = pThis->InlineMapCoords();

	const short width = pThis->Type->GetFoundationWidth();
	const short height = pThis->Type->GetFoundationHeight(false);

	if (width > 2)
		mapCoords.X += static_cast<short>(std::ceil(width / 2.0) - 1);
	if (height > 2)
		mapCoords.Y += static_cast<short>(std::ceil(height / 2.0) - 1);

	REF_STACK(CoordStruct, location, STACK_OFFSET(0xD0, -0xC0));
	auto coords = (CoordStruct*)&location.Z;
	coords->X = (mapCoords.X << 8) + 128;
	coords->Y = (mapCoords.Y << 8) + 128;
	coords->Z = pThis->Location.Z;

	return 0x449F12;
}

ASMJIT_PATCH(0x4419A9, BuildingClass_Destroy_ExplodeAnim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, X, ECX);
	GET(int, Y, EDX);
	GET(int, Z, EAX);
	GET(int, zAdd, EDI);

	CoordStruct nLoc { X , Y , Z + zAdd };
	const int idx = pThis->Type->Explosion.Count == 1 ?
		0 : ScenarioClass::Instance->Random.RandomFromMax(pThis->Type->Explosion.Count - 1);

	if (auto const pType = pThis->Type->Explosion.Items[idx])
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomFromMax(3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, nLoc, nDelay, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	R->Stack(0x20, nLoc.X);
	R->Stack(0x24, nLoc.Y);
	R->Stack(0x28, nLoc.Z);
	return 0x441A24;
}

ASMJIT_PATCH(0x441AC4, BuildingClass_Destroy_Fire3Anim, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	LEA_STACK(CoordStruct*, pCoord, 0x64 - 0x54);

	if (auto pType = RulesExtData::Instance()->DefaultExplodeFireAnim)
	{
		const auto nDelay = ScenarioClass::Instance->Random.RandomRanged(1, 3);
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pCoord, nDelay + 3, 1, AnimFlag::AnimFlag_600, 0, false),
			pThis->GetOwningHouse(),
			nullptr,
			false
		);
	}

	return 0x441B1F;
}

ASMJIT_PATCH(0x441D1F, BuildingClass_Destroy_DestroyAnim, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
	return 0x0;
}

ASMJIT_PATCH(0x450B48, BuildingClass_Anim_AI_UnitAbsorb, 0x6)
{
	GET(BuildingTypeClass*, pThis, EAX);
	R->CL(pThis->InfantryAbsorb || pThis->UnitAbsorb);
	return 0x450B4E;
}

ASMJIT_PATCH(0x4421F2, BuildingClass_Destroyed_PlaceCrate, 0x6)
{
	//GET(BuildingClass*, pThis, ESI);
	GET(BuildingTypeClass*, pThisType, EDX);
	GET_STACK(CellStruct, cell, 0x10);

	const PowerupEffects defaultcrate = pThisType->CrateBeneathIsMoney ? PowerupEffects::Money : (PowerupEffects)CrateTypeClass::Array.size();
	const auto CrateType = &TechnoTypeExtContainer::Instance.Find(pThisType)->Destroyed_CrateType;
	PowerupEffects crate = CrateType->isset() ? (PowerupEffects)CrateType->Get() : defaultcrate;
	R->EAX(MapClass::Instance->Place_Crate(cell, crate));
	return 0x442226;
}

ASMJIT_PATCH(0x451932, BuildingClass_AnimLogic_Ownership, 0x5)
{
	GET(BuildingClass*, pThis, ESI);
	GET(int, _X, ECX);
	GET(int, _Y, EDX);
	GET(int, _Z, EAX);
	GET(int, animIdx, EBP);
	GET_STACK(int, delay, 0x48);

	CoordStruct coord { _X , _Y , _Z };
	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(AnimTypeClass::Array->Items[animIdx]);

	auto pAnim = GameCreate<AnimClass>(AnimTypeClass::Array->Items[animIdx], coord, delay, 1, AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_1000, 0, 0);
	if (!pTypeExt->NoOwner)
	{
		((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pThis;
		pAnim->SetHouse(pThis->Owner);
	}

	((FakeAnimClass*)pAnim)->_GetExtData()->ParentBuilding = pThis;
	R->EBP(pAnim);
	return 0x45197B;
}

//BuildingClass_ClearFactory
DEFINE_JUMP(LJMP, 0x4495FF, 0x44961A);
DEFINE_JUMP(LJMP, 0x449657, 0x449672);

//remove unused function
//BuildingClass_Destroy
DEFINE_JUMP(LJMP, 0x4417A7, 0x44180A)

//BuildingClass_Mission_Unload_DisableLog
DEFINE_JUMP(LJMP, 0x44DE2F, 0x44DE3C);