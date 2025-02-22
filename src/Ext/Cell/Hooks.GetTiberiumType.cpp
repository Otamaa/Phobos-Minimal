#include "Body.h"

#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <OverlayClass.h>
#include <FileSystem.h>
#include <IsometricTileTypeClass.h>

#include <Ext/Tiberium/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/IsometricTileType/Body.h>

DEFINE_JUMP(CALL, 0x47C206, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x47C966, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x47D820, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x47F863, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x47FBCE, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x480B6E, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x4818F3, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x4819AE, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x483639, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x4836A6, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x483725, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x48376B, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x48379E, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x4837EE, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x484FF3, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
//DEFINE_JUMP(CALL, 0x485013, MiscTools::to_DWORD(&CellExtData::GetTiberiumType)); JMP

//DEFINE_JUMP(CALL, 0x485026, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_HOOK(0x485020, CellClass_GetTibValue, 0x6) {
	GET(FakeCellClass*, pThis, ECX);
	GET_STACK(DWORD, caller , 0x0);

	//if (((int)*((DWORD*)pThis)) == -1) {
	//	Debug::FatalError(__FUNCTION__" called with invalid cell , caller %x", caller);
	//	R->EAX(0);
	//	return 0x485052;
	//}

	int type = pThis->_GetTiberiumType();
	R->EAX(type == -1 ? 0 : TiberiumClass::Array->Items[type]->Value);
	return 0x485052;
}


DEFINE_JUMP(LJMP, 0x485010, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));

DEFINE_JUMP(CALL, 0x487393, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x4873E9, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x5FDA77, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x74A3A2, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
DEFINE_JUMP(CALL, 0x74A602, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));
//DEFINE_JUMP(LJMP, 0x5FDD20, MiscTools::to_DWORD(&CellExtData::GetTiberiumType));

DEFINE_JUMP(CALL, 0x424D9C, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x452FEC, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x522EFD, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x5A2D29, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x5FD1BB, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x72291E, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x723009, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x72340E, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));
DEFINE_JUMP(CALL, 0x73D541, MiscTools::to_DWORD(&FakeCellClass::_GetTiberiumType));