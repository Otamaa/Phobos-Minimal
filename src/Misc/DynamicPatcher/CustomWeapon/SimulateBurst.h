#pragma once

#include <Utilities/TemplateDef.h>
#include <Misc/DynamicPatcher/Weapon/AttachFireData.h>

struct SimulateBurst
{
	WeaponTypeClass* WeaponType;
	TechnoClass* Shooter;
	AbstractClass* Target;
	CoordStruct FLH;
	int Burst;
	int MinRange;
	int Range;
	AttachFireData FireData;
	FireBulletToTarget Callback;
	int FlipY;
	int Flag;
	int Index;
	CDTimerClass Timer;

	SimulateBurst(WeaponTypeClass* pWeaponType, TechnoClass* pShooter, AbstractClass* pTarget, CoordStruct flh, int burst, int minRange, int range,const AttachFireData& fireData, int flipY, FireBulletToTarget callback)
		:WeaponType { pWeaponType }
		, Shooter { pShooter }
		, Target { pTarget }
		, FLH { flh }
		, Burst { burst }
		, MinRange { minRange }
		, Range { range }
		, FireData { fireData }
		, Callback { callback }
		, FlipY { flipY }
		, Flag { flipY }
		, Index { 0 }
		, Timer { fireData.SimulateBurstDelay }
	{  }

	SimulateBurst()
		:WeaponType { nullptr }
		, Shooter { nullptr }
		, Target { nullptr }
		, FLH { CoordStruct::Empty }
		, Burst { 0 }
		, MinRange { 0 }
		, Range { 0 }
		, FireData { }
		, Callback { nullptr }
		, FlipY {  }
		, Flag {  }
		, Index { 0 }
		, Timer { }
	{ }

	~SimulateBurst() = default;

	bool CanFire() {
		if (Timer.Expired())
		{
			Timer.Start(FireData.SimulateBurstDelay);
			return true;
		}
		return false;
	}

	void CountOne() {
		Index++;
		switch (FireData.SimulateBurstMode)
		{
		case 1:
			FlipY *= -1;
			break;
		case 2:
			FlipY = (Index < Burst / 2.0f) ? Flag : -Flag;
			break;
		default:
			break;
		}
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	SimulateBurst(const SimulateBurst& other) = default;
	SimulateBurst& operator=(const SimulateBurst& other) = default;

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		//Debug::Log("Processing Element From SimulateBurst ! \n");

		return Stm
			.Process(WeaponType)
			.Process(Shooter)
			.Process(Target)
			.Process(FLH)
			.Process(Burst)
			.Process(Range)
			.Process(FireData)
			.Process(FlipY)
			.Process(Flag)
			.Process(Index)
			.Process(Timer)
			.Success()
			//&& Stm.RegisterChange(this)
			;
		;
	}

};

template <>
struct Savegame::ObjectFactory<SimulateBurst>
{
	std::unique_ptr<SimulateBurst> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<SimulateBurst>();
	}
};
