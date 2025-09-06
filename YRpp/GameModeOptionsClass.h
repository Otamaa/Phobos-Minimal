#pragma once

#include <wchar.h>

#include <Helpers/CompileTime.h>

class StartingSlotClass;

struct AISlotsStruct
{
	int Difficulties[8];
	int Countries[8];
	int Colors[8];
	int Starts[8];
	int Allies[8];
};
static_assert(sizeof(AISlotsStruct) == 0xA0, "Invalid Size!");

class GameModeOptionsClass
{
public:
	// this is the same as SessionClass::Instance->Config
	static COMPILETIMEEVAL reference<GameModeOptionsClass, 0xA8B250u> const Instance{};

	bool Load(IStream* pStm)
	{ JMP_THIS(0x69B5B0); }

	bool Save(IStream* pStm)
	{ JMP_THIS(0x69B560); }

public:

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