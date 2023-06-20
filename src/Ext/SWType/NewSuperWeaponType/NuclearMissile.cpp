#include "NuclearMissile.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <New/Type/CursorTypeClass.h>

#include <Misc/AresData.h>

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
			TechnoExt::ExtMap.Find(pSilo)->LinkedSW = pThis;
			pSilo->QueueMission(Mission::Missile, false);
			pSilo->NextMission();

			pThis->Owner->NukeTarget = Coords;
			fired = true;
		}

		if (!fired)
		{
			// if we reached this, there is no silo launch. still launch a missile.
			if (auto const pWeapon = pData->Nuke_Payload)
			{
				if (auto const pProjectile = pWeapon->Projectile)
				{
					// get damage and warhead. they are not available during
					// initialisation, so we gotta fall back now if they are invalid.
					auto const damage = this->GetDamage(pData);
					auto const pWarhead = this->GetWarhead(pData);

					// create a bullet and the psi warning
					if (auto const pBullet = BulletTypeExt::ExtMap.Find(pProjectile)->CreateBullet(pCell, pSilo, damage, pWarhead,
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
						const bool bNotVert = !pProjectile->Vertical;
						VelocityClass vel { 0.0, bNotVert ? 100.0 : 0.0,  bNotVert ? 0x0 : -100.0 };

						auto high = target;

						if(!bNotVert)
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
	pData->Nuke_Payload = WeaponTypeClass::FindOrAllocate(GameStrings::NukePayload); //use for nuke pointing down
	//SW->WeaponType = used for nuke pointing up !
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
	if (pData->SW_Warhead.Get(nullptr))
		return pData->SW_Warhead;

	if (auto pPayload = pData->Nuke_Payload) {
		return pPayload->Warhead;
	}

	return nullptr; // :p
}

int SW_NuclearMissile::GetDamage(const SWTypeExt::ExtData* pData) const
{
	auto damage = pData->SW_Damage.Get(-1);
	if (damage < 0) {
		damage = pData->Nuke_Payload ? pData->Nuke_Payload->Damage : 0;
	}
	return damage;
}
