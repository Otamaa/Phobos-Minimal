#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Constructs.h>

class PaletteManager final : public Enumerable<PaletteManager>
{
public:
	enum class Mode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	UniqueGamePtrB<ConvertClass> Convert_Temperate;
	UniqueGamePtrB<ConvertClass> Convert;
	UniqueGamePtr<BytePalette> Palette;

	PaletteManager(const char* const pTitle);
	virtual ~PaletteManager() override = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

public:

	template<PaletteManager::Mode nMode>
	constexpr ConvertClass* GetConvert() const
	{
		return nMode == Mode::Default ? Convert.get() : Convert_Temperate.get();
	}

	template<PaletteManager::Mode nMode>
	constexpr ConvertClass* GetOrDefaultConvert(ConvertClass* const& pDefault) const
	{
		const auto nRet = GetConvert<nMode>();
		return nRet ? nRet : pDefault;
	}

private:

	void Clear_Internal();
	void CreateConvert();
	void LoadFromName(const char* PaletteName);
};
