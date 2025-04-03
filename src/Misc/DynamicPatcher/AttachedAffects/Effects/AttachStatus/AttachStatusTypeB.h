#pragma once

#include "AttachStatusType.h"

struct AttachStatusTypeB
{
	AttachStatusTypeB() :
		FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, Cloakable { false }
		, ForceDecloak { false }
	{ };

	double FirepowerMultiplier;
	double ArmorMultiplier;
	double SpeedMultiplier;
	double ROFMultiplier;
	bool Cloakable;
	bool ForceDecloak;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(FirepowerMultiplier)
			.Process(ArmorMultiplier)
			.Process(SpeedMultiplier)
			.Process(ROFMultiplier)
			.Process(Cloakable)
			.Process(ForceDecloak)
			.Success()
			;
	}
};
