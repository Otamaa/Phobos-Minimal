#pragma region Includes
#include <Utilities/Macro.h>

#include "Body.h"

#include <TechnoClass.h>
#include <FootClass.h>

#include <Misc/PhobosGlobal.h>

#pragma endregion

#ifndef ASTAR_HOOKS

// ENTRY  , 42C954 , stack 3C TECHNO , size 7 , return 0
// END 42CB3F 5 , 42CCCB

ASMJIT_PATCH(0x42D197, AStarClass_Attempt_Entry, 0x5)
{
	GET_STACK(TechnoClass*, pTech, 0x24);
	GET_STACK(CellStruct*, from, 0x1C);
	GET_STACK(CellStruct*, to, 0x20);

	PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };
	return 0x0;
}

ASMJIT_PATCH(0x42D45B, AStarClass_Attempt_Exit, 0x6)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42D44C, AStarClass_Attempt_Exit, 0x6)

ASMJIT_PATCH(0x42C8ED, AStarClass_FindHierarcial_Exit, 0x5)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42C8E2, AStarClass_FindHierarcial_Exit, 0x5)

ASMJIT_PATCH(0x42C2A7, AStarClass_FindHierarcial_Entry, 0x5)
{
	GET(TechnoClass*, pTech, ESI);
	GET_BASE(CellStruct*, from, 0x8);
	GET_BASE(CellStruct*, to, 0xC);

	if (pTech)
		PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };

	return 0x0;
}

ASMJIT_PATCH(0x42C954, AStarClass_FindPath_Entry, 0x7)
{
	GET_STACK(TechnoClass*, pTech, 0x3C);
	GET_STACK(CellStruct*, from, 0x34);
	GET_STACK(CellStruct*, to, 0x38);

	PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };
	R->ESI(pTech);
	R->EBX(R->EAX());
	return R->EDI<int>() == -1 ? 0x42C963 : 0x42C95F;
}

ASMJIT_PATCH(0x42CCC8, AStarClass_FindPath_Exit, 0x6)
{
	PhobosGlobal::Instance()->PathfindTechno.Clear();
	return 0x0;
}ASMJIT_PATCH_AGAIN(0x42CB3C, AStarClass_FindPath_Exit, 0x6)

#endif
