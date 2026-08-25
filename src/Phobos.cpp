#include <Phobos.h>

#include <Phobos.lib.h>

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/CSFText.h>
#include <Utilities/GameConfig.h>
#include <Utilities/Parser.h>
#include <Utilities/Handle.h>

#include <Misc/Patches.h>
#include <Misc/PhobosGlobal.h>
#include <Misc/Spawner/Main.h>
#include <Misc/Hooks.CRT.h>

#include <Dbghelp.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <cfenv>
#include <WinBase.h>
#include <CD.h>
#include <aclapi.h>
#include <GameOptionsClass.h>
#include <LaserDrawClass.h>

#include <Phobos.Lua.h>
#include <Phobos.UI.h>
#include <Phobos.Defines.h>

#include <MessageBoxLogging.h>

#include <Misc/Renderer/GlobalColorPacker.h>
#include <Misc/Exception/ExceptionHandler.h>

#include <Ext/Convert/Body.h>

#pragma region defines
HANDLE Phobos::hInstance;
char Phobos::readBuffer[readLength] {};
wchar_t Phobos::wideBuffer[readLength] {};
const char Phobos::readDelims[4] { "," };
const char Phobos::readDefval[4] { "" };
std::string Phobos::AppIconPath {};
std::string Phobos::AppCursor {};
DrawDamageMode Phobos::Debug_DisplayDamageNumbers { DrawDamageMode::disabled };
const wchar_t* Phobos::VersionDescription { L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping." };
bool Phobos::ShouldQuickSave { false };
std::wstring Phobos::CustomGameSaveDescription {};
PVOID Phobos::pExceptionHandler { nullptr };
HMODULE Phobos::comctl32Handle { NULL };
ExceptionHandlerMode Phobos::ExceptionMode { ExceptionHandlerMode::Default };

bool Phobos::HasCNCnet { false };

std::mt19937 Phobos::Random::_engine;

bool Phobos::UI::DisableEmptySpawnPositions { false };
bool Phobos::UI::ExtendedToolTips { false };
int Phobos::UI::MaxToolTipWidth { 0 };
bool Phobos::UI::ShowHarvesterCounter { false };
double Phobos::UI::HarvesterCounter_ConditionYellow { 0.99 };
double Phobos::UI::HarvesterCounter_ConditionRed { 0.5 };
bool Phobos::UI::ShowProducingProgress { false };
bool Phobos::UI::ShowPowerDelta { false };
double Phobos::UI::PowerDelta_ConditionYellow { 0.75 };
double Phobos::UI::PowerDelta_ConditionRed { 1.0 };
bool Phobos::UI::CenterPauseMenuBackground { false };
bool Phobos::UI::WeedsCounter_Show { false };
bool Phobos::UI::UnlimitedColor { false };
bool Phobos::UI::AnchoredToolTips { false };

bool Phobos::UI::SuperWeaponSidebar { false };
int Phobos::UI::SuperWeaponSidebar_Interval { 0 };
int Phobos::UI::SuperWeaponSidebar_LeftOffset { 0 };
int Phobos::UI::SuperWeaponSidebar_CameoHeight { 48 };
int Phobos::UI::SuperWeaponSidebar_Max { 0 };
int Phobos::UI::SuperWeaponSidebar_MaxColumns { INT32_MAX };
int Phobos::UI::CreditsIndicator_MaxStep = 143;
bool Phobos::UI::SuperWeaponSidebar_Pyramid = true;

NullableCSF Phobos::UI::CostLabel { L"$" };
NullableCSF Phobos::UI::PowerLabel { L"\u26a1" }; // ⚡
NullableCSF Phobos::UI::PowerBlackoutLabel { L"\u26a1\u274c" };// ⚡❌
NullableCSF Phobos::UI::TimeLabel { L"\u231a" };// ⌚
NullableCSF Phobos::UI::HarvesterLabel { L"\u26cf" };// ⛏
NullableCSF Phobos::UI::PercentLabel { L"\u231a" };// ⌚
NullableCSF Phobos::UI::SWShotsFormat { L"%d/%d shots" };
NullableCSF Phobos::UI::BattlePoints_Label { L"\u2605: " }; // ★: 
NullableCSF Phobos::UI::BattlePointsSidebar_Label { L"\u2605: " };// ★:

const wchar_t* Phobos::UI::GameTimeText { };

const wchar_t* Phobos::UI::ShowBriefingResumeButtonLabel { L"" };
char Phobos::UI::ShowBriefingResumeButtonStatusLabel[0x20] { "" };

const wchar_t* Phobos::UI::BuidingRadarJammedLabel { L"" };
const wchar_t* Phobos::UI::BuidingFakeLabel { L"" };
const wchar_t* Phobos::UI::Power_Label { L"" };
const wchar_t* Phobos::UI::Drain_Label { L"" };
const wchar_t* Phobos::UI::Storage_Label { L"" };
const wchar_t* Phobos::UI::Radar_Label { L"" };
const wchar_t* Phobos::UI::Tech_Label { L"" };
const wchar_t* Phobos::UI::Spysat_Label { L"" };


int Phobos::UI::uiColorText;
int Phobos::UI::uiColorTextButton { 0xFFFF }; // #1644: needed for CD prompt
int Phobos::UI::uiColorTextCheckbox;
int Phobos::UI::uiColorTextRadio;
int Phobos::UI::uiColorTextLabel { 0xFFFF }; // #1644: needed for CD prompt
int Phobos::UI::uiColorTextList;
int Phobos::UI::uiColorTextCombobox;
int Phobos::UI::uiColorTextGroupbox;
int Phobos::UI::uiColorTextEdit;
int Phobos::UI::uiColorTextSlider;
int Phobos::UI::uiColorTextObserver;
int Phobos::UI::uiColorCaret;
int Phobos::UI::uiColorSelection;
int Phobos::UI::uiColorSelectionCombobox;
int Phobos::UI::uiColorSelectionList;
int Phobos::UI::uiColorSelectionObserver;
int Phobos::UI::uiColorBorder1;
int Phobos::UI::uiColorBorder2;
int Phobos::UI::uiColorDisabled;
int Phobos::UI::uiColorDisabledLabel;
int Phobos::UI::uiColorDisabledButton;
int Phobos::UI::uiColorDisabledCombobox;
int Phobos::UI::uiColorDisabledCheckbox;
int Phobos::UI::uiColorDisabledList;
int Phobos::UI::uiColorDisabledSlider;
int Phobos::UI::uiColorDisabledObserver;
ColorData Phobos::UI::Colors[16 + 1];

bool Phobos::UI::BattlePointsSidebar_Label_InvertPosition {};
bool Phobos::UI::BattlePointsSidebar_AlwaysShow { false };

bool Phobos::Config::HideWarning { false };
bool Phobos::Config::ToolTipDescriptions { true };
bool Phobos::Config::ToolTipBlur { false };
bool Phobos::Config::PriorityDeployFiltering { true };
bool Phobos::Config::TypeSelectUseIFVMode { true };
bool Phobos::Config::PrioritySelectionFiltering { true };
bool Phobos::Config::DevelopmentCommands { true };
bool Phobos::Config::ArtImageSwap { false };
bool Phobos::Config::EnableBuildingPlacementPreview { false };
bool Phobos::Config::EnableSelectBox { false };
bool Phobos::Config::TogglePowerInsteadOfRepair { false };
bool Phobos::Config::ShowTechnoNamesIsActive { false };
bool Phobos::Config::RealTimeTimers { false };
bool Phobos::Config::RealTimeTimers_Adaptive { false };
int Phobos::Config::CampaignDefaultGameSpeed { 2 };
bool Phobos::Config::DigitalDisplay_Enable { false };
bool Phobos::Config::MessageDisplayInCenter { false };
bool Phobos::Config::MessageApplyHoverState { false };
int Phobos::Config::MessageDisplayInCenter_BoardOpacity { 40 };
int Phobos::Config::MessageDisplayInCenter_LabelsCount { 4 };
int Phobos::Config::MessageDisplayInCenter_RecordsCount { 12 };
bool Phobos::Config::ShowBuildingStatistics { false };
bool Phobos::Config::ApplyShadeCountFi { true };
bool Phobos::Config::SaveVariablesOnScenarioEnd { false };
bool Phobos::Config::MultiThreadSinglePlayer { false };
bool Phobos::Config::UseImprovedPathfindingBlockageHandling { false };
bool Phobos::Config::HideLightFlashEffects { false };
bool Phobos::Config::HideLaserTrailEffects { false };
bool Phobos::Config::HideShakeEffects { false };
bool Phobos::Config::DebugFatalerrorGenerateDump { false };
bool Phobos::Config::SaveGameOnScenarioStart { true };
bool Phobos::Config::ShowPowerDelta { true };
bool Phobos::Config::ShowHarvesterCounter { true };
bool Phobos::Config::ShowWeedsCounter { false };
bool Phobos::Config::UseNewInheritance { false };
bool Phobos::Config::UseNewIncludes { false };
bool Phobos::Config::ApplyShadeCountFix { true };
bool Phobos::Config::ShowFlashOnSelecting { true };
bool Phobos::Config::AutoBuilding_Enable { false };
bool Phobos::Config::ScrollSidebarStripInTactical { true };
bool Phobos::Config::ScrollSidebarStripWhenHoldKey { true };

bool Phobos::Config::UnitPowerDrain { false };
bool Phobos::Config::AllowSwitchNoMoveCommand = false;
bool Phobos::Config::AllowDistributionCommand = false;
bool Phobos::Config::AllowDistributionCommand_SpreadMode = true;
bool Phobos::Config::AllowDistributionCommand_SpreadModeScroll = true;
bool Phobos::Config::AllowDistributionCommand_FilterMode = true;
bool Phobos::Config::AllowDistributionCommand_AffectsAllies = true;
bool Phobos::Config::AllowDistributionCommand_AffectsEnemies = true;
bool Phobos::Config::ApplyNoMoveCommand = true;
int Phobos::Config::DistributionSpreadMode = 2;
int Phobos::Config::DistributionFilterMode = 2;

int Phobos::Config::SuperWeaponSidebar_RequiredSignificance { 0 };
bool Phobos::Config::SuperWeaponSidebarCommands { false };

bool Phobos::Config::SelectedDisplay_Enable { true };
bool Phobos::Config::SelectedDisplay_Expand { false };
int Phobos::Config::SelectedDisplay_MaxCameo { 10 };

DWORD  Phobos::Config::InternalVersion { 0x1414D121 };
std::string  Phobos::Config::ModName { "Yuri's Revenge" };
std::string  Phobos::Config::ModVersion { "1.001" };
int  Phobos::Config::ModIdentifier;
CSFText  Phobos::Config::ModNote;
byte  Phobos::Config::GFX_DX_Force;
int  Phobos::Config::colorCount { 8 };
int  Phobos::Config::version;

bool Phobos::Config::ShowPowerPlantEnhancerRange = false;
bool Phobos::Config::FixTransparencyBlitters = false;
Simd::Level Phobos::Config::MaxSimdLevel = Simd::Level::AVX2;
bool Phobos::Misc::CustomGS { false };
int Phobos::Misc::CustomGS_ChangeInterval[7] { -1, -1, -1, -1, -1, -1, -1 };
int Phobos::Misc::CustomGS_ChangeDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
int Phobos::Misc::CustomGS_DefaultDelay[7] { 0, 1, 2, 3, 4, 5, 6 };

bool Phobos::Config::ShowGameTime = false;
int Phobos::Config::ShowGameTime_BoardOpacity = 40;

bool Phobos::Config::SelectCapturedCommand = false;

bool Phobos::Otamaa::DisableCustomRadSite { false };
bool Phobos::Otamaa::IsAdmin { false };
bool Phobos::Otamaa::ShowHealthPercentEnabled { false };
bool Phobos::Otamaa::ExeTerminated { false };
bool Phobos::Otamaa::DoingLoadGame { false };
bool Phobos::Otamaa::AllowAIControl { false };
bool Phobos::Otamaa::OutputMissingStrings { false };
bool Phobos::Otamaa::OutputAudioLogs { false };
bool Phobos::Otamaa::StrictParser { false };
bool Phobos::Otamaa::ParserErrorDetected { false };
bool Phobos::Otamaa::TrackParserErrors { false };
bool Phobos::Otamaa::NoLogo { false };
bool Phobos::Otamaa::NoCD { false };
bool Phobos::Otamaa::CompatibilityMode { false };
bool Phobos::Otamaa::ReplaceGameMemoryAllocator { false };
bool Phobos::Otamaa::AllowMultipleInstance { false };
DWORD Phobos::Otamaa::PhobosBaseAddress { false };

#pragma endregion

bool Phobos::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)
		.Process(Phobos::Config::UnitPowerDrain)
		.Success();
}

bool Phobos::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)
		.Process(Phobos::Config::UnitPowerDrain)
		.Success();
}

struct GraphicsRuntimeAPI
{
	enum class Type
	{
		UNK, DX, DXGI, OGL, VK
	};

	GraphicsRuntimeAPI(const std::vector<dllData>& dlls)
		: name { "Unknown" }, type { Type::UNK }
	{
		for (auto& dll : dlls)
		{
			if (_strnicmp(dll.ModuleName.c_str(), "d3d", 3) == 0
				|| IS_SAME_STR_(dll.ModuleName.c_str(), "dxgi.dll")
				|| IS_SAME_STR_(dll.ModuleName.c_str(), "ddraw.dll")
				)
			{
				name = "DirectX";
				type = Type::DX;
				break;
			}
			else if (IS_SAME_STR_("opengl32.dll", dll.ModuleName.c_str()))
			{
				name = "OpenGL";
				type = Type::OGL;
				break;
			}
			else if (IS_SAME_STR_("vulkan-1.dll", dll.ModuleName.c_str()))
			{
				name = "Vulkan";
				type = Type::VK;
				break;
			}
		}
	}

	~GraphicsRuntimeAPI() = default;

	FORCEDINLINE COMPILETIMEEVAL const char* GetName() const
	{
		return name.c_str();
	}

	FORCEDINLINE COMPILETIMEEVAL Type GetType()
	{
		return type;
	}

private:
	std::string name;
	Type type;
};

// remove the comment if you want to run the dll with patched gamemd
//#define NO_SYRINGE

bool IsRunningInAppContainer()
{
	static bool s_checked = false;
	static bool s_isAppContainer = false;

	if (!s_checked)
	{
		HANDLE hToken;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			DWORD dwLength = 0;
			GetTokenInformation(hToken, TokenAppContainerSid, nullptr, 0, &dwLength);
			s_isAppContainer = (GetLastError() != ERROR_NOT_FOUND);
			CloseHandle(hToken);
		}
		s_checked = true;
	}

	return s_isAppContainer;
}

void OptimizeProcessForSecurity()
{
	if (IsRunningInAppContainer())
	{
		Debug::Log("App Container detected. Optimizing object creation.\n");

		// Set process mitigations only if available (Windows 8+)
		typedef BOOL(WINAPI* SetProcessMitigationPolicyFunc)(PROCESS_MITIGATION_POLICY, PVOID, SIZE_T);


#ifdef _Enable_these
		static bool s_checked = false;
		static bool s_isAppContainer = false;
		if (!s_checked)
		{
			SID_IDENTIFIER_AUTHORITY _ID {};
			HANDLE _Token {};
			DWORD _RetLength {};
			PSID _PID {};

			if (AllocateAndInitializeSid(&_ID, 1u, 0, 0, 0, 0, 0, 0, 0, 0, &_PID))
			{
				if (OpenProcessToken(Patch::CurrentProcess, 8u, &_Token))
				{
					GetTokenInformation(_Token, TokenUser, 0, 0, &_RetLength);
					if (_RetLength <= 0x400)
					{
						HLOCAL _Alloc = LocalAlloc(0x40u, 0x400u);
						if (GetTokenInformation(_Token, TokenUser, _Alloc, 0x400u, &_RetLength))
						{
							ACL _Acl {};
							if (InitializeAcl(&_Acl, 0x400u, 2u)
							&& AddAccessDeniedAce(&_Acl, 2u, 0xFAu, _PID)
							&& AddAccessAllowedAce(&_Acl, 2u, 0x100701u, _PID))
							{
								SetSecurityInfo(Patch::CurrentProcess, SE_KERNEL_OBJECT, 0x80000004, 0, 0, &_Acl, 0);
							}
						}

						s_checked = true;
					}
				}
			}

			if (_PID)
				FreeSid(_PID);
		}
#endif

		for (auto& module : Patch::ModuleDatas)
		{
			if (IS_SAME_STR_(module.ModuleName.c_str(), "kernel32.dll"))
			{
				SetProcessMitigationPolicyFunc pSetProcessMitigationPolicy =
					(SetProcessMitigationPolicyFunc)GetProcAddress(module.Handle, "SetProcessMitigationPolicy");

				if (pSetProcessMitigationPolicy)
				{
					// Use simplified mitigation
					pSetProcessMitigationPolicy((PROCESS_MITIGATION_POLICY)1, nullptr, 0);
				}
			}
		}

		// Reduce security descriptor checks
		SetThreadToken(nullptr, nullptr);
	}
}

#pragma region PhobosFunctions
void Phobos::CheckProcessorFeatures()
{
#ifdef _REQ_SSE

#if _M_IX86_FP != 2 //only SSE
	static_assert(false, "Phobos compiled using unsupported architecture.");
#endif

	const BOOL supported = IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
	Debug::Log("Phobos requires a CPU with SSE support. %s.\n",
		supported ? "Available" : "Not available");

	if (!supported)
	{
		//doesnot get inlined when using reference<>
		MessageBoxA(Game::hWnd.get(),
			"This version of Phobos requires a CPU with SSE"
			" support.\n\nYour CPU does not support SSE. "
			"Game will now exit.",
			"Phobos - CPU Requirements", MB_ICONERROR);

		Debug::Log("Game will now exit.\n");
		Debug::ExitGame(533u);
	}
#endif

}

void Phobos::PassiveSaveGame()
{
	GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_SAVING_GAME));
	const auto name = "Map." + Debug::GetCurTimeA() + ".sav";

	if (ScenarioClass::SaveGame(name.c_str(), Phobos::CustomGameSaveDescription.c_str()))
		GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_GAME_WAS_SAVED));
	else
		GeneralUtils::PrintMessage(StringTable::FetchString(GameStrings::TXT_ERROR_SAVING_GAME));
}


void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	DWORD_PTR processAffinityMask = 1; // limit to first processor
	// Enabled by default in all builds: an attached debugger receives
	// exceptions first, so the handler does not get in the way of debugging.
	bool dontSetExceptionHandler = false;

	// > 1 because the exe path itself counts as an argument, too!

	for (int i = 1; i < nNumArgs; i++)
	{
		const auto pArg = ppArgs[i];

		if (IS_SAME_STR_I(pArg, "-EXCEPTION") == 0)
		{
			ExceptionMode = ExceptionHandlerMode::NoRemove;
		} if (_stricmp(pArg, "-FullCrashDump") == 0) {
			ExceptionHandler::GenerateFullCrashDump = true;
		}
		else if (!strncasecmp(pArg, "-AFFINITY:", 0xAu))
		{
			int result = 1;
			if (Parser<int>::Parse(pArg + 0xAu, &result) && result < 0)
			{
				result = 0;
			}

			processAffinityMask = result;
		}
		else
		{
			const std::string cur = pArg;
			if (cur.starts_with("-ExceptionHandler="))
			{

				const size_t delim = cur.find("=");
				const std::string value = cur.substr(delim + 1, cur.size() - delim - 1);

				if (!value.empty())
				{
					Parser<bool>::Parse(value.data(), &dontSetExceptionHandler);
				}
			}
		}

		SpawnerMain::CmdLineParse(pArg);
	}

	if (Debug::LogEnabled) {
		SpawnerMain::PrintInitializeLog();
	} else {
		Debug::DeactivateLogger();
		Debug::LogFileRemove();
		Debug::made = false;// reset
	}

	Phobos::CheckProcessorFeatures();
	// Optimize for app container environments
	OptimizeProcessForSecurity();

	Game::DontSetExceptionHandler = dontSetExceptionHandler;
	Debug::Log("ExceptionHandler is %s .\n", dontSetExceptionHandler ? "not present" : "present");

	// Phobos replaces the game's exception handler with its own (see
	// ExceptionHandler.cpp); it is reachable exactly when the game's main
	// loop handler is armed, so it shares the -ExceptionHandler toggle.
	if (!dontSetExceptionHandler)
		ExceptionHandler::Init();
	
	if (processAffinityMask)
	{
		Debug::Log("Set Process Affinity: %d (%d).\n", processAffinityMask, processAffinityMask);
		SetProcessAffinityMask(Patch::CurrentProcess, processAffinityMask);
	}

	if (!CDDriveManagerClass::Instance->NumCDDrives)
	{
		Debug::Log("No CD drives detected. Switching to NoCD mode.\n");
		Phobos::Otamaa::NoCD = true;
	}

	if (Phobos::Otamaa::NoCD)
	{
		Debug::Log("Optimizing list of CD drives for NoCD mode.\n");
		__stosd(reinterpret_cast<unsigned long*>(CDDriveManagerClass::Instance->CDDriveNames.data()), 0xFFFFFFFF, CDDriveManagerClass::Instance->CDDriveNames.size());

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

void Phobos::ThrowUsageWarning(CCINIClass* pINI)
{
	//there is only few mod(s) that using this
	//just add your mod name or remove these code if you dont like it
	if (!Phobos::Otamaa::IsAdmin)
	{
		if (pINI->ReadString(GameStrings::General(), GameStrings::Name, "", Phobos::readBuffer) <= 0)
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
	const int marginX = Phobos::Config::MessageDisplayInCenter ? 28 : 10;
	int coordY = 0;

	if (!Phobos::Config::HideWarning)
	{
		const auto pSurface = DSurface::Composite();
		if (!pSurface || VTable::Get(pSurface) != DSurface::vtable)
			return;

		const auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, Point2D::Empty, TextPrintType::LASTPOINT, 2, 0);

		RectangleStruct rect {
			pSurface->Get_Width() - wanted.Width - marginX,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		pSurface->Fill_Rect(rect, COLOR_BLACK);
		pSurface->DSurfaceDrawText(Phobos::VersionDescription, &location, COLOR_RED);
		coordY = rect.Height;
	}

	if (!Phobos::Config::ShowGameTime || HouseClass::CurrentPlayer->IsObserver()) // already has a timer
		return;

	const auto& timer = ScenarioClass::Instance->ElapsedTimer;
	int currentTime = timer.TimeLeft;

	if (timer.StartTime != -1)
		currentTime += SystemTimer::GetTime() - timer.StartTime;

	currentTime /= 60;
	const int hours = currentTime / 3600;
	const int minutes = (currentTime / 60) % 60;
	const int seconds = currentTime % 60;
	const auto text = Phobos::UI::GameTimeText;
	static fmt::basic_memory_buffer<wchar_t> buffer;
	buffer.clear();

	if (hours > 0) {
		fmt::format_to(std::back_inserter(buffer), L"{} {}:{}:{}", text, hours, minutes, seconds);
	} else {
		fmt::format_to(std::back_inserter(buffer), L"{} {}:{}", text, minutes, seconds);
	}
	buffer.push_back(L'\0');

	auto wantedB = Drawing::GetTextDimensions(buffer.data(), Point2D::Empty, TextPrintType::LASTPOINT, 2, 0);

	RectangleStruct rectB = {
		DSurface::Composite->Get_Width() - wantedB.Width - marginX,
		coordY,
		wantedB.Width + 10,
		wantedB.Height + 10
	};

	Point2D locationB { rectB.X + 5, rectB.Y + 5 };
	ColorStruct color { 0x0, 0x0 ,0x0 };
	DSurface::Composite->Fill_Rect_Trans(&rectB, &color, Phobos::Config::ShowGameTime_BoardOpacity);
	DSurface::Composite->DSurfaceDrawText(buffer.data(), &locationB, COLOR_WHITE);
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
		EMIT_MSGBOXW(
		L"You can now attach a debugger.\n\n"
		L"Press OK to continue YR execution.",
		L"Debugger Notice");
	}
	else
	{
		EMIT_MSGBOXW(
		L"You can now attach a debugger.\n\n"
		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"
		L"Press OK to continue YR execution.",
		L"Debugger Notice");
	}

}

#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

//https://opengrok.libreoffice.org/xref/core/vcl/win/app/salinst.cxx?r=c35f8114#868
static std::string GetOsVersionQuick()
{
	std::string aVer { "Windows " }; // capacity for string like "Windows 6.1 Service Pack 1 build 7601"
	HMODULE kernel = NULL;
	HMODULE ntdll = NULL;

	for (auto& module : Patch::ModuleDatas)
	{
		if (IS_SAME_STR_(module.ModuleName.c_str(), "kernel32.dll"))
			kernel = module.Handle;
		else if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll"))
			ntdll = module.Handle;
	}
	// GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
	// subject to manifest-based behavior since Windows 8.1, so give wrong results.
	// Another approach would be to use NetWkstaGetInfo, but that has some small
	// reported delays (some milliseconds), and might get slower in domains with
	// poor network connections.
	// So go with a solution described at https://web.archive.org/web/20090228100958/http://msdn.microsoft.com/en-us/library/ms724429.aspx
	bool bHaveVerFromKernel32 = false;
	if (kernel)
	{
		wchar_t szPath[MAX_PATH] {};
		DWORD dwCount = GetModuleFileNameW(kernel, szPath, std::size(szPath));
		if (dwCount != 0 && dwCount < std::size(szPath))
		{
			dwCount = GetFileVersionInfoSizeW(szPath, nullptr);
			if (dwCount != 0)
			{
				std::unique_ptr<char[]> ver(new char[dwCount]);
				if (GetFileVersionInfoW(szPath, 0, dwCount, ver.get()) != FALSE)
				{
					void* pBlock = nullptr;
					UINT dwBlockSz = 0;
					if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
					{
						VS_FIXEDFILEINFO* vi1 = static_cast<VS_FIXEDFILEINFO*>(pBlock);
						aVer += (std::to_string(HIWORD(vi1->dwProductVersionMS)) + "."
									+ std::to_string(LOWORD(vi1->dwProductVersionMS)));
						bHaveVerFromKernel32 = true;
					}
				}
			}
		}
	}
	// Now use RtlGetVersion (which is not subject to deprecation for GetVersion(Ex) API)
	// to get build number and SP info
	bool bHaveVerFromRtlGetVersion = false;
	if (ntdll)
	{
		auto const RtlGetVersion_t =
			(NTSTATUS(WINAPI*)(LPOSVERSIONINFOEXW))GetProcAddress(ntdll, "RtlGetVersion");

		if (RtlGetVersion_t != NULL)
		{
			OSVERSIONINFOEXW vi2 {}; // initialize with zeroes - a better alternative to memset
			vi2.dwOSVersionInfoSize = sizeof(vi2);

			if (NULL == RtlGetVersion_t(&vi2))
			{
				if (vi2.dwBuildNumber < 21996)
				{

					if (!bHaveVerFromKernel32) // we failed above; let's hope this would be useful
						aVer += std::to_string(vi2.dwMajorVersion) + "." + std::to_string(vi2.dwMinorVersion);

					aVer += (" ");

					if (vi2.szCSDVersion[0])
						aVer += (PhobosCRT::WideStringToString(vi2.szCSDVersion) + " ");

				}
				else
				{
					aVer = "Windows 11 ";
				}

				aVer += ("Build " + std::to_string(vi2.dwBuildNumber));

				bHaveVerFromRtlGetVersion = true;
			}
		}
	}

	if (!bHaveVerFromKernel32 && !bHaveVerFromRtlGetVersion)
		aVer += "unknown";

	return aVer;
}

#include <commctrl.h>

// gamemd.exe has no manifest, so its windows bind the ancient Common Controls
// v5 and render Win9x-style. Activating Phobos' embedded manifest (resource 2,
// carrying the comctl32 v6 dependency - see ExceptionHandler.rc) for the rest
// of the process' lifetime makes every window created on the main thread use
// modern visual styles: game dialogs, message boxes and the crash dialog.
void ActivateCommonControls6()
{
	char modulePath[MAX_PATH] = { };
	GetModuleFileNameA(static_cast<HMODULE>(Phobos::hInstance), modulePath, sizeof(modulePath));

	ACTCTXA actCtx = { };
	actCtx.cbSize = sizeof(actCtx);
	actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
	actCtx.lpSource = modulePath;
	actCtx.lpResourceName = MAKEINTRESOURCEA(2); // ISOLATIONAWARE_MANIFEST_RESOURCE_ID

	HANDLE const hActCtx = CreateActCtxA(&actCtx);
	ULONG_PTR cookie = 0;
	if (hActCtx != INVALID_HANDLE_VALUE)
		ActivateActCtx(hActCtx, &cookie); // deliberately never deactivated

	// gamemd.exe has no manifest, so it loaded Common Controls v5 at startup
	// and the v6 theming subclasses were never installed. With the context
	// now active, loading comctl32 resolves to the v6 side-by-side assembly;
	// InitCommonControlsEx then registers its themed classes. Without this,
	// activating the manifest alone leaves controls rendering unthemed.
	using InitCommonControlsEx_t = BOOL(WINAPI*)(const INITCOMMONCONTROLSEX*);
	if (auto const pInit = reinterpret_cast<InitCommonControlsEx_t>(GetProcAddress(Phobos::comctl32Handle, "InitCommonControlsEx"))) {
			INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES };
			pInit(&icc);
	}
}

void Phobos::ExeRun()
{
	Phobos::Otamaa::ExeTerminated = false;
	Game::Savegame_Magic = Phobos::GetVersionNumber();
	Game::bVideoBackBuffer = false;
	Game::bAllowVRAMSidebar = false;

	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Cursor_36).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IvanBomb).FrameRate = 4;

	Phobos::comctl32Handle = LoadLibraryA("comctl32.dll");

	if (Phobos::comctl32Handle == NULL)
		Debug::FatalErrorAndExit("Uneable to load comctl32.dll !");

	ActivateCommonControls6();

	Patch::PrintAllModuleAndBaseAddr();

#if !defined(NO_SYRINGE)
	Phobos::InitAdminDebugMode();
#endif

	int i = 0;

	for (auto& dlls : Patch::ModuleDatas)
	{
		Debug::Log("Module [(%d) %s: Base address = %x]\n", i++, dlls.ModuleName.c_str(), dlls.BaseAddr);

		if (IS_SAME_STR_(dlls.ModuleName.c_str(), "cncnet5.dll"))
		{
			Debug::FatalErrorAndExit("This dll dont need cncnet5.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), ARES_DLL_S))
		{
			Debug::FatalErrorAndExit("This dll dont need Ares.dll to run!, please remove first");
		}
		else if (IS_SAME_STR_(dlls.ModuleName.c_str(), PHOBOS_DLL_S))
		{
			Phobos::Otamaa::PhobosBaseAddress = dlls.BaseAddr;
		}
	}

	Patch::WindowsVersion = std::move(GetOsVersionQuick());
	Debug::Log("Running on %s .\n", Patch::WindowsVersion.c_str());
	GraphicsRuntimeAPI gRuntimeAPI(Patch::ModuleDatas);
	Debug::Log("Running on %s API.\n", gRuntimeAPI.GetName());
	TheaterTypeClass::AddDefaults();
	CursorTypeClass::AddDefaults();
}

void Phobos::ExeTerminate()
{
	Debug::DeactivateLogger();

	if (!Phobos::Otamaa::ExeTerminated)
	{
		Phobos::Otamaa::ExeTerminated = true;

		for (auto& handle : Handles::Array) {
			handle->detachptr();
		}
		Handles::Array.clear();
		Patch::ModuleDatas.clear();
	}
}

#pragma warning( push )
#pragma warning (disable : 4091)
#pragma warning (disable : 4245)
bool Phobos::DetachFromDebugger()
{
	DWORD ret = false;
	HMODULE ntdll = NULL;

	for (auto& module : Patch::ModuleDatas)
	{
		if (IS_SAME_STR_(module.ModuleName.c_str(), "ntdll.dll"))
		{
			ntdll = module.Handle;
			break;
		}
	}

	if (ntdll != NULL)
	{

		auto const NtRemoveProcessDebug =
			(NTSTATUS(WINAPI*)(HANDLE, HANDLE))GetProcAddress(ntdll, "NtRemoveProcessDebug");
		auto const NtSetInformationDebugObject =
			(NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(ntdll, "NtSetInformationDebugObject");
		auto const NtQueryInformationProcess =
			(NTSTATUS(WINAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG))GetProcAddress(ntdll, "NtQueryInformationProcess");
		auto const NtClose =
			(NTSTATUS(WINAPI*)(HANDLE))GetProcAddress(ntdll, "NtClose");

		HANDLE hDebug {};
		NTSTATUS status = NtQueryInformationProcess(Patch::CurrentProcess, 30, &hDebug, sizeof(HANDLE), 0);
		if (0 <= status)
		{
			ULONG killProcessOnExit = FALSE;
			status = NtSetInformationDebugObject(
				hDebug, 1, &killProcessOnExit, sizeof(ULONG), NULL);

			if (0 <= status)
			{
				const auto pid = Patch::GetDebuggerProcessId(GetProcessId(Patch::CurrentProcess));
				status = NtRemoveProcessDebug(Patch::CurrentProcess, hDebug);
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
	}

	return ret;
}
#pragma warning( pop )


#pragma endregion

#include <Misc/Multithread.h>
#include <Phobos.Hookers.h>

NOINLINE void EnableLargeAddressAwareFlag(HANDLE curProc)
{
	BYTE* base = (BYTE*)curProc; // base of gamemd.exe
	DWORD peOffset = *(DWORD*)(base + 0x3C);
	WORD* characteristics = (WORD*)(base + peOffset + 0x18);

	DWORD oldProtect;
	if (VirtualProtect(characteristics, sizeof(WORD), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		*characteristics |= 0x20; // IMAGE_FILE_LARGE_ADDRESS_AWARE
		VirtualProtect(characteristics, sizeof(WORD), oldProtect, &oldProtect);
		Debug::Log("LARGEADDRESSAWARE flag set via injector.\n");
	}
	else
	{
		Debug::Log("Failed to change protection for Characteristics.\n");
	}
}

NOINLINE bool IsGamemdExe(HMODULE curProc)
{

	constexpr static const wchar_t* gameExecutables[] = {
		L"gamemd.exe",      // Yuri's Revenge
		L"gamepp.exe",      // Possible variant
	};

	constexpr static size_t executableCount = sizeof(gameExecutables) / sizeof(gameExecutables[0]);


	wchar_t filename[MAX_PATH];
	GetModuleFileNameW(curProc, filename, MAX_PATH);

	// Get just the filename without path
	const wchar_t* execName = wcsrchr(filename, L'\\');
	if (execName)
	{
		execName++; // Skip the backslash
	}
	else
	{
		execName = filename; // No path separator found
	}

	std::wstring path(filename);
	std::ranges::transform(path, path.begin(), ::towlower);

	for (size_t i = 0; i < executableCount; ++i) {
		if (path.find(gameExecutables[i]) != std::wstring::npos) {
			return true;
		}
	}

	return false;
}

LPVOID saved_lpReserved;
bool IsInitialized = false;

NOINLINE void ApplyEarlyFuncs()
{
	{
		if (!IsInitialized)
			exit(ERROR);

		const auto time = Debug::GetCurTimeA();
		const char* loadMode = saved_lpReserved ? "statically" : "dynamicly";

		Debug::Log("Phobos is being loaded (%s) %s.\n", time.c_str(), loadMode);
		LuaData::LuaDir = std::move(PhobosCRT::WideStringToString(Debug::ApplicationFilePath));
		LuaData::LuaDir += "\\Resources";

		void* buffer {};
		int len = Patch::GetSection(Phobos::hInstance, PATCH_SECTION_NAME, &buffer);

		//msvc add padding between them so dont forget !
		struct _patch : public Patch {
			BYTE _paddings[3];
		};

		_patch* end = (_patch*)((DWORD)buffer + len);

		for (_patch* begin = (_patch*)buffer; begin < end; begin++) {
			begin->Apply();
		}

		Debug::Log("Applying %d Static Patche(s).\n", std::distance((_patch*)buffer, end));

		len = Patch::GetSection(Phobos::hInstance, SYRINGE_HOOKS_SECTION_NAME, &buffer);
		Debug::Log("Applying %d Syringe hook(s).\n", std::distance((hookdecl*)buffer, (hookdecl*)((DWORD)buffer + len)));

		PhobosHookers::Initasmjit();
		Phobos::ExecuteLua();

		char buf[1024] {};

		if (GetEnvironmentVariable("__COMPAT_LAYER", buf, sizeof(buf))) {
			Debug::Log("Compatibility modes detected : %s .\n", buf);
		}
	}
}

constexpr int MAX_MODULE_SECTIONS = 96;

struct ImageSectionRange
{
	LPVOID Base;
	SIZE_T Size;
	DWORD Characteristics;
	char Name[IMAGE_SIZEOF_SHORT_NAME + 1];
};

struct ImageSectionInfo
{
	ImageSectionRange Sections[MAX_MODULE_SECTIONS];
	int SectionCount;
};

class MapViewOfFileClass {
 public:
    explicit MapViewOfFileClass(const wchar_t *fileName):
    File(INVALID_HANDLE_VALUE),
    FileMapping(NULL),
    FileBase(NULL),
    DosHeader(NULL),
    NTHeader(NULL),
    OptionalHeader(NULL),
    SectionHeaders(NULL)
{
    File = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (File != INVALID_HANDLE_VALUE) {
			FileMapping = CreateFileMapping(File, NULL, PAGE_READONLY, 0, 0, NULL);

			if (FileMapping != NULL) {
				FileBase = MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, 0);

				if (FileBase != NULL) {
					DosHeader = (PIMAGE_DOS_HEADER)FileBase;

					if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
						NTHeader = (PIMAGE_NT_HEADERS)((uint8_t *)DosHeader + DosHeader->e_lfanew);

						if (NTHeader->Signature == IMAGE_NT_SIGNATURE) {
							OptionalHeader = (PIMAGE_OPTIONAL_HEADER)&NTHeader->OptionalHeader;
							SectionHeaders = IMAGE_FIRST_SECTION(NTHeader);
						}
					}
				}
			}
		}
	}

    ~MapViewOfFileClass(){
		if (FileBase != NULL)
			UnmapViewOfFile(FileBase);
		if (FileMapping != NULL)
			CloseHandle(FileMapping);
		if (File != INVALID_HANDLE_VALUE)
			CloseHandle(File);
	}

    LPVOID GetMapViewOfFile() const { return FileBase; }
    PIMAGE_DOS_HEADER GetDosHeader() const { return DosHeader; }
    PIMAGE_NT_HEADERS GetNtHeader() const { return NTHeader; }
    PIMAGE_OPTIONAL_HEADER GetOptionalHeader() const { return OptionalHeader; }
    PIMAGE_SECTION_HEADER GetSectionHeaders() const { return SectionHeaders; }
    WORD GetSectionHeaderCount() const { return NTHeader ? NTHeader->FileHeader.NumberOfSections : 0; }

private:
    HANDLE File;
    HANDLE FileMapping;
	LPVOID FileBase;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NTHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER SectionHeaders;
};

bool GetModuleSectionInfo(ImageSectionInfo &info)
{
	info = {};

	HMODULE module = GetModuleHandleW(NULL);
	if (module != NULL)
	{
		auto module_base = reinterpret_cast<uintptr_t>(module);
		auto DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);

		if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE)
		{
			auto NTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + DosHeader->e_lfanew);

			if (NTHeader->Signature == IMAGE_NT_SIGNATURE)
			{
				PIMAGE_SECTION_HEADER section_headers = IMAGE_FIRST_SECTION(NTHeader);

				for (WORD index = 0; index < NTHeader->FileHeader.NumberOfSections; ++index)
				{
					const IMAGE_SECTION_HEADER& section = section_headers[index];
					const DWORD section_size = section.Misc.VirtualSize != 0 ? section.Misc.VirtualSize : section.SizeOfRawData;
					const DWORD section_content_flags = IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_UNINITIALIZED_DATA;
					const DWORD section_memory_flags = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

					if (section_size == 0)
					{
						continue;
					}

					if ((section.Characteristics & section_content_flags) == 0 || (section.Characteristics & section_memory_flags) == 0)
					{
						continue;
					}

					if (info.SectionCount >= MAX_MODULE_SECTIONS)
					{
						return false;
					}

					ImageSectionRange& range = info.Sections[info.SectionCount++];
					range.Base = reinterpret_cast<LPVOID>(module_base + section.VirtualAddress);
					range.Size = SIZE_T(section_size);
					range.Characteristics = section.Characteristics;
					std::memcpy(range.Name, section.Name, IMAGE_SIZEOF_SHORT_NAME);
					range.Name[IMAGE_SIZEOF_SHORT_NAME] = '\0';
				}

				return info.SectionCount > 0;
			}
        }
    }

    return false;
}

struct ProtectedSectionInfo
{
	LPVOID Base;
	SIZE_T Size;
	DWORD OriginalProtect;
	char Name[IMAGE_SIZEOF_SHORT_NAME + 1];
};

static bool startPatching = false;
int ProtectedSectionCount = 0;
ProtectedSectionInfo ProtectedSections[MAX_MODULE_SECTIONS];

static bool RestoreProtectedSections()
{
	bool success = true;

	for (int index = ProtectedSectionCount - 1; index >= 0; --index) {
		DWORD old_protect;
		ProtectedSectionInfo& section = ProtectedSections[index];

		if (VirtualProtect(section.Base, section.Size, section.OriginalProtect, &old_protect) == FALSE)
		{
			success = false;
			break;
		}
	}

	if (success) {
		ProtectedSectionCount = 0;
	}

	return success;
}


bool StartPatching() {
	if(startPatching){
		return true;
	}

	bool success = false;
    ImageSectionInfo info;

	if (GetModuleSectionInfo(info)) {
        success = true;
		ProtectedSectionCount = 0;

		for (int index = 0; index < info.SectionCount; ++index) {
			DWORD original_protect;
			const ImageSectionRange& section = info.Sections[index];

			if (VirtualProtect(section.Base, section.Size, PAGE_EXECUTE_READWRITE, &original_protect) == FALSE) {
				 success = false;
				RestoreProtectedSections();
				break;
        }

			ProtectedSectionInfo& protected_section = ProtectedSections[ProtectedSectionCount++];
			protected_section.Base = section.Base;
			protected_section.Size = section.Size;
			protected_section.OriginalProtect = original_protect;
			std::memcpy(protected_section.Name, section.Name, sizeof(protected_section.Name));
        }
    }

	startPatching = success;

    return success;
}

bool StopPatching()
{
	if (!startPatching) {
		return true;
	}

	bool success = RestoreProtectedSections();

    startPatching = success;

    return success;
}

typedef DWORD(__stdcall* FP_GetVersion)();
static COMPILETIMEEVAL referencefunc<FP_GetVersion, 0x7E1288> const Game_GetVersion {};

DWORD __stdcall GetVersion_Wrapper() {
	auto ver = Game_GetVersion.invoke()();
	CRTHooks::_set_fp_mode();
	//LuaData::ApplyCoreHooks();
	Phobos::ExeRun();
	return ver;
}

bool __fastcall Parse_Command_Line(int argc, char* argv[]) {
	JMP_STD(0x52F620);
}

bool __fastcall Phobos_Parse_Command_Line(int argc, char* argv[]) {

	if (argc > 1) {
		Debug::Log("Parsing command line arguments...\n");
	}

	if (!Parse_Command_Line(argc, argv)) {
		return false;
	}

	Phobos::CmdLineParse(argv, argc);

	Debug::LogDeferredFinalize();

#ifdef MATHTEST
	MathTesters::InspectMathDetailed();
#endif

	return true;
}

std::wstring ExceptionHandler::CommandLines;

void ParseEarlyArgs(LPWSTR* argv , int argc)
{

	if (argv) {
		for (int i = 1; i < argc; i++) {
			ExceptionHandler::CommandLines += L" ";
			ExceptionHandler::CommandLines += argv[i];

			if (IS_SAME_WSTR(argv[i], L"-Icon") && i + 1 < argc) {
				// Convert wide string to narrow string
				char buffer[MAX_PATH];
				WideCharToMultiByte(CP_ACP, 0, argv[i + 1], -1, buffer, MAX_PATH, NULL, NULL);
				Phobos::AppIconPath = buffer;
			} else  if (IS_SAME_WSTR(argv[i], L"-b=" _STR(BUILD_NUMBER))) {
				Phobos::Config::HideWarning = true;
			} else if (IS_SAME_WSTR(argv[i], L"-LOG")) {
				Debug::LogEnabled = true;
				Debug::InitLogger();
			} else  if (IS_SAME_WSTR(argv[i], L"-AI-CONTROL")) {
				Phobos::Otamaa::AllowAIControl = true;
			} else if (IS_SAME_WSTR(argv[i], L"-LOG-CSF")) {
				Phobos::Otamaa::OutputMissingStrings = true;
			} else if (IS_SAME_WSTR(argv[i], L"-LOG-AUDIO")) {
				Phobos::Otamaa::OutputAudioLogs = true;
			} else if (IS_SAME_WSTR(argv[i], L"-STRICT")) {
				Phobos::Otamaa::StrictParser = true;
			} else if (IS_SAME_WSTR(argv[i], L"-NOLOGO")) {
				Phobos::Otamaa::NoLogo = true;
			} else if (IS_SAME_WSTR(argv[i], L"-CD")) {
				Phobos::Otamaa::NoCD = true;
			} else if (IS_SAME_WSTR(argv[i], L"-Inheritance")) {
				Phobos::Config::UseNewInheritance = true;
			} else if (IS_SAME_WSTR(argv[i], L"-Include")) {
				Phobos::Config::UseNewIncludes = true;
			}
		}

		LocalFree(argv);
	}

	if (Debug::LogEnabled) {
		Debug::Log("DLL injection successful, logging enabled via command line.\n");
		Debug::Log("Initialized Phobos " PRODUCT_VERSION ".\n");
		Debug::Log("args %ls\n", ExceptionHandler::CommandLines.c_str());
		CRTHooks::Print_FPUMode();
	}
}

HICON __stdcall LoadIconA_Wrapper(HINSTANCE hInstance, LPCSTR lpIconName)
{
	if (!Phobos::AppIconPath.empty()) {
		Debug::LogInfo("Applying AppIcon from \"{}\"", Phobos::AppIconPath.c_str());
		return (HICON)LoadImageA(hInstance, Phobos::AppIconPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	} else {
		return LoadIconA(hInstance, lpIconName);
	}
}

HCURSOR __stdcall LoadCursorA_Wrapper(HINSTANCE hInstance, LPCSTR lpCursorName)
{
	if (!Phobos::AppCursor.empty()) {
		return LoadCursorFromFileA(Phobos::AppCursor.c_str());
	} else {
		return LoadCursorA(hInstance, lpCursorName);
	}
}

//#include <Misc/ReShade/Runtime/dll_main.h>

BOOL APIENTRY DllMain(HANDLE hInstance, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (IsGamemdExe(nullptr))
		{
			//this is dangerious but this keep shit from breaking early 
			//GlobalColorPacker::SetColorPacker();;
			Patch::CurrentProcess = GetCurrentProcess();
			PhobosThreadGuard::SetMainThread();
			Phobos::hInstance = hInstance;
			saved_lpReserved = lpReserved;
			ExceptionHandler::ReserveExceptionStack();

			int argc;
			LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

            if (!StartPatching()) {
                return FALSE;
            }

			CRTHooks::_set_fp_mode();

			DisableThreadLibraryCalls((HMODULE)hInstance);
			Debug::PrepareLogFile();

			IsInitialized = true;
			PhobosHookers::InitMinHook();
			//if (ReshadeContainer::Attach((HMODULE)hInstance) == FALSE)
			//	return FALSE;

			CRTHooks::Apply();
			ConvertExtData::AllocTLS();

			Patch::Apply_CALL(0x6BC08C, Phobos_Parse_Command_Line);
			Patch::Apply_CALL6(0x7CD835, GetVersion_Wrapper);
			Patch::Apply_TYPED<DWORD>(0x7B853C, { 1 });
			Patch::Apply_TYPED<char>(0x82612C + 13, { '\n' });
			Imports::LoadIconA  = LoadIconA_Wrapper;
			Imports::LoadCursorA = LoadCursorA_Wrapper;
			ParseEarlyArgs(argv, argc);

			ApplyEarlyFuncs();
		}
	}
	break;
	case DLL_PROCESS_DETACH:
	{
		if (!StopPatching()) {
			return FALSE;
		}

		Phobos::hInstance = nullptr;

		bool g_isProcessTerminating = (lpReserved != nullptr);

		if (g_isProcessTerminating && IsInitialized)
		{
			Multithreading::ShutdownMultitheadMode();
			Debug::DeactivateLogger();
			//ReshadeContainer::Detach();
			PhobosHookers::CleanupTrampolines();
		}
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		return FALSE;
	}

	return TRUE;
}
