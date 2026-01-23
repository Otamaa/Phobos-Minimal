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

	struct nothing
	{
		char Data[0x68];

		template<typename T>
		FORCEDINLINE T* As() {
			return reinterpret_cast<T*>(this->Data);
		}

		template<typename T>
		FORCEDINLINE void Set(T* data , size_t size) {
			memcpy(this->Data, data, size);
		}

		template<typename T>
		FORCEDINLINE void Set(T* data) {
			memcpy(this->Data, data, T::size());
		}

	} nothing;
	static_assert(sizeof(nothing) == 0x68);

	struct unkData
	{
		DWORD Checksum;
		WORD CommandCount;
		BYTE Delay;
		BYTE ExtraData[0x61];
	} unkData;
	static_assert(sizeof(unkData) == 0x68);

	struct Animation
	{
		int ID; // Anim ID
		int AnimOwner; // House ID
		CellStruct Location;
		BYTE ExtraData[0x5C];
	} Animation;
	static_assert(sizeof(Animation) == 0x68);

	struct FrameInfo
	{
		BYTE CRC;
		WORD CommandCount;
		BYTE Delay;
		BYTE ExtraData[0x64];
	} FrameInfo;
	static_assert(sizeof(FrameInfo) == 0x68);

	struct Target
	{
		TargetClass Whom;
		BYTE ExtraData[0x63];
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
		BYTE ExtraData[0x51];
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
		BYTE ExtraData[0x50];
	} MegaMission_F; // Seems unused in YR?
	static_assert(sizeof(MegaMission_F) == 0x68);

	struct Production
	{
		int RTTI_ID;
		int Heap_ID;
		int IsNaval;
		BYTE ExtraData[0x5C];
	} Production;
	static_assert(sizeof(Production) == 0x68);

	struct Unknown_LongLong
	{
		int Unknown_0; //4
		long long Data; //8
		int Unknown_C; //4
		BYTE ExtraData[0x58];
	} Unknown_LongLong;
	//static OPTIONALINLINE COMPILETIMEEVAL size_t TotalSizeOfAdditinalData_1 = sizeof(EventData::Unknown_LongLong);
	static_assert(sizeof(Unknown_LongLong) == 0x68);

	struct Unknown_Tuple
	{
		int Unknown_0;
		int Unknown_4;
		int Data;
		int Unknown_C;
		BYTE ExtraData[0x58];
	} Unknown_Tuple;
	static_assert(sizeof(Unknown_Tuple) == 0x68);

	struct Place
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		CellStruct Location;
		BYTE ExtraData[0x58];
	} Place;
	static_assert(sizeof(Place) == 0x68);

	struct SpecialPlace
	{
		int ID;
		CellStruct Location;
		BYTE ExtraData[0x60];
	} SpecialPlace;
	static_assert(sizeof(SpecialPlace) == 0x68);

	struct Specific
	{
		AbstractType RTTIType;
		int ID;
		BYTE ExtraData[0x60];
	} Specific;
	static_assert(sizeof(Specific) == 0x68);

};

class EventClass;

template<size_t Length>
struct EventList
{
public:
	int Count;
	int Head;
	int Tail;
	EventClass List[Length];
	int Timings[Length];
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

	static bool AddEvent(EventClass* pEvent) {
		return OutList->Add(pEvent);
	}

	static bool AddEventWithTimeStamp(EventClass* pEvent) {
		pEvent->Frame = static_cast<DWORD>(Unsorted::CurrentFrame);
		return OutList->Add(pEvent);
	}

	EventClass()
	{
		__stosb(reinterpret_cast<unsigned char*>(this), 0, sizeof(*this));
	}

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
	explicit EventClass(int houseIndex, EventType eventType, int id, int rtti)
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