#pragma once

#include <Timers.h>
#include <CellStruct.h>

#include "SavegameDef.h"

class BuildingTypeClass;
struct PlacingBuildingStruct
{
	BuildingTypeClass* Type;
	BuildingTypeClass* DrawType;
	int Times;
	CDTimerClass Timer;
	CellStruct TopLeft;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<PlacingBuildingStruct*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Type)
			.Process(DrawType)
			.Process(Times)
			.Process(Timer)
			.Process(TopLeft)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};