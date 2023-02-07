#pragma once
#include <IsometricTileTypeClass.h>
#include <IsometricTileClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <ScenarioClass.h>
#include <set>

class IsometricTileTypeExt
{
public:
	static constexpr DWORD Canary = 0x91577125;
	using base_type = IsometricTileTypeClass;

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		Valueable<int> Tileset;
		Valueable<bool> BlockJumpjet;
		ExtData(IsometricTileTypeClass* OwnerObject) : Extension<IsometricTileTypeClass>(OwnerObject)
			, Tileset { -1 }
			, BlockJumpjet { false }
		{ }

		virtual ~ExtData() override  = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static int CurrentTileset;
#ifdef IsoTilePalette

	static std::map<std::string, int> PalettesInitHelper;
	static std::map<int, int> LoadedPalettesLookUp;
	static std::vector<std::map<TintStruct, LightConvertClass*>> LoadedPalettes;
	static std::vector<UniqueGamePtr<BytePalette>> CustomPalettes;

	static LightConvertClass* IsometricTileTypeExt::InitDrawer(int nLookUpIdx, int red, int green, int blue);
	static void LoadPaletteFromName(int nTileset,const std::string_view PaletteName);
#endif
	class ExtContainer final : public Container<IsometricTileTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};