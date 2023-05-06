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
		int GenericStatus;
		int FailedCounter;

		// #442
		int AngerNodeModifier;
		bool OnlyTargetHouseEnemy;
		int OnlyTargetHouseEnemyMode;

		// #599
		bool ConditionalJump_Evaluation;
		int ConditionalJump_ComparatorMode;
		int ConditionalJump_ComparatorValue;
		int ConditionalJump_Counter;
		int ConditionalJump_Index;
		bool AbortActionAfterKilling;
		bool ConditionalJump_EnabledKillsCount;
		bool ConditionalJump_ResetVariablesIfJump;

		//#691
		std::vector<ScriptClass*> PreviousScriptList;

		//#565
		int TriggersSideIdx;
		int TriggersHouseIdx;

		//#791
		std::vector<std::vector<bool>> MapPath_Grid; // Used for marking visited/analyzed cells
		std::vector<MapPathCellElement> MapPath_Queue; // Cells that will be analyzed for finding a path
		bool MapPath_InProgress;
		TechnoClass* MapPath_StartTechno;
		TechnoClass* MapPath_EndTechno;
		std::vector<TechnoClass*> MapPath_BridgeRepairHuts;
		std::vector<TechnoClass*> MapPath_ValidBridgeRepairHuts;
		std::vector<TechnoClass*> MapPath_CheckedBridgeRepairHuts;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)
			, WaitNoTargetAttempts { 0 }
			, NextSuccessWeightAward { 0 }
			, IdxSelectedObjectFromAIList { -1 }
			, CloseEnough { -1 }
			, Countdown_RegroupAtLeader { -1 }
			, MoveMissionEndMode { 0 }
			, WaitNoTargetCounter { 0 }
			, WaitNoTargetTimer { 0 }
			, ForceJump_Countdown { -1 }
			, ForceJump_InitialCountdown { -1 }
			, ForceJump_RepeatMode { false }
			, TeamLeader { nullptr }
			, GenericStatus { 0 }
			, FailedCounter { -1 }

			, AngerNodeModifier { 5000 }
			, OnlyTargetHouseEnemy { false }
			, OnlyTargetHouseEnemyMode { -1 }

			, ConditionalJump_Evaluation { false }
			, ConditionalJump_ComparatorMode { 3 }
			, ConditionalJump_ComparatorValue { 1 }
			, ConditionalJump_Counter { 0 }
			, ConditionalJump_Index { -1000000 }
			, AbortActionAfterKilling { false }
			, ConditionalJump_EnabledKillsCount { false }
			, ConditionalJump_ResetVariablesIfJump { false }

			, TriggersSideIdx { -1 }
			, TriggersHouseIdx { -1 }

			, MapPath_Grid { }
			, MapPath_Queue { }
			, MapPath_InProgress { false }
			, MapPath_StartTechno { nullptr }
			, MapPath_EndTechno { nullptr }
			, MapPath_BridgeRepairHuts { }
			, MapPath_ValidBridgeRepairHuts { }
			, MapPath_CheckedBridgeRepairHuts { }
		{ }

		virtual ~ExtData() override = default;
		void InvalidatePointer(void* ptr, bool bRemoved);
		bool InvalidateIgnorable(void* ptr) const;
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

	bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
};
