#pragma once

#include <Base/Macros.h>

#include <ArrayClasses.h>
#include <GeneralDefinitions.h>
#include <MPGameModeClass.h>
#include <GameModeOptionsClass.h>
#include <Helpers/CompileTime.h>
#include <MessageListClass.h>
#include <CCFileClass.h>
#include <IPX.h>

#pragma warning(push)
#pragma warning(disable : 4324)

struct GameTypePreferencesStruct
{
	DWORD idxMPMode;
	DWORD idxScenario;
	int GameSpeed;
	int Credits;
	int UnitCount;
	bool ShortGame;
	bool SessionOptionsClass;
	bool BuildOffAlly;
	bool MCVRepacks;
	bool CratesAppear;
	Vector3D<int> SlotData[8];
};

typedef GameTypePreferencesStruct SessionOptionsClass;
static_assert(sizeof(GameTypePreferencesStruct) == 0x7C, " Invalid Size ! ");

#pragma pack(push, 1)

struct NodeNameType
{
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<NodeNameType*>,0xA8DA74> const Array{};

public:

	wchar_t Name[20];
	sockaddr_in Address;
	char Serial[19];
	int Country;
	int InitialCountry;
	int Color;
	int InitialColor;
	int StartPoint;
	int InitialStartPoint;
	int Team;
	int InitialTeam;
	DWORD SpectatorFlag; // 0xFFFFFFFF if Spectator
	int HouseIndex;
	int Time;
	DWORD unknown_int_77;
	int Clan;
	DWORD unknown_int_7F;
	BYTE unknown_byte_83;
	BYTE unknown_byte_84;
};
static_assert(sizeof(NodeNameType) == 0x85);

struct SerialSettingsType
{
	int Port;
	int IRQ;
	int Baud;
	int DialMethod;
	int InitStringIndex;
	int CallWaitStringIndex;
	int Compression;
	int ErrorCorrection;
	char CallWaitString[16];
	char ModemName[63];
};

static_assert(sizeof(SerialSettingsType) == 0x6F, " Invalid Size ! ");

struct GlobalPacketType
{
	static COMPILETIMEEVAL reference<GlobalPacketType, 0xA8D671> const Instance {};

	int Command;
	char Name[20];
	char Serial[23];
	union
	{
		struct
		{
			unsigned int IsOpen : 1;
			unsigned int IsFirestorm : 1;
		} GameInfo;
		struct
		{
			int House;
			int Color;
			unsigned long NameCRC;
			unsigned long MinVersion;
			unsigned long MaxVersion;
			int CheatCheck;
			int AICheatCheck;
			int ArtCheatCheck;
			int BuildNumber;
		} PlayerInfo;
		struct
		{
			char pad[0x83 - 0x2F];
			unsigned int FileLength;
			char ShortFileName[12];
			unsigned char FileDigest[32];

		} ScenarioInfo;
		struct
		{
			char Buf[400];
			int Color;
			unsigned long NameCRC;
		} Message;
		struct
		{
			int OneWay;
		} ResponseTime;
		struct
		{
			int Why;
		} Reject;
		struct
		{
			unsigned long ID;
			int Color;
		} Chat;
		struct
		{
			int Percent;
		} Progress;
		struct
		{
			unsigned long ID1;
			unsigned long ID2;
		} Kick;

		char padding[455 - sizeof(Command) - sizeof(Name) - sizeof(Serial)];
	};
};

static_assert(sizeof(GlobalPacketType) == 0x1C7, "GlobalPacketType has wrong size!");

struct MangledPorts{
	DWORD OtherMangledPorts;
	DWORD dword_307E;
	DWORD dword_3082;
	DWORD dword_3086;
};
static_assert(sizeof(MangledPorts) == 16, "MangledPorts has wrong size!");
static_assert(alignof(MangledPorts) == 1, "MangledPorts has wrong size!");
#pragma pack(pop)

struct TrapCoords
{
	int X, Y, Z;
};

static_assert(sizeof(TrapCoords) == 0xC, " Invalid Size ! ");

struct NatStruct
{
	char field_0;
	char field_1;
	char field_2;
	char field_3;
	char field_4;
	char field_5;
	char field_6;
	char field_7;
	char field_8;
};
static_assert(sizeof(NatStruct) == 9u, "NatStruct has wrong size!");


struct  ALIGN(4) MPStatsStruct
{
	char Name[64];
	int MaxRoundTrip;
	int Resends;
	int Lost;
	int PercentLost;
	int MaxAvgRoundTrip;
	int FrameSyncStalls;
	int CommandCoundStalls;
	IPAddressClass Address;
};
static_assert(sizeof(MPStatsStruct)  == 0x68,   "MPStatsStruct wrong size");

class INIClass;
class ALIGN(4) MultiMission
{
public:
	wchar_t Description[44];
	char FileName[260];
	char Digest[32];
	bool IsOfficial;
	int MinPlayers;
	int MaxPlayers;
	DynamicVectorClass<int> DiskIDs;
	char field_1A0;
	DynamicVectorClass<const char*> GameModeStrings;

public:
	MultiMission(INIClass* a2, const char* section) { JMP_THIS(0x69A3B0); }
	MultiMission(const char* filename, const wchar_t* description, const char* digest, int official, int somestring2, int minplayers, int maxplayers)
	{ JMP_THIS(0x69A980); }

	~MultiMission() = default;
};

static_assert(sizeof(MultiMission) == 0x1BC, " Invalid Size ! ");

class SessionClass
{
public:
	DEFINE_REFERENCE(SessionClass, Instance, 0xA8B238u)

	// Incoming global-packet receive buffers, filled by
	// IPXManagerClass::Get_Global_Message. GlobalReceivePacket is the raw packet
	// (engine GlobalPacketType, 0x1C7 bytes; cast to the desired packet struct);
	// GlobalReceiveAddress is the sender's address.
	DEFINE_REFERENCE(char, GlobalReceivePacket, 0xA8D638)
	DEFINE_REFERENCE(IPXAddressClass, GlobalReceiveAddress, 0xA8D804)
	DEFINE_REFERENCE(DynamicVectorClass<MultiMission*>, MPlayerScenarios, 0xA8B8C8)


public:

	void Callback(int progress) const {
		JMP_THIS(0x69AE90);
	}

	int Game_GetLinkedColor(int a1) {
		JMP_THIS(0x69A310)
	}

	static bool FORCEDINLINE IsSkirmish() 
	{ return Instance->GameMode == GameMode::Skirmish; }

	static bool FORCEDINLINE IsCampaign()
	{ return Instance->GameMode == GameMode::Campaign; }

	static bool FORCEDINLINE IsSingleplayer()
	{ return IsCampaign() || IsSkirmish(); }

	static FORCEDINLINE bool IsMultiplayer()
	{ return Instance->GameMode == GameMode::LAN
			|| Instance->GameMode == GameMode::Internet; }

	// Drops a player's connection by house index: prints the "connection
	// lost" (error == 1) / "left game" (error == 0) message, deletes the IPX
	// connection, queues the remove-player event and reassigns the host.
	static void __fastcall Destroy_Connection(int id, int error)
	{ JMP_STD(0x5DA750) }

	void ReadScenarioDescriptions()
		{ JMP_THIS(0x699980) }

	bool CreateConnections()
		{ JMP_THIS(0x697B70) }

	void Resume()
		{ JMP_THIS(0x69BAB0) }

	void CallWaitString() 
		{ JMP_THIS(0x6977C0); }

	// Is the given house (or the local player, when null) the game's master/host?
	// Reads MasterPlayerID, falling back to MasterPlayerName, then to the first
	// non-defeated human house.
	bool Am_I_Master(HouseClass* who = nullptr)
		{ JMP_THIS(0x697E70) }

	// --- Fields the static layout above does not name, accessed by fixed offset.
	//     Offsets verified against gamemd.exe.

	// Non-zero while the session is suspended: the dialog message handler then
	// services Call_Back() instead of recursively running the main loop.
	FORCEDINLINE int& Suspended()
		{ return this->pendingoutofsyncmessages; }

	// Non-zero while an in-game frame is running (set by the resume routine at
	// 0x69BAB0, cleared by the pause routine at 0x69BB40). The owner-draw painter
	// (OwnerDraw::Draw_Menu) checks this to choose the dialog backdrop: when set
	// it redraws the in-game sidebar behind a band-1 dialog, when clear it draws
	// the full multiplayer menu screen (mpyscrnl). Clear it to give an in-game
	// dialog the menu backdrop, and restore it afterwards.
	FORCEDINLINE bool& InGameFrameActive()
		{ return this->CurrentlyInGame; }

	// House index of the current game master/host, or -1 if none assigned.
	FORCEDINLINE int& MasterPlayerID()
		{ return this->chatid_3074; }

	// UTF-16 name of the current game master/host (wchar_t[21]).
	FORCEDINLINE wchar_t* MasterPlayerName()
		{ return this->newgamehost; }

public:
	GameMode GameMode;
	MPGameModeClass* MPGameMode;
	DWORD unknown_08;
	DWORD unknown_0C;
	DWORD unknown_10;
	DWORD CommProtocol;
	GameModeOptionsClass Config;
	DWORD UniqueID;
	char Handle[20];
	int PlayerColor;
	DWORD unknown_160;
	DWORD unknown_164;
	DWORD unknown_168;
	DWORD unknown_16C;
	DWORD unknown_170;
	int MPlayerHouse;
	int idxSide2;
	int MPlayerPrefColor;
	int Color2;
	int Side;
	int Side2;
	SessionOptionsClass Skirmish;
	SessionOptionsClass LAN;
	SessionOptionsClass WOL;
	DWORD MultiplayerObserver;
	DWORD unknown_304;
	bool WOLLimitResolution;
	int LastNickSlot;
	int MPlayerMax;
	int MPlayerCount;
	int MaxAhead;
	int FrameSendRate;
	int DesiredFrameRate;
	int ProcessTimer;
	int ProcessTicks;
	int ProcessFrames;
	int MaxMaxAhead;
	int PrecalcMaxAhead;
	int PrecalcDesiredFrameRate;
	std::array<MPStatsStruct , 8u> MPStats;
	bool EnableMultiplayerDebug;
	bool DrawMPDebugStats;
	char field_67E;
	char field_67F;
	int LoadGame;
	int SaveGame;
	char field_688;
	bool SawCompletion;
	bool OutOfSync;
	char field_68B;
	int GameVersion;
	DynamicVectorClass<MultiMission*> MultiMission;
	char ScenarioFilename[0x202]; // 0x6A8
	char ScenarioDigest[34];
	int ScenarioFileLength;
	char ScenarioIsOfficial;
	int ScenarioMaxPlayers;
	char PlayersToSendScenario[8];
	int ScenarioSentCount;
	IPAddressClass HostAddress;
	DynamicVectorClass<GlobalPacketType*> GlobalPacketVector;
	int field_908[8];
	char array_928[32][8];
	MessageListClass Messages;
	IPAddressClass MessageAddress;
	char SomeMask[8];
	char LANTaunts;
	char WOLTaunts;
	wchar_t LastMessage[113];
	int Bitfield;
	char LANScrollText;
	char WOLScrollText;
	char field_1FC2;
	char field_1FC3;
	MPlayerScoreType Score[8];
	int GamesPlayed;
	int NumScores;
	int Winner;
	int CurGame;
	CCFileClass RecordFile;
	unsigned int Record : 1;
	unsigned int Play : 1;
	unsigned int Attract : 1;
	int IsBridge;
	IPAddressClass BridgeNet;
	char NetStealth;
	char NetProtect;
	char NetOpen;
	wchar_t GameName[20];
	GlobalPacketType GPacket;
	char field_25C7;
	int GPacketLen;
	IPAddressClass GAddress;
	wchar_t GProductID;
	char MetaPacket[558];
	int MetaSize;
	DynamicVectorClass<NodeNameType*> Games;
	DynamicVectorClass<NodeNameType*> Players;
	DynamicVectorClass<NodeNameType*> StartSpots;
	int NodeNamePointerCount;
	void* NodeNamePointers[8];
	char bool_2878;
	char field_2879;
	char field_287A;
	char field_287B;
	int pendingoutofsyncmessages;
	int ModemService;
	int CurPhoneIdx;
	SerialSettingsType SerialDefaults;
	char field_28F7;
	int ModemType;
	DynamicVectorClass<void*> PhoneBook;
	DynamicVectorClass<void*> InitStrings;
	DynamicVectorClass <void*>CallWaitStrings;
	int array_2944[8];
	int LatencyFudge;
	char ViaPacketRouter;
	char field_2969;
	char field_296A;
	char field_296B;
	IPAddressClass PacketRouterAddress;
	IPAddressClass PacketRouterAddressTranslate;
	short PacketRouterPort;
	char field_2986;
	char field_2987;
	IPAddressClass LocalChatAddress;
	IPAddressClass CoopPlayerAddresses[8];
	int FirewallBehaviors[8];
	int field_2A14[128];
	int ConnectionResults[128];
	int NumConnectionPairs;
	char field_2E18;
	char field_2E19;
	char field_2E1A;
	char field_2E1B;
	char field_2E1C;
	char field_2E1D;
	char field_2E1E;
	char field_2E1F;
	char field_2E20;
	char field_2E21;
	char field_2E22;
	char field_2E23;
	char field_2E24;
	char field_2E25;
	char field_2E26;
	char field_2E27;
	NatStruct NatStructs[32];
	char SendDelay;
	char field_2F49;
	char field_2F4A;
	char field_2F4B;
	int TalkToPorts[8];
	int KeepAliveTimerIndex[8];
	CDTimerClass KeepAliveTimers[8];
	int KeepAliveTimerCount;
	int PortBase;
	int ForcePortBase;
	char field_2FF8[32];
	char field_3018;
	char field_3019;
	char field_301A;
	char field_301B;
	int PortNumberOverride;
	wchar_t newgamehost[21];
	char field_304A;
	char field_304B;
	int timings_304C[8];
	char char_306C;
	char field_306D;
	short word_306E;
	short word_3070;
	char field_3072;
	char field_3073;
	int chatid_3074;
	char char_3078;
	char field_3079;
	MangledPorts CurrentMangledPorts;
	char field_308A;
	char field_308B;
	int field_308C;
	char NOROUTER;
	int FallbackHost;
	char DontDoOwnerTalk;
	int Rout_resp;
	int timing_30A0;
	int timing_30A4;
	int timing_30A8;
	int TrapFrame;
	int TrapObjType;
	int TrapObject;
	CoordStruct TrapCoord;
	float TrapTarget1;
	char TrapTarget2;
	int TrapCell;
	int TrapCheckHeap;
	int TrapPrintCRC;
	char func_69BAB0_called_30D8;
	char field_30D9;
	char field_30DA;
	bool CurrentlyInGame; // at least used for deciding dialog backgrounds
};

//COMPILE_TIME_SIZEOF(SessionClass);
static_assert(sizeof(SessionClass) == 0x30DC, " SessionClass Invalid Size ! ");
static_assert(offsetof(SessionClass, Config) == 0x18, "Config");
static_assert(offsetof(SessionClass, ModemType) == 0x28F8, "ModemType");
static_assert(offsetof(SessionClass, RecordFile)   == 0x2354, "RecordFile");
static_assert(offsetof(SessionClass, IsBridge) == 0x23C4, "IsBridge");
static_assert(offsetof(SessionClass, BridgeNet)    == 0x23C8, "BridgeNet");
static_assert(offsetof(SessionClass, Score)        == 0x1FC4, "Score");
static_assert(offsetof(SessionClass, MPStats)      == 0x033C, "MPStats");
static_assert(offsetof(SessionClass, Messages)     == 0x0A28, "Messages");
static_assert(offsetof(SessionClass, UniqueID)     == 0x0144, "UniqueID");
static_assert(offsetof(SessionClass, Skirmish)     == 0x018C, "Skirmish");
static_assert(offsetof(SessionClass, WOLTaunts) == 0x1ED9, "WOLTaunts");
static_assert(offsetof(SessionClass, LastMessage)  == 0x1EDA, "LastMessage");
static_assert(offsetof(SessionClass, Bitfield) == 0x1FBC, "Bitfield");
static_assert(offsetof(SessionClass, char_3078) == 0x3078, "field_3078");
static_assert(offsetof(SessionClass, field_3079) == 0x3079, "field_3079");
static_assert(sizeof(SessionClass::field_3079) == 1, "field_3079 size");
static_assert(offsetof(SessionClass, CurrentMangledPorts) == 0x307A, "OtherPlayersMangledPorts");
static_assert(offsetof(SessionClass, field_308A) == 0x308A, "field_308A");
static_assert(sizeof(MPlayerScoreType) == 0x70,   "MPlayerScoreType wrong size");
static_assert(sizeof(MPStatsStruct)    == 0x68,   "MPStatsStruct wrong size");
static_assert(sizeof(MessageListClass) == 0x149C, "MessageListClass wrong size");
static_assert(sizeof(GlobalPacketType) == 0x1C7,  "GlobalPacketType wrong size");
static_assert(sizeof(CCFileClass)      == 0x6C,   "CCFileClass wrong size");
static_assert(sizeof(IPXAddressClass)  == 0x0C,   "IPXAddressClass wrong size");
static_assert(sizeof(CoordStruct) == 12, "CoordStruct wrong size");
static_assert(offsetof(SessionClass, TalkToPorts) == 0x2F4C, "TalkToPorts");
static_assert(offsetof(SessionClass, KeepAliveTimerIndex) == 0x2F6C, "KeepAliveTimerIndex");
static_assert(offsetof(SessionClass, KeepAliveTimers) == 0x2F8C, "KeepAliveTimers");
static_assert(offsetof(SessionClass, PacketRouterAddress) == 0x296C, "PacketRouterAddress wrong size");
static_assert(offsetof(SessionClass, NatStructs) == 0x2E28, "NatStructs wrong size");
static_assert(offsetof(SessionClass, KeepAliveTimerCount) == 0x2FEC, "KeepAliveTimerCount wrong size");
static_assert(offsetof(SessionClass, PortNumberOverride) == 0x301C, "PortNumberOverride wrong size");
static_assert(offsetof(SessionClass, timings_304C) == 0x304C, "timings_304C wrong size");
static_assert(offsetof(SessionClass, chatid_3074) == 0x3074, "chatid_3074 wrong size");
static_assert(offsetof(SessionClass, newgamehost) == 0x3020, "newgamehost wrong size");
static_assert(alignof(MessageListClass) == 4);
static_assert(alignof(CCFileClass) == 4);
static_assert(alignof(NodeNameType) == 1);
static_assert(alignof(GlobalPacketType) == 1);
static_assert(alignof(SerialSettingsType) == 1);
static_assert(alignof(CDTimerClass) == 4);
static_assert(alignof(GameModeOptionsClass) == 4);
static_assert(alignof(MultiMission) == 4);
static_assert(alignof(MPStatsStruct) == 4);
static_assert(alignof(NatStruct) == 1);

#pragma warning(pop)