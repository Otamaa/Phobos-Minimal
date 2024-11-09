#pragma once

#include <Helpers/CompileTime.h>

#include <TargetClass.h>
#include <Unsorted.h>
#include <GeneralDefinitions.h>

#pragma pack(push, 1)
union EventData
{
	EventData() { }

	struct nothing
	{
		char Data[0x68];
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
	//static inline constexpr size_t TotalSizeOfAdditinalData_1 = sizeof(EventData::Unknown_LongLong);
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
	static constexpr reference<const char*, 0x82091C, 18> const EventNames {};
	static constexpr reference<const char*, 0x82091C, 27> const AddEventNames {};
	static constexpr reference<uint8_t, 0x8208ECu, 46u> const EventLength {};

	static constexpr reference<EventList<0x80>, 0xA802C8> OutList {};

	// If the event is a MegaMission, then add it to this list
	static constexpr reference<EventList<0x100>, 0xA83ED0> MegaMissionList {};
	static constexpr reference<EventList<0x4000>, 0x8B41F8> DoList {};

	// this points to CRCs from 0x100 last frames
	static constexpr reference<DWORD, 0xB04474, 256> const LatestFramesCRC {};
	static constexpr reference<DWORD, 0xAC51FC> const CurrentFrameCRC {};

	static bool AddEvent(const EventClass* pEvent)
	{
		if (OutList->Count >= 128)
			return false;

		memcpy((OutList->List + OutList->Tail), pEvent, sizeof(EventClass));
		OutList->Timings[OutList->Tail] = static_cast<int>(Imports::TimeGetTime.get()());
		OutList->Tail = (OutList->Tail + 1) & 0x7F;
		++OutList->Count;

		return true;
	}

	static bool AddEventWithTimeStamp(EventClass* event)
	{
		event->Frame = static_cast<DWORD>(Unsorted::CurrentFrame);
		if (OutList->Count >= 128)
			return false;

		memcpy((OutList->List + OutList->Tail), event, sizeof(EventClass));
		OutList->Timings[OutList->Tail] = static_cast<int>(Imports::TimeGetTime.get()());
		OutList->Tail = (OutList->Tail + 1) & 0x7F;
		++OutList->Count;
		return true;
	}

	EventClass()
	{
		memset(this, 0, sizeof(*this));
	}

	explicit EventClass(int houseIndex, EventType eventType);

	// Special
	explicit EventClass(int houseIndex, int id);

	// Target
	explicit EventClass(int houseIndex, EventType eventType, int id, int rtti);

	// Sellcell
	explicit EventClass(int houseIndex, EventType eventType, const CellStruct& cell);

	// Archive & Planning_Connect
	explicit EventClass(int houseIndex, EventType eventType, TargetClass src, TargetClass dest);

	// Anim
	explicit EventClass(int houseIndex, int anim_id, HouseClass* pHouse, const CellStruct& cell);

	// MegaMission
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, TargetClass follow);

	// MegaMission_F
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, SpeedType speed, int/*MPHType*/ maxSpeed);

	// Production
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rtti_id, int heap_id, bool is_naval);

	// Unknown_LongLong
	explicit EventClass(int houseIndex, EventType eventType, int unknown_0, const int& unknown_c);

	// Unknown_Tuple
	explicit EventClass(int houseIndex, EventType eventType, int unknown_0, int unknown_4, int* unknown_c);

	// Place
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rttitype, int heapid, int is_naval, const CellStruct& cell);

	// SpecialPlace
	explicit EventClass(int houseIndex, EventType eventType, int id, const CellStruct& cell);

	// Specific?, maybe int[2] otherwise
	explicit EventClass(int houseIndex, EventType eventType, AbstractType rttitype, int id);

	// Address Change
	explicit EventClass(int houseIndex, void*/*IPAddressClass*/ ip, char unknown_0);

	/**
	 * dam girl, you crazy
	 *
	 * this gets called when you click-command your objects
	 * from inside TechnoClass::ClickedMission, where this is the selected object
	 *
	 * selfID and selfWhatAmI are results of Pack(this)
	 * PackedTarget and PackedTargetCell are pointers to the Pack()ed versions of the Target and TargetCell of ClickedMission
	 */
	static bool __fastcall CreateClickedMissionEvent (Mission Mission, TargetClass* PackedTarget, int selfID, char selfWhatAmI, TargetClass* PackedTargetCell)
		;//{ JMP_STD(0x646E90); }

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

static inline constexpr size_t TotalSizeOfMaindata = sizeof(EventClassMainData);
static inline constexpr size_t TotalSizeOfAdditinalData = sizeof(EventData);

static_assert(sizeof(EventClass) == (TotalSizeOfMaindata + TotalSizeOfAdditinalData));