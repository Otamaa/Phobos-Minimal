#include "Header.h"

#include <ScenarioClass.h>
#include <LoadOptionsClass.h>

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x67D04E, Game_Save_SavegameInformation, 7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x4A4, 0x3F4));

	// remember the Ares version and a mod id
	Info.Version = AresGlobalData::version;
	Info.InternalVersion = AresGlobalData::InternalVersion + PHOBOSSAVEGAME_ID;
	sprintf_s(Info.ExecutableName.data(), "GAMEMD.EXE + Phobos Minimal %d ", BUILD_NUMBER);

	return 0;
}

ASMJIT_PATCH(0x559F31, LoadOptionsClass_GetFileInfo, 9)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));

	// compare equal if same mod and same Ares version (or compatible)
	auto same = (Info.Version == (AresGlobalData::version)
		&& DWORD(Info.InternalVersion - PHOBOSSAVEGAME_ID) == AresGlobalData::InternalVersion);

	R->ECX(&Info);
	return same ? 0x559F60u : 0x559F48u;
}

// ASMJIT_PATCH(0x67CEFE, Game_Save_FixLog, 7)
// {
// 	GET(const char*, pFilename, EDI);
// 	GET(const wchar_t*, pSaveName, ESI);
//
// 	Debug::LogInfo("\nSAVING GAME [%s - %ls]", pFilename, pSaveName);
//
// 	return 0x67CF0D;
// }

ASMJIT_PATCH(0x74fdc0, GetModuleVersion, 5)
{
	R->EAX("Ares r21.352.1218");
	return 0x74FEEF;
}

ASMJIT_PATCH(0x74fae0, GetModuleInternalVersion, 5)
{
	R->EAX("1.001/Ares 3.0p1");
	return 0x74FC7B;
}

/* fixes to reorder the savegame */
ASMJIT_PATCH(0x67D315, SaveGame_EarlySaveSides, 5)
{
	GET(LPSTREAM, pStm, ESI);
	return (Game::Save_Sides(pStm, SideClass::Array) >= 0)
		? 0
		: 0x67E0B8
		;
}

ASMJIT_PATCH(0x67E09A, SaveGame_LateSkipSides, 5)
{
	GET(int, success, EAX);
	return success >= 0
		? 0x67E0C2
		: 0x67E0B8
		;
}
