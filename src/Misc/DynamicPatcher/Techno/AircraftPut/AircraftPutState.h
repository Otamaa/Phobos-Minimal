#pragma once

#include <Utilities/SavegameDef.h>

struct AircraftPutState
{
	bool AircraftPutOffset;
	bool AircraftPutOffsetFlag;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<AircraftPutState*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::LogInfo("Processing Element From DamageSelfType ! ");

		return Stm
			.Process(AircraftPutOffset)
			.Process(AircraftPutOffsetFlag)
			.Success()
			;
	}
};
