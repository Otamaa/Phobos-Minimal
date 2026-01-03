#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Building/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Helpers/Enumerators.h>

#include "Header.h"

#include <InfantryClass.h>
#include <CaptureManagerClass.h>

ASMJIT_PATCH(0x442DF2, BuildingClass_Demolish_Tunnel, 6)
{
	GET_STACK(AbstractClass*, pKiller, 0x90);
	GET(BuildingClass*, pTarget, EDI);

	if (auto pTunnelData = HouseExtData::GetTunnelVector(pTarget->Type, pTarget->Owner))
		TunnelFuncs::DestroyTunnel(&pTunnelData->Vector, pTarget, flag_cast_to<TechnoClass*>(pKiller));

	return 0;
}

ASMJIT_PATCH(0x73A23F, UnitClass_UpdatePosition_Tunnel, 0x6)
{
	enum { Entered = 0x73A315, FailedToEnter = 0x73A796, Nothing = 0x0 };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pTarget, EBX);

	if (pThis->GetCurrentMission() != Mission::Enter)
		return Nothing;

	if (pThis->Destination != pTarget)
		return Nothing;

	const auto pTunnelData = HouseExtData::GetTunnelVector(pTarget->Type, pTarget->Owner);
	if (!pTunnelData)
		return Nothing;

	return TunnelFuncs::CanEnterTunnel(&pTunnelData->Vector, pTarget, pThis) ?
		Entered : FailedToEnter;
}

ASMJIT_PATCH(0x73F606, UnitClass_IsCellOccupied_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;
	const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (!canbeDestination)
		canbeDestination = pBuildingTypeExt->TunnelType >= 0;

	return canbeDestination ? 0x73F616 : 0x73F628;
}

ASMJIT_PATCH(0x741CE5, UnitClass_SetDestination_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;
	const auto pBuildingTypeExt = BuildingTypeExtContainer::Instance.Find(pBuilding->Type);

	if (!canbeDestination)
		canbeDestination = pBuildingTypeExt->TunnelType >= 0;

	return canbeDestination ? 0x741CF5 : 0x741D12;
}

ASMJIT_PATCH(0x43C716, BuildingClass_ReceivedRadioCommand_RequestCompleteEnter_Tunnel, 6)
{
	GET(BuildingClass*, pThis, ESI);
	enum
	{
		ProcessTechnoClassRadio = 0x43CCF2,
		DoNothing = 0x0,
	};

	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->TunnelType > -1
		? ProcessTechnoClassRadio : DoNothing;
}

ASMJIT_PATCH(0x44731C, BuildingClass_GetActionOnObject_Tunnel, 6)
{
	enum { RetActionSelf = 0x4472E7, Nothing = 0x0 };
	GET(BuildingClass*, pThis, ESI);

	bool FindSameTunnel = false;
	if (const auto nTunnelVec = HouseExtData::GetTunnelVector(pThis->Type, pThis->Owner))
		FindSameTunnel = !nTunnelVec->Vector.empty();

	return FindSameTunnel ? RetActionSelf : Nothing;
}

ASMJIT_PATCH(0x44A37F, BuildingClass_Mi_Selling_Tunnel_TryToPlacePassengers, 6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(CellStruct*, CellArr, EDI);
	GET(CellStruct, nCell, EDX);
	GET_STACK(int, nDev, 0x30);

	const auto nTunnelVec = HouseExtData::GetTunnelVector(pThis->Type, pThis->Owner);

	if (!nTunnelVec || TunnelFuncs::FindSameTunnel(pThis))
		return 0x0;

	int nOffset = 0;
	auto nPos = nTunnelVec->Vector.end();

	while (std::begin(nTunnelVec->Vector) != std::end(nTunnelVec->Vector))
	{
		nOffset++;
		auto const& [status, pPtr] = TunnelFuncs::UnlimboOne(&nTunnelVec->Vector, pThis, (nCell.X + CellArr[nOffset % nDev].X) | ((nCell.Y + CellArr[nOffset % nDev].Y) << 16));
		if (!status)
		{
			TunnelFuncs::KillFootClass(pPtr, nullptr);
		}
	}

	nTunnelVec->Vector.clear();

	return 0x0;
}

ASMJIT_PATCH(0x44D880, BuildingClass_Mi_Unload_Tunnel, 5)
{
	GET(BuildingClass*, pThis, ECX);
	auto pThisType = pThis->Type;

	const auto nTunnelVec = HouseExtData::GetTunnelVector(pThisType, pThis->Owner);

	if (!nTunnelVec)
		return 0x0; //something on `TechnoClass::AI` is causing building uneable to
	// properly reset the mission after Unload + Turret
	// seems strange
	// method used below is one that working for the thing

	if (!nTunnelVec->Vector.empty()) {
		TunnelFuncs::HandleUnload(&nTunnelVec->Vector, pThis);

		if (nTunnelVec->Vector.empty())
		{
			pThis->ShouldLoseTargetNow = true;
			pThis->ForceMission(Mission::Guard);
			pThis->SetTarget(nullptr);
			R->EAX(1);
			return 0x44E388;
		}
	}

	auto miss = pThis->GetCurrentMissionControl();
	R->EAX(int(miss->Rate * 900.0) + ScenarioClass::Instance->Random.RandomFromMax(2));

	return 0x44E388;
}

// Phobos hook on higher part of this for grinding
// ASMJIT_PATCH(0x43C326, BuildingClass_ReceivedRadioCommand_QueryCanEnter_Tunnel, 0xA)
// {
// 	enum
// 	{
// 		RetRadioNegative = 0x43C3F0,
// 		ContineCheck = 0x43C4F8,
// 		RetRadioRoger = 0x43C535,
// 		ReturnStatic = 0x43C31A
// 	};

// 	GET(BuildingClass*, pThisBld, ESI);
// 	GET(TechnoClass*, pRecpt, EDI);

// 	const auto  nMission = pThisBld->GetCurrentMission();

// 	if (pThisBld->BState == BStateType::Construction ||
// 		nMission == Mission::Construction ||
// 		nMission == Mission::Selling)
// 		return RetRadioNegative;

// 	const auto pBldType = pThisBld->Type;
// 	const auto pBldTypeExt = BuildingTypeExtContainer::Instance.Find(pBldType);
// 	const auto pRectpType = pRecpt->GetTechnoType();

// 	const bool isAmphibious = pRectpType->MovementZone == MovementZone::Amphibious
// 		|| pRectpType->MovementZone == MovementZone::AmphibiousCrusher
// 		|| pRectpType->MovementZone == MovementZone::AmphibiousDestroyer;

// 	if (!isAmphibious && pBldType->Naval != pRectpType->Naval)
// 		return RetRadioNegative;

// 	if ((pRectpType->BalloonHover || pRectpType->JumpJet)
// 		|| !pThisBld->HasPower
// 		|| !TechnoTypeExtData::PassangersAllowed(pBldType, pRectpType)
// 		)
// 		return RetRadioNegative;

// 	if (pRecpt->CaptureManager && pRecpt->CaptureManager->IsControllingSomething())
// 		return RetRadioNegative;

// 	const bool IsTunnel = pBldTypeExt->TunnelType >= 0;
// 	const bool IsUnitAbsorber = pBldType->UnitAbsorb;
// 	const bool IsInfAbsorber = pBldType->InfantryAbsorb;
// 	const bool IsAbsorber = IsUnitAbsorber || IsInfAbsorber;

// 	if (!IsAbsorber && !IsTunnel && !pThisBld->HasFreeLink(pRecpt) && !Unsorted::ScenarioInit())
// 		return RetRadioNegative;

// 	const auto whatRept = pRecpt->WhatAmI();

// 	//tunnel check is taking predicate
// 	if (IsTunnel)
// 	{
// 		if (pThisBld->IsMindControlled())
// 			return RetRadioNegative;

// 		const auto pTunnelData = HouseExtData::GetTunnelVector(pBldType, pThisBld->Owner);

// 		if (((int)pTunnelData->Vector.size() >= pTunnelData->MaxCap)
// 			|| (pBldType->SizeLimit < pRectpType->Size))
// 		{
// 			R->EBX(pBldType);
// 			return ContineCheck;
// 		}

// 		return RetRadioRoger;
// 	}

// 	//next is for absorbers
// 	if(IsAbsorber) {
// 		if ((IsUnitAbsorber && whatRept == UnitClass::AbsID) || (IsInfAbsorber && whatRept == InfantryClass::AbsID)) {
// 			if (pThisBld->Passengers.NumPassengers >= pBldType->Passengers
// 				|| pBldType->SizeLimit < pRectpType->Size) {
// 				R->EBX(pBldType);
// 				return ContineCheck;
// 			}

// 			return RetRadioRoger;
// 		}

// 		return RetRadioNegative;
// 	}

// 	R->EBX(pBldType);
// 	return ContineCheck;
// }

ASMJIT_PATCH(0x51ED8E, InfantryClass_GetActionOnObject_Tunnel, 6)
{
	enum
	{
		CanEnter = 0x51ED9Cu,
		CannotEnter = 0x51EE3Bu
	};

	//GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EAX);

	if (pTargetType->Passengers > 0 && !TechnoTypeExtContainer::Instance.Find(pTargetType)->NoManualEnter)
		return CanEnter;

	bool Enterable = false;
	if (const auto pBuildingTarget = cast_to<BuildingClass*>(pTarget)) {
		Enterable = BuildingTypeExtContainer::Instance.Find(pBuildingTarget->Type)->TunnelType >= 0;
	}

	return Enterable ? CanEnter :
		CannotEnter;
}

ASMJIT_PATCH(0x51A2AD, InfantryClass_UpdatePosition_Tunnel, 9)
{
	enum { CanEnter = 0x51A396, CannotEnter = 0x51A488, Nothing = 0x0 };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	if (!RulesExtData::Instance()->Infantry_IgnoreBuildingSizeLimit) {
		return pBld->Passengers.NumPassengers + 1 <= pBld->Type->Passengers
			&& static_cast<int>(pThis->Type->Size) <= pBld->Type->SizeLimit
			? 0 : 0x51A4BF;
	}

	if (const auto nTunnelVec = HouseExtData::GetTunnelVector(pBld->Type, pBld->Owner))
	{
		return TunnelFuncs::CanEnterTunnel(&nTunnelVec->Vector, pBld, pThis) ? CanEnter : CannotEnter;
	}

	return Nothing;
}

ASMJIT_PATCH(0x44351A, BuildingClass_ActionOnObject_Tunnel, 6)
{
	enum
	{
		MissionUnload = 0x443534
		, CheckOccupant = 0x443545
	};

	GET(BuildingClass*, pThis, EBX);

	auto const pType = pThis->Type;
	if (pThis->Absorber())
		return MissionUnload;

	return BuildingTypeExtContainer::Instance.Find(pType)->TunnelType >= 0 ?
		MissionUnload : CheckOccupant;
}