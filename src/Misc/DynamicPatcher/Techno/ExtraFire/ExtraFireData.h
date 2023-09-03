#pragma once
#include <CoordStruct.h>
#include <Utilities/TemplateDef.h>

class WeaponTypeClass;

struct ExtraFireData
{
	struct FLHData
	{
		Valueable<CoordStruct> PrimaryWeaponFLH { {0,0,0} };
		Nullable<CoordStruct> ElitePrimaryWeaponFLH { };
		Valueable<CoordStruct> SecondaryWeaponFLH { {0,0,0} };
		Nullable<CoordStruct> EliteSecondaryWeaponFLH { };

		ValueableVector<CoordStruct> WeaponXFLH { };
		NullableVector<CoordStruct> EliteWeaponXFLH { };

		FLHData* AsPointer() const{
			return const_cast<FLHData*>(this);
		}

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
	};

	struct WeaponData
	{
		ValueableVector<WeaponTypeClass*> PrimaryWeapons { };
		ValueableVector<WeaponTypeClass*> SecondaryWeapons { };
		NullableVector<WeaponTypeClass*> ElitePrimaryWeapons { };
		NullableVector<WeaponTypeClass*> EliteSecondaryWeapons { };

		ValueableVector<std::vector<WeaponTypeClass*>> WeaponX { };
		ValueableVector<std::vector<WeaponTypeClass*>> EliteWeaponX { };

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
	};

	FLHData AttachedFLH { };
	WeaponData AttachedWeapon { };

	//confuse ? , yeah me too :kekw:
	void ReadArt(INI_EX& parserArt, const char* pSection_Art);
	void ReadRules(INI_EX& parserRules,const char* pSection_rules);

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::Log("Loading Element From ExtraFireData ! \n");
		AttachedFLH.Serialize(Stm);
		AttachedWeapon.Serialize(Stm);
	}

};
