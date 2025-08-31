#include "GenericWarhead.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Misc/Ares/Hooks/Classes/AttachedAffects.h>
#include <Misc/DamageArea.h>

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

void SW_GenericWarhead::Initialize(SWTypeExtData* pData)
{
	pData->This()->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Offensive;
}

WarheadTypeClass* SW_GenericWarhead::GetWarhead(const SWTypeExtData* pData) const
{
	if (pData->SW_Warhead.isset())
		return pData->SW_Warhead;

	if (pData->This()->WeaponType)
		return pData->This()->WeaponType->Warhead;

	return nullptr;
}

int SW_GenericWarhead::GetDamage(const SWTypeExtData* pData) const
{
	if (pData->SW_Damage.isset())
		return pData->SW_Damage;

	if (pData->This()->WeaponType)
		return pData->This()->WeaponType->Damage;

	return 0;
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto const pType = pThis->Type;
	auto const pData = SWTypeExtContainer::Instance.Find(pType);

	const auto pFirer = this->GetFirer(pThis, Coords, false);
	const auto nDeferement = pData->SW_Deferment.Get(-1);
	const auto pWarhead = this->GetWarhead(pData);

	if (!pWarhead) {
		Debug::LogInfo("launch GenericWarhead SW ([{}]) Without Warhead", pThis->Type->ID);
		return true;
	}

	if(nDeferement <= 0)
		GenericWarheadStateMachine::SentPayload(pFirer, pThis, pData, this, Coords);
	else
		this->newStateMachine(nDeferement, Coords, pThis, pFirer);

	return true;
}

bool SW_GenericWarhead::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}

void SW_GenericWarhead::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->This()->ID;

	INI_EX exINI(pINI);
	pData->Generic_Warhead_Detonate.Read(exINI, section, "GenericWarhead.Detonate");
}

void GenericWarheadStateMachine::Update()
{
	if (this->Finished())
	{
		auto pData = this->GetTypeExtData();

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		const auto sound = pData->SW_ActivationSound.Get(-1);
		if (sound != -1) {
			VocClass::PlayGlobal(sound, Panning::Center, 1.0);
		}

		SentPayload(this->Firer , this->Super , pData, this->Type , this->Coords);
	}
}

void GenericWarheadStateMachine::SentPayload(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType , const CellStruct& loc)
{
	const auto pWarhead = pNewType->GetWarhead(pData);
	auto const pCell = MapClass::Instance->GetCellAt(loc);
	const auto damage = pNewType->GetDamage(pData);
	auto detonationCoords = pCell->GetCoordsWithBridge();

	if (pData->Generic_Warhead_Detonate)
	{
		AbstractClass* pTarget = pCell->GetSomeObject({}, pCell->ContainsBridge());
		WarheadTypeExtData::DetonateAt(
		pWarhead,
		pTarget ? pTarget : pCell,
		detonationCoords,
		pFirer,
		damage,
		 pSuper->Owner
		);
	}
	else
	{
		// crush, kill, destroy
		auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		WarheadTypeExtData::CreateIonBlast(pWarhead, detonationCoords);
		pWHExt->applyIronCurtain(detonationCoords, pSuper->Owner, damage);
		WarheadTypeExtData::applyEMP(pWarhead, detonationCoords, pFirer);
		AresAE::applyAttachedEffect(pWarhead, detonationCoords, pSuper->Owner);

		// Otamaa : design changes here is intended
		// as MC now part of bigger `Detonate` function , that also check various state
		// TODO : make everything work together better , for now this may give an headache for
		//		  someone that touching the code  , please bear it with me for a while !
		DamageArea::Apply(&detonationCoords, damage, pFirer, pWarhead, pWarhead->Tiberium, pSuper->Owner);

		if (auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, detonationCoords))
		{
			//Otamaa Added
			auto pAnim = GameCreate<AnimClass>(pAnimType, detonationCoords);
			pAnim->Owner = pSuper->Owner;
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
