#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>

#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

bool DisguiseAllowed(const TechnoTypeExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

//https://github.com/Phobos-developers/Phobos/pull/1073/
DEFINE_HOOK(0x6FE562, TechnoClass_FireAt_BurstRandomTarget, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(BulletClass*, pBullet, EAX);

	if (!pBullet || pWeapon->Burst < 2)
		return 0;

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	if (pWeaponExt->Burst_Retarget <= 0.0)
		return 0;

	const int retargetProbability = pWeaponExt->Burst_Retarget > 1.0 ? 100 : (int)std::round(pWeaponExt->Burst_Retarget * 100);

	if (retargetProbability < ScenarioClass::Instance->Random.RandomRanged(1, 100))
		return 0;

	auto pThisType = pThis->GetTechnoType();
	std::vector<TechnoClass*> candidates;
	auto originalTarget = pThis->Target;
	auto pThisExt = TechnoExtContainer::Instance.Find(pThis);
	int minimumRange = pWeapon->MinimumRange;
	int range = pWeapon->Range;
	int airRange = pWeapon->Range + pThisType->AirRangeBonus;

	for (auto pTarget : *TechnoClass::Array)
	{
		if (pTarget == originalTarget)
			continue;

		const auto FireError = pThis->GetFireError(pTarget, pThisExt->CurrentWeaponIdx, false);
		if (FireError != FireError::OK)
			continue;

		int distanceFromAttacker = pThis->DistanceFrom(pTarget);
		if (distanceFromAttacker < minimumRange)
			continue;

		const auto rangemax = (pTarget->IsInAir() ? airRange : range);

		if (pWeapon->OmniFire) {
			if(distanceFromAttacker <= rangemax)
				candidates.push_back(pTarget);
		}
		else
		{
			int distanceFromOriginalTarget = pTarget->DistanceFrom(originalTarget);

			if (distanceFromAttacker <= rangemax && distanceFromOriginalTarget <= rangemax) {
				candidates.push_back(pTarget);
			}
		}
	}

	if (!candidates.empty()) {
		// Pick one new target from the list of targets inside the weapon range
		pBullet->Target = candidates[ScenarioClass::Instance->Random.RandomFromMax(candidates.size() - 1)];
	}

	return 0;
}

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire_PreFiringChecks, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));

	enum { FireIllegal = 0x6FCB7E, Continue = 0x0 , FireCant = 0x6FCD29 };

	auto const pObjectT = generic_cast<ObjectClass*>(pTarget);

	if (pWeapon->Warhead->MakesDisguise && pObjectT) {
		if (!DisguiseAllowed(TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType()), pObjectT->GetDisguise(true)))
			return FireIllegal;
	}

	// Ares TechnoClass_GetFireError_OpenToppedGunnerTemporal
	// gunners and opentopped together do not support temporals, because the gunner
	// takes away the TemporalImUsing from the infantry, and thus it is missing
	// when the infantry fires out of the opentopped vehicle
	if (pWeapon->Warhead->Temporal && pThis->Transporter) {
		auto const pType = pThis->Transporter->GetTechnoType();
		if (pType->Gunner && pType->OpenTopped) {
			if(!pThis->TemporalImUsing)
				return FireCant;
		}
	}

	//if (!TechnoExtData::FireOnceAllowFiring(pThis, pWeapon, pTarget))
	//	return FireCant;

	if (!TechnoExtData::ObjectHealthAllowFiring(pObjectT, pWeapon))
		return FireIllegal;

	if (PassengersFunctional::CanFire(pThis))
		return FireIllegal;

	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeapon->Warhead))
		return FireIllegal;

	if (!TechnoExtData::InterceptorAllowFiring(pThis, pObjectT))
		return FireIllegal;

	auto const& [pTargetTechno, targetCell] = TechnoExtData::GetTargets(pObjectT, pTarget);

	// AAOnly doesn't need to be checked if LandTargeting=1.
	if ((!pTargetTechno
		|| pTargetTechno->GetTechnoType()->LandTargeting != LandTargetingType::Land_not_okay)
		) {
		if (BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AAOnly)
			return FireIllegal;
	}

	if (!TechnoExtData::CheckCellAllowFiring(targetCell, pWeapon))
		return FireIllegal;

	if (pTargetTechno)
	{

		if (!TechnoExtData::TechnoTargetAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		//if (!TechnoExtData::TargetTechnoShieldAllowFiring(pTargetTechno, pWeapon))
		//	return FireIllegal;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;
	}

	return Continue;
}

// Weapon Firing
DEFINE_HOOK(0x6FE43B, TechnoClass_FireAt_OpenToppedDmgMult, 0x6) //7
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		if (auto const  pTransport = pThis->Transporter)
		{
			float nDamageMult = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType())->OpenTopped_DamageMultiplier
				.Get(RulesClass::Instance->OpenToppedDamageMultiplier);
			R->EAX(int(nDamage * nDamageMult));
			return ApplyDamageMult;
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
{
	enum { Continue = 0x0, DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass*, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	switch (TechnoExtData::ApplyAreaFire(pThis, pCell, pWeaponType))
	{
	case AreaFireReturnFlag::Continue:
	{
		R->EAX(pCell);
		return Continue;
	}
	case AreaFireReturnFlag::DoNotFire:
	{
		return DoNotFire;
	}
	case AreaFireReturnFlag::SkipSetTarget:
	{
		R->EAX(pThis);
		return SkipSetTarget;
	}
	default:
		return Continue;
		break;
	}
}

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		auto const  pExt = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType());
		if (pTransport->Deactivated && !pExt->OpenTopped_AllowFiringIfDeactivated)
			return DisallowFiring;

		//if(IS_SAME_STR_("LYNXBUFF" , pThis->get_ID()))
		// if(pTransport->IsBeingWarpedOut()){
		 //	return DisallowFiring;
		//}
	}

	return 0;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = pThis->GetTechnoType();

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = pWeapon->Range;
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThisType);

		if (pThisType->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding.Get())
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else //if (pPassenger->HasTurret())
					//tWeaponIndex = pPassenger->CurrentWeaponNumber;
					tWeaponIndex = pPassenger->SelectWeapon(pThis->Target);

				const auto pTWeapon = pPassenger->GetWeapon(tWeaponIndex);

				if (pTWeapon->WeaponType && pTWeapon->WeaponType->FireInTransport)
				{
					if (pTWeapon->WeaponType->Range < smallestRange)
					{
						smallestRange = pTWeapon->WeaponType->Range;
					}
				}

				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	if (result == 0 && pThis->WhatAmI() == AircraftClass::AbsID && pThisType->OpenTopped)
	{
		result = pThisType->GuardRange;
		if (result == 0)
			Debug::Log("Warning ! , range of Aircraft[%s] return 0 result will cause Aircraft to stuck ! \n", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x20, 0x8));
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = specific_cast<CellClass*>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_none &&
			((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach)) && !pCell->ContainsBridge())
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = specific_cast<TerrainClass*>(pTarget))
	{
		pCell = pTerrain->GetCell();

		if (pType->LandTargeting == LandTargetingType::Land_not_okay && pCell->LandType != LandType::Water && pCell->LandType != LandType::Beach)
			return DisallowFiring;
		//else if (pType->LandTargeting == LandTargetingType::Land_secondary && nWeaponIdx == 0)
		//	return DisallowFiring;
		else if (pType->NavalTargeting == NavalTargetingType::Naval_none && ((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pCell->ContainsBridge()))
			return DisallowFiring;
	}

	return 0;
}

DEFINE_HOOK(0x6FC815, TechnoClass_CanFire_CellTargeting, 0x7)
{
	enum
	{
		LandTargetingCheck = 0x6FC857,
		SkipLandTargetingCheck = 0x6FC879
	};

	GET(AbstractClass*, pTarget, EBX);
	GET(TechnoClass*, pThis, ESI);

	CellClass* pCell = specific_cast<CellClass*>(pTarget);
	if (!pCell)
		return SkipLandTargetingCheck;

	if (pCell->ContainsBridge())
		return LandTargetingCheck;

	if (pCell->LandType == LandType::Water && pThis->GetTechnoType()->NavalTargeting == NavalTargetingType::Naval_none)
		return LandTargetingCheck;

	return pCell->LandType == LandType::Beach || pCell->LandType == LandType::Water ?
		LandTargetingCheck : SkipLandTargetingCheck;
}

#pragma region WallWeaponStuff

// NOINLINE WeaponStruct* TechnoClass_GetWeaponAgainstWallWrapper(TechnoClass* pThis) {
// 	auto const weaponPrimary = pThis->GetWeapon(0);
//
// 	if (!weaponPrimary->WeaponType->Warhead->Wall) {
// 		auto const weaponSecondary = pThis->GetWeapon(1);
//
// 		if (weaponSecondary
// 			&& weaponSecondary->WeaponType
// 			&& weaponSecondary->WeaponType->Warhead->Wall
// 			 && !TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType())->NoSecondaryWeaponFallback) {
// 			return weaponSecondary;
// 		}
// 	}
//
// 	return weaponPrimary;
// }

DEFINE_HOOK(0x51C1F1, InfantryClass_CanEnterCell_WallWeapon, 0x5)
{
	enum { SkipGameCode = 0x51C1FE };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

DEFINE_HOOK(0x73F495, UnitClass_CanEnterCell_WallWeapon, 0x6)
{
	enum { SkipGameCode = 0x73F4A1 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

DEFINE_HOOK(0x70095A, TechnoClass_WhatAction_WallWeapon, 0x6) {
	GET_STACK(OverlayTypeClass*, pOverlayTypeClass, STACK_OFFSET(0x2C, -0x18));
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));
	return 0;
}

namespace CellEvalTemp {
	int weaponIndex;
}

DEFINE_HOOK(0x6F8C9D, TechnoClass_EvaluateCell_SetContext, 0x7) {
	GET(int, weaponIndex, EAX);

	CellEvalTemp::weaponIndex = weaponIndex;

	return 0;
}

DEFINE_HOOK(0x6F8CDF, TechnoClass_EvaluateCell_GetWeapon, 0xA)
{
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeapon(CellEvalTemp::weaponIndex));
	return 0x6F8CE9;
}

DEFINE_HOOK(0x6F8DCC, TechnoClass_EvaluateCell_GetWeaponRange, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeaponRange(CellEvalTemp::weaponIndex));
	return 0x6F8DD8;
}

#pragma endregion

//DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_DropPassenger, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(AbstractClass*, pTarget, EDI);
//	GET(WeaponTypeClass*, pWeapon, EBX);
//
//	if (pThis->Passengers.FirstPassenger)
//	{
//		// TODO : implement this for UnitClass
//		pThis->DropOffParadropCargo();
//	}
//
//	return 0x0;
//}