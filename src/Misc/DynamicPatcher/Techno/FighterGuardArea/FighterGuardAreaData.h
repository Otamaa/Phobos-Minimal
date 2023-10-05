#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct FighterAreaGuardData
{
	bool Enable { false };

	Valueable<bool> AreaGuard { false };
	Valueable<bool> AutoGuard { false };
	Valueable<bool> DefaultToGuard { false };
	Valueable<int> GuardRange { 5 };
	Valueable<bool> AutoFire { true };
	Valueable<int> MaxAmmo { 1 };
	Valueable<int> MinAmmo { 0 };
	Valueable<int> GuardRadius { 5 };
	Valueable<bool> FindRangeAroundSelf { false };
	Valueable<int> ChaseRange { 30 };
	Valueable<bool> Clockwise { false };
	Valueable<bool> Randomwise { false };

	void Read(INI_EX& parser, const char* pSection, TechnoTypeClass* pType);

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Enable)

			.Process(AreaGuard)
			.Process(AutoGuard)
			.Process(DefaultToGuard)
			.Process(GuardRange)
			.Process(AutoFire)
			.Process(MaxAmmo)
			.Process(MinAmmo)
			.Process(GuardRadius)
			.Process(FindRangeAroundSelf)
			.Process(ChaseRange)
			.Process(Clockwise)
			.Process(Randomwise)
			;

		//Stm.RegisterChange(this);
	}
};
#endif