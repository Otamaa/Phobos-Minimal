#pragma once
#include <TeamClass.h>
#include <Ext/Abstract/Body.h>

#include <Utilities/Iterator.h>
#include <Utilities/MapPathCellElement.h>

class TechnoTypeClass;
class HouseClass;
class FootClass;
class TeamExt
{
public:
	class ExtData final : public Extension<TeamClass>
	{
	public:
		static constexpr size_t Canary = 0x414B4B41;
		using base_type = TeamClass;

	public:
		int WaitNoTargetAttempts { 0 };
		double NextSuccessWeightAward { 0 };
		int IdxSelectedObjectFromAIList { -1 };
		double CloseEnough { -1 };
		int Countdown_RegroupAtLeader { -1 };
		int MoveMissionEndMode { 0 };
		int WaitNoTargetCounter { 0 };
		CDTimerClass WaitNoTargetTimer { 0 };
		CDTimerClass ForceJump_Countdown { 0 };
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

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)

		{ }

		virtual ~ExtData() override {
			GameDelete<true, true>(PreviousScript);
			PreviousScript = nullptr;
		}

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
		static bool InvalidateIgnorable(AbstractClass* ptr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NOINLINE GroupAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pThat);
};
