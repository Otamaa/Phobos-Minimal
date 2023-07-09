#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/CursorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/TheaterTypeClass.h>

#include <Ext/LineTrail/Body.h>

#include <New/AnonymousType/PassengerDeletionTypeClass.h>

#include <FileSystem.h>

#include <Misc/DynamicPatcher/Techno/ExtraFire/ExtraFireData.h>
#include <Misc/DynamicPatcher/Techno/DamageSelf/DamageSelfType.h>
#include <Misc/DynamicPatcher/Techno/AircraftDive/AircraftDiveData.h>
#include <Misc/DynamicPatcher/Techno/AircraftPut/AircraftPutData.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxData.h>
#include <Misc/DynamicPatcher/Techno/JumjetFaceTarget/JJFacingData.h>
#include <Misc/DynamicPatcher/Techno/Passengers/Passengers.h>
#include <Misc/DynamicPatcher/Techno/SpawnSupport/SpawnSupportData.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/FighterGuardArea/FighterGuardAreaData.h>

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <Utilities/MultiBoolFixedArray.h>

struct ImageStatusses
{
	VoxelStruct Images;
	bool Loaded;

	static void ReadVoxel(ImageStatusses& arg0, const char* const nKey, bool a4);

};

class Matrix3D;

class TechnoTypeExt
{
public:
	using ImageVector = std::vector<VoxelStruct>;

	static void InitImageData(ImageVector& nVec, size_t size = 1);
	static void ClearImageData(ImageVector& nVec, size_t pos = 0);

	class ExtData : public Extension<TechnoTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x11111111;
		using base_type = TechnoTypeClass;
#ifndef ENABLE_NEWEXT
		//static constexpr size_t ExtOffset = 0x35C;
		static constexpr size_t ExtOffset = 0xDF4;
#endif

	public:

		Valueable<bool> HealthBar_Hide { false };
		Valueable<CSFText> UIDescription {};
		Valueable<bool> LowSelectionPriority { false };
		PhobosFixedString<0x20> GroupAs { NONE_STR };
		Valueable<int> RadarJamRadius { 0 };
		Nullable<int> InhibitorRange {};
		Nullable<int> DesignatorRange {};

		//Enemy Inhibitors
		Nullable<int> SuppressorRange {};

		//Enemy Designator
		Nullable<int> AttractorRange {};

		Valueable<Leptons> MindControlRangeLimit;

		MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_EliteAbilities {};
		MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_VeteranAbilities {};

		ValueableIdxVector<ImmunityTypeClass> E_ImmuneToType {};
		ValueableIdxVector<ImmunityTypeClass> V_ImmuneToType {};
		ValueableIdxVector<ImmunityTypeClass> R_ImmuneToType {};

		Valueable<bool> Interceptor { false };
		Valueable<AffectedHouse> Interceptor_CanTargetHouses { AffectedHouse::Enemies };
		Promotable<Leptons> Interceptor_GuardRange {};
		Promotable<Leptons> Interceptor_MinimumGuardRange {};
		Valueable<int> Interceptor_Weapon { -1 };
		Nullable<bool> Interceptor_DeleteOnIntercept {};
		Nullable<WeaponTypeClass*> Interceptor_WeaponOverride {};
		Valueable<bool> Interceptor_WeaponReplaceProjectile { false };
		Valueable<bool> Interceptor_WeaponCumulativeDamage { false };
		Valueable<bool> Interceptor_KeepIntact { false };
		Valueable<bool> Interceptor_ConsiderWeaponRange { false };
		Valueable<bool> Interceptor_OnlyTargetBullet { false };

		Nullable<PartialVector3D<int>> TurretOffset {};
		Nullable<bool> TurretShadow {};
		ValueableVector<int> ShadowIndices {};
		Valueable<bool> Powered_KillSpawns { false };
		Valueable<bool> Spawn_LimitedRange { false };
		Valueable<int> Spawn_LimitedExtraRange { 0 };
		Nullable<int> Spawner_DelayFrames {};
		Nullable<bool> Harvester_Counted {};
		Valueable<bool> Promote_IncludeSpawns { false };
		Valueable<bool> ImmuneToCrit { false };
		Valueable<bool> MultiMindControl_ReleaseVictim { false };
		Valueable<int> CameoPriority { 0 };
		Valueable<bool> NoManualMove { false };
		Nullable<int> InitialStrength {};

		std::unique_ptr<PassengerDeletionTypeClass> PassengerDeletionType {};

		Valueable<bool> Death_NoAmmo { false };
		Valueable<int> Death_Countdown { 0 };
		Valueable<KillMethod> Death_Method { KillMethod::Explode };
		Valueable<bool> Death_WithMaster { false };

		ValueableVector<TechnoTypeClass*> AutoDeath_Nonexist {};
		Valueable<AffectedHouse> AutoDeath_Nonexist_House { AffectedHouse::Owner };
		Valueable<bool> AutoDeath_Nonexist_Any { false };
		Valueable<bool> AutoDeath_Nonexist_AllowLimboed { true };
		ValueableVector<TechnoTypeClass*> AutoDeath_Exist {};
		Valueable<AffectedHouse> AutoDeath_Exist_House { AffectedHouse::Owner };
		Valueable<bool> AutoDeath_Exist_Any { false };
		Valueable<bool> AutoDeath_Exist_AllowLimboed { true };
		Nullable<AnimTypeClass*> AutoDeath_VanishAnimation {};

		Valueable<SlaveReturnTo> Slaved_ReturnTo { SlaveReturnTo::Killer };
		Valueable<ShieldTypeClass*> ShieldType { nullptr };

		NullablePromotable<AnimTypeClass*> WarpOut {};
		NullablePromotable<AnimTypeClass*> WarpIn {};
		NullablePromotable<AnimTypeClass*> WarpAway {};
		NullablePromotable<bool> ChronoTrigger {};
		NullablePromotable<int> ChronoDistanceFactor {};
		NullablePromotable<int> ChronoMinimumDelay {};
		NullablePromotable<int> ChronoRangeMinimum {};
		NullablePromotable<int> ChronoDelay {};

		Promotable<WeaponTypeClass*> WarpInWeapon { nullptr };
		NullablePromotable<WeaponTypeClass*> WarpInMinRangeWeapon { nullptr };
		Promotable<WeaponTypeClass*> WarpOutWeapon { nullptr };
		Promotable<bool> WarpInWeapon_UseDistanceAsDamage { false };

		ValueableVector<AnimTypeClass*> OreGathering_Anims {};
		ValueableVector<int> OreGathering_Tiberiums {};
		ValueableVector<int> OreGathering_FramesPerDir {};

		std::vector<std::vector<CoordStruct>> WeaponBurstFLHs {};
		std::vector<std::vector<CoordStruct>> EliteWeaponBurstFLHs {};

		Valueable<bool> DestroyAnim_Random { true };
		Valueable<bool> NotHuman_RandomDeathSequence { false };

		Nullable<InfantryTypeClass*> DefaultDisguise {};

		Nullable<int> OpenTopped_RangeBonus {};
		Nullable<float> OpenTopped_DamageMultiplier {};
		Nullable<int> OpenTopped_WarpDistance {};
		Valueable<bool> OpenTopped_IgnoreRangefinding { false };
		Valueable<bool> OpenTopped_AllowFiringIfDeactivated { true };
		Valueable<bool> OpenTopped_ShareTransportTarget { true };

		Valueable<bool> AutoFire { false };
		Valueable<bool> AutoFire_TargetSelf { false };

		Valueable<bool> NoSecondaryWeaponFallback { false };
		Valueable<bool> NoSecondaryWeaponFallback_AllowAA { false };

		Valueable<int> NoAmmoWeapon { -1 };
		Valueable<int> NoAmmoAmount { 0 };

		Nullable<bool> JumpjetAllowLayerDeviation {};
		Nullable<bool> JumpjetTurnToTarget {};
		Nullable<bool> JumpjetCrash_Rotate {};

		Valueable<bool> DeployingAnim_AllowAnyDirection { false };
		Valueable<bool> DeployingAnim_KeepUnitVisible { false };
		Valueable<bool> DeployingAnim_ReverseForUndeploy { true };
		Valueable<bool> DeployingAnim_UseUnitDrawer { true };

		Nullable<SelfHealGainType> SelfHealGainType {};

		Valueable<int> ForceWeapon_Naval_Decloaked { -1 };
		Valueable<int> ForceWeapon_UnderEMP { -1 };
		Valueable<int> ForceWeapon_Cloaked { -1 };
		Valueable<int> ForceWeapon_Disguised { -1 };
		Valueable<bool> ImmuneToEMP { false };
		Valueable<bool> Ammo_Shared { false };
		Valueable<int> Ammo_Shared_Group { -1 };
		Valueable<bool> Passengers_SyncOwner { false };
		Valueable<bool> Passengers_SyncOwner_RevertOnExit { true };

		Valueable<bool> Aircraft_DecreaseAmmo { true };

		struct LaserTrailDataEntry
		{
			int idxType;
			CoordStruct FLH;
			bool IsOnTurret;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

			// For some Fcking unknown reason `emplace_back` doesnt knowh the default contructor for this
			LaserTrailDataEntry(int nIdx, const CoordStruct& nFlh, bool OnTur) :
				idxType { nIdx }
				, FLH { nFlh }
				, IsOnTurret { OnTur }
			{
			}

			LaserTrailDataEntry() :
				idxType { -1 }
				, FLH { 0,0,0 }
				, IsOnTurret { false }
			{
			}

			//virtual ~LaserTrailDataEntry() = default;
			~LaserTrailDataEntry() = default;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<LaserTrailDataEntry> LaserTrailData {};
		Valueable<CSFText> EnemyUIName {};
		Valueable<bool> UseDisguiseMovementSpeed { false };

		Valueable<bool> DrawInsignia { true };
		Promotable<SHPStruct*> Insignia { nullptr };
		Promotable<int> InsigniaFrame { -1 };
		Nullable<bool> Insignia_ShowEnemy {};
		Valueable<Point3D> InsigniaFrames { { -1, -1, -1 } };
		Valueable<CoordStruct> InsigniaDrawOffset { {0, 0, 0} };

		Nullable<PartialVector2D<double>> InitialStrength_Cloning {};

		Nullable<SHPStruct*> SHP_SelectBrdSHP {};
		Valueable<PaletteManager*> SHP_SelectBrdPAL {}; //CustomPalette::PaletteMode::Temperate
		Nullable<bool> UseCustomSelectBrd {};
		Nullable<Point3D> SelectBrd_Frame {};
		Nullable<Point2D> SelectBrd_DrawOffset {};
		Nullable<int> SelectBrd_TranslucentLevel {};
		Nullable<bool> SelectBrd_ShowEnemy {};

		Nullable<CoordStruct> PronePrimaryFireFLH {};
		Nullable<CoordStruct> ProneSecondaryFireFLH {};
		Nullable<CoordStruct> DeployedPrimaryFireFLH {};
		Nullable<CoordStruct> DeployedSecondaryFireFLH {};

		Nullable<CoordStruct> E_PronePrimaryFireFLH {};
		Nullable<CoordStruct> E_ProneSecondaryFireFLH {};
		Nullable<CoordStruct> E_DeployedPrimaryFireFLH {};
		Nullable<CoordStruct> E_DeployedSecondaryFireFLH {};

		std::vector<std::vector<CoordStruct>> CrouchedWeaponBurstFLHs {};
		std::vector<std::vector<CoordStruct>> EliteCrouchedWeaponBurstFLHs {};
		std::vector<std::vector<CoordStruct>> DeployedWeaponBurstFLHs {};
		std::vector<std::vector<CoordStruct>> EliteDeployedWeaponBurstFLHs {};
		std::vector<CoordStruct> AlternateFLHs {};

		Nullable<bool> IronCurtain_SyncDeploysInto {};
		Valueable<IronCurtainFlag> IronCurtain_Effect { IronCurtainFlag::Default };
		Nullable<WarheadTypeClass*> IronCurtain_KillWarhead {};

		ValueableIdx<VoxClass> EVA_Sold { -1 };
		ValueableIdx<VocClass> SellSound { -1};

		Valueable<bool> MobileRefinery { false };
		Valueable<int> MobileRefinery_TransRate { 30 };
		Valueable<float> MobileRefinery_CashMultiplier { 1.0 };
		Valueable<int> MobileRefinery_AmountPerCell { 0 };
		ValueableVector<int> MobileRefinery_FrontOffset {};
		ValueableVector<int> MobileRefinery_LeftOffset {};
		Valueable<bool> MobileRefinery_Display { true };
		Valueable<ColorStruct> MobileRefinery_DisplayColor { { 57, 197, 187 } };
		ValueableVector<AnimTypeClass*> MobileRefinery_Anims {};
		Valueable<bool> MobileRefinery_AnimMove { false };
		Valueable<bool> Explodes_KillPassengers { true };

		Nullable<int> DeployFireWeapon {};
		Nullable<WeaponTypeClass*> RevengeWeapon {};
		Valueable<AffectedHouse> RevengeWeapon_AffectsHouses { AffectedHouse::All };

		Valueable<TargetZoneScanType> TargetZoneScanType { TargetZoneScanType::Same };
		Nullable<bool> GrapplingAttack {};

		Valueable<bool> FacingRotation_Disable { false };
		Valueable<bool> FacingRotation_DisalbeOnEMP { false };
		Valueable<bool> FacingRotation_DisalbeOnDeactivated { false };
		Valueable<bool> FacingRotation_DisableOnDriverKilled { true };


		Valueable<bool> DontShake { true };
		NullableIdx<VocClass>DiskLaserChargeUp { };

		Nullable<AnimTypeClass*>DrainAnimationType { };
		Nullable<int> DrainMoneyFrameDelay { };
		Nullable<int> DrainMoneyAmount { };
		Valueable<bool> DrainMoney_Display { false };
		Valueable<AffectedHouse> DrainMoney_Display_Houses { AffectedHouse::All };
		Valueable<bool> DrainMoney_Display_AtFirer { true };
		Valueable<Point2D> DrainMoney_Display_Offset { { 0, 0 } };

		Nullable<float> TalkBubbleTime { };
		Nullable <int> AttackingAircraftSightRange { };
		NullableIdx<VoxClass>SpyplaneCameraSound { };
		Nullable<int> ParadropRadius { };
		Nullable<int> ParadropOverflRadius { };
		Valueable<bool> Paradrop_DropPassangers { true };
		Valueable<int> Paradrop_MaxAttempt { 5 };

		Valueable<bool> IsCustomMissile { false };
		Valueable<RocketStruct> CustomMissileData { };
		Valueable<WarheadTypeClass*> CustomMissileWarhead { nullptr };
		Valueable<WarheadTypeClass*> CustomMissileEliteWarhead { nullptr };
		Valueable<AnimTypeClass*> CustomMissileTakeoffAnim { nullptr };
		Valueable<AnimTypeClass*> CustomMissilePreLauchAnim { nullptr };
		Valueable<AnimTypeClass*> CustomMissileTrailerAnim { nullptr };
		Valueable<int> CustomMissileTrailerSeparation { 3 };
		Valueable<WeaponTypeClass*> CustomMissileWeapon { nullptr };
		Valueable<WeaponTypeClass*> CustomMissileEliteWeapon { nullptr };
		Promotable<bool> CustomMissileRaise { true };
		Nullable<Point2D> CustomMissileOffset { };

		Valueable<bool> Draw_MindControlLink { true };

		NullableVector<int> Overload_Count { };
		NullableVector<int> Overload_Damage { };
		NullableVector<int> Overload_Frames { };
		NullableIdx<VocClass> Overload_DeathSound { };
		Nullable<ParticleSystemTypeClass*> Overload_ParticleSys { };
		Nullable<int> Overload_ParticleSysCount { };
		Nullable<WarheadTypeClass*> Overload_Warhead { };

		Nullable<AnimTypeClass*> Landing_Anim { };
		Valueable<AnimTypeClass*> Landing_AnimOnWater { nullptr };
		Nullable<AnimTypeClass*> TakeOff_Anim { };
		std::vector<CoordStruct> HitCoordOffset { };
		Valueable<bool> HitCoordOffset_Random  { true };
		Promotable<WeaponTypeClass*> DeathWeapon { };
		Valueable<WeaponTypeClass*> CrashWeapon_s { nullptr };
		Promotable<WeaponTypeClass*> CrashWeapon { };
		Valueable<bool> DeathWeapon_CheckAmmo { false };
		Valueable<bool> Disable_C4WarheadExp { false };
		Valueable<double> CrashSpinLevelRate { 1.0 };
		Valueable<double> CrashSpinVerticalRate { 1.0 };
		ValueableIdx<VocClass> ParasiteExit_Sound  { -1 };

		Nullable<SHPStruct*> PipShapes01 { };
		Nullable<SHPStruct*> PipShapes02 { };
		Nullable<SHPStruct*> PipGarrison { };
		Valueable<int> PipGarrison_FrameIndex { 0 };
		Valueable<PaletteManager*> PipGarrison_Palette { 0 }; //CustomPalette::PaletteMode::Default

		Valueable<bool> HealthNumber_Show { false };
		Valueable<bool> HealthNumber_Percent { false };
		Nullable<Point2D> Healnumber_Offset { };
		Nullable<SHPStruct*> HealthNumber_SHP { };
		Nullable<Point2D> Healnumber_Decrement { };
		Nullable<SHPStruct*> HealthBarSHP { };
		Nullable<SHPStruct*> HealthBarSHP_Selected { };
		Valueable<int> HealthBarSHPBracketOffset { 0 };
		Valueable<Point3D> HealthBarSHP_HealthFrame { { 18, 16, 17 } };
		Valueable<PaletteManager*> HealthBarSHP_Palette { }; //CustomPalette::PaletteMode::Temperate

		Valueable<Point2D> HealthBarSHP_PointOffset { { 0, 0 } };
		Valueable<bool> HealthbarRemap { false };

		Nullable<SHPStruct*> GClock_Shape { };
		Nullable<int> GClock_Transculency { };
		Valueable<PaletteManager*> GClock_Palette { }; //CustomPalette::PaletteMode::Default

		Valueable<bool> ROF_Random { true };
		Nullable<Point2D> Rof_RandomMinMax { };

		ValueableIdx<VoxClass> Eva_Complete { -1 };
		ValueableIdx<VocClass> VoiceCreate { -1 };
		Valueable<bool>CreateSound_Enable { true };

		Valueable<bool> SlaveFreeSound_Enable { true };
		NullableIdx<VocClass>SlaveFreeSound { };
		Valueable<bool> NoAirportBound_DisableRadioContact { false };
		Nullable<AnimTypeClass*> SinkAnim { };
		Nullable<double> Tunnel_Speed { };
		Nullable<HoverTypeClass*> HoverType { };

		Valueable<bool> Gattling_Overload { false };
		Nullable<int> Gattling_Overload_Damage { };
		Nullable<int> Gattling_Overload_Frames { };
		NullableIdx<VocClass> Gattling_Overload_DeathSound { };
		Nullable<ParticleSystemTypeClass*> Gattling_Overload_ParticleSys { };
		Nullable<int> Gattling_Overload_ParticleSysCount { };
		Nullable<WarheadTypeClass*>  Gattling_Overload_Warhead { };

		Valueable<bool> IsHero { false };
		Valueable<bool> IsDummy { false };

		ValueableVector<WeaponTypeClass*> FireSelf_Weapon { };
		ValueableVector<int> FireSelf_ROF { };
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_GreenHeath { };
		ValueableVector<int> FireSelf_ROF_GreenHeath { };
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_YellowHeath { };
		ValueableVector<int> FireSelf_ROF_YellowHeath { };
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_RedHeath { };
		ValueableVector<int> FireSelf_ROF_RedHeath { };

		Nullable<bool> AllowFire_IroncurtainedTarget { };
		Valueable<int> EngineerCaptureDelay { 0 };

		Nullable<ColorStruct> CommandLine_Move_Color { };
		Nullable<ColorStruct> CommandLine_Attack_Color { };
		Nullable<bool> CloakMove { };
		Nullable<bool> PassiveAcquire_AI { };
		Valueable<bool> TankDisguiseAsTank { false };
		ValueableVector<ObjectTypeClass*> DisguiseDisAllowed { };
		Valueable<bool> ChronoDelay_Immune { false };
		std::vector<LineTrailData> LineTrailData { };
		Nullable<int> PoseDir { };
		Valueable<bool> Firing_IgnoreGravity { };

		Promotable<int> Survivors_PassengerChance { -1 };
		Nullable<CoordStruct> Spawner_SpawnOffsets { };
		Valueable<bool> Spawner_SpawnOffsets_OverrideWeaponFLH { false };
		Nullable<UnitTypeClass*> Unit_AI_AlternateType { };

		Nullable<bool> ConsideredNaval { };
		Nullable<bool> ConsideredVehicle { };

		// Ares 0.1
		ValueableIdxVector<TheaterTypeClass> Prerequisite_RequiredTheaters {};
		std::vector<ValueableVector<int>> Prerequisite {};
		Valueable<int> Prerequisite_Lists { 1 };
		ValueableVector<int> Prerequisite_Negative {};

		Nullable<int> Riparius_FrameIDx { };
		Nullable<int> Cruentus_FrameIDx { };
		Nullable<int> Vinifera_FrameIDx { };
		Nullable<int> Aboreus_FrameIDx { };

		Promotable<int> CrushLevel {};
		Promotable<int> CrushableLevel {};
		Promotable<int> DeployCrushableLevel {};

		Valueable<float> Experience_VictimMultiple { 1.0f };
		Valueable<float> Experience_KillerMultiple { 1.0f };

		Nullable<Leptons> NavalRangeBonus { };

		Nullable<bool> AI_LegalTarget { };
		Valueable<bool> DeployFire_UpdateFacing { true };
		Nullable<TechnoTypeClass*> Fake_Of { };
		Valueable<bool> CivilianEnemy { false };
		Valueable<bool> ImmuneToBerserk { false };
		Valueable<double> Berzerk_Modifier { 1.0 };

		Valueable<bool> IgnoreToProtect { false };
		Valueable<int> TargetLaser_Time { 15 };
		ValueableVector<int> TargetLaser_WeaponIdx { };

		Nullable<bool> CurleyShuffle { };

		Valueable<bool> PassengersGainExperience { false };
		Valueable<bool> ExperienceFromPassengers { true };
		Valueable<double> PassengerExperienceModifier { 1.0 };
		Valueable<double> MindControlExperienceSelfModifier { 0.0 };
		Valueable<double> MindControlExperienceVictimModifier { 1.0 };
		Valueable<double> SpawnExperienceOwnerModifier { 0.0 };
		Valueable<double> SpawnExperienceSpawnModifier { 1.0 };
		Valueable<bool> ExperienceFromAirstrike { false };
		Valueable<double> AirstrikeExperienceModifier { 1.0 };

		Valueable<bool> Promote_IncludePassengers { false };
		ValueableIdx<VoxClass> Promote_Elite_Eva { -1 };
		ValueableIdx<VoxClass> Promote_Vet_Eva { -1 };
		NullableIdx<VocClass> Promote_Elite_Sound { };
		NullableIdx<VocClass> Promote_Vet_Sound { };
		Nullable<int> Promote_Elite_Flash { };
		Nullable<int> Promote_Vet_Flash { };

		Valueable<TechnoTypeClass*> Promote_Vet_Type { nullptr };
		Valueable<TechnoTypeClass*> Promote_Elite_Type { nullptr };

		Valueable<AnimTypeClass*> Promote_Vet_Anim { nullptr };
		Valueable<AnimTypeClass*> Promote_Elite_Anim { nullptr };

		Valueable<double> Promote_Vet_Exp { 0.0 };
		Valueable<double> Promote_Elite_Exp { 0.0 };
		Nullable<DirType8> DeployDir { };

		ValueableVector<TechnoTypeClass*> PassengersWhitelist { };
		ValueableVector<TechnoTypeClass*> PassengersBlacklist { };

		Valueable<bool> NoManualUnload { false };
		Valueable<bool> NoSelfGuardArea { false };
		Valueable<bool> NoManualFire { false };
		Valueable<bool> NoManualEnter { false };
		Valueable<bool> NoManualEject { false };

		Valueable<bool> Passengers_BySize { true };
		Nullable<bool> Crashable { };

		Valueable<TechnoTypeClass*> Convert_Deploy { nullptr };
		Valueable<TechnoTypeClass*> Convert_Script { nullptr };

		Nullable<Leptons> Harvester_LongScan { };
		Nullable<Leptons> Harvester_ShortScan { };
		Nullable<Leptons> Harvester_ScanCorrection { };

		Nullable<int> Harvester_TooFarDistance { };
		Nullable<int> Harvester_KickDelay { };

		Nullable<int> TurretRot { };
		Valueable<UnitTypeClass*> WaterImage { nullptr };

		Valueable<int> FallRate_Parachute { 1 };
		Valueable<int> FallRate_NoParachute { 1 };
		Nullable<int>  FallRate_ParachuteMax { };
		Nullable<int> FallRate_NoParachuteMax { };

		ImageVector BarrelImageData { };
		ImageVector TurretImageData { };
		VoxelStruct SpawnAltData { nullptr , nullptr };

		ValueableVector<CSFText> WeaponUINameX { };
		Valueable<bool> NoShadowSpawnAlt { false };

		std::vector<WeaponStruct> AdditionalWeaponDatas { };
		std::vector<WeaponStruct> AdditionalEliteWeaponDatas { };
		std::vector<int> AdditionalTurrentWeapon { };

		Valueable<bool> OmniCrusher_Aggressive { false };
		Valueable<bool> CrusherDecloak { true };
		Valueable<bool> Crusher_SupressLostEva { false };

		Promotable<float> CrushFireDeathWeapon { 0.0f };
		Promotable<int> CrushDamage { 0 };
		Nullable<WarheadTypeClass*> CrushDamageWarhead { };
		NullablePromotable<Leptons> CrushRange { };

		NullableIdx<VocClass> DigInSound { };
		NullableIdx<VocClass> DigOutSound { };
		Nullable<AnimTypeClass*> DigInAnim { };
		Nullable<AnimTypeClass*> DigOutAnim { };

		ValueableIdx<VoxClass> EVA_UnitLost { -1 };

		//Build stuffs
		Nullable<double> BuildTime_Speed { };
		Nullable<int> BuildTime_Cost { };
		Nullable<double> BuildTime_LowPowerPenalty { };
		Nullable<double> BuildTime_MinLowPower { };
		Nullable<double> BuildTime_MaxLowPower { };
		Nullable<double> BuildTime_MultipleFactory { };

		Nullable<int> CloakStages { };

		// particles
		Nullable<bool> DamageSparks { };

		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSmoke { };
		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSparks { };

		Valueable<bool> GattlingCyclic { false };
		NullableIdx<VocClass> CloakSound { };
		NullableIdx<VocClass> DecloakSound { };

		ValueableIdx<VocClass> VoiceRepair { -1 };
		Valueable<int> ReloadAmount { 1 };
		Nullable<int> EmptyReloadAmount { };

		Nullable<bool> TiberiumProof { };
		Valueable<bool> TiberiumSpill { false };
		Nullable<bool> TiberiumRemains { };
		Nullable<int> TiberiumTransmogrify { };

		Valueable<bool> SensorArray_Warn { true };
		Valueable<double> IronCurtain_Modifier { 1.0 };
		Valueable<double> ForceShield_Modifier { 1.0 };
		Valueable<int> Survivors_PilotCount { -1 }; //!< Defines the number of pilots inside this vehicle if Crewed=yes; maximum number of pilots who can survive. Defaults to 0 if Crewed=no; defaults to 1 if Crewed=yes. // NOTE: Flag in INI is called Survivor.Pilots

		// berserk
		Nullable<double> BerserkROFMultiplier { };

		// refinery and storage related
		Valueable<bool> Refinery_UseStorage { false };

		//CustomPalette CameoPal { };
		//PhobosPCXFile CameoPCX { };
		//PhobosPCXFile AltCameoPCX { };

		Valueable <bool> VirtualUnit { false };

		Nullable<CoordStruct> PrimaryCrawlFLH { };
		Nullable<CoordStruct> Elite_PrimaryCrawlFLH { };
		Nullable<CoordStruct> SecondaryCrawlFLH { };
		Nullable<CoordStruct> Elite_SecondaryCrawlFLH { };

		Valueable<bool> MissileHoming { false };

		ExtraFireData MyExtraFireData { };
		AircraftDiveData MyDiveData { };
		AircraftPutData MyPutData { };
		GiftBoxData MyGiftBoxData { };
		//JJFacingData  MyJJData { };
		PassengersData MyPassangersData { };
		SpawnSupportFLHData MySpawnSupportFLH { };
		SpawnSupportData MySpawnSupportDatas { };
		TrailsReader Trails { };
		FighterAreaGuardData MyFighterData { };
		DamageSelfType DamageSelfData { };

		AresAttachEffectTypeClass AttachedEffect;

		Valueable<AnimTypeClass*> NoAmmoEffectAnim { nullptr };
		Valueable<int> AttackFriendlies_WeaponIdx { -1 };
		Nullable<WORD> PipScaleIndex {};

		Nullable<SHPStruct*> AmmoPip {};
		Valueable<Point2D> AmmoPip_Offset { };
		Valueable<PaletteManager*> AmmoPip_Palette { }; //CustomPalette::PaletteMode::Default

		struct InsigniaData
		{
			Promotable<SHPStruct*> Shapes { nullptr };
			Promotable<int> Frame { -1 };
			Valueable<Point3D> Frames { { -1, -1, -1 } };

			inline bool Load(PhobosStreamReader& stm, bool registerForChange)
			{
				return this->Serialize(stm);
			}

			inline bool Save(PhobosStreamWriter& stm) const
			{
				return const_cast<InsigniaData*>(this)->Serialize(stm);
			}

		private:
			template <typename T>
			inline bool Serialize(T& stm)
			{
				return stm
					.Process(Shapes)
					.Process(Frame)
					.Process(Frames)
					.Success();
			}
		};

		std::vector<InsigniaData> Insignia_Weapon {};

		Valueable<int> VHPscan_Value { 2 };
		Valueable<bool> CloakAllowed { true };

		ValueableVector<TechnoTypeClass*> InitialPayload_Types {};
		ValueableVector<int> InitialPayload_Nums {};
		ValueableVector<Rank> InitialPayload_Vet {};
		ValueableVector<bool> InitialPayload_AddToTransportTeam {};

		Valueable<bool> AlternateTheaterArt { false };

		Valueable<bool> HijackerOneTime { false };
		Valueable<int> HijackerKillPilots { 0 };

		ValueableIdx<VocClass> HijackerEnterSound { -1 };
		ValueableIdx<VocClass> HijackerLeaveSound { -1 };

		Valueable<bool> HijackerBreakMindControl{ true };
		Valueable<bool> HijackerAllowed { true };

		Promotable<int> Survivors_PilotChance { -1 };

		ValueableIdx<CursorTypeClass*> Cursor_Deploy { (int)MouseCursorType::Deploy };
		ValueableIdx<CursorTypeClass*> Cursor_NoDeploy { (int)MouseCursorType::NoDeploy };
		ValueableIdx<CursorTypeClass*> Cursor_Enter { (int)MouseCursorType::Enter };
		ValueableIdx<CursorTypeClass*> Cursor_NoEnter { (int)MouseCursorType::NoEnter };
		ValueableIdx<CursorTypeClass*> Cursor_Move { (int)MouseCursorType::Move };
		ValueableIdx<CursorTypeClass*> Cursor_NoMove { (int)MouseCursorType::NoMove };

		Valueable<bool> ImmuneToAbduction { false }; //680, 1362
		Valueable<bool> UseROFAsBurstDelays { false };

		Valueable<bool> Chronoshift_Crushable { true };
		Valueable<bool> CanBeReversed { false };
		Nullable<TechnoTypeClass*> ReversedAs {};
		Valueable<int> AssaulterLevel { 0 };

		Nullable<double> SelfHealing_Rate {};
		Promotable<int> SelfHealing_Amount { 1 };
		Promotable<double> SelfHealing_Max { 1.0 };
		Promotable<int> SelfHealing_CombatDelay { 0 };

		Valueable<bool> Bounty { false };
		Valueable<bool> HasSpotlight { false };

		Nullable<int> Crew_TechnicianChance {};
		Nullable<int> Crew_EngineerChance {};
		Valueable<bool> Saboteur { false };

		Nullable<int> RadialIndicatorRadius {};
		Nullable<ColorStruct> RadialIndicatorColor {};

		Valueable<int> GapRadiusInCells { 0 };
		Valueable<int> SuperGapRadiusInCells { 0 };

		// smoke when damaged
		Nullable<int> SmokeChanceRed {};
		Nullable<int> SmokeChanceDead {};
		Valueable<AnimTypeClass*> SmokeAnim { nullptr };

		Nullable<bool> CarryallAllowed {};
		Nullable<int> CarryallSizeLimit {};

		NullableIdx<VocClass> VoiceAirstrikeAttack {};
		NullableIdx<VocClass> VoiceAirstrikeAbort {};

		// hunter seeker
		Nullable<int> HunterSeekerDetonateProximity {};
		Nullable<int> HunterSeekerDescendProximity {};
		Nullable<int> HunterSeekerAscentSpeed {};
		Nullable<int> HunterSeekerDescentSpeed {};
		Nullable<int> HunterSeekerEmergeSpeed {};
		Valueable<bool> HunterSeekerIgnore { false };

		Valueable<bool> CanPassiveAcquire_Guard { true };
		Valueable<bool> CanPassiveAcquire_Cloak { true };

		Valueable<bool> CrashSpin { true };
		Valueable<int> AirRate { 0 };
		Nullable<bool> Unsellable {};

		Nullable<AffectedHouse> CreateSound_afect {};
		Valueable<bool> Chronoshift_Allow { true };
		Valueable<bool> Chronoshift_IsVehicle { false };

		Valueable<double> FactoryPlant_Multiplier {1.0};
		Nullable<bool> MassSelectable {};

		Nullable<bool> TiltsWhenCrushes_Vehicles {};
		Nullable<bool> TiltsWhenCrushes_Overlays {};
		Nullable<double> CrushForwardTiltPerFrame {};
		Valueable<double> CrushOverlayExtraForwardTilt { 0.2 };
		Valueable<double> CrushSlowdownMultiplier { 0.2 };

		Valueable<float> ShadowScale  { -1.0f };

		NullableVector<int> AIIonCannonValue {};
		mutable OptionalStruct<bool, true> GenericPrerequisite {};
		Nullable<int> ExtraPower_Amount {};

		Nullable<bool> Bounty_Display { };
		Promotable<int> Bounty_Value { 0 };

		ValueableVector<TechnoTypeClass*> BountyAllow {};
		ValueableVector<TechnoTypeClass*> BountyDissallow {};

		Promotable<double> BountyBonusmult { 1.0 };
		Nullable<BountyValueOption> Bounty_Value_Option { };
		Promotable<double> Bounty_Value_mult { 1.0 };
		Valueable<bool> Bounty_IgnoreEnablers { false };
		bool RecheckTechTreeWhenDie { false };
		ValueableVector<SuperWeaponTypeClass*> Linked_SW {};

		Valueable<bool> CanDrive { false }; //!< Whether this TechnoType can act as the driver of vehicles whose driver has been killed. Request #733.
		ValueableVector<TechnoTypeClass*> Operators {};
		Valueable<bool> Operator_Any { false };
		Nullable<bool> AlwayDrawRadialIndicator { };
		Nullable<double> ReloadRate { };

		Nullable<AnimTypeClass*> CloakAnim { };
		Nullable<AnimTypeClass*> DecloakAnim { };
		Nullable<bool> Cloak_KickOutParasite { };

		ValueableVector<AnimTypeClass*> DeployAnims {};
		PhobosMap<TechnoTypeClass*, Valueable<float>> SpecificExpFactor {};
		Valueable<bool> Initial_DriverKilled { false };

		NullableIdx<VocClass> VoiceCantDeploy {};

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject)
			, AttachedEffect { OwnerObject }
		{ }

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromINIFile_Aircraft(CCINIClass* pINI);
		void LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI);
		void Initialize();

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		bool IsCountedAsHarvester() const;

		void AdjustCrushProperties();

		// Ares 0.A
		const char* GetSelectionGroupID() const;

		bool IsGenericPrerequisite() const;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static double TurretMultiOffsetDefaultMult;
	static double TurretMultiOffsetOneByEightMult;

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);

	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<std::vector<CoordStruct>>& nFLH, std::vector<std::vector<CoordStruct>>& nEFlh, const char* pPrefixTag);
	static void GetFLH(INI_EX& exArtINI, const char* pArtSection, Nullable<CoordStruct>& nFlh, Nullable<CoordStruct>& nEFlh, const char* pFlag);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const std::string& pID);
	static AnimTypeClass* GetSinkAnim(TechnoClass* pThis);
	static double GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules);
	static bool PassangersAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pPassanger);
	static VoxelStruct* GetBarrelsVoxelData(TechnoTypeClass* const pThis, size_t const nIdx);
	static VoxelStruct* GetTurretVoxelData(TechnoTypeClass* const pThis, size_t const nIdx);
};