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

	explicit DelayFireWeapon(int weaponIndex, AbstractClass* ptarget, int delay, int count) noexcept :
		FireOwnWeapon { true }
		, WeaponIndex { weaponIndex }
		, Weapon { nullptr }
		, Target { ptarget }
		, Delay { delay }
		, Count { count }
		, Timer { delay }
	{ }

	explicit DelayFireWeapon(WeaponTypeClass* pWeapon, AbstractClass* pTarget, int delay, int count) noexcept :
		FireOwnWeapon { false }
		, WeaponIndex { -1 }
		, Weapon { pWeapon }
		, Target { pTarget }
		, Delay { delay }
		, Count { count }
		, Timer { delay }
	{ }

	DelayFireWeapon() noexcept :
		FireOwnWeapon { false }
		, WeaponIndex { -1 }
		, Weapon { nullptr }
		, Target { nullptr }
		, Delay { 0 }
		, Count { 1 }
		, Timer { 0 }
	{ }

	~DelayFireWeapon() = default;

	bool TimesUp() const;
	void ReduceOnce();
	bool NotDone() const;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm);

private:
	template <typename T>
	bool Serialize(T& Stm);
};
#endif