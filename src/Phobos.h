#pragma once
#include <YRPPGlobal.h>

#include <Phobos.version.h>
#include <Base/Always.h>
#include <GameStrings.h>

#include <vector>
#include <string>
#include <Wstring.h>

#include <format>
#include <random>

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
constexpr auto UISETTINGS_SECTION = "UISettings";

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
	class Random
	{
	public:
		static void SetRandomSeed(int seed)
		{
			_engine.seed(seed);
		}

		static int RandomRanged(int min, int max)
		{
			std::uniform_int_distribution<int> dis(min, max);
			return dis(_engine);
		}

		static double RandomDouble()
		{
			return RandomRanged(1, INT_MAX) / (double)((unsigned int)INT_MAX + 1);
		}
	private:
		inline static std::minstd_rand _engine {};
	};

	static void CmdLineParse(char**, int);

	static void ExeRun();
	static void ExeTerminate();
	static void DrawVersionWarning();
	static void ExecuteLua();
	static void CheckProcessorFeatures();
	static void InitAdminDebugMode();
	static void ThrowUsageWarning(CCINIClass* pINI);
	static void InitConsole();
	//variables
	static HANDLE hInstance;

	static constexpr size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];
	static const char readDefval[4];

	static std::string AppIconPath;
	static bool Debug_DisplayDamageNumbers;
	static const wchar_t* VersionDescription;

	static bool DetachFromDebugger();
	static HRESULT SaveGameDataAfter(IStream* pStm);
	static void LoadGameDataAfter(IStream* pStm);

	static bool EnableConsole;

	static bool ShouldQuickSave;
	static std::wstring CustomGameSaveDescription;
	static void PassiveSaveGame();

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
		static bool WeedsCounter_Show;
		static bool UnlimitedColor;
		static bool AnchoredToolTips;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* PowerBlackoutLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* PercentLabel;

		static const wchar_t* BuidingRadarJammedLabel;
		static const wchar_t* BuidingFakeLabel;
		static const wchar_t* ShowBriefingResumeButtonLabel;
		static char ShowBriefingResumeButtonStatusLabel[0x20];

		static const wchar_t* Power_Label;
		static const wchar_t* Drain_Label;
		static const wchar_t* Storage_Label;
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

		static bool MultiThreadSinglePlayer;
		static bool UseImprovedPathfindingBlockageHandling;
		static bool HideLightFlashEffects;

		static bool DebugFatalerrorGenerateDump;
		static bool SaveGameOnScenarioStart;

		static bool ShowPowerDelta;
		static bool ShowHarvesterCounter;
		static bool ShowWeedsCounter;

		static bool UseNewInheritance;
		static bool UseNewIncludes;
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
		static bool IsAdmin;
		static bool ShowHealthPercentEnabled;
		static bool ExeTerminated;
		static bool DoingLoadGame;
		static bool AllowAIControl;
		static bool OutputMissingStrings;
		static bool OutputAudioLogs;
		static bool StrictParser;
		static bool ParserErrorDetected;
		static bool TrackParserErrors;
		static bool NoLogo;
		static bool NoCD;
		static bool CompatibilityMode;
	};

	struct Defines
	{
		static inline constexpr ColorStruct ShieldPositiveDamageColor = ColorStruct { 0, 160, 255 };
		static inline constexpr ColorStruct ShieldNegativeDamageColor = ColorStruct { 0, 255, 230 };
		static inline constexpr ColorStruct InterceptedPositiveDamageColor = ColorStruct { 255, 128, 128 };
		static inline constexpr ColorStruct InterceptedNegativeDamageColor = ColorStruct { 128, 255, 128 };
	};

	static FORCEINLINE unsigned Round_Up(unsigned number, int a)
	{
		return (number + (a - 1)) & (~(a - 1));
	}

	static void* __cdecl _allocate(unsigned int size)
	{
		return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Round_Up(size, 4));
	}

	static unsigned int __cdecl _msize(void* ptr)
	{
		return HeapSize(GetProcessHeap(), 0, ptr);
	}

	static char* __cdecl _strdup(const char* string)
	{
		char* str;
		char* p;
		int len = 0;

		while (string[len])
		{
			len++;
		}
		str = (char*)_allocate(len + 1);
		p = str;
		while (*string)
		{
			*p++ = *string++;
		}
		*p = '\0';
		return str;
	}

	static void* __cdecl _count_allocate(unsigned int count, unsigned int size)
	{
		return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Round_Up(size, 4) * count);
	}

	static void* __cdecl _reallocate(void* ptr, unsigned int size)
	{
		return HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, Round_Up(size, 4));
	}

	static void __cdecl _free(void* ptr)
	{
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr);
	}

	static void __cdecl _dump_memory_leaks()
	{
		//need _DEBUG
		_CrtDumpMemoryLeaks();
	}
};
