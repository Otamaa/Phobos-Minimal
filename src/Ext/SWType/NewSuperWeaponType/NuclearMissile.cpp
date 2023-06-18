#include "NuclearMissile.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <New/Type/CursorTypeClass.h>

SuperClass* SW_NuclearMissile::CurrentNukeType = nullptr;

bool SW_NuclearMissile::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::Nuke);
}

SuperWeaponFlags SW_NuclearMissile::Flags() const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_NuclearMissile::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	if (pThis->IsCharged)
	{
		auto const pType = pThis->Type;
		auto const pData = SWTypeExt::ExtMap.Find(pType);

		auto const pCell = MapClass::Instance->GetCellAt(Coords);
		auto const target = pCell->GetCoordsWithBridge();

		// the nuke has two ways to fire. first the granted way used by nukes
		// collected from crates. second, the normal way firing from a silo.
		BuildingClass* pSilo = nullptr;

		if ((!pThis->Granted || !pThis->OneTime) && pData->Nuke_SiloLaunch)
		{
			// find a building owned by the player that can fire this SWType
			auto const& Buildings = pThis->Owner->Buildings;
			auto it = std::find_if(Buildings.begin(), Buildings.end(), [=](BuildingClass* pBld) 
			{
				return this->IsLaunchSiteEligible(pData, Coords, pBld, false);
			});

			if (it != Buildings.end())
			{
				pSilo = *it;
			}
		}

		// via silo
		bool fired = false;
		if (pSilo)
		{
			// setup the missile and start the fire mission
			pSilo->FiringSWType = pType->ArrayIndex;
			pSilo->QueueMission(Mission::Missile, false);
			pSilo->NextMission();

			pThis->Owner->NukeTarget = Coords;
			fired = true;
		}

		if (!fired)
		{
			// if we reached this, there is no silo launch. still launch a missile.
			if (auto const pWeapon = this->GetNukePayload(pType))
			{
				if (auto const pProjectile = pWeapon->Projectile)
				{
					// get damage and warhead. they are not available during
					// initialisation, so we gotta fall back now if they are invalid.
					auto const damage = this->GetDamage(pData);
					auto const pWarhead = this->GetWarhead(pData);

					// create a bullet and the psi warning
					if (auto const pBullet = BulletTypeExt::ExtMap.Find(pProjectile)->CreateBullet(nullptr, pSilo, damage, pWarhead,
						pWeapon->Speed, WeaponTypeExt::ExtMap.Find(pWeapon)->GetProjectileRange(), pWeapon->Bright || pWeapon->Warhead->Bright, true))
					{
						pBullet->SetWeaponType(pWeapon);

						if (auto const pAnimType = pData->Nuke_PsiWarning) {
							pThis->Owner->PsiWarn(pCell, pBullet, pAnimType->ID);
						}

						// remember the fired SW type
						BulletExt::ExtMap.Find(pBullet)->NukeSW = pThis;

						// aim the bullet downward and put
						// it over the target area.
						VelocityClass vel { 0.0, 0.0, !pProjectile->Vertical ? 100.0 : -100.0 };

						auto high = target;

						if(!pProjectile->Vertical)
							high.Z += 20000;

						pBullet->MoveTo(high, vel);
						fired = true;
					}
				}
			}
		}

		if (fired)
		{
			// allies can see the target location before the enemy does
			if (pData->SW_RadarEvent)
			{
				if (pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
				{
					RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
				}
			}

			VocClass::PlayAt(pData->SW_ActivationSound.Get(RulesClass::Instance->DigSound), target, nullptr);
			pThis->Owner->RecheckTechTree = true;
			return true;
		}
	}

	return false;
}

void SW_NuclearMissile::Initialize(SWTypeExt::ExtData* pData)
{ 
	// default values for the original Nuke
	pData->Nuke_Payload = WeaponTypeClass::FindOrAllocate(GameStrings::NukePayload);
	pData->Nuke_PsiWarning = AnimTypeClass::Find(GameStrings::PSIWARN);

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_NuclearSiloDetected());
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_NuclearMissileReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_NuclearMissileLaunched());

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Nuke;
	pData->CursorType = (int)MouseCursorType::Nuke;
}

void SW_NuclearMissile::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->Get()->ID;

	INI_EX exINI(pINI);

	pData->Nuke_Payload.Read(exINI, section, "Nuke.Payload", true);
	pData->Nuke_TakeOff.Read(exINI, section, "Nuke.TakeOff");
	pData->Nuke_PsiWarning.Read(exINI, section, "Nuke.PsiWarning");
	pData->Nuke_SiloLaunch.Read(exINI, section, "Nuke.SiloLaunch");

}

bool SW_NuclearMissile::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	return pBuilding->Type->NukeSilo && NewSWType::IsLaunchSite(pData, pBuilding);
}

WarheadTypeClass* SW_NuclearMissile::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Warhead.isset())
		return pData->SW_Warhead;

	if (auto const pPayload = SW_NuclearMissile::GetNukePayload(pData->Get()))
		return pPayload->Warhead;

	return nullptr;
}

int SW_NuclearMissile::GetDamage(const SWTypeExt::ExtData* pData) const
{
	if (pData->SW_Damage.Get(0) != 0)
		return pData->SW_Damage;

	const auto pPayload = SW_NuclearMissile::GetNukePayload(pData->Get());
	if (pPayload && pPayload->Damage != 0)
		return pPayload->Damage;

	return RulesClass::Instance->AtomDamage;
}

WeaponTypeClass* SW_NuclearMissile::GetNukePayload(SuperWeaponTypeClass* pSuper)
{
	if(auto pPayload = SWTypeExt::ExtMap.Find(pSuper)->Nuke_Payload.Get())
		return pPayload;

	return pSuper->WeaponType;
}
