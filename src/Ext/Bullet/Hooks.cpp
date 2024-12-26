#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include <Ext/Bullet/Trajectories/StraightTrajectory.h>

DEFINE_HOOK(0x466705, BulletClass_AI, 0x6) //8
{
	enum { retContunue = 0x0 , retDead = 0x466781 };
	GET(FakeBulletClass* const, pThis, EBP);

	const auto pBulletExt = pThis->_GetExtData();
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

	auto const pTypeExt = pThis->_GetTypeExtData();

	if (pTypeExt->PreExplodeRange.isset())
	{
		const auto ThisCoord = pThis->GetCoords();
		const auto TargetCoords = pThis->GetBulletTargetCoords();

		if (Math::abs(ThisCoord.DistanceFrom(TargetCoords))
			<= pTypeExt->PreExplodeRange.Get(0) * 256)
			if (BulletExtData::HandleBulletRemove(pThis, true, true))
				return retDead;
	}

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted) {
		if (BulletExtData::HandleBulletRemove(pThis, pBulletExt->Intercepted_Detonate, true))
			return retDead;
	}


	if(!pBulletExt->Trajectory || !PhobosTrajectory::BlockDrawTrail(pBulletExt->Trajectory)){

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

		TrailsManager::AI(pThis->_AsBullet());
	}
	//if (!pThis->Type->Inviso && pBulletExt->InitialBulletDir.has_value())
	//	pBulletExt->InitialBulletDir = DirStruct((-1) * Math::atan2(pThis->Velocity.Y, pThis->Velocity.X));

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl_Override, 0x6)
{
	GET(FakeBulletClass*, pThis, ESI);

	const auto pControlledAnimType = pThis->_GetWarheadTypeExtData()->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	const auto pTechno = flag_cast_to<TechnoClass*>(pThis->Target);
	const auto Controller = pThis->Owner;

	R->AL(CaptureExt::CaptureUnit(Controller->CaptureManager,
		pTechno, TechnoTypeExtContainer::Instance.Find(Controller->GetTechnoType())->MultiMindControl_ReleaseVictim, false  , pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	//GET(BulletClass* const, pThis, EBP);
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExtData::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x4671BF;
}

// we handle ScreenShake thru warhead
DEFINE_JUMP(LJMP, 0x4690D4, 0x469130)
//DEFINE_SKIP_HOOK(0x4690D4 , BulletClass_Logics_Shake_Handled ,0x6 , 469130);

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(FakeBulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (!pHouse)
		R->ECX(pThis->_GetExtData()->Owner);

	return 0;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
DEFINE_HOOK(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B, Continue = 0x0 };
	GET(FakeBulletClass*, pThis, EBP);
	return (pThis->Type->Inviso && pThis->_GetExtData()->IsInterceptor)
		? DetonateBullet : Continue;
}

static FORCEINLINE void TryDetonateFull(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner) == FullMapDetonateResult::TargetValid)
	{
		pThis->Target = pTechno;
		pThis->Location = pTechno->GetCoords();
		pThis->Detonate(pTechno->GetCoords());
	}
}

static FORCEINLINE void TryDetonateDamageArea(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner) == FullMapDetonateResult::TargetValid)
	{
		int damage = (pThis->Health * pThis->DamageMultiplier) >> 8;
		pWHExt->DamageAreaWithTarget(pTechno->GetCoords(), damage, pThis->Owner, pThis->WH, true, pOwner, pTechno);
	}
}

#include <Misc/PhobosGlobal.h>
#include <InfantryClass.h>

DEFINE_HOOK(0x4690C1, BulletClass_Logics_Detonate, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(FakeBulletClass* const, pThis, ESI);

	if (!BulletExtData::IsReallyAlive(pThis)) {
		return ReturnFromFunction;
	}

	if (pThis->WH)
	{
		const auto pWHExt = pThis->_GetWarheadTypeExtData();

		if (pWHExt->DetonateOnAllMapObjects &&
			!pWHExt->WasDetonatedOnAllMapObjects &&
			pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
			pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			auto const originalLocation = pThis->Location;
			const auto pHouse = BulletExtData::GetHouse(pThis);
			BulletExtContainer::Instance.Find(pThis)->OriginalTarget = pThis->Target;

			constexpr auto copy_dvc = []<typename T>(std::vector<T>& dest, const DynamicVectorClass<T>&dvc) {
				dest.resize(dvc.Count);
				std::copy(dvc.begin(), dvc.end(), dest.begin());
				return &dest;
			};

			void (*tryDetonate)(BulletClass*, TechnoClass*, WarheadTypeExtData*, HouseClass*)
					= pWHExt->DetonateOnAllMapObjects_Full ? TryDetonateFull : TryDetonateDamageArea;

			auto& CurCopyArray = PhobosGlobal::Instance()->CurCopyArray[pThis->WH];

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				auto const aircraft = copy_dvc(CurCopyArray.Aircraft ,*AircraftClass::Array);

				for (auto pAircraft : *aircraft)
					tryDetonate(pThis, pAircraft, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
			{
				auto const buildings = copy_dvc(CurCopyArray.Building ,*BuildingClass::Array);

				for (auto pBuilding : *buildings)
					tryDetonate(pThis, pBuilding, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				auto const infantry = copy_dvc(CurCopyArray.Infantry ,*InfantryClass::Array);

				for (auto pInf : *infantry)
					tryDetonate(pThis, pInf, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
			{
				auto const units = copy_dvc(CurCopyArray.Unit ,*UnitClass::Array);

				for (auto const pUnit : *units)
					tryDetonate(pThis, pUnit, pWHExt, pHouse);
			}

			pThis->Target = pThis->_GetExtData()->OriginalTarget;
			pThis->Location = originalLocation;
			pWHExt->WasDetonatedOnAllMapObjects = false;
			return ReturnFromFunction;
		}
	}

	return 0;
}

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(FakeBulletClass*, pThis, ESI);

	auto const pCell = pThis->GetCell();
	const bool isLand = !pCell ? true :
	pCell->LandType != LandType::Water || pCell->ContainsBridge();

	if (!isLand && pThis->_GetWarheadTypeExtData()->Debris_Conventional.Get())
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

DEFINE_HOOK(0x469B44, BulletClass_Logics_LandTypeCheck, 0x6)
{
	enum { SkipChecks = 0x469BA2 };

	GET(FakeBulletClass*, pThis, ESI);

	return pThis->_GetWarheadTypeExtData()->Conventional_IgnoreUnits ? SkipChecks : 0;
}
