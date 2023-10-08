#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Misc/AresData.h>

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

DEFINE_OVERRIDE_HOOK(0x709D38, TechnoClass_DrawPipscale_Passengers, 7)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);

	if (pType->PipScale != PipScale::Passengers)
		return 0x70A083;

	GET(int, nBracketPosDeltaY, ESI);
	GET_STACK(SHPStruct*, pShp, 0x1C);
	GET_STACK(RectangleStruct*, pRect, 0x80);
	GET_STACK(int, nPosX, 0x50);
	GET_STACK(int, nPosY, 0x54);
	GET_STACK(int, nBracketPosDeltaX, 0x58);

	Point2D nPos = { nPosX ,nPosY };
	int nForGunner = 0;
	bool fail = false;
	const auto pData = TunnelFuncs::PopulatePassangerPIPData(pThis, pType, fail);

	if (fail)
		return 0x70A083;

	for (auto nDPos = pData->begin(); nDPos != pData->end(); ++nDPos)
	{
		DSurface::Temp->DrawSHP
		(FileSystem::PALETTE_PAL,
			pShp,
			*nDPos,
			&nPos,
			pRect,
			BlitterFlags(0x600),
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			0,
			0,
			0,
			0
		);

		++nForGunner;
		const auto nXHere = nBracketPosDeltaX + nPos.X;
		const auto nYHere = nBracketPosDeltaY + nPos.Y;
		nPos.X += nBracketPosDeltaX;
		nPos.Y += nBracketPosDeltaY;

		if ((bool)nForGunner == pType->Gunner)
		{
			nPos.X += nXHere + nBracketPosDeltaX;
			nPos.Y += nYHere + nBracketPosDeltaY;
		}
	}

	return 0x70A4EC;
}

DEFINE_OVERRIDE_HOOK(0x442DF2, BuildingClass_Demolish_Tunnel, 6)
{
	GET_STACK(AbstractClass*, pKiller, 0x90);
	GET(BuildingClass*, pTarget, EDI);

	if (auto pTunnelData = HouseExt::GetTunnelVector(pTarget->Type, pTarget->Owner))
		TunnelFuncs::DestroyTunnel(&pTunnelData->Vector, pTarget, generic_cast<TechnoClass*>(pKiller));

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71A995, TemporalClass_Update_Tunnel, 5)
{
	GET(TemporalClass*, pThis, ESI);
	GET(BuildingClass*, pTarget, EBP);

	if (const auto pTunnelData = HouseExt::GetTunnelVector(pTarget->Type, pTarget->Owner))
		TunnelFuncs::DestroyTunnel(&pTunnelData->Vector, pTarget, pThis->Owner);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73A23F, UnitClass_UpdatePosition_Tunnel, 0x6)
{
	enum { Entered = 0x73A315, FailedToEnter = 0x73A796, Nothing = 0x0 };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pTarget, EBX);

	if (pThis->GetCurrentMission() != Mission::Enter)
		return Nothing;

	if (pThis->Destination != pTarget)
		return Nothing;

	const auto pTunnelData = HouseExt::GetTunnelVector(pTarget->Type, pTarget->Owner);
	if (!pTunnelData)
		return Nothing;

	return TunnelFuncs::CanEnterTunnel(&pTunnelData->Vector, pTarget, pThis) ?
		Entered : FailedToEnter;
}

DEFINE_OVERRIDE_HOOK(0x73F606, UnitClass_IsCellOccupied_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;
	const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (!canbeDestination)
		canbeDestination = pBuildingTypeExt->TunnelType >= 0;

	return canbeDestination ? 0x73F616 : 0x73F628;
}

DEFINE_OVERRIDE_HOOK(0x741CE5, UnitClass_SetDestination_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;
	const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (!canbeDestination)
		canbeDestination = pBuildingTypeExt->TunnelType >= 0;

	return canbeDestination ? 0x741CF5 : 0x741D12;
}

DEFINE_OVERRIDE_HOOK(0x43C716, BuildingClass_ReceivedRadioCommand_RequestCompleteEnter_Tunnel, 6)
{
	GET(BuildingClass*, pThis, ESI);
	enum
	{
		ProcessTechnoClassRadio = 0x43CCF2,
		DoNothing = 0x0,
	};

	return BuildingTypeExt::ExtMap.Find(pThis->Type)->TunnelType > -1
		? ProcessTechnoClassRadio : DoNothing;
}

DEFINE_OVERRIDE_HOOK(0x44731C, BuildingClass_GetActionOnObject_Tunnel, 6)
{
	enum { RetActionSelf = 0x4472E7, Nothing = 0x0 };
	GET(BuildingClass*, pThis, ESI);

	bool FindSameTunnel = false;
	if (const auto nTunnelVec = HouseExt::GetTunnelVector(pThis->Type, pThis->Owner))
		FindSameTunnel = !nTunnelVec->Vector.empty();

	return FindSameTunnel ? RetActionSelf : Nothing;
}

DEFINE_HOOK(0x7014B9, TechnoClass_SetOwningHouse_Tunnel, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis)) {

		const auto nTunnelVec = HouseExt::GetTunnelVector(pBuilding->Type, pThis->Owner);

		if (!nTunnelVec || TunnelFuncs::FindSameTunnel(pBuilding))
			return 0x0;

		for (auto nPos = nTunnelVec->Vector.begin();
			nPos != nTunnelVec->Vector.end(); ++nPos) {
			TunnelFuncs::KillFootClass(*nPos, nullptr);
		}

		nTunnelVec->Vector.clear();
	}

	return 0x0;
}

DEFINE_OVERRIDE_HOOK(0x44A37F, BuildingClass_Mi_Selling_Tunnel_TryToPlacePassengers, 6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(CellStruct*, CellArr, EDI);
	GET(CellStruct, nCell, EDX);
	GET_STACK(int, nDev, 0x30);

	const auto nTunnelVec = HouseExt::GetTunnelVector(pThis->Type, pThis->Owner);

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

DEFINE_OVERRIDE_HOOK(0x44D8A7, BuildingClass_Mi_Unload_Tunnel, 6)
{
	GET(BuildingClass*, pThis, EBP);
	const auto nTunnelVec = HouseExt::GetTunnelVector(pThis->Type, pThis->Owner);

	if (!nTunnelVec || nTunnelVec->Vector.empty())
		return 0x0;

	TunnelFuncs::HandleUnload(&nTunnelVec->Vector, pThis);
	return 0x44DC84;
}


// Phobos hook on higher part of this for grinding
DEFINE_OVERRIDE_HOOK(0x43C326, BuildingClass_ReceivedRadioCommand_QueryCanEnter_Tunnel, 0xA)
{
	enum
	{
		RetRadioNegative = 0x43C3F0,
		ContineCheck = 0x43C4F8,
		RetRadioRoger = 0x43C535,
		ReturnStatic = 0x43C31A
	};

	GET(BuildingClass*, pThisBld, ESI);
	GET(TechnoClass*, pRecpt, EDI);

	const auto  nMission = pThisBld->GetCurrentMission();

	if (pThisBld->BState == BStateType::Construction ||
		nMission == Mission::Construction ||
		nMission == Mission::Selling)
		return RetRadioNegative;

	const auto pBldType = pThisBld->Type;
	const auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldType);
	const auto pRectpType = pRecpt->GetTechnoType();

	const bool isAmphibious = pRectpType->MovementZone == MovementZone::Amphibious
		|| pRectpType->MovementZone == MovementZone::AmphibiousCrusher
		|| pRectpType->MovementZone == MovementZone::AmphibiousDestroyer;

	if (!isAmphibious && pBldType->Naval != pRectpType->Naval)
		return RetRadioNegative;

	if ((pRectpType->BalloonHover || pRectpType->JumpJet)
		|| !pThisBld->HasPower
		|| !TechnoTypeExt::PassangersAllowed(pBldType, pRectpType)
		)
		return RetRadioNegative;

	if (pRecpt->CaptureManager && pRecpt->CaptureManager->IsControllingSomething())
		return RetRadioNegative;

	const bool IsTunnel = pBldTypeExt->TunnelType >= 0;
	const bool IsUnitAbsorber = pBldType->UnitAbsorb;
	const bool IsInfAbsorber = pBldType->InfantryAbsorb;
	const bool IsAbsorber = IsUnitAbsorber || IsInfAbsorber;

	if (!IsAbsorber && !IsTunnel && !pThisBld->HasFreeLink(pRecpt) && !Unsorted::ScenarioInit())
		return RetRadioNegative;

	const auto whatRept = pRecpt->WhatAmI();

	//tunnel check is taking predicate
	if (IsTunnel)
	{
		if (pThisBld->IsMindControlled())
			return RetRadioNegative;

		const auto pTunnelData = HouseExt::GetTunnelVector(pBldType, pThisBld->Owner);

		if (((int)pTunnelData->Vector.size() >= pTunnelData->MaxCap)
			|| (pBldType->SizeLimit < pRectpType->Size))
		{
			R->EBX(pBldType);
			return ContineCheck;
		}

		return RetRadioRoger;
	}

	//next is for absorbers
	if(IsAbsorber) {
		if ((IsUnitAbsorber && whatRept == UnitClass::AbsID) || (IsInfAbsorber && whatRept == InfantryClass::AbsID)) {
			if (pThisBld->Passengers.NumPassengers >= pBldType->Passengers
				|| pBldType->SizeLimit < pRectpType->Size) {
				R->EBX(pBldType);
				return ContineCheck;
			}

			return RetRadioRoger;
		}

		return RetRadioNegative;
	}

	R->EBX(pBldType);
	return ContineCheck;
}

DEFINE_OVERRIDE_HOOK(0x51ED8E, InfantryClass_GetActionOnObject_Tunnel, 6)
{
	enum
	{
		CanEnter = 0x51ED9Cu,
		CannotEnter = 0x51EE3Bu
	};

	//GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EAX);

	if (pTargetType->Passengers > 0 && !TechnoTypeExt::ExtMap.Find(pTargetType)->NoManualEnter)
		return CanEnter;

	bool Enterable = false;
	if (const auto pBuildingTarget = specific_cast<BuildingClass*>(pTarget))
	{
		Enterable = BuildingTypeExt::ExtMap.Find(pBuildingTarget->Type)->TunnelType >= 0;
	}

	return Enterable ? CanEnter :
		CannotEnter;
}

DEFINE_OVERRIDE_HOOK(0x51A2AD, InfantryClass_UpdatePosition_Tunnel, 9)
{
	enum { CanEnter = 0x51A396, CannotEnter = 0x51A488, Nothing = 0x0 };

	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	if (const auto nTunnelVec = HouseExt::GetTunnelVector(pBld->Type, pBld->Owner))
	{
		return TunnelFuncs::CanEnterTunnel(&nTunnelVec->Vector, pBld, pThis) ? CanEnter : CannotEnter;
	}

	return Nothing;
}

DEFINE_OVERRIDE_HOOK(0x44351A, BuildingClass_ActionOnObject_Tunnel, 6)
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

	return BuildingTypeExt::ExtMap.Find(pType)->TunnelType >= 0 ?
		MissionUnload : CheckOccupant;
}