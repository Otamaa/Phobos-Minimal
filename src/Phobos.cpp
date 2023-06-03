#include <Phobos.h>

#ifdef ENABLE_CLR
#include <Scripting/CLR.h>
#endif
#include "Phobos_ECS.h"

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <filesystem>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <Misc/AresData.h>
#include <Misc/Patches.h>

#include "Phobos.Threads.h"

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>

struct scoped_handle
{
	HANDLE handle;

	scoped_handle() :

		handle(INVALID_HANDLE_VALUE)
	{
	}
	scoped_handle(HANDLE handle) :
		handle(handle) { }

	scoped_handle(scoped_handle&& other) noexcept :
		handle(other.handle)
	{
		other.handle = NULL;
	}

	~scoped_handle() { if (handle != NULL && handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }

	bool IsValid() const { return (handle != NULL && handle != INVALID_HANDLE_VALUE); }

	operator HANDLE() const { return handle; }

	HANDLE* operator&() { return &handle; }
	const HANDLE* operator&() const { return &handle; }
};

struct scoped_library
{
	HMODULE handle;

	scoped_library() :

		handle(NULL)
	{
	}

	scoped_library(HMODULE handle) :
		handle(handle) { }

	scoped_library(scoped_library&& other) noexcept :
		handle(other.handle)
	{
		other.handle = NULL;
	}

	~scoped_library() { if (handle != NULL) FreeLibrary(handle); }

	bool IsValid() const { return handle != NULL; }

	operator HMODULE() const { return handle; }

	HMODULE* operator&() { return &handle; }
	const HMODULE* operator&() const { return &handle; }
};

#pragma region DEFINES
#ifndef IS_RELEASE_VER
bool Phobos::Config::HideWarning = false;
#endif

HANDLE Phobos::hInstance = NULL;
char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";
const char Phobos::readDefval[4] = "";


#ifdef ENABLE_NEWHOOKS
const wchar_t* Phobos::VersionDescription = L"Phobos Unstable build (" _STR(BUILD_NUMBER) L"). DO NOT SHIP IN MODS!";
#elif defined(COMPILE_PORTED_DP_FEATURES)
const wchar_t* Phobos::VersionDescription = L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping.";
#else
const wchar_t* Phobos::VersionDescription = L"Beta version #" _STR(BUILD_NUMBER) L".provided by Otamaa";
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
const wchar_t* Phobos::UI::PowerBlackoutLabel = L"";
const wchar_t* Phobos::UI::TimeLabel;
const wchar_t* Phobos::UI::HarvesterLabel;
const wchar_t* Phobos::UI::PercentLabel;
bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
bool Phobos::UI::CenterPauseMenuBackground = false;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::ToolTipBlur = false;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::AllowParallelAIQueues = true;
bool Phobos::Config::EnableBuildingPlacementPreview = false;

bool Phobos::Config::RealTimeTimers = false;
bool Phobos::Config::RealTimeTimers_Adaptive = false;
int Phobos::Config::CampaignDefaultGameSpeed = 2;

bool Phobos::Misc::CustomGS = false;
int Phobos::Misc::CustomGS_ChangeInterval[7] = { -1, -1, -1, -1, -1, -1, -1 };
int Phobos::Misc::CustomGS_ChangeDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };
int Phobos::Misc::CustomGS_DefaultDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };

bool Phobos::Config::EnableSelectBrd = false;

bool Phobos::Config::ForbidParallelAIQueues_Infantry = false;
bool Phobos::Config::ForbidParallelAIQueues_Vehicle = false;
bool Phobos::Config::ForbidParallelAIQueues_Navy = false;
bool Phobos::Config::ForbidParallelAIQueues_Aircraft = false;
bool Phobos::Config::ForbidParallelAIQueues_Building = false;

bool Phobos::Config::TogglePowerInsteadOfRepair = false;
bool Phobos::Config::ShowTechnoNamesIsActive = false;

std::string Phobos::AppIconPath;
char Phobos::AppName[0x40] = "";

bool Phobos::Debug_DisplayDamageNumbers = false;

bool Phobos::Otamaa::DisableCustomRadSite = false;
TCHAR Phobos::Otamaa::PCName[MAX_COMPUTERNAME_LENGTH + 1];
bool Phobos::Otamaa::IsAdmin = false;
bool Phobos::Otamaa::ShowHealthPercentEnabled = false;
bool Phobos::Otamaa::ExeTerminated = true;
bool Phobos::Otamaa::DoingLoadGame = false;

bool Phobos::EnableConsole = false;

#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif

#pragma endregion

#pragma region PhobosFunctions

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	//std::vector<std::string_view> args(ppArgs, std::next(ppArgs, static_cast<std::ptrdiff_t>(nNumArgs)));

	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const auto pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}

		/*
		if (_stricmp(pArg, "-Console") == 0)
		{
			Phobos::EnableConsole = true;
		}

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

void Phobos::Config::Read()
{
	auto const& pRA2MD = CCINIClass::INI_RA2MD;

	Phobos::Config::ToolTipDescriptions = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipDescriptions", true);
	Phobos::Config::ToolTipBlur = pRA2MD->ReadBool(PHOBOS_STR, "ToolTipBlur", false);
	Phobos::Config::PrioritySelectionFiltering = pRA2MD->ReadBool(PHOBOS_STR, "PrioritySelectionFiltering", true);
	Phobos::Config::EnableBuildingPlacementPreview = pRA2MD->ReadBool(PHOBOS_STR, "ShowBuildingPlacementPreview", false);
	Phobos::Config::EnableSelectBrd = pRA2MD->ReadBool(PHOBOS_STR, "EnableSelectBrd", false);

	Phobos::Config::RealTimeTimers = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers", false);
	Phobos::Config::RealTimeTimers_Adaptive = pRA2MD->ReadBool(PHOBOS_STR, "RealTimeTimers.Adaptive", false);

	// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
	Phobos::Config::CampaignDefaultGameSpeed = 6 - pRA2MD->ReadInteger(PHOBOS_STR, "CampaignDefaultGameSpeed", 4);

	if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
		Phobos::Config::CampaignDefaultGameSpeed = 2;

	{
		BYTE defaultspeed = (BYTE)Phobos::Config::CampaignDefaultGameSpeed;
		// We overwrite the instructions that force GameSpeed to 2 (GS4)
		PatchWrapper defaultspeed_patch1 { 0x55D77A , sizeof(defaultspeed), &defaultspeed };

		// when speed control is off. Doesn't need a hook.
		PatchWrapper defaultspeed_patch2 { 0x55D78D , sizeof(defaultspeed), &defaultspeed };
	}

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

			pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerBlackoutLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

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

			Phobos::Config::TogglePowerInsteadOfRepair =
				pINI_UIMD->ReadBool(SIDEBAR_SECTION_T, "TogglePowerInsteadOfRepair", false);

			Phobos::UI::CenterPauseMenuBackground =
				pINI_UIMD->ReadBool(SIDEBAR_SECTION_T, "CenterPauseMenuBackground", Phobos::UI::CenterPauseMenuBackground);

		}

		Phobos::CloseConfig(pINI_UIMD);
	}
	else
	{
		Debug::Log(FAILEDTOLOADUIMD_MSG);
	}

	if (CCINIClass* pINI = Phobos::OpenConfig(GameStrings::RULESMD_INI))
	{
		// there is only few mod(s) that using this 
		// just add your mod name or remove these code if you dont like it 
		//if (!Phobos::Otamaa::IsAdmin)
		//{
		//	std::string ModNameTemp;
		//	pINI->ReadString(GENERAL_SECTION, "Name", "", Phobos::readBuffer);
		//	ModNameTemp = Phobos::readBuffer;

		//	if (!ModNameTemp.empty())
		//	{
		//		std::size_t nFInd = ModNameTemp.find("Rise of the East");

		//		if (nFInd == std::string::npos)
		//			nFInd = ModNameTemp.find("Ion Shock");

		//		if (nFInd == std::string::npos)
		//			nFInd = ModNameTemp.find("New War");

		//		if (nFInd == std::string::npos)
		//			nFInd = ModNameTemp.find("NewWar");

		//		if (nFInd == std::string::npos)
		//		{
		//			MessageBoxW(NULL,
		//				L"This is not Official Phobos Build.\n\n"
		//				L"Please contact original Mod Author for Assistance !.",
		//				L"Warning !", MB_OK);
		//		}
		//	}
		//}

		if (!Phobos::Otamaa::IsAdmin)
			Phobos::Config::DevelopmentCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

		Phobos::Otamaa::DisableCustomRadSite = pINI->ReadBool(PHOBOS_STR, "DisableCustomRadSite", false);
		Phobos::Config::ArtImageSwap = pINI->ReadBool(GENERAL_SECTION, "ArtImageSwap", false);
		Phobos::Config::AllowParallelAIQueues = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowParallelAIQueues", false);

		Phobos::Config::ForbidParallelAIQueues_Infantry = pINI->ReadBool(GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Infantry", Phobos::Config::AllowParallelAIQueues);
		Phobos::Config::ForbidParallelAIQueues_Vehicle = pINI->ReadBool(GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Vehicle", Phobos::Config::AllowParallelAIQueues);
		Phobos::Config::ForbidParallelAIQueues_Navy = pINI->ReadBool(GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Navy", Phobos::Config::AllowParallelAIQueues);
		Phobos::Config::ForbidParallelAIQueues_Aircraft = pINI->ReadBool(GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Aircraft", Phobos::Config::AllowParallelAIQueues);
		Phobos::Config::ForbidParallelAIQueues_Building = pINI->ReadBool(GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Building", Phobos::Config::AllowParallelAIQueues);

		if (pINI->ReadBool(GENERAL_SECTION, "CustomGS", false))
		{
			Phobos::Misc::CustomGS = true;

			char tempBuffer[0x20];
			for (size_t i = 0; i <= 6; ++i)
			{
				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "CustomGS%d.ChangeDelay", 6 - i);
				int temp = pINI->ReadInteger(GENERAL_SECTION, tempBuffer, -1);
				if (temp >= 0 && temp <= 6)
					Phobos::Misc::CustomGS_ChangeDelay[i] = 6 - temp;

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "CustomGS%d.DefaultDelay", 6 - i);
				temp = pINI->ReadInteger(GENERAL_SECTION, tempBuffer, -1);
				if (temp >= 1)
					Phobos::Misc::CustomGS_DefaultDelay[i] = 6 - temp;

				IMPL_SNPRNINTF(tempBuffer, sizeof(tempBuffer), "CustomGS%d.ChangeInterval", 6 - i);
				temp = pINI->ReadInteger(GENERAL_SECTION, tempBuffer, -1);
				if (temp >= 1)
					Phobos::Misc::CustomGS_ChangeInterval[i] = temp;
			}
		}

		if (pINI->ReadBool(GENERAL_SECTION, "FixTransparencyBlitters", false))
		{
			BlittersFix::Apply();
		}

		if (pINI->ReadBool(GENERAL_SECTION, "SkirmishUnlimitedColors", false))
		{
			ALLOCATE_LOCAL_PATCH(SkirmishColorPatch, 0x69A310,
				0x8B, 0x44, 0x24, 0x04, 0xD1, 0xE0, 0x40, 0xC2, 0x04, 0x00);
		}

		Phobos::CloseConfig(pINI);
	}
}

void Phobos::DrawVersionWarning()
{
	if (!Phobos::Config::HideWarning)
	{
		const auto pSurface = DSurface::Composite();
		if (!pSurface)
			return;

		const auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0,0 }, 0, 2, 0);

		RectangleStruct rect {
			pSurface->Get_Width() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		pSurface->Fill_Rect(rect, COLOR_BLACK);
		pSurface->DSurfaceDrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
}

void InitAdminDebugMode()
{
	if (!Phobos::Config::HideWarning)
	{
		DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
		GetComputerName(Phobos::Otamaa::PCName, &dwSize);

		if (IS_SAME_STR_(Phobos::Otamaa::PCName, ADMIN_STR))
		{
			Phobos::Otamaa::IsAdmin = true;
#ifndef COMPILE_PORTED_DP_FEATURES
			Phobos::EnableConsole = true;
#endif

#ifndef DETACH_DEBUGGER
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
#endif
		}
	}

}

void InitConsole()
{
	if (Phobos::EnableConsole)
	{
		if (!Console::Create())
		{
			MessageBoxW(NULL,
			L"Failed to allocate the debug console!",
			L"Debug Console Notice", MB_OK);
		}
	}
}

bool FC CustomPalette_Read_Static(CustomPalette* pThis, DWORD, INI_EX* pEx, const char* pSection, const char* pKey, const char* pDefault)
{
	return pThis->Read(pEx->GetINI(), pSection, pKey, pDefault);
}

void Phobos::ExeRun()
{
	Phobos::Otamaa::ExeTerminated = false;

//	if (Patch::GetModuleBaseAddress("PatcherLoader.dll"))
//	{
//		MessageBoxW(NULL,
//		L"This version of phobos is not suppose to be run with DP.\n\n"
//		L"Press OK to Closing the game .",
//		L"Notice", MB_OK);
//
//		Phobos::ExeTerminate();
//		exit(0);
//	}

	if (!AresData::Init())
	{
		MessageBoxW(NULL,
		L"This version of phobos is only support Ares 3.0p1.\n\n"
		L"Press OK to Closing the game .",
		L"Notice", MB_OK);

		Phobos::ExeTerminate();
		exit(0);
	}

	DWORD protect_flag;
	for (auto const& nData : AresData::AresCustomPaletteReadFuncFinal)
	{
		const auto Data = _CALL(nData, GET_OFFSET(CustomPalette_Read_Static));
		VirtualProtect((LPVOID)nData, sizeof(Data), PAGE_EXECUTE_READWRITE, &protect_flag);
		memcpy((LPVOID)nData, (LPVOID)(&Data), sizeof(Data));
		VirtualProtect((LPVOID)nData, sizeof(Data), protect_flag, nullptr);
	}

	Patch::ApplyStatic();
	PoseDirOverride::Apply();
	InitAdminDebugMode();
	InitConsole();
}

#include <New/Type/TheaterTypeClass.h>

void Phobos::ExeTerminate()
{
	TheaterTypeClass::Array.clear();
	AresData::UnInit();
	Phobos::Otamaa::ExeTerminated = true;
	Console::Release();
}

#pragma warning( push )
#pragma warning (disable : 4091)
#pragma warning (disable : 4245)
bool Phobos::DetachFromDebugger()
{
	HMODULE hModule = LoadLibraryA("ntdll.dll");
	DWORD ret = false;

	if (hModule != NULL)
	{

		auto const NtRemoveProcessDebug =
			(NTSTATUS(__stdcall*)(HANDLE, HANDLE))GetProcAddress(hModule, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(hModule, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(__stdcall*)(HANDLE))GetProcAddress(hModule, "NtClose");

		HANDLE hDebug {};
		HANDLE hCurrentProcess = GetCurrentProcess();
		NTSTATUS status = NtQueryInformationProcess(hCurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
			ULONG killProcessOnExit = FALSE;
			status = NtSetInformationDebugObject(
				hDebug, 1, &killProcessOnExit, sizeof(ULONG), NULL);

			if (0 <= status)
			{
				const auto pid = Patch::GetDebuggerProcessId(GetProcessId(hCurrentProcess));
				status = NtRemoveProcessDebug(hCurrentProcess, hDebug);
				if (0 <= status)
				{
					HANDLE hDbgProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
					if (hDbgProcess != NULL)
					{
						ret = TerminateProcess(hDbgProcess, EXIT_SUCCESS);
						CloseHandle(hDbgProcess);
					}
				}
			}

			NtClose(hDebug);
		}

		if (hCurrentProcess != NULL)
			CloseHandle(hCurrentProcess);

		FreeLibrary(hModule);
	}

	return ret;
}
#pragma warning( pop )

#ifdef ENABLE_ENCRYPTION_HOOKS
//ToDo: Decrypt ?
BOOL __stdcall ReadFIle_(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

//ToDo: EncryptHere ?
BOOL __stdcall CloseHandle_(HANDLE hFile)
{
	return CloseHandle(hFile);
}

//ToDo: EncryptHere ?
HANDLE __stdcall CreateFileA_(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

//ToDo: Decrypt ?
HRESULT __stdcall OleLoadFromStream_(LPSTREAM pStm, REFIID iidInterface, LPVOID* ppvObj)
{
	return OleLoadFromStream(pStm, iidInterface, ppvObj);
}
#endif

#pragma endregion

#pragma region Unsorted
void NAKED _ExeTerminate()
{
	// Call WinMain
	SET_REG32(EAX, 0x6BB9A0);
	CALL(EAX);
	PUSH_REG(EAX);

	Phobos::ExeTerminate();

	// Jump back
	POP_REG(EAX);
	SET_REG32(EBX, 0x7CD8EF);
	__asm {jmp ebx};
}
#pragma endregion

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		Phobos::hInstance = hInstance;
		/**
		*  Set the FPU mode to match the game (rounding towards zero [chop mode]).
		*/
		_set_controlfp(_RC_CHOP, _MCW_RC);

		/**
		 *  And this is required for the std c++ lib.
		 */
		fesetround(FE_TOWARDZERO);
#ifdef ENABLE_ENCRYPTION_HOOKS
		Imports::ReadFile = ReadFIle_;
		Imports::CreateFileA = CreateFileA_;
		Imports::CloseHandle = CloseHandle_;
		Imports::OleLoadFromStream = OleLoadFromStream_;
#endif
	}
	break;
	case DLL_PROCESS_DETACH:
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

//DEFINE_JUMP(LJMP, 0x7CD8EA, GET_OFFSET(_ExeTerminate));
DEFINE_HOOK(0x6BE131, Game_ExeTerminate, 0x5)
{
	Phobos::ExeTerminate();
	return 0x0;
}

DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
{
	Phobos::ExeRun();
	Patch::PrintAllModuleAndBaseAddr();
	return 0;
}

//Disable MousePresent check
//DEFINE_JUMP(LJMP, 0x6BD8A4, 0x6BD8C2);

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();

	return 0;
}

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::Read();
	return 0;
}

#ifdef ENABLE_TLS
DEFINE_HOOK(0x52BA78, _YR_GameInit_Pre, 0x5)
{
	if ((TLS_Thread::dwTlsIndex_SHPDRaw_1 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		Debug::FatalErrorAndExit("TlsAlloc 1 error ! \n");

	if ((TLS_Thread::dwTlsIndex_SHPDRaw_2 = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		Debug::FatalErrorAndExit("TlsAlloc 2 error ! \n");

	return 0;
}

static DWORD FC DeInt_72AC40()
{ JMP_STD(0x72AC40); }

static DWORD Phobos_EndProgHandle_add()
{
	Debug::Log("Cleaning up phobos ! \n");
	TlsFree(TLS_Thread::dwTlsIndex_SHPDRaw_1);
	TlsFree(TLS_Thread::dwTlsIndex_SHPDRaw_2);
	return DeInt_72AC40();
}

DEFINE_JUMP(CALL, 0x6BE118, GET_OFFSET(Phobos_EndProgHandle_add));
#endif

DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
	Phobos::DrawVersionWarning();
	return 0;
}

#pragma endregion