#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <BitFont.h>
#include <New/Entity/FlyingStrings.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

#include <GameOptionsClass.h>

DEFINE_HOOK(0x483D8E, CellClass_CheckPassability_DestroyableObstacle, 0x6)
{
	enum { IsBlockage = 0x483CD4 };

	GET(FakeBuildingClass*, pBuilding, ESI);

	if (pBuilding->_GetTypeExtData()->IsDestroyableObstacle)
		return IsBlockage;

	return 0;
}

DEFINE_HOOK(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
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

DEFINE_HOOK(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(FakeBuildingClass*, pThis, ESI);

	return pThis->_GetTypeExtData()->SellBuildupLength == pThis->Animation.Value
		? Continue : Skip;
}

DEFINE_HOOK(0x739717, UnitClass_TryToDeploy_Transfer, 0x8)
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

//DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
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

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x6) //was 0
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

// DEFINE_HOOK(0x43FE73, BuildingClass_AI_FlyingStrings, 0x6)
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

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 , Continue = 0x0 };

	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x9C, -0x4));

	return  WarheadTypeExtContainer::Instance.Find(args.WH)->AllowDamageOnSelf ? SkipCheck : Continue;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
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

DEFINE_HOOK(0x465D40, BuildingTypeClass_IsUndeployable_ConsideredVehicle, 0x6)
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

DEFINE_HOOK(0x445FD6, BuildingTypeClass_GrandOpening_StorageActiveAnimations, 0x6)
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

DEFINE_HOOK(0x450D9C, BuildingTypeClass_AI_Anims_IncludeWeeder_1, 0x6)
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

// DEFINE_HOOK(0x450E12, BuildingTypeClass_AI_Anims_IncludeWeede_2, 0x7)
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