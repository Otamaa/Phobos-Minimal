#include "Body.h"

#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <OverlayClass.h>
#include <FileSystem.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/IsometricTileType/Body.h>

DEFINE_FUNCTION_JUMP(CALL, 0x47C206, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x47C966, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x47D820, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x47F863, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x47FBCE, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x480B6E, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x4818F3, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x4819AE, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x483639, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x4836A6, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x483725, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x48376B, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x48379E, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x4837EE, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x484FF3, CellExtData::GetTiberiumType);
//DEFINE_FUNCTION_JUMP(CALL, 0x485013, CellExtData::GetTiberiumType)); JMP

//DEFINE_FUNCTION_JUMP(CALL, 0x485026, CellExtData::GetTiberiumType));
ASMJIT_PATCH(0x485020, CellClass_GetTibValue, 0x6) {
	GET(FakeCellClass*, pThis, ECX);
//	GET_STACK(DWORD, caller , 0x0);

	//if (((int)*((DWORD*)pThis)) == -1) {
	//	Debug::FatalError(__FUNCTION__" called with invalid cell , caller %x", caller);
	//	R->EAX(0);
	//	return 0x485052;
	//}

	int type = pThis->_GetTiberiumType();
	R->EAX(type == -1 ? 0 : TiberiumClass::Array->Items[type]->Value);
	return 0x485052;
}


DEFINE_FUNCTION_JUMP(LJMP, 0x485010, FakeCellClass::_GetTiberiumType);

DEFINE_FUNCTION_JUMP(CALL, 0x487393, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x4873E9, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x5FDA77, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x74A3A2, CellExtData::GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x74A602, CellExtData::GetTiberiumType);
//DEFINE_FUNCTION_JUMP(LJMP, 0x5FDD20, CellExtData::GetTiberiumType));

DEFINE_FUNCTION_JUMP(CALL, 0x424D9C, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x452FEC, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x522EFD, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x5A2D29, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x5FD1BB, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x72291E, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x723009, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x72340E, FakeCellClass::_GetTiberiumType);
DEFINE_FUNCTION_JUMP(CALL, 0x73D541, FakeCellClass::_GetTiberiumType);