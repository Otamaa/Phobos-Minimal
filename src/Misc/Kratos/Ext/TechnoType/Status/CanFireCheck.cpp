#include "../TechnoStatus.h"

#include <Misc/Kratos/Extension/BulletTypeExt.h>
#include <Misc/Kratos/Extension/WarheadTypeExt.h>
#include <Misc/Kratos/Extension/WeaponTypeExt.h>

#include <Misc/Kratos/Ext/Helper/Scripts.h>

bool TechnoStatus::CanFire_CheckWeapon(AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	HouseClass* pHouse = pTechno->Owner;
	WeaponTypeExt::TypeData* weaponData = GetTypeData<WeaponTypeExt, WeaponTypeExt::TypeData>(pWeapon);
	if (weaponData && !weaponData->DontNeedMoney)
	{
		int needMoney = weaponData->NoMoneyNoTalk;
		if (needMoney == 0 && NoMoneyNoTalk->IsAlive())
		{
			needMoney = NoMoneyNoTalk->Data.Money;
		}
		if (needMoney != 0 && pHouse)
		{
			int money = pHouse->Available_Money();
			if (needMoney > 0 ? money < needMoney : money > Math::abs(needMoney))
			{
				return _CannotFire;
			}
		}
	}
	return !_CannotFire;
}

bool TechnoStatus::CanFire_CheckBullet(AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoTypeClass* pType = pTechno->GetTechnoType();
	if (pType && pTarget)
	{
		// AAOnly
		if (!pTarget->IsInAir()
			&& pType->LandTargeting != LandTargetingType::Land_not_okay
			&& pWeapon->Projectile->AA)
		{
			BulletTypeExt::TypeData* bulletData = GetTypeData<BulletTypeExt, BulletTypeExt::TypeData>(pWeapon->Projectile);
			if (bulletData && bulletData->AAOnly)
			{
				return _CannotFire;
			}
		}
	}
	return !_CannotFire;
}
