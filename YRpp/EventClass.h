#pragma once

#include <Helpers/CompileTime.h>

#include <TargetClass.h>
#include <Unsorted.h>
#include <GeneralDefinitions.h>
#include <QueueClass.h>

#pragma pack(push, 1)
union EventData
{
	EventData() { }
	~EventData() = default;

	struct nothing
	{
		std::array<char, 0x68> Data;

		template<typename T>
		FORCEDINLINE T* As() {
			return reinterpret_cast<T*>(this->Data.data());
		}

		template<typename T>
		FORCEDINLINE void Set(T* data , size_t size) {
			memcpy(this->Data.data(), data, size);
		}

		template<typename T>
		FORCEDINLINE void Set(T* data) {
			memcpy(this->Data.data(), data, T::size());
		}

	} nothing;
	static_assert(sizeof(nothing) == 0x68);

	struct unkData
	{
		DWORD Checksum;
		WORD CommandCount;
		BYTE Delay;
		std::array<BYTE, 0x61> ExtraData;
	} unkData;
	static_assert(sizeof(unkData) == 0x68);

	struct Animation
	{
		int ID; // Anim ID
		int AnimOwner; // House ID
		CellStruct Location;
		std::array<BYTE, 0x5C> ExtraData;
	} Animation;
	static_assert(sizeof(Animation) == 0x68);

	struct FrameInfo
	{
		BYTE CRC;
		WORD CommandCount;
		BYTE Delay;
		std::array<BYTE, 0x64> ExtraData;
	} FrameInfo;
	static_assert(sizeof(FrameInfo) == 0x68);

	struct Target
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Target;
	static_assert(sizeof(Target) == 0x68);

	struct MegaMission
	{
		TargetClass Whom;
		BYTE Mission; // Mission but should be byte
		char _gap_;
		TargetClass Target;
		TargetClass Destination;
		TargetClass Follow;
		bool IsPlanningEvent;
		PlanningOrderState PlanningState;
		std::array<BYTE, 0x50> ExtraData;
	} MegaMission;
	static_assert(sizeof(MegaMission) == 0x68);

	struct MegaMission_F
	{
		TargetClass Whom;
		BYTE Mission;
		TargetClass Target;
		TargetClass Destination;
		int Speed;
		int MaxSpeed;
		std::array<BYTE, 0x50> ExtraData;
	} MegaMission_F; // Seems unused in YR?
	static_assert(sizeof(MegaMission_F) == 0x68);

	struct Production
	{
		int RTTI_ID;
		int Heap_ID;
		int IsNaval;
		std::array<BYTE, 0x5C> ExtraData;
	} Production;
	static_assert(sizeof(Production) == 0x68);

	struct Unknown_LongLong
	{
		int Unknown_0; //4
		long long Data; //8
		int Unknown_C; //4
		std::array<BYTE, 0x58> ExtraData;
	} Unknown_LongLong;
	//static OPTIONALINLINE COMPILETIMEEVAL size_t TotalSizeOfAdditinalData_1 = sizeof(EventData::Unknown_LongLong);
	static_assert(sizeof(Unknown_LongLong) == 0x68);

	struct Unknown_Tuple
	{
		int Unknown_0;
		int Unknown_4;
		int Data;
		int Unknown_C;
		std::array<BYTE, 0x58> ExtraData;
	} Unknown_Tuple;
	static_assert(sizeof(Unknown_Tuple) == 0x68);

	struct Place
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		CellStruct Location;
		std::array<BYTE, 0x58> ExtraData;
	} Place;
	static_assert(sizeof(Place) == 0x68);

	struct SpecialPlace
	{
		int ID;
		CellStruct Location;
		std::array<BYTE, 0x60> ExtraData;
	} SpecialPlace;
	static_assert(sizeof(SpecialPlace) == 0x68);

	struct Specific
	{
		AbstractType RTTIType;
		int ID;
		std::array<BYTE, 0x60> ExtraData;
	} Specific;
	static_assert(sizeof(Specific) == 0x68);

	struct Poweron
	{
		TargetClass Target;
		std::array<BYTE, 0x63> ExtraData;
	} Poweron;
	static_assert(sizeof(Poweron) == 0x68);

	struct Poweroff
	{
		TargetClass Target;
		std::array<BYTE, 0x63> ExtraData;
	} Poweroff;
	static_assert(sizeof(Poweroff) == 0x68);

	struct Ally
	{
		int HouseID;
		std::array<BYTE, 0x64> ExtraData;
	} Ally;
	static_assert(sizeof(Ally) == 0x68);

	struct Idle
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Idle;
	static_assert(sizeof(Idle) == 0x68);

	struct Scatter
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Scatter;
	static_assert(sizeof(Scatter) == 0x68);

	struct Destruct
	{
		std::array<BYTE, 0x68> ExtraData;
	} Destruct;
	static_assert(sizeof(Destruct) == 0x68);

	struct Deploy
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Deploy;
	static_assert(sizeof(Deploy) == 0x68);

	struct Detonate
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Detonate;
	static_assert(sizeof(Detonate) == 0x68);

	struct Options
	{ std::array<BYTE, 0x68> ExtraData; } Options;
	static_assert(sizeof(Options) == 0x68);

	struct GameSpeed
	{
		int m_GameSpeed;
		std::array<BYTE, 0x64> ExtraData;
	} GameSpeed;
	static_assert(sizeof(GameSpeed) == 0x68);

	// This event starts production of the specified object type. The house can
	// determine from the type and ID value, what object to begin production on and
	// what factory to use.
	struct Produce
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		std::array<BYTE, 0x5C> ExtraData;
	} Produce;
	static_assert(sizeof(Produce) == 0x68);

	struct Suspend
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		std::array<BYTE, 0x5C> ExtraData;
	} Suspend;
	static_assert(sizeof(Suspend) == 0x68);

	// This event is generated when the player cancels production of the specified
	// object type. From the object type, the exact factory can be inferred.
	struct Abandon
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		std::array<BYTE, 0x5C> ExtraData;
	} Abandon;
	static_assert(sizeof(Abandon) == 0x68);

	// Toggles the primary factory state of the specified building.
	struct Primary
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Primary;
	static_assert(sizeof(Primary) == 0x68);

	struct Exit
	{ std::array<BYTE, 0x68> ExtraData; } Exit;
	static_assert(sizeof(Exit) == 0x68);

	// Starts or stops repair on the specified object. This event is triggered by the
	// player clicking the repair wrench on a building.
	struct Repair
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Repair;
	static_assert(sizeof(Repair) == 0x68);

	// Tells a building/unit to sell. This event is triggered by the player clicking the
	// sell animating cursor over the building or unit.
	struct Sell
	{
		TargetClass Whom;
		std::array<BYTE, 0x63> ExtraData;
	} Sell;
	static_assert(sizeof(Sell) == 0x68);

	// Used to sell walls
	struct SellCell
	{
		CellStruct Location;
		std::array<BYTE, 0x68 - sizeof(CellStruct)> ExtraData;
	} SellCell;
	static_assert(sizeof(SellCell) == 0x68);

	// Update the special control flags. This is necessary so that in a multiplayer
	// game, all machines will agree on the rules. If these options change during
	// game play, then all players are informed that options have changed.
	struct Special
	{
		ScenarioFlags SpecialFlags;
		std::array<BYTE, 0x68 - sizeof(ScenarioFlags)> ExtraData;
	} Special;
	static_assert(sizeof(Special) == 0x68);

	struct FrameSync
	{ std::array<BYTE, 0x68> ExtraData; } FrameSync;
	static_assert(sizeof(FrameSync) == 0x68);

	struct Message
	{ std::array<BYTE, 0x68> ExtraData; } Message;
	static_assert(sizeof(Message) == 0x68);

	struct ResponseTime
	{
		char unknown;
		std::array<BYTE, 0x68 - sizeof(char)> ExtraData;
	} ResponseTime;
	static_assert(sizeof(ResponseTime) == 0x68);

	struct SaveGame
	{ std::array<BYTE, 0x68> ExtraData; } SaveGame;
	static_assert(sizeof(SaveGame) == 0x68);

	// Update the archive target for this building.
	struct Archive
	{
		struct main
		{
			TargetClass Target1;
			TargetClass Target2;
		}m_Target;
		std::array<BYTE, 0x68 - sizeof(Archive::main)> ExtraData;
	} Archive;
	static_assert(sizeof(Archive) == 0x68);

	struct AddPlayer
	{
		struct main
		{
			// Local buffer allocated and filled by the network decoder.
			void* Data;
			unsigned int DataSize; // Buffer length in bytes.
			// Compressed packets carry DataSize followed by the buffer bytes.
			// Uncompressed packets append the bytes after the full EventClass.
			// The receiver replaces Data with its own allocation in both formats.
		}m_Data;
		std::array<BYTE, 0x68 - sizeof(AddPlayer::main)> ExtraData;
	} AddPlayer;
	static_assert(sizeof(AddPlayer) == 0x68);

	struct Timing
	{
		struct main
		{
			unsigned short RequestedFPS;
			unsigned short MaxAhead;
			unsigned char FrameSendRate;
		}m_Data;
		std::array<BYTE, 0x68 - sizeof(Timing::main)> ExtraData;
	} Timing;
	static_assert(sizeof(Timing) == 0x68);

	struct ProcessTime
	{
		unsigned short Time;
		std::array<BYTE, 0x68 - sizeof(unsigned short)> ExtraData;
	} ProcessTime;
	static_assert(sizeof(ProcessTime) == 0x68);

	struct PageUser
	{ std::array<BYTE, 0x68> ExtraData; } PageUser;
	static_assert(sizeof(PageUser) == 0x68);

	struct RemovePlayer
	{
		int HouseID;
		std::array<BYTE, 0x64> ExtraData;
	} RemovePlayer;
	static_assert(sizeof(RemovePlayer) == 0x68);

	struct LatencyFudge
	{
		int LatencyFudge;
		std::array<BYTE, 0x64> ExtraData;
	} LatencyFudge;
	static_assert(sizeof(LatencyFudge) == 0x68);

	struct MegafFameInfo
	{
		std::array<BYTE, 0x68> ExtraData;
	} MegafFameInfo;
	static_assert(sizeof(MegafFameInfo) == 0x68);

	struct PacketTiming
	{
		char Unknown[0x40];
		std::array<BYTE, 0x68 - 0x40> ExtraData;
	} PacketTiming;
	static_assert(sizeof(PacketTiming) == 0x68);

	struct AboutToExit
	{ std::array<BYTE, 0x68> ExtraData; } AboutToExit;
	static_assert(sizeof(AboutToExit) == 0x68);

	struct FallbackHost
	{
		int m_FallbackHost;
		std::array<BYTE, 0x64> ExtraData;
	} FallbackHost;
	static_assert(sizeof(FallbackHost) == 0x68);

	struct AddressChange
	{
		char PlayerID;
		DWORD Address;
		std::array<BYTE, 0x63> ExtraData;
	} AddressChange;
	static_assert(sizeof(AddressChange) == 0x68);

	struct PlanConnect
	{
		struct main
		{
			TargetClass Target1;
			TargetClass Target2;
		}m_Targets;
		std::array<BYTE, 0x68 - sizeof(PlanConnect::main)> ExtraData;
	} PlanConnect;
	static_assert(sizeof(PlanConnect) == 0x68);

	struct PlanCommit
	{ std::array<BYTE, 0x68 > ExtraData; } PlanCommit;
	static_assert(sizeof(PlanCommit) == 0x68);

	struct PlanNodeDelete
	{
		TargetClass Target;
		std::array<BYTE, 0x68 - sizeof(TargetClass)> ExtraData;
	} PlanNodeDelete;
	static_assert(sizeof(PlanNodeDelete) == 0x68);

	struct AllCheer
	{
		char Unknown[4];
		std::array<BYTE, 0x64> ExtraData;
	} AllCheer;
	static_assert(sizeof(AllCheer) == 0x68);

	struct AbandonAll
	{
		char Unknown[12];
		std::array<BYTE, 0x5C> ExtraData;
	} AbandonAll;
	static_assert(sizeof(AbandonAll) == 0x68);
};

class EventClass;

template<size_t Length>
struct EventList
{
public:
	int Count;
	int Head;
	int Tail;
	std::array<EventClass, Length> List;
	std::array<int, Length> Timings;
};

class EventClass
{
public:
	static COMPILETIMEEVAL size_t EventLength_Max = 128;

	static COMPILETIMEEVAL reference<const char*, 0x82091C, 18u> const EventNames {};
	static COMPILETIMEEVAL reference<const char*, 0x82091C, 27u> const AddEventNames {};
	static COMPILETIMEEVAL reference<uint8_t, 0x8208ECu, 44u> const EventLength {};

	static COMPILETIMEEVAL reference<QueueClass<EventClass, 128u>, 0xA802C8> OutList {};

	// If the event is a MegaMission, then add it to this list
	static COMPILETIMEEVAL reference<QueueClass<EventClass, 256u>, 0xA83ED0> MegaMissionList {};

	// 8 houses, 8-time cache targets
	static COMPILETIMEEVAL reference<DWORD, 0xAC50FC, 16u> MegaMissionTargetNum {};
	static COMPILETIMEEVAL reference2D<TargetClass, 0xAFA468, 16u, 128u> MegaMissionTargets{};

	static COMPILETIMEEVAL reference<QueueClass<EventClass,0x4000>, 0x8B41F8> DoList {};

	// this points to CRCs from 0x100 last frames
	static COMPILETIMEEVAL reference<DWORD, 0xB04474, 256u> const LatestFramesCRC {};
	static COMPILETIMEEVAL reference<DWORD, 0xAC51FC> const CurrentFrameCRC {};

	// The engine's out-of-sync sync dump, called from Execute_DoList (0x64CC68)
	// when a FRAMEINFO CRC mismatch is found: writes SYNC*.TXT for the offending
	// event so a desync can be diagnosed. Print_CRCs_Current_Player writes the
	// local player's SYNC<player>.TXT; Print_CRCs_All_Players writes
	// SYNC<player>_<slot>.TXT for each of the 256 frame slots, and the engine
	// only uses it when Unsorted::EnableMPSyncDebug is set.
	static void __fastcall Print_CRCs_Current_Player(EventClass* ev)
		{ JMP_STD(0x64DEA0); }

	static void __fastcall Print_CRCs_All_Players(int frame_slot, EventClass* ev)
		{ JMP_STD(0x6516F0); }

private: 
	//change 30/10/2026 - Otamaa
	//					  Develop implementing way to increase the command Queue count 
	//					  so calling these function directly are stricly prohibited now

	static bool AddEvent(EventClass* pEvent) {
		return OutList->Add(pEvent);
	}

	static bool AddEventWithTimeStamp(EventClass* pEvent) {
		pEvent->Frame = static_cast<DWORD>(Unsorted::CurrentFrame.get());
		return OutList->Add(pEvent);
	}

	template<typename... Args>
	static bool CreateEvent(Args&&... args) {
		EventClass ev(std::forward<Args>(args)...);
		return OutList->Add(&ev);
	}

public:

	EventClass()
	{
		__stosb(reinterpret_cast<unsigned char*>(this), 0, sizeof(*this));
	}

	~EventClass() = default;

	explicit EventClass(int houseIndex, EventType eventType)
	{
		JMP_THIS(0x4C66C0);
	}

	// Special
	explicit EventClass(int houseIndex, int id)
	{
		JMP_THIS(0x4C65A0);
	}

	// Target
	explicit EventClass(int houseIndex, EventType eventType, int id, AbstractType rtti)
	{
		JMP_THIS(0x4C65E0);
	}

	// Sellcell
	explicit EventClass(int houseIndex, EventType eventType, const CellStruct& cell)
	{
		JMP_THIS(0x4C6650);
	}

	// Archive & Planning_Connect
	explicit EventClass(int houseIndex, EventType eventType, TargetClass src, TargetClass dest)
	{
		JMP_THIS(0x4C6780);
	}

	// Anim
	explicit EventClass(int houseIndex, int anim_id, HouseClass* pHouse, const CellStruct& cell)
	{
		JMP_THIS(0x4C6800);
	}

	// MegaMission
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, TargetClass follow)
	{
		JMP_THIS(0x4C6860);
	}

	// MegaMission_F
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, SpeedType speed, int/*MPHType*/ maxSpeed)
	{
		JMP_THIS(0x4C68E0);
	}

	// Production
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rtti_id, int heap_id, bool is_naval)
	{
		JMP_THIS(0x4C6970);
	}

	// Unknown_LongLong
	explicit EventClass(int houseIndex, EventType eventType, int unknown_0, const int& unknown_c)
	{
		JMP_THIS(0x4C69E0);
	}

	// Unknown_Tuple
	explicit EventClass(int houseIndex, EventType eventType, int unknown_0, int unknown_4, int* unknown_c)
	{
		JMP_THIS(0x4C6A60);
	}

	// Place
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rttitype, int heapid, int is_naval, const CellStruct& cell)
	{
		JMP_THIS(0x4C6AE0);
	}

	// SpecialPlace
	explicit EventClass(int houseIndex, EventType eventType, int id, const CellStruct& cell)
	{
		JMP_THIS(0x4C6B60);
	}

	// Specific?, maybe int[2] otherwise
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rttitype, int id)
	{
		JMP_THIS(0x4C6BE0);
	}

	// Address Change
	explicit EventClass(int houseIndex, void*/*IPAddressClass*/ ip, char unknown_0)
	{
		JMP_THIS(0x4C6C50);
	}

	/**
	 * dam girl, you crazy
	 *
	 * this gets called when you click-command your objects
	 * from inside TechnoClass::ClickedMission, where this is the selected object
	 *
	 * selfID and selfWhatAmI are results of Pack(this)
	 * PackedTarget and PackedTargetCell are pointers to the Pack()ed versions of the Target and TargetCell of ClickedMission
	 */
	static bool __fastcall CreateClickedMissionEvent
	(Mission Mission, TargetClass* PackedTarget, int selfID, char selfWhatAmI, TargetClass* PackedTargetCell)
	{
		JMP_FAST(0x646E90);
	}

	explicit EventClass(const EventClass& another)
	{
		memcpy(this, &another, sizeof(*this));
	}

	EventClass& operator=(const EventClass& another)
	{
		if (this != &another)
			memcpy(this, &another, sizeof(*this));

		return *this;
	}

public:

	EventType Type;
	bool IsExecuted;
	char HouseIndex; // '-1' stands for not a valid house
	uint32_t Frame; // 'Frame' is the frame that the command should execute on.

	EventData Data;

public:

	bool operator==(const EventClass& q) const
	{
		return memcmp(this, &q, sizeof(q)) == 0;
	};

};

struct EventClassMainData
{
	EventType Type;
	bool IsExecuted;
	char HouseIndex; // '-1' stands for not a valid house
	uint32_t Frame;
};
#pragma pack(pop)

static OPTIONALINLINE COMPILETIMEEVAL size_t TotalSizeOfMaindata = sizeof(EventClassMainData);
static OPTIONALINLINE COMPILETIMEEVAL size_t TotalSizeOfAdditinalData = sizeof(EventData);

static_assert(sizeof(EventClass) == (TotalSizeOfMaindata + TotalSizeOfAdditinalData));