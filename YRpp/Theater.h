#pragma once

#include <GeneralDefinitions.h>
#include <Helpers/CompileTime.h>

struct Theater	//US English spelling to keep it consistent with the game
{
public:
	static COMPILETIMEEVAL reference<Theater, 0x7E1B78u, 6u> const Array{};

	static Theater& GetTheater(const TheaterType& theater) {
		return Array[static_cast<int>(theater)];
	}

	static Theater* Get(const TheaterType& theater) {
		return &Array[static_cast<int>(theater)];
	}

	static int __fastcall FindIndexById(const char* pName)
		{ JMP_FAST(0x48DBE0); }

	static void __fastcall Init(TheaterType theater)
		{ JMP_FAST(0x5349C0); }

	static void __fastcall SetTheaterLetter(char* string, int theater)
		{ JMP_FAST(0x5F96B0); }

	char	Identifier[0x10];		//e.g. "TEMPERATE"
	char	UIName[0x20];			//e.g. "Name:Temperate"
	char	ControlFileName[0xA];	//e.g. "TEMPERAT" -> INI and MIX
	char	ArtFileName[0xA];		//e.g. "ISOTEMP" -> MIX
	char	PaletteFileName[0xA];	//e.g. "ISOTEM" -> PAL
	char	Extension[0x4];			//e.g. "TEM" -> Iso tile file extension
	char	MMExtension[0x4];		//e.g. "MMT" -> Marble Madness tile file extension
	char	Letter[0x2];			//e.g. "T" -> Theater specific IDs (GTCNST, NTWEAP, YTBARRACKS)

	//unused, was probably lighting stuff once
	float	RadarTerrainBrightness;		//0.0 to 1.0 BrightnessAtMinLevel
	float	RadarTerrainBrightnessAtMaxLevel; //5c
	float	unknown_float_60;
	float	unknown_float_64;
	int		unknown_int_68;
	int		unknown_int_6C;
};

static_assert(sizeof(Theater) == 0x70, "Invalid Size !");
static_assert(offsetof(Theater, Extension) == 0x4E, "Theater ClassMember Shifted !");
