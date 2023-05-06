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
	static int CurrentTileset;
#ifdef IsoTilePalette

	static std::map<std::string, int> PalettesInitHelper;
	static std::map<int, int> LoadedPalettesLookUp;
	static std::vector<std::map<TintStruct, LightConvertClass*>> LoadedPalettes;
	static std::vector<UniqueGamePtr<BytePalette>> CustomPalettes;

	static LightConvertClass* IsometricTileTypeExt::InitDrawer(int nLookUpIdx, int red, int green, int blue);
	static void LoadPaletteFromName(int nTileset, const std::string_view PaletteName);
#endif

	class ExtData final : public Extension<IsometricTileTypeClass>
	{
	public:
		static constexpr DWORD Canary = 0x91577125;
		using base_type = IsometricTileTypeClass;

	public:
		Valueable<int> Tileset;
		Valueable<bool> BlockJumpjet;
		ExtData(IsometricTileTypeClass* OwnerObject) : Extension<IsometricTileTypeClass>(OwnerObject)
			, Tileset { -1 }
			, BlockJumpjet { false }
		{ }

		virtual ~ExtData() override  = default;
		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); } 
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<IsometricTileTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		static bool LoadGlobals(PhobosStreamReader& Stm)
		{
			return Stm
				.Process(IsometricTileTypeExt::CurrentTileset)
#ifdef IsoTilePalette
				.Process(IsometricTileTypeExt::PalettesInitHelper)
				.Process(IsometricTileTypeExt::LoadedPalettesLookUp)
				.Process(IsometricTileTypeExt::LoadedPalettes)
				.Process(IsometricTileTypeExt::CustomPalettes)
#endif
				.Success();
		}

		static bool SaveGlobals(PhobosStreamWriter& Stm)
		{
			return Stm

				.Process(IsometricTileTypeExt::CurrentTileset)
#ifdef IsoTilePalette
				.Process(IsometricTileTypeExt::PalettesInitHelper)
				.Process(IsometricTileTypeExt::LoadedPalettesLookUp)
				.Process(IsometricTileTypeExt::LoadedPalettes)
				.Process(IsometricTileTypeExt::CustomPalettes)
#endif
				.Success();
		}
	};

	static ExtContainer ExtMap;
};