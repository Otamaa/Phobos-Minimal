#include "Body.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/Unit/Body.h>

#include <Locomotor/Cast.h>
#include <TerrainTypeClass.h>
#include <OverlayTypeClass.h>

#include <Misc/DynamicPatcher/Techno/Passengers/PassengersFunctional.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <TerrainClass.h>

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

	if (!WeaponTypeExtContainer::Instance.Find(pWeapon)->TurretRecoil_Suppress)
	{	GET(TechnoClass* const, pThis, ESI);
		TechnoExtContainer::Instance.Find(pThis)->RecordRecoilData();
	}

	return SkipGameCode;
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
	pThis->RockingSidewaysPerFrame = (float)(force_result * Math::sin(theta) * Math::pow(pThis->Type->VoxelScaleX / pThis->Type->VoxelScaleY, 2.0));

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

ASMJIT_PATCH(0x772AA2, WeaponTypeClass_AllowedThreats_AAOnly, 0x5)
{
	GET(BulletTypeClass* const, pType, ECX);

	if (BulletTypeExtContainer::Instance.Find(pType)->AAOnly) {
		R->EAX(4);
		return 0x772AB3;
	}

	return 0;
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

// Reimplements the game function with few changes / optimizations
ASMJIT_PATCH(0x7012C0, TechnoClass_WeaponRange, 0x8) //4
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, 0x4);

	int result = 0;
	auto const pThisType = GET_TECHNOTYPE(pThis);

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
				int openTWeaponIndex = GET_TECHNOTYPE(pPassenger)->OpenTransportWeapon;
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

			attachEffect->DiscardOnFire();
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
				|| (rtti == AbstractType::Unit && !pThis->HasTurret() && !GET_TECHNOTYPE(pThis)->Voxel)))
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
						pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation.Get();

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
	auto pTypeExt = GET_TECHNOTYPEEXT(pThis);
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