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

class GameModeOptionsClass
{
public:
	// this is the same as SessionClass::Instance->Config
	static constexpr reference<GameModeOptionsClass, 0xA8B250u> const Instance{};

	int MPModeIndex;
	int ScenarioIndex;
	bool Bases;
	int Money;
	bool BridgeDestruction;
	bool Crates;
	bool ShortGame;
	bool SWAllowed;
	bool BuildOffAlly;
	int GameSpeed;
	bool MultiEngineer;
	int UnitCount;
	int AIPlayers;
	int AIDifficulty;
	AISlotsStruct AISlots;
	bool AlliesAllowed;
	bool HarvesterTruce;
	bool CTF;
	bool FogOfWar;
	bool MCVRedeploy;
	wchar_t MapDescription[45];
};

typedef GameModeOptionsClass GameOptionsType ;
static_assert(sizeof(GameModeOptionsClass) == 0x12C, "Invalid Size !");