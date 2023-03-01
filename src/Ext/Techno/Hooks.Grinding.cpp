#include "Body.h"

#include <InfantryClass.h>
#include <InputManagerClass.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>

#include <New/Entity/FlyingStrings.h>

DEFINE_HOOK(0x43C30A, BuildingClass_ReceiveMessage_Grinding, 0x6)
{
	enum { Continue = 0x0 , ReturnStatic = 0x43C31A, ReturnNegative = 0x43CB68, ReturnRoger = 0x43CCF2 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pFrom, EDI);

	if (pThis->Type->Grinding)
	{
		auto const pExt = BuildingExt::ExtMap.Find(pThis);

		if (pExt->LimboID != -1 || pThis->Owner->Type->MultiplayPassive)
			return ReturnStatic;

		if (!pThis->Owner->IsAlliedWith(pFrom))
			return ReturnStatic;

		if (pThis->GetCurrentMission() == Mission::Construction || pThis->GetCurrentMission() == Mission::Selling ||
			pThis->BState == BStateType::Construction || !pThis->HasPower || pFrom->GetTechnoType()->BalloonHover)
		{
			return ReturnNegative;
		}

		const bool isAmphibious = pFrom->GetTechnoType()->MovementZone == MovementZone::Amphibious || pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousCrusher ||
			pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousDestroyer;

		if (!isAmphibious && (pThis->GetTechnoType()->Naval && !pFrom->GetTechnoType()->Naval ||
			!pThis->GetTechnoType()->Naval && pFrom->GetTechnoType()->Naval))
		{
			return ReturnNegative;
		}

		return BuildingExt::CanGrindTechno(pThis, pFrom) ? ReturnRoger : ReturnNegative;
	}

	return Continue;
}

DEFINE_HOOK(0x4D4CD3, FootClass_Mission_Eaten_Grinding, 0x6)
{
	enum { Continue = 0x0,  LoseDestination = 0x4D4D43 };

	GET(FootClass*, pThis, ESI);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis->Destination))
	{
		if (pBuilding->Type->Grinding && !BuildingExt::CanGrindTechno(pBuilding, pThis))
		{
			pThis->SetDestination(nullptr, false);
			return LoseDestination;
		}
	}

	return Continue;
}

DEFINE_HOOK(0x51F0AF, InfantryClass_WhatAction_Grinding, 0x5)
{
	enum { Skip = 0x51F05E, ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(Action, action, EBP);

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		if (pBuilding->Type->Grinding && pThis->Owner->IsControlledByCurrentPlayer() && !pBuilding->IsBeingWarpedOut() &&
			pThis->Owner->IsAlliedWith(pTarget) && (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Grinding_AllowAllies || action == Action::Select))
		{
			action = BuildingExt::CanGrindTechno(pBuilding, pThis) ? Action::Repair : Action::NoEnter;
			R->EBP(action);
			return ReturnValue;
		}
	}

 return Skip;
}

DEFINE_HOOK(0x51E63A, InfantryClass_WhatAction_Grinding_Engineer, 0x6)
{
	enum { Continue = 0x0, ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		const bool canBeGrinded = pBuilding->Type->Grinding && BuildingExt::CanGrindTechno(pBuilding, pThis);
		R->EBP(canBeGrinded ? Action::Repair : Action::NoGRepair);
		return ReturnValue;
	}

	return Continue;
}

DEFINE_HOOK(0x740134, UnitClass_WhatAction_Grinding, 0x9) //0
{
	enum { Continue = 0x7401C1 };

	GET(UnitClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EDI);
	GET(Action, action, EBX);

	if (InputManagerClass::Instance->IsForceFireKeyPressed() && pThis->IsArmed())
		return Continue;

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		if (pThis->Owner->IsControlledByCurrentPlayer() && !pBuilding->IsBeingWarpedOut() &&
			pThis->Owner->IsAlliedWith(pTarget) && (pBuilding->Type->Grinding || action == Action::Select))
		{
			if (pThis->SendCommand(RadioCommand::QueryCanEnter, pTarget) == RadioCommand::AnswerPositive)
			{
				bool isFlying = pThis->GetTechnoType()->MovementZone == MovementZone::Fly;
				bool canBeGrinded = BuildingExt::CanGrindTechno(pBuilding, pThis);
				action = pBuilding->Type->Grinding ? canBeGrinded && !isFlying ? Action::Repair : Action::NoEnter : !isFlying ? Action::Enter : Action::NoEnter;
				R->EBX(action);
			}
			else if (pBuilding->Type->Grinding)
			{
				R->EBX(Action::NoEnter);
			}
		}
	}

	return Continue;
}

DEFINE_HOOK(0x4DFABD, FootClass_Try_Grinding_CheckIfAllowed, 0x8)
{
	enum { Continue = 0x0 , Skip = 0x4DFB30 };
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingExt::CanGrindTechno(pBuilding, pThis) 
		? Continue : Skip;
}

DEFINE_HOOK(0x51986A, InfantryClass_PerCellProcess_GrindingSetBalance, 0xA)
{
	GET(BuildingClass*, pBuilding, EBX);
	HouseExt::LastGrindingBlanceInf = pBuilding->Owner->Available_Money();
	return 0x0;
}

DEFINE_HOOK(0x5198B3, InfantryClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x0, PlayAnims = 0x5198CE };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	if (auto const MyParasite = pThis->ParasiteEatingMe){
		pBuilding->Owner->GiveMoney(MyParasite->GetRefund());
		MyParasite->ParasiteImUsing->SuppressionTimer.Start(50);
		MyParasite->ParasiteImUsing->ExitUnit();
	}

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExt::LastGrindingBlanceInf;

	return BuildingExt::DoGrindingExtras(pBuilding, pThis , totalRefund) ? PlayAnims : Continue;
}

DEFINE_HOOK(0x73A0A5, UnitClass_PerCellProcess_GrindingSetBalance, 0x5)
{
	GET(BuildingClass*, pBuilding, EBX);
	HouseExt::LastGrindingBlanceUnit = pBuilding->Owner->Available_Money();
	return 0;
}

DEFINE_HOOK(0x73A1C3, UnitClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x0 , PlayAnim = 0x73A1DE };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExt::LastGrindingBlanceUnit;

	return BuildingExt::DoGrindingExtras(pBuilding, pThis, totalRefund) ? PlayAnim : Continue;

}

DEFINE_HOOK(0x519790, InfantryClass_UpdatePosition_Grinding_SkipDiesound, 0xA)
{
	enum { Play = 0x0 , DoNotPlay = 0x51986A };
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Grinding_PlayDieSound.Get() ? 
		Play : DoNotPlay;
}

DEFINE_HOOK(0x739FBC, UnitClass_UpdatePosition_Grinding_SkipDiesound, 0x5)
{
	enum { Play = 0x0, DoNotPlay = 0x073A0A5 };
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingTypeExt::ExtMap.Find(pBuilding->Type)->Grinding_PlayDieSound.Get() ?
		Play : DoNotPlay;
}

DEFINE_HOOK(0x73E3DB, UnitClass_Mission_Unload_NoteBalanceBefore, 0x6)
{
	GET(HouseClass* const, pHouse, EBX);
	HouseExt::LastHarvesterBalance = pHouse->Available_Money();
	return 0;
}

DEFINE_HOOK(0x73E4D0, UnitClass_Mission_Unload_CheckBalanceAfter, 0xA)
{
	GET(HouseClass* const, pHouse, EBX);
	GET(BuildingClass* const, pDock, EDI);

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pDock->Type);

	if(pTypeExt->Refinery_DisplayDumpedMoneyAmount){	
		const int delta = pHouse->Available_Money() - HouseExt::LastHarvesterBalance;
		FlyingStrings::AddMoneyString(delta, delta, pDock, AffectedHouse::All
				, pDock->GetRenderCoords(), pTypeExt->Refinery_DisplayRefund_Offset);
	}

	return 0;
}

DEFINE_HOOK(0x522D50, InfantryClass_SlaveGiveMoney_RecordBalanceBefore, 0x5)
{
	//this is techno class , not building class , wtf
	GET_STACK(BuildingClass* const, slaveMiner, 0x4);

	const auto pBuilding = specific_cast<BuildingClass* const >(slaveMiner);

	if (!pBuilding)
		return 0x0;

	HouseExt::LastSlaveBalance = slaveMiner->Owner->Available_Money();
	return 0x0;
}

DEFINE_HOOK(0x522E4F, InfantryClass_SlaveGiveMoney_CheckBalanceAfter, 0x6)
{
	//this is techno class , not building class , wtf
	GET_STACK(TechnoClass* const, slaveMiner, STACK_OFFSET(0x18, 0x4));

	const auto pBuilding = specific_cast<BuildingClass* const >(slaveMiner);

	if (!pBuilding) {
		return 0x0;
	}

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (pTypeExt->Refinery_DisplayDumpedMoneyAmount)
	{
		const int delta = slaveMiner->Owner->Available_Money() - HouseExt::LastSlaveBalance;
		FlyingStrings::AddMoneyString(delta, delta, slaveMiner, AffectedHouse::All
				, slaveMiner->GetRenderCoords(), pTypeExt->Refinery_DisplayRefund_Offset);

	}

	return 0x0;
}