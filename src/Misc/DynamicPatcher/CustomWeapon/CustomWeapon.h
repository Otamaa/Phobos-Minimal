#pragma once

#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Utilities/VectorHelper.h>
#include <queue>
#include "SimulateBurst.h"
#include "DelayFireWeapon.h"

class WeaponTypeClass;
class TechnoClass;
struct SimulateBurstManager
{
	std::deque<SimulateBurst> simulateBurstQueue {};

	void Clear()
	{
		simulateBurstQueue.clear();
	}

	void Update(TechnoClass* pAttacker);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult);
	void SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst);
	void SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst);
	TechnoClass* WhoIsShooter(TechnoClass* pAttacker) const;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

struct DelayFireManager
{
	std::deque<DelayFireWeapon> DelayFires {};

public:

	void Clear();
	void Insert(int weaponIndex, AbstractClass* pTarget, int delay = 0, int count = 1);
	void Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay = 0, int count = 1);
	void Update(TechnoClass* pAttacker);

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	static void TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

};

struct WeaponTimers
{
	std::vector<std::pair<WeaponTypeClass*, CDTimerClass>> Timers;

	CDTimerClass& operator[](WeaponTypeClass* pWeapon)
	{
		// Find existing timer
		for (auto& pair : Timers)
		{
			if (pair.first == pWeapon)
			{
				return pair.second;
			}
		}

		// Not found, add new
		Timers.emplace_back(pWeapon, CDTimerClass {});
		return Timers.back().second;
	}

	void Clear()
	{
		Timers.clear();
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Stm.Process(Timers).Success() && Stm.RegisterChange(this);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return Stm.Process(Timers).Success();
	}
};
