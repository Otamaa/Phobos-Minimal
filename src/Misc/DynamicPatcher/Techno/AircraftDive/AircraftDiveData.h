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

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From AircraftDiveData ! \n");
		Stm
			.Process(Enable)
			.Process(Distance)
			.Process(Speed)
			.Process(Delay)
			.Process(FlightLevel)
			.Process(PullUpAfterFire)
			;
	}
};
