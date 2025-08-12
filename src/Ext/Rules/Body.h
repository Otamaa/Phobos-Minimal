#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/TemplateDefB.h>

#include <Utilities/Debug.h>

#include <ScriptTypeClass.h>

#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutDataRules.h>

#include <New/AnonymousType/MultipleFactoryCaps.h>
#include <New/HugeBar.h>

class AITriggerTypeClass;
class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;
class DigitalDisplayTypeClass;
class CursorTypeClass;
class SelectBoxTypeClass;
class RulesExtData final
{
private:
	static std::unique_ptr<RulesExtData> Data;

public:
	static COMPILETIMEEVAL size_t Canary = 0x12341234;
	using base_type = RulesClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<Point3D> Pips_Shield { { -1, -1, -1 } };
	Valueable<Point3D> Pips_Shield_Buildings { { -1, -1, -1 } };
	Valueable<int> RadApplicationDelay_Building { 0 };
	Valueable<int> RadBuildingDamageMaxCount { -1 };

	PhobosFixedString<32u> MissingCameo { GameStrings::XXICON_SHP() };

	std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists { };
	std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists { };
	std::vector<std::vector<HouseTypeClass*>> AIHateHousesLists { };
	std::vector<std::vector<PhobosFixedString<0x18>>> AIConditionsLists { };
	std::vector<std::vector<AITriggerTypeClass*>> AITriggersLists { };
	std::vector<std::vector<HouseTypeClass*>> AIHousesLists { };

	Valueable<double> JumpjetCrash { 5.0 };
	Valueable<bool> JumpjetNoWobbles { false };
	Valueable<bool> JumpjetAllowLayerDeviation { true };
	Valueable<bool> JumpjetTurnToTarget { false };
	Valueable<bool> JumpjetCrash_Rotate { true };
	Valueable<bool> JumpjetClimbPredictHeight { false };
	Valueable<bool> JumpjetClimbWithoutCutOut { false };

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
	Valueable<Point2D> Pips_Generic_Size { { 4, 0 } };
	Valueable<Point2D> Pips_Generic_Buildings_Size { { 4, 2 } };
	Valueable<Point2D> Pips_Ammo_Size { { 4, 0 } };
	Valueable<Point2D> Pips_Ammo_Buildings_Size { { 4, 2 } };

	ValueableVector<int> Pips_Tiberiums_Frames {};
	NullableVector<int> Pips_Tiberiums_DisplayOrder {};

	Nullable<int> InfantryGainSelfHealCap {};
	Nullable<int> UnitsGainSelfHealCap {};

	Valueable<bool> EnemyInsignia { true };
	Valueable<bool> DrawInsigniaOnlyOnSelected {};
	Valueable<Point2D> DrawInsignia_AdjustPos_Infantry { { 5, 2 } };
	Valueable<Point2D> DrawInsignia_AdjustPos_Buildings { { 10, 6 } };
	Nullable<BuildingSelectBracketPosition> DrawInsignia_AdjustPos_BuildingsAnchor {};
	Valueable<Point2D> DrawInsignia_AdjustPos_Units { { 10, 6 } };
	Valueable<bool> DrawInsignia_UsePixelSelectionBracketDelta {};

	Valueable<AffectedHouse> DisguiseBlinkingVisibility { AffectedHouse::Owner | AffectedHouse::Allies };

	Valueable<SHPStruct*> SHP_SelectBrdSHP_INF { nullptr };
	CustomPalette SHP_SelectBrdPAL_INF { CustomPalette::PaletteMode::Temperate }; //

	Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT { nullptr };
	CustomPalette SHP_SelectBrdPAL_UNIT { CustomPalette::PaletteMode::Temperate }; //CustomPalette::PaletteMode::Temperate

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
	Valueable<bool> UseGlobalRadApplicationDelay { true };
	Valueable<bool> IronCurtain_KeptOnDeploy { true };
	Valueable<bool> ForceShield_KeptOnDeploy { false };
	Valueable<IronCurtainFlag> IronCurtain_EffectOnOrganics { IronCurtainFlag::Kill};
	Valueable<WarheadTypeClass*> IronCurtain_KillOrganicsWarhead { nullptr };
	Valueable<IronCurtainFlag> ForceShield_EffectOnOrganics { IronCurtainFlag::Ignore };
	Valueable<WarheadTypeClass*> ForceShield_KillOrganicsWarhead { nullptr };

	Valueable<PartialVector2D<int>> ROF_RandomDelay { { 0, 2 } };

	Valueable<ColorStruct> ToolTip_Background_Color { {0, 0, 0} };
	Valueable<int> ToolTip_Background_Opacity { 100 };
	Valueable<float> ToolTip_Background_BlurSize { 0.f };
	Valueable<bool> ToolTip_ExcludeSidebar { false };

	Valueable<bool> Crate_LandOnly { false };
	Valueable<int> UnitCrateVehicleCap { 50 };
	Valueable<int> FreeMCV_CreditsThreshold { 1500 };

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
	Valueable<int> DroppodTrailerSpawnDelay { 6 };
	Valueable<SHPStruct*> Droppod_ImageInfantry {};

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
	Valueable<bool> Building_PlacementPreview { true };
	Nullable<PartialVector3D<float>> AI_AutoSellHealthRatio {};

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

	Valueable<bool> AllowParallelAIQueues { true };
	Nullable<bool> ForbidParallelAIQueues_Infantry { };
	Nullable<bool> ForbidParallelAIQueues_Vehicle { };
	Nullable<bool> ForbidParallelAIQueues_Navy { };
	Nullable<bool> ForbidParallelAIQueues_Aircraft { };
	Nullable<bool> ForbidParallelAIQueues_Building { };

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

	Valueable<bool> DamageAirConsiderBridges { false };
	Valueable<bool> DiskLaserAnimEnabled { false };

	ValueableIdx<ColorScheme> AnimRemapDefaultColorScheme { 0 };

	Valueable<double> EngineerDamage { 0.0 };
	Valueable<bool> EngineerAlwaysCaptureTech { true };
	ValueableIdx<CursorTypeClass*> EngineerDamageCursor { 87 };
	Nullable<Mission> EMPAIRecoverMission { };
	ValueableIdx<ColorScheme> TimerBlinkColorScheme { 5 };

	bool AllowBypassBuildLimit[3] = { false };

	Valueable<bool> DegradeEnabled { false };
	Nullable<double> DegradePercentage {};
	Valueable<int> DegradeAmountNormal { 0 };
	Valueable<int> DegradeAmountConsumer { 1 };

	Valueable<bool> TogglePowerAllowed { false };
	Valueable<int> TogglePowerDelay { 45 };
	Valueable<int> TogglePowerIQ { -1 };

	Valueable<bool> GainSelfHealAllowMultiplayPassive { false };
	Valueable<bool> GainSelfHealFromPlayerControl { false };
	Valueable<bool> GainSelfHealFromAllies { false };

	Nullable<double> VeinsDamagingWeightTreshold {};
	CustomPalette VeinholePal { CustomPalette::PaletteMode::Temperate };
	Valueable<WarheadTypeClass*> Veinhole_Warhead { nullptr };
	Valueable<float> Veins_PerCellAmount { 1.0f };

	// firestorm
	Valueable<AnimTypeClass*> FirestormActiveAnim {};
	Valueable<AnimTypeClass*> FirestormIdleAnim {};
	Valueable<AnimTypeClass*> FirestormGroundAnim {};
	Valueable<AnimTypeClass*> FirestormAirAnim;
	Nullable<WarheadTypeClass*> FirestormWarhead {};
	Valueable<double> DamageToFirestormDamageCoefficient {};

	MultipleFactoryCaps MultipleFactoryCap {};
	Nullable<int> CloakHeight {};

	Valueable<bool> CanDrive { false };

	AnimTypeClass* DefaultAircraftDamagedSmoke { nullptr };
	Valueable<PartialVector3D<double>> AIDetectDisguise_Percent { { 1.0 , 1.0 , 1.0} };

	Valueable<bool> DisplayIncome { false };
	Valueable<bool> DisplayIncome_AllowAI { true };
	Valueable<AffectedHouse> DisplayIncome_Houses { AffectedHouse::All };

	Valueable<double> DisplayCreditsDelay { 0.02 };

	Valueable<bool> TypeSelectUseDeploy { true };
	Nullable<int> StartInMultiplayerUnitCost { };

	FPSCounterMode FPSCounter { FPSCounterMode::disabled };

	Valueable<int> SelectFlashTimer { 0 };

	Valueable<bool> WarheadParticleAlphaImageIsLightFlash { false };
	Valueable<int> CombatLightDetailLevel { 0 };
	Valueable<int> LightFlashAlphaImageDetailLevel { 0 };

	Valueable<AnimTypeClass*> Promote_Vet_Anim { nullptr };
	Valueable<AnimTypeClass*> Promote_Elite_Anim { nullptr };

	Valueable<ParticleSystemTypeClass*> DefaultGlobalParticleInstance { nullptr };

	Nullable<bool> PlacementGrid_TranslucencyWithPreview {};
	double Shield_ConditionGreen {};
	double Shield_ConditionYellow {};
	double Shield_ConditionRed {};

	double ConditionYellow_Terrain {};

	Valueable<bool> HeightShadowScaling { false };
	Valueable<double> HeightShadowScaling_MinScale { 0.0 };
	double AirShadowBaseScale_log { 0.693376137 };

	Valueable<int> VeinsAttack_interval { 2 };
	Valueable<int> BuildingFlameSpawnBlockFrames { 0 };
	AircraftPutDataRules MyPutData { };

	Nullable<SHPStruct*> PrimaryFactoryIndicator { };
	CustomPalette PrimaryFactoryIndicator_Palette { };

	BulletTypeClass* DefautBulletType { nullptr };

	ValueableIdx<SuperWeaponTypeClass> AIChronoSphereSW {};
	ValueableIdx<SuperWeaponTypeClass> AIChronoWarpSW {};

	Valueable<double> DamageOwnerMultiplier { 1.0 };
	Valueable<double> DamageAlliesMultiplier { 1.0 };
	Valueable<double> DamageEnemiesMultiplier { 1.0 };
	Nullable<double> DamageOwnerMultiplier_NotAffectsEnemies {};
	Nullable<double> DamageAlliesMultiplier_NotAffectsEnemies {};

	Valueable<bool> FactoryProgressDisplay { false };
	Valueable<bool> MainSWProgressDisplay { false };

	Valueable<bool> CombatAlert { false };
	Valueable<bool> CombatAlert_MakeAVoice { false };
	Valueable<bool> CombatAlert_IgnoreBuilding { true };
	Valueable<bool> CombatAlert_EVA { true };
	Valueable<bool> CombatAlert_UseFeedbackVoice { false };
	Valueable<bool> CombatAlert_UseAttackVoice { false };
	Valueable<bool> CombatAlert_SuppressIfInScreen { true };
	Valueable<int> CombatAlert_Interval { 150 };
	Valueable<bool> CombatAlert_SuppressIfAllyDamage { true };

	Valueable<int> SubterraneanHeight { -256 };

	Nullable<Vector3D<float>> VoxelLightSource {};
	Nullable<Vector3D<float>> VoxelShadowLightSource {};
	Valueable<bool> UseFixedVoxelLighting { false };

	std::vector<std::unique_ptr<HugeBar>> HugeBar_Config {};

	Valueable<bool> RegroupWhenMCVDeploy { true };
	Valueable<bool> AISellAllOnLastLegs { true };
	Valueable<int> AISellAllDelay { 0 };
	Valueable<bool> AIAllInOnLastLegs { true };
	Valueable<bool> RepairBaseNodes { };
	Valueable<bool> MCVRedeploysInCampaign { false };

	Valueable<double> AircraftLevelLightMultiplier { 1.0 };
	Valueable<double> AircraftCellLightLevelMultiplier { 0.0 };
	Valueable<double> JumpjetLevelLightMultiplier { 0.0 };
	Valueable<double> JumpjetCellLightLevelMultiplier { 0.0 };
	Valueable<bool> JumpjetCellLightApplyBridgeHeight { true };

	Nullable<int> AINormalTargetingDelay { };
	Nullable<int> PlayerNormalTargetingDelay { };
	Nullable<int> AIGuardAreaTargetingDelay { };
	Nullable<int> PlayerGuardAreaTargetingDelay { };
	Nullable<int> AIAttackMoveTargetingDelay { };
	Nullable<int> PlayerAttackMoveTargetingDelay { };
	Valueable<bool> DistributeTargetingFrame { false };
	Valueable<bool> DistributeTargetingFrame_AIOnly { true };

	Valueable<bool> CheckUnitBaseNormal { false };
	Valueable<bool> ExtendedBuildingPlacing { false };
	Valueable<bool> CheckExpandPlaceGrid { false };
	Valueable<CoordStruct> ExpandLandGridFrames { { 1, 0, 0 } };
	Valueable<CoordStruct> ExpandWaterGridFrames { { 1, 0, 0 } };

	Valueable<AnimTypeClass*> DefaultExplodeFireAnim {};
	Nullable<int> AISuperWeaponDelay {};

	Valueable<int> ChronoSpherePreDelay { 60 };
	Valueable<int> ChronoSphereDelay { 0 };
	Valueable<bool> EnablePowerSurplus { false };
	Valueable<bool> ShakeScreenUseTSCalculation { false };

	int SubterraneanSpeed { 19 };

	Valueable<bool> UnitIdleRotateTurret { false };
	Valueable<bool> UnitIdlePointToMouse { false };
	Valueable<int> UnitIdleActionRestartMin { 150 };
	Valueable<int> UnitIdleActionRestartMax { 300 };
	Valueable<int> UnitIdleActionIntervalMin { 150 };
	Valueable<int> UnitIdleActionIntervalMax { 450 };

	Valueable<bool> ExpandAircraftMission {};
	Valueable<bool> AssignUnitMissionAfterParadropped { false };

	Valueable<Leptons> VisualScatter_Min { Leptons(8) };
	Valueable<Leptons> VisualScatter_Max { Leptons(52) };

	struct LandTypeExt {
		Valueable<double> Bounce_Elasticity {};

		void LoadFromStream(PhobosStreamReader& Stm)
		{
			Stm.
				Process(Bounce_Elasticity);
		}

		void SaveToStream(PhobosStreamWriter& Stm)
		{
			Stm.
				Process(Bounce_Elasticity);
		}
	};

	std::array<LandTypeExt, 12u> LandTypeConfigExts {};
	HelperedVector<TechnoTypeClass*> Secrets {};

	Valueable<bool> NoQueueUpToEnter {};
	Valueable<bool> NoQueueUpToUnload {};

	Valueable<bool> NoRearm_UnderEMP { false };
	Valueable<bool> NoRearm_Temporal { false };
	Valueable<bool> NoReload_UnderEMP { false };
	Valueable<bool> NoReload_Temporal { false };

	Valueable<int> AttackMindControlledDelay {};

	Valueable<bool> Cameo_AlwaysExist { false };
	Valueable<SHPStruct*> Cameo_OverlayShapes { FileSystem::PIPS_SHP };
	Valueable<Vector3D<int>> Cameo_OverlayFrames { { -1, -1, -1 } };
	CustomPalette Cameo_OverlayPalette {};
	Valueable<bool> MergeBuildingDamage { false };

	Valueable<bool>ExpandBuildingQueue { false };

	Valueable<bool> AutoBuilding { false };
	Valueable<bool> AIAngerOnAlly { true };

	Valueable<bool> BuildingTypeSelectable {};
	Valueable<bool> BuildingWaypoint {};

	Valueable<bool> AIAutoDeployMCV { true };
	Valueable<bool> AISetBaseCenter { true };
	Valueable<bool> AIBiasSpawnCell { false };
	Valueable<bool> AIForbidConYard { false };

	Valueable<bool> JumpjetTilt {};

	Valueable<bool> NoTurret_TrackTarget { true };

	Valueable<bool> RecountBurst { false };
	Valueable<ColorStruct> AirstrikeLineColor { { 255, 0, 0 } };

	Valueable<bool> AmphibiousEnter {};
	Valueable<bool> AmphibiousUnload {};

	AnimTypeClass* XGRYMED1_ {};
	AnimTypeClass* XGRYMED2_ {};
	AnimTypeClass* XGRYSML1_ {};

	Valueable<bool> GiveMoneyIfStorageFull { false }; // vanilla behaviour

	struct ProneSpeedData {
		Valueable<double> Crawls { 0.33333334 };
		Valueable<double> NoCrawls { 1.5 };

		COMPILETIMEEVAL OPTIONALINLINE double getSpeed(bool crawls) const {
			return (crawls ? Crawls : NoCrawls).Get();
		}

		void Load(PhobosStreamReader& Stm)
		{
			Stm
				.Process(Crawls)
				.Process(NoCrawls)
				;
		}

		void Save(PhobosStreamWriter& Stm)
		{
			Stm
				.Process(Crawls)
				.Process(NoCrawls)
				;
		}

	} InfantrySpeedData {};

	Valueable<bool> ColorAddUse8BitRGB { false };
	Valueable<double> IronCurtain_ExtraTintIntensity {};
	Valueable<double> ForceShield_ExtraTintIntensity {};
	Valueable<double> DamagedSpeed { 0.75 };

	Valueable<SelectBoxTypeClass*> DefaultInfantrySelectBox {};
	Valueable<SelectBoxTypeClass*> DefaultUnitSelectBox {};
	Valueable<float> HarvesterDumpAmount { 0.0f };

	Valueable<bool> HarvesterScanAfterUnload {};
	Valueable<bool> AttackMove_Aggressive { false };
	Valueable<bool> AttackMove_UpdateTarget { false };
	Valueable<bool> Infantry_IgnoreBuildingSizeLimit { true };

	Valueable<bool> BattlePoints {};
	Valueable<int> BattlePoints_DefaultValue {};
	Nullable<int> BattlePoints_DefaultFriendlyValue {};

	Valueable<bool> SuperWeaponSidebar_AllowByDefault { true };

	bool CampaignAllowHarvesterScanUnderShroud[3] {};
	Valueable<AffectedHouse> BerzerkTargeting { AffectedHouse::All };

	Valueable<bool> AttackMove_IgnoreWeaponCheck { false };
	Nullable<bool> AttackMove_StopWhenTargetAcquired {};

	Valueable<int> PenetratesTransport_Level { 10 };
	Valueable<bool> DamageWallRecursivly { true };
	Valueable<int> AirstrikeLineZAdjust { 0 };
	Valueable<int> AdjacentWallDamage { 200 };
	Valueable<bool> InfantryAutoDeploy { false };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	void ReplaceVoxelLightSources();

	void Initialize();

	void LoadFromStream(PhobosStreamReader& Stm) {
		this->Serialize(Stm);
		this->ReplaceVoxelLightSources();
	}

	void SaveToStream(PhobosStreamWriter& Stm) {
		this->Serialize(Stm);
		this->ReplaceVoxelLightSources();
	}

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void s_LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void s_LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI);
	static void LoadEarlyOptios(RulesClass* pThis, CCINIClass* pINI);
	static void LoadVeryEarlyBeforeAnyData(RulesClass* pRules, CCINIClass* pINI);
	static void LoadEndOfAudioVisual(RulesClass* pRules, CCINIClass* pINI);

	static RulesExtData* Instance()
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

class NOVTABLE FakeRulesClass : public RulesClass
{
public:
	void _ReadColors(CCINIClass* pINI);
	void _ReadGeneral(CCINIClass* pINI);
};
static_assert(sizeof(FakeRulesClass) == sizeof(RulesClass), "Invalid Size !");