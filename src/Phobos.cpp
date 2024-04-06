#include <Phobos.h>

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <filesystem>

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>
#include <WinBase.h>
#include <CD.h>
#include <aclapi.h>

#include <Phobos.UI.h>
HANDLE Phobos::hInstance;

DEFINE_HOOK(0x5D4E66, Windows_Message_Handler_Add, 0x7)
{
	PhobosWindowClass::Callback();
	return 0x0;
}

#pragma endregion
BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		Phobos::hInstance = hInstance;
	}
	break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

#pragma region hooks

DEFINE_HOOK(0x7cd8ef, Game_ExeTerminate, 9)

{
	PhobosWindowClass::Destroy();
	return 0x0;
}

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)

{
	PhobosWindowClass::Create();
	return 0;
}
#pragma endregion