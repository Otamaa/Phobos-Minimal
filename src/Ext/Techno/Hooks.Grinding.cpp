#include "Body.h"

#include <InfantryClass.h>
#include <WWKeyboardClass.h>

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
		auto const pExt = BuildingExtContainer::Instance.Find(pThis);

		if (pExt->LimboID != -1 || pThis->Owner->Type->MultiplayPassive)
			return ReturnStatic;

		if (!pThis->Owner->IsAlliedWith(pFrom))
			return ReturnStatic;

		auto const pFromTechnoType = pFrom->GetTechnoType();
		auto const nMission = pThis->GetCurrentMission();

		if (nMission == Mission::Construction
			|| nMission == Mission::Selling
			|| pThis->BState == BStateType::Construction
			|| !pThis->HasPower
			|| pFromTechnoType->BalloonHover)
		{
			return ReturnNegative;
		}

		const bool isAmphibious = pFromTechnoType->MovementZone == MovementZone::Amphibious
			|| pFromTechnoType->MovementZone == MovementZone::AmphibiousCrusher
			|| pFromTechnoType->MovementZone == MovementZone::AmphibiousDestroyer;

		if (!isAmphibious && (pThis->Type->Naval && !pFromTechnoType->Naval ||
			!pThis->Type->Naval && pFromTechnoType->Naval))
		{
			return ReturnNegative;
		}

		return BuildingExtData::CanGrindTechno(pThis, pFrom) ? ReturnRoger : ReturnNegative;
	}

	return Continue;
}

DEFINE_HOOK(0x4D4CD3, FootClass_Mission_Eaten_Grinding, 0x6)
{
	enum { Continue = 0x0,  LoseDestination = 0x4D4D43 };

	GET(FootClass*, pThis, ESI);

	if (auto const pBuilding = cast_to<BuildingClass*>(pThis->Destination))
	{
		if (pBuilding->Type->Grinding && !BuildingExtData::CanGrindTechno(pBuilding, pThis))
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

	if (auto pBuilding = cast_to<BuildingClass*, false>(pTarget))
	{
		if (action == Action::Select
			&& pBuilding->Type->Grinding
			&& pThis->Owner->IsControlledByHuman()
			&& !pBuilding->IsBeingWarpedOut() )
		{
			action = BuildingExtData::CanGrindTechno(pBuilding, pThis) ? Action::Repair : Action::NoEnter;
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

	if (auto pBuilding = cast_to<BuildingClass*>(pTarget))
	{
		const bool canBeGrinded = pBuilding->Type->Grinding && BuildingExtData::CanGrindTechno(pBuilding, pThis);
		Action ret = canBeGrinded ? Action::Repair : Action::NoGRepair;

		if(ret == Action::NoGRepair &&
			(pBuilding->Type->InfantryAbsorb
			|| BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->TunnelType != -1
			|| pBuilding->Type->Hospital && pThis->GetHealthPercentage() < RulesClass::Instance->ConditionGreen
			|| pBuilding->Type->Armory && pThis->Type->Trainable
		  )){
			ret = pThis->SendCommand(RadioCommand::QueryCanEnter, pTarget) == RadioCommand::AnswerPositive ?
				Action::Enter : Action::NoEnter;
		}

		R->EBP(ret);
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

	if (WWKeyboardClass::Instance->IsForceFireKeyPressed() && pThis->IsArmed())
		return Continue;

	if (auto pBuilding = cast_to<BuildingClass*>(pTarget))
	{
		if (action == Action::Select
			&& pThis->Owner->IsControlledByHuman()
			&& !pBuilding->IsBeingWarpedOut())
		{
			if (pThis->SendCommand(RadioCommand::QueryCanEnter, pTarget) == RadioCommand::AnswerPositive)
			{
				const bool isFlying = pThis->GetTechnoType()->MovementZone == MovementZone::Fly;
				const bool canBeGrinded = BuildingExtData::CanGrindTechno(pBuilding, pThis);
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

#pragma region PosGrind

DEFINE_HOOK(0x4DFABD, FootClass_Try_Grinding_CheckIfAllowed, 0x8)
{
	enum { Continue = 0x0 , Skip = 0x4DFB30 };
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingExtData::CanGrindTechno(pBuilding, pThis)
		? Continue : Skip;
}

DEFINE_HOOK(0x51986A, InfantryClass_PerCellProcess_GrindingSetBalance, 0xA)
{
	GET(BuildingClass*, pBuilding, EBX);
	HouseExtData::LastGrindingBlanceInf = pBuilding->Owner->Available_Money();
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
	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExtData::LastGrindingBlanceInf;

	return BuildingExtData::DoGrindingExtras(pBuilding, pThis , totalRefund) ? PlayAnims : Continue;
}

DEFINE_HOOK(0x73A0A5, UnitClass_PerCellProcess_GrindingSetBalance, 0x5)
{
	GET(BuildingClass*, pBuilding, EBX);
	HouseExtData::LastGrindingBlanceUnit = pBuilding->Owner->Available_Money();
	return 0;
}

DEFINE_HOOK(0x73A1C3, UnitClass_PerCellProcess_Grinding, 0x5)
{
	enum { Continue = 0x0 , PlayAnim = 0x73A1DE };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExtData::LastGrindingBlanceUnit;

	return BuildingExtData::DoGrindingExtras(pBuilding, pThis, totalRefund) ? PlayAnim : Continue;
}

DEFINE_HOOK(0x519790, InfantryClass_UpdatePosition_Grinding_SkipDiesound, 0xA)
{
	enum { Play = 0x0 , DoNotPlay = 0x51986A };
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Grinding_PlayDieSound.Get() ?
		Play : DoNotPlay;
}

DEFINE_HOOK(0x739FBC, UnitClass_UpdatePosition_Grinding_SkipDiesound, 0x5)
{
	enum { Play = 0x0, DoNotPlay = 0x073A0A5 };
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Grinding_PlayDieSound.Get() ?
		Play : DoNotPlay;
}

// Unload more than once per ore dump if the harvester contains more than 1 tiberium type
// DEFINE_HOOK(0x73E3DB, UnitClass_Mission_Unload_NoteBalanceBefore, 0x6)
// {
// 	GET(HouseClass* const, pHouse, EBX); // this is the house of the refinery, not the harvester
// 	// GET(BuildingClass* const, pDock, EDI);

// 	HouseExtData::LastHarvesterBalance = pHouse->Available_Money();// Available_Money takes silos into account

// 	return 0;
// }


//moved to the dumping hook
//DEFINE_HOOK(0x73E4D0, UnitClass_Mission_Unload_CheckBalanceAfter, 0xA)
//{
//	GET(HouseClass* const, pHouse, EBX);
//	GET(BuildingClass* const, pDock, EDI);
//
//	if(BuildingTypeExtContainer::Instance.Find(pDock->Type)->Refinery_DisplayDumpedMoneyAmount){
//		BuildingExtContainer::Instance.Find(pDock)->AccumulatedIncome +=
//			pHouse->Available_Money() - HouseExtData::LastHarvesterBalance;
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x522D50, InfantryClass_SlaveGiveMoney_RecordBalanceBefore, 0x5)
//{
//	GET_STACK(TechnoClass* const, slaveMiner, 0x4);
//	HouseExtData::LastSlaveBalance = slaveMiner->Owner->Available_Money();
//	return 0;
//}
//
//DEFINE_HOOK(0x522E4F, InfantryClass_SlaveGiveMoney_CheckBalanceAfter, 0x6)
//{
//	GET_STACK(TechnoClass* const, slaveMiner, STACK_OFFSET(0x18, 0x4));
//
//	int money = slaveMiner->Owner->Available_Money() - HouseExtData::LastSlaveBalance;
//
//	if (auto pBld = specific_cast<BuildingClass*>(slaveMiner)) {
//		if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Refinery_DisplayDumpedMoneyAmount) {
//			BuildingExtContainer::Instance.Find(pBld)->AccumulatedIncome += money;
//		}
//	}
//	else if (auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(slaveMiner->GetTechnoType()->DeploysInto))
//	{
//		if (pBldTypeExt->Refinery_DisplayDumpedMoneyAmount.Get())
//		{
//			FlyingStrings::AddMoneyString(money, money, slaveMiner, AffectedHouse::All
//				, slaveMiner->Location, pBldTypeExt->Refinery_DisplayRefund_Offset);
//		}
//	}
//
//	return 0;
//}

#pragma endregion