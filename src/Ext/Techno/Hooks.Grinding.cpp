#include "Body.h"

#include <InfantryClass.h>
#include <WWKeyboardClass.h>

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>

#include <New/Entity/FlyingStrings.h>

#include <CaptureManagerClass.h>

ASMJIT_PATCH(0x43C30A, BuildingClass_ReceiveMessage_Grinding, 0x6)
{
	enum {
		ReturnStatic = 0x43C31A,
		ReturnNegative = 0x43CB68,
		ReturnRoger = 0x43CCF2,
		ContineCheck = 0x43C506,
	};

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pFrom, EDI);

	auto const pFromTechnoType = pFrom->GetTechnoType();
	const bool isAmphibious = pFromTechnoType->MovementZone == MovementZone::Amphibious
	|| pFromTechnoType->MovementZone == MovementZone::AmphibiousCrusher
	|| pFromTechnoType->MovementZone == MovementZone::AmphibiousDestroyer;

	if (!isAmphibious && pThis->Type->Naval != pFromTechnoType->Naval) {
		return ReturnNegative;
	}

	auto const nMission = pThis->GetCurrentMission();
	auto const pExt = BuildingExtContainer::Instance.Find(pThis);
	const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	if (nMission == Mission::Construction
		|| nMission == Mission::Selling
		|| pThis->BState == BStateType::Construction
		|| !pThis->HasPower
		|| pFromTechnoType->BalloonHover)
	{
		return ReturnNegative;
	}

	if (!pThis->Owner->IsAlliedWith(pFrom) || pExt->LimboID != -1 || pThis->Owner->Type->MultiplayPassive)
		return ReturnStatic;

	if (pThis->Type->Grinding) {
		return BuildingExtData::CanGrindTechnoSimplified(pThis, pFrom) ? ReturnRoger : ReturnNegative;
	}

	if (!TechnoTypeExtData::PassangersAllowed(pThis->Type, pFromTechnoType))
		return ReturnNegative;

	if (pFrom->CaptureManager && pFrom->CaptureManager->IsControllingSomething())
		return ReturnNegative;

	const bool IsTunnel = pBldTypeExt->TunnelType >= 0;
	const bool IsUnitAbsorber = pThis->Type->UnitAbsorb;
	const bool IsInfAbsorber = pThis->Type->InfantryAbsorb;
	const bool IsAbsorber = IsUnitAbsorber || IsInfAbsorber;

	if (!IsAbsorber && !IsTunnel)
	{
		if (!pThis->HasFreeLink(pFrom) && !Unsorted::ScenarioInit())
			return ReturnNegative;

		R->EBX(pThis->Type);
		return ContineCheck;
	}

	const auto whatRept = pFrom->WhatAmI();

	//tunnel check is taking predicate
	if (IsTunnel) {

		if (pThis->IsMindControlled())
			return ReturnNegative;

		const auto pTunnelData = HouseExtData::GetTunnelVector(pThis->Type, pThis->Owner);

		if (((int)pTunnelData->Vector.size() + 1 > pTunnelData->MaxCap)
			|| (pThis->Type->SizeLimit < pFromTechnoType->Size))
		{
			R->EBX(pThis->Type);
			return ContineCheck;
		}

		return ReturnRoger;
	}

	//next is for absorbers
	if(IsAbsorber) {

		if ((IsUnitAbsorber && whatRept == UnitClass::AbsID) || (IsInfAbsorber && whatRept == InfantryClass::AbsID)) {
			if (pThis->Passengers.NumPassengers >= pThis->Type->Passengers
				|| pThis->Type->SizeLimit < pFromTechnoType->Size) {
				R->EBX(pThis->Type);
				return ContineCheck;
			}

			return ReturnRoger;
		}

		return ReturnNegative;
	}

	R->EBX(pThis->Type);
	return ContineCheck;
}

ASMJIT_PATCH(0x4D4CD3, FootClass_Mission_Eaten_Grinding, 0x6)
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

ASMJIT_PATCH(0x51F0AF, InfantryClass_WhatAction_Grinding, 0x5)
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
#include <Misc/Ares/Hooks/Header.h>

void PlayDieSounds(TechnoClass* pTechno) {

	auto pTechnoType = pTechno->GetTechnoType();

	if (pTechnoType->VoiceDie.Count > 0 && pTechno->Owner->ControlledByCurrentPlayer())
	{
		const int idx = pTechnoType->VoiceDie.Count == 1 ? 0 :
			Random2Class::NonCriticalRandomNumber->Random() % pTechnoType->VoiceDie.Count;
		VocClass::SafeImmedietelyPlayAt(idx, &pTechno->Location, nullptr);
	}

	if (pTechnoType->DieSound.Count > 0)
	{
		const int idx = pTechnoType->DieSound.Count == 1 ? 0 :
			Random2Class::NonCriticalRandomNumber->Random() % pTechnoType->DieSound.Count;

		VocClass::SafeImmedietelyPlayAt(idx, &pTechno->Location, nullptr);
	}
}

ASMJIT_PATCH(0x739FBC, UnitClass_PerCellProcess_Grinding, 0x6)
{
	enum { Continue = 0x73A1BC , PlayAnim = 0x73A1DE , RemoveUnit = 0x73A222 };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pBuilding, EBX);

	auto pTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (!pBuilding->Type->Grinding || pTypeExt->Grinding_PlayDieSound) {
		PlayDieSounds(pThis);
	}

	HouseExtData::LastGrindingBlanceUnit = pBuilding->Owner->Available_Money();
	pBuilding->Owner->TransactMoney(pThis->GetRefund());

	const bool pParentReverseEngineered = pBuilding->Type->Grinding && BuildingExtData::ReverseEngineer(pBuilding, pThis);

	//https://bugs.launchpad.net/ares/+bug/1925359
	TechnoExt_ExtData::AddPassengers(pBuilding, pThis , pParentReverseEngineered);

	if (auto const MyParasite = pThis->ParasiteEatingMe) {
		pBuilding->Owner->GiveMoney(MyParasite->GetRefund());
		MyParasite->ParasiteImUsing->SuppressionTimer.Start(50);
		MyParasite->ParasiteImUsing->ExitUnit();
	}

	if (const auto FirstTag = pThis->AttachedTag) {
		FirstTag->RaiseEvent(TriggerEvent::DestroyedByAnything, pThis, CellStruct::Empty, false, nullptr);
	}

	if (!pBuilding->Type->Grinding)
		return RemoveUnit;

	if (pParentReverseEngineered)
	{
		if (pBuilding->Owner->ControlledByCurrentPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredVehicle");
			VoxClass::Play(GameStrings::EVA_NewTechAcquired());
		}

		if (const auto FirstTag = pBuilding->AttachedTag)
		{
			FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, pBuilding, CellStruct::Empty, false, pThis);

			if (auto pSecondTag = pBuilding->AttachedTag)
			{
				pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, pBuilding, CellStruct::Empty, false, nullptr);
			}
		}
	}

	// #368: refund hijackers
	if (pThis->HijackerInfantryType != -1) {
		pBuilding->Owner->TransactMoney(InfantryTypeClass::Array->Items[pThis->HijackerInfantryType]->GetRefund(pThis->Owner, 0));
	}

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExtData::LastGrindingBlanceUnit;

	return BuildingExtData::DoGrindingExtras(pBuilding, pThis, totalRefund) ? PlayAnim : Continue;
}

ASMJIT_PATCH(0x740134, UnitClass_WhatAction_Grinding, 0x9) //0
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
				action = pBuilding->Type->Grinding ? canBeGrinded && !isFlying ? Action::Repair : Action::NoEnter :

				((((FakeBuildingClass*)pBuilding)->_GetTypeExtData()->AllowRepairFlyMZone && isFlying) || !isFlying) ? Action::Enter : Action::NoEnter;


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

ASMJIT_PATCH(0x4DFABD, FootClass_Try_Grinding_CheckIfAllowed, 0x8)
{
	enum { Continue = 0x0 , Skip = 0x4DFB30 };
	GET(FootClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);
	return BuildingExtData::CanGrindTechno(pBuilding, pThis)
		? Continue : Skip;
}

ASMJIT_PATCH(0x519790, InfantryClass_PerCellProcess_Grinding, 0xA)
{
	enum { Continue = 0x5198AD, PlayAnims = 0x5198CE  , RemoveInfantry = 0x51A02A };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EBX);

	if (!pBuilding->Type->Grinding || BuildingTypeExtContainer::Instance.Find(pBuilding->Type)->Grinding_PlayDieSound)
	{
		PlayDieSounds(pThis);
	}

	HouseExtData::LastGrindingBlanceInf = pBuilding->Owner->Available_Money();

	pBuilding->Owner->TransactMoney(pThis->GetRefund());

	if (auto const MyParasite = pThis->ParasiteEatingMe){
		pBuilding->Owner->GiveMoney(MyParasite->GetRefund());
		MyParasite->ParasiteImUsing->SuppressionTimer.Start(50);
		MyParasite->ParasiteImUsing->ExitUnit();
	}

	if (const auto FirstTag = pThis->AttachedTag) {
		FirstTag->RaiseEvent(TriggerEvent::DestroyedByAnything, pThis, CellStruct::Empty, false, nullptr);
	}

	if (!pBuilding->Type->Grinding)
		return RemoveInfantry;

	if (BuildingExtData::ReverseEngineer(pBuilding, pThis)) {

		if (pBuilding->Owner->ControlledByCurrentPlayer())
		{
			VoxClass::Play("EVA_ReverseEngineeredInfantry");
			VoxClass::Play(GameStrings::EVA_NewTechAcquired());
		}

		//Ares 3.0 Added
		if (const auto FirstTag = pBuilding->AttachedTag)
		{
			//80
			FirstTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerType, pBuilding, CellStruct::Empty, false, pThis);

			//79
			if (const auto pSecondTag = pBuilding->AttachedTag)
				pSecondTag->RaiseEvent((TriggerEvent)AresTriggerEvents::ReverseEngineerAnything, pBuilding, CellStruct::Empty, false, nullptr);
		}
	}

	const int totalRefund = pBuilding->Owner->Available_Money() - HouseExtData::LastGrindingBlanceInf;

	// Calculated like this because it is easier than tallying up individual refunds for passengers and parasites.
	return BuildingExtData::DoGrindingExtras(pBuilding, pThis , totalRefund) ? PlayAnims : Continue;
}
