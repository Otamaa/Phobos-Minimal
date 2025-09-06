#pragma once
#include <TeamClass.h>
#include <Utilities/PooledContainer.h>

#include <Utilities/Iterator.h>
#include <Utilities/MapPathCellElement.h>

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
		PreviousScript(nullptr)
	{ }
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
	static void Clear()
	{
		Array.clear();
	}

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

	//
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