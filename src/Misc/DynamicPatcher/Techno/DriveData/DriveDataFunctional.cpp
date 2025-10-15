#include "DriveDataFunctional.h"
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

void DriveDataFunctional::AI(TechnoExtData* pThis)
{
	auto pTechno = pThis->This();

	if (!TechnoExtData::IsActive(pTechno))
		return;

	Mission const mission = pTechno->CurrentMission;

	auto pDrive = pThis->Get_DriveData();

	if (!pDrive)
		pDrive = &Phobos::gEntt->emplace<DriveData>(pThis->MyEntity);

	switch (mission)
	{
	case Mission::Move:
	case Mission::AttackMove:
		if (Mission::Move != pDrive->LastMission && Mission::AttackMove != pDrive->LastMission)
		{
			pDrive->nState = DrivingState::Start;
		}
		else
		{
			pDrive->nState = DrivingState::Moving;
		}
		break;
	default:
		if (Mission::Move == pDrive->LastMission || Mission::AttackMove == pDrive->LastMission)
		{
			pDrive->nState = DrivingState::Stop;
		}
		else
		{
			pDrive->nState = DrivingState::StandStill;
		}
		break;
	}

	pDrive->LastMission = mission;
}
