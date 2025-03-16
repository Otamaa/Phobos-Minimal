ASMJIT_PATCH(0x518FBC, InfantryClass_DrawIt_DontRenderSHP, 0x6)
{
	enum { SkipDrawCode = 0x5192B5 };

	GET(InfantryClass*, pThis, EBP);

	if (TechnoExtContainer::Instance.Find(pThis)->IsWebbed && pThis->ParalysisTimer.GetTimeLeft() > 0)
		return SkipDrawCode;

	return 0;
}

//broke spawner building ?
//ASMJIT_PATCH(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8) //7
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
//ASMJIT_PATCH(0x5218E0, InfantryClass_SelectWeapon_IsTargetTechnoAlive, 0x9)
//{
//	GET(InfantryClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	if(pTarget && Is_Techno(pTarget) && !static_cast<TechnoClass*>(pTarget)->IsAlive)
//		Debug::LogInfo("Caller[%x] InfantryClass_SelectWeapon[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]", callerAddress, pThis->get_ID(), static_cast<TechnoClass*>(pTarget), static_cast<TechnoClass*>(pTarget)->align_154->OriginalHouseType->ID);
//
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6F3330, TechnoClass_SelectWeapon_IsTechnoTargetAlive, 5)
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

