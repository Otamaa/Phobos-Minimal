/*
#pragma once

#include <MapClass.h>
#include <AStarClass.h>
#include <ArrayClasses.h>

// Backport of MapClass::RecalculateSubZones (game address 0x584550–0x584E42).
// Redirected via DEFINE_FUNCTION_JUMP(LJMP, 0x584550, FakeMapClass::_RecalculateSubZones).
class NOVTABLE FakeMapClass final
{
public:
    static void __fastcall _RecalculateSubZones(MapClass* pThis, discard_t, CellStruct const& cell);
};
*/