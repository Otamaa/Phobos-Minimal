#include "Body.h"

#include <InfantryClass.h>
#include <InputManagerClass.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

#include <New/Entity/FlyingStrings.h>

DEFINE_HOOK(0x43C30A, BuildingClass_ReceiveMessage_Grinding, 0x6)
{
	enum { ReturnStatic = 0x43C31A, ReturnNegative = 0x43CB68, ReturnRoger = 0x43CCF2 };

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pFrom, EDI);

	if (pThis->Type->Grinding)
	{
		if (!pThis->Owner->IsAlliedWith(pFrom))
			return ReturnStatic;

		if (pThis->GetCurrentMission() == Mission::Construction || pThis->GetCurrentMission() == Mission::Selling ||
			pThis->BState == 0 || !pThis->HasPower || pFrom->GetTechnoType()->BalloonHover)
		{
			return ReturnNegative;
		}

		bool isAmphibious = pFrom->GetTechnoType()->MovementZone == MovementZone::Amphibious || pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousCrusher ||
			pFrom->GetTechnoType()->MovementZone == MovementZone::AmphibiousDestroyer;

		if (!isAmphibious && (pThis->GetTechnoType()->Naval && !pFrom->GetTechnoType()->Naval ||
			!pThis->GetTechnoType()->Naval && pFrom->GetTechnoType()->Naval))
		{
			return ReturnNegative;
		}

		return BuildingExt::CanGrindTechno(pThis, pFrom) ? ReturnRoger : ReturnNegative;
	}

	return 0;
}

DEFINE_HOOK(0x4D4CD3, FootClass_Mission_Eaten_Grinding, 0x6)
{
	enum { LoseDestination = 0x4D4D43 };

	GET(FootClass*, pThis, ESI);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis->Destination))
	{
		if (pBuilding->Type->Grinding && !BuildingExt::CanGrindTechno(pBuilding, pThis))
		{
			pThis->SetDestination(nullptr, false);
			return LoseDestination;
		}
	}

	return 0;
}

DEFINE_HOOK(0x51F0AF, InfantryClass_WhatAction_Grinding, 0x7) //0
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
	enum { ReturnValue = 0x51F17E };

	GET(InfantryClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	if (auto pBuilding = abstract_cast<BuildingClass*>(pTarget))
	{
		bool canBeGrinded = BuildingExt::CanGrindTechno(pBuilding, pThis);
		R->EBP(canBeGrinded ? Action::Repair : Action::NoGRepair);
		return ReturnValue;
	}

	return 0;
}

DEFINE_HOOK(0x740134, UnitClass_WhatAction_Grinding, 0x7) //0
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
	enum { Skip = 0x4DFB30 };

	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	if (!BuildingExt::CanGrindTechno(pBuilding, pThis))
		return Skip;

	return 0;
}

DEFINE_HOOK(0x5198B3, InfantryClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x5198CE };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	return BuildingExt::DoGrindingExtras(pBuilding, pThis) ? Continue : 0;
}

DEFINE_HOOK(0x73A1C3, UnitClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x73A1DE };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	return BuildingExt::DoGrindingExtras(pBuilding, pThis) ? Continue : 0;
}

//DEFINE_HOOK(0x73E3DB, UnitClass_Mission_Unload_NoteBalanceBefore, 0x6)
//{
//	GET(HouseClass* const, pHouse, EBX);
//	HouseExt::LastHarvesterBalance = pHouse->Balance;
//	return 0;
//}
//
//DEFINE_HOOK(0x73E4D0, UnitClass_Mission_Unload_CheckBalanceAfter, 0xA)
//{
//	GET(HouseClass* const, pHouse, EBX);
//	GET(BuildingClass* const, pDock, EDI);
//
//	int delta = pHouse->Balance - HouseExt::LastHarvesterBalance;
//	auto pBldExt = BuildingExt::ExtMap.Find(pDock);
//	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pDock->Type);
//
//	FlyingStrings::AddMoneyString(delta, delta, pDock, pTypeExt->Grinding_DisplayRefund_Houses
//			, pDock->GetRenderCoords(), pTypeExt->Grinding_DisplayRefund_Offset);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x522D50, InfantryClass_SlaveGiveMoney_RecordBalanceBefore, 0x5)
//{
//	GET_STACK(BuildingClass* const, slaveMiner, 0x4);
//	HouseExt::LastSlaveBalance = slaveMiner->Owner->Balance;
//	return 0;
//}
//
//DEFINE_HOOK(0x522E4F, InfantryClass_SlaveGiveMoney_CheckBalanceAfter, 0x6)
//{
//	GET_STACK(BuildingClass* const, slaveMiner, STACK_OFFSET(0x18, 0x4));
//
//	int delta = slaveMiner->Owner->Balance - HouseExt::LastSlaveBalance;
//	auto pBldExt = BuildingExt::ExtMap.Find(slaveMiner);
//	auto pTypeExt = BuildingTypeExt::ExtMap.Find(slaveMiner->Type);
//
//	FlyingStrings::AddMoneyString(delta, delta, slaveMiner, pTypeExt->Grinding_DisplayRefund_Houses
//			, slaveMiner->GetRenderCoords(), pTypeExt->Grinding_DisplayRefund_Offset);
//
//
//	return 0;
//}