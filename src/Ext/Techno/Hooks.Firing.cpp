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

#include <Utilities/Macro.h>

#include <TerrainClass.h>

bool DisguiseAllowed(const TechnoTypeExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

ASMJIT_PATCH(0x6FED2F, TechnoClass_FireAt_VerticalInitialFacing, 0x6)
{
	enum { Continue = 0x6FED39, SkipGameCode = 0x6FED8F };

	GET(FakeBulletTypeClass*, pBulletType, EAX);

	if (pBulletType->_GetExtData()->VerticalInitialFacing.Get(pBulletType->Voxel || pBulletType->Vertical))
		return Continue;

	return SkipGameCode;
}

ASMJIT_PATCH(0x6FF0DD, TechnoClass_FireAt_TurretRecoil, 0x6)
{
	enum { SkipGameCode = 0x6FF15B };

	GET_STACK(WeaponTypeClass* const, pWeapon, STACK_OFFSET(0xB0, -0x70));

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->TurretRecoil_Suppress ? SkipGameCode : 0;
}

// An example for quick tilting test
ASMJIT_PATCH(0x7413DD, UnitClass_Fire_RecoilForce, 0x6)
{
	GET(UnitClass* const, pThis, ESI);

	if (!pThis->IsVoxel())
		return 0;

	GET(BulletClass* const, pTraj, EDI);

	const auto& force = WeaponTypeExtContainer::Instance.Find(pTraj->WeaponType)->RecoilForce;

	if (!force.isset() || Math::abs(force.Get()) < 0.005)
		return 0x0;

	double force_result = force / MaxImpl(pThis->Type->Weight, 1.);

	if (Math::abs(force.Get()) < 0.002)
		return 0;

	const double theta = pThis->GetRealFacing().GetRadian<32>() - pThis->PrimaryFacing.Current().GetRadian<32>();

	pThis->RockingForwardsPerFrame = (float)(-force_result * Math::cos(theta));
	pThis->RockingSidewaysPerFrame = (float)(force_result * Math::sin(theta) * std::pow(pThis->Type->VoxelScaleX / pThis->Type->VoxelScaleY, 2));

	return 0;
}

#include <Ext/Infantry/Body.h>

ASMJIT_PATCH(0x6FF905, TechnoClass_FireAt_FireOnce, 0x6) {
	GET(TechnoClass*, pThis, ESI);

	if (auto const pInf = cast_to<InfantryClass*, false>(pThis))
	{
		GET(WeaponTypeClass*, pWeapon, EBX);

		if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->FireOnce_ResetSequence)
			InfantryExtContainer::Instance.Find(pInf)->SkipTargetChangeResetSequence = true;
	}

	return 0;
}

ASMJIT_PATCH(0x51B20E, InfantryClass_AssignTarget_FireOnce, 0x6)
{
	enum { SkipGameCode = 0x51B255 };

	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBX);

	auto const pExt = InfantryExtContainer::Instance.Find(pThis);

	if (!pTarget && pExt->SkipTargetChangeResetSequence)
	{
		pThis->IsFiring = false;
		pExt->SkipTargetChangeResetSequence = false;
		return SkipGameCode;
	}

	return 0;
}

//https://github.com/Phobos-developers/Phobos/pull/1073/
// ASMJIT_PATCH(0x6FE562, TechnoClass_FireAt_BurstRandomTarget, 0x6)
// {
// 	GET(TechnoClass*, pThis, ESI);
// 	GET(WeaponTypeClass*, pWeapon, EBX);
// 	GET(BulletClass*, pBullet, EAX);
//
// 	if (!pBullet || pWeapon->Burst < 2)
// 		return 0;
//
// 	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
// 	if (pWeaponExt->Burst_Retarget <= 0.0)
// 		return 0;
//
// 	const int retargetProbability = pWeaponExt->Burst_Retarget > 1.0 ? 100 : (int)std::round(pWeaponExt->Burst_Retarget * 100);
//
// 	if (retargetProbability < ScenarioClass::Instance->Random.RandomRanged(1, 100))
// 		return 0;
//
// 	auto pThisType = pThis->GetTechnoType();
// 	std::vector<TechnoClass*> candidates;
// 	auto originalTarget = pThis->Target;
// 	auto pThisExt = TechnoExtContainer::Instance.Find(pThis);
// 	int minimumRange = pWeapon->MinimumRange;
// 	int range = pWeapon->Range;
// 	int airRange = pWeapon->Range + pThisType->AirRangeBonus;
//
// 	for (auto pTarget : *TechnoClass::Array)
// 	{
// 		if (pTarget == originalTarget)
// 			continue;
//
// 		const auto FireError = pThis->GetFireError(pTarget, pThisExt->CurrentWeaponIdx, false);
// 		if (FireError != FireError::OK)
// 			continue;
//
// 		int distanceFromAttacker = pThis->DistanceFrom(pTarget);
// 		if (distanceFromAttacker < minimumRange)
// 			continue;
//
// 		const auto rangemax = (pTarget->IsInAir() ? airRange : range);
//
// 		if (pWeapon->OmniFire) {
// 			if(distanceFromAttacker <= rangemax)
// 				candidates.push_back(pTarget);
// 		}
// 		else
// 		{
// 			int distanceFromOriginalTarget = pTarget->DistanceFrom(originalTarget);
//
// 			if (distanceFromAttacker <= rangemax && distanceFromOriginalTarget <= rangemax) {
// 				candidates.push_back(pTarget);
// 			}
// 		}
// 	}
//
// 	if (!candidates.empty()) {
// 		Pick one new target from the list of targets inside the weapon range
// 		pBullet->Target = candidates[ScenarioClass::Instance->Random.RandomFromMax(candidates.size() - 1)];
// 	}
//
// 	return 0;
// }

// Skips bridge-related coord checks to allow AA to target air units on bridges over water.
ASMJIT_PATCH(0x6FCBE6, TechnoClass_CanFire_BridgeAAFix, 0x6)
{
	enum { SkipChecks = 0x6FCCBD };

	GET(TechnoClass*, pTarget, EBP);

	if (pTarget->IsInAir())
		return SkipChecks;

	return 0;
}

ASMJIT_PATCH(0x6FC3FE, TechnoClass_CanFire_Immunities, 0x6)
{
	enum { FireIllegal = 0x6FC86A, ContinueCheck = 0x6FC425 };

	//GET(TechnoClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWarhead, EAX);
	GET(TechnoClass*, pTarget, EBP);

	if (pTarget)
	{
		//const auto nRank = pTarget->Veterancy.GetRemainingLevel();

		//const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWarhead);
		//if(pWHExt->ImmunityType.isset() &&
		//	 TechnoExtData::HasImmunity(nRank, pTarget , pWHExt->ImmunityType.Get()))
		//	return FireIllegal;

		if (pWarhead->Psychedelic && TechnoExtData::IsPsionicsImmune(pTarget))
			return FireIllegal;
	}

	return ContinueCheck;
}

ASMJIT_PATCH(0x772AA2, WeaponTypeClass_AllowedThreats_AAOnly, 0x5)
{
	GET(BulletTypeClass* const, pType, ECX);

	if (BulletTypeExtContainer::Instance.Find(pType)->AAOnly) {
		R->EAX(4);
		return 0x772AB3;
	}

	return 0;
}

// Pre-Firing Checks
ASMJIT_PATCH(0x6FC339, TechnoClass_CanFire_PreFiringChecks, 0x6) //8
{
	GET(TechnoClass*, pThis, ESI);
	GET(FakeWeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));

	enum { FireIllegal = 0x6FCB7E, Continue = 0x0 , FireCant = 0x6FCD29 };

	auto const pObjectT = flag_cast_to<ObjectClass*, false>(pTarget);
	auto const pTechnoT = flag_cast_to<TechnoClass*, false>(pTarget);
	auto const pWeaponExt = pWeapon->_GetExtData();

	if (pWeaponExt->NoRepeatFire > 0) {
		if (pTechnoT) {
			const auto pTargetTechnoExt = TechnoExtContainer::Instance.Find(pTechnoT);

			if ((Unsorted::CurrentFrame - pTargetTechnoExt->LastBeLockedFrame) < pWeaponExt->NoRepeatFire)
				return FireIllegal;
		}
	}

	if (auto pTerrain = cast_to<TerrainClass*, false>(pTarget))
		if (pTerrain->Type->Immune)
			return FireIllegal;

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

	if (!pWeaponExt->SkipWeaponPicking && !TechnoExtData::ObjectHealthAllowFiring(pObjectT, pWeapon))
		return FireIllegal;

	if (PassengersFunctional::CanFire(pThis))
		return FireIllegal;

	if (!TechnoExtData::CheckFundsAllowFiring(pThis, pWeapon->Warhead))
		return FireIllegal;

	if (!TechnoExtData::InterceptorAllowFiring(pThis, pObjectT))
		return FireIllegal;

	auto const& [pTargetTechno, targetCell] = TechnoExtData::GetTargets(pObjectT, pTarget);

	// AAOnly doesn't need to be checked if LandTargeting=1.
	if (pThis->GetTechnoType()->LandTargeting != LandTargetingType::Land_not_okay && pWeapon->Projectile->AA && pTarget && !pTarget->IsInAir()) {
		if (BulletTypeExtContainer::Instance.Find(pWeapon->Projectile)->AAOnly)
			return FireIllegal;
	}

	if (!TechnoExtData::CheckCellAllowFiring(targetCell, pWeapon))
		return FireIllegal;

	if (pTargetTechno)
	{
		const auto pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);

		if (pWeaponExt->OnlyAttacker.Get() && !pTargetExt->ContainFirer(pWeapon, pThis))
			return FireIllegal;

		if (pThis->Berzerk && !EnumFunctions::CanTargetHouse(RulesExtData::Instance()->BerzerkTargeting, pThis->Owner, pTargetTechno->Owner))
			return FireIllegal;

		if (!TechnoExtData::TechnoTargetAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		//if (!TechnoExtData::TargetTechnoShieldAllowFiring(pTargetTechno, pWeapon))
		//	return FireIllegal;

		if (!TechnoExtData::TargetFootAllowFiring(pThis, pTargetTechno, pWeapon))
			return FireIllegal;

		if (pWeapon->Warhead->Airstrike)
		{
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pWHExt->AirstrikeTargets))
				return FireIllegal;

			if (!TechnoTypeExtContainer::Instance.Find(
					pTargetTechno->GetTechnoType())->AllowAirstrike.Get(
						pTargetTechno->AbstractFlags & AbstractFlags::Foot || static_cast<BuildingClass*>(pTargetTechno)->Type->CanC4))
				return FireIllegal;
		}
	}

	return Continue;
}

ASMJIT_PATCH(0x6FDE0E, TechnoClass_FireAt_OnlyAttacker, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_BASE(AbstractClass* const, pTarget, 0x8);

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeaponExt->OnlyAttacker.Get() && pTarget == pThis->Target
		&& pTarget->AbstractFlags & AbstractFlags::Techno)
	{
		const auto pTargetExt = TechnoExtContainer::Instance.Find(static_cast<TechnoClass*>(pTarget));
		pTargetExt->AddFirer(pWeapon, pThis);
	}

	return 0;
}

ASMJIT_PATCH(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6) //7
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

ASMJIT_PATCH(0x6FC5C7, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { Illegal = 0x6FC86A, OutOfRange = 0x6FC0DF, Continue = 0x6FC5D5 };

	//GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTransport, EAX);
	//GET_STACK(int, weaponIndex, STACK_OFFSET(0x20, 0x8));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType());

	if (pTransport->Transporter || (pTransport->Deactivated && !pTypeExt->OpenTopped_AllowFiringIfDeactivated))
		return Illegal;

	if (pTypeExt->OpenTopped_CheckTransportDisableWeapons
		&& TechnoExtContainer::Instance.Find(pTransport)->AE.flags.DisableWeapons
		//&& pThis->GetWeapon(weaponIndex)->WeaponType
		) return Illegal;
		//return OutOfRange;

	return Continue;
}

// Reimplements the game function with few changes / optimizations
ASMJIT_PATCH(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = pThis->GetTechnoType();

	if (const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		result = WeaponTypeExtData::GetRangeWithModifiers(pWeapon, pThis);
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
					int TWeaponRange = WeaponTypeExtData::GetRangeWithModifiers(pTWeapon->WeaponType, pPassenger);

					if (TWeaponRange < smallestRange) {
						smallestRange = TWeaponRange;
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
			Debug::LogInfo("Warning ! , range of Aircraft[{}] return 0 result will cause Aircraft to stuck ! ", pThis->get_ID());
	}

	R->EAX(result);
	return 0x701393;
}

ASMJIT_PATCH(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	//GET_STACK(int, nWeaponIdx, STACK_OFFSET(0x20, 0x8));
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = cast_to<CellClass*, false>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_none &&
			((pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach)) && !pCell->ContainsBridge())
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = cast_to<TerrainClass*, false>(pTarget))
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

ASMJIT_PATCH(0x6FC815, TechnoClass_CanFire_CellTargeting, 0x7)
{
	enum
	{
		LandTargetingCheck = 0x6FC857,
		SkipLandTargetingCheck = 0x6FC879
	};

	GET(AbstractClass*, pTarget, EBX);
	GET(TechnoClass*, pThis, ESI);

	CellClass* pCell = cast_to<CellClass*, false>(pTarget);
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


ASMJIT_PATCH(0x51C1F1, InfantryClass_CanEnterCell_WallWeapon, 0x5)
{
	enum { SkipGameCode = 0x51C1FE };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x73F495, UnitClass_CanEnterCell_WallWeapon, 0x6)
{
	enum { SkipGameCode = 0x73F4A1 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayTypeClass, ESI);

	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));

	return SkipGameCode;
}

ASMJIT_PATCH(0x70095A, TechnoClass_WhatAction_WallWeapon, 0x6) {
	GET_STACK(OverlayTypeClass*, pOverlayTypeClass, STACK_OFFSET(0x2C, -0x18));
	GET(TechnoClass*, pThis, ESI);
	R->EAX(pThis->GetWeapon(TechnoExtData::GetWeaponIndexAgainstWall(pThis, pOverlayTypeClass)));
	return 0;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6F8C10, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9572, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F971F, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9904, FakeTechnoClass::_EvaluateJustCell);
DEFINE_FUNCTION_JUMP(CALL, 0x6F9AB8, FakeTechnoClass::_EvaluateJustCell);
#pragma endregion

ASMJIT_PATCH(0x6FDDC0, TechnoClass_FireAt_Early, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AE.flags.HasOnFireDiscardables) {
		for (auto& attachEffect : pExt->PhobosAE) {
				if(!attachEffect || attachEffect->ShouldBeDiscarded)
					continue;

			if(GeneralUtils::Contains<DiscardCondition>(attachEffect->GetType()->DiscardOn ,DiscardCondition::Firing))
				attachEffect->ShouldBeDiscarded = true;
		}
	}

	// if (pThis->Passengers.FirstPassenger)
	// {
	// 	// TODO : implement this for UnitClass
	// 	pThis->DropOffParadropCargo();
	// }

	if (pWeapon) {
		auto pWeaponExt = pWeapon->_GetExtData();

		auto& timer = pExt->DelayedFireTimer;
		if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
			pExt->ResetDelayedFireTimer();

		if (pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
		{
			auto const rtti = pThis->WhatAmI();

			if (pWeaponExt->DelayedFire_PauseFiringSequence && (rtti == AbstractType::Infantry
				|| (rtti == AbstractType::Unit && !pThis->HasTurret() && !pThis->GetTechnoType()->Voxel)))
			{
				return 0;
			}

			if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
			{
				if (timer.InProgress())
					return 0x6FDE03;

				if (!timer.HasStarted())
				{
					pExt->DelayedFireWeaponIndex = weaponIndex;
					timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
					auto pAnimType = pWeaponExt->DelayedFire_Animation;

					if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
						pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation;

					auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

					if (pWeaponExt->DelayedFire_AnimOffset.isset())
						firingCoords = pWeaponExt->DelayedFire_AnimOffset;

					pExt->CreateDelayedFireAnim( pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

					if (pWeaponExt->DelayedFire_InitialBurstSymmetrical)
						pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
							pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, {firingCoords.X, -firingCoords.Y, firingCoords.Z});

					return 0x6FDE03;
				}
				else
				{
					pExt->ResetDelayedFireTimer();
				}
			}
		}

		if (const auto pTargetTechno = flag_cast_to<TechnoClass*>(pTarget)) {
				auto const pTargetExt = TechnoExtContainer::Instance.Find(pTargetTechno);
			if (pWeaponExt->NoRepeatFire > 0) {
				pTargetExt->LastBeLockedFrame = Unsorted::CurrentFrame;
			}

			if (pWeaponExt->AttachEffect_Enable) {
				auto const info = &pWeaponExt->AttachEffects;
				PhobosAttachEffectClass::Attach(pTargetTechno, pThis->Owner, pThis, pWeapon->Warhead, info);
				PhobosAttachEffectClass::Detach(pTargetTechno, info);
				PhobosAttachEffectClass::DetachByGroups(pTargetTechno, info);
			}
		}
	}

	return 0x0;
}

// ASMJIT_PATCH(0x5206B7, InfantryClass_FiringAI_Entry, 0x6)
// {
// 	GET(InfantryClass*, pThis, EBP);
//
// 	if (!pThis->Target || !pThis->IsFiring)
// 		TechnoExtContainer::Instance.Find(pThis)->DelayedFireSequencePaused = false;
//
// 	return 0;
// }


ASMJIT_PATCH(0x6FCDD2, TechnoClass_AssignTarget_Changed, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pNewTarget, EDI);

	if (!pNewTarget)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);
		pExt->ResetDelayedFireTimer();
	}

	return 0;
}

ASMJIT_PATCH(0x6FDD7D, TechnoClass_FireAt_UpdateWeaponType, 0x5) {

	enum { CanNotFire = 0x6FDE03 };

	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(TechnoClass*, pThis, ESI);

	const auto pWH = pWeapon->Warhead;
	auto pExt = TechnoExtContainer::Instance.Find(pThis);
	auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	auto pWHExt =  WarheadTypeExtContainer::Instance.Find(pWH);

	if (pWeapon->LimboLaunch) {
		if (!pWH->Parasite && pWHExt->UnlimboDetonate) {
			if (const auto pFoot = flag_cast_to<FootClass*, false>(pThis)) {
				if (pFoot->Locomotor->Is_Really_Moving_Now())
					return CanNotFire;
			}
		}
	}

	{
		if (pThis->CurrentBurstIndex && pWeapon != pExt->LastWeaponType && pTypeExt->RecountBurst.Get(RulesExtData::Instance()->RecountBurst)) {
			if (pExt->LastWeaponType && pExt->LastWeaponType->Burst) {

				const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pExt->LastWeaponType->Burst;
				const auto rof = static_cast<int>(ratio * pExt->LastWeaponType->ROF * pExt->AE.ROFMultiplier) - (Unsorted::CurrentFrame - pThis->LastFireBulletFrame);

				if (rof > 0){
					pThis->ROF = rof;
					pThis->RearmTimer.Start(rof);
					pThis->CurrentBurstIndex = 0;
					pExt->LastWeaponType = pWeapon;

					return CanNotFire;
				}
			}

			pThis->CurrentBurstIndex = 0;

		}

		pExt->LastWeaponType = pWeapon;
	}

	return 0;
}