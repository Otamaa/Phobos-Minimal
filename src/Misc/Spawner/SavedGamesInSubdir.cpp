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
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <filesystem>
#include <HouseClass.h>
#include <LoadOptionsClass.h>
#include <optional>

static inline std::string save_gamePath;

#define SET_SAVENAME(name) static COMPILETIMEEVAL wchar_t* SaveName = L ##name

namespace SavedGames
{
	int HowManyTimesISavedForThisScenario = 0;

		//issue #18
	struct CustomMissionID
	{
		SET_SAVENAME("CustomMissionID");
		int Number;

		explicit CustomMissionID() :
			Number {
				SessionClass::IsCampaign() ?
				SpawnerMain::GetGameConfigs()->CustomMissionID : 0 }
		{
		}

		operator int() const
		{
			return Number;
		}

		CustomMissionID(noinit_t()) { }
	};

		// More fun
	struct ExtraMetaInfo
	{
		int CurrentFrame;
		int SavedCount;
		int TechnoCount;

		SET_SAVENAME("Spawner Extra Info");

		explicit ExtraMetaInfo()
			:CurrentFrame { Unsorted::CurrentFrame }
			, SavedCount { HowManyTimesISavedForThisScenario }
			, TechnoCount { TechnoClass::Array->Count }
		{
		}

		ExtraMetaInfo(noinit_t()) { }
	};

	template<typename T>
	bool AppendToStorage(IStorage* pStorage)
	{
		IStream* pStream = nullptr;
		bool ret = false;
		HRESULT hr = pStorage->CreateStream(
			T::SaveName,
			STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
			0,
			0,
			&pStream
		);

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			T info {};
			ULONG written = 0;
			hr = pStream->Write(&info, sizeof(info), &written);
			ret = SUCCEEDED(hr) && written == sizeof(info);
			pStream->Release();
		}

		return ret;
	}

	template<typename T>
	std::optional<T> ReadFromStorage(IStorage* pStorage)
	{
		IStream* pStream = nullptr;
		bool hasValue = false;
		HRESULT hr = pStorage->OpenStream(
			T::SaveName,
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			0,
			&pStream
		);

		T info;

		if (SUCCEEDED(hr) && pStream != nullptr)
		{
			ULONG read = 0;
			hr = pStream->Read(&info, sizeof(info), &read);
			hasValue = SUCCEEDED(hr) && read == sizeof(info);

			pStream->Release();
		}

		return hasValue ? std::make_optional(info) : std::nullopt;
	}

	OPTIONALINLINE bool CreateSubdir()
	{
		if (!std::filesystem::exists(SpawnerMain::GetGameConfigs()->SavedGameDir))
		{
			Debug::LogInfo("[Spawner] Folder Saved Games does not exist, creating...");
			if (!std::filesystem::create_directories(SpawnerMain::GetGameConfigs()->SavedGameDir))
			{
				Debug::LogInfo("\tCannot create folder Saved Games!");
				return false;
			}
			Debug::LogInfo("\tDone.");
		}
		return true;
	}

	OPTIONALINLINE char* FormatPath(const char* pFileName) {
		save_gamePath = SpawnerMain::GetGameConfigs()->SavedGameDir;
		save_gamePath += "\\";
		save_gamePath += pFileName;
		return save_gamePath.data();
	}
}

#ifdef incomplete_ExtraSavedGamesData

ASMJIT_PATCH(0x559921, LoadOptionsClass_FillList_FilterFiles, 0x6)
{
	GET(FileEntryClass*, pEntry, EBP);
	enum { NullThisEntry = 0x559959 };

	// there was a qsort later and filters out these but we could have just removed them right here
	if (pEntry->IsWrongVersion || !pEntry->IsValid)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	};

	static OLECHAR wNameBuffer[0x100] {};
	SavedGames::FormatPath(Phobos::readBuffer, pEntry->Filename.data());
	MultiByteToWideChar(CP_UTF8, 0, Phobos::readBuffer, -1, wNameBuffer, std::size(wNameBuffer));
	IStorage* pStorage = nullptr;
	bool shouldDelete = false;
	if (SUCCEEDED(StgOpenStorage(wNameBuffer, NULL,
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStorage)
	))
	{
		auto id = SavedGames::ReadFromStorage<SavedGames::CustomMissionID>(pStorage);

		if (SpawnerMain::GetGameConfigs()->CustomMissionID != id.value_or(0))
			shouldDelete = true;
	}

	if (pStorage)
		pStorage->Release();

	if (shouldDelete)
	{
		GameDelete(pEntry);
		return NullThisEntry;
	}

	return 0;
}

ASMJIT_PATCH(0x683AFE, StartNewScenario_ClearCounter, 0x6)
{
	SavedGames::HowManyTimesISavedForThisScenario = 0;
	return 0;
}

ASMJIT_PATCH(0x67D2E3, SaveGame_AdditionalInfoForClient, 0x6)
{
	GET_STACK(IStorage*, pStorage, STACK_OFFSET(0x4A0, -0x490));
	SavedGames::HowManyTimesISavedForThisScenario++;

	if (pStorage)
	{
		if (SessionClass::IsCampaign() && SpawnerMain::GetGameConfigs()->CustomMissionID)
			SavedGames::AppendToStorage<SavedGames::CustomMissionID>(pStorage);
		if (SavedGames::AppendToStorage<SavedGames::ExtraMetaInfo>(pStorage))
			Debug::LogInfo("[Spawner] Extra meta info appended on sav file");
	}

	return 0;
}

ASMJIT_PATCH(0x67E4DC, LoadGame_AdditionalInfoForClient, 0x7)
{
	LEA_STACK(const wchar_t*, filename, STACK_OFFSET(0x518, -0x4F4));
	IStorage* pStorage = nullptr;

	if (SUCCEEDED(StgOpenStorage(filename, NULL,
		STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStorage)
	))
	{
		if (auto id = SavedGames::ReadFromStorage<SavedGames::CustomMissionID>(pStorage))
		{
			int num = id->Number;
			Debug::LogInfo("[Spawner] sav file CustomMissionID = {}", num);
			SpawnerMain::GetGameConfigs()->CustomMissionID = num;
			ScenarioClass::Instance->EndOfGame = true;
		}
		else
		{
			SpawnerMain::GetGameConfigs()->CustomMissionID = 0;
		}

		if (auto info = SavedGames::ReadFromStorage<SavedGames::ExtraMetaInfo>(pStorage))
		{
			Debug::LogInfo("[Spawner] CurrentFrame = {}, TechnoCount = {}, HowManyTimesSaved = {} "
				, info->CurrentFrame
				, info->TechnoCount
				, info->SavedCount
			);

			SavedGames::HowManyTimesISavedForThisScenario = info->SavedCount;
		}
	}
	if (pStorage)
		pStorage->Release();

	return 0;
}
#endif

ASMJIT_PATCH(0x67E475, LoadGame_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		GET(char*, pFileName, ESI);
		R->ESI(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

ASMJIT_PATCH(0x559EB0, DeleteSave_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		REF_STACK(char*, pFileName, 0x4);
		pFileName = SavedGames::FormatPath(pFileName);
	}

	return 0;
}

ASMJIT_PATCH(0x67CF11, SaveGame_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		if (!SavedGames::CreateSubdir())
			return 0;

		GET(char*, pFileName, EDI);
		R->EDI(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

// Create random filename for save
// Not used. But it does not hurt in case of using a third-party library
// WW compiler made OPTIONALINLINE in LoadOptionsClass_Dialog
ASMJIT_PATCH(0x55961C, LoadOptionsClass_RandomFilename_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		GET(char*, pFileName, ESI);
		R->ESI(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

// Finds free file name
ASMJIT_PATCH(0x5592D2, LoadOptionsClass_Dialog_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		GET(char*, pFileName, EDX);
		R->EDX(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

// Used for disable buttons in a dialogs
ASMJIT_PATCH(0x559C98, LoadOptionsClass_HasSaves_SGInSubdir, 0xB)
{
	LEA_STACK(void*, pFindFileData, STACK_OFFSET(0x348, -0x140));
	LEA_STACK(char*, pFileName, STACK_OFFSET(0x348, -0x33C));

	if (SpawnerMain::Configs::Enabled) {
		pFileName = SavedGames::FormatPath(pFileName); // Always "Saved Games\*.SAV"
	}

	R->EAX(pFileName);
	R->EDX(pFindFileData);

	return 0x559C98 + 0xB;
}

// Fill a list of files
ASMJIT_PATCH(0x559886, LoadOptionsClass_FillList_SGInSubdir, 0x8)
{
	GET(struct _WIN32_FIND_DATAA*, pFind, EDX);
	GET(char*, pFileName, EAX);

	if (SpawnerMain::Configs::Enabled) {
		pFileName = SavedGames::FormatPath(pFileName); // Always "Saved Games\*.SAV"
	}

	HANDLE result = FindFirstFileA(pFileName, pFind);
	R->EAX(result);

	return 0x559886 + 0x8;
}

ASMJIT_PATCH(0x67FD26, LoadOptionsClass_ReadSaveInfo_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled) {
		GET(char*, pFileName, ECX);
		R->ECX(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

#undef SET_SAVENAME
