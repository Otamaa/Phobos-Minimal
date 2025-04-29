#include "PaletteManager.h"

#include <Utilities/GeneralUtils.h>

#include <MixFileClass.h>

Enumerable<PaletteManager>::container_t Enumerable<PaletteManager>::Array;

PaletteManager::PaletteManager(const char* const pTitle) : Enumerable<PaletteManager>(pTitle)
, Convert_Temperate {}
, Convert {}
, Palette {}
, NoTemperate { false }
{
	this->CachedName = GeneralUtils::ApplyTheaterSuffixToString(pTitle).c_str();
	this->LoadFromCachedName();
}

void PaletteManager::Clear_Internal()
{
	this->Palette.release();
	if(this->Convert_Temperate && !this->NoTemperate){
		GameDelete(this->Convert_Temperate);
		this->Convert_Temperate = nullptr;
	}

	if (this->Convert) {
		GameDelete(this->Convert);
		this->Convert = nullptr;
	}

	this->ColorschemeDataVector = nullptr;
}

void PaletteManager::CreateConvert()
{
	if (!this->Palette) {
		Debug::LogInfo("[{}] Missing Palette Data ! " , this->Name.data());
		return;
	}

	if(!NoTemperate) {
		this->Convert_Temperate = (GameCreate<ConvertClass>(this->Palette.get(), &FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 53, false));
		this->Convert = (GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate(), 1, false));
	}
	else {
		this->Convert_Temperate = this->Convert = (GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate(), 1, false));
	}

	std::string realname = _strlwr(this->Name.data());

	// the function will handle the name change
	if(realname.find("~~~") != std::string::npos){
		realname.erase(realname.find("~~~"));

		if (realname.find(".pal") != std::string::npos)
			realname.erase(realname.find(".pal"));

		this->ColorschemeDataVector = (ColorScheme::GeneratePalette(realname.data()));
	} else { //dont need extension
		std::string cachedWithExt = _strlwr(this->CachedName.data());

		if (cachedWithExt.find(".pal") != std::string::npos)
			cachedWithExt.erase(cachedWithExt.find(".pal"));

		this->ColorschemeDataVector = (ColorScheme::GeneratePalette(cachedWithExt.data()));
	}
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
		Debug::LogInfo("[{}] Palette FailedToLoad ! ", this->Name.data());
		return;
	}
}

void FindOrAllocateDefaultConvers(const char* name  , bool noTemperate) {
	auto pUnitSno = PaletteManager::FindOrAllocate(name);
	pUnitSno->NoTemperate = noTemperate;

	if (pUnitSno->Convert_Temperate && !noTemperate)
	{
		GameDelete(pUnitSno->Convert_Temperate);
		pUnitSno->Convert_Temperate = nullptr;
	}

	if (pUnitSno->Convert)
	{
		GameDelete(pUnitSno->Convert);
		pUnitSno->Convert = nullptr;
	}

	pUnitSno->ColorschemeDataVector = nullptr;

	if (auto pPal = (BytePalette*)MixFileClass::Retrieve(pUnitSno->CachedName.data(), false)) {

		for (auto& color : pPal->Entries) {
			color.R <<= 2;
			color.G <<= 2;
			color.B <<= 2;
		}

		pUnitSno->Palette.reset(pPal);
		pUnitSno->CreateConvert();
	}

	Debug::Log("Allocating Pal [%s]\n", name);
}

struct DefaultPaletteData {
	const char* const PaletteName;
	bool NoTemperate;
};

constexpr DefaultPaletteData const DefaultPalettes[]
{
	{   "TEMPERAT.PAL"	, false },
	{	"UNITSNO.PAL"	, false },
	{	"WAYPOINT.PAL"	, true	},
	{	"ANIM.PAL"		, false },
	{	"MOUSEPAL.PAL"	, false },
	{	"CAMEO.PAL"		, false },
	{	"GRFXTXT.PAL"	, true	},
	{	"PALETTE.PAL"	, false },
};

void PaletteManager::InitDefaultConverts()
{
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

	//Debug::LogInfo("[%s] - [%s] Palette  FailedToLoad ! ", this->Name.data(), this->CachedName.data());
	return false;
}

void PaletteManager::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Clear_Internal();
	this->CachedName =
		GeneralUtils::ApplyTheaterSuffixToString((const char*)this->Name.data()).c_str();

	bool hasPalette = false;

	if (!Stm.Load(hasPalette))
		return;

	if (hasPalette) {
		this->Palette.reset(GameCreate<BytePalette>());

		if (!Stm.Load(*this->Palette))
			return;

		this->CreateConvert();
	}
}

void PaletteManager::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm.Save(this->Palette != nullptr);
	if (this->Palette) {
		Stm.Save(*this->Palette);
	}
}

ASMJIT_PATCH(0x534DBE, GameInitDefault, 0x5)
{
	for (auto& [name, tem] : DefaultPalettes) {
		FindOrAllocateDefaultConvers(name, tem);
	}
	Debug::Log("PaletteManager total [%d]\n", PaletteManager::Array.size());
	return 0x0;
}