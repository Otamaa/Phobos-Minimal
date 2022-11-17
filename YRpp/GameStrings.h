#pragma once

#include <Helpers/CompileTime.h>

#ifndef GAMEMD_STR
#define GAMEMD_STR(name,addr)\
static constexpr constant_ptr<const char,addr> const name;

namespace GameStrings
{
	// unsorted names
	GAMEMD_STR(YURI_S_REVENGE, 0x849F48u);
	GAMEMD_STR(BLOWFISH_DLL  , 0x840A78u);
	GAMEMD_STR(XXICON_SHP    , 0x8204FCu);
	GAMEMD_STR(AllStr, 0x81811Cu);
	GAMEMD_STR(NoneStr, 0x817474u);
	//..
	
	// ini file names
	GAMEMD_STR(UIMD_INI, 0x827DC8u);
	GAMEMD_STR(THEMEMD_INI, 0x825D94u);
	GAMEMD_STR(EVAMD_INI, 0x825DF0u);
	GAMEMD_STR(SOUNDMD_INI, 0x825E50u);
	GAMEMD_STR(BATTLEMD_INI, 0x826198u);
	GAMEMD_STR(AIMD_INI, 0x82621Cu);
	GAMEMD_STR(ARTMD_INI, 0x826254u);
	GAMEMD_STR(RULESMD_INI, 0x826260u);
	GAMEMD_STR(RA2MD_INI, 0x826444u);
	GAMEMD_STR(MAPSELMD_INI, 0x830370u);
	//..
	
	// ini section names
	GAMEMD_STR(General, 0x826278u);
	GAMEMD_STR(Basic       , 0x82BF9Cu);
	GAMEMD_STR(AudioVisual, 0x839EA8u);
	GAMEMD_STR(CombatDamage, 0x839E8Cu);
	GAMEMD_STR(Radiation, 0x839E80u);
	GAMEMD_STR(ToolTips    , 0x833188u);
	GAMEMD_STR(SideBar, 0x848AD4u);
	GAMEMD_STR(SpecialWeapons, 0x839EB4u);
	GAMEMD_STR(JumpjetControls, 0x839D58u);
	//..
	
	// EVA entry names
	GAMEMD_STR(EVA_StructureSold      , 0x819030u);
	GAMEMD_STR(EVA_UnitSold, 0x822630u);
	GAMEMD_STR(EVA_OreMinerUnderAttack, 0x824784u);
	//...

	// Messages
	GAMEMD_STR(FailedToLoadUIMDMsg, 0x827DACu);
	//..

	// Hardcoded Names
	GAMEMD_STR(Anim_RING1, 0x8182F0u);
	GAMEMD_STR(Anim_INVISO, 0x8182F8u);
	//..
}

#undef GAMEMD_STR
#endif