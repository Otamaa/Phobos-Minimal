#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Weapon Selection
DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { ReturnGameCode = 0x6F3341, ReturnHandled = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (!pThis)
		Debug::FatalErrorAndExit(__FUNCTION__" Has Missing TechnoPointer ! \n");

	if (pTarget)
	{
		if (const auto pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType()))
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
		auto pTechnoType = pTechno->GetTechnoType();
		if (!pTechnoType)
			return 0;

		auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target);
		if (!pTarget)
			return 0;

		auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		if (auto pTechnoTypeExt = TechnoTypeExt::GetExtData(pTechnoType))
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

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	enum { Primary = 0x6F37AD,
		Secondary = 0x6F3745,
		FurtherCheck = 0x6F3754,
		OriginalCheck = 0x6F36E3
	};

	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	if (const auto pTypeExt = TechnoTypeExt::GetExtData(pThis->GetTechnoType()))
	{
		if (const auto pSecondary = pThis->GetWeapon(1))
		{
			if (const auto pSecondaryExt = WeaponTypeExt::ExtMap.Find(pSecondary->WeaponType))
			{
				if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondaryExt->CanTarget.Get(), true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondaryExt->CanTarget.Get()) ||
						!EnumFunctions::CanTargetHouse(pSecondaryExt->CanTargetHouses.Get(), pThis->Owner, pTargetTechno->Owner))))
				{
					return Primary;
				}

				if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
				{
					if (pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
						return Primary;

					if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pPrimaryExt->CanTarget.Get(), true)) ||
						(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pPrimaryExt->CanTarget.Get()) ||
							!EnumFunctions::CanTargetHouse(pPrimaryExt->CanTargetHouses.Get(), pThis->Owner, pTargetTechno->Owner))))
					{
						return Secondary;
					}
				}
			}
		}

		if (!pTargetTechno)
			return Primary;

		if (const auto pTargetExt = TechnoExt::GetExtData(pTargetTechno))
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
