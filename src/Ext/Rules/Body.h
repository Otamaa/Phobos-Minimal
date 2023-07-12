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
class DigitalDisplayTypeClass;
class RulesExt
{
public:
	class ExtData final : public Extension<RulesClass>
	{
	public:
		static constexpr size_t Canary = 0x12341234;
		using base_type = RulesClass;

	public:
		Valueable<Point3D> Pips_Shield { { -1, -1, -1 } };
		Valueable<Point3D> Pips_Shield_Buildings { { -1, -1, -1 } };
		Valueable<int> RadApplicationDelay_Building { 0 };
		PhobosFixedString<32u> MissingCameo { GameStrings::XXICON_SHP() };

		std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists { };
		std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists { };
		std::vector<std::vector<HouseTypeClass*>> AIHateHousesLists { };
		std::vector<std::vector<std::string>> AIConditionsLists { };
		std::vector<std::vector<AITriggerTypeClass*>> AITriggersLists { };
		std::vector<std::vector<HouseTypeClass*>> AIHousesLists { };

		Valueable<double> JumpjetCrash { 5.0 };
		Valueable<bool> JumpjetNoWobbles { false };
		Valueable<bool> JumpjetAllowLayerDeviation { true };
		Valueable<bool> JumpjetTurnToTarget { false };
		Valueable<bool> JumpjetCrash_Rotate { true };

		Valueable<int> Storage_TiberiumIndex { -1 };
		Valueable<int> PlacementGrid_TranslucentLevel { 0 };
		Valueable<int> BuildingPlacementPreview_TranslucentLevel { 3 };

		Nullable<SHPStruct*> Pips_Shield_Background_SHP {};
		Valueable<Point3D> Pips_Shield_Building { { -1, -1, -1 } };
		Nullable<int> Pips_Shield_Building_Empty {};

		Valueable<Point2D> Pips_SelfHeal_Infantry { { 13, 20 } };
		Valueable<Point2D> Pips_SelfHeal_Units { { 13, 20 } };
		Valueable<Point2D> Pips_SelfHeal_Buildings { { 13, 20 } };
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset { { 25, -35 } };
		Valueable<Point2D> Pips_SelfHeal_Units_Offset { { 33, -32 } };
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset { { 15, 10 } };

		Nullable<int> InfantryGainSelfHealCap {};
		Nullable<int> UnitsGainSelfHealCap {};
		Valueable<bool> EnemyInsignia { true };
		Valueable<AffectedHouse> DisguiseBlinkingVisibility { AffectedHouse::Owner };

		Valueable<SHPStruct*> SHP_SelectBrdSHP_INF { nullptr };
		Valueable<PaletteManager*> SHP_SelectBrdPAL_INF { }; //CustomPalette::PaletteMode::Temperate

		Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT { nullptr };
		Valueable<PaletteManager*> SHP_SelectBrdPAL_UNIT { }; //CustomPalette::PaletteMode::Temperate

		Nullable<bool> UseSelectBrd {};

		Valueable<Point3D> SelectBrd_Frame_Infantry { {0, 0, 0} };
		Valueable<Point2D> SelectBrd_DrawOffset_Infantry { {0, 0} };
		Valueable<Point3D> SelectBrd_Frame_Unit { {3, 3, 3} };
		Valueable<Point2D> SelectBrd_DrawOffset_Unit { {0, 0} };

		Valueable<int> SelectBrd_DefaultTranslucentLevel { 0 };
		Valueable<bool> SelectBrd_DefaultShowEnemy { true };

		Valueable<bool> RadWarhead_Detonate { false };
		Valueable<bool> RadHasOwner { false };
		Valueable<bool> RadHasInvoker { false };
		Valueable<bool> IronCurtain_SyncDeploysInto { false };

		Valueable<PartialVector2D<int>> ROF_RandomDelay { { 0, 2 } };

		Valueable<ColorStruct> ToolTip_Background_Color { {0, 0, 0} };
		Valueable<int> ToolTip_Background_Opacity { 100 };
		Valueable<float> ToolTip_Background_BlurSize { 0.f };

		Valueable<bool> Crate_LandOnly { false };

		Valueable<bool> NewTeamsSelector { false };
		Valueable<bool> NewTeamsSelector_SplitTriggersByCategory { true };
		Valueable<bool> NewTeamsSelector_EnableFallback { false };
		Valueable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith { -1 };
		Valueable<double> NewTeamsSelector_UnclassifiedCategoryPercentage { 0.25 };
		Valueable<double> NewTeamsSelector_GroundCategoryPercentage { 0.25 };
		Valueable<double> NewTeamsSelector_NavalCategoryPercentage { 0.25 };
		Valueable<double> NewTeamsSelector_AirCategoryPercentage { 0.25 };

		Valueable<bool> IC_Flash { true };
		Valueable<int> VeteranFlashTimer { 0 };

		Valueable<bool> Tiberium_DamageEnabled { false };
		Valueable<bool> Tiberium_HealEnabled { false };
		Valueable<WarheadTypeClass*> Tiberium_ExplosiveWarhead { nullptr };
		Valueable<AnimTypeClass*> Tiberium_ExplosiveAnim { nullptr };

		Valueable<int> OverlayExplodeThreshold { 0 };
		Valueable<bool> AlliedSolidTransparency { true };
		NullableIdx<VocClass> DecloakSound { };

		Valueable<double> StealthSpeakDelay { 1.0 };
		Valueable<double> SubterraneanSpeakDelay { 1.0 };

		Valueable<int> RandomCrateMoney { 0 };

		Valueable<int> ChronoSparkleDisplayDelay { 24 };
		Valueable<ChronoSparkleDisplayPosition> ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots };
		Valueable<bool> RepairStopOnInsufficientFunds { false };
		Valueable<AnimTypeClass*> DropPodTrailer { nullptr };
		Valueable<AnimTypeClass*> ElectricDeath { nullptr };

		// hunter seeker
		ValueableVector<BuildingTypeClass*> HunterSeekerBuildings {};
		Valueable<int> HunterSeekerDetonateProximity { 0 };
		Valueable<int> HunterSeekerDescendProximity { 0 };
		Valueable<int> HunterSeekerAscentSpeed { 0 };
		Valueable<int> HunterSeekerDescentSpeed { 0 };
		Valueable<int> HunterSeekerEmergeSpeed { 0 };

		Valueable<bool> Units_UnSellable { true };

		Valueable<bool> DrawTurretShadow { false };
		ValueableVector<BuildingTypeClass*> Bounty_Enablers {};
		Valueable<bool> Bounty_Display { false };
		Valueable<BountyValueOption> Bounty_Value_Option { BountyValueOption::Value };

		Nullable<ParticleTypeClass*> VeinholeParticle {};
		Valueable<ParticleTypeClass*> DefaultVeinParticle { nullptr };
		Valueable<AnimTypeClass*> DefaultSquidAnim { nullptr };
		PhobosFixedString<0x18> NukeWarheadName {};
		Nullable<bool> Building_PlacementPreview {};
		NullableVector<float> AI_AutoSellHealthRatio {};

		Valueable<AnimTypeClass*> CarryAll_LandAnim { nullptr };
		Valueable<AnimTypeClass*> DropShip_LandAnim { nullptr };
		Valueable<AnimTypeClass*> Aircraft_LandAnim { nullptr };
		Valueable<AnimTypeClass*> Aircraft_TakeOffAnim { nullptr };

		Valueable<bool> DisablePathfindFailureLog { false };
		Valueable<AffectedHouse> CreateSound_PlayerOnly { AffectedHouse::All };

		int CivilianSideIndex { -1 };
		int SpecialCountryIndex { -1 };
		int NeutralCountryIndex { -1 };

		ValueableVector<BuildingTypeClass*> WallTowers {};
		Valueable<bool> AutoAttackICedTarget { false };
		Nullable<float> AI_SpyMoneyStealPercent { };
		Valueable<int> DoggiePanicMax { 300 };
		Valueable<int> HunterSeeker_Damage { 1000 };

		Valueable<bool> AutoRepelAI { true };
		Valueable<bool> AutoRepelPlayer { true };
		Nullable<int> AIFriendlyDistance {};
		Valueable<double> BerserkROFMultiplier { 0.5 };
		Valueable<bool> TeamRetaliate { false };
		Valueable<double> AI_CostMult { 1.0 };

		Valueable<double> DeactivateDim_Powered { 0.5 };
		Valueable<double> DeactivateDim_EMP { 0.8 };
		Valueable<double> DeactivateDim_Operator { 0.65 };

		Valueable<int> ChainReact_Multiplier { 5 };
		Valueable<int> ChainReact_SpreadChance { 80 };
		Valueable<int> ChainReact_MinDelay { 15 };
		Valueable<int> ChainReact_MaxDelay { 120 };

		Valueable<bool> ChronoInfantryCrush { true };

		Valueable<bool> EnemyWrench { true };

		Valueable<AircraftTypeClass*> DefaultParaPlane { nullptr };
		Valueable<int> DropPodMinimum { 0 };
		Valueable<int> DropPodMaximum { 0 };
		ValueableVector<TechnoTypeClass*> DropPodTypes {};

		Valueable<bool> ReturnStructures { false };
		Valueable<CSFText> MessageSilosNeeded {};

		Valueable<AnimTypeClass*> CloakAnim { nullptr };
		Valueable<AnimTypeClass*> DecloakAnim { nullptr };
		Valueable<bool> Cloak_KickOutParasite { true };

		ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes {};
		ValueableVector<DigitalDisplayTypeClass*> Infantry_DefaultDigitalDisplayTypes {};
		ValueableVector<DigitalDisplayTypeClass*> Vehicles_DefaultDigitalDisplayTypes {};
		ValueableVector<DigitalDisplayTypeClass*> Aircraft_DefaultDigitalDisplayTypes {};

		AircraftPutDataRules MyPutData { };

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);

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
};
