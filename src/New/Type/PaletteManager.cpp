#include "PaletteManager.h"

#include <Utilities/GeneralUtils.h>

Enumerable<PaletteManager>::container_t Enumerable<PaletteManager>::Array;

PaletteManager::PaletteManager(const char* const pTitle) : Enumerable<PaletteManager>(pTitle)
, Convert_Temperate {}
, Convert {}
, Palette {}
{
	this->CachedName = GeneralUtils::ApplyTheaterSuffixToString(pTitle).c_str();
	this->LoadFromCachedName();
}

void PaletteManager::Clear_Internal()
{
	Palette.release();
	Convert_Temperate.release();
	Convert.release();
}

void PaletteManager::CreateConvert()
{
	if (!this->Palette) {
		Debug::Log("[%s] Missing Palette Data ! \n" , this->Name.data());
		return;
	}

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
	else
	{
		Debug::Log("[%s] Palette FailedToLoad ! \n", this->Name.data());
		return;
	}
}

bool PaletteManager::LoadFromCachedName()
{
	this->Clear_Internal();

	if (auto pPal = FileSystem::AllocatePalette(this->CachedName.data()))
	{
		this->Palette.reset(pPal);
		this->CreateConvert();
		return true;
	}

	//load pal direcly from game mixes if not found
	if (auto pPal = (BytePalette*)MixFileClass::Retrieve(this->CachedName.data(), false))
	{
		for (auto& color : pPal->Entries)
		{
			color.R <<= 2;
			color.G <<= 2;
			color.B <<= 2;
		}

		this->Palette.reset(pPal);
		this->CreateConvert();
		return true;
	}

	Debug::Log("[%s] - [%s] Palette  FailedToLoad ! \n", this->Name.data(), this->CachedName.data());
	return false;
}

void PaletteManager::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Clear_Internal();
	this->CachedName =
		GeneralUtils::ApplyTheaterSuffixToString((const char*)this->Name.data()).c_str();

	bool hasPalette = false;
	auto ret = Stm.Load(hasPalette);

	if (ret && hasPalette)
	{
		this->Palette.reset(GameCreate<BytePalette>());
		ret = Stm.Load(*this->Palette);

		if (ret)
		{
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