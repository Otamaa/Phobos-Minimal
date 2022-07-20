#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

struct DelayFireWeapon
{
	bool FireOwnWeapon;
	int WeaponIndex;
	WeaponTypeClass* Weapon;
	AbstractClass* Target;
	int Delay;
	int Count;
	TimerStruct Timer;


	DelayFireWeapon(int weaponIndex, AbstractClass* ptarget, int delay, int count) :
		FireOwnWeapon { true }
		, WeaponIndex { weaponIndex }
		, Weapon { nullptr }
		, Target { ptarget }
		, Delay { delay }
		, Count { count }
		, Timer { delay }
	{ }

	DelayFireWeapon(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay, int count) :
		FireOwnWeapon { false }
		, WeaponIndex { -1 }
		, Weapon { pWeapon }
		, Target { pTarget }
		, Delay { delay }
		, Count { count }
		, Timer { delay }
	{ }

	DelayFireWeapon() :
		FireOwnWeapon { false }
		, WeaponIndex { -1 }
		, Weapon { nullptr }
		, Target { nullptr }
		, Delay { 0 }
		, Count { 1 }
		, Timer { 0 }
	{ }

	~DelayFireWeapon() = default;

	bool TimesUp() const
	{
		return Timer.Expired();
	}

	void ReduceOnce()
	{
		Count--;
		Timer.Start(Delay);
	}

	bool NotDone() const
	{
		return Count > 0;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::Log("Loading Element From DelayFireManager ! \n");  return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(FireOwnWeapon)
			.Process(WeaponIndex)
			.Process(Weapon)
			.Process(Target)
			.Process(Delay)
			.Process(Count)
			.Process(Timer)
			.Success()
			;
	}
};
#endif