#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExt
{
public:
	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		using base_type = HouseTypeClass;
		static constexpr DWORD Canary = 0x1111111A;

	public:
		bool SettingsInherited { false };

		Nullable<int> SurvivorDivisor { };
		Nullable<InfantryTypeClass*> Crew { };
		Nullable<InfantryTypeClass*> Engineer { };
		Nullable<InfantryTypeClass*> Technician { };
		Nullable<AircraftTypeClass*> ParaDropPlane { };
		Nullable<AircraftTypeClass*> SpyPlane { };
		Nullable<UnitTypeClass*> HunterSeeker { };

		ValueableVector<TechnoTypeClass*> ParaDropTypes { };
		ValueableVector<int> ParaDropNum { };

		Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith { };
		Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage { };
		Nullable<double> NewTeamsSelector_GroundCategoryPercentage { };
		Nullable<double> NewTeamsSelector_NavalCategoryPercentage { };
		Nullable<double> NewTeamsSelector_AirCategoryPercentage { };

		Valueable<bool> GivesBounty { true };
		Nullable<bool> CanBeDriven {};

		Nullable<AnimTypeClass*> ParachuteAnim {};
		Valueable<bool> StartInMultiplayer_WithConst { false };
		ValueableVector<BuildingTypeClass*> Powerplants {};

		ValueableVector<BuildingTypeClass*> VeteranBuildings {};
		PhobosFixedString<0x20> TauntFile {}; //Taunt filename format (should contain %d !!!)

		Nullable<bool> Degrades {};
		Nullable<InfantryTypeClass*> Disguise {};

		ValueableVector<TechnoTypeClass*> StartInMultiplayer_Types {};

		PhobosFixedString<0x20> LoadScreenBackground {};
		PhobosFixedString<0x20> LoadScreenPalette {};

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void InheritSettings(HouseTypeClass* pThis);

		void Initialize();

		Iterator<BuildingTypeClass*> GetPowerplants() const;
		Iterator<BuildingTypeClass*> GetDefaultPowerplants() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt::ExtData>
	{
	public:
		CONSTEXPR_NOCOPY_CLASS(HouseTypeExt::ExtData, "HouseTypeClass");

	public:
		virtual bool Load(HouseTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
};