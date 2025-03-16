#include <New/Type/TheaterTypeClass.h>

#include <Phobos.h>

#include <Ext/Scenario/Body.h>
#include <MixFileClass.h>
#include <Utilities/Macro.h>

#define CURRENT_THEATER (*ScenarioClass::Instance).Theater

#ifndef DISABLE_THEATER_HOOKS
ASMJIT_PATCH(0x48DBE0, TheaterTypeClass_FindIndex, 0x5)
{
	GET(char*, nTheaterName, ECX);

	if (TheaterTypeClass::Array.empty())
		TheaterTypeClass::LoadAllTheatersToArray();

	R->EAX<int>(TheaterTypeClass::FindIndexById(nTheaterName));
	return 0x48DC12;
}

#pragma region IsoTileTypeHooks

ASMJIT_PATCH(0x54547F, IsometricTileTypeClass_ReadINI_SetPaletteISO, 0x6)
{
	LEA_STACK(char*, outBuffs, 0x6B0);
	LEA_STACK(CCFileClass*, file_c, 0xA10 - 0x668);

	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);

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
	const bool Exist = file_c->Exists();

	if (!Exist)
		GameDebugLog::Log("Failed to load IsometricTileTypeClass Palette %s For [%s]", outBuffs, pTheater->Name.data());
	else
	{
		if (file_c->Read(FileSystem::ISOx_PAL(), sizeof(BytePalette)))
		{
			GameDebugLog::Log("Loaded IsometricTileTypeClass Palette %s For [%s]", outBuffs, pTheater->Name.data());

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
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);

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
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x5452F8;
}

//here theater index is multiplied by `sizeof(Theater)` !
ASMJIT_PATCH(0x546662, IsometricTileTypeClass_TheaterType_makepath, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->IsometricTileTypeExtension ? pTheater->IsometricTileTypeExtension : pTheater->Extension).data());
	return 0x546668;
}

ASMJIT_PATCH(0x546753, IsometricTileTypeClass_TheaterType_MMx, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX(pTheater->MMExtension.data());
	return 0x546759;
}

ASMJIT_PATCH(0x546833, IsometricTileTypeClass_FallbackTheater, 0x5)
{
	GET(char*, pFileName, EDX);
	LEA_STACK(char*, pBuffer, STACK_OFFS(0x10, 0x2C0));
	GET_STACK(bool, bSomething, STACK_OFFS(0x10, 0x9FE));

	CRT::_makepath(pBuffer, 0, 0, pFileName, TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->FallbackTheaterExtension.data());
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

		auto pImage = FileSystem::LoadFile(_ext.c_str(), forceShp);
		if (!pImage)
		{
			_ext[1] = 'G';
			pImage = FileSystem::LoadFile(_ext.c_str(), forceShp);
		}

		pType->Image = static_cast<SHPStruct*>(pImage);
	}

	if (const auto pShp = pType->Image)
	{
		auto const size = std::max(pShp->Width, pShp->Height);
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

ASMJIT_PATCH(0x5349E3, ScenarioClass_InitTheater_Handle, 0x6)
{
	GET(TheaterType, nType, EDI);

	Debug::LogInfo("Init For Theater [{}]", (void*)nType);
	ScenarioClass::Instance->Theater = nType;
	typedef int(*wsprintfA_ptr)(LPSTR, LPCSTR, ...);
	GET(wsprintfA_ptr, pFunc, EBP);

	if (nType == TheaterType::None)
	{
		//for some stupid reason this return to invalid
		//that mean it not parsed properly ?
		Debug::LogInfo("TheaterType is invalid ! , fallback to Temperate!");
		ScenarioClass::Instance->Theater = TheaterType::Temperate;
		nType = TheaterType::Temperate;
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

	GameDebugLog::Log("Theater[%s] Mix [%s , %s , %s , %s , %s]", pTheater->Name.data(),
	pRootMix, pRootMixMD, pExpansionMixMD, pSuffixMix, pDataMix);

	// any errors triggered before this line are irrelevant
	// caused by reading the section while only certain flags from it are needed
	// and before other global lists are initialized
	Phobos::Otamaa::TrackParserErrors = true;

	Game::SetProgress(8);
	R->EBX(pTheater->ControlFileName.data());
	R->Stack(STACK_OFFS(0x6C, 0x58), pTheater->Extension.data());

	return 0x0534A68;
}

ASMJIT_PATCH(0x534A9D, ScenarioClass_initTheater_TheaterType_ArticCheck, 0x6)
{
	enum { AllocateMix = 0x534AA6, NextFunc = 0x534AD6 };
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	//this one usefull for loading additional mix files
	return pTheater->RootMixMD || pTheater->IsArctic ?
		AllocateMix : NextFunc;
}

#pragma endregion

#pragma region replacedMakepath
////AnimType
ASMJIT_PATCH(0x4279BB, AnimTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EDX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x4279C1;
}

ASMJIT_PATCH(0x427AF1, AnimTypeClass_TheaterSuffix_2, 0x5)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x427AF6;
}

ASMJIT_PATCH(0x428903, AnimTypeClass_TheaterSuffix_3, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428909;
}

ASMJIT_PATCH(0x428CBF, AnimTypeClass_TheaterSuffix_4, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EAX((pTheater->AnimTypeExtension ? pTheater->AnimTypeExtension : pTheater->Extension).data());
	return 0x428CC5;
}

//BuildingType
ASMJIT_PATCH(0x45E9FD, BuildingTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA03;
}

ASMJIT_PATCH(0x45EA60, BuildingTypeClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->BuildingTypeExtension ? pTheater->BuildingTypeExtension : pTheater->Extension).data());
	return 0x45EA66;
}

//OverlayType
ASMJIT_PATCH(0x5FE673, OverlayTypeClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FE679;
}

ASMJIT_PATCH(0x5FEB94, OverlayTypeClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEB9A;
}

ASMJIT_PATCH(0x5FEE42, OverlayTypeClass_TheaterSuffix_3, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->EDX((pTheater->OverlayTypeExtension ? pTheater->OverlayTypeExtension : pTheater->Extension).data());
	return 0x5FEE48;
}

//SmudgeTypes
ASMJIT_PATCH(0x6B54CF, SmudgeTypesClass_TheaterSuffix_1, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B54D5;
}

ASMJIT_PATCH(0x6B57A7, SmudgeTypesClass_TheaterSuffix_2, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->SmudgeTypeExtension ? pTheater->SmudgeTypeExtension : pTheater->Extension).data());
	return 0x6B57AD;
}

//TerrainType
ASMJIT_PATCH(0x71DCE4, TerrainTypeClass_TheaterSuffix, 0x6)
{
	const auto pTheater = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER);
	R->ECX((pTheater->TerrainTypeExtension ? pTheater->TerrainTypeExtension : pTheater->Extension).data());
	return 0x71DCEA;
}

//objectType , this maybe alredy overriden by ares
ASMJIT_PATCH(0x5F915C, ObjectTypeClass_TheaterSuffix_3, 0x6)
{
	R->EDX(TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->Extension.data());
	return 0x5F9162;
}

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

	CRT::strcpy(pNameProcessed, pOriginalName);
	CRT::strcat(pNameProcessed, TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->Extension.data());
	CRT::strcat(pNameProcessed, GameStrings::DOT_SEPARATOR());
	CRT::strcat(pNameProcessed, GameStrings::PAL());
	CRT::strupr(pNameProcessed);

	const auto pFile = MixFileClass::Retrieve(pNameProcessed, false);
	//const auto nRest = !pFile ? "Failed to" : "Successfully";

	if (!pFile && Phobos::Otamaa::IsAdmin)
		Debug::LogInfo("Failed to load [{}] as [{}] !", pOriginalName, pNameProcessed);

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
	VeinholeMonsterClass::VeinSHPData = (SHPFrame*)MixFileClass::Retrieve(buffer);
	return 0x74D48A;
}

ASMJIT_PATCH(0x534CA9, Init_Theaters_SetPaletteUnit, 0x8)
{
	if (auto const& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->PaletteUnit)
	{
		R->ESI(MixFileClass::Retrieve(data.c_str(), false));
		return 0x534CCA;
	}

	return 0x0;
}

ASMJIT_PATCH(0x534BEE, ScenarioClass_initTheater_TheaterType_OverlayPalette, 0x5)
{
	if (const auto& data = TheaterTypeClass::FindFromTheaterType_NoCheck(CURRENT_THEATER)->PaletteOverlay)
	{
		R->EAX(MixFileClass::Retrieve(data.c_str(), false));
		return 0x534C09;
	}

	return 0x0;
}

ASMJIT_PATCH(0x546C8B, IsometricTileTypeClass_ReadData_LunarLimitation, 0x8)
{
	GET_STACK(TheaterType, theater, 0xB4);
	return TheaterTypeClass::FindFromTheaterType_NoCheck(theater)->IsLunar ? 0x546C95 : 0x546CBF;
}

#undef CURRENT_THEATER

#endif