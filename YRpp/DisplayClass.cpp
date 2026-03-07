#include "DisplayClass.h"

#include <Unsorted.h>

bool DisplayClass::PassesProximityCheck()
{
	CellStruct zoneCell = Unsorted::Display_ZoneCell.get();
	CellStruct zoneOffset = Unsorted::Display_ZoneOffset.get();
	CellStruct center = zoneCell + zoneOffset;
	return PassesProximityCheck((ObjectTypeClass*)Unsorted::Display_PendingObject.get(), Unsorted::Display_PendingHouse.get(), Unsorted::CursorSize.get(), &center);
}
