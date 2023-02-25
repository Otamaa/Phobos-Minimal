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

	if (BuildingExt::ExtMap.Find(pThis)->DeployedTechno && pThis->LastTarget)
	{
		pThis->SetTarget(pThis->LastTarget);
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

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
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0x9C, -0x4));

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH);
	return pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get() ? SkipCheck : 0x0;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
    enum { ContinueCheck = 0x440B58, ShouldNotRebuild = 0x440B81 };
	GET(BuildingClass* const, pThis, ESI);
	if(SessionClass::IsCampaign())
	{
		if (!pThis->BeingProduced ||!HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
		return ShouldNotRebuild;
	}

	return ContinueCheck;
}

/*
DEFINE_HOOK(0x4506D4, BuildingClass_UpdateRepair_Campaign, 0x6)
{
	enum { GoRepair = 0x4506F5, SkipRepair = 0x450813 };
	GET(BuildingClass*, pThis, ESI);
	//GET(HouseClass*, pHouse, ECX);

	if (pThis->BeingProduced && SessionClass::IsCampaign())
		if (HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
			return GoRepair;

	return 0x0;
}*/

// Note:
/*
Ares has a hook at 0x4571E0 (the beginning of BuildingClass::Infiltrate) and completely overwrites the function.
Our logic has to be executed at the end (0x4575A2). The hook there assumes that registers have the exact content
they had in the beginning (when Ares hook started, executed, and jumped) in order to work when Ares logic is used.
However, this will fail if Ares is not involved (either DLL not included or with SpyEffect.Custom=no on BuildingType),
because by the time we reach our hook, the registers will be different and we'll be reading garbage. That's why
there is a second hook at 0x45759D, which is only executed when Ares doesn't jump over this function. There,
we execute our custom logic and then use EAX (which isn't used later, so it's safe to write to it) to "mark"
that we're done with 0x77777777. This way, when we reach the other hook, we check for this very specific value
to prevent spy effects from happening twice.
The value itself doesn't matter, it just needs to be unique enough to not be accidentally produced by the game there.
*/

//#define INFILTRATE_HOOK_MAGIC 0x77777777
//DEFINE_HOOK(0x45759D, BuildingClass_Infiltrate_NoAres, 0x5)
//{
//	GET_STACK(HouseClass*, pInfiltratorHouse, STACK_OFFSET(0x14, -0x4));
//	GET(BuildingClass*, pBuilding, EBP);
//
//	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
//	R->EAX<int>(INFILTRATE_HOOK_MAGIC);
//	return 0;
//}
//
//DEFINE_HOOK(0x4575A2, BuildingClass_Infiltrate_AfterAres, 0xE)
//{
//	 Check if we've handled it already
//	if (R->EAX<int>() == INFILTRATE_HOOK_MAGIC)
//	{
//		R->EAX<int>(0);
//		return 0;
//	}
//
//	GET_STACK(HouseClass*, pInfiltratorHouse, -0x4);
//	GET(BuildingClass*, pBuilding, ECX);
//
//	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
//	return 0;
//}
//
//#undef INFILTRATE_HOOK_MAGIC

DEFINE_HOOK(0x51A002, InfantryClass_PCP_InfitrateBuilding, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	auto const pHouse = pThis->Owner;
	pBuilding->Infiltrate(pHouse);

	//if (
		BuildingExt::HandleInfiltrate(pBuilding, pHouse)
	//	)
		//Debug::Log("Phobos CustomSpy Affect Return True ! \n")
			;

	return 0x51A010;
}

DEFINE_HOOK(0x465D40, BuildingTypeClass_IsUndeployable_ConsideredVehicle, 0x6)
{
	enum { ReturnFromFunction = 0x465D6A , Continue = 0x0 };

	GET(BuildingTypeClass*, pThis, ECX);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pThis);

	if (pExt->ConsideredVehicle.isset() && pExt->ConsideredVehicle.Get())
	{
		R->EAX(true);
		return ReturnFromFunction;
	}

	return Continue;
}