#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/MultiBoolFixedArray.h>

#include <New/Entity/InsigniaData.h>
#include <New/Entity/LaserTrailDataEntry.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/CursorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/TheaterTypeClass.h>
#include <New/Type/DroppodProperties.h>
#include <New/Type/CrateTypeClass.h>

#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

#include <New/Entity/PassengerDeletionTypeClass.h>
#include <New/Entity/TiberiumEaterTypeClass.h>
#include <New/Entity/BlockTypeClass.h>
#include <New/Entity/AresAttachEffectTypeClass.h>

#include <Misc/Defines.h>

#include <Ext/ObjectType/Body.h>

#include <FileSystem.h>

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

	~ImageStatusses()
	{
		if (Loaded) {
			GameDelete<true, true>(Images.VXL);
			GameDelete<true, true>(Images.HVA);

			Images.VXL = nullptr;
			Images.HVA = nullptr;
			Loaded = false;
		}
	}

	static ImageStatusses ReadVoxel(const char* const nKey);

	void swap(VoxelStruct& from) {

		if (from.VXL != this->Images.VXL) {
			std::swap(from.VXL, this->Images.VXL);
		}

		if (from.HVA != this->Images.HVA) {
			std::swap(from.HVA, this->Images.HVA);
		}

		this->Loaded = this->Images.VXL || this->Images.HVA;
	}

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		if(Loaded){
			GameDelete<true, true>(Images.VXL);
			GameDelete<true, true>(Images.HVA);

			Images.VXL = nullptr;
			Images.HVA = nullptr;
			Loaded = false;
		}

		return true;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		return true;
	}
};

struct BurstFLHBundle
{
	std::vector<CoordStruct> Flh {};
	std::vector<CoordStruct> EFlh {};

	OPTIONALINLINE bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		// support pointer to this type
		return Stm
			.Process(this->Flh, RegisterForChange)
			.Process(this->EFlh, RegisterForChange)
			.Success()
			;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& Stm) const
	{
		// remember this object address
		return Stm
			.Process(this->Flh)
			.Process(this->EFlh)
			.Success()
			;
	}
};

class Matrix3D;
class DigitalDisplayTypeClass;
class SelectBoxTypeClass;
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
	Valueable<AffectedHouse> RadarJamHouses { AffectedHouse::Enemies };
	Valueable<int> RadarJamDelay { 30 };
	ValueableVector<BuildingTypeClass*> RadarJamAffect {};
	ValueableVector<BuildingTypeClass*> RadarJamIgnore {};

	Nullable<int> InhibitorRange {};
	Nullable<int> DesignatorRange {};

	//Enemy Inhibitors
	Nullable<int> SuppressorRange {};

	//Enemy Designator
	Nullable<int> AttractorRange {};

	Valueable<Leptons> MindControlRangeLimit {};
	Valueable<bool> MindControl_IgnoreSize { true };
	Valueable<int> MindControlSize { 1 };

	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_EliteAbilities {};
	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_VeteranAbilities {};

	ValueableIdxVector<ImmunityTypeClass> E_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass> V_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass> R_ImmuneToType {};

	Valueable<bool> Interceptor { false };
	Valueable<AffectedHouse> Interceptor_CanTargetHouses { AffectedHouse::Enemies };
	Promotable<Leptons> Interceptor_GuardRange {};
	Valueable<bool> Interceptor_GuardRange_IsCylindrical { false };
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
	Valueable<bool> Interceptor_ApplyFirepowerMult { true };

	Nullable<PartialVector3D<int>> TurretOffset {};
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

	PassengerDeletionTypeClass PassengerDeletionType {};

	Valueable<bool> Death_NoAmmo { false };
	Valueable<int> Death_Countdown { 0 };
	Valueable<KillMethod> Death_Method { KillMethod::None };
	Valueable<bool> Death_WithMaster { false };
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
	Valueable<bool> AutoDeath_Nonexist_AllowLimboed { true };
	ValueableVector<TechnoTypeClass*> AutoDeath_Exist {};
	Valueable<AffectedHouse> AutoDeath_Exist_House { AffectedHouse::Owner };
	Valueable<bool> AutoDeath_Exist_Any { false };
	Valueable<bool> AutoDeath_Exist_AllowLimboed { true };
	Valueable<AnimTypeClass*> AutoDeath_VanishAnimation { nullptr };
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

	Valueable<bool> DestroyAnim_Random { true };
	PhobosMap<WarheadTypeClass*, std::vector<AnimTypeClass*>> DestroyAnimSpecific {};
	Valueable<bool> NotHuman_RandomDeathSequence { false };

	Valueable<InfantryTypeClass*> DefaultDisguise { nullptr };

	Nullable<int> OpenTopped_RangeBonus {};
	Nullable<float> OpenTopped_DamageMultiplier {};
	Nullable<bool> OpenTopped_DecloakToFire {};
	Nullable<int> OpenTopped_WarpDistance {};
	Valueable<bool> OpenTopped_IgnoreRangefinding { false };
	Valueable<bool> OpenTopped_AllowFiringIfDeactivated { true };
	Valueable<bool> OpenTopped_ShareTransportTarget { true };
	Valueable<bool> OpenTopped_UseTransportRangeModifiers { false };
	Valueable<bool> OpenTopped_CheckTransportDisableWeapons { false };

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
	Valueable<bool> Passengers_SyncOwner { false };
	Valueable<bool> Passengers_SyncOwner_RevertOnExit { true };

	Valueable<bool> Aircraft_DecreaseAmmo { true };

	ValueableVector<LaserTrailDataEntry> LaserTrailData {};
	Valueable<CSFText> EnemyUIName {};
	Valueable<bool> UseDisguiseMovementSpeed { false };

	Promotable<SHPStruct*> Insignia {};
	Valueable<Vector3D<int>> InsigniaFrames { { -1, -1, -1 } };
	Promotable<int> InsigniaFrame { -1 };
	Nullable<bool> Insignia_ShowEnemy {};
	std::vector<InsigniaData> Insignia_Weapon {};
	std::vector<Promotable<SHPStruct*>> Insignia_Passengers {};
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
	ValueableIdx<VoxClass> EVA_Sold { -1 };
	ValueableIdx<VocClass> SellSound { -1 };

	Valueable<bool> Explodes_KillPassengers { true };

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
	NullableIdx<VocClass> DiskLaserChargeUp {};
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

	Valueable<AffectedHouse> Draw_MindControlLink { AffectedHouse::All };

	NullableVector<int> Overload_Count {};
	NullableVector<int> Overload_Damage {};
	NullableVector<int> Overload_Frames {};
	NullableIdx<VocClass> Overload_DeathSound {};
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
	Valueable<double> CrashSpinVerticalRate { 1.0 };
	ValueableIdx<VocClass> ParasiteExit_Sound { -1 };

	Nullable<SHPStruct*> PipShapes01 {};
	Nullable<SHPStruct*> PipShapes02 {};
	Nullable<SHPStruct*> PipGarrison {};
	Valueable<int> PipGarrison_FrameIndex { 0 };
	CustomPalette PipGarrison_Palette { CustomPalette::PaletteMode::Default };

	Valueable<bool> HealthNumber_Show { false };
	Valueable<bool> HealthNumber_Percent { false };
	Nullable<Point2D> Healnumber_Offset {};
	Nullable<SHPStruct*> HealthNumber_SHP {};
	Nullable<Point2D> Healnumber_Decrement {};
	Nullable<SHPStruct*> HealthBarSHP {};
	Nullable<SHPStruct*> HealthBarSHP_Selected {};
	Valueable<int> HealthBarSHPBracketOffset { 0 };
	Valueable<Point3D> HealthBarSHP_HealthFrame { { 18, 16, 17 } };
	CustomPalette HealthBarSHP_Palette { CustomPalette::PaletteMode::Temperate };

	Valueable<Point2D> HealthBarSHP_PointOffset { { 0, 0 } };
	Valueable<bool> HealthbarRemap { false };

	Valueable<SHPStruct*> GClock_Shape {};
	Valueable<TranslucencyLevel> GClock_Transculency {};
	CustomPalette GClock_Palette { CustomPalette::PaletteMode::Default };

	Valueable<bool> ROF_Random { true };
	Nullable<Point2D> Rof_RandomMinMax {};

	ValueableIdx<VoxClass> Eva_Complete { -1 };
	ValueableIdx<VocClass> VoiceCreate { -1 };
	Valueable<bool> VoiceCreate_Instant { false };
	Valueable<bool> CreateSound_Enable { true };

	Valueable<bool> SlaveFreeSound_Enable { true };
	NullableIdx<VocClass> SlaveFreeSound {};
	Valueable<bool> NoAirportBound_DisableRadioContact { false };
	Nullable<AnimTypeClass*> SinkAnim {};
	Nullable<double> Tunnel_Speed {};
	Valueable<HoverTypeClass*> HoverType { nullptr };

	Valueable<bool> Gattling_Overload { false };
	Nullable<int> Gattling_Overload_Damage {};
	Nullable<int> Gattling_Overload_Frames {};
	NullableIdx<VocClass> Gattling_Overload_DeathSound {};
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
	ValueableIdx<VoxClass> Promote_Elite_Eva { -1 };
	ValueableIdx<VoxClass> Promote_Vet_Eva { -1 };
	NullableIdx<VocClass> Promote_Elite_Sound {};
	NullableIdx<VocClass> Promote_Vet_Sound {};
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
	Valueable<int> Convert_Deploy_Delay { -1 };
	Valueable<TechnoTypeClass*> Convert_Script { nullptr };
	ValueableVector<int> Convert_Script_Prereq {};
	Valueable<TechnoTypeClass*> Convert_Water { nullptr };
	Valueable<TechnoTypeClass*> Convert_Land { nullptr };
	Valueable<bool> Convert_ResetMindControl { false };

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

	NullableIdx<VocClass> DigInSound {};
	NullableIdx<VocClass> DigOutSound {};
	Nullable<AnimTypeClass*> DigInAnim {};
	Nullable<AnimTypeClass*> DigOutAnim {};

	ValueableIdx<VoxClass> EVA_UnitLost { -1 };

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
	NullableIdx<VocClass> CloakSound {};
	NullableIdx<VocClass> DecloakSound {};

	ValueableIdx<VocClass> VoiceRepair { -1 };
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

	Nullable<SHPStruct*> AmmoPip_shape {};
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

	ValueableIdx<VocClass> HijackerEnterSound { -1 };
	ValueableIdx<VocClass> HijackerLeaveSound { -1 };

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
	ValueableIdx<VocClass> Bounty_ReceiveSound {};

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

	NullableIdx<VocClass> VoiceCantDeploy {};
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
	Nullable<SHPStruct*> HealthBar_Border {};
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

	Valueable<bool> ReloadInTransport { false };
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

	ValueableIdxVector<VocClass> VoicePickup {};

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
	Valueable<bool> BuildLimitGroup_ContentIfAnyMatch { false };
	Valueable<bool> BuildLimitGroup_NotBuildableIfQueueMatch { false };
	ValueableVector<TechnoTypeClass*> BuildLimitGroup_ExtraLimit_Types {};
	ValueableVector<int> BuildLimitGroup_ExtraLimit_Nums {};
	ValueableVector<int> BuildLimitGroup_ExtraLimit_MaxCount {};
	Valueable<int> BuildLimitGroup_ExtraLimit_MaxNum { 0 };
#pragma endregion

	NullableVector<int> Tiberium_PipIdx {};
	Nullable<int> Tiberium_EmptyPipIdx {};
	Valueable<SHPStruct*> Tiberium_PipShapes { nullptr };
	CustomPalette Tiberium_PipShapes_Palette {};

	Nullable<ColorStruct> Tint_Color {};
	Valueable<double> Tint_Intensity { 0.0 };
	Valueable<AffectedHouse> Tint_VisibleToHouses { AffectedHouse::All };

	AEAttachInfoTypeClass PhobosAttachEffects {};

	Valueable<bool> KeepTargetOnMove { false };
	Valueable<int> KeepTargetOnMove_Weapon { -1 };
	Nullable<Leptons> KeepTargetOnMove_ExtraDistance {};
	Valueable<bool> KeepTargetOnMove_NoMorePursuit { true };

	Nullable<bool> AllowAirstrike {};

	Nullable<bool> ForbidParallelAIQueues {};

	Valueable<bool> IgnoreForBaseCenter { false };

	Nullable<AnimTypeClass*> Wake {};
	Valueable<bool> Spawner_AttackImmediately { false };
	Valueable<bool> Spawner_UseTurretFacing { false };

	ValueableIdx<VoxClass> EVA_Combat { -1 };
	Nullable<bool> CombatAlert {};
	Nullable<bool> CombatAlert_UseFeedbackVoice {};
	Nullable<bool> CombatAlert_UseAttackVoice {};
	Nullable<bool> CombatAlert_UseEVA {};
	Nullable<bool> CombatAlert_NotBuilding {};
	Nullable<int> SubterraneanHeight {};

	Valueable<Leptons> Spawner_RecycleRange { Leptons(-1) };
	Valueable<CoordStruct> Spawner_RecycleFLH { { 0, 0, 0 } };
	Valueable<bool> Spawner_RecycleOnTurret { false };
	Valueable<AnimTypeClass*> Spawner_RecycleAnim { nullptr };

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
	Valueable<bool> ForceWeapon_InRange_ApplyRangeModifiers { false };
	ValueableVector<int> ForceAAWeapon_InRange {};
	ValueableVector<double> ForceAAWeapon_InRange_Overrides {};
	Valueable<bool> ForceAAWeapon_InRange_ApplyRangeModifiers { false };
	Valueable<bool> ForceWeapon_InRange_TechnoOnly { false };

	Nullable<bool> UnitIdleRotateTurret {};
	Nullable<bool> UnitIdlePointToMouse {};

	Valueable<double> FallingDownDamage { 1.0 };
	Nullable<double> FallingDownDamage_Water {};
	Valueable<bool> FallingDownDamage_AllowEMP { true };

	Valueable<bool> HoverDrownable { true };

	bool ExtraThreat_Enabled;
	Nullable<double> ExtraThreat_IsThreat { false };
	Valueable<bool> AlwaysConsideredThreat {};
	Nullable<double> ExtraThreat_InRange {};
	Nullable<double> ExtraThreatCoefficient_InRangeDistance {};
	Nullable<double> ExtraThreatCoefficient_Facing {};
	Nullable<double> ExtraThreatCoefficient_DistanceToLastTarget {};

	NullableIdx<CrateTypeClass> DropCrate {};

	Promotable<WarheadTypeClass*> WhenCrushed_Warhead {};
	Promotable<WeaponTypeClass*> WhenCrushed_Weapon {};
	NullablePromotable<int> WhenCrushed_Damage {};
	Valueable<bool> WhenCrushed_Warhead_Full { true };

	PhobosMap<AbstractTypeClass*, TechnoTypeClass*> Convert_ToHouseOrCountry {};

	Valueable<bool> SuppressKillWeapons { false };
	ValueableVector<WeaponTypeClass*> SuppressKillWeapons_Types {};

	Nullable<bool> NoQueueUpToEnter {};
	Nullable<bool> NoQueueUpToUnload {};

	Nullable<bool> NoRearm_UnderEMP {};
	Nullable<bool> NoRearm_Temporal {};
	Nullable<bool> NoReload_UnderEMP {};
	Nullable<bool> NoReload_Temporal {};

	Nullable<bool> Cameo_AlwaysExist {};
	ValueableVector<TechnoTypeClass*> Cameo_AuxTechnos {};
	ValueableVector<TechnoTypeClass*> Cameo_NegTechnos {};
	bool CameoCheckMutex { false }; // Not read from ini
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
	Valueable<double> JumpjetTilt_ForwardAccelFactor { 1.0 };
	Valueable<double> JumpjetTilt_ForwardSpeedFactor { 1.0 };
	Valueable<double> JumpjetTilt_SidewaysRotationFactor { 1.0 };
	Valueable<double> JumpjetTilt_SidewaysSpeedFactor { 1.0 };

	Nullable<bool> NoTurret_TrackTarget {};

	Nullable<bool> RecountBurst {};

	Nullable<ColorStruct> AirstrikeLineColor {};

	Nullable<int> InitialSpawnsNumber {};
	ValueableVector<AircraftTypeClass*> Spawns_Queue {};

	Nullable<bool> Sinkable {};
	Valueable<int> SinkSpeed { 5 };
	Valueable<bool> Sinkable_SquidGrab { true };

	int SpawnerRange { 0 };
	int EliteSpawnerRange { 0 };

	Nullable<bool> AmphibiousEnter {};
	Nullable<bool> AmphibiousUnload {};

	Valueable<bool> AlternateFLH_OnTurret { true };
	Valueable<bool> AlternateFLH_ApplyVehicle { false };
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

	Valueable<bool> DigitalDisplay_Health_FakeAtDisguise { true };

	Valueable<int> EngineerRepairAmount { 0 };

	Nullable<bool> DebrisTypes_Limit {};
	ValueableVector<int> DebrisMinimums {};
	Valueable<bool> AttackMove_Follow { false };
	Valueable<bool> AttackMove_Follow_IncludeAir { false };
	Nullable<bool> AttackMove_StopWhenTargetAcquired {};
	Valueable<bool> AttackMove_PursuitTarget { false };
	Valueable<bool> SkipCrushSlowdown { false };

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

	ValueableIdx<VocClass> VoiceIFVRepair { -1 };
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
	ValueableIdx<VocClass> VoiceEnterAggressiveMode { -1 };
	ValueableIdx<VocClass> VoiceExitAggressiveMode { -1 };
	ValueableIdx<VocClass> VoiceEnterCeasefireMode { -1 };
	ValueableIdx<VocClass> VoiceExitCeasefireMode { -1 };

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

	Valueable<int> OpenTransport_RangeBonus { 0 };
	Valueable<float> OpenTransport_DamageMultiplier { 1.0f };

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

	Valueable<bool> DriverKilled_KeptPassengers {};
	Nullable<bool> DriverKilled_KillPassengers {};

	int TintColorAirstrike {};
#pragma endregion

public:

	TechnoTypeExtData(TechnoTypeClass* pObj) : ObjectTypeExtData(pObj)
	{ }

	void InitializeConstant();
	virtual void Initialize();

	TechnoTypeExtData(TechnoTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

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
#ifndef Logged
	template<typename T>
	void Serialize(T& Stm)
	{
		auto debugProcess = [&Stm](auto& field, const char* fieldName) -> auto&
			{
				if constexpr (std::is_same_v<T, PhobosStreamWriter>)
				{
					size_t beforeSize = Stm.Getstream()->Size();
					auto& result = Stm.Process(field);
					size_t afterSize = Stm.Getstream()->Size();
					GameDebugLog::Log("[TechnoTypextData] SAVE %s: size %zu -> %zu (+%zu)\n",
						fieldName, beforeSize, afterSize, afterSize - beforeSize);
					return result;
				}
				else
				{
					size_t beforeOffset = Stm.Getstream()->Offset();
					bool beforeSuccess = Stm.Success();
					auto& result = Stm.Process(field);
					size_t afterOffset = Stm.Getstream()->Offset();
					bool afterSuccess = Stm.Success();
					GameDebugLog::Log("[TechnoTypextData] LOAD %s: offset %zu -> %zu (+%zu), success: %s -> %s\n",
						fieldName, beforeOffset, afterOffset, afterOffset - beforeOffset,
						beforeSuccess ? "true" : "false", afterSuccess ? "true" : "false");
					if (!afterSuccess && beforeSuccess)
						GameDebugLog::Log("[TechnoTypextData] ERROR: %s caused stream failure!\n", fieldName);
					return result;
				}
			};

		debugProcess(this->Landing_Anim, "Landing_Anim");
		debugProcess(this->Landing_AnimOnWater, "Landing_AnimOnWater");
		debugProcess(this->HealthBar_Hide, "HealthBar_Hide");
		debugProcess(this->HealthBar_HidePips, "HealthBar_HidePips");
		debugProcess(this->HealthBar_Permanent, "HealthBar_Permanent");
		debugProcess(this->HealthBar_Permanent_PipScale, "HealthBar_Permanent_PipScale");
		debugProcess(this->UIDescription, "UIDescription");
		debugProcess(this->LowSelectionPriority, "LowSelectionPriority");
		debugProcess(this->LowDeployPriority, "LowDeployPriority");
		debugProcess(this->MindControlRangeLimit, "MindControlRangeLimit");
		debugProcess(this->MindControl_IgnoreSize, "MindControl_IgnoreSize");
		debugProcess(this->MindControlSize, "MindControlSize");
		debugProcess(this->Phobos_EliteAbilities, "Phobos_EliteAbilities");
		debugProcess(this->Phobos_VeteranAbilities, "Phobos_VeteranAbilities");
		debugProcess(this->E_ImmuneToType, "E_ImmuneToType");
		debugProcess(this->V_ImmuneToType, "V_ImmuneToType");
		debugProcess(this->R_ImmuneToType, "R_ImmuneToType");
		debugProcess(this->Interceptor, "Interceptor");
		debugProcess(this->Interceptor_CanTargetHouses, "Interceptor_CanTargetHouses");
		debugProcess(this->Interceptor_GuardRange, "Interceptor_GuardRange");
		debugProcess(this->Interceptor_MinimumGuardRange, "Interceptor_MinimumGuardRange");
		debugProcess(this->Interceptor_TargetingDelay, "Interceptor_TargetingDelay");
		debugProcess(this->Interceptor_Weapon, "Interceptor_Weapon");
		debugProcess(this->Interceptor_DeleteOnIntercept, "Interceptor_DeleteOnIntercept");
		debugProcess(this->Interceptor_WeaponOverride, "Interceptor_WeaponOverride");
		debugProcess(this->Interceptor_WeaponReplaceProjectile, "Interceptor_WeaponReplaceProjectile");
		debugProcess(this->Interceptor_WeaponCumulativeDamage, "Interceptor_WeaponCumulativeDamage");
		debugProcess(this->Interceptor_KeepIntact, "Interceptor_KeepIntact");
		debugProcess(this->Interceptor_ConsiderWeaponRange, "Interceptor_ConsiderWeaponRange");
		debugProcess(this->Interceptor_OnlyTargetBullet, "Interceptor_OnlyTargetBullet");
		debugProcess(this->Interceptor_ApplyFirepowerMult, "Interceptor_ApplyFirepowerMult");
		debugProcess(this->GroupAs, "GroupAs");
		debugProcess(this->RadarJamRadius, "RadarJamRadius");
		debugProcess(this->RadarJamHouses, "RadarJamHouses");
		debugProcess(this->RadarJamDelay, "RadarJamDelay");
		debugProcess(this->RadarJamAffect, "RadarJamAffect");
		debugProcess(this->RadarJamIgnore, "RadarJamIgnore");
		debugProcess(this->InhibitorRange, "InhibitorRange");
		debugProcess(this->DesignatorRange, "DesignatorRange");
		debugProcess(this->TurretOffset, "TurretOffset");
		debugProcess(this->TurretShadow, "TurretShadow");
		debugProcess(this->ShadowIndices, "ShadowIndices");
		debugProcess(this->ShadowIndex_Frame, "ShadowIndex_Frame");
		debugProcess(this->ShadowSizeCharacteristicHeight, "ShadowSizeCharacteristicHeight");
		debugProcess(this->Powered_KillSpawns, "Powered_KillSpawns");
		debugProcess(this->Spawn_LimitedRange, "Spawn_LimitedRange");
		debugProcess(this->Spawn_LimitedExtraRange, "Spawn_LimitedExtraRange");
		debugProcess(this->AdvancedDrive_Reverse, "AdvancedDrive_Reverse");
		debugProcess(this->AdvancedDrive_Reverse_FaceTarget, "AdvancedDrive_Reverse_FaceTarget");
		debugProcess(this->AdvancedDrive_Reverse_FaceTargetRange, "AdvancedDrive_Reverse_FaceTargetRange");
		debugProcess(this->AdvancedDrive_Reverse_MinimumDistance, "AdvancedDrive_Reverse_MinimumDistance");
		debugProcess(this->AdvancedDrive_Reverse_RetreatDuration, "AdvancedDrive_Reverse_RetreatDuration");
		debugProcess(this->AdvancedDrive_Reverse_Speed, "AdvancedDrive_Reverse_Speed");
		debugProcess(this->AdvancedDrive_Hover, "AdvancedDrive_Hover");
		debugProcess(this->AdvancedDrive_Hover_Sink, "AdvancedDrive_Hover_Sink");
		debugProcess(this->AdvancedDrive_Hover_Spin, "AdvancedDrive_Hover_Spin");
		debugProcess(this->AdvancedDrive_Hover_Tilt, "AdvancedDrive_Hover_Tilt");
		debugProcess(this->AdvancedDrive_Hover_Height, "AdvancedDrive_Hover_Height");
		debugProcess(this->AdvancedDrive_Hover_Dampen, "AdvancedDrive_Hover_Dampen");
		debugProcess(this->AdvancedDrive_Hover_Bob, "AdvancedDrive_Hover_Bob");
		debugProcess(this->Harvester_CanGuardArea, "Harvester_CanGuardArea");
		debugProcess(this->Harvester_CanGuardArea_RequireTarget, "Harvester_CanGuardArea_RequireTarget");
		debugProcess(this->TiberiumEaterType, "TiberiumEaterType");
		debugProcess(this->Spawner_DelayFrames, "Spawner_DelayFrames");
		debugProcess(this->Harvester_Counted, "Harvester_Counted");
		debugProcess(this->Promote_IncludeSpawns, "Promote_IncludeSpawns");
		debugProcess(this->ImmuneToCrit, "ImmuneToCrit");
		debugProcess(this->MultiMindControl_ReleaseVictim, "MultiMindControl_ReleaseVictim");
		debugProcess(this->CameoPriority, "CameoPriority");
		debugProcess(this->NoManualMove, "NoManualMove");
		debugProcess(this->InitialStrength, "InitialStrength");
		debugProcess(this->Death_NoAmmo, "Death_NoAmmo");
		debugProcess(this->Death_Countdown, "Death_Countdown");
		debugProcess(this->Death_Method, "Death_Method");
		debugProcess(this->AutoDeath_Nonexist, "AutoDeath_Nonexist");
		debugProcess(this->AutoDeath_Nonexist_House, "AutoDeath_Nonexist_House");
		debugProcess(this->AutoDeath_Nonexist_Any, "AutoDeath_Nonexist_Any");
		debugProcess(this->AutoDeath_Nonexist_AllowLimboed, "AutoDeath_Nonexist_AllowLimboed");
		debugProcess(this->AutoDeath_Exist, "AutoDeath_Exist");
		debugProcess(this->AutoDeath_Exist_House, "AutoDeath_Exist_House");
		debugProcess(this->AutoDeath_Exist_Any, "AutoDeath_Exist_Any");
		debugProcess(this->AutoDeath_Exist_AllowLimboed, "AutoDeath_Exist_AllowLimboed");
		debugProcess(this->AutoDeath_VanishAnimation, "AutoDeath_VanishAnimation");
		debugProcess(this->Convert_AutoDeath, "Convert_AutoDeath");
		debugProcess(this->Death_WithMaster, "Death_WithMaster");
		debugProcess(this->AutoDeath_MoneyExceed, "AutoDeath_MoneyExceed");
		debugProcess(this->AutoDeath_MoneyBelow, "AutoDeath_MoneyBelow");
		debugProcess(this->AutoDeath_LowPower, "AutoDeath_LowPower");
		debugProcess(this->AutoDeath_FullPower, "AutoDeath_FullPower");
		debugProcess(this->AutoDeath_PassengerExceed, "AutoDeath_PassengerExceed");
		debugProcess(this->AutoDeath_PassengerBelow, "AutoDeath_PassengerBelow");
		debugProcess(this->AutoDeath_ContentIfAnyMatch, "AutoDeath_ContentIfAnyMatch");
		debugProcess(this->AutoDeath_OwnedByPlayer, "AutoDeath_OwnedByPlayer");
		debugProcess(this->AutoDeath_OwnedByAI, "AutoDeath_OwnedByAI");
		debugProcess(this->Slaved_ReturnTo, "Slaved_ReturnTo");
		debugProcess(this->Death_IfChangeOwnership, "Death_IfChangeOwnership");
		debugProcess(this->ShieldType, "ShieldType");
		debugProcess(this->WarpOut, "WarpOut");
		debugProcess(this->WarpIn, "WarpIn");
		debugProcess(this->WarpAway, "WarpAway");
		debugProcess(this->ChronoTrigger, "ChronoTrigger");
		debugProcess(this->ChronoDistanceFactor, "ChronoDistanceFactor");
		debugProcess(this->ChronoMinimumDelay, "ChronoMinimumDelay");
		debugProcess(this->ChronoRangeMinimum, "ChronoRangeMinimum");
		debugProcess(this->ChronoDelay, "ChronoDelay");
		debugProcess(this->WarpInWeapon, "WarpInWeapon");
		debugProcess(this->WarpInMinRangeWeapon, "WarpInMinRangeWeapon");
		debugProcess(this->WarpOutWeapon, "WarpOutWeapon");
		debugProcess(this->WarpInWeapon_UseDistanceAsDamage, "WarpInWeapon_UseDistanceAsDamage");
		debugProcess(this->OreGathering_Anims, "OreGathering_Anims");
		debugProcess(this->OreGathering_Tiberiums, "OreGathering_Tiberiums");
		debugProcess(this->OreGathering_FramesPerDir, "OreGathering_FramesPerDir");
		debugProcess(this->LaserTrailData, "LaserTrailData");
		debugProcess(this->DestroyAnim_Random, "DestroyAnim_Random");
		debugProcess(this->DestroyAnimSpecific, "DestroyAnimSpecific");
		debugProcess(this->NotHuman_RandomDeathSequence, "NotHuman_RandomDeathSequence");
		debugProcess(this->DefaultDisguise, "DefaultDisguise");
		debugProcess(this->PassengerDeletionType, "PassengerDeletionType");
		debugProcess(this->OpenTopped_RangeBonus, "OpenTopped_RangeBonus");
		debugProcess(this->OpenTopped_DamageMultiplier, "OpenTopped_DamageMultiplier");
		debugProcess(this->OpenTopped_DecloakToFire, "OpenTopped_DecloakToFire");
		debugProcess(this->OpenTopped_WarpDistance, "OpenTopped_WarpDistance");
		debugProcess(this->OpenTopped_IgnoreRangefinding, "OpenTopped_IgnoreRangefinding");
		debugProcess(this->OpenTopped_AllowFiringIfDeactivated, "OpenTopped_AllowFiringIfDeactivated");
		debugProcess(this->OpenTopped_ShareTransportTarget, "OpenTopped_ShareTransportTarget");
		debugProcess(this->OpenTopped_UseTransportRangeModifiers, "OpenTopped_UseTransportRangeModifiers");
		debugProcess(this->OpenTopped_CheckTransportDisableWeapons, "OpenTopped_CheckTransportDisableWeapons");
		debugProcess(this->AutoFire, "AutoFire");
		debugProcess(this->AutoFire_TargetSelf, "AutoFire_TargetSelf");
		debugProcess(this->NoSecondaryWeaponFallback, "NoSecondaryWeaponFallback");
		debugProcess(this->NoSecondaryWeaponFallback_AllowAA, "NoSecondaryWeaponFallback_AllowAA");
		debugProcess(this->AllowWeaponSelectAgainstWalls, "AllowWeaponSelectAgainstWalls");
		debugProcess(this->NoAmmoWeapon, "NoAmmoWeapon");
		debugProcess(this->NoAmmoAmount, "NoAmmoAmount");
		debugProcess(this->JumpjetAllowLayerDeviation, "JumpjetAllowLayerDeviation");
		debugProcess(this->JumpjetTurnToTarget, "JumpjetTurnToTarget");
		debugProcess(this->JumpjetCrash_Rotate, "JumpjetCrash_Rotate");
		debugProcess(this->DeployingAnims, "DeployingAnims");
		debugProcess(this->DeployingAnim_KeepUnitVisible, "DeployingAnim_KeepUnitVisible");
		debugProcess(this->DeployingAnim_ReverseForUndeploy, "DeployingAnim_ReverseForUndeploy");
		debugProcess(this->DeployingAnim_UseUnitDrawer, "DeployingAnim_UseUnitDrawer");
		debugProcess(this->SelfHealGainType, "SelfHealGainType");
		debugProcess(this->EnemyUIName, "EnemyUIName");
		debugProcess(this->ForceWeapon_Naval_Decloaked, "ForceWeapon_Naval_Decloaked");
		debugProcess(this->ForceWeapon_UnderEMP, "ForceWeapon_UnderEMP");
		debugProcess(this->ForceWeapon_Cloaked, "ForceWeapon_Cloaked");
		debugProcess(this->ForceWeapon_Disguised, "ForceWeapon_Disguised");
		debugProcess(this->ImmuneToEMP, "ImmuneToEMP");
		debugProcess(this->Ammo_Shared, "Ammo_Shared");
		debugProcess(this->Ammo_Shared_Group, "Ammo_Shared_Group");
		debugProcess(this->Passengers_SyncOwner, "Passengers_SyncOwner");
		debugProcess(this->Passengers_SyncOwner_RevertOnExit, "Passengers_SyncOwner_RevertOnExit");
		debugProcess(this->Aircraft_DecreaseAmmo, "Aircraft_DecreaseAmmo");
		debugProcess(this->UseDisguiseMovementSpeed, "UseDisguiseMovementSpeed");
		debugProcess(this->Insignia, "Insignia");
		debugProcess(this->InsigniaFrames, "InsigniaFrames");
		debugProcess(this->InsigniaFrame, "InsigniaFrame");
		debugProcess(this->Insignia_ShowEnemy, "Insignia_ShowEnemy");
		debugProcess(this->Insignia_Weapon, "Insignia_Weapon");
		debugProcess(this->Insignia_Passengers, "Insignia_Passengers");
		debugProcess(this->InsigniaFrame_Passengers, "InsigniaFrame_Passengers");
		debugProcess(this->InsigniaFrames_Passengers, "InsigniaFrames_Passengers");
		debugProcess(this->InitialStrength_Cloning, "InitialStrength_Cloning");
		debugProcess(this->SelectBox, "SelectBox");
		debugProcess(this->HideSelectBox, "HideSelectBox");
		debugProcess(this->Explodes_KillPassengers, "Explodes_KillPassengers");
		debugProcess(this->DeployFireWeapon, "DeployFireWeapon");
		debugProcess(this->RevengeWeapon, "RevengeWeapon");
		debugProcess(this->RevengeWeapon_AffectsHouses, "RevengeWeapon_AffectsHouses");
		debugProcess(this->TargetZoneScanType, "TargetZoneScanType");
		debugProcess(this->GrapplingAttack, "GrapplingAttack");
		debugProcess(this->PronePrimaryFireFLH, "PronePrimaryFireFLH");
		debugProcess(this->ProneSecondaryFireFLH, "ProneSecondaryFireFLH");
		debugProcess(this->DeployedPrimaryFireFLH, "DeployedPrimaryFireFLH");
		debugProcess(this->DeployedSecondaryFireFLH, "DeployedSecondaryFireFLH");
		debugProcess(this->E_PronePrimaryFireFLH, "E_PronePrimaryFireFLH");
		debugProcess(this->E_ProneSecondaryFireFLH, "E_ProneSecondaryFireFLH");
		debugProcess(this->E_DeployedPrimaryFireFLH, "E_DeployedPrimaryFireFLH");
		debugProcess(this->E_DeployedSecondaryFireFLH, "E_DeployedSecondaryFireFLH");
		debugProcess(this->WeaponBurstFLHs, "WeaponBurstFLHs");
		debugProcess(this->CrouchedWeaponBurstFLHs, "CrouchedWeaponBurstFLHs");
		debugProcess(this->DeployedWeaponBurstFLHs, "DeployedWeaponBurstFLHs");
		debugProcess(this->IronCurtain_KeptOnDeploy, "IronCurtain_KeptOnDeploy");
		debugProcess(this->ForceShield_KeptOnDeploy, "ForceShield_KeptOnDeploy");
		debugProcess(this->IronCurtain_Effect, "IronCurtain_Effect");
		debugProcess(this->IronCurtain_KillWarhead, "IronCurtain_KillWarhead");
		debugProcess(this->ForceShield_Effect, "ForceShield_Effect");
		debugProcess(this->ForceShield_KillWarhead, "ForceShield_KillWarhead");
		debugProcess(this->SellSound, "SellSound");
		debugProcess(this->EVA_Sold, "EVA_Sold");
		debugProcess(this->AlternateFLHs, "AlternateFLHs");
		debugProcess(this->Spawner_SpawnOffsets, "Spawner_SpawnOffsets");
		debugProcess(this->Spawner_SpawnOffsets_OverrideWeaponFLH, "Spawner_SpawnOffsets_OverrideWeaponFLH");
		debugProcess(this->FacingRotation_Disable, "FacingRotation_Disable");
		debugProcess(this->FacingRotation_DisalbeOnEMP, "FacingRotation_DisalbeOnEMP");
		debugProcess(this->FacingRotation_DisalbeOnDeactivated, "FacingRotation_DisalbeOnDeactivated");
		debugProcess(this->FacingRotation_DisableOnDriverKilled, "FacingRotation_DisableOnDriverKilled");
		debugProcess(this->DontShake, "DontShake");
		debugProcess(this->DiskLaserChargeUp, "DiskLaserChargeUp");
		debugProcess(this->DiskLaserDetonate, "DiskLaserDetonate");
		debugProcess(this->DrainAnimationType, "DrainAnimationType");
		debugProcess(this->DrainMoneyFrameDelay, "DrainMoneyFrameDelay");
		debugProcess(this->DrainMoneyAmount, "DrainMoneyAmount");
		debugProcess(this->DrainMoney_Display, "DrainMoney_Display");
		debugProcess(this->DrainMoney_Display_Houses, "DrainMoney_Display_Houses");
		debugProcess(this->DrainMoney_Display_OnTarget, "DrainMoney_Display_OnTarget");
		debugProcess(this->DrainMoney_Display_OnTarget_UseDisplayIncome, "DrainMoney_Display_OnTarget_UseDisplayIncome");
		debugProcess(this->DrainMoney_Display_Offset, "DrainMoney_Display_Offset");
		debugProcess(this->TalkBubbleTime, "TalkBubbleTime");
		debugProcess(this->Draw_MindControlLink, "Draw_MindControlLink");
		debugProcess(this->Overload_Count, "Overload_Count");
		debugProcess(this->Overload_Damage, "Overload_Damage");
		debugProcess(this->Overload_Frames, "Overload_Frames");
		debugProcess(this->Overload_DeathSound, "Overload_DeathSound");
		debugProcess(this->Overload_ParticleSys, "Overload_ParticleSys");
		debugProcess(this->Overload_ParticleSysCount, "Overload_ParticleSysCount");
		debugProcess(this->Overload_Warhead, "Overload_Warhead");
		debugProcess(this->HitCoordOffset, "HitCoordOffset");
		debugProcess(this->HitCoordOffset_Random, "HitCoordOffset_Random");
		debugProcess(this->DeathWeapon, "DeathWeapon");
		debugProcess(this->CrashWeapon, "CrashWeapon");
		debugProcess(this->CrashWeapon_s, "CrashWeapon_s");
		debugProcess(this->DeathWeapon_CheckAmmo, "DeathWeapon_CheckAmmo");
		debugProcess(this->Disable_C4WarheadExp, "Disable_C4WarheadExp");
		debugProcess(this->CrashSpinLevelRate, "CrashSpinLevelRate");
		debugProcess(this->CrashSpinVerticalRate, "CrashSpinVerticalRate");
		debugProcess(this->ParasiteExit_Sound, "ParasiteExit_Sound");
		debugProcess(this->PipShapes01, "PipShapes01");
		debugProcess(this->PipShapes02, "PipShapes02");
		debugProcess(this->PipGarrison, "PipGarrison");
		debugProcess(this->PipGarrison_FrameIndex, "PipGarrison_FrameIndex");
		debugProcess(this->PipGarrison_Palette, "PipGarrison_Palette");
		debugProcess(this->HealthNumber_Show, "HealthNumber_Show");
		debugProcess(this->HealthNumber_Percent, "HealthNumber_Percent");
		debugProcess(this->Healnumber_Offset, "Healnumber_Offset");
		debugProcess(this->HealthNumber_SHP, "HealthNumber_SHP");
		debugProcess(this->Healnumber_Decrement, "Healnumber_Decrement");
		debugProcess(this->HealthBarSHP, "HealthBarSHP");
		debugProcess(this->HealthBarSHP_Selected, "HealthBarSHP_Selected");
		debugProcess(this->HealthBarSHPBracketOffset, "HealthBarSHPBracketOffset");
		debugProcess(this->HealthBarSHP_HealthFrame, "HealthBarSHP_HealthFrame");
		debugProcess(this->HealthBarSHP_Palette, "HealthBarSHP_Palette");
		debugProcess(this->HealthBarSHP_PointOffset, "HealthBarSHP_PointOffset");
		debugProcess(this->HealthbarRemap, "HealthbarRemap");
		debugProcess(this->GClock_Shape, "GClock_Shape");
		debugProcess(this->GClock_Transculency, "GClock_Transculency");
		debugProcess(this->GClock_Palette, "GClock_Palette");
		debugProcess(this->ROF_Random, "ROF_Random");
		debugProcess(this->Rof_RandomMinMax, "Rof_RandomMinMax");
		debugProcess(this->Eva_Complete, "Eva_Complete");
		debugProcess(this->VoiceCreate, "VoiceCreate");
		debugProcess(this->VoiceCreate_Instant, "VoiceCreate_Instant");
		debugProcess(this->CreateSound_Enable, "CreateSound_Enable");
		debugProcess(this->SlaveFreeSound_Enable, "SlaveFreeSound_Enable");
		debugProcess(this->SlaveFreeSound, "SlaveFreeSound");
		debugProcess(this->NoAirportBound_DisableRadioContact, "NoAirportBound_DisableRadioContact");
		debugProcess(this->SinkAnim, "SinkAnim");
		debugProcess(this->Tunnel_Speed, "Tunnel_Speed");
		debugProcess(this->HoverType, "HoverType");
		debugProcess(this->Gattling_Overload, "Gattling_Overload");
		debugProcess(this->Gattling_Overload_Damage, "Gattling_Overload_Damage");
		debugProcess(this->Gattling_Overload_Frames, "Gattling_Overload_Frames");
		debugProcess(this->Gattling_Overload_DeathSound, "Gattling_Overload_DeathSound");
		debugProcess(this->Gattling_Overload_ParticleSys, "Gattling_Overload_ParticleSys");
		debugProcess(this->Gattling_Overload_ParticleSysCount, "Gattling_Overload_ParticleSysCount");
		debugProcess(this->Gattling_Overload_Warhead, "Gattling_Overload_Warhead");
		debugProcess(this->IsDummy, "IsDummy");
		debugProcess(this->FireSelf_Weapon, "FireSelf_Weapon");
		debugProcess(this->FireSelf_ROF, "FireSelf_ROF");
		debugProcess(this->FireSelf_Weapon_GreenHeath, "FireSelf_Weapon_GreenHeath");
		debugProcess(this->FireSelf_ROF_GreenHeath, "FireSelf_ROF_GreenHeath");
		debugProcess(this->FireSelf_Weapon_YellowHeath, "FireSelf_Weapon_YellowHeath");
		debugProcess(this->FireSelf_ROF_YellowHeath, "FireSelf_ROF_YellowHeath");
		debugProcess(this->FireSelf_Weapon_RedHeath, "FireSelf_Weapon_RedHeath");
		debugProcess(this->FireSelf_ROF_RedHeath, "FireSelf_ROF_RedHeath");
		debugProcess(this->AllowFire_IroncurtainedTarget, "AllowFire_IroncurtainedTarget");
		debugProcess(this->EngineerCaptureDelay, "EngineerCaptureDelay");
		debugProcess(this->CommandLine_Move_Color, "CommandLine_Move_Color");
		debugProcess(this->CommandLine_Attack_Color, "CommandLine_Attack_Color");
		debugProcess(this->PassiveAcquire_AI, "PassiveAcquire_AI");
		debugProcess(this->CanPassiveAquire_Naval, "CanPassiveAquire_Naval");
		debugProcess(this->TankDisguiseAsTank, "TankDisguiseAsTank");
		debugProcess(this->DisguiseDisAllowed, "DisguiseDisAllowed");
		debugProcess(this->ChronoDelay_Immune, "ChronoDelay_Immune");
		debugProcess(this->PoseDir, "PoseDir");
		debugProcess(this->Firing_IgnoreGravity, "Firing_IgnoreGravity");
		debugProcess(this->Survivors_PassengerChance, "Survivors_PassengerChance");
		debugProcess(this->Prerequisite_RequiredTheaters, "Prerequisite_RequiredTheaters");
		debugProcess(this->Prerequisites, "Prerequisites");
		debugProcess(this->Prerequisite_Lists, "Prerequisite_Lists");
		debugProcess(this->Prerequisite_Negative, "Prerequisite_Negative");
		debugProcess(this->Prerequisite_Display, "Prerequisite_Display");
		debugProcess(this->BuildLimit_Requires, "BuildLimit_Requires");
		debugProcess(this->ConsideredNaval, "ConsideredNaval");
		debugProcess(this->ConsideredVehicle, "ConsideredVehicle");
		debugProcess(this->PrimaryCrawlFLH, "PrimaryCrawlFLH");
		debugProcess(this->Elite_PrimaryCrawlFLH, "Elite_PrimaryCrawlFLH");
		debugProcess(this->SecondaryCrawlFLH, "SecondaryCrawlFLH");
		debugProcess(this->Elite_SecondaryCrawlFLH, "Elite_SecondaryCrawlFLH");
		debugProcess(this->BountyAllow, "BountyAllow");
		debugProcess(this->BountyDissallow, "BountyDissallow");
		debugProcess(this->BountyBonusmult, "BountyBonusmult");
		debugProcess(this->Bounty_IgnoreEnablers, "Bounty_IgnoreEnablers");
		debugProcess(this->Tiberium_EmptyPipIdx, "Tiberium_EmptyPipIdx");
		debugProcess(this->Tiberium_PipIdx, "Tiberium_PipIdx");
		debugProcess(this->Tiberium_PipShapes, "Tiberium_PipShapes");
		debugProcess(this->Tiberium_PipShapes_Palette, "Tiberium_PipShapes_Palette");
		debugProcess(this->CrushLevel, "CrushLevel");
		debugProcess(this->CrushableLevel, "CrushableLevel");
		debugProcess(this->DeployCrushableLevel, "DeployCrushableLevel");
		debugProcess(this->Experience_KillerMultiple, "Experience_KillerMultiple");
		debugProcess(this->Experience_VictimMultiple, "Experience_VictimMultiple");
		debugProcess(this->NavalRangeBonus, "NavalRangeBonus");
		debugProcess(this->AI_LegalTarget, "AI_LegalTarget");
		debugProcess(this->DeployFire_UpdateFacing, "DeployFire_UpdateFacing");
		debugProcess(this->Fake_Of, "Fake_Of");
		debugProcess(this->CivilianEnemy, "CivilianEnemy");
		debugProcess(this->ImmuneToBerserk, "ImmuneToBerserk");
		debugProcess(this->Berzerk_Modifier, "Berzerk_Modifier");
		debugProcess(this->TargetLaser_Time, "TargetLaser_Time");
		debugProcess(this->TargetLaser_WeaponIdx, "TargetLaser_WeaponIdx");
		debugProcess(this->CurleyShuffle, "CurleyShuffle");
		debugProcess(this->PassengersGainExperience, "PassengersGainExperience");
		debugProcess(this->ExperienceFromPassengers, "ExperienceFromPassengers");
		debugProcess(this->PassengerExperienceModifier, "PassengerExperienceModifier");
		debugProcess(this->MindControlExperienceSelfModifier, "MindControlExperienceSelfModifier");
		debugProcess(this->MindControlExperienceVictimModifier, "MindControlExperienceVictimModifier");
		debugProcess(this->SpawnExperienceOwnerModifier, "SpawnExperienceOwnerModifier");
		debugProcess(this->SpawnExperienceSpawnModifier, "SpawnExperienceSpawnModifier");
		debugProcess(this->ExperienceFromAirstrike, "ExperienceFromAirstrike");
		debugProcess(this->AirstrikeExperienceModifier, "AirstrikeExperienceModifier");
		debugProcess(this->Promote_IncludePassengers, "Promote_IncludePassengers");
		debugProcess(this->Promote_Elite_Eva, "Promote_Elite_Eva");
		debugProcess(this->Promote_Vet_Eva, "Promote_Vet_Eva");
		debugProcess(this->Promote_Elite_Sound, "Promote_Elite_Sound");
		debugProcess(this->Promote_Vet_Sound, "Promote_Vet_Sound");
		debugProcess(this->Promote_Elite_Flash, "Promote_Elite_Flash");
		debugProcess(this->Promote_Vet_Flash, "Promote_Vet_Flash");
		debugProcess(this->Promote_Vet_Type, "Promote_Vet_Type");
		debugProcess(this->Promote_Elite_Type, "Promote_Elite_Type");
		debugProcess(this->Promote_Vet_Anim, "Promote_Vet_Anim");
		debugProcess(this->Promote_Elite_Anim, "Promote_Elite_Anim");
		debugProcess(this->Promote_Vet_PlaySpotlight, "Promote_Vet_PlaySpotlight");
		debugProcess(this->Promote_Elite_PlaySpotlight, "Promote_Elite_PlaySpotlight");
		debugProcess(this->Promote_Vet_Exp, "Promote_Vet_Exp");
		debugProcess(this->Promote_Elite_Exp, "Promote_Elite_Exp");
		debugProcess(this->DeployDir, "DeployDir");
		debugProcess(this->PassengersWhitelist, "PassengersWhitelist");
		debugProcess(this->PassengersBlacklist, "PassengersBlacklist");
		debugProcess(this->NoManualUnload, "NoManualUnload");
		debugProcess(this->NoSelfGuardArea, "NoSelfGuardArea");
		debugProcess(this->NoManualFire, "NoManualFire");
		debugProcess(this->NoManualEnter, "NoManualEnter");
		debugProcess(this->NoManualEject, "NoManualEject");
		debugProcess(this->Passengers_BySize, "Passengers_BySize");
		debugProcess(this->Convert_Deploy, "Convert_Deploy");
		debugProcess(this->Convert_Deploy_Delay, "Convert_Deploy_Delay");
		debugProcess(this->Convert_Script, "Convert_Script");
		debugProcess(this->Convert_Water, "Convert_Water");
		debugProcess(this->Convert_Land, "Convert_Land");
		debugProcess(this->Harvester_LongScan, "Harvester_LongScan");
		debugProcess(this->Harvester_ShortScan, "Harvester_ShortScan");
		debugProcess(this->Harvester_ScanCorrection, "Harvester_ScanCorrection");
		debugProcess(this->Harvester_TooFarDistance, "Harvester_TooFarDistance");
		debugProcess(this->Harvester_KickDelay, "Harvester_KickDelay");
		debugProcess(this->TurretRot, "TurretRot");
		debugProcess(this->WaterImage, "WaterImage");
		debugProcess(this->WaterImage_Yellow, "WaterImage_Yellow");
		debugProcess(this->WaterImage_Red, "WaterImage_Red");
		debugProcess(this->Image_Yellow, "Image_Yellow");
		debugProcess(this->Image_Red, "Image_Red");
		debugProcess(this->FallRate_Parachute, "FallRate_Parachute");
		debugProcess(this->FallRate_NoParachute, "FallRate_NoParachute");
		debugProcess(this->FallRate_ParachuteMax, "FallRate_ParachuteMax");
		debugProcess(this->FallRate_NoParachuteMax, "FallRate_NoParachuteMax");
		debugProcess(this->WeaponUINameX, "WeaponUINameX");
		debugProcess(this->NoShadowSpawnAlt, "NoShadowSpawnAlt");
		debugProcess(this->AdditionalWeaponDatas, "AdditionalWeaponDatas");
		debugProcess(this->AdditionalEliteWeaponDatas, "AdditionalEliteWeaponDatas");
		debugProcess(this->AdditionalTurrentWeapon, "AdditionalTurrentWeapon");
		debugProcess(this->OmniCrusher_Aggressive, "OmniCrusher_Aggressive");
		debugProcess(this->CrusherDecloak, "CrusherDecloak");
		debugProcess(this->Crusher_SupressLostEva, "Crusher_SupressLostEva");
		debugProcess(this->CrushFireDeathWeapon, "CrushFireDeathWeapon");
		debugProcess(this->CrushDamage, "CrushDamage");
		debugProcess(this->CrushDamageWarhead, "CrushDamageWarhead");
		debugProcess(this->CrushDamagePlayWHAnim, "CrushDamagePlayWHAnim");
		debugProcess(this->CrushRange, "CrushRange");
		debugProcess(this->DigInSound, "DigInSound");
		debugProcess(this->DigOutSound, "DigOutSound");
		debugProcess(this->DigInAnim, "DigInAnim");
		debugProcess(this->DigOutAnim, "DigOutAnim");
		debugProcess(this->EVA_UnitLost, "EVA_UnitLost");
		debugProcess(this->BuildTime_Speed, "BuildTime_Speed");
		debugProcess(this->BuildTime_Cost, "BuildTime_Cost");
		debugProcess(this->BuildTime_LowPowerPenalty, "BuildTime_LowPowerPenalty");
		debugProcess(this->BuildTime_MinLowPower, "BuildTime_MinLowPower");
		debugProcess(this->BuildTime_MaxLowPower, "BuildTime_MaxLowPower");
		debugProcess(this->BuildTime_MultipleFactory, "BuildTime_MultipleFactory");
		debugProcess(this->CloakStages, "CloakStages");
		debugProcess(this->DamageSparks, "DamageSparks");
		debugProcess(this->ParticleSystems_DamageSmoke, "ParticleSystems_DamageSmoke");
		debugProcess(this->ParticleSystems_DamageSparks, "ParticleSystems_DamageSparks");
		debugProcess(this->GattlingCyclic, "GattlingCyclic");
		debugProcess(this->CloakSound, "CloakSound");
		debugProcess(this->DecloakSound, "DecloakSound");
		debugProcess(this->VoiceRepair, "VoiceRepair");
		debugProcess(this->ReloadAmount, "ReloadAmount");
		debugProcess(this->EmptyReloadAmount, "EmptyReloadAmount");
		debugProcess(this->TiberiumProof, "TiberiumProof");
		debugProcess(this->TiberiumSpill, "TiberiumSpill");
		debugProcess(this->TiberiumRemains, "TiberiumRemains");
		debugProcess(this->TiberiumTransmogrify, "TiberiumTransmogrify");
		debugProcess(this->SensorArray_Warn, "SensorArray_Warn");
		debugProcess(this->IronCurtain_Modifier, "IronCurtain_Modifier");
		debugProcess(this->ForceShield_Modifier, "ForceShield_Modifier");
		debugProcess(this->Survivors_PilotCount, "Survivors_PilotCount");
		debugProcess(this->Survivors_Pilots, "Survivors_Pilots");
		debugProcess(this->Ammo_AddOnDeploy, "Ammo_AddOnDeploy");
		debugProcess(this->Ammo_AutoDeployMinimumAmount, "Ammo_AutoDeployMinimumAmount");
		debugProcess(this->Ammo_AutoDeployMaximumAmount, "Ammo_AutoDeployMaximumAmount");
		debugProcess(this->Ammo_DeployUnlockMinimumAmount, "Ammo_DeployUnlockMinimumAmount");
		debugProcess(this->Ammo_DeployUnlockMaximumAmount, "Ammo_DeployUnlockMaximumAmount");
		debugProcess(this->ImmuneToWeb, "ImmuneToWeb");
		debugProcess(this->Webby_Anims, "Webby_Anims");
		debugProcess(this->Webby_Modifier, "Webby_Modifier");
		debugProcess(this->Webby_Duration_Variation, "Webby_Duration_Variation");
		debugProcess(this->BerserkROFMultiplier, "BerserkROFMultiplier");
		debugProcess(this->Refinery_UseStorage, "Refinery_UseStorage");
		debugProcess(this->VHPscan_Value, "VHPscan_Value");
		debugProcess(this->SelfHealing_Rate, "SelfHealing_Rate");
		debugProcess(this->SelfHealing_Amount, "SelfHealing_Amount");
		debugProcess(this->SelfHealing_Max, "SelfHealing_Max");
		debugProcess(this->SelfHealing_CombatDelay, "SelfHealing_CombatDelay");
		debugProcess(this->Bounty, "Bounty");
		debugProcess(this->HasSpotlight, "HasSpotlight");
		debugProcess(this->Spot_Height, "Spot_Height");
		debugProcess(this->Spot_Distance, "Spot_Distance");
		debugProcess(this->Spot_AttachedTo, "Spot_AttachedTo");
		debugProcess(this->Spot_DisableR, "Spot_DisableR");
		debugProcess(this->Spot_DisableG, "Spot_DisableG");
		debugProcess(this->Spot_DisableB, "Spot_DisableB");
		debugProcess(this->Spot_DisableColor, "Spot_DisableColor");
		debugProcess(this->Spot_Reverse, "Spot_Reverse");
		debugProcess(this->CloakAllowed, "CloakAllowed");
		debugProcess(this->InitialPayload_Types, "InitialPayload_Types");
		debugProcess(this->InitialPayload_Nums, "InitialPayload_Nums");
		debugProcess(this->InitialPayload_Vet, "InitialPayload_Vet");
		debugProcess(this->InitialPayload_AddToTransportTeam, "InitialPayload_AddToTransportTeam");
		debugProcess(this->AlternateTheaterArt, "AlternateTheaterArt");
		debugProcess(this->HijackerOneTime, "HijackerOneTime");
		debugProcess(this->HijackerKillPilots, "HijackerKillPilots");
		debugProcess(this->HijackerEnterSound, "HijackerEnterSound");
		debugProcess(this->HijackerLeaveSound, "HijackerLeaveSound");
		debugProcess(this->HijackerBreakMindControl, "HijackerBreakMindControl");
		debugProcess(this->HijackerAllowed, "HijackerAllowed");
		debugProcess(this->Survivors_PilotChance, "Survivors_PilotChance");
		debugProcess(this->Crew_TechnicianChance, "Crew_TechnicianChance");
		debugProcess(this->Crew_EngineerChance, "Crew_EngineerChance");
		debugProcess(this->Saboteur, "Saboteur");
		debugProcess(this->Cursor_Deploy, "Cursor_Deploy");
		debugProcess(this->Cursor_NoDeploy, "Cursor_NoDeploy");
		debugProcess(this->Cursor_Enter, "Cursor_Enter");
		debugProcess(this->Cursor_NoEnter, "Cursor_NoEnter");
		debugProcess(this->Cursor_Move, "Cursor_Move");
		debugProcess(this->Cursor_NoMove, "Cursor_NoMove");
		debugProcess(this->ImmuneToAbduction, "ImmuneToAbduction");
		debugProcess(this->UseROFAsBurstDelays, "UseROFAsBurstDelays");
		debugProcess(this->Chronoshift_Crushable, "Chronoshift_Crushable");
		debugProcess(this->CanBeReversed, "CanBeReversed");
		debugProcess(this->ReversedAs, "ReversedAs");
		debugProcess(this->AssaulterLevel, "AssaulterLevel");
		debugProcess(this->RadialIndicatorRadius, "RadialIndicatorRadius");
		debugProcess(this->RadialIndicatorColor, "RadialIndicatorColor");
		debugProcess(this->GapRadiusInCells, "GapRadiusInCells");
		debugProcess(this->SuperGapRadiusInCells, "SuperGapRadiusInCells");
		debugProcess(this->SmokeChanceRed, "SmokeChanceRed");
		debugProcess(this->SmokeChanceDead, "SmokeChanceDead");
		debugProcess(this->SmokeAnim, "SmokeAnim");
		debugProcess(this->CarryallAllowed, "CarryallAllowed");
		debugProcess(this->CarryallSizeLimit, "CarryallSizeLimit");
		debugProcess(this->VoiceAirstrikeAttack, "VoiceAirstrikeAttack");
		debugProcess(this->VoiceAirstrikeAbort, "VoiceAirstrikeAbort");
		debugProcess(this->HunterSeekerDetonateProximity, "HunterSeekerDetonateProximity");
		debugProcess(this->HunterSeekerDescendProximity, "HunterSeekerDescendProximity");
		debugProcess(this->HunterSeekerAscentSpeed, "HunterSeekerAscentSpeed");
		debugProcess(this->HunterSeekerDescentSpeed, "HunterSeekerDescentSpeed");
		debugProcess(this->HunterSeekerEmergeSpeed, "HunterSeekerEmergeSpeed");
		debugProcess(this->HunterSeekerIgnore, "HunterSeekerIgnore");
		debugProcess(this->Bounty_Display, "Bounty_Display");
		debugProcess(this->Bounty_Value, "Bounty_Value");
		debugProcess(this->Bounty_Value_PercentOf, "Bounty_Value_PercentOf");
		debugProcess(this->Bounty_ReceiveSound, "Bounty_ReceiveSound");
		debugProcess(this->Bounty_Value_Option, "Bounty_Value_Option");
		debugProcess(this->Bounty_Value_mult, "Bounty_Value_mult");
		debugProcess(this->CanPassiveAcquire_Guard, "CanPassiveAcquire_Guard");
		debugProcess(this->CanPassiveAcquire_Cloak, "CanPassiveAcquire_Cloak");
		debugProcess(this->CrashSpin, "CrashSpin");
		debugProcess(this->AirRate, "AirRate");
		debugProcess(this->Unsellable, "Unsellable");
		debugProcess(this->CreateSound_afect, "CreateSound_afect");
		debugProcess(this->Chronoshift_Allow, "Chronoshift_Allow");
		debugProcess(this->Chronoshift_IsVehicle, "Chronoshift_IsVehicle");
		debugProcess(this->SuppressorRange, "SuppressorRange");
		debugProcess(this->AttractorRange, "AttractorRange");
		debugProcess(this->FactoryPlant_Multiplier, "FactoryPlant_Multiplier");
		debugProcess(this->MassSelectable, "MassSelectable");
		debugProcess(this->TiltsWhenCrushes_Vehicles, "TiltsWhenCrushes_Vehicles");
		debugProcess(this->TiltsWhenCrushes_Overlays, "TiltsWhenCrushes_Overlays");
		debugProcess(this->CrushForwardTiltPerFrame, "CrushForwardTiltPerFrame");
		debugProcess(this->CrushOverlayExtraForwardTilt, "CrushOverlayExtraForwardTilt");
		debugProcess(this->CrushSlowdownMultiplier, "CrushSlowdownMultiplier");
		debugProcess(this->ShadowScale, "ShadowScale");
		debugProcess(this->AIIonCannonValue, "AIIonCannonValue");
		debugProcess(this->GenericPrerequisite, "GenericPrerequisite");
		debugProcess(this->ExtraPower_Amount, "ExtraPower_Amount");
		debugProcess(this->RecheckTechTreeWhenDie, "RecheckTechTreeWhenDie");
		debugProcess(this->Linked_SW, "Linked_SW");
		debugProcess(this->CanDrive, "CanDrive");
		debugProcess(this->Operators, "Operators");
		debugProcess(this->Operator_Any, "Operator_Any");
		debugProcess(this->AlwayDrawRadialIndicator, "AlwayDrawRadialIndicator");
		debugProcess(this->ReloadRate, "ReloadRate");
		debugProcess(this->CloakAnim, "CloakAnim");
		debugProcess(this->DecloakAnim, "DecloakAnim");
		debugProcess(this->Cloak_KickOutParasite, "Cloak_KickOutParasite");
		debugProcess(this->DeployAnims, "DeployAnims");
		debugProcess(this->SpecificExpFactor, "SpecificExpFactor");
		debugProcess(this->Initial_DriverKilled, "Initial_DriverKilled");
		debugProcess(this->VoiceCantDeploy, "VoiceCantDeploy");
		debugProcess(this->DigitalDisplay_Disable, "DigitalDisplay_Disable");
		debugProcess(this->DigitalDisplayTypes, "DigitalDisplayTypes");
		debugProcess(this->EmptyAmmoPip, "EmptyAmmoPip");
		debugProcess(this->PipWrapAmmoPip, "PipWrapAmmoPip");
		debugProcess(this->AmmoPipSize, "AmmoPipSize");
		debugProcess(this->ProduceCashDisplay, "ProduceCashDisplay");
		debugProcess(this->FactoryOwners, "FactoryOwners");
		debugProcess(this->FactoryOwners_Forbidden, "FactoryOwners_Forbidden");
		debugProcess(this->Wake, "Wake");
		debugProcess(this->Spawner_AttackImmediately, "Spawner_AttackImmediately");
		debugProcess(this->Spawner_UseTurretFacing, "Spawner_UseTurretFacing");
		debugProcess(this->FactoryOwners_HaveAllPlans, "FactoryOwners_HaveAllPlans");
		debugProcess(this->FactoryOwners_HasAllPlans, "FactoryOwners_HasAllPlans");
		debugProcess(this->Drain_Local, "Drain_Local");
		debugProcess(this->Drain_Amount, "Drain_Amount");
		debugProcess(this->HealthBar_Sections, "HealthBar_Sections");
		debugProcess(this->HealthBar_Border, "HealthBar_Border");
		debugProcess(this->HealthBar_BorderFrame, "HealthBar_BorderFrame");
		debugProcess(this->HealthBar_BorderAdjust, "HealthBar_BorderAdjust");
		debugProcess(this->Crashable, "Crashable");
		debugProcess(this->IsBomb, "IsBomb");
		debugProcess(this->ParachuteAnim, "ParachuteAnim");
		debugProcess(this->Cloneable, "Cloneable");
		debugProcess(this->ClonedAt, "ClonedAt");
		debugProcess(this->ClonedAs, "ClonedAs");
		debugProcess(this->AI_ClonedAs, "AI_ClonedAs");
		debugProcess(this->BuiltAt, "BuiltAt");
		debugProcess(this->EMP_Sparkles, "EMP_Sparkles");
		debugProcess(this->EMP_Modifier, "EMP_Modifier");
		debugProcess(this->EMP_Threshold, "EMP_Threshold");
		debugProcess(this->PoweredBy, "PoweredBy");
		debugProcess(this->CameoPCX, "CameoPCX");
		debugProcess(this->AltCameoPCX, "AltCameoPCX");
		debugProcess(this->CameoPal, "CameoPal");
		debugProcess(this->LandingDir, "LandingDir");
		debugProcess(this->AttachedEffect, "AttachedEffect");
		debugProcess(this->NoAmmoEffectAnim, "NoAmmoEffectAnim");
		debugProcess(this->AttackFriendlies_WeaponIdx, "AttackFriendlies_WeaponIdx");
		debugProcess(this->AttackFriendlies_AutoAttack, "AttackFriendlies_AutoAttack");
		debugProcess(this->PipScaleIndex, "PipScaleIndex");
		debugProcess(this->AmmoPip, "AmmoPip");
		debugProcess(this->AmmoPip_Palette, "AmmoPip_Palette");
		debugProcess(this->AmmoPipOffset, "AmmoPipOffset");
		debugProcess(this->AmmoPip_Offset, "AmmoPip_Offset");
		debugProcess(this->AmmoPip_shape, "AmmoPip_shape");
		debugProcess(this->ShowSpawnsPips, "ShowSpawnsPips");
		debugProcess(this->SpawnsPip, "SpawnsPip");
		debugProcess(this->EmptySpawnsPip, "EmptySpawnsPip");
		debugProcess(this->SpawnsPipSize, "SpawnsPipSize");
		debugProcess(this->SpawnsPipOffset, "SpawnsPipOffset");
		debugProcess(this->Secret_RequiredHouses, "Secret_RequiredHouses");
		debugProcess(this->Secret_ForbiddenHouses, "Secret_ForbiddenHouses");
		debugProcess(this->RequiredStolenTech, "RequiredStolenTech");
		debugProcess(this->ReloadInTransport, "ReloadInTransport");
		debugProcess(this->Weeder_TriggerPreProductionBuildingAnim, "Weeder_TriggerPreProductionBuildingAnim");
		debugProcess(this->Weeder_PipIndex, "Weeder_PipIndex");
		debugProcess(this->Weeder_PipEmptyIndex, "Weeder_PipEmptyIndex");
		debugProcess(this->CanBeDriven, "CanBeDriven");
		debugProcess(this->CloakPowered, "CloakPowered");
		debugProcess(this->CloakDeployed, "CloakDeployed");
		debugProcess(this->ProtectedDriver, "ProtectedDriver");
		debugProcess(this->ProtectedDriver_MinHealth, "ProtectedDriver_MinHealth");
		debugProcess(this->KeepAlive, "KeepAlive");
		debugProcess(this->SpawnDistanceFromTarget, "SpawnDistanceFromTarget");
		debugProcess(this->SpawnHeight, "SpawnHeight");
		debugProcess(this->HumanUnbuildable, "HumanUnbuildable");
		debugProcess(this->NoIdleSound, "NoIdleSound");
		debugProcess(this->Soylent_Zero, "Soylent_Zero");
		debugProcess(this->Prerequisite_Power, "Prerequisite_Power");
		debugProcess(this->PassengerTurret, "PassengerTurret");
		debugProcess(this->DetectDisguise_Percent, "DetectDisguise_Percent");
		debugProcess(this->EliteArmor, "EliteArmor");
		debugProcess(this->VeteranArmor, "VeteranArmor");
		debugProcess(this->DeployedArmor, "DeployedArmor");
		debugProcess(this->Cloakable_IgnoreArmTimer, "Cloakable_IgnoreArmTimer");
		debugProcess(this->Untrackable, "Untrackable");
		debugProcess(this->Convert_Script_Prereq, "Convert_Script_Prereq");
		debugProcess(this->LargeVisceroid, "LargeVisceroid");
		debugProcess(this->DropPodProp, "DropPodProp");
		debugProcess(this->VoicePickup, "VoicePickup");
		debugProcess(this->CrateGoodie_RerollChance, "CrateGoodie_RerollChance");
		debugProcess(this->Destroyed_CrateType, "Destroyed_CrateType");
		debugProcess(this->Infantry_DimWhenEMPEd, "Infantry_DimWhenEMPEd");
		debugProcess(this->Infantry_DimWhenDisabled, "Infantry_DimWhenDisabled");
		debugProcess(this->Convert_HumanToComputer, "Convert_HumanToComputer");
		debugProcess(this->Convert_ComputerToHuman, "Convert_ComputerToHuman");
		debugProcess(this->AutoDeath_OnOwnerChange, "AutoDeath_OnOwnerChange");
		debugProcess(this->AutoDeath_OnOwnerChange_HumanToComputer, "AutoDeath_OnOwnerChange_HumanToComputer");
		debugProcess(this->AutoDeath_OnOwnerChange_ComputerToHuman, "AutoDeath_OnOwnerChange_ComputerToHuman");
		debugProcess(this->TalkbubbleVoices, "TalkbubbleVoices");
		debugProcess(this->HarvesterDumpAmount, "HarvesterDumpAmount");
		debugProcess(this->HarvesterScanAfterUnload, "HarvesterScanAfterUnload");
		debugProcess(this->AttackMove_Aggressive, "AttackMove_Aggressive");
		debugProcess(this->AttackMove_UpdateTarget, "AttackMove_UpdateTarget");
		debugProcess(this->NoExtraSelfHealOrRepair, "NoExtraSelfHealOrRepair");
		debugProcess(this->BuildLimitGroup_Types, "BuildLimitGroup_Types");
		debugProcess(this->BuildLimitGroup_Nums, "BuildLimitGroup_Nums");
		debugProcess(this->BuildLimitGroup_Factor, "BuildLimitGroup_Factor");
		debugProcess(this->BuildLimitGroup_ContentIfAnyMatch, "BuildLimitGroup_ContentIfAnyMatch");
		debugProcess(this->BuildLimitGroup_NotBuildableIfQueueMatch, "BuildLimitGroup_NotBuildableIfQueueMatch");
		debugProcess(this->BuildLimitGroup_ExtraLimit_Types, "BuildLimitGroup_ExtraLimit_Types");
		debugProcess(this->BuildLimitGroup_ExtraLimit_Nums, "BuildLimitGroup_ExtraLimit_Nums");
		debugProcess(this->BuildLimitGroup_ExtraLimit_MaxCount, "BuildLimitGroup_ExtraLimit_MaxCount");
		debugProcess(this->BuildLimitGroup_ExtraLimit_MaxNum, "BuildLimitGroup_ExtraLimit_MaxNum");
		debugProcess(this->Tint_Color, "Tint_Color");
		debugProcess(this->Tint_Intensity, "Tint_Intensity");
		debugProcess(this->Tint_VisibleToHouses, "Tint_VisibleToHouses");
		debugProcess(this->PhobosAttachEffects, "PhobosAttachEffects");
		debugProcess(this->KeepTargetOnMove, "KeepTargetOnMove");
		debugProcess(this->KeepTargetOnMove_Weapon, "KeepTargetOnMove_Weapon");
		debugProcess(this->KeepTargetOnMove_ExtraDistance, "KeepTargetOnMove_ExtraDistance");
		debugProcess(this->KeepTargetOnMove_NoMorePursuit, "KeepTargetOnMove_NoMorePursuit");
		debugProcess(this->AllowAirstrike, "AllowAirstrike");
		debugProcess(this->ForbidParallelAIQueues, "ForbidParallelAIQueues");
		debugProcess(this->IgnoreForBaseCenter, "IgnoreForBaseCenter");
		debugProcess(this->EVA_Combat, "EVA_Combat");
		debugProcess(this->CombatAlert, "CombatAlert");
		debugProcess(this->CombatAlert_UseFeedbackVoice, "CombatAlert_UseFeedbackVoice");
		debugProcess(this->CombatAlert_UseAttackVoice, "CombatAlert_UseAttackVoice");
		debugProcess(this->CombatAlert_UseEVA, "CombatAlert_UseEVA");
		debugProcess(this->CombatAlert_NotBuilding, "CombatAlert_NotBuilding");
		debugProcess(this->SubterraneanHeight, "SubterraneanHeight");
		debugProcess(this->Spawner_RecycleRange, "Spawner_RecycleRange");
		debugProcess(this->Spawner_RecycleFLH, "Spawner_RecycleFLH");
		debugProcess(this->Spawner_RecycleOnTurret, "Spawner_RecycleOnTurret");
		debugProcess(this->Spawner_RecycleAnim, "Spawner_RecycleAnim");
		debugProcess(this->HugeBar, "HugeBar");
		debugProcess(this->HugeBar_Priority, "HugeBar_Priority");
		debugProcess(this->SprayOffsets, "SprayOffsets");
		debugProcess(this->AINormalTargetingDelay, "AINormalTargetingDelay");
		debugProcess(this->PlayerNormalTargetingDelay, "PlayerNormalTargetingDelay");
		debugProcess(this->AIGuardAreaTargetingDelay, "AIGuardAreaTargetingDelay");
		debugProcess(this->PlayerGuardAreaTargetingDelay, "PlayerGuardAreaTargetingDelay");
		debugProcess(this->DistributeTargetingFrame, "DistributeTargetingFrame");
		debugProcess(this->AIAttackMoveTargetingDelay, "AIAttackMoveTargetingDelay");
		debugProcess(this->PlayerAttackMoveTargetingDelay, "PlayerAttackMoveTargetingDelay");
		debugProcess(this->CanBeBuiltOn, "CanBeBuiltOn");
		debugProcess(this->UnitBaseNormal, "UnitBaseNormal");
		debugProcess(this->UnitBaseForAllyBuilding, "UnitBaseForAllyBuilding");
		debugProcess(this->ChronoSpherePreDelay, "ChronoSpherePreDelay");
		debugProcess(this->ChronoSphereDelay, "ChronoSphereDelay");
		debugProcess(this->PassengerWeapon, "PassengerWeapon");
		debugProcess(this->RefinerySmokeParticleSystemOne, "RefinerySmokeParticleSystemOne");
		debugProcess(this->RefinerySmokeParticleSystemTwo, "RefinerySmokeParticleSystemTwo");
		debugProcess(this->RefinerySmokeParticleSystemThree, "RefinerySmokeParticleSystemThree");
		debugProcess(this->RefinerySmokeParticleSystemFour, "RefinerySmokeParticleSystemFour");
		debugProcess(this->SubterraneanSpeed, "SubterraneanSpeed");
		debugProcess(this->ForceWeapon_InRange, "ForceWeapon_InRange");
		debugProcess(this->ForceWeapon_InRange_Overrides, "ForceWeapon_InRange_Overrides");
		debugProcess(this->ForceWeapon_InRange_ApplyRangeModifiers, "ForceWeapon_InRange_ApplyRangeModifiers");
		debugProcess(this->ForceAAWeapon_InRange, "ForceAAWeapon_InRange");
		debugProcess(this->ForceAAWeapon_InRange_Overrides, "ForceAAWeapon_InRange_Overrides");
		debugProcess(this->ForceAAWeapon_InRange_ApplyRangeModifiers, "ForceAAWeapon_InRange_ApplyRangeModifiers");
		debugProcess(this->ForceWeapon_InRange_TechnoOnly, "ForceWeapon_InRange_TechnoOnly");
		debugProcess(this->UnitIdleRotateTurret, "UnitIdleRotateTurret");
		debugProcess(this->UnitIdlePointToMouse, "UnitIdlePointToMouse");
		debugProcess(this->FallingDownDamage, "FallingDownDamage");
		debugProcess(this->FallingDownDamage_Water, "FallingDownDamage_Water");
		debugProcess(this->FallingDownDamage_AllowEMP, "FallingDownDamage_AllowEMP");
		debugProcess(this->HoverDrownable, "HoverDrownable");
		debugProcess(this->ExtraThreat_Enabled, "ExtraThreat_Enabled");
		debugProcess(this->ExtraThreat_IsThreat, "ExtraThreat_IsThreat");
		debugProcess(this->AlwaysConsideredThreat, "AlwaysConsideredThreat");
		debugProcess(this->ExtraThreat_InRange, "ExtraThreat_InRange");
		debugProcess(this->ExtraThreatCoefficient_InRangeDistance, "ExtraThreatCoefficient_InRangeDistance");
		debugProcess(this->ExtraThreatCoefficient_Facing, "ExtraThreatCoefficient_Facing");
		debugProcess(this->ExtraThreatCoefficient_DistanceToLastTarget, "ExtraThreatCoefficient_DistanceToLastTarget");
		debugProcess(this->DropCrate, "DropCrate");
		debugProcess(this->WhenCrushed_Warhead, "WhenCrushed_Warhead");
		debugProcess(this->WhenCrushed_Weapon, "WhenCrushed_Weapon");
		debugProcess(this->WhenCrushed_Damage, "WhenCrushed_Damage");
		debugProcess(this->WhenCrushed_Warhead_Full, "WhenCrushed_Warhead_Full");
		debugProcess(this->Convert_ToHouseOrCountry, "Convert_ToHouseOrCountry");
		debugProcess(this->SuppressKillWeapons, "SuppressKillWeapons");
		debugProcess(this->SuppressKillWeapons_Types, "SuppressKillWeapons_Types");
		debugProcess(this->NoQueueUpToEnter, "NoQueueUpToEnter");
		debugProcess(this->NoQueueUpToUnload, "NoQueueUpToUnload");
		debugProcess(this->NoRearm_UnderEMP, "NoRearm_UnderEMP");
		debugProcess(this->NoRearm_Temporal, "NoRearm_Temporal");
		debugProcess(this->NoReload_UnderEMP, "NoReload_UnderEMP");
		debugProcess(this->NoReload_Temporal, "NoReload_Temporal");
		debugProcess(this->RateDown_Ammo, "RateDown_Ammo");
		debugProcess(this->RateDown_Delay, "RateDown_Delay");
		debugProcess(this->RateDown_Cover, "RateDown_Cover");
		debugProcess(this->RateDown_Reset, "RateDown_Reset");
		debugProcess(this->CanManualReload, "CanManualReload");
		debugProcess(this->CanManualReload_ResetROF, "CanManualReload_ResetROF");
		debugProcess(this->CanManualReload_DetonateWarhead, "CanManualReload_DetonateWarhead");
		debugProcess(this->CanManualReload_DetonateConsume, "CanManualReload_DetonateConsume");
		debugProcess(this->Cameo_AlwaysExist, "Cameo_AlwaysExist");
		debugProcess(this->Cameo_AuxTechnos, "Cameo_AuxTechnos");
		debugProcess(this->Cameo_NegTechnos, "Cameo_NegTechnos");
		debugProcess(this->CameoCheckMutex, "CameoCheckMutex");
		debugProcess(this->UIDescription_Unbuildable, "UIDescription_Unbuildable");
		debugProcess(this->GreyCameoPCX, "GreyCameoPCX");
		debugProcess(this->Power, "Power");
		debugProcess(this->BunkerableAnyway, "BunkerableAnyway");
		debugProcess(this->JumpjetTilt, "JumpjetTilt");
		debugProcess(this->JumpjetTilt_ForwardAccelFactor, "JumpjetTilt_ForwardAccelFactor");
		debugProcess(this->JumpjetTilt_ForwardSpeedFactor, "JumpjetTilt_ForwardSpeedFactor");
		debugProcess(this->JumpjetTilt_SidewaysRotationFactor, "JumpjetTilt_SidewaysRotationFactor");
		debugProcess(this->JumpjetTilt_SidewaysSpeedFactor, "JumpjetTilt_SidewaysSpeedFactor");
		debugProcess(this->NoTurret_TrackTarget, "NoTurret_TrackTarget");
		debugProcess(this->RecountBurst, "RecountBurst");
		debugProcess(this->LaserTargetColor, "LaserTargetColor");
		debugProcess(this->AirstrikeLineColor, "AirstrikeLineColor");
		debugProcess(this->InitialSpawnsNumber, "InitialSpawnsNumber");
		debugProcess(this->Spawns_Queue, "Spawns_Queue");
		debugProcess(this->Sinkable, "Sinkable");
		debugProcess(this->SinkSpeed, "SinkSpeed");
		debugProcess(this->Sinkable_SquidGrab, "Sinkable_SquidGrab");
		debugProcess(this->SpawnerRange, "SpawnerRange");
		debugProcess(this->EliteSpawnerRange, "EliteSpawnerRange");
		debugProcess(this->AmphibiousEnter, "AmphibiousEnter");
		debugProcess(this->AmphibiousUnload, "AmphibiousUnload");
		debugProcess(this->AlternateFLH_OnTurret, "AlternateFLH_OnTurret");
		debugProcess(this->AlternateFLH_ApplyVehicle, "AlternateFLH_ApplyVehicle");
		debugProcess(this->DamagedSpeed, "DamagedSpeed");
		debugProcess(this->RadarInvisibleToHouse, "RadarInvisibleToHouse");
		debugProcess(this->BattlePoints, "BattlePoints");
		debugProcess(this->ForceWeapon_Check, "ForceWeapon_Check");
		debugProcess(this->Convert_ResetMindControl, "Convert_ResetMindControl");
		debugProcess(this->FireUp, "FireUp");
		debugProcess(this->FireUp_ResetInRetarget, "FireUp_ResetInRetarget");
		debugProcess(this->DigitalDisplay_Health_FakeAtDisguise, "DigitalDisplay_Health_FakeAtDisguise");
		debugProcess(this->EngineerRepairAmount, "EngineerRepairAmount");
		debugProcess(this->DebrisTypes_Limit, "DebrisTypes_Limit");
		debugProcess(this->DebrisMinimums, "DebrisMinimums");
		debugProcess(this->AttackMove_Follow, "AttackMove_Follow");
		debugProcess(this->AttackMove_Follow_IncludeAir, "AttackMove_Follow_IncludeAir");
		debugProcess(this->AttackMove_StopWhenTargetAcquired, "AttackMove_StopWhenTargetAcquired");
		debugProcess(this->AttackMove_PursuitTarget, "AttackMove_PursuitTarget");
		debugProcess(this->SkipCrushSlowdown, "SkipCrushSlowdown");
		debugProcess(this->RecuitedAs, "RecuitedAs");
		debugProcess(this->ForceWeapon_Buildings, "ForceWeapon_Buildings");
		debugProcess(this->ForceWeapon_Defenses, "ForceWeapon_Defenses");
		debugProcess(this->ForceWeapon_Infantry, "ForceWeapon_Infantry");
		debugProcess(this->ForceWeapon_Naval_Units, "ForceWeapon_Naval_Units");
		debugProcess(this->ForceWeapon_Units, "ForceWeapon_Units");
		debugProcess(this->ForceWeapon_Aircraft, "ForceWeapon_Aircraft");
		debugProcess(this->ForceAAWeapon_Infantry, "ForceAAWeapon_Infantry");
		debugProcess(this->ForceAAWeapon_Units, "ForceAAWeapon_Units");
		debugProcess(this->ForceAAWeapon_Aircraft, "ForceAAWeapon_Aircraft");
		debugProcess(this->AttackMove_Follow_IfMindControlIsFull, "AttackMove_Follow_IfMindControlIsFull");
		debugProcess(this->PenetratesTransport_Level, "PenetratesTransport_Level");
		debugProcess(this->PenetratesTransport_PassThroughMultiplier, "PenetratesTransport_PassThroughMultiplier");
		debugProcess(this->PenetratesTransport_FatalRateMultiplier, "PenetratesTransport_FatalRateMultiplier");
		debugProcess(this->PenetratesTransport_DamageMultiplier, "PenetratesTransport_DamageMultiplier");
		debugProcess(this->VoiceIFVRepair, "VoiceIFVRepair");
		debugProcess(this->VoiceWeaponAttacks, "VoiceWeaponAttacks");
		debugProcess(this->VoiceEliteWeaponAttacks, "VoiceEliteWeaponAttacks");
		debugProcess(this->DefaultVehicleDisguise, "DefaultVehicleDisguise");
		debugProcess(this->TurretResponse, "TurretResponse");
		debugProcess(this->Unload_SkipPassengers, "Unload_SkipPassengers");
		debugProcess(this->Unload_NoPassengers, "Unload_NoPassengers");
		debugProcess(this->Unload_SkipHarvester, "Unload_SkipHarvester");
		debugProcess(this->Unload_NoTiberiums, "Unload_NoTiberiums");
		debugProcess(this->BlockType, "BlockType");
		debugProcess(this->CanBlock, "CanBlock");
		debugProcess(this->ForceWeapon_Capture, "ForceWeapon_Capture");
		debugProcess(this->MultiWeapon, "MultiWeapon");
		debugProcess(this->MultiWeapon_IsSecondary, "MultiWeapon_IsSecondary");
		debugProcess(this->MultiWeapon_SelectCount, "MultiWeapon_SelectCount");
		debugProcess(this->ReadMultiWeapon, "ReadMultiWeapon");
		debugProcess(this->IsSimpleDeployer_ConsiderPathfinding, "IsSimpleDeployer_ConsiderPathfinding");
		debugProcess(this->IsSimpleDeployer_DisallowedLandTypes, "IsSimpleDeployer_DisallowedLandTypes");
		debugProcess(this->PassiveAcquireMode, "PassiveAcquireMode");
		debugProcess(this->PassiveAcquireMode_Togglable, "PassiveAcquireMode_Togglable");
		debugProcess(this->VoiceEnterAggressiveMode, "VoiceEnterAggressiveMode");
		debugProcess(this->VoiceExitAggressiveMode, "VoiceExitAggressiveMode");
		debugProcess(this->VoiceEnterCeasefireMode, "VoiceEnterCeasefireMode");
		debugProcess(this->VoiceExitCeasefireMode, "VoiceExitCeasefireMode");
		debugProcess(this->PlayerGuardModePursuit, "PlayerGuardModePursuit");
		debugProcess(this->PlayerGuardModeStray, "PlayerGuardModeStray");
		debugProcess(this->PlayerGuardModeGuardRangeMultiplier, "PlayerGuardModeGuardRangeMultiplier");
		debugProcess(this->PlayerGuardModeGuardRangeAddend, "PlayerGuardModeGuardRangeAddend");
		debugProcess(this->PlayerGuardStationaryStray, "PlayerGuardStationaryStray");
		debugProcess(this->AIGuardModePursuit, "AIGuardModePursuit");
		debugProcess(this->AIGuardModeStray, "AIGuardModeStray");
		debugProcess(this->AIGuardModeGuardRangeMultiplier, "AIGuardModeGuardRangeMultiplier");
		debugProcess(this->AIGuardModeGuardRangeAddend, "AIGuardModeGuardRangeAddend");
		debugProcess(this->AIGuardStationaryStray, "AIGuardStationaryStray");
		debugProcess(this->ThreatTypes, "ThreatTypes");
		debugProcess(this->CombatDamages, "CombatDamages");
		debugProcess(this->AttackFriendlies, "AttackFriendlies");
		debugProcess(this->TeamMember_ConsideredAs, "TeamMember_ConsideredAs");
		debugProcess(this->WeaponGroupAs, "WeaponGroupAs");
		debugProcess(this->CanGoAboveTarget, "CanGoAboveTarget");
		debugProcess(this->OpenTransport_RangeBonus, "OpenTransport_RangeBonus");
		debugProcess(this->OpenTransport_DamageMultiplier, "OpenTransport_DamageMultiplier");
		debugProcess(this->ParadropMission, "ParadropMission");
		debugProcess(this->AIParadropMission, "AIParadropMission");
		debugProcess(this->AreaGuardRange, "AreaGuardRange");
		debugProcess(this->MaxGuardRange, "MaxGuardRange");
		debugProcess(this->JumpjetClimbIgnoreBuilding, "JumpjetClimbIgnoreBuilding");
		debugProcess(this->BarrelOverTurret, "BarrelOverTurret");
		debugProcess(this->BarrelOffset, "BarrelOffset");
		debugProcess(this->ExtraBarrelCount, "ExtraBarrelCount");
		debugProcess(this->ExtraBarrelOffsets, "ExtraBarrelOffsets");
		debugProcess(this->ExtraTurretCount, "ExtraTurretCount");
		debugProcess(this->ExtraTurretOffsets, "ExtraTurretOffsets");
		debugProcess(this->BurstPerTurret, "BurstPerTurret");
		debugProcess(this->DriverKilled_KeptPassengers, "DriverKilled_KeptPassengers");
		debugProcess(this->DriverKilled_KillPassengers, "DriverKilled_KillPassengers");
		debugProcess(this->TintColorAirstrike, "TintColorAirstrike");

	}
#else
	template<typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->Landing_Anim)
			.Process(this->Landing_AnimOnWater)
			.Process(this->HealthBar_Hide)
			.Process(this->HealthBar_HidePips)
			.Process(this->HealthBar_Permanent)
			.Process(this->HealthBar_Permanent_PipScale)
			.Process(this->UIDescription)
			.Process(this->LowSelectionPriority)
			.Process(this->LowDeployPriority)
			.Process(this->MindControlRangeLimit)
			.Process(this->MindControl_IgnoreSize)
			.Process(this->MindControlSize)
			.Process(this->Phobos_EliteAbilities)
			.Process(this->Phobos_VeteranAbilities)
			.Process(this->E_ImmuneToType)
			.Process(this->V_ImmuneToType)
			.Process(this->R_ImmuneToType)
			.Process(this->Interceptor)
			.Process(this->Interceptor_CanTargetHouses)
			.Process(this->Interceptor_GuardRange)
			.Process(this->Interceptor_MinimumGuardRange)
			.Process(this->Interceptor_TargetingDelay)
			.Process(this->Interceptor_Weapon)
			.Process(this->Interceptor_DeleteOnIntercept)
			.Process(this->Interceptor_WeaponOverride)
			.Process(this->Interceptor_WeaponReplaceProjectile)
			.Process(this->Interceptor_WeaponCumulativeDamage)
			.Process(this->Interceptor_KeepIntact)
			.Process(this->Interceptor_ConsiderWeaponRange)
			.Process(this->Interceptor_OnlyTargetBullet)
			.Process(this->Interceptor_ApplyFirepowerMult)
			.Process(this->GroupAs)
			.Process(this->RadarJamRadius)
			.Process(this->RadarJamHouses)
			.Process(this->RadarJamDelay)
			.Process(this->RadarJamAffect)
			.Process(this->RadarJamIgnore)
			.Process(this->InhibitorRange)
			.Process(this->DesignatorRange)
			.Process(this->TurretOffset)
			.Process(this->TurretShadow)
			.Process(this->ShadowIndices)
			.Process(this->ShadowIndex_Frame)
			.Process(this->ShadowSizeCharacteristicHeight)
			.Process(this->Powered_KillSpawns)
			.Process(this->Spawn_LimitedRange)
			.Process(this->Spawn_LimitedExtraRange)

			.Process(this->AdvancedDrive_Reverse)
			.Process(this->AdvancedDrive_Reverse_FaceTarget)
			.Process(this->AdvancedDrive_Reverse_FaceTargetRange)
			.Process(this->AdvancedDrive_Reverse_MinimumDistance)
			.Process(this->AdvancedDrive_Reverse_RetreatDuration)
			.Process(this->AdvancedDrive_Reverse_Speed)
			.Process(this->AdvancedDrive_Hover)
			.Process(this->AdvancedDrive_Hover_Sink)
			.Process(this->AdvancedDrive_Hover_Spin)
			.Process(this->AdvancedDrive_Hover_Tilt)
			.Process(this->AdvancedDrive_Hover_Height)
			.Process(this->AdvancedDrive_Hover_Dampen)
			.Process(this->AdvancedDrive_Hover_Bob)

			.Process(this->Harvester_CanGuardArea)
			.Process(this->Harvester_CanGuardArea_RequireTarget)
			.Process(this->TiberiumEaterType)
			.Process(this->Spawner_DelayFrames)
			.Process(this->Harvester_Counted)
			.Process(this->Promote_IncludeSpawns)
			.Process(this->ImmuneToCrit)
			.Process(this->MultiMindControl_ReleaseVictim)
			.Process(this->CameoPriority)
			.Process(this->NoManualMove)
			.Process(this->InitialStrength)

			.Process(this->Death_NoAmmo)
			.Process(this->Death_Countdown)
			.Process(this->Death_Method)
			.Process(this->AutoDeath_Nonexist)
			.Process(this->AutoDeath_Nonexist_House)
			.Process(this->AutoDeath_Nonexist_Any)
			.Process(this->AutoDeath_Nonexist_AllowLimboed)

			.Process(this->AutoDeath_Exist)
			.Process(this->AutoDeath_Exist_House)
			.Process(this->AutoDeath_Exist_Any)
			.Process(this->AutoDeath_Exist_AllowLimboed)
			.Process(this->AutoDeath_VanishAnimation)
			.Process(this->Convert_AutoDeath)
			.Process(this->Death_WithMaster)
			.Process(this->AutoDeath_MoneyExceed)
			.Process(this->AutoDeath_MoneyBelow)
			.Process(this->AutoDeath_LowPower)
			.Process(this->AutoDeath_FullPower)
			.Process(this->AutoDeath_PassengerExceed)
			.Process(this->AutoDeath_PassengerBelow)
			.Process(this->AutoDeath_ContentIfAnyMatch)
			.Process(this->AutoDeath_OwnedByPlayer)
			.Process(this->AutoDeath_OwnedByAI)
			.Process(this->Slaved_ReturnTo)
			.Process(this->Death_IfChangeOwnership)

			.Process(this->ShieldType)
			.Process(this->WarpOut)
			.Process(this->WarpIn)
			.Process(this->WarpAway)
			.Process(this->ChronoTrigger)
			.Process(this->ChronoDistanceFactor)
			.Process(this->ChronoMinimumDelay)
			.Process(this->ChronoRangeMinimum)
			.Process(this->ChronoDelay)
			.Process(this->WarpInWeapon)
			.Process(this->WarpInMinRangeWeapon)
			.Process(this->WarpOutWeapon)
			.Process(this->WarpInWeapon_UseDistanceAsDamage)
			.Process(this->OreGathering_Anims)
			.Process(this->OreGathering_Tiberiums)
			.Process(this->OreGathering_FramesPerDir)
			.Process(this->LaserTrailData)
			.Process(this->DestroyAnim_Random)
			.Process(this->DestroyAnimSpecific)
			.Process(this->NotHuman_RandomDeathSequence)
			.Process(this->DefaultDisguise)

			.Process(this->PassengerDeletionType)

			.Process(this->OpenTopped_RangeBonus)
			.Process(this->OpenTopped_DamageMultiplier)
			.Process(this->OpenTopped_DecloakToFire)
			.Process(this->OpenTopped_WarpDistance)
			.Process(this->OpenTopped_IgnoreRangefinding)
			.Process(this->OpenTopped_AllowFiringIfDeactivated)
			.Process(this->OpenTopped_ShareTransportTarget)
			.Process(this->OpenTopped_UseTransportRangeModifiers)
			.Process(this->OpenTopped_CheckTransportDisableWeapons)
			.Process(this->AutoFire)
			.Process(this->AutoFire_TargetSelf)
			.Process(this->NoSecondaryWeaponFallback)
			.Process(this->NoSecondaryWeaponFallback_AllowAA)
			.Process(this->AllowWeaponSelectAgainstWalls)
			.Process(this->NoAmmoWeapon)
			.Process(this->NoAmmoAmount)
			.Process(this->JumpjetAllowLayerDeviation)
			.Process(this->JumpjetTurnToTarget)
			.Process(this->JumpjetCrash_Rotate)
			.Process(this->DeployingAnims)
			.Process(this->DeployingAnim_KeepUnitVisible)
			.Process(this->DeployingAnim_ReverseForUndeploy)
			.Process(this->DeployingAnim_UseUnitDrawer)
			.Process(this->SelfHealGainType)
			.Process(this->EnemyUIName)
			.Process(this->ForceWeapon_Naval_Decloaked)
			.Process(this->ForceWeapon_UnderEMP)
			.Process(this->ForceWeapon_Cloaked)
			.Process(this->ForceWeapon_Disguised)
			.Process(this->ImmuneToEMP)
			.Process(this->Ammo_Shared)
			.Process(this->Ammo_Shared_Group)
			.Process(this->Passengers_SyncOwner)
			.Process(this->Passengers_SyncOwner_RevertOnExit)
			.Process(this->Aircraft_DecreaseAmmo)
			.Process(this->UseDisguiseMovementSpeed)

			.Process(this->Insignia)
			.Process(this->InsigniaFrames)
			.Process(this->InsigniaFrame)
			.Process(this->Insignia_ShowEnemy)
			.Process(this->Insignia_Weapon)
			.Process(this->Insignia_Passengers)
			.Process(this->InsigniaFrame_Passengers)
			.Process(this->InsigniaFrames_Passengers)

			.Process(this->InitialStrength_Cloning)
			.Process(this->SelectBox)
			.Process(this->HideSelectBox)

			.Process(this->Explodes_KillPassengers)

			.Process(this->DeployFireWeapon)
			.Process(this->RevengeWeapon)
			.Process(this->RevengeWeapon_AffectsHouses)
			.Process(this->TargetZoneScanType)

			.Process(this->GrapplingAttack)

			.Process(this->PronePrimaryFireFLH)
			.Process(this->ProneSecondaryFireFLH)
			.Process(this->DeployedPrimaryFireFLH)
			.Process(this->DeployedSecondaryFireFLH)
			.Process(this->E_PronePrimaryFireFLH)
			.Process(this->E_ProneSecondaryFireFLH)
			.Process(this->E_DeployedPrimaryFireFLH)
			.Process(this->E_DeployedSecondaryFireFLH)

			.Process(this->WeaponBurstFLHs)
			.Process(this->CrouchedWeaponBurstFLHs)
			.Process(this->DeployedWeaponBurstFLHs)

			.Process(this->IronCurtain_KeptOnDeploy)
			.Process(this->ForceShield_KeptOnDeploy)
			.Process(this->IronCurtain_Effect)
			.Process(this->IronCurtain_KillWarhead)
			.Process(this->ForceShield_Effect)
			.Process(this->ForceShield_KillWarhead)
			.Process(this->SellSound)
			.Process(this->EVA_Sold)

			.Process(this->AlternateFLHs)
			.Process(this->Spawner_SpawnOffsets)

			.Process(this->Spawner_SpawnOffsets_OverrideWeaponFLH)

			.Process(this->FacingRotation_Disable)
			.Process(this->FacingRotation_DisalbeOnEMP)
			.Process(this->FacingRotation_DisalbeOnDeactivated)
			.Process(this->FacingRotation_DisableOnDriverKilled)

			.Process(this->DontShake)

			.Process(this->DiskLaserChargeUp)
			.Process(this->DiskLaserDetonate)

			.Process(this->DrainAnimationType)
			.Process(this->DrainMoneyFrameDelay)
			.Process(this->DrainMoneyAmount)
			.Process(this->DrainMoney_Display)
			.Process(this->DrainMoney_Display_Houses)
			.Process(this->DrainMoney_Display_OnTarget)
			.Process(this->DrainMoney_Display_OnTarget_UseDisplayIncome)
			.Process(this->DrainMoney_Display_Offset)

			.Process(this->TalkBubbleTime)

			.Process(this->Draw_MindControlLink)

			.Process(this->Overload_Count)
			.Process(this->Overload_Damage)
			.Process(this->Overload_Frames)
			.Process(this->Overload_DeathSound)
			.Process(this->Overload_ParticleSys)
			.Process(this->Overload_ParticleSysCount)
			.Process(this->Overload_Warhead)

			.Process(this->HitCoordOffset)
			.Process(this->HitCoordOffset_Random)

			.Process(this->DeathWeapon)
			.Process(this->CrashWeapon)
			.Process(this->CrashWeapon_s)
			.Process(this->DeathWeapon_CheckAmmo)
			.Process(this->Disable_C4WarheadExp)

			.Process(this->CrashSpinLevelRate)
			.Process(this->CrashSpinVerticalRate)

			.Process(this->ParasiteExit_Sound)

			.Process(this->PipShapes01)
			.Process(this->PipShapes02)
			.Process(this->PipGarrison)
			.Process(this->PipGarrison_FrameIndex)
			.Process(this->PipGarrison_Palette)

			.Process(this->HealthNumber_Show)
			.Process(this->HealthNumber_Percent)
			.Process(this->Healnumber_Offset)
			.Process(this->HealthNumber_SHP)
			.Process(this->Healnumber_Decrement)

			.Process(this->HealthBarSHP)
			.Process(this->HealthBarSHP_Selected)
			.Process(this->HealthBarSHPBracketOffset)
			.Process(this->HealthBarSHP_HealthFrame)
			.Process(this->HealthBarSHP_Palette)
			.Process(this->HealthBarSHP_PointOffset)
			.Process(this->HealthbarRemap)

			.Process(this->GClock_Shape)
			.Process(this->GClock_Transculency)
			.Process(this->GClock_Palette)

			.Process(this->ROF_Random)
			.Process(this->Rof_RandomMinMax)

			.Process(this->Eva_Complete)
			.Process(this->VoiceCreate)
			.Process(this->VoiceCreate_Instant)
			.Process(this->CreateSound_Enable)

			.Process(this->SlaveFreeSound_Enable)
			.Process(this->SlaveFreeSound)
			.Process(this->NoAirportBound_DisableRadioContact)
			.Process(this->SinkAnim)
			.Process(this->Tunnel_Speed)
			.Process(this->HoverType)

			.Process(this->Gattling_Overload)
			.Process(this->Gattling_Overload_Damage)
			.Process(this->Gattling_Overload_Frames)
			.Process(this->Gattling_Overload_DeathSound)
			.Process(this->Gattling_Overload_ParticleSys)
			.Process(this->Gattling_Overload_ParticleSysCount)
			.Process(this->Gattling_Overload_Warhead)

			.Process(this->IsDummy)

			.Process(this->FireSelf_Weapon)
			.Process(this->FireSelf_ROF)
			.Process(this->FireSelf_Weapon_GreenHeath)
			.Process(this->FireSelf_ROF_GreenHeath)
			.Process(this->FireSelf_Weapon_YellowHeath)
			.Process(this->FireSelf_ROF_YellowHeath)
			.Process(this->FireSelf_Weapon_RedHeath)
			.Process(this->FireSelf_ROF_RedHeath)
			.Process(this->AllowFire_IroncurtainedTarget)
			.Process(this->EngineerCaptureDelay)
			.Process(this->CommandLine_Move_Color)
			.Process(this->CommandLine_Attack_Color)
			.Process(this->PassiveAcquire_AI)
			.Process(this->CanPassiveAquire_Naval)
			.Process(this->TankDisguiseAsTank)
			.Process(this->DisguiseDisAllowed)
			.Process(this->ChronoDelay_Immune)
			.Process(this->PoseDir)
			.Process(this->Firing_IgnoreGravity)
			.Process(this->Survivors_PassengerChance)

			.Process(this->Prerequisite_RequiredTheaters)
			.Process(this->Prerequisites)
			.Process(this->Prerequisite_Lists)
			.Process(this->Prerequisite_Negative)
			.Process(this->Prerequisite_Display)
			.Process(this->BuildLimit_Requires)
			.Process(this->ConsideredNaval)
			.Process(this->ConsideredVehicle)

			.Process(this->PrimaryCrawlFLH)
			.Process(this->Elite_PrimaryCrawlFLH)
			.Process(this->SecondaryCrawlFLH)
			.Process(this->Elite_SecondaryCrawlFLH)

			.Process(this->BountyAllow)
			.Process(this->BountyDissallow)
			.Process(this->BountyBonusmult)
			.Process(this->Bounty_IgnoreEnablers)
			.Process(this->Tiberium_EmptyPipIdx)
			.Process(this->Tiberium_PipIdx)
			.Process(this->Tiberium_PipShapes)
			.Process(this->Tiberium_PipShapes_Palette)

			.Process(this->CrushLevel)
			.Process(this->CrushableLevel)
			.Process(this->DeployCrushableLevel)

			.Process(this->Experience_KillerMultiple)
			.Process(this->Experience_VictimMultiple)
			.Process(this->NavalRangeBonus)
			.Process(this->AI_LegalTarget)
			.Process(this->DeployFire_UpdateFacing)
			.Process(this->Fake_Of)
			.Process(this->CivilianEnemy)
			.Process(this->ImmuneToBerserk)
			.Process(this->Berzerk_Modifier)

			.Process(this->TargetLaser_Time)
			.Process(this->TargetLaser_WeaponIdx)
			.Process(this->CurleyShuffle)

			.Process(this->PassengersGainExperience)
			.Process(this->ExperienceFromPassengers)
			.Process(this->PassengerExperienceModifier)
			.Process(this->MindControlExperienceSelfModifier)
			.Process(this->MindControlExperienceVictimModifier)
			.Process(this->SpawnExperienceOwnerModifier)
			.Process(this->SpawnExperienceSpawnModifier)
			.Process(this->ExperienceFromAirstrike)
			.Process(this->AirstrikeExperienceModifier)
			.Process(this->Promote_IncludePassengers)
			.Process(this->Promote_Elite_Eva)
			.Process(this->Promote_Vet_Eva)
			.Process(this->Promote_Elite_Sound)
			.Process(this->Promote_Vet_Sound)
			.Process(this->Promote_Elite_Flash)
			.Process(this->Promote_Vet_Flash)

			.Process(this->Promote_Vet_Type)
			.Process(this->Promote_Elite_Type)
			.Process(this->Promote_Vet_Anim)
			.Process(this->Promote_Elite_Anim)
			.Process(this->Promote_Vet_PlaySpotlight)
			.Process(this->Promote_Elite_PlaySpotlight)
			.Process(this->Promote_Vet_Exp)
			.Process(this->Promote_Elite_Exp)
			.Process(this->DeployDir)
			.Process(this->PassengersWhitelist)
			.Process(this->PassengersBlacklist)
			.Process(this->NoManualUnload)
			.Process(this->NoSelfGuardArea)
			.Process(this->NoManualFire)
			.Process(this->NoManualEnter)
			.Process(this->NoManualEject)
			.Process(this->Passengers_BySize)
			.Process(this->Convert_Deploy)
			.Process(this->Convert_Deploy_Delay)
			.Process(this->Convert_Script)
			.Process(this->Convert_Water)
			.Process(this->Convert_Land)
			.Process(this->Harvester_LongScan)
			.Process(this->Harvester_ShortScan)
			.Process(this->Harvester_ScanCorrection)

			.Process(this->Harvester_TooFarDistance)
			.Process(this->Harvester_KickDelay)

			.Process(this->TurretRot)

			.Process(this->WaterImage)
			.Process(this->WaterImage_Yellow)
			.Process(this->WaterImage_Red)

			.Process(this->Image_Yellow)
			.Process(this->Image_Red)

			.Process(this->FallRate_Parachute)
			.Process(this->FallRate_NoParachute)
			.Process(this->FallRate_ParachuteMax)
			.Process(this->FallRate_NoParachuteMax)

			.Process(this->WeaponUINameX)
			.Process(this->NoShadowSpawnAlt)

			.Process(this->AdditionalWeaponDatas)
			.Process(this->AdditionalEliteWeaponDatas)
			.Process(this->AdditionalTurrentWeapon)
			.Process(this->OmniCrusher_Aggressive)
			.Process(this->CrusherDecloak)
			.Process(this->Crusher_SupressLostEva)
			.Process(this->CrushFireDeathWeapon)
			.Process(this->CrushDamage)
			.Process(this->CrushDamageWarhead)
			.Process(this->CrushDamagePlayWHAnim)
			.Process(this->CrushRange)
			.Process(this->DigInSound)
			.Process(this->DigOutSound)
			.Process(this->DigInAnim)
			.Process(this->DigOutAnim)
			.Process(this->EVA_UnitLost)

			.Process(this->BuildTime_Speed)
			.Process(this->BuildTime_Cost)
			.Process(this->BuildTime_LowPowerPenalty)
			.Process(this->BuildTime_MinLowPower)
			.Process(this->BuildTime_MaxLowPower)
			.Process(this->BuildTime_MultipleFactory)
			.Process(this->CloakStages)
			.Process(this->DamageSparks)
			.Process(this->ParticleSystems_DamageSmoke)
			.Process(this->ParticleSystems_DamageSparks)
			.Process(this->GattlingCyclic)
			.Process(this->CloakSound)
			.Process(this->DecloakSound)
			.Process(this->VoiceRepair)
			.Process(this->ReloadAmount)
			.Process(this->EmptyReloadAmount)
			.Process(this->TiberiumProof)
			.Process(this->TiberiumSpill)
			.Process(this->TiberiumRemains)
			.Process(this->TiberiumTransmogrify)
			.Process(this->SensorArray_Warn)
			.Process(this->IronCurtain_Modifier)
			.Process(this->ForceShield_Modifier)
			.Process(this->Survivors_PilotCount)
			.Process(this->Survivors_Pilots)
			.Process(this->Ammo_AddOnDeploy)
			.Process(this->Ammo_AutoDeployMinimumAmount)
			.Process(this->Ammo_AutoDeployMaximumAmount)
			.Process(this->Ammo_DeployUnlockMinimumAmount)
			.Process(this->Ammo_DeployUnlockMaximumAmount)
			.Process(this->ImmuneToWeb)
			.Process(this->Webby_Anims)
			.Process(this->Webby_Modifier)
			.Process(this->Webby_Duration_Variation)
			.Process(this->BerserkROFMultiplier)
			.Process(this->Refinery_UseStorage)
			.Process(this->VHPscan_Value)

			.Process(this->SelfHealing_Rate)
			.Process(this->SelfHealing_Amount)
			.Process(this->SelfHealing_Max)
			.Process(this->SelfHealing_CombatDelay)
			.Process(this->Bounty)
			.Process(this->HasSpotlight)
			.Process(this->Spot_Height)
			.Process(this->Spot_Distance)
			.Process(this->Spot_AttachedTo)
			.Process(this->Spot_DisableR)
			.Process(this->Spot_DisableG)
			.Process(this->Spot_DisableB)
			.Process(this->Spot_DisableColor)
			.Process(this->Spot_Reverse)
			.Process(this->CloakAllowed)
			.Process(this->InitialPayload_Types)
			.Process(this->InitialPayload_Nums)
			.Process(this->InitialPayload_Vet)
			.Process(this->InitialPayload_AddToTransportTeam)

			.Process(this->AlternateTheaterArt)

			.Process(this->HijackerOneTime)
			.Process(this->HijackerKillPilots)
			.Process(this->HijackerEnterSound)
			.Process(this->HijackerLeaveSound)
			.Process(this->HijackerBreakMindControl)
			.Process(this->HijackerAllowed)
			.Process(this->Survivors_PilotChance)
			.Process(this->Crew_TechnicianChance)
			.Process(this->Crew_EngineerChance)

			.Process(this->Saboteur)

			.Process(this->Cursor_Deploy)
			.Process(this->Cursor_NoDeploy)
			.Process(this->Cursor_Enter)
			.Process(this->Cursor_NoEnter)
			.Process(this->Cursor_Move)
			.Process(this->Cursor_NoMove)

			.Process(this->ImmuneToAbduction)
			.Process(this->UseROFAsBurstDelays)
			.Process(this->Chronoshift_Crushable)
			.Process(this->CanBeReversed)
			.Process(this->ReversedAs)
			.Process(this->AssaulterLevel)
			.Process(this->RadialIndicatorRadius)
			.Process(this->RadialIndicatorColor)
			.Process(this->GapRadiusInCells)
			.Process(this->SuperGapRadiusInCells)
			.Process(this->SmokeChanceRed)
			.Process(this->SmokeChanceDead)
			.Process(this->SmokeAnim)
			.Process(this->CarryallAllowed)
			.Process(this->CarryallSizeLimit)
			.Process(this->VoiceAirstrikeAttack)
			.Process(this->VoiceAirstrikeAbort)
			.Process(this->HunterSeekerDetonateProximity)
			.Process(this->HunterSeekerDescendProximity)
			.Process(this->HunterSeekerAscentSpeed)
			.Process(this->HunterSeekerDescentSpeed)
			.Process(this->HunterSeekerEmergeSpeed)
			.Process(this->HunterSeekerIgnore)

			.Process(this->Bounty_Display)
			.Process(this->Bounty_Value)
			.Process(this->Bounty_Value_PercentOf)
			.Process(this->Bounty_ReceiveSound)
			.Process(this->Bounty_Value_Option)
			.Process(this->Bounty_Value_mult)

			.Process(this->CanPassiveAcquire_Guard)
			.Process(this->CanPassiveAcquire_Cloak)
			.Process(this->CrashSpin)
			.Process(this->AirRate)
			.Process(this->Unsellable)
			.Process(this->CreateSound_afect)
			.Process(this->Chronoshift_Allow)
			.Process(this->Chronoshift_IsVehicle)
			.Process(this->SuppressorRange)
			.Process(this->AttractorRange)
			.Process(this->FactoryPlant_Multiplier)
			.Process(this->MassSelectable)
			.Process(this->TiltsWhenCrushes_Vehicles)
			.Process(this->TiltsWhenCrushes_Overlays)
			.Process(this->CrushForwardTiltPerFrame)
			.Process(this->CrushOverlayExtraForwardTilt)
			.Process(this->CrushSlowdownMultiplier)
			.Process(this->ShadowScale)
			.Process(this->AIIonCannonValue)
			.Process(this->GenericPrerequisite)
			.Process(this->ExtraPower_Amount)
			.Process(this->RecheckTechTreeWhenDie)
			.Process(this->Linked_SW)
			.Process(this->CanDrive)
			.Process(this->Operators)
			.Process(this->Operator_Any)
			.Process(this->AlwayDrawRadialIndicator)
			.Process(this->ReloadRate)
			.Process(this->CloakAnim)
			.Process(this->DecloakAnim)
			.Process(this->Cloak_KickOutParasite)
			.Process(this->DeployAnims)
			.Process(this->SpecificExpFactor)
			.Process(this->Initial_DriverKilled)
			.Process(this->VoiceCantDeploy)
			.Process(this->DigitalDisplay_Disable)
			.Process(this->DigitalDisplayTypes)
			.Process(this->EmptyAmmoPip)
			.Process(this->PipWrapAmmoPip)
			.Process(this->AmmoPipSize)
			.Process(this->ProduceCashDisplay)
			.Process(this->FactoryOwners)
			.Process(this->FactoryOwners_Forbidden)
			.Process(this->Wake)
			.Process(this->Spawner_AttackImmediately)
			.Process(this->Spawner_UseTurretFacing)
			.Process(this->FactoryOwners_HaveAllPlans)
			.Process(this->FactoryOwners_HasAllPlans)
			.Process(this->Drain_Local)
			.Process(this->Drain_Amount)

			.Process(this->HealthBar_Sections)
			.Process(this->HealthBar_Border)
			.Process(this->HealthBar_BorderFrame)
			.Process(this->HealthBar_BorderAdjust)

			.Process(this->Crashable)
			.Process(this->IsBomb)
			.Process(this->ParachuteAnim)

			.Process(this->Cloneable)
			.Process(this->ClonedAt)
			.Process(this->ClonedAs)
			.Process(this->AI_ClonedAs)
			.Process(this->BuiltAt)
			.Process(this->EMP_Sparkles)
			.Process(this->EMP_Modifier)
			.Process(this->EMP_Threshold)
			.Process(this->PoweredBy)

			.Process(this->CameoPCX)
			.Process(this->AltCameoPCX)
			.Process(this->CameoPal)
			.Process(this->LandingDir)

			.Process(this->AttachedEffect)
			.Process(this->NoAmmoEffectAnim)
			.Process(this->AttackFriendlies_WeaponIdx)
			.Process(this->AttackFriendlies_AutoAttack)
			.Process(this->PipScaleIndex)
			.Process(this->AmmoPip)
			.Process(this->AmmoPip_Palette)
			.Process(this->AmmoPipOffset)
			.Process(this->AmmoPip_Offset)
			.Process(this->AmmoPip_shape)

			.Process(this->ShowSpawnsPips)
			.Process(this->SpawnsPip)
			.Process(this->EmptySpawnsPip)
			.Process(this->SpawnsPipSize)
			.Process(this->SpawnsPipOffset)
			.Process(this->Secret_RequiredHouses)
			.Process(this->Secret_ForbiddenHouses)
			.Process(this->RequiredStolenTech)
			.Process(this->ReloadInTransport)
			.Process(this->Weeder_TriggerPreProductionBuildingAnim)
			.Process(this->Weeder_PipIndex)
			.Process(this->Weeder_PipEmptyIndex)

			.Process(this->CanBeDriven)
			.Process(this->CloakPowered)
			.Process(this->CloakDeployed)
			.Process(this->ProtectedDriver)
			.Process(this->ProtectedDriver_MinHealth)
			.Process(this->KeepAlive)
			.Process(this->SpawnDistanceFromTarget)
			.Process(this->SpawnHeight)
			.Process(this->HumanUnbuildable)
			.Process(this->NoIdleSound)
			.Process(this->Soylent_Zero)
			.Process(this->Prerequisite_Power)
			.Process(this->PassengerTurret)
			.Process(this->DetectDisguise_Percent)
			.Process(this->EliteArmor)
			.Process(this->VeteranArmor)
			.Process(this->DeployedArmor)
			.Process(this->Cloakable_IgnoreArmTimer)
			.Process(this->Untrackable)
			.Process(this->Convert_Script_Prereq)
			.Process(this->LargeVisceroid)
			.Process(this->DropPodProp)
			.Process(this->VoicePickup)
			.Process(this->CrateGoodie_RerollChance)
			.Process(this->Destroyed_CrateType)
			.Process(this->Infantry_DimWhenEMPEd)
			.Process(this->Infantry_DimWhenDisabled)
			.Process(this->Convert_HumanToComputer)
			.Process(this->Convert_ComputerToHuman)
			.Process(this->AutoDeath_OnOwnerChange)
			.Process(this->AutoDeath_OnOwnerChange_HumanToComputer)
			.Process(this->AutoDeath_OnOwnerChange_ComputerToHuman)
			.Process(this->TalkbubbleVoices)
			.Process(this->HarvesterDumpAmount)
			.Process(this->HarvesterScanAfterUnload)
			.Process(this->AttackMove_Aggressive)
			.Process(this->AttackMove_UpdateTarget)
			.Process(this->NoExtraSelfHealOrRepair)

#pragma region BuildLimitGroup
			.Process(this->BuildLimitGroup_Types)
			.Process(this->BuildLimitGroup_Nums)
			.Process(this->BuildLimitGroup_Factor)
			.Process(this->BuildLimitGroup_ContentIfAnyMatch)
			.Process(this->BuildLimitGroup_NotBuildableIfQueueMatch)
			.Process(this->BuildLimitGroup_ExtraLimit_Types)
			.Process(this->BuildLimitGroup_ExtraLimit_Nums)
			.Process(this->BuildLimitGroup_ExtraLimit_MaxCount)
			.Process(this->BuildLimitGroup_ExtraLimit_MaxNum)
#pragma endregion

			.Process(this->Tint_Color)
			.Process(this->Tint_Intensity)
			.Process(this->Tint_VisibleToHouses)

			.Process(this->PhobosAttachEffects)

			.Process(this->KeepTargetOnMove)
			.Process(this->KeepTargetOnMove_Weapon)
			.Process(this->KeepTargetOnMove_ExtraDistance)
			.Process(this->KeepTargetOnMove_NoMorePursuit)
			.Process(this->AllowAirstrike)
			.Process(this->ForbidParallelAIQueues)
			.Process(this->IgnoreForBaseCenter)
			.Process(this->EVA_Combat)
			.Process(this->CombatAlert)
			.Process(this->CombatAlert_UseFeedbackVoice)
			.Process(this->CombatAlert_UseAttackVoice)
			.Process(this->CombatAlert_UseEVA)
			.Process(this->CombatAlert_NotBuilding)
			.Process(this->SubterraneanHeight)

			.Process(this->Spawner_RecycleRange)
			.Process(this->Spawner_RecycleFLH)
			.Process(this->Spawner_RecycleOnTurret)
			.Process(this->Spawner_RecycleAnim)

			.Process(this->HugeBar)
			.Process(this->HugeBar_Priority)
			.Process(this->SprayOffsets)
			.Process(this->AINormalTargetingDelay)
			.Process(this->PlayerNormalTargetingDelay)
			.Process(this->AIGuardAreaTargetingDelay)
			.Process(this->PlayerGuardAreaTargetingDelay)
			.Process(this->DistributeTargetingFrame)
			.Process(this->AIAttackMoveTargetingDelay)
			.Process(this->PlayerAttackMoveTargetingDelay)
			.Process(this->CanBeBuiltOn)
			.Process(this->UnitBaseNormal)
			.Process(this->UnitBaseForAllyBuilding)
			.Process(this->ChronoSpherePreDelay)
			.Process(this->ChronoSphereDelay)
			.Process(this->PassengerWeapon)


			.Process(this->RefinerySmokeParticleSystemOne)
			.Process(this->RefinerySmokeParticleSystemTwo)
			.Process(this->RefinerySmokeParticleSystemThree)
			.Process(this->RefinerySmokeParticleSystemFour)

			.Process(this->SubterraneanSpeed)

			.Process(this->ForceWeapon_InRange)
			.Process(this->ForceWeapon_InRange_Overrides)
			.Process(this->ForceWeapon_InRange_ApplyRangeModifiers)
			.Process(this->ForceAAWeapon_InRange)
			.Process(this->ForceAAWeapon_InRange_Overrides)
			.Process(this->ForceAAWeapon_InRange_ApplyRangeModifiers)
			.Process(this->ForceWeapon_InRange_TechnoOnly)

			.Process(this->UnitIdleRotateTurret)
			.Process(this->UnitIdlePointToMouse)

			.Process(this->FallingDownDamage)
			.Process(this->FallingDownDamage_Water)
			.Process(this->FallingDownDamage_AllowEMP)
			.Process(this->HoverDrownable)

			.Process(this->ExtraThreat_Enabled)
			.Process(this->ExtraThreat_IsThreat)
			.Process(this->AlwaysConsideredThreat)
			.Process(this->ExtraThreat_InRange)
			.Process(this->ExtraThreatCoefficient_InRangeDistance)
			.Process(this->ExtraThreatCoefficient_Facing)
			.Process(this->ExtraThreatCoefficient_DistanceToLastTarget)

			.Process(this->DropCrate)

			.Process(this->WhenCrushed_Warhead)
			.Process(this->WhenCrushed_Weapon)
			.Process(this->WhenCrushed_Damage)
			.Process(this->WhenCrushed_Warhead_Full)
			.Process(this->Convert_ToHouseOrCountry)

			.Process(this->SuppressKillWeapons)
			.Process(this->SuppressKillWeapons_Types)

			.Process(this->NoQueueUpToEnter)
			.Process(this->NoQueueUpToUnload)

			.Process(this->NoRearm_UnderEMP)
			.Process(this->NoRearm_Temporal)
			.Process(this->NoReload_UnderEMP)
			.Process(this->NoReload_Temporal)

			.Process(this->RateDown_Ammo)
			.Process(this->RateDown_Delay)
			.Process(this->RateDown_Cover)
			.Process(this->RateDown_Reset)

			.Process(this->CanManualReload)
			.Process(this->CanManualReload_ResetROF)
			.Process(this->CanManualReload_DetonateWarhead)
			.Process(this->CanManualReload_DetonateConsume)

			.Process(this->Cameo_AlwaysExist)
			.Process(this->Cameo_AuxTechnos)
			.Process(this->Cameo_NegTechnos)
			.Process(this->CameoCheckMutex)
			.Process(this->UIDescription_Unbuildable)
			.Process(this->GreyCameoPCX)

			.Process(this->Power)
			.Process(this->BunkerableAnyway)

			.Process(this->JumpjetTilt)
			.Process(this->JumpjetTilt_ForwardAccelFactor)
			.Process(this->JumpjetTilt_ForwardSpeedFactor)
			.Process(this->JumpjetTilt_SidewaysRotationFactor)
			.Process(this->JumpjetTilt_SidewaysSpeedFactor)

			.Process(this->NoTurret_TrackTarget)
			.Process(this->RecountBurst)

			.Process(this->LaserTargetColor)
			.Process(this->AirstrikeLineColor)

			.Process(this->InitialSpawnsNumber)
			.Process(this->Spawns_Queue)

			.Process(this->Sinkable)
			.Process(this->SinkSpeed)
			.Process(this->Sinkable_SquidGrab)

			.Process(this->SpawnerRange)
			.Process(this->EliteSpawnerRange)

			.Process(this->AmphibiousEnter)
			.Process(this->AmphibiousUnload)

			.Process(this->AlternateFLH_OnTurret)
			.Process(this->AlternateFLH_ApplyVehicle)
			.Process(this->DamagedSpeed)
			.Process(this->RadarInvisibleToHouse)

			.Process(this->BattlePoints)
			.Process(this->ForceWeapon_Check)
			.Process(this->Convert_ResetMindControl)

			.Process(this->FireUp)
			.Process(this->FireUp_ResetInRetarget)

			.Process(this->DigitalDisplay_Health_FakeAtDisguise)
			.Process(this->EngineerRepairAmount)

			.Process(this->DebrisTypes_Limit)
			.Process(this->DebrisMinimums)

			.Process(this->AttackMove_Follow)
			.Process(this->AttackMove_Follow_IncludeAir)
			.Process(this->AttackMove_StopWhenTargetAcquired)
			.Process(this->AttackMove_PursuitTarget)

			.Process(this->SkipCrushSlowdown)
			.Process(this->RecuitedAs)

			.Process(this->ForceWeapon_Buildings)
			.Process(this->ForceWeapon_Defenses)
			.Process(this->ForceWeapon_Infantry)
			.Process(this->ForceWeapon_Naval_Units)
			.Process(this->ForceWeapon_Units)
			.Process(this->ForceWeapon_Aircraft)
			.Process(this->ForceAAWeapon_Infantry)
			.Process(this->ForceAAWeapon_Units)
			.Process(this->ForceAAWeapon_Aircraft)

			.Process(this->AttackMove_Follow_IfMindControlIsFull)

			.Process(this->PenetratesTransport_Level)
			.Process(this->PenetratesTransport_PassThroughMultiplier)
			.Process(this->PenetratesTransport_FatalRateMultiplier)
			.Process(this->PenetratesTransport_DamageMultiplier)

			.Process(this->VoiceIFVRepair)
			.Process(this->VoiceWeaponAttacks)
			.Process(this->VoiceEliteWeaponAttacks)
			.Process(this->DefaultVehicleDisguise)
			.Process(this->TurretResponse)
			.Process(this->Unload_SkipPassengers)
			.Process(this->Unload_NoPassengers)
			.Process(this->Unload_SkipHarvester)
			.Process(this->Unload_NoTiberiums)
			.Process(this->BlockType)
			.Process(this->CanBlock)

			.Process(this->ForceWeapon_Capture)
			.Process(this->MultiWeapon)
			.Process(this->MultiWeapon_IsSecondary)
			.Process(this->MultiWeapon_SelectCount)
			.Process(this->ReadMultiWeapon)

			.Process(this->IsSimpleDeployer_ConsiderPathfinding)
			.Process(this->IsSimpleDeployer_DisallowedLandTypes)
			.Process(this->PassiveAcquireMode)
			.Process(this->PassiveAcquireMode_Togglable)
			.Process(this->VoiceEnterAggressiveMode)
			.Process(this->VoiceExitAggressiveMode)
			.Process(this->VoiceEnterCeasefireMode)
			.Process(this->VoiceExitCeasefireMode)

			.Process(this->PlayerGuardModePursuit)
			.Process(this->PlayerGuardModeStray)
			.Process(this->PlayerGuardModeGuardRangeMultiplier)
			.Process(this->PlayerGuardModeGuardRangeAddend)
			.Process(this->PlayerGuardStationaryStray)
			.Process(this->AIGuardModePursuit)
			.Process(this->AIGuardModeStray)
			.Process(this->AIGuardModeGuardRangeMultiplier)
			.Process(this->AIGuardModeGuardRangeAddend)
			.Process(this->AIGuardStationaryStray)
			.Process(this->ThreatTypes)
			.Process(this->CombatDamages)
			.Process(this->AttackFriendlies)
			.Process(this->TeamMember_ConsideredAs)
			.Process(this->WeaponGroupAs)
			.Process(this->CanGoAboveTarget)
			.Process(this->OpenTransport_RangeBonus)
			.Process(this->OpenTransport_DamageMultiplier)
			.Process(this->ParadropMission)
			.Process(this->AIParadropMission)
			.Process(this->AreaGuardRange)
			.Process(this->MaxGuardRange)
			.Process(this->JumpjetClimbIgnoreBuilding)
			.Process(this->BarrelOverTurret)
			.Process(this->BarrelOffset)
			.Process(this->ExtraBarrelCount)
			.Process(this->ExtraBarrelOffsets)
			.Process(this->ExtraTurretCount)
			.Process(this->ExtraTurretOffsets)
			.Process(this->BurstPerTurret)
			.Process(this->DriverKilled_KeptPassengers)
			.Process(this->DriverKilled_KillPassengers)
			.Process(this->TintColorAirstrike)
			;
	}
#endif

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
	static SHPStruct* __fastcall __GetCameo(TechnoTypeClass* pThis);

};

class TechnoTypeExtContainer final //: public Container<TechnoTypeExtData>
{
public:
	static TechnoTypeExtContainer Instance;


	static int __fastcall __Repair_Cost(TechnoTypeClass* pThis);

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
