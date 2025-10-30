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
		ValueableVector<CoordStruct> EliteWeaponXFLH { };

		FLHData* AsPointer() const{
			return const_cast<FLHData*>(this);
		}

		OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Stm
				.Process(PrimaryWeaponFLH)
				.Process(ElitePrimaryWeaponFLH)
				.Process(SecondaryWeaponFLH)
				.Process(EliteSecondaryWeaponFLH)
				.Process(WeaponXFLH)
				.Process(EliteWeaponXFLH);
		}

		OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
		{
			return Stm
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

		OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Stm
				.Process(PrimaryWeapons)
				.Process(SecondaryWeapons)
				.Process(ElitePrimaryWeapons)
				.Process(EliteSecondaryWeapons)
				.Process(WeaponX)
				.Process(EliteWeaponX)
				;
		}

		OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
		{
			return Stm
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

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
		return Stm
			.Process(AttachedFLH)
			.Process(AttachedWeapon)
			;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const {
		return Stm
			.Process(AttachedFLH)
			.Process(AttachedWeapon)
			;
	}

};
