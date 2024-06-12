#pragma once

#include "Version.h"
#include <Base/Always.h>

enum class StateEffectTypes
{
	unk,
	AntiBullet,
	BlackHole,
	DamageReaction,
	Deselect,
	DestroyAnim,
	DestroySelf,
	DisableWeapon,
	ECM,
	Freeze,
	GiftBox,
	NoMoneyNoTalk,
	OverrideWeapon,
	Paintball,
	Pump,
	Scatter,
	Teleport,
	Transform,
};

class EventSystem;
class Event;

void ExtTypeRegistryClear(EventSystem* sender, Event e, void* args);
void DetachAll(EventSystem* sender, Event e, void* args);

class Kratos
{
public:

	static bool IsScenarioClear;

	constexpr void GenerateSaveGameID(DWORD& _version)
	{
		_version += KRATOS_SAVEGAME_ID;
	}

	constexpr void GenerateLoadGameID(DWORD& _version)
	{
		_version -= KRATOS_SAVEGAME_ID;
	}

	static void Initialize();
	static void LoadGameGlobal(IStream* pStm);
	static void SaveGameGlobal(IStream* pStm);
	static void ClearScenarioStart();
	static void ClearScenarioEnd();

	static void LoadGameStart(const char* name);
	static void LoadGameEnd(const char* name);

	static void SaveGameStart(const char* name);
	static void SaveGameEnd(const char* name);

	static void Render();
	static void RenderLate();

	static void ScenarioStart();

	static void LoadGameInStream(IStream* stream);
	static void LoadGameInStreamEnd(IStream* stream);

	static void SaveGameInStream(IStream* stream);
	static void SaveGameInStreamEnd(IStream* stream);
};
