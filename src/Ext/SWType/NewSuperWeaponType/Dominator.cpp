#include "Dominator.h"

SuperClass* SW_PsychicDominator::CurrentPsyDom = nullptr;

std::vector<const char*> SW_PsychicDominator::GetTypeString() const
{
	return { "NewDominator" };
}

bool SW_PsychicDominator::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::PsychicDominator);
}

SuperWeaponFlags SW_PsychicDominator::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_PsychicDominator::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		// we do not use PsyDom::Start() here. instead, we set a global state and
		// let the state machine take care of everything.
		SW_PsychicDominator::CurrentPsyDom = pThis;
		this->newStateMachine(Coords, pThis);
	}

	return true;
}

bool SW_PsychicDominator::AbortFire(SuperClass* pSW, bool IsPlayer)
{
	// be one with Yuri! and only one.
	if (PsyDom::IsActive())
	{
		if (IsPlayer)
		{
			SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW->Type);
			pData->PrintMessage(pData->Message_Abort, pSW->Owner);
		}
		return true;
	}
	return false;
}

void SW_PsychicDominator::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::PsychicDominator;
	// Defaults to PsychicDominator values
	pData->Dominator_FirstAnimHeight = 750;
	pData->Dominator_SecondAnimHeight = 0;
	pData->Dominator_Ripple = true;
	pData->Dominator_Capture = true;
	pData->Dominator_CaptureMindControlled = true;
	pData->Dominator_CapturePermaMindControlled = true;
	pData->Dominator_CaptureImmuneToPsionics = false;
	pData->Dominator_PermanentCapture = true;

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_PsychicDominatorDetected);
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_PsychicDominatorReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_PsychicDominatorActivated);

	pData->Message_Abort = GameStrings::DominatorActive_msg();

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::PsychicDominator;
	pData->SW_AffectsTarget = SuperWeaponTarget::Infantry | SuperWeaponTarget::Unit;
	pData->CursorType = (int)MouseCursorType::PsychicDominator;

}

void SW_PsychicDominator::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);
	pData->Dominator_FirstAnimHeight.Read(exINI, section, "Dominator.FirstAnimHeight");
	pData->Dominator_SecondAnimHeight.Read(exINI, section, "Dominator.SecondAnimHeight");
	pData->Dominator_FirstAnim.Read(exINI, section, "Dominator.FirstAnim");
	pData->Dominator_SecondAnim.Read(exINI, section, "Dominator.SecondAnim");
	pData->Dominator_ControlAnim.Read(exINI, section, "Dominator.ControlAnim");
	pData->Dominator_FireAtPercentage.Read(exINI, section, "Dominator.FireAtPercentage");
	pData->Dominator_Capture.Read(exINI, section, "Dominator.Capture");
	pData->Dominator_Ripple.Read(exINI, section, "Dominator.Ripple");
	pData->Dominator_CaptureMindControlled.Read(exINI, section, "Dominator.CaptureMindControlled");
	pData->Dominator_CapturePermaMindControlled.Read(exINI, section, "Dominator.CapturePermaMindControlled");
	pData->Dominator_CaptureImmuneToPsionics.Read(exINI, section, "Dominator.CaptureImmuneToPsionics");
	pData->Dominator_PermanentCapture.Read(exINI, section, "Dominator.PermanentCapture");
}

bool SW_PsychicDominator::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

WarheadTypeClass* SW_PsychicDominator::GetWarhead(const SWTypeExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->DominatorWarhead);
}

int SW_PsychicDominator::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->DominatorDamage);
}

SWRange SW_PsychicDominator::GetRange(const SWTypeExtData* pData) const
{
	return pData->SW_Range->empty() ? SWRange(RulesClass::Instance->DominatorCaptureRange) : pData->SW_Range;
}

void PsychicDominatorStateMachine::Update()
{
	// waiting. lurking in the shadows.
	if (this->Deferment > 0)
	{
		if (--this->Deferment)
		{
			return;
		}
	}

	SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(this->Super->Type);

	switch (PsyDom::Status)
	{
	case PsychicDominatorStatus::FirstAnim:
	{
		// here are the contents of PsyDom::Start().
		CellClass* pTarget = MapClass::Instance->GetCellAt(this->Coords);
		CoordStruct coords = pTarget->GetCoords();
		coords.Z += pData->Dominator_FirstAnimHeight;

		AnimClass* pAnim = nullptr;
		if (AnimTypeClass* pAnimType = pData->Dominator_FirstAnim.Get(RulesClass::Instance->DominatorFirstAnim)) {
			auto pCreated = GameCreate<AnimClass>(pAnimType, coords);
			pCreated->SetHouse(this->Super->Owner);
			pAnim = pCreated;
		}

		PsyDom::Anim = pAnim;

		auto sound = pData->SW_ActivationSound.Get(RulesClass::Instance->PsychicDominatorActivateSound);
		if (sound != -1)
		{
			VocClass::PlayAt(sound, coords, nullptr);
		}

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		PsyDom::Status = PsychicDominatorStatus::Fire;

		// most likely LightUpdateTimer
		ScenarioClass::Instance->AmbientTimer.Start(1);
		ScenarioClass::UpdateLighting();

		return;
	}
	case PsychicDominatorStatus::Fire:
	{
		// wait for some percentage of the first anim to be
		// played until we strike.
		AnimClass* pAnim = PsyDom::Anim;
		if (pAnim)
		{
			int currentFrame = pAnim->Animation.Value;
			short frameCount = pAnim->Type->GetImage()->Frames;
			int percentage = pData->Dominator_FireAtPercentage.Get(RulesClass::Instance->DominatorFireAtPercentage);
			if (frameCount * percentage / 100 > currentFrame)
			{
				return;
			}
		}

		PsyDom::Fire();

		PsyDom::Status = PsychicDominatorStatus::SecondAnim;
		return;
	}
	case PsychicDominatorStatus::SecondAnim:
	{
		// wait for the second animation to finish. (there may be up to
		// 10 frames still to be played.)
		AnimClass* pAnim = PsyDom::Anim;
		if (pAnim)
		{
			int currentFrame = pAnim->Animation.Value;
			short frameCount = pAnim->Type->GetImage()->Frames;

			if (frameCount - currentFrame > 10)
			{
				return;
			}
		}

		PsyDom::Status = PsychicDominatorStatus::Reset;
		return;
	}
	case PsychicDominatorStatus::Reset:
	{
		// wait for the last frame... WTF?
		AnimClass* pAnim = PsyDom::Anim;
		if (pAnim && pAnim->Type)
		{
			int currentFrame = pAnim->Animation.Value;
			short frameCount = pAnim->Type->GetImage()->Frames;

			if (frameCount - currentFrame > 1)
			{
				return;
			}
		}

		PsyDom::Status = PsychicDominatorStatus::Over;

		PsyDom::Coords = CellStruct::Empty;
		PsyDom::Anim = nullptr;
		ScenarioClass::UpdateLighting();

		return;
	}
	case PsychicDominatorStatus::Over:
	{
		// wait for the light to go away.
		if (ScenarioClass::Instance->AmbientCurrent != ScenarioClass::Instance->AmbientTarget)
		{
			return;
		}

		// clean up
		SW_PsychicDominator::CurrentPsyDom = nullptr;
		PsyDom::Status = PsychicDominatorStatus::Inactive;
		ScenarioClass::UpdateLighting();
		this->Clock.TimeLeft = 0;
	}
	}
}

bool PsychicDominatorStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(this->Deferment, RegisterForChange)
		.Success();
}

bool PsychicDominatorStateMachine::Save(PhobosStreamWriter& Stm) const
{
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(this->Deferment)
		.Success();
}
