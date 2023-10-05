#pragma once
#include <Utilities/TemplateDef.h>

class PassengersData
{
public:

	Valueable<bool> PassiveAcquire { true };
	Valueable<bool> ForceFire { false };
	Valueable<bool> MobileFire { true };
	Valueable<bool> SameFire { true };

	void Read(INI_EX& parser, const char* pSection, bool Allocate = false);

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(PassiveAcquire)
			.Process(ForceFire)
			.Process(MobileFire)
			.Process(SameFire)
			;

		//Stm.RegisterChange(this);
	}
};
