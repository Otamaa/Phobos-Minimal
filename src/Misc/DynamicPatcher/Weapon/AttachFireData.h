#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct AttachFireData
{

	Valueable<bool> UseROF { true };
	Valueable<bool> CheckRange { false };
	Valueable<bool> RadialFire { false };
	Valueable<int> RadialAngle { 180 };
	Valueable<bool> SimulateBurst { false };
	Valueable<int> SimulateBurstDelay { 7 };
	Valueable<int> SimulateBurstMode { 0 };
	Valueable<bool> OnlyFireInTransport { false };
	Valueable<bool> UseAlternateFLH { false };

	void Read(INI_EX& parser, const char* pSection);

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(UseROF)
			.Process(CheckRange)
			.Process(RadialFire)
			.Process(RadialAngle)
			.Process(SimulateBurst)
			.Process(SimulateBurstDelay)
			.Process(SimulateBurstMode)
			.Process(OnlyFireInTransport)
			.Process(UseAlternateFLH)
			.Success()
			;
	}
};
#endif