#pragma once

#include <Theater.h>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/CSFText.h>

class CCINIClass;
class TheaterTypeClass final : public Enumerable<TheaterTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "TheaterTypes";
	static COMPILETIMEEVAL const char* ClassName = "TheaterTypeClass";

public:
	TheaterTypeClass(const char* const pTitle) :
		Enumerable<TheaterTypeClass>(pTitle)
	{}

	TheaterTypeClass(const char* const pTitle, const Theater* theater ,bool IsArtic , bool AllowMapGen , bool islunar) : Enumerable<TheaterTypeClass>(pTitle),
		UIName(),
		ControlFileName(theater->ControlFileName),
		ArtFileName(theater->ArtFileName),
		PaletteFileName(theater->PaletteFileName),
		Extension(theater->Extension),
		MMExtension(theater->MMExtension),
		Letter(theater->Letter),
		IsArctic(IsArtic),
		IsAllowedInMapGenerator(AllowMapGen),
		IsLunar(islunar),
		LowRadarBrightness1(theater->RadarTerrainBrightness),
		HighRadarBrightness(theater->RadarTerrainBrightnessAtMaxLevel),
		unknown_float_60(theater->unknown_float_60),
		unknown_float_64(theater->unknown_float_64),
		unknown_int_68(theater->unknown_int_68),
		unknown_int_6C(theater->unknown_int_6C),
		PaletteUnit(),
		PaletteISO(),
		TerrainControl(),
		PaletteOverlay(),
		RootMix(),
		RootMixMD(),
		ExpansionMDMix(),
		SuffixMix(),
		DataMix(),
		TerrainTypeExtension(),
		SmudgeTypeExtension(),
		AnimTypeExtension(),
		OverlayTypeExtension(),
		IsometricTileTypeExtension(),
		BuildingTypeExtension(),
		FallbackTheaterExtension(Theater::Array[0].Extension)
	{
		UIName = theater->UIName;
	}

	virtual ~TheaterTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI);

	static void AddDefaults();
	static void LoadAllTheatersToArray();
	static TheaterTypeClass* FindFromTheaterType(TheaterType nType) {
		
		if (auto pTH = TryFindFromIndex((int)nType))
			return pTH;
			
		return GetArray()[0].get();
	}

	static FORCEDINLINE TheaterTypeClass* FindFromTheaterType_NoCheck(TheaterType nType) {
		return GetArray()[(int)nType].get();
	}

	static OPTIONALINLINE void AllocateWithDefault(const char* Title, const Theater& theater, bool IsArtic, bool AllowMapGen, bool islunar) {
		Array.emplace_back((std::make_unique<TheaterTypeClass>(Title, &theater , IsArtic , AllowMapGen , islunar)));
	}

	void LoadFromStream(PhobosStreamReader& Stm) {
		this->Serialize(Stm);
	}

	void SaveToStream(PhobosStreamWriter& Stm) {
		this->Serialize(Stm);
	}

private:

	template<typename T>
	void Serialize(T& Stm)
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
			.Process(IsLunar)
			.Process(LowRadarBrightness1)
			.Process(HighRadarBrightness)
			.Process(unknown_float_60)
			.Process(unknown_float_64)
			.Process(unknown_int_68)
			.Process(unknown_int_6C)
			.Process(PaletteUnit)
			.Process(PaletteISO)
			.Process(TerrainControl)
			.Process(PaletteOverlay)

			.Process(RootMix)
			.Process(RootMixMD)
			.Process(ExpansionMDMix)
			.Process(SuffixMix)
			.Process(DataMix)

			.Process(TerrainTypeExtension)
			.Process(SmudgeTypeExtension)
			.Process(AnimTypeExtension)
			.Process(OverlayTypeExtension)
			.Process(IsometricTileTypeExtension)
			.Process(BuildingTypeExtension)
			.Process(FallbackTheaterExtension)
			;
	}

	bool IsDefaultTheater();
	static CCINIClass* GetConfigINI();
public:
	//2, UIname
	Valueable<CSFText> UIName {};
	//3, root
	PhobosFixedString<10> ControlFileName { "X" };
	//4, datamix
	PhobosFixedString<10> ArtFileName { "X" };
	//5, expansionmix
	PhobosFixedString<10> PaletteFileName { "X" };
	//6, suffix
	PhobosFixedString<4> Extension { "X" };
	//7, mmxsuffix
	PhobosFixedString<4> MMExtension { "X" };
	//8, Letter
	PhobosFixedString<2> Letter { "X" };
	Valueable<bool> IsArctic { false };
	Valueable<bool> IsAllowedInMapGenerator { false };
	Valueable<bool> IsLunar { false };
	Valueable<float> LowRadarBrightness1 { 0.0f };
	Valueable<float> HighRadarBrightness { 1.0f };
	Valueable<float> unknown_float_60 { 0.0f };
	Valueable<float> unknown_float_64 { 0.0f };
	Valueable<int> unknown_int_68 { 0 };
	Valueable<int> unknown_int_6C { 0 };

	PhobosFixedString<0x80> PaletteUnit {};
	PhobosFixedString<0x80> PaletteISO {};
	PhobosFixedString<0x80> TerrainControl {};
	PhobosFixedString<0x80> PaletteOverlay {};

	PhobosFixedString<16> RootMix {};
	PhobosFixedString<16> RootMixMD {};
	PhobosFixedString<16> ExpansionMDMix {};
	PhobosFixedString<16> SuffixMix {};
	PhobosFixedString<16> DataMix {};

	PhobosFixedString<4> TerrainTypeExtension {};
	PhobosFixedString<4> SmudgeTypeExtension {};
	PhobosFixedString<4> AnimTypeExtension {};
	PhobosFixedString<4> OverlayTypeExtension {};
	PhobosFixedString<4> IsometricTileTypeExtension {};
	PhobosFixedString<4> BuildingTypeExtension {};

	PhobosFixedString<4> FallbackTheaterExtension {};

};