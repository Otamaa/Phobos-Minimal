#include "SonarPulse.h"

#include <Utilities/Helpers.h>
#include <Misc/AresData.h>

std::vector<const char*> SW_SonarPulse::GetTypeString() const
{
	return { "SonarPulse" };
}

SuperWeaponFlags SW_SonarPulse::Flags(const SWTypeExt::ExtData* pData) const
{
	if (this->GetRange(pData).WidthOrRange > 0)
		return SuperWeaponFlags::NoEvent;

	return SuperWeaponFlags::None;
}

bool SW_SonarPulse::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	auto Detect = [pThis, pData](TechnoClass* const pTechno) -> bool
	{
		// is this thing affected at all?
		if (!pData->IsHouseAffected(pThis->Owner, pTechno->Owner))
		{
			return true;
		}

		if (!pData->IsTechnoAffected(pTechno))
		{
			return true;
		}

		auto& nTime = GetCloakSkipTimer(pTechno);

		auto const delay = MaxImpl(
			nTime.GetTimeLeft(), pData->Sonar_Delay.Get());

		nTime.Start(delay);

		// actually detect this
		if (pTechno->CloakState != CloakState::Uncloaked)
		{
			pTechno->Uncloak(true);
			pTechno->NeedsRedraw = true;
		}

		return true;
	};

	auto const range = this->GetRange(pData);

	if (range.WidthOrRange < 0)
	{
		// decloak everything regardless of ranges
		for (auto const pTechno : *TechnoClass::Array)
		{
			Detect(pTechno);
		}
	}
	else
	{
		// decloak everything in range
		Helpers::Alex::DistinctCollector<TechnoClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(Coords, range.WidthOrRange, range.Height, items);
		items.apply_function_for_each(Detect);

		// radar event only if this isn't full map sonar
		if (pData->SW_RadarEvent)
		{
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
		}
	}

	return true;
}

void SW_SonarPulse::Initialize(SWTypeExt::ExtData* pData)
{	// some defaults
	pData->SW_RadarEvent = false;

	pData->Sonar_Delay = 60;

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Stealth;
	pData->SW_AffectsHouse = AffectedHouse::Enemies;
	pData->SW_AffectsTarget = SuperWeaponTarget::Water;
	pData->SW_RequiresTarget = SuperWeaponTarget::Water;
	pData->SW_AIRequiresTarget = SuperWeaponTarget::Water;
}

void SW_SonarPulse::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);

	// full map detection?
	pData->Get()->Action = (GetRange(pData).WidthOrRange < 0) ? Action::None : (Action)AresNewActionType::SuperWeaponAllowed;
}

bool SW_SonarPulse::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

SWRange SW_SonarPulse::GetRange(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange{ 10 } :pData->SW_Range.Get();
}
