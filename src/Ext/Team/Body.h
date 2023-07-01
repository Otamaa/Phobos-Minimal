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
		double NextSuccessWeightAward { 0.0 };
		int IdxSelectedObjectFromAIList { -1 };
		int CloseEnough { -1 };
		int Countdown_RegroupAtLeader { -1 };
		int MoveMissionEndMode { 0 };
		int WaitNoTargetCounter { 0 };
		CDTimerClass WaitNoTargetTimer {};
		CDTimerClass ForceJump_Countdown {};
		int ForceJump_InitialCountdown { -1 };
		bool ForceJump_RepeatMode { false };
		FootClass* TeamLeader { nullptr };
		int GenericStatus { 0 };
		int FailedCounter { -1 };

		// #442
		int AngerNodeModifier { 5000 };
		bool OnlyTargetHouseEnemy { false };
		int OnlyTargetHouseEnemyMode { -1 };

		// #599
		bool ConditionalJump_Evaluation { false };
		int ConditionalJump_ComparatorMode { 3 };
		int ConditionalJump_ComparatorValue { 1 };
		int ConditionalJump_Counter { 0 };
		int ConditionalJump_Index { -1000000 };
		bool AbortActionAfterKilling { false };
		bool ConditionalJump_EnabledKillsCount { false };
		bool ConditionalJump_ResetVariablesIfJump { false };

		//#691
		std::vector<ScriptClass*> PreviousScriptList { };

		//#565
		int TriggersSideIdx { -1 };
		int TriggersHouseIdx { -1 };

		//#791
		std::vector<std::vector<bool>> MapPath_Grid { }; // Used for marking visited/analyzed cells
		std::vector<MapPathCellElement> MapPath_Queue { }; // Cells that will be analyzed for finding a path
		bool MapPath_InProgress { false };
		TechnoClass* MapPath_StartTechno { nullptr };
		TechnoClass* MapPath_EndTechno { nullptr };
		std::vector<TechnoClass*> MapPath_BridgeRepairHuts { };
		std::vector<TechnoClass*> MapPath_ValidBridgeRepairHuts { };
		std::vector<TechnoClass*> MapPath_CheckedBridgeRepairHuts { };

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)

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
