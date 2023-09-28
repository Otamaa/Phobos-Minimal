#include "GenericWarhead.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Misc/AresData.h>

#include <Misc/Ares/Hooks/Classes/AttachedAffects.h>

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData* pData)
{
	pData->OwnerObject()->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Offensive;
}

WarheadTypeClass* SW_GenericWarhead::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Warhead.isset())
		return pData->SW_Warhead;

	if (pData->Get()->WeaponType)
		return pData->Get()->WeaponType->Warhead;

	return nullptr;
}

int SW_GenericWarhead::GetDamage(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Damage.isset())
		return pData->SW_Damage;

	if (pData->Get()->WeaponType)
		return pData->Get()->WeaponType->Damage;

	return 0;
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	const auto pFirer = this->GetFirer(pThis, Coords, false);
	const auto nDeferement = pData->SW_Deferment.Get(-1);
	const auto pWarhead = this->GetWarhead(pData);

	if (!pWarhead) {
		Debug::Log("launch GenericWarhead SW ([%s]) Without Waarhead\n", pThis->Type->ID);
		return true;
	}

	this->newStateMachine(nDeferement < 0 ? 10 : nDeferement, Coords, pThis, pFirer);
	return true;
}

bool SW_GenericWarhead::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void SW_GenericWarhead::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->Get()->ID;

	INI_EX exINI(pINI);
	pData->Generic_Warhead_Detonate.Read(exINI, section, "GenericWarhead.Detonate");
}

void GenericWarheadStateMachine::Update()
{
	if (this->Finished())
	{
		this->SentPayload();
	}
}

void GenericWarheadStateMachine::SentPayload()
{
	auto pData = GetTypeExtData();

	pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

	const auto sound = pData->SW_ActivationSound.Get(-1);
	if (sound != -1) {
		VocClass::PlayGlobal(sound, Panning::Center, 1.0);
	}

	const auto pWarhead = this->Type->GetWarhead(pData);
	auto const pCell = MapClass::Instance->GetCellAt(Coords);
	const auto damage = this->Type->GetDamage(pData);
	auto detonationCoords = pCell->GetCoordsWithBridge();

	if (pData->Generic_Warhead_Detonate)
	{
		AbstractClass* pTarget = pCell->GetSomeObject({}, pCell->ContainsBridge());
		WarheadTypeExt::DetonateAt(
		pWarhead,
		pTarget ? pTarget : pCell,
		detonationCoords,
		Firer,
		damage,
		 this->Super->Owner
		);
	}
	else
	{
		// crush, kill, destroy
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		WarheadTypeExt::CreateIonBlast(pWarhead, detonationCoords);
		pWHExt->applyIronCurtain(detonationCoords, this->Super->Owner, damage);
		WarheadTypeExt::applyEMP(pWarhead, detonationCoords, Firer);
		AresAE::applyAttachedEffect(pWarhead, detonationCoords, this->Super->Owner);
		//AresData::applyAE(pWarhead, { &detonationCoords, this->Super->Owner });

		MapClass::DamageArea(detonationCoords, damage, Firer, pWarhead, pWarhead->Tiberium, this->Super->Owner);

		if (auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, detonationCoords))
		{
			//Otamaa Added
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, detonationCoords))
				pAnim->Owner = this->Super->Owner;
		}

		MapClass::FlashbangWarheadAt(damage, pWarhead, detonationCoords, false, SpotlightFlags::None);
	}
}

bool  GenericWarheadStateMachine::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return SWStateMachine::Load(Stm , RegisterForChange)
		&& Stm
		.Process(Firer)
		.Success();
}

bool  GenericWarheadStateMachine::Save(PhobosStreamWriter& Stm) const
{
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(Firer)
		.Success();
}

void  GenericWarheadStateMachine::InvalidatePointer(AbstractClass* ptr, bool remove)
{
	AnnounceInvalidPointer(Firer, ptr ,remove);
}