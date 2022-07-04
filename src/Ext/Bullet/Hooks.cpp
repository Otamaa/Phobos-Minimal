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
			if (auto pTypeExt = BulletTypeExt::GetExtData(pThis->Type))
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

	auto const pBulletCurOwner = pThis->GetOwningHouse();
	if (pThis->Owner && pBulletCurOwner && pBulletCurOwner != pBulletExt->Owner)
		pBulletExt->Owner = pBulletCurOwner;

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

	auto const pData = BulletTypeExt::GetExtData(pBullet->Type);

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
DEFINE_JUMP(LJMP,0x4690D4, 0x469130)

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

typedef void (*v_table_ptr)();

typedef struct _cpp_object
{
	v_table_ptr* vtable;
} cpp_object;

typedef struct
{
	unsigned int signature;
	int base_class_offset;
	unsigned int flags;
	unsigned int type_descriptor;
	unsigned int type_hierarchy;
	unsigned int object_locator;
} rtti_object_locator;

/* Get type info from an object (internal) */
static const rtti_object_locator* RTTI_GetObjectLocator(void* inptr)
{
	cpp_object* cppobj = (cpp_object*)inptr;
	const rtti_object_locator* obj_locator = 0;

	if (!IsBadReadPtr(cppobj, sizeof(void*)) &&
		!IsBadReadPtr(cppobj->vtable - 1, sizeof(void*)) &&
		!IsBadReadPtr((void*)cppobj->vtable[-1], sizeof(rtti_object_locator)))
	{
		obj_locator = (rtti_object_locator*)cppobj->vtable[-1];
	}

	return obj_locator;
}

DEFINE_HOOK(0x468E9F, BulletClass_Logics_SnapOnTarget, 0x6)
{
	enum { NoSnap = 0x468FF4, ForceSnap = 0x468EC7 };

	GET(BulletClass*, pThis, ESI);

	if (pThis->Type->Inviso) {
		R->EAX(pThis->Type);
		return ForceSnap;
	}

	if (auto const pExt = BulletExt::GetExtData(pThis)) {
		if (pExt->Trajectory &&
			 pExt->Trajectory->Flag == TrajectoryFlag::Straight &&
			!pExt->SnappedToTarget) {
				return NoSnap;
		}
	}

	return 0;
}

/*
DEFINE_HOOK(0x469211, BulletClass_Logics_MindControlAlternative1, 0x6)
{
	GET(BulletClass*, pBullet, ESI);

	if (!pBullet->Target)
		return 0;

	auto pBulletWH = pBullet->WH;
	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (auto pTargetType = pTarget->GetTechnoType())
		{
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				double currentHealthPerc = pTarget->GetHealthPercentage();
				bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				if (pWarheadExt->MindControl_Threshold < 0.0 || pWarheadExt->MindControl_Threshold > 1.0)
					pWarheadExt->MindControl_Threshold = flipComparations ? 0.0 : 1.0;

				bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

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

	auto pBulletWH = pBullet->WH;
	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (auto pTargetType = pTarget->GetTechnoType())
		{
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				double currentHealthPerc = pTarget->GetHealthPercentage();
				bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					auto pAttacker = pBullet->Owner;
					auto pAttackingHouse = pBullet->Owner->Owner;
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
					if (pWarheadExt->MindControl_AlternateWarhead->AnimList.Count > 0) {
						pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, Map[pTarget->Location]->LandType, pTarget->Location);
					}

					R->EBX(pAnimType);
				}
			}
		}
	}

	return 0;
}*/