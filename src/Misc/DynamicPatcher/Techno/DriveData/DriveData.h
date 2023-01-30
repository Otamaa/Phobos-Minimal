#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
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

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(nState)
			.Process(LastMission)
			;
	}
};
#endif