#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/GameUniquePointers.h>
#include <Utilities/Handle.h>
#include <Utilities/INIparser.h>

class PaletteManager final : public Enumerable<PaletteManager>
{
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

	PaletteMode Mode { PaletteMode::Default };
	std::string Name { };
	UniqueGamePtr<ConvertClass> Convert { nullptr };
	UniqueGamePtr<BytePalette> Palette { nullptr };
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector {};

	CustomPalette() = default;
	explicit CustomPalette(PaletteMode mode) noexcept : Mode(mode) { };

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	ConvertClass* GetOrDefaultConvert(ConvertClass* pDefault) const {
		return this->Convert.get() ? this->Convert.get() : pDefault;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Allocate(std::string name);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	void Clear();

private:
	void CreateConvert();
};
