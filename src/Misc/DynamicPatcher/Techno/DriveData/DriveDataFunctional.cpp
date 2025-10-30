#include "DriveDataFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

void DriveDataFunctional::AI(TechnoExtData* pThis)
{
	auto pTechno = pThis->This();

	if (!TechnoExtData::IsActive(pTechno))
		return;

	Mission const mission = pTechno->CurrentMission;
	auto& nDriveData = pThis->MyDriveData;

	switch (mission)
	{
	case Mission::Move:
	case Mission::AttackMove:
		if (Mission::Move != nDriveData.LastMission && Mission::AttackMove != nDriveData.LastMission)
		{
			nDriveData.nState = DrivingState::Start;
		}
		else
		{
			nDriveData.nState = DrivingState::Moving;
		}
		break;
	default:
		if (Mission::Move == nDriveData.LastMission || Mission::AttackMove == nDriveData.LastMission)
		{
			nDriveData.nState = DrivingState::Stop;
		}
		else
		{
			nDriveData.nState = DrivingState::StandStill;
		}
		break;
	}

	nDriveData.LastMission = mission;
}
