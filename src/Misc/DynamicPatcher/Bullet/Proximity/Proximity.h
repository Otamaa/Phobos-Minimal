#pragma once

#include "ProximityData.h"

#include <GeneralStructures.h>

class BuildingClass;
class TechnoClass;
class CellClass;
struct Proximity
{
	ProximityData Data;

	bool Enable;
	CellClass* pCheckedCell;
	ValueableVector<BuildingClass*> BuildingMarks;

	bool count;
	int times;
	bool safe;
	CDTimerClass safeTimer;

	Proximity(ProximityData data, TechnoClass* pAttacker, int safeDelay)
		: Data { data }
		, Enable { true }
		, pCheckedCell { nullptr }
		, BuildingMarks { }
		, safe { safeDelay > 0 }
		, safeTimer { }
		, count { data.PenetrationTimes > 0 }
		, times { data.PenetrationTimes }
	{ }

	Proximity()
		: Data { }
		, Enable { true }
		, pCheckedCell { nullptr }
		, BuildingMarks { }
		, safe { false }
		, safeTimer { }
		, count { false }
		, times { 0 }
	{ }

	~Proximity() = default;

	//========================
	bool IsSafe()
	{
		if (safe)
			safe = safeTimer.InProgress();

		return safe;
	}

	void ThroughOnce()
	{
		if (count)
			times--;
	}

	bool Explodes()
	{
		return !Data.Penetration || (count && times <= 0);
	}

	bool CheckAndMarkBuilding(BuildingClass* pBuilding)
	{
		bool find = false;

		if (!BuildingMarks.empty())
			if (BuildingMarks.Contains(pBuilding))
				find = true;

		if (!find)
			BuildingMarks.push_back(pBuilding);

		return find;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::Log("Loading Element From Proximity ! \n"); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Data)
			.Process(Enable)
			.Process(pCheckedCell)
			.Process(BuildingMarks)
			.Process(count)
			.Process(times)
			.Process(safe)
			.Process(safeTimer)
			.Success()
			;
	}
};