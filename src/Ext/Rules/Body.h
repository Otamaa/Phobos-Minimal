#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/TemplateDefB.h>

#include <Utilities/Debug.h>
#include <Utilities/VectorHelper.h>

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

#pragma region ClassMembers

	// ============================================================
	// Nested types (definitions only, no memory impact)
	// ============================================================
	struct LandTypeExt
	{
		Valueable<double> Bounce_Elasticity {};

		void LoadFromStream(PhobosStreamReader& Stm)
		{
			Stm.Process(Bounce_Elasticity);
		}

		void SaveToStream(PhobosStreamWriter& Stm)
		{
			Stm.Process(Bounce_Elasticity);
		}
	};

	struct ProneSpeedData
	{
		static COMPILETIMEEVAL double CRAWLING_SPEED_MULTIPLIER = std::bit_cast<double>(0x3FF5555555555555ull);
		static COMPILETIMEEVAL double PRONE_SPEED_MULTIPLIER = std::bit_cast<double>(0x3FF8000000000000ull);

		Valueable<double> Crawls { CRAWLING_SPEED_MULTIPLIER };
		Valueable<double> NoCrawls { PRONE_SPEED_MULTIPLIER };

		COMPILETIMEEVAL OPTIONALINLINE double getSpeed(bool crawls) const
		{
			return (crawls ? Crawls : NoCrawls).Get();
		}

		void Load(PhobosStreamReader& Stm)
		{
			Stm.Process(Crawls).Process(NoCrawls);
		}

		void Save(PhobosStreamWriter& Stm)
		{
			Stm.Process(Crawls).Process(NoCrawls);
		}
	};

	// ============================================================
	// Large aggregates (fixed-size strings, arrays, custom types)
	// ============================================================
	PhobosFixedString<32u> MissingCameo { GameStrings::XXICON_SHP() };
	PhobosFixedString<0x18> NukeWarheadName {};
	MultipleFactoryCaps MultipleFactoryCap {};
	AircraftPutDataRules MyPutData {};
	ProneSpeedData InfantrySpeedData {};
	std::array<LandTypeExt, 12u> LandTypeConfigExts {};

	CustomPalette SHP_SelectBrdPAL_INF { CustomPalette::PaletteMode::Temperate };
	CustomPalette SHP_SelectBrdPAL_UNIT { CustomPalette::PaletteMode::Temperate };
	CustomPalette VeinholePal { CustomPalette::PaletteMode::Temperate };
	CustomPalette PrimaryFactoryIndicator_Palette {};
	CustomPalette Cameo_OverlayPalette {};

	Valueable<CSFText> MessageSilosNeeded {};

	// ============================================================
	// Vectors of vectors (nested containers)
	// ============================================================
	std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists {};
	std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists {};
	std::vector<std::vector<HouseTypeClass*>> AIHateHousesLists {};
	std::vector<std::vector<PhobosFixedString<0x18>>> AIConditionsLists {};
	std::vector<std::vector<AITriggerTypeClass*>> AITriggersLists {};
	std::vector<std::vector<HouseTypeClass*>> AIHousesLists {};
	std::vector<std::unique_ptr<HugeBar>> HugeBar_Config {};

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	ValueableVector<int> Pips_Tiberiums_Frames {};
	NullableVector<int> Pips_Tiberiums_DisplayOrder {};
	ValueableVector<BuildingTypeClass*> HunterSeekerBuildings {};
	ValueableVector<BuildingTypeClass*> Bounty_Enablers {};
	ValueableVector<BuildingTypeClass*> WallTowers {};
	ValueableVector<TechnoTypeClass*> DropPodTypes {};
	ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes {};
	ValueableVector<DigitalDisplayTypeClass*> Infantry_DefaultDigitalDisplayTypes {};
	ValueableVector<DigitalDisplayTypeClass*> Vehicles_DefaultDigitalDisplayTypes {};
	ValueableVector<DigitalDisplayTypeClass*> Aircraft_DefaultDigitalDisplayTypes {};
	HelperedVector<TechnoTypeClass*> Secrets {};

	// ============================================================
	// 8-byte aligned: Plain pointers
	// ============================================================
	AnimTypeClass* DefaultAircraftDamagedSmoke { nullptr };
	BulletTypeClass* DefaultBulletType { nullptr };
	AnimTypeClass* XGRYMED1_ {};
	AnimTypeClass* XGRYMED2_ {};
	AnimTypeClass* XGRYSML1_ {};

	// ============================================================
	// Valueable<pointer> (8 bytes each)
	// ============================================================
	Valueable<WarheadTypeClass*> IronCurtain_KillOrganicsWarhead { nullptr };
	Valueable<WarheadTypeClass*> ForceShield_KillOrganicsWarhead { nullptr };
	Valueable<WarheadTypeClass*> Tiberium_ExplosiveWarhead { nullptr };
	Valueable<AnimTypeClass*> Tiberium_ExplosiveAnim { nullptr };
	Valueable<AnimTypeClass*> DropPodTrailer { nullptr };
	Valueable<SHPStruct*> Droppod_ImageInfantry {};
	Valueable<AnimTypeClass*> ElectricDeath { nullptr };
	Valueable<ParticleTypeClass*> DefaultVeinParticle { nullptr };
	Valueable<AnimTypeClass*> DefaultSquidAnim { nullptr };
	Valueable<AnimTypeClass*> CarryAll_LandAnim { nullptr };
	Valueable<AnimTypeClass*> DropShip_LandAnim { nullptr };
	Valueable<AnimTypeClass*> Aircraft_LandAnim { nullptr };
	Valueable<AnimTypeClass*> Aircraft_TakeOffAnim { nullptr };
	Valueable<AircraftTypeClass*> DefaultParaPlane { nullptr };
	Valueable<AnimTypeClass*> CloakAnim { nullptr };
	Valueable<AnimTypeClass*> DecloakAnim { nullptr };
	Valueable<WarheadTypeClass*> Veinhole_Warhead { nullptr };
	Valueable<AnimTypeClass*> FirestormActiveAnim {};
	Valueable<AnimTypeClass*> FirestormIdleAnim {};
	Valueable<AnimTypeClass*> FirestormGroundAnim {};
	Valueable<AnimTypeClass*> FirestormAirAnim {};
	Valueable<ParticleSystemTypeClass*> DefaultGlobalParticleInstance { nullptr };
	Valueable<AnimTypeClass*> DefaultExplodeFireAnim {};
	Valueable<AnimTypeClass*> Promote_Vet_Anim { nullptr };
	Valueable<AnimTypeClass*> Promote_Elite_Anim { nullptr };
	Valueable<SHPStruct*> SHP_SelectBrdSHP_INF { nullptr };
	Valueable<SHPStruct*> SHP_SelectBrdSHP_UNIT { nullptr };
	Valueable<SHPStruct*> Cameo_OverlayShapes { FileSystem::PIPS_SHP };
	Valueable<SelectBoxTypeClass*> DefaultInfantrySelectBox {};
	Valueable<SelectBoxTypeClass*> DefaultUnitSelectBox {};

	// ============================================================
	// Nullable<pointer> (pointer + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<SHPStruct*> Pips_Shield_Background_SHP {};
	Nullable<ParticleTypeClass*> VeinholeParticle {};
	Nullable<WarheadTypeClass*> FirestormWarhead {};
	Nullable<SHPStruct*> PrimaryFactoryIndicator {};

	// ============================================================
	// Nullable<Vector3D<float>> (~16 bytes)
	// ============================================================
	Nullable<Vector3D<float>> VoxelLightSource {};
	Nullable<Vector3D<float>> VoxelShadowLightSource {};

	// ============================================================
	// Nullable<PartialVector3D<float>> (~16 bytes)
	// ============================================================
	Nullable<PartialVector3D<float>> AI_AutoSellHealthRatio {};

	// ============================================================
	// Valueable<PartialVector3D<double>> (24 bytes)
	// ============================================================
	Valueable<PartialVector3D<double>> AIDetectDisguise_Percent { { 1.0, 1.0, 1.0 } };

	// ============================================================
	// Valueable<Point3D> / Valueable<CoordStruct> (12 bytes each)
	// ============================================================
	Valueable<Point3D> Pips_Shield { { -1, -1, -1 } };
	Valueable<Point3D> Pips_Shield_Buildings { { -1, -1, -1 } };
	Valueable<Point3D> Pips_Shield_Building { { -1, -1, -1 } };
	Valueable<Point3D> SelectBrd_Frame_Infantry { { 0, 0, 0 } };
	Valueable<Point3D> SelectBrd_Frame_Unit { { 3, 3, 3 } };
	Valueable<ColorStruct> ToolTip_Background_Color { { 0, 0, 0 } };
	Valueable<ColorStruct> AirstrikeLineColor { { 255, 0, 0 } };
	Valueable<CoordStruct> ExpandLandGridFrames { { 1, 0, 0 } };
	Valueable<CoordStruct> ExpandWaterGridFrames { { 1, 0, 0 } };
	Valueable<Vector3D<int>> Cameo_OverlayFrames { { -1, -1, -1 } };

	// ============================================================
	// Valueable<Point2D> / Valueable<PartialVector2D<int>> (8 bytes each)
	// ============================================================
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
	Valueable<Point2D> DrawInsignia_AdjustPos_Infantry { { 5, 2 } };
	Valueable<Point2D> DrawInsignia_AdjustPos_Buildings { { 10, 6 } };
	Valueable<Point2D> DrawInsignia_AdjustPos_Units { { 10, 6 } };
	Valueable<Point2D> SelectBrd_DrawOffset_Infantry { { 0, 0 } };
	Valueable<Point2D> SelectBrd_DrawOffset_Unit { { 0, 0 } };
	Valueable<PartialVector2D<int>> ROF_RandomDelay { { 0, 2 } };

	// ============================================================
	// Valueable<double> (8 bytes each)
	// ============================================================
	Valueable<double> JumpjetCrash { 5.0 };
	Valueable<double> StealthSpeakDelay { 1.0 };
	Valueable<double> SubterraneanSpeakDelay { 1.0 };
	Valueable<double> NewTeamsSelector_UnclassifiedCategoryPercentage { 0.25 };
	Valueable<double> NewTeamsSelector_GroundCategoryPercentage { 0.25 };
	Valueable<double> NewTeamsSelector_NavalCategoryPercentage { 0.25 };
	Valueable<double> NewTeamsSelector_AirCategoryPercentage { 0.25 };
	Valueable<double> EngineerDamage { 0.0 };
	Valueable<double> BerserkROFMultiplier { 0.5 };
	Valueable<double> AI_CostMult { 1.0 };
	Valueable<double> DeactivateDim_Powered { 0.5 };
	Valueable<double> DeactivateDim_EMP { 0.8 };
	Valueable<double> DeactivateDim_Operator { 0.65 };
	Valueable<double> DamageToFirestormDamageCoefficient {};
	Valueable<double> DisplayCreditsDelay { 0.02 };
	Valueable<double> AircraftLevelLightMultiplier { 1.0 };
	Valueable<double> AircraftCellLightLevelMultiplier { 0.0 };
	Valueable<double> JumpjetLevelLightMultiplier { 0.0 };
	Valueable<double> JumpjetCellLightLevelMultiplier { 0.0 };
	Valueable<double> IronCurtain_ExtraTintIntensity {};
	Valueable<double> ForceShield_ExtraTintIntensity {};
	Valueable<double> DamagedSpeed { 0.75 };
	Valueable<double> DamageOwnerMultiplier { 1.0 };
	Valueable<double> DamageAlliesMultiplier { 1.0 };
	Valueable<double> DamageEnemiesMultiplier { 1.0 };
	Valueable<double> PlayerGuardModeGuardRangeMultiplier { 2.0 };
	Valueable<double> AIGuardModeGuardRangeMultiplier { 2.0 };

	// ============================================================
	// Nullable<double> (double + bool + padding ≈ 16 bytes)
	// ============================================================
	Nullable<double> DegradePercentage {};
	Nullable<double> VeinsDamagingWeightTreshold {};
	Nullable<double> DamageOwnerMultiplier_Berzerk {};
	Nullable<double> DamageAlliesMultiplier_Berzerk {};
	Nullable<double> DamageEnemiesMultiplier_Berzerk {};
	Nullable<double> DamageOwnerMultiplier_NotAffectsEnemies {};
	Nullable<double> DamageAlliesMultiplier_NotAffectsEnemies {};

	// ============================================================
	// Nullable<int> (int + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<int> InfantryGainSelfHealCap {};
	Nullable<int> UnitsGainSelfHealCap {};
	Nullable<int> Pips_Shield_Building_Empty {};
	Nullable<int> CloakHeight {};
	Nullable<int> StartInMultiplayerUnitCost {};
	Nullable<int> AIFriendlyDistance {};
	Nullable<int> AINormalTargetingDelay {};
	Nullable<int> PlayerNormalTargetingDelay {};
	Nullable<int> AIGuardAreaTargetingDelay {};
	Nullable<int> PlayerGuardAreaTargetingDelay {};
	Nullable<int> AIAttackMoveTargetingDelay {};
	Nullable<int> PlayerAttackMoveTargetingDelay {};
	Nullable<int> AISuperWeaponDelay {};
	Nullable<int> BattlePoints_DefaultFriendlyValue {};
	Nullable<int> AIAdjacentMax_Campaign {};
	Valueable<int> PowerSurplus_ScaleToDrainAmount {};
	// ============================================================
	// Nullable<float> (float + bool + padding ≈ 8 bytes)
	// ============================================================
	Nullable<float> AI_SpyMoneyStealPercent {};

	// ============================================================
	// Nullable<enum/other 4-byte types> (~8 bytes)
	// ============================================================
	Nullable<BuildingSelectBracketPosition> DrawInsignia_AdjustPos_BuildingsAnchor {};
	Nullable<Mission> EMPAIRecoverMission {};

	// ============================================================
	// NullableIdx (~8 bytes each)
	// ============================================================
	NullableIdx<VocClass> DecloakSound {};

	// ============================================================
	// Nullable<bool> (bool + bool ≈ 2-4 bytes, group for alignment)
	// ============================================================
	Nullable<bool> UseSelectBrd {};
	Nullable<bool> ForbidParallelAIQueues_Infantry {};
	Nullable<bool> ForbidParallelAIQueues_Vehicle {};
	Nullable<bool> ForbidParallelAIQueues_Navy {};
	Nullable<bool> ForbidParallelAIQueues_Aircraft {};
	Nullable<bool> ForbidParallelAIQueues_Building {};
	Nullable<bool> PlacementGrid_TranslucencyWithPreview {};
	Nullable<bool> NoQueueUpToEnter_Buildings {};
	Nullable<bool> NoQueueUpToUnload_Buildings {};
	Nullable<bool> AttackMove_StopWhenTargetAcquired {};

	// ============================================================
	// Valueable<int> / ValueableIdx (4 bytes each)
	// ============================================================
	Valueable<int> RadApplicationDelay_Building { 0 };
	Valueable<int> RadBuildingDamageMaxCount { -1 };
	Valueable<int> Storage_TiberiumIndex { -1 };
	Valueable<int> PlacementGrid_TranslucentLevel { 0 };
	Valueable<int> BuildingPlacementPreview_TranslucentLevel { 3 };
	Valueable<int> SelectBrd_DefaultTranslucentLevel { 0 };
	Valueable<int> ToolTip_Background_Opacity { 100 };
	Valueable<int> UnitCrateVehicleCap { 50 };
	Valueable<int> FreeMCV_CreditsThreshold { 1500 };
	Valueable<int> NewTeamsSelector_MergeUnclassifiedCategoryWith { -1 };
	Valueable<int> VeteranFlashTimer { 0 };
	Valueable<int> OverlayExplodeThreshold { 0 };
	Valueable<int> RandomCrateMoney { 0 };
	Valueable<int> ChronoSparkleDisplayDelay { 24 };
	Valueable<int> DroppodTrailerSpawnDelay { 6 };
	Valueable<int> HunterSeekerDetonateProximity { 0 };
	Valueable<int> HunterSeekerDescendProximity { 0 };
	Valueable<int> HunterSeekerAscentSpeed { 0 };
	Valueable<int> HunterSeekerDescentSpeed { 0 };
	Valueable<int> HunterSeekerEmergeSpeed { 0 };
	Valueable<int> DoggiePanicMax { 300 };
	Valueable<int> HunterSeeker_Damage { 1000 };
	Valueable<int> ChainReact_Multiplier { 5 };
	Valueable<int> ChainReact_SpreadChance { 80 };
	Valueable<int> ChainReact_MinDelay { 15 };
	Valueable<int> ChainReact_MaxDelay { 120 };
	Valueable<int> DropPodMinimum { 0 };
	Valueable<int> DropPodMaximum { 0 };
	Valueable<int> DegradeAmountNormal { 0 };
	Valueable<int> DegradeAmountConsumer { 1 };
	Valueable<int> TogglePowerDelay { 45 };
	Valueable<int> TogglePowerIQ { -1 };
	Valueable<int> SelectFlashTimer { 0 };
	Valueable<int> CombatLightDetailLevel { 0 };
	Valueable<int> LightFlashAlphaImageDetailLevel { 0 };
	Valueable<int> VeinsAttack_interval { 2 };
	Valueable<int> BuildingFlameSpawnBlockFrames { 0 };
	Valueable<int> AISellAllDelay { 0 };
	Valueable<int> SubterraneanHeight { -256 };
	Valueable<int> UnitIdleActionRestartMin { 150 };
	Valueable<int> UnitIdleActionRestartMax { 300 };
	Valueable<int> UnitIdleActionIntervalMin { 150 };
	Valueable<int> UnitIdleActionIntervalMax { 450 };
	Valueable<int> ExtendedAircraftMissions_UnlandDamage { -1 };
	Valueable<int> AttackMindControlledDelay {};
	Valueable<int> BattlePoints_DefaultValue {};
	Valueable<int> PenetratesTransport_Level { 10 };
	Valueable<int> AdjacentWallDamage { 200 };
	Valueable<int> AirstrikeLineZAdjust { 0 };
	Valueable<int> AIAdjacentMax { -1 };
	Valueable<int> WarheadAnimZAdjust { -15 };
	Valueable<int> ChronoSpherePreDelay { 60 };
	Valueable<int> ChronoSphereDelay { 0 };

	ValueableIdx<ColorScheme> AnimRemapDefaultColorScheme { 0 };
	ValueableIdx<CursorTypeClass*> EngineerDamageCursor { 87 };
	ValueableIdx<ColorScheme> TimerBlinkColorScheme { 5 };
	ValueableIdx<SuperWeaponTypeClass> AIChronoSphereSW {};
	ValueableIdx<SuperWeaponTypeClass> AIChronoWarpSW {};
	ValueableIdx<VocClass> StartDistributionModeSound { -1 };
	ValueableIdx<VocClass> EndDistributionModeSound { -1 };
	ValueableIdx<VocClass> AddDistributionModeCommandSound { -1 };

	// ============================================================
	// Valueable<float> (4 bytes each)
	// ============================================================
	Valueable<float> ToolTip_Background_BlurSize { 0.f };
	Valueable<float> Veins_PerCellAmount { 1.0f };
	Valueable<float> HarvesterDumpAmount { 0.0f };

	// ============================================================
	// Valueable<Leptons> (4 bytes each, assuming Leptons = int)
	// ============================================================
	Valueable<Leptons> VisualScatter_Min { Leptons(8) };
	Valueable<Leptons> VisualScatter_Max { Leptons(52) };
	Valueable<Leptons> PlayerGuardModeGuardRangeAddend {};
	Valueable<Leptons> PlayerGuardModeGuardRangeMax { Leptons(4096) };
	Valueable<Leptons> PlayerGuardStationaryStray { Leptons(-256) };
	Valueable<Leptons> AIGuardModeGuardRangeAddend {};
	Valueable<Leptons> AIGuardModeGuardRangeMax { Leptons(4096) };
	Valueable<Leptons> AIGuardStationaryStray { Leptons(-256) };
	Valueable<Leptons> ChasingExtraRange {};
	Valueable<Leptons> PrefiringExtraRange {};
	Valueable<Leptons> ExtraRange_FirerMoving {};

	// ============================================================
	// Valueable<enum> (4 bytes each)
	// ============================================================
	Valueable<AffectedHouse> DisguiseBlinkingVisibility { AffectedHouse::Owner | AffectedHouse::Allies };
	Valueable<IronCurtainFlag> IronCurtain_EffectOnOrganics { IronCurtainFlag::Kill };
	Valueable<IronCurtainFlag> ForceShield_EffectOnOrganics { IronCurtainFlag::Ignore };
	Valueable<ChronoSparkleDisplayPosition> ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots };
	Valueable<BountyValueOption> Bounty_Value_Option { BountyValueOption::Value };
	Valueable<AffectedHouse> CreateSound_PlayerOnly { AffectedHouse::All };
	Valueable<AffectedHouse> DisplayIncome_Houses { AffectedHouse::All };
	Valueable<AffectedHouse> BerzerkTargeting { AffectedHouse::All };
	FPSCounterMode FPSCounter { FPSCounterMode::disabled };

	Valueable<Mission> ParadropMission { Mission::None };
	Valueable<Mission> AIParadropMission { Mission::Hunt };

	// ============================================================
	// Plain int (4 bytes each)
	// ============================================================
	int CivilianSideIndex { -1 };
	int SpecialCountryIndex { -1 };
	int NeutralCountryIndex { -1 };
	int SubterraneanSpeed { 19 };

	// ============================================================
	// Plain double (8 bytes each)
	// ============================================================
	double Shield_ConditionGreen {};
	double Shield_ConditionYellow {};
	double Shield_ConditionRed {};
	double ConditionYellow_Terrain {};
	double AirShadowBaseScale_log { 0.693376137 };
	Valueable<double> HeightShadowScaling_MinScale { 0.0 };

	// ============================================================
	// Valueable<bool> (1 byte each, packed together)
	// ============================================================
	Valueable<bool> JumpjetNoWobbles { false };
	Valueable<bool> JumpjetAllowLayerDeviation { true };
	Valueable<bool> JumpjetTurnToTarget { false };
	Valueable<bool> JumpjetCrash_Rotate { true };
	Valueable<bool> JumpjetClimbPredictHeight { false };
	Valueable<bool> JumpjetClimbWithoutCutOut { false };
	Valueable<bool> EnemyInsignia { true };
	Valueable<bool> DrawInsigniaOnlyOnSelected {};
	Valueable<bool> DrawInsignia_UsePixelSelectionBracketDelta {};
	Valueable<bool> SelectBrd_DefaultShowEnemy { true };
	Valueable<bool> RadWarhead_Detonate { false };
	Valueable<bool> RadHasOwner { false };
	Valueable<bool> RadHasInvoker { false };
	Valueable<bool> UseGlobalRadApplicationDelay { true };
	Valueable<bool> IronCurtain_KeptOnDeploy { true };
	Valueable<bool> ForceShield_KeptOnDeploy { false };
	Valueable<bool> AllowWeaponSelectAgainstWalls { false };
	Valueable<bool> ToolTip_ExcludeSidebar { false };
	Valueable<bool> Crate_LandOnly { false };
	Valueable<bool> NewTeamsSelector { false };
	Valueable<bool> NewTeamsSelector_SplitTriggersByCategory { true };
	Valueable<bool> NewTeamsSelector_EnableFallback { false };
	Valueable<bool> IC_Flash { true };
	Valueable<bool> Tiberium_DamageEnabled { false };
	Valueable<bool> Tiberium_HealEnabled { false };
	Valueable<bool> AlliedSolidTransparency { true };
	Valueable<bool> RepairStopOnInsufficientFunds { false };
	Valueable<bool> Units_UnSellable { true };
	Valueable<bool> DrawTurretShadow { false };
	Valueable<bool> Bounty_Display { false };
	Valueable<bool> Building_PlacementPreview { true };
	Valueable<bool> DisablePathfindFailureLog { false };
	Valueable<bool> CanTargetAI_IronCurtained { false };
	Valueable<bool> CanTarget_IronCurtained { true };
	Valueable<bool> AutoTarget_IronCurtained { true };
	Valueable<bool> AutoRepelAI { true };
	Valueable<bool> AutoRepelPlayer { true };
	Valueable<bool> TeamRetaliate { false };
	Valueable<bool> ChronoInfantryCrush { true };
	Valueable<bool> EnemyWrench { true };
	Valueable<bool> AllowParallelAIQueues { true };
	Valueable<bool> ReturnStructures { false };
	Valueable<bool> Cloak_KickOutParasite { true };
	Valueable<bool> DamageAirConsiderBridges { false };
	Valueable<bool> DiskLaserAnimEnabled { false };
	Valueable<bool> EngineerAlwaysCaptureTech { true };
	Valueable<bool> DegradeEnabled { false };
	Valueable<bool> TogglePowerAllowed { false };
	Valueable<bool> GainSelfHealAllowMultiplayPassive { false };
	Valueable<bool> GainSelfHealFromPlayerControl { false };
	Valueable<bool> GainSelfHealFromAllies { false };
	Valueable<bool> CanDrive { false };
	Valueable<bool> DisplayIncome { false };
	Valueable<bool> DisplayIncome_AllowAI { true };
	Valueable<bool> TypeSelectUseDeploy { true };
	Valueable<bool> WarheadParticleAlphaImageIsLightFlash { false };
	Valueable<bool> Promote_Vet_PlaySpotlight { false };
	Valueable<bool> Promote_Elite_PlaySpotlight { false };
	Valueable<bool> HeightShadowScaling { false };
	Valueable<bool> UseFixedVoxelLighting { false };
	Valueable<bool> RegroupWhenMCVDeploy { true };
	Valueable<bool> AISellAllOnLastLegs { true };
	Valueable<bool> AIAllInOnLastLegs { true };
	Valueable<bool> RepairBaseNodes {};
	Valueable<bool> MCVRedeploysInCampaign { false };
	Valueable<bool> JumpjetCellLightApplyBridgeHeight { true };
	Valueable<bool> DistributeTargetingFrame { false };
	Valueable<bool> DistributeTargetingFrame_AIOnly { true };
	Valueable<bool> CheckUnitBaseNormal { false };
	Valueable<bool> ExtendedBuildingPlacing { false };
	Valueable<bool> CheckExpandPlaceGrid { false };
	Valueable<bool> EnablePowerSurplus { false };
	Valueable<bool> ShakeScreenUseTSCalculation { false };
	Valueable<bool> UnitIdleRotateTurret { false };
	Valueable<bool> UnitIdlePointToMouse { false };
	Valueable<bool> ExpandAircraftMission {};
	Valueable<bool> AssignUnitMissionAfterParadropped { false };
	Valueable<bool> NoQueueUpToEnter {};
	Valueable<bool> NoQueueUpToUnload {};
	Valueable<bool> NoRearm_UnderEMP { false };
	Valueable<bool> NoRearm_Temporal { false };
	Valueable<bool> NoReload_UnderEMP { false };
	Valueable<bool> NoReload_Temporal { false };
	Valueable<bool> Cameo_AlwaysExist { false };
	Valueable<bool> MergeBuildingDamage { false };
	Valueable<bool> ExpandBuildingQueue { false };
	Valueable<bool> AutoBuilding { false };
	Valueable<bool> AIAngerOnAlly { false };
	Valueable<bool> BuildingTypeSelectable {};
	Valueable<bool> BuildingWaypoint {};
	Valueable<bool> AIAutoDeployMCV { true };
	Valueable<bool> AISetBaseCenter { true };
	Valueable<bool> AIBiasSpawnCell { false };
	Valueable<bool> AIForbidConYard { false };
	Valueable<bool> AINodeWallsOnly { false };
	Valueable<bool> AICleanWallNode { false };
	Valueable<bool> JumpjetTilt {};
	Valueable<bool> NoTurret_TrackTarget { true };
	Valueable<bool> RecountBurst { false };
	Valueable<bool> AmphibiousEnter {};
	Valueable<bool> AmphibiousUnload {};
	Valueable<bool> GiveMoneyIfStorageFull { false };
	Valueable<bool> ColorAddUse8BitRGB { false };
	Valueable<bool> HarvesterScanAfterUnload {};
	Valueable<bool> AttackMove_Aggressive { false };
	Valueable<bool> AttackMove_UpdateTarget { false };
	Valueable<bool> Infantry_IgnoreBuildingSizeLimit { true };
	Valueable<bool> BattlePoints {};
	Valueable<bool> SuperWeaponSidebar_AllowByDefault { true };
	Valueable<bool> AttackMove_IgnoreWeaponCheck { false };
	Valueable<bool> DamageWallRecursivly { true };
	Valueable<bool> InfantryAutoDeploy { false };
	Valueable<bool> EnablePassiveAcquireMode { false };
	Valueable<bool> UseRetintFix { true };
	Valueable<bool> AISellCapturedBuilding { true };
	Valueable<bool> PlayerGuardModePursuit { true };
	Valueable<bool> AIGuardModePursuit { true };
	Valueable<bool> IgnoreCenterMinorRadarEvent { false };
	Valueable<bool> IvanBombAttachToCenter { false };
	Valueable<bool> FallingDownTargetingFix { false };
	Valueable<bool> AIAirTargetingFix { false };
	Valueable<bool> SortCameoByName { false };
	Valueable<bool> AllowDeployControlledMCV { false };
	Valueable<bool> TypeSelectUseIFVMode { false };
	Valueable<bool> BuildingRadioLink_SyncOwner { true };
	Valueable<bool> ApplyPerTargetEffectsOnDetonate { true };
	Valueable<bool> ChasingExtraRange_CloseRangeOnly { true };
	Valueable<bool> PrefiringExtraRange_IncludeBurst { true };
	Valueable<bool> FiringAnim_Update { false };
	Valueable<bool> ExtendedPlayerRepair { false };
	Valueable<bool> UpdateInvisoImmediately { false };
	Valueable<bool> AutoTarget_NoThreatBuildings { false };
	Valueable<bool> AutoTargetAI_NoThreatBuildings { true };
	Valueable<bool> WalkLocomotorMakesWake { false };
	Valueable<bool> FactoryProgressDisplay { false };
	Valueable<bool> MainSWProgressDisplay { false };
	Valueable<bool> CombatAlert { false };
	Valueable<bool> CombatAlert_MakeAVoice { false };
	Valueable<bool> CombatAlert_IgnoreBuilding { true };
	Valueable<bool> CombatAlert_EVA { true };
	Valueable<bool> CombatAlert_UseFeedbackVoice { false };
	Valueable<bool> CombatAlert_UseAttackVoice { false };
	Valueable<bool> CombatAlert_SuppressIfInScreen { true };
	Valueable<bool> CombatAlert_SuppressIfAllyDamage { true };
	Valueable<int> CombatAlert_Interval { 150 };
	Valueable<bool> AllowBerzerkOnAllies { false };
	Valueable<bool> UnitsUnsellable { false };
	Valueable<bool> DisableOveroptimizationInTargeting { false };
	// ============================================================
	// Plain bool arrays and plain bools (at the very end)
	// ============================================================
	bool AllowBypassBuildLimit[3] = { false };
	bool CampaignAllowHarvesterScanUnderShroud[3] {};

	Valueable<bool> DrainMoneyDisplay { false };
	Valueable<AffectedHouse> DrainMoneyDisplay_Houses { AffectedHouse::All };
	Valueable<bool> DrainMoneyDisplay_OnTarget  { false };
	Valueable<bool> DrainMoneyDisplay_OnTarget_UseDisplayIncome { true };

	Valueable<bool> CylinderRangefinding { false };
	Nullable<bool> DefaultToGuardArea {} ;
#pragma endregion

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
	static void InitializeAfterAllRulesLoaded();

	COMPILETIMEEVAL FORCEDINLINE static RulesExtData* Instance()
	{
		return Data.get();
	}

	FORCEDINLINE static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);

	static std::unordered_map<VoxelStruct*, std::string > Owners;
};

class NOVTABLE FakeRulesClass : public RulesClass
{
public:
	void _ReadColors(CCINIClass* pINI);
	void _ReadGeneral(CCINIClass* pINI);
};
static_assert(sizeof(FakeRulesClass) == sizeof(RulesClass), "Invalid Size !");