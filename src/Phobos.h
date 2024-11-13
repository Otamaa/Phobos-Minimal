#pragma once

#include <Phobos.version.h>

#include <GameStrings.h>
#include <Utilities/Patch.h>

#include <Wstring.h>
#include <format>
#include <random>
#include <ColorStruct.h>

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
#define IS_SAME_STR_I(a,b) ( _stricmp(a,b) == 0)
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

enum class ExceptionHandlerMode {
	Default = 0,
	Full = 1,
	NoRemove = 2
};

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

	static bool DetachFromDebugger();
	static HRESULT SaveGameDataAfter(IStream* pStm);
	static void LoadGameDataAfter(IStream* pStm);
	static void PassiveSaveGame();

	//variables
	static inline HANDLE hInstance;
	static inline  constexpr size_t readLength = 2048;
	static inline  char readBuffer[readLength];
	static inline  wchar_t wideBuffer[readLength];
	static inline  const char readDelims[4] { "," };
	static inline  const char readDefval[4] { "" };
	static inline std::string AppIconPath;
	static inline bool Debug_DisplayDamageNumbers;
	static inline const wchar_t* VersionDescription { L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping." };
	static inline bool EnableConsole;
	static inline bool ShouldQuickSave;
	static inline std::wstring CustomGameSaveDescription;
	static inline PVOID pExceptionHandler;
	static inline ExceptionHandlerMode ExceptionMode;

	static inline bool HasCNCnet;
	static inline std::vector<Patch> Patches;

	struct UI
	{
		static inline bool DisableEmptySpawnPositions;
		static inline bool ExtendedToolTips;
		static inline int MaxToolTipWidth;
		static inline bool ShowHarvesterCounter;
		static inline double HarvesterCounter_ConditionYellow { 0.99 };
		static inline double HarvesterCounter_ConditionRed { 0.5 };
		static inline bool ShowProducingProgress;
		static inline bool ShowPowerDelta;
		static inline double PowerDelta_ConditionYellow { 0.75 };
		static inline double PowerDelta_ConditionRed { 1.0 };
		static inline bool CenterPauseMenuBackground;
		static inline bool WeedsCounter_Show;
		static inline bool UnlimitedColor;
		static inline bool AnchoredToolTips;

		static inline bool ExclusiveSWSidebar;
		static inline int ExclusiveSWSidebar_Interval;
		static inline int ExclusiveSWSidebar_Max;
		static inline int ExclusiveSWSidebar_MaxColumn { INT32_MAX };

		static inline const wchar_t* CostLabel;
		static inline const wchar_t* PowerLabel;
		static inline const wchar_t* PowerBlackoutLabel;
		static inline const wchar_t* TimeLabel;
		static inline const wchar_t* HarvesterLabel;
		static inline const wchar_t* PercentLabel;

		static inline const wchar_t* BuidingRadarJammedLabel;
		static inline const wchar_t* BuidingFakeLabel;
		static inline const wchar_t* ShowBriefingResumeButtonLabel { L"" };
		static inline char ShowBriefingResumeButtonStatusLabel[0x20];

		static inline const wchar_t* Power_Label;
		static inline const wchar_t* Drain_Label;
		static inline const wchar_t* Storage_Label;
		static inline const wchar_t* Radar_Label;
		static inline const wchar_t* Spysat_Label;

		static inline const wchar_t* SWShotsFormat { L"" };

	};

	struct Config
	{
		static void Read();

		static inline bool HideWarning;
		static inline bool ToolTipDescriptions { true };
		static inline bool ToolTipBlur;
		static inline bool PrioritySelectionFiltering { true };
		static inline bool DevelopmentCommands { true };
		static inline bool ArtImageSwap;

		static inline bool EnableBuildingPlacementPreview;
		static inline bool EnableSelectBrd;

		static inline bool TogglePowerInsteadOfRepair;
		static inline bool ShowTechnoNamesIsActive;

		static inline bool RealTimeTimers;
		static inline bool RealTimeTimers_Adaptive;
		static inline int CampaignDefaultGameSpeed { 2 };

		static inline bool DigitalDisplay_Enable;

		static inline bool ApplyShadeCountFi { true };

		static inline bool SaveVariablesOnScenarioEnd;

		static inline bool MultiThreadSinglePlayer;
		static inline bool UseImprovedPathfindingBlockageHandling;
		static inline bool HideLightFlashEffects;

		static inline bool DebugFatalerrorGenerateDump;
		static inline bool SaveGameOnScenarioStart { true };

		static inline bool ShowPowerDelta { true };
		static inline bool ShowHarvesterCounter;
		static inline bool ShowWeedsCounter;

		static inline bool UseNewInheritance;
		static inline bool UseNewIncludes;
		static inline bool ApplyShadeCountFix { true };
	};

	struct Misc
	{
		static inline bool CustomGS;
		static inline int CustomGS_ChangeInterval[7] { -1, -1, -1, -1, -1, -1, -1 };
		static inline int CustomGS_ChangeDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
		static inline int CustomGS_DefaultDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
	};

	struct Otamaa
	{
		static inline bool DisableCustomRadSite;
		static inline bool IsAdmin;
		static inline bool ShowHealthPercentEnabled;
		static inline bool ExeTerminated;
		static inline bool DoingLoadGame;
		static inline bool AllowAIControl;
		static inline bool OutputMissingStrings;
		static inline bool OutputAudioLogs;
		static inline bool StrictParser;
		static inline bool ParserErrorDetected;
		static inline bool TrackParserErrors;
		static inline bool NoLogo;
		static inline bool NoCD;
		static inline bool CompatibilityMode;
		static inline bool ReplaceGameMemoryAllocator;
		static inline bool AllowMultipleInstance;

		static inline DWORD PhobosBaseAddress;
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
