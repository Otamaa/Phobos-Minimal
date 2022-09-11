#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Weapon Selection
DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { ReturnGameCode = 0x6F3341, ReturnHandled = 0x6F3406 };

	GET(const TechnoClass*, pThis, ESI);
	GET_STACK(const AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (pTarget)
	{
		if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pTypeExt->Interceptor.Get() && pTarget->WhatAmI() == AbstractType::Bullet)
			{
				R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
				return ReturnHandled;
			}
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());

	return ReturnGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
		{
			if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget.Get(), true))
				return Secondary;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x8)
{
	enum
	{
		ReturnHandled = 0x6F37AF
	};

	GET(TechnoClass*, pTechno, ECX);

	if (pTechno && pTechno->Target)
	{
		const auto pTechnoType = pTechno->GetTechnoType();
		if (!pTechnoType)
			return 0;

		const auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target);
		if (!pTarget)
			return 0;

		const auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		if (const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType))
		{
			if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0
				&& pTargetType->Cloakable && pTargetType->Naval
				&& pTarget->CloakState == CloakState::Uncloaked)
			{
				R->EAX(pTechnoTypeExt->ForceWeapon_Naval_Decloaked.Get());
				return ReturnHandled;
			}
		}
	}

	return 0;
}

//broke spawner building ?
//DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8) //7
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(TechnoClass*, pTargetTechno, EBP);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));
//
//	enum { Primary = 0x6F37AD,
//		Secondary = 0x6F3745,
//		FurtherCheck = 0x6F3754,
//		OriginalCheck = 0x6F36E3
//	};
//
//	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
//		int weaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, !pTypeExt->NoSecondaryWeaponFallback);
//		return (weaponIndex != -1) && weaponIndex == 1 ? Secondary : Primary;
//	}
//
//	return OriginalCheck;
//}

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (const auto pSecondary = pThis->GetWeapon(1))
		{
			if (const auto pSecondaryExt = WeaponTypeExt::ExtMap.Find(pSecondary->WeaponType))
			{
				if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondaryExt->CanTarget, true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondaryExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pSecondaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
				{
					return Primary;
				}

				if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
				{
					if (pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
						return Primary;

					if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pPrimaryExt->CanTarget, true)) ||
						(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pPrimaryExt->CanTarget) ||
							!EnumFunctions::CanTargetHouse(pPrimaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
					{
						return Secondary;
					}
				}
			}
		}

		if (!pTargetTechno)
			return Primary;

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive())
				{
					if (pThis->GetWeapon(1) && !(pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1)))
					{
						if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
							return Secondary;
						else
							return FurtherCheck;
					}

					return Primary;
				}
			}
		}
	}

	return OriginalCheck;
}

DEFINE_HOOK_AGAIN(0x6FF660, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
DEFINE_HOOK(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	if (pThis->WhatAmI() == AbstractType::Building && pWeapon->IsLaser)
	{
		if (auto const pExt = TechnoExt::GetExtData(pThis))
		{
			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = weaponIndex;
			else
				pExt->CurrentLaserWeaponIndex.clear();
		}
	}

	return 0;
}

/*
DEFINE_HOOK(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { ReturnValue = 0x6F37AF };

	//GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x18, 0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	int returnValue = 0;

	if(pTargetTechno && pTargetTechno->IsInAir()) {
		returnValue = !pWeapon->Projectile->AA && pSecWeapon->Projectile->AA ? 1 : 0;
	}

	R->EAX(returnValue);
	return ReturnValue;
}
*/

//DEFINE_HOOK(0x6F3436, TechnoClass_SelectGattlingWeapon, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(AbstractClass*, pTarget, EBP);
//
//	enum { Odd = 0x6F346A, Even = 0x6F345C, Origin = 0 };
//	int gattlingStage = pThis->CurrentGattlingStage;
//	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
//
//	if (pTypeExt->Gattling_SelectWeaponByVersus)
//	{
//		WeaponTypeClass* pWeaponOdd = pThis->GetWeapon(gattlingStage << 1)->WeaponType;
//		WeaponTypeClass* pWeaponEven = pThis->GetWeapon(gattlingStage << 1 | 1)->WeaponType;
//
//		if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
//		{
//			R->ESI(gattlingStage);
//
//			//auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
//			Armor nArmor = pTargetTechno->GetTechnoType()->Armor;
//
//			if (pTargetTechno->IsInAir())
//			{
//				if (!pWeaponEven->Projectile->AA)
//					return Odd;
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponEven->Warhead, nArmor) != 0.0)
//					return Even;
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, nArmor) == 0.0)
//					return Even;
//
//				return Odd;
//			}
//			else
//			{
//				if (!pWeaponOdd->Projectile->AG)
//					return Even;
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, nArmor) != 0.0)
//					return Odd;
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponEven->Warhead, nArmor) == 0.0)
//					return Odd;
//
//				return Even;
//			}
//		}
//		else
//		{
//			if (auto pTargetObject = abstract_cast<ObjectClass*>(pTarget))
//			{
//				ObjectTypeClass* pTargetType = pTargetObject->GetType();
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, pTargetType->Armor) != 0.0)
//					return Odd;
//
//				if (GeneralUtils::GetWarheadVersusArmor(pWeaponEven->Warhead, pTargetType->Armor) == 0.0)
//					return Odd;
//
//				return Even;
//			}
//			else
//			{
//				return Odd;
//			}
//		}
//	}
//
//	return Origin;
//}

/*
DEFINE_HOOK(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { ReturnValue = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	int primaryIndex = 2 * pThis->CurrentGattlingStage;
	int secondaryIndex = primaryIndex + 1;
	int pickedWeaponIdx = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, primaryIndex, secondaryIndex, true);
	int weaponIndex = primaryIndex;

	if (pickedWeaponIdx != -1)
	{
		weaponIndex = pickedWeaponIdx;
	}
	else if (pTargetTechno)
	{
		auto const pWeapon = pThis->GetWeapon(primaryIndex)->WeaponType;
		auto const pWeaponSec = pThis->GetWeapon(secondaryIndex)->WeaponType;
		bool skipRemainingChecks = false;

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive() && !pShield->CanBeTargeted(pWeapon))
				{
					weaponIndex = secondaryIndex;
					skipRemainingChecks = true;
				}
			}
		}


		if (!skipRemainingChecks)
		{
			if (GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTargetTechno->GetTechnoType()->Armor) == 0.0)
				weaponIndex = secondaryIndex;
			else if (pTargetTechno->IsInAir() && !pWeapon->Projectile->AA && pWeaponSec->Projectile->AA)
				weaponIndex = secondaryIndex;
		}
	}

	R->EAX(weaponIndex);
	return ReturnValue;
}*/