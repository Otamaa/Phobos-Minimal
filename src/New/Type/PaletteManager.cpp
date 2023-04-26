#include "PaletteManager.h"

#include <Utilities/GeneralUtils.h>

Enumerable<PaletteManager>::container_t Enumerable<PaletteManager>::Array;

PaletteManager::PaletteManager(const char* const pTitle) : Enumerable<PaletteManager>(pTitle)
, Convert_Temperate {}
, Convert {}
, Palette {}
{
	this->LoadFromName(GeneralUtils::ApplyTheaterSuffixToString(pTitle).c_str());
}

void PaletteManager::Clear_Internal()
{
	Palette.reset();
	Convert_Temperate.reset();
	Convert.reset();
}

void PaletteManager::CreateConvert()
{
	if (!this->Palette)
		Debug::FatalErrorAndExit("[%s] Missing Palette Data ! \n" , this->Name.data());

	this->Convert_Temperate.reset(GameCreate<ConvertClass>(this->Palette.get(), &FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 53, false));
	this->Convert.reset(GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate(), 1, false));
}

void PaletteManager::LoadFromName(const char* PaletteName)
{
	this->Clear_Internal();

	if (auto pPal = FileSystem::AllocatePalette(PaletteName))
	{
		this->Palette.reset(pPal);
		this->CreateConvert();
	}
}

void PaletteManager::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Clear_Internal();

	bool hasPalette = false;
	if (Stm.Load(hasPalette) && hasPalette) {
		this->Palette.reset(GameCreate<BytePalette>());
		if (Stm.Load(*this->Palette)) {
			this->CreateConvert();
		}
	}
}

void PaletteManager::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm.Save(this->Palette != nullptr);
	if (this->Palette) {
		Stm.Save(*this->Palette);
	}
}