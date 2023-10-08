#pragma once
#include <YRPPGlobal.h>

#include <Phobos.version.h>
#include <Base/Always.h>
#include <GameStrings.h>

#include <vector>
#include <string>
#include <Wstring.h>

#ifndef NANOPRINTF_IMPLEMENTATION
#define IMPL_SNPRNINTF _snprintf_s
#else
#include <ExtraHeaders/nanoprintf.h>
#define IMPL_SNPRNINTF npf_snprintf
#endif

#define IMPL_STRCMPI(a ,b) _strcmpi(a,b)
#define IMPL_STRCMP(a ,b) strcmp(a,b)
#define IMPL_WSTRCMPI(a , b) _wcsicmp(a , b)

#define IS_SAME_STR_(a ,b) (IMPL_STRCMPI(a,b) == 0)
#define IS_SAME_STR_N(a ,b) (IMPL_STRCMP(a,b) == 0)
#define IS_SAME_WSTR(a,b) (IMPL_WSTRCMPI(a,b) == 0)

class CCINIClass;
class AbstractClass;

#define NONE_STR2 GameStrings::NoneStrb()
constexpr const char* NULL_STR = "null";
constexpr const char* NULL_STR2 = "<null>";
constexpr const char* DEFAULT_STR = "default";
constexpr const char* DEFAULT_STR2 = "<default>";
constexpr const char* PHOBOS_STR = "Phobos";
constexpr const char* ADMIN_STR = "WIN-56N2RLUDAST";

constexpr const char* GLOBALCONTROLS_SECTION = "GlobalControls";
constexpr const char* SIDEBAR_SECTION_T = "Sidebar";

constexpr const wchar_t* ARES_DLL =  L"Ares.dll";
constexpr const char* ARES_DLL_S = "Ares.dll";
constexpr const wchar_t* PHOBOS_DLL = L"Phobos.dll";
constexpr const char* PHOBOS_DLL_S = "Phobos.dll";
constexpr const wchar_t* GAMEMD_EXE = L"gamemd.exe";
constexpr const char* UIMD_ = "uimd.ini";

#define TOOLTIPS_SECTION GameStrings::ToolTips()
#define SIDEBAR_SECTION GameStrings::SideBar()
#define GENERAL_SECTION GameStrings::General()
#define RADIATION_SECTION GameStrings::Radiation()
#define AUDIOVISUAL_SECTION GameStrings::AudioVisual()
#define SPECIALWEAPON_SECTION GameStrings::SpecialWeapons()
#define JUMPJET_SECTION GameStrings::JumpjetControls()
#define COMBATDAMAGE_SECTION GameStrings::CombatDamage()

#define FAILEDTOLOADUIMD_MSG GameStrings::FailedToLoadUIMDMsg()

#define UIMD_FILENAME GameStrings::UIMD_INI()
#define RA2MD_FILENAME GameStrings::RA2MD_INI()

#define RING1_NAME GameStrings::Anim_RING1()
#define INVISO_NAME GameStrings::Anim_INVISO()

//= "<all>";
#define ALL_STR GameStrings::AllStr()
// "<none>";
#define NONE_STR GameStrings::NoneStr()

#define Eva_structureSold GameStrings::EVA_StructureSold()
#define Eva_UnitSold GameStrings::EVA_UnitSold()
#define Eva_OreMinerUnderAttack GameStrings::EVA_OreMinerUnderAttack()


struct Phobos final
{
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	static void ExeRun();
	static void ExeTerminate();
	static void DrawVersionWarning();

	//variables
	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];
	static const char readDefval[4];

	static std::string AppIconPath;
	static char AppName[0x40];
	static bool Debug_DisplayDamageNumbers;
	static const wchar_t* VersionDescription;

	static bool DetachFromDebugger();
	static HRESULT SaveGameDataAfter(IStream* pStm);
	static void LoadGameDataAfter(IStream* pStm);

	static bool EnableConsole;

	struct UI
	{
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;
		static bool ShowHarvesterCounter;
		static double HarvesterCounter_ConditionYellow;
		static double HarvesterCounter_ConditionRed;
		static bool ShowProducingProgress;
		static bool ShowPowerDelta;
		static double PowerDelta_ConditionYellow;
		static double PowerDelta_ConditionRed;
		static bool CenterPauseMenuBackground;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* PowerBlackoutLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* PercentLabel;

		static const wchar_t* BuidingRadarJammedLabel;
	};

	struct Config
	{
		static void Read();

		static bool HideWarning;
		static bool ToolTipDescriptions;
		static bool ToolTipBlur;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;

		static bool EnableBuildingPlacementPreview;
		static bool EnableSelectBrd;

		static bool TogglePowerInsteadOfRepair;
		static bool ShowTechnoNamesIsActive;

		static bool RealTimeTimers;
		static bool RealTimeTimers_Adaptive;
		static int CampaignDefaultGameSpeed;

		static bool DigitalDisplay_Enable;

		static bool ApplyShadeCountFix;

		static bool SaveVariablesOnScenarioEnd;
	};

	struct Misc
	{
		static bool CustomGS;
		static int CustomGS_ChangeInterval[7];
		static int CustomGS_ChangeDelay[7];
		static int CustomGS_DefaultDelay[7];
	};

	struct Otamaa
	{
		static bool DisableCustomRadSite;
		static TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
		static bool IsAdmin;
		static bool ShowHealthPercentEnabled;
		static bool ExeTerminated;
		static bool DoingLoadGame;
	};

	struct Defines
	{
		static inline constexpr ColorStruct ShieldPositiveDamageColor = ColorStruct { 0, 160, 255 };
		static inline constexpr ColorStruct ShieldNegativeDamageColor = ColorStruct { 0, 255, 230 };

	};
};
