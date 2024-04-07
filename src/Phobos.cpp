#include "Phobos.h"

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>
#include <WinBase.h>
#include <CD.h>
#include <aclapi.h>

#include <Utilities/Debug.h>
#include <Utilities/Parser.h>
#include <Utilities/Patch.h>
#include <Utilities/GeneralUtils.h>

#pragma region DEFINES

#ifndef IS_RELEASE_VER
bool Phobos::Config::HideWarning = false;
#endif

HANDLE Phobos::hInstance = NULL;
char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";
const char Phobos::readDefval[4] = "";

const wchar_t* Phobos::VersionDescription = L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping.";

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
const wchar_t* Phobos::UI::BuidingRadarJammedLabel;
const wchar_t* Phobos::UI::ShowBriefingResumeButtonLabel = L"";
char Phobos::UI::ShowBriefingResumeButtonStatusLabel[32];

bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
bool Phobos::UI::CenterPauseMenuBackground = false;
bool Phobos::UI::UnlimitedColor = false;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::ToolTipBlur = false;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;

bool Phobos::Config::EnableBuildingPlacementPreview = false;

bool Phobos::Config::RealTimeTimers = false;
bool Phobos::Config::RealTimeTimers_Adaptive = false;
int Phobos::Config::CampaignDefaultGameSpeed = 2;

bool Phobos::Config::MultiThreadSinglePlayer = false;
bool Phobos::Config::HideLightFlashEffects = false;

bool Phobos::Config::DebugFatalerrorGenerateDump = false;
bool Phobos::Config::SaveGameOnScenarioStart = true;

bool Phobos::Misc::CustomGS = false;
int Phobos::Misc::CustomGS_ChangeInterval[7] = { -1, -1, -1, -1, -1, -1, -1 };
int Phobos::Misc::CustomGS_ChangeDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };
int Phobos::Misc::CustomGS_DefaultDelay[7] = { 0, 1, 2, 3, 4, 5, 6 };

bool Phobos::Config::EnableSelectBrd = false;

bool Phobos::Config::TogglePowerInsteadOfRepair = false;
bool Phobos::Config::ShowTechnoNamesIsActive = false;

bool Phobos::Config::DigitalDisplay_Enable = false;

bool Phobos::Config::ApplyShadeCountFix = true;
bool Phobos::Config::SaveVariablesOnScenarioEnd = false;

std::string Phobos::AppIconPath;

bool Phobos::Debug_DisplayDamageNumbers = false;

bool Phobos::Otamaa::DisableCustomRadSite = false;
bool Phobos::Otamaa::IsAdmin = false;
bool Phobos::Otamaa::ShowHealthPercentEnabled = false;
bool Phobos::Otamaa::ExeTerminated = true;
bool Phobos::Otamaa::DoingLoadGame = false;
bool Phobos::Otamaa::AllowAIControl = false;
bool Phobos::Otamaa::OutputMissingStrings = false;
bool Phobos::Otamaa::OutputAudioLogs = false;
bool Phobos::Otamaa::StrictParser = false;
bool Phobos::Otamaa::ParserErrorDetected = false;
bool Phobos::Otamaa::TrackParserErrors = false;
bool Phobos::Otamaa::NoLogo = false;
bool Phobos::Otamaa::NoCD = false;
bool Phobos::Otamaa::CompatibilityMode = false;

bool Phobos::EnableConsole = false;

#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif

#pragma endregion

#pragma region PhobosFunctions

void NOINLINE Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	DWORD_PTR processAffinityMask = 1; // limit to first processor
	bool consoleEnabled = false;
	bool dontSetExceptionHandler = false;

	// > 1 because the exe path itself counts as an argument, too!
	std::string args;
	for (int i = 1; i < nNumArgs; i++)
	{
		const auto pArg = ppArgs[i];
		args += " ";
		args += pArg;

		if (IS_SAME_STR_(pArg, "-Icon"))
		{
			Phobos::AppIconPath = ppArgs[++i];
		}
		else if (IS_SAME_STR_(pArg, "-LOG"))
		{
			Debug::LogEnabled = true;
		}
		else if (IS_SAME_STR_(pArg, "-CONSOLE"))
		{
			consoleEnabled = true;
		}
		else if (IS_SAME_STR_(pArg, "-AI-CONTROL"))
		{
			Phobos::Otamaa::AllowAIControl = true;
		}
		else if (IS_SAME_STR_(pArg, "-LOG-CSF"))
		{
			Phobos::Otamaa::OutputMissingStrings = true;
		}
		else if (IS_SAME_STR_(pArg, "-LOG-AUDIO"))
		{
			Phobos::Otamaa::OutputAudioLogs = true;
		}
		else if (IS_SAME_STR_(pArg, "-STRICT"))
		{
			Phobos::Otamaa::StrictParser = true;
		}
		else if (IS_SAME_STR_(pArg, "-NOLOGO"))
		{
			Phobos::Otamaa::NoLogo = true;
		}
		else if (IS_SAME_STR_(pArg, "-CD"))
		{
			Phobos::Otamaa::NoCD = true;
		}
		else if (IS_SAME_STR_(pArg, "-b=" _STR(BUILD_NUMBER)))
		{
			Phobos::Config::HideWarning = true;
		}
		else if (!strncasecmp(pArg, "-AFFINITY:", 0xAu))
		{
			int result = 1;
			if (Parser<int>::Parse(pArg + 0xAu, &result) && result < 0) {
				result = 0;
			}

			processAffinityMask = result;
		} else {
			const std::string cur = pArg;
			if (cur.starts_with("-ExceptionHandler=")) {

				const size_t delim = cur.find("=");
				const std::string value = cur.substr(delim + 1, cur.size() - delim - 1);

				if (!value.empty()) {
					Parser<bool>::Parse(value.data(), &dontSetExceptionHandler);
				}
			}
		}

	}

	if (Debug::LogEnabled) {
		Debug::LogFileOpen();
		Debug::Log("Initialized Phobos %s .\n" , PRODUCT_VERSION);
		Debug::Log("args %s\n", args.c_str());

		if (consoleEnabled)
			Phobos::EnableConsole = true;

	}

	Game::DontSetExceptionHandler = dontSetExceptionHandler;
	Debug::Log("ExceptionHandler is %s\n", dontSetExceptionHandler ? "not present" : "present");

	if (processAffinityMask)
	{
		Debug::Log("Set Process Affinity: %lu (%x)\n", processAffinityMask, processAffinityMask);
		auto const process = GetCurrentProcess();
		SetProcessAffinityMask(process, processAffinityMask);
	}

	if (!CDDriveManagerClass::Instance->NumCDDrives)
	{
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		Phobos::Otamaa::NoCD = true;
	}

	if (Phobos::Otamaa::NoCD)
	{
		Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
		std::memset(CDDriveManagerClass::Instance->CDDriveNames, -1, 26);

		char drv[] = "a:\\";
		for (int i = 0; i < 26; ++i)
		{
			drv[0] = 'a' + (i + 2) % 26;
			if (GetDriveTypeA(drv) == DRIVE_FIXED)
			{
				CDDriveManagerClass::Instance->CDDriveNames[0] = (i + 2) % 26;
				CDDriveManagerClass::Instance->NumCDDrives = 1;
				break;
			}
		}
	}
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
	Phobos::Config::DigitalDisplay_Enable = pRA2MD->ReadBool(PHOBOS_STR, "DigitalDisplay.Enable", false);

	// Custom game speeds, 6 - i so that GS6 is index 0, just like in the engine
	Phobos::Config::CampaignDefaultGameSpeed = 6 - pRA2MD->ReadInteger(PHOBOS_STR, "CampaignDefaultGameSpeed", 4);

	if (Phobos::Config::CampaignDefaultGameSpeed > 6 || Phobos::Config::CampaignDefaultGameSpeed < 0)
		Phobos::Config::CampaignDefaultGameSpeed = 2;

	{
		BYTE defaultspeed = (BYTE)Phobos::Config::CampaignDefaultGameSpeed;
		// We overwrite the instructions that force GameSpeed to 2 (GS4)
		Patch::Apply_RAW(0x55D77A, sizeof(defaultspeed), &defaultspeed);

		// when speed control is off. Doesn't need a hook.
		Patch::Apply_RAW(0x55D78D , sizeof(defaultspeed), &defaultspeed);
	}

	CCFileClass UIMD_ini { UIMD_FILENAME };

	if(UIMD_ini.Exists() && UIMD_ini.Open(FileAccessMode::Read))
	{
		CCINIClass INI_UIMD { };
		INI_UIMD.ReadCCFile(&UIMD_ini);
		Debug::Log("Loading early %s file\n", UIMD_FILENAME);

		// LoadingScreen
		{
			Phobos::UI::DisableEmptySpawnPositions =
				INI_UIMD.ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
		}

		// UISettings
		{
			INI_UIMD.ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonLabel", "GUI:Resume", Phobos::readBuffer);
			Phobos::UI::ShowBriefingResumeButtonLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"");

			INI_UIMD.ReadString(UISETTINGS_SECTION, "ShowBriefingResumeButtonStatusLabel", "STT:BriefingButtonReturn", Phobos::readBuffer);
			strcpy_s(Phobos::UI::ShowBriefingResumeButtonStatusLabel, Phobos::readBuffer);
		}

		// ToolTips
		{
			Phobos::UI::ExtendedToolTips =
				INI_UIMD.ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

			Phobos::UI::MaxToolTipWidth =
				INI_UIMD.ReadInteger(TOOLTIPS_SECTION, "MaxWidth", 0);

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "CostLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "PowerLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1"); // ⚡

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "PowerBlackoutLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "TimeLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "PercentLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::PercentLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚

			INI_UIMD.ReadString(TOOLTIPS_SECTION, "RadarJammedLabel", NONE_STR, Phobos::readBuffer);
			Phobos::UI::BuidingRadarJammedLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"Radar Jammed");
		}

		// Sidebar
		{
			Phobos::UI::ShowHarvesterCounter =
				INI_UIMD.ReadBool(SIDEBAR_SECTION_T, "HarvesterCounter.Show", false);

			INI_UIMD.ReadString(SIDEBAR_SECTION_T, "HarvesterCounter.Label", NONE_STR, Phobos::readBuffer);
			Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

			Phobos::UI::HarvesterCounter_ConditionYellow =
				INI_UIMD.ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

			Phobos::UI::HarvesterCounter_ConditionRed =
				INI_UIMD.ReadDouble(SIDEBAR_SECTION_T, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

			Phobos::UI::ShowProducingProgress =
				INI_UIMD.ReadBool(SIDEBAR_SECTION_T, "ProducingProgress.Show", false);

			Phobos::UI::ShowPowerDelta =
				INI_UIMD.ReadBool(SIDEBAR_SECTION_T, "PowerDelta.Show", false);

			Phobos::UI::PowerDelta_ConditionYellow =
				INI_UIMD.ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

			Phobos::UI::PowerDelta_ConditionRed =
				INI_UIMD.ReadDouble(SIDEBAR_SECTION_T, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);

			Phobos::Config::TogglePowerInsteadOfRepair =
				INI_UIMD.ReadBool(SIDEBAR_SECTION_T, "TogglePowerInsteadOfRepair", false);

			Phobos::UI::CenterPauseMenuBackground =
				INI_UIMD.ReadBool(SIDEBAR_SECTION_T, "CenterPauseMenuBackground", Phobos::UI::CenterPauseMenuBackground);

		}
	}
	else
	{
		Debug::Log(FAILEDTOLOADUIMD_MSG);
	}

	CCFileClass RULESMD_ini { GameStrings::RULESMD_INI() };
	if (RULESMD_ini.Exists() && RULESMD_ini.Open(FileAccessMode::Read))
	{
		CCINIClass INI_RulesMD { };
		INI_RulesMD.ReadCCFile(&RULESMD_ini);

		Debug::Log("Loading early %s file\n", GameStrings::RULESMD_INI());


		if (!Phobos::Otamaa::IsAdmin)
			Phobos::Config::DevelopmentCommands = INI_RulesMD.ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

		Phobos::Otamaa::DisableCustomRadSite = INI_RulesMD.ReadBool(PHOBOS_STR, "DisableCustomRadSite", false);
		Phobos::Config::ArtImageSwap = INI_RulesMD.ReadBool(GENERAL_SECTION, "ArtImageSwap", false);

		if (INI_RulesMD.ReadBool(GENERAL_SECTION, "CustomGS", false))
		{
			Phobos::Misc::CustomGS = true;

			//char tempBuffer[0x20];
			for (size_t i = 0; i <= 6; ++i)
			{
				std::string _buffer = "CustomGS";
				_buffer += std::to_string(6 - i);

				int temp = INI_RulesMD.ReadInteger(GENERAL_SECTION, (_buffer + ".ChangeDelay").c_str(), -1);
				if (temp >= 0 && temp <= 6)
					Phobos::Misc::CustomGS_ChangeDelay[i] = 6 - temp;

				temp = INI_RulesMD.ReadInteger(GENERAL_SECTION, (_buffer + ".DefaultDelay").c_str(), -1);
				if (temp >= 1)
					Phobos::Misc::CustomGS_DefaultDelay[i] = 6 - temp;

				temp = INI_RulesMD.ReadInteger(GENERAL_SECTION, (_buffer + ".ChangeInterval").c_str(), -1);
				if (temp >= 1)
					Phobos::Misc::CustomGS_ChangeInterval[i] = temp;
			}
		}

		Phobos::Config::MultiThreadSinglePlayer = INI_RulesMD.ReadBool(GENERAL_SECTION, "MultiThreadSinglePlayer", false);
		Phobos::Config::HideLightFlashEffects = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "HideLightFlashEffects", false);
		Phobos::Config::SaveVariablesOnScenarioEnd = INI_RulesMD.ReadBool(GENERAL_SECTION, "SaveVariablesOnScenarioEnd", Phobos::Config::SaveVariablesOnScenarioEnd);
		Phobos::Config::ApplyShadeCountFix = INI_RulesMD.ReadBool(AUDIOVISUAL_SECTION, "ApplyShadeCountFix", Phobos::Config::ApplyShadeCountFix);
		Phobos::Config::SaveGameOnScenarioStart = CCINIClass::INI_RA2MD->ReadBool(PHOBOS_STR, "SaveGameOnScenarioStart", true);
		Phobos::UI::UnlimitedColor = INI_RulesMD.ReadBool(GENERAL_SECTION, "SkirmishUnlimitedColors", Phobos::UI::UnlimitedColor);
	}
}

void Phobos::ThrowUsageWarning(CCINIClass* pINI)
{
	//there is only few mod(s) that using this
	//just add your mod name or remove these code if you dont like it
	if (!Phobos::Otamaa::IsAdmin)
	{
		if(pINI->ReadString(GENERAL_SECTION, GameStrings::Name, "", Phobos::readBuffer) <= 0)
			return;

		const std::string ModNameTemp = Phobos::readBuffer;

		if (!ModNameTemp.empty())
		{
			std::size_t nFInd = ModNameTemp.find("Rise of the East");
			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("Ion Shock");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("New War");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("NewWar");

			if (nFInd == std::string::npos)
			{
				MessageBoxW(NULL,
					L"This is not Official Phobos Build.\n\n"
					L"Please contact original Mod Author for Assistance !.",
					L"Warning !", MB_OK);
			}
		}
	}
}

void Phobos::DrawVersionWarning()
{
	if (!Phobos::Config::HideWarning)
	{
		const auto pSurface = DSurface::Composite();
		if (!pSurface || VTable::Get(pSurface) != DSurface::vtable)
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

void Phobos::InitAdminDebugMode()
{
	if (!Phobos::Otamaa::IsAdmin)
		return;

	// this thing can cause game to lockup when loading data
	//better disable it for release
	const bool Detached =
		Phobos::DetachFromDebugger();

	if (Detached)
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

void Phobos::InitConsole()
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

bool HasCNCnet = false;

void Phobos::ExeRun()
{
	Phobos::Otamaa::ExeTerminated = false;
	Game::Savegame_Magic = 0x1414D121;
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	Patch::PrintAllModuleAndBaseAddr();
	Phobos::InitAdminDebugMode();

	for (auto&dlls : Patch::ModuleDatas) {
		if (IS_SAME_STR_(dlls.ModuleName.c_str(), "cncnet5.dll")) {
			HasCNCnet = true;
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), ARES_DLL_S)) {
			Debug::FatalErrorAndExit("dont need Ares.dll to run! \n");
		}
	}

}

void Phobos::ExeTerminate()
{
	Phobos::Otamaa::ExeTerminated = true;
	Debug::LogFileClose(111);
	Console::Release();
}

#pragma warning( push )
#pragma warning (disable : 4091)
#pragma warning (disable : 4245)
bool Phobos::DetachFromDebugger()
{
	DWORD ret = false;

	for (auto& module : Patch::ModuleDatas) {

		if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll") && module.Handle != NULL) {

			auto const NtRemoveProcessDebug =
				(NTSTATUS(__stdcall*)(HANDLE, HANDLE))GetProcAddress(module.Handle, "NtRemoveProcessDebug");
			auto const NtSetInformationDebugObject =
				(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(module.Handle, "NtSetInformationDebugObject");
			auto const NtQueryInformationProcess =
				(NTSTATUS(__stdcall*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(module.Handle, "NtQueryInformationProcess");
			auto const NtClose =
				(NTSTATUS(__stdcall*)(HANDLE))GetProcAddress(module.Handle, "NtClose");

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
		}
	}

	return ret;
}
#pragma warning( pop )

#pragma endregion

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
		//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		//std::atexit(Phobos::_dump_memory_leaks);

		Phobos::hInstance = hInstance;
		Debug::LogFileRemove();
		/* There is an issue with these , sometime it will crash when game DynamicVector::resize called
		  not really sure what is the real cause atm .///*/
		Patch::Apply_LJMP(0x7D107D, &_msize);
		Patch::Apply_LJMP(0x7D5408, &_strdup);
		Patch::Apply_LJMP(0x7C8E17, &malloc);
		Patch::Apply_LJMP(0x7C9430, &malloc);
		Patch::Apply_LJMP(0x7D3374, &calloc);
		Patch::Apply_LJMP(0x7D0F45, &realloc);
		Patch::Apply_LJMP(0x7C8B3D, &free);
		Patch::Apply_LJMP(0x7C93E8, &free);
		Patch::Apply_LJMP(0x7C9CC2, &std::strtok);
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
	Phobos::ExeTerminate();
	CRT::exit_noreturn(0);
	return 0x0;
}

DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
{
	Patch::ApplyStatic();
	Phobos::ExeRun();

	return 0;
}

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();
	Phobos::InitConsole();

	return 0;
}
#pragma endregion