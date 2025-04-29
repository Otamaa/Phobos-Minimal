#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/GameUniquePointers.h>
#include <Utilities/Handle.h>

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
	UniqueGamePtr<BytePalette> Palette;
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
