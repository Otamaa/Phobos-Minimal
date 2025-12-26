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
	std::string TileSetName;
	Valueable<int> Tileset;
	CustomPalette Palette;
	Valueable<bool> AllowVeins;
	ValueableVector<TiberiumClass*> AllowedTiberiums;
	ValueableVector<SmudgeTypeClass*> AllowedSmudges;
#pragma endregion

public:
	IsometricTileTypeExtData(IsometricTileTypeClass* pObj) : ObjectTypeExtData(pObj)
		, TileSetName("")
		, Tileset(-1)
		, Palette()
		, AllowVeins()
		, AllowedTiberiums()
		, AllowedSmudges()
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