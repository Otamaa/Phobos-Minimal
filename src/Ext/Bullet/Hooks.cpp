#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>

#include <Utilities/Macro.h>

#include <TechnoClass.h>
#include <TacticalClass.h>


#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

#include <Ext/Bullet/Trajectories/StraightTrajectory.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_Phobos, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (auto pExt = BulletExt::GetExtData(pThis))
	{
		pExt->Owner = pThis->Owner ? pThis->Owner->GetOwningHouse() : nullptr;

		if (pThis->Type)
		{
			//pExt->CurrentStrength = pThis->Type->Strength;

			if (auto pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
			{
				pExt->TypeExt = pTypeExt;
				pExt->CurrentStrength = pTypeExt->Health.Get();
				if (pTypeExt->LaserTrail_Types.size() > 0)
					pExt->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

				pExt->InitializeLaserTrails(pTypeExt);
			}
		}
	}


#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::Construct(pThis);
#endif
	return 0;
}

DEFINE_HOOK(0x466705, BulletClass_AI, 0x8)
{
	GET(BulletClass*, pThis, EBP);
	auto pBulletExt = BulletExt::GetExtData(pThis);

	if (!pBulletExt)
		return 0;

	if (pThis->Owner && pThis->Owner->GetOwningHouse() && pThis->Owner->GetOwningHouse() != pBulletExt->Owner)
		pBulletExt->Owner = pThis->Owner->GetOwningHouse();

	if (pThis->WeaponType && pThis->WH)
	{
		if (!pBulletExt->BrightCheckDone) {
			pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
			pBulletExt->BrightCheckDone = true;
		}
	}

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
	{
		if (pBulletExt->Intercepted_Detonate)
			pThis->Detonate(pThis->GetCoords());

		if(!pThis->InLimbo)
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
		const BulletVelocity& velocity = pThis->Velocity;

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

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto pTechno = generic_cast<TechnoClass*>(pThis->Target);

	R->AL(CaptureExt::CaptureUnit(pThis->Owner->CaptureManager, pTechno, pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
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

DEFINE_HOOK(0x4690D4, BulletClass_Logics_ScreenShake, 0x6)
{
	// we handle it thru warhead
	return 0x469130;
}

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (auto const pExt = BulletExt::GetExtData(pThis))
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

	if (auto const pExt = BulletExt::GetExtData(pThis))
		if (pThis->Type->Inviso && pExt->IsInterceptor)
			return DetonateBullet;

	return 0;
}

DEFINE_HOOK(0x468E9F, BulletClass_Logics_SnapOnTarget, 0x6)
{
	enum { NoSnap = 0x468FF4, ForceSnap = 0x468EC7 };

	GET(BulletClass*, pThis, ESI);

	if (pThis->Type->Inviso)
		return ForceSnap;

	if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pTypeExt->TrajectoryType &&
			pTypeExt->TrajectoryType->Flag == TrajectoryFlag::Straight
		)
		{
			auto type = static_cast<StraightTrajectoryType*>(pTypeExt->TrajectoryType);

			if (type && !type->SnapOnTarget)
				return NoSnap;
		}
	}

	return 0;
}