#pragma once
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

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<AircraftDive*>(this)->Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Speed)
			.Process(DataDelay)
			.Process(ZOffset)
			.Process(Delay)
			.Process(CanDive)
			.Success() && Stm.RegisterChange(this);

	}
};
