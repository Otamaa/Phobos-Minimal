#pragma once

#include <AbstractClass.h>
#include <GeneralDefinitions.h>
#include <Randomizer.h>
#include <vector>
#include <array>
#include <string>

// These determine how many of each type of sync log event are stored in the buffers.
// Any events added beyond this count overwrite old ones.
static COMPILETIMEEVAL unsigned int RNGCalls_Size = 4096;
static COMPILETIMEEVAL unsigned int FacingChanges_Size = 1024;
static COMPILETIMEEVAL unsigned int TargetChanges_Size = 1024;
static COMPILETIMEEVAL unsigned int DestinationChanges_Size = 1024;
static COMPILETIMEEVAL unsigned int MissionOverrides_Size = 256;
static COMPILETIMEEVAL unsigned int AnimCreations_Size = 512;

// Intention: Data structure that stores sync event records.
// Stores fixed amount of items determined by size template arg. Any further additions
// start to overwrite items from oldest to newest, cycling endlessly where need be.
// Likewise read/get returns items in order they were added from oldest to newest.
// TODO: Clean up / improve
template <typename T, unsigned int size>
class SyncLogEventBuffer
{
private:
	std::array<T, size> Data {};
	int LastWritePosition { 0 };
	int LastReadPosition { -1 };
	bool HasBeenFilled { false };

public:

	template< class... Args >
	bool Emplace(Args&&... args)
	{
		Data[LastWritePosition] = T(std::forward<Args>(args)...);
		LastWritePosition++;

		if (static_cast<size_t>(LastWritePosition) >= Data.size())
		{
			HasBeenFilled = true;
			LastWritePosition = 0;
			return true;
		}

		return false;
	}

	COMPILETIMEEVAL T* Get()
	{
		if (!LastWritePosition && LastReadPosition == -1 && !HasBeenFilled)
			return nullptr;

		if (LastReadPosition == -1 && HasBeenFilled)
			LastReadPosition = LastWritePosition;
		else if (LastReadPosition == -1 || static_cast<size_t>(LastReadPosition) >= Data.size())
			LastReadPosition = 0;

		return &Data[LastReadPosition++];
	}

	COMPILETIMEEVAL size_t Size() { return Data.size(); }
};

struct SyncLogEvent
{
	bool Initialized { false };
	unsigned int Caller { 0u };
	unsigned int Frame { 0u };
};

struct RNGCallSyncLogEvent : SyncLogEvent
{
	int Type; // 0 = Invalid, 1 = Unranged, 2 = Ranged
	bool IsCritical;
	unsigned int Index1;
	unsigned int Index2;
	int Min;
	int Max;

public:

	COMPILETIMEEVAL RNGCallSyncLogEvent() : SyncLogEvent {},
		Type { },
		IsCritical { },
		Index1 { },
		Index2 { },
		Min { },
		Max { }
	{ }

	COMPILETIMEEVAL RNGCallSyncLogEvent(int type, bool isCrititical, unsigned int idx1, unsigned int idx2, unsigned int Caller, unsigned int Frame, int Min, int Max)
		: SyncLogEvent { true , Caller, Frame } ,
		Type { type },
		IsCritical { isCrititical },
		Index1 { idx1 },
		Index2 { idx2 },
		Min { Min },
		Max { Max }

	{
	}
};

struct FacingChangeSyncLogEvent : SyncLogEvent
{
	unsigned short Facing;

public:

	COMPILETIMEEVAL FacingChangeSyncLogEvent() : SyncLogEvent {}, Facing {} { }
	COMPILETIMEEVAL FacingChangeSyncLogEvent(unsigned short facing, unsigned int Caller, unsigned int Frame)
		: SyncLogEvent { true , Caller, Frame },
		Facing { facing }
	{
	}
};

struct TargetChangeSyncLogEvent : SyncLogEvent
{
	AbstractType Type;
	DWORD ID;
	AbstractType TargetType;
	DWORD TargetID;

public:

	COMPILETIMEEVAL TargetChangeSyncLogEvent() : SyncLogEvent {},
		Type { },
		ID { },
		TargetType { },
		TargetID { }
	{ }

	COMPILETIMEEVAL TargetChangeSyncLogEvent(const AbstractType& type, const DWORD& id, const AbstractType& targetType, const DWORD& targetID, unsigned int Caller, unsigned int Frame)
		: SyncLogEvent { true , Caller, Frame },
		Type { type },
		ID { id },
		TargetType { targetType },
		TargetID { targetID }
	{
	}
};

struct MissionOverrideSyncLogEvent : SyncLogEvent
{
	AbstractType Type;
	DWORD ID;
	int Mission;

public:

	COMPILETIMEEVAL MissionOverrideSyncLogEvent() : SyncLogEvent {},
		Type { },
		ID { },
		Mission { }
	{ }

	COMPILETIMEEVAL MissionOverrideSyncLogEvent(const AbstractType& type, const DWORD& id, int mission, unsigned int Caller, unsigned int Frame)
		: SyncLogEvent { true , Caller, Frame },
		Type { type },
		ID { id },
		Mission { mission }
	{
	}
};

struct AnimCreationSyncLogEvent : SyncLogEvent
{
	CoordStruct Coords;

public:

	COMPILETIMEEVAL AnimCreationSyncLogEvent() : SyncLogEvent {},
		Coords { }
	{ }

	COMPILETIMEEVAL AnimCreationSyncLogEvent(const CoordStruct& coords, unsigned int Caller, unsigned int Frame)
		: SyncLogEvent { true , Caller, Frame }, Coords { coords }
	{
	}
};

class TeamClass;
class SyncLogger
{
private:
	static SyncLogEventBuffer<RNGCallSyncLogEvent, RNGCalls_Size> RNGCalls;
	static SyncLogEventBuffer<FacingChangeSyncLogEvent, FacingChanges_Size> FacingChanges;
	static SyncLogEventBuffer<TargetChangeSyncLogEvent, TargetChanges_Size> TargetChanges;
	static SyncLogEventBuffer<TargetChangeSyncLogEvent, DestinationChanges_Size> DestinationChanges;
	static SyncLogEventBuffer<MissionOverrideSyncLogEvent, MissionOverrides_Size> MissionOverrides;
	static SyncLogEventBuffer<AnimCreationSyncLogEvent, AnimCreations_Size> AnimCreations;

	static void WriteRNGCalls(FILE* const pLogFile, int frameDigits);
	static void WriteFacingChanges(FILE* const pLogFile, int frameDigits);
	static void WriteTargetChanges(FILE* const pLogFile, int frameDigits);
	static void WriteDestinationChanges(FILE* const pLogFile, int frameDigits);
	static void WriteMissionOverrides(FILE* const pLogFile, int frameDigits);
	static void WriteAnimCreations(FILE* const pLogFile, int frameDigits);
	static void WriteTeams(FILE* const pLogFile);

public:
	static bool HooksDisabled;
	static int AnimCreations_HighestX;
	static int AnimCreations_HighestY;
	static int AnimCreations_HighestZ;
	static int TeamTypeClass_MaxIDLength;
	static int ScriptTypeClass_MaxIDLength;
	static int HouseTypeClass_MaxIDLength;
	static int HouseName_MaxIDLength;

	static void AddRNGCallSyncLogEvent(Random2Class* pRandomizer, int type, unsigned int callerAddress, int min = 0, int max = 0);
	static void AddFacingChangeSyncLogEvent(unsigned short facing, unsigned int callerAddress);
	static void AddTargetChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress);
	static void AddDestinationChangeSyncLogEvent(AbstractClass* pObject, AbstractClass* pTarget, unsigned int callerAddress);
	static void AddMissionOverrideSyncLogEvent(AbstractClass* pObject, int mission, unsigned int callerAddress);
	static void AddAnimCreationSyncLogEvent(const CoordStruct& coords, unsigned int callerAddress);
	static void WriteSyncLog(const std::string& logFilename);
	static void SetTeamLoggingPadding(TeamClass* pTeam);
};
