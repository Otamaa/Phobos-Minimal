#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>

#include <Ext/AbstractType/Body.h>

#include <Utilities/PhobosFixedString.h>
#include <Utilities/PhobosPCXFile.h>

class TechnoTypeClass;
class InfantryTypeClass;
class AircraftTypeClass;
class UnitTypeClass;
class AnimTypeClass;
class BuildingTypeClass;
struct SHPStruct;
class HouseTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = HouseTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "HouseTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "HouseTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	bool SettingsInherited;
	Valueable<int> SurvivorDivisor;
	Valueable<InfantryTypeClass*> Crew;
	Valueable<InfantryTypeClass*> Engineer;
	Valueable<InfantryTypeClass*> Technician;
	Valueable<AircraftTypeClass*> ParaDropPlane;
	Valueable<AircraftTypeClass*> SpyPlane;
	Valueable<UnitTypeClass*> HunterSeeker;
	ValueableVector<TechnoTypeClass*> ParaDropTypes;
	ValueableVector<int> ParaDropNum;
	Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith;
	Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage;
	Nullable<double> NewTeamsSelector_GroundCategoryPercentage;
	Nullable<double> NewTeamsSelector_NavalCategoryPercentage;
	Nullable<double> NewTeamsSelector_AirCategoryPercentage;
	Valueable<bool> GivesBounty;
	Nullable<bool> CanBeDriven;
	Valueable<AnimTypeClass*> ParachuteAnim;
	Valueable<bool> StartInMultiplayer_WithConst;
	ValueableVector<BuildingTypeClass*> Powerplants;
	ValueableVector<BuildingTypeClass*> VeteranBuildings;
	ValueableVector<std::string> TauntFile; //Taunt filename format (should contain %d !!!)
	Valueable<std::string> TauntFileName;
	Nullable<bool> Degrades;
	Valueable<InfantryTypeClass*> Disguise;
	NullableVector<TechnoTypeClass*> StartInMultiplayer_Types;
	PhobosFixedString<0x20> LoadScreenBackground;
	PhobosFixedString<0x20> LoadScreenPalette;
	ValueableIdx<ColorScheme> LoadTextColor; //The text color used for non-Campaign modes
	Valueable<unsigned int> RandomSelectionWeight;
	Valueable<CSFText> LoadScreenName;
	Valueable<CSFText> LoadScreenSpecialName;
	Valueable<CSFText> LoadScreenBrief;
	Valueable<CSFText> StatusText;
	PhobosPCXFile FlagFile; //Flag
	PhobosPCXFile ObserverFlag;
	SHPStruct* ObserverFlagSHP;
	Valueable<bool> ObserverFlagYuriPAL;
	PhobosPCXFile ObserverBackground;
	SHPStruct* ObserverBackgroundSHP;
	Valueable<bool> BattlePoints;
	Valueable<bool> BattlePoints_CanUseStandardPoints;
#pragma endregion

public:
	HouseTypeExtData(HouseTypeClass* pObj) :
		AbstractTypeExtData(pObj),

		SettingsInherited(false),

		SurvivorDivisor(-1),
		Crew(nullptr),
		Engineer(nullptr),
		Technician(nullptr),

		ParaDropPlane(nullptr),
		SpyPlane(nullptr),
		HunterSeeker(nullptr),

		ParaDropTypes(),
		ParaDropNum(),

		NewTeamsSelector_MergeUnclassifiedCategoryWith(),
		NewTeamsSelector_UnclassifiedCategoryPercentage(),
		NewTeamsSelector_GroundCategoryPercentage(),
		NewTeamsSelector_NavalCategoryPercentage(),
		NewTeamsSelector_AirCategoryPercentage(),

		GivesBounty(true),
		CanBeDriven(),
		ParachuteAnim(nullptr),

		StartInMultiplayer_WithConst(false),
		Powerplants(),
		VeteranBuildings(),

		TauntFile(),
		TauntFileName(""),

		Degrades(),
		Disguise(nullptr),
		StartInMultiplayer_Types(),

		LoadScreenBackground(""),
		LoadScreenPalette(""),

		LoadTextColor(-1),
		RandomSelectionWeight(1),

		LoadScreenName(),
		LoadScreenSpecialName(),
		LoadScreenBrief(),
		StatusText(),

		FlagFile(),
		ObserverFlag(),
		ObserverFlagSHP(nullptr),
		ObserverFlagYuriPAL(false),

		ObserverBackground(),
		ObserverBackgroundSHP(nullptr),

		BattlePoints(false),
		BattlePoints_CanUseStandardPoints(false)
	{
		this->AbsType = HouseTypeClass::AbsID;
		this->Initialize();
	}

	HouseTypeExtData(HouseTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~HouseTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<HouseTypeExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<HouseTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	HouseTypeClass* This() const { return reinterpret_cast<HouseTypeClass*>(this->AttachedToObject); }
	const HouseTypeClass* This_Const() const { return reinterpret_cast<const HouseTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

public:

	void LoadFromRulesFile(CCINIClass* pINI);
	void InheritSettings(HouseTypeClass* pThis);
	void Initialize();

	Iterator<BuildingTypeClass*> GetPowerplants() const;
	Iterator<BuildingTypeClass*> GetDefaultPowerplants() const;

public:

	static int PickRandomCountry();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class HouseTypeExtContainer final : public Container<HouseTypeExtData>
	, public ReadWriteContainerInterfaces<HouseTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "HouseTypeExtContainer";

public:
	static HouseTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(HouseTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(HouseTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeHouseTypeClass : public HouseTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	HouseTypeExtData* _GetExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeHouseTypeClass) == sizeof(HouseTypeClass), "Invalid Size !");