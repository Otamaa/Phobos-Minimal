#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/GameUniquePointers.h>
#include <Utilities/Handle.h>
#include <Utilities/INIparser.h>

class PaletteManager final : public Enumerable<PaletteManager>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "Palettes";
	static COMPILETIMEEVAL const char* ClassName = "PaletteManager";

public:
	enum class Mode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	FixedString<32> CachedName;
	ConvertClass* Convert_Temperate;
	ConvertClass* Convert;
	BytePalette* Palette;
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector;
	bool NoTemperate;

	PaletteManager(const char* const pTitle);

	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

public:

	template<PaletteManager::Mode nMode>
	COMPILETIMEEVAL ConvertClass* GetConvert() const
	{
		return nMode == Mode::Default ? Convert : Convert_Temperate;
	}

	template<PaletteManager::Mode nMode>
	COMPILETIMEEVAL ConvertClass* GetOrDefaultConvert(ConvertClass* const& pDefault) const
	{
		const auto nRet = GetConvert<nMode>();
		return nRet ? nRet : pDefault;
	}

	bool LoadFromCachedName();

	void Clear_Internal();
	void CreateConvert();

	static void InitDefaultConverts();

private:

	void LoadFromName(const char* PaletteName);
};


class CustomPalette
{
public:
	enum class PaletteMode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	// --- Public mirror fields (kept for source compatibility) -------
	PaletteMode Mode;
	std::string Name;

	CustomPalette() noexcept
		: Mode(PaletteMode::Default), Name() {}
	explicit CustomPalette(PaletteMode mode) noexcept
		: Mode(mode), Name() {}
	~CustomPalette() = default;

	MOVEABLE_ONLY(CustomPalette);

	// --- Accessors (replace old direct field reads) -----------------
	ConvertClass* GetConvert() const;
	BytePalette* GetPalette() const;
	DynamicVectorClass<ColorScheme*>* GetColorschemeDataVector() const;

	ConvertClass* GetOrDefaultConvert(ConvertClass* pDefault) const
	{
		auto* c = GetConvert();
		return c ? c : pDefault;
	}

	// --- Same API as the old class ----------------------------------
	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Allocate(std::string name);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
	void Clear();

	// --- Registry management ----------------------------------------
	// Call at game shutdown to free every cached palette at once.
	static void   ClearRegistry();
	// Debug helper — count of unique cached palettes.
	static size_t RegistrySize();

private:
	struct PaletteResource;

	struct RegistryKey
	{
		std::string NameLower;
		PaletteMode Mode;

		bool operator==(const RegistryKey& o) const noexcept
		{
			return Mode == o.Mode && NameLower == o.NameLower;
		}
	};

	struct RegistryKeyHash
	{
		size_t operator()(const RegistryKey& k) const noexcept
		{
			return std::hash<std::string>{}(k.NameLower)
				^ (static_cast<size_t>(k.Mode) * 0x9E3779B97F4A7C15ULL);
		}
	};

	std::shared_ptr<PaletteResource> Resource;

	// Meyers-singleton so there's no static init order issue.
	static std::unordered_map<
		RegistryKey,
		std::shared_ptr<PaletteResource>,
		RegistryKeyHash>& Registry();

	// Helpers
	static std::string ResolveTheaterSuffix(std::string name);
	static std::string ToLower(std::string s);

	std::shared_ptr<PaletteResource>
		GetOrCreateResource(const std::string& resolvedName, PaletteMode mode);

	void BindResource(std::shared_ptr<PaletteResource> res);
};
