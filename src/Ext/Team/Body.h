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
class TeamExtData
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x414B4B41;
	using base_type = TeamClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	int WaitNoTargetAttempts { 0 };
	double NextSuccessWeightAward { 0 };
	int IdxSelectedObjectFromAIList { -1 };
	double CloseEnough { -1 };
	int Countdown_RegroupAtLeader { -1 };
	int MoveMissionEndMode { 0 };
	int WaitNoTargetCounter { 0 };
	CDTimerClass WaitNoTargetTimer { };
	CDTimerClass ForceJump_Countdown { };
	int ForceJump_InitialCountdown { -1 };
	bool ForceJump_RepeatMode { false };
	FootClass* TeamLeader { nullptr };

	SuperClass* LastFoundSW { nullptr };

	bool ConditionalJump_Evaluation { false };
	int ConditionalJump_ComparatorMode { 3 };
	int ConditionalJump_ComparatorValue { 1 };
	int ConditionalJump_Counter { 0 };
	int ConditionalJump_Index { -1000000 };
	bool AbortActionAfterKilling { false };
	bool ConditionalJump_EnabledKillsCount { false };
	bool ConditionalJump_ResetVariablesIfJump { false };

	int TriggersSideIdx { -1 };
	int TriggersHouseIdx { -1 };

	int AngerNodeModifier { 5000 };
	bool OnlyTargetHouseEnemy { false };
	int OnlyTargetHouseEnemyMode { -1 };

	ScriptTypeClass* PreviousScript { nullptr };
	std::vector<BuildingClass*> BridgeRepairHuts {};

	//{
	//	//if(!Phobos::Otamaa::ExeTerminated) {
	//	//	GameDelete<true, true>(PreviousScript);
	//	//}
	//
	//	PreviousScript = nullptr;
	//}

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat);

	static bool IsEligible(TechnoClass* pGoing, TechnoTypeClass* reinfocement);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TeamExtData) -
			(4u //AttachedToObject
				- 4u //inheritance
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class TeamExtContainer final : public Container<TeamExtData>
{
public:
	static TeamExtContainer Instance;
	static StaticObjectPool<TeamExtData, 10000> pools;

	TeamExtData* AllocateUnchecked(TeamClass* key)
	{
		TeamExtData* val = pools.allocate();

		if (val)
		{
			val->AttachedToObject = key;
		}
		else
		{
			Debug::FatalErrorAndExit("The amount of [TeamExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		}

		return val;
	}

	void Remove(TeamClass* key)
	{
		if (TeamExtData* Item = TryFind(key))
		{
			RemoveExtOf(key, Item);
		}
	}

	void RemoveExtOf(TeamClass* key, TeamExtData* Item)
	{
		pools.deallocate(Item);
		this->ClearExtAttribute(key);
	}

};

class NOVTABLE FakeTeamClass : public TeamClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

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