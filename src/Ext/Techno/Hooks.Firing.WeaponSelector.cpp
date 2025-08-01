#include "Body.h"

#include <Ext/BuildingType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/EnumFunctions.h>

// Weapon Selection
// TODO : check
ASMJIT_PATCH(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum {
		ReturnGameCode = 0x6F3341,
		ReturnHandled = 0x6F3406 ,
		CheckOccupy = 0x6F3379,
		SetWeaponSlot = 0x6F3360
	};

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(const AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	if (pTarget) {
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor() && pTarget->WhatAmI() == BulletClass::AbsID) {
			R->EAX(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
			return ReturnHandled;
		}
	}

	if(pTypeExt->AttachedToObject->TurretCount > 0 && !pTypeExt->AttachedToObject->IsGattling) {
		if (pTypeExt->MultiWeapon && (pThis->WhatAmI() != AbstractType::Unit || !pTypeExt->AttachedToObject->Gunner)) {
			return CheckOccupy;
		}

		return SetWeaponSlot;
	}

	return CheckOccupy;
}

ASMJIT_PATCH(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745, UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (const auto pCell = cast_to<CellClass*>(pTarget))
	{
		auto const pWeaponPrimary = pThis->GetWeapon(0)->WeaponType;
		auto const pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;
		const auto pPrimaryExt = WeaponTypeExtContainer::Instance.Find(pWeaponPrimary);

		if (pWeaponSecondary && !pPrimaryExt->SkipWeaponPicking && (!EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true, true)
			|| (pPrimaryExt->AttachEffect_CheckOnFirer && !pPrimaryExt->HasRequiredAttachedEffects(pThis, pThis))
		))
		{
			R->EAX(1);
			return UseWeaponIndex;
		}
		else if (pCell->OverlayTypeIndex != -1)
		{
			auto const pOverlayType = OverlayTypeClass::Array()->Items[pCell->OverlayTypeIndex];

			if (pOverlayType->Wall && pCell->OverlayData >> 4 != pOverlayType->DamageLevels)
			{
				R->EAX(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayType));
				return UseWeaponIndex;
			}
		}
	}

	return 0;
}

int ApplyForceWeaponInRange(TechnoClass* pThis, AbstractClass* pTarget)
{
	int forceWeaponIndex = -1;
	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	const bool useAASetting = !pTypeExt->ForceAAWeapon_InRange.empty() && pTarget->IsInAir();
	auto const& weaponIndices = useAASetting ? pTypeExt->ForceAAWeapon_InRange : pTypeExt->ForceWeapon_InRange;
	auto const& rangeOverrides = useAASetting ? pTypeExt->ForceAAWeapon_InRange_Overrides : pTypeExt->ForceWeapon_InRange_Overrides;
	const bool applyRangeModifiers = useAASetting ? pTypeExt->ForceAAWeapon_InRange_ApplyRangeModifiers : pTypeExt->ForceWeapon_InRange_ApplyRangeModifiers;

	const int defaultWeaponIndex = pThis->SelectWeapon(pTarget);
	const int currentDistance = pThis->DistanceFrom(pTarget);
	auto const pDefaultWeapon = pThis->GetWeapon(defaultWeaponIndex)->WeaponType;

	for (size_t i = 0; i < weaponIndices.size(); i++)
	{
		int range = 0;

		// Value below 0 means Range won't be overriden
		if (i < rangeOverrides.size() && rangeOverrides[i] > 0)
			range = static_cast<int>(rangeOverrides[i] * Unsorted::LeptonsPerCell);

		if (weaponIndices[i] >= 0)
		{
			if (range > 0 || applyRangeModifiers)
			{
				auto const pWeapon = weaponIndices[i] == defaultWeaponIndex ? pDefaultWeapon : pThis->GetWeapon(weaponIndices[i])->WeaponType;
				range = range > 0 ? range : pWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis, range);
			}

			if (currentDistance <= range)
			{
				forceWeaponIndex = weaponIndices[i];
				break;
			}
		}
		else
		{
			if (range > 0 || applyRangeModifiers)
			{
				range = range > 0 ? range : pDefaultWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExtData::GetRangeWithModifiers(pDefaultWeapon, pThis, range);
			}

			// Don't force weapon if range satisfied
			if (currentDistance <= range)
				break;
		}
	}

	return forceWeaponIndex;
}

namespace ForceWeaponInRangeTemp
{
	bool SelectWeaponByRange = false;
}

//this hook disregard everything and return weapon index
ASMJIT_PATCH(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	GET(TechnoTypeClass*, pThisTechnoType, EAX);
	//GET(TechnoClass*, pTarget, EBP);
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pAbsTarget, STACK_OFFSET(0x18, 0x4));
	//GET(WeaponTypeClass* , pSecondary , EDI);
	//GET(WeaponTypeClass* , pSecondary , EBX);

	const auto pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pThisTechnoType);

	if(!pAbsTarget)
		return 0x0;

	int forceWeaponIndex = -1;


	auto const pTarget = flag_cast_to<TechnoClass*, false>(pAbsTarget);


	if (!ForceWeaponInRangeTemp::SelectWeaponByRange && pTechnoTypeExt->ForceWeapon_Check) {
		TechnoTypeClass* pTargetType = nullptr;

		if (pTarget)
		{
			pTargetType = pTarget->GetTechnoType();

			if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0
				&& pTargetType->Cloakable
				&& pTargetType->Naval
				&& pTarget->CloakState == CloakState::Uncloaked)
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Naval_Decloaked;
			}
			else if (pTechnoTypeExt->ForceWeapon_Cloaked >= 0
				&& pTarget->CloakState == CloakState::Cloaked)
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Cloaked;
			}
			else if (pTechnoTypeExt->ForceWeapon_Disguised >= 0
				&& pTarget->IsDisguised())
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Disguised;
			}
			else if (pTechnoTypeExt->ForceWeapon_UnderEMP >= 0
				&& pTarget->IsUnderEMP())
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_UnderEMP;
			}
		}

		if (forceWeaponIndex == -1
			&& (pTarget || !pTechnoTypeExt->ForceWeapon_InRange_TechnoOnly)
			&& (!pTechnoTypeExt->ForceWeapon_InRange.empty() || !pTechnoTypeExt->ForceAAWeapon_InRange.empty()))
		{
			ForceWeaponInRangeTemp::SelectWeaponByRange = true;
			forceWeaponIndex = ApplyForceWeaponInRange(pThis, pAbsTarget);
			ForceWeaponInRangeTemp::SelectWeaponByRange = false;
		}

		if (forceWeaponIndex == -1 && pTargetType)
		{
			switch (pTarget->WhatAmI())
			{
			case AbstractType::Building:
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Buildings;

				if (pTechnoTypeExt->ForceWeapon_Defenses >= 0)
				{
					auto const pBuildingType = static_cast<BuildingTypeClass*>(pTargetType);

					if (pBuildingType->BuildCat == BuildCat::Combat)
						forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Defenses;
				}

				break;
			}
			case AbstractType::Infantry:
			{
				forceWeaponIndex = (pTechnoTypeExt->ForceAAWeapon_Infantry >= 0 && pTarget->IsInAir())
					? pTechnoTypeExt->ForceAAWeapon_Infantry : pTechnoTypeExt->ForceWeapon_Infantry;

				break;
			}
			case AbstractType::Unit:
			{
				forceWeaponIndex = (pTechnoTypeExt->ForceAAWeapon_Units >= 0 && pTarget->IsInAir())
					? pTechnoTypeExt->ForceAAWeapon_Units : ((pTechnoTypeExt->ForceWeapon_Naval_Units >= 0 && pTargetType->Naval)
					? pTechnoTypeExt->ForceWeapon_Naval_Units : pTechnoTypeExt->ForceWeapon_Units);

				break;
			}
			case AbstractType::Aircraft:
			{
				forceWeaponIndex = (pTechnoTypeExt->ForceAAWeapon_Aircraft >= 0 && pTarget->IsInAir())
					? pTechnoTypeExt->ForceAAWeapon_Aircraft : pTechnoTypeExt->ForceWeapon_Aircraft;

				break;
			}
			}
		}

		if (forceWeaponIndex >= 0) {
			R->EAX(forceWeaponIndex);
			return 0x6F37AF;
		}
	}

	 const int multiWeaponIndex = pTechnoTypeExt->SelectMultiWeapon(pThis, pTarget);

	 if (multiWeaponIndex >= 0)
	 {
 		R->EAX(multiWeaponIndex);
 		return 0x6F37AF;
	 }

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
//	if(auto pObj = flag_cast_to<ObjectClass*>(pTarget)){
//		if(!pObj->IsAlive) {
//			Debug::LogInfo("[{}] {} {} Attempt to target death Object of {}!"
//				, callerAddress ,(void*)pThis , pThis->get_ID() , (void*)pTarget);
//
//			R->Stack(0x4 , nullptr);
//		}
//	}
//
//	return 0x0;
//}
//#pragma optimize("", on )

ASMJIT_PATCH(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, 0x18 + 0x4);
	GET_STACK(WeaponTypeClass*, pSecondary, 0x10); //secondary
	GET_STACK(WeaponTypeClass*, pPrimary, 0x14); //primary

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
	//	Debug::LogInfo("Caller[%x] Techno[%s] Trying to target possibly dead Techno[%x] FromOwner [%s]", calleraddr ,  pThis->get_ID(), pTargetTechno , pTargetTechno->align_154->OriginalHouseType->ID);
	//	calleraddr = -1;
	//	return OriginalCheck;
	//}

	if (const auto pShield = pTargetExt->GetShield())
	{
		if (pShield->IsActive())
		{
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pSecondary && pSecondary->Projectile->AA;

			if (pSecondary && (allowFallback || (allowAAFallback && secondaryIsAA) || TechnoExtData::CanFireNoAmmoWeapon(pThis, 1)))
			{
				if (!pShield->CanBeTargeted(pPrimary))
					return Secondary;
				else
					return FurtherCheck;
			}

			return Primary;
		}
	}

	const int nArmor = (int)TechnoExtData::GetArmor(pTargetTechno);
	//if ((size_t)nArmor > ArmorTypeClass::Array.size())
	//	Debug::LogInfo(__FUNCTION__" Armor is more that avaible ArmorTypeClass ");

	const auto vsData_Secondary = &WarheadTypeExtContainer::Instance.Find(pSecondary->Warhead)->Verses[nArmor];

	if (vsData_Secondary->Verses == 0.0)
		return Primary;

	const auto vsData_Primary = &WarheadTypeExtContainer::Instance.Find(pPrimary->Warhead)->Verses[nArmor];

	return vsData_Primary->Verses != 0.0 ? FurtherCheck : Secondary;
}

// ASMJIT_PATCH(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(WeaponTypeClass* const, pWeapon, EBX);
// 	GET_BASE(int, weaponIndex, 0xC);
//
// 	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeapon->IsLaser)
// 	{
// 		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
//
// 		if (pExt->CurrentLaserWeaponIndex.empty())
// 			pExt->CurrentLaserWeaponIndex = weaponIndex;
// 		else
// 			pExt->CurrentLaserWeaponIndex.clear();
// 	}
//
// 	return 0;
// }

ASMJIT_PATCH(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
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

ASMJIT_PATCH(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
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

			if (Math::abs(
				//GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead , pTargetTechno->GetTechnoType()->Armor)
				WarheadTypeExtContainer::Instance.Find(pWeaponOdd->Warhead)->GetVerses(
					TechnoExtData::GetArmor(pTargetTechno)).Verses
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

// ASMJIT_PATCH(0x6F34B7, TechnoClass_WhatWeaponShouldIUse_AllowAirstrike, 0x6)
// {
// 	enum { SkipGameCode = 0x6F34BD };
// 	GET(BuildingTypeClass*, pThis, ECX);
//
// 	if (pThis)
// 	{
// 		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
// 		return SkipGameCode;
// 	}

// 	return 0x0;
// }
//
// ASMJIT_PATCH(0x51EAF2, TechnoClass_WhatAction_AllowAirstrike, 0x6)
// {
// 	enum { SkipGameCode = 0x51EAF8 };
// 	GET(BuildingTypeClass*, pThis, ESI);
//
// 	if (pThis)
// 	{
// 		R->AL(BuildingTypeExtContainer::Instance.Find(pThis)->AllowAirstrike.Get(pThis->CanC4));
// 		return SkipGameCode;
// 	}
//
// 	return 0x0;
// }

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
ASMJIT_PATCH(0x70E1A0, TechnoClass_GetTurretWeapon_LaserWeapon, 0x5)
{
	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(DWORD , caller , 0x0);

	if(!pThis)
		Debug::FatalError("Caller %u " , caller);

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