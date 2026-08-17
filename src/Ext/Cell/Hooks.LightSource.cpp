#include "Body.h"

#include <LightSourceClass.h>
#include <BuildingClass.h>

#include <Helpers/Enumerators.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

ASMJIT_PATCH(0x5547C8, LightSourceClass_CTOR_SetInCells, 0x5)
{
	GET(LightSourceClass*, pThis, ESI);

	for (CellRangeEnumerator cell(CellClass::Coord2Cell(pThis->Location), 
		pThis->LightVisibility / Unsorted::LeptonsPerCell + 0.5);
		cell; ++cell)
	{
		const auto pCell = MapClass::Instance->TryGetCellAt(*cell);

		if (!pCell)
			continue;

		const auto pCellExt = CellExtContainer::Instance.Find(pCell);
		pCellExt->CoveringLights.emplace_back(pThis);
	}

	return 0;
}

ASMJIT_PATCH(0x555176, LightSourceClass_DTOR_ResetInCells, 0x6)
{
	GET(LightSourceClass*, pThis, ESI);

	for (CellRangeEnumerator cell(CellClass::Coord2Cell(pThis->Location), 
		pThis->LightVisibility / Unsorted::LeptonsPerCell + 0.5);
		cell; ++cell)
	{
		const auto pCell = MapClass::Instance->TryGetCellAt(*cell);

		if (!pCell)
			continue;

		const auto pCellExt = CellExtContainer::Instance.Find(pCell);
		const auto it = std::ranges::find(pCellExt->CoveringLights, pThis);

		if (it != pCellExt->CoveringLights.cend())
			pCellExt->CoveringLights.erase(it);
	}

	return 0;
}

ASMJIT_PATCH(0x48427D, CellClass_ProcessColourComponents_LightSourceCount, 0x6)
{
	GET(FakeCellClass*, pThis, EDI);
	
	R->ECX(static_cast<int>(pThis->_GetExtData()->CoveringLights.size()));
	return R->Origin() + 0x6;
}ASMJIT_PATCH_AGAIN(0x48444C, CellClass_ProcessColourComponents_LightSourceCount, 0x6)

ASMJIT_PATCH(0x48428B, CellClass_ProcessColourComponents_LightSourceItem, 0x6)
{
	enum { ApplyItem = 0x484294 };

	GET(FakeCellClass*, pThis, EDI);
	GET(int, idx, EAX);

	R->ESI(pThis->_GetExtData()->CoveringLights[idx]);
	return ApplyItem;
}

DEFINE_JUMP(LJMP, 0x4842DC, 0x4842E2) // Skip useless code

ASMJIT_PATCH(0x554BF6, LightSourceClass_554AF0_Distance_Optimize, 0x5)
{
	enum { InRange = 0x554C4B, OutOfRange = 0x554CE4 };

	GET(LightSourceClass*, pThis, EDI);

	REF_STACK(CellStruct, cell, STACK_OFFSET(0x30, -0x24));

	const auto cellCoords = CellClass::Cell2Coord(cell);

	if (MapClass::Instance->IsLocationFogged(cellCoords))
		return OutOfRange;

	const int diffX = pThis->Location.X - cellCoords.X;
	const int diffY = pThis->Location.Y - cellCoords.Y;
	const double distanceSqr = static_cast<double>(diffX) * diffX + static_cast<double>(diffY) * diffY;

	if (static_cast<double>(distanceSqr) > static_cast<double>(pThis->LightVisibility) * pThis->LightVisibility)
		return OutOfRange;


	if (const auto pCell = MapClass::Instance->TryGetCellAt(cell))
	{
		pCell->MarkForRedraw();

		if (const auto pBuilding = pCell->GetBuilding())
			pBuilding->MarkForRedraw();
	}

	return InRange;
}

DEFINE_JUMP(LJMP, 0x554D0A, 0x554D16) // Skip GScreenClass::MarkNeedsRedraw(1);