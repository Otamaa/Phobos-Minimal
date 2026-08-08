#include <New/Type/TheaterTypeClass.h>

#include <Phobos.h>

#include <Ext/Scenario/Body.h>
#include <MixFileClass.h>
#include <Utilities/Macro.h>

#ifndef DISABLE_THEATER_HOOKS
ASMJIT_PATCH(0x48DBE0, TheaterTypeClass_FindIndex, 0x5)
{
	GET(char*, nTheaterName, ECX);
	R->EAX<int>(TheaterTypeClass::FindIndexById(nTheaterName));
	return 0x48DC12;
}

#pragma region IsoTileTypeHooks

ASMJIT_PATCH(0x54547F, IsometricTileTypeClass_ReadINI_SetPaletteISO, 0x6)
{
	LEA_STACK(char*, outBuffs, 0x6B0);
	LEA_STACK(CCFileClass*, file_c, 0xA10 - 0x668);

	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);

	char* buffer = nullptr;
	if (auto& data = pTheater->PaletteISO)
	{
		buffer = (data.data());
	}
	else
	{
		//Isometric pal = ISO+Extension.pal
		buffer = (pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data();
	}

	//0x8295F4 -> 'ISO%s.PAL'
	CRT::sprintf(outBuffs, reinterpret_cast<const char*>(0x8295F4), buffer);
	file_c->CCFileClass::CCFileClass(outBuffs);
	const bool Exist = file_c->IsAvaible();

	if (!Exist)
		Debug::Log("Failed to load IsometricTileTypeClass Palette %s For [%s]", outBuffs, pTheater->Name.data());
	else
	{
		if (file_c->Read(FileSystem::ISOx_PAL(), sizeof(BytePalette)))
		{
			Debug::Log("Loaded IsometricTileTypeClass Palette %s For [%s]\n", outBuffs, pTheater->Name.data());

			for (size_t i = 0; i < BytePalette::EntriesCount; ++i)
			{
				auto& data = FileSystem::ISOx_PAL->at(i);
				data.R *= 4;
				data.G *= 4;
				data.B *= 4;
			}
		}
	}

	return 0x5454EB;

}

ASMJIT_PATCH(0x5454F0, IsometricTileTypeClass_ReadINI_TerrainControl, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);

	if (auto& data = pTheater->TerrainControl)
	{
		R->ECX<char*>(data.data());
		return 0x545513;
	}

	//Ini  file = ControlFilename+MD.ini
	R->EDX(pTheater->ControlFileName.data());
	return 0x5454F6;
}

ASMJIT_PATCH(0x5452F2, IsometricTileTypeClass_TheaterType_Slope, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x5452F8;
}

//here theater index is multiplied by `sizeof(Theater)` !
ASMJIT_PATCH(0x546662, IsometricTileTypeClass_TheaterType_makepath, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x546668;
}

ASMJIT_PATCH(0x546753, IsometricTileTypeClass_TheaterType_MMx, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EAX(pTheater->MMExtension.data());
	return 0x546759;
}

ASMJIT_PATCH(0x546833, IsometricTileTypeClass_FallbackTheater, 0x5)
{
	GET(char*, pFileName, EDX);
	LEA_STACK(char*, pBuffer, STACK_OFFS(0x10, 0x2C0));
	GET_STACK(bool, bSomething, STACK_OFFS(0x10, 0x9FE));

	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	CRT::_makepath(pBuffer, 0, 0, pFileName, pTheater->FallbackTheaterExtension.data());
	R->AL(bSomething);
	return 0x54684F;
}

#pragma endregion

#pragma region AresHooks
#include <Utilities/Cast.h>
#include <Ext/TechnoType/Body.h>
#include <SmudgeTypeClass.h>
#include <TerrainTypeClass.h>

ASMJIT_PATCH(0x5F9634, ObjectTypeClass_LoadFromINI, 6)
{
	GET(ObjectTypeClass*, pType, EBX);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x1B0, -4));

	if (TechnoTypeClass* const pTechnoType = type_cast<TechnoTypeClass* const>(pType))
	{
		INI_EX exINI(pINI);
		TechnoTypeExtContainer::Instance.Find(pTechnoType)->AlternateTheaterArt.Read(exINI, pType->ID, "AlternateTheaterArt");
	}

	return 0;
}

// SHP file loading
ASMJIT_PATCH(0x5F9070, ObjectTypeClass_Load2DArt, 6)
{
	GET(ObjectTypeClass* const, pType, ECX);

	auto const scenarioTheater = ScenarioClass::Instance->Theater;
	if (scenarioTheater == TheaterType::None)
		Debug::FatalError(__FUNCTION__" for [(%s) %s] Cannot Proceed With Negative theater Index! ", pType->ID, pType->GetThisClassName());

	auto const TheaterData = TheaterTypeClass::FindFromTheaterType_NoCheck(scenarioTheater);
	const auto what = pType->WhatAmI();
	TechnoTypeExtData* pTypeData = nullptr;

	if (TechnoTypeClass* const pThisTechno = type_cast<TechnoTypeClass* const>(pType))
	{
		pTypeData = TechnoTypeExtContainer::Instance.Find(pThisTechno);
	}

	// extension object is not present if not techno type
	if (pTypeData && pTypeData->AlternateTheaterArt)
	{
		if (!pType->ArcticArtInUse)
		{ // this flag is not used anywhere outside this function, so I'll just hijack it
			pType->ArcticArtInUse = true;
			std::string _baseName = pType->ImageFile;
			_baseName += TheaterData->Letter.data();

			if (!CCINIClass::INI_Art->GetSection(_baseName.c_str()))
			{
				pType->ArcticArtInUse = false;
				_baseName = pType->ImageFile;
			}

			PhobosCRT::strCopy(pType->ImageFile, _baseName.c_str());
		}
	}
	else if (pType->AlternateArcticArt && TheaterData->IsArctic && !pType->ImageAllocated)
	{
		if (!pType->ArcticArtInUse)
		{
			char basename[MAX_PATH];
			IMPL_SNPRNINTF(basename, sizeof(basename), GameStrings::STRFORMAT_A(), pType->ImageFile);
			PhobosCRT::strCopy(pType->ImageFile, basename);
			pType->ArcticArtInUse = true;
		}
	}
	else
	{
		pType->ArcticArtInUse = false;
	}

	std::string _ext = pType->ImageFile;
	_ext += ".";
	_ext += pType->Theater ? TheaterData->Extension.c_str() : "SHP";

	if (!pType->Theater && pType->NewTheater && scenarioTheater != TheaterType::None)
	{
		if (isalpha(static_cast<unsigned char>(_ext[0])))
		{
			// evil hack to uppercase
			auto const c1 = static_cast<unsigned char>(_ext[1]) & ~0x20;
			if (c1 == 'A' || c1 == 'T')
			{
				_ext[1] = TheaterData->Letter.data()[0];
			}
		}
	}

	if (pType->ImageAllocated && pType->Image)
	{
		GameDelete<true, false>(pType->Image);
	}

	pType->Image = nullptr;
	pType->ImageAllocated = false;


	// what? it's what the game does, evidently those load somewhere else
	const bool IsTerrainOrSmudge = what == SmudgeTypeClass::AbsID || what == TerrainTypeClass::AbsID;

	if (!IsTerrainOrSmudge)
	{
		const auto forceShp = what == OverlayTypeClass::AbsID || what == AnimTypeClass::AbsID;

		auto pImage = FakeFileLoader::_Retrieve(_ext.c_str(), forceShp);
		if (!pImage)
		{
			_ext[1] = 'G';
			pImage = FakeFileLoader::_Retrieve(_ext.c_str(), forceShp);
		}

		pType->Image = static_cast<SHPCaches*>(pImage);
	}

	if (const auto pShp = pType->Image)
	{
		auto const size = std::max(pShp->CurrentHeader.Width, pShp->CurrentHeader.Height);
		pType->MaxDimension = std::max(size, static_cast<short>(8));
	}

	return 0x5F92C3;
}

ASMJIT_PATCH(0x5F96B0, ObjectTypeClass_TheaterSpecificID, 6)
{
	GET(char*, basename, ECX);
	GET(TheaterType, Theater, EDX);

	if (Theater != TheaterType::None)
	{
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if (isalpha(static_cast<unsigned char>(c0)))
		{
			if (c1 == 'A' || c1 == 'T')
			{
				basename[1] = TheaterTypeClass::FindFromTheaterType_NoCheck(Theater)
					->Letter.data()[0];
			}
		}
	}
	return 0x5F9702;
}

#pragma endregion

#pragma region ScenarioClass_InitTheater

//How mix loaded ?
// controlfilename.mix - inside RA2 mix ,overlay and tree stuffs , must ?
// ArtFileName.mix - inside RA2 mix , c_shadow , civ building shp , majority of terrain files , terraub files us must
// Extension.mix - inside RA2 mix ,wake1 , wake2 ,ammo01 shp , ignorable ?
// PaletteFileName+md.mix inside RA2MD mix , mostly building shp , ignorable ?
// controlfilename+md.mix inside RA2MD mix , mostly building shp , ignorable ?
// palette can be specified with : "PaletteUnit" "PaletteISO"  "PaletteOverlay"
// ini file for theater control can be specified with : "TerrainControl"
// if both not specified , game will decide it with their naming convention

TheaterType lastTheater;
#ifdef _OLDCODE

ASMJIT_PATCH(0x5349E3, ScenarioClass_InitTheater_Handle, 0x6)
{
	GET(TheaterType, nType, EDI);

	const auto thName = nType == TheaterType::None ? "unknown" : TheaterTypeClass::Array[(size_t)nType]->Name.data();
	Debug::LogInfo("Init For Theater [{} - {}]", (int)nType , thName);
	ScenarioClass::Instance->Theater = nType;
	lastTheater = nType;
	typedef int(*wsprintfA_ptr)(LPSTR, LPCSTR, ...);
	GET(wsprintfA_ptr, pFunc, EBP);

	if (nType == TheaterType::None) {
		Debug::FatalError("TheaterType is invalid ! , fallback to Temperate!");
	}

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(nType);

	// buffer size 16
	LEA_STACK(char*, pRootMix, STACK_OFFS(0x6C, 0x50));

	if (!pTheater->RootMix)
		pFunc(pRootMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->ControlFileName.data());
	else
		CRT::strcpy(pRootMix, pTheater->RootMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pRootMixMD, STACK_OFFS(0x6C, 0x40));

	if (!pTheater->RootMixMD)
		pFunc(pRootMixMD, GameStrings::STRFORMAT_MD_DOT_MIX(), pTheater->ControlFileName.data());
	else
		CRT::strcpy(pRootMixMD, pTheater->RootMixMD.c_str());

	// buffer size 16
	LEA_STACK(char*, pExpansionMixMD, STACK_OFFS(0x6C, 0x20));

	if (!pTheater->ExpansionMDMix)
		pFunc(pExpansionMixMD, GameStrings::STRFORMAT_MD_DOT_MIX(), pTheater->PaletteFileName.data());
	else
		CRT::strcpy(pExpansionMixMD, pTheater->ExpansionMDMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pSuffixMix, STACK_OFFS(0x6C, 0x30));

	if (!pTheater->SuffixMix)
		pFunc(pSuffixMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->Extension.data());
	else
		CRT::strcpy(pSuffixMix, pTheater->SuffixMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pDataMix, STACK_OFFS(0x6C, 0x10));

	if (!pTheater->DataMix)
		pFunc(pDataMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->ArtFileName.data());
	else
		CRT::strcpy(pDataMix, pTheater->DataMix.c_str());

	Debug::Log("Theater[%s] Mix [%s , %s , %s , %s , %s]\n", pTheater->Name.data(),
	pRootMix, pRootMixMD, pExpansionMixMD, pSuffixMix, pDataMix);

	// any errors triggered before this line are irrelevant
	// caused by reading the section while only certain flags from it are needed
	// and before other global lists are initialized
	Phobos::Otamaa::TrackParserErrors = true;

	SessionClass::Instance->Callback(8);
	R->EBX(pTheater->ControlFileName.data());
	R->Stack(STACK_OFFS(0x6C, 0x58), pTheater->Extension.data());

	return 0x0534A68;
}

ASMJIT_PATCH(0x534A9D, ScenarioClass_initTheater_TheaterType_ArticCheck, 0x6)
{
	enum { AllocateMix = 0x534AA6, NextFunc = 0x534AD6 };
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	//this one usefull for loading additional mix files
	return pTheater->RootMixMD || pTheater->IsArctic ?
		AllocateMix : NextFunc;
}

ASMJIT_PATCH(0x534BEE, ScenarioClass_initTheater_TheaterType_OverlayPalette, 0x5)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);

	if (const auto& data = pTheater->PaletteOverlay)
	{
		R->EAX(FakeFileLoader::_Retrieve(data.c_str(), false));
		return 0x534C09;
	}

	return 0x0;
}

ASMJIT_PATCH(0x534CA9, ScenarioClass_initTheater_TheaterType_SetPaletteUnit, 0x8)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);

	if (auto const& data = pTheater->PaletteUnit)
	{
		R->ESI(FakeFileLoader::_Retrieve(data.c_str(), false));
		return 0x534CCA;
	}

	return 0x0;
}
#endif

static constexpr double satBase = std::bit_cast<double>(0x3FEBECDE5DA115A9ULL); // 0.8726646259971648
static constexpr double satStep = std::bit_cast<double>(0x3FA7D45E2DC37C4CULL); // 0.04654211338651545
static constexpr double valBase = std::bit_cast<double>(0x3FD657184AE74487ULL); // 0.3490658503988659
static constexpr double valStep = std::bit_cast<double>(0x3FB4D9D2680B0CC2ULL); // 0.08144869842640204
static constexpr double valAngleFirstIter = std::bit_cast<double>(0x3FC921FB54442D19ULL); // 0.1963495408493621 (11.25°)

LightConvertClass* __fastcall Generate_Color_Spread_Light_Convert(
	HSVClass* hsv,
	BytePalette* pal1,
	BytePalette* pal2,
	BytePalette* pal3,
	Surface* surface,
	int                 count,
	int                 r,
	int                 g,
	int                 b,
	char* indexes)
{
	const int   baseHue = hsv->Hue;
	const double baseSat = static_cast<double>(hsv->Saturation);
	const double baseVal = static_cast<double>(hsv->Value);

	*pal3 = *pal1;

	int i = 0;
	do {
		const double id = static_cast<double>(i);

		double valAngle = id * valStep + valBase;
		double satAngle = id * satStep + satBase;

		if (i == 0)
			valAngle = valAngleFirstIter;

		HSVClass tempHSV {
			baseHue,
			(Math::sin(satAngle) * baseSat),
			(Math::cos(valAngle) * baseVal)
		};

		(*pal3)[(i + 16) % BytePalette::EntriesCount] = tempHSV;
		++i;
	}
	while (i < 16);

	//// Set ShadeCount to 53 to initialize the palette fully shaded - this is required to make it not draw over shroud for some reason.
	//ASMJIT_PATCH(0x68C4C4, GenerateColorSpread_ShadeCountSet, 0x5)
	count = Phobos::Config::ApplyShadeCountFix && count == 1 ? 53 : count;

	return GameCreate<LightConvertClass>(pal3, pal2, surface, r, g, b, false, indexes, count);
}

DEFINE_FUNCTION_JUMP(CALL, 0x68C7E9 , Generate_Color_Spread_Light_Convert)
DEFINE_FUNCTION_JUMP(CALL, 0x68C8B2, Generate_Color_Spread_Light_Convert)

void __fastcall Init_Theaters(TheaterType theater)
{
	auto DestroyMix = [](MixFileClass* mix) {
		GameDelete<true,true>(mix);
	};

	auto NewMix = [](const char* filename) {
		auto pKey = MixFileClass::Key();
		return GameCreate<MixFileClass>(filename, pKey);
	};

    // --- Hook 0x5349E3: resolve MIX filenames via TheaterTypeClass ---
    // Vanilla built these with wsprintfA from Theaters[].Root/Suffix/etc.
    // Phobos uses TheaterTypeClass fields with per-field overrides.
    {
        const auto thName = theater == TheaterType::None
            ? "unknown"
            : TheaterTypeClass::Array[(size_t)theater]->Name.data();
        Debug::LogInfo("Init For Theater [{} - {}]", (int)theater, thName);
    }
 
    if (theater == TheaterType::None)
        Debug::FatalError("TheaterType is invalid ! , fallback to Temperate!");
 
    const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
 
    // Each name: use Phobos override field if set, else build from vanilla template string.
    char rootMix[16];
    if (!pTheater->RootMix)
        sprintf_s(rootMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->ControlFileName.data());
    else
        strcpy_s(rootMix, pTheater->RootMix.c_str());
 
    char rootMixMD[16];
    if (!pTheater->RootMixMD)
        sprintf_s(rootMixMD, GameStrings::STRFORMAT_MD_DOT_MIX(), pTheater->ControlFileName.data());
    else
        strcpy_s(rootMixMD, pTheater->RootMixMD.c_str());
 
    char expansionMixMD[16];
    if (!pTheater->ExpansionMDMix)
        sprintf_s(expansionMixMD, GameStrings::STRFORMAT_MD_DOT_MIX(), pTheater->PaletteFileName.data());
    else
        strcpy_s(expansionMixMD, pTheater->ExpansionMDMix.c_str());
 
    char suffixMix[16];
    if (!pTheater->SuffixMix)
        sprintf_s(suffixMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->Extension.data());
    else
        strcpy_s(suffixMix, pTheater->SuffixMix.c_str());
 
    char dataMix[16];
    if (!pTheater->DataMix)
        sprintf_s(dataMix, GameStrings::STRFORMAT_DOT_MIX(), pTheater->ArtFileName.data());
    else
        strcpy_s(dataMix, pTheater->DataMix.c_str());
 
    Debug::Log("Theater[%s] Mix [%s , %s , %s , %s , %s]\n",
        pTheater->Name.data(),
        rootMix, rootMixMD, expansionMixMD, suffixMix, dataMix);
 
    // Parser error tracking: errors before this point are irrelevant
    // (section read before global lists are initialized).
    Phobos::Otamaa::TrackParserErrors = true;
 
    ScenarioClass::Instance->Theater = theater;
    lastTheater = theater;
    SessionClass::Instance->Callback(8);
 
	DEFINE_REFERENCE(MixFileClass*, TheaterRoot ,0x884E0C)

    // Note: vanilla had a LastTheater early-return guard here.
    // Hook 0x5349E3 skipped it (always reloads); Phobos always proceeds.
 
    // --- Destroy and reload theater MIX chain ---
 
    // Hook 0x534A9D: arctic/custom-MD check replaces vanilla THEATER_SNOW check.
    // Vanilla: only allocate TheaterRoot for THEATER_SNOW.
    // Phobos:  also allocate if RootMixMD override is set OR IsArctic flag.
    DestroyMix(TheaterRoot);
    TheaterRoot = nullptr;
    if (pTheater->RootMixMD || pTheater->IsArctic) {
        TheaterRoot = NewMix(rootMixMD);
    }
 
	DEFINE_REFERENCE(MixFileClass*,  dword_884E08, 0x884E08)

    // <root>.MIX (unnamed theater data global)
    DestroyMix(dword_884E08);
    dword_884E08 = NewMix(rootMix);
 
	DEFINE_REFERENCE(MixFileClass*,  MixFile_Theater_TEM, 0x884E10)

    // <suffix>.MIX
    DestroyMix(MixFile_Theater_TEM);
    MixFile_Theater_TEM = NewMix(suffixMix);
 
    SessionClass::Instance->Callback(6);
 
	DEFINE_REFERENCE(MixFileClass*,  MixFile_Theater_ISOTEM, 0x884E20)
    // <expMix>MD.MIX
    DestroyMix(MixFile_Theater_ISOTEM);
    MixFile_Theater_ISOTEM = NewMix(expansionMixMD);
 
	DEFINE_REFERENCE(MixFileClass*,  MixFile_Theater_ISOTEMP, 0x884E1C)
    // <dataMix>.MIX
    DestroyMix(MixFile_Theater_ISOTEMP);
    MixFile_Theater_ISOTEMP = NewMix(dataMix);
 
    SessionClass::Instance->Callback(12);

    // --- Load game palette ---
    // Hook 0x534BEE: try PaletteOverlay first; fall back to vanilla <root>.PAL retrieve.
    {
		BytePalette* palFile = nullptr;
 
        if (const auto& overlay = pTheater->PaletteOverlay) {
            // Phobos override: retrieve directly by name from FakeFileLoader.
			palFile = static_cast<BytePalette*>(FakeFileLoader::_Retrieve(overlay.c_str(), false));
        } else  {
            // Vanilla path: retrieve <root>.PAL from MIX.
            char palName[16];
            sprintf_s(palName, "%s.PAL", pTheater->ControlFileName.data());
			palFile = static_cast<BytePalette*>(FakeFileLoader::Retrieve(palName, 0));
        }
 
        if (palFile) {
            // Each palette entry is 3 bytes [R, G, B] in 6-bit VGA format; shift left 2 → 8-bit.
            for (int i = 0; i < 256; ++i) {
                ColorStruct& entry = FileSystem::TEMPERAT_PAL->Entries[i];

				entry.R = palFile->Entries[i].R << 2;
				entry.G = palFile->Entries[i].G << 2;
				entry.B = palFile->Entries[i].B << 2;
            }
        } else {
            // Fallback: synthesize gradient palette.
            // Assembly: Red=i, Green=0xFF-i, Blue=i*4.
            for (int i = 0; i < 256; ++i) {
                ColorStruct& entry = FileSystem::TEMPERAT_PAL->Entries[i];

                entry.R   = static_cast<uint8_t>(i);
                entry.G = static_cast<uint8_t>(0xFF - i);
                entry.B  = static_cast<uint8_t>(i * 4);
            }
        }
    }
 
	DEFINE_REFERENCE(BytePalette, Plaette, 0x885A80)
    Plaette = FileSystem::TEMPERAT_PAL();
 
    // --- Load unit palette from UNIT<suffix>.PAL ---
    {
		BytePalette* unitPalData = nullptr;

		if (auto const& data = pTheater->PaletteUnit){
			unitPalData = static_cast<BytePalette*>(FakeFileLoader::Retrieve(data.c_str(), 0));
		} else {
			char unitPalName[16];
			sprintf_s(unitPalName, "UNIT%s.PAL", pTheater->Extension.data());
			unitPalData = static_cast<BytePalette*>(FakeFileLoader::Retrieve(unitPalName, 0));
		}

        Game::CallBack();
 
        if (unitPalData)
            FileSystem::UNITPAL = *unitPalData;
 
        // Shift all unit palette entries from 6-bit to 8-bit.
        for (int j = 0; j < 256; ++j) {
            ColorStruct& entry = FileSystem::UNITPAL->Entries[j];

            entry.R   = static_cast<uint8_t>(entry.R)   << 2;
            entry.G = static_cast<uint8_t>(entry.G) << 2;
            entry.B  = static_cast<uint8_t>(entry.B)  << 2;
        }
    }
 
    // --- Build color scheme light converts ---
    // Progress steps 12..25 distributed proportionally across ColorSchemes.
    {
		DEFINE_NONSTATIC_ARRAY_REFERENCE(char, 256, _ColorBuffer, 0x83E1AC);

        const int count    = ColorScheme::Array->Count;
        const int divisor  = count / 13; // 0x4EC4EC4F reciprocal in asm — exact /13
        int       progress = 12;
 
        for (int i = 0; i < count; ++i)  {
			auto pCS = ColorScheme::Array->Items[i];

			if (pCS->LightConvert) {
				GameDelete<true,false>(pCS->LightConvert);
				pCS->LightConvert = nullptr;
			}

			pCS->LightConvert = Generate_Color_Spread_Light_Convert(
				&pCS->BaseColor,
				FileSystem::UNITPAL.operator->(),
				 FileSystem::TEMPERAT_PAL.operator->(),
				&pCS->Colors,
				DSurface::Primary(),
				pCS->ShadeCount,
				1000, 1000, 1000,
				_ColorBuffer()
			);

            int newProgress = (divisor > 0) ? (i / divisor + 12) : 12;
            if (newProgress >= 25)
                newProgress = 25;
 
            if (newProgress != progress) {
                SessionClass::Instance->Callback(newProgress);
                progress = newProgress;
            }
 
            Game::CallBack();
        }
    }
 
    Game::INIColors_6267A0((int)theater);
    TechnoTypeClass::SetPalettes();
    SessionClass::Instance->Callback(25);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5349C0 , Init_Theaters)

#pragma endregion

#pragma region replacedMakepath


////AnimType
ASMJIT_PATCH(0x4279BB, AnimTypeClass_TheaterSuffix_1, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EDX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x4279C1;
}

ASMJIT_PATCH(0x427AF1, AnimTypeClass_TheaterSuffix_2, 0x5)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x427AF6;
}

ASMJIT_PATCH(0x428903, AnimTypeClass_TheaterSuffix_3, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428909;
}

ASMJIT_PATCH(0x428CBF, AnimTypeClass_TheaterSuffix_4, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428CC5;
}

//BuildingType
ASMJIT_PATCH(0x45E9FD, BuildingTypeClass_TheaterSuffix_1, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA03;
}

ASMJIT_PATCH(0x45EA60, BuildingTypeClass_TheaterSuffix_2, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA66;
}

//OverlayType
ASMJIT_PATCH(0x5FE673, OverlayTypeClass_TheaterSuffix_1, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FE679;
}

ASMJIT_PATCH(0x5FEB94, OverlayTypeClass_TheaterSuffix_2, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEB9A;
}

ASMJIT_PATCH(0x5FEE42, OverlayTypeClass_TheaterSuffix_3, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EDX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEE48;
}

//SmudgeTypes
ASMJIT_PATCH(0x6B54CF, SmudgeTypesClass_TheaterSuffix_1, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B54D5;
}

ASMJIT_PATCH(0x6B57A7, SmudgeTypesClass_TheaterSuffix_2, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B57AD;
}

//TerrainType
ASMJIT_PATCH(0x71DCE4, TerrainTypeClass_TheaterSuffix, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->ECX((pTheater->TerrainTypeExtension ? pTheater->TerrainTypeExtension : pTheater->Extension).data());
	return 0x71DCEA;
}

//objectType , this maybe alredy overriden by ares
ASMJIT_PATCH(0x5F915C, ObjectTypeClass_TheaterSuffix_3, 0x6)
{
	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	R->EDX(pTheater->Extension.data());
	return 0x5F9162;
}

//#pragma optimize("", on )
#pragma endregion

ASMJIT_PATCH(0x6DAE3E, TacticalClass_DrawWaypoints_SelectColor, 0x8)
{
	GET(ScenarioClass*, pScen, ECX);

	const int color = 14;//default;
	if ((int)pScen->Theater == -1)
	{
		Debug::LogInfo(__FUNCTION__" Scenario is negative idx , default to Temperate");
		R->EAX(color);
		return 0;
	}
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(pScen->Theater);

	R->EAX(pTheater->IsArctic ? 12 : 14);
	return 0;
}

#pragma region CellAndTerrainStuffs

ASMJIT_PATCH(0x483DF0, CellClass_CheckPassability_ArtictA, 0x5)
{
	GET(TheaterType, theater, EAX);

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	return pTheater->IsArctic ?
		0x483E0C : 0x483DF5;
}

ASMJIT_PATCH(0x483E03, CellClass_CheckPassability_ArtictB, 0x9)
{
	GET(TheaterType, theater, EAX);

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);
	return pTheater->IsArctic ?
		0x483E0C : 0x483CD4;
}

ASMJIT_PATCH(0x71C076, TerrainClass_ClearOccupyBit_Theater, 0x7)
{
	enum { setArticOccupy = 0x71C08D, setTemperatOccupy = 0x71C07F };

	if (lastTheater == TheaterType::None)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Scenario is negative idx , default to Temperate");
	}

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(lastTheater);
	return pTheater->IsArctic ?
		setArticOccupy : setTemperatOccupy;
}

ASMJIT_PATCH(0x71C116, TerrainClass_SetOccupyBit_Theater, 0x7)
{
	enum { setArticOccupy = 0x71C12D, setTemperatOccupy = 0x71C11F };
	GET(ScenarioClass*, pScen, EAX);

	if ((int)pScen->Theater == -1)
	{
		Debug::LogInfo(__FUNCTION__" Scenario is negative idx , default to Temperate");
		return setTemperatOccupy;
	}

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(pScen->Theater);
	return pTheater->IsArctic ?
		setArticOccupy : setTemperatOccupy;
}

ASMJIT_PATCH(0x47C30C, CellClass_CellColor_AdjustBrightness, 0x7)
{
	GET(TheaterType, nTheater, EAX);
	GET(ColorStruct*, pThatColor, ECX);
	LEA_STACK(ColorStruct*, pThisColor, STACK_OFFS(0x14, 0xC));

	R->EAX(pThisColor->AdjustBrightness(pThatColor, TheaterTypeClass::FindFromTheaterType_NoCheck(nTheater)->LowRadarBrightness1.Get()));
	return 0x47C329;
}

ASMJIT_PATCH(0x4758D4, CCINIClass_PutTheater_replace, 0x6)
{
	GET_STACK(TheaterType, nTheater, 0xC);
	R->EDX(TheaterTypeClass::FindFromTheaterType_NoCheck(nTheater)->Name.data());
	return 0x4758DA;
}

#pragma endregion

//RMG not fully supported YET !
ASMJIT_PATCH(0x5997B4, RMGClass_TheaterType_initRandomMap, 0x7)
{
	GET(TheaterType, nIndex, EAX);
	if ((size_t)nIndex == TheaterTypeClass::Array.size())
		nIndex = (TheaterType)0;

	R->ECX(TheaterTypeClass::FindFromTheaterType_NoCheck(nIndex)->Name.data());
	return 0x5997C6;
}

//DEFINE_SKIP_HOOK(0x6275B7, scheme_62759_ProcessOtherPalettes_RemoveThseCall , 0x7 , 627680);
DEFINE_JUMP(LJMP, 0x6275B7, 0x627680);

ASMJIT_PATCH(0x627699, TheaterTypeClass_ProcessOtherPalettes_Process, 0x6)
{
	GET_STACK(char*, pOriginalName, STACK_OFFS(0x424, -0x4));
	LEA_STACK(char*, pNameProcessed, STACK_OFFS(0x424, 0x400));

	auto theater = ScenarioClass::Instance->Theater;
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(theater);

	CRT::strcpy(pNameProcessed, pOriginalName);
	CRT::strcat(pNameProcessed, pTheater->Extension.data());
	CRT::strcat(pNameProcessed, GameStrings::DOT_SEPARATOR());
	CRT::strcat(pNameProcessed, GameStrings::PAL());
	CRT::strupr(pNameProcessed);

	const auto pFile = FakeFileLoader::_Retrieve(pNameProcessed, false);

	static std::map<std::string, bool> AlreadLogged {};

	if(Phobos::Otamaa::IsAdmin){
		auto& logged = AlreadLogged[pOriginalName];

		if (!pFile && !logged) {
		Debug::LogInfo("Failed to load [{}] as [{}] !", pOriginalName, pNameProcessed);
			logged = true;
		}
	}

	// cant use PaletteManager atm , because this will be modified after load done
	// so if PaletteManager used , that mean the color entries will get modified
	// for second time !
	R->EAX(pFile);
	return 0x6276A4;
}

#include <VeinholeMonsterClass.h>

// Picked from Ares custom theater branch
ASMJIT_PATCH(0x74D450, TheaterTypeClass_ProcessVeinhole, 0x7)
{
	GET(TheaterType, index, ECX);
	char buffer[32];
	CRT::sprintf(buffer, GameStrings::VEINHOLE_(), TheaterTypeClass::FindFromTheaterType_NoCheck(index)->Extension.c_str());
	VeinholeMonsterClass::VeinSHPData = (SHPFrame*)FakeFileLoader::_Retrieve(buffer, false);
	return 0x74D48A;
}

ASMJIT_PATCH(0x546C8B, IsometricTileTypeClass_ReadData_LunarLimitation, 0x8)
{
	GET_STACK(TheaterType, theater, 0xB4);
	return TheaterTypeClass::FindFromTheaterType_NoCheck(theater)->IsLunar ? 0x546C95 : 0x546CBF;
}

#undef CURRENT_THEATER

#endif