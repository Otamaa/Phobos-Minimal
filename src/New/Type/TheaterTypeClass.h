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
	TheaterTypeClass(const char* const pTitle = "<none>") :
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
		LowRadarBrightness1(1.0f)
	{ UIName = "Name:<none>"; }

	virtual ~TheaterTypeClass() override = default;
	virtual void LoadFromStream(PhobosStreamReader& Stm)
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
			;
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
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
			;
	}

	virtual void LoadFromINI(CCINIClass* pINI) override;

	static void AddDefaults();
	static void LoadAllTheatersToArray();
	static TheaterType From_Name(const char* pName);
	static const TheaterTypeClass& As_Reference(TheaterType nType);
	static const TheaterTypeClass* As_Pointer(TheaterType nType);
	static TheaterTypeClass* As_Pointer_(TheaterType nType);
	static const TheaterTypeClass& As_Reference(const char* pName);
	static const TheaterTypeClass* As_Pointer(const char* pName);
	static const char* GetIdentifier(TheaterType type);
	static const char* GetUIName(TheaterType type);
	static const char* GetControlFileName(TheaterType type);
	static const char* GetArtFileName(TheaterType type);
	static const char* GetPaletteFileName(TheaterType type);
	static const char* GetExtension(TheaterType type);
	static char* GetCharExtension(TheaterType type);
	static const char* GetMMExtension(TheaterType type);
	static const char* GetLetter(TheaterType type);
	static bool GetIsArtic(TheaterType type);
	static bool GetAllowMapGen(TheaterType type);
	static float GetLowRadarBrightness(TheaterType type, bool bSecond = false);

private:

	bool IsDefaultTheater();
	static CCINIClass* GetConfigINI();
	void LoadConfiguration(CCINIClass* pINI);

	Valueable<CSFText> UIName;//2
	PhobosFixedString<10>ControlFileName;//3 
	PhobosFixedString<10>ArtFileName;//4 , datamix
	PhobosFixedString<10>PaletteFileName;//5 , expansionmix
	PhobosFixedString<4>Extension;//6, suffix
	PhobosFixedString<4>MMExtension;//7
	PhobosFixedString<2>Letter; //Letter , 8
	Valueable<bool> IsArctic;
	Valueable<bool> IsAllowedInMapGenerator;
	Valueable<float> LowRadarBrightness1;
};