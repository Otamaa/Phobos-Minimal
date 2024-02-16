#include "Main.h"

#include <Phobos.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>

#include <LoadOptionsClass.h>
#include <UDPInterfaceClass.h>
#include <IPXManagerClass.h>

#include <Misc/Ares/Hooks/Header.h>

#include "DumpTypeDataArrayToFile.h"
#include "NetHack.h"
#include "ProtocolZero/Body.h"

std::unique_ptr<SpawnerMain::GameConfigs> SpawnerMain::GameConfigs::Config = nullptr;
UniqueGamePtrB<MixFileClass> SpawnerMain::MixFile {};

bool SpawnerMain::Configs::Enabled = false;
bool SpawnerMain::Configs::Active = false;

bool SpawnerMain::Configs::DumpTypes = false;

bool SpawnerMain::Configs::MPDebug = false;
bool SpawnerMain::Configs::SingleProcAffinity = true;

bool SpawnerMain::Configs::DisableEdgeScrolling = false;
bool SpawnerMain::Configs::QuickExit = true;
bool SpawnerMain::Configs::SkipScoreScreen = false;
bool SpawnerMain::Configs::DDrawHandlesClose = false;
bool SpawnerMain::Configs::SpeedControl = false;

bool SpawnerMain::Configs::WindowedMode = false;
bool SpawnerMain::Configs::NoWindowFrame = false;
int SpawnerMain::Configs::DDrawTargetFPS = -1;

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

void SpawnerMain::ExeRun() {
}
#ifdef IS_ANTICHEAT_VER
#define SPAWNER_PRODUCT_NAME "YR-Spawner (AntiCheat)"
#else
#define SPAWNER_PRODUCT_NAME "YR-Spawner"
#endif

#define SPAWNER_FILE_DESCRIPTION "CnCNet5: Spawner"

void SpawnerMain::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; ++i)
	{
		const char* pArg = ppArgs[i];

		if (0 == _stricmp(pArg, "-SPAWN"))
		{
			Phobos::Otamaa::NoCD = true;
			SpawnerMain::Configs::Enabled = true;
		}
		else if (0 == _stricmp(pArg, "-DumpTypes"))
		{
			SpawnerMain::Configs::DumpTypes = true;
		}
	}
}

void SpawnerMain::PrintInitializeLog() {
	Debug::Log("Initialized " SPAWNER_PRODUCT_NAME "\n");
}

void SpawnerMain::LoadConfigurations()
{
	if (auto pINI = &CCINIClass::INI_RA2MD) {

		if (pINI->GetSection(GameStrings::Options())) {
			SpawnerMain::Configs::MPDebug = pINI->ReadBool(GameStrings::Options(), "MPDEBUG", SpawnerMain::Configs::MPDebug);
			SpawnerMain::Configs::SingleProcAffinity = pINI->ReadBool(GameStrings::Options(), "SingleProcAffinity", SpawnerMain::Configs::SingleProcAffinity);
			SpawnerMain::Configs::DisableEdgeScrolling = pINI->ReadBool(GameStrings::Options(), "DisableEdgeScrolling", SpawnerMain::Configs::DisableEdgeScrolling);
			SpawnerMain::Configs::QuickExit = pINI->ReadBool(GameStrings::Options(), "QuickExit", SpawnerMain::Configs::QuickExit);
			SpawnerMain::Configs::SkipScoreScreen = pINI->ReadBool(GameStrings::Options(), "SkipScoreScreen", SpawnerMain::Configs::SkipScoreScreen);
			SpawnerMain::Configs::DDrawHandlesClose = pINI->ReadBool(GameStrings::Options(), "DDrawHandlesClose", SpawnerMain::Configs::DDrawHandlesClose);
			SpawnerMain::Configs::SpeedControl = pINI->ReadBool(GameStrings::Options(), "SpeedControl", SpawnerMain::Configs::SpeedControl);
		}

		if (pINI->GetSection(GameStrings::Video())) {
			SpawnerMain::Configs::WindowedMode = pINI->ReadBool(GameStrings::Video(), "Video.Windowed", SpawnerMain::Configs::WindowedMode);
			SpawnerMain::Configs::NoWindowFrame = pINI->ReadBool(GameStrings::Video(), "NoWindowFrame", SpawnerMain::Configs::NoWindowFrame);
			SpawnerMain::Configs::DDrawTargetFPS = pINI->ReadInteger(GameStrings::Video(), "DDrawTargetFPS", SpawnerMain::Configs::DDrawTargetFPS);
		}
	}
}

void SpawnerMain::ApplyStaticOptions()
{
	if (SpawnerMain::Configs::MPDebug)
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

	if (SpawnerMain::Configs::SingleProcAffinity)
	{
		auto const process = GetCurrentProcess();
		DWORD_PTR const processAffinityMask = 1;
		SetProcessAffinityMask(process, processAffinityMask);
	}

	if (SpawnerMain::Configs::WindowedMode)
	{
		GameOptionsClass::WindowedMode = true;

		if (SpawnerMain::Configs::NoWindowFrame) {
			Patch::Apply_RAW(0x777CC0, // CreateMainWindow
			{
				0x68, 0x00, 0x00, 0x0A, 0x86 // push    0x860A0000; vs 0x02CA0000
			});
		}
	}

	if (SpawnerMain::Configs::SpeedControl)
	{
		auto& speedControl = *reinterpret_cast<bool*>(0xA8EDDCu);
		speedControl = true;
	}

	// Set 3rd party ddraw.dll options
	for (auto& dllData : Patch::ModuleDatas) {
		if (IS_SAME_STR_(dllData.ModuleName.c_str(), "ddraw.dll")) {
			if (bool* gameHandlesClose = (bool*)GetProcAddress(dllData.Handle, "GameHandlesClose"))
				*gameHandlesClose = !SpawnerMain::Configs::DDrawHandlesClose;

			LPDWORD TargetFPS = (LPDWORD)GetProcAddress(dllData.Handle, "TargetFPS");
			if (TargetFPS && SpawnerMain::Configs::DDrawTargetFPS != -1)
				*TargetFPS = SpawnerMain::Configs::DDrawTargetFPS;

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


constexpr char* PlayerSectionArray[8] = {
	"Settings",
	"Other1",
	"Other2",
	"Other3",
	"Other4",
	"Other5",
	"Other6",
	"Other7"
};

constexpr char* MultiTagArray[8] = {
	"Multi1",
	"Multi2",
	"Multi3",
	"Multi4",
	"Multi5",
	"Multi6",
	"Multi7",
	"Multi8"
};

constexpr char* AlliancesSectionArray[8] = {
	"Multi1_Alliances",
	"Multi2_Alliances",
	"Multi3_Alliances",
	"Multi4_Alliances",
	"Multi5_Alliances",
	"Multi6_Alliances",
	"Multi7_Alliances",
	"Multi8_Alliances"
};

constexpr char* AlliancesTagArray[8] = {
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

void SpawnerMain::GameConfigs::LoadFromINIFile(CCINIClass* pINI)
{
	if (!pINI || !pINI->GetSection(GameStrings::Settings()))
		return;

	{ // Game Mode Options
		MPModeIndex = pINI->ReadInteger(GameStrings::Settings(), "GameMode", MPModeIndex);
		Bases = pINI->ReadBool(GameStrings::Settings(), "Bases", Bases);
		Credits = pINI->ReadInteger(GameStrings::Settings(), "Credits", Credits);
		BridgeDestroy = pINI->ReadBool(GameStrings::Settings(), "BridgeDestroy", BridgeDestroy);
		Crates = pINI->ReadBool(GameStrings::Settings(), "Crates", Crates);
		ShortGame = pINI->ReadBool(GameStrings::Settings(), "ShortGame", ShortGame);
		SuperWeapons = pINI->ReadBool(GameStrings::Settings(), "Superweapons", SuperWeapons);
		BuildOffAlly = pINI->ReadBool(GameStrings::Settings(), "BuildOffAlly", BuildOffAlly);
		GameSpeed = pINI->ReadInteger(GameStrings::Settings(), "GameSpeed", GameSpeed);
		MultiEngineer = pINI->ReadBool(GameStrings::Settings(), "MultiEngineer", MultiEngineer);
		UnitCount = pINI->ReadInteger(GameStrings::Settings(), "UnitCount", UnitCount);
		AIPlayers = pINI->ReadInteger(GameStrings::Settings(), "AIPlayers", AIPlayers);
		AIDifficulty = pINI->ReadInteger(GameStrings::Settings(), "AIDifficulty", AIDifficulty);
		AlliesAllowed = pINI->ReadBool(GameStrings::Settings(), "AlliesAllowed", AlliesAllowed);
		HarvesterTruce = pINI->ReadBool(GameStrings::Settings(), "HarvesterTruce", HarvesterTruce);
		FogOfWar = pINI->ReadBool(GameStrings::Settings(), "FogOfWar", FogOfWar);
		MCVRedeploy = pINI->ReadBool(GameStrings::Settings(), "MCVRedeploy", MCVRedeploy);

		if (((CCINIClassDummy*)pINI)->ReadString_WithoutAresHook(GameStrings::Settings(), "UIGameMode", "", Phobos::readBuffer, Phobos::readLength) > 0)
			MultiByteToWideChar(CP_UTF8, 0, Phobos::readBuffer, strlen(Phobos::readBuffer), UIGameMode, std::size(UIGameMode));
	}

	// SaveGame Options
	LoadSaveGame = pINI->ReadBool(GameStrings::Settings(), "LoadSaveGame", LoadSaveGame);
	/* SaveGameName  pINI->ReadString(GameStrings::Settings(), "SaveGameName", SaveGameName, SaveGameName, sizeof(SaveGameName));*/

	{ // Scenario Options
		Seed = pINI->ReadInteger(GameStrings::Settings(), "Seed", Seed);
		TechLevel = pINI->ReadInteger(GameStrings::Settings(), "TechLevel", TechLevel);
		IsCampaign = pINI->ReadBool(GameStrings::Settings(), "IsSinglePlayer", IsCampaign);
		Tournament = pINI->ReadInteger(GameStrings::Settings(), "Tournament", Tournament);
		WOLGameID = pINI->ReadInteger(GameStrings::Settings(), "GameID", WOLGameID);
		/* ScenarioName  pINI->ReadString(pSettingsSection, "Scenario", ScenarioName, ScenarioName, sizeof(ScenarioName));*/
		/* MapHash       pINI->ReadString(pSettingsSection, "MapHash", MapHash, MapHash, sizeof(MapHash));*/

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
	QuickMatch = pINI->ReadBool(GameStrings::Settings(), "QuickMatch", QuickMatch);
	SkipScoreScreen = pINI->ReadBool(GameStrings::Settings(), "SkipScoreScreen", SkipScoreScreen);
	WriteStatistics = pINI->ReadBool(GameStrings::Settings(), "WriteStatistics", WriteStatistics);
	AINamesByDifficulty = pINI->ReadBool(GameStrings::Settings(), "AINamesByDifficulty", AINamesByDifficulty);
	ContinueWithoutHumans = pINI->ReadBool(GameStrings::Settings(), "ContinueWithoutHumans", ContinueWithoutHumans);
	DefeatedBecomesObserver = pINI->ReadBool(GameStrings::Settings(), "DefeatedBecomesObserver", DefeatedBecomesObserver);
	Observer_ShowAIOnSidebar = pINI->ReadBool(GameStrings::Settings(), "Observer.ShowAIOnSidebar", Observer_ShowAIOnSidebar);
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

	SpawnerMain::GameConfigs::Config = std::make_unique<SpawnerMain::GameConfigs>();

	CCFileClass file{ "SPAWN.INI" };

	if (!file.Exists()) {
		Debug::Log(" %s Failed to Open file %s for\n", __FUNCTION__, file.FileName);
		
	} else {

		if (!file.Open(FileAccessMode::ReadWrite)) {
			Debug::Log(" %s Failed to Open file %s for\n", __FUNCTION__, file.FileName);
		}

		CCINIClass ini{};
		ini.ReadCCFile(&file);
		SpawnerMain::GameConfigs::GetGameConfigs()->LoadFromINIFile(&ini);
	}

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
	Patch::Apply_TYPED<int>(0x6843C7, { SpawnerMain::GameConfigs::GetGameConfigs()->ConnTimeout }); //  Scenario_Load_Wait

	{ // Add support unicode player name in ingame chat
		Patch::Apply_RAW(0x48D930, { 0x8B, 0xC1, 0x90, 0x90, 0x90 }); // mov eax, ecx
		Patch::Apply_RAW(0x55F0AD, { 0x8B, 0xC1, 0x90, 0x90, 0x90 }); // mov eax, ecx
	}

	// Show GameMode in DiplomacyDialog in Skirmish
	Patch::Apply_LJMP(0x658117, 0x658126); // RadarClass_DiplomacyDialog

	// Leaves bottom bar closed for losing players during last game frames
	Patch::Apply_LJMP(0x6D1639, 0x6D1640); // TabClass_6D1610
}

bool SpawnerMain::GameConfigs::StartGame() {

	if (SpawnerMain::Configs::Active)
		return 0;

	SpawnerMain::Configs::Active = true;
	Game::IsActive = true;

	Game::InitUIStuff();

	char* pScenarioName = Config->ScenarioName;

	if (Config->IsCampaign)
	{
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
	}

	SpawnerMain::GameConfigs::LoadSidesStuff();

	bool result = Config->LoadSaveGame
		? LoadSavedGame(Config->SaveGameName)
		: StartNewScenario(pScenarioName);

	if (SpawnerMain::Configs::DumpTypes)
		DumpTypeDataArrayToFile::Dump();

	WWMouseClass::PrepareScreen();

	return result;
}

void SpawnerMain::GameConfigs::AssignHouses() {
	ScenarioClass::AssignHouses();

	const int count = MinImpl(HouseClass::Array->Count, (int)std::size(SpawnerMain::GameConfigs::Config->Houses));
	for (int indexOfHouseArray = 0; indexOfHouseArray < count; indexOfHouseArray++)
	{
		const auto pHouse = HouseClass::Array->GetItem(indexOfHouseArray);

		if (pHouse->Type->MultiplayPassive)
			continue;

		const auto pHousesConfig = &SpawnerMain::GameConfigs::GetGameConfigs()->Houses[indexOfHouseArray];
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
		if (SpawnerMain::GameConfigs::GetGameConfigs()->AINamesByDifficulty && !pHouse->IsHumanPlayer)
		{
			const auto pAIConfig = &SpawnerMain::GameConfigs::GetGameConfigs()->Players[indexOfHouseArray];

			switch (pAIConfig->Difficulty)
			{
			case 0:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AIHard));
				break;

			case 1:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AINormal));
				break;

			case 2:
				wcscpy_s(pHouse->UIName, StringTable::LoadString(GameStrings::GUI_AIEasy));
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

bool SpawnerMain::GameConfigs::StartNewScenario(const char* pScenarioName) {
	if (pScenarioName[0] == 0)
	{
		Debug::Log("[Spawner] Failed Read Scenario [%s]\n", pScenarioName);

		MessageBox::Show(
			StringTable::LoadString(GameStrings::TXT_UNABLE_READ_SCENARIO),
			StringTable::LoadString(GameStrings::TXT_OK),
			0);

		return false;
	}

	const auto pSession = &SessionClass::Instance;
	const auto pGameModeOptions = &GameModeOptionsClass::Instance;

	strcpy_s(&Game::ScenarioName, 0x200, pScenarioName);
	pSession->ReadScenarioDescriptions();

	{ // Set MPGameMode
		pSession->MPGameMode = MPGameModeClass::Get(SpawnerMain::GameConfigs::GetGameConfigs()->MPModeIndex);
		if (!pSession->MPGameMode)
			pSession->MPGameMode = MPGameModeClass::Get(1);
	}

	{ // Set Options
		pGameModeOptions->MPModeIndex = SpawnerMain::GameConfigs::GetGameConfigs()->MPModeIndex;
		// pGameModeOptions->ScenarioIndex
		pGameModeOptions->Bases = SpawnerMain::GameConfigs::GetGameConfigs()->Bases;
		pGameModeOptions->Money = SpawnerMain::GameConfigs::GetGameConfigs()->Credits;
		pGameModeOptions->BridgeDestruction = SpawnerMain::GameConfigs::GetGameConfigs()->BridgeDestroy;
		pGameModeOptions->Crates = SpawnerMain::GameConfigs::GetGameConfigs()->Crates;
		pGameModeOptions->ShortGame = SpawnerMain::GameConfigs::GetGameConfigs()->ShortGame;
		pGameModeOptions->SWAllowed = SpawnerMain::GameConfigs::GetGameConfigs()->SuperWeapons;
		pGameModeOptions->BuildOffAlly = SpawnerMain::GameConfigs::GetGameConfigs()->BuildOffAlly;
		pGameModeOptions->GameSpeed = SpawnerMain::GameConfigs::GetGameConfigs()->GameSpeed;
		pGameModeOptions->MultiEngineer = SpawnerMain::GameConfigs::GetGameConfigs()->MultiEngineer;
		pGameModeOptions->UnitCount = SpawnerMain::GameConfigs::GetGameConfigs()->UnitCount;
		pGameModeOptions->AIPlayers = SpawnerMain::GameConfigs::GetGameConfigs()->AIPlayers;
		pGameModeOptions->AIDifficulty = SpawnerMain::GameConfigs::GetGameConfigs()->AIDifficulty;
		// pGameModeOptions->AISlots
		pGameModeOptions->AlliesAllowed = SpawnerMain::GameConfigs::GetGameConfigs()->AlliesAllowed;
		pGameModeOptions->HarvesterTruce = SpawnerMain::GameConfigs::GetGameConfigs()->HarvesterTruce;
		// pGameModeOptions->CaptureTheFlag
		pGameModeOptions->FogOfWar = SpawnerMain::GameConfigs::GetGameConfigs()->FogOfWar;
		pGameModeOptions->MCVRedeploy = SpawnerMain::GameConfigs::GetGameConfigs()->MCVRedeploy;
		wcscpy(pGameModeOptions->MapDescription, SpawnerMain::GameConfigs::GetGameConfigs()->UIMapName);

		Game::Seed = SpawnerMain::GameConfigs::GetGameConfigs()->Seed;
		Game::TechLevel = SpawnerMain::GameConfigs::GetGameConfigs()->TechLevel;
		Game::PlayerColor = SpawnerMain::GameConfigs::GetGameConfigs()->Players[0].Color;
		GameOptionsClass::Instance->GameSpeed = SpawnerMain::GameConfigs::GetGameConfigs()->GameSpeed;
	}

	{ // Added AI Players
		const auto pAISlots = &pGameModeOptions->AISlots;
		for (char slotIndex = 0; slotIndex < (char)std::size(pAISlots->Allies); ++slotIndex)
		{
			const auto pPlayerConfig = &SpawnerMain::GameConfigs::GetGameConfigs()->Players[slotIndex];
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
		const char maxPlayers = SpawnerMain::GameConfigs::GetGameConfigs()->IsCampaign ? 1 : (char)std::size(SpawnerMain::GameConfigs::GetGameConfigs()->Players);
		for (char playerIndex = 0; playerIndex < maxPlayers; playerIndex++)
		{
			const auto pPlayer = &SpawnerMain::GameConfigs::GetGameConfigs()->Players[playerIndex];
			if (!pPlayer->IsHuman)
				continue;

			const auto pNode = GameCreate<NodeNameType>();
			NodeNameType::Array->AddItem(pNode);

			wcscpy_s(pNode->Name, pPlayer->Name);
			pNode->Country = pPlayer->Country;
			pNode->Color = pPlayer->Color;
			pNode->Time = -1;

			if (pPlayer->IsObserver && !SpawnerMain::GameConfigs::GetGameConfigs()->IsCampaign)
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
				if (Port != (u_short)SpawnerMain::GameConfigs::GetGameConfigs()->ListenPort)
					NetHack::PortHack = false;
			}
		}

		Game::PlayerCount = NodeNameType::Array->Count;
	}

	{ // Set SessionType
		if (SpawnerMain::GameConfigs::GetGameConfigs()->IsCampaign)
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
		return ScenarioClass::StartScenario(pScenarioName, 1, 0);
	}
	else if (SessionClass::IsSkirmish())
	{
		return ScenarioClass::StartScenario(pScenarioName, 0, -1);
	}
	else
	{
		SpawnerMain::GameConfigs::InitNetwork();
		if (!ScenarioClass::StartScenario(pScenarioName, 0, -1))
			return false;

		pSession->GameMode = GameMode::LAN;
		pSession->CreateConnections();
		return true;
	}
}

bool SpawnerMain::GameConfigs::LoadSavedGame(const char* saveGameName) {

	if (!saveGameName[0] || !StaticLoadOptionsClass::LoadMission(saveGameName))
	{
		Debug::Log("[Spawner] Failed Load Game [%s]\n", saveGameName);

		MessageBox::Show(
			StringTable::LoadString(GameStrings::TXT_ERROR_LOADING_GAME),
			StringTable::LoadString(GameStrings::TXT_OK),
			0);

		return false;
	}

	return true;
}

void SpawnerMain::GameConfigs::InitNetwork() {
	Tunnel::Id = htons((u_short)SpawnerMain::GameConfigs::GetGameConfigs()->TunnelId);
	Tunnel::Ip = inet_addr(SpawnerMain::GameConfigs::GetGameConfigs()->TunnelIp);
	Tunnel::Port = htons((u_short)SpawnerMain::GameConfigs::GetGameConfigs()->TunnelPort);

	auto& ListenPort = *reinterpret_cast<u_short*>(0x841F30u);
	ListenPort = Tunnel::Port ? 0 : (u_short)SpawnerMain::GameConfigs::GetGameConfigs()->ListenPort;

	UDPInterfaceClass::Instance = GameCreate<UDPInterfaceClass>();
	UDPInterfaceClass::Instance->Init();
	UDPInterfaceClass::Instance->OpenSocket();
	UDPInterfaceClass::Instance->StartListening();
	UDPInterfaceClass::Instance->DiscardInBuffers();
	UDPInterfaceClass::Instance->DiscardOutBuffers();
	IPXManagerClass::Instance->SetTiming(60, -1, 600, 1);

	Game::Network::PlanetWestwoodStartTime = time(NULL);
	Game::Network::GameStockKeepingUnit = 0x2901;

	ProtocolZero::Enable = (SpawnerMain::GameConfigs::GetGameConfigs()->Protocol == 0);
	if (ProtocolZero::Enable)
	{
		Game::Network::FrameSendRate = 2;
		Game::Network::PreCalcMaxAhead = SpawnerMain::GameConfigs::GetGameConfigs()->PreCalcMaxAhead;
		ProtocolZero::MaxLatencyLevel = std::clamp(
			SpawnerMain::GameConfigs::GetGameConfigs()->MaxLatencyLevel,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_1,
			(byte)LatencyLevelEnum::LATENCY_LEVEL_MAX
		);
	}
	else
	{
		Game::Network::FrameSendRate = SpawnerMain::GameConfigs::GetGameConfigs()->FrameSendRate;
	}

	Game::Network::MaxAhead = SpawnerMain::GameConfigs::GetGameConfigs()->MaxAhead == -1
		? Game::Network::FrameSendRate * 6
		: SpawnerMain::GameConfigs::GetGameConfigs()->MaxAhead;

	Game::Network::MaxMaxAhead = 0;
	Game::Network::ProtocolVersion = 2;
	Game::Network::LatencyFudge = 0;
	Game::Network::RequestedFPS = 60;
	Game::Network::Tournament = SpawnerMain::GameConfigs::GetGameConfigs()->Tournament;
	Game::Network::WOLGameID = SpawnerMain::GameConfigs::GetGameConfigs()->WOLGameID;
	Game::Network::ReconnectTimeout = SpawnerMain::GameConfigs::GetGameConfigs()->ReconnectTimeout;

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

DEFINE_HOOK(0x6BD7CB, WinMain_SpawnerInit, 0x5) { 
	SpawnerMain::GameConfigs::Init();
	return 0x0;
}

// Add support unicode player name in ingame chat
DEFINE_HOOK(0x55EDD2, MessageInput_UnicodePlayerName, 0x5) {
	if (!SpawnerMain::Configs::Enabled)
		return 0;

	wcscpy(reinterpret_cast<wchar_t*>(0xA8D63C), NodeNameType::Array->GetItem(0)->Name);
	return 0x55EE00;
}

// Display UIGameMode if is set
// Otherwise use mode name from MPModesMD.ini
DEFINE_HOOK(0x65812E, RadarClass_DiplomacyDialog_UIGameMode, 0x6) {
	enum { Show = 0x65813E, DontShow = 0x65814D };

	if (SpawnerMain::Configs::Enabled && SpawnerMain::GameConfigs::GetGameConfigs()->UIGameMode[0])
	{
		R->EBX(R->EAX());
		R->EAX(SpawnerMain::GameConfigs::GetGameConfigs()->UIGameMode);
		return Show;
	}

	if (!SessionClass::Instance->MPGameMode)
		return DontShow;

	return 0;
}

// Clear UIGameMode on game load
DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix_Spawner, 0x6) {
	if (SpawnerMain::Configs::Enabled)
		SpawnerMain::GameConfigs::GetGameConfigs()->UIGameMode[0] = 0;

	return 0;
}

#pragma region MPlayerDefeated
namespace MPlayerDefeated
{
	HouseClass* pThis = nullptr;
}

DEFINE_HOOK(0x4FC0B6, HouseClass_MPlayerDefeated_SaveArgument, 0x5) {
	MPlayerDefeated::pThis = (SpawnerMain::Configs::Enabled && !SessionClass::IsCampaign())
		? R->ECX<HouseClass*>()
		: nullptr;

	return 0;
}

// Skip match-end logic if MPlayerDefeated called for observer
DEFINE_HOOK_AGAIN(0x4FC332, HouseClass_MPlayerDefeated_SkipObserver, 0x5)
DEFINE_HOOK(0x4FC262, HouseClass__MPlayerDefeated_SkipObserver, 0x6) {
	enum { ProcEpilogue = 0x4FC6BC };

	if (!MPlayerDefeated::pThis)
		return 0;

	return MPlayerDefeated::pThis->IsObserver()
		? ProcEpilogue
		: 0;
}

DEFINE_HOOK(0x4FC551, HouseClass_MPlayerDefeated_NoEnemies, 0x5) {
	enum { ProcEpilogue = 0x4FC6BC };

	if (!MPlayerDefeated::pThis)
		return 0;

	for (const auto pHouse : *HouseClass::Array)
	{
		if (pHouse->Defeated || pHouse == MPlayerDefeated::pThis || pHouse->Type->MultiplayPassive)
			continue;

		if ((pHouse->IsHumanPlayer || SpawnerMain::GameConfigs::GetGameConfigs()->ContinueWithoutHumans) && pHouse->IsMutualAllie(MPlayerDefeated::pThis))
		{
			Debug::Log("[Spawner] MPlayer_Defeated() - Defeated player has a living ally");
			if (SpawnerMain::GameConfigs::GetGameConfigs()->DefeatedBecomesObserver)
				MPlayerDefeated::pThis->MakeObserver();

			return ProcEpilogue;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4FC57C, HouseClass_MPlayerDefeated_CheckAliveAndHumans, 0x7) {
	enum { ProcEpilogue = 0x4FC6BC, FinishMatch = 0x4FC591 };

	if (!MPlayerDefeated::pThis)
		return 0;

	GET_STACK(int, numHumans, STACK_OFFSET(0xC0, -0xA8));
	GET_STACK(int, numAlive, STACK_OFFSET(0xC0, -0xAC));

	bool continueWithoutHumans = SpawnerMain::GameConfigs::GetGameConfigs()->ContinueWithoutHumans ||
		(SessionClass::IsSkirmish() && HouseClass::CurrentPlayer->IsInitiallyObserver());

	if (numAlive > 1 && (numHumans != 0 || continueWithoutHumans))
	{
		if (SpawnerMain::GameConfigs::GetGameConfigs()->DefeatedBecomesObserver)
			MPlayerDefeated::pThis->MakeObserver();

		return ProcEpilogue;
	}

	return FinishMatch;
}

#pragma endregion MPlayerDefeated

DEFINE_HOOK(0x6BD7DC, InitBootstrapMixFiles_CustomMixes, 0x5)
{
	StaticVars::aresMIX.reset(GameCreate<MixFileClass>("ares.mix"));
	SpawnerMain::MixFile.reset(GameCreate<MixFileClass>("cncnet.mix"));
	return 0;
}