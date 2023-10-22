#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExtData final
{
public:
	using base_type = HouseTypeClass;
	static constexpr DWORD Canary = 0x1111111A;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
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

	NullableVector<TechnoTypeClass*> StartInMultiplayer_Types {};

	PhobosFixedString<0x20> LoadScreenBackground {};
	PhobosFixedString<0x20> LoadScreenPalette {};

	ValueableIdx<ColorScheme> LoadTextColor { -1 }; //The text color used for non-Campaign modes
	Valueable<unsigned int> RandomSelectionWeight { 1 };

	Valueable<CSFText> LoadScreenName {};
	Valueable<CSFText> LoadScreenSpecialName {};
	Valueable<CSFText> LoadScreenBrief {};

	Valueable<CSFText> StatusText {};

	PhobosPCXFile FlagFile {}; //Flag
	PhobosPCXFile ObserverFlag {};
	SHPStruct* ObserverFlagSHP {};
	Valueable<bool> ObserverFlagYuriPAL { false };

	PhobosPCXFile ObserverBackground {};
	SHPStruct* ObserverBackgroundSHP {};

	HouseTypeExtData() noexcept = default;
	~HouseTypeExtData() noexcept = default;

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromRulesFile(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InheritSettings(HouseTypeClass* pThis);

	void Initialize();

	Iterator<BuildingTypeClass*> GetPowerplants() const;
	Iterator<BuildingTypeClass*> GetDefaultPowerplants() const;

	static int PickRandomCountry();
private:
	template <typename T>
	void Serialize(T& Stm);
};

class HouseTypeExtContainer final : public Container<HouseTypeExtData>
{
public:
	static HouseTypeExtContainer Instance;

	CONSTEXPR_NOCOPY_CLASSB(HouseTypeExtContainer, HouseTypeExtData, "HouseTypeClass");

};