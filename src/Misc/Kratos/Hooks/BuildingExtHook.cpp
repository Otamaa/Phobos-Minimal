#include <exception>
#include <Windows.h>

#include <AnimClass.h>
#include <TechnoClass.h>
#include <UnitClass.h>

#include <Misc/Kratos/Kratos.h>
#include <Utilities/Macro.h>

#include <Misc/Kratos/Extension/TechnoExt.h>

#include <Misc/Kratos/Ext/Common/CommonStatus.h>
#include <Misc/Kratos/Ext/TechnoType/TechnoStatus.h>

#include <PlanningTokenClass.h>

#ifndef _ENABLE_HOOKS

// GiftBox will release a building onto a cell of existing buildings.
// if that gift is not a virtual unit like Stand. I will add it to the occupy objects,
// When a building want to clear occupy spot, check if there has another building at the same cell,
// Skip the change the OccFlag of this cell.
ASMJIT_PATCH(0x453E02, BuildingClass_Clear_Occupy_Spot_Skip, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(CellClass*, pCell, EAX);
	ObjectClass* pObject = pCell->FirstObject;
	do
	{
		if (pObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Building:
			{
				if (pObject != pTechno)
				{
					// skip change the OccFlag of this cell
					return 0x453E12;
				}
				break;
			}
			}
		}
	} while (pObject && (pObject = pObject->NextObject) != nullptr);
	return 0;
}

#pragma region BuildingWaypoints

static bool __fastcall BuildingTypeClass_CanUseWaypoint(BuildingTypeClass* pThis)
{
	return General::Data()->BuildingWaypoints;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4610, BuildingTypeClass_CanUseWaypoint)

ASMJIT_PATCH(0x4AE95E, DisplayClass_sub_4AE750_DisallowBuildingNonAttackPlanning, 0x5)
{
	enum { SkipGameCode = 0x4AE982 };

	GET(ObjectClass* const, pObject, ECX);
	LEA_STACK(CellStruct*, pCell, STACK_OFFSET(0x20, 0x8));

	const auto action = pObject->MouseOverCell(*pCell);

	if (!PlanningNodeClass::PlanningModeActive || pObject->WhatAmI() != AbstractType::Building || action == Action::Attack)
		pObject->CellClickedAction(action, pCell, pCell, false);

	return SkipGameCode;
}

#pragma endregion
#endif