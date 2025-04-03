#pragma once
#include <Utilities/TemplateDef.h>

struct AttachStatusType
{
	AttachStatusType() :
		FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, Cloakable { false }
		, ForceDecloak { false }
	{ };

	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<double> ROFMultiplier;
	Valueable<bool> Cloakable;
	Valueable<bool> ForceDecloak;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(FirepowerMultiplier)
			.Process(ArmorMultiplier)
			.Process(SpeedMultiplier)
			.Process(ROFMultiplier)
			.Process(Cloakable)
			.Process(ForceDecloak)
			;
	}
};
