#pragma once

#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Utilities/VectorHelper.h>
#include <queue>
#include "SimulateBurst.h"
#include "DelayFireWeapon.h"

class WeaponTypeClass;
class TechnoClass;
struct CustomWeaponManager
{
	HelperedVector<std::unique_ptr<SimulateBurst>> simulateBurstQueue {};

	void Clear()
	{
		simulateBurstQueue.clear();
	}

	void reserve(size_t newsize) {
		simulateBurstQueue.reserve(newsize);
	}

	void Update(TechnoClass* pAttacker);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult);
	void SimulateBurstFire(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst);
	void SimulateBurstFireOnce(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, SimulateBurst* burst);
	TechnoClass* WhoIsShooter(TechnoClass* pAttacker) const;

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<CustomWeaponManager*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(simulateBurstQueue)
			.Success()
			&& Stm.RegisterChange(this)
			;
	}
};

struct FireWeaponManager
{
	HelperedVector<std::unique_ptr<DelayFireWeapon>> DelayFires {};
	CustomWeaponManager CWeaponManager {};

public:

	void Clear();
	void Insert(int weaponIndex, AbstractClass* pTarget, int delay = 0, int count = 1);
	void Insert(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay = 0, int count = 1);
	bool FireCustomWeapon(TechnoClass* pShooter, TechnoClass* pAttacker, AbstractClass* pTarget, WeaponTypeClass* pWeapon, const CoordStruct& flh, const CoordStruct& bulletSourcePos, double rofMult = 1);
	void TechnoClass_Update_CustomWeapon(TechnoClass* pAttacker);
	void FireWeaponManager_Clear();
	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<FireWeaponManager*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(DelayFires)
			.Process(CWeaponManager)
			.Success()
			&& Stm.RegisterChange(this)
			;
		;
	}
};