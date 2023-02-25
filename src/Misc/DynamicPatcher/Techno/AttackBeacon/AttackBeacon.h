#pragma once

#include <Utilities/SavegameDef.h>

struct AttackBeacon
{
	int Count;
	int Timer;
	ValueableVector<Mission> EligibleMissions;

	AttackBeacon(int nCount, int nTimer) :
		Count { nCount }
		, Timer { nTimer }
		, EligibleMissions { }
	{	EligibleMissions.reserve(5);
		EligibleMissions.push_back(Mission::None);
		EligibleMissions.push_back(Mission::Sleep);
		EligibleMissions.push_back(Mission::Guard);
		EligibleMissions.push_back(Mission::Area_Guard);
		EligibleMissions.push_back(Mission::Stop);
	}

	AttackBeacon() :
		Count { 0 }
		, Timer { 0 }
		, EligibleMissions { }
	{ }

	bool IsReady() {
		return --Timer <= 0 && (Count > 0 ? Count-- > 0 : true);
	}

	void Reload(int nRate, int nCount) {
		Count = nCount;
		Timer = nRate;
	}	
	
	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From AttackBeacon ! \n");

		Stm
			.Process(Count)
			.Process(Timer)
			.Process(EligibleMissions)
			;
	}
};