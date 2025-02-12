#pragma once

#include <Phobos.version.h>

#include <GameStrings.h>
#include <Utilities/Patch.h>

#include <Wstring.h>
#include <format>
#include <random>
#include <ColorStruct.h>

class CCINIClass;
class AbstractClass;

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
		OPTIONALINLINE static std::minstd_rand _engine {};
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
	static OPTIONALINLINE HANDLE hInstance;
	static OPTIONALINLINE COMPILETIMEEVAL size_t readLength = 2048;
	static OPTIONALINLINE char readBuffer[readLength] {};
	static OPTIONALINLINE wchar_t wideBuffer[readLength]  {};
	static OPTIONALINLINE const char readDelims[4] { "," };
	static OPTIONALINLINE const char readDefval[4] { "" };
	static OPTIONALINLINE std::string AppIconPath {};
	static OPTIONALINLINE bool Debug_DisplayDamageNumbers { false };
	static OPTIONALINLINE const wchar_t* VersionDescription { L"Phobos Otamaa Unofficial development build #" _STR(BUILD_NUMBER) L". Please test before shipping." };
	static OPTIONALINLINE bool EnableConsole { false };
	static OPTIONALINLINE bool ShouldQuickSave { false };
	static OPTIONALINLINE std::wstring CustomGameSaveDescription {};
	static OPTIONALINLINE PVOID pExceptionHandler { nullptr };
	static OPTIONALINLINE ExceptionHandlerMode ExceptionMode { ExceptionHandlerMode::Default };

	static OPTIONALINLINE bool HasCNCnet { false };
	static OPTIONALINLINE std::vector<Patch> Patches {};

	struct UI
	{
		static OPTIONALINLINE bool DisableEmptySpawnPositions { false };
		static OPTIONALINLINE bool ExtendedToolTips { false };
		static OPTIONALINLINE int MaxToolTipWidth { 0 };
		static OPTIONALINLINE bool ShowHarvesterCounter { false };
		static OPTIONALINLINE double HarvesterCounter_ConditionYellow { 0.99 };
		static OPTIONALINLINE double HarvesterCounter_ConditionRed { 0.5 };
		static OPTIONALINLINE bool ShowProducingProgress { false };
		static OPTIONALINLINE bool ShowPowerDelta { false };
		static OPTIONALINLINE double PowerDelta_ConditionYellow { 0.75 };
		static OPTIONALINLINE double PowerDelta_ConditionRed { 1.0 };
		static OPTIONALINLINE bool CenterPauseMenuBackground { false };
		static OPTIONALINLINE bool WeedsCounter_Show { false };
		static OPTIONALINLINE bool UnlimitedColor { false };
		static OPTIONALINLINE bool AnchoredToolTips { false };

		static OPTIONALINLINE bool SuperWeaponSidebar { false };
		static OPTIONALINLINE int SuperWeaponSidebar_Interval { 0 };
		static OPTIONALINLINE int SuperWeaponSidebar_LeftOffset { 0 };
		static OPTIONALINLINE int SuperWeaponSidebar_CameoHeight { 48 };
		static OPTIONALINLINE int SuperWeaponSidebar_Max { 0 };
		static OPTIONALINLINE int SuperWeaponSidebar_MaxColumns { INT32_MAX};

		static OPTIONALINLINE const wchar_t* CostLabel { nullptr };
		static OPTIONALINLINE const wchar_t* PowerLabel { nullptr };
		static OPTIONALINLINE const wchar_t* PowerBlackoutLabel { nullptr };
		static OPTIONALINLINE const wchar_t* TimeLabel { nullptr };
		static OPTIONALINLINE const wchar_t* HarvesterLabel { nullptr };
		static OPTIONALINLINE const wchar_t* PercentLabel { nullptr };

		static OPTIONALINLINE const wchar_t* BuidingRadarJammedLabel{ nullptr };
		static OPTIONALINLINE const wchar_t* BuidingFakeLabel{ nullptr };
		static OPTIONALINLINE const wchar_t* ShowBriefingResumeButtonLabel { L"" };
		static OPTIONALINLINE char ShowBriefingResumeButtonStatusLabel[0x20];

		static OPTIONALINLINE const wchar_t* Power_Label { nullptr };
		static OPTIONALINLINE const wchar_t* Drain_Label { nullptr };
		static OPTIONALINLINE const wchar_t* Storage_Label { nullptr };
		static OPTIONALINLINE const wchar_t* Radar_Label { nullptr };
		static OPTIONALINLINE const wchar_t* Spysat_Label { nullptr };

		static OPTIONALINLINE const wchar_t* SWShotsFormat { L"" };

	};

	struct Config
	{
		static void Read();

		static OPTIONALINLINE bool HideWarning { false };
		static OPTIONALINLINE bool ToolTipDescriptions { true };
		static OPTIONALINLINE bool ToolTipBlur { false };
		static OPTIONALINLINE bool PrioritySelectionFiltering { true };
		static OPTIONALINLINE bool DevelopmentCommands { true };
		static OPTIONALINLINE bool ArtImageSwap { false };

		static OPTIONALINLINE bool EnableBuildingPlacementPreview { false };
		static OPTIONALINLINE bool EnableSelectBrd { false };

		static OPTIONALINLINE bool TogglePowerInsteadOfRepair { false };
		static OPTIONALINLINE bool ShowTechnoNamesIsActive { false };

		static OPTIONALINLINE bool RealTimeTimers { false };
		static OPTIONALINLINE bool RealTimeTimers_Adaptive { false };
		static OPTIONALINLINE int CampaignDefaultGameSpeed { 2 };

		static OPTIONALINLINE bool DigitalDisplay_Enable { false };
		static OPTIONALINLINE bool ShowBuildingStatistics { false };

		static OPTIONALINLINE bool ApplyShadeCountFi { true };

		static OPTIONALINLINE bool SaveVariablesOnScenarioEnd { false };

		static OPTIONALINLINE bool MultiThreadSinglePlayer { false };
		static OPTIONALINLINE bool UseImprovedPathfindingBlockageHandling { false };
		static OPTIONALINLINE bool HideLightFlashEffects { false };

		static OPTIONALINLINE bool DebugFatalerrorGenerateDump { false };
		static OPTIONALINLINE bool SaveGameOnScenarioStart { true };

		static OPTIONALINLINE bool ShowPowerDelta { true };
		static OPTIONALINLINE bool ShowHarvesterCounter { false };
		static OPTIONALINLINE bool ShowWeedsCounter { false };

		static OPTIONALINLINE bool UseNewInheritance { false };
		static OPTIONALINLINE bool UseNewIncludes { false };
		static OPTIONALINLINE bool ApplyShadeCountFix { true };
		static OPTIONALINLINE bool ShowFlashOnSelecting { true };
		
		static OPTIONALINLINE bool AutoBuilding_Enable { false };

		static bool ScrollSidebarStripInTactical { true };
		static bool ScrollSidebarStripWhenHoldKey { true };
	};

	struct Misc
	{
		static OPTIONALINLINE bool CustomGS { false };
		static OPTIONALINLINE int CustomGS_ChangeInterval[7] { -1, -1, -1, -1, -1, -1, -1 };
		static OPTIONALINLINE int CustomGS_ChangeDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
		static OPTIONALINLINE int CustomGS_DefaultDelay[7] { 0, 1, 2, 3, 4, 5, 6 };
	};

	struct Otamaa
	{
		static OPTIONALINLINE bool DisableCustomRadSite { false };
		static OPTIONALINLINE bool IsAdmin { false };
		static OPTIONALINLINE bool ShowHealthPercentEnabled { false };
		static OPTIONALINLINE bool ExeTerminated { false };
		static OPTIONALINLINE bool DoingLoadGame { false };
		static OPTIONALINLINE bool AllowAIControl { false };
		static OPTIONALINLINE bool OutputMissingStrings { false };
		static OPTIONALINLINE bool OutputAudioLogs { false };
		static OPTIONALINLINE bool StrictParser { false };
		static OPTIONALINLINE bool ParserErrorDetected { false };
		static OPTIONALINLINE bool TrackParserErrors { false };
		static OPTIONALINLINE bool NoLogo { false };
		static OPTIONALINLINE bool NoCD { false };
		static OPTIONALINLINE bool CompatibilityMode { false };
		static OPTIONALINLINE bool ReplaceGameMemoryAllocator { false };
		static OPTIONALINLINE bool AllowMultipleInstance { false };

		static OPTIONALINLINE DWORD PhobosBaseAddress { false };
	};

	struct Defines
	{
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct ShieldPositiveDamageColor = ColorStruct { 0, 160, 255 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct ShieldNegativeDamageColor = ColorStruct { 0, 255, 230 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct InterceptedPositiveDamageColor = ColorStruct { 255, 128, 128 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct InterceptedNegativeDamageColor = ColorStruct { 128, 255, 128 };
	};

	static FORCEDINLINE unsigned Round_Up(unsigned number, int a)
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
