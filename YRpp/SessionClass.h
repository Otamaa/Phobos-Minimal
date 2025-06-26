#pragma once

#include <Base/Always.h>
#include <ArrayClasses.h>
#include <GeneralDefinitions.h>
#include <MPGameModeClass.h>
#include <GameModeOptionsClass.h>
#include <Helpers/CompileTime.h>
#include <MessageListClass.h>
#include <CCFileClass.h>
#include <WinSock.h>

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

struct GlobalPacketType
{
	static COMPILETIMEEVAL reference<GlobalPacketType, 0xA8D671> const Instance {};

public:

	int Command;
	char field_4;
	char field_5;
	char field_6;
	char field_7;
	char field_8;
	char field_9;
	char field_A;
	char field_B;
	char field_C;
	char field_D;
	char field_E;
	char field_F;
	char field_10;
	char field_11;
	char field_12;
	char field_13;
	char field_14;
	char field_15;
	char field_16;
	char field_17;
	char field_18;
	char field_19;
	char field_1A;
	char field_1B;
	char field_1C;
	char field_1D;
	char field_1E;
	char field_1F;
	char field_20;
	char field_21;
	char field_22;
	char field_23;
	char field_24;
	char field_25;
	char field_26;
	char field_27;
	char field_28;
	char field_29;
	char field_2A;
	char field_2B;
	char field_2C;
	char field_2D;
	char field_2E;
	int Chat_ID;
	char field_33;
	char field_34;
	char field_35;
	char field_36;
	char field_37;
	char field_38;
	char field_39;
	char field_3A;
	char field_3B;
	char field_3C;
	char field_3D;
	char field_3E;
	char field_3F;
	char field_40;
	char field_41;
	char field_42;
	char field_43;
	char field_44;
	char field_45;
	char field_46;
	char field_47;
	char field_48;
	char field_49;
	char field_4A;
	char field_4B;
	char field_4C;
	char field_4D;
	char field_4E;
	char field_4F;
	char field_50;
	char field_51;
	char field_52;
	char field_53;
	char field_54;
	char field_55;
	char field_56;
	char field_57;
	char field_58;
	char field_59;
	char field_5A;
	char field_5B;
	char field_5C;
	char field_5D;
	char field_5E;
	char field_5F;
	char field_60;
	char field_61;
	char field_62;
	char field_63;
	char field_64;
	char field_65;
	char field_66;
	char field_67;
	char field_68;
	char field_69;
	char field_6A;
	char field_6B;
	char field_6C;
	char field_6D;
	char field_6E;
	char field_6F;
	char field_70;
	char field_71;
	char field_72;
	char field_73;
	char field_74;
	char field_75;
	char field_76;
	char field_77;
	char field_78;
	char field_79;
	char field_7A;
	char field_7B;
	char field_7C;
	char field_7D;
	char field_7E;
	char field_7F;
	char field_80;
	char field_81;
	char field_82;
	char field_83;
	char field_84;
	char field_85;
	char field_86;
	char field_87;
	char field_88;
	char field_89;
	char field_8A;
	char field_8B;
	char field_8C;
	char field_8D;
	char field_8E;
	char field_8F;
	char field_90;
	char field_91;
	char field_92;
	char field_93;
	char field_94;
	char field_95;
	char field_96;
	char field_97;
	char field_98;
	char field_99;
	char field_9A;
	char field_9B;
	char field_9C;
	char field_9D;
	char field_9E;
	char field_9F;
	char field_A0;
	char field_A1;
	char field_A2;
	char field_A3;
	char field_A4;
	char field_A5;
	char field_A6;
	char field_A7;
	char field_A8;
	char field_A9;
	char field_AA;
	char field_AB;
	char field_AC;
	char field_AD;
	char field_AE;
	char field_AF;
	char field_B0;
	char field_B1;
	char field_B2;
	char field_B3;
	char field_B4;
	char field_B5;
	char field_B6;
	char field_B7;
	char field_B8;
	char field_B9;
	char field_BA;
	char field_BB;
	char field_BC;
	char field_BD;
	char field_BE;
	char field_BF;
	char field_C0;
	char field_C1;
	char field_C2;
	char field_C3;
	char field_C4;
	char field_C5;
	char field_C6;
	char field_C7;
	char field_C8;
	char field_C9;
	char field_CA;
	char field_CB;
	char field_CC;
	char field_CD;
	char field_CE;
	char field_CF;
	char field_D0;
	char field_D1;
	char field_D2;
	char field_D3;
	char field_D4;
	char field_D5;
	char field_D6;
	char field_D7;
	char field_D8;
	char field_D9;
	char field_DA;
	char field_DB;
	char field_DC;
	char field_DD;
	char field_DE;
	char field_DF;
	char field_E0;
	char field_E1;
	char field_E2;
	char field_E3;
	char field_E4;
	char field_E5;
	char field_E6;
	char field_E7;
	char field_E8;
	char field_E9;
	char field_EA;
	char field_EB;
	char field_EC;
	char field_ED;
	char field_EE;
	char field_EF;
	char field_F0;
	char field_F1;
	char field_F2;
	char field_F3;
	char field_F4;
	char field_F5;
	char field_F6;
	char field_F7;
	char field_F8;
	char field_F9;
	char field_FA;
	char field_FB;
	char field_FC;
	char field_FD;
	char field_FE;
	char field_FF;
	char field_100;
	char field_101;
	char field_102;
	char field_103;
	char field_104;
	char field_105;
	char field_106;
	char field_107;
	char field_108;
	char field_109;
	char field_10A;
	char field_10B;
	char field_10C;
	char field_10D;
	char field_10E;
	char field_10F;
	char field_110;
	char field_111;
	char field_112;
	char field_113;
	char field_114;
	char field_115;
	char field_116;
	char field_117;
	char field_118;
	char field_119;
	char field_11A;
	char field_11B;
	char field_11C;
	char field_11D;
	char field_11E;
	char field_11F;
	char field_120;
	char field_121;
	char field_122;
	char field_123;
	char field_124;
	char field_125;
	char field_126;
	char field_127;
	char field_128;
	char field_129;
	char field_12A;
	char field_12B;
	char field_12C;
	char field_12D;
	char field_12E;
	char field_12F;
	char field_130;
	char field_131;
	char field_132;
	char field_133;
	char field_134;
	char field_135;
	char field_136;
	char field_137;
	char field_138;
	char field_139;
	char field_13A;
	char field_13B;
	char field_13C;
	char field_13D;
	char field_13E;
	char field_13F;
	char field_140;
	char field_141;
	char field_142;
	char field_143;
	char field_144;
	char field_145;
	char field_146;
	char field_147;
	char field_148;
	char field_149;
	char field_14A;
	char field_14B;
	char field_14C;
	char field_14D;
	char field_14E;
	char field_14F;
	char field_150;
	char field_151;
	char field_152;
	char field_153;
	char field_154;
	char field_155;
	char field_156;
	char field_157;
	char field_158;
	char field_159;
	char field_15A;
	char field_15B;
	char field_15C;
	char field_15D;
	char field_15E;
	char field_15F;
	char field_160;
	char field_161;
	char field_162;
	char field_163;
	char field_164;
	char field_165;
	char field_166;
	char field_167;
	char field_168;
	char field_169;
	char field_16A;
	char field_16B;
	char field_16C;
	char field_16D;
	char field_16E;
	char field_16F;
	char field_170;
	char field_171;
	char field_172;
	char field_173;
	char field_174;
	char field_175;
	char field_176;
	char field_177;
	char field_178;
	char field_179;
	char field_17A;
	char field_17B;
	char field_17C;
	char field_17D;
	char field_17E;
	char field_17F;
	char field_180;
	char field_181;
	char field_182;
	char field_183;
	char field_184;
	char field_185;
	char field_186;
	char field_187;
	char field_188;
	char field_189;
	char field_18A;
	char field_18B;
	char field_18C;
	char field_18D;
	char field_18E;
	char field_18F;
	char field_190;
	char field_191;
	char field_192;
	char field_193;
	char field_194;
	char field_195;
	char field_196;
	char field_197;
	char field_198;
	char field_199;
	char field_19A;
	char field_19B;
	char field_19C;
	char field_19D;
	char field_19E;
	char field_19F;
	char field_1A0;
	char field_1A1;
	char field_1A2;
	char field_1A3;
	char field_1A4;
	char field_1A5;
	char field_1A6;
	char field_1A7;
	char field_1A8;
	char field_1A9;
	char field_1AA;
	char field_1AB;
	char field_1AC;
	char field_1AD;
	char field_1AE;
	char field_1AF;
	char field_1B0;
	char field_1B1;
	char field_1B2;
	char field_1B3;
	char field_1B4;
	char field_1B5;
	char field_1B6;
	char field_1B7;
	char field_1B8;
	char field_1B9;
	char field_1BA;
	char field_1BB;
	char field_1BC;
	char field_1BD;
	char field_1BE;
	char field_1BF;
	char field_1C0;
	char field_1C1;
	char field_1C2;
	char field_1C3;
	char field_1C4;
	char field_1C5;
	char field_1C6;
};

static_assert(sizeof(GlobalPacketType) == 0x1C7, " Invalid Size ! ");

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

#pragma pack(pop)

class IPXAddressClass
{
	unsigned char NetworkNumber[4];
	unsigned char NodeAddress[6];
};
typedef IPXAddressClass IPAddressClass;

struct MPStatsStruct
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

static_assert(sizeof(NatStruct) == 0x9, " Invalid Size ! ");

struct TTimerClass
{
	int Started;
	int Timer;
	int Accumulated;
};

static_assert(sizeof(TTimerClass) == 0xC, " Invalid Size ! ");

struct TrapCoords
{
	int X, Y, Z;
};

static_assert(sizeof(TrapCoords) == 0xC, " Invalid Size ! ");

class MultiMission
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
};

static_assert(sizeof(MultiMission) == 0x1BC, " Invalid Size ! ");
#pragma pack(push, 4)
class SessionClass
{
public:
	static COMPILETIMEEVAL reference<SessionClass, 0xA8B238u> const Instance {};

	int Game_GetLinkedColor(int a1) {
		JMP_THIS(0x69A310)
	}

	static bool IsSkirmish()
	{
		return Instance->GameMode == GameMode::Skirmish;
	}

	static bool IsCampaign()
	{ return Instance->GameMode == GameMode::Campaign; }

	static bool IsSingleplayer()
	{
		return IsCampaign() || IsSkirmish();
	}

	static bool IsMultiplayer()
	{
		return Instance->GameMode == GameMode::LAN
			|| Instance->GameMode == GameMode::Internet;
	}

	void ReadScenarioDescriptions()
		{ JMP_THIS(0x699980) }

	bool CreateConnections()
		{ JMP_THIS(0x697B70) }

	void Resume()
		{ JMP_THIS(0x69BAB0) }
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
	ArrayWrapper<MPStatsStruct , 8u> MPStats;
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
	PROTECTED_PROPERTY(BYTE, unknown_8AA[0x1F62]);
	DynamicVectorClass<NodeNameType*> unknown_vector_280C;
	DynamicVectorClass<NodeNameType*> unknown_vector_2824;
	DynamicVectorClass<NodeNameType*> StartSpots;
	PROTECTED_PROPERTY(DWORD, unknown_2854[0x221]);
	bool CurrentlyInGame; // at least used for deciding dialog backgrounds
};
#pragma pack(pop)
//COMPILE_TIME_SIZEOF(SessionClass);
static_assert(sizeof(SessionClass) == 0x30DC, " Invalid Size ! ");