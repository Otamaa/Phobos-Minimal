#pragma once
#include <YRPPGlobal.h>

#include <Phobos.version.h>
#include <Base/Always.h>
#include <GameStrings.h>

#include <vector>
#include <string>
#include <Wstring.h>

#define IS_SAME_STR_(a ,b) CRT::strcmpi(a,b) == 0

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
private:
	NO_CONSTRUCT_CLASS(Phobos)
public:
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	static void ExeRun();
	static void ExeTerminate();

	//variables
	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];
	static const char readDefval[4];

	static uintptr_t AresBaseAddress;

	static std::string AppIconPath;
	static char AppName[0x40];
	static bool Debug_DisplayDamageNumbers;
	static const wchar_t* VersionDescription;

	static bool DetachFromDebugger();
	static HRESULT SaveGameDataAfter(IStream* pStm);
	static void LoadGameDataAfter(IStream* pStm);

	static bool EnableConsole;

	class UI
	{
		NO_CONSTRUCT_CLASS(UI)
	public:
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

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* PowerBlackoutLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* PercentLabel;
	};

	class Config
	{
		NO_CONSTRUCT_CLASS(Config)
	public:
		static bool HideWarning;
		static bool ToolTipDescriptions;
		static bool ToolTipBlur;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;
		static bool AllowParallelAIQueues;
		static bool EnableBuildingPlacementPreview;
		static bool EnableSelectBrd;

		static bool ForbidParallelAIQueues_Infantry;
		static bool ForbidParallelAIQueues_Vehicle;
		static bool ForbidParallelAIQueues_Navy;
		static bool ForbidParallelAIQueues_Aircraft;
		static bool ForbidParallelAIQueues_Building;

		static bool TogglePowerInsteadOfRepair;
		static bool ShowTechnoNamesIsActive;
	};

	class Otamaa
	{
		NO_CONSTRUCT_CLASS(Otamaa)
	public:
		static bool DisableCustomRadSite;
		static TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
		static bool IsAdmin;
		static bool ShowHealthPercentEnabled;
		static bool ExeTerminated;
	};

	class Defines
	{
		NO_CONSTRUCT_CLASS(Defines)
	public:

		static inline constexpr ColorStruct ShieldPositiveDamageColor = ColorStruct { 0, 160, 255 };
		static inline constexpr ColorStruct ShieldNegativeDamageColor = ColorStruct { 0, 255, 230 };

	};
};
