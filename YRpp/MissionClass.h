/*
	Base class for all game objects with missions (yeah... not many).
*/

#pragma once

#include <ObjectClass.h>

class CCINIClass;

enum class MissionFlags : int
{
	CurrentMission = 0,
	SuspendedMission,
	QueuedMission
};

class MissionControlClass;
class NOVTABLE MissionClass : public ObjectClass
{
public:

	static Mission __fastcall GetMissionById(const char* pName) {
		JMP_FAST(0x5B3910);
	}

	static const char* __fastcall MissionToString(Mission nMission) {
		JMP_FAST(0x5B3950);
	}

	//Destructor
	virtual ~MissionClass() RX;

	//MissionClass
	virtual bool QueueMission(Mission mission, bool start_mission) R0; //assign
	virtual bool NextMission() R0;//commence
	virtual void ForceMission(Mission mission) RX;

	virtual void Override_Mission(Mission mission, AbstractClass* tarcom = nullptr, AbstractClass* navcom = nullptr) RX; //Vt_1F4
	virtual bool Mission_Revert() R0; //Restore_Mission
	virtual bool MissionIsOverriden() const R0;//vt_1FC
	virtual bool ReadyToNextMission() const R0; //200

	virtual int Mission_Sleep() R0;
	virtual int Mission_Harmless() R0;
	virtual int Mission_Ambush() R0;
	virtual int Mission_Attack() R0;
	virtual int Mission_Capture() R0;
	virtual int Mission_Eaten() R0;
	virtual int Mission_Guard() R0;
	virtual int Mission_AreaGuard() R0;
	virtual int Mission_Harvest() R0;
	virtual int Mission_Hunt() R0;
	virtual int Mission_Move() R0;
	virtual int Mission_Retreat() R0;
	virtual int Mission_Return() R0;
	virtual int Mission_Stop() R0;
	virtual int Mission_Unload() R0;
	virtual int Mission_Enter() R0;
	virtual int Mission_Construction() R0;
	virtual int Mission_Selling() R0;
	virtual int Mission_Repair() R0;
	virtual int Mission_Missile() R0;
	virtual int Mission_Open() R0;
	virtual int Mission_Rescue() R0;
	virtual int Mission_Patrol() R0;
	virtual int Mission_ParaDropApproach() R0;
	virtual int Mission_ParaDropOverfly() R0;
	virtual int Mission_Wait() R0;
	virtual int Mission_SpyPlaneApproach() R0;
	virtual int Mission_SpyPlaneOverfly() R0;

	static bool __fastcall IsRecruitableMission(Mission mission) { JMP_FAST(0x5B36E0); }

	MissionControlClass* GetCurrentMissionControl() const { JMP_THIS(0x5B3A00); }
	void Shorten_Mission_Timer() { UpdateTimer = 0; }
	bool HasSuspendedMission() const { JMP_THIS(0x5B3A10); }
	int MissionTime() const { JMP_THIS(0x5B3A20); }
	Mission GetMission() const { JMP_THIS(0x5B3040); }

	//Constructor
	MissionClass() noexcept
		: MissionClass(noinit_t())
	{ THISCALL(0x5B2DA0); }

protected:
	explicit __forceinline MissionClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	Mission  CurrentMission;
	Mission  SuspendedMission; //B0
	Mission  QueuedMission;
	bool     unknown_bool_B8;
	int      MissionStatus;
	int      CurrentMissionStartTime;	//in frames
	int      MissionAccumulateTime;
	DECLARE_PROPERTY(CDTimerClass, UpdateTimer);
};

static_assert(sizeof(MissionClass) == 0xD4);

class MissionControlClass
{
public:
	static COMPILETIMEEVAL reference<MissionControlClass, 0xA8E3A8u, 32> const Controls {};
	static COMPILETIMEEVAL reference<const char*, 0x816CACu, 31> const Names {};

	//with safety check
	static MissionControlClass* GetMissionControl(Mission nIN) {
		if (nIN >= Mission::count) {
			return nullptr;
		}

		return &Controls[(size_t)nIN + 1];
	}

	//without safety check
	static MissionControlClass* GetMissionControlOf(Mission m) {
		return &MissionControlClass::Controls[(int)m];
	}

	const char* MissionTypeToString() {
		return MissionClass::MissionToString(this->MissionType);
	}

	MissionControlClass() {
		JMP_THIS(0x5B3700);
	}

	int NormalDelay() const {
		return static_cast<int>(TICKS_PER_MINUTE * Rate);
	}

	int AADelay() const {
		return static_cast<int>(TICKS_PER_MINUTE * AARate);
	}

	void LoadFromINI(CCINIClass* pINI) {
		JMP_THIS(0x5B3760);
	}


public:
	Mission MissionType;
	bool NoThreat;
	bool Zombie;
	bool Recruitable;
	bool Paralyzed;
	bool Retaliate;
	bool Scatter;
	double Rate; //default 0.016
	double AARate; //default 0.016
};

static_assert(sizeof(MissionControlClass) == 0x20);