#include <AbstractClass.h>
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
	static std::vector<std::unique_ptr<TunnelTypeClass>> Array;

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

std::vector<std::unique_ptr<TunnelTypeClass>> TunnelTypeClass::Array {};

// to prevent recompile everything when stuffs not really finished 
struct DummyHouseExt
{
	static DummyHouseExt* Get(HouseClass* pThis)
	{
		return (DummyHouseExt*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
	}

public:
	std::unordered_map<size_t , std::vector<FootClass*>> Tunnels {};
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

namespace Funcs
{
	void EnterTunnel(std::vector<FootClass*>* pTunnelData , BuildingClass* pTunnel, FootClass* pFoot)
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
	
	bool CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer)
	{
		if (pEnterer->SendCommand(RadioCommand::QueryCanEnter, pTunnel) != RadioCommand::AnswerPositive)
			return false;

		Funcs::EnterTunnel(pTunnelData, pTunnel, pEnterer);
		return true;
	}
}

DEFINE_OVERRIDE_HOOK(0x73A23F, UnitClass_UpdatePosition_Tunnel, 0x6)
{
	enum { Entered = 0x73A315, FailedToEnter = 0x73A796, Nothing = 0x0 };

	GET(UnitClass*, pThis, EBP);
	GET(BuildingClass*, pTarget, EBX);

	if(pThis->GetCurrentMission() != Mission::Eaten)
	  return Nothing;

	if(pThis->Destination != pTarget)
		return Nothing;

	const auto pTunnelData = Funcs::GetTunnel(pTarget, pTarget->Owner);
	if (!pTunnelData)
		return Nothing;

	return Funcs::CanEnterTunnel(pTunnelData, pTarget, pThis) ?
		Entered : FailedToEnter;
}