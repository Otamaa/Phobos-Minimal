#pragma once
#include <YRPPGlobal.h>

#include <Base/Always.h>
#include <GameStrings.h>

#include <vector>
#include <string>
#include <Wstring.h>

#include <format>

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
	//variables
	static HANDLE hInstance;

};
