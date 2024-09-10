#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class HouseTypeExtData final
{
public:
	using base_type = HouseTypeClass;
	static constexpr size_t Canary = 0x1111111A;

#ifndef aaa
	static constexpr size_t ExtOffset = 0xC4;//ARES
#endif

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	bool SettingsInherited { false };

	Valueable<int> SurvivorDivisor { -1 };
	Valueable<InfantryTypeClass*> Crew { nullptr };
	Valueable<InfantryTypeClass*> Engineer { nullptr };
	Valueable<InfantryTypeClass*> Technician { nullptr };
	Valueable<AircraftTypeClass*> ParaDropPlane { nullptr };
	Valueable<AircraftTypeClass*> SpyPlane { nullptr };
	Valueable<UnitTypeClass*> HunterSeeker { nullptr };

	ValueableVector<TechnoTypeClass*> ParaDropTypes { };
	ValueableVector<int> ParaDropNum { };

	Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith { };
	Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage { };
	Nullable<double> NewTeamsSelector_GroundCategoryPercentage { };
	Nullable<double> NewTeamsSelector_NavalCategoryPercentage { };
	Nullable<double> NewTeamsSelector_AirCategoryPercentage { };

	Valueable<bool> GivesBounty { true };
	Nullable<bool> CanBeDriven {};

	Valueable<AnimTypeClass*> ParachuteAnim { nullptr };
	Valueable<bool> StartInMultiplayer_WithConst { false };
	ValueableVector<BuildingTypeClass*> Powerplants {};

	ValueableVector<BuildingTypeClass*> VeteranBuildings {};
	ValueableVector<std::string> TauntFile {}; //Taunt filename format (should contain %d !!!)
	Valueable<std::string> TauntFileName {};

	Nullable<bool> Degrades {};
	Valueable<InfantryTypeClass*> Disguise {};

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

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromRulesFile(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InheritSettings(HouseTypeClass* pThis);

	void Initialize();

	Iterator<BuildingTypeClass*> GetPowerplants() const;
	Iterator<BuildingTypeClass*> GetDefaultPowerplants() const;

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(HouseTypeExtData) -
			(4u //AttachedToObject
			 );
	}

	static int PickRandomCountry();
private:
	template <typename T>
	void Serialize(T& Stm);
};

class HouseTypeExtContainer final : public Container<HouseTypeExtData>
{
public:
	static HouseTypeExtContainer Instance;
	PhobosMap<HouseTypeClass*, HouseTypeExtData*> Map;

	virtual bool Load(HouseTypeClass* key, IStream* pStm);

	void Clear()
	{
		this->Map.clear();
	}

	HouseTypeExtContainer() : Container<HouseTypeExtData> { "HouseTypeClass" }
		, Map {}
	{ }

	virtual ~HouseTypeExtContainer() override = default;

private:
	HouseTypeExtContainer(const HouseTypeExtContainer&) = delete;
	HouseTypeExtContainer(HouseTypeExtContainer&&) = delete;
	HouseTypeExtContainer& operator=(const HouseTypeExtContainer& other) = delete;

};