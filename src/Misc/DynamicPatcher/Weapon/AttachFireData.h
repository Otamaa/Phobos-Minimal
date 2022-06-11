#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct AttachFireData
{

	Valueable<bool> UseROF;
	Valueable<bool> CheckRange;
	Valueable<bool> RadialFire;
	Valueable<int> RadialAngle;
	Valueable<bool> SimulateBurst;
	Valueable<int> SimulateBurstDelay;
	Valueable<int> SimulateBurstMode;
	Valueable<bool> OnlyFireInTransport;
	Valueable<bool> UseAlternateFLH;

	AttachFireData() :
		UseROF { true }
		, CheckRange { false }
		, RadialFire { false }
		, RadialAngle { 180 }
		, SimulateBurst { false }
		, SimulateBurstDelay { 7 }
		, SimulateBurstMode { 0 }
		, OnlyFireInTransport { false }
		, UseAlternateFLH { false }
	{ }

	~AttachFireData() = default;

	void Read(INI_EX& parser, const char* pSection)
	{
		UseROF.Read(parser, pSection, "AttachFire.UseROF");
		CheckRange.Read(parser, pSection, "AttachFire.CheckRange");
		RadialFire.Read(parser, pSection, "AttachFire.RadialFire");
		RadialAngle.Read(parser, pSection, "AttachFire.RadialAngle");
		SimulateBurst.Read(parser, pSection, "AttachFire.SimulateBurst");
		SimulateBurstDelay.Read(parser, pSection, "AttachFire.SimulateBurstDelay");
		SimulateBurstMode.Read(parser, pSection, "AttachFire.SimulateBurstMode");
		OnlyFireInTransport.Read(parser, pSection, "AttachFire.OnlyFireInTransport");
		UseAlternateFLH.Read(parser, pSection, "AttachFire.UseAlternateFLH");
	}

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