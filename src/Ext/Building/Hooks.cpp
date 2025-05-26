	#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <BitFont.h>
#include <New/Entity/FlyingStrings.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

#include <GameOptionsClass.h>

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
			pThis->Overpowerers.RemoveAt(idx);
			continue;
		}

		const auto pWeapon = pCharger->GetWeapon(1)->WeaponType;

		if (!pWeapon || !pWeapon->Warhead || !pWeapon->Warhead->ElectricAssault)
		{
			pThis->Overpowerers.RemoveAt(idx);
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

ASMJIT_PATCH(0x483D8E, CellClass_CheckPassability_DestroyableObstacle, 0x6)
{
	enum { IsBlockage = 0x483CD4 };

	GET(FakeBuildingClass*, pBuilding, ESI);

	if (pBuilding->_GetTypeExtData()->IsDestroyableObstacle)
		return IsBlockage;

	return 0;
}

ASMJIT_PATCH(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
{
	enum { Apply = 0x43D6EF, Skip = 0x43D712 };

	GET(FakeBuildingClass*, pThis, ESI);
	GET(Mission, mission, EAX);

	if (
		(mission != Mission::Selling && mission != Mission::Construction) ||
			pThis->_GetTypeExtData()->ZShapePointMove_OnBuildup
		)
		return Apply;

	return Skip;
}

ASMJIT_PATCH(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(FakeBuildingClass*, pThis, ESI);

	return pThis->_GetTypeExtData()->SellBuildupLength == pThis->Animation.Stage
		? Continue : Skip;
}

ASMJIT_PATCH(0x739717, UnitClass_TryToDeploy_Transfer, 0x8)
{
	GET(UnitClass*, pUnit, EBP);
	GET(FakeBuildingClass*, pStructure, EBX);

	if (R->AL())
	{
		if (pUnit->Type->DeployToFire && pUnit->Target)
			pStructure->LastTarget = pUnit->Target;

		pStructure->_GetExtData()->DeployedTechno = true;

		return 0x73971F;
	}

	return 0x739A6E;
}

//ASMJIT_PATCH(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
//{
//	GET(UnitClass*, pUnit, EBP);
//	GET(BuildingClass*, pStructure, EBX);
//
//	if (pUnit->Type->DeployToFire && pUnit->Target)
//		pStructure->LastTarget = pUnit->Target;
//
//	BuildingExtContainer::Instance.Find(pStructure)->DeployedTechno = true;
//
//	return 0;
//}

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



ASMJIT_PATCH(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
    enum { ContinueCheck = 0x440B58, ShouldNotRebuild = 0x440B81 };
	GET(FakeBuildingClass* const, pThis, ESI);

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

	R->EAX(pBldExt->Type->ConsideredVehicle.Get(pThis->UndeploysInto && FoundationEligible));
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

ASMJIT_PATCH(0x450D9C, BuildingTypeClass_AI_Anims_IncludeWeeder_1, 0x6)
{
	GET(FakeBuildingClass*, pBuilding, ESI);

	const auto pTypeExt = pBuilding->_GetTypeExtData();

	if (pTypeExt->Storage_ActiveAnimations.Get(pBuilding->Type->Refinery || pBuilding->Type->Weeder))
	{
		R->EAX(pBuilding->Type->Weeder ?
			int(4 * pBuilding->Owner->OwnedWeed.GetTotalAmount() / RulesClass::Instance->WeedCapacity) :
			int(4 * TechnoExtContainer::Instance.Find(pBuilding)->TiberiumStorage.GetAmounts() / pBuilding->Type->Storage)
		);

		return 0x450DDC;
	}

	return 0x450F9E;
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

ASMJIT_PATCH(0x444B83, BuildingClass_ExitObject_BarracksExitCell, 0x7)
{
	enum { SkipGameCode = 0x444C7C };
	GET(FakeBuildingClass*, pThis, ESI);
	GET(int, xCoord, EBP);
	GET(int, yCoord, EDX);
	REF_STACK(CoordStruct, resultCoords, STACK_OFFSET(0x140, -0x108));

	if (pThis->_GetTypeExtData()->BarracksExitCell.isset()) {
		auto const exitCoords = pThis->Type->ExitCoord;
		resultCoords = CoordStruct { xCoord + exitCoords.X, yCoord + exitCoords.Y, exitCoords.Z };
		return SkipGameCode;
	}

	return 0;
}

#pragma region EnableBuildingProductionQueue

ASMJIT_PATCH(0x6AB689, SelectClass_Action_SkipBuildingProductionCheck, 0x5)
{
	enum { SkipGameCode = 0x6AB6CE };
	return RulesExtData::Instance()->ExpandBuildingQueue ? SkipGameCode : 0;
}

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

ASMJIT_PATCH(0x6A9C54, StripClass_DrawStrip_FindFactoryDehardCode, 0x6)
{
	GET(TechnoTypeClass* const, pType, ECX);
	LEA_STACK(BuildCat*, pBuildCat, STACK_OFFSET(0x490, -0x490));

	if (const auto pBuildingType = cast_to<BuildingTypeClass*>(pType))
		*pBuildCat = pBuildingType->BuildCat;

	return 0;
}

ASMJIT_PATCH(0x6A9789, StripClass_DrawStrip_NoGreyCameo, 0x6)
{
	enum { ContinueCheck = 0x6A9799, SkipGameCode = 0x6A97FB };

	GET(TechnoTypeClass* const, pType, EBX);
	GET_STACK(bool, clicked, STACK_OFFSET(0x48C, -0x475));

	if (!RulesExtData::Instance()->ExpandBuildingQueue) {
		if (pType->WhatAmI() == AbstractType::BuildingType && clicked)
			return SkipGameCode;
	}
	else if (const auto pBuildingType = cast_to<BuildingTypeClass*>(pType))
	{
		if (const auto pFactory = HouseClass::CurrentPlayer->GetPrimaryFactory(AbstractType::BuildingType, pType->Naval, pBuildingType->BuildCat))
		{
			if (const auto pProduct = cast_to<BuildingClass*>(pFactory->Object))
			{
				if (pFactory->IsDone() && pProduct->Type != pType && ((pProduct->Type->BuildCat != BuildCat::Combat) ^ (pBuildingType->BuildCat == BuildCat::Combat)))
					return SkipGameCode;
			}
		}
	}

	return ContinueCheck;
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

	if (const auto pBuildingType = cast_to<BuildingTypeClass*>(pType))
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