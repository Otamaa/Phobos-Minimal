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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

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
	HouseTypeExtData(HouseTypeClass* pObj) : AbstractTypeExtData(pObj),
		SettingsInherited(false),
		SurvivorDivisor(-1),
		Crew(nullptr),
		Engineer(nullptr),
		Technician(nullptr),
		ParaDropPlane(nullptr),
		SpyPlane(nullptr),
		HunterSeeker(nullptr),
		GivesBounty(true),
		ParachuteAnim(nullptr),
		StartInMultiplayer_WithConst(false),
		LoadTextColor(-1),
		RandomSelectionWeight(1),
		ObserverFlagSHP(nullptr),
		ObserverFlagYuriPAL(false),
		ObserverBackgroundSHP(nullptr),
		BattlePoints(false),
		BattlePoints_CanUseStandardPoints(false)
	{
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

	virtual HouseTypeClass* This() const override { return reinterpret_cast<HouseTypeClass*>(this->AbstractTypeExtData::This()); }
	virtual const HouseTypeClass* This_Const() const override { return reinterpret_cast<const HouseTypeClass*>(this->AbstractTypeExtData::This_Const()); }

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
{
public:
	static HouseTypeExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

};

class NOVTABLE FakeHouseTypeClass : public HouseTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);
	bool _ReadFromINI(CCINIClass* pINI);

	HouseTypeExtData* _GetExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeHouseTypeClass) == sizeof(HouseTypeClass), "Invalid Size !");