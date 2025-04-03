#pragma once

#include "ProximityRangeData.h"

struct ProximityRange
{
	//ProximityRangeData Data;
	bool Enable;
	int Range;

	ProximityRange(int range)
		: Enable { range > 0 }
		, Range { range }
	{ }

	ProximityRange()
		: Enable { false }
		, Range { 0 }
	{ }

	~ProximityRange() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From ProximityRange ! ");  return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Enable)
			.Process(Range)
			.Success()
			;
	}
};
