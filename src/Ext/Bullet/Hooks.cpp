#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Utilities/Macro.h>

#include <TechnoClass.h>
#include <TacticalClass.h>


#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#endif

#include <Ext/Bullet/Trajectories/StraightTrajectory.h>

DEFINE_HOOK(0x466705, BulletClass_AI, 0x6) //8
{
	GET(BulletClass*, pThis, EBP);
	const auto pBulletExt = BulletExt::ExtMap.Find(pThis);

	if (!pBulletExt)
		return 0;

	bool bChangeOwner = false;
	auto const pBulletCurOwner = pThis->GetOwningHouse();
	if (pThis->Owner && pBulletCurOwner && pBulletCurOwner != pBulletExt->Owner)
	{
		bChangeOwner = true;
		pBulletExt->Owner = pBulletCurOwner;
	}

	if (pThis->WeaponType && pThis->WH)
	{
		if (!pBulletExt->BrightCheckDone)
		{
			pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
			pBulletExt->BrightCheckDone = true;
		}
	}

	bool DetonateNow = false;
	bool HandleRemove = false;

	if (pBulletExt->TypeExt->PreExplodeRange.isset())
	{
		const auto ThisCoord = pThis->GetCoords();
		const auto TargetCoords = pThis->GetTargetCoords();
		HandleRemove = DetonateNow = abs(ThisCoord.DistanceFrom(TargetCoords))
			<= pBulletExt->TypeExt->PreExplodeRange.Get(0) * 256;
	}

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
	{
		if (!DetonateNow && pBulletExt->Intercepted_Detonate)
		{
			DetonateNow = true;
		}

		HandleRemove = true;
	}

	if (DetonateNow) { pThis->Detonate(pThis->GetCoords()); }

	if (HandleRemove)
	{

		if (!pThis->InLimbo)
			pThis->Limbo();

		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo)
		{
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	if ((!pBulletExt->LaserTrails.empty()))
	{
		CoordStruct location = pThis->GetCoords();
		const VelocityClass& velocity = pThis->Velocity;

		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		for (auto const& trail : pBulletExt->LaserTrails)
		{
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			if (trail->Type->IsHouseColor.Get() && bChangeOwner && pBulletExt->Owner)
				trail->CurrentColor = pBulletExt->Owner->LaserColor;

			trail->Update(drawnCoords);
		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::AI(pThis);
#endif

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto const pTechno = generic_cast<TechnoClass*>(pThis->Target);

	R->AL(CaptureExt::CaptureUnit(pThis->Owner->CaptureManager, pTechno, pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	//GET(BulletClass* const, pThis, EBP);
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x4671BF;
}

DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
{
	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };

	GET(BulletClass*, pBullet, EDI);

	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (auto const pObject = pBullet->GetCell()->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
			return Shrapnel;
	}
	else if (pData->Shrapnel_AffectsGround)
		return Shrapnel;

	return Skip;
}

// we handle ScreenShake thru warhead
DEFINE_JUMP(LJMP, 0x4690D4, 0x469130)

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
		if (!pHouse)
			R->ECX(pExt->Owner);

	return 0;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
DEFINE_HOOK(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B };

	GET(BulletClass*, pThis, EBP);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
		if (pThis->Type->Inviso && pExt->IsInterceptor)
			return DetonateBullet;

	return 0;
}

/*
DEFINE_HOOK(0x468D3F, BulletClass_IsForcedToExplode_AirTarget, 0x8)
{
	enum { DontExplode = 0x468D73 };

	GET(const BulletClass*, pThis, ESI);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory && pExt->Trajectory->Flag == TrajectoryFlag::Straight)
		{
			// Straight trajectory has its own proximity checks.
			return DontExplode;
		}
	}

	return 0;
}*/

DEFINE_HOOK(0x469211, BulletClass_Logics_MindControlAlternative1, 0x6)
{
	GET(BulletClass*, pBullet, ESI);

	if (!pBullet->Target)
		return 0;

	const auto pBulletWH = pBullet->WH;
	const auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (const auto pTargetType = pTarget->GetTechnoType())
		{
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				const double currentHealthPerc = pTarget->GetHealthPercentage();
				const bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				if (pWarheadExt->MindControl_Threshold < 0.0 || pWarheadExt->MindControl_Threshold > 1.0)
					pWarheadExt->MindControl_Threshold = flipComparations ? 0.0 : 1.0;

				const bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				const bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					const int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					const int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

					if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage && realDamage > 1)
						pTarget->Health = realDamage;

					return 0x469343;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x469BD6, BulletClass_Logics_MindControlAlternative2, 0x6)
{
	GET(BulletClass*, pBullet, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);

	if (!pBullet->Target)
		return 0;

	const auto pBulletWH = pBullet->WH;
	const auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (const auto pTargetType = pTarget->GetTechnoType())
		{
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				const double currentHealthPerc = pTarget->GetHealthPercentage();
				const bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				const bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				const bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					const auto pAttacker = pBullet->Owner;
					const auto pAttackingHouse = pBullet->Owner->Owner;
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

					if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage)
					{
						pTarget->Health += abs(realDamage);
						realDamage = 1;
						pTarget->ReceiveDamage(&realDamage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
						pTarget->Health = 1;
					}
					else
					{
						pTarget->ReceiveDamage(&damage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
					}

					pAnimType = nullptr;

					// If the alternative Warhead have AnimList tag declared then use it
					if (pWarheadExt->MindControl_AlternateWarhead->AnimList.Count > 0)
					{
						pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, Map[pTarget->Location]->LandType, pTarget->Location);
					}

					R->EBX(pAnimType);
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			const auto pExt = BulletExt::ExtMap.Find(pThis);
			const auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->Owner;

			std::for_each(TechnoClass::Array->begin(), TechnoClass::Array->end(), [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					pThis->Target = pTechno;
					pThis->Detonate(pTechno->GetCoords());
				}
			});

			pWHExt->WasDetonatedOnAllMapObjects = false;

			return ReturnFromFunction;
		}
	}

	return 0;
}

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFS(0x3C, 0x30));

	if (pThis->Type->Cluster > 0)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			const int min = pTypeExt->Cluster_Scatter_Min.Get(Leptons(256));
			const int max = pTypeExt->Cluster_Scatter_Max.Get(Leptons(512));
			auto coords = origCoords;

			for (int i = 0; i < pThis->Type->Cluster; i++)
			{
				pThis->Detonate(coords);

				if (!pThis->IsAlive)
					break;

				int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
				coords = MapClass::GetRandomCoordsNear(origCoords, distance, false);
			}

			return SkipGameCode;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4687F8, BulletClass_Unlimbo_FlakScatter, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(float, mult, STACK_OFFS(0x5C, 0x44));

	if (pThis->WeaponType) {
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type)) {
			const int defaultmax = RulesClass::Instance->BallisticScatter;
			const int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
			const int max = pTypeExt->BallisticScatter_Max.Get(Leptons(defaultmax));
			R->EAX(static_cast<int>((mult * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)) / pThis->WeaponType->Range));
		}
	}

	return 0;
}

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	const bool isLand = pThis->GetCell()->LandType != LandType::Water || pThis->GetCell()->ContainsBridge();

	if (pWHExt && !isLand && pWHExt->Debris_Conventional.Get())
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

#ifndef ENABLE_NEWHOOKS
DEFINE_HOOK(0x468E9F, BulletClass_Logics_SnapOnTarget, 0x6) //C
{
	enum { NoSnap = 0x468FF4, ForceSnap = 0x468EC7 };

	GET(BulletClass*, pThis, ESI);

	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return ForceSnap;
	}

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory &&
			 pExt->Trajectory->Flag == TrajectoryFlag::Straight &&
			!pExt->SnappedToTarget)
		{
			return NoSnap;
		}
	}

	return 0;
}
#else
/// TODO : snap checks test
DEFINE_HOOK(0x467CCA, BulletClass_AI_TargetSnapChecks, 0x6) //was C
{
	enum { SkipAirburstCheck = 0x467CDE , SkipSnapFunc = 0x467E53 };

	GET(BulletClass*, pThis, EBP);

	auto nRet = [=]() {
		R->EAX(pThis->Type);
		return SkipAirburstCheck;
	};

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		return nRet();
	}
	else if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return !pExt->SnappedToTarget ? nRet() : SkipSnapFunc;
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E61, BulletClass_Explode_TargetSnapChecks1, 0x6) //was C
{
	enum { SkipAirburstChecks = 0x468E7B , SkipCoordFunc = 0x468E9F };

	GET(BulletClass*, pThis, ESI);
	auto nRet = [=]() {
		R->EAX(pThis->Type);
		return SkipAirburstChecks;
	};

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		return nRet();
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}
	else if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return !pExt->SnappedToTarget ? nRet() : SkipCoordFunc;
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E9F, BulletClass_Explode_TargetSnapChecks2, 0x6) //was C
{
	enum { SkipInitialChecks = 0x468EC7, SkipSetCoordinate = 0x468F23 };

	GET(BulletClass*, pThis, ESI);

	// Do not require EMEffect=no & Airburst=no to check target coordinate snapping for Inviso projectiles.
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipInitialChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}

	// Do not force Trajectory=Straight projectiles to detonate at target coordinates under certain circumstances.
	// Fixes issues with walls etc.
	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return SkipSetCoordinate;
				//return !pExt->SnappedToTarget ? SkipInitialChecks : SkipSetCoordinate;
		}
	}

	return 0;
}
#endif