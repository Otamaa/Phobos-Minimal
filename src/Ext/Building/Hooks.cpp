#include "Body.h"

#include <UnitClass.h>
#include <Utilities/Macro.h>

#include <BitFont.h>
#include <New/Entity/FlyingStrings.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>


DEFINE_HOOK(0x739717, UnitClass_TryToDeploy_Transfer, 0x8)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (R->AL())
	{
		if (pUnit->Type->DeployToFire && pUnit->Target)
			pStructure->LastTarget = pUnit->Target;

		BuildingExt::ExtMap.Find(pStructure)->DeployedTechno = true;

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
//	BuildingExt::ExtMap.Find(pStructure)->DeployedTechno = true;
//
//	return 0;
//}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x6) //was 0
{
	GET(BuildingClass*, pThis, ESI);

	Mission nMission = Mission::Guard;
	if (BuildingExt::ExtMap.Find(pThis)->DeployedTechno && pThis->LastTarget) {
		pThis->SetTarget(pThis->LastTarget);
		nMission = Mission::Attack;
	}

	pThis->QueueMission(nMission, false);
	return 0x449AE8;
}

DEFINE_HOOK(0x43FE73, BuildingClass_AI_FlyingStrings, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (Unsorted::CurrentFrame % 15 != 0)
		return 0;

	auto const pExt = BuildingExt::ExtMap.Find(pThis);
	if (pExt->AccumulatedGrindingRefund) {
		const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
		FlyingStrings::AddMoneyString(true, pExt->AccumulatedGrindingRefund, pThis, AffectedHouse::All, pThis->GetRenderCoords(), pTypeExt->Grinding_DisplayRefund_Offset);
		pExt->AccumulatedGrindingRefund = 0;
	}

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 , Continue = 0x0 };

	REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0x9C, -0x4));

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
	return pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get() ?
	SkipCheck : Continue;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
    enum { ContinueCheck = 0x440B58, ShouldNotRebuild = 0x440B81 };
	GET(BuildingClass* const, pThis, ESI);
	if(SessionClass::IsCampaign())
	{
		if (!pThis->BeingProduced ||
			!HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
		return ShouldNotRebuild;
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x465D40, BuildingTypeClass_IsUndeployable_ConsideredVehicle, 0x6)
{
	enum { ReturnFromFunction = 0x465D6A , Continue = 0x0 };

	GET(BuildingTypeClass*, pThis, ECX);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (pExt->ConsideredVehicle.isset()) {
		R->EAX(pExt->ConsideredVehicle.Get());
		return ReturnFromFunction;
	}

	return Continue;
}

