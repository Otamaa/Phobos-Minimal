#pragma once
#include <IsometricTileTypeClass.h>
#include <IsometricTileClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <ScenarioClass.h>
#include <set>

class IsometricTileTypeExtData final
{
public:

	static COMPILETIMEEVAL size_t Canary = 0x91577125;
	using base_type = IsometricTileTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };

public :
	Valueable<int> Tileset { -1 };
	Valueable<PaletteManager*> Palette {};


	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(IsometricTileTypeExtData) -
			(4u //AttachedToObject
			 );
	}
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

//	IsometricTileTypeExtContainer() : Container<IsometricTileTypeExtData> { "IsometricTileTypeClass" }
//	{ }
//
//	virtual ~IsometricTileTypeExtContainer() override = default;
//
//private:
//	IsometricTileTypeExtContainer(const IsometricTileTypeExtContainer&) = delete;
//	IsometricTileTypeExtContainer(IsometricTileTypeExtContainer&&) = delete;
//	IsometricTileTypeExtContainer& operator=(const IsometricTileTypeExtContainer& other) = delete;
};