#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ParticleSystemClass.h>
#include <FootClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#ifdef PARTONE
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Fix techno target coordinates (used for fire angle calculations, target lines etc) to take building target coordinate offsets into accord.
// This, for an example, fixes a vanilla bug where Destroyer has trouble targeting Naval Yards with its cannon weapon from certain angles.
DEFINE_HOOK(0x70BCDC, TechnoClass_GetTargetCoords_BuildingFix, 0x6)
{
	GET(const AbstractClass* const, pTarget, ECX);
	LEA_STACK(CoordStruct*, nCoord, 0x40 - 0x18);

	if (const auto pBuilding = specific_cast<const BuildingClass*>(pTarget)) {
		//auto const& nTargetCoord = pBuilding->Type->TargetCoordOffset;
		//Debug::Log("__FUNCTION__ Building  Target [%s] with TargetCoord %d , %d , %d \n", pBuilding->get_ID(), nTargetCoord.X , nTargetCoord.Y , nTargetCoord.Z);
		pBuilding->GetTargetCoords(nCoord);

		R->EAX(nCoord);
		return 0x70BCE6u;

	}// else {
	//	pTarget->GetCenterCoords(nCoord);
	//}

	return 0x0;
}

// Fix railgun target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x70C6AF, TechnoClass_Railgun_TargetCoords, 0x6)
{
	GET(AbstractClass*, pTarget, EBX);
	GET(CoordStruct*, pBuffer, ECX);

	switch (pTarget->WhatAmI())
	{
	case BuildingClass::AbsID:
	{
		static_cast<BuildingClass*>(pTarget)->GetTargetCoords(pBuffer);
		break;
	}
	case CellClass::AbsID:
	{
		auto const pCell = static_cast<CellClass*>(pTarget);
		pCell->GetCoords(pBuffer);
		if (pCell->ContainsBridge()) {
			pBuffer->Z += Unsorted::BridgeHeight;
		}

		break;
	}
	default:
		pTarget->GetCoords(pBuffer);
		break;
	}

	R->EAX(pBuffer);
	return 0x70C6B5;
}

// Do not adjust map coordinates for railgun particles that are below cell coordinates.
DEFINE_HOOK(0x62B897, ParticleClass_CTOR_RailgunCoordAdjust, 0x5)
{
	enum { SkipCoordAdjust = 0x62B8CB  ,Continue = 0x0};

	GET(ParticleClass*, pThis, ESI);
	const auto pParticleSys = pThis->ParticleSystem;
	const auto pParticleTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

	if(pParticleSys
	&& (pParticleSys->Type->BehavesLike == ParticleSystemTypeBehavesLike::Railgun
		|| pParticleSys->Type->BehavesLike == ParticleSystemTypeBehavesLike::Fire)
	){
		GET(CoordStruct*, pCoordBase, EDI);
		LEA_STACK(CoordStruct* , pCoord, 0x10);

		//restore overriden instruction
		if(!pParticleTypeExt->ReadjustZ) {
			pCoord->X = pCoordBase->X;
			pCoord->Y = pCoordBase->Y;
			pCoord->Z = pCoordBase->Z;
		}
		else
		{
			pCoord->X = pCoordBase->X;
			pCoord->Y = pCoordBase->Y;

			auto nZ = MapClass::Instance->GetZPos(pCoordBase);
			if (pCoordBase->Z <= nZ)
				pCoord->Z = nZ;
			else
				pCoord->Z = pCoordBase->Z;
		}

		return SkipCoordAdjust;
	}

	return Continue;
}

// Fix fire particle target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x62FA20, ParticleSystemClass_FireAI_TargetCoords, 0x6)
{
	enum { SkipGameCode = 0x62FA51, Continue = 0x62FBAF };

	GET(ParticleSystemClass*, pThis, ESI);
	GET(TechnoClass*, pOwner, EBX);

	if (ParticleSystemTypeExtContainer::Instance
			.Find(pThis->Type)->AdjustTargetCoordsOnRotation
		&& pOwner->PrimaryFacing.Is_Rotating())
	{
		auto coords = pThis->TargetCoords;
		R->EAX(&coords);
		return SkipGameCode;
	}

	return Continue;
}

// Fix fire particles being disallowed from going upwards.
DEFINE_HOOK(0x62D685, ParticleSystemClass_FireAt_Coords, 0x5)
{
	enum { SkipGameCode = 0x62D6B7 };

	// Game checks if MapClass::GetCellFloorHeight() for currentCoords is larger than for previousCoords and sets the flags on ParticleClass to
	// remove it if so. Below is an attempt to create a smarter check that allows upwards movement and does not needlessly collide with elevation
	// but removes particles when colliding with flat ground. It doesn't work perfectly and covering all edge-cases is difficult or impossible so
	// preference was to disable it. Keeping the code here commented out, however.

	/*
	GET(ParticleClass*, pThis, ESI);
	REF_STACK(CoordStruct, currentCoords, STACK_OFFSET(0x24, -0x18));
	REF_STACK(CoordStruct, previousCoords, STACK_OFFSET(0x24, -0xC));
	auto const sourceLocation = pThis->ParticleSystem ? pThis->ParticleSystem->Location : CoordStruct { INT_MAX, INT_MAX, INT_MAX };
	auto const pCell = MapClass::Instance->TryGetCellAt(currentCoords);
	int cellFloor = MapClass::Instance->GetCellFloorHeight(currentCoords);
	bool downwardTrajectory = currentCoords.Z < previousCoords.Z;
	bool isBelowSource = cellFloor < sourceLocation.Z - Unsorted::LevelHeight * 2;
	bool isRamp = pCell ? pCell->SlopeIndex : false;
	if (!isRamp && isBelowSource && downwardTrajectory && currentCoords.Z < cellFloor)
	{
		pThis->unknown_12D = 1;
		pThis->unknown_131 = 1;
	}
	*/

	return SkipGameCode;
}
#endif
namespace FireAtTemp
{
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
}

#ifdef PERFORMANCE_HEAVY
// https://github.com/Phobos-developers/Phobos/pull/825
// Todo :  Otamaa : massive FPS drops !
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Adjust target coordinates for laser drawing.
DEFINE_HOOK(0x6FD38D, TechnoClass_LaserZap_Obstacles, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	auto coords = *pTargetCoords;
	auto pObstacleCell = FireAtTemp::pObstacleCell;

	if (pObstacleCell)
		coords = pObstacleCell->GetCoordsWithBridge();

	R->EAX(&coords);
	return 0;
}

// Set obstacle cell.
DEFINE_HOOK(0x6FF15F, TechnoClass_FireAt_ObstacleCellSet, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET_BASE(AbstractClass*, pTarget, 0x8);
	LEA_STACK(CoordStruct*, pSourceCoords, STACK_OFFSET(0xB0, -0x6C));

	auto coords = pTarget->GetCenterCoords();

	if (const auto pBuilding = specific_cast<BuildingClass*>(pTarget))
		coords = pBuilding->GetTargetCoords();

	FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(*pSourceCoords, coords, pWeapon->Projectile, pThis->Owner);

	return 0;
}

// Apply obstacle logic to fire & spark particle system targets.
DEFINE_HOOK_AGAIN(0x6FF1D7, TechnoClass_FireAt_SparkFireTargetSet, 0x5)
DEFINE_HOOK(0x6FF189, TechnoClass_FireAt_SparkFireTargetSet, 0x5)
{
	if (FireAtTemp::pObstacleCell)
	{
		if (R->Origin() == 0x6FF189)
			R->ECX(FireAtTemp::pObstacleCell);
		else
			R->EDX(FireAtTemp::pObstacleCell);
	}

	return 0;
}

// Cut railgun logic off at obstacle coordinates.
DEFINE_HOOK(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { Continue = 0x70CA79, Stop = 0x70CAD8 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	if (MapClass::Instance->GetCellAt(coords) == FireAtTemp::pObstacleCell)
		return Stop;

	return Continue;
}

#endif

// Adjust target for bolt / beam / wave drawing.
// same hook with TechnoClass_FireAt_FeedbackWeapon
DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_Additional, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));
	GET(AbstractClass*, pTarget, EDI);

#ifdef PERFORMANCE_HEAVY
	//TargetSet
	FireAtTemp::originalTargetCoords = *pTargetCoords;
	FireAtTemp::pOriginalTarget = pTarget;

	if (FireAtTemp::pObstacleCell)
	{
		const auto coords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();

		pTargetCoords->X = coords.X;
		pTargetCoords->Y = coords.Y;
		pTargetCoords->Z = coords.Z;
		//Sonic wave using base stack
		R->EDI(FireAtTemp::pObstacleCell);
	}
#endif

	//FeedbackWeapon
	auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeaponExt->FeedbackWeapon.isset())
	{
		if (auto fbWeapon = pWeaponExt->FeedbackWeapon.Get())
		{
			if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
				return 0;

			WeaponTypeExtData::DetonateAt(fbWeapon, pThis, pThis, true, nullptr);

			//pThis techno was die after after getting affect of FeedbackWeapon
			//if the function not bail out , it will crash the game because the vtable is already invalid
			if(!pThis->IsAlive) {
				return 0x6FF92F;
			}
		}
	}

	return 0;
}

#ifndef ENABLE_THESE_THINGS
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x70C862, TechnoClass_Railgun_AmbientDamageIgnoreTarget1, 0x5)
{
	enum { IgnoreTarget = 0x70CA59, Continue = 0x0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_IgnoreTarget ?
		IgnoreTarget : Continue;
}

DEFINE_HOOK(0x70CA8B, TechnoClass_Railgun_AmbientDamageIgnoreTarget2, 0x6)
{
	enum { IgnoreTarget = 0x70CBB0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);
	REF_STACK(DynamicVectorClass<ObjectClass*>, objects, 0xC0 - 0xAC);

	if (WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_IgnoreTarget)
	{
		R->EAX(objects.Count);
		return IgnoreTarget;
	}

	return 0;
}

DEFINE_HOOK(0x70CBDA, TechnoClass_Railgun_AmbientDamageWarhead, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(TechnoClass*, pSource, EDX);

	R->Stack<HouseClass*>(0xC, pSource->Owner);
	R->EDX(WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_Warhead.Get(pWeapon->Warhead));
	return 0x70CBE0;
}
#endif

#ifndef aaa
DEFINE_OVERRIDE_HOOK(0x6FF656, TechnoClass_FireAt_Additionals, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));
	GET_BASE(int, weaponIndex, 0xC);
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));

	//remove ammo rounds depending on weapon
	TechnoExt_ExtData::DecreaseAmmo(pThis, pWeaponType);

#ifdef PERFORMANCE_HEAVY
	// Restore original target values and unset obstacle cell.
	*pTargetCoords = std::exchange(FireAtTemp::originalTargetCoords, CoordStruct::Empty);
	std::exchange(FireAtTemp::pOriginalTarget, nullptr);
	std::exchange(FireAtTemp::pObstacleCell, nullptr);
	R->EDI(pTarget);
#endif

	//TechnoClass_FireAt_ToggleLaserWeaponIndex
	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeaponType->IsLaser)
	{
		auto const pExt = TechnoExtContainer::Instance.Find(pThis);

		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	//TechnoClass_FireAt_BurstOffsetFix_2
	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pWeaponType->Burst;

	if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
	{
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor())
		{
			BulletExtContainer::Instance.Find(pBullet)->IsInterceptor = true;
			BulletExtContainer::Instance.Find(pTargetObject)->InterceptedStatus = InterceptedStatus::Targeted;

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			{
				WarheadTypeExtContainer::Instance.Find(pWeaponType->Warhead)->InterceptBullets(pThis, pWeaponType, pTargetObject->Location);
			}
		}
	}

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);

	if (pWeaponExt->ShakeLocal.Get() && pThis->IsOnMyView())
	{

		if (pWeaponExt->Xhi || pWeaponExt->Xlo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

		if (pWeaponExt->Yhi || pWeaponExt->Ylo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));
	}

	return 0x6FF660;
}
#endif