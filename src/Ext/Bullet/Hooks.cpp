#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>

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
	GET(BulletClass* const, pThis, EBP);

	const auto pBulletExt = BulletExt::ExtMap.Find(pThis);
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

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->PreExplodeRange.isset())
	{
		const auto ThisCoord = pThis->GetCoords();
		const auto TargetCoords = pThis->GetBulletTargetCoords();

		if (abs(ThisCoord.DistanceFrom(TargetCoords))
			<= pTypeExt->PreExplodeRange.Get(0) * 256)
			BulletExt::HandleBulletRemove(pThis, true, true);
	}

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted) {
		BulletExt::HandleBulletRemove(pThis, pBulletExt->Intercepted_Detonate, true);
	}

	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	if ((!pBulletExt->LaserTrails.empty()))
	{
		const CoordStruct& location = pThis->Location;
		const VelocityClass& velocity = pThis->Velocity;

		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		for (auto& trail : pBulletExt->LaserTrails)
		{
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail.LastLocation.isset())
				trail.LastLocation = location;

			if (trail.Type->IsHouseColor.Get() && bChangeOwner && pBulletExt->Owner)
				trail.CurrentColor = pBulletExt->Owner->LaserColor;

			trail.Update(drawnCoords);
		}
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailsManager::AI(pThis);
#endif

	//if (!pThis->Type->Inviso && pBulletExt->InitialBulletDir.has_value())
	//	pBulletExt->InitialBulletDir = DirStruct((-1) * std::atan2(pThis->Velocity.Y, pThis->Velocity.X));

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

// we handle ScreenShake thru warhead
DEFINE_JUMP(LJMP, 0x4690D4, 0x469130)

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (!pHouse)
		R->ECX(BulletExt::ExtMap.Find(pThis)->Owner);

	return 0;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
DEFINE_HOOK(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B, Continue = 0x0 };
	GET(BulletClass*, pThis, EBP);
	return (pThis->Type->Inviso && BulletExt::ExtMap.Find(pThis)->IsInterceptor)
		? DetonateBullet : Continue;
}

DEFINE_HOOK(0x469211, BulletClass_Logics_MindControlAlternative1, 0x6)
{
	enum { ContinueFlow = 0, IgnoreMindControl = 0x469AA4 };

	GET(BulletClass*, pBullet, ESI);

	return BulletExt::ApplyMCAlternative(pBullet) ? IgnoreMindControl : ContinueFlow;
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass* const, pThis, ESI);

	if (pThis->WH)
	{
		auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

		if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;

			for (auto const pTechno : *TechnoClass::Array)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, BulletExt::GetHouse(pThis)) == FullMapDetonateResult::TargetValid)
				{
					pThis->Target = pTechno;
					pThis->Detonate(pTechno->Location);

					if (!BulletExt::IsReallyAlive(pThis))
						break;
				}
			}

			pWHExt->WasDetonatedOnAllMapObjects = false;

			return ReturnFromFunction;
		}
	}

	return 0;
}

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const pCell = pThis->GetCell();
	const bool isLand = !pCell ? true : 
	pCell->LandType != LandType::Water || pCell->ContainsBridge();
	
	if (!isLand && pWHExt->Debris_Conventional.Get())
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}
