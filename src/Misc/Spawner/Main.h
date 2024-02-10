#pragma once

/*
* Copyright (c) 2012, 2013, 2014 Toni Spets <toni.spets@iki.fi>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <Memory.h>

class CCINIClass;
struct SpawnerMain
{
	struct Configs {
		static bool Enabled; // false
		static bool Active; //false

		static bool DumpTypes; // false

		static bool MPDebug; // false
		static bool SingleProcAffinity;  // true

		static bool DisableEdgeScrolling;  // false
		static bool QuickExit;  // false
		static bool SkipScoreScreen;  // false
		static bool DDrawHandlesClose;  // false
		static bool SpeedControl;  // false

		static bool WindowedMode;  // false
		static bool NoWindowFrame;  // false
		static int DDrawTargetFPS;  // -1
	};

	struct GameConfigs {
		static void Init();
		static bool StartGame();
		static void AssignHouses();

	private:
		static std::unique_ptr<GameConfigs> Config;

	private:
		static bool StartNewScenario(const char* scenarioName);
		static bool LoadSavedGame(const char* scenarioName);

		static void InitNetwork();
		static void LoadSidesStuff();

	public:

		static GameConfigs* GetGameConfigs() {
			return Config.get();
		}

		// Used to create NodeNameType
		// The order of entries may differ from HouseConfig
		struct PlayerConfig
		{
			bool IsHuman;
			wchar_t Name[20];
			int Color;
			int Country;
			int Difficulty;
			bool IsObserver;
			char Ip[0x20];
			int Port;

			PlayerConfig()
				: IsHuman { false }
				, Name { L"" }
				, Color { -1 }
				, Country { -1 }
				, Difficulty { -1 }
				, IsObserver { false }
				, Ip { "0.0.0.0" }
				, Port { -1 }
			{
			}

			void LoadFromINIFile(CCINIClass* pINI, int index);
		};

		// Used to augment the generated HouseClass
		// The order of entries may differ from PlayerConfig
		struct HouseConfig
		{
			bool IsObserver;
			int SpawnLocations;
			int Alliances[8];

			HouseConfig()
				: IsObserver { false }
				, SpawnLocations { -2 }
				, Alliances { -1, -1, -1, -1, -1, -1, -1, -1 }
			{
			}

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

		// SaveGame Options
		bool LoadSaveGame;
		char SaveGameName[60];

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
		bool Ra2Mode;
		bool QuickMatch;
		bool SkipScoreScreen;
		bool WriteStatistics;
		bool AINamesByDifficulty;
		bool ContinueWithoutHumans;
		bool DefeatedBecomesObserver;
		bool Observer_ShowAIOnSidebar;

		GameConfigs() // default values
			// Game Mode Options
			: MPModeIndex { 1 }
			, Bases { true }
			, Credits { 10000 }
			, BridgeDestroy { true }
			, Crates { false }
			, ShortGame { false }
			, SuperWeapons { true }
			, BuildOffAlly { false }
			, GameSpeed { 0 }
			, MultiEngineer { false }
			, UnitCount { 0 }
			, AIPlayers { 0 }
			, AIDifficulty { 1 }
			, AlliesAllowed { false }
			, HarvesterTruce { false }
			, FogOfWar { false }
			, MCVRedeploy { true }
			, UIGameMode { L"" }

			// SaveGame
			, LoadSaveGame { false }
			, SaveGameName { "" }

			// Scenario Options
			, Seed { 0 }
			, TechLevel { 10 }
			, IsCampaign { false }
			, Tournament { 0 }
			, WOLGameID { 0xDEADBEEF }
			, ScenarioName { "spawnmap.ini" }
			, MapHash { "" }
			, UIMapName { L"" }

			// Network Options
			, Protocol { 2 }
			, FrameSendRate { 4 }
			, ReconnectTimeout { 2400 }
			, ConnTimeout { 3600 }
			, MaxAhead { -1 }
			, PreCalcMaxAhead { 0 }
			, MaxLatencyLevel { 0xFF }

			// Tunnel Options
			, TunnelId { 0 }
			, TunnelIp { "0.0.0.0" }
			, TunnelPort { 0 }
			, ListenPort { 1234 }

			// Players Options
			, Players {
				PlayerConfig(),
				PlayerConfig(),
				PlayerConfig(),
				PlayerConfig(),

				PlayerConfig(),
				PlayerConfig(),
				PlayerConfig(),
				PlayerConfig()
			}

			// Houses Options
			, Houses {
				HouseConfig(),
				HouseConfig(),
				HouseConfig(),
				HouseConfig(),

				HouseConfig(),
				HouseConfig(),
				HouseConfig(),
				HouseConfig()
			}

			// Extended Options
			, Ra2Mode { false }
			, QuickMatch { false }
			, SkipScoreScreen { Configs::SkipScoreScreen }
			, WriteStatistics { false }
			, AINamesByDifficulty { false }
			, ContinueWithoutHumans { false }
			, DefeatedBecomesObserver { false }
			, Observer_ShowAIOnSidebar { false }
		{
		}

		void LoadFromINIFile(CCINIClass* pINI);
	};

	static void ExeRun();
	static void CmdLineParse(char**, int);
	static void PrintInitializeLog();

	static void LoadConfigurations(); // Early load settings from ra2md
	static void ApplyStaticOptions(); // Apply all the settings

	static void InitMixes();
};