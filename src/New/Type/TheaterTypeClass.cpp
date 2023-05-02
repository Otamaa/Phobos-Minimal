#include "TheaterTypeClass.h"

#include <Phobos.h>

Enumerable<TheaterTypeClass>::container_t Enumerable<TheaterTypeClass>::Array;

const char* Enumerable<TheaterTypeClass>::GetMainSection()
{
	return "TheaterTypes";
}

void TheaterTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name.data();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exIni(pINI);

	UIName.Read(exIni, pSection, "UIName");
	ControlFileName.Read(pINI, pSection, "ControlFileName");
	ArtFileName.Read(pINI, pSection, "ArtFileName");
	PaletteFileName.Read(pINI, pSection, "PaletteFileName");
	Extension.Read(pINI, pSection, "Extension");
	MMExtension.Read(pINI, pSection, "MMExtension");
	Letter.Read(pINI, pSection, "ImageLetter");
	IsArctic.Read(exIni, pSection, "IsArtic");
	IsAllowedInMapGenerator.Read(exIni, pSection, "IsAllowedInMapGenerator");
	LowRadarBrightness1.Read(exIni, pSection, "LowRadarBrightness");

	PaletteUnit.Read(pINI, pSection, "PaletteUnit");
	PaletteISO.Read(pINI, pSection, "IsometricPalette");
	TerrainControl.Read(pINI, pSection, "TerrainControl");
	PaletteOverlay.Read(pINI, pSection, "PaletteOverlay");

	RootMix.Read(pINI, pSection, "ControlMix");
	RootMixMD.Read(pINI, pSection, "ControlMDMix");
	ExpansionMDMix.Read(pINI, pSection, "ExpansionMDMix");
	SuffixMix.Read(pINI, pSection, "ExtensionMix");
	DataMix.Read(pINI, pSection, "ArtMix");
	TerrainTypeExtension.Read(pINI, pSection, "TerrainTypeExtension");
	SmudgeTypeExtension.Read(pINI, pSection, "SmudgeTypeExtension");
	AnimTypeExtension.Read(pINI, pSection, "AnimTypeExtension");
	OverlayTypeExtension.Read(pINI, pSection, "OverlayTypeExtension");
	IsometricTileTypeExtension.Read(pINI, pSection, "IsometricTileTypeExtension");
	BuildingTypeExtension.Read(pINI, pSection, "BuildingTypeExtension");
	FallbackTheaterExtension.Read(pINI, pSection, "FallbackTheaterExtension");
}

bool TheaterTypeClass::IsDefaultTheater()
{
	for (size_t i = 0; i < 6; ++i)
	{
		if (IS_SAME_STR_(Name.data(), Theater::Array[i].Identifier))
			return true;
	}

	return false;
}

CCINIClass* TheaterTypeClass::GetConfigINI()
{
	const auto nFileName = "Theaters.ini";

	if (CCINIClass* pINI = GameCreate<CCINIClass>())
	{
		if (auto pTheaterIni = GameCreate<CCFileClass>(nFileName))
		{
			if (pTheaterIni->Exists())
			{
				pINI->ReadCCFile(pTheaterIni);
				return pINI;
			}
			else
			{
				Debug::Log("%s not found!\n", nFileName);
			}
		}
	}

	return nullptr;
}

void TheaterTypeClass::LoadAllTheatersToArray()
{
	TheaterTypeClass::AddDefaults();

	if (auto pINI = GetConfigINI())
	{
		const char* pSection = TheaterTypeClass::GetMainSection();
		if (pINI->GetSection(pSection))
		{
			const auto nKeyCount = pINI->GetKeyCount(pSection);

			for (int i = 0; i < nKeyCount; ++i)
			{
				if (pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer))
				{
					if (auto pTheater = FindOrAllocate(Phobos::readBuffer))
						pTheater->LoadFromINI(pINI);
					else
						Debug::Log("Error Reading %s \"%s\".\n", pSection, Phobos::readBuffer);
				}
			}
		}

		GameDelete<true, false>(pINI);
		pINI = nullptr;
	}
}

void TheaterTypeClass::AddDefaults()
{
	if (Array.empty())
		Array.reserve(Theater::Array.size());

	for (size_t i = 0; i < Theater::Array.size(); ++i)
	{
		auto pTheater = Allocate(Theater::Array[i].Identifier);
		Debug::Log("Allocating Default Theater [%s] ! \n", Theater::Array[i].Identifier);
		pTheater->UIName = Theater::Array[i].UIName;
		pTheater->ControlFileName = Theater::Array[i].ControlFileName;
		pTheater->ArtFileName = Theater::Array[i].ArtFileName;
		pTheater->Extension = Theater::Array[i].Extension;
		pTheater->PaletteFileName = Theater::Array[i].PaletteFileName;
		pTheater->MMExtension = Theater::Array[i].MMExtension;
		pTheater->Letter = Theater::Array[i].Letter;
		pTheater->IsArctic = i == (size_t)TheaterType::Snow;
		const bool bDisallowed = i == (size_t)TheaterType::NewUrban || i == (size_t)TheaterType::Lunar;
		pTheater->IsAllowedInMapGenerator = !bDisallowed;
		pTheater->LowRadarBrightness1 = Theater::Array[i].RadarTerrainBrightness;
		pTheater->HighRadarBrightness = Theater::Array[i].RadarTerrainBrightnessAtMaxLevel;
		pTheater->unknown_float_60 = Theater::Array[i].unknown_float_60;
		pTheater->unknown_float_64 = Theater::Array[i].unknown_float_64;
		pTheater->unknown_int_68 = Theater::Array[i].unknown_int_68;
		pTheater->unknown_int_6C = Theater::Array[i].unknown_int_6C;
		pTheater->FallbackTheaterExtension = "TEM";
	}
}

TheaterTypeClass* TheaterTypeClass::FindFromTheaterType(TheaterType nType)
{
	return nType != TheaterType::None && (size_t)nType < Array.size() ?
		Array[(int)nType].get() : Array[0].get();
}

#define CURRENT_THEATER (*ScenarioClass::Instance).Theater

void TheaterTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	//Debug::Log("Loading TheaterTypeClass ! \n");
	//this->Swizzle(Stm);
}

void TheaterTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	//this->Swizzle(Stm);
}

DEFINE_HOOK(0x48DBE0, TheaterTypeClass_FindIndex, 0x0)
{
	GET(char*, nTheaterName, ECX);

	R->EAX<int>(TheaterTypeClass::FindIndexById(nTheaterName));
	return 0x48DC12;
}

#pragma region IsoTileTypeHooks

DEFINE_HOOK(0x54547F, IsometricTileTypeClass_ReadINI_SetPaletteISO, 0x6)
{
	auto& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->PaletteISO;
	if (data)
	{
		R->ECX<char*>(data.data());
		return 0x5454A2;
	}

	return 0x0;
}

DEFINE_HOOK(0x5454F0, IsometricTileTypeClass_ReadINI_TerrainControl, 0x6)
{
	auto& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->TerrainControl;
	if (data)
	{
		R->ECX<char*>(data.data());
		return 0x545513;
	}

	return 0x0;
}

DEFINE_HOOK(0x5452F2, IsometricTileTypeClass_TheaterType_Slope, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x5452F8;
}

//Isometric pal = ISO+Extension.pal
DEFINE_HOOK(0x54547F, IsometricTileTypeClass_TheaterType_ISOPal, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x545485;
}

//Ini  file = ControlFilename+MD.ini
DEFINE_HOOK(0x5454F0, IsometricTileTypeClass_TheaterType_IniFile, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EDX(pTheater->ControlFileName.data());
	return 0x5454F6;
}

//here theater index is multiplied by `sizeof(Theater)` !
DEFINE_HOOK(0x546662, IsometricTileTypeClass_TheaterType_makepath, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x546668;
}

DEFINE_HOOK(0x546753, IsometricTileTypeClass_TheaterType_MMx, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX(pTheater->MMExtension.data());
	return 0x546759;
}

DEFINE_HOOK(0x546833, IsometricTileTypeClass_FallbackTheater , 0x5)
{
	GET(char*, pFileName, EDX);
	LEA_STACK(char*, pBuffer, STACK_OFFS(0x10, 0x2C0));
	GET_STACK(bool, bSomething, STACK_OFFS(0x10, 0x9FE));
	
	CRT::_makepath(pBuffer, 0, 0, pFileName , TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->FallbackTheaterExtension.data());
	R->AL(bSomething);
	return 0x54684F;
}
//DEFINE_JUMP(CALL, 0x54692B, GET_OFFSET(_ReplaceMakePath::Exec2)); //roadtile urb
#pragma endregion


#pragma region AresHooks
#include <Utilities/Cast.h>
#include <Ext/TechnoType/Body.h>
#include <SmudgeTypeClass.h>
#include <TerrainTypeClass.h>

DEFINE_OVERRIDE_HOOK(0x5F9634, ObjectTypeClass_LoadFromINI, 6)
{
	GET(ObjectTypeClass*, pType, EBX);
	GET_STACK(CCINIClass*, pINI, STACK_OFFS(0x1B0, -4));

	if (TechnoTypeClass* const pTechnoType = type_cast<TechnoTypeClass* const>(pType))
	{
		INI_EX exINI(pINI);
		TechnoTypeExt::ExtMap.Find(pTechnoType)->AlternateTheaterArt.Read(exINI, pType->ID, "AlternateTheaterArt");
	}

	return 0;
}

// SHP file loading
DEFINE_OVERRIDE_HOOK(0x5F9070, ObjectTypeClass_Load2DArt, 6)
{
	GET(ObjectTypeClass* const, pType, ECX);

	auto const scenarioTheater = ScenarioClass::Instance->Theater;
	auto const& TheaterData = TheaterTypeClass::FindFromTheaterType_NoCheck(scenarioTheater);

	TechnoTypeExt::ExtData* pTypeData = nullptr;

	if (TechnoTypeClass* const pThisTechno = type_cast<TechnoTypeClass* const>(pType)) {
		pTypeData = TechnoTypeExt::ExtMap.Find(pThisTechno);
	}

	char basename[MAX_PATH];

	// extension object is not present if not techno type
	if (pTypeData && pTypeData->AlternateTheaterArt)
	{
		if (!pType->ArcticArtInUse)
		{ // this flag is not used anywhere outside this function, so I'll just hijack it
			pType->ArcticArtInUse = true;
			IMPL_SNPRNINTF(basename, sizeof(basename), "%s%s", pType->ImageFile, TheaterData->Letter.data());
			if (!CCINIClass::INI_Art->GetSection(basename))
			{
				pType->ArcticArtInUse = false;
				IMPL_SNPRNINTF(basename, sizeof(basename), "%s", pType->ImageFile);
			}

			PhobosCRT::strCopy(pType->ImageFile, basename);
		}
	}
	else if (pType->AlternateArcticArt && TheaterData->IsArctic && !pType->ImageAllocated)
	{
		if (!pType->ArcticArtInUse)
		{
			IMPL_SNPRNINTF(basename, sizeof(basename), "%sA", pType->ImageFile);
			PhobosCRT::strCopy(pType->ImageFile, basename);
			pType->ArcticArtInUse = true;
		}
	}
	else
	{
		pType->ArcticArtInUse = false;
	}

	auto const pExt = (pType->Theater ? TheaterData->Extension.c_str() : "SHP");
	IMPL_SNPRNINTF(basename, sizeof(basename), "%s.%s", pType->ImageFile, pExt);

	if (!pType->Theater && pType->NewTheater && scenarioTheater != TheaterType::None)
	{
		if (isalpha(static_cast<unsigned char>(basename[0])))
		{
			// evil hack to uppercase
			auto const c1 = static_cast<unsigned char>(basename[1]) & ~0x20;
			if (c1 == 'A' || c1 == 'T')
			{
				basename[1] = TheaterData->Letter.data()[0];
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
	const bool IsTerrainOrSmudge = VTable::Get(pType) == SmudgeTypeClass::vtable || Is_TerrainType(pType);

	if (!IsTerrainOrSmudge)
	{
		const auto forceShp = VTable::Get(pType) == OverlayTypeClass::vtable || Is_AnimType(pType);

		auto pImage = FileSystem::LoadFile(basename, forceShp);
		if (!pImage)
		{
			basename[1] = 'G';
			pImage = FileSystem::LoadFile(basename, forceShp);
		}

		pType->Image = static_cast<SHPStruct*>(pImage);
	}

	if (const auto pShp = pType->Image)
	{
		auto const& size = std::max(pShp->Width, pShp->Height);
		pType->MaxDimension = std::max(size, static_cast<short>(8));
	}

	return 0x5F92C3;
}

DEFINE_OVERRIDE_HOOK(0x5F96B0, ObjectTypeClass_TheaterSpecificID, 6)
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
				basename[1] = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)
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

#include <Ext/Scenario/Body.h>
#include <Misc/AresData.h>

DEFINE_HOOK(0x5349E3, ScenarioClass_InitTheater_Handle, 0x6)
{
	GET(TheaterType, nType, EDI);
	ScenarioClass::Instance->Theater = nType;
	typedef int(*wsprintfA_ptr)(LPSTR, LPCSTR, ...);
	GET(wsprintfA_ptr, pFunc, EBP);

	const auto& pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(nType);

	// buffer size 16
	LEA_STACK(char*, pRootMix, STACK_OFFS(0x6C, 0x50));

	if (!pTheater->RootMix)
		pFunc(pRootMix, "%s.MIX", pTheater->ControlFileName.data());
	else
		CRT::strcpy(pRootMix, pTheater->RootMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pRootMixMD, STACK_OFFS(0x6C, 0x40));

	if (!pTheater->RootMixMD)
		pFunc(pRootMixMD, "%sMD.MIX", pTheater->ControlFileName.data());
	else
		CRT::strcpy(pRootMixMD, pTheater->RootMixMD.c_str());

	// buffer size 16
	LEA_STACK(char*, pExpansionMixMD, STACK_OFFS(0x6C, 0x20));

	if (!pTheater->ExpansionMDMix)
		pFunc(pExpansionMixMD, "%sMD.MIX", pTheater->PaletteFileName.data());
	else
		CRT::strcpy(pExpansionMixMD, pTheater->ExpansionMDMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pSuffixMix, STACK_OFFS(0x6C, 0x30));

	if (!pTheater->SuffixMix)
		pFunc(pSuffixMix, "%s.MIX", pTheater->Extension.data());
	else
		CRT::strcpy(pSuffixMix, pTheater->SuffixMix.c_str());

	// buffer size 16
	LEA_STACK(char*, pDataMix, STACK_OFFS(0x6C, 0x10));

	if (!pTheater->DataMix)
		pFunc(pDataMix, "%s.MIX", pTheater->ArtFileName.data());
	else
		CRT::strcpy(pDataMix, pTheater->DataMix.c_str());
	
	GameDebugLog::Log("Theater[%s] Mix [%s , %s , %s , %s , %s]\n", pTheater->Name.data() ,
	pRootMix , pRootMixMD, pExpansionMixMD, pSuffixMix, pDataMix );

	// any errors triggered before this line are irrelevant
	// caused by reading the section while only certain flags from it are needed
	// and before other global lists are initialized
	Debug_bTrackParseErrors = true;
	Game::SetProgress(8);
	R->EBX(pTheater->ControlFileName.data());
	R->Stack(STACK_OFFS(0x6C, 0x58), pTheater->Extension.data());
	
	return 0x0534A68;
}

DEFINE_HOOK(0x534A9D, ScenarioClass_initTheater_TheaterType_ArticCheck, 0x6)
{
	enum { AllocateMix = 0x534AA6 , NextFunc = 0x534AD6 };
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	//this one usefull for loading additional mix files
	return pTheater->RootMixMD || pTheater->IsArctic ?
		AllocateMix : NextFunc;
}

#pragma endregion


#pragma region replacedMakepath
//AnimType
DEFINE_HOOK(0x4279BB, AnimTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EDX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x4279C1;
}

DEFINE_HOOK(0x427AF1, AnimTypeClass_TheaterSuffix_2, 0x5)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x427AF6;
}

DEFINE_HOOK(0x428903, AnimTypeClass_TheaterSuffix_3, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428909;
}

DEFINE_HOOK(0x428CBF, AnimTypeClass_TheaterSuffix_4, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428CC5;
}

//BuildingType 
DEFINE_HOOK(0x45E9FD, BuildingTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA03;
}

DEFINE_HOOK(0x45EA60, BuildingTypeClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA66;
}

//OverlayType
DEFINE_HOOK(0x5FE673, OverlayTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FE679;
}

DEFINE_HOOK(0x5FEB94, OverlayTypeClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEB9A;
}

DEFINE_HOOK(0x5FEE42, OverlayTypeClass_TheaterSuffix_3, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EDX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEE48;
}

//SmudgeTypes
DEFINE_HOOK(0x6B54CF, SmudgeTypesClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B54D5;
}

DEFINE_HOOK(0x6B57A7, SmudgeTypesClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B57AD;
}

//TerrainType 
DEFINE_HOOK(0x71DCE4, TerrainTypeClass_TheaterSuffix, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->TerrainTypeExtension ? pTheater->TerrainTypeExtension : pTheater->Extension).data());
	return 0x71DCEA;
}

//objectType , this maybe alredy overriden by ares
DEFINE_HOOK(0x5F915C, ObjectTypeClass_TheaterSuffix_3, 0x6)
{
	R->EDX(TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->Extension.data());
	return 0x5F9162;
}

#pragma endregion

#pragma region CellAndTerrainStuffs
DEFINE_HOOK(0x483DE5, CellClass_CheckPassability_Artict, 0x6)
{
	return TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->IsArctic ? 0x483E0C : 0x483DF5;
}

DEFINE_HOOK(0x6DAE3E, TacticalClass_DrawWaypoints_SelectColor, 0x8)
{
	GET(ScenarioClass*, pScen, ECX);
	R->EAX(TheaterTypeClass::FindFromTheaterType_NoCheck(pScen->Theater)->IsArctic ? 12 : 14);
	return 0;
}

DEFINE_HOOK(0x71C076, TerrainClass_ClearOccupyBit_Theater, 0x7)
{
	GET(ScenarioClass*, pScen, EAX);
	return TheaterTypeClass::FindFromTheaterType_NoCheck(pScen->Theater)->IsArctic ? 0x71C08D : 0x71C07F;
}

DEFINE_HOOK(0x71C076, TerrainClass_SetOccupyBit_Theater, 0x7)
{
	GET(ScenarioClass*, pScen, EAX);
	return TheaterTypeClass::FindFromTheaterType_NoCheck(pScen->Theater)->IsArctic ? 0x71C12D : 0x71C11F;
}

DEFINE_HOOK(0x47C30C, CellClass_CellColor_AdjustBrightness, 0x7)
{
	GET(TheaterType, nTheater, EAX);
	GET(ColorStruct*, pThatColor, ECX);
	LEA_STACK(ColorStruct*, pThisColor, STACK_OFFS(0x14, 0xC));

	R->EAX(pThisColor->AdjustBrightness(pThatColor , TheaterTypeClass::FindFromTheaterType_NoCheck(nTheater)->LowRadarBrightness1.Get()));
	return 0x47C329;
}

DEFINE_HOOK(0x4758D4, CCINIClass_PutTheater_replace, 0x6)
{
	GET_STACK(TheaterType, nTheater, 0xC);
	R->EDX(TheaterTypeClass::FindFromTheaterType_NoCheck(nTheater)->Name.data());
	return 0x4758DA;
}
#pragma endregion

//RMG not fully supported YET !
DEFINE_HOOK(0x5997C0, RMGClass_TheaterType_initRandomMap, 0x6)
{
	GET(TheaterType, nIndex, EAX);
	R->ECX(TheaterTypeClass::FindFromTheaterType_NoCheck(nIndex)->Name.data());
	return 0x5997C6;
}


DEFINE_JUMP(LJMP, 0x6275B7, 0x627680);

#include <MixFileClass.h>

DEFINE_HOOK(0x627699, TheaterTypeClass_ProcessOtherPalettes_Process, 0x6)
{
	GET_STACK(char*, pOriginalName, STACK_OFFS(0x424, -0x4));
	LEA_STACK(char*, pNameProcessed, STACK_OFFS(0x424, 0x400));

	CRT::strcpy(pNameProcessed, pOriginalName);
	CRT::strcat(pNameProcessed, TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->Extension.data());
	CRT::strcat(pNameProcessed, ".");
	CRT::strcat(pNameProcessed,"PAL");
	CRT::strupr(pNameProcessed);

	Debug::Log("Loading [%s] as [%s] !\n", pOriginalName, pNameProcessed);
	// cant use PaletteManager atm , because this will be modified after load done 
	// so if PaletteManager used , that mean the color enries will get modified 
	// for second time !
	R->EAX(MixFileClass::Retrieve(pNameProcessed , false));
	return 0x6276A4;
}

// Picked from Ares custom theater branch
DEFINE_HOOK(0x74D45A, TheaterTypeClass_ProcessVeinhole, 0x0)
{
	GET(TheaterType, index, ECX);
	R->EAX(TheaterTypeClass::FindFromTheaterType_NoCheck(index)->Extension.data());
	R->ECX<DWORD>(R->ESP());
	return 0x74D468;
}

DEFINE_HOOK(0x534CA9, Init_Theaters_SetPaletteUnit, 0x8)
{
	auto const& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->PaletteUnit;
	if(data){
		R->ESI(MixFileClass::Retrieve(data.c_str(), false));
		return 0x534CCA;
	}

	return 0x0;
}

DEFINE_HOOK(0x534BEE, ScenarioClass_initTheater_TheaterType_OverlayPalette, 0x5)
{
	const auto& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->PaletteOverlay;
	if (data) {
		R->EAX(MixFileClass::Retrieve(data.c_str(), false));
		return 0x534C09;
	}

	return 0x0;
}

#undef CURRENT_THEATER