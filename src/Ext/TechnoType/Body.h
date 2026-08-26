#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>

#include <Utilities/PhobosPCXFile.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/MultiBoolFixedArray.h>
#include <Utilities/CSFText.h>

#include <Misc/Defines.h>
#include <Ext/ObjectType/Body.h>

#include <New/Type/PaletteManager.h>
#include <New/Type/DroppodProperties.h>

#include <New/Entity/AresAttachEffectTypeClass.h>
#include <New/Entity/TheaterSpecificSHP.h>
#include <New/Entity/DropshipLoadoutClass.h>
#include <New/Entity/BlockTypeClass.h>
#include <New/Entity/TiberiumEaterTypeClass.h>
#include <New/Entity/PassengerDeletionTypeClass.h>
#include <New/Entity/InsigniaData.h>
#include <New/Entity/LaserTrailDataEntry.h>

#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

class VoxClass;
class VocClass;
class CrateTypeClass;
class CursorTypeClass;
class HoverTypeClass;
class ShieldTypeClass;
class DigitalDisplayTypeClass;
class SelectBoxTypeClass;
class ImmunityTypeClass;
class Matrix3D;
struct Leptons;

struct JumpjetTiltVoxelIndexKey
{
	unsigned bodyFrame : 5;
	unsigned bodyFace : 5;
	unsigned slopeIndex : 6;
	unsigned isSpawnAlt : 1;
	unsigned forwards : 7;
	unsigned sideways : 7;
	unsigned reserved : 1;
};

struct PhobosVoxelIndexKey
{
	union
	{
		VoxelIndexKey Base;
		union
		{
			JumpjetTiltVoxelIndexKey JumpjetTiltVoxel;
			// add other definitions here as needed
		} CustomIndexKey;
		DWORD Value;
	};

	// add funcs here if needed
	constexpr bool IsCleanKey() const { return Base.Value == 0; }
	constexpr bool IsJumpjetKey() const { return Base.MainVoxel.Reserved != 0; }
};

static_assert(sizeof(PhobosVoxelIndexKey) == sizeof(VoxelIndexKey), "PhobosVoxelIndexKey size mismatch");

class ArmorTypeClass;
struct ImageStatusses
{
	VoxelStruct Images;
	bool Loaded;

	~ImageStatusses();

	static ImageStatusses ReadVoxel(const char* const nKey);

	void swap(VoxelStruct& from);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

struct BurstFLHBundle
{
	std::vector<CoordStruct> Flh {};
	std::vector<CoordStruct> EFlh {};

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
};

class TechnoTypeExtData : public ObjectTypeExtData
{
public:
	//using ImageVector = std::vector<VoxelStruct>;
	using ColletiveCoordStructVectorData = std::array<std::vector<std::vector<CoordStruct>>*, 3u>;
	using base_type = TechnoTypeClass;

public:

#pragma region ClassMembers
	Nullable<AnimTypeClass*> Landing_Anim {};
	Valueable<AnimTypeClass*> Landing_AnimOnWater { nullptr };

	Valueable<bool> HealthBar_Hide { false };
	Valueable<bool> HealthBar_HidePips { false };
	Valueable<bool> HealthBar_Permanent { false };
	Valueable<bool> HealthBar_Permanent_PipScale { false };
	Valueable<CSFText> UIDescription {};
	Valueable<bool> LowSelectionPriority { false };
	Valueable<bool> LowDeployPriority { false };
	PhobosFixedString<0x20> GroupAs {};

	Valueable<int> RadarJamRadius { 0 };
	Nullable<AffectedHouse> RadarJamHouses { };
	Nullable<int> RadarJamDelay { };
	ValueableVector<BuildingTypeClass*> RadarJamAffect {};
	ValueableVector<BuildingTypeClass*> RadarJamIgnore {};

	Nullable<int> InhibitorRange {};
	Nullable<int> DesignatorRange {};

	//Enemy Inhibitors
	Nullable<int> SuppressorRange {};

	//Enemy Designator
	Nullable<int> AttractorRange {};

	Valueable<Leptons> MindControlRangeLimit {};
	Nullable<bool> MindControl_IgnoreSize { };
	Valueable<int> MindControlSize { 1 };

	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_EliteAbilities {};
	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_VeteranAbilities {};

	ValueableIdxVector<ImmunityTypeClass*> E_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass*> V_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass*> R_ImmuneToType {};

	Valueable<bool> Interceptor { false };
	Valueable<AffectedHouse> Interceptor_CanTargetHouses { AffectedHouse::Enemies };
	Promotable<Leptons> Interceptor_GuardRange {};
	Nullable<bool> Interceptor_GuardRange_IsCylindrical { };
	Promotable<Leptons> Interceptor_MinimumGuardRange {};
	Promotable<int> Interceptor_TargetingDelay { 1 };

	Valueable<int> Interceptor_Weapon { 0 };
	Nullable<bool> Interceptor_DeleteOnIntercept {};
	Nullable<WeaponTypeClass*> Interceptor_WeaponOverride {};
	Valueable<bool> Interceptor_WeaponReplaceProjectile { false };
	Valueable<bool> Interceptor_WeaponCumulativeDamage { false };
	Valueable<bool> Interceptor_KeepIntact { false };
	Valueable<bool> Interceptor_ConsiderWeaponRange { false };
	Valueable<bool> Interceptor_OnlyTargetBullet { false };
	Nullable<bool> Interceptor_ApplyFirepowerMult { };

	Nullable<PartialVector3D<int>> TurretOffset {};
	Valueable<bool> Powered_KillSpawns { false };
	Valueable<bool> Spawn_LimitedRange { false };
	Valueable<int> Spawn_LimitedExtraRange { 0 };
	Nullable<int> Spawner_DelayFrames {};
	bool Harvester_Counted {};
	Nullable<bool> Promote_IncludeSpawns { };
	Valueable<bool> ImmuneToCrit { false };
	Nullable<bool> MultiMindControl_ReleaseVictim { };
	Valueable<bool> NoManualMove { false };
	Nullable<int> InitialStrength {};

	PassengerDeletionTypeClass PassengerDeletionType {};

	Valueable<bool> Death_NoAmmo { false };
	Valueable<int> Death_Countdown { 0 };
	Valueable<KillMethod> Death_Method { KillMethod::None };
	Valueable<bool> Death_WithMaster { false };
	Nullable<bool> AutoDeath_AllowLimboed {};
	Nullable<bool> AutoDeath_OnOwnerChange_IgnoreRevertOnExit {};
	Valueable<int> AutoDeath_MoneyExceed { -1 };
	Valueable<int> AutoDeath_MoneyBelow { -1 };
	Valueable<bool> AutoDeath_LowPower { false };
	Valueable<bool> AutoDeath_FullPower { false };
	Valueable<int> AutoDeath_PassengerExceed { -1 };
	Valueable<int> AutoDeath_PassengerBelow { -1 };
	Valueable<bool> AutoDeath_ContentIfAnyMatch { false };
	Valueable<bool> AutoDeath_OwnedByPlayer { false };
	Valueable<bool> AutoDeath_OwnedByAI { false };

	Valueable<bool> Death_IfChangeOwnership { false };

	ValueableVector<TechnoTypeClass*> AutoDeath_Nonexist {};
	Valueable<AffectedHouse> AutoDeath_Nonexist_House { AffectedHouse::Owner };
	Valueable<bool> AutoDeath_Nonexist_Any { false };
	Nullable<bool> AutoDeath_Nonexist_AllowLimboed { };
	ValueableVector<TechnoTypeClass*> AutoDeath_Exist {};
	Valueable<AffectedHouse> AutoDeath_Exist_House { AffectedHouse::Owner };
	Valueable<bool> AutoDeath_Exist_Any { false };
	Nullable<bool> AutoDeath_Exist_AllowLimboed { };
	Valueable<AnimTypeClass*> AutoDeath_VanishAnimation { nullptr };

	Valueable<PowerStatus> AutoDeath_PlayerPowerStatus { PowerStatus::None };
	Valueable<int> AutoDeath_PlayerMoney_Max { -1 };
	Valueable<int> AutoDeath_PlayerMoney_Min { -1 };

	Valueable<TechnoTypeClass*> Convert_AutoDeath {};
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
	NullablePromotable<WeaponTypeClass*> WarpInMinRangeWeapon {};
	Promotable<WeaponTypeClass*> WarpOutWeapon { nullptr };
	Promotable<bool> WarpInWeapon_UseDistanceAsDamage { false };

	ValueableVector<AnimTypeClass*> OreGathering_Anims {};
	ValueableVector<int> OreGathering_Tiberiums {};
	ValueableVector<int> OreGathering_FramesPerDir {};

	Nullable<bool> DestroyAnim_Random { };
	PhobosMap<WarheadTypeClass*, std::vector<AnimTypeClass*>> DestroyAnimSpecific {};
	Nullable<bool> NotHuman_RandomDeathSequence { };

	Valueable<InfantryTypeClass*> DefaultDisguise { nullptr };

	Nullable<int> OpenTopped_RangeBonus {};
	Nullable<float> OpenTopped_DamageMultiplier {};
	Nullable<bool> OpenTopped_DecloakToFire {};
	Nullable<bool> OpenTopped_FireWhileMoving {};
	Nullable<bool> OpenTopped_AllowFiringIfAttackedByLocomotor {};
	Nullable<int> OpenTopped_WarpDistance {};
	Nullable<bool> OpenTopped_IgnoreRangefinding { };
	Nullable<bool> OpenTopped_AllowFiringIfDeactivated { };
	Nullable<bool> OpenTopped_ShareTransportTarget { };
	Nullable<bool> OpenTopped_UseTransportRangeModifiers { };
	Nullable<bool> OpenTopped_CheckTransportDisableWeapons { };
	Nullable<int> OpenTransport_RangeBonus { };
	Nullable<float> OpenTransport_DamageMultiplier { };
	Nullable<bool> OpenTransport_FireWhileMoving {};

	Valueable<bool> AutoFire { false };
	Valueable<bool> AutoFire_TargetSelf { false };

	Valueable<bool> NoSecondaryWeaponFallback { false };
	Valueable<bool> NoSecondaryWeaponFallback_AllowAA { false };
	Nullable<bool> AllowWeaponSelectAgainstWalls {};

	Valueable<int> NoAmmoWeapon { -1 };
	Valueable<int> NoAmmoAmount { 0 };

	Nullable<bool> JumpjetAllowLayerDeviation {};
	Nullable<bool> JumpjetTurnToTarget {};
	Nullable<bool> JumpjetCrash_Rotate {};

	ValueableVector<AnimTypeClass*> DeployingAnims {};
	Valueable<bool> DeployingAnim_KeepUnitVisible { false };
	Valueable<bool> DeployingAnim_ReverseForUndeploy { true };
	Valueable<bool> DeployingAnim_UseUnitDrawer { true };

	Nullable<SelfHealGainType> SelfHealGainType {};

	Valueable<int> ForceWeapon_Naval_Decloaked { -1 };
	Valueable<int> ForceWeapon_UnderEMP { -1 };
	Valueable<int> ForceWeapon_Cloaked { -1 };
	Valueable<int> ForceWeapon_Disguised { -1 };
	Nullable<bool> ImmuneToEMP {};
	Valueable<bool> Ammo_Shared { false };
	Valueable<int> Ammo_Shared_Group { -1 };
	Nullable<bool> Passengers_SyncOwner { };
	Nullable<bool> Passengers_SyncOwner_RevertOnExit { };

	Valueable<bool> Aircraft_DecreaseAmmo { true };

	ValueableVector<LaserTrailDataEntry> LaserTrailData {};
	Valueable<CSFText> EnemyUIName {};
	Nullable<bool> UseDisguiseMovementSpeed { };

	Promotable<SHPCaches*> Insignia {};
	Valueable<Vector3D<int>> InsigniaFrames { { -1, -1, -1 } };
	Promotable<int> InsigniaFrame { -1 };
	Nullable<bool> Insignia_ShowEnemy {};
	std::vector<InsigniaData> Insignia_Weapon {};
	std::vector<Promotable<SHPCaches*>> Insignia_Passengers {};
	std::vector<Promotable<int>> InsigniaFrame_Passengers {};
	std::vector<Valueable<Vector3D<int>>> InsigniaFrames_Passengers {};

	Nullable<PartialVector2D<double>> InitialStrength_Cloning {};

	Nullable<SelectBoxTypeClass*> SelectBox {};
	Valueable<bool> HideSelectBox { false };

	Nullable<CoordStruct> PronePrimaryFireFLH {};
	Nullable<CoordStruct> ProneSecondaryFireFLH {};
	Nullable<CoordStruct> DeployedPrimaryFireFLH {};
	Nullable<CoordStruct> DeployedSecondaryFireFLH {};

	Nullable<CoordStruct> E_PronePrimaryFireFLH {};
	Nullable<CoordStruct> E_ProneSecondaryFireFLH {};
	Nullable<CoordStruct> E_DeployedPrimaryFireFLH {};
	Nullable<CoordStruct> E_DeployedSecondaryFireFLH {};

	std::vector<BurstFLHBundle> WeaponBurstFLHs {};
	std::vector<BurstFLHBundle> CrouchedWeaponBurstFLHs {};
	std::vector<BurstFLHBundle> DeployedWeaponBurstFLHs {};
	std::vector<CoordStruct> AlternateFLHs {};

	Nullable<bool> IronCurtain_KeptOnDeploy {};
	Nullable<bool> ForceShield_KeptOnDeploy {};
	Nullable<IronCurtainFlag> IronCurtain_Effect {};
	Nullable<WarheadTypeClass*> IronCurtain_KillWarhead {};
	Nullable<IronCurtainFlag> ForceShield_Effect {};
	Nullable<WarheadTypeClass*> ForceShield_KillWarhead {};
	ValueableIdx<VoxClass*> EVA_Sold { -1 };
	ValueableIdx<VocClass*> SellSound { -1 };

	Nullable<bool> Explodes_KillPassengers {};

	Nullable<int> DeployFireWeapon {};
	Valueable<WeaponTypeClass*> RevengeWeapon { nullptr };
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses { AffectedHouse::All };

	Valueable<TargetZoneScanType> TargetZoneScanType { TargetZoneScanType::Same };
	Nullable<bool> GrapplingAttack {};

	Nullable<bool> FacingRotation_Disable {};
	Valueable<bool> FacingRotation_DisalbeOnEMP { false };
	Valueable<bool> FacingRotation_DisalbeOnDeactivated { false };
	Valueable<bool> FacingRotation_DisableOnDriverKilled { true };

	Valueable<bool> DontShake { true };
	NullableIdx<VocClass*> DiskLaserChargeUp {};
	Valueable<bool> DiskLaserDetonate { false };
	Nullable<AnimTypeClass*> DrainAnimationType {};
	Nullable<int> DrainMoneyFrameDelay {};
	Nullable<int> DrainMoneyAmount {};
	Nullable<bool> DrainMoney_Display {};
	Nullable<AffectedHouse> DrainMoney_Display_Houses {};
	Nullable<bool> DrainMoney_Display_OnTarget {};
	Nullable<bool> DrainMoney_Display_OnTarget_UseDisplayIncome {};
	Valueable<Point2D> DrainMoney_Display_Offset { { 0, 0 } };

	Nullable<float> TalkBubbleTime {};

	Nullable<AffectedHouse> Draw_MindControlLink { };

	NullableVector<int> Overload_Count {};
	NullableVector<int> Overload_Damage {};
	NullableVector<int> Overload_Frames {};
	NullableIdx<VocClass*> Overload_DeathSound {};
	Nullable<ParticleSystemTypeClass*> Overload_ParticleSys {};
	Nullable<int> Overload_ParticleSysCount {};
	Nullable<WarheadTypeClass*> Overload_Warhead {};

	std::vector<CoordStruct> HitCoordOffset {};
	Valueable<bool> HitCoordOffset_Random { true };
	Promotable<WeaponTypeClass*> DeathWeapon {};
	Valueable<WeaponTypeClass*> CrashWeapon_s { nullptr };
	Promotable<WeaponTypeClass*> CrashWeapon {};
	Valueable<bool> DeathWeapon_CheckAmmo { false };
	Valueable<bool> Disable_C4WarheadExp { false };
	Valueable<double> CrashSpinLevelRate { 1.0 };
	Nullable<float> CrashSpin_Multiplier {};

	Valueable<double> CrashSpinVerticalRate { 1.0 };
	ValueableIdx<VocClass*> ParasiteExit_Sound { -1 };

	Nullable<SHPCaches*> PipShapes01 {};
	Nullable<SHPCaches*> PipShapes02 {};
	Nullable<SHPCaches*> PipGarrison {};
	Valueable<int> PipGarrison_FrameIndex { 0 };
	CustomPalette PipGarrison_Palette { CustomPalette::PaletteMode::Default };

	Valueable<bool> HealthNumber_Show { false };
	Valueable<bool> HealthNumber_Percent { false };
	Nullable<Point2D> Healnumber_Offset {};
	Nullable<SHPCaches*> HealthNumber_SHP {};
	Nullable<Point2D> Healnumber_Decrement {};
	Nullable<SHPCaches*> HealthBarSHP {};
	Nullable<SHPCaches*> HealthBarSHP_Selected {};
	Valueable<int> HealthBarSHPBracketOffset { 0 };
	Valueable<Point3D> HealthBarSHP_HealthFrame { { 18, 16, 17 } };
	CustomPalette HealthBarSHP_Palette { CustomPalette::PaletteMode::Temperate };

	Valueable<Point2D> HealthBarSHP_PointOffset { { 0, 0 } };
	Valueable<bool> HealthbarRemap { false };

	Valueable<SHPCaches*> GClock_Shape {};
	Valueable<TranslucencyLevel> GClock_Transculency {};
	CustomPalette GClock_Palette { CustomPalette::PaletteMode::Default };

	Valueable<bool> ROF_Random { true };
	Nullable<Point2D> Rof_RandomMinMax {};

	ValueableIdx<VoxClass*> Eva_Complete { -1 };
	ValueableIdx<VocClass*> VoiceCreate { -1 };
	Valueable<bool> VoiceCreate_Instant { false };
	Valueable<bool> CreateSound_Enable { true };

	Valueable<bool> SlaveFreeSound_Enable { true };
	NullableIdx<VocClass*> SlaveFreeSound {};
	Valueable<bool> NoAirportBound_DisableRadioContact { false };
	Nullable<AnimTypeClass*> SinkAnim {};
	Nullable<double> Tunnel_Speed {};
	Valueable<HoverTypeClass*> HoverType { nullptr };

	Valueable<bool> Gattling_Overload { false };
	Nullable<int> Gattling_Overload_Damage {};
	Nullable<int> Gattling_Overload_Frames {};
	NullableIdx<VocClass*> Gattling_Overload_DeathSound {};
	Nullable<ParticleSystemTypeClass*> Gattling_Overload_ParticleSys {};
	Nullable<int> Gattling_Overload_ParticleSysCount {};
	Nullable<WarheadTypeClass*> Gattling_Overload_Warhead {};

	Valueable<bool> IsDummy { false };

	ValueableVector<WeaponTypeClass*> FireSelf_Weapon {};
	ValueableVector<int> FireSelf_ROF {};
	ValueableVector<WeaponTypeClass*> FireSelf_Weapon_GreenHeath {};
	ValueableVector<int> FireSelf_ROF_GreenHeath {};
	ValueableVector<WeaponTypeClass*> FireSelf_Weapon_YellowHeath {};
	ValueableVector<int> FireSelf_ROF_YellowHeath {};
	ValueableVector<WeaponTypeClass*> FireSelf_Weapon_RedHeath {};
	ValueableVector<int> FireSelf_ROF_RedHeath {};

	Nullable<bool> AllowFire_IroncurtainedTarget {};
	Valueable<int> EngineerCaptureDelay { 0 };

	Nullable<ColorStruct> CommandLine_Move_Color {};
	Nullable<ColorStruct> CommandLine_Attack_Color {};
	Nullable<bool> PassiveAcquire_AI {};
	Nullable<bool> CanPassiveAquire_Naval {};
	Valueable<bool> TankDisguiseAsTank { false };
	ValueableVector<ObjectTypeClass*> DisguiseDisAllowed {};
	Valueable<bool> ChronoDelay_Immune { false };
	Nullable<int> PoseDir {};
	Valueable<bool> Firing_IgnoreGravity { false };

	Promotable<int> Survivors_PassengerChance { -1 };
	Nullable<CoordStruct> Spawner_SpawnOffsets {};
	Valueable<bool> Spawner_SpawnOffsets_OverrideWeaponFLH { false };

	Nullable<bool> ConsideredNaval {};
	Nullable<bool> ConsideredVehicle {};

	// Ares 0.1
	DWORD Prerequisite_RequiredTheaters { 0xFFFFFFFF };
	std::vector<std::vector<int>> Prerequisites {};
	Valueable<int> Prerequisite_Lists { 1 };
	ValueableVector<int> Prerequisite_Negative {};
	ValueableVector<int> Prerequisite_Display {};

	ValueableVector<int> BuildLimit_Requires {};

	Promotable<int> CrushLevel {};
	Promotable<int> CrushableLevel {};
	Promotable<int> DeployCrushableLevel {};

	Valueable<float> Experience_VictimMultiple { 1.0f };
	Valueable<float> Experience_KillerMultiple { 1.0f };

	Nullable<Leptons> NavalRangeBonus {};

	Nullable<bool> AI_LegalTarget {};
	Valueable<bool> DeployFire_UpdateFacing { true };
	Valueable<TechnoTypeClass*> Fake_Of { nullptr };
	Valueable<bool> CivilianEnemy { false };
	Valueable<bool> ImmuneToBerserk { false };
	Valueable<double> Berzerk_Modifier { 1.0 };

	//Valueable<bool> IgnoreToProtect { false };
	Valueable<int> TargetLaser_Time { 15 };
	ValueableVector<int> TargetLaser_WeaponIdx {};

	Nullable<bool> CurleyShuffle {};

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
	ValueableIdx<VoxClass*> Promote_Elite_Eva { -1 };
	ValueableIdx<VoxClass*> Promote_Vet_Eva { -1 };
	NullableIdx<VocClass*> Promote_Elite_Sound {};
	NullableIdx<VocClass*> Promote_Vet_Sound {};
	Nullable<int> Promote_Elite_Flash {};
	Nullable<int> Promote_Vet_Flash {};

	Valueable<TechnoTypeClass*> Promote_Vet_Type { nullptr };
	Valueable<TechnoTypeClass*> Promote_Elite_Type { nullptr };

	Nullable<AnimTypeClass*> Promote_Vet_Anim {};
	Nullable<AnimTypeClass*> Promote_Elite_Anim {};

	Nullable<bool> Promote_Vet_PlaySpotlight {};
	Nullable<bool> Promote_Elite_PlaySpotlight {};

	Valueable<double> Promote_Vet_Exp { 0.0 };
	Valueable<double> Promote_Elite_Exp { 0.0 };
	Nullable<FacingType> DeployDir {};

	ValueableVector<TechnoTypeClass*> PassengersWhitelist {};
	ValueableVector<TechnoTypeClass*> PassengersBlacklist {};

	Valueable<bool> NoManualUnload { false };
	Valueable<bool> NoSelfGuardArea { false };
	Valueable<bool> NoManualFire { false };
	Valueable<bool> NoManualEnter { false };
	Valueable<bool> NoManualEject { false };

	Valueable<bool> Passengers_BySize { true };
	//Nullable<bool> Crashable { };

	Valueable<TechnoTypeClass*> Convert_Deploy { nullptr };
	Valueable<TechnoTypeClass*> Convert_Undeploy { nullptr };
	Valueable<int> Convert_Deploy_Delay { -1 };
	Valueable<TechnoTypeClass*> Convert_Script { nullptr };
	ValueableVector<int> Convert_Script_Prereq {};
	Valueable<TechnoTypeClass*> Convert_Water { nullptr };
	Valueable<TechnoTypeClass*> Convert_Land { nullptr };
	Nullable<bool> Convert_ResetMindControl { };

	Valueable<int> Ammo_AutoConvertMinimumAmount { -1 };
	Valueable<int> Ammo_AutoConvertMaximumAmount { -1 };
	Valueable<TechnoTypeClass*> Ammo_AutoConvertType { };

	Valueable<double> Convert_Health_AbovePercent { };
	Valueable<double> Convert_Health_BelowPercent { };
	Valueable<TechnoTypeClass*> Convert_Health { };

	Nullable<Leptons> Harvester_LongScan {};
	Nullable<Leptons> Harvester_ShortScan {};
	Nullable<Leptons> Harvester_ScanCorrection {};

	Nullable<int> Harvester_TooFarDistance {};
	Nullable<int> Harvester_KickDelay {};

	Nullable<int> TurretRot {};

	Valueable<UnitTypeClass*> WaterImage { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Yellow { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Red { nullptr };

	Valueable<TechnoTypeClass*> Image_Yellow { nullptr };
	Valueable<TechnoTypeClass*> Image_Red { nullptr };

	Valueable<int> FallRate_Parachute { 1 };
	Valueable<int> FallRate_NoParachute { 1 };
	Nullable<int> FallRate_ParachuteMax {};
	Nullable<int> FallRate_NoParachuteMax {};

	std::vector<VoxelStruct> BarrelImageData {};
	std::vector<VoxelStruct> TurretImageData {};
	VoxelStruct SpawnAltData {};

	ValueableVector<CSFText> WeaponUINameX {};
	Valueable<bool> NoShadowSpawnAlt { false };

	std::vector<WeaponStruct> AdditionalWeaponDatas {};
	std::vector<WeaponStruct> AdditionalEliteWeaponDatas {};
	std::vector<int> AdditionalTurrentWeapon {};

	Valueable<bool> OmniCrusher_Aggressive { false };
	Valueable<bool> CrusherDecloak { true };
	Valueable<bool> Crusher_SupressLostEva { false };

	Promotable<float> CrushFireDeathWeapon { 0.0f };
	Promotable<int> CrushDamage { 0 };
	Nullable<WarheadTypeClass*> CrushDamageWarhead {};
	Valueable<bool> CrushDamagePlayWHAnim { false };
	NullablePromotable<Leptons> CrushRange {};

	NullableIdx<VocClass*> DigInSound {};
	NullableIdx<VocClass*> DigOutSound {};
	Nullable<AnimTypeClass*> DigInAnim {};
	Nullable<AnimTypeClass*> DigOutAnim {};

	ValueableIdx<VoxClass*> EVA_UnitLost { -1 };

	//Build stuffs
	Nullable<double> BuildTime_Speed {};
	Nullable<int> BuildTime_Cost {};
	Nullable<double> BuildTime_LowPowerPenalty {};
	Nullable<double> BuildTime_MinLowPower {};
	Nullable<double> BuildTime_MaxLowPower {};
	Nullable<double> BuildTime_MultipleFactory {};

	Nullable<int> CloakStages {};

	// particles
	Nullable<bool> DamageSparks {};

	NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSmoke {};
	NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSparks {};

	Valueable<bool> GattlingCyclic { false };
	NullableIdx<VocClass*> CloakSound {};
	NullableIdx<VocClass*> DecloakSound {};

	ValueableIdx<VocClass*> VoiceRepair { -1 };
	Valueable<int> ReloadAmount { 1 };
	Nullable<int> EmptyReloadAmount {};

	Nullable<bool> TiberiumProof {};
	Valueable<bool> TiberiumSpill { false };
	Nullable<bool> TiberiumRemains {};
	Nullable<int> TiberiumTransmogrify {};

	Valueable<bool> SensorArray_Warn { true };
	Valueable<double> IronCurtain_Modifier { 1.0 };
	Valueable<double> ForceShield_Modifier { 1.0 };
	Valueable<int> Survivors_PilotCount { -1 };
	std::vector<InfantryTypeClass*> Survivors_Pilots {};

	Valueable<int> Ammo_AddOnDeploy { 0 };
	Valueable<int> Ammo_AutoDeployMinimumAmount { -1 };
	Valueable<int> Ammo_AutoDeployMaximumAmount { -1 };
	Valueable<int> Ammo_DeployUnlockMinimumAmount { -1 };
	Valueable<int> Ammo_DeployUnlockMaximumAmount { -1 };

	// berserk
	Nullable<double> BerserkROFMultiplier {};

	// refinery and storage related
	Valueable<bool> Refinery_UseStorage { false };

	//CustomPalette CameoPal { };
	//PhobosPCXFile CameoPCX { };
	//PhobosPCXFile AltCameoPCX { };

	Nullable<CoordStruct> PrimaryCrawlFLH {};
	Nullable<CoordStruct> Elite_PrimaryCrawlFLH {};
	Nullable<CoordStruct> SecondaryCrawlFLH {};
	Nullable<CoordStruct> Elite_SecondaryCrawlFLH {};

	AresAttachEffectTypeClass AttachedEffect {};

	Valueable<AnimTypeClass*> NoAmmoEffectAnim { nullptr };
	Valueable<int> AttackFriendlies_WeaponIdx { -1 };
	Valueable<bool> AttackFriendlies_AutoAttack { true };

	Nullable<WORD> PipScaleIndex {};

	Nullable<SHPCaches*> AmmoPip_shape {};
	Valueable<Point2D> AmmoPip_Offset {};
	CustomPalette AmmoPip_Palette { CustomPalette::PaletteMode::Default };
	Valueable<Point2D> AmmoPipOffset { { 0, 0 } };

	Nullable<bool> ShowSpawnsPips {};
	Valueable<int> SpawnsPip { 1 };
	Valueable<int> EmptySpawnsPip { 0 };
	Nullable<Point2D> SpawnsPipSize {};
	Valueable<Point2D> SpawnsPipOffset { { 0, 0 } };

	Valueable<int> VHPscan_Value { 2 };
	Valueable<bool> CloakAllowed { true };

	ValueableVector<TechnoTypeClass*> InitialPayload_Types {};
	ValueableVector<int> InitialPayload_Nums {};
	ValueableVector<Rank> InitialPayload_Vet {};
	ValueableVector<bool> InitialPayload_AddToTransportTeam {};

	Valueable<bool> AlternateTheaterArt { false };

	Valueable<bool> HijackerOneTime { false };
	Valueable<int> HijackerKillPilots { 0 };

	ValueableIdx<VocClass*> HijackerEnterSound { -1 };
	ValueableIdx<VocClass*> HijackerLeaveSound { -1 };

	Valueable<bool> HijackerBreakMindControl { true };
	Valueable<bool> HijackerAllowed { true };

	Promotable<int> Survivors_PilotChance { -1 };

	ValueableIdx<CursorTypeClass*> Cursor_Deploy { (int)MouseCursorType::Deploy };
	ValueableIdx<CursorTypeClass*> Cursor_NoDeploy { (int)MouseCursorType::NoDeploy };
	ValueableIdx<CursorTypeClass*> Cursor_Enter { (int)MouseCursorType::Enter };
	ValueableIdx<CursorTypeClass*> Cursor_NoEnter { (int)MouseCursorType::NoEnter };
	ValueableIdx<CursorTypeClass*> Cursor_Move { (int)MouseCursorType::Move };
	ValueableIdx<CursorTypeClass*> Cursor_NoMove { (int)MouseCursorType::NoMove };

	Valueable<bool> ImmuneToAbduction { false };
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

	// spotlights
	Valueable<bool> HasSpotlight { false };
	Valueable<int> Spot_Height { 430 };
	Valueable<int> Spot_Distance { 1024 };
	Valueable<SpotlightAttachment> Spot_AttachedTo { SpotlightAttachment::Body };
	Valueable<bool> Spot_DisableR { false };
	Valueable<bool> Spot_DisableG { false };
	Valueable<bool> Spot_DisableB { false };
	Valueable<bool> Spot_DisableColor { false };
	Valueable<bool> Spot_Reverse { false };

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
	Nullable<AnimTypeClass*> SmokeAnim {};

	Nullable<bool> CarryallAllowed {};
	Nullable<int> CarryallSizeLimit {};

	NullableIdx<VocClass*> VoiceAirstrikeAttack {};
	NullableIdx<VocClass*> VoiceAirstrikeAbort {};

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

	Valueable<double> FactoryPlant_Multiplier { 1.0f };
	Nullable<bool> MassSelectable {};

	Nullable<bool> TiltsWhenCrushes_Vehicles {};
	Nullable<bool> TiltsWhenCrushes_Overlays {};
	Nullable<double> CrushForwardTiltPerFrame {};
	Nullable<double> CrushOverlayExtraForwardTilt {};
	Valueable<double> CrushSlowdownMultiplier { 0.2 };

	Valueable<float> ShadowScale { -1.0f };

	Nullable<PartialVector3D<int>> AIIonCannonValue {};
	mutable OptionalStruct<bool, true> GenericPrerequisite {};
	Nullable<int> ExtraPower_Amount {};

	Nullable<bool> Bounty_Display {};
	Promotable<int> Bounty_Value { 0 };
	Promotable<float> Bounty_Value_PercentOf { 100.0f };
	ValueableIdx<VocClass*> Bounty_ReceiveSound {};

	ValueableVector<TechnoTypeClass*> BountyAllow {};
	ValueableVector<TechnoTypeClass*> BountyDissallow {};

	Promotable<double> BountyBonusmult { 1.0 };
	Nullable<BountyValueOption> Bounty_Value_Option {};
	Promotable<double> Bounty_Value_mult {};
	Valueable<bool> Bounty_IgnoreEnablers { false };
	bool RecheckTechTreeWhenDie { false };
	std::set<SuperWeaponTypeClass*> Linked_SW {};

	Nullable<bool> CanDrive {};
	ValueableVector<TechnoTypeClass*> Operators {};
	Valueable<bool> Operator_Any { false };
	Nullable<bool> AlwayDrawRadialIndicator {};
	Nullable<double> ReloadRate {};

	Nullable<AnimTypeClass*> CloakAnim {};
	Nullable<AnimTypeClass*> DecloakAnim {};
	Nullable<bool> Cloak_KickOutParasite {};

	ValueableVector<AnimTypeClass*> DeployAnims {};
	PhobosMap<TechnoTypeClass*, Valueable<float>> SpecificExpFactor {};
	Valueable<bool> Initial_DriverKilled { false };

	NullableIdx<VocClass*> VoiceCantDeploy {};
	Valueable<bool> DigitalDisplay_Disable { false };
	ValueableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes {};

	Valueable<int> AmmoPip { 13 };
	Valueable<int> EmptyAmmoPip { -1 };
	Valueable<int> PipWrapAmmoPip { 14 };
	Nullable<Point2D> AmmoPipSize {};

	Valueable<bool> ProduceCashDisplay { false };

	ValueableVector<HouseTypeClass*> FactoryOwners {};
	ValueableVector<HouseTypeClass*> FactoryOwners_Forbidden {};
	Valueable<bool> FactoryOwners_HaveAllPlans { false };
	Valueable<bool> FactoryOwners_HasAllPlans { false };

	Valueable<bool> Drain_Local { false };
	Valueable<int> Drain_Amount { 0 };

	Nullable<int> HealthBar_Sections {};
	Nullable<SHPCaches*> HealthBar_Border {};
	Nullable<int> HealthBar_BorderFrame {};
	Nullable<int> HealthBar_BorderAdjust {};

	Nullable<bool> Crashable {};

	Valueable<bool> IsBomb { false };
	Valueable<AnimTypeClass*> ParachuteAnim { nullptr };

	Valueable<TechnoTypeClass*> ClonedAs {};
	Valueable<TechnoTypeClass*> AI_ClonedAs {};
	Valueable<bool> Cloneable { true };
	ValueableVector<BuildingTypeClass*> ClonedAt {};
	ValueableVector<BuildingTypeClass*> BuiltAt {};
	Nullable<AnimTypeClass*> EMP_Sparkles {};
	Valueable<double> EMP_Modifier { 1.0 };
	int EMP_Threshold { -1 };

	ValueableVector<BuildingTypeClass*> PoweredBy {};

	Valueable<bool> ImmuneToWeb { false };
	NullableVector<AnimTypeClass*> Webby_Anims {};
	Valueable<double> Webby_Modifier { 1.0 };
	Nullable<int> Webby_Duration_Variation {};

	PhobosPCXFile CameoPCX {};
	PhobosPCXFile AltCameoPCX {};
	CustomPalette CameoPal { CustomPalette::PaletteMode::Default };
	Nullable<int> LandingDir {};

	// new secret lab
	DWORD Secret_RequiredHouses { 0xFFFFFFFFu };
	DWORD Secret_ForbiddenHouses { 0u };

	std::bitset<MaxHouseCount> RequiredStolenTech {};

	Nullable<bool> ReloadInTransport { };
	Valueable<bool> Weeder_TriggerPreProductionBuildingAnim { false };

	Nullable<int> Weeder_PipIndex {};
	Nullable<int> Weeder_PipEmptyIndex {};
	Valueable<bool> CanBeDriven { true };

	Valueable<bool> CloakPowered { false };
	Valueable<bool> CloakDeployed { false };

	Valueable<bool> ProtectedDriver { false };
	Nullable<double> ProtectedDriver_MinHealth {};
	Nullable<bool> KeepAlive {};

	Nullable<Leptons> SpawnDistanceFromTarget {};
	Nullable<int> SpawnHeight {};

	Valueable<bool> HumanUnbuildable { false };
	Valueable<bool> NoIdleSound { false };
	Valueable<bool> Soylent_Zero { false };

	Nullable<int> Prerequisite_Power {};

	Valueable<bool> PassengerTurret { false };

	Nullable<PartialVector3D<double>> DetectDisguise_Percent {};

	Nullable<Armor> EliteArmor {};
	Nullable<Armor> VeteranArmor {};
	Nullable<Armor> DeployedArmor {};

	Valueable<bool> Cloakable_IgnoreArmTimer { false };

	Valueable<bool> Untrackable { false };

	Nullable<UnitTypeClass*> LargeVisceroid {};
	NullableDroppodProperties DropPodProp {};

	Nullable<int> LaserTargetColor {};

	ValueableIdxVector<VocClass*> VoicePickup {};

	Valueable<double> CrateGoodie_RerollChance { 0.0 };
	NullableIdx<CrateTypeClass*> Destroyed_CrateType {};

	Nullable<bool> Infantry_DimWhenEMPEd {};
	Nullable<bool> Infantry_DimWhenDisabled {};

	Valueable<TechnoTypeClass*> Convert_HumanToComputer {};
	Valueable<TechnoTypeClass*> Convert_ComputerToHuman {};

	Valueable<bool> AutoDeath_OnOwnerChange { false };
	Nullable<bool> AutoDeath_OnOwnerChange_HumanToComputer {};
	Nullable<bool> AutoDeath_OnOwnerChange_ComputerToHuman {};

	Nullable<bool> TurretShadow {};
	Valueable<int> ShadowIndex_Frame { 0 };
	PhobosMap<int, int> ShadowIndices {};
	Nullable<int> ShadowSizeCharacteristicHeight {};

	std::vector<ValueableIdxVector<VocClass>> TalkbubbleVoices {};

	Nullable<float> HarvesterDumpAmount {};
	Valueable<bool> NoExtraSelfHealOrRepair { false };
	Nullable<bool> HarvesterScanAfterUnload {};
	Nullable<bool> AttackMove_Aggressive {};
	Nullable<bool> AttackMove_UpdateTarget {};

	//add this just in case the implementation chages
#pragma region BuildLimitGroup
	ValueableVector<TechnoTypeClass*> BuildLimitGroup_Types {};
	ValueableVector<int> BuildLimitGroup_Nums {};
	Valueable<int> BuildLimitGroup_Factor { 1 };
	Nullable<bool> BuildLimitGroup_ContentIfAnyMatch { };
	Nullable<bool> BuildLimitGroup_NotBuildableIfQueueMatch { };
	ValueableVector<TechnoTypeClass*> BuildLimitGroup_ExtraLimit_Types {};
	ValueableVector<int> BuildLimitGroup_ExtraLimit_Nums {};
	ValueableVector<int> BuildLimitGroup_ExtraLimit_MaxCount {};
	Valueable<int> BuildLimitGroup_ExtraLimit_MaxNum { 0 };
#pragma endregion

	NullableVector<int> Tiberium_PipIdx {};
	Nullable<int> Tiberium_EmptyPipIdx {};
	Valueable<SHPCaches*> Tiberium_PipShapes { nullptr };
	CustomPalette Tiberium_PipShapes_Palette {};

	Valueable<ColorStruct> Tint_Color {};
	Valueable<double> Tint_Intensity { 0.0 };
	Valueable<AffectedHouse> Tint_VisibleToHouses { AffectedHouse::All };

	AEAttachInfoTypeClass PhobosAttachEffects {};

	Valueable<bool> KeepTargetOnMove { false };
	Valueable<int> KeepTargetOnMove_Weapon { -1 };
	Valueable<Leptons> KeepTargetOnMove_ExtraDistance {};
	Valueable<bool> KeepTargetOnMove_NoMorePursuit { true };

	Nullable<bool> AllowAirstrike {};

	Valueable<bool> ForbidParallelAIQueues { };

	Valueable<bool> IgnoreForBaseCenter { false };

	Nullable<AnimTypeClass*> Wake {};
	Nullable<bool> Spawner_AttackImmediately {};
	Nullable<bool> Spawner_UseTurretFacing {};

	ValueableIdx<VoxClass*> EVA_Combat { -1 };
	Nullable<bool> CombatAlert {};
	Nullable<bool> CombatAlert_UseFeedbackVoice {};
	Nullable<bool> CombatAlert_UseAttackVoice {};
	Nullable<bool> CombatAlert_UseEVA {};
	Nullable<bool> CombatAlert_NotBuilding {};
	Nullable<int> SubterraneanHeight {};

	Nullable<Leptons> Spawner_RecycleRange {};
	Valueable<CoordStruct> Spawner_RecycleFLH { { 0, 0, 0 } };
	Nullable<bool> Spawner_RecycleOnTurret { };
	Valueable<AnimTypeClass*> Spawner_RecycleAnim { nullptr };
	Valueable<bool> Spawner_ReturnOnRepairDone { true };

	Valueable<bool> HugeBar { false };
	Valueable<int> HugeBar_Priority { -1 };

	std::vector<Valueable<CoordStruct>> SprayOffsets {};

	Nullable<int> AINormalTargetingDelay {};
	Nullable<int> PlayerNormalTargetingDelay {};
	Nullable<int> AIGuardAreaTargetingDelay {};
	Nullable<int> PlayerGuardAreaTargetingDelay {};
	Nullable<bool> DistributeTargetingFrame {};
	Nullable<int> PlayerAttackMoveTargetingDelay {};
	Nullable<int> AIAttackMoveTargetingDelay {};

	Valueable<bool> CanBeBuiltOn { false };
	Valueable<bool> UnitBaseNormal { false };
	Valueable<bool> UnitBaseForAllyBuilding { false };

	Nullable<int> ChronoSpherePreDelay {};
	Nullable<int> ChronoSphereDelay {};

	Valueable<bool> PassengerWeapon { false };

	Nullable<ParticleSystemTypeClass*> RefinerySmokeParticleSystemOne {};
	Nullable<ParticleSystemTypeClass*> RefinerySmokeParticleSystemTwo {};
	Nullable<ParticleSystemTypeClass*> RefinerySmokeParticleSystemThree {};
	Nullable<ParticleSystemTypeClass*> RefinerySmokeParticleSystemFour {};

	int SubterraneanSpeed { -1 };

	ValueableVector<int> ForceWeapon_InRange {};
	ValueableVector<double> ForceWeapon_InRange_Overrides {};
	Nullable<bool> ForceWeapon_InRange_ApplyRangeModifiers {};
	ValueableVector<int> ForceAAWeapon_InRange {};
	ValueableVector<double> ForceAAWeapon_InRange_Overrides {};
	Nullable<bool> ForceAAWeapon_InRange_ApplyRangeModifiers {};
	Nullable<bool> ForceWeapon_InRange_TechnoOnly {};

	Nullable<bool> UnitIdleRotateTurret {};
	Nullable<bool> UnitIdlePointToMouse {};

	Nullable<double> FallingDownDamage { };
	Nullable<double> FallingDownDamage_Water {};
	Nullable<bool> FallingDownDamage_AllowEMP {};

	Nullable<bool> HoverDrownable { };

	bool ExtraThreat_Enabled;
	Nullable<double> ExtraThreat_IsThreat { };
	Valueable<bool> AlwaysConsideredThreat {};
	Nullable<double> ExtraThreat_InRange {};
	Nullable<double> ExtraThreatCoefficient_InRangeDistance {};
	Nullable<double> ExtraThreatCoefficient_Facing {};
	Nullable<double> ExtraThreatCoefficient_DistanceToLastTarget {};

	NullableIdx<CrateTypeClass*> DropCrate {};

	Promotable<WarheadTypeClass*> WhenCrushed_Warhead {};
	Promotable<WeaponTypeClass*> WhenCrushed_Weapon {};
	NullablePromotable<int> WhenCrushed_Damage {};
	Valueable<bool> WhenCrushed_Warhead_Full { true };

	PhobosMap<AbstractTypeClass*, TechnoTypeClass*> Convert_ToHouseOrCountry {};

	Valueable<bool> SuppressKillWeapons { false };
	ValueableVector<WeaponTypeClass*> SuppressKillWeapons_Types {};

	Nullable<bool> NoQueueUpToEnter {};
	Nullable<bool> NoQueueUpToUnload {};
	Nullable<int> NoQueueUpToEnter_BoardDistance {};

	Nullable<bool> NoRearm_UnderEMP {};
	Nullable<bool> NoRearm_Temporal {};
	Nullable<bool> NoReload_UnderEMP {};
	Nullable<bool> NoReload_Temporal {};

	Nullable<bool> Cameo_AlwaysExist {};
	ValueableVector<TechnoTypeClass*> Cameo_AuxTechnos {};
	ValueableVector<TechnoTypeClass*> Cameo_NegTechnos {};
	DWORD Cameo_RequiredHouses { 0xFFFFFFFF };
	bool Cameo_AlwaysExistCheckMutex { false };
	bool Cameo_AlwaysExistRequirementMet { false };
	bool Cameo_AlwaysExistForCurrentPlayerActive { false };
	bool Cameo_AlwaysExistIsGreyCameoAbandonedProduct { true };
	Valueable<CSFText> UIDescription_Unbuildable {};
	PhobosPCXFile GreyCameoPCX {};

	Valueable<int> RateDown_Ammo { -2 };
	Valueable<int> RateDown_Delay { 0 };
	Valueable<int> RateDown_Cover { 0 };
	Valueable<bool> RateDown_Reset { false };

	Valueable<bool> CanManualReload { false };
	Valueable<bool> CanManualReload_ResetROF { true };
	Valueable<WarheadTypeClass*> CanManualReload_DetonateWarhead {};
	Valueable<int> CanManualReload_DetonateConsume { 0 };

	Nullable<int> Power {};
	Valueable<bool> BunkerableAnyway { false };

	Nullable<bool> JumpjetTilt {};
	Nullable<double> JumpjetTilt_ForwardAccelFactor {};
	Nullable<double> JumpjetTilt_ForwardSpeedFactor {};
	Nullable<double> JumpjetTilt_SidewaysRotationFactor {};
	Nullable<double> JumpjetTilt_SidewaysSpeedFactor {};

	Nullable<bool> NoTurret_TrackTarget {};

	Nullable<bool> RecountBurst {};

	Nullable<ColorStruct> AirstrikeLineColor {};

	Nullable<int> InitialSpawnsNumber {};
	ValueableVector<AircraftTypeClass*> Spawns_Queue {};

	Nullable<bool> Sinkable {};
	Valueable<int> SinkSpeed { 5 };
	Nullable<bool> Sinkable_SquidGrab { };

	int SpawnerRange { 0 };
	int EliteSpawnerRange { 0 };

	Nullable<bool> AmphibiousEnter {};
	Nullable<bool> AmphibiousUnload {};

	Nullable<bool> AlternateFLH_OnTurret { };
	Nullable<bool> AlternateFLH_ApplyVehicle { };
	Nullable<double> DamagedSpeed {};

	Nullable<AffectedHouse> RadarInvisibleToHouse {};

	Valueable<bool> AdvancedDrive_Reverse { true };
	Valueable<bool> AdvancedDrive_Reverse_FaceTarget { true };
	Valueable<Leptons> AdvancedDrive_Reverse_FaceTargetRange { Leptons(4096) };
	Valueable<Leptons> AdvancedDrive_Reverse_MinimumDistance { Leptons(640) };
	Valueable<int> AdvancedDrive_Reverse_RetreatDuration { 150 };
	Valueable<double> AdvancedDrive_Reverse_Speed { 0.85 };
	Valueable<bool> AdvancedDrive_Hover { false };
	Valueable<bool> AdvancedDrive_Hover_Sink { true };
	Valueable<bool> AdvancedDrive_Hover_Spin { true };
	Valueable<bool> AdvancedDrive_Hover_Tilt { true };
	Nullable<int> AdvancedDrive_Hover_Height {};
	Nullable<double> AdvancedDrive_Hover_Dampen {};
	Nullable<double> AdvancedDrive_Hover_Bob {};

	Valueable<bool> Harvester_CanGuardArea { false };
	Valueable<bool> Harvester_CanGuardArea_RequireTarget { false };

	std::unique_ptr<TiberiumEaterTypeClass> TiberiumEaterType {};

	Nullable<int> BattlePoints {};

	bool ForceWeapon_Check { false };

	Valueable<int> FireUp { -1 };
	Valueable<bool> FireUp_ResetInRetarget { true };

	Nullable<bool> DigitalDisplay_Health_FakeAtDisguise { };

	Valueable<int> EngineerRepairAmount { 0 };

	Nullable<bool> DebrisTypes_Limit {};
	ValueableVector<int> DebrisMinimums {};
	Valueable<bool> AttackMove_Follow { false };
	Valueable<bool> AttackMove_Follow_IncludeAir { false };
	Nullable<bool> AttackMove_StopWhenTargetAcquired {};
	Valueable<bool> AttackMove_PursuitTarget { false };
	Nullable<bool> SkipCrushSlowdown { };

	Nullable<bool> ApproachTarget_StopWhenInRange {};
	Valueable<bool> ApproachTarget_PursuitTarget {};

	Nullable<TechnoTypeClass*> RecuitedAs {};

	Valueable<bool> MultiWeapon { false };
	ValueableVector<bool> MultiWeapon_IsSecondary {};
	Valueable<int> MultiWeapon_SelectCount { 2 };
	bool ReadMultiWeapon { false };

	Valueable<int> ForceWeapon_Buildings { -1 };
	Valueable<int> ForceWeapon_Defenses { -1 };
	Valueable<int> ForceWeapon_Infantry { -1 };
	Valueable<int> ForceWeapon_Naval_Units { -1 };
	Valueable<int> ForceWeapon_Units { -1 };
	Valueable<int> ForceWeapon_Aircraft { -1 };
	Valueable<int> ForceAAWeapon_Infantry { -1 };
	Valueable<int> ForceAAWeapon_Units { -1 };
	Valueable<int> ForceAAWeapon_Aircraft { -1 };
	Valueable<int> ForceWeapon_Capture { -1 };

	Valueable<bool> AttackMove_Follow_IfMindControlIsFull { false };

	Nullable<int> PenetratesTransport_Level {};
	Valueable<double> PenetratesTransport_PassThroughMultiplier { 1.0 };
	Valueable<double> PenetratesTransport_FatalRateMultiplier { 1.0 };
	Valueable<double> PenetratesTransport_DamageMultiplier { 1.0 };

	ValueableIdx<VocClass*> VoiceIFVRepair { -1 };
	ValueableVector<int> VoiceWeaponAttacks {};
	ValueableVector<int> VoiceEliteWeaponAttacks {};
	Valueable<UnitTypeClass*> DefaultVehicleDisguise {};
	Nullable<bool> TurretResponse {};

	Valueable<bool> Unload_SkipPassengers { false };
	Valueable<bool> Unload_NoPassengers { false };
	Valueable<bool> Unload_SkipHarvester { false };
	Valueable<bool> Unload_NoTiberiums { false };

	std::unique_ptr<BlockTypeClass> BlockType {};
	Valueable<bool> CanBlock { false };

	Valueable<bool> IsSimpleDeployer_ConsiderPathfinding { false };
	Nullable<LandTypeFlags> IsSimpleDeployer_DisallowedLandTypes {};

	Valueable<PassiveAcquireModes> PassiveAcquireMode { PassiveAcquireModes::Normal };
	Valueable<bool> PassiveAcquireMode_Togglable { true };
	ValueableIdx<VocClass*> VoiceEnterAggressiveMode { -1 };
	ValueableIdx<VocClass*> VoiceExitAggressiveMode { -1 };
	ValueableIdx<VocClass*> VoiceEnterCeasefireMode { -1 };
	ValueableIdx<VocClass*> VoiceExitCeasefireMode { -1 };

	Nullable<bool> PlayerGuardModePursuit {};
	Nullable<Leptons> PlayerGuardModeStray {};
	Nullable<double> PlayerGuardModeGuardRangeMultiplier {};
	Nullable<Leptons> PlayerGuardModeGuardRangeAddend {};
	Nullable<Leptons> PlayerGuardStationaryStray {};
	Nullable<bool> AIGuardModePursuit {};
	Nullable<Leptons> AIGuardModeStray {};
	Nullable<double> AIGuardModeGuardRangeMultiplier {};
	Nullable<Leptons> AIGuardModeGuardRangeAddend {};
	Nullable<Leptons> AIGuardStationaryStray {};

	Vector2D<ThreatType> ThreatTypes { ThreatType::Normal, ThreatType::Normal };
	Vector2D<int> CombatDamages {};
	Vector2D<bool> AttackFriendlies {};

	ValueableVector<TechnoTypeClass*> TeamMember_ConsideredAs {};
	std::vector<PhobosFixedString<0x20>> WeaponGroupAs {};
	Valueable<bool> CanGoAboveTarget { false };

	Nullable<Mission> ParadropMission {};
	Nullable<Mission> AIParadropMission {};
	Nullable<Leptons> AreaGuardRange {};
	Nullable<Leptons> MaxGuardRange {};

	Nullable<bool> JumpjetClimbIgnoreBuilding {};

	Nullable<bool> BarrelOverTurret {};
	Valueable<int> BarrelOffset { 0 };
	Valueable<int> ExtraBarrelCount { 0 };
	std::vector<int> ExtraBarrelOffsets {};
	Valueable<int> ExtraTurretCount { 0 };
	std::vector<CoordStruct> ExtraTurretOffsets {};
	Valueable<int> BurstPerTurret { 0 };

	Nullable<bool> DriverKilled_KeptPassengers {};
	Nullable<bool> DriverKilled_KillPassengers {};

	Nullable<bool> Parasite_AllowWaterExit {};
	Nullable<bool> FlyNoWobbles {};
	Nullable<int> EliteSight {};
	Nullable<int> VeteranSight {};

	Nullable<double> VeteranReload {};
	Nullable<double> VeteranEmptyReload {};

	int TintColorAirstrike {};

	Nullable<bool> UncloakWhenLowHealth {};
	Nullable<bool> NoAutoFire_AI {};

	ValueableVector<TechnoTypeClass*> WreckageType { };
	Nullable<double> WreckageInitialHealthPercent { };
	Valueable<bool> WreckageDeactive { true };
	Valueable<bool> WreckageMarkUp { true };
	Valueable<OwnerHouseKind> WreckageOwner { OwnerHouseKind::Default };
	Valueable<bool> WreckageLeaveOnWater { false };
	Valueable<bool>WreckageLeaveInAir { false };
	Valueable<bool> WreckageSwapLocomotor { false };

	Valueable<bool> TargetExtraThreat { false };
	ValueableVector<DirStruct> TargetExtraThreat_Angles {};
	ValueableVector<double>  TargetExtraThreat_Multipliers {};
	Valueable<bool> TargetExtraThreat_Turret { true };

	Valueable<DisplayInfoType> SelectedInfo_UpperType { DisplayInfoType::Shield };
	Valueable<int> SelectedInfo_UpperIndex {};
	Valueable<ColorStruct> SelectedInfo_UpperColor {};
	Valueable<int> SelectedInfo_UpperDivisor {};
	Valueable<DisplayInfoType> SelectedInfo_BelowType { DisplayInfoType::Health };
	Valueable<int> SelectedInfo_BelowIndex {};
	Valueable<ColorStruct> SelectedInfo_BelowColor {};
	Valueable<int> SelectedInfo_BelowDivisor {};
	Valueable<DisplayInfoType> SelectedInfo_CameoType { DisplayInfoType::Ammo };
	Valueable<int> SelectedInfo_CameoIndex {};
	Nullable<SHPCaches*> SelectedInfo_Button {};
	Nullable<CSFText> UIDescription_HoveredInfo {};

	Nullable<double> VeteranRange {};
	Nullable<double> VeteranCritChance {};

	ValueableVector<int> DefaultToGuardArea_Modes {};
	ValueableVector<int> DefaultToGuardArea_AIModes {};
#pragma endregion

public:

	TechnoTypeExtData(TechnoTypeClass* pObj) : ObjectTypeExtData(pObj)
	{ }

	void InitializeConstant();
	virtual void Initialize();

	TechnoTypeExtData() = default;

	virtual ~TechnoTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	TechnoTypeClass* This() const { return reinterpret_cast<TechnoTypeClass*>(this->AttachedToObject); }
	const TechnoTypeClass* This_Const() const { return reinterpret_cast<const TechnoTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

public:

	void LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI);

	bool IsSecondary(int nWeaponIndex);
	const std::string GetGunnerID(int idx) const;

	void AdjustCrushProperties();
	void CalculateSpawnerRange();
	void ResetSpawnerRange() {
		this->SpawnerRange = 0;
		this->EliteSpawnerRange = 0;
	}

	// Ares 0.A
	const char* GetSelectionGroupID() const;

	bool IsGenericPrerequisite() const;

	void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0, int turIdx = -1);

	static WeaponStruct* GetWeaponStruct(TechnoTypeClass* pThis, int nWeaponIndex, bool isElite);

private:

	template<typename T>
	void Serialize(T& Stm);

public:
	static COMPILETIMEEVAL double TurretMultiOffsetDefaultMult { 1.0 };
	static COMPILETIMEEVAL double TurretMultiOffsetOneByEightMult { 0.125 };
	static bool SelectWeaponMutex;

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);

	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<BurstFLHBundle>& nFLH, const char* pPrefixTag);
	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, ColletiveCoordStructVectorData& nFLH, ColletiveCoordStructVectorData& nEFlh, const char** pPrefixTag);
	static void GetFLH(INI_EX& exArtINI, const char* pArtSection, Nullable<CoordStruct>& nFlh, Nullable<CoordStruct>& nEFlh, const char* pFlag);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const std::string& pID);
	static AnimTypeClass* GetSinkAnim(TechnoClass* pThis);
	static double GetTunnelSpeed(TechnoClass* pThis, RulesClass* pRules);
	static bool PassangersAllowed(TechnoTypeClass* pThis, TechnoTypeClass* pPassanger);

	static VoxelStruct* GetBarrelsVoxel(TechnoTypeClass* const pThis, int const nIdx);
	static VoxelStruct* GetTurretsVoxel(TechnoTypeClass* const pThis, int const nIdx);

	static VoxelStruct* GetBarrelsVoxelFixedUp(TechnoTypeClass* const pThis, int const nIdx);
	static VoxelStruct* GetTurretsVoxelFixedUp(TechnoTypeClass* const pThis, int const nIdx);

	static bool CanBeBuiltAt(TechnoTypeClass* pProduct, BuildingTypeClass* pFactoryType);

	static bool CameoIsElite(TechnoTypeClass* pType, HouseClass* pHouse);
	static BSurface* GetPCXSurface(TechnoTypeClass* pType, HouseClass* pHouse);

	static bool CarryallCanLift(AircraftTypeClass* pCarryAll, UnitClass* Target);

	static void LoadTurrets(TechnoTypeClass* pType, CCINIClass* pINI);
	static int* GetTurretWeaponIndex(TechnoTypeClass* pType, size_t idx);
	static WeaponStruct* GetWeapon(TechnoTypeClass* pType, int const idx, bool elite);
	static void ReadWeaponStructDatas(TechnoTypeClass* pType, CCINIClass* pRules);

	int SelectForceWeapon(TechnoClass* pThis, AbstractClass* pTarget);
	int SelectMultiWeapon(TechnoClass* pThis, AbstractClass* pTarget);
	int SelectPhobosWeapon(TechnoClass* pThis, AbstractClass* pTarget);

	void ParseVoiceWeaponAttacks(INI_EX& exINI, const char* pSection, ValueableVector<int>& n, ValueableVector<int>& nE);
	void UpdateAdditionalAttributes(CCINIClass* const pINI);

};

class NOVTABLE FakeTechnoTypeClass
//: public TechnoTypeClass
{
public:
	//TODO : replace bigger hook with LJMP patch
	static WeaponStruct* __fastcall __GetWeapon(TechnoTypeClass* pThis , discard_t , int which);
	static WeaponStruct* __fastcall __GetEliteWeapon(TechnoTypeClass* pThis , discard_t ,int which);
	static int  __fastcall __GetWeaponTurretIndex(TechnoTypeClass* pThis , discard_t ,int which);
	static SHPCaches* __fastcall __GetCameo(TechnoTypeClass* pThis);

};

class TechnoTypeExtContainer final //: public Container<TechnoTypeExtData>
{
public:
	static TechnoTypeExtContainer Instance;


	static int __fastcall __Repair_Cost(TechnoTypeClass* pThis);
	static int __fastcall  __Time_To_Build(TechnoTypeClass* pThis);

	COMPILETIMEEVAL FORCEDINLINE  TechnoTypeExtData* GetExtAttribute(TechnoTypeClass* key)
	{
		return (TechnoTypeExtData*)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	COMPILETIMEEVAL FORCEDINLINE TechnoTypeExtData* Find(TechnoTypeClass* key)
	{
		return this->GetExtAttribute(key);
	}

	COMPILETIMEEVAL FORCEDINLINE TechnoTypeExtData* TryFind(TechnoTypeClass* key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}
};
