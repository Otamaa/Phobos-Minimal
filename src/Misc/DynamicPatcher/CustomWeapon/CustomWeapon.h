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
	std::vector<SimulateBurst> simulateBurstQueue {};

	CustomWeaponManager() = default;
	~CustomWeaponManager() = default;

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

	void InvalidatePointer(void* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Loading Element From CustomWeaponManager ! \n");
		return Stm
			.Process(simulateBurstQueue)
			;
	}
};

struct FireWeaponManager
{
	//std::queue<DelayFireWeapon> DelayFires;
	std::vector<std::unique_ptr<DelayFireWeapon>> DelayFires {};
	std::unique_ptr<CustomWeaponManager> CWeaponManager {};

	FireWeaponManager() = default;
	~FireWeaponManager() = default;

	void Clear();
	void Insert(int weaponIndex, AbstractClass* pTarget, int delay = 0, int count = 1);
	void Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay = 0, int count = 1);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult = 1, FireBulletToTarget callback = nullptr);
	void TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker);
	void FireWeaponManager_Clear();
	void InvalidatePointer(void* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Loading Element From FireWeaponManager ! \n");

		return Stm
			.Process(DelayFires)
			.Process(CWeaponManager)
			;
	}
};
#endif