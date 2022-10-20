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
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->Interceptor.Get() && pTarget->WhatAmI() == AbstractType::Bullet)
		{
			R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
			return ReturnHandled;
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

	if (const auto pCell = specific_cast<CellClass*>(pTarget))
	{
		if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType)->CanTarget.Get(), true))
			return Secondary;
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x8)
{
	enum
	{
		ReturnHandled = 0x6F37AF
	};

	GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pTechno, ECX);

	if (pTechno && pTarget)
	{
		const auto pThisTechnoType = pTechno->GetTechnoType();
		const auto pTargetType = pTarget->GetTechnoType();

		if (!pThisTechnoType || !pTargetType)
			return 0;

		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThisTechnoType);

		if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0
			&& pTargetType->Cloakable && pTargetType->Naval
			&& pTarget->CloakState == CloakState::Uncloaked)
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_Naval_Decloaked.Get());
			return ReturnHandled;
		}

		if (pTechnoTypeExt->ForceWeapon_Cloaked >= 0 &&
				pTarget->CloakState == CloakState::Cloaked)
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_Cloaked);
			return ReturnHandled;
		}

		if (pTechnoTypeExt->ForceWeapon_Disguised >= 0 &&
			   pTarget->IsDisguised())
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_Disguised);
			return ReturnHandled;
		}

		if (pTechnoTypeExt->ForceWeapon_UnderEMP >= 0 && pTarget->IsUnderEMP())
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_UnderEMP);
			return ReturnHandled;
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

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	{
		int weaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, !pTypeExt->NoSecondaryWeaponFallback);

		if (weaponIndex != -1)
			return weaponIndex == 1 ? Secondary : Primary;

		if (!pTargetTechno)
			return Primary;

		if (const auto pShield = TechnoExt::ExtMap.Find(pTargetTechno)->Shield.get())
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
		auto const pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	return 0;
}

DEFINE_HOOK(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x18, 0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	if (!pWeapon->Projectile->AA && pSecWeapon->Projectile->AA && pTargetTechno && pTargetTechno->IsInAir())
		return Secondary;

	return Primary;
}

DEFINE_HOOK(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { ReturnValue = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	int oddWeaponIndex = 2 * pThis->CurrentGattlingStage;
	int evenWeaponIndex = oddWeaponIndex + 1;
	int chosenWeaponIndex = oddWeaponIndex;
	int eligibleWeaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true);

	if (eligibleWeaponIndex != -1)
	{
		chosenWeaponIndex = eligibleWeaponIndex;
	}
	else if (pTargetTechno)
	{
		auto const pWeaponOdd = pThis->GetWeapon(oddWeaponIndex)->WeaponType;
		auto const pWeaponEven = pThis->GetWeapon(evenWeaponIndex)->WeaponType;
		bool skipRemainingChecks = false;

		if (const auto pShield = TechnoExt::ExtMap.Find(pTargetTechno)->Shield.get())
		{
			if (pShield->IsActive() && !pShield->CanBeTargeted(pWeaponOdd))
			{
				chosenWeaponIndex = evenWeaponIndex;
				skipRemainingChecks = true;
			}
		}

		if (!skipRemainingChecks)
		{
			if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, pTargetTechno->GetTechnoType()->Armor) == 0.0)
			{
				chosenWeaponIndex = evenWeaponIndex;
			}
			else
			{
				auto pCell = pTargetTechno->GetCell();
				bool isOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pTargetTechno->IsInAir();

				if (!pTargetTechno->OnBridge && isOnWater)
				{
					int navalTargetWeapon = pThis->SelectNavalTargeting(pTargetTechno);

					if (navalTargetWeapon == 2)
						chosenWeaponIndex = evenWeaponIndex;
				}
				else if ((pTargetTechno->IsInAir() && !pWeaponOdd->Projectile->AA && pWeaponEven->Projectile->AA) ||
					!pTargetTechno->IsInAir() && pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_secondary)
				{
					chosenWeaponIndex = evenWeaponIndex;
				}
			}
		}
	}

	R->EAX(chosenWeaponIndex);
	return ReturnValue;
}