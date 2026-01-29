#pragma once
#include <Utilities/TemplateDef.h>
#include <Utilities/Savegame.h>

struct AttachFireData
{
	Valueable<int> SimulateBurstDelay { 7 };
	Valueable<int> SimulateBurstMode { 0 };
	Valueable<int> RadialAngle { 180 };
	Valueable<bool> UseROF { true };
	Valueable<bool> CheckRange { false };
	Valueable<bool> RadialFire { false };
	Valueable<bool> SimulateBurst { false };
	Valueable<bool> OnlyFireInTransport { false };
	Valueable<bool> UseAlternateFLH { false };

	void Read(INI_EX& parser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
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

	bool Save(PhobosStreamWriter& Stm) const {
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
