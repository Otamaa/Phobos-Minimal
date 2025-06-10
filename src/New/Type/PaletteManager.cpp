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
	if (this->Palette)
	{
		GameDelete(this->Palette);
		this->Palette = nullptr;
	}

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
		this->Convert_Temperate = (GameCreate<ConvertClass>(this->Palette, &FileSystem::TEMPERAT_PAL(), DSurface::Primary(), 53, false));
		this->Convert = (GameCreate<ConvertClass>(this->Palette, this->Palette, DSurface::Alternate(), 1, false));
	}
	else {
		this->Convert_Temperate = this->Convert = (GameCreate<ConvertClass>(this->Palette, this->Palette, DSurface::Alternate(), 1, false));
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
		this->Palette = pPal;
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
	pUnitSno->Clear_Internal();

	if (auto pPal = (BytePalette*)MixFileClass::Retrieve(pUnitSno->CachedName.data(), false)) {


		for (auto& color : pPal->Entries) {
			color.R <<= 2;
			color.G <<= 2;
			color.B <<= 2;
		}

		Debug::LogInfo("Resetting {} to {} of {} !\n",
			static_cast<const void*>(pUnitSno->Palette),
			static_cast<const void*>(pPal),
			pUnitSno->CachedName.data());

		pUnitSno->Palette = pPal;
		pUnitSno->CreateConvert();
	}

	if (!pUnitSno->Palette)
		Debug::FatalError("Failed to allocate %s palette!", pUnitSno->CachedName.data());
	else
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
	for (auto& [name, tem] : DefaultPalettes) {
		FindOrAllocateDefaultConvers(name, tem);
	}

}

bool PaletteManager::LoadFromCachedName()
{
	this->Clear_Internal();

	if (auto pPal = FileSystem::AllocatePalette(this->CachedName.data()))
	{
		this->Palette = pPal;
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

		this->Palette = pPal;
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
	uintptr_t was = 0u;

	if (!Stm.Load(hasPalette) || !Stm.Load(was))
		return;

	if (hasPalette) {
		this->Palette = GameCreate<BytePalette>();
		SwizzleManagerClass::Instance->Here_I_Am((long)was, this->Palette);
		if (!Stm.Load(*this->Palette))
			return;

		this->CreateConvert();
	}
}

void PaletteManager::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm.Save(this->Palette != nullptr);
	Stm.Save((uintptr_t)this->Palette);
	if (this->Palette) {
		Stm.Save(*this->Palette);
	}
}

bool CustomPalette::Allocate(std::string name)
{
	if (auto const pSuffix = strstr(name.data(), "~~~"))
	{
		auto const theater = ScenarioClass::Instance->Theater;
		auto const pExtension = Theater::GetTheater(theater).Extension;
		name[0] = pExtension[0];
		name[1] = pExtension[1];
		name[2] = pExtension[2];
	}

	this->Clear();

	if (auto pPal = FileSystem::AllocatePalette(name.c_str()))
	{
		this->Name = name;
		this->Palette.reset(pPal);
		this->CreateConvert();
	}

	return this->Convert != nullptr;
}

bool CustomPalette::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (parser.ReadString(pSection, pKey))
	{
		if (auto const pSuffix = strstr(parser.value(), "~~~"))
		{
			auto const theater = ScenarioClass::Instance->Theater;
			auto const pExtension = Theater::GetTheater(theater).Extension;
			pSuffix[0] = pExtension[0];
			pSuffix[1] = pExtension[1];
			pSuffix[2] = pExtension[2];
		}

		this->Clear();

		if (auto pPal = FileSystem::AllocatePalette(parser.value()))
		{
			this->Palette.reset(pPal);
			this->CreateConvert();
		}

		return this->Convert != nullptr;
	}
	return false;
}

bool CustomPalette::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->Clear();

	bool hasPalette = false;
	auto ret = Stm.Load(this->Mode) && Stm.Load(this->Name) && Stm.Load(hasPalette);

	if (ret && hasPalette)
	{
		this->Palette.reset(GameCreate<BytePalette>());
		ret = Stm.Load(*this->Palette);

		if (ret)
		{
			this->CreateConvert();
		}
	}

	return ret;
}

bool CustomPalette::Save(PhobosStreamWriter& Stm) const
{
	Stm.Save(this->Mode);
	Stm.Save(this->Name);
	Stm.Save(this->Palette != nullptr);
	if (this->Palette)
	{
		Stm.Save(*this->Palette);
	}
	return true;
}

void CustomPalette::Clear()
{
	this->Convert = nullptr;
	this->Palette = nullptr;
	this->ColorschemeDataVector = nullptr;
}

void CustomPalette::CreateConvert()
{
	ConvertClass* buffer = nullptr;
	if (this->Mode == PaletteMode::Temperate)
	{
		buffer = GameCreate<ConvertClass>(
			*this->Palette.get(), FileSystem::TEMPERAT_PAL, DSurface::Primary,
			53, false);
	}
	else
	{
		buffer = GameCreate<ConvertClass>(
			*this->Palette.get(), *this->Palette.get(), DSurface::Alternate,
			1, false);
	}

	this->Convert.reset(buffer);
	this->ColorschemeDataVector = (ColorScheme::GeneratePalette(this->Name.data()));
}