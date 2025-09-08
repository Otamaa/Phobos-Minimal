#include <LoadOptionsClass.h>

#include <Utilities/Macro.h>
#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Spawner/Main.h>
#include <Misc/Spawner/SavedGamesInSubdir.h>

ASMJIT_PATCH(0x559F31, LoadOptionsClass_GetFileInfo, 9)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));

	// compare equal if same mod and same Ares version (or compatible)
	auto same = (Info.Version == (AresGlobalData::version)
		&& DWORD(Info.InternalVersion) == Game::Savegame_Magic());

	R->ECX(&Info);
	return same ? 0x559F60u : 0x559F48u;
}

// Create random filename for save
// Not used. But it does not hurt in case of using a third-party library
// WW compiler made OPTIONALINLINE in LoadOptionsClass_Dialog

ASMJIT_PATCH(0x559EB0, LoadOptionsClass_DeleteSave_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled)
	{
		REF_STACK(char*, pFileName, 0x4);
		pFileName = SavedGames::FormatPath(pFileName);
	}

	return 0;
}

ASMJIT_PATCH(0x55961C, LoadOptionsClass_RandomFilename_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled)
	{
		GET(char*, pFileName, ESI);
		R->ESI(SavedGames::FormatPath(pFileName));
	}

	return 0;
}

// Finds free file name
ASMJIT_PATCH(0x5592D2, LoadOptionsClass_Dialog_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled)
	{
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

	if (SpawnerMain::Configs::Enabled)
	{
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

	if (SpawnerMain::Configs::Enabled)
	{
		pFileName = SavedGames::FormatPath(pFileName); // Always "Saved Games\*.SAV"
	}

	HANDLE result = FindFirstFileA(pFileName, pFind);
	R->EAX(result);

	return 0x559886 + 0x8;
}

ASMJIT_PATCH(0x67FD26, LoadOptionsClass_ReadSaveInfo_SGInSubdir, 0x5)
{
	if (SpawnerMain::Configs::Enabled)
	{
		GET(char*, pFileName, ECX);
		R->ECX(SavedGames::FormatPath(pFileName));
	}

	return 0;
}
