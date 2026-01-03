#pragma once
#include <TeamClass.h>
#include <Utilities/PooledContainer.h>

#include <Utilities/Iterator.h>
#include <Utilities/MapPathCellElement.h>
#include <Utilities/PhobosFixedString.h>

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
	static COMPILETIMEEVAL const char* ClassName = "TeamExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TeamClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	PhobosFixedString<0x18> Name;
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
		Name(),
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

	TeamClass* This() const { return reinterpret_cast<TeamClass*>(this->AttachedToObject); }
	const TeamClass* This_Const() const { return reinterpret_cast<const TeamClass*>(this->AttachedToObject); }

public:

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat);

	static bool IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement);
	static bool IsEligible(TechnoTypeClass* pGoing, TechnoTypeClass* reinfocement);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TeamExtContainer final : public Container<TeamExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TeamExtContainer";

public:
	static TeamExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

};

class NOVTABLE FakeTeamClass : public TeamClass
{
public:

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
	void _Flash_For(int a2);
	int _Get_Stray();
	bool _Does_Any_Member_Have_Ammo();

	static void _fastcall _Suspend_Teams(int priority, HouseClass* house);
	//

	void _Coordinate_Do(ScriptActionNode* pNode ,CellStruct unused);

	void _TMission_Unload(ScriptActionNode* nNode, bool arg3);
	void _TMission_Load(ScriptActionNode* nNode, bool arg3);
	void _TMission_Deploy(ScriptActionNode* nNode, bool arg3);
	void _TMission_Scout(ScriptActionNode* nNode, bool arg3);
	void _TMission_Move_To_Own_Building(ScriptActionNode* nNode, bool arg3);
	void _TMission_Attack_Enemy_Building(ScriptActionNode* nNode, bool arg3);
	void _TMission_Chrono_prep_for_abwp(ScriptActionNode* nNode, bool arg3);
	void _TMission_Chrono_prep_for_aq(ScriptActionNode* nNode, bool arg3);
	void _TMission_Play_Animation(ScriptActionNode* nNode, bool arg3);
	void _TMission_Iron_Curtain_Me(ScriptActionNode* nNode, bool arg3);
	void _TMission_Guard(ScriptActionNode* nNode, bool arg3);
	void _TMission_GatherAtBase(ScriptActionNode* nNode, bool arg3);
	void _TMission_GatherAtEnemy(ScriptActionNode* nNode, bool arg3);
	void _TMission_ChangeHouse(ScriptActionNode* nNode, bool arg3);
	void _TMission_Enter_Grinder(ScriptActionNode* nNode, bool arg3);
	void _TMission_Enter_Bio_Reactor(ScriptActionNode* nNode, bool arg3);
	void _TMission_Occupy_Battle_Bunker(ScriptActionNode* nNode, bool arg3);
	void _TMission_Occupy_Tank_Bunker(ScriptActionNode* nNode, bool arg3);
	void _TMission_Attack(ScriptActionNode* nNode, bool arg3);
	void _TMission_Attack_Waypoint(ScriptActionNode* nNode, bool arg3);
	void _TMission_Move_To_Cell(ScriptActionNode* nNode, bool arg3);
	void _TMission_Loop(ScriptActionNode* nNode, bool arg3);
	void _TMission_Player_wins(ScriptActionNode* nNode, bool arg3);
	void _TMission_Player_loses(ScriptActionNode* nNode, bool arg3);
	void _TMission_Talk_bubble(ScriptActionNode* nNode, bool arg3);

	//

	TeamExtData* _GetExtData() {
		return *reinterpret_cast<TeamExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};

static_assert(sizeof(FakeTeamClass) == sizeof(TeamClass), "Invalid Size !");