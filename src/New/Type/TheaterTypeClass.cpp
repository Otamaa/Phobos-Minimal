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
	}
}

TheaterTypeClass* TheaterTypeClass::FindFromTheaterType(TheaterType nType)
{
	return nType != TheaterType::None && (size_t)nType < Array.size() ?
		Array[(int)nType].get() : nullptr;
}

#define CURRENT_THEATER (*ScenarioClass::Instance).Theater

//TODO : 
// Ares CustomPalette using the array adress to get stuffs , if we put index that more than those 
// it is possible it will cause out of bound error
namespace Log
{
	bool bEnableLog = false;

	void Exec(const char* pFunct, TheaterType nTheater)
	{
		if (bEnableLog)
		{
			Debug::Log("%s Executed! for [%d = %s] \n", pFunct, nTheater, 
				TheaterTypeClass::FindFromTheaterType(nTheater)->Name.data());
		}
	}

	void Exec(const char* pFunct, int nIdx, const char* pTheaterName)
	{
		if (bEnableLog)
			Debug::Log("%s Executed! for [%d = %s] \n", pFunct, nIdx, pTheaterName);
	}
}

DEFINE_HOOK(0x48DBE0, TheaterTypeClass_FindIndex, 0x0)
{
	GET(char*, nTheaterName, ECX);

	auto nIdx = TheaterTypeClass::FindIndexById(nTheaterName);

	Log::Exec(__FUNCTION__, nIdx, nTheaterName);

	R->EAX<int>(nIdx);
	return 0x48DC12;
}

#pragma region IsoTileTypeHooks

DEFINE_HOOK(0x5452F2, IsometricTileTypeClass_TheaterType_Slope, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	return 0x5452F8;
}

//Isometric pal = ISO+Extension.pal
DEFINE_HOOK(0x54547F, IsometricTileTypeClass_TheaterType_ISOPal, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	return 0x545485;
}

//Ini  file = ControlFilename+MD.ini
DEFINE_HOOK(0x5454F0, IsometricTileTypeClass_TheaterType_IniFile, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EDX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->ControlFileName.data());
	return 0x5454F6;
}

//here theater index is multiplied by `sizeof(Theater` !
DEFINE_HOOK(0x546662, IsometricTileTypeClass_TheaterType_makepath, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	return 0x546668;
}

DEFINE_HOOK(0x546753, IsometricTileTypeClass_TheaterType_MMx, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->MMExtension.data());
	return 0x546759;
}

namespace _ReplaceMakePath
{
	void __cdecl Exec(char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5 //this were real theater input
	)
	{
		Log::Exec(__FUNCTION__, CURRENT_THEATER);
		CRT::_makepath(arg1, arg2, arg3, arg4, TheaterTypeClass::Array[(int)TheaterType::Temperate]->Extension.data());
	}

	void __cdecl Exec_urb(char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5)
	{
		Log::Exec(__FUNCTION__, CURRENT_THEATER);
		CRT::_makepath(arg1, arg2, arg3, arg4, TheaterTypeClass::Array[(int)TheaterType::Urban]->Extension.data());
	}

	void __cdecl Exec2(char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5)
	{
		Log::Exec(__FUNCTION__, CURRENT_THEATER);
		CRT::_makepath(arg1, arg2, arg3, arg4, TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	}
}
//
////these were fallback theater if some file doest exist
//DEFINE_JUMP(CALL, 0x546843, GET_OFFSET(_ReplaceMakePath::Exec2)); //roadtile tem
// Marble madness 546793 // tem
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

	Log::Exec(__FUNCTION__, CURRENT_THEATER);
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

	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	auto const scenarioTheater = ScenarioClass::Instance->Theater;
	auto const& TheaterData = TheaterTypeClass::FindFromTheaterType(scenarioTheater);

	TechnoTypeExt::ExtData* pTypeData = nullptr;

	if (TechnoTypeClass* const pThisTechno = type_cast<TechnoTypeClass* const>(pType))
	{
		pTypeData = TechnoTypeExt::ExtMap.Find(reinterpret_cast<TechnoTypeClass*>(pType));
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

	auto const pExt = (pType->Theater ? TheaterData->Extension.data() : "SHP");
	IMPL_SNPRNINTF(basename, sizeof(basename), "%s.%s", pType->ImageFile, pExt);

	if (!pType->Theater && pType->NewTheater && scenarioTheater != TheaterType::None)
	{
		if (isalpha(static_cast<unsigned char>(basename[0])))
		{
			// evil hack to uppercase
			auto const c1 = static_cast<unsigned char>(basename[1]) & ~0x20;
			if (c1 == 'A' || c1 == 'T')
			{
				basename[1] = TheaterData->Letter[0];
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

	Log::Exec(__FUNCTION__, CURRENT_THEATER);

	if (Theater != TheaterType::None)
	{
		char c0 = basename[0];
		char c1 = basename[1] & ~0x20; // evil hack to uppercase
		if (isalpha(static_cast<unsigned char>(c0)))
		{
			if (c1 == 'A' || c1 == 'T')
			{
				basename[1] = TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Letter.data()[0];
			}
		}
	}
	return 0x5F9702;
}
#pragma endregion

#pragma region ScenarioClass_InitTheater

//How mix loaded ? 
// controlfilename.mix
// ArtFileName.mix
// Extension.mix
// PaletteFileName+md.mix
// controlfilename+md.mix

#include <Ext/Scenario/Body.h>

DEFINE_HOOK(0x5349E3, ScenarioClass_InitTheater_TheaterType_ControlFileName, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(TheaterType, nType, EDI);
	R->EBX(TheaterTypeClass::FindFromTheaterType(nType)->ControlFileName.data());
	return 0x5349E9;
}

DEFINE_HOOK(0x5349F2, ScenarioClass_InitTheater_TheaterType_ArtMix, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->ECX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->ArtFileName.data());
	return 0x5349F8;
}

DEFINE_HOOK(0x534A05, ScenarioClass_InitTheater_TheaterType_Control, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	return 0x534A0B;
}

DEFINE_HOOK(0x534A1C, ScenarioClass_initTheater_TheaterType_MdMix, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->ECX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->PaletteFileName.data());
	return 0x534A22;
}

DEFINE_HOOK(0x534A9D, ScenarioClass_initTheater_TheaterType_ArticCheck, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	return TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->IsArctic ? 0x534AA6 : 0x534AD6;
}

DEFINE_HOOK(0x534A3C, ScenarioClass_initTheater_TheaterType_Name, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->ECX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Name.data());
	return 0x534A42;
}

#pragma endregion

#pragma region replacedMakepath
//AnimType
DEFINE_JUMP(CALL, 0x4279CC, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x427B07, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x428910, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x428CCC, GET_OFFSET(_ReplaceMakePath::Exec2));

//BuildingType 
DEFINE_JUMP(CALL, 0x45EA08, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x45EA6B, GET_OFFSET(_ReplaceMakePath::Exec2));

//OverlayType
DEFINE_JUMP(CALL, 0x5FE67E, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x5FEBA1, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x5FEE4F, GET_OFFSET(_ReplaceMakePath::Exec2));

//SmudgeTypes
DEFINE_JUMP(CALL, 0x6B54DC, GET_OFFSET(_ReplaceMakePath::Exec2));
DEFINE_JUMP(CALL, 0x6B57B4, GET_OFFSET(_ReplaceMakePath::Exec2));

//TerrainType 
DEFINE_JUMP(CALL, 0x71DCEF, GET_OFFSET(_ReplaceMakePath::Exec2));

//objectType
DEFINE_JUMP(CALL, 0x5F9169, GET_OFFSET(_ReplaceMakePath::Exec2));

#pragma endregion

#pragma region CellAndTerrainStuffs
DEFINE_HOOK(0x483DE5, CellClass_CheckPassability_Artict, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	return TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->IsArctic ? 0x483E0C : 0x483DF5;
}

DEFINE_HOOK(0x6DAE3E, TacticalClass_DrawWaypoints_SelectColor, 0x8)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(ScenarioClass*, pScen, ECX);
	R->EAX(TheaterTypeClass::FindFromTheaterType(pScen->Theater)->IsArctic ? 12 : 14);
	return 0;
}

DEFINE_HOOK(0x71C076, TerrainClass_ClearOccupyBit_Theater, 0x7)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(ScenarioClass*, pScen, EAX);
	return TheaterTypeClass::FindFromTheaterType(pScen->Theater)->IsArctic ? 0x71C08D : 0x71C07F;
}

DEFINE_HOOK(0x71C076, TerrainClass_SetOccupyBit_Theater, 0x7)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(ScenarioClass*, pScen, EAX);
	return TheaterTypeClass::FindFromTheaterType(pScen->Theater)->IsArctic ? 0x71C12D : 0x71C11F;
}

DEFINE_HOOK(0x47C318, CellClass_ClellColor_AdjustBrightness, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	R->EAX(TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->LowRadarBrightness1.Get());
	return 0x47C31E;
}

DEFINE_HOOK(0x4758D4, CCINIClass_PutTheater_replace, 0x6)
{
	GET_STACK(TheaterType, nTheater, 0xC);
	const auto nIdend = TheaterTypeClass::FindFromTheaterType(nTheater)->Name.data();
	Debug::Log(__FUNCTION__" Exec Theater[%s] \n", nIdend);
	R->EDX(nIdend);
	return 0x4758DA;
}
#pragma endregion

DEFINE_HOOK(0x5997C0, RMGClass_TheaterType_initRandomMap, 0x6)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(TheaterType, nIndex, EAX);
	R->ECX(TheaterTypeClass::FindFromTheaterType(nIndex)->Name.data());
	return 0x5997C6;
}

DEFINE_JUMP(LJMP, 0x6275B7, 0x627680);
//DEFINE_HOOK(0x6275B7, TheaterTypeClass_ProcessOtherPalettes_screwthese, 0x6) { return 0x627680; }

#include <MixFileClass.h>

DEFINE_HOOK(0x627699, TheaterTypeClass_ProcessOtherPalettes_Process, 0x6)
{
	GET_STACK(char*, pOriginalName, STACK_OFFS(0x424, -0x4));
	LEA_STACK(char*, pNameProcessed, STACK_OFFS(0x424, 0x400));

	Log::Exec(__FUNCTION__, CURRENT_THEATER);

	CRT::strcpy(pNameProcessed, pOriginalName);
	CRT::strcat(pNameProcessed, TheaterTypeClass::FindFromTheaterType(CURRENT_THEATER)->Extension.data());
	CRT::strcat(pNameProcessed, ".");
	CRT::strcat(pNameProcessed,"PAL");
	CRT::strupr(pNameProcessed);

	const auto pFile = MixFileClass::Retrieve(pNameProcessed, false);
	if (!pFile)
		Debug::Log("Failed to load %s File ! \n", pNameProcessed);

	R->EAX(pFile);
	return 0x6276A4;
}

// Picked from Ares custom theater branch
DEFINE_HOOK(0x74D45A, TheaterTypeClass_ProcessVeinhole, 0x0)
{
	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	GET(TheaterType, index, ECX);
	R->EAX(TheaterTypeClass::FindFromTheaterType(index)->Extension.data());
	R->ECX<DWORD>(R->ESP());
	return 0x74D468;
}
#undef CURRENT_THEATER