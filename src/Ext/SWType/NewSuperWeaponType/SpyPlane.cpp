#include "SpyPlane.h"

#include <Ext/Techno/Body.h>

std::vector<const char*> SW_SpyPlane::GetTypeString() const
{
	return { "Airstrike" };
}

bool SW_SpyPlane::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::SpyPlane);
}

bool SW_SpyPlane::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW);

	if (pThis->IsCharged)
	{
		if (CellClass* pTarget = MapClass::Instance->GetCellAt(Coords))
		{
			const auto& PlaneIdxes = pData->SpyPlanes_TypeIndex;
			const auto& PlaneCounts = pData->SpyPlanes_Count;
			const auto& PlaneMissions = pData->SpyPlanes_Mission;
			const auto& PlaneRank = pData->SpyPlanes_Rank;

			const auto IsEmpty = PlaneIdxes.empty();
			const size_t nSize = IsEmpty ? 1 : PlaneIdxes.size();
			const int Default = HouseExt::GetSpyPlane(pThis->Owner)->ArrayIndex;

			for (auto idx = 0u; idx < nSize; idx++)
			{
				const int Amount = idx >= PlaneCounts.size() ? 1 : PlaneCounts[idx];
				const Mission Mission = idx >= PlaneMissions.size() ? Mission::SpyplaneApproach : PlaneMissions[idx];
				const Rank Rank = idx >= PlaneRank.size() ? Rank::Rookie : PlaneRank[idx];

				TechnoExt::SendPlane(IsEmpty ? Default : PlaneIdxes[idx]->ArrayIndex,
					Amount, 
					pThis->Owner,
					Rank, 
					Mission,
					pTarget, 
					nullptr);
			}

			return true;
		}
		else
		{
			Debug::Log("SpyPlane [%s] SW Invalid Target ! \n", pThis->get_ID());
		}
	}

	return false;
}

void SW_SpyPlane::Initialize(SWTypeExt::ExtData* pData)
{
	// Defaults to Spy Plane values

	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_SpyPlaneReady);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = (int)MouseCursorType::SpyPlane;
}

void SW_SpyPlane::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->Get()->ID;

	INI_EX exINI(pINI);

	pData->SpyPlanes_TypeIndex.Read(exINI ,section , "SpyPlane.Type");
	pData->SpyPlanes_Count.Read(exINI, section, "SpyPlane.Count");
	pData->SpyPlanes_Mission.Read(exINI, section, "SpyPlane.Mission");
	pData->SpyPlanes_Rank.Read(exINI, section, "SpyPlane.Rank");
}