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
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

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
	IsometricTileTypeExtData(IsometricTileTypeClass* pObj) : ObjectTypeExtData(pObj),
		Tileset(-1)
	{ }
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

	virtual IsometricTileTypeClass* This() const override { return reinterpret_cast<IsometricTileTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const IsometricTileTypeClass* This_Const() const override { return reinterpret_cast<const IsometricTileTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

public:

	static LightConvertClass* GetLightConvert(IsometricTileTypeClass* pOvrl , int r, int g, int b);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class IsometricTileTypeExtContainer final : public Container<IsometricTileTypeExtData>
{
public:
	static IsometricTileTypeExtContainer Instance;
	static std::map<std::string, std::map<TintStruct, LightConvertClass*>> LightConvertEntities;
	static int CurrentTileset;

	void Clear() {
		Array.clear();
		LightConvertEntities.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(CurrentTileset)
			.Success()
			;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(CurrentTileset)
			.Success()
			;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class FakeIsometricTileTypeClass : public IsometricTileTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);
};