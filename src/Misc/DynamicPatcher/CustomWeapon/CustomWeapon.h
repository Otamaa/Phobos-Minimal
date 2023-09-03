#pragma once

#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <queue>
#include "SimulateBurst.h"
#include "DelayFireWeapon.h"

class WeaponTypeClass;
class TechnoClass;
struct CustomWeaponManager
{
	std::vector<SimulateBurst> simulateBurstQueue {};

	void Clear()
	{
		simulateBurstQueue.clear();
	}

	void reserve(size_t newsize) {
		simulateBurstQueue.reserve(newsize);
	}

	void Update(TechnoClass* pAttacker);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult, FireBulletToTarget callback);
	void SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst);
	void SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst& burst);
	TechnoClass* WhoIsShooter(TechnoClass* pAttacker) const;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From CustomWeaponManager ! \n");
		return Stm
			.Process(simulateBurstQueue)
			.Success()
			&& Stm.RegisterChange(this);
			;
	}
};

template <>
struct Savegame::ObjectFactory<CustomWeaponManager>
{
	std::unique_ptr<CustomWeaponManager> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<CustomWeaponManager>();
	}
};

struct FireWeaponManager
{
	//std::queue<DelayFireWeapon> DelayFires;
	std::vector<DelayFireWeapon> DelayFires {};
	CustomWeaponManager CWeaponManager {};

	//FireWeaponManager() = default;
	//~FireWeaponManager() = default;

	void Init() { 
		// DelayFires.reserve(100);
		// CWeaponManager.reserve(100);
	}

	void Clear();
	void Insert(int weaponIndex, AbstractClass* pTarget, int delay = 0, int count = 1);
	void Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay = 0, int count = 1);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult = 1, FireBulletToTarget callback = nullptr);
	void TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker);
	void FireWeaponManager_Clear();
	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From FireWeaponManager ! \n");

		return Stm
			.Process(DelayFires)
			.Process(CWeaponManager)
			;
	}
};