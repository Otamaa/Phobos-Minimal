#include "NuclearMissile.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Building/Body.h>

#include <New/Type/CursorTypeClass.h>

#include <Misc/AresData.h>

SuperWeaponTypeClass* SW_NuclearMissile::CurrentNukeType = nullptr;

std::vector<const char*> SW_NuclearMissile::GetTypeString() const
{
	return { "NewNuke" };
}

bool SW_NuclearMissile::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::Nuke);
}

SuperWeaponFlags SW_NuclearMissile::Flags(const SWTypeExtData* pData) const
{
	return SuperWeaponFlags::NoEvent;
}

bool SW_NuclearMissile::Activate(SuperClass* const pThis, const CellStruct& Coords, bool const IsPlayer)
{
	if (pThis->IsCharged)
	{
		auto const pType = pThis->Type;
		auto const pData = SWTypeExtContainer::Instance.Find(pType);

		auto const pCell = MapClass::Instance->GetCellAt(Coords);
		auto const target = pCell->GetCoordsWithBridge();

		// the nuke has two ways to fire. first the granted way used by nukes
		// collected from crates. second, the normal way firing from a silo.
		BuildingClass* pSilo = nullptr;

		if ((!pThis->Granted || !pThis->OneTime) && pData->Nuke_SiloLaunch) {
			// find a building owned by the player that can fire this SWType
			if(auto pBld = specific_cast<BuildingClass*>(this->GetFirer(pThis, Coords, false)))
			 pSilo = pBld;
		}

		// via silo
		bool fired = false;
		if (pSilo)
		{
			// setup the missile and start the fire mission
			pSilo->FiringSWType = pType->ArrayIndex;
			TechnoExtContainer::Instance.Find(pSilo)->LinkedSW = pThis;
			TechnoExtContainer::Instance.Find(pSilo)->SuperTarget = Coords;
			pThis->Owner->NukeTarget = Coords;

			pSilo->QueueMission(Mission::Missile, false);
			pSilo->NextMission();
			fired = true;
		}

		if (!fired)
		{
			// if we reached this, there is no silo launch. still launch a missile.
			if (auto const pWeapon = pData->Nuke_Payload) {
				fired = SW_NuclearMissile::DropNukeAt(pType, target, this->GetAlternateLauchSite(pData, pThis), pThis->Owner, pWeapon);
			}
		}

		if (fired)
		{
			// allies can see the target location before the enemy does
			if (pData->SW_RadarEvent)
			{
				if (pThis->Owner->IsAlliedWith_(HouseClass::CurrentPlayer))
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

void SW_NuclearMissile::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::Nuke;
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

void SW_NuclearMissile::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);

	pData->Nuke_Payload.Read(exINI, section, "Nuke.Payload");
	pData->Nuke_TakeOff.Read(exINI, section, "Nuke.TakeOff");
	pData->Nuke_PsiWarning.Read(exINI, section, "Nuke.PsiWarning");
	pData->Nuke_SiloLaunch.Read(exINI, section, "Nuke.SiloLaunch");

}

bool SW_NuclearMissile::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	const auto pBldExt = BuildingExtContainer::Instance.Find(pBuilding);
	if(pBldExt->LimboID != -1)
		return false;

	if(!this->IsLaunchsiteAlive(pBuilding))
		return false;

	return pBuilding->Type->NukeSilo && this->IsSWTypeAttachedToThis(pData, pBuilding);
}

WarheadTypeClass* SW_NuclearMissile::GetWarhead(const SWTypeExtData* pData) const
{
	if (pData->SW_Warhead.Get(nullptr))
		return pData->SW_Warhead;

	if (auto pPayload = pData->Nuke_Payload) {
		return pPayload->Warhead;
	}

	return nullptr; // :p
}

int SW_NuclearMissile::GetDamage(const SWTypeExtData* pData) const
{
	auto damage = pData->SW_Damage.Get(-1);
	if (damage < 0) {
		damage = pData->Nuke_Payload ? pData->Nuke_Payload->Damage : 0;
	}
	return damage;
}

BuildingClass* SW_NuclearMissile::GetAlternateLauchSite(const SWTypeExtData* pData, SuperClass* pThis)
{
	for (auto pBuilding : pThis->Owner->Buildings) {
		if (!this->IsLaunchsiteAlive(pBuilding))
			continue;

		if (this->IsSWTypeAttachedToThis(pData, pBuilding))
			return pBuilding;
	}

	return nullptr;
}

bool SW_NuclearMissile::DropNukeAt(SuperWeaponTypeClass* pSuper, CoordStruct const& to, TechnoClass* Owner, HouseClass* OwnerHouse, WeaponTypeClass* pPayload)
{
	if (!pPayload->Projectile)
		return false;

	const auto pCell = MapClass::Instance->GetCellAt(to);

	if (auto const pBullet = GameCreate<BulletClass>()
		//BulletTypeExtContainer::Instance.Find(pPayload->Projectile)->CreateBullet(pCell, Owner, 0, nullptr,
		//pPayload->Speed, WeaponTypeExtContainer::Instance.Find(pPayload)->GetProjectileRange(), false , false)
		)
	{
		pBullet->Construct(pPayload->Projectile, pCell, Owner, 0, nullptr, pPayload->Speed, false);
		pBullet->SetWeaponType(pPayload);

		int Damage = pPayload->Damage;
		WarheadTypeClass* pWarhead = pPayload->Warhead;

		if(pSuper){
			auto const pData = SWTypeExtContainer::Instance.Find(pSuper);
			BulletClass::CreateDamagingBulletAnim(OwnerHouse,
				pCell,
				pBullet,
				pData->Nuke_PsiWarning
			);

			auto pNewType = NewSWType::GetNewSWType(pData);
			Damage = pNewType->GetDamage(pData);
			pWarhead = pNewType->GetWarhead(pData);

			// remember the fired SW type
			BulletExtContainer::Instance.Find(pBullet)->NukeSW = pSuper;
		}

		pBullet->Health = Damage; //Yes , this is
		pBullet->WH = pWarhead;
		pBullet->Bright = pPayload->Bright || pWarhead->Bright;
		pBullet->Range = WeaponTypeExtContainer::Instance.Find(pPayload)->GetProjectileRange();

		if(!Owner)
			BulletExtContainer::Instance.Find(pBullet)->Owner = OwnerHouse;

#ifndef vanilla
		// aim the bullet downward and put
		// it over the target area.
		const bool bNotVert = !pPayload->Projectile->Vertical;
		VelocityClass vel { 0.0, bNotVert ? 100.0 : 0.0,  bNotVert ? 0.0 : -100.0 };

		CoordStruct high = to;

		if (!bNotVert)
			high.Z += pPayload->Projectile->DetonationAltitude;

		return pBullet->MoveTo(high, vel);

#else

		CoordStruct nOffs { 0 , 0, pPayload->Projectile->DetonationAltitude };
		CoordStruct dest = to + nOffs;

		auto nCos = Math::cos(1.570748388432313); // Accuracy is different from the game
		auto nSin = Math::sin(1.570748388432313); // Accuracy is different from the game

		double nX = nCos * nCos * -1.0;
		double nY = nCos * nSin * -1.0;
		double nZ = nSin * -1.0;

		return pBullet->MoveTo(dest, { nX , nY , nZ });
#endif
	}

	return false;
}
