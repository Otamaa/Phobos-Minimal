#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES

#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <queue>
#include "SimulateBurst.h"
#include "DelayFireWeapon.h"

class WeaponTypeClass;
class TechnoClass;
struct CustomWeaponManager
{
	//std::queue<SimulateBurst> simulateBurstQueue;
	std::vector<SimulateBurst> simulateBurstQueue;

	void Clear()
	{
		//while (!simulateBurstQueue.empty()) simulateBurstQueue.pop();
		simulateBurstQueue.clear();
	}

	void Update(TechnoClass* pAttacker);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult, FireBulletToTarget callback);
	void SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst);
	void SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst);
	TechnoClass* WhoIsShooter(TechnoClass* pAttacker) const;

	void InvalidatePointer(void* ptr, bool bRemoved)
	{
		if (!simulateBurstQueue.empty())
		{
			for (size_t pos = 0; pos < simulateBurstQueue.size(); pos++)
			{
				if (simulateBurstQueue[pos].Target == ptr)
				{
					Debug::Log("Found Invalid Target from CustomWeaponManager ! , Cleaning Up ! \n");
					simulateBurstQueue.erase(simulateBurstQueue.begin() + pos); //because it queue , we need to remove it instead

				}

				if (simulateBurstQueue[pos].Shooter == ptr)
				{
					Debug::Log("Found Shooter Target from CustomWeaponManager ! , Cleaning Up ! \n");
					simulateBurstQueue.erase(simulateBurstQueue.begin() + pos); //because it queue , we need to remove it instead
				}

			}
		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From CustomWeaponManager ! \n");
		Stm
			.Process(simulateBurstQueue)
			;
	}
};

struct FireWeaponManager
{
	//std::queue<DelayFireWeapon> DelayFires;
	std::vector<DelayFireWeapon> DelayFires;

	CustomWeaponManager CWeaponManager;

	void Clear()
	{
		//while (!DelayFires.empty()) DelayFires.pop();
		DelayFires.clear();
	}

	void Insert(int weaponIndex, AbstractClass* pTarget, int delay = 0, int count = 1)
	{
		DelayFires.emplace_back(weaponIndex, pTarget, delay, count);
	}

	void Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay = 0, int count = 1)
	{
		DelayFires.emplace_back(pWeapon, pTarget, delay, count);
	}

	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult = 1, FireBulletToTarget callback = nullptr)
	{
		return CWeaponManager.FireCustomWeapon(pShooter, pAttacker, pTarget, pWeapon, flh, bulletSourcePos, rofMult, callback);
	}

	void TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker);

	void FireWeaponManager_Clear()
	{
		Clear();
		CWeaponManager.Clear();
	}

	void InvalidatePointer(void* ptr, bool bRemoved)
	{
		if (!DelayFires.empty())
		{
			for (size_t pos = 0 ; pos < DelayFires.size(); pos++)
			{
				if (DelayFires[pos].Target == ptr)
				{
					Debug::Log("Found Invalid Target from FireWeaponManager ! , Cleaning Up ! \n");
					DelayFires.erase(DelayFires.begin() + pos); //because it queue , we need to remove it instead
				}
			}
		}

		CWeaponManager.InvalidatePointer(ptr, bRemoved);
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From FireWeaponManager ! \n");

		Stm
			.Process(DelayFires)
			;
		CWeaponManager.Serialize(Stm);
	}
};
#endif