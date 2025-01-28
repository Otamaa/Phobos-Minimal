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

#include <TerrainClass.h>

bool DisguiseAllowed(const TechnoTypeExtData* pThis, ObjectTypeClass* pThat)
{
	if (!pThis->DisguiseDisAllowed.empty() && pThis->DisguiseDisAllowed.Contains(pThat))
		return false;

	return true;
}

// An example for quick tilting test
DEFINE_HOOK(0x7413DD, UnitClass_Fire_RecoilForce, 0x6)
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

DEFINE_HOOK(0x6FF905, TechnoClass_FireAt_FireOnce, 0x6) {
	GET(TechnoClass*, pThis, ESI);

	if (auto const pInf = cast_to<InfantryClass*, false>(pThis))
	{
		GET(WeaponTypeClass*, pWeapon, EBX);

		if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->FireOnce_ResetSequence)
			InfantryExtContainer::Instance.Find(pInf)->SkipTargetChangeResetSequence = true;
	}

	return 0;
}

DEFINE_HOOK(0x51B20E, InfantryClass_AssignTarget_FireOnce, 0x6)
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
// DEFINE_HOOK(0x6FE562, TechnoClass_FireAt_BurstRandomTarget, 0x6)
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
DEFINE_HOOK(0x6FCBE6, TechnoClass_CanFire_BridgeAAFix, 0x6)
{
	enum { SkipChecks = 0x6FCCBD };

	GET(TechnoClass*, pTarget, EBP);

	if (pTarget->IsInAir())
		return SkipChecks;

	return 0;
}

DEFINE_HOOK(0x6FC3FE, TechnoClass_CanFire_Immunities, 0x6)
{
	enum { FireIllegal = 0x6FC86A, ContinueCheck = 0x6FC425 };

	GET(TechnoClass*, pThis, ESI);
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

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire_PreFiringChecks, 0x6) //8
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
	if (pThis->GetTechnoType()->LandTargeting != LandTargetingType::Land_not_okay && pWeapon->Projectile->AA && pTarget && !pTarget->IsInAir()) {
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

DEFINE_HOOK(0x6FC5C7, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { Illegal = 0x6FC86A, OutOfRange = 0x6FC0DF, Continue = 0x6FC5D5 };

	//GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTransport, EAX);

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pTransport->GetTechnoType());

	if (pTransport->Transporter || (pTransport->Deactivated && !pTypeExt->OpenTopped_AllowFiringIfDeactivated))
		return Illegal;

	if (pTypeExt->OpenTopped_CheckTransportDisableWeapons && TechnoExtContainer::Instance.Find(pTransport)->AE.DisableWeapons)
		return OutOfRange;

	return Continue;
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

DEFINE_HOOK(0x6FC815, TechnoClass_CanFire_CellTargeting, 0x7)
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

DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_Early, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->AE.HasOnFireDiscardables) {
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

		auto& timer = pExt->DelayedFireTimer;

		if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
			pExt->ResetDelayedFireTimer();

		if (pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
		{
			if (pThis->WhatAmI() == AbstractType::Infantry && pWeaponExt->DelayedFire_PauseFiringSequence)
				return 0;

			if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
			{
				if (timer.InProgress())
					return 0x6FDE03 ;

				if (!timer.HasStarted())
				{
					pExt->DelayedFireWeaponIndex = weaponIndex;
					timer.Start(MaxImpl(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
					auto pAnimType = pWeaponExt->DelayedFire_Animation;

					if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
						pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation;

					pExt->CreateDelayedFireAnim(pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
						pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOffset.isset(), pWeaponExt->DelayedFire_AnimOffset.Get());

					return 0x6FDE03 ;
				}
				else
				{
					pExt->ResetDelayedFireTimer();
				}
			}
		}
	}
	return 0x0;
}

DEFINE_HOOK(0x5206B7, InfantryClass_FiringAI_Entry, 0x6)
{
	GET(InfantryClass*, pThis, EBP);

	if (!pThis->Target || !pThis->IsFiring)
		TechnoExtContainer::Instance.Find(pThis)->FiringSequencePaused = false;

	return 0;
}

DEFINE_HOOK(0x6FABC4, TechnoClass_AI_AnimationPaused, 0x6)
{
	enum { SkipGameCode = 0x6FAC31 };

	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

	if (pExt->FiringSequencePaused)
		return SkipGameCode;

	return 0;
}

DEFINE_HOOK(0x6FCDD2, TechnoClass_AssignTarget_Changed, 0x6)
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