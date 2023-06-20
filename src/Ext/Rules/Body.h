#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/TemplateDefB.h>

#include <Utilities/Debug.h>

#include <ScriptTypeClass.h>

#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataRules.h>


class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;

class RulesExt
{
public:
	class ExtData final : public Extension<RulesClass>
	{
	public:
		static constexpr size_t Canary = 0x12341234;
		using base_type = RulesClass;

	public:
		Valueable<Point3D> Pips_Shield;
		Valueable<Point3D> Pips_Shield_Buildings;
		Valueable<int> RadApplicationDelay_Building;
		PhobosFixedString<32u> MissingCameo;

		std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists;
		std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists;
		std::vector<std::vector<HouseTypeClass*>> AIHateHousesLists;
		std::vector<std::vector<std::string>> AIConditionsLists;
		std::vector<std::vector<AITriggerTypeClass*>> AITriggersLists;
		std::vector<std::vector<HouseTypeClass*>> AIHousesLists;

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
		Valueable<AffectedHouse> DisguiseBlinkingVisibility;

		Valueable<SHPStruct*> SHP_SelectBrdSHP_INF;
		Valueable<PaletteManager*> SHP_SelectBrdPAL_INF; //CustomPalette::PaletteMode::Temperate

		Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT;
		Valueable<PaletteManager*> SHP_SelectBrdPAL_UNIT; //CustomPalette::PaletteMode::Temperate

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

		Valueable<bool> Tiberium_DamageEnabled;
		Valueable<bool> Tiberium_HealEnabled;
		Valueable<WarheadTypeClass*> Tiberium_ExplosiveWarhead;
		Valueable<AnimTypeClass*> Tiberium_ExplosiveAnim;

		Valueable<int> OverlayExplodeThreshold;
		Valueable<bool> AlliedSolidTransparency;
		NullableIdx<VocClass> DecloakSound;

		Valueable<double> StealthSpeakDelay;
		Valueable<double> SubterraneanSpeakDelay;

		Valueable<int> RandomCrateMoney;

		Valueable<int> ChronoSparkleDisplayDelay;
		Valueable<ChronoSparkleDisplayPosition> ChronoSparkleBuildingDisplayPositions;
		Valueable<bool> RepairStopOnInsufficientFunds;
		Valueable<AnimTypeClass*> DropPodTrailer;
		Valueable<AnimTypeClass*> ElectricDeath;

		// hunter seeker
		ValueableVector<BuildingTypeClass*> HunterSeekerBuildings {};
		Valueable<int> HunterSeekerDetonateProximity;
		Valueable<int> HunterSeekerDescendProximity;
		Valueable<int> HunterSeekerAscentSpeed;
		Valueable<int> HunterSeekerDescentSpeed;
		Valueable<int> HunterSeekerEmergeSpeed;

		Valueable<bool> Units_UnSellable;

		Valueable<bool> DrawTurretShadow;
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
		Valueable<AffectedHouse> CreateSound_PlayerOnly;

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
		Valueable<double> BerserkROFMultiplier;
		Valueable<bool> TeamRetaliate;
		Valueable<double> AI_CostMult;

		Valueable<double> DeactivateDim_Powered;
		Valueable<double> DeactivateDim_EMP;
		Valueable<double> DeactivateDim_Operator;

		Valueable<int> ChainReact_Multiplier;
		Valueable<int> ChainReact_SpreadChance;
		Valueable<int> ChainReact_MinDelay;
		Valueable<int> ChainReact_MaxDelay;

		Valueable<bool> ChronoInfantryCrush;

		Valueable<bool> EnemyWrench;

		Valueable<AircraftTypeClass*> DefaultParaPlane;
		Valueable<int> DropPodMinimum;
		Valueable<int> DropPodMaximum;
		ValueableVector<TechnoTypeClass*> DropPodTypes;

		Valueable<bool> ReturnStructures { false };
		Valueable<CSFText> MessageSilosNeeded {};

		AircraftPutDataRules MyPutData;

#pragma endregion

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Buildings { { -1,-1,-1 } }
			, RadApplicationDelay_Building { 0 }
			, MissingCameo { GameStrings::XXICON_SHP() }

			, AITargetTypesLists { }
			, AIScriptsLists { }
			, AIHateHousesLists { }
			, AIConditionsLists { }
			, AITriggersLists { }
			, AIHousesLists { }

			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }
			, JumpjetCrash_Rotate { true }
			, Storage_TiberiumIndex { -1 }
			, PlacementGrid_TranslucentLevel { 0 }
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
			, EnemyInsignia { true }
			, DisguiseBlinkingVisibility { AffectedHouse::Owner }

			, SHP_SelectBrdSHP_INF { nullptr }
			, SHP_SelectBrdPAL_INF {}
			, SHP_SelectBrdSHP_UNIT { nullptr }
			, SHP_SelectBrdPAL_UNIT {}
			, UseSelectBrd {}

			, SelectBrd_Frame_Infantry { {0,0,0} }
			, SelectBrd_DrawOffset_Infantry { {0,0} }

			, SelectBrd_Frame_Unit { {3,3,3} }
			, SelectBrd_DrawOffset_Unit { {0,0} }

			, SelectBrd_DefaultTranslucentLevel { 0 }
			, SelectBrd_DefaultShowEnemy { true }

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

			, Tiberium_DamageEnabled { false }
			, Tiberium_HealEnabled { false }
			, Tiberium_ExplosiveWarhead { nullptr }
			, Tiberium_ExplosiveAnim { nullptr }
			, OverlayExplodeThreshold { 0 }
			, AlliedSolidTransparency { true }
			, DecloakSound { }
			, StealthSpeakDelay { 1.0 }
			, SubterraneanSpeakDelay { 1.0 }

			, RandomCrateMoney { 0 }

			, ChronoSparkleDisplayDelay { 24 }
			, ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots }
			, RepairStopOnInsufficientFunds { false }
			, DropPodTrailer { nullptr }
			, ElectricDeath { nullptr }
			, HunterSeekerDetonateProximity { 0 }
			, HunterSeekerDescendProximity { 0 }
			, HunterSeekerAscentSpeed { 0 }
			, HunterSeekerDescentSpeed { 0 }
			, HunterSeekerEmergeSpeed { 0 }

			, Units_UnSellable { true }
			, DrawTurretShadow { false }

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
			, CreateSound_PlayerOnly { AffectedHouse::All }

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
			, BerserkROFMultiplier { 0.5 }
			, TeamRetaliate { false }
			, AI_CostMult { 1.0 }
			, DeactivateDim_Powered { 0.5 }
			, DeactivateDim_EMP { 0.8 }
			, DeactivateDim_Operator { 0.65 }
			, ChainReact_Multiplier { 5 }
			, ChainReact_SpreadChance { 80 }
			, ChainReact_MinDelay { 15 }
			, ChainReact_MaxDelay { 120 }
			, ChronoInfantryCrush { true }

			, EnemyWrench { true }
			, DefaultParaPlane { nullptr }

			,DropPodMinimum {0}
			,DropPodMaximum {0}
			,DropPodTypes { }

			, MyPutData { }
		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);

		void Initialize();
		void InitializeAfterTypeData(RulesClass* pThis);

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

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

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);
	static void FillDefaultPrerequisites(CCINIClass* pRules);
};
