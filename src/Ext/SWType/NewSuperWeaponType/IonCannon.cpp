#include "IonCannon.h"
#include <Ext/WarheadType/Body.h>

std::vector<const char*> SW_IonCannon::GetTypeString() const
{
	return { "IonCannon" };
}

SuperWeaponFlags SW_IonCannon::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_IonCannon::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged) {
		this->newStateMachine(Coords, pThis, this->GetFirer(pThis , Coords, false));
	}

	return true;
}

void SW_IonCannon::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::IonCannon;
	pData->IonCannon_BeamHeight = 750;
	pData->IonCannon_BlastHeight = 0;
	pData->IonCannon_Beam = RulesClass::Instance->IonBeam;
	pData->IonCannon_Blast = RulesClass::Instance->IonBlast;
	pData->IonCannon_FireAtPercentage = 20;
	pData->IonCannon_Ripple = true;

	pData->EVA_Detected = VoxClass::FindIndexById("EVA_IonCannonDetected");
	pData->EVA_Ready = VoxClass::FindIndexById("EVA_IonCannonReady");
	pData->EVA_Activated = VoxClass::FindIndexById("EVA_IonCannonActivated");
}

void SW_IonCannon::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);
	pData->IonCannon_BeamHeight.Read(exINI, section, "IonCannon.BeamAnimHeight");
	pData->IonCannon_BlastHeight.Read(exINI, section, "IonCannon.BlastAnimHeight");
	pData->IonCannon_Beam.Read(exINI, section, "IonCannon.BeamAnim");
	pData->IonCannon_Blast.Read(exINI, section, "IonCannon.BlastAnim");
	pData->IonCannon_FireAtPercentage.Read(exINI, section, "IonCannon.FireAtPercentage");
	pData->IonCannon_Ripple.Read(exINI, section, "IonCannon.Ripple");
}

WarheadTypeClass* SW_IonCannon::GetWarhead(const SWTypeExtData* pData) const
{
	return pData->SW_Warhead.Get(RulesClass::Instance->IonCannonWarhead);
}

int SW_IonCannon::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(RulesClass::Instance->IonCannonDamage);
}

bool SW_IonCannon::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void IonCannonStateMachine::Update()
{
	if (this->Deferment > 0)
	{
		if (--this->Deferment)
		{
			return;
		}
	}

	SWTypeExtData* pData = this->GetTypeExtData();

	switch (this->Status)
	{
	case IonCannonStatus::FirstAnim:
	{
		CellClass* pTarget = MapClass::Instance->GetCellAt(this->Coords);
		CoordStruct coords = pTarget->GetCoords();
		coords.Z += pData->IonCannon_BeamHeight;

		if (AnimTypeClass* pAnimType = pData->IonCannon_Beam.Get())
		{
			auto pCreated = GameCreate<AnimClass>(pAnimType, coords);
			pCreated->SetHouse(this->Owner);
			this->Anim = pCreated;
		}

		const auto sound = pData->SW_ActivationSound.Get();
		if (sound != -1) {
			VocClass::PlayAt(sound, coords, nullptr);
		}

		pData->PrintMessage(pData->Message_Activate, this->Owner);

		this->Status = IonCannonStatus::Fire;
		return;
	}
	case IonCannonStatus::Fire:
	{
		// wait for some percentage of the first anim to be
		// played until we strike.
		if (AnimClass* pAnim = this->Anim)
		{
			int currentFrame = pAnim->Animation.Stage;
			short frameCount = pAnim->Type->GetImage()->Frames;
			int percentage = pData->IonCannon_FireAtPercentage.Get();
			if (frameCount * percentage / 100 > currentFrame)
			{
				return;
			}
		}

		this->Fire();
		this->Status = IonCannonStatus::SecondAnim;
		return;
	}
	case IonCannonStatus::SecondAnim:
	{
		// wait for the second animation to finish. (there may be up to
		// 10 frames still to be played.)
		if (AnimClass* pAnim = this->Anim)
		{
			int currentFrame = pAnim->Animation.Stage;
			short frameCount = pAnim->Type->GetImage()->Frames;

			if (frameCount - currentFrame > 10)
			{
				return;
			}
		}

		this->Status = IonCannonStatus::Reset;
		return;
	}
	case IonCannonStatus::Reset:
	{
		// wait for the last frame... WTF?
		if (AnimClass* pAnim = this->Anim)
		{
			if(pAnim->Type) {
				int currentFrame = pAnim->Animation.Stage;
				short frameCount = pAnim->Type->GetImage()->Frames;

				if (frameCount - currentFrame > 1)
				{
					return;
				}
			}
		}

		this->Status = IonCannonStatus::Over;
		this->Coords = CellStruct::Empty;
		this->Anim = nullptr;
		return;
	}
	case IonCannonStatus::Over:
	{
		// clean up
		this->Status = IonCannonStatus::Inactive;
		this->Clock.TimeLeft = 0;
	}
	}
}

bool IonCannonStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(this->Deferment)
		.Process(this->Status)
		.Process(this->Owner, RegisterForChange)
		.Process(this->Anim, RegisterForChange)
		.Process(this->Firer, RegisterForChange)
		.Success();
}

bool IonCannonStateMachine::Save(PhobosStreamWriter& Stm) const
{
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(this->Deferment)
		.Process(this->Status)
		.Process(this->Owner)
		.Process(this->Anim)
		.Process(this->Firer)
		.Success();
}

void IonCannonStateMachine::Fire()
{
	const auto pData = this->GetTypeExtData();
	CellStruct cell = this->Coords;
	CellClass* pTarget = MapClass::Instance->GetCellAt(cell);
	CoordStruct coords = pTarget->GetCoordsWithBridge();

	// blast!
	if (pData->IonCannon_Ripple)
	{
		auto pBlast = GameCreate<IonBlastClass>(coords);
		pBlast->DisableIonBeam = TRUE;
	}

	// tell!
	if (pData->SW_RadarEvent)
	{
		RadarEventClass::Create(RadarEventType::SuperweaponActivated, cell);
	}

	// anim
	this->Anim = nullptr;

	if (AnimTypeClass* pAnimType = pData->IonCannon_Blast.Get())
	{
		CoordStruct animCoords = coords;
		animCoords.Z += pData->IonCannon_BlastHeight;
		auto pCreated = GameCreate<AnimClass>(pAnimType, animCoords);
		pCreated->SetHouse(this->Owner);
		this->Anim = pCreated;
	}

	// kill
	auto damage = this->Type->GetDamage(pData);
	auto pWarhead = this->Type->GetWarhead(pData);

	if (pWarhead && damage != 0) {
		WarheadTypeExtData::DetonateAt(pWarhead, pTarget, coords, this->Firer, damage, this->Owner);
	}
}

void IonCannonStateMachine::InvalidatePointer(AbstractClass* ptr, bool remove)
{
	AnnounceInvalidPointer(this->Firer, ptr , remove);
	AnnounceInvalidPointer(this->Owner, ptr);
	AnnounceInvalidPointer(this->Anim, ptr);
}
