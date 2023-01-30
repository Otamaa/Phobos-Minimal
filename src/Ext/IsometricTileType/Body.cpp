#include "Body.h"

#include <ScenarioClass.h>
//#include <Ext/Convert/Body.h>


IsometricTileTypeExt::ExtContainer IsometricTileTypeExt::ExtMap;
int IsometricTileTypeExt::CurrentTileset = -1;

#ifdef IsoTilePalette
std::map<std::string, int> IsometricTileTypeExt::PalettesInitHelper;
std::map<int, int> IsometricTileTypeExt::LoadedPalettesLookUp;
std::vector<std::map<TintStruct, LightConvertClass*>> IsometricTileTypeExt::LoadedPalettes;
std::vector<UniqueGamePtr<BytePalette>> IsometricTileTypeExt::CustomPalettes;

LightConvertClass* IsometricTileTypeExt::InitDrawer(int nLookUpIdx, int red, int green, int blue)
{
	if (!DSurface::Primary())
		return nullptr;

	if (!IsometricTileTypeExt::CustomPalettes[nLookUpIdx].get())
		Debug::FatalErrorAndExit("Failed to Receive BytePalette for [%d] ! \n", nLookUpIdx);

	//vector of map
	auto& drawers = IsometricTileTypeExt::LoadedPalettes[nLookUpIdx];

	auto createDrawer = [&]() -> LightConvertClass*
	{
		const int nShadeCount = red + blue + green < 2000 ? 27: 53;

		auto pDrawer = GameCreate<LightConvertClass>(
			IsometricTileTypeExt::CustomPalettes[nLookUpIdx].get(), &FileSystem::TEMPERAT_PAL(),
			DSurface::Primary(), red, green, blue, drawers.size() != 0, nullptr, nShadeCount
			);

		LightConvertClass::Array->AddItem(pDrawer);
		return pDrawer;
	};

	ScenarioClass::Instance->ScenarioLighting(&red, &green, &blue);
	TintStruct finder = { red,green,blue };

	if (drawers.size() == 0)
		return drawers[finder] = createDrawer();

	auto itr = drawers.find(finder);
	if (itr != drawers.end())
		return itr->second;
	else
		return drawers[finder] = createDrawer();
}

void IsometricTileTypeExt::LoadPaletteFromName(int nTileset, const std::string_view PaletteName)
{
	auto itr = PalettesInitHelper.find(PaletteName.data());
	if (itr != PalettesInitHelper.end())
	{
		// This palette had been created already, so we just set this map
		LoadedPalettesLookUp[nTileset] = itr->second;
		return;
	}

	// We need to load this palette
	PalettesInitHelper[PaletteName.data()] = static_cast<int>(IsometricTileTypeExt::LoadedPalettes.size());
	LoadedPalettesLookUp[nTileset] = static_cast<int>(IsometricTileTypeExt::LoadedPalettes.size());
	std::map<TintStruct, LightConvertClass*> buffer;
	IsometricTileTypeExt::LoadedPalettes.push_back(buffer);
	IsometricTileTypeExt::CustomPalettes.push_back(nullptr);
	IsometricTileTypeExt::CustomPalettes.back().reset(FileSystem::AllocatePalette(PaletteName.data()));
}
#endif
// =============================
// load / save

void IsometricTileTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	this->Tileset = IsometricTileTypeExt::CurrentTileset;

	char pSection[16];
	sprintf(pSection, "TileSet%04d", IsometricTileTypeExt::CurrentTileset);

	if (pINI->GetSection(pSection))
	{
#ifdef IsoTilePalette
		auto const theater = ScenarioClass::Instance->Theater;
		auto const pExtension = Theater::GetTheater(theater).Extension;
		char pDefault[] = "iso~~~.pal";
		pDefault[3] = pExtension[0];
		pDefault[4] = pExtension[1];
		pDefault[5] = pExtension[2];
		pINI->ReadString(pSection, "CustomPalette", pDefault, Phobos::readBuffer);
		IsometricTileTypeExt::LoadPaletteFromName(this->Tileset, Phobos::readBuffer);
#endif
		INI_EX exINI(pINI);
		this->BlockJumpjet.Read(exINI, pSection, "BlockJumjet");
	}
}

template <typename T>
void IsometricTileTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Tileset)
	    .Process(this->BlockJumpjet)
		;
}

void IsometricTileTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<IsometricTileTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void IsometricTileTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<IsometricTileTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool IsometricTileTypeExt::LoadGlobals(PhobosStreamReader& Stm)
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

bool IsometricTileTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
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
// =============================
// container

IsometricTileTypeExt::ExtContainer::ExtContainer() : TExtensionContainer("IsometricTileTypeClass") { }
IsometricTileTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x5449F2, IsometricTileTypeClass_CTOR, 0x5)
//{
//	GET(IsometricTileTypeClass*, pItem, EBP);
//
//	IsometricTileTypeExt::ExtMap.JustAllocate(pItem , pItem , "Invalid !");
//
//	return 0;
//}
//
//DEFINE_HOOK(0x544BC2, IsometricTileTypeClass_DTOR, 0x8)
//{
//	GET(IsometricTileTypeClass*, pItem, ESI);
//
//	IsometricTileTypeExt::ExtMap.Remove(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x549D70, IsometricTileTypeClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x549C80, IsometricTileTypeClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(IsometricTileTypeClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	IsometricTileTypeExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x549D5D, IsometricTileTypeClass_Load_Suffix, 0x5)
//{
//	IsometricTileTypeExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x549D8A, IsometricTileTypeClass_Save_Suffix, 0x6)
//{
//	IsometricTileTypeExt::ExtMap.SaveStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x54642E, IsometricTileTypeClass_LoadFromINI, 0x6)
//{
//	GET(IsometricTileTypeClass*, pItem, EBP);
//	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xA10, 0x9D8));
//
//	IsometricTileTypeExt::ExtMap.LoadFromINI(pItem, pINI);
//	return 0;
//}
//
//DEFINE_HOOK(0x545FA3, IsometricTileTypeClass_LoadFromINI_SetTileSet, 0x8)
//{
//	IsometricTileTypeExt::CurrentTileset = R->EDI();
//
//	return 0;
//}