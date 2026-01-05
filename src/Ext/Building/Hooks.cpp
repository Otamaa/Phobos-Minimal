	#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <BitFont.h>
#include <New/Entity/FlyingStrings.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Cast.h>

#include <GameOptionsClass.h>

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

// ASMJIT_PATCH(0x739717, UnitClass_TryToDeploy_Transfer, 0x8)
// {
// 	GET(UnitClass*, pUnit, EBP);
// 	GET(FakeBuildingClass*, pStructure, EBX);

// 	if (R->AL())
// 	{
// 		if (pUnit->Type->DeployToFire && pUnit->Target)
// 			pStructure->LastTarget = pUnit->Target;

// 		pStructure->_GetExtData()->DeployedTechno = true;

// 		return 0x73971F;
// 	}

// 	return 0x739A6E;
// }

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

// ASMJIT_PATCH(0x43FE73, BuildingClass_AI_FlyingStrings, 0x6)
// {
// 	GET(BuildingClass*, pThis, ESI);
//
// 	if (Unsorted::CurrentFrame % 15 != 0)
// 		return 0;
//
// 	auto const pExt = BuildingExtContainer::Instance.Find(pThis);
// 	if (pExt->AccumulatedGrindingRefund) {
// 		FlyingStrings::AddMoneyString(true,
// 			pExt->AccumulatedGrindingRefund,
// 			pThis, AffectedHouse::All,
// 			pThis->GetRenderCoords(),
// 			pExt->Type->Grinding_DisplayRefund_Offset);
// 		pExt->AccumulatedGrindingRefund = 0;
// 	}
//
// 	return 0;
// }

//ASMJIT_PATCH(0x440580, BuildingClass_Unlimbo_Addition, 0x5)
//{
//	GET(FakeBuildingClass* const, pThis, ESI);
//	FakeHouseClass* pHouse = (FakeHouseClass*)pThis->Owner;
//	pHouse->_GetExtData()->TunnelsBuildings.emplace(pThis);
//	return 0x0;
//}

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

// ASMJIT_PATCH(0x450E12, BuildingTypeClass_AI_Anims_IncludeWeede_2, 0x7)
// {
// 	GET(BuildingClass*, pBuilding, ESI);
//
// 	R->EAX(pBuilding->Type->Weeder ?
// 		int(4 * pBuilding->Owner->OwnedWeed.GetTotalAmount() / RulesClass::Instance->WeedCapacity) :
// 		int(4 * pBuilding->Tiberium.GetTotalAmount() / pBuilding->Type->Storage)
// 	);
//
// 	return 0x450E3E;
// }

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

ASMJIT_PATCH(0x4C9C7B, FactoryClass_QueueProduction_ForceCheckBuilding, 0x7)
{
	enum { SkipGameCode = 0x4C9C9E };
	return RulesExtData::Instance()->ExpandBuildingQueue ? SkipGameCode : 0;
}

#ifdef OLD
ASMJIT_PATCH(0x4FAAD8, HouseClass_AbandonProduction_RewriteForBuilding, 0x8)
{
	enum { CheckSame = 0x4FAB3D, SkipCheck = 0x4FAB64, Return = 0x4FAC9B };

	GET_STACK(const bool, all, STACK_OFFSET(0x18, 0x10));
	GET(const int, index, EBX);
	GET(const BuildCat, buildCat, ECX);
	GET(const AbstractType, absType, EBP);
	GET(FactoryClass* const, pFactory, ESI);

	if (buildCat == BuildCat::DontCare || all)
	{
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

	if (!pFactory->Object)
		return SkipCheck;

	if (!pFactory->RemoveOneFromQueue(TechnoTypeClass::GetByTypeAndIndex(absType, index)))
		return CheckSame;

	SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true; // Added, force redraw strip
	SidebarClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(absType, index, 0));
	return Return;
}
#else 
//DEFINE_JUMP(LJMP, 0x4FABEE, 0x4FAB3D)

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
#endif

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

	bool shouldSkip = PlanningNodeClass::PlanningModeActive && pObject->WhatAmI() == AbstractType::Building && action != Action::Attack;

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

ASMJIT_PATCH(0x440042, BuildingClass_UpdateDelayedFiring_PrefiringMark1, 0x9)
{
	GET(FakeBuildingClass*, pThis, ESI);
	pThis->_GetExtData()->IsFiringNow = (int)pThis->PrismStage && pThis->DelayBeforeFiring <= 1;
	return 0;
}

ASMJIT_PATCH(0x4400F9, BuildingClass_UpdateDelayedFiring_PrefiringMar2, 0x7)
{
	GET(FakeBuildingClass*, pThis, ESI);
	pThis->_GetExtData()->IsFiringNow = false;
	return 0;
}

#pragma endregion