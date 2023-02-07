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
	static constexpr size_t Canary = 0x414B4B41;
	using base_type = TeamClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public TExtension<TeamClass>
	{
	public:
		int WaitNoTargetAttempts;
		double NextSuccessWeightAward;
		int IdxSelectedObjectFromAIList;
		double CloseEnough;
		int Countdown_RegroupAtLeader;
		int MoveMissionEndMode;
		int WaitNoTargetCounter;
		TimerStruct WaitNoTargetTimer;
		TimerStruct ForceJump_Countdown;
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

		ExtData(TeamClass* OwnerObject) : TExtension<TeamClass>(OwnerObject)
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
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<TeamExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
			case AbstractType::Unit:
			case AbstractType::Infantry:
				return false;
			default:
				return true;
			}
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const Iterator<TechnoTypeClass*>& list);
	static bool HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwns(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
	static bool NeutralOwnsAll(AITriggerTypeClass* pThis, const Iterator<TechnoTypeClass*>& list);
};
