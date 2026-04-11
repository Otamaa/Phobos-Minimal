#pragma once

#include <Timers.h>
#include <CellStruct.h>


class BuildingTypeClass;
class PhobosStreamReader;
class PhobosStreamWriter;
struct PlacingBuildingStruct
{
	BuildingTypeClass* Type;
	BuildingTypeClass* DrawType;
	int Times;
	CDTimerClass Timer;
	CellStruct TopLeft;

public:

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

};