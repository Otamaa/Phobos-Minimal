#pragma once

#include <wchar.h>

#include <Helpers/CompileTime.h>

class StartingSlotClass;

struct AISlotsStruct
{
	ArrayWrapper<int , 8u> AIDifficulties;
	ArrayWrapper<StartingSlotClass*, 8u> StartingSpots;
	ArrayWrapper<int, 8u> Colours;
	ArrayWrapper<int, 8u> Starts;
	ArrayWrapper<int, 8u> Teams;
	
};
static_assert(sizeof(AISlotsStruct) == 0xA0, "Invalid Size!");

class GameModeOptionsClass
{
public:
	// this is the same as SessionClass::Instance->Config
	static constexpr reference<GameModeOptionsClass, 0xA8B250u> const Instance{};

	int MPModeIndex;
	int ScenarioIndex;
	bool Bases;
	PROTECTED_PROPERTY(BYTE, align_9[3]);
	int Money;
	bool BridgeDestruction;
	bool Crates;
	bool ShortGame;
	bool SWAllowed;
	bool BuildOffAlly;
	PROTECTED_PROPERTY(BYTE, align_15[3]);
	int GameSpeed;
	bool MultiEngineer;
	PROTECTED_PROPERTY(BYTE, align_1D[3]);
	int UnitCount;
	int AIPlayers;
	int AIDifficulty;
	AISlotsStruct AISlots;
	bool AlliesAllowed;
	bool HarvesterTruce;
	bool CTF;
	bool FogOfWar;
	bool MCVRedeploy;
	PROTECTED_PROPERTY(BYTE, align_D1);
	wchar_t MapDescription[45];
};

typedef GameModeOptionsClass GameOptionsType ;
static_assert(sizeof(GameModeOptionsClass) == 0x12C, "Invalid Size !");