#pragma once
#include <TeamClass.h>
#include <Utilities/PooledContainer.h>

#include <Utilities/Iterator.h>
#include <Utilities/MapPathCellElement.h>

#include <TeamTypeClass.h>

class TechnoTypeClass;
class HouseClass;
class FootClass;
class SuperClass;
class AITriggerTypeClass;
class TeamExtData final : public AbstractExtended
{
public:
	using base_type = TeamClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	int WaitNoTargetAttempts;
	double NextSuccessWeightAward;
	int IdxSelectedObjectFromAIList;
	double CloseEnough;
	int Countdown_RegroupAtLeader;
	int MoveMissionEndMode;
	int WaitNoTargetCounter;
	CDTimerClass WaitNoTargetTimer;
	CDTimerClass ForceJump_Countdown;
	int ForceJump_InitialCountdown;
	bool ForceJump_RepeatMode;
	FootClass* TeamLeader;
	SuperClass* LastFoundSW;
	bool ConditionalJump_Evaluation;
	int ConditionalJump_ComparatorMode;
	int ConditionalJump_ComparatorValue;
	int ConditionalJump_Counter;
	int ConditionalJump_Index;
	bool AbortActionAfterKilling;
	bool ConditionalJump_EnabledKillsCount;
	bool ConditionalJump_ResetVariablesIfJump;
	int TriggersSideIdx;
	int TriggersHouseIdx;
	int AngerNodeModifier;
	bool OnlyTargetHouseEnemy;
	int OnlyTargetHouseEnemyMode;
	ScriptTypeClass* PreviousScript;
	std::vector<BuildingClass*> BridgeRepairHuts;
#pragma endregion

public:
	TeamExtData(TeamClass* pObj) : AbstractExtended(pObj),
		WaitNoTargetAttempts(0),
		NextSuccessWeightAward(0.0),
		IdxSelectedObjectFromAIList(-1),
		CloseEnough(-1.0),
		Countdown_RegroupAtLeader(-1),
		MoveMissionEndMode(0),
		WaitNoTargetCounter(0),
		WaitNoTargetTimer(),
		ForceJump_Countdown(),
		ForceJump_InitialCountdown(-1),
		ForceJump_RepeatMode(false),
		TeamLeader(nullptr),
		LastFoundSW(nullptr),
		ConditionalJump_Evaluation(false),
		ConditionalJump_ComparatorMode(3),
		ConditionalJump_ComparatorValue(1),
		ConditionalJump_Counter(0),
		ConditionalJump_Index(-1000000),
		AbortActionAfterKilling(false),
		ConditionalJump_EnabledKillsCount(false),
		ConditionalJump_ResetVariablesIfJump(false),
		TriggersSideIdx(-1),
		TriggersHouseIdx(-1),
		AngerNodeModifier(5000),
		OnlyTargetHouseEnemy(false),
		OnlyTargetHouseEnemyMode(-1),
		PreviousScript(nullptr),
		BridgeRepairHuts()
	{
		this->Name = pObj->Type->ID;
		this->AbsType = TeamClass::AbsID;
	}

	TeamExtData(TeamClass* pObj, noinit_t nn) : AbstractExtended(pObj, nn) { }

	virtual ~TeamExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TeamExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<TeamExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual TeamClass* This() const override { return reinterpret_cast<TeamClass*>(this->AbstractExtended::This()); }
	virtual const TeamClass* This_Const() const override { return reinterpret_cast<const TeamClass*>(this->AbstractExtended::This_Const()); }

public:

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat);

	static bool IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TeamExtContainer final : public Container<TeamExtData>
{
public:
	static TeamExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

};

class NOVTABLE FakeTeamClass : public TeamClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	void _Detach(AbstractClass* target, bool all);

	void _AI();
	bool _CoordinateRegroup();
	void _TeamClass_6EA080();
	void _CoordinateMove();
	void _AssignMissionTarget(AbstractClass* new_target);
	bool _Recalculate();
	void _Regroup();
	void _Calc_Center(AbstractClass** outCell, FootClass** outClosestMember);
	bool _Lagging_Units();
	bool _Add(FootClass* obj);
	bool _Add2(FootClass* obj, bool ignoreQuantity);
	bool _Can_Add(FootClass* attacker, int* entry, bool ignoreQuantity);
	bool _Remove(FootClass* obj, int typeindex, bool enterIdleMode);
	bool _Recruit(int a3);
	void _Took_Damage(FootClass* a2, DamageState result, ObjectClass* source);
	void _Coordinate_Attack();
	bool _Coordinate_Conscript(FootClass* a2);
	bool _Is_A_Member(FootClass* member);
	bool _Is_Leaving_Map();
	bool _Has_Entered_Map();
	bool _has_aircraft();
	void _Scan_Limit();
	FootClass* _Fetch_A_Leader();
	void _GetTaskForceMissingMemberTypes(std::vector<TechnoTypeClass*>& missings);
	int _Get_Stray();
	bool _Does_Any_Member_Have_Ammo();

	static void _fastcall _Suspend_Teams(int priority, HouseClass* house);
	//

	void _Coordinate_Do(ScriptActionNode* pNode ,CellStruct unused);
	/*TODO backports all vanilla codes
	TMissionFunc(Go_bezerk, 0x6EDD90)
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
	TMissionFunc(Change_team, 0x6ECFB0)
	TMissionFunc(Change_script, 0x6ED030)
	TMissionFunc(Attack, 0x6ED090)
	TMissionFunc(Load, 0x6ED200)
	TMissionFunc(Deploy, 0x6ED4D0)
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
	TMissionFunc(Iron_curtain_me,	0x6EFC70)
	TMissionFunc(Chrono_prep_for_abwp,	0x6EFE60)
	TMissionFunc(Chrono_prep_for_aq,	0x6F0130)
	*/
	void _TMission_Guard(ScriptActionNode* nNode, bool arg3);
	void _TMission_GatherAtBase(ScriptActionNode* nNode, bool arg3);
	void _TMission_GatherAtEnemy(ScriptActionNode* nNode, bool arg3);
	void _TMission_ChangeHouse(ScriptActionNode* nNode, bool arg3);
	//

	TeamExtData* _GetExtData() {
		return *reinterpret_cast<TeamExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};

static_assert(sizeof(FakeTeamClass) == sizeof(TeamClass), "Invalid Size !");