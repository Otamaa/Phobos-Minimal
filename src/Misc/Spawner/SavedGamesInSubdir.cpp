/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#include "Main.h"
#include "SavedGamesInSubdir.h"
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <filesystem>
#include <HouseClass.h>
#include <LoadOptionsClass.h>
#include <optional>

static inline std::string save_gamePath;

#define SET_SAVENAME(name) static COMPILETIMEEVAL wchar_t* SaveName = L ##name

int SavedGames::HowManyTimesISavedForThisScenario = 0;

bool SavedGames::CreateSubdir()
{
	if (!std::filesystem::exists(SpawnerMain::GetGameConfigs()->SavedGameDir))
	{
		Debug::LogInfo("[Spawner] Folder Saved Games does not exist, creating...");
		std::error_code ec;
		std::filesystem::create_directories(SpawnerMain::GetGameConfigs()->SavedGameDir, ec);
		if (ec)
		{
			Debug::LogInfo("\tCannot create folder Saved Games {} !", ec.message());
			return false;
		}
		Debug::LogInfo("\tDone.");
	}
	return true;
}

char* SavedGames::FormatPath(const char* pFileName)
{
	save_gamePath = SpawnerMain::GetGameConfigs()->SavedGameDir;
	save_gamePath += "\\";
	save_gamePath += pFileName;
	return save_gamePath.data();
}

CustomMissionID::CustomMissionID() : Number { SpawnerMain::GetGameConfigs()->CustomMissionID } { }

//issue #18 : Save game filter for 3rd party campaigns

ASMJIT_PATCH(0x559921, LoadOptionsClass_FillList_FilterFiles, 0x6)
{
	GET(FileEntryClass*, pEntry, EBP);
	enum { NullThisEntry = 0x559959 };
	/*
	// there was a qsort later and filters out these but we could have just removed them right here
	if (pEntry->IsWrongVersion || !pEntry->IsValid)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	};
	*/
	OLECHAR wNameBuffer[0x100] {};
	MultiByteToWideChar(CP_UTF8, 0, SavedGames::FormatPath(pEntry->Filename.data()), -1, wNameBuffer, std::size(wNameBuffer));
	IStoragePtr pStorage = nullptr;
	bool shouldDelete = false;
	if (SUCCEEDED(StgOpenStorage(wNameBuffer, NULL,
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStorage)
	))
	{
		auto id = SavedGames::ReadFromStorage<CustomMissionID>(pStorage);

		if (SpawnerMain::GetGameConfigs()->CustomMissionID != id.value_or(0))
			shouldDelete = true;
	}

	if (shouldDelete)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	}

	return 0;
}

// Custom missions especially can contain paths in scenario filenames which cause
// the initial save game to fail, remove the paths before filename and make the
// filename uppercase to match with usual savegame names.
ASMJIT_PATCH(0x55DC85, MainLoop_SaveGame_SanitizeFilename, 0x7)
{
	LEA_STACK(char*, pFilename, STACK_OFFSET(0x1C4, -0x178));
	LEA_STACK(const wchar_t*, pDescription, STACK_OFFSET(0x1C4, -0x70));

	char* slash1 = strrchr(pFilename, '/');
	char* slash2 = strrchr(pFilename, '\\');
	char* lastSlash = (slash1 > slash2) ? slash1 : slash2;

	if (lastSlash != NULL)
	{
		pFilename = lastSlash + 1;
		*lastSlash = '\0';
	}

	for (char* p = pFilename; *p; ++p)
		*p = (char)toupper((unsigned char)*p);

	R->ECX(pFilename);
	R->EDX(pDescription);

	return 0x55DC90;
}

#undef SET_SAVENAME