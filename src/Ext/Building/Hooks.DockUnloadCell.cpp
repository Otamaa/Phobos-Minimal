#include "Body.h"

#include <Misc/Ares/Hooks/Header.h>

#ifdef ___Duplicates

ASMJIT_PATCH(0x7376D9, UnitClass_ReceivedRadioCommand_DockUnload_Facing, 5)
{
	GET(UnitClass* const, pUnit, ESI);
	GET(DirStruct* const, nCurrentFacing, EAX);

	const auto nDecidedFacing = TechnoExt_ExtData::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing)
		return 0x73771B;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73770C;
}

ASMJIT_PATCH(0x73DF66, UnitClass_Mi_Unload_DockUnload_Facing, 5)
{
	GET(UnitClass* const, pUnit, ESI);
	GET(DirStruct* const, nCurrentFacing, EAX);

	const auto nDecidedFacing = TechnoExt_ExtData::UnloadFacing(pUnit);

	if (*nCurrentFacing == nDecidedFacing || pUnit->IsRotating)
		return 0x73DFBD;

	pUnit->Locomotor.GetInterfacePtr()->Do_Turn(nDecidedFacing);

	return 0x73DFB0;
}

ASMJIT_PATCH(0x43CA80, BuildingClass_ReceivedRadioCommand_DockUnloadCell, 7)
{
	GET(CellStruct* const, pCell, EAX);
	GET(BuildingClass* const, pThis, ESI);

	const auto nBuff = TechnoExt_ExtData::UnloadCell(pThis);
	R->DX(pCell->X + nBuff.X);
	R->AX(pCell->Y + nBuff.Y);

	return 0x43CA8D;
}

ASMJIT_PATCH(0x73E013, UnitClass_Mi_Unload_DockUnloadCell1, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E05F;
}

ASMJIT_PATCH(0x73E17F, UnitClass_Mi_Unload_DockUnloadCell2, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E1CB;
}

ASMJIT_PATCH(0x73E2BF, UnitClass_Mi_Unload_DockUnloadCell3, 6)
{
	GET(UnitClass* const, pThis, ESI);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x73E30B;
}

ASMJIT_PATCH(0x741BDB, UnitClass_SetDestination_DockUnloadCell, 7)
{
	GET(UnitClass* const, pThis, EBP);
	R->EAX(TechnoExt_ExtData::BuildingUnload(pThis));
	return 0x741C28;
}
#endif

#include <TeamClass.h>

ASMJIT_PATCH(0x73F7DD, UnitClass_IsCellOccupied_Bib, 0x8)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(UnitClass*, pThis, EBX);

	if (pThis && pBuilding->Owner->IsAlliedWith(pThis))
	{
		if (pThis->Type->Passengers > 0)
		{
			if (auto pTeam = pThis->Team)
			{
				if (auto pScript = pTeam->CurrentScript)
				{
					auto mission = pScript->GetCurrentAction();
					if (mission.Action == TeamMissionType::Gather_at_base && TeamMissionType((int)mission.Action + 1) == TeamMissionType::Load)
					{
						//dont fucking load the passenger here
						return 0x73F823;
					}
				}
			}
		}

		return 0x0;
	}

	return 0x73F823;
}