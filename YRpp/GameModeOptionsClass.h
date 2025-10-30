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

	bool Save(LPSTREAM stream)
	{
		ULONG written = 0;

		// Write the size of the class first
		const DWORD dataSize = sizeof(GameModeOptionsClass);
		HRESULT hr = stream->Write(&dataSize, sizeof(DWORD), &written);

		if (FAILED(hr) || written != sizeof(DWORD)) {
			return false;
		}

		// Write the entire object data
		hr = stream->Write(this, dataSize, &written);
		if (FAILED(hr) || written != dataSize) {
			return false;
		}

		return true;
	}

	bool Load(LPSTREAM stream)
	{
		ULONG read = 0;

		// Read the size into a separate variable (NOT into the stream pointer!)
		DWORD dataSize = 0;
		HRESULT hr = stream->Read(&dataSize, sizeof(DWORD), &read);

		if (FAILED(hr) || read != sizeof(DWORD)) {
			return false;
		}

		// Validate the size matches what we expect
		if (dataSize != sizeof(GameModeOptionsClass)) {
			// Size mismatch - possible corruption or version incompatibility
			return false;
		}

		// Read the object data
		hr = stream->Read(this, dataSize, &read);

		if (FAILED(hr) || read != dataSize) {
			return false;
		}

		// Reset ScenarioIndex as done in original
		this->ScenarioIndex = -1;

		return true;
	}

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