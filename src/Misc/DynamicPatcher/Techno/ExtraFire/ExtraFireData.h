#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <CoordStruct.h>
#include <Utilities/TemplateDef.h>

class WeaponTypeClass;

struct ExtraFireData
{
	struct FLHData
	{
		Valueable<CoordStruct> PrimaryWeaponFLH;
		Nullable<CoordStruct> ElitePrimaryWeaponFLH;
		Valueable<CoordStruct> SecondaryWeaponFLH;
		Nullable<CoordStruct> EliteSecondaryWeaponFLH;

		ValueableVector<CoordStruct> WeaponXFLH;
		NullableVector<CoordStruct> EliteWeaponXFLH;

		template <typename T>
		void Serialize(T& Stm)
		{
			Stm
				.Process(PrimaryWeaponFLH)
				.Process(ElitePrimaryWeaponFLH)
				.Process(SecondaryWeaponFLH)
				.Process(EliteSecondaryWeaponFLH)
				.Process(WeaponXFLH)
				.Process(EliteWeaponXFLH);
		}

		FLHData() :
			PrimaryWeaponFLH { {0,0,0} }
			, ElitePrimaryWeaponFLH { }
			, SecondaryWeaponFLH { {0,0,0} }
			, EliteSecondaryWeaponFLH { }
			, WeaponXFLH { }
			, EliteWeaponXFLH { }
		{ }

		~FLHData() = default;
	};

	struct WeaponData
	{
		ValueableVector<WeaponTypeClass*> PrimaryWeapons;
		ValueableVector<WeaponTypeClass*> SecondaryWeapons;
		NullableVector<WeaponTypeClass*> ElitePrimaryWeapons;
		NullableVector<WeaponTypeClass*> EliteSecondaryWeapons;

		ValueableVector<std::vector<WeaponTypeClass*>> WeaponX;
		ValueableVector<std::vector<WeaponTypeClass*>> EliteWeaponX;

		template <typename T>
		void Serialize(T& Stm)
		{
			Stm
				.Process(PrimaryWeapons)
				.Process(SecondaryWeapons)
				.Process(ElitePrimaryWeapons)
				.Process(EliteSecondaryWeapons)
				.Process(WeaponX)
				.Process(EliteWeaponX)
				;
		}

		WeaponData():
			  PrimaryWeapons { }
			, SecondaryWeapons { }
			, ElitePrimaryWeapons { }
			, EliteSecondaryWeapons { }
			, WeaponX { }
			, EliteWeaponX { }
		{ }

		~WeaponData() = default;
	};

	FLHData AttachedFLH;
	WeaponData AttachedWeapon;

	ExtraFireData() :
		AttachedFLH { }
		, AttachedWeapon { }
	{ }

	~ExtraFireData() = default;

	//confuse ? , yeah me too :kekw:
	void ReadArt(INI_EX& parserArt, const char* pSection_Art);
	void ReadRules(INI_EX& parserRules,const char* pSection_rules);

	template <typename T>
	void Serialize(T& Stm)
	{
		Debug::Log("Loading Element From ExtraFireData ! \n");
		AttachedFLH.Serialize(Stm);
		AttachedWeapon.Serialize(Stm);
	}

};
#endif