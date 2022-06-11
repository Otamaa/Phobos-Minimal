#pragma once
#include <Utilities/TemplateDef.h>
#include "../Base.h"

struct AttachStatusType : public EffectType
{
	AttachStatusType() :
		FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, Cloakable { false }
		, ForceDecloak { false }
	{ };

	AttachStatusType(const AttachStatusType& nAnother) :
		FirepowerMultiplier { nAnother.FirepowerMultiplier.Get() }
		, ArmorMultiplier { nAnother.ArmorMultiplier.Get() }
		, SpeedMultiplier { nAnother.SpeedMultiplier.Get() }
		, ROFMultiplier { nAnother.ROFMultiplier.Get() }
		, Cloakable { nAnother.Cloakable.Get() }
		, ForceDecloak { nAnother.ForceDecloak.Get() }
	{ };

	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<double> ROFMultiplier;
	Valueable<bool> Cloakable;
	Valueable<bool> ForceDecloak;

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
