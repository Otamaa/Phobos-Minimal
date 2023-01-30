#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ParticleSystemClass.h>
#include <FootClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Fix techno target coordinates (used for fire angle calculations, target lines etc) to take building target coordinate offsets into accord.
// This, for an example, fixes a vanilla bug where Destroyer has trouble targeting Naval Yards with its cannon weapon from certain angles.
DEFINE_HOOK(0x70BCDC, TechnoClass_GetTargetCoords_BuildingFix, 0x6)
{
	GET(const AbstractClass* const, pTarget, ECX);
	LEA_STACK(CoordStruct*, nCoord, 0x28);

	if (const auto pBuilding = specific_cast<const BuildingClass*>(pTarget)) {
		//auto const& nTargetCoord = pBuilding->Type->TargetCoordOffset;
		//Debug::Log("__FUNCTION__ Building  Target [%s] with TargetCoord %d , %d , %d \n", pBuilding->get_ID(), nTargetCoord.X , nTargetCoord.Y , nTargetCoord.Z);
		pBuilding->GetTargetCoords(nCoord);
	} else {
		pTarget->GetCenterCoords(nCoord);
	}

	R->EAX(nCoord);
	return 0x70BCE6u;
}

// Fix railgun target coordinates potentially differing from actual target coords.
DEFINE_HOOK(0x70C6AF, TechnoClass_Railgun_TargetCoords, 0x6)
{
	GET(AbstractClass*, pTarget, EBX);
	GET(CoordStruct*, pBuffer, ECX);

	switch (pTarget->WhatAmI())
	{
	case AbstractType::Building:
	{
		auto const pBuilding = static_cast<BuildingClass*>(pTarget);
		pBuilding->GetTargetCoords(pBuffer);
	}
	case AbstractType::Cell:
	{
		auto const pCell = static_cast<CellClass*>(pTarget);
		pCell->GetCoords(pBuffer);
		if (pCell->ContainsBridge())
		{
			pBuffer->Z += CellClass::BridgeHeight;
		}
	}
	default:
		pTarget->GetCenterCoords(pBuffer);
	}

	R->EAX(pBuffer);
	return 0x70C6B5;
}

// Do not adjust map coordinates for railgun particles that are below cell coordinates.
DEFINE_HOOK(0x62B8BC, ParticleClass_CTOR_RailgunCoordAdjust, 0x6)
{
	enum { SkipCoordAdjust = 0x62B8CB };

	GET(ParticleClass*, pThis, ESI);

	if (pThis->ParticleSystem && pThis->ParticleSystem->Type->BehavesLike == BehavesLike::Railgun)
		return SkipCoordAdjust;

	return 0;
}

#ifdef PERFORMANCE_HEAVY

namespace FireAtTemp
{
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
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

 //Cut railgun logic off at obstacle coordinates.
DEFINE_HOOK(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { NextCheck = 0x70CA79, Stop = 0x70CAD8 , Continue = 0x0 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	if(FireAtTemp::pObstacleCell) {
	  const auto pCell = MapClass::Instance->GetCellAt(coords);

	  return (pCell == FireAtTemp::pObstacleCell) ? Stop : NextCheck;
	}

	return Continue;
}

 //Adjust target coordinates for laser drawing.
DEFINE_HOOK(0x6FD38D, TechnoClass_LaserZap_Obstacles, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	auto coords = *pTargetCoords;
	if (const auto pObstacleCell = FireAtTemp::pObstacleCell)
		coords = pObstacleCell->GetCoordsWithBridge();

	R->EAX(&coords);
	return 0;
}

// Adjust target for bolt / beam / wave drawing.
DEFINE_HOOK(0x6FF57D, TechnoClass_FireAt_TargetSet, 0x6)
{
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));
	GET(AbstractClass*, pTarget, EDI);

	FireAtTemp::originalTargetCoords = *pTargetCoords;
	FireAtTemp::pOriginalTarget = pTarget;

	if (FireAtTemp::pObstacleCell)
	{
		auto coords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
		pTargetCoords->X = coords.X;
		pTargetCoords->Y = coords.Y;
		pTargetCoords->Z = coords.Z;

		R->EDI(FireAtTemp::pObstacleCell);
	}

	return 0;
}

// Restore original target values and unset obstacle cell.
DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_ObstacleCellUnset, 0x6)
{
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));

	const auto coords = FireAtTemp::originalTargetCoords;
	pTargetCoords->X = coords.X;
	pTargetCoords->Y = coords.Y;
	pTargetCoords->Z = coords.Z;
	const auto target = FireAtTemp::pOriginalTarget;

	FireAtTemp::originalTargetCoords = CoordStruct::Empty;
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;

	R->EDI(target);

	return 0;
}
#endif