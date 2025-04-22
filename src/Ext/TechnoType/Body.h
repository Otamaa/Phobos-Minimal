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
class TechnoTypeExtData
{
public:
	using ImageVector = std::vector<VoxelStruct>;
	using ColletiveCoordStructVectorData = std::array<std::vector<std::vector<CoordStruct>>*, 3u>;

	static COMPILETIMEEVAL size_t Canary = 0x22544444;
	using base_type = TechnoTypeClass;

	//static COMPILETIMEEVAL size_t ExtOffset = 0x35C;
	static COMPILETIMEEVAL size_t ExtOffset = 0x2FC;
	//static COMPILETIMEEVAL size_t ExtOffset = 0xDF4;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	AbstractType AttachtoType { AbstractType::None };
	Valueable<bool> HealthBar_Hide { false };
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
	Valueable<int> Interceptor_Weapon { -1 };
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
	Valueable<KillMethod> Death_Method { KillMethod::Explode };
	Valueable<bool> Death_WithMaster { false };
	Valueable<int> AutoDeath_MoneyExceed { -1 };
	Valueable<int> AutoDeath_MoneyBelow { -1 };
	Valueable<bool> AutoDeath_LowPower { false };
	Valueable<bool> AutoDeath_FullPower { false };
	Valueable<int> AutoDeath_PassengerExceed { -1 };
	Valueable<int> AutoDeath_PassengerBelow { -1 };
	Valueable<bool> AutoDeath_ContentIfAnyMatch { true };
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

	Valueable<bool> DrawInsignia { true };
	Promotable<SHPStruct*> Insignia { nullptr };
	Promotable<int> InsigniaFrame { -1 };
	Nullable<bool> Insignia_ShowEnemy {};
	Valueable<Point3D> InsigniaFrames { { -1, -1, -1 } };
	Valueable<CoordStruct> InsigniaDrawOffset { {0, 0, 0} };

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

	Valueable<bool> MobileRefinery { false };
	Valueable<int> MobileRefinery_TransRate { 30 };
	Valueable<float> MobileRefinery_CashMultiplier { 1.0 };
	Valueable<int> MobileRefinery_AmountPerCell { 0 };
	ValueableVector<int> MobileRefinery_FrontOffset {};
	ValueableVector<int> MobileRefinery_LeftOffset {};
	Valueable<bool> MobileRefinery_Display { true };
	Valueable<AffectedHouse> MobileRefinery_Display_House { AffectedHouse::All };
	ValueableVector<AnimTypeClass*> MobileRefinery_Anims {};
	Valueable<bool> MobileRefinery_AnimMove { false };
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
	Valueable<bool> VoiceCreate_Instant { false };
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

	Nullable<Leptons> Harvester_LongScan { };
	Nullable<Leptons> Harvester_ShortScan { };
	Nullable<Leptons> Harvester_ScanCorrection { };

	Nullable<int> Harvester_TooFarDistance { };
	Nullable<int> Harvester_KickDelay { };

	Nullable<int> TurretRot { };

	Valueable<UnitTypeClass*> WaterImage { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Yellow { nullptr };
	Valueable<UnitTypeClass*> WaterImage_Red { nullptr };

	Valueable<UnitTypeClass*> Image_Yellow { nullptr };
	Valueable<UnitTypeClass*> Image_Red{ nullptr };

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
	Valueable<PaletteManager*> AmmoPip_Palette { }; //CustomPalette::PaletteMode::Default
	Valueable<Point2D> AmmoPipOffset { { 0,0 } };

	Nullable<bool> ShowSpawnsPips {};
	Valueable<int> SpawnsPip { 1 };
	Valueable<int> EmptySpawnsPip { 0 };
	Nullable<Point2D> SpawnsPipSize { };
	Valueable<Point2D> SpawnsPipOffset { { 0,0 } };

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
	Valueable<PaletteManager*> CameoPal {};  //CustomPalette::PaletteMode::Default
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
	Valueable<PaletteManager*> Tiberium_PipShapes_Palette {};

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

	Valueable<double> Skilled_ReverseSpeed { 0.85 };
	Valueable<double> Skilled_FaceTargetRange { 16.0 };
	Valueable<bool> Skilled_ConfrontEnemies { true };
	Valueable<int> Skilled_RetreatDuration { 150 };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromINIFile_Aircraft(CCINIClass* pINI);
	void LoadFromINIFile_EvaluateSomeVariables(CCINIClass* pINI);
	void InitializeConstant();
	void Initialize();
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
	bool IsCountedAsHarvester();

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

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TechnoTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);

public:
static COMPILETIMEEVAL double TurretMultiOffsetDefaultMult { 1.0 };
	static COMPILETIMEEVAL double TurretMultiOffsetOneByEightMult { 0.125 };

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
	static bool CanBeBuiltAt(TechnoTypeClass* pProduct, BuildingTypeClass* pFactoryType);
};

class FakeTechnoTypeClass : public TechnoTypeClass
{
public:
	//TODO : replace bigger hook with LJMP patch
	WeaponStruct* GetWeapon(int which);
	WeaponStruct* GetEliteWeapon(int which);
	int GetWeaponTurretIndex(int which);
};

class TechnoTypeExtContainer final : public Container<TechnoTypeExtData>
{
public:
	static TechnoTypeExtContainer Instance;

	PhobosMap<TechnoTypeClass*, TechnoTypeExtData*> Map {};

	virtual bool Load(TechnoTypeClass* key, IStream* pStm) override;

	void Clear() {
		this->Map.clear();
	}

//	TechnoTypeExtContainer() : Container<TechnoTypeExtData> { "TechnoTypeClass" }
//		, Map {}
//	{ }
//
//	virtual ~TechnoTypeExtContainer() override = default;
//
//private:
//	TechnoTypeExtContainer(const TechnoTypeExtContainer&) = delete;
//	TechnoTypeExtContainer(TechnoTypeExtContainer&&) = delete;
//	TechnoTypeExtContainer& operator=(const TechnoTypeExtContainer& other) = delete;
};
