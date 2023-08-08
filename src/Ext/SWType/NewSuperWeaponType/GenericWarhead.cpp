#include "GenericWarhead.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Misc/AresData.h>

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData* pData)
{
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

	auto pWarhead = GetWarhead(pData);

	if (!pWarhead) {
		Debug::Log("launch GenericWarhead SW ([%s]) Without Waarhead\n", pType->ID);
		return true;
	}

	auto const pCell = MapClass::Instance->GetCellAt(Coords);
	const auto pFirer = this->GetFirer(pThis, Coords, false);
	const auto damage = GetDamage(pData);
	auto detonationCoords = pCell->GetCoordsWithBridge();

	if(pData->Generic_Warhead_Detonate){
		AbstractClass* pTarget = pCell->GetSomeObject({}, pCell->ContainsBridge());
		WarheadTypeExt::DetonateAt(
		pWarhead ,
		pTarget ? pTarget : pCell ,
		detonationCoords,
		pFirer,
		damage,
		pThis->Owner
		);
	}
	else
	{
		// crush, kill, destroy
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		WarheadTypeExt::CreateIonBlast(pWarhead, detonationCoords);
		pWHExt->applyIronCurtain(detonationCoords, pThis->Owner, damage);
		AresData::applyEMP(pWarhead, &detonationCoords, pFirer);
		AresData::applyAE(pWarhead, &detonationCoords, pThis->Owner);

		MapClass::DamageArea(detonationCoords, damage, pFirer, pWarhead, pWarhead->Tiberium, pThis->Owner);

		if (auto const pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, detonationCoords))
		{
				//Otamaa Added
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, detonationCoords))
				pAnim->Owner = pThis->Owner;
		}

		MapClass::FlashbangWarheadAt(damage, pWarhead, detonationCoords, false, SpotlightFlags::None);
	}

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
