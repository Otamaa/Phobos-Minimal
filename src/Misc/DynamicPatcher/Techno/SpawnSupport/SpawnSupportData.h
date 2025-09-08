#pragma once
#include <Utilities/TemplateDef.h>

class SpawnSupportFLHData
{
public:

	Valueable<CoordStruct> SpawnSupportFLH;
	Valueable<CoordStruct> EliteSpawnSupportFLH;

	Valueable<CoordStruct> SpawnHitFLH;
	Valueable<CoordStruct> EliteSpawnHitFLH;

	void Read(INI_EX& nParser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SpawnSupportFLHData*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(SpawnSupportFLH)
			.Process(EliteSpawnSupportFLH)
			.Process(SpawnHitFLH)
			.Process(EliteSpawnHitFLH)
			;

	}
};

class SpawnSupportData
{
public:

	Valueable<bool> Enable;
	Valueable<WeaponTypeClass*> SupportWeapon;
	Valueable<WeaponTypeClass*> EliteSupportWeapon;
	Valueable<bool> SwitchFLH;
	Valueable<bool> Always;

	Valueable<bool> FireOnce;
	Valueable<int> Delay;

	void Read(INI_EX& nParser, const char* pSection);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SpawnSupportData*>(this)->Serialize(Stm); }

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Enable)
			.Process(SupportWeapon)
			.Process(EliteSupportWeapon)
			.Process(SwitchFLH)
			.Process(Always)
			.Process(FireOnce)
			.Process(Delay)
			;
	}
};
