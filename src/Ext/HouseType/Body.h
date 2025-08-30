#pragma once
#include <HouseTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/Container.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/TemplateDef.h>
#include <Ext/AbstractType/Body.h>

class HouseTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = HouseTypeClass;

public:

#pragma region ClassMembers

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
	Valueable<bool> BattlePoints {};
	Valueable<bool> BattlePoints_CanUseStandardPoints {};
#pragma endregion

	HouseTypeExtData(AircraftTypeClass* pObj) : AbstractTypeExtData(pObj) { this->Initialize(); }
	HouseTypeExtData(AircraftTypeClass* pObj, noinit_t& nn) : AbstractTypeExtData(pObj, nn) { }

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

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
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
	virtual bool WriteToINI(CCINIClass* pINI) const { }

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

	virtual bool WriteDataToTheByteStream(HouseTypeExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(HouseTypeExtData::base_type* key, IStream* pStm) { };

};

class NOVTABLE FakeHouseTypeClass : public HouseTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	HouseTypeExtData* _GetExtData() {
		return *reinterpret_cast<HouseTypeExtData**>(((DWORD)this) + 0x18);
	}
};
static_assert(sizeof(FakeHouseTypeClass) == sizeof(HouseTypeClass), "Invalid Size !");