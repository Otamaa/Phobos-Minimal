#include "TheaterTypeClass.h"

#include <Phobos.h>

#include <Ext/Scenario/Body.h>

#include <MixFileClass.h>

#include <Utilities/Macro.h>

Enumerable<TheaterTypeClass>::container_t Enumerable<TheaterTypeClass>::Array;

const char* Enumerable<TheaterTypeClass>::GetMainSection()
{
	return "TheaterTypes";
}

void TheaterTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.data();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exIni(pINI);

	UIName.Read(exIni, pSection, "UIName");
	ControlFileName.Read(pINI, pSection, "ControlFileName");
	ArtFileName.Read(pINI, pSection, "ArtFileName");
	PaletteFileName.Read(pINI, pSection, "PaletteFileName");
	Extension.Read(pINI, pSection, "Extension");
	MMExtension.Read(pINI, pSection, "MMExtension");
	Letter.Read(pINI, pSection, "ImageLetter");
	IsArctic.Read(exIni, pSection, "IsArtic");
	IsLunar.Read(exIni, pSection, "IsLunar");
	IsAllowedInMapGenerator.Read(exIni, pSection, "IsAllowedInMapGenerator");
	LowRadarBrightness1.Read(exIni, pSection, "LowRadarBrightness");

	PaletteUnit.Read(pINI, pSection, "PaletteUnit");
	PaletteISO.Read(pINI, pSection, "IsometricPalette");
	TerrainControl.Read(pINI, pSection, "TerrainControl");
	PaletteOverlay.Read(pINI, pSection, "PaletteOverlay");

	RootMix.Read(pINI, pSection, "ControlMix");
	RootMixMD.Read(pINI, pSection, "ControlMDMix");
	ExpansionMDMix.Read(pINI, pSection, "ExpansionMDMix");
	SuffixMix.Read(pINI, pSection, "ExtensionMix");
	DataMix.Read(pINI, pSection, "ArtMix");
	TerrainTypeExtension.Read(pINI, pSection, "TerrainTypeExtension");
	SmudgeTypeExtension.Read(pINI, pSection, "SmudgeTypeExtension");
	AnimTypeExtension.Read(pINI, pSection, "AnimTypeExtension");
	OverlayTypeExtension.Read(pINI, pSection, "OverlayTypeExtension");
	IsometricTileTypeExtension.Read(pINI, pSection, "IsometricTileTypeExtension");
	BuildingTypeExtension.Read(pINI, pSection, "BuildingTypeExtension");
	FallbackTheaterExtension.Read(pINI, pSection, "FallbackTheaterExtension");
}

bool TheaterTypeClass::IsDefaultTheater()
{
	for (size_t i = 0; i < 6; ++i)
	{
		if (IS_SAME_STR_(Name.data(), Theater::Array[i].Identifier))
			return true;
	}

	return false;
}

CCINIClass* TheaterTypeClass::GetConfigINI()
{


	return nullptr;
}

#include <Utilities/GameConfig.h>

void TheaterTypeClass::LoadAllTheatersToArray()
{
	GameConfig _conf { "Theaters.ini" };

	_conf.OpenINIAction([](CCINIClass* pINI) {
		const char* pSection = TheaterTypeClass::GetMainSection();

		if (pINI->GetSection(pSection)) {
			for (int i = 0; i < pINI->GetKeyCount(pSection); ++i) {
				if (pINI->ReadString(pSection,
				pINI->GetKeyName(pSection, i),
				Phobos::readDefval,
				Phobos::readBuffer) > 0
					)
				{
					if (auto pTheater = FindOrAllocate(Phobos::readBuffer))
						pTheater->LoadFromINI(pINI);
					else
						Debug::LogInfo("Error Reading {} \"{}\".", pSection, Phobos::readBuffer);
				}
			}
		}
	});
}

void TheaterTypeClass::AddDefaults()
{
	if (Array.empty()) {
		Array.reserve(Theater::Array.size());

		for (size_t i = 0; i < Theater::Array.size(); ++i) {
			AllocateWithDefault(
				Theater::Array[i].Identifier ,
				Theater::Array[i] ,
				i == (size_t)TheaterType::Snow ,
				!(i == (size_t)TheaterType::NewUrban || i == (size_t)TheaterType::Lunar),
				 i == (size_t)TheaterType::Lunar
			);
		}
	}
}