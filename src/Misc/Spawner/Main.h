#pragma once

/**
*  yrpp-spawner
*
*  Copyright(C) 2022-present CnCNet
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.If not, see <http://www.gnu.org/licenses/>.
*/
#include <Memory.h>
#include <Utilities/GameUniquePointers.h>

#include <MixFileClass.h>

#include <list>
#include <vector>
#include <string>

class GameSpeedSlider
{
public:
	// Enabled only when spawner active and config flag set.
	static bool IsEnabled();
	static bool IsDisabled();
};

class CCINIClass;
class EventClass;
struct SpawnerMain
{
	struct Configs {
		static bool Enabled; // false
		static bool Active; //false
		static bool DoSave;
		static int NextAutoSaveFrame;
		static int NextAutoSaveNumber;

	public:

		bool DumpTypes {false};
		bool MPDebug { false };
		bool SingleProcAffinity { true };

		bool DisableEdgeScrolling { false };
		bool QuickExit { true };
		bool SkipScoreScreen { false };
		bool DDrawHandlesClose { false };
		bool SpeedControl { false };

		bool WindowedMode { false };
		bool NoWindowFrame { false };
		int DDrawTargetFPS { -1 };

		bool AllowTaunts { true };
		bool AllowChat { true };
	public:

		static Configs m_Ptr;
	};

	struct GameConfigs {
		static void Init();
		static bool StartGame();
		static void AssignHouses();
		static void After_Main_Loop();
		static void RespondToSaveGame(EventClass* event);

	public:
		static  GameConfigs m_Ptr;

	private:
		static bool StartScenario(const char* scenarioName);
		static bool LoadSavedGame(const char* scenarioName);
		static bool Reconcile_Players();
		static void InitNetwork();
		static void LoadSidesStuff();

	public:

		// Used to create NodeNameType
		// The order of entries may differ from HouseConfig
		struct PlayerConfig
		{
			bool IsHuman { false };
			wchar_t Name[20] { L"" };
			int Color { -1 };
			int Country { -1 };
			int Difficulty { -1 };
			bool IsObserver { false };
			char Ip[0x20] { "0.0.0.0" };
			int Port { -1 };

			void LoadFromINIFile(CCINIClass* pINI, int index);
		};

		// Used to augment the generated HouseClass
		// The order of entries may differ from PlayerConfig
		struct HouseConfig
		{
			bool IsObserver { false };
			int SpawnLocations { -2 };
			double CreditsFactor { 1.0 };
			int HandicapDifficulty { -1 };
			int Alliances[8] { -1, -1, -1, -1, -1, -1, -1, -1 };

			void LoadFromINIFile(CCINIClass* pINI, int index);
		};

	public:
		// Game Mode Options
		int  MPModeIndex;
		bool Bases;
		int  Credits;
		bool BridgeDestroy;
		bool Crates;
		bool ShortGame;
		bool SuperWeapons;
		bool BuildOffAlly;
		int  GameSpeed;
		bool MultiEngineer;
		int  UnitCount;
		int  AIPlayers;
		int  AIDifficulty;
		bool AlliesAllowed;
		bool HarvesterTruce;
		bool FogOfWar;
		bool MCVRedeploy;
		wchar_t UIGameMode[60];
		bool SpecialHouseIsAlly;

		// SaveGame Options
		bool LoadSaveGame;
		char SavedGameDir[MAX_PATH]; // Nested paths are also supported, e.g. "Saved Games\\Yuri's Revenge"
		char SaveGameName[60];

		int AutoSaveCount;
		int AutoSaveInterval;
		int NextAutoSaveNumber;

		int CustomMissionID;

		// Scenario Options
		int  Seed;
		int  TechLevel;
		bool IsCampaign;
		int  Tournament;
		DWORD WOLGameID;
		char ScenarioName[260];
		char MapHash[0xff];
		wchar_t UIMapName[45];

		// Network Options
		int Protocol;
		int FrameSendRate;
		int ReconnectTimeout;
		int ConnTimeout;
		int MaxAhead;
		int PreCalcMaxAhead;
		byte MaxLatencyLevel;
		bool ForceMultiplayer;

		// Tunnel Options
		int  TunnelId;
		char TunnelIp[0x20];
		int  TunnelPort;
		int  ListenPort;

		// Players Options
		PlayerConfig Players[8];

		// Houses Options
		HouseConfig Houses[8];

		// Extended Options
		bool QuickMatch;
		bool DisableGameSpeed;
		bool SpawnerHackMPNodes;
		bool SkipScoreScreen;
		bool WriteStatistics;
		bool AINamesByDifficulty;
		bool ContinueWithoutHumans;
		bool DefeatedBecomesObserver;
		bool Observer_ShowAIOnSidebar;
		bool Observer_ShowMultiplayPassive;

		// Custom mixes
		// Note: std::list and std::string will be realised followed to RAII concept. It is pretty save instead of const char*.

		std::list<std::string> PreloadMixes;
		std::list<std::string> PostloadMixes;

		GameConfigs();
		~GameConfigs() = default;

		void LoadFromINIFile(CCINIClass* pINI);
	};

	static std::list<MixFileClass*> LoadedMixFiles;

	static void CmdLineParse(char*);
	static void PrintInitializeLog();

	static void LoadConfigurations(); // Early load settings from ra2md
	static void ApplyStaticOptions(); // Apply all the settings

	static COMPILETIMEEVAL Configs* GetMainConfigs() {
		return &Configs::m_Ptr;
	}

	static COMPILETIMEEVAL GameConfigs* GetGameConfigs() {
		return &GameConfigs::m_Ptr;
	}
};