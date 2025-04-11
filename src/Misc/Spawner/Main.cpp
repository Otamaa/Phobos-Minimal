#include "Main.h"

#include <Phobos.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/GameConfig.h>

#include <LoadOptionsClass.h>
#include <UDPInterfaceClass.h>
#include <IPXManagerClass.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Multithread.h>

#include <Ext/Event/Body.h>

#include <Ext/House/Body.h>

#include "DumpTypeDataArrayToFile.h"
#include "NetHack.h"
#include "ProtocolZero.h"

#include <GameOptionsClass.h>
#include <MessageBox.h>

SpawnerMain::GameConfigs SpawnerMain::GameConfigs::m_Ptr {};

FORCEDINLINE void ReadListFromSection(CCINIClass* pINI, const char* pSection, std::list<std::string>& strings)
{
	if (!pINI->GetSection(pSection))
		return;

	for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
	{
		char buffer[0x80];

		if (pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), Phobos::readDefval, buffer) > 0) {
			strings.emplace_back(buffer);
		}
	}
}

SpawnerMain::Configs SpawnerMain::Configs::m_Ptr {};

class NOVTABLE StaticLoadOptionsClass {
public:
	// constructor
	StaticLoadOptionsClass() {
		JMP_THIS(0x558740);
	}

	// virtuals
	virtual ~StaticLoadOptionsClass() {
		JMP_THIS(0x55A0D0);
	}

	static bool LoadMission(const char* pFilename) {
		JMP_THIS(0x559D60);
	}

	//Properties
	LoadOptionsMode Mode;
	const char* Extension; //"SAV", "SED" for MapSeedClass
	const wchar_t* Description;
	unsigned int SpaceRequirement; //default is 0x800 = 2048
	DWORD unknown_14;
	DWORD unknown_18;
	DWORD unknown_1C;
	DECLARE_PROPERTY(DynamicVectorClass<FileEntryClass*>, FileEntries);
};

#ifdef IS_ANTICHEAT_VER
#define SPAWNER_PRODUCT_NAME "YR-Spawner (AntiCheat)"
#else
#define SPAWNER_PRODUCT_NAME "YR-Spawner"
#endif

#define SPAWNER_FILE_DESCRIPTION "CnCNet5: Spawner"

void SpawnerMain::CmdLineParse(char* pArg)
{
	if (0 == strcasecmp(pArg, "-SPAWN"))
	{
		Phobos::Otamaa::NoCD = true;
		SpawnerMain::Configs::Enabled = true;
	}
	else if (0 == strcasecmp(pArg, "-DumpTypes"))
	{
		SpawnerMain::Configs::m_Ptr.DumpTypes = true;
	}
}

void SpawnerMain::PrintInitializeLog() {
	Debug::LogInfo("Initialized " SPAWNER_PRODUCT_NAME "");
}

void SpawnerMain::LoadConfigurations()
{
	if (auto pINI = &CCINIClass::INI_RA2MD) {

		if (pINI->GetSection(GameStrings::Options())) {
			auto pMainConfigs = SpawnerMain::GetMainConfigs();
			pMainConfigs->MPDebug = pINI->ReadBool(GameStrings::Options(), "MPDEBUG", pMainConfigs->MPDebug);
			pMainConfigs->SingleProcAffinity = pINI->ReadBool(GameStrings::Options(), "SingleProcAffinity", pMainConfigs->SingleProcAffinity);
			pMainConfigs->DisableEdgeScrolling = pINI->ReadBool(GameStrings::Options(), "DisableEdgeScrolling", pMainConfigs->DisableEdgeScrolling);
			pMainConfigs->QuickExit = pINI->ReadBool(GameStrings::Options(), "QuickExit", pMainConfigs->QuickExit);
			pMainConfigs->SkipScoreScreen = pINI->ReadBool(GameStrings::Options(), "SkipScoreScreen", pMainConfigs->SkipScoreScreen);
			pMainConfigs->DDrawHandlesClose = pINI->ReadBool(GameStrings::Options(), "DDrawHandlesClose", pMainConfigs->DDrawHandlesClose);
			pMainConfigs->SpeedControl = pINI->ReadBool(GameStrings::Options(), "SpeedControl", pMainConfigs->SpeedControl);
			pMainConfigs->AllowTaunts = pINI->ReadBool(GameStrings::Options(), "AllowTaunts", pMainConfigs->AllowTaunts);
			pMainConfigs->AllowChat = pINI->ReadBool(GameStrings::Options(), "AllowChat", pMainConfigs->AllowChat);

		}

		if (pINI->GetSection(GameStrings::Video())) {
			auto pMainConfigs = SpawnerMain::GetMainConfigs();
			pMainConfigs->WindowedMode = pINI->ReadBool(GameStrings::Video(), "Video.Windowed", pMainConfigs->WindowedMode);
			pMainConfigs->NoWindowFrame = pINI->ReadBool(GameStrings::Video(), "NoWindowFrame", pMainConfigs->NoWindowFrame);
			pMainConfigs->DDrawTargetFPS = pINI->ReadInteger(GameStrings::Video(), "DDrawTargetFPS", pMainConfigs->DDrawTargetFPS);
		}
	}
}

void SpawnerMain::ApplyStaticOptions()
{
	auto pMainConfigs = SpawnerMain::GetMainConfigs();

	if (pMainConfigs->MPDebug)
	{
		Game::EnableMPDebug = true;
		Game::DrawMPDebugStats = true;
		Game::EnableMPSyncDebug = true;

		// Fixes text layout in the MPDebug panel
		Patch::Apply_TYPED<DWORD>(0x542A19, { 312 });
		Patch::Apply_TYPED<DWORD>(0x542AA6, { 322 });
		Patch::Apply_TYPED<DWORD>(0x542B08, { 332 });
		Patch::Apply_TYPED<DWORD>(0x542B72, { 342 });
		Patch::Apply_TYPED<DWORD>(0x542BD4, { 352 });
		Patch::Apply_TYPED<DWORD>(0x542C94, { 362 });
		Patch::Apply_TYPED<DWORD>(0x542CF7, { 372 });
		Patch::Apply_TYPED<DWORD>(0x542D5E, { 382 });
		Patch::Apply_TYPED<DWORD>(0x542DC2, { 392 });
	}

	if (pMainConfigs->SingleProcAffinity) {
		DWORD_PTR const processAffinityMask = 1;
		SetProcessAffinityMask(Patch::CurrentProcess, processAffinityMask);
	}

	if (pMainConfigs->WindowedMode)
	{
		GameOptionsClass::WindowedMode = true;

		if (pMainConfigs->NoWindowFrame) {
			Patch::Apply_RAW(0x777CC0, // CreateMainWindow
			{
				0x68, 0x00, 0x00, 0x0A, 0x86 // push    0x860A0000; vs 0x02CA0000
			});
		}
	}

	if (pMainConfigs->SpeedControl)
	{
		Game::SpeedControl = true;
	}

	if(SessionClass::Instance->GameMode == GameMode::LAN)
		Game::LANTaunts = pMainConfigs->AllowTaunts;
	else if(SessionClass::Instance->GameMode == GameMode::Internet)
		Game::WOLTaunts =  pMainConfigs->AllowTaunts;

	// Set 3rd party ddraw.dll options
	for (auto& dllData : Patch::ModuleDatas) {
		if (IS_SAME_STR_(dllData.ModuleName.c_str(), "ddraw.dll")) {
			if (bool* gameHandlesClose = (bool*)GetProcAddress(dllData.Handle, "GameHandlesClose"))
				*gameHandlesClose = !pMainConfigs->DDrawHandlesClose;

			LPDWORD TargetFPS = (LPDWORD)GetProcAddress(dllData.Handle, "TargetFPS");
			if (TargetFPS && pMainConfigs->DDrawTargetFPS != -1)
				*TargetFPS = pMainConfigs->DDrawTargetFPS;

			break;
		}
	}

	//if (HMODULE hDDraw = LoadLibraryA("ddraw.dll")) {
	//
	//	if (bool* gameHandlesClose = (bool*)GetProcAddress(hDDraw, "GameHandlesClose"))
	//		*gameHandlesClose = !SpawnerMain::Configs::DDrawHandlesClose;
	//
	//	LPDWORD TargetFPS = (LPDWORD)GetProcAddress(hDDraw, "TargetFPS");
	//	if (TargetFPS && SpawnerMain::Configs::DDrawTargetFPS != -1)
	//		*TargetFPS = SpawnerMain::Configs::DDrawTargetFPS;
	//}
}


COMPILETIMEEVAL char* PlayerSectionArray[8] = {
	"Settings",
	"Other1",
	"Other2",
	"Other3",
	"Other4",
	"Other5",
	"Other6",
	"Other7"
};

COMPILETIMEEVAL char* MultiTagArray[8] = {
	"Multi1",
	"Multi2",
	"Multi3",
	"Multi4",
	"Multi5",
	"Multi6",
	"Multi7",
	"Multi8"
};

COMPILETIMEEVAL char* AlliancesSectionArray[8] = {
	"Multi1_Alliances",
	"Multi2_Alliances",
	"Multi3_Alliances",
	"Multi4_Alliances",
	"Multi5_Alliances",
	"Multi6_Alliances",
	"Multi7_Alliances",
	"Multi8_Alliances"
};

COMPILETIMEEVAL char* AlliancesTagArray[8] = {
	"HouseAllyOne",
	"HouseAllyTwo",
	"HouseAllyThree",
	"HouseAllyFour",
	"HouseAllyFive",
	"HouseAllySix",
	"HouseAllySeven",
	"HouseAllyEight"
};

class CCINIClassDummy : public CCINIClass {
public:

	int NOINLINE ReadString_WithoutAresHook(const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
	{
		EPILOG_THISCALL;

		// It's 5 bytes corrupted by the ares hook
		_asm { sub  esp, 0xC };
		_asm { xor eax, eax };

		_asm { mov edx, 0x528A10 + 5 };
		_asm { jmp edx }
	}
};
static_assert(sizeof(CCINIClassDummy) == sizeof(CCINIClass), "Invalid Size !");

#include <string>

void SpawnerMain::GameConfigs::LoadFromINIFile(CCINIClass* pINI)
{
	if (!pINI || !pINI->GetSection(GameStrings::Settings()))
		return;

	{ // Game Mode Options
		MPModeIndex = pINI->ReadInteger(GameStrings::Settings(), GameStrings::GameMode, MPModeIndex);
		Bases = pINI->ReadBool(GameStrings::Settings(), GameStrings::Bases, Bases);
		Credits = pINI->ReadInteger(GameStrings::Settings(), GameStrings::Credits, Credits);
		BridgeDestroy = pINI->ReadBool(GameStrings::Settings(), "BridgeDestroy", BridgeDestroy);
		Crates = pINI->ReadBool(GameStrings::Settings(), GameStrings::Crates, Crates);
		ShortGame = pINI->ReadBool(GameStrings::Settings(), GameStrings::ShortGame, ShortGame);
		// cncnet/spawner using `Superweapons` is this typo or intention ? , idk
		// please report if there is a problem @Otamaa
		SuperWeapons = pINI->ReadBool(GameStrings::Settings(), GameStrings::SuperWeapons, SuperWeapons);
		SuperWeapons = pINI->ReadBool(GameStrings::Settings(), "Superweapons", SuperWeapons);
		BuildOffAlly = pINI->ReadBool(GameStrings::Settings(), GameStrings::BuildOffAlly, BuildOffAlly);
		GameSpeed = pINI->ReadInteger(GameStrings::Settings(), GameStrings::GameSpeed, GameSpeed);
		MultiEngineer = pINI->ReadBool(GameStrings::Settings(), GameStrings::MultiEngineer, MultiEngineer);
		UnitCount = pINI->ReadInteger(GameStrings::Settings(), GameStrings::UnitCount, UnitCount);
		AIPlayers = pINI->ReadInteger(GameStrings::Settings(), GameStrings::AIPlayers, AIPlayers);
		AIDifficulty = pINI->ReadInteger(GameStrings::Settings(), GameStrings::AIDifficulty, AIDifficulty);
		AlliesAllowed = pINI->ReadBool(GameStrings::Settings(), GameStrings::AlliesAllowed, AlliesAllowed);
		HarvesterTruce = pINI->ReadBool(GameStrings::Settings(), GameStrings::HarvesterTruce, HarvesterTruce);
		FogOfWar = pINI->ReadBool(GameStrings::Settings(), GameStrings::FogOfWar, FogOfWar);
		MCVRedeploy = pINI->ReadBool(GameStrings::Settings(), GameStrings::MCVRedeploy, MCVRedeploy);

		if (((CCINIClassDummy*)pINI)->ReadString_WithoutAresHook(GameStrings::Settings(), "UIGameMode", Phobos::readDefval, Phobos::readBuffer, Phobos::readLength) > 0)
			MultiByteToWideChar(CP_UTF8, 0, Phobos::readBuffer, strlen(Phobos::readBuffer), UIGameMode, std::size(UIGameMode));
	}

	// SaveGame Options
	LoadSaveGame = pINI->ReadBool(GameStrings::Settings(), "LoadSaveGame", LoadSaveGame);
	/* SavedGameDir */
	pINI->ReadString(GameStrings::Settings(), "SavedGameDir", SavedGameDir, SavedGameDir, sizeof(SavedGameDir));
	/* SaveGameName */
	pINI->ReadString(GameStrings::Settings(), "SaveGameName", SaveGameName, SaveGameName, sizeof(SaveGameName));

	AutoSaveCount = pINI->ReadInteger(GameStrings::Settings(), "AutoSaveCount", AutoSaveCount);
	AutoSaveInterval = pINI->ReadInteger(GameStrings::Settings(), "AutoSaveInterval", AutoSaveInterval);
	NextAutoSaveNumber = pINI->ReadInteger(GameStrings::Settings(), "NextAutoSaveNumber", NextAutoSaveNumber);

	CustomMissionID = pINI->ReadInteger(GameStrings::Settings(), "CampaignID", CustomMissionID);

	{ // Scenario Options
		Seed = pINI->ReadInteger(GameStrings::Settings(), "Seed", Seed);
		TechLevel = pINI->ReadInteger(GameStrings::Settings(), GameStrings::TechLevel, TechLevel);
		IsCampaign = pINI->ReadBool(GameStrings::Settings(), "IsSinglePlayer", IsCampaign);
		Tournament = pINI->ReadInteger(GameStrings::Settings(), "Tournament", Tournament);
		WOLGameID = pINI->ReadInteger(GameStrings::Settings(), "GameID", WOLGameID);
		/* ScenarioName*/
		pINI->ReadString(GameStrings::Settings(), GameStrings::Scenario, ScenarioName, ScenarioName, sizeof(ScenarioName));
		/* MapHash*/
		pINI->ReadString(GameStrings::Settings(), "MapHash", MapHash, MapHash, sizeof(MapHash));

		if (((CCINIClassDummy*)pINI)->ReadString_WithoutAresHook(GameStrings::Settings(), "UIMapName", "", Phobos::readBuffer, Phobos::readLength) > 0)
			MultiByteToWideChar(CP_UTF8, 0, Phobos::readBuffer, strlen(Phobos::readBuffer), UIMapName, std::size(UIMapName));
	}

	{ // Network Options
		Protocol = pINI->ReadInteger(GameStrings::Settings(), "Protocol", Protocol);
		FrameSendRate = pINI->ReadInteger(GameStrings::Settings(), "FrameSendRate", FrameSendRate);
		ReconnectTimeout = pINI->ReadInteger(GameStrings::Settings(), "ReconnectTimeout", ReconnectTimeout);
		ConnTimeout = pINI->ReadInteger(GameStrings::Settings(), "ConnTimeout", ConnTimeout);
		MaxAhead = pINI->ReadInteger(GameStrings::Settings(), "MaxAhead", MaxAhead);
		PreCalcMaxAhead = pINI->ReadInteger(GameStrings::Settings(), "PreCalcMaxAhead", PreCalcMaxAhead);
		MaxLatencyLevel = (byte)pINI->ReadInteger(GameStrings::Settings(), "MaxLatencyLevel", (int)MaxLatencyLevel);
	}

	{ // Tunnel Options
		// Otama : ?????????
		TunnelId = pINI->ReadInteger(GameStrings::Settings(), "Port", TunnelId);
		ListenPort = pINI->ReadInteger(GameStrings::Settings(), "Port", ListenPort);

		if (pINI->GetSection(GameStrings::Tunnel())) {
			pINI->ReadString(GameStrings::Tunnel(), "Ip", TunnelIp, TunnelIp, sizeof(TunnelIp));
			TunnelPort = pINI->ReadInteger(GameStrings::Tunnel(), "Port", TunnelPort);
		}
	}

	// Players Options
	for (char i = 0; i < (char)std::size(Players); ++i)
	{
		(&Players[i])->LoadFromINIFile(pINI, i);
		(&Houses[i])->LoadFromINIFile(pINI, i);
	}

	// Extended Options
	SpawnerHackMPNodes = pINI->ReadBool(GameStrings::Settings(), "UseMPAIBaseNodes", SpawnerHackMPNodes);
	QuickMatch = pINI->ReadBool(GameStrings::Settings(), "QuickMatch", QuickMatch);
	SkipScoreScreen = pINI->ReadBool(GameStrings::Settings(), "SkipScoreScreen", SkipScoreScreen);
	WriteStatistics = pINI->ReadBool(GameStrings::Settings(), "WriteStatistics", WriteStatistics);
	AINamesByDifficulty = pINI->ReadBool(GameStrings::Settings(), "AINamesByDifficulty", AINamesByDifficulty);
	ContinueWithoutHumans = pINI->ReadBool(GameStrings::Settings(), "ContinueWithoutHumans", ContinueWithoutHumans);
	DefeatedBecomesObserver = pINI->ReadBool(GameStrings::Settings(), "DefeatedBecomesObserver", DefeatedBecomesObserver);
	Observer_ShowAIOnSidebar = pINI->ReadBool(GameStrings::Settings(), "Observer.ShowAIOnSidebar", Observer_ShowAIOnSidebar);
	Observer_ShowMultiplayPassive = pINI->ReadBool(GameStrings::Settings(), "Observer.ShowMultiplayPassiveOnSidebar",  Phobos::Otamaa::IsAdmin  ? true : Observer_ShowMultiplayPassive);
	// Custom Mixes
	ReadListFromSection(pINI, "PreloadMixes", PreloadMixes);
	ReadListFromSection(pINI, "PostloadMixes", PostloadMixes);
}

void SpawnerMain::GameConfigs::PlayerConfig::LoadFromINIFile(CCINIClass* pINI, int index)
{
	if (!pINI || index >= 8)
		return;

	const char* pSection = PlayerSectionArray[index];
	const char* pMultiTag = MultiTagArray[index];

	if (pINI->GetSection(pSection))
	{
		this->IsHuman = true;
		this->Difficulty = -1;

		if (((CCINIClassDummy*)pINI)->ReadString_WithoutAresHook(pSection, "Name", Phobos::readDefval, Phobos::readBuffer, Phobos::readLength))
			MultiByteToWideChar(CP_UTF8, 0, Phobos::readBuffer, -1, this->Name, std::size(this->Name));

		this->Color = pINI->ReadInteger(pSection, GameStrings::Color(), this->Color);
		this->Country = pINI->ReadInteger(pSection, "Side", this->Country);
		this->IsObserver = pINI->ReadBool(pSection, "IsSpectator", this->IsObserver);

		pINI->ReadString(pSection, "Ip", this->Ip, this->Ip, sizeof(this->Ip));
		this->Port = pINI->ReadInteger(pSection, "Port", this->Port);
	}
	else if (!IsHuman)
	{
		this->Color = pINI->ReadInteger("HouseColors", pMultiTag, this->Color);
		this->Country = pINI->ReadInteger("HouseCountries", pMultiTag, this->Country);
		this->Difficulty = pINI->ReadInteger("HouseHandicaps", pMultiTag, this->Difficulty);
	}

	Debug::LogInfo("Reading Config for[{}  - {}] Color [{}]", pSection, pMultiTag, this->Color);
}

void SpawnerMain::GameConfigs::HouseConfig::LoadFromINIFile(CCINIClass* pINI, int index)
{
	if (!pINI || index >= 8)
		return;

	const char* pAlliancesSection = AlliancesSectionArray[index];
	const char* pMultiTag = MultiTagArray[index];

	this->IsObserver = pINI->ReadBool("IsSpectator", pMultiTag, this->IsObserver);
	this->SpawnLocations = pINI->ReadInteger("SpawnLocations", pMultiTag, SpawnLocations);

	if (pINI->GetSection(pAlliancesSection))
	{
		for (int i = 0; i < 8; i++)
			this->Alliances[i] = pINI->ReadInteger(pAlliancesSection, AlliancesTagArray[i], this->Alliances[i]);
	}
}

void SpawnerMain::GameConfigs::Init() {

	if (!SpawnerMain::Configs::Enabled)
		return;

	//SpawnerMain::GameConfigs::m_Ptr = std::make_unique<SpawnerMain::GameConfigs>();

	GameConfig file { "SPAWN.INI" };
	file.OpenINIAction([&file](CCINIClass* pFile) {
		Debug::LogInfo("SpawnerMain::GameConfigs::Init Reading file {}", file.filename());
		SpawnerMain::GameConfigs::m_Ptr.LoadFromINIFile(pFile);
	});

	Patch::Apply_CALL(0x48CDD3, SpawnerMain::GameConfigs::StartGame); // Main_Game
	Patch::Apply_CALL(0x48CFAA, SpawnerMain::GameConfigs::StartGame); // Main_Game

	{ // HousesStuff
		Patch::Apply_CALL(0x68745E, SpawnerMain::GameConfigs::AssignHouses); // Read_Scenario_INI
		Patch::Apply_CALL(0x68ACFF, SpawnerMain::GameConfigs::AssignHouses); // ScenarioClass::Read_INI

		Patch::Apply_LJMP(0x5D74A0, 0x5D7570); // MPGameModeClass_AllyTeams
		Patch::Apply_LJMP(0x501721, 0x501736); // HouseClass_ComputerParanoid
		Patch::Apply_LJMP(0x686A9E, 0x686AC6); // RemoveAIPlayers
	}

	{ // NetHack
		Patch::Apply_CALL(0x7B3D75, NetHack::SendTo);   // UDPInterfaceClass::Message_Handler
		Patch::Apply_CALL(0x7B3EEC, NetHack::RecvFrom); // UDPInterfaceClass::Message_Handler
	}

	{ // Skip Intro, EA_WWLOGO and LoadScreen
		Patch::Apply_LJMP(0x52CB50, 0x52CB6E); // InitIntro_Skip
	}

	{ // Cooperative
		Patch::Apply_LJMP(0x553321, 0x5533C5); // LoadProgressMgr_Draw_CooperativeDescription
		Patch::Apply_LJMP(0x55D0DF, 0x55D0E8); // AuxLoop_Cooperative_EndgameCrashFix
	}

	// Set ConnTimeout
	Patch::Apply_TYPED<int>(0x6843C7, { SpawnerMain::GameConfigs::m_Ptr.ConnTimeout }); //  Scenario_Load_Wait

	// Show GameMode in DiplomacyDialog in Skirmish
	Patch::Apply_LJMP(0x658117, 0x658126); // RadarClass_DiplomacyDialog

	// Leaves bottom bar closed for losing players during last game frames
	Patch::Apply_LJMP(0x6D1639, 0x6D1640); // TabClass_6D1610
}

bool SpawnerMain::GameConfigs::StartGame() {

	if (SpawnerMain::Configs::Active)
		return 0;

	SpawnerMain::Configs::Active = true;
	Game::IsActive() = true;

	Game::InitUIStuff();

	char* pScenarioName = m_Ptr.ScenarioName;

	if (strstr(pScenarioName, "RA2->"))
		pScenarioName += sizeof("RA2->") - 1;

	if (strstr(pScenarioName, "PlayMovies->"))
	{
		pScenarioName += sizeof("PlayMovies->") - 1;
		char* context = nullptr;
		char* movieName = strtok_s(pScenarioName, Phobos::readDelims, &context);
		for (; movieName; movieName = strtok_s(nullptr, Phobos::readDelims, &context))
			Game::PlayMovie(movieName);

		return false;
	}


	SpawnerMain::GameConfigs::LoadSidesStuff();

	bool result = StartScenario(pScenarioName);

	if (SpawnerMain::GetMainConfigs()->DumpTypes)
		DumpTypeDataArrayToFile::Dump();

	WWMouseClass::PrepareScreen();

	return result;
}

void SpawnerMain::GameConfigs::AssignHouses() {
	ScenarioClass::AssignHouses();

	const int count = MinImpl(HouseClass::Array->Count, (int)std::size(SpawnerMain::GameConfigs::m_Ptr.Houses));
	for (int indexOfHouseArray = 0; indexOfHouseArray < count; indexOfHouseArray++)
	{
		const auto pHouse = HouseClass::Array->GetItem(indexOfHouseArray);

		if (pHouse->Type->MultiplayPassive)
			continue;

		const auto pHousesConfig = &SpawnerMain::GameConfigs::m_Ptr.Houses[indexOfHouseArray];
		const int nSpawnLocations = pHousesConfig->SpawnLocations;
		const bool isObserver = pHouse->IsHumanPlayer && (
			pHousesConfig->IsObserver
			|| nSpawnLocations == -1
			|| nSpawnLocations == 90
			);

		// Set Alliances
		for (char i = 0; i < (char)std::size(pHousesConfig->Alliances); ++i)
		{
			const int alliesIndex = pHousesConfig->Alliances[i];
			if (alliesIndex != -1)
				pHouse->Allies.Add(alliesIndex);
		}

		// Set AI UIName
		if (SpawnerMain::GameConfigs::m_Ptr.AINamesByDifficulty && !pHouse->IsHumanPlayer)
		{
			const auto pAIConfig = &SpawnerMain::GameConfigs::m_Ptr.Players[indexOfHouseArray];

			switch (pAIConfig->Difficulty)
			{
			case 0:
				wcscpy_s(pHouse->UIName, StringTable::FetchString(GameStrings::GUI_AIHard));
				break;

			case 1:
				wcscpy_s(pHouse->UIName, StringTable::FetchString(GameStrings::GUI_AINormal));
				break;

			case 2:
				wcscpy_s(pHouse->UIName, StringTable::FetchString(GameStrings::GUI_AIEasy));
				break;
			default:
				break;
			}
		}

		// Set SpawnLocations
		if (!isObserver)
		{
			pHouse->StartingPoint = (nSpawnLocations != -2)
				? std::clamp(nSpawnLocations, 0, 7)
				: nSpawnLocations;
		}
		else
		{
			if (pHouse->MakeObserver())
				TabClass::Instance->ThumbActive = false;

			{ // Remove SpawnLocations for Observer
				ScenarioClass* pScenarioClass = ScenarioClass::Instance;
				for (char i = 0; i < (char)std::size(pScenarioClass->HouseIndices); ++i)
				{
					if (pHouse->ArrayIndex == pScenarioClass->HouseIndices[i])
						pScenarioClass->HouseIndices[i] = -1;
				}

				pHouse->StartingPoint = -1;
			}
		}
	}
}

bool SpawnerMain::GameConfigs::Reconcile_Players() {
	int i {};
	bool found {};
	int house {};
	HouseClass* pHouse = nullptr;

	// Just use this as Playernodes.
	auto& players = SessionClass::Instance->StartSpots;

	if (players.Count == 0)
		return true;

	for (i = 0; i < players.Count; i++)
	{
		found = false;

		for (house = 0; house < players.Count; house++)
		{
			pHouse = HouseClass::Array->Items[house];
			if (!pHouse)
				continue;

			//for (wchar_t c : players.Items[i]->Name)
			//	Debug::LogAndMessage("%c", (char)c);

			//Debug::LogAndMessage("\n");

			//for (wchar_t c : pHouse->UIName)
			//	Debug::LogAndMessage("%c", (char)c);

			//Debug::LogAndMessage("\n");

			if (!wcscmp(players.Items[i]->Name, pHouse->UIName))
			{
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}

	for (house = 0; house < players.Count; house++)
	{
		pHouse = HouseClass::Array->Items[house];

		if (!pHouse)
			continue;

		if (!pHouse->IsHumanPlayer)
			continue;

		found = false;
		for (i = 0; i < players.Count; i++)
		{
			if (!wcscmp(players.Items[i]->Name, pHouse->UIName))
			{
				found = true;
				players.Items[i]->HouseIndex = house;
				break;
			}
		}

		if (!found)
		{
			/**
			 *  Turn the player's house over to the computer's AI
			 */
			pHouse->IsHumanPlayer = false;
			pHouse->Production = true;
			pHouse->StaticData.IQLevel = RulesClass::Instance->MaxIQLevels;

			static wchar_t buffer[21];
			std::swprintf(buffer, sizeof(buffer), L"%s (AI)", pHouse->UIName);
			std::wcscpy(pHouse->UIName, buffer);
			SessionClass::Instance->MPlayerCount--;
		}
	}

	return SessionClass::Instance->MPlayerCount == players.Count;
}

void SpawnerMain::GameConfigs::RespondToSaveGame(EventClass* event) {
	SpawnerMain::Configs::DoSave = true;
}

void Print_Saving_Game_Message2() {
	const int message_delay = (int)(RulesClass::Instance->MessageDelay * 900);
	MessageListClass::Instance->AddMessage(nullptr, 0, L"Saving game...", 4, TextPrintType::Point6Grad | TextPrintType::UseGradPal | TextPrintType::FullShadow, message_delay, false);
	MapClass::Instance->MarkNeedsRedraw(2);
	MapClass::Instance->Render();
}

void SpawnerMain::GameConfigs::After_Main_Loop() {
	auto pConfig = &GameConfigs::m_Ptr;

	const bool doSaveCampaign = SessionClass::Instance->GameMode == GameMode::Campaign && pConfig->AutoSaveCount > 0 && pConfig->AutoSaveInterval > 0;
	const bool doSaveMP = SpawnerMain::Configs::Active && SessionClass::Instance->GameMode == GameMode::LAN && pConfig->AutoSaveInterval > 0;

	if (doSaveCampaign || doSaveMP)
	{
		if (Unsorted::CurrentFrame == SpawnerMain::Configs::NextAutoSaveFrame)
		{
			SpawnerMain::Configs::DoSave = true;
		}
	}

	if (SpawnerMain::Configs::DoSave)
	{

		Print_Saving_Game_Message2();

		if (SessionClass::Instance->GameMode == GameMode::Campaign)
		{
			const std::string saveFileName = std::format("AUTOSAVE{}.SAV" , SpawnerMain::Configs::NextAutoSaveNumber + 1);
			const std::wstring saveDescription = std::format(L"Mission Auto-Save (Slot {})", SpawnerMain::Configs::NextAutoSaveNumber + 1);

			ScenarioClass::PauseGame();
			Game::CallBack();

			ScenarioClass::Instance->SaveGame(saveFileName.c_str(), saveDescription.c_str());
			ScenarioClass::ResumeGame();
			SpawnerMain::Configs::NextAutoSaveNumber = (SpawnerMain::Configs::NextAutoSaveNumber + 1) % pConfig->AutoSaveCount;

			SpawnerMain::Configs::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}
		else if (SessionClass::Instance->GameMode == GameMode::LAN)
		{

			ScenarioClass::Instance->SaveGame("SAVEGAME.NET", L"Multiplayer Game");
			SpawnerMain::Configs::NextAutoSaveFrame = Unsorted::CurrentFrame + pConfig->AutoSaveInterval;
		}

		SpawnerMain::Configs::DoSave = false;
	}
}

bool SpawnerMain::GameConfigs::StartScenario(const char* pScenarioName) {
	if (pScenarioName[0] == 0 && !SpawnerMain::GameConfigs::m_Ptr.LoadSaveGame)
	{
		Debug::LogInfo("[Spawner] Failed Read Scenario [{}]", pScenarioName);

		MessageBox::Show(
			StringTable::FetchString(GameStrings::TXT_UNABLE_READ_SCENARIO),
			StringTable::FetchString(GameStrings::TXT_OK),
			0);

		return false;
	}

	const auto pSession = &SessionClass::Instance;
	const auto pGameModeOptions = &GameModeOptionsClass::Instance;

	strcpy_s(&Game::ScenarioName, 0x200, pScenarioName);
	pSession->ReadScenarioDescriptions();

	{ // Set MPGameMode
		pSession->MPGameMode = MPGameModeClass::Get(SpawnerMain::GameConfigs::m_Ptr.MPModeIndex);
		if (!pSession->MPGameMode)
			pSession->MPGameMode = MPGameModeClass::Get(1);
	}

	{ // Set Options
		pGameModeOptions->MPModeIndex = SpawnerMain::GameConfigs::m_Ptr.MPModeIndex;
		// pGameModeOptions->ScenarioIndex
		pGameModeOptions->Bases = SpawnerMain::GameConfigs::m_Ptr.Bases;
		pGameModeOptions->Money = SpawnerMain::GameConfigs::m_Ptr.Credits;
		pGameModeOptions->BridgeDestruction = SpawnerMain::GameConfigs::m_Ptr.BridgeDestroy;
		pGameModeOptions->Crates = SpawnerMain::GameConfigs::m_Ptr.Crates;
		pGameModeOptions->ShortGame = SpawnerMain::GameConfigs::m_Ptr.ShortGame;
		pGameModeOptions->SWAllowed = SpawnerMain::GameConfigs::m_Ptr.SuperWeapons;
		pGameModeOptions->BuildOffAlly = SpawnerMain::GameConfigs::m_Ptr.BuildOffAlly;
		pGameModeOptions->GameSpeed = SpawnerMain::GameConfigs::m_Ptr.GameSpeed;
		pGameModeOptions->MultiEngineer = SpawnerMain::GameConfigs::m_Ptr.MultiEngineer;
		pGameModeOptions->UnitCount = SpawnerMain::GameConfigs::m_Ptr.UnitCount;
		pGameModeOptions->AIPlayers = SpawnerMain::GameConfigs::m_Ptr.AIPlayers;
		pGameModeOptions->AIDifficulty = SpawnerMain::GameConfigs::m_Ptr.AIDifficulty;
		// pGameModeOptions->AISlots
		pGameModeOptions->AlliesAllowed = SpawnerMain::GameConfigs::m_Ptr.AlliesAllowed;
		pGameModeOptions->HarvesterTruce = SpawnerMain::GameConfigs::m_Ptr.HarvesterTruce;
		// pGameModeOptions->CaptureTheFlag
		pGameModeOptions->FogOfWar = SpawnerMain::GameConfigs::m_Ptr.FogOfWar;
		pGameModeOptions->MCVRedeploy = SpawnerMain::GameConfigs::m_Ptr.MCVRedeploy;
		wcscpy(pGameModeOptions->MapDescription, SpawnerMain::GameConfigs::m_Ptr.UIMapName);

		Game::Seed = SpawnerMain::GameConfigs::m_Ptr.Seed;
		Game::TechLevel = SpawnerMain::GameConfigs::m_Ptr.TechLevel;
		Game::PlayerColor = SpawnerMain::GameConfigs::m_Ptr.Players[0].Color;
		GameOptionsClass::Instance->GameSpeed = SpawnerMain::GameConfigs::m_Ptr.GameSpeed;
		SpawnerMain::Configs::NextAutoSaveNumber = SpawnerMain::GameConfigs::m_Ptr.NextAutoSaveNumber;
	}

	{ // Added AI Players
		const auto pAISlots = &pGameModeOptions->AISlots;
		for (char slotIndex = 0; slotIndex < (char)std::size(pAISlots->Allies); ++slotIndex)
		{
			const auto pPlayerConfig = &SpawnerMain::GameConfigs::m_Ptr.Players[slotIndex];
			if (pPlayerConfig->IsHuman)
				continue;

			pAISlots->Difficulties[slotIndex] = pPlayerConfig->Difficulty;
			pAISlots->Countries[slotIndex] = pPlayerConfig->Country;
			pAISlots->Colors[slotIndex] = pPlayerConfig->Color;
			pAISlots->Allies[slotIndex] = -1;
		}
	}

	{ // Added Human Players
		NetHack::PortHack = true;
		const char maxPlayers = SpawnerMain::GameConfigs::m_Ptr.IsCampaign ? 1 : (char)std::size(SpawnerMain::GameConfigs::m_Ptr.Players);
		for (char playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
		{
			const auto pPlayer = &SpawnerMain::GameConfigs::m_Ptr.Players[playerIndex];
			if (!pPlayer->IsHuman)
				continue;

			const auto pNode = GameCreate<NodeNameType>();
			NodeNameType::Array->AddItem(pNode);

			wcscpy_s(pNode->Name, pPlayer->Name);
			pNode->Country = pPlayer->Country;
			pNode->Color = pPlayer->Color;
			pNode->Time = -1;

			if (pPlayer->IsObserver && !SpawnerMain::GameConfigs::m_Ptr.IsCampaign)
			{
				if (pNode->Country < 0)
					pNode->Country = -3;

				pNode->SpectatorFlag = 0xFFFFFFFF;

				if (playerIndex == 0)
					Game::ObserverMode = true;
			}

			if (playerIndex > 0)
			{
				pNode->Address.sin_addr.s_addr = playerIndex;

				const auto Ip = inet_addr(pPlayer->Ip);
				const auto Port = htons((u_short)pPlayer->Port);
				ListAddress::Array[playerIndex - 1].Ip = Ip;
				ListAddress::Array[playerIndex - 1].Port = Port;
				if (Port != (u_short)SpawnerMain::GameConfigs::m_Ptr.ListenPort)
					NetHack::PortHack = false;
			}
		}

		Game::PlayerCount = NodeNameType::Array->Count;
	}

	{ // Set SessionType
		if (SpawnerMain::GameConfigs::m_Ptr.IsCampaign)
			pSession->GameMode = GameMode::Campaign;
		else if (Game::PlayerCount > 1)
			pSession->GameMode = GameMode::Internet; // HACK: will be set to LAN later
		else
			pSession->GameMode = GameMode::Skirmish;
	}

	Game::InitRandom();

	// StartScenario
	if (SessionClass::IsCampaign())
	{
		pGameModeOptions->Crates = true;

#ifdef incomplete_ExtraSavedGamesData
		// Rename MISSIONMD.INI to this
		// because Ares has LoadScreenText.Color and Phobos has Starkku's PR #1145
		if (SpawnerMain::GetGameConfigs()->CustomMissionID) // before parsing
		{
			Patch::Apply_RAW(0x839724, "Spawn.ini");
			Patch::Apply_RAW(0x839734, { 00, 00, 00 }); //pad it bruh
		}

		bool result = ScenarioClass::StartScenario(pScenarioName, 1, 0);

		if (SpawnerMain::GetGameConfigs()->CustomMissionID) // after parsing
			ScenarioClass::Instance->EndOfGame = true;

		return result;
#else
		//char keee_[46];
		//sprintf(keee_, "%sSav", ScenarioClass::Instance->UIName);
		//wcsncpy(ScenarioClass::Instance->UINameLoaded, StringTable::FetchString(keee_), 0x2Du);
  //      if ( !wcsncmp(ScenarioClass::Instance->UINameLoaded, L"MISSING:", 8u) ) {
  //          wcsncpy(ScenarioClass::Instance->UINameLoaded, ScenarioClass::Instance->Name, 0x2Du);
  //      }
  //     ScenarioClass::Instance->UINameLoaded[44] = 0;

		//Debug::LogInfo("Loading Scenario Name [%s]" , wstring_to_utf8(ScenarioClass::Instance->UINameLoaded).c_str());
		return SpawnerMain::GameConfigs::m_Ptr.LoadSaveGame ? SpawnerMain::GameConfigs::LoadSavedGame(SpawnerMain::GameConfigs::m_Ptr.SaveGameName)
			: ScenarioClass::StartScenario(pScenarioName, 1, 0);
#endif
	}
	else if (SessionClass::IsSkirmish())
	{
		return SpawnerMain::GameConfigs::m_Ptr.LoadSaveGame ? SpawnerMain::GameConfigs::LoadSavedGame(SpawnerMain::GameConfigs::m_Ptr.SaveGameName)
			: ScenarioClass::StartScenario(pScenarioName, 0, -1);
	}
	else
	{
		SpawnerMain::GameConfigs::InitNetwork();
		const bool result = SpawnerMain::GameConfigs::m_Ptr.LoadSaveGame ? SpawnerMain::GameConfigs::LoadSavedGame(SpawnerMain::GameConfigs::m_Ptr.SaveGameName)
			: ScenarioClass::StartScenario(pScenarioName, 0, -1);

		if (!result)
			return false;

		pSession->GameMode = GameMode::LAN;
		if (SpawnerMain::GameConfigs::m_Ptr.LoadSaveGame && !SpawnerMain::GameConfigs::Reconcile_Players())
			return false;

		if (!pSession->CreateConnections())
			return false;

		if (!SpawnerMain::GetMainConfigs()->AllowChat)
		{
			Game::ChatMask[0] = false;
			Game::ChatMask[1] = false;
			Game::ChatMask[2] = false;
			Game::ChatMask[3] = false;
			Game::ChatMask[4] = false;
			Game::ChatMask[5] = false;
			Game::ChatMask[6] = false;
			Game::ChatMask[7] = false;
		}

		return true;
	}
}

//#pragma optimize("", on )

bool SpawnerMain::GameConfigs::LoadSavedGame(const char* saveGameName) {

	if (!saveGameName[0] || !StaticLoadOptionsClass::LoadMission(saveGameName))
	{
		Debug::LogInfo("[Spawner] Failed Load Game [{}]", saveGameName);

		MessageBox::Show(
			StringTable::FetchString(GameStrings::TXT_ERROR_LOADING_GAME),
			StringTable::FetchString(GameStrings::TXT_OK),
			0);

		return false;
	}

	return true;
}

void SpawnerMain::GameConfigs::InitNetwork() {
	Tunnel::Id = htons((u_short)SpawnerMain::GameConfigs::m_Ptr.TunnelId);
	Tunnel::Ip = inet_addr(SpawnerMain::GameConfigs::m_Ptr.TunnelIp);
	Tunnel::Port = htons((u_short)SpawnerMain::GameConfigs::m_Ptr.TunnelPort);

	Game::PlanetWestwoodPortNumber = Tunnel::Port ? u_short() : (u_short)SpawnerMain::GameConfigs::m_Ptr.ListenPort;

	UDPInterfaceClass::Instance = GameCreate<UDPInterfaceClass>();
	UDPInterfaceClass::Instance->Init();
	UDPInterfaceClass::Instance->OpenSocket();
	UDPInterfaceClass::Instance->StartListening();
	UDPInterfaceClass::Instance->DiscardInBuffers();
	UDPInterfaceClass::Instance->DiscardOutBuffers();
	IPXManagerClass::Instance->SetTiming(60, -1, 600, 1);

	Game::Network::PlanetWestwoodStartTime = time(NULL);
	Game::Network::GameStockKeepingUnit = 0x2901;

	EventExt::ProtocolZero::Enable = (SpawnerMain::GameConfigs::m_Ptr.Protocol == 0);
	if (EventExt::ProtocolZero::Enable)
	{
		Game::Network::FrameSendRate = 2;
		Game::Network::PreCalcMaxAhead = SpawnerMain::GameConfigs::m_Ptr.PreCalcMaxAhead;
		EventExt::ProtocolZero::MaxLatencyLevel = std::clamp(
			SpawnerMain::GameConfigs::m_Ptr.MaxLatencyLevel,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_1,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_MAX
		);
	}
	else
	{
		Game::Network::FrameSendRate = SpawnerMain::GameConfigs::m_Ptr.FrameSendRate;
	}

	Game::Network::MaxAhead = SpawnerMain::GameConfigs::m_Ptr.MaxAhead == -1
		? Game::Network::FrameSendRate * 6
		: SpawnerMain::GameConfigs::m_Ptr.MaxAhead;

	Game::Network::MaxMaxAhead = 0;
	Game::Network::ProtocolVersion = 2;
	Game::Network::LatencyFudge = 0;
	Game::Network::RequestedFPS = 60;
	Game::Network::Tournament = SpawnerMain::GameConfigs::m_Ptr.Tournament;
	Game::Network::WOLGameID = SpawnerMain::GameConfigs::m_Ptr.WOLGameID;
	Game::Network::ReconnectTimeout = SpawnerMain::GameConfigs::m_Ptr.ReconnectTimeout;

	if (SpawnerMain::GameConfigs::m_Ptr.QuickMatch)
	{
		Game::EnableMPDebug     = false;
		Game::DrawMPDebugStats  = false;
		Game::EnableMPSyncDebug = false;
	}

	Game::Network::Init();
}

void SpawnerMain::GameConfigs::LoadSidesStuff()
{
	RulesClass* pRules = RulesClass::Instance;
	CCINIClass* pINI = CCINIClass::INI_Rules;

	pRules->Read_Countries(pINI);
	pRules->Read_Sides(pINI);

	for (auto const& pItem : *HouseTypeClass::Array)
		pItem->LoadFromINI(pINI);
}

ASMJIT_PATCH(0x6BD7CB, WinMain_SpawnerInit, 0x5) {
	SpawnerMain::GameConfigs::Init();
	return 0x0;
}

// Display UIGameMode if is set
// Otherwise use mode name from MPModesMD.ini
ASMJIT_PATCH(0x65812E, RadarClass_DiplomacyDialog_UIGameMode, 0x6) {
	enum { Show = 0x65813E, DontShow = 0x65814D };

	if (SpawnerMain::Configs::Enabled && SpawnerMain::GameConfigs::m_Ptr.UIGameMode[0])
	{
		R->EBX(R->EAX());
		R->EAX(SpawnerMain::GameConfigs::m_Ptr.UIGameMode);
		return Show;
	}

	if (!SessionClass::Instance->MPGameMode)
		return DontShow;

	return 0;
}


// ASMJIT_PATCH(0x689669, ScenarioClass_Load_Suffix_Spawner, 0x6) {
// 	if (SpawnerMain::Configs::Enabled)
// 		SpawnerMain::GameConfigs::m_Ptr.UIGameMode[0] = 0;
//
// 	return 0;
// }

#pragma region MPlayerDefeated
namespace MPlayerDefeated
{
	HouseClass* pThis = nullptr;
}

ASMJIT_PATCH(0x4FC0B6, HouseClass_MPlayerDefeated_SaveArgument, 0x5) {
	MPlayerDefeated::pThis = (SpawnerMain::Configs::Enabled && !SessionClass::IsCampaign())
		? R->ECX<HouseClass*>()
		: nullptr;

	return 0;
}

// Skip match-end logic if MPlayerDefeated called for observer
ASMJIT_PATCH(0x4FC262, HouseClass_MPlayerDefeated_SkipObserver, 0x6) {
	enum { ProcEpilogue = 0x4FC6BC };

	if (!MPlayerDefeated::pThis)
		return 0;

	return MPlayerDefeated::pThis->IsObserver()
		? ProcEpilogue
		: 0;
}ASMJIT_PATCH_AGAIN(0x4FC332, HouseClass_MPlayerDefeated_SkipObserver, 0x5)


ASMJIT_PATCH(0x4FC551, HouseClass_MPlayerDefeated_NoEnemies, 0x5) {
	enum { ProcEpilogue = 0x4FC6BC };

	if (!MPlayerDefeated::pThis)
		return 0;

	for (const auto pHouse : *HouseClass::Array)
	{
		if (pHouse->Defeated || pHouse == MPlayerDefeated::pThis || pHouse->Type->MultiplayPassive)
			continue;

		if ((pHouse->IsHumanPlayer || SpawnerMain::GameConfigs::m_Ptr.ContinueWithoutHumans)
			&& HouseExtData::IsMutualAllies(pHouse , MPlayerDefeated::pThis))
		{
			Debug::LogInfo("[Spawner] MPlayer_Defeated() - Defeated player has a living ally");
			if (SpawnerMain::GameConfigs::m_Ptr.DefeatedBecomesObserver)
				MPlayerDefeated::pThis->MakeObserver();

			return ProcEpilogue;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x4FC57C, HouseClass_MPlayerDefeated_CheckAliveAndHumans, 0x7) {
	enum { ProcEpilogue = 0x4FC6BC, FinishMatch = 0x4FC591 };

	if (!MPlayerDefeated::pThis)
		return 0;

	GET_STACK(int, numHumans, STACK_OFFSET(0xC0, -0xA8));
	GET_STACK(int, numAlive, STACK_OFFSET(0xC0, -0xAC));

	bool continueWithoutHumans = SpawnerMain::GameConfigs::m_Ptr.ContinueWithoutHumans ||
		(SessionClass::IsSkirmish() && HouseClass::CurrentPlayer->IsInitiallyObserver());

	if (numAlive > 1 && (numHumans != 0 || continueWithoutHumans))
	{
		if (SpawnerMain::GameConfigs::m_Ptr.DefeatedBecomesObserver)
			MPlayerDefeated::pThis->MakeObserver();

		return ProcEpilogue;
	}

	return FinishMatch;
}

#pragma endregion MPlayerDefeated

static bool MainLoop_replaceA()
{
	// Main loop.
	bool result = Game::MainLoop();
	if (!result){
		SpawnerMain::GameConfigs::After_Main_Loop();
	}
	return result;
}

static bool MainLoop_replaceB()
{
	// Main loop.
	bool result = Game::MainLoop();

	if (result)
	{
		// Run the MainLoop (sadly not enough space to hook after it),
		// then decide if we should exit multithread mode.
		if (Phobos::Config::MultiThreadSinglePlayer)
			Multithreading::ExitMultithreadMode();
	}
	else
	{
		SpawnerMain::GameConfigs::After_Main_Loop();
	}
	return result;
}

DEFINE_FUNCTION_JUMP(CALL, 0x624271, MainLoop_replaceA);
DEFINE_FUNCTION_JUMP(CALL, 0x623D72, MainLoop_replaceA);
DEFINE_FUNCTION_JUMP(CALL, 0x62314E, MainLoop_replaceA);
DEFINE_FUNCTION_JUMP(CALL, 0x60D407, MainLoop_replaceA);
DEFINE_FUNCTION_JUMP(CALL, 0x608206, MainLoop_replaceA);
DEFINE_FUNCTION_JUMP(CALL, 0x48CE8A, MainLoop_replaceB);

ASMJIT_PATCH(0x52DAED, Game_Start_ResetGlobal, 0x7)
{
	SpawnerMain::Configs::DoSave = false;
	SpawnerMain::Configs::NextAutoSaveFrame = -1;
	SpawnerMain::Configs::NextAutoSaveNumber = 0;
	return 0;
}

ASMJIT_PATCH(0x686B20, INIClass_ReadScenario_AutoSave, 0x6)
{
	/**
	 *  Schedule the next autosave.
	 */
	SpawnerMain::Configs::NextAutoSaveFrame = Unsorted::CurrentFrame;
	SpawnerMain::Configs::NextAutoSaveFrame += SpawnerMain::GameConfigs::m_Ptr.AutoSaveInterval;
	return 0;
}

ASMJIT_PATCH(0x4C7A14, EventClass_RespondToEvent_SaveGame, 0x5)
{
	SpawnerMain::Configs::DoSave = true;
	return 0x4C7B42;
}

#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>

// Allow allies to repair on service depot
ASMJIT_PATCH(0x700594, TechnoClass_WhatAction_AllowAlliesRepair, 0x5)
{
	enum { Allow = 0x70059D, DisAllow = 0x7005E6 };

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pObject, EDI);

	if(auto const pBuilding = cast_to<FakeBuildingClass* const>(pObject)){
		if (pBuilding->_GetTypeExtData()->AllowAlliesRepair) { 
			return (pBuilding->Owner->IsAlliedWith(pThis))
				? Allow
				: DisAllow;
		}
	}

	return pThis->Owner != pObject->GetOwningHouse() ? DisAllow : Allow;
}

// Allow Dogs & Flak Troopers to get a speed boost
// Skip Crawls check on InfantryType
// https://github.com/CnCNet/yr-patches/issues/15
// ASMJIT_PATCH(0x51D77A, InfantryClass_DoAction_AllowReceiveSpeedBoost, 5) {
// 	enum { Allow = 0x51D793, DisAllow = 0x51DADA };
//
// 	GET(FakeInfantryClass*, pThis, ESI);
// 	GET(DoType, sequence, EDI);
//
// 	if (pThis->_GetTypeExtData()->AllowReceiveSpeedBoost)
// 		return Allow;
//
// 	return sequence == DoType::Down && !pThis->Crawling ? DisAllow : Allow;
// }