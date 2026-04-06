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

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates (strings, fixed-size buffers)
	// ============================================================
	PhobosFixedString<0x20> LoadScreenBackground { "" };
	PhobosFixedString<0x20> LoadScreenPalette { "" };
	Valueable<std::string> TauntFileName { "" };
	Valueable<CSFText> LoadScreenName {};
	Valueable<CSFText> LoadScreenSpecialName {};
	Valueable<CSFText> LoadScreenBrief {};
	Valueable<CSFText> StatusText {};
	PhobosPCXFile FlagFile {};
	PhobosPCXFile ObserverFlag {};
	PhobosPCXFile ObserverBackground {};

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableVector<TechnoTypeClass*> ParaDropTypes {};
	ValueableVector<int> ParaDropNum {};
	ValueableVector<BuildingTypeClass*> Powerplants {};
	ValueableVector<BuildingTypeClass*> VeteranBuildings {};
	ValueableVector<std::string> TauntFile {};
	NullableVector<TechnoTypeClass*> StartInMultiplayer_Types {};

	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	SHPStruct* ObserverFlagSHP { nullptr };
	SHPStruct* ObserverBackgroundSHP { nullptr };

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<InfantryTypeClass*> Crew { nullptr };
	Valueable<InfantryTypeClass*> Engineer { nullptr };
	Valueable<InfantryTypeClass*> Technician { nullptr };
	Valueable<AircraftTypeClass*> ParaDropPlane { nullptr };
	Valueable<AircraftTypeClass*> SpyPlane { nullptr };
	Valueable<UnitTypeClass*> HunterSeeker { nullptr };
	Valueable<AnimTypeClass*> ParachuteAnim { nullptr };
	Valueable<InfantryTypeClass*> Disguise { nullptr };

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> NewTeamsSelector_UnclassifiedCategoryPercentage {};
	Nullable<double> NewTeamsSelector_GroundCategoryPercentage {};
	Nullable<double> NewTeamsSelector_NavalCategoryPercentage {};
	Nullable<double> NewTeamsSelector_AirCategoryPercentage {};

	// ============================================================
	// Nullable<int> (int + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes)
	// ============================================================
	Nullable<bool> CanBeDriven {};
	Nullable<bool> Degrades {};

	// ============================================================
	// Valueable<int> / ValueableIdx (4 bytes each)
	// ============================================================
	Valueable<int> SurvivorDivisor { -1 };
	ValueableIdx<ColorScheme> LoadTextColor { -1 };
	Valueable<unsigned int> RandomSelectionWeight { 1 };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> GivesBounty { true };
	Valueable<bool> StartInMultiplayer_WithConst { false };
	Valueable<bool> ObserverFlagYuriPAL { false };
	Valueable<bool> BattlePoints { false };
	Valueable<bool> BattlePoints_CanUseStandardPoints { false };

	// ============================================================
	// Plain bool (1 byte, at the very end)
	// ============================================================
	bool SettingsInherited { false };
	// 6 bools = 6 bytes, pads to 8 for alignment
#pragma endregion

public:

	HouseTypeExtData(HouseTypeClass* pObj) : AbstractTypeExtData(pObj)	
	{
		this->AbsType = HouseTypeClass::AbsID;
		this->InitiliazeConstants();
	}

	HouseTypeExtData(HouseTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~HouseTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType type) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved, type);
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
	void InitiliazeConstants();

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
	
	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

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