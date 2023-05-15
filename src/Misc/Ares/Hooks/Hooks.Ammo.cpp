#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WarheadType/Body.h>

#include <WWKeyboardClass.h>
#include <Conversions.h>

#pragma region Funcs
int GetAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	for (int i = pExt->Ammo; i > 0; --i)
		pThis->DecreaseAmmo();

	return pExt->Ammo;
}

void DecreaseAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon)
{
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (GetAmmo(pThis, pWeapon) > 0) {
		if (!Is_Aircraft(pThis)) {
			if (pTypeExt->NoAmmoWeapon > -1 && pTypeExt->NoAmmoEffectAnim) {
				const auto pCurWeapon = pThis->GetWeapon(pTypeExt->NoAmmoWeapon);
				if (pThis->Ammo <= pTypeExt->NoAmmoAmount && pCurWeapon->WeaponType != pWeapon) {
					if (auto pAnim = GameCreate<AnimClass>(pTypeExt->NoAmmoEffectAnim.Get(), pThis->Location)) {
						pAnim->SetOwnerObject(pThis);
						pAnim->SetHouse(pThis->Owner);
					}
				}
			}
		}

		if (Is_Building(pThis)) {
			const auto Ammo = reinterpret_cast<BuildingClass*>(pThis)->Type->Ammo;
			if (Ammo > 0 && pThis->Ammo < Ammo)
				pThis->StartReloading();
		}
	}
}
#pragma endregion

//weapons can take more than one round of ammo
DEFINE_OVERRIDE_HOOK(0x6FCA0D, TechnoClass_CanFire_Ammo, 6)
{
	enum { FireErrAmmo = 0x6FCA17u, Continue = 0x6FCA26u };
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	const auto nAmmo = pThis->Ammo;
	if (nAmmo < 0)
		return Continue;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const bool IsDisabled = pTypeExt->NoAmmoWeapon == -1;

	if (!IsDisabled)
	{
		if ((nAmmo - WeaponTypeExt::ExtMap.Find(pWeapon)->Ammo) >= 0)
			return Continue;
	}

	return (!IsDisabled || !nAmmo) ? FireErrAmmo : Continue;
}

//remove ammo rounds depending on weapon
DEFINE_OVERRIDE_HOOK(0x6FF656, TechnoClass_Fire_Ammo, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);

	DecreaseAmmo(pThis, pWeapon);

	return 0x6FF660;
}

DEFINE_OVERRIDE_HOOK(0x7413FF, UnitClass_Fire_Ammo, 7)
{
	GET(UnitClass*, pThis, ESI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x20, 0x8));
	const auto pWP = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWP->Ammo > 0)
		pThis->StartReloading();

	return 0x741406;
}

DEFINE_OVERRIDE_HOOK(0x51DF8C, InfantryClass_Fire_Ammo, 6)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(int, nWeaponIdx, 0x10);

	const auto pWeapon = pThis->GetWeapon(nWeaponIdx)->WeaponType;
	const auto pWP = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWP->Ammo > 0)
	{
		if (pThis->Type->Ammo > 0 && pThis->Ammo < pThis->Type->Ammo)
		{
			pThis->StartReloading();
		}
	}

	return 0;
}

// variable amounts of rounds to reload
DEFINE_OVERRIDE_HOOK(0x6FB05B, TechnoClass_Reload_ReloadAmount, 6)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();
	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	int amount = pExt->ReloadAmount;
	if (!pThis->Ammo)
	{
		amount = pExt->EmptyReloadAmount.Get(amount);
	}

	// clamping to support negative values
	pThis->Ammo = std::clamp((pThis->Ammo + amount), 0, pType->Ammo);

	return 0x6FB061;
}

DEFINE_OVERRIDE_HOOK(0x6F3410, TechnoClass_SelectWeapon_NoAmmoWeapon, 5)
{
	GET(TechnoClass*, pThis, ESI);
	const auto pType = pThis->GetTechnoType();

	if (pType->Ammo < 0)
		return 0x0;

	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->NoAmmoWeapon < 0 || pThis->Ammo > pExt->NoAmmoAmount)
		return 0x0;

	R->EAX(pExt->NoAmmoWeapon);
	return 0x6F3406;
}