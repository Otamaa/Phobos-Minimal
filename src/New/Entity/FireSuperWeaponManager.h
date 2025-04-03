#pragma once

#include <Utilities/SavegameDef.h>
#include "LauchSWData.h"
#include <../Misc/DynamicPatcher/Helpers/Helpers.h>

struct FireSuperWeapon
{
	HouseClass* House;
	CellStruct Location;
	LauchSWData Data;

private:

	int count;
	int initDelay;
	CDTimerClass initDelayTimer;
	int delay;
	CDTimerClass delayTimer;

public:

	FireSuperWeapon(HouseClass* pHouse, CellStruct location, LauchSWData data) :
		House { pHouse } ,
		Location { location },
		Data { data } ,
		count { 0 } ,
		initDelay { 0 } ,
		initDelayTimer { } ,
		delay { 0 } ,
		delayTimer { }
	{
		if (initDelay > 0)
			initDelayTimer.Start(initDelay);

		if (delay > 0)
			delayTimer.Start(delay);
	}

	bool CanLaunch()
	{
		return initDelayTimer.Expired() && delayTimer.Expired();
	}

	bool Cooldown()
	{
		count++;
		delayTimer.Start(delay);
		return IsDone();
	}

	bool IsDone()
	{
		return count >= 1;
	}

};

class FireSuperWeaponManager
{
	// Burst发射模式下剩余待发射的队列
	std::queue<FireSuperWeapon> fireSuperWeaponQueue;

public :

	FireSuperWeaponManager():
		fireSuperWeaponQueue { }
	{ }

	void Order(HouseClass* pHouse, CoordStruct location, LauchSWData data)
	{
		CellStruct cellStruct = Map[location]->MapCoords;
		fireSuperWeaponQueue.emplace(pHouse, cellStruct , data);
	}

	void Launch(HouseClass* pHouse, CoordStruct location, LauchSWData data)
	{
		CellStruct cellStruct = Map[location]->MapCoords;
		LaunchSuperWeapons(pHouse, cellStruct, data);
	}

	void Reset()
	{
		while (!fireSuperWeaponQueue.empty()) fireSuperWeaponQueue.pop();
	}

	void Update()
	{
		for (int i = 0; i < fireSuperWeaponQueue.size(); i++)
		{
			FireSuperWeapon fireSuperWeapon = fireSuperWeaponQueue.front();
			fireSuperWeaponQueue.pop();

			if (fireSuperWeapon.CanLaunch())
			{
				LaunchSuperWeapons(fireSuperWeapon.House, fireSuperWeapon.Location, fireSuperWeapon.Data);
				fireSuperWeapon.Cooldown();
			}

			if (!fireSuperWeapon.IsDone())
			{
				fireSuperWeaponQueue.emplace(fireSuperWeapon);
			}
		}
	}

private:

	void LaunchSuperWeapons(HouseClass* pHouse, CellStruct targetPos, LauchSWData data)
	{
		if (data.LaunchWhat)
		{
			// Check House alive
			if (!pHouse || pHouse->Defeated)
			{
				// find civilian
				pHouse = HouseExtData::FindCivilianSide();

				if (!pHouse)
				{
					//Debug::LogInfo("Want to fire a super weapon %s, but house is null.", data.LaunchWhat->get_ID());
					return;
				}
			}

			//int superCount = data.Supers.Count;
			//int chanceCount = null != data.Chances ? data.Chances.Count : 0;

			for (int index = 0; index < superCount; index++)
			{
				// 检查概率
				if (data.Chances.Bingo(index))
				{
					if (!pType.IsNull)
					{
						Pointer<SuperClass> pSuper = pHouse.Ref.FindSuperWeapon(pType);
						if (pSuper.Ref.IsCharged || !data.RealLaunch)
						{
							pSuper.Ref.IsCharged = true;
							pSuper.Ref.Launch(targetPos, true);
							pSuper.Ref.IsCharged = false;
							pSuper.Ref.Reset();
						}
					}
				}
			}
		}
	}
};
