#pragma once

#include <AbstractClass.h>
#include <ScriptClass.h>
#include <array>

class HouseClass;
class ObjectClass;
class FootClass;
class CellClass;
class ScriptClass;
class TagClass;
class TeamTypeClass;
class TechnoTypeClass;

class DECLSPEC_UUID("0E272DCF-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE TeamClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Team;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F4730;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TeamClass*>, 0x8B40E8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6EC560);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6EC450);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x6EC540);

	//Destructor
	virtual ~TeamClass() JMP_THIS(0x6F0450);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x6F0430);
	virtual int ClassSize() const override JMP_THIS(0x6F0440);
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x6EAE60);

	void DTOR_IMPL() {
		JMP_THIS(0x6E8DE0);
	}

	void _scalar_dtor(char flags) {
		this->DTOR_IMPL();

		if ((flags & 1) != 0)
			GameDelete<false,false>(this);
	}

	// fills dest with all types needed to complete this team. each type is
	// included as often as it is needed.
	void GetTaskForceMissingMemberTypes(DynamicVectorClass<TechnoTypeClass *>* dest) const { JMP_THIS(0x6EF4D0); }
	bool IsReallyLeavingMap() const { JMP_THIS(0x6EC300); }
	void LiberateMember(FootClass* pFoot, int idx=-1, byte count=0) const { JMP_THIS(0x6EA870); }
	void RemoveMember(FootClass* pFoot, int idx = -1, byte count = 0) const { JMP_THIS(0x6EA870); }
	bool Recuit(int nIdx) const { JMP_THIS(0x6EAA90); }
	// if bKeepQuantity is false, this will not change the quantity of each techno member
	bool AddMember(FootClass* pFoot) const { JMP_THIS(0x6EA4F0); }
	bool AddMember(FootClass* pFoot, bool bForce) { JMP_THIS(0x6EA500); }
	bool CanAddMember(FootClass* a2, int* entry, char somebool) const { JMP_THIS(0x6EA610); }
	bool HasMissionRemaining() const JMP_THIS(0x6915D0);
	FootClass* FetchLeader() const { JMP_THIS(0x6EC3D0); }
	bool Reacalculate() const { JMP_THIS(0x6EA3E0); }
	void TookDamage(FootClass* a2, DamageState result, ObjectClass* source) const { JMP_THIS(0x6EB380); }
	bool Team_Func6EA080() const { JMP_THIS(0x6EA080); }
	void Regroup() const { JMP_THIS(0x6EA0D0); }
	void CalCulateCenter(AbstractClass **a2, FootClass **a3) const { JMP_THIS(0x6EAEE0); }
	void CoordinateAttack() const { JMP_THIS(0x6EB490); }
	bool CoordinateRegroup() const { JMP_THIS(0x6EB870); }
	bool CoordinateMove() const { JMP_THIS(0x6EBAD0); }
	bool LaggingUnits() const { JMP_THIS(0x6EBF50); }
	bool CoordinateConscript(FootClass* a2) const { JMP_THIS(0x6EC130); }
	bool IsAMember(FootClass* member) const { JMP_THIS(0x6EC220); }
	bool HasEnteredMap() const { JMP_THIS(0x6EC370); }
	void ScanLimit() const { JMP_THIS(0x6EC3A0); }
	void AssignMissionTarget(AbstractClass* new_target) const { JMP_THIS(0x6E9050); }
	bool HasAircraft() const { JMP_THIS(0x6EF470); }
	int GetStrayDistance () const { JMP_THIS(0x6F03B0); } //return in lepton
	bool DoesAnyMemberHaveAmmo() const { JMP_THIS(0x6F03F0); }

	static std::array<const DWORD, (size_t)TeamMissionType::count> TMissionFuncTable;

	void NOINLINE ExecuteTMission(TeamMissionType action, ScriptActionNode* pNode, bool arg3) const
	{
		if (action == TeamMissionType::none
			|| action == TeamMissionType::count
			)
			return;

		using fp_type = void(__thiscall*)(const TeamClass*, ScriptActionNode*, bool);
		reinterpret_cast<fp_type>(TMissionFuncTable[((int)action)])(this, pNode, arg3);
	}
//
#define TMissionFunc(name , addr ) void TMission_## name ##(ScriptActionNode*, bool) { JMP_THIS(addr);}

	TMissionFunc(Go_bezerk, 0x6EDD90)
	TMissionFunc(Do, 0x6ED7E0)
	TMissionFunc(Goto_nearby_shroud, 0x6EC730)
	TMissionFunc(Movecell, 0x6EC770)
	TMissionFunc(Move, 0x6EC7D0)
	TMissionFunc(Att_waypt, 0x6EC9A0)
	TMissionFunc(Attack_building_at_waypoint, 0x6ECA70)
	TMissionFunc(Enter_grinder, 0x6ECB50)
	TMissionFunc(Occupy_tank_bunker, 0x6ECBA0)
	TMissionFunc(Enter_bio_reactor, 0x6ECBF0)
	TMissionFunc(Occupy_battle_bunker, 0x6ECC40)
	TMissionFunc(Garrison_building, 0x6ECC90)
	TMissionFunc(Patrol, 0x6ECCE0)
	TMissionFunc(Spy, 0x6ECE60)
	TMissionFunc(Scatter, 0x6ECF10)
	TMissionFunc(Change_house, 0x6ECF50)
	TMissionFunc(Change_team, 0x6ECFB0)
	TMissionFunc(Change_script, 0x6ED030)
	TMissionFunc(Attack, 0x6ED090)
	TMissionFunc(Load, 0x6ED200)
	TMissionFunc(Deploy, 0x6ED4D0)
	TMissionFunc(Guard, 0x6ED770)
	TMissionFunc(Set_global, 0x6EDA90)
	TMissionFunc(Clear_global, 0x6EDAC0)
	TMissionFunc(Set_local, 0x6EDAF0)
	TMissionFunc(Clear_local, 0x6EDB20)
	TMissionFunc(Hound_dog, 0x6EDB50)
	TMissionFunc(Unpanic, 0x6EDC70)
	TMissionFunc(Force_facing, 0x6EDCA0)
	TMissionFunc(Panic, 0x6EDD60)
	TMissionFunc(Go_Berzerk, 0x6EDD90)
	TMissionFunc(Idle_anim, 0x6EDDC0)
	TMissionFunc(Loop, 0x6EDE10)
	TMissionFunc(Player_wins, 0x6EDE40)
	TMissionFunc(Player_loses, 0x6EDE60)
	TMissionFunc(Play_speech, 0x6EDE80)
	TMissionFunc(Play_sound, 0x6EDE90)
	TMissionFunc(Play_movie, 0x6EDEC0)
	TMissionFunc(Play_music, 0x6EDEF0)
	TMissionFunc(Reduce_tiberium, 0x6EDF10)
	TMissionFunc(Begin_production, 0x6EDF90)
	TMissionFunc(Fire_sale, 0x6EDFB0)
	TMissionFunc(Self_destruct, 0x6EDFD0)
	TMissionFunc(Delete_team_members, 0x6EE050)
	TMissionFunc(Ion_storm_start_in, 0x6EE0A0)
	TMissionFunc(Ion_storn_end, 0x6EE0E0)
	TMissionFunc(Center_view_on_team, 0x6EE100)
	TMissionFunc(Reshroud_map, 0x6EE1B0)
	TMissionFunc(Reveal_map, 0x6EE1D0)
	TMissionFunc(Wait_till_fully_loaded, 0x6EE1F0)
	TMissionFunc(Truck_unload, 0x6EE230)
	TMissionFunc(Truck_load, 0x6EE2A0)
	TMissionFunc(Attack_enemy_building, 0x6EE310)
	TMissionFunc(Moveto_enemy_building, 0x6EE3F0)
	TMissionFunc(Move_to_own_building, 0x6EE5C0)
	TMissionFunc(Scout, 0x6EE800)
	TMissionFunc(Unload, 0x6EF110)
	TMissionFunc(Success, 0x6EF450)
	TMissionFunc(Flash, 0x6EF5C0)
	TMissionFunc(Play_anim, 0x6EF610)
	TMissionFunc(Talk_bubble, 0x6EF6D0)
	TMissionFunc(Gather_at_enemy, 0x6EF700)
	TMissionFunc(Gather_at_base, 0x6EFA10)
	TMissionFunc(Iron_curtain_me,	0x6EFC70)
	TMissionFunc(Chrono_prep_for_abwp,	0x6EFE60)
	TMissionFunc(Chrono_prep_for_aq,	0x6F0130)

#undef TMissionFunc
//
	static void __fastcall Suspend_Teams(int priority, HouseClass* house) { JMP_STD(0x6EC250); }
	static ThreatType __fastcall ThreatFromQuarry(QuarryType q) { JMP_THIS(0x645BB0); }

	const char* get_ID() const;

	//Constructor
	TeamClass(TeamTypeClass* pType , HouseClass* pOwner, int investigate_me) noexcept
		: TeamClass(noinit_t())
	{ JMP_THIS(0x6E8A90); }

protected:
	explicit __forceinline TeamClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TeamTypeClass* Type; //24
	ScriptClass*   CurrentScript; //28
	HouseClass*    OwnerHouse; //2C
	HouseClass*    TargetHouse; //30
	AbstractClass* Zone; //34
	FootClass*	   ClosestMember; //38
	AbstractClass* QueuedFocus; //3C
	AbstractClass* ArchiveTarget; //40 Focus
	int            unknown_44;  //44
	int            TotalObjects;  //48
	int            TotalThreatValue; //4C
	int            CreationFrame; //50
	FootClass *    FirstUnit; //54
	CDTimerClass   GuardAreaTimer; //58
	CDTimerClass   SuspendTimer; //64
	TagClass*      Tag; //70
	bool           IsTransient; //74
	bool           NeedsReGrouping; //75
	bool           GuardSlowerIsNotUnderStrength; //76
	bool           IsForcedActive; //77

	bool           IsHasBeen; //78
	bool           IsFullStrength; //79
	bool           IsUnderStrength; //7A
	bool           IsReforming; //7B

	bool           IsLagging; //7C
	bool           NeedsToDisappear; //7D
	bool           JustDisappeared; //7E
	bool           IsMoving; //7F

	bool           StepCompleted; //80 can proceed to the next step of the script
	bool           TargetNotAssigned; //81
	bool           IsLeavingMap; //82
	bool           IsSuspended; //83

	bool           AchievedGreatSuccess; //84 executed script action 49, 0

	int CountObjects [6]; //88 counts of each object specified in the Type
};

static_assert(sizeof(TeamClass) == 0xA0 , "Invalid Size !");

/*
00000000 struct TeamClass // sizeof=0xA0
00000000 {
00000000     AbstractClass a;
00000024     TeamTypeClass *Class;          // official
00000028     ScriptClass *Script;           // confirmed
0000002C     HouseClass *House;             // official
00000030     HouseClass *__ScoutHouse;
00000034     CellClass *Zone;               // ...
00000034                                    // official
00000038     FootClass *ClosestMember;      // official
0000003C     _DWORD MissionTarget;          // official
00000040     ObjectClass *Target;           // official
00000044     _DWORD __INVESTIGATE44;
00000048     _DWORD Total;                  // official
0000004C     _DWORD Risk;                   // official
00000050     _DWORD __CreationFrame;        // Frame when CTOR was executed
00000054     FootClass *Member;             // official
00000058     CDTimerClass TimeOut;          // official
00000064     CDTimerClass SuspendTimer;     // official
00000070     _DWORD Tag;                    // confirmed, Trigger in RA
00000074     _BYTE __DestroyTeamType74__IsTransient; // destroys this teams TeamType in DTOR
00000075     _BYTE __needs_regrouping;      // Coordinate_Regroup was true or Took Damage
00000076     _BYTE __GuardSlowerIsNotUnderStrength;
00000077     _BYTE IsForcedActive;          // official
00000078     _BYTE IsHasBeen;               // official
00000079     _BYTE IsFullStrength;          // official
0000007A     _BYTE IsUnderStrength;         // official
0000007B     _BYTE IsReforming;             // official
0000007C     bool IsLagging;                // official
0000007D     _BYTE IsAltered;               // official
0000007E     _BYTE JustAltered;             // official
0000007F     _BYTE IsMoving;                // official
00000080     _BYTE IsNextMission;           // official
00000081     _BYTE __target_not_assigned;   // set after Assign Target is called
00000082     _BYTE IsLeaveMap;              // official
00000083     _BYTE Suspended;               // official
00000084     _BYTE __was_a_success;         // calls AITriggerTypeClass register functions, based on state
00000085     // padding byte
00000086     // padding byte
00000087     // padding byte
00000088     int Quantity[6] __tabform(,,0); // official
000000A0 };
*/