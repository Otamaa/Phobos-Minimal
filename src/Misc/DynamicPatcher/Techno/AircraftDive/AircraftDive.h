#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/SavegameDef.h>

class AircraftDive
{
public:
	int Speed;
	int DataDelay;

	int ZOffset;
	int Delay;
	bool CanDive;

	AircraftDive() :
		Speed { 0 }
		, DataDelay { 0 }
		, ZOffset { 0 }
		, Delay { 0 }
		, CanDive { false }

	{ }

	AircraftDive(int nSpeed, int nDataDelay) :
		Speed { nSpeed }
		, DataDelay { nDataDelay }
		, ZOffset { 0 }
		, Delay { 0 }
		, CanDive { false }

	{ }

	~AircraftDive() = default;

	AircraftDive(const AircraftDive& other) = default;
	AircraftDive& operator=(const AircraftDive& other) = default;

	int Diving()
	{
		if (--Delay < 0)
		{
			ZOffset += Speed;
			Delay = DataDelay;
		}
		return ZOffset;
	}

	void Reset()
	{
		ZOffset = 0;
		Delay = DataDelay;
		CanDive = true;
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Speed)
			.Process(DataDelay)
			.Process(ZOffset)
			.Process(Delay)
			.Process(CanDive)
			;
	}
};
#endif