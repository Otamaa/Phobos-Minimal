#pragma once

enum EventType : char
{
  EMPTY = 0
, POWERON = 1
, POWEROFF = 2
, ALLY = 3
, MEGAMISSION = 4
, MEGAMISSION_F = 5
, IDLE = 6
, SCATTER = 7
, DESTRUCT = 8
, DEPLOY = 9
, DETONATE = 10
, PLACE = 11
, OPTIONS = 12
, GAMESPEED = 13
, PRODUCE = 14
, SUSPEND = 15
, ABANDON = 16
, PRIMARY = 17
, SPECIAL_PLACE = 18
, EXIT = 19
, ANIMATION = 20
, REPAIR = 21
, SELL = 22
, SELLCELL = 23
, SPECIAL = 24
, FRAMESYNC = 25
, MESSAGE = 26
, RESPONSTIME = 27
, FRAMEINFO = 28
, SAVEGAME = 29
, ARCHIVE = 30
, ADDPLAYER = 31
, TIMING = 32
, PROCESS_TIME = 33
, PAGEUSER = 34
, REMOVEPLAYER = 35
, LATENCYFUDGE = 36
, MEGAFRAMEINFO = 37
, PACKETTIMING = 38
, ABOUTTOEXIT = 39
, FALLBACKHOST = 40
, ADDRESSCHANGE = 41
, PLANCONNECT = 42
, PLANCOMMIT = 43
, PLANNODEDELETE = 44
, ALLCHEER = 45
, ABANDON_ALL = 46
, LAST_EVENT = 47
};

#pragma pack(push , 1)
class EventClass
{
public:

	EventType Type;
	char IsExec;
	char OwnerIndex;
	int Frame;
	int CRC;
	short CommandCount;
	char Data_FrameInfo_Delay;
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
	char takemissionfield_1C;
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
	char field_2F;
	char field_30;
	char field_31;
	char field_32;
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
};
#pragma pack(pop)

static_assert(sizeof(EventClass) == 111, "Invalid size.");