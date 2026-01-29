#pragma once
#include <IsometricTileTypeClass.h>
#include <IsometricTileClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/PaletteManager.h>

#include <ScenarioClass.h>
#include <set>

#include <Ext/ObjectType/Body.h>

class IsometricTileTypeExtData final : public ObjectTypeExtData
{
public:

	using base_type = IsometricTileTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "IsometricTileTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "IsometricTileTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public :

#pragma region ClassMembers
	// ============================================================
	// Large aggregates
	// ============================================================
	std::string TileSetName;
	CustomPalette Palette;

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableVector<TiberiumClass*> AllowedTiberiums;
	ValueableVector<SmudgeTypeClass*> AllowedSmudges;

	// ============================================================
	// 4-byte aligned: Valueable<int>
	// ============================================================
	Valueable<int> Tileset;

	// ============================================================
	// 1-byte aligned: Valueable<bool> (at the end)
	// ============================================================
	Valueable<bool> AllowVeins;
	// 1 byte, pads to 4 for alignment

#pragma endregion

public:
	IsometricTileTypeExtData(IsometricTileTypeClass* pObj) : ObjectTypeExtData(pObj)
		// Large aggregates
		, TileSetName("")
		, Palette()
		// Vectors
		, AllowedTiberiums()
		, AllowedSmudges()
		// Valueable<int>
		, Tileset(-1)
		// Valueable<bool>
		, AllowVeins()
	{
		this->AbsType = IsometricTileTypeClass::AbsID;
	}

	IsometricTileTypeExtData(IsometricTileTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~IsometricTileTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<IsometricTileTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<IsometricTileTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	IsometricTileTypeClass* This() const { return reinterpret_cast<IsometricTileTypeClass*>(this->AttachedToObject); }
	const IsometricTileTypeClass* This_Const() const { return reinterpret_cast<const IsometricTileTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

public:

	static LightConvertClass* GetLightConvert(IsometricTileTypeClass* pOvrl , int r, int g, int b);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class IsometricTileTypeExtContainer final : public Container<IsometricTileTypeExtData>
	, public ReadWriteContainerInterfaces<IsometricTileTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "IsometricTileTypeExtContainer";
	using base_t = Container<IsometricTileTypeExtData>;

public:
	static IsometricTileTypeExtContainer Instance;
	std::map<std::string, std::map<TintStruct, LightConvertClass*>> LightConvertEntities;
	int CurrentTileset;

	virtual bool LoadAll(const json& root) { return true; }
	virtual bool SaveAll(json& root) { return true; }
	virtual void Clear() { 
		this->base_t::Clear();
		this->LightConvertEntities.clear();
		this->CurrentTileset = -1;
	}

	virtual void LoadFromINI(IsometricTileTypeClass* key, CCINIClass* pINI, bool parseFailAddr) { }
	virtual void WriteToINI(IsometricTileTypeClass* key, CCINIClass* pINI) { }
};

class FakeIsometricTileTypeClass : public IsometricTileTypeClass
{
public:

};