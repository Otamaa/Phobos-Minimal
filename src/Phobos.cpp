#include <Phobos.h>

#ifdef ENABLE_CLR
#include <Scripting/CLR.h>
#endif

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <filesystem>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <Misc/Patches.h>

#include <Misc/PhobosGlobal.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Spawner/Main.h>

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>
#include <WinBase.h>
#include <CD.h>
#include <aclapi.h>

#include "Lib/Lua542/lua.hpp"

#pragma region DEFINES
struct luastatedeleter {
	void operator ()(lua_State* l) noexcept {
		if (l) {
			lua_close(l);
		}
	}
};
#ifndef IS_RELEASE_VER
bool Phobos::Config::HideWarning = false;
#endif

using unique_luastate = std::unique_ptr<lua_State, luastatedeleter>;
#define make_unique_luastate(to) unique_luastate to {}; to.reset(luaL_newstate())
#define close_unique_luastate(to) to.reset(nullptr)

// TODO : encryption support
// Otamaa : change this variable if you want to load desired name lua file
static const char* filename = "renameinternal.lua";

static std::unordered_map<uintptr_t, std::string> map_replaceAddrTo {};
static std::string MainWindowStr {};

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
char Phobos::AppName[0x40] = "";

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

bool Phobos::EnableConsole = false;

#ifdef ENABLE_TLS
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_1;
DWORD TLS_Thread::dwTlsIndex_SHPDRaw_2;
#endif

#pragma endregion

#pragma region PhobosFunctions
void Phobos::CheckProcessorFeatures()
{
	BOOL supported = FALSE;
#if _M_IX86_FP == 0 // IA32
	Debug::Log("Phobos is not using enhanced instruction set.\n");
#elif _M_IX86_FP == 1 // SSE
#define INSTRUCTION_SET_NAME "SSE"
	supported = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
#elif _M_IX86_FP == 2 && !__AVX__ // SEE2, not AVX
#define INSTRUCTION_SET_NAME "SSE2"
	supported = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
#else // all others, untested. add more #elifs to support more
	static_assert(false, "Phobos compiled using unsupported architecture.");
#endif

#ifdef INSTRUCTION_SET_NAME
	Debug::Log("Phobos requires a CPU with " INSTRUCTION_SET_NAME " support. %s.\n",
		supported ? "Available" : "Not available");

	if (!supported)
	{
		//doesnot get inlined when using reference<>
		MessageBoxA(Game::hWnd.get(),
			"This version of Phobos requires a CPU with " INSTRUCTION_SET_NAME
			" support.\n\nYour CPU does not support " INSTRUCTION_SET_NAME ". "
			"Game will now exit.",
			"Phobos - CPU Requirements", MB_ICONERROR);

		Debug::Log("Game will now exit.\n");
		Debug::ExitGame(533u);
	}
#endif
}

void Phobos::ExecuteLua()
{
	make_unique_luastate(unique_lua);
	auto L = unique_lua.get();

	if (luaL_dofile(L, "adminmode.lua") == LUA_OK)
	{
		lua_getglobal(L, "AdminMode");

		if (lua_isstring(L, -1) == 1)
		{
			std::string adminName = lua_tostring(L, -1);
			if (adminName.size() <= MAX_COMPUTERNAME_LENGTH + 1)
			{
				DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
				TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
				GetComputerName(PCName, &dwSize);

				if (IS_SAME_STR_(PCName, adminName.c_str()))
				{
					Phobos::EnableConsole = true;
					Phobos::Config::MultiThreadSinglePlayer = true;
					Phobos::Config::DebugFatalerrorGenerateDump = true;
					Phobos::Otamaa::IsAdmin = true;
				}
			}
		}
	}

	if (luaL_dofile(L, filename) == LUA_OK)
	{
		lua_getglobal(L, "Replaces"); // T ==> -1

		// get the first table
		if (lua_istable(L, -1))
		{ // is T table ?
			const int replace_size = (int)lua_rawlen(L, -1);

			for (size_t i = 0; i < replace_size; i++)
			{
				lua_pushinteger(L, i + 1);
				lua_gettable(L, -2);

				if (lua_istable(L, -2))
				{
					lua_pushstring(L, "Addr");
					lua_gettable(L, -2);
					const auto addr = (uintptr_t)lua_tointeger(L, -1);
					auto& result = map_replaceAddrTo[addr];
					const auto maxlen = strlen((const char*)addr);
					lua_pop(L, 1);

					lua_pushstring(L, "To");
					lua_gettable(L, -2);
					result = lua_tostring(L, -1);
					lua_pop(L, 1);
					DWORD protectFlag;

					if (Phobos::Otamaa::IsAdmin)
					{
						std::string copy = trim(result.c_str());
						Debug::Log("Patching string [%d] [0x%x - %s (%d) - max %d]\n", i, addr, copy.c_str(), result.size(), maxlen);
					}

					// do not exceed maximum length of the string , otherwise it will broke the .exe file
					Patch::Apply_withmemcpy(addr, result.c_str(), protectFlag, PAGE_READWRITE, (size_t)maxlen);
				}

				lua_pop(L, 1);
			}
		}

		lua_getglobal(L, "MainWindowString");
		if (lua_isnil(L, -1) == 0 && lua_isinteger(L, -1) != 1 && lua_isstring(L, -1) == 1)
		{
			MainWindowStr = lua_tostring(L, -1);
			Patch::Apply_OFFSET(0x777CC6, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CCB, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D6D, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777D72, (uintptr_t)MainWindowStr.c_str());
			Patch::Apply_OFFSET(0x777CA1, (uintptr_t)MainWindowStr.c_str());
		}

		lua_getglobal(L, "MovieMDINI");
		if (lua_isnil(L, -1) == 0 && lua_isinteger(L, -1) != 1 && lua_isstring(L, -1) == 1)
		{
			StaticVars::MovieMDINI = lua_tostring(L, -1);
		}
	}
	else
	{
		Debug::Log("Cannot find %s file\n", filename);
	}
}

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	DWORD_PTR processAffinityMask = 1; // limit to first processor
	bool consoleEnabled = false;

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
			auto nData = atoi(pArg + 0xAu);
			if (nData < 0)
				nData = 0;

			processAffinityMask = nData;
		}

		SpawnerMain::CmdLineParse(pArg);
	}

#ifndef aaa
	if (Debug::LogEnabled) {
		Debug::LogFileOpen();
		Debug::Log("Initialized Phobos %s .\n" , PRODUCT_VERSION);
		Debug::Log("args %s\n", args.c_str());

		if (consoleEnabled)
			Phobos::EnableConsole = true;

		SpawnerMain::PrintInitializeLog();
	}
#endif

	Phobos::CheckProcessorFeatures();

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
		Debug::Log("Loading early %s file\n", UIMD_ini.FileName);

		AresGlobalData::ReadAresRA2MD(&INI_UIMD);

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

		Debug::Log("Loading early %s file\n", RULESMD_ini.FileName);


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

		if (INI_RulesMD.ReadBool(GENERAL_SECTION, "FixTransparencyBlitters", false)) {
			BlittersFix::Apply();
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
#ifdef DETACH_DEBUGGER
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
#endif
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
			Debug::FatalErrorAndExit("This dll dont need Ares.dll to run! \n");
		}
	}

	PhobosGlobal::Init();
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
		Patch::Apply_LJMP(0x7D107D, &Phobos::_msize);
		Patch::Apply_LJMP(0x7D5408, &Phobos::_strdup);
		Patch::Apply_LJMP(0x7C8E17, &Phobos::_allocate);
		Patch::Apply_LJMP(0x7C9430, &Phobos::_allocate);
		Patch::Apply_LJMP(0x7D3374, &Phobos::_count_allocate);
		Patch::Apply_LJMP(0x7D0F45, &Phobos::_reallocate);
		Patch::Apply_LJMP(0x7C8B3D, &Phobos::_free);
		Patch::Apply_LJMP(0x7C93E8, &Phobos::_free);
		Patch::Apply_LJMP(0x7C9CC2, &std::strtok);
	}
	break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

// =============================
//#pragma region SyringeHandshake
//SYRINGE_HANDSHAKE(pInfo)
//{
//	const DWORD YR_SIZE_1000 = 0x496110;
//	const DWORD YR_SIZE_1001 = 0x497110;
//	const DWORD YR_SIZE_1001_UC = 0x497FE0;
//	const DWORD YR_SIZE_NPATCH = 0x5AB000;
//
//	const DWORD YR_TIME_1000 = 0x3B846665;
//	const DWORD YR_TIME_1001 = 0x3BDF544E;
//
//	const DWORD YR_CRC_1000 = 0xB701D792;
//	const DWORD YR_CRC_1001_CD = 0x098465B3;
//	const DWORD YR_CRC_1001_TFD = 0xEB903080;
//	const DWORD YR_CRC_1001_UC = 0x1B499086;
//
//	if (pInfo)
//	{
//		constexpr const char* AcceptMsg = "Found Yuri's Revenge %s. Applying Phobos " FILE_VERSION_STR ".";
//		constexpr const char* PatchDetectedMessage = "Found %s. Phobos " FILE_VERSION_STR" is not compatible with Exe patched Gamemd.";
//
//		const char* desc = nullptr;
//		const char* msg = nullptr;
//		bool allowed = false;
//
//		 accept tfd and cd version 1.001
//		if (pInfo->exeTimestamp == YR_TIME_1001)
//		{
//			 don't accept expanded exes
//			switch (pInfo->exeFilesize)
//			{
//			case YR_SIZE_1001:
//			case YR_SIZE_1001_UC:
//
//				 all versions allowed
//				switch (pInfo->exeCRC)
//				{
//				case YR_CRC_1001_CD:
//					desc = "1.001 (CD)";
//					break;
//				case YR_CRC_1001_TFD:
//					desc = "1.001 (TFD)";
//					break;
//				case YR_CRC_1001_UC:
//					desc = "1.001 (UC)";
//					break;
//				default:
//					 no-cd, network port or other changes
//					desc = "1.001 (modified)";
//				}
//				msg = AcceptMsg;
//				allowed = true;
//				break;
//
//			case YR_SIZE_NPATCH:
//				 known patch size
//				desc = "RockPatch or an NPatch-derived patch";
//				msg = PatchDetectedMessage;
//				break;
//			default:
//				 expanded exe, unknown make
//				desc = "an unknown game patch";
//				msg = PatchDetectedMessage;
//			}
//		}
//		else if (pInfo->exeTimestamp == YR_TIME_1000)
//		{
//			 upgrade advice for version 1.000
//			desc = "1.000";
//			msg = "Found Yuri's Revenge 1.000 but Phobos " FILE_VERSION_STR " requires version 1.001. Please update your copy of Yuri's Revenge first.";
//		}
//		else
//		{
//			 does not even compute...
//			msg = "Unknown executable. Phobos " FILE_VERSION_STR " requires Command & Conquer Yuri's Revenge version 1.001 (gamemd.exe).";
//		}
//
//		 generate the output message
//		if (pInfo->Message)
//		{
//			sprintf_s(pInfo->Message, pInfo->cchMessage, msg, desc);
//		}
//
//		return allowed ? S_OK : S_FALSE;
//	}
//
//	return E_POINTER;
//}
//#pragma endregion

#pragma region hooks

//DEFINE_HOOK(0x6BBFCE, WinMain_SetFPU_SyncDLL, 0x5)
//{
//	_set_controlfp(_RC_CHOP, _MCW_RC);
//
//	fesetround(FE_TOWARDZERO);
//	return 0x0;
//}

//DEFINE_JUMP(LJMP, 0x7CD8EA, GET_OFFSET(_ExeTerminate));
#ifndef aaa
DEFINE_HOOK(0x7cd8ef, Game_ExeTerminate, 9)
#else
DEFINE_HOOK(0x7cd8ef, Game_ExeTerminate, 9)
#endif
{
	Phobos::ExeTerminate();
	CRT::exit_noreturn(0);
	return 0x0;
}

//DEFINE_HOOK(0x7C8B3D, Game_freeMem, 0x9)
//{
//	GET_STACK(void*, ptr, 0x4);
//
//	Debug::Log(__FUNCTION__ " CalledFrom 0x%08x ptr [0x%08x]\n", R->Stack<uintptr_t>(0x0) , ptr);
//	CRT::free(ptr);
//	return 0x7C8B47;
//}
#ifndef aaa
DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
#else
DEFINE_HOOK(0x7CD810, Game_ExeRun, 0x9)
#endif
{
	//Imports::ReadFile = ReadFIle_;
	//Imports::CreateFileA = CreatefileA_;
	//Imports::CloseHandle = CloseHandle_;

	Patch::ApplyStatic();

#ifdef ENABLE_ENCRYPTION_HOOKS
	Imports::OleLoadFromStream = OleLoadFromStream_;
#endif

	Phobos::ExeRun();
	SpawnerMain::ExeRun(HasCNCnet);

	return 0;
}

#ifndef aaa
//6BDB0C
DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
#else
DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
#endif
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();
	Phobos::ExecuteLua();
	Phobos::InitConsole();
	return 0;
}
#pragma endregion