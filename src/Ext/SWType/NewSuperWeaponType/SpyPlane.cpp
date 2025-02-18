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
	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW);

	if (pThis->IsCharged)
	{
		if (CellClass* pTarget = MapClass::Instance->GetCellAt(Coords))
		{
			const auto nDeferement = pData->SW_Deferment.Get(-1);

			if (nDeferement <= 0)
				SpyPlaneStateMachine::SendSpyPlane(pThis, pData, this, pTarget);
			else
				this->newStateMachine(nDeferement, Coords, pThis, pTarget);

			return true;
		}
		else
		{
			Debug::LogInfo("SpyPlane [{}] SW Invalid Target ! ", pThis->get_ID());
		}
	}

	return false;
}

void SW_SpyPlane::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::SpyPlane;
	// Defaults to Spy Plane values
	pData->SW_RadarEvent = false;

	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_SpyPlaneReady);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
	pData->CursorType = (int)MouseCursorType::SpyPlane;
}

void SW_SpyPlane::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);

	pData->SpyPlanes_TypeIndex.Read(exINI ,section , "SpyPlane.Type");
	pData->SpyPlanes_Count.Read(exINI, section, "SpyPlane.Count");
	pData->SpyPlanes_Mission.Read(exINI, section, "SpyPlane.Mission");
	pData->SpyPlanes_Rank.Read(exINI, section, "SpyPlane.Rank");
}

bool SW_SpyPlane::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void SpyPlaneStateMachine::Update()
{
	if (this->Finished()) {
		auto pData = this->GetTypeExtData();

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		auto const sound = pData->SW_ActivationSound.Get(-1);
		if (sound != -1) {
			VocClass::PlayGlobal(sound, Panning::Center, 1.0);
		}

		this->SendSpyPlane(this->Super, pData, this->Type, this->target);
	}
}

void SpyPlaneStateMachine::SendSpyPlane(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, CellClass* target)
{

	const auto Default = HouseExtData::GetSpyPlane(pSuper->Owner);

	const auto& PlaneIdxes = pData->SpyPlanes_TypeIndex;
	const auto& PlaneCounts = pData->SpyPlanes_Count;
	const auto& PlaneMissions = pData->SpyPlanes_Mission;
	const auto& PlaneRank = pData->SpyPlanes_Rank;

	const auto IsEmpty = PlaneIdxes.empty();
	const size_t nSize = IsEmpty ? 1 : PlaneIdxes.size();

	for (auto idx = 0u; idx < nSize; idx++)
	{
		const int Amount = idx >= PlaneCounts.size() ? 1 : PlaneCounts[idx];
		const Mission Mission = idx >= PlaneMissions.size() ? Mission::SpyplaneApproach : PlaneMissions[idx];
		const Rank Rank = idx >= PlaneRank.size() ? Rank::Rookie : PlaneRank[idx];
		const auto Plane = IsEmpty ? Default : PlaneIdxes[idx];

		if (!Plane || Plane->Strength == 0)
			continue;

		TechnoExtData::SendPlane(Plane,
			Amount,
			pSuper->Owner,
			Rank,
			Mission,
			target,
			nullptr);
	}
}
