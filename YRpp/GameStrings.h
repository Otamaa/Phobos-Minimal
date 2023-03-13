#pragma once

#include <Helpers/CompileTime.h>

#ifndef GAMEMD_STR
#define GAMEMD_STR(name,addr)\
static constexpr constant_ptr<const char,addr> const name {}

struct GameStrings
{
	// unsorted names
	GAMEMD_STR(YURI_S_REVENGE, 0x849F48u);
	GAMEMD_STR(BLOWFISH_DLL, 0x840A78u);
	GAMEMD_STR(XXICON_SHP, 0x8204FCu);
	GAMEMD_STR(LSSOBS_SHP, 0x8297F4u);
	GAMEMD_STR(_800, 0x8297DCu);
	GAMEMD_STR(_640, 0x8297E0u);
	GAMEMD_STR(AllStr, 0x81811Cu);
	// <none>
	GAMEMD_STR(NoneStr, 0x817474u);
	// none
	GAMEMD_STR(NoneStrb, 0x817694u);
	GAMEMD_STR(RandomStr, 0x81C008u);
	GAMEMD_STR(NAMStr, 0x841F43u);
	GAMEMD_STR(OVERLAY, 0x833450u);
	GAMEMD_STR(Image, 0x819420u);
	GAMEMD_STR(Suicide, 0x843050u);
	//..

	// mix file names
	GAMEMD_STR(LOCALMD_MIX, 0x826644u);
	GAMEMD_STR(RA2MD_MIX, 0x82667Cu);
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
	GAMEMD_STR(Basic, 0x82BF9Cu);
	GAMEMD_STR(AudioVisual, 0x839EA8u);
	GAMEMD_STR(CrateRules, 0x839E9Cu);
	GAMEMD_STR(CombatDamage, 0x839E8Cu);
	GAMEMD_STR(Radiation, 0x839E80u);
	GAMEMD_STR(ToolTips, 0x833188u);
	GAMEMD_STR(SideBar, 0x848AD4u);
	GAMEMD_STR(SpecialWeapons, 0x839EB4u);
	GAMEMD_STR(JumpjetControls, 0x839D58u);
	GAMEMD_STR(OverlayPack, 0x833484u);
	GAMEMD_STR(OverlayDataPack, 0x833474u);
	GAMEMD_STR(Colors, 0x839E5Cu);
	GAMEMD_STR(Waypoints, 0x82DB0Cu);
	GAMEMD_STR(VariableNames, 0x83D824u);

	GAMEMD_STR(ShakeYhi, 0x847C84);
	GAMEMD_STR(ShakeYlo, 0x847C90);
	GAMEMD_STR(ShakeXhi, 0x847C9C);
	GAMEMD_STR(ShakeXlo, 0x847CA8);

	GAMEMD_STR(SpySat, 0x81AE58);

	GAMEMD_STR(SplashList , 0x83B1FC);

	GAMEMD_STR(Sound, 0x844748);

	GAMEMD_STR(DestroyAnim, 0x843FF0);

	GAMEMD_STR(LightBlueTint , 0x81A8EC);
	GAMEMD_STR(LightGreenTint, 0x81A8FC);
	GAMEMD_STR(LightRedTint, 0x81A90C);
	GAMEMD_STR(LightIntensity, 0x81A91C);
	GAMEMD_STR(LightVisibility, 0x81A92C);

	GAMEMD_STR(WarpAway, 0x83CDB8);
	GAMEMD_STR(WarpOut, 0x83CDC4);
	GAMEMD_STR(WarpIn, 0x83CDCC);

	GAMEMD_STR(ChronoTrigger, 0x83C6D8);
	GAMEMD_STR(ChronoDistanceFactor, 0x83C6E8);
	GAMEMD_STR(ChronoMinimumDelay, 0x83C6C4);
	GAMEMD_STR(ChronoRangeMinimum, 0x83C6B0);
	GAMEMD_STR(ChronoDelay, 0x83C714);

	GAMEMD_STR(EliteAbilities, 0x8434D4);
	GAMEMD_STR(VeteranAbilities, 0x8434E4);

	GAMEMD_STR(PoseDir, 0x83ABB8);
	GAMEMD_STR(FlyBack, 0x817FF0);

	GAMEMD_STR(Strength, 0x832B78);
	GAMEMD_STR(Armor, 0x81D9D4);
	GAMEMD_STR(Gravity, 0x83A34C);

	GAMEMD_STR(MissileROTVar , 0x83CAB4);
	GAMEMD_STR(MissileSafetyAltitude, 0x83CA9C);

	GAMEMD_STR(Parachute , 0x83CCD4);
	GAMEMD_STR(Scalable, 0x81B064);

	GAMEMD_STR(RepairStep, 0x83BDE8);
	GAMEMD_STR(RepairRate, 0x83BDD0);

	GAMEMD_STR(SuperWeapons, 0x83D4FC);

	GAMEMD_STR(OccupyDamageMultiplier, 0x83B08C);
	GAMEMD_STR(OccupyROFMultiplier, 0x83B078);

	GAMEMD_STR(BunkerDamageMultiplier, 0x83B04C);
	GAMEMD_STR(BunkerROFMultMultiplier, 0x83B038);

	GAMEMD_STR(BunkerWallsUpSound, 0x83A828);
	GAMEMD_STR(BunkerWallsDownSound, 0x83A810);

	GAMEMD_STR(EnterBioReactorSound, 0x83A704);
	GAMEMD_STR(LeaveBioReactorSound, 0x83A6EC);

	GAMEMD_STR(TalkBubbleTime, 0x83B404);

	GAMEMD_STR(DrainAnimationType, 0x83AF74);
	GAMEMD_STR(DrainMoneyAmount, 0x83AF48);
	GAMEMD_STR(DrainMoneyFrameDelay, 0x83AF5C);
	GAMEMD_STR(DiskLaserChargeUp, 0x83A670);
	
	GAMEMD_STR(PrerequisitePower, 0x83CC08);
	GAMEMD_STR(PrerequisiteFactory, 0x83CBF4);
	GAMEMD_STR(PrerequisiteBarracks, 0x83CBDC);
	GAMEMD_STR(PrerequisiteRadar, 0x83CBC8);
	GAMEMD_STR(PrerequisiteTech, 0x83CBB4);
	GAMEMD_STR(PrerequisiteProc, 0x83CBA0);

	GAMEMD_STR(MCVRedeploys, 0x83CF68);

	GAMEMD_STR(Verses, 0x847C38);
	//..

	// EVA entry names
	GAMEMD_STR(EVA_StructureSold, 0x819030u);
	GAMEMD_STR(EVA_UnitSold, 0x822630u);
	GAMEMD_STR(EVA_OreMinerUnderAttack, 0x824784u);
	GAMEMD_STR(EVA_UnitReady, 0x8249A0u);
	GAMEMD_STR(EVA_ConstructionComplete, 0x83FA80u);
	//...

	// Messages
	GAMEMD_STR(FailedToLoadUIMDMsg, 0x827DACu);
	//..

	// Hardcoded Names
	GAMEMD_STR(Anim_RING1, 0x8182F0u);
	GAMEMD_STR(Anim_INVISO, 0x8182F8u);
	GAMEMD_STR(XXICON, 0x844530u);
	GAMEMD_STR(GASCLUDM1, 0x84610C);
	GAMEMD_STR(CARYLAND, 0x822420);
	GAMEMD_STR(DROPLAND, 0x82242C);
	GAMEMD_STR(SQDG, 0x83665C);
	//..
	
	// CSF Labels
	GAMEMD_STR(TXT_TO_REPLAY        , 0x83DB24);
	GAMEMD_STR(TXT_OK               , 0x825FB0);
	GAMEMD_STR(TXT_CANCEL           , 0x825FD0);
	GAMEMD_STR(TXT_CONTROL          , 0x82729C);
	GAMEMD_STR(TXT_INTERFACE        , 0x826FEC);
	GAMEMD_STR(TXT_SELECTION        , 0x827250);
	GAMEMD_STR(TXT_TAUNT            , 0x827218);
	GAMEMD_STR(TXT_TEAM             , 0x826FA4);
	GAMEMD_STR(TXT_SAVING_GAME      , 0x820DD4);
	GAMEMD_STR(TXT_GAME_WAS_SAVED   , 0x829FE0);
	GAMEMD_STR(TXT_ERROR_SAVING_GAME, 0x829EBC);
	//..

	// Colors
	GAMEMD_STR(Red, 0x820538u);
	GAMEMD_STR(DarkRed, 0x836ED8u);
	GAMEMD_STR(Green, 0x82053Cu);
	GAMEMD_STR(Yellow, 0x820524u);
	GAMEMD_STR(Grey, 0x822728u);
	GAMEMD_STR(LightGrey, 0x836ECCu);
	// ..

	// RTTI
	GAMEMD_STR(Units, 0x8458F0u);
	GAMEMD_STR(Infantry, 0x8173D0u);
	// ..

	// Side/Country/House
	GAMEMD_STR(Neutral, 0x82BA08u);
	GAMEMD_STR(Civilian, 0x818164u);
	GAMEMD_STR(Special, 0x817318);
	// ..
};

#undef GAMEMD_STR
#endif