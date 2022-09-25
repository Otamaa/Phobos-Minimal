#include "Body.h"
#include <Ext/WeaponType/Body.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>

//template<> const DWORD Extension<AircraftClass>::Canary = 0x55555765;
//AircraftExt::ExtContainer AircraftExt::ExtMap;

void _fastcall AircraftExt::TriggerCrashWeapon(TechnoClass* pThis,DWORD, int nMult)
{
	if (auto pType = pThis->GetTechnoType())
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt)
		{
			if (auto const pWeapon = pTypeExt->CrashWeapon.GetOrDefault(pThis, pTypeExt->CrashWeapon_s.Get()))
			{
				auto pWeaponExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

				if (BulletClass* pBullet = pWeaponExt->CreateBullet(pThis->GetCell(), pThis,
					pWeapon))
				{
					const CoordStruct& coords = pThis->GetCoords();
					pBullet->SetWeaponType(pWeapon);
					pBullet->Limbo();
					pBullet->SetLocation(coords);
					pBullet->Explode(true);
					pBullet->UnInit();
					goto playDestroyAnim;
				}
			}
		}

		pThis->FireDeathWeapon(nMult);

	playDestroyAnim:

		if (pType->DestroyAnim.Size() > 0)
		{
			auto const facing = pThis->PrimaryFacing.Current().GetFacing<256>();
			int idxAnim = 0;

			if (pTypeExt && !pTypeExt->DestroyAnim_Random.Get())
			{
				if (pType->DestroyAnim.Count >= 8)
				{
					idxAnim = pType->DestroyAnim.Count;
					if (pType->DestroyAnim.Count % 2 == 0)
						idxAnim *= static_cast<int>(facing / 256.0);
				}
			}
			else
			{
				if (pType->DestroyAnim.Count > 1)
					idxAnim = ScenarioClass::Instance->Random.RandomRanged(0, (pType->DestroyAnim.Count - 1));
			}

			if (AnimTypeClass* pAnimType = pType->DestroyAnim[idxAnim])
			{
				if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords()))
				{
					auto const pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
					auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);

					if (!pAnimTypeExt || !pAnimExt)
						return;

					if (AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Owner))
						pAnimExt->Invoker = pThis;

					if (pAnimTypeExt->CreateUnit_InheritDeathFacings.Get())
						pAnimExt->DeathUnitFacing = static_cast<short>(facing);

					if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get()) {
						if (pThis->HasTurret()) {
							pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
						}
					}
				}
			}
		}
	}
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	int weaponIndex = pThis->SelectWeapon(pTarget);
	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = (int)shotNumber;

			pThis->Fire(pThis->Target, weaponIndex);
		}
	}
}

// =============================
// load / save
/*
template <typename T>
void AircraftExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void AircraftExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AircraftClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AircraftClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool AircraftExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool AircraftExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

AircraftExt::ExtContainer::ExtContainer() : Container("AircraftClass") { }
AircraftExt::ExtContainer::~ExtContainer() = default;
*/
