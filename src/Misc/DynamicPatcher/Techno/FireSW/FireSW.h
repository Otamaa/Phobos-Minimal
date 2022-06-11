#pragma once

#include <Utilities/TemplateDef.h>

struct FireSuperWeaponData
{
	ValueableVector<SuperWeaponTypeClass*> SuperWeapons;
	Valueable<bool> AnyWeapon;
	Valueable<int> WeaponIndex;
	Valueable<bool> ToTarget;
	Valueable<bool> RealLaunch;

	FireSuperWeaponData() :
		SuperWeapons { }
		, AnyWeapon { false }
		, WeaponIndex { 0 }
		, ToTarget { true }
		, RealLaunch { true }
	{}

	~FireSuperWeaponData() = default;

	void Read(INI_EX& nParser, const char* pSection)
	{
		SuperWeapons.Read(nParser, pSection, "FireSuperWeapon.Types");
		AnyWeapon.Read(nParser, pSection, "FireSuperWeapon.AnyWeapon");
		WeaponIndex.Read(nParser, pSection, "FireSuperWeapon.Weapon");
		ToTarget.Read(nParser, pSection, "FireSuperWeapon.ToTarget");
		RealLaunch.Read(nParser, pSection, "FireSuperWeapon.RealLaunch");
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(SuperWeapons)
			.Process(AnyWeapon)
			.Process(WeaponIndex)
			.Process(ToTarget)
			.Process(RealLaunch)
			;
	}
};