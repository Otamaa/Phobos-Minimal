#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>
#include <Misc/AresData.h>

#include <HoverLocomotionClass.h>

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

struct TunnelTypeClass
{
	static std::vector<TunnelTypeClass> Array;

public:
	std::string Name;

public:

	static int Find(std::string const& ID)
	{
		const auto It = std::find_if(Array.begin(), Array.end(), [&](auto const& pData) { return pData->Name == ID; });

		if (It == Array.end())
			return -1;

		return std::distance(Array.begin(), It);
	}
};

std::vector<<TunnelTypeClass> TunnelTypeClass::Array {};

// to prevent recompile everything when stuffs not really finished 
struct DummyHouseExt
{
	static DummyHouseExt* Get(HouseClass* pThis)
	{
		return (DummyHouseExt*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
	}

public:
	std::unordered_map<size_t, std::vector<FootClass*>> Tunnels {};
};

struct DummyTechnoTypeExt
{
	static DummyTechnoTypeExt* Get(TechnoTypeClass* pThis)
	{
		return (DummyTechnoTypeExt*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
	}

};

struct DummyBuildingTypeExt
{
	static DummyBuildingTypeExt* Get(BuildingTypeClass* pThis)
	{
		return (DummyBuildingTypeExt*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
	}

	ValueableIdx<TunnelTypeClass> TunnelType {};
	ValueableIdx<VocClass> EnterTunnelSound {};
	ValueableIdx<VocClass> ExitTunnelSound {};
};

struct DummyTechnoExt
{
	static DummyTechnoExt* Get(TechnoClass* pThis)
	{
		return (DummyTechnoExt*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
	}


};

struct PipDrawData
{
	int PipIdx;
	int DrawCount;
	bool IsActive;

	PipDrawData() :PipIdx { 0 }
		, DrawCount { 1 }
		, IsActive { false }
	{
	}

	PipDrawData(int nIdx, int nDrawCount, bool bIsActive) :PipIdx { nIdx }
		, DrawCount { nDrawCount }
		, IsActive { bIsActive }
	{
	}

	bool operator==(const PipDrawData& nSec)
	{
		return PipIdx == nSec.PipIdx && DrawCount == nSec.DrawCount && IsActive == nSec.IsActive;
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
			.Process(IsActive)
			.Success()
			;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return
			Stm
			.Process(PipIdx)
			.Process(DrawCount)
			.Process(IsActive)
			.Success()
			;
	}
};

namespace Funcs
{
	void EnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pFoot)
	{
		const auto pTunnelTypeExt = DummyBuildingTypeExt::Get(pTunnel->Type);

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

		const auto nSound = pTunnelTypeExt->EnterTunnelSound;
		if (nSound >= 0)
		{
			const auto nCoord = pTunnel->Location;
			VocClass::PlayIndexAtPos(nSound, nCoord);
		}

		if (!pFoot->Limbo())
			Debug::Log("Techno[%s] Trying to enter Tunnel[%s] but failed ! \n", pFoot->get_ID(), pTunnel->get_ID());

		pFoot->Undiscover();

		const auto nCurMission = pFoot->GetCurrentMission();
		if (nCurMission == Mission::Hunt)
			pFoot->AbortMotion();

		pTunnelData->push_back(pFoot);
	}

	std::vector<FootClass*>* GetTunnel(BuildingClass* pBldCandidate, HouseClass* pOwner)
	{
		const auto pExt = DummyBuildingTypeExt::Get(pBldCandidate->Type);
		if (pExt->TunnelType.ToUnsigned() >= TunnelTypeClass::Array.size())
			return nullptr;

		const auto pHouseExt = DummyHouseExt::Get(pOwner);
		return &pHouseExt->Tunnels[pExt->TunnelType];
	}

	Iterator<PipDrawData> PopulatePassangerPIPData(TechnoClass* pThis, TechnoTypeClass* pType)
	{
		const int nPassangersTotal = pType->Passengers;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		static std::vector<PipDrawData> PipData;
		const bool building = Is_Building(pThis);
		auto nSlotLeft = nPassangersTotal;
		auto pPassenger = pThis->Passengers.GetFirstPassenger();

		if (building)
		{
			const auto pBld = static_cast<BuildingClass*>(pThis);
			const auto pTunnelData = GetTunnel(pBld, pThis->Owner);
			const bool Absorber = static_cast<BuildingClass*>(pThis)->Absorber();

			if (!pTunnelData)
			{
				PipData.clear();
				PipData.resize(nPassangersTotal);

				for (int i = 0; i < pThis->Passengers.NumPassengers; ++i)
				{
					if (!pPassenger)
						break;

					const auto pExt = TechnoExt::ExtMap.Find(pPassenger);

					auto nSize = (int)(pExt->Type->Size - 1.0);
					if (nSize <= 0)
						nSize = 1;

					int nPip = 1;
					if (Is_Infantry(pPassenger))
						nPip = (int)(static_cast<InfantryTypeClass*>(pExt->Type)->Pip);
					else if (Is_Unit(pPassenger))
						nPip = 5;

					int nSizeCount = Absorber ? 1 : nSize;
					nSlotLeft -= nSizeCount;
					PipData[i] = { nPip , nSizeCount, true };

					pPassenger = generic_cast<FootClass*>(pPassenger->NextObject);
				}
			}
			else
			{
				PipData.clear();
				PipData.resize(pTunnelData->size());

				int nIdx = 0;
				std::for_each(pTunnelData->begin(), pTunnelData->end(), [&](FootClass* pContent)
				{
					int nPip = 1;
					if (Is_Infantry(pContent))
						nPip = (int)(static_cast<InfantryClass*>(pContent)->Type->Pip);
					else if (Is_Unit(pContent))
						nPip = 4;

					PipData[nIdx] = { nPip , 1, true };
					++nIdx;
				});

				return PipData;
			}
		}
		else
		{
			PipData.clear();
			PipData.resize(nPassangersTotal);

			for (int i = 0; i < pThis->Passengers.NumPassengers; ++i)
			{
				if (!pPassenger)
					break;

				const auto pExt = TechnoExt::ExtMap.Find(pPassenger);

				auto nSize = (int)(pExt->Type->Size - 1.0);
				if (nSize <= 0)
					nSize = 1;

				int nPip = 1;
				if (Is_Infantry(pPassenger))
					nPip = (int)(static_cast<InfantryTypeClass*>(pExt->Type)->Pip);
				else if (Is_Unit(pPassenger))
					nPip = 5;

				nSlotLeft -= nSize;
				PipData[i] = { nPip  , pTypeExt->Passengers_BySize.Get() ? nSize : 1 , true };

				pPassenger = generic_cast<FootClass*>(pPassenger->NextObject);
			}
		}

		if (nSlotLeft > 0)
			PipData[pThis->Passengers.NumPassengers] = { 0 , nSlotLeft , true };

		return PipData;
	}

	bool CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer)
	{
		if (pEnterer->SendCommand(RadioCommand::QueryCanEnter, pTunnel) != RadioCommand::AnswerPositive)
			return false;

		Funcs::EnterTunnel(pTunnelData, pTunnel, pEnterer);
		return true;
	}

	void DestroyTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, TechnoClass* pKiller)
	{
		if (pTunnelData->empty())
			return;

		const auto pOwner = pTunnel->Owner;
		const auto It = std::find_if_not(pOwner->Buildings.begin(), pOwner->Buildings.end(), [pTunnel](BuildingClass* pBld)
{

	if (pBld->Health > 0 && !pBld->InLimbo && pBld->IsOnMap)
	{
		const auto nCurMission = pBld->CurrentMission;
		if (nCurMission != Mission::Construction && nCurMission != Mission::Selling && pTunnel != pBld)
		{
			const auto pThatExt = DummyBuildingTypeExt::Get(pBld->Type);
			const auto pThisExt = DummyBuildingTypeExt::Get(pTunnel->Type);
			if (pThatExt->TunnelType == pThisExt->TunnelType)
			{
				return false;
			}
		}
	}

	return true;
		});


		if (It != pOwner->Buildings.end())
		{
			std::for_each(pTunnelData->begin(), pTunnelData->end(), [=](FootClass* pFoot)
 {

	 if (auto pTeam = pFoot->Team)
		 pTeam->RemoveMember(pFoot);

	 pFoot->RegisterDestruction(pKiller);
	 pFoot->UnInit();
			});
		}
	}
}

#include <TemporalClass.h>

DEFINE_OVERRIDE_HOOK(0x442DF2, BuildingClass_Demolish_Tunnel, 6)
{
	GET_STACK(AbstractClass*, pKiller, 0x90);
	GET(BuildingClass*, pTarget, EDI);

	if (const auto pTunnelData = Funcs::GetTunnel(pTarget, pTarget->Owner))
	{
		TechnoClass* pTKiller = generic_cast<TechnoClass*>(pKiller);
		Funcs::DestroyTunnel(pTunnelData, pTarget, pTKiller);
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x71A995, TemporalClass_Update_Tunnel, 5)
{
	GET(TemporalClass*, pThis, ESI);
	GET(BuildingClass*, pTarget, EBP);

	if (const auto pTunnelData = Funcs::GetTunnel(pTarget, pTarget->Owner))
		Funcs::DestroyTunnel(pTunnelData, pTarget, pThis->Owner);

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

	const auto pTunnelData = Funcs::GetTunnel(pTarget, pTarget->Owner);
	if (!pTunnelData)
		return Nothing;

	return Funcs::CanEnterTunnel(pTunnelData, pTarget, pThis) ?
		Entered : FailedToEnter;
}

DEFINE_OVERRIDE_HOOK(0x741CE5, UnitClass_SetDestination_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	const auto pExt = DummyBuildingTypeExt::Get(pBuilding->Type);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;

	if (!canbeDestination)
		canbeDestination = pExt->TunnelType >= 0;

	return canbeDestination ? 0x741CF5 : 0x741D12;
}

DEFINE_OVERRIDE_HOOK(0x73F606, UnitClass_IsCellOccupied_Tunnel, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	const auto pExt = DummyBuildingTypeExt::Get(pBuilding->Type);

	bool canbeDestination = pBuilding->Type->UnitAbsorb;

	if (!canbeDestination)
		canbeDestination = pExt->TunnelType >= 0;

	return canbeDestination ? 0x73F616 : 0x73F628;
}

struct DrawPipDataStruct
{
	DWORD Array; //0
	int Y; // 4
	SHPStruct* Shape; //8
	int Number; //c
};

DEFINE_OVERRIDE_HOOK(0x709D38, TechnoClass_DrawPipscale_Passengers, 7)
{
	GET(TechnoClass* const, pThis, EBP);
	GET(TechnoTypeClass*, pType, EAX);

	if (pType->PipScale != PipScale::Passengers)
		return 0x70A083;

	GET(int, nBracketPosDeltaY, ESI);
	//Databunde + 0x8
	GET_STACK(SHPStruct*, pShp, STACK_OFFS(0x74, 0x60) + 0X8);
	//GET_STACK(DrawPipDataStruct, nDataBundle, STACK_OFFS(0x74, 0x60) + 0X8);
	GET_STACK(RectangleStruct*, pRect, STACK_OFFS(0x74, -0xC));
	GET_STACK(Point2D, nPoint, STACK_OFFS(0x74, 0x24));
	GET_STACK(int, nBracketPosDeltaX, STACK_OFFS(0x74, 0x1C));

	Point2D nPos = { nPoint.X ,nPoint.Y };

	for (auto const& [FrameIdx, drawcount , active] : Funcs::PopulatePassangerPIPData(pThis, pType))
	{
		if (!active)
			continue;

		for (int s = 0; s < drawcount; ++s)
		{
			DSurface::Temp->DrawSHP
			(FileSystem::PALETTE_PAL,
				pShp,
				FrameIdx,
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

			auto nX = nBracketPosDeltaX + nPoint.X;
			auto nY = nBracketPosDeltaY + nPoint.Y;
			nPos.X += nBracketPosDeltaX;
			nPos.Y += nBracketPosDeltaY;

			if (pType->Gunner)
			{
				nPos.X += nX + nBracketPosDeltaX;
				nPos.Y += nY + nBracketPosDeltaY;
			}
		}
	}

	return 0x70A4EC;
}