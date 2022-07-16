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

	DrivingState nState;
	Mission LastMission;

	DriveData() :
		nState { DrivingState::Stop }
		, LastMission { Mission::None }
	{}

	~DriveData() = default;

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