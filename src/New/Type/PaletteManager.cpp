#include "PaletteManager.h"

#include <Utilities/GeneralUtils.h>
#include <Utilities/SavegameDef.h>

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

	if (auto pPal = (BytePalette*)FakeFileLoader::_Retrieve(pUnitSno->CachedName.data(), false)) {


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
	if (auto pPal = (BytePalette*)FakeFileLoader::_Retrieve(this->CachedName.data(), false))
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
	static_assert("Not implemented!");
}

void PaletteManager::SaveToStream(PhobosStreamWriter& Stm)
{
	static_assert("Not implemented!");
}

// =====================================================================
// Internal shared resource — one per unique (resolvedName, mode).
// =====================================================================
struct CustomPalette::PaletteResource
{
	std::string                        Name;    // resolved name
	PaletteMode                        Mode;
	UniqueGamePtr<BytePalette>         Palette;
	UniqueGamePtr<ConvertClass>        Convert;
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector = nullptr;

	PaletteResource(std::string name, PaletteMode mode)
		: Name(std::move(name)), Mode(mode) {}

	// Build from the .pal file on disk. Returns true on success.
	bool BuildFromFile()
	{
		if (auto* pPal = FileSystem::AllocatePalette(this->Name.c_str()))
		{
			this->Palette.reset(pPal);
			this->BuildConvert();
			return this->Convert != nullptr;
		}
		return false;
	}

	// Build the ConvertClass + colorscheme from an already-populated Palette.
	void BuildConvert()
	{
		ConvertClass* buffer = nullptr;
		if (this->Mode == PaletteMode::Temperate)
		{
			buffer = GameCreate<ConvertClass>(
				*this->Palette.get(), FileSystem::TEMPERAT_PAL,
				DSurface::Primary, 53, false);
		}
		else
		{
			buffer = GameCreate<ConvertClass>(
				*this->Palette.get(), *this->Palette.get(),
				DSurface::Alternate, 1, false);
		}
		this->Convert.reset(buffer);

		// Colorscheme — strip extension from name and feed to generator
		std::string filename = this->Name;
		const auto dot = filename.find_last_of('.');
		if (dot != std::string::npos)
			filename.erase(dot);
		this->ColorschemeDataVector =
			ColorScheme::GeneratePalette(filename.data());
	}
};

// =====================================================================
// Registry singleton (Meyers) + public management API
// =====================================================================
std::unordered_map<
	CustomPalette::RegistryKey,
	std::shared_ptr<CustomPalette::PaletteResource>,
	CustomPalette::RegistryKeyHash>&
	CustomPalette::Registry()
{
	static std::unordered_map<
		RegistryKey,
		std::shared_ptr<PaletteResource>,
		RegistryKeyHash> s;
	return s;
}

void CustomPalette::ClearRegistry()
{
	Registry().clear();
}

size_t CustomPalette::RegistrySize()
{
	return Registry().size();
}

// =====================================================================
// Helpers
// =====================================================================
std::string CustomPalette::ToLower(std::string s)
{
	for (auto& c : s)
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return s;
}

std::string CustomPalette::ResolveTheaterSuffix(std::string name)
{
	const std::string target = "~~~";
	if (name.find(target) == std::string::npos)
		return name;

	const std::string replacement =
		Theater::GetTheater(ScenarioClass::Instance->Theater).Extension;

	size_t pos = 0;
	while ((pos = name.find(target, pos)) != std::string::npos)
	{
		name.replace(pos, target.length(), replacement);
		pos += replacement.length();
	}
	return name;
}

std::shared_ptr<CustomPalette::PaletteResource>
CustomPalette::GetOrCreateResource(const std::string& resolvedName, PaletteMode mode)
{
	RegistryKey key { ToLower(resolvedName), mode };
	auto& reg = Registry();

	if (auto it = reg.find(key); it != reg.end())
		return it->second;

	auto res = std::make_shared<PaletteResource>(resolvedName, mode);
	if (!res->BuildFromFile())
		return nullptr; // do NOT cache failures — the file may appear later

	reg.emplace(std::move(key), res);
	return res;
}

void CustomPalette::BindResource(std::shared_ptr<PaletteResource> res)
{
	Resource = std::move(res);
	if (Resource)
	{
		Mode = Resource->Mode;
		Name = Resource->Name;
	}
}

// =====================================================================
// Accessors
// =====================================================================
ConvertClass* CustomPalette::GetConvert() const
{
	return Resource ? Resource->Convert.get() : nullptr;
}

BytePalette* CustomPalette::GetPalette() const
{
	return Resource ? Resource->Palette.get() : nullptr;
}

DynamicVectorClass<ColorScheme*>* CustomPalette::GetColorschemeDataVector() const
{
	return Resource ? Resource->ColorschemeDataVector : nullptr;
}

// =====================================================================
// Public API — mirrors the old class
// =====================================================================
void CustomPalette::Clear()
{
	// Drop the local handle; the registry retains the resource so
	// any UI / other handles continue to work.
	Resource.reset();
	Name.clear();
	// Mode left as-is: it represents the "desired" mode for the next bind.
}

bool CustomPalette::Allocate(std::string name)
{
	const std::string resolved = ResolveTheaterSuffix(std::move(name));

	auto res = GetOrCreateResource(resolved, Mode);
	if (!res)
	{
		Clear();
		return false;
	}
	BindResource(std::move(res));
	return true;
}

bool CustomPalette::Read(INI_EX& parser, const char* pSection, const char* pKey)
{
	if (!parser.ReadString(pSection, pKey))
		return false;

	const std::string resolved = ResolveTheaterSuffix(parser.value());

	auto res = GetOrCreateResource(resolved, Mode);
	if (!res)
	{
		Clear();
		return false;
	}
	BindResource(std::move(res));
	return true;
}

bool CustomPalette::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	Clear();

	bool hasPalette = false;
	if (!Stm.Process(hasPalette)) return false;

	PaletteMode mode;
	if (!Stm.Process(mode)) return false;
	Mode = mode;

	if (!hasPalette)
		return true;

	std::string name;
	if (!Stm.Process(name, RegisterForChange))
		return false;

	RegistryKey key { ToLower(name), mode };
	auto& reg = Registry();

	if (auto it = reg.find(key); it != reg.end())
	{
		// Cache hit — drain the saved bytes into a throwaway so the
		// stream advances, then bind the cached (current) resource.
		BytePalette scratch;
		if (!Stm.Process(scratch)) return false;
		BindResource(it->second);
		return true;
	}

	// Cache miss — rebuild from the saved bytes. This path also covers
	// the case where the .pal file is missing from disk now.
	auto res = std::make_shared<PaletteResource>(name, mode);
	res->Palette.reset(GameCreate<BytePalette>());
	if (!Stm.Process(*res->Palette))
		return false;

	res->BuildConvert();
	if (!res->Convert)
		return false;

	reg.emplace(std::move(key), res);
	BindResource(std::move(res));
	return true;
}

bool CustomPalette::Save(PhobosStreamWriter& Stm) const
{
	const bool hasPalette = Resource && Resource->Palette != nullptr;

	Stm.Process(const_cast<bool&>(hasPalette));

	PaletteMode modeToSave = Resource ? Resource->Mode : Mode;
	Stm.Process(modeToSave);

	if (hasPalette)
	{
		std::string nameCopy = Resource->Name;
		Stm.Process(nameCopy);
		Stm.Process(*Resource->Palette);
	}
	return true;
}