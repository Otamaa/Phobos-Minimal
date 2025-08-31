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

public:

#pragma region ClassMembers

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
#pragma endregion

public:

	TeamExtData(TeamClass* pObj) : AbstractExtended(pObj) { }
	TeamExtData(TeamClass* pObj, noinit_t& nn) : AbstractExtended(pObj, nn) { }

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

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(TeamExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(TeamExtData::base_type* key, IStream* pStm) { return true;  };

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