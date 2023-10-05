#pragma once
#include <Utilities/TemplateDef.h>

enum class DrivingState : int
{
	Moving = 0, StandStill = 1, Start = 2, Stop = 3
};

class DriveData
{
public:

	DrivingState nState { DrivingState::Stop };
	Mission LastMission { Mission::None };

	inline bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm
			.Process(nState)
			.Process(LastMission)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}

	inline bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm
			.Process(nState)
			.Process(LastMission)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(nState)
			.Process(LastMission)
			.Success()
			//&& Stm.RegisterChange(this)
			;
	}
};
