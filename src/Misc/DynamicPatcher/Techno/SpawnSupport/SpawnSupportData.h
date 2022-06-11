#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Utilities/TemplateDef.h>

class SpawnSupportFLHData
{
public:

	Valueable<CoordStruct> SpawnSupportFLH;
	Valueable<CoordStruct> EliteSpawnSupportFLH;

	Valueable<CoordStruct> SpawnHitFLH;
	Valueable<CoordStruct> EliteSpawnHitFLH;

	void Read(INI_EX& nParser, const char* pSection)
	{
		SpawnSupportFLH.Read(nParser, pSection, "SupportWeaponFLH");
		EliteSpawnSupportFLH.Read(nParser, pSection, "EliteSupportWeaponFLH");

		if (EliteSpawnSupportFLH.Get() == CoordStruct::Empty)
			EliteSpawnSupportFLH = SpawnSupportFLH;

		SpawnHitFLH.Read(nParser, pSection, "SupportWeaponHitFLH");
		EliteSpawnHitFLH.Read(nParser, pSection, "EliteSupportWeaponHitFLH");

		if (EliteSpawnHitFLH.Get() == CoordStruct::Empty)
			EliteSpawnHitFLH = SpawnHitFLH;
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
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

	void Read(INI_EX& nParser, const char* pSection)
	{
		Enable.Read(nParser, pSection, "SupportSpawns");

		if(Enable)
		{
			SupportWeapon.Read(nParser, pSection, "SupportSpawns.Weapon", true);
			EliteSupportWeapon.Read(nParser, pSection, "SupportSpawns.EliteWeapon", true);

			if (!EliteSupportWeapon)
				SupportWeapon = EliteSupportWeapon;

			SwitchFLH.Read(nParser, pSection, "SupportSpawns.SwitchFLH");
			Always.Read(nParser, pSection, "SupportSpawns.AlwaysFire");
		}

		FireOnce.Read(nParser, pSection, "SpawnFireOnce");
		Delay.Read(nParser, pSection, "SpawnFireOnceDelay");
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
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
#endif