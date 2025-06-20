/*
	[TeamTypes]
*/

#pragma once

#include <AbstractTypeClass.h>
#include <ScriptTypeClass.h>
#include <TaskForceClass.h>

//forward declarations
class FootClass;
class TagClass;
class TeamClass;
class TechnoTypeClass;
#pragma pack(push, 4)
class DECLSPEC_UUID("D1DBA64E-0778-11D2-ACA5-006008055BB5")
	NOVTABLE TeamTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TeamType;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TeamTypeClass*>, 0xA8ECA0u> const Array {};

	IMPL_Find(TeamTypeClass)

	static TeamTypeClass* __fastcall FindByNameAndId(const char* pID) {
		JMP_STD(0x6F0FC0);
	}

	static TeamTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x6F1920);
	}

	IMPL_FindIndexById(TeamTypeClass)
	IMPL_FindIndexByName(TeamTypeClass)

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TeamTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	static bool LoadFromINIList(CCINIClass *pINI, bool IsGlobal)
		{ PUSH_VAR8(IsGlobal); SET_REG32(ECX, pINI); ASM_CALL(0x6F19B0); }

	TeamClass * CreateTeam(HouseClass *pHouse)
		{ JMP_THIS(0x6F09C0); }

	void DestroyAllInstances()
		{ JMP_THIS(0x6F0A70); }

	int GetGroup() const
		{ JMP_THIS(0x6F1870); }

	CellStruct* GetWaypoint(CellStruct *buffer) const
		{ JMP_THIS(0x6F18A0); }

	CellStruct* GetTransportWaypoint(CellStruct *buffer) const
		{ JMP_THIS(0x6F18E0); }

	bool CanRecruitUnit(FootClass* pUnit, HouseClass* pOwner) const
		{ JMP_THIS(0x6F1320); }

	void FlashAllInstances(int Duration)
		{ JMP_THIS(0x6F1F30); }

	TeamClass * FindFirstInstance() const
		{ JMP_THIS(0x6F1F70); }

	void ProcessTaskForce()
		{ JMP_THIS(0x6F1FA0); }

	static void __fastcall ProcessAllTaskforces()
		{ JMP_STD(0x6F2040); }

	HouseClass* GetHouse() const
		{ JMP_THIS(0x6F2070); }

	//Constructor
	TeamTypeClass(const char* pID) noexcept
		: TeamTypeClass(noinit_t())
	{ JMP_THIS(0x6F06E0); }

protected:
	explicit __forceinline TeamTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int      ArrayIndex; //98
	int      Group; //9C
	int      VeteranLevel; //A0
	bool     Loadable; //A4
	bool     Full; //A5
	bool     Annoyance; //A6
	bool     GuardSlower; //A7
	bool     Recruiter;  //A8
	bool     Autocreate;  //A9
	bool     Prebuild; //AA
	bool     Reinforce; //AB
	bool     Whiner; //AC
	bool     Aggressive; //AD
	bool     LooseRecruit; //AE
	bool     Suicide; //AF
	bool     DropPod; //B0
	bool     UseTransportOrigin; //B1
	bool     DropshipLoadout; //B2
	bool     OnTransOnly; //B3
	int      Priority; //B4
	int      Max; //B8
	int      field_BC; //BC
	int      MindControlDecision; //C0
	HouseClass *     Owner; //C4
	int      idxHouse; //C8 idx for MP
	int      TechLevel; //CC
	TagClass* Tag; //D0
	int      Waypoint; //D4
	int      TransportWaypoint; //D8
	int      cntInstances; //DC
	ScriptTypeClass*  ScriptType; //E0
	TaskForceClass*   TaskForce; //E4
	int      IsGlobal; //E8
	int      field_EC; //EC
	bool     field_F0; //F0
	bool     field_F1; //F1
	bool     AvoidThreats; //F2
	bool     IonImmune; //F3
	bool     TransportsReturnOnUnload; //F4
	bool     AreTeamMembersRecruitable; //F5
	bool     IsBaseDefense; //F6
	bool     OnlyTargetHouseEnemy; //F7

};
#pragma pack(pop)

static_assert(sizeof(TeamTypeClass) == 0xF8, "Invalid size.");