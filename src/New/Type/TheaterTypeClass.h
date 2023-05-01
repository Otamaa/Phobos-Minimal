#pragma once

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <ScenarioClass.h>
#include <Utilities/Constructs.h>
#include <CCINIClass.h>
#include <Theater.h>
#include <string>
#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

#include <Phobos.CRT.h>

class TheaterTypeClass final : public Enumerable<TheaterTypeClass>
{
public:
	TheaterTypeClass(const char* const pTitle = NONE_STR) :
		Enumerable<TheaterTypeClass>(pTitle),
		UIName(),
		ControlFileName("X"),
		ArtFileName("X"),
		PaletteFileName("X"),
		Extension("X"),
		MMExtension("X"),
		Letter("X"),
		IsArctic(false),
		IsAllowedInMapGenerator(false),
		LowRadarBrightness1(0.0f),
		HighRadarBrightness(1.0f),
		unknown_float_60(0.0f),
		unknown_float_64(0.0f),
		unknown_int_68(0),
		unknown_int_6C(0),
		FallbackTheater(TheaterType::Temperate),
		PaletteUnit (),
		PaletteISO(),
		TerrainControl(),
		PaletteOverlay(),
		RootMix(),
		RootMixMD(),
		ExpansionMDMix(),
		SuffixMix(),
		DataMix()
	{ UIName = "Name:<none>"; }

	virtual ~TheaterTypeClass() override = default;
	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->Swizzle(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->Swizzle(Stm);
	}

	virtual void LoadFromINI(CCINIClass* pINI) override;

	static void AddDefaults();
	static void LoadAllTheatersToArray();
	static TheaterTypeClass* FindFromTheaterType(TheaterType nType);
	static TheaterTypeClass* FindFromTheaterType_NoCheck(TheaterType nType) {
		return Array[(int)nType].get();
	}

	// no !
	static void Clear() {}

private:

	template<typename T>
	void Swizzle(T& Stm)
	{
		Stm
			.Process(UIName)
			.Process(ControlFileName)
			.Process(ArtFileName)
			.Process(PaletteFileName)
			.Process(Extension)
			.Process(MMExtension)
			.Process(Letter)
			.Process(IsArctic)
			.Process(IsAllowedInMapGenerator)
			.Process(LowRadarBrightness1)
			.Process(HighRadarBrightness)
			.Process(unknown_float_60)
			.Process(unknown_float_64)
			.Process(unknown_int_68)
			.Process(unknown_int_6C)
			.Process(FallbackTheater)
			.Process(PaletteUnit)
			.Process(PaletteISO)
			.Process(TerrainControl)
			.Process(PaletteOverlay)

			.Process(RootMix)
			.Process(RootMixMD)
			.Process(ExpansionMDMix)
			.Process(SuffixMix)
			.Process(DataMix)
			;
	}

	bool IsDefaultTheater();
	static CCINIClass* GetConfigINI();
public:
	//2, UIname
	Valueable<CSFText> UIName;
	//3, root
	PhobosFixedString<10>ControlFileName;
	//4, datamix
	PhobosFixedString<10>ArtFileName;
	//5, expansionmix
	PhobosFixedString<10>PaletteFileName;
	//6, suffix
	PhobosFixedString<4>Extension;
	//7, mmxsuffix
	PhobosFixedString<4>MMExtension;
	//8, Letter
	PhobosFixedString<2>Letter;
	Valueable<bool> IsArctic;
	Valueable<bool> IsAllowedInMapGenerator;
	Valueable<float> LowRadarBrightness1;
	Valueable<float> HighRadarBrightness;
	Valueable<float> unknown_float_60;
	Valueable<float> unknown_float_64;
	Valueable<int> unknown_int_68;
	Valueable<int> unknown_int_6C;
	TheaterType FallbackTheater;

	PhobosFixedString<0x80> PaletteUnit;
	PhobosFixedString<0x80> PaletteISO;
	PhobosFixedString<0x80> TerrainControl;
	PhobosFixedString<0x80> PaletteOverlay;

	PhobosFixedString<16> RootMix;
	PhobosFixedString<16> RootMixMD;
	PhobosFixedString<16> ExpansionMDMix;
	PhobosFixedString<16> SuffixMix;
	PhobosFixedString<16> DataMix;
};