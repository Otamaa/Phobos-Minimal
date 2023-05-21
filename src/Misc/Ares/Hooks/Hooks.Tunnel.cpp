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

struct PipDrawData
{
	int PipIdx;
	int DrawCount;

	PipDrawData() :PipIdx { 0 }
		, DrawCount { 1 }
	{
	}

	PipDrawData(int nIdx, int nDrawCount) :PipIdx { nIdx }, DrawCount { nDrawCount }
	{ }

	bool operator==(const PipDrawData& nSec)
	{
		return false;
	}

	bool operator !=(const PipDrawData& nSec)
	{
		return !(*this == nSec);
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return
			Stm
			.Process(PipIdx)
			.Process(DrawCount)
			.Success()
			;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return
			Stm
			.Process(PipIdx)
			.Process(DrawCount)
			.Success()
			;
	}
};

bool NOINLINE FindSameTunnel(BuildingClass* pTunnel)
{
	const auto pOwner = pTunnel->Owner;
	const auto It = std::find_if_not(pOwner->Buildings.begin(), pOwner->Buildings.end(),
		[pTunnel](BuildingClass* pBld)
	{
		if (pBld->Health > 0 && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto nCurMission = pBld->CurrentMission;
			if (nCurMission != Mission::Construction && nCurMission != Mission::Selling && pTunnel != pBld)
			{
				const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
				const auto pBuildingTypeExt2 = BuildingTypeExt::ExtMap.Find(pTunnel->Type);

				if (pBuildingTypeExt->TunnelType == pBuildingTypeExt2->TunnelType)
				{
					return false;
				}
			}
		}

		return true;
	});

	//found new building
	return It != pOwner->Buildings.end();
}

void NOINLINE KillFootClass(FootClass* pFoot, TechnoClass* pKiller)
{
	if (auto pTeam = pFoot->Team)
		pTeam->RemoveMember(pFoot);

	pFoot->RegisterDestruction(pKiller);
	GameDelete<true, false>(pFoot);
}

void NOINLINE DestroyTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, TechnoClass* pKiller)
{
	if (FindSameTunnel(pTunnel))
		return;

	for (auto nPos = pTunnelData->begin(); nPos != pTunnelData->end(); ++nPos)
	{
		KillFootClass(*nPos, pKiller);
	}
}

NOINLINE HouseExt::ExtData::TunnelData* GetTunnelVector(HouseClass* pHouse, size_t nTunnelIdx)
{
	auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (nTunnelIdx >= pHouseExt->Tunnels.size())
	{
		while (pHouseExt->Tunnels.size() < TunnelTypeClass::Array.size())
		{
			pHouseExt->Tunnels.push_back({});

			const auto nMaxSize = TunnelTypeClass::Array[pHouseExt->Tunnels.size() - 1]->Passengers;
			pHouseExt->Tunnels.back().Vector.reserve(nMaxSize);
			pHouseExt->Tunnels.back().MaxCap = nMaxSize;
		}
	}

	return &pHouseExt->Tunnels[nTunnelIdx];
}

NOINLINE HouseExt::ExtData::TunnelData* GetTunnels(BuildingTypeClass* pBld, HouseClass* pHouse)
{
	const auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBld);

	if ((size_t)pBuildingTypeExt->TunnelType >= TunnelTypeClass::Array.size())
		return nullptr;

	return GetTunnelVector(pHouse, pBuildingTypeExt->TunnelType);
}

void NOINLINE EnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pFoot)
{
	pFoot->SetTarget(nullptr);
	pFoot->OnBridge = false;
	pFoot->unknown_C4 = 0;
	pFoot->GattlingValue = 0;
	pFoot->SetGattlingStage(0);

	if (auto const pCapturer = pFoot->MindControlledBy)
	{
		if (const auto pCmanager = pCapturer->CaptureManager)
		{
			pCmanager->FreeUnit(pFoot);
		}
	}

	if (!pFoot->Limbo())
	{
		Debug::Log("Techno[%s] Trying to enter Tunnel[%s] but failed ! \n", pFoot->get_ID(), pTunnel->get_ID());
		return;
	}

	VocClass::PlayIndexAtPos(pTunnel->Type->EnterTransportSound, pTunnel->Location);

	pFoot->Undiscover();

	if (pFoot->GetCurrentMission() == Mission::Hunt)
		pFoot->AbortMotion();

	pTunnelData->push_back(pFoot);
}

bool NOINLINE CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer)
{
	if (pEnterer->SendCommand(RadioCommand::QueryCanEnter, pTunnel) != RadioCommand::AnswerPositive)
		return false;

	EnterTunnel(pTunnelData, pTunnel, pEnterer);
	return true;
}

static std::vector<int> PipData;

#pragma optimize("", off )
NOINLINE std::vector<int>* PopulatePassangerPIPData(TechnoClass* pThis, TechnoTypeClass* pType, bool& Fail)
{
	int nPassangersTotal = pType->GetPipMax();
	if (nPassangersTotal < 0)
		nPassangersTotal = 0;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto pBld = specific_cast<BuildingClass*>(pThis);

	PipData.clear();

	if (pBld)
	{
		const HouseExt::ExtData::TunnelData* pTunnelData = GetTunnels(pBld->Type, pThis->Owner);
		const bool Absorber = pBld->Absorber();

		if (!pTunnelData)
		{
			if (pThis->Passengers.NumPassengers >= nPassangersTotal)
			{
				Fail = true;
				return &PipData;
			}

			PipData.resize(nPassangersTotal);

			int nCargoSize = 0;
			for (auto pPassenger = pThis->Passengers.GetFirstPassenger();
				pPassenger;
				pPassenger = generic_cast<FootClass*>(pPassenger->NextObject))
			{
				const auto pPassengerType = pPassenger->GetTechnoType();

				auto nSize = pTypeExt->Passengers_BySize.Get(!Absorber) ? (int)pPassengerType->Size : 1 ;
				if (nSize <= 0)
					nSize = 1;

				int nPip = 1;
				if (Is_Infantry(pPassenger))
					nPip = (int)(static_cast<InfantryTypeClass*>(pPassengerType)->Pip);
				else if (Is_Unit(pPassenger))
					nPip = 5;

				//fetch first cargo size and change the pip
				PipData[nCargoSize] = nPip;
				for (int i = nSize - 1; i > 0; --i)
				{ //check if the extra size is there and increment it to 
			   // total size
					PipData[nCargoSize + i] = 3;     //set extra size to pip index 3
				}

				nCargoSize += nSize;
			}

			return &PipData;
		}
		else
		{
			const int nTotal = pTunnelData->MaxCap > nPassangersTotal ? nPassangersTotal : pTunnelData->MaxCap;
			PipData.resize(nTotal);

			for (size_t i = 0; i < pTunnelData->Vector.size(); ++i)
			{
				auto const& pContent = pTunnelData->Vector[i];

				int nPip = 1;
				if (Is_Infantry(pContent))
					nPip = (int)(static_cast<InfantryClass*>(pContent)->Type->Pip);
				else if (Is_Unit(pContent))
					nPip = 4;

				PipData[i] = nPip;
			}

			return &PipData;
		}
	}
	else
	{
		if (pThis->Passengers.NumPassengers >= nPassangersTotal)
		{
			Fail = true;
			return &PipData;
		}

		PipData.resize(nPassangersTotal);

		int nCargoSize = 0;
		for (auto pPassenger = pThis->Passengers.GetFirstPassenger();
			pPassenger; 
			pPassenger = generic_cast<FootClass*>(pPassenger->NextObject))
		{
			const auto pPassengerType = pPassenger->GetTechnoType();

			auto nSize = pTypeExt->Passengers_BySize.Get(true) ? (int)pPassengerType->Size : 1 ;
			if (nSize <= 0)
				nSize = 1;

			int nPip = 1;
			if (Is_Infantry(pPassenger))
				nPip = (int)(static_cast<InfantryTypeClass*>(pPassengerType)->Pip);
			else if (Is_Unit(pPassenger))
				nPip = 5;

			//fetch first cargo size and change the pip
			PipData[nCargoSize] = nPip;
			for (int i = nSize - 1; i > 0; --i) { //check if the extra size is there and increment it to 
												// total size
				PipData[nCargoSize + i] = 3;     //set extra size to pip index 3
			}

			nCargoSize += nSize;
		}

		return &PipData;
	}
}

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
	const auto pData = PopulatePassangerPIPData(pThis, pType, fail);

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
#pragma optimize("", on )

std::pair<bool,FootClass*> NOINLINE UnlimboOne(std::vector<FootClass*>* pVector, BuildingClass* pTunnel, DWORD Where)
{
	auto pPassenger = *std::prev(pVector->end());
	pPassenger->OnBridge = pTunnel->OnBridge;
	auto nCoord = pTunnel->GetCoords();
	pPassenger->SetLocation(nCoord);

	bool Succeeded = false;
	++Unsorted::IKnowWhatImDoing();
	Succeeded = pPassenger->Unlimbo(nCoord, (DirType)((((short)pTunnel->PrimaryFacing.Current().Raw >> 7) + 1) >> 1));
	--Unsorted::IKnowWhatImDoing();

	pVector->pop_back();

	if (Succeeded) {
		pPassenger->Scatter(CoordStruct::Empty, true, false);
		return { true,  pPassenger };
	}

	return { false,  pPassenger };
}

DEFINE_OVERRIDE_HOOK(0x442DF2, BuildingClass_Demolish_Tunnel, 6)
{
	GET_STACK(AbstractClass*, pKiller, 0x90);
	GET(BuildingClass*, pTarget, EDI);

	if (auto pTunnelData = GetTunnels(pTarget->Type, pTarget->Owner))
	{
		DestroyTunnel(&pTunnelData->Vector, pTarget, generic_cast<TechnoClass*>(pKiller));
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71A995, TemporalClass_Update_Tunnel, 5)
{
	GET(TemporalClass*, pThis, ESI);
	GET(BuildingClass*, pTarget, EBP);

	if (const auto pTunnelData = GetTunnels(pTarget->Type, pTarget->Owner))
		DestroyTunnel(&pTunnelData->Vector, pTarget, pThis->Owner);

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x73A23F, UnitClass_UpdatePosition_Tunnel, 0x6)
{
	enum { Entered = 0x73A315, FailedToEnter = 0x73A796, Nothing = 0x0 };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pTarget, EBX);

	if (pThis->GetCurrentMission() != Mission::Eaten)
		return Nothing;

	if (pThis->Destination != pTarget)
		return Nothing;

	const auto pTunnelData = GetTunnels(pTarget->Type, pTarget->Owner);
	if (!pTunnelData)
		return Nothing;

	return CanEnterTunnel(&pTunnelData->Vector, pTarget, pThis) ?
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

	return BuildingTypeExt::ExtMap.Find(pThis->Type)->TunnelType > -1 ? ProcessTechnoClassRadio : DoNothing;
}

DEFINE_OVERRIDE_HOOK(0x44731C, BuildingClass_GetActionOnObject_Tunnel, 6)
{
	enum { RetActionSelf = 0x4472E7, Nothing = 0x0 };
	GET(BuildingClass*, pThis, ESI);

	bool FindSameTunnel = false;
	if (const auto nTunnelVec = GetTunnels(pThis->Type, pThis->Owner))
		FindSameTunnel = !nTunnelVec->Vector.empty();

	return FindSameTunnel ? RetActionSelf : Nothing;
}

DEFINE_OVERRIDE_HOOK(0x44A37F, BuildingClass_Mi_Selling_Tunnel_TryToPlacePassengers, 6)
{
	GET(BuildingClass*, pThis, EBP);
	GET(CellStruct*, CellArr, EDX);
	GET(CellStruct, nCell, EDI);
	GET_STACK(int, nDev, 0x30);

	const auto nTunnelVec = GetTunnels(pThis->Type, pThis->Owner);

	if (!nTunnelVec || FindSameTunnel(pThis))
		return 0x0;

	int nOffset = 0;
	while(std::begin(nTunnelVec->Vector) != std::end(nTunnelVec->Vector))
	{
		nOffset++;
		auto const& [status, pPtr] = UnlimboOne(&nTunnelVec->Vector, pThis, (nCell.X + CellArr[nOffset % nDev].X) | ((nCell.Y + CellArr[nOffset % nDev].Y) << 16));
		if (!status){
			KillFootClass(pPtr, nullptr);
		}
	}

	return 0x0;
}

bool NOINLINE UnloadOnce(FootClass* pFoot, BuildingClass* pTunnel)
{
	const auto facing = (((((short)pTunnel->PrimaryFacing.Current().Raw + 0x8000) >> 12) + 1) >> 1);
	const auto Loc = pTunnel->GetMapCoords();

	int nOffset = 0;
	bool IsLessThanseven = true;
	bool Succeeded = false;
	CellStruct nResult;
	CellClass* CurrentAdj = nullptr;
	CellClass* NextCell = nullptr;
	int nFacing = 0;

	while (true)
	{
		nFacing = (facing + nOffset) & 7;
		const CellStruct tmpCoords = CellSpread::AdjacentCell[nFacing];
		nResult = tmpCoords + Loc;
		CellStruct next = tmpCoords + nResult;

		CurrentAdj = MapClass::Instance->GetCellAt(nResult);
		NextCell = MapClass::Instance->GetCellAt(next);

		const auto nLevel = pTunnel->GetCellLevel();
		const auto nOccupyResult = pFoot->IsCellOccupied(CurrentAdj, nFacing, nLevel, nullptr, true);
		const auto nNextnOccupyResult = pFoot->IsCellOccupied(NextCell, nFacing, nLevel, nullptr, true);

		if ((!(int)nNextnOccupyResult) &&
			(!IsLessThanseven || (!(int)nOccupyResult)) &&
			(CurrentAdj->Flags & CellFlags::BridgeHead) == CellFlags::Empty)
			break;

		bool bHere = false;
		if (nOffset != 7)
			bHere = IsLessThanseven;

		++nOffset;
		IsLessThanseven = bHere;

		if (nOffset >= 16)
			return false;
	}

	const auto pFootType = pFoot->GetTechnoType();
	++Unsorted::IKnowWhatImDoing();

	CoordStruct nResultC = CellClass::Cell2Coord(nResult);
	if (pFoot->WhatAmI() == AbstractType::Infantry)
	{
		nResultC = MapClass::Instance->PickInfantrySublocation(nResultC, false);
	}
	else
	{
		const auto nNearby = MapClass::Instance->NearByLocation(nResult, pFootType->SpeedType, -1, MovementZone::None, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
		nResultC = CellClass::Cell2Coord(nNearby);
	}

	Succeeded = pFoot->Unlimbo(nResultC, DirType(32 * (nFacing & 0x3FFFFFF)));
	--Unsorted::IKnowWhatImDoing();

	if (Succeeded)
	{
		VocClass::PlayIndexAtPos(pTunnel->Type->LeaveTransportSound, pTunnel->Location);
		pFoot->QueueMission(Mission::Move, false);
		pFoot->SetDestination(IsLessThanseven ? NextCell : CurrentAdj, true);
		return true;
	}

	return false;
}

void NOINLINE HandleUnload(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel)
{
	auto nPos = pTunnelData->end();

	if (pTunnelData->begin() != nPos) {
		UnloadOnce(*std::prev(nPos), pTunnel);
		pTunnelData->pop_back();
	}
}

DEFINE_OVERRIDE_HOOK(0x44D8A7, BuildingClass_Mi_Unload_Tunnel, 6)
{
	GET(BuildingClass*, pThis, EBP);
	const auto nTunnelVec = GetTunnels(pThis->Type, pThis->Owner);

	if (!nTunnelVec || nTunnelVec->Vector.empty())
		return 0x0;

	HandleUnload(&nTunnelVec->Vector, pThis);
	return 0x44DC84;
}

//NOT DONE YET
DEFINE_OVERRIDE_HOOK(0x43C326, BuildingClass_ReceivedRadioCommand_QueryCanEnter_Tunnel, 0xA)
{
	enum
	{
		RetRadioNegative = 0x43C3F0,
		ContineCheck = 0x43C4F8,
		RetRadioRoger = 0x43C535
	};

	GET(BuildingClass*, pThis, ESI);
	GET(TechnoClass*, pRecepient, EDI);

	auto const nMission = pThis->GetCurrentMission();

	if (nMission == Mission::Construction || nMission == Mission::Selling || pThis->BState == BStateType::Construction)
		return RetRadioNegative;

	auto const pType = pThis->Type;
	auto const pRecipientType = pRecepient->GetTechnoType();
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pRecipientType->MovementZone != MovementZone::Amphibious && pType->Naval != pRecipientType->Naval)
		return RetRadioNegative;

	if ((pRecipientType->BalloonHover || pRecipientType->JumpJet)
		|| !pThis->HasPower
		|| !TechnoTypeExt::PassangersAllowed(pType, pRecipientType))
		return RetRadioNegative;

	const bool IsUnitAbsorb = pType->UnitAbsorb;
	const bool IsInfAbsorb = pType->InfantryAbsorb;
	bool Absorber = false;
	const bool Istunnel = pTypeExt->TunnelType >= 0;

	if (IsUnitAbsorb || IsInfAbsorb)
	{
		Absorber = true;
	}
	else if (!Istunnel)
	{
		if (!pThis->HasFreeLink(pRecepient) && !Unsorted::IKnowWhatImDoing())
		{
			return RetRadioNegative;
		}
		else
		{
			R->EBX(pType);
			return ContineCheck;
		}
	}

	if (auto const pCapture = pRecepient->CaptureManager)
	{
		if (pCapture->IsControllingSomething())
		{
			return RetRadioNegative;
		}
	}

	if (Istunnel)
	{
		const auto nTunnelVec = GetTunnels(pType, pThis->Owner);

		// out of limit
		if ((int)nTunnelVec->Vector.size() > nTunnelVec->MaxCap)
		{
			R->EBX(pType);
			return ContineCheck;
		}

		if (pType->SizeLimit < pRecipientType->Size)
		{
			R->EBX(pType);
			return ContineCheck;
		}

		return RetRadioRoger;
	}

	if (!Absorber)
	{
		R->EBX(pType);
		return ContineCheck;
	}
	else
	{
		auto const pRecpWhat = pRecepient->WhatAmI();
		if ((pRecpWhat == AbstractType::Infantry && IsUnitAbsorb)
			|| (pRecpWhat == AbstractType::Infantry && IsInfAbsorb))
		{
			if (pThis->Passengers.NumPassengers >= pType->Passengers)
			{
				R->EBX(pType);
				return ContineCheck;
			}

			if (pType->SizeLimit < pRecipientType->Size)
			{
				R->EBX(pType);
				return ContineCheck;
			}

			return RetRadioRoger;
		}

		return RetRadioNegative;
	}
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

	if (const auto nTunnelVec = GetTunnels(pBld->Type, pBld->Owner))
	{
		return CanEnterTunnel(&nTunnelVec->Vector, pBld, pThis) ? CanEnter : CannotEnter;
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