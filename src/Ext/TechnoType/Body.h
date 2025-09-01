#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/OptionalStruct.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/PhobosMap.h>

#include <New/InsigniaData.h>
#include <New/LaserTrailDataEntry.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/CursorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/TheaterTypeClass.h>
#include <New/Type/DroppodProperties.h>
#include <New/Type/CrateTypeClass.h>

#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

#include <New/AnonymousType/PassengerDeletionTypeClass.h>
#include <New/AnonymousType/TiberiumEaterTypeClass.h>
#include <New/AnonymousType/BlockTypeClass.h>

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

#include <Misc/Defines.h>

#include <Ext/ObjectType/Body.h>

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

	static ImageStatusses ReadVoxel(const char* const nKey, bool a4);

	void swap(VoxelStruct& from) {

		if (from.VXL != this->Images.VXL) {
			std::swap(from.VXL, this->Images.VXL);
		}

		if (from.HVA != this->Images.HVA) {
			std::swap(from.HVA, this->Images.HVA);
		}
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
	using ImageVector = std::vector<VoxelStruct>;
	using ColletiveCoordStructVectorData = std::array<std::vector<std::vector<CoordStruct>>*, 3u>;
	using base_type = TechnoTypeClass;

public:
#pragma region ClassMembers

	AbstractType AttachtoType { AbstractType::None };
	Valueable<bool> HealthBar_Hide { false };
	Valueable<bool> HealthBar_HidePips { false };
	Valueable<bool> HealthBar_Permanent { false };
	Valueable<bool> HealthBar_Permanent_PipScale { false };
	Valueable<CSFText> UIDescription {};
	Valueable<bool> LowSelectionPriority { false };
	PhobosFixedString<0x20> GroupAs { GameStrings::NoneStrb() };
	Valueable<int> RadarJamRadius { 0 };
	Nullable<int> InhibitorRange {};
	Nullable<int> DesignatorRange {};

	//Enemy Inhibitors
	Nullable<int> SuppressorRange {};

	//Enemy Designator
	Nullable<int> AttractorRange {};

	Valueable<Leptons> MindControlRangeLimit {};

	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_EliteAbilities {};
	MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_VeteranAbilities {};

	ValueableIdxVector<ImmunityTypeClass> E_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass> V_ImmuneToType {};
	ValueableIdxVector<ImmunityTypeClass> R_ImmuneToType {};

	Valueable<bool> Interceptor { false };
	Valueable<AffectedHouse> Interceptor_CanTargetHouses { AffectedHouse::Enemies };
	Promotable<Leptons> Interceptor_GuardRange {};
	Promotable<Leptons> Interceptor_MinimumGuardRange {};
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
	NullablePromotable<WeaponTypeClass*> WarpInMinRangeWeapon { nullptr };
	Promotable<WeaponTypeClass*> WarpOutWeapon { nullptr };
	Promotable<bool> WarpInWeapon_UseDistanceAsDamage { false };

	ValueableVector<AnimTypeClass*> OreGathering_Anims {};
	ValueableVector<int> OreGathering_Tiberiums {};
	ValueableVector<int> OreGathering_FramesPerDir {};

	Valueable<bool> DestroyAnim_Random { true };
	PhobosMap<WarheadTypeClass* , std::vector<AnimTypeClass*>> DestroyAnimSpecific {};
	Valueable<bool> NotHuman_RandomDeathSequence { false };

	Valueable<InfantryTypeClass*> DefaultDisguise { nullptr };

	Nullable<int> OpenTopped_RangeBonus {};
	Nullable<float> OpenTopped_DamageMultiplier {};
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
	Nullable<bool> ImmuneToEMP { };
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
	Valueable<bool> HideSelectBox {};

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

	Nullable<bool> FacingRotation_Disable { };
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
	NullableIdx<VoxClass> SpyplaneCameraSound { };
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
	Valueable<int> CustomMissileInaccuracy { };
	Valueable<int> CustomMissileTrailAppearDelay { 2 };

	Promotable<bool> CustomMissileRaise { true };
	Nullable<Point2D> CustomMissileOffset { };

	Valueable<AffectedHouse> Draw_MindControlLink { AffectedHouse::All };

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
	Valueable<bool> HitCoordOffset_Random { true };
	Promotable<WeaponTypeClass*> DeathWeapon { };
	Valueable<WeaponTypeClass*> CrashWeapon_s { nullptr };
	Promotable<WeaponTypeClass*> CrashWeapon { };
	Valueable<bool> DeathWeapon_CheckAmmo { false };
	Valueable<bool> Disable_C4WarheadExp { false };
	Valueable<double> CrashSpinLevelRate { 1.0 };
	Valueable<double> CrashSpinVerticalRate { 1.0 };
	ValueableIdx<VocClass> ParasiteExit_Sound { -1 };

	Nullable<SHPStruct*> PipShapes01 { };
	Nullable<SHPStruct*> PipShapes02 { };
	Nullable<SHPStruct*> PipGarrison { };
	Valueable<int> PipGarrison_FrameIndex { 0 };
	CustomPalette PipGarrison_Palette { }; //CustomPalette::PaletteMode::Default

	Valueable<bool> HealthNumber_Show { false };
	Valueable<bool> HealthNumber_Percent { false };
	Nullable<Point2D> Healnumber_Offset { };
	Nullable<SHPStruct*> HealthNumber_SHP { };
	Nullable<Point2D> Healnumber_Decrement { };
	Nullable<SHPStruct*> HealthBarSHP { };
	Nullable<SHPStruct*> HealthBarSHP_Selected { };
	Valueable<int> HealthBarSHPBracketOffset { 0 };
	Valueable<Point3D> HealthBarSHP_HealthFrame { { 18, 16, 17 } };
	CustomPalette HealthBarSHP_Palette { CustomPalette::PaletteMode::Temperate }; //

	Valueable<Point2D> HealthBarSHP_PointOffset { { 0, 0 } };
	Valueable<bool> HealthbarRemap { false };

	Nullable<SHPStruct*> GClock_Shape { };
	Nullable<int> GClock_Transculency { };
	CustomPalette GClock_Palette { }; //CustomPalette::PaletteMode::Default

	Valueable<bool> ROF_Random { true };
	Nullable<Point2D> Rof_RandomMinMax { };

	ValueableIdx<VoxClass> Eva_Complete { -1 };
	ValueableIdx<VocClass> VoiceCreate { -1 };
	Valueable<bool> VoiceCreate_Instant { false };
	Valueable<bool>CreateSound_Enable { true };

	Valueable<bool> SlaveFreeSound_Enable { true };
	NullableIdx<VocClass>SlaveFreeSound { };
	Valueable<bool> NoAirportBound_DisableRadioContact { false };
	Nullable<AnimTypeClass*> SinkAnim { };
	Nullable<double> Tunnel_Speed { };
	Valueable<HoverTypeClass*> HoverType { };

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
	Nullable<bool> PassiveAcquire_AI { };
	Nullable<bool> CanPassiveAquire_Naval { };
	Valueable<bool> TankDisguiseAsTank { false };
	ValueableVector<ObjectTypeClass*> DisguiseDisAllowed { };
	Valueable<bool> ChronoDelay_Immune { false };
	Nullable<int> PoseDir { };
	Valueable<bool> Firing_IgnoreGravity { };

	Promotable<int> Survivors_PassengerChance { -1 };
	Nullable<CoordStruct> Spawner_SpawnOffsets { };
	Valueable<bool> Spawner_SpawnOffsets_OverrideWeaponFLH { false };

	Nullable<bool> ConsideredNaval { };
	Nullable<bool> ConsideredVehicle { };

	// Ares 0.1
	DWORD Prerequisite_RequiredTheaters { 0xFFFFFFFF };
	std::vector<ValueableVector<int>> Prerequisites {};
	Valueable<int> Prerequisite_Lists { 1 };
	ValueableVector<int> Prerequisite_Negative {};
	ValueableVector<int> Prerequisite_Display {};

	ValueableVector<int> BuildLimit_Requires {};

	Promotable<int> CrushLevel {};
	Promotable<int> CrushableLevel {};
	Promotable<int> DeployCrushableLevel {};

	Valueable<float> Experience_VictimMultiple { 1.0f };
	Valueable<float> Experience_KillerMultiple { 1.0f };

	Nullable<Leptons> NavalRangeBonus { };

	Nullable<bool> AI_LegalTarget { };
	Valueable<bool> DeployFire_UpdateFacing { true };
	Valueable<TechnoTypeClass*> Fake_Of { nullptr };
	Valueable<bool> CivilianEnemy { false };
	Valueable<bool> ImmuneToBerserk { false };
	Valueable<double> Berzerk_Modifier { 1.0 };

	//Valueable<bool> IgnoreToProtect { false };
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

	Nullable<AnimTypeClass*> Promote_Vet_Anim { };
	Nullable<AnimTypeClass*> Promote_Elite_Anim { };

	Valueable<double> Promote_Vet_Exp { 0.0 };
	Valueable<double> Promote_Elite_Exp { 0.0 };
	Nullable<FacingType> DeployDir { };

	ValueableVector<TechnoTypeClass*> PassengersWhitelist { };
	ValueableVector<TechnoTypeClass*> PassengersBlacklist { };

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
	ValueableVector<int> Convert_Scipt_Prereq {};
	Valueable<TechnoTypeClass*> Convert_Water { nullptr };
	Valueable<TechnoTypeClass*> Convert_Land { nullptr };
	Valueable<bool> Convert_ResetMindControl { false };

	Nullable<Leptons> Harvester_LongScan { };
	Nullable<Leptons> Harvester_ShortScan { };
	Nullable<Leptons> Harvester_ScanCorrection { };

	Nullable<int> Harvester_TooFarDistance { };
	Nullable<int> Harvester_KickDelay { };

	Nullable<int> TurretRot { };

	Valueable<UnitTypeClass*> WaterImage { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Yellow { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Red { nullptr };

	Valueable<TechnoTypeClass*> Image_Yellow { nullptr };
	Valueable<TechnoTypeClass*> Image_Red{ nullptr };

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
	Valueable<bool> CrushDamagePlayWHAnim { false };
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
	std::vector<InfantryTypeClass*> Survivors_Pilots {};

	Valueable<int> Ammo_AddOnDeploy { 0 };
	Valueable<int> Ammo_AutoDeployMinimumAmount { -1 };
	Valueable<int> Ammo_AutoDeployMaximumAmount { -1 };
	Valueable<int> Ammo_DeployUnlockMinimumAmount { -1 };
	Valueable<int> Ammo_DeployUnlockMaximumAmount { -1 };

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

	AresAttachEffectTypeClass AttachedEffect { };

	Valueable<AnimTypeClass*> NoAmmoEffectAnim { nullptr };
	Valueable<int> AttackFriendlies_WeaponIdx { -1 };
	Valueable<bool> AttackFriendlies_AutoAttack { true };

	Nullable<WORD> PipScaleIndex { };

	Nullable<SHPStruct*> AmmoPip_shape { };
	Valueable<Point2D> AmmoPip_Offset { };
	CustomPalette AmmoPip_Palette { }; //CustomPalette::PaletteMode::Default
	Valueable<Point2D> AmmoPipOffset { { 0,0 } };

	Nullable<bool> ShowSpawnsPips {};
	Valueable<int> SpawnsPip { 1 };
	Valueable<int> EmptySpawnsPip { 0 };
	Nullable<Point2D> SpawnsPipSize { };
	Valueable<Point2D> SpawnsPipOffset { { 0,0 } };

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

	Valueable<double> FactoryPlant_Multiplier { 1.0 };
	Nullable<bool> MassSelectable {};

	Nullable<bool> TiltsWhenCrushes_Vehicles {};
	Nullable<bool> TiltsWhenCrushes_Overlays {};
	Nullable<double> CrushForwardTiltPerFrame {};
	Nullable<double> CrushOverlayExtraForwardTilt { };
	Valueable<double> CrushSlowdownMultiplier { 0.2 };

	Valueable<float> ShadowScale { -1.0f };

	Nullable<PartialVector3D<int>> AIIonCannonValue {};
	mutable OptionalStruct<bool, true> GenericPrerequisite {};
	Nullable<int> ExtraPower_Amount {};

	Nullable<bool> Bounty_Display { };
	Promotable<int> Bounty_Value { 0 };
	Promotable<float> Bounty_Value_PercentOf { 100.0 };
	ValueableIdx<VocClass> Bounty_ReceiveSound {};

	ValueableVector<TechnoTypeClass*> BountyAllow {};
	ValueableVector<TechnoTypeClass*> BountyDissallow {};

	Promotable<double> BountyBonusmult { 1.0 };
	Nullable<BountyValueOption> Bounty_Value_Option { };
	Promotable<double> Bounty_Value_mult { 1.0 };
	Valueable<bool> Bounty_IgnoreEnablers { false };
	bool RecheckTechTreeWhenDie { false };
	ValueableVector<SuperWeaponTypeClass*> Linked_SW {};

	Nullable<bool> CanDrive { }; //!< Whether this TechnoType can act as the driver of vehicles whose driver has been killed. Request #733.
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

	NullableIdx<VocClass> VoiceCantDeploy { };
	Valueable<bool> DigitalDisplay_Disable { false };
	ValueableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes {};

	Valueable<int> AmmoPip { 13 };
	Valueable<int> EmptyAmmoPip { -1 };
	Valueable<int> PipWrapAmmoPip { 14 };
	Nullable<Point2D> AmmoPipSize {};

	Valueable<bool> ProduceCashDisplay { false };

	ValueableVector<HouseTypeClass*> FactoryOwners { };
	ValueableVector<HouseTypeClass*> FactoryOwners_Forbidden { };
	Valueable<bool> FactoryOwners_HaveAllPlans { false };
	Valueable<bool> FactoryOwners_HasAllPlans { false };

	Valueable<bool> Drain_Local { false };
	Valueable<int> Drain_Amount { 0 };

	Nullable<int> HealthBar_Sections { 0 };
	Nullable<SHPStruct*> HealthBar_Border { };
	Nullable<int> HealthBar_BorderFrame { };
	Nullable<int> HealthBar_BorderAdjust { };

	Nullable<bool> Crashable {};

	Valueable<bool> IsBomb { false };
	Valueable<AnimTypeClass*> ParachuteAnim { nullptr };

	Valueable<TechnoTypeClass*> ClonedAs {};
	Valueable<TechnoTypeClass*> AI_ClonedAs {};
	Valueable<bool> Cloneable { true };
	ValueableVector<BuildingTypeClass*> ClonedAt {};
	ValueableVector<BuildingTypeClass const*> BuiltAt {};
	Nullable<AnimTypeClass*> EMP_Sparkles {};
	Valueable<double> EMP_Modifier { 1.0 };
	int EMP_Threshold { -1 };

	ValueableVector<BuildingTypeClass*> PoweredBy {};  //!< The buildingtype this unit is powered by or NULL.

	Valueable<bool> ImmuneToWeb { false };
	NullableVector<AnimTypeClass*> Webby_Anims {};
	Valueable<double> Webby_Modifier { 1.0 };
	Nullable<int> Webby_Duration_Variation { 0 };

	PhobosPCXFile CameoPCX {};
	PhobosPCXFile AltCameoPCX {};
	CustomPalette CameoPal {};  //CustomPalette::PaletteMode::Default
	Nullable<int> LandingDir {};

	// new secret lab
	DWORD Secret_RequiredHouses { 0xFFFFFFFFu };
	DWORD Secret_ForbiddenHouses { 0u };

	std::bitset<MaxHouseCount> RequiredStolenTech {};

	Valueable<bool> ReloadInTransport { false };
	Valueable<bool> Weeder_TriggerPreProductionBuildingAnim { false };

	Nullable<int> Weeder_PipIndex { };
	Nullable<int> Weeder_PipEmptyIndex { };
	Valueable<bool> CanBeDriven { true };

	Valueable<bool> CloakPowered { false };
	Valueable<bool> CloakDeployed { false };

	Valueable<bool> ProtectedDriver { false }; //!< Whether the driver of this vehicle cannot be killed, i.e. whether this vehicle is immune to KillDriver. Request #733.
	Nullable<double> ProtectedDriver_MinHealth { }; //!< The health level the unit has to be below so the driver can be killed
	Nullable<bool> KeepAlive { };

	Nullable<Leptons> SpawnDistanceFromTarget { };
	Nullable<int> SpawnHeight { };

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

	Nullable<UnitTypeClass*> LargeVisceroid { };
	NullableDroppodProperties DropPodProp {};

	Nullable<int> LaserTargetColor {};

	ValueableIdxVector<VocClass> VoicePickup {};

	Valueable<double> CrateGoodie_RerollChance { 0.0 };
	NullableIdx<CrateTypeClass*> Destroyed_CrateType {};

	Nullable<bool> Infantry_DimWhenEMPEd {};
	Nullable<bool> Infantry_DimWhenDisabled {};

	Valueable<TechnoTypeClass*> Convert_HumanToComputer { };
	Valueable<TechnoTypeClass*> Convert_ComputerToHuman { };

	Nullable<bool> TurretShadow {};
	Valueable<int> ShadowIndex_Frame { 0 };
	PhobosMap<int, int> ShadowIndices {};
	Nullable<int> ShadowSizeCharacteristicHeight {};

	std::vector<ValueableIdxVector<VocClass>> TalkbubbleVoices {};

	Nullable<float> HarvesterDumpAmount { };
	Valueable<bool> NoExtraSelfHealOrRepair { false };
	Nullable<bool> HarvesterScanAfterUnload { };
	Nullable<bool> AttackMove_Aggressive { };
	Nullable<bool> AttackMove_UpdateTarget { };

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
	Valueable<SHPStruct*> Tiberium_PipShapes {};
	CustomPalette Tiberium_PipShapes_Palette {};

	Nullable<ColorStruct> Tint_Color {};
	Valueable<double> Tint_Intensity { 0.0 };
	Valueable<AffectedHouse> Tint_VisibleToHouses { AffectedHouse::All };

	AEAttachInfoTypeClass PhobosAttachEffects {};

	Valueable<bool> KeepTargetOnMove {};
	Nullable<Leptons> KeepTargetOnMove_ExtraDistance {};
	Valueable<bool> KeepTargetOnMove_NoMorePursuit { true };

	Nullable<bool> AllowAirstrike {};

	Nullable<bool> ForbidParallelAIQueues {};
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

	Valueable<Leptons> Spawner_RecycleRange { Leptons{ -1 } };
	Valueable<CoordStruct> Spawner_RecycleFLH { {0,0,0} };
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
	Valueable<bool> ForceWeapon_InRange_ApplyRangeModifiers {};
	ValueableVector<int> ForceAAWeapon_InRange {};
	ValueableVector<double> ForceAAWeapon_InRange_Overrides {};
	Valueable<bool> ForceAAWeapon_InRange_ApplyRangeModifiers {};
	Valueable<bool> ForceWeapon_InRange_TechnoOnly {};

	Nullable<bool> UnitIdleRotateTurret {};
	Nullable<bool> UnitIdlePointToMouse {};

	Valueable<double> FallingDownDamage { 1.0 };
	Nullable<double> FallingDownDamage_Water {};

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
	Valueable<int> RateDown_Delay {};
	Valueable<int> RateDown_Cover {};
	Valueable<bool> RateDown_Reset {};

	Valueable<bool> CanManualReload { false };
	Valueable<bool> CanManualReload_ResetROF { true };
	Valueable<WarheadTypeClass*> CanManualReload_DetonateWarhead {};
	Valueable<int> CanManualReload_DetonateConsume {};

	Nullable<int> Power {};
	Valueable<bool> BunkerableAnyway {};

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

	int SpawnerRange {};
	int EliteSpawnerRange {};

	Nullable<bool> AmphibiousEnter {};
	Nullable<bool> AmphibiousUnload {};

	Valueable<bool> AlternateFLH_OnTurret { true };
	Nullable<double> DamagedSpeed {};

	Nullable<AffectedHouse> RadarInvisibleToHouse {};

	Valueable<bool> AdvancedDrive_Reverse { true };
	Valueable<bool> AdvancedDrive_Reverse_FaceTarget { true };
	Valueable<Leptons> AdvancedDrive_Reverse_FaceTargetRange { Leptons(4096) };
	Valueable<Leptons> AdvancedDrive_Reverse_MinimumDistance { Leptons(640) };
	Valueable<int> AdvancedDrive_Reverse_RetreatDuration { 150 };
	Valueable<double> AdvancedDrive_Reverse_Speed { 0.85 };
	Valueable<bool> AdvancedDrive_Hover {};
	Valueable<bool> AdvancedDrive_Hover_Sink { true };
	Valueable<bool> AdvancedDrive_Hover_Spin { true };
	Valueable<bool> AdvancedDrive_Hover_Tilt { true };
	Nullable<int> AdvancedDrive_Hover_Height {};
	Nullable<double> AdvancedDrive_Hover_Dampen {};
	Nullable<double> AdvancedDrive_Hover_Bob {};

	Valueable<bool> Harvester_CanGuardArea {};

	std::unique_ptr<TiberiumEaterTypeClass> TiberiumEaterType {};

	Nullable<int> BattlePoints {};

	bool ForceWeapon_Check {};
	Valueable<bool> FiringForceScatter { true };

	Valueable<int> FireUp { -1 };
	Valueable<bool> FireUp_ResetInRetarget { true };

	Nullable<bool> ExtendedAircraftMissions_SmoothMoving {};
	Nullable<bool> ExtendedAircraftMissions_EarlyDescend {};
	Nullable<bool> ExtendedAircraftMissions_RearApproach {};
	Valueable<bool> DigitalDisplay_Health_FakeAtDisguise { true };

	Valueable<int> EngineerRepairAmount { 0 };

	Nullable<bool> DebrisTypes_Limit {};
	ValueableVector<int> DebrisMinimums {};
	Valueable<bool> AttackMove_Follow { false };
	Valueable<bool> AttackMove_Follow_IncludeAir { false };
	Nullable<bool> AttackMove_StopWhenTargetAcquired { };
	Valueable<bool> AttackMove_PursuitTarget { false };
	Valueable<bool> SkipCrushSlowdown { false };

	Nullable <TechnoTypeClass*> RecuitedAs {};

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

	Valueable<bool> AttackMove_Follow_IfMindControlIsFull { };

	Nullable<int> PenetratesTransport_Level { };
	Valueable<double> PenetratesTransport_PassThroughMultiplier { 1.0 };
	Valueable<double> PenetratesTransport_FatalRateMultiplier { 1.0 };
	Valueable<double> PenetratesTransport_DamageMultiplier { 1.0 };

	ValueableIdx<VocClass> VoiceIFVRepair { -1 };
	ValueableVector<int> VoiceWeaponAttacks { };
	ValueableVector<int> VoiceEliteWeaponAttacks { };
	Valueable<UnitTypeClass*> DefaultVehicleDisguise { };
	Nullable<bool> TurretResponse { };

	std::unique_ptr<BlockTypeClass> BlockType {};
	Valueable<bool> CanBlock {};
#pragma endregion
public:

	void InitializeConstant();
	void Initialize();

	TechnoTypeExtData(TechnoTypeClass* pObj) : ObjectTypeExtData(pObj) {
		this->InitializeConstant();
		this->Initialize();
	}

	TechnoTypeExtData(TechnoTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~TechnoTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TechnoTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<TechnoTypeExtData*>(this)->Serialize(Stm);
	}

	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual TechnoTypeClass* This() const override { return reinterpret_cast<TechnoTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const TechnoTypeClass* This_Const() const override { return reinterpret_cast<const TechnoTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

public:

	void LoadFromINIFile_Aircraft(CCINIClass* pINI);
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

	void ApplyTurretOffset(Matrix3D* mtx, double factor);

	static WeaponStruct* GetWeaponStruct(TechnoTypeClass* pThis, int nWeaponIndex, bool isElite);

private:
	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->AttachtoType)
			.Process(this->HealthBar_Hide)
			.Process(this->HealthBar_HidePips)
			.Process(this->HealthBar_Permanent)
			.Process(this->HealthBar_Permanent_PipScale)
			.Process(this->UIDescription)
			.Process(this->LowSelectionPriority)
			.Process(this->MindControlRangeLimit)
			.Process(this->Phobos_EliteAbilities)
			.Process(this->Phobos_VeteranAbilities)
			.Process(this->E_ImmuneToType)
			.Process(this->V_ImmuneToType)
			.Process(this->R_ImmuneToType)
			.Process(this->Interceptor)
			.Process(this->Interceptor_CanTargetHouses)
			.Process(this->Interceptor_GuardRange)
			.Process(this->Interceptor_MinimumGuardRange)
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
			//;

		//Debug::LogInfo("%s AboutToLoad WeaponFLhA" , This()->ID);
		//Stm
		//	.Process(this->WeaponBurstFLHs)
		//	;
		//Debug::LogInfo("Done WeaponFLhA");

		//Stm
			.Process(this->PassengerDeletionType)

			.Process(this->OpenTopped_RangeBonus)
			.Process(this->OpenTopped_DamageMultiplier)
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
			.Process(this->NoAmmoWeapon)
			.Process(this->NoAmmoAmount)
			.Process(this->JumpjetAllowLayerDeviation)
			.Process(this->JumpjetTurnToTarget)
			.Process(this->JumpjetCrash_Rotate)
			.Process(this->DeployingAnim_AllowAnyDirection)
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
			.Process(this->ProneSecondaryFireFLH)
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
			//;
			//Debug::LogInfo("AboutToLoad WeaponFLhB");
			//Stm.Process(this->CrouchedWeaponBurstFLHs);
			//Debug::LogInfo("Done WeaponFLhB");
			//Debug::LogInfo("AboutToLoad WeaponFLhC");
			//Stm.Process(this->DeployedWeaponBurstFLHs);
			//Debug::LogInfo("Done WeaponFLhC");
			//Stm
			.Process(this->AlternateFLHs)
			.Process(this->Spawner_SpawnOffsets)

			.Process(this->Spawner_SpawnOffsets_OverrideWeaponFLH)
			//;

		//Debug::LogInfo("AboutToLoad Otammaa");
#pragma region Otamaa
		//Stm
			.Process(this->FacingRotation_Disable)
			.Process(this->FacingRotation_DisalbeOnEMP)
			.Process(this->FacingRotation_DisalbeOnDeactivated)
			.Process(this->FacingRotation_DisableOnDriverKilled)

			.Process(this->DontShake)

			.Process(this->DiskLaserChargeUp)

			.Process(this->DrainAnimationType)
			.Process(this->DrainMoneyFrameDelay)
			.Process(this->DrainMoneyAmount)
			.Process(this->DrainMoney_Display)
			.Process(this->DrainMoney_Display_Houses)
			.Process(this->DrainMoney_Display_AtFirer)
			.Process(this->DrainMoney_Display_Offset)

			.Process(this->TalkBubbleTime)

			.Process(this->AttackingAircraftSightRange)

			.Process(this->SpyplaneCameraSound)

			.Process(this->ParadropRadius)
			.Process(this->ParadropOverflRadius)
			.Process(this->Paradrop_DropPassangers)
			.Process(this->Paradrop_MaxAttempt)

			.Process(this->IsCustomMissile)
			.Process(this->CustomMissileData)
			.Process(this->CustomMissileWarhead)
			.Process(this->CustomMissileEliteWarhead)
			.Process(this->CustomMissileTakeoffAnim)
			.Process(this->CustomMissilePreLauchAnim)
			.Process(this->CustomMissileTrailerAnim)
			.Process(this->CustomMissileTrailerSeparation)
			.Process(this->CustomMissileWeapon)
			.Process(this->CustomMissileEliteWeapon)
			.Process(this->CustomMissileInaccuracy)
			.Process(this->CustomMissileTrailAppearDelay)
			.Process(this->CustomMissileRaise)
			.Process(this->CustomMissileOffset)

			.Process(this->Draw_MindControlLink)

			.Process(this->Overload_Count)
			.Process(this->Overload_Damage)
			.Process(this->Overload_Frames)
			.Process(this->Overload_DeathSound)
			.Process(this->Overload_ParticleSys)
			.Process(this->Overload_ParticleSysCount)
			.Process(this->Overload_Warhead)

			.Process(this->Landing_Anim)
			.Process(this->Landing_AnimOnWater)
			.Process(this->TakeOff_Anim)

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

			.Process(this->IsHero)
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


			.Process(this->VirtualUnit)

			.Process(this->PrimaryCrawlFLH)
			.Process(this->Elite_PrimaryCrawlFLH)
			.Process(this->SecondaryCrawlFLH)
			.Process(this->Elite_SecondaryCrawlFLH)

			.Process(this->BountyAllow)
			.Process(this->BountyDissallow)
			.Process(this->BountyBonusmult)
			.Process(this->Bounty_IgnoreEnablers)
			.Process(this->MissileHoming)

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
			//.Process(this->IgnoreToProtect)
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
			//.Process(this->Crashable)
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

			//.Process(this->BarrelImageData)
			//.Process(this->TurretImageData)
			//.Process(this->SpawnAltData)

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
#pragma endregion
			;

		this->MyExtraFireData.Serialize(Stm);
		this->MyDiveData.Serialize(Stm);
		this->MyPutData.Serialize(Stm);
		this->MyGiftBoxData.Serialize(Stm);
		//this->MyJJData.Serialize(Stm);
		this->MyPassangersData.Serialize(Stm);
		this->MySpawnSupportFLH.Serialize(Stm);
		this->MySpawnSupportDatas.Serialize(Stm);
		this->Trails.Serialize(Stm);
		this->MyFighterData.Serialize(Stm);
		this->DamageSelfData.Serialize(Stm);


		Stm.Process(this->AttachedEffect)
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
			.Process(this->Convert_Scipt_Prereq)
			.Process(this->LargeVisceroid)
			.Process(this->DropPodProp)
			.Process(this->VoicePickup)
			.Process(this->CrateGoodie_RerollChance)
			.Process(this->Destroyed_CrateType)
			.Process(this->Infantry_DimWhenEMPEd)
			.Process(this->Infantry_DimWhenDisabled)
			.Process(this->Convert_HumanToComputer)
			.Process(this->Convert_ComputerToHuman)
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
			.Process(this->KeepTargetOnMove_ExtraDistance)
			.Process(this->KeepTargetOnMove_NoMorePursuit)
			.Process(this->AllowAirstrike)
			.Process(this->ForbidParallelAIQueues)

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
			.Process(this->DamagedSpeed)
			.Process(this->RadarInvisibleToHouse)

			.Process(this->BattlePoints)
			.Process(this->ForceWeapon_Check)
			.Process(this->FiringForceScatter)
			.Process(this->Convert_ResetMindControl)

			.Process(this->FireUp)
			.Process(this->FireUp_ResetInRetarget)

			.Process(this->ExtendedAircraftMissions_SmoothMoving)
			.Process(this->ExtendedAircraftMissions_EarlyDescend)
			.Process(this->ExtendedAircraftMissions_RearApproach)
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

			.Process(this->BlockType)
			.Process(this->CanBlock)

			.Process(this->ForceWeapon_Capture)
			.Process(this->MultiWeapon)
			.Process(this->MultiWeapon_IsSecondary)
			.Process(this->MultiWeapon_SelectCount)
			.Process(this->ReadMultiWeapon)
			;
	}

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

	int SelectForceWeapon(TechnoClass* pThis, AbstractClass* pTarget);
	int SelectMultiWeapon(TechnoClass* const pThis, AbstractClass* const pTarget);

	void ParseVoiceWeaponAttacks(INI_EX& exINI, const char* pSection, ValueableVector<int>& n, ValueableVector<int>& nE);
};

class NOVTABLE FakeTechnoTypeClass : public TechnoTypeClass
{
public:
	//TODO : replace bigger hook with LJMP patch
	WeaponStruct* GetWeapon(int which);
	WeaponStruct* GetEliteWeapon(int which);
	int GetWeaponTurretIndex(int which);
};

class TechnoTypeExtContainer final //: public Container<TechnoTypeExtData>
{
public:
	static TechnoTypeExtContainer Instance;

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
