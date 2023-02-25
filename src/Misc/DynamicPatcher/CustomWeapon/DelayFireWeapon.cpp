#include "DelayFireWeapon.h"

#ifdef COMPILE_PORTED_DP_FEATURES

bool DelayFireWeapon::TimesUp() const
{
	return Timer.Expired();
}

void DelayFireWeapon::ReduceOnce()
{
	Count--;
	Timer.Start(Delay);
}

bool DelayFireWeapon::NotDone() const
{
	return Count > 0;
}

bool DelayFireWeapon::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
  //Debug::Log("Loading Element From DelayFireManager ! \n");
  return Serialize(Stm); 
}

bool DelayFireWeapon::Save(PhobosStreamWriter& Stm)
{ return Serialize(Stm); }

template <typename T>
bool DelayFireWeapon::Serialize(T& Stm)
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
		&& Stm.RegisterChange(this);
		;
}

#endif