#include "DelayFireWeapon.h"

bool DelayFireWeapon::TimesUp() const { return Timer.Expired(); }

void DelayFireWeapon::ReduceOnce()
{
	Count--;
	Timer.Start(Delay);
}

bool DelayFireWeapon::NotDone() const { return Count > 0; }
bool DelayFireWeapon::Load(PhobosStreamReader& Stm, bool RegisterForChange) { return Serialize(Stm);  }
bool DelayFireWeapon::Save(PhobosStreamWriter& Stm) const { return
	const_cast<DelayFireWeapon*>(this)->Serialize(Stm);
}

template <typename T>
bool DelayFireWeapon::Serialize(T& Stm)
{
	return Stm
		.Process(FireOwnWeapon)
		.Process(WeaponIndex)
		.Process(Weapon, true)
		.Process(Target, true)
		.Process(Delay)
		.Process(Count)
		.Process(Timer)
		.Success()
		;
}
