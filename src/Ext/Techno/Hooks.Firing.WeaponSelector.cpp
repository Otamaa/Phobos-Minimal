#include "Body.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/EnumFunctions.h>

// Weapon Selection
// TODO : check
DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { ReturnGameCode = 0x6F3341, ReturnHandled = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(const AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (pTarget)
	{
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor() && pTarget->WhatAmI() == BulletClass::AbsID)
		{
			R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
			return ReturnHandled;
		}

		//if (pTypeExt->Get()->AttackFriendlies && pTypeExt->AttackFriendlies_WeaponIdx != -1)
		//{
		//	R->EAX(pTypeExt->Interceptor_Weapon);
		//	return ReturnHandled;
		//}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());

	return ReturnGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745, UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = specific_cast<CellClass*>(pTarget))
	{
		auto const pWeaponPrimary = pThis->GetWeapon(0)->WeaponType;
		auto const pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;
		const auto pPrimaryExt = WeaponTypeExtContainer::Instance.Find(pWeaponPrimary);

		if (pWeaponSecondary && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true, true))
		{
			R->EAX(1);
			return UseWeaponIndex;
		}
		else if (pCell->OverlayTypeIndex != -1)
		{
			R->EAX(TechnoExtData::GetWeaponIndexAgainstWall(pThis, OverlayTypeClass::Array()->GetItem(pCell->OverlayTypeIndex)));
			return UseWeaponIndex;
		}
	}

	return 0;
}

//this hook disregard everything and return weapon index
DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	enum
	{
		ReturnHandled = 0x6F37AF
	};

	GET(TechnoTypeClass*, pThisTechnoType, EAX);
	GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ECX);
	//GET(WeaponTypeClass* , pSecondary , EDI);
	//GET(WeaponTypeClass* , pSecondary , EBX);

	if (pTarget)
	{
		const auto pTargetType = pTarget->GetTechnoType();
		const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pThisTechnoType);

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
			R->EAX(pTechnoTypeExt->ForceWeapon_Cloaked.Get());
			return ReturnHandled;
		}

		if (pTechnoTypeExt->ForceWeapon_Disguised >= 0 &&
			   pTarget->IsDisguised())
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_Disguised.Get());
			return ReturnHandled;
		}

		if (pTechnoTypeExt->ForceWeapon_UnderEMP >= 0 && pTarget->IsUnderEMP())
		{
			R->EAX(pTechnoTypeExt->ForceWeapon_UnderEMP.Get());
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
//	if (const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())) {
//		int weaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, !pTypeExt->NoSecondaryWeaponFallback);
//		return (weaponIndex != -1) && weaponIndex == 1 ? Secondary : Primary;
//	}
//
//	return OriginalCheck;
//}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
//int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
//{
//	CellClass* pTargetCell = nullptr;
//
//	// Ignore target cell for airborne target technos.
//	if (!pTargetTechno || !pTargetTechno->IsInAir())
//	{
//		if (auto const pCell = abstract_cast<CellClass*>(pTarget))
//			pTargetCell = pCell;
//		else if (auto const pObject = abstract_cast<ObjectClass*>(pTarget))
//			pTargetCell = pObject->GetCell();
//	}
//
//	auto const pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
//	auto const pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);
//
//	if (!pWeaponStructOne && !pWeaponStructTwo)
//		return -1;
//	else if (!pWeaponStructTwo)
//		return weaponIndexOne;
//	else if (!pWeaponStructOne)
//		return weaponIndexTwo;
//
//	auto const pWeaponOne = pWeaponStructOne->WeaponType;
//	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;
//
//	if (auto const pSecondExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponTwo))
//	{
//		if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true , true)) ||
//			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
//				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
//		{
//			return weaponIndexOne;
//		}
//		else if (auto const pFirstExt = WeaponTypeExtContainer::Instance.TryFind(pWeaponOne))
//		{
//			bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
//
//			if (!allowFallback && (!allowAAFallback || !secondaryIsAA) && !TechnoExtData::CanFireNoAmmoWeapon(pThis, 1))
//				return weaponIndexOne;
//
//			if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true , true)) ||
//				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
//					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
//			{
//				return weaponIndexTwo;
//			}
//		}
//	}
//
//	auto const pType = pThis->GetTechnoType();
//
//	// Handle special case with NavalTargeting / LandTargeting.
//	if (!pTargetTechno && pTargetCell && (pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary)
//		&& pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
//	{
//		return weaponIndexTwo;
//	}
//
//	return -1;
//}
//
//#include <Ares_TechnoExt.h>

//#pragma optimize("", off )
//static uintptr_t calleraddr = -1;
//DEFINE_HOOK(0x5218E0, InfantryClass_SelectWeapon_IsTargetTechnoAlive, 0x9)
//{
//	GET(InfantryClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	if(pTarget && Is_Techno(pTarget) && !static_cast<TechnoClass*>(pTarget)->IsAlive)
//		Debug::Log("Caller[%x] InfantryClass_SelectWeapon[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]\n", callerAddress, pThis->get_ID(), static_cast<TechnoClass*>(pTarget), static_cast<TechnoClass*>(pTarget)->align_154->OriginalHouseType->ID);
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6F3330, TechnoClass_SelectWeapon_IsTechnoTargetAlive, 5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	calleraddr = callerAddress;
//
//	return 0x0;
//}
//#pragma optimize("", on )

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, 0x18 + 0x4);
	//GET_STACK(WeaponTypeClass*, pPrimary,0x14);
	//GET_STACK(WeaponTypeClass*, pSecondary ,0x10);

	enum
	{
		Primary = 0x6F37AD,
		Secondary = 0x6F3745,
		Secondary_b = 0x6F3807,
		FurtherCheck = 0x6F3754,
		OriginalCheck = 0x6F36E3
	};

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	bool allowFallback = !pTypeExt->NoSecondaryWeaponFallback;
	bool allowAAFallback = allowFallback ? true : pTypeExt->NoSecondaryWeaponFallback_AllowAA;
	const int weaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, allowFallback, allowAAFallback);

	if (weaponIndex != -1)
		return weaponIndex == 1 ? Secondary : Primary;

	if (!pTargetTechno || !pTargetTechno->IsAlive)
		return Primary;

	//select weapon is executed with dead target ?
	const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);

	//if (!pTargetExt) {
	//	Debug::Log("Caller[%x] Techno[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]\n", calleraddr ,  pThis->get_ID(), pTargetTechno , pTargetTechno->align_154->OriginalHouseType->ID);
	//	calleraddr = -1;
	//	return OriginalCheck;
	//}

	if (const auto pShield = pTargetExt->GetShield())
	{
		if (pShield->IsActive())
		{
			const auto secondary = pThis->GetWeapon(1)->WeaponType;
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && secondary && secondary->Projectile->AA;

			if (secondary && (allowFallback || (allowAAFallback && secondaryIsAA) || TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)))
			{
				if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
					return Secondary;
				else
					return FurtherCheck;
			}

			return Primary;
		}
	}

	return OriginalCheck;
}

DEFINE_HOOK(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeapon->IsLaser)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

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

	//GET(TechnoClass*, pThis, ESI);
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
	int eligibleWeaponIndex = TechnoExtData::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true,true);

	if (eligibleWeaponIndex != -1)
	{
		chosenWeaponIndex = eligibleWeaponIndex;
	}
	else if (pTargetTechno)
	{
		auto const pWeaponOdd = pThis->GetWeapon(oddWeaponIndex)->WeaponType;
		auto const pWeaponEven = pThis->GetWeapon(evenWeaponIndex)->WeaponType;
		bool skipRemainingChecks = false;

		if (const auto pShield = TechnoExtContainer::Instance.Find(pTargetTechno)->GetShield())
		{
			if (pShield->IsActive() && !pShield->CanBeTargeted(pWeaponOdd))
			{
				chosenWeaponIndex = evenWeaponIndex;
				skipRemainingChecks = true;
			}
		}

		if (!skipRemainingChecks)
		{

			if (std::abs(
				//GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead , pTargetTechno->GetTechnoType()->Armor)
				WarheadTypeExtContainer::Instance.Find(pWeaponOdd->Warhead)->GetVerses(TechnoExtData::GetArmor(pTargetTechno)).Verses
			) == 0.0)
			{
				chosenWeaponIndex = evenWeaponIndex;
			}
			else
			{
				const auto pCell = pTargetTechno->GetCell();
				bool isOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pTargetTechno->IsInAir();

				if (!pTargetTechno->OnBridge && isOnWater)
				{
					NavalTargetingType navalTargetWeapon = pThis->SelectNavalTargeting(pTargetTechno);

					if (navalTargetWeapon == NavalTargetingType::Underwater_only)
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

DEFINE_HOOK(0x6F34B7, TechnoClass_WhatWeaponShouldIUse_AllowAirstrike, 0x6)
{
	enum { SkipGameCode = 0x6F34BD };
	GET(BuildingTypeClass*, pThis, ECX);

	if (pThis)
	{
		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
		return SkipGameCode;
	}

	return 0x0;
}

DEFINE_HOOK(0x51EAF2, TechnoClass_WhatAction_AllowAirstrike, 0x6)
{
	enum { SkipGameCode = 0x51EAF8 };
	GET(BuildingTypeClass*, pThis, ESI);

	if (pThis)
	{
		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
		return SkipGameCode;
	}

	return 0x0;
}

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A0, TechnoClass_GetTurretWeapon_LaserWeapon, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);

	if (pThis->WhatAmI() == BuildingClass::AbsID)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (!pExt->CurrentLaserWeaponIndex.empty()) {
			R->EAX(pThis->GetWeapon(pExt->CurrentLaserWeaponIndex));
			return 0x70E1C8;
		}
	}

	return 0;
}