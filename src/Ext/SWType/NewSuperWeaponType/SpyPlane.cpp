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
			const auto PlaneIdxesSize = (int)PlaneIdxes.size();
			const auto nSize = IsEmpty ? 1 : PlaneIdxesSize;
			const auto pDefault = IsEmpty ? HouseExt::GetSpyPlane(pThis->Owner) : nullptr;

			for (auto idx = 0; idx < nSize; idx++)
			{
				const int Amount = idx >= PlaneCounts.size() ? 1 : PlaneCounts[idx];
				const Mission Mission = idx >= PlaneMissions.size() ? Mission::SpyplaneApproach : PlaneMissions[idx];
				const Rank Rank = idx >= PlaneRank.size() ? Rank::Rookie : PlaneRank[idx];

				TechnoExt::SendPlane(IsEmpty ? pDefault->ArrayIndex : PlaneIdxes[idx],
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

	char tempBuffer[32];
	INI_EX exINI(pINI);

	pData->SpyPlanes_TypeIndex.clear();
	pData->SpyPlanes_Count.clear();
	pData->SpyPlanes_Mission.clear();
	pData->SpyPlanes_Rank.clear();

	NullableIdx<AircraftTypeClass> reader;

	reader.Read(exINI, section, "SpyPlane.Count");
	if (reader.isset() || reader.Get() != -1)
	{
		pData->SpyPlanes_TypeIndex.push_back(reader.Get());

		Nullable<int> nIntDummy;
		nIntDummy.Read(exINI, section, "SpyPlane.Count");
		if (nIntDummy.isset()) {
			pData->SpyPlanes_Count.push_back(abs(nIntDummy.Get()));
		}

		Nullable<Mission> nMissionDummy;
		nMissionDummy.Read(exINI, section, "SpyPlane.Mission");
		if (nMissionDummy.isset()) {
			pData->SpyPlanes_Mission.push_back(nMissionDummy.Get());
		}

		Nullable<Rank> nRankDummy;
		nRankDummy.Read(exINI, section, "SpyPlane.Rank");
		if (nRankDummy.isset() && nRankDummy.Get() != Rank::Rookie) {
			pData->SpyPlanes_Rank.push_back(nRankDummy.Get());
		}
	}
	else
	{
		for (int i = 0; ; ++i)
		{
			NullableIdx<AircraftTypeClass> nTypeDummy;

			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "SpyPlane%d.Type", i);
			nTypeDummy.Read(exINI, section, tempBuffer);

			if (!nTypeDummy.isset() || nTypeDummy == -1) 
				break;

			pData->SpyPlanes_TypeIndex.push_back(nTypeDummy.Get());

			Valueable<int> nIntDummy{ 1 };

			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "SpyPlane%d.Count", i);
			nIntDummy.Read(exINI, section, tempBuffer);

			pData->SpyPlanes_Count.push_back(abs(nIntDummy.Get()));

			Valueable<Mission> nMissionDummy{ Mission::SpyplaneApproach };

			IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "SpyPlane%d.Mission", i);
			nMissionDummy.Read(exINI, section, tempBuffer);

			pData->SpyPlanes_Mission.push_back(nMissionDummy.Get());

			Valueable<Rank> nRankDummy { Rank::Rookie };
			nRankDummy.Read(exINI, section, "SpyPlane%s.Rank");
			pData->SpyPlanes_Rank.push_back(nRankDummy.Get());
		}
	}
}