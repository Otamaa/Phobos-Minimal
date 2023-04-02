#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/TemplateDefB.h>

#include <Utilities/Debug.h>

#include <ScriptTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataRules.h>

#endif

class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;

class RulesExt
{
public:
	static constexpr size_t Canary = 0x12341234;
	using base_type = RulesClass;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		Valueable<Point3D> Pips_Shield;
		Valueable<Point3D> Pips_Shield_Buildings;
		Valueable<int> RadApplicationDelay_Building;
		PhobosFixedString<32u> MissingCameo;

		std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists;
		std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists;
		std::vector<std::vector<std::string>> AIHousesLists;
		std::vector<std::vector<std::string>> AIConditionsLists;
		std::vector<std::vector<AITriggerTypeClass*>> AITriggersLists;

		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<bool> JumpjetTurnToTarget;
		Valueable<bool> JumpjetCrash_Rotate;

		Valueable<int> Storage_TiberiumIndex;
		Valueable<int> PlacementGrid_TranslucentLevel;
		Valueable<int> BuildingPlacementPreview_TranslucentLevel;

		Nullable<SHPStruct*> Pips_Shield_Background_SHP;
		Valueable<Point3D> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;

		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;

		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> EnemyInsignia;
		Valueable<bool> ShowAllyDisguiseBlinking;

		Valueable<SHPStruct*> SHP_SelectBrdSHP_INF;
		CustomPalette SHP_SelectBrdPAL_INF;
		Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT;
		CustomPalette SHP_SelectBrdPAL_UNIT;

		Nullable<bool> UseSelectBrd;

		Valueable<Point3D> SelectBrd_Frame_Infantry;
		Valueable<Point2D> SelectBrd_DrawOffset_Infantry;
		Valueable<Point3D> SelectBrd_Frame_Unit;
		Valueable<Point2D> SelectBrd_DrawOffset_Unit;

		Valueable<int> SelectBrd_DefaultTranslucentLevel;
		Valueable<bool> SelectBrd_DefaultShowEnemy;

		Valueable<bool> RadWarhead_Detonate;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<bool> IronCurtain_SyncDeploysInto;

		Valueable<PartialVector2D<int>> ROF_RandomDelay;

		Valueable<ColorStruct> ToolTip_Background_Color;
		Valueable<int> ToolTip_Background_Opacity;
		Valueable<float> ToolTip_Background_BlurSize;

		Valueable<bool> Crate_LandOnly;

		std::vector<std::pair<std::string, std::vector<int>>> GenericPrerequisitesData;

		Valueable<bool> NewTeamsSelector;
		Valueable<bool> NewTeamsSelector_SplitTriggersByCategory;
		Valueable<bool> NewTeamsSelector_EnableFallback;
		Valueable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith;
		Valueable<double> NewTeamsSelector_UnclassifiedCategoryPercentage;
		Valueable<double> NewTeamsSelector_GroundCategoryPercentage;
		Valueable<double> NewTeamsSelector_NavalCategoryPercentage;
		Valueable<double> NewTeamsSelector_AirCategoryPercentage;

		Valueable<bool> IC_Flash;
		Valueable<int> VeteranFlashTimer;
	#pragma region Otamaa

		Nullable<ParticleTypeClass*> VeinholeParticle;
		Valueable<ParticleTypeClass*> DefaultVeinParticle;
		Valueable<AnimTypeClass*> DefaultSquidAnim;
		PhobosFixedString<0x19> NukeWarheadName;
		Nullable<bool> Building_PlacementPreview;
		NullableVector<float> AI_AutoSellHealthRatio;

		Valueable<AnimTypeClass*> CarryAll_LandAnim;
		Valueable<AnimTypeClass*> DropShip_LandAnim;
		Valueable<AnimTypeClass*> Aircraft_LandAnim;
		Valueable<AnimTypeClass*> Aircraft_TakeOffAnim;

		Valueable<bool> DisablePathfindFailureLog;
		Valueable<bool> CreateSound_PlayerOnly;

		int CivilianSideIndex;
		int SpecialCountryIndex;
		int NeutralCountryIndex;

		ValueableVector<BuildingTypeClass*> WallTowers;
		Valueable<bool> AutoAttackICedTarget;
		Nullable<float> AI_SpyMoneyStealPercent;
		Valueable<int> DoggiePanicMax;
		Valueable<int> HunterSeeker_Damage;

		Valueable<bool> AutoRepelAI;
		Valueable<bool> AutoRepelPlayer;
		Nullable<int> AIFriendlyDistance;

#ifdef COMPILE_PORTED_DP_FEATURES
		AircraftPutDataRules MyPutData;
#endif
	#pragma endregion

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Buildings { { -1,-1,-1 } }
			, RadApplicationDelay_Building { 0 }
			, MissingCameo { GameStrings::XXICON_SHP() }
			, AITargetTypesLists { }
			, AIScriptsLists { }
			, AIHousesLists { }
			, AIConditionsLists { }
			, AITriggersLists { }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }
			, JumpjetCrash_Rotate { true }
			, Storage_TiberiumIndex { -1 }
			, PlacementGrid_TranslucentLevel{ 0 }
			, BuildingPlacementPreview_TranslucentLevel { 3 }
			, Pips_Shield_Background_SHP {}
			, Pips_Shield_Building { { -1,-1,-1 } }
			, Pips_Shield_Building_Empty {}

			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }

			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, EnemyInsignia{ true }
			, ShowAllyDisguiseBlinking{ false }

			, SHP_SelectBrdSHP_INF { nullptr }
			, SHP_SelectBrdPAL_INF { CustomPalette::PaletteMode::Temperate }
			, SHP_SelectBrdSHP_UNIT { nullptr }
			, SHP_SelectBrdPAL_UNIT { CustomPalette::PaletteMode::Temperate }
			, UseSelectBrd{}

			, SelectBrd_Frame_Infantry { {0,0,0} }
			, SelectBrd_DrawOffset_Infantry { {0,0} }

			, SelectBrd_Frame_Unit { {3,3,3} }
			, SelectBrd_DrawOffset_Unit{ {0,0} }

			, SelectBrd_DefaultTranslucentLevel{ 0 }
			, SelectBrd_DefaultShowEnemy{ true }

			, RadWarhead_Detonate { false }
			, RadHasOwner { false }
			, RadHasInvoker { false }
			, IronCurtain_SyncDeploysInto { false }
			, ROF_RandomDelay { { 0 ,2 } }

			, ToolTip_Background_Color { {0, 0, 0} }
			, ToolTip_Background_Opacity { 100 }
			, ToolTip_Background_BlurSize { 0.f }
			, Crate_LandOnly { false }

			, GenericPrerequisitesData { }
			, NewTeamsSelector { false }
			, NewTeamsSelector_SplitTriggersByCategory { true }
			, NewTeamsSelector_EnableFallback { false }
			, NewTeamsSelector_MergeUnclassifiedCategoryWith { -1 }
			, NewTeamsSelector_UnclassifiedCategoryPercentage { 0.25 }
			, NewTeamsSelector_GroundCategoryPercentage { 0.25 }
			, NewTeamsSelector_NavalCategoryPercentage { 0.25 }
			, NewTeamsSelector_AirCategoryPercentage { 0.25 }
			
			, IC_Flash { true }
			, VeteranFlashTimer { 0 }

			, VeinholeParticle { }
			, DefaultVeinParticle { nullptr }
			, DefaultSquidAnim { nullptr }
			, NukeWarheadName { }
			, Building_PlacementPreview { }
			, AI_AutoSellHealthRatio { }
			, CarryAll_LandAnim { nullptr }
			, DropShip_LandAnim { nullptr }
			, Aircraft_LandAnim { nullptr }
			, Aircraft_TakeOffAnim { nullptr }
			, DisablePathfindFailureLog { false }
			, CreateSound_PlayerOnly { false }

			, CivilianSideIndex { -1 }
			, SpecialCountryIndex { -1 }
			, NeutralCountryIndex { -1 }

			, WallTowers { }
			, AutoAttackICedTarget { false }
			, AI_SpyMoneyStealPercent { }
			, DoggiePanicMax { 300 }
			, HunterSeeker_Damage { 1000 }
			, AutoRepelAI { true }
			, AutoRepelPlayer { true }
			, AIFriendlyDistance { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, MyPutData { }
#endif
		{ }

		virtual ~ExtData() = default;
		void Uninitialize() { }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void InitializeConstants() override;
		void InitializeAfterTypeData(RulesClass* pThis);

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override  { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI);
	static void LoadEarlyOptios(RulesClass* pThis, CCINIClass* pINI);
	static void LoadVeryEarlyBeforeAnyData(RulesClass* pRules, CCINIClass* pINI);
	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		if (auto pGlobal = Global()) {
			if (pGlobal->InvalidateIgnorable(ptr))
				return;

			pGlobal->InvalidatePointer(ptr, removed);
		}
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);

	static void FillDefaultPrerequisites(CCINIClass* pRules);
};
