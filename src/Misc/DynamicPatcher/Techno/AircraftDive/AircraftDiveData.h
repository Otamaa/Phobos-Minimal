#pragma once
#include <Utilities/TemplateDef.h>

class AircraftDiveData
{
public:

	Valueable<bool> Enable { false };
	Valueable<int> Distance { 0 };
	Valueable<int> Speed { 1 };
	Valueable<int> Delay { 0 };
	Valueable<int> FlightLevel { 300 };
	Valueable<bool> PullUpAfterFire { false };

	void Read(INI_EX& parser, const char* pSection, bool Allocate = false);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<AircraftDiveData*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::LogInfo("Loading Element From AircraftDiveData ! ");
		return Stm
			.Process(Enable)
			.Process(Distance)
			.Process(Speed)
			.Process(Delay)
			.Process(FlightLevel)
			.Process(PullUpAfterFire)
			;
	}
};
