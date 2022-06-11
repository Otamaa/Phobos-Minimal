#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class PassengersData
{
public:

	Valueable<bool> PassiveAcquire;
	Valueable<bool> ForceFire;
	Valueable<bool> MobileFire;
	Valueable<bool> SameFire;

	PassengersData()
		: PassiveAcquire { true }
		, ForceFire { false }
		, MobileFire { true }
		, SameFire { true }
	{ }

	~PassengersData() = default;

	void Read(INI_EX& parser, const char* pSection, bool Allocate = false)
	{
		PassiveAcquire.Read(parser, pSection, "Passengers.PassiveAcquire");
		ForceFire.Read(parser, pSection, "Passengers.ForceFire");
		MobileFire.Read(parser, pSection, "Passengers.MobileFire");
		SameFire.Read(parser, pSection, "Passengers.SameFire");
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(PassiveAcquire)
			.Process(ForceFire)
			.Process(MobileFire)
			.Process(SameFire)
			;
	}
};
#endif