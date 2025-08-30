#pragma once

#include <OverlayTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosMap.h>
#include <New/Type/PaletteManager.h>

#include <Ext/ObjectType/Body.h>

class OverlayTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = OverlayTypeClass;

public:

#pragma region ClassMembeers

	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	Valueable<int> ZAdjust { 0 };

#pragma endregion

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(OverlayTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class OverlayTypeExtContainer final : public Container<OverlayTypeExtData>
{
public:
	static OverlayTypeExtContainer Instance;

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

	virtual bool WriteDataToTheByteStream(OverlayTypeExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(OverlayTypeExtData::base_type* key, IStream* pStm) { };
};

