#pragma once

#include <Phobos.version.h>

#include <GameStrings.h>
#include <Utilities/Patch.h>

#include <Wstring.h>
#include <random>
#include <ColorStruct.h>
#include <MessageBoxLogging.h>

#include <expected>
#include <filesystem>
#include <mutex>
#include <unordered_map>

#pragma warning( push )
#pragma warning (disable : 4244)

#include <Lib/asmjit/x86.h>
#include <Lib/fmt/core.h>
#include <Lib/fmt/xchar.h>
#include <Lib/fmt/printf.h>

#include <Lib/magic_enum/magic_enum_all.hpp>
#include <Lib/pdqsort/pdqsort.h>

#pragma warning( pop )


enum class DrawDamageMode : BYTE
{
	disabled, damageOnly, withWH, count
};

enum class FPSCounterMode
{
	disabled, Full, FPSOnly, FPSandAVG, count
};

class CCINIClass;
class AbstractClass;

enum class ExceptionHandlerMode {
	Default = 0,
	Full = 1,
	NoRemove = 2
};

class PhobosStreamWriter;
class PhobosStreamReader;
struct Phobos final : public IPersistStream
{
public:

	static unsigned GetVersionNumber();

	class Random
	{
	public:

		static void SetRandomSeed(int seed) {
			_engine.seed(seed);
		}

		template<typename T>
		static T RandomRanged(T min, T max) {
			std::uniform_int_distribution<T> dis(min, max);
			return dis(_engine);
		}

		static double RandomDouble() {
			return RandomRanged(1, INT_MAX) / (double)((unsigned int)INT_MAX + 1);
		}

	private:
		static std::mt19937 _engine;
	};

	static void CmdLineParse(char**, int);

	static void ExeRun();
	static void ExeTerminate();
	static void DrawVersionWarning();
	static void ExecuteLua();
	static void CheckProcessorFeatures();
	static void InitAdminDebugMode();
	static void ThrowUsageWarning(CCINIClass* pINI);

	static bool DetachFromDebugger();
	static HRESULT SaveGameDataAfter(IStream* pStm);
	static HRESULT LoadGameDataAfter(IStream* pStm);
	static void PassiveSaveGame();

	//variables
	static HANDLE hInstance;
	static OPTIONALINLINE COMPILETIMEEVAL size_t readLength { 2048 }; // this variable can be used anywhere , keep the inline
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength] ;
	static const char readDelims[4];
	static const char readDefval[4];
	static std::string AppIconPath;
	static DrawDamageMode Debug_DisplayDamageNumbers;
	static const wchar_t* VersionDescription;
	static bool ShouldQuickSave;
	static std::wstring CustomGameSaveDescription;
	static PVOID pExceptionHandler;
	static ExceptionHandlerMode ExceptionMode;

	static bool HasCNCnet;

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

		static bool SuperWeaponSidebar;
		static int SuperWeaponSidebar_Interval;
		static int SuperWeaponSidebar_LeftOffset;
		static int SuperWeaponSidebar_CameoHeight;
		static int SuperWeaponSidebar_Max;
		static int SuperWeaponSidebar_MaxColumns;
		static bool SuperWeaponSidebar_Pyramid;

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
		static const wchar_t* Radar_Label;
		static const wchar_t* Spysat_Label;
		static const wchar_t* Tech_Label;

		static const wchar_t* SWShotsFormat;

		static const wchar_t* BattlePoints_Label;
		static const wchar_t* BattlePointsSidebar_Label;
		static bool BattlePointsSidebar_Label_InvertPosition;
		static bool BattlePointsSidebar_AlwaysShow;
	};

	struct Config
	{
		static void Read_RA2MD();
		static void Read_UIMD();
		static void Read_RULESMD();
		
		static bool HideWarning;
		static bool ToolTipDescriptions;
		static bool ToolTipBlur;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;

		static bool EnableBuildingPlacementPreview;
		static bool EnableSelectBox;

		static bool TogglePowerInsteadOfRepair;
		static bool ShowTechnoNamesIsActive;

		static bool RealTimeTimers;
		static bool RealTimeTimers_Adaptive;
		static int CampaignDefaultGameSpeed;

		static bool DigitalDisplay_Enable;
		static bool MessageDisplayInCenter;
		static bool MessageApplyHoverState;
		static int MessageDisplayInCenter_BoardOpacity;
		static int MessageDisplayInCenter_LabelsCount;
		static int MessageDisplayInCenter_RecordsCount;

		static bool ShowBuildingStatistics;

		static bool ApplyShadeCountFi;

		static bool SaveVariablesOnScenarioEnd;

		static bool MultiThreadSinglePlayer;
		static bool UseImprovedPathfindingBlockageHandling;
		static bool HideLightFlashEffects;
		static bool HideLaserTrailEffects;
		static bool HideShakeEffects;

		static bool DebugFatalerrorGenerateDump;
		static bool SaveGameOnScenarioStart;

		static bool ShowPowerDelta;
		static bool ShowHarvesterCounter;
		static bool ShowWeedsCounter;

		static bool UseNewInheritance;
		static bool UseNewIncludes;
		static bool ApplyShadeCountFix;
		static bool ShowFlashOnSelecting;

		static bool AutoBuilding_Enable;

		static bool ScrollSidebarStripInTactical;
		static bool ScrollSidebarStripWhenHoldKey;

		static bool UnitPowerDrain;

		static bool AllowSwitchNoMoveCommand;
		static bool AllowDistributionCommand;
		static bool AllowDistributionCommand_SpreadMode;
		static bool AllowDistributionCommand_SpreadModeScroll;
		static bool AllowDistributionCommand_FilterMode;
		static bool AllowDistributionCommand_AffectsAllies;
		static bool AllowDistributionCommand_AffectsEnemies;
		static bool ApplyNoMoveCommand;
		static int DistributionSpreadMode;
		static int DistributionFilterMode;
		static int SuperWeaponSidebar_RequiredSignificance;

		static bool SuperWeaponSidebarCommands;
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
		static bool AllowAIContro;
		static bool OutputMissingStrings;
		static bool OutputAudioLogs;
		static bool StrictParser;
		static bool ParserErrorDetected;
		static bool TrackParserErrors;
		static bool NoLogo;
		static bool NoCD;
		static bool CompatibilityMode;
		static bool ReplaceGameMemoryAllocator;
		static bool AllowMultipleInstance;
		static bool AllowAIControl;
		static DWORD PhobosBaseAddress;
	};

	struct Defines
	{
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct ShieldPositiveDamageColor = ColorStruct { 0, 160, 255 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct ShieldNegativeDamageColor = ColorStruct { 0, 255, 230 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct InterceptedPositiveDamageColor = ColorStruct { 255, 128, 128 };
		static OPTIONALINLINE COMPILETIMEEVAL ColorStruct InterceptedNegativeDamageColor = ColorStruct { 128, 255, 128 };
	};

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);
};
