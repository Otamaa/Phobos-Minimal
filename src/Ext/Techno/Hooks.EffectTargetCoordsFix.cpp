#include "Body.h"

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
#include <Ext/Wave/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Misc/AresTrajectoryHelper.h>

#ifndef PARTONE
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Fix techno target coordinates (used for fire angle calculations, target lines etc) to take building target coordinate offsets into accord.
// This, for an example, fixes a vanilla bug where Destroyer has trouble targeting Naval Yards with its cannon weapon from certain angles.
ASMJIT_PATCH(0x70BCDC, TechnoClass_GetTargetCoords_BuildingFix, 0x6)
{
	GET(const AbstractClass* const, pTarget, ECX);
	LEA_STACK(CoordStruct*, nCoord, 0x40 - 0x18);

	if (const auto pBuilding = cast_to<const BuildingClass*, false>(pTarget)) {
		//auto const& nTargetCoord = pBuilding->Type->TargetCoordOffset;
		//Debug::LogInfo("__FUNCTION__ Building  Target [%s] with TargetCoord %d , %d , %d ", pBuilding->get_ID(), nTargetCoord.X , nTargetCoord.Y , nTargetCoord.Z);
		pBuilding->GetTargetCoords(nCoord);

		R->EAX(nCoord);
		return 0x70BCE6u;

	}// else {
	//	pTarget->GetCenterCoords(nCoord);
	//}

	return 0x0;
}

// Fix railgun target coordinates potentially differing from actual target coords.
ASMJIT_PATCH(0x70C6AF, TechnoClass_Railgun_TargetCoords, 0x6)
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
ASMJIT_PATCH(0x62B897, ParticleClass_CTOR_RailgunCoordAdjust, 0x5)
{
	enum { SkipCoordAdjust = 0x62B8CB  ,Continue = 0x0};

	GET(ParticleClass*, pThis, ESI);
	const auto pParticleSys = pThis->ParticleSystem;
	const auto pParticleTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

	if(pParticleSys && pParticleSys->Type
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
ASMJIT_PATCH(0x62FA20, ParticleSystemClass_FireAI_TargetCoords, 0x6)
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
ASMJIT_PATCH(0x62D685, ParticleSystemClass_FireAt_Coords, 0x5)
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

#ifndef PERFORMANCE_HEAVY


#endif

#include <Conversions.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Wave/Body.h>

#ifndef ENABLE_THESE_THINGS
#include <Ext/WeaponType/Body.h>

ASMJIT_PATCH(0x70C862, TechnoClass_Railgun_AmbientDamageIgnoreTarget1, 0x5)
{
	enum { IgnoreTarget = 0x70CA59, Continue = 0x0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_IgnoreTarget ?
		IgnoreTarget : Continue;
}

ASMJIT_PATCH(0x70CA8B, TechnoClass_Railgun_AmbientDamageIgnoreTarget2, 0x6)
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

ASMJIT_PATCH(0x70CBDA, TechnoClass_Railgun_AmbientDamageWarhead, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(TechnoClass*, pSource, EDX);

	R->Stack<HouseClass*>(0xC, pSource->Owner);
	R->EDX(WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_Warhead.Get(pWeapon->Warhead));
	return 0x70CBE0;
}
#endif

