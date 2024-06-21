#pragma once

#include <string>

#include <CCINIClass.h>
#include <SessionClass.h>
#include <ScenarioClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Stream.h>
#include <Utilities/SavegameDef.h>

#include <Misc/DynamicPatcher/Common/EventSystems/EventSystem.h>

class INIConstant
{
public:
	inline static std::string _rulesName{ "" }; // rulesmd.ini
	inline static std::string _artName{ "" };   // artmd.ini
	inline static std::string _aiName{ "" };	 // aimd.ini

	inline static std::string _ra2md{ "" }; // ra2md.ini

	inline static std::string _gameModeName{ "" };
	inline static std::string _mapName{ "" };

	static std::string_view GetRulesName()
	{
		if (_rulesName.empty())
		{
			_rulesName = (LPCSTR)GameStrings::RULESMD_INI.get();
		}
		return _rulesName;
	}

	static std::string_view GetArtName()
	{
		if (_artName.empty())
		{
			_artName = (LPCSTR)GameStrings::ARTMD_INI.get();
		}
		return _artName;
	}

	static std::string_view GetAIName()
	{
		if (_aiName.empty())
		{
			_aiName = (LPCSTR)GameStrings::AIMD_INI.get();
		}
		return _aiName;
	}

	static std::string_view GetRa2md()
	{
		return _ra2md;
	}

	static std::string_view GetGameModeName()
	{
		return _gameModeName;
	}

	static std::string_view GetMapName()
	{
		if (_mapName.empty())
		{
			_mapName = ScenarioClass::Instance->FileName;
		}
		return _mapName;
	}

	static void SetGameModeName(EventSystem* sender, Event e, void* args)
	{
		// 直接进战役为空指针
		MPGameModeClass* pMPGame = SessionClass::Instance->MPGameMode;
		if (pMPGame)
		{
			_gameModeName = pMPGame->INIFilename.Buffer;
		}
#ifdef DEBUG
		Debug::Log("Config file info:\n  Rules = \"%s\"\n  Art = \"%s\"\n  Ai = \"%s\"\n  MapName = \"%s\"\n  GameMode = \"%s\"\n",
			GetRulesName().data(),
			GetArtName().data(),
			GetAIName().data(),
			GetMapName().data(),
			GetGameModeName().data());
#endif // DEBUG
	}

#pragma region Save/Load
	static void SaveGameModeName(EventSystem* sender, Event e, void* args)
	{
		SaveGameEventArgs* arg = (SaveGameEventArgs*)args;
		if (arg->IsStartInStream())
		{
			PhobosByteStream saver(_gameModeName.size());
			PhobosStreamWriter writer(saver);
			writer.Process(_gameModeName, false);
			saver.WriteBlockToStream(arg->Stream);
		}
	}
	static void LoadGameModeName(EventSystem* sender, Event e, void* args)
	{
		LoadGameEventArgs* arg = (LoadGameEventArgs*)args;
		if (arg->IsStartInStream())
		{
			PhobosByteStream loader(0);
			loader.ReadBlockFromStream(arg->Stream);
			PhobosStreamReader reader(loader);
			reader.Process(_gameModeName, false);
		}
	}
#pragma endregion

};
