#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/GameUniquePointers.h>

class PaletteManager final : public Enumerable<PaletteManager>
{
public:
	enum class Mode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	FixedString<32> CachedName;
	Handle<ConvertClass* , UninitConvert> Convert_Temperate;
	Handle<ConvertClass*, UninitConvert> Convert;
	UniqueGamePtr<BytePalette> Palette;
	DynamicVectorClass<ColorScheme*>* ColorschemeDataVector;

	PaletteManager(const char* const pTitle);
	virtual ~PaletteManager() override
	{
		if(auto pVec = std::exchange(this->ColorschemeDataVector, nullptr)) {
			for (int i = 0; i < pVec->Count; ++i) {
				if (auto pScheme = std::exchange(pVec->Items[i], nullptr)) {
						GameDelete<true,false>(pScheme);
				}
			}

			GameDelete(pVec);
		}
	}

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

	bool LoadFromCachedName();
private:

	void Clear_Internal();
	void CreateConvert();
	void LoadFromName(const char* PaletteName);

};
