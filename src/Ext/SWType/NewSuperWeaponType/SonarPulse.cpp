#include "SonarPulse.h"

#include <Utilities/Helpers.h>

#include <Ext/Techno/Body.h>

std::vector<const char*> SW_SonarPulse::GetTypeString() const
{
	return { "SonarPulse" };
}

SuperWeaponFlags SW_SonarPulse::Flags(const SWTypeExtData* pData) const
{
	if (this->GetRange(pData).WidthOrRange > 0)
		return SuperWeaponFlags::NoEvent;

	return SuperWeaponFlags::None;
}

bool SW_SonarPulse::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);

	const auto nDeferement = pData->SW_Deferment.Get(-1);

	if (nDeferement <= 0)
		SonarPulseStateMachine::SendSonarPulse(pThis, pData, this, Coords);
	else
		this->newStateMachine(nDeferement, Coords, pThis);

	return true;
}

void SW_SonarPulse::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action(AresNewActionType::SuperWeaponAllowed);
	// some defaults
	pData->SW_RadarEvent = false;

	pData->Sonar_Delay = 60;

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Stealth;
	pData->SW_AffectsHouse = AffectedHouse::Enemies;
	pData->SW_AffectsTarget = SuperWeaponTarget::Water;
	pData->SW_RequiresTarget = SuperWeaponTarget::Water;
	pData->SW_AIRequiresTarget = SuperWeaponTarget::Water;
}

void SW_SonarPulse::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	pData->Sonar_Delay = pINI->ReadInteger(section, "SonarPulse.Delay", pData->Sonar_Delay);

	// full map detection?
	pData->This()->Action = (GetRange(pData).WidthOrRange < 0) ? Action::None : (Action)AresNewActionType::SuperWeaponAllowed;
}

bool SW_SonarPulse::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

SWRange SW_SonarPulse::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange{ 10 } :pData->SW_Range.Get();
}

void SonarPulseStateMachine::Update()
{
	if (this->Finished())
	{
		SendSonarPulse(this->Super, this->GetTypeExtData(), this->Type, this->Coords);
	}
}

void SonarPulseStateMachine::SendSonarPulse(SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, const CellStruct& loc)
{
	pData->PrintMessage(pData->Message_Activate, pSuper->Owner);

	auto const sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1) {
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	auto const range = pNewType->GetRange(pData);

	SonarPulseStateMachine::ApplySonarPulse(pSuper, loc, range);
}

void SonarPulseStateMachine::ApplySonarPulse(SuperClass* pSuper, const CellStruct& Coords, const SWRange& range)
{
	const auto pData = SWTypeExtContainer::Instance.Find(pSuper->Type);
	auto Detect = [pSuper, pData](TechnoClass* const pTechno) -> bool
		{
			// is this thing affected at all?
			if (!pData->IsHouseAffected(pSuper->Owner, pTechno->Owner))
			{
				return true;
			}

			if (!pData->IsTechnoAffected(pTechno))
			{
				return true;
			}

			auto& nTime = TechnoExtContainer::Instance.Find(pTechno)->CloakSkipTimer;

			nTime.Start(MaxImpl(
				nTime.GetTimeLeft(), pData->Sonar_Delay.Get()));

			// actually detect this
			if (pTechno->CloakState != CloakState::Uncloaked)
			{
				pTechno->Uncloak(true);
				pTechno->NeedsRedraw = true;
			}

			return true;
	};


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
		if (pData->SW_RadarEvent) {
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
		}
	}
}
