#pragma once
#include <YRPPGlobal.h>

#include <Phobos.version.h>
#include <Base/Always.h>

#include <vector>
#include <string>
#include <Wstring.h>

#define IS_SAME_STR_(a ,b) _strcmpi(a,b) == 0

class CCINIClass;
class AbstractClass;

constexpr const char* NONE_STR2 = "none";
constexpr const char* NULL_STR = "null";
constexpr const char* NULL_STR2 = "<null>";
constexpr const char* DEFAULT_STR = "default";
constexpr const char* DEFAULT_STR2 = "<default>";
constexpr const char* PHOBOS_STR = "Phobos";
constexpr const char* ADMIN_STR = "Otamaa";

constexpr const char* GLOBALCONTROLS_SECTION = "GlobalControls";
constexpr const char* SIDEBAR_SECTION_T = "Sidebar";

constexpr const wchar_t* ARES_DLL =  L"Ares.dll";
constexpr const char* ARES_DLL_S = "Ares.dll";
constexpr const wchar_t* GAMEMD_EXE = L"gamemd.exe";
constexpr const char* UIMD_ = "uimd.ini";

#define TOOLTIPS_SECTION reinterpret_cast<const char*>(0x833188)
#define SIDEBAR_SECTION reinterpret_cast<const char*>(0x848AD4)
#define GENERAL_SECTION reinterpret_cast<const char*>(0x826278)
#define RADIATION_SECTION reinterpret_cast<const char*>(0x839E80)
#define AUDIOVISUAL_SECTION reinterpret_cast<const char*>(0x839EA8)
#define SPECIALWEAPON_SECTION reinterpret_cast<const char*>(0x839EB4)
#define JUMPJET_SECTION reinterpret_cast<const char*>(0x839D58)

#define FAILEDTOLOADUIMD_MSG reinterpret_cast<const char*>(0x827DAC)

#define UIMD_FILENAME reinterpret_cast<const char*>(0x827DC8)
#define RA2MD_FILENAME reinterpret_cast<const char*>(0x826444)

#define RING1_NAME reinterpret_cast<const char*>(0x8182F0)
#define INVISO_NAME reinterpret_cast<const char*>(0x8182F8)

//= "<all>";
#define ALL_STR reinterpret_cast<const char*>(0x81811C)
// "<none>";
#define NONE_STR reinterpret_cast<const char*>(0x817474)

struct Phobos final
{
private:
	NO_CONSTRUCT_CLASS(Phobos)
public:
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	//variables
	static HMODULE hInstance;

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
	static void Clear();
	static void PointerGotInvalid(AbstractClass* const pInvalid, bool const removed);
	static HRESULT SaveGameData(IStream* pStm);
	static void LoadGameData(IStream* pStm);

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
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* PercentLabel;
	};

	class Config
	{
		NO_CONSTRUCT_CLASS(Config)
	public:
		static bool ToolTipDescriptions;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;
		static bool AllowParallelAIQueues;
		static bool EnableBuildingPlacementPreview;
		static bool EnableSelectBrd;
	};

	class Otamaa
	{
		NO_CONSTRUCT_CLASS(Otamaa)
	public:
		static bool DisableCustomRadSite;
		static TCHAR PCName[MAX_COMPUTERNAME_LENGTH + 1];
		static bool IsAdmin;
		static bool ShowHealthPercentEnabled;
	};
};
