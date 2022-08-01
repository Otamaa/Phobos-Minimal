#include <Phobos.h>

#ifdef ENABLE_CLR
#include <Scripting/CLR.h>
#endif
#include "Phobos_ECS.h"

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <filesystem>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <New/Entity/ElectricBoltClass.h>
#include <Misc/Patches.h>

#include "Phobos.Threads.h"
//#include "Phobos.Lua.h"

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>

#ifndef IS_RELEASE_VER
bool Phobos::Config::HideWarning = false;
#endif

HMODULE Phobos::hInstance = NULL;
char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";
const char Phobos::readDefval[4] = "";

uintptr_t Phobos::AresBaseAddress;


#ifdef ENABLE_NEWHOOKS
const wchar_t* Phobos::VersionDescription = L"Phobos Unstable build (" _STR(BUILD_NUMBER) L"). DO NOT SHIP IN MODS!";
#elif defined(COMPILE_PORTED_DP_FEATURES)
const wchar_t* Phobos::VersionDescription = L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping.";
#else
const wchar_t* Phobos::VersionDescription = L"Beta version #" _STR(BUILD_NUMBER) L".provided by Otamaa" ;
#endif

bool Phobos::UI::DisableEmptySpawnPositions = false;
bool Phobos::UI::ExtendedToolTips = false;
int Phobos::UI::MaxToolTipWidth = 0;
bool Phobos::UI::ShowHarvesterCounter = false;
double Phobos::UI::HarvesterCounter_ConditionYellow = 0.99;
double Phobos::UI::HarvesterCounter_ConditionRed = 0.5;
bool Phobos::UI::ShowProducingProgress = false;
const wchar_t* Phobos::UI::CostLabel;
const wchar_t* Phobos::UI::PowerLabel;
const wchar_t* Phobos::UI::TimeLabel;
const wchar_t* Phobos::UI::HarvesterLabel;
const wchar_t* Phobos::UI::PercentLabel;
bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::AllowParallelAIQueues = true;
bool Phobos::Config::EnableBuildingPlacementPreview = false;
bool Phobos::Config::EnableSelectBrd = false;

bool Phobos::Config::ForbidParallelAIQueues_Infantry = false;
bool Phobos::Config::ForbidParallelAIQueues_Vehicle = false;
bool Phobos::Config::ForbidParallelAIQueues_Navy = false;
bool Phobos::Config::ForbidParallelAIQueues_Aircraft = false;
bool Phobos::Config::ForbidParallelAIQueues_Building = false;

std::string Phobos::AppIconPath;
char Phobos::AppName[0x40] = "";

bool Phobos::Debug_DisplayDamageNumbers = false;

bool Phobos::Otamaa::DisableCustomRadSite = false;
TCHAR Phobos::Otamaa::PCName[MAX_COMPUTERNAME_LENGTH + 1];
bool Phobos::Otamaa::IsAdmin = false;
bool Phobos::Otamaa::ShowHealthPercentEnabled = false;
#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif
#pragma region Tools

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}

		/*
		if (!_stricmp(pArg, "-Name"))
		{
			char nBuff[0x800] = "";
			strcpy_s(nBuff, ppArgs[++i]);
			if (auto v7 = strstr(nBuff, "\""))
			{
				strcpy_s(nBuff, v7 + 1);
				strcat_s(nBuff, " ");
				for (auto b = &ppArgs[++i]; !strstr(*b, "\""); b = &ppArgs[i])
				{
					strcat_s(nBuff, *b);
					strcat_s(nBuff, " ");
					++b;
				}

				*strstr(ppArgs[i], "\"") = 0;
				strcat_s(nBuff, ppArgs[i]);
			}

			strcpy_s(Phobos::AppName, 64, nBuff);
		}
		*/
#ifndef ENABLE_NEWHOOKS
		if (_stricmp(pArg, "-b=" _STR(BUILD_NUMBER)) == 0)
		{
			Phobos::Config::HideWarning = true;
		}
#endif
	}

	Debug::Log("Initialized Phobos " PRODUCT_VERSION "\n");
}

CCINIClass* Phobos::OpenConfig(const char* file)
{
	return CCINIClass::LoadINIFile(file);
}

void Phobos::CloseConfig(CCINIClass*& pINI)
{
	CCINIClass::UnloadINIFile(pINI);
}

std::filesystem::path get_module_path(HMODULE module)
{
	WCHAR buf[4096];
	return GetModuleFileNameW(module, buf, ARRAYSIZE(buf)) ? buf : std::filesystem::path();
}

#pragma warning( push )
#pragma warning (disable : 4091)
#pragma warning (disable : 4245)
bool Phobos::DetachFromDebugger()
{
	HMODULE hModule = LoadLibraryA("ntdll.dll");
	if (hModule != NULL) {
		auto const NtRemoveProcessDebug =
			(NTSTATUS(__stdcall*)(HANDLE, HANDLE))GetProcAddress(hModule, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(__stdcall*)(HANDLE))GetProcAddress(hModule, "NtClose");

		HANDLE hDebug;
		HANDLE hCurrentProcess = GetCurrentProcess();
		NTSTATUS status = NtQueryInformationProcess(hCurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
			ULONG killProcessOnExit = FALSE;
			status = NtSetInformationDebugObject(
				hDebug,
				1,
				&killProcessOnExit,
				sizeof(ULONG),
				NULL
			);
			if (0 <= status)
			{
				const auto pid = Patch::GetDebuggerProcessId(GetProcessId(hCurrentProcess));
				status = NtRemoveProcessDebug(hCurrentProcess, hDebug);
				if (0 <= status)
				{
					sprintf_s(Phobos::readBuffer, "taskkill /F /PID %d", pid);
					WinExec(Phobos::readBuffer, SW_HIDE);
					NtClose(hDebug);
					FreeLibrary(hModule);
					return true;
				}
			}
			NtClose(hDebug);
		}

		FreeLibrary(hModule);
	}

	return false;
}

#pragma warning( pop )
#pragma endregion

//static SIZE_T Ares_BynarySize = 0x0;  // Raw size of binary in bytes.
//static DWORD OldProtect = 0;
//static int VirtualProtectResult = 0;
//static HMODULE AresDllHmodule = nullptr;
//static std::filesystem::path  g_target_executable_path;
//static std::wstring Ares_dll_Fullpath;

#ifdef Ares_ExperimentalHooks
#include "ViniferaStyle_Hooks.h"

DECLARE_PATCH(Ares_RecalculateStats_intercept_Armor)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, edi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x88);
	Debug::Log("Ares_CellClass_CrateBeingCollected_Armor2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}

DECLARE_PATCH(Ares_RecalculateStats_intercept_FP)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, edi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x80);
	Debug::Log("Ares_CellClass_CrateBeingCollected_FirePower2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}

DECLARE_PATCH(Ares_RecalculateStats_intercept_Speed)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, esi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x90);
	Debug::Log("CellClass_CrateBeingCollected_Speed2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}
#endif

#ifdef ENABLE_CLR
DEFINE_HOOK(0x6BB9D2, PatcherLoader_Action,0x6)
{
	if (AllocConsole())
	{
		CLR::Init();
		CLR::Load();
	}
	else
	{
		MessageBoxW(NULL, TEXT("Alloc console error"), TEXT("Phobos"), MB_OK);
	}

	return 0x0;
}
#endif

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		Phobos::hInstance = hModule;
		//PhobosLua::Construct();
		//g_target_executable_path = get_module_path(nullptr);
		//Ares_dll_Fullpath = (g_target_executable_path.parent_path() / ARES_DLL).wstring();
		//Phobos::AresBaseAddress = GetModuleBaseAddress(ARES_DLL_S);

		//if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, Ares_dll_Fullpath.c_str(), &AresDllHmodule))
		//{
		//	Phobos::AresModuleExportData = enumerate_module_exports(AresDllHmodule);
		//	Ares_BynarySize = (SIZE_T)std::filesystem::file_size(get_module_path(AresDllHmodule));
			//Vinifera_Hooker::StartHooking(AresDllHmodule);
			//Vinifera_Style::RegisterHooks();
			//Patch_Call(GetAddr("CellClass_CrateBeingCollected_Armor2", 0x2D), &Ares_RecalculateStats_intercept_Armor);
			//Patch_Call(GetAddr("CellClass_CrateBeingCollected_Firepower2", 0x2D), &Ares_RecalculateStats_intercept_FP);
			//Patch_Call(GetAddr("CellClass_CrateBeingCollected_Speed2", 0x47), &Ares_RecalculateStats_intercept_Speed);
		//}

		//Phobos::init_Crt();

	}
	break;

	case DLL_PROCESS_DETACH:
		//FreeConsole();
		//PhobosLua::Destroy();
		//MyEcs.quit();
		//uninstall();
		//if (AresDllHmodule) {
		//	Vinifera_Hooker::StopHooking(AresDllHmodule);
		//}
		break;
	}

	return TRUE;
}

// =============================
#pragma region SyringeHandshake
SYRINGE_HANDSHAKE(pInfo)
{
	//const DWORD YR_SIZE_1000 = 0x496110;
	const DWORD YR_SIZE_1001 = 0x497110;
	const DWORD YR_SIZE_1001_UC = 0x497FE0;
	const DWORD YR_SIZE_NPATCH = 0x5AB000;

	const DWORD YR_TIME_1000 = 0x3B846665;
	const DWORD YR_TIME_1001 = 0x3BDF544E;

	//const DWORD YR_CRC_1000 = 0xB701D792;
	const DWORD YR_CRC_1001_CD = 0x098465B3;
	const DWORD YR_CRC_1001_TFD = 0xEB903080;
	const DWORD YR_CRC_1001_UC = 0x1B499086;

	if (pInfo)
	{
		const char* AcceptMsg = "Found Yuri's Revenge %s. Applying Phobos " _STR(BUILD_NUMBER) ".";
		const char* PatchDetectedMessage = "Found %s. Phobos " _STR(BUILD_NUMBER) " is not compatible with other patches.";

		const char* desc = nullptr;
		const char* msg = nullptr;
		bool allowed = false;

		// accept tfd and cd version 1.001
		if (pInfo->exeTimestamp == YR_TIME_1001)
		{
			// don't accept expanded exes
			switch (pInfo->exeFilesize)
			{
			case YR_SIZE_1001:
			case YR_SIZE_1001_UC:

				// all versions allowed
				switch (pInfo->exeCRC)
				{
				case YR_CRC_1001_CD:
					desc = "1.001 (CD)";
					break;
				case YR_CRC_1001_TFD:
					desc = "1.001 (TFD)";
					break;
				case YR_CRC_1001_UC:
					desc = "1.001 (UC)";
					break;
				default:
					// no-cd, network port or other changes
					desc = "1.001 (modified)";
				}
				msg = AcceptMsg;
				allowed = true;
				break;

			case YR_SIZE_NPATCH:
				// known patch size
				desc = "RockPatch or an NPatch-derived patch";
				msg = PatchDetectedMessage;
				break;
			default:
				// expanded exe, unknown make
				desc = "an unknown game patch";
				msg = PatchDetectedMessage;
			}
		}
		else if (pInfo->exeTimestamp == YR_TIME_1000)
		{
			// upgrade advice for version 1.000
			desc = "1.000";
			msg = "Found Yuri's Revenge 1.000 but Phobos " _STR(BUILD_NUMBER) " requires version 1.001. Please update your copy of Yuri's Revenge first.";
		}
		else
		{
			// does not even compute...
			msg = "Unknown executable. Phobos " _STR(BUILD_NUMBER) " requires Command & Conquer Yuri's Revenge version 1.001 (gamemd.exe).";
		}

		// generate the output message
		if (pInfo->Message)
		{
			sprintf_s(pInfo->Message, pInfo->cchMessage, msg, desc);
		}

		return allowed ? S_OK : S_FALSE;
	}

	return E_POINTER;
}
#pragma endregion

#pragma region hooks
#ifdef ENABLE_ENCRYPTION_HOOKS
//ToDo: Decrypt ?
BOOL __stdcall ReadFIle_(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
	return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

//ToDo: EncryptHere ?
BOOL __stdcall CloseHandle_(HANDLE hFile) {
	return CloseHandle(hFile);
}

//ToDo: EncryptHere ?
HANDLE __stdcall CreateFileA_(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile){
	return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

//ToDo: Decrypt ?
HRESULT __stdcall OleLoadFromStream_(LPSTREAM pStm, REFIID iidInterface, LPVOID* ppvObj){
	return OleLoadFromStream(pStm, iidInterface, ppvObj);
}
#endif
DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
{
	Patch::ApplyStatic();

	if (!Phobos::Config::HideWarning)
	{
		DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
		GetComputerName(Phobos::Otamaa::PCName, &dwSize);

		if (IS_SAME_STR_(Phobos::Otamaa::PCName, ADMIN_STR))
		{
			Phobos::Otamaa::IsAdmin = true;

			if (Phobos::DetachFromDebugger())
			{
				MessageBoxW(NULL,
				L"You can now attach a debugger.\n\n"

				L"Press OK to continue YR execution.",
				L"Debugger Notice", MB_OK);
			}
			else
			{
				MessageBoxW(NULL,
				L"You can now attach a debugger.\n\n"

				L"To attach a debugger find the YR process in Process Hacker "
				L"/ Visual Studio processes window and detach debuggers from it, "
				L"then you can attach your own debugger. After this you should "
				L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

				L"Press OK to continue YR execution.",
				L"Debugger Notice", MB_OK);
			}
		}
	}

	//PhobosLua::Test();

#ifdef ENABLE_ENCRYPTION_HOOKS
	typedef BOOL (__stdcall* _imp_ReadFile__)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	typedef BOOL (__stdcall* _imp_CloseHandle__)(HANDLE hFile);
	typedef HANDLE(__stdcall* _imp_CreateFileA__)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE  hTemplateFile);
	typedef HRESULT(__stdcall* _imp_OleLoadFromStream__)(LPSTREAM pStm, REFIID iidInterface, LPVOID* ppvObj);
	DWORD protect_flag;

	//Patch game stuffs
	Patch::Apply<_imp_ReadFile__>(0x7E111C,ReadFIle_, protect_flag);
	Patch::Apply<_imp_CloseHandle__>(0x7E11E0, CloseHandle_, protect_flag);
	Patch::Apply<_imp_CreateFileA__>(0x7E11BC, CreateFileA_, protect_flag);
	Patch::Apply<_imp_OleLoadFromStream__>(0x7E15F8, OleLoadFromStream_, protect_flag);
#endif

	return 0;
}

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	//MyEcs = flecs::world(nNumArgs, ppArgs);
	Phobos::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::ToolTipDescriptions = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "ToolTipDescriptions", true);
	Phobos::Config::PrioritySelectionFiltering = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "PrioritySelectionFiltering", true);
	Phobos::Config::EnableBuildingPlacementPreview = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "ShowBuildingPlacementPreview", false);
	Phobos::Config::EnableSelectBrd = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "EnableSelectBrd", false);

	if (CCINIClass* pINI_UIMD = Phobos::OpenConfig(UIMD_FILENAME))
	{
		// LoadingScreen
		{
			Phobos::UI::DisableEmptySpawnPositions =
				pINI_UIMD->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
		}

		// ToolTips
		{
			Phobos::UI::ExtendedToolTips =
				pINI_UIMD->ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

			Phobos::UI::MaxToolTipWidth =
				pINI_UIMD->ReadInteger(TOOLTIPS_SECTION, "MaxWidth", 0);

			pINI_UIMD->ReadString(TOOLTIPS_SECTION, "CostLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

			pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1"); // ⚡

			pINI_UIMD->ReadString(TOOLTIPS_SECTION, "TimeLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚

			pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PercentLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PercentLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚
		}

		// Sidebar
		{
			Phobos::UI::ShowHarvesterCounter =
				pINI_UIMD->ReadBool(SIDEBAR_SECTION_T, "HarvesterCounter.Show", false);

			pINI_UIMD->ReadString(SIDEBAR_SECTION_T, "HarvesterCounter.Label", NONE_STR, Phobos::readBuffer);
			Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

			Phobos::UI::HarvesterCounter_ConditionYellow =
				pINI_UIMD->ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

			Phobos::UI::HarvesterCounter_ConditionRed =
				pINI_UIMD->ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

			Phobos::UI::ShowProducingProgress =
				pINI_UIMD->ReadBool(SIDEBAR_SECTION_T, "ProducingProgress.Show", false);

			Phobos::UI::ShowPowerDelta =
				pINI_UIMD->ReadBool(SIDEBAR_SECTION_T, "PowerDelta.Show", false);

			Phobos::UI::PowerDelta_ConditionYellow =
				pINI_UIMD->ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

			Phobos::UI::PowerDelta_ConditionRed =
				pINI_UIMD->ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);
		}

		if (pINI_UIMD->ReadBool(GENERAL_SECTION, "FixTransparencyBlitters", true))
		{
			BlittersFix::Apply();
		}

		Phobos::CloseConfig(pINI_UIMD);
	}
	else
	{
		Debug::Log(FAILEDTOLOADUIMD_MSG);
	}

	return 0;
}

#ifdef ENABLE_CRT_HOOKS
DEFINE_HOOK(0x6BBFCE, _WinMain_InitPhobos_, 0xB)
{
	_set_controlfp(_RC_CHOP, _MCW_RC);
	fesetround(FE_TOWARDZERO);
	return 0x0;
}
#endif

#ifdef ENABLE_DLL_SYNC_FETCHRESOURCE
DEFINE_HOOK(0x4A3B4B, _YR_Fetch_Resource, 0x9)
{

	GET(LPCSTR, lpName, ECX);
	GET(LPCSTR, lpType, EDX);

	HMODULE hModule = Phobos::hInstance;
	if (HRSRC hResInfo = FindResource(hModule, lpName, lpType))
	{
		if (HGLOBAL hResData = LoadResource(hModule, hResInfo))
		{
			LockResource(hResData);
			R->EAX(hResData);
			return 0x4A3B73; //Resource locked and loaded (omg what a pun), return!
		}
	}

	return 0; //Nothing was found, try the game's own resources.
}
#endif

#ifdef ENABLE_TLS
DEFINE_HOOK(0x52BA78, _YR_GameInit_Pre, 0x5)
{
	if ((TLS_Thread::dwTlsIndex_SHPDRaw_1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		Debug::FatalErrorAndExit("TlsAlloc 1 error ! \n");

	if ((TLS_Thread::dwTlsIndex_SHPDRaw_2 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		Debug::FatalErrorAndExit("TlsAlloc 2 error ! \n");

	return 0;
}

static DWORD __fastcall DeInt_72AC40()
{ JMP_STD(0x72AC40); }

static DWORD Phobos_EndProgHandle_add()
{
	Debug::Log("Cleaning up phobos ! \n");
	TlsFree(TLS_Thread::dwTlsIndex_SHPDRaw_1);
	TlsFree(TLS_Thread::dwTlsIndex_SHPDRaw_2);
	return DeInt_72AC40();
}

DEFINE_JUMP(CALL,0x6BE118, GET_OFFSET(Phobos_EndProgHandle_add));
#endif

DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
	if (!Phobos::Config::HideWarning) {
		const auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0,0 }, 0, 2, 0);

		RectangleStruct rect = {
			DSurface::Composite->Get_Width() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		DSurface::Composite->Fill_Rect(rect, COLOR_BLACK);
		DSurface::Composite->DSurfaceDrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
	return 0;
}

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x7C8E17, operator_new_AddExtraSize, 0x6)
{
	GET_STACK(int, nDataSize, 0x4);
	R->Stack(0x4, nDataSize + 0x4);
	return 0x0;
}
#endif
/*
DEFINE_HOOK(0x6BE1C2, _YR_ProgramEnd, 0x8)
{
	return 0x0;
}

DEFINE_HOOK_AGAIN(0x777D71, WindowName_ApplyCustom, 0x5)
DEFINE_HOOK(0x777CCA, WindowName_ApplyCustom, 0x5)
{
	if (Phobos::AppName && strlen(Phobos::AppName) > 0)
		R->ESP(Phobos::AppName);

	return 0x0;
}

DEFINE_HOOK(0x7D107D, Game_msize_replace, 0x8)
{
	GET_STACK(void*, pVoid, 0x4);
	size_t nSize = 0;
	CRT::_lock(9);
	if (CRT::_sbh_find_block(pVoid))
	{
		DWORD nPtr = *reinterpret_cast<DWORD*>(pVoid);
		nSize = (nPtr - 4) - 9;
		CRT::_unlock(9);
	}
	else
	{
		CRT::_unlock(9);
		nSize = HeapSize(CRT_Heap, 0, pVoid);
	}

	R->EAX(nSize);
	return 0x7D10C1;
}

DEFINE_HOOK(0x7D5408, Game_strdup_replace, 0x5)
{
	GET_STACK(const char*, In, 0x4);

	char* str;
	char* p;
	int len = 0;

	while (In[len]) {
		len++;
	}

	str = (char*)CRT::malloc(len + 1);
	p = str;

	while (*In) {
		*p++ = *In++;
	}

	*p = '\0';

	R->EAX(str);
	return 0x7D542E;
}
*/
#pragma endregion