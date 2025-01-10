#include "Body.h"

#include <OverlayClass.h>
#include <TerrainClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/Terrain/Body.h>

#include <Helpers/Macro.h>

#define IS_CELL_OCCUPIED(pCell)\
pCell->OccupationFlags & 0x20 || pCell->OccupationFlags & 0x40 || pCell->OccupationFlags & 0x80 || pCell->GetInfantry(false) \

// Passable TerrainTypes Hook #1 - Do not set occupy bits.
DEFINE_HOOK(0x71C110, TerrainClass_SetOccupyBit_PassableTerrain, 0x5)
{
	enum { Skip = 0x71C1A0 };

	GET(TerrainClass*, pThis, ECX);

	return (TerrainTypeExtContainer::Instance.Find(pThis->Type)->IsPassable) ? Skip : 0;
}

// Passable TerrainTypes Hook #2 - Do not display attack cursor unless force-firing.
DEFINE_HOOK(0x7002E9, TechnoClass_WhatAction_PassableTerrain, 0x5)
{
	enum { ReturnAction = 0x70020E };

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(bool, isForceFire, STACK_OFFS(0x1C, -0x8));

	if (!pThis->Owner->IsControlledByHuman() || !pThis->IsControllable())
		return 0;

	if (auto const pTerrain = cast_to<TerrainClass* , false>(pTarget))
	{
		if (TerrainExtData::CanMoveHere(pThis , pTerrain) && !isForceFire)
		{
			R->EBP(Action::Move);
			return ReturnAction;
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #3 - Count passable TerrainTypes as completely passable.
DEFINE_HOOK(0x483DDF, CellClass_CheckPassability_PassableTerrain, 0x6)
{
	enum { SkipToNextObject = 0x483DCD, ReturnFromFunction = 0x483E25, BreakFromLoop = 0x483DDF };

	GET(CellClass*, pThis, EDI);
	GET(TerrainClass*, pTerrain, ESI);

	if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->IsPassable) {
		pThis->Passability = PassabilityType::Passable;
		return ReturnFromFunction;
	}

	return 0x0;
}

// Passable TerrainTypes Hook #4 - Make passable for vehicles.
DEFINE_HOOK(0x73FBA7, UnitClass_CanEnterCell_PassableTerrain, 0x5)
{
	enum { ReturnPassable = 0x73FD37, SkipTerrainChecks = 0x73FA7C };

	GET(UnitClass*, pThis, EBX);
	GET(TerrainClass*, pTerrain, ESI);

	if (TerrainExtData::CanMoveHere(pThis, pTerrain)) {
		if (IS_CELL_OCCUPIED(pTerrain->GetCell()))
			return SkipTerrainChecks;

		R->EBP(0);
		return ReturnPassable;
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #1 - Allow placing buildings on top of them.
DEFINE_HOOK_AGAIN(0x47C80E, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)
DEFINE_HOOK(0x47C745, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)
{
	enum { Skip = 0x47C85F, SkipFlags = 0x47C6A0 };

	GET(CellClass*, pThis, EDI);

	if (auto const pTerrain = pThis->GetTerrain(false))
	{
		if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn)
		{
			if (IS_CELL_OCCUPIED(pThis))
				return Skip;
			else
				return SkipFlags;
		}
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #2 - Allow placing laser fences on top of them.
DEFINE_HOOK(0x47C657, CellClass_IsClearTo_Build_BuildableTerrain_LF, 0x6)
{
	enum { Skip = 0x47C6A0, Return = 0x47C6D1 };

	GET(CellClass*, pThis, EDI);

	if (auto pObj = pThis->FirstObject)
	{
		bool isEligible = true;

		while (true)
		{
			const auto what = pObj->WhatAmI();

			isEligible = what != BuildingClass::AbsID;

			if(what == TerrainClass::AbsID) {
				if (auto const pTerrain = static_cast<TerrainClass*>(pObj)) {
					isEligible = TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn;
				}
			}

			if (!isEligible)
				break;

			pObj = pObj->NextObject;

			if (!pObj)
				return Skip;
		}

		return Return;
	}

	return Skip;
}

// Buildable-upon TerrainTypes Hook #3 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		return (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn) ? ContinueChecks : DontDraw;
	}

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #4 - Remove them when buildings are placed on them.
DEFINE_HOOK(0x5684B1, MapClass_PlaceDown_BuildableTerrain, 0x6)
{
	GET(ObjectClass*, pObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pObject->WhatAmI() == BuildingClass::AbsID)
	{
		if (auto const pTerrain = pCell->GetTerrain(false))
		{
			if (TerrainTypeExtContainer::Instance.Find(pTerrain->Type)->CanBeBuiltOn)
			{
				pCell->RemoveContent(pTerrain, false);
				TerrainTypeExtData::Remove(pTerrain);
			}
		}
	}

	return 0;
}

#undef IS_CELL_OCCUPIED