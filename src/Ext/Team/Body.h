#pragma once
#include <TeamClass.h>
#include <Utilities/Container.h>

#include <Utilities/Iterator.h>
#include <Utilities/MapPathCellElement.h>

class TechnoTypeClass;
class HouseClass;
class FootClass;
class TeamExtData final
{
public:
	static constexpr size_t Canary = 0x414B4B41;
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

	ScriptClass* PreviousScript { nullptr };
	std::vector<BuildingClass*> BridgeRepairHuts {};

	~TeamExtData() noexcept
	{
		if(!Phobos::Otamaa::ExeTerminated) {
			GameDelete<true, true>(PreviousScript);
		}

		PreviousScript = nullptr;
	}

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool InvalidateIgnorable(AbstractClass* ptr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat);

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(TeamExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class TeamExtContainer final : public Container<TeamExtData>
{
public:
	static std::vector<TeamExtData*> Pool;
	static TeamExtContainer Instance;

	TeamExtData* AllocateUnchecked(TeamClass* key)
	{
		TeamExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
			val = DLLAllocWithoutCTOR<TeamExtData>();
		}

		if (val) {
			val->TeamExtData::TeamExtData();
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	TeamExtData* Allocate(TeamClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (TeamExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(TeamClass* key)
	{
		if (TeamExtData* Item = TryFind(key))
		{
			Item->~TeamExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	//CONSTEXPR_NOCOPY_CLASSB(TeamExtContainer, TeamExtData, "TeamClass");
};