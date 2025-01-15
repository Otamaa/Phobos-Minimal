#pragma once
#include <Utilities/TemplateDef.h>

enum class DrivingState : char
{
	Moving = 0, StandStill = 1, Start = 2, Stop = 3
};

class DriveData
{
public:

	DrivingState nState { DrivingState::Stop };
	Mission LastMission { Mission::None };

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Serialize(Stm);
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<DriveData*>(this)->Serialize(Stm);
	}

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(nState)
			.Process(LastMission)
			.Success()
			&& Stm.RegisterChange(this)
			;
	}
};
