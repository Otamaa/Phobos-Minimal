#include "Body.h"
#include <Ext/WeaponType/Body.h>

#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>

AircraftExt::ExtContainer AircraftExt::ExtMap;

void _fastcall AircraftExt::TriggerCrashWeapon(TechnoClass* pThis, DWORD, int nMult)
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

					if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get())
					{
						if (pThis->HasTurret())
						{
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

			pThis->Fire(pTarget, weaponIndex);
		}
	}
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx)
{
	auto pWeaponStruct = pThis->GetWeapon(WeaponIdx);

	if (!pWeaponStruct)
		return;

	auto weaponType = pWeaponStruct->WeaponType;

	if (!weaponType)
		return;

	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = (int)shotNumber;

			pThis->Fire(pTarget, WeaponIdx);
		}
	}
}

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon)
{
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeapon->Burst > 0)
	{
		for (int i = 0; i < pWeapon->Burst; i++)
		{
			if (pWeapon->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = (int)shotNumber;

			pThis->Fire(pTarget, WeaponIdx);
		}
	}
}
// =============================
// load / save

template <typename T>
void AircraftExt::ExtData::Serialize(T& Stm)
{
	Stm


		;
}

void AircraftExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<AircraftClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<AircraftClass>::Serialize(Stm);
	this->Serialize(Stm);
}

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

AircraftExt::ExtContainer::ExtContainer() : TExtensionContainer("AircraftClass") { }
AircraftExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x413F6A, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);

	AircraftExt::ExtMap.JustAllocate(pItem, !pItem, "Invalid !");

	return 0;
}

DEFINE_HOOK(0x41426F, AircraftClass_DTOR, 0x7)
{
	GET(AircraftClass*, pItem, EDI);

	AircraftExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AircraftExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
{
	AircraftExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
{
	AircraftExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x41B685, AircraftClass_Detach, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFSET(0x8, 0x8));

	if (const auto pExt = AircraftExt::ExtMap.Find(pThis))
		pExt->InvalidatePointer(target, all);

	return 0x0;
}