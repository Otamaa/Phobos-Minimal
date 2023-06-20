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

struct ImageStatusses {
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

		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<int> RadarJamRadius;
		Nullable<int> InhibitorRange;
		Nullable<int> DesignatorRange;

		//Enemy Inhibitors
		Nullable<int> SuppressorRange;

		//Enemy Designator
		Nullable<int> AttractorRange;

		Valueable<Leptons> MindControlRangeLimit;

		MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_EliteAbilities;
		MultiBoolFixedArray<(int)PhobosAbilityType::count> Phobos_VeteranAbilities;

		ValueableIdxVector<ImmunityTypeClass> E_ImmuneToType;
		ValueableIdxVector<ImmunityTypeClass> V_ImmuneToType;
		ValueableIdxVector<ImmunityTypeClass> R_ImmuneToType;

		Valueable<bool> Interceptor;
		Valueable<AffectedHouse> Interceptor_CanTargetHouses;
		Promotable<Leptons> Interceptor_GuardRange;
		Promotable<Leptons> Interceptor_MinimumGuardRange;
		Valueable<int> Interceptor_Weapon;
		Nullable<bool> Interceptor_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptor_WeaponOverride;
		Valueable<bool> Interceptor_WeaponReplaceProjectile;
		Valueable<bool> Interceptor_WeaponCumulativeDamage;
		Valueable<bool> Interceptor_KeepIntact;
		Valueable<bool> Interceptor_ConsiderWeaponRange;
		Valueable<bool> Interceptor_OnlyTargetBullet;

		Nullable<PartialVector3D<int>> TurretOffset;
		Nullable<bool> TurretShadow;
		ValueableVector<int> ShadowIndices;
		Valueable<bool> Powered_KillSpawns;
		Valueable<bool> Spawn_LimitedRange;
		Valueable<int> Spawn_LimitedExtraRange;
		Nullable<int> Spawner_DelayFrames;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;

		std::unique_ptr<PassengerDeletionTypeClass> PassengerDeletionType;

		Valueable<bool> Death_NoAmmo;
		Valueable<int> Death_Countdown;
		Valueable<KillMethod> Death_Method;
		Valueable<bool> Death_WithMaster;

		ValueableVector<TechnoTypeClass*> AutoDeath_Nonexist;
		Valueable<AffectedHouse> AutoDeath_Nonexist_House;
		Valueable<bool> AutoDeath_Nonexist_Any;
		Valueable<bool> AutoDeath_Nonexist_AllowLimboed;
		ValueableVector<TechnoTypeClass*> AutoDeath_Exist;
		Valueable<AffectedHouse> AutoDeath_Exist_House;
		Valueable<bool> AutoDeath_Exist_Any;
		Valueable<bool> AutoDeath_Exist_AllowLimboed;
		Nullable<AnimTypeClass*> AutoDeath_VanishAnimation;

		Valueable<SlaveReturnTo> Slaved_ReturnTo;
		Valueable<ShieldTypeClass*> ShieldType;

		NullablePromotable<AnimTypeClass*> WarpOut;
		NullablePromotable<AnimTypeClass*> WarpIn;
		NullablePromotable<AnimTypeClass*> WarpAway;
		NullablePromotable<bool> ChronoTrigger;
		NullablePromotable<int> ChronoDistanceFactor;
		NullablePromotable<int> ChronoMinimumDelay;
		NullablePromotable<int> ChronoRangeMinimum;
		NullablePromotable<int> ChronoDelay;

		Promotable<WeaponTypeClass*> WarpInWeapon;
		Promotable<WeaponTypeClass*> WarpInMinRangeWeapon;
		Promotable<WeaponTypeClass*> WarpOutWeapon;
		Promotable<bool> WarpInWeapon_UseDistanceAsDamage;

		ValueableVector<AnimTypeClass*> OreGathering_Anims;
		ValueableVector<int> OreGathering_Tiberiums;
		ValueableVector<int> OreGathering_FramesPerDir;

		std::vector<std::vector<CoordStruct>> WeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteWeaponBurstFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;
		Valueable<bool> OpenTopped_IgnoreRangefinding;
		Valueable<bool> OpenTopped_AllowFiringIfDeactivated;
		Valueable<bool> OpenTopped_ShareTransportTarget;

		Valueable<bool> AutoFire;
		Valueable<bool> AutoFire_TargetSelf;

		Valueable<bool> NoSecondaryWeaponFallback;
		Valueable<bool> NoSecondaryWeaponFallback_AllowAA;

		Valueable<int> NoAmmoWeapon;
		Valueable<int> NoAmmoAmount;

		Nullable<bool> JumpjetAllowLayerDeviation;
		Nullable<bool> JumpjetTurnToTarget;
		Nullable<bool> JumpjetCrash_Rotate;

		Valueable<bool> DeployingAnim_AllowAnyDirection;
		Valueable<bool> DeployingAnim_KeepUnitVisible;
		Valueable<bool> DeployingAnim_ReverseForUndeploy;
		Valueable<bool> DeployingAnim_UseUnitDrawer;

		Nullable<SelfHealGainType> SelfHealGainType;

		Valueable<int> ForceWeapon_Naval_Decloaked;
		Valueable<int> ForceWeapon_UnderEMP;
		Valueable<int> ForceWeapon_Cloaked;
		Valueable<int> ForceWeapon_Disguised;
		Valueable<bool> ImmuneToEMP;
		Valueable<bool> Ammo_Shared;
		Valueable<int> Ammo_Shared_Group;
		Valueable<bool> Passengers_SyncOwner;
		Valueable<bool> Passengers_SyncOwner_RevertOnExit;

		Valueable<bool> Aircraft_DecreaseAmmo;

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

		ValueableVector<LaserTrailDataEntry> LaserTrailData;
		Valueable<CSFText> EnemyUIName;
		Valueable<bool> UseDisguiseMovementSpeed;

		Valueable<bool> DrawInsignia;
		Promotable<SHPStruct*> Insignia;
		Promotable<int> InsigniaFrame;
		Nullable<bool> Insignia_ShowEnemy;
		Valueable<Point3D> InsigniaFrames;
		Valueable<CoordStruct> InsigniaDrawOffset;

		Nullable<PartialVector2D<double>> InitialStrength_Cloning;

		Nullable<SHPStruct*> SHP_SelectBrdSHP;
		Valueable<PaletteManager*> SHP_SelectBrdPAL; //CustomPalette::PaletteMode::Temperate
		Nullable<bool> UseCustomSelectBrd;
		Nullable<Point3D> SelectBrd_Frame;
		Nullable<Point2D> SelectBrd_DrawOffset;
		Nullable<int> SelectBrd_TranslucentLevel;
		Nullable<bool> SelectBrd_ShowEnemy;

		Nullable<CoordStruct> PronePrimaryFireFLH;
		Nullable<CoordStruct> ProneSecondaryFireFLH;
		Nullable<CoordStruct> DeployedPrimaryFireFLH;
		Nullable<CoordStruct> DeployedSecondaryFireFLH;

		Nullable<CoordStruct> E_PronePrimaryFireFLH;
		Nullable<CoordStruct> E_ProneSecondaryFireFLH;
		Nullable<CoordStruct> E_DeployedPrimaryFireFLH;
		Nullable<CoordStruct> E_DeployedSecondaryFireFLH;

		std::vector<std::vector<CoordStruct>> CrouchedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteCrouchedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> DeployedWeaponBurstFLHs;
		std::vector<std::vector<CoordStruct>> EliteDeployedWeaponBurstFLHs;
		std::vector<CoordStruct> AlternateFLHs;

		Nullable<bool> IronCurtain_SyncDeploysInto;
		Valueable<IronCurtainFlag> IronCurtain_Effect;
		Nullable<WarheadTypeClass*> IronCurtain_KillWarhead;

		NullableIdx<VoxClass> EVA_Sold;
		NullableIdx<VocClass> SellSound;

		Valueable<bool> MobileRefinery;
		Valueable<int> MobileRefinery_TransRate;
		Valueable<float> MobileRefinery_CashMultiplier;
		Valueable<int> MobileRefinery_AmountPerCell;
		ValueableVector<int> MobileRefinery_FrontOffset;
		ValueableVector<int> MobileRefinery_LeftOffset;
		Valueable<bool> MobileRefinery_Display;
		Valueable<ColorStruct> MobileRefinery_DisplayColor;
		ValueableVector<AnimTypeClass*> MobileRefinery_Anims;
		Valueable<bool> MobileRefinery_AnimMove;
		Valueable<bool> Explodes_KillPassengers;

		Nullable<int> DeployFireWeapon;
		Nullable<WeaponTypeClass*> RevengeWeapon;
		Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;

		Valueable<TargetZoneScanType> TargetZoneScanType;
		Nullable<bool> GrapplingAttack;
#pragma region Otamaa
		Valueable<bool> FacingRotation_Disable;
		Valueable<bool> FacingRotation_DisalbeOnEMP;
		Valueable<bool> FacingRotation_DisalbeOnDeactivated;
		Valueable<bool> FacingRotation_DisableOnDriverKilled;


		Valueable<bool> DontShake;
		NullableIdx<VocClass>DiskLaserChargeUp;

		Nullable<AnimTypeClass*>DrainAnimationType;
		Nullable<int> DrainMoneyFrameDelay;
		Nullable<int> DrainMoneyAmount;
		Valueable<bool> DrainMoney_Display;
		Valueable<AffectedHouse> DrainMoney_Display_Houses;
		Valueable<bool> DrainMoney_Display_AtFirer;
		Valueable<Point2D> DrainMoney_Display_Offset;

		Nullable<float> TalkBubbleTime;
		Nullable <int> AttackingAircraftSightRange;
		NullableIdx<VoxClass>SpyplaneCameraSound;
		Nullable<int> ParadropRadius;
		Nullable<int> ParadropOverflRadius;
		Valueable<bool> Paradrop_DropPassangers;
		Valueable<int> Paradrop_MaxAttempt;

		Valueable<bool> IsCustomMissile;
		Valueable<RocketStruct> CustomMissileData;
		Valueable<WarheadTypeClass*> CustomMissileWarhead;
		Valueable<WarheadTypeClass*> CustomMissileEliteWarhead;
		Valueable<AnimTypeClass*> CustomMissileTakeoffAnim;
		Valueable<AnimTypeClass*> CustomMissilePreLauchAnim;
		Valueable<AnimTypeClass*> CustomMissileTrailerAnim;
		Valueable<int> CustomMissileTrailerSeparation;
		Valueable<WeaponTypeClass*> CustomMissileWeapon;
		Valueable<WeaponTypeClass*> CustomMissileEliteWeapon;
		Promotable<bool> CustomMissileRaise;
		Nullable<Point2D> CustomMissileOffset;

		Valueable<bool> Draw_MindControlLink;

		NullableVector<int> Overload_Count;
		NullableVector<int> Overload_Damage;
		NullableVector<int> Overload_Frames;
		NullableIdx<VocClass> Overload_DeathSound;
		Nullable<ParticleSystemTypeClass*> Overload_ParticleSys;
		Nullable<int> Overload_ParticleSysCount;
		Nullable<WarheadTypeClass*> Overload_Warhead;

		Nullable<AnimTypeClass*> Landing_Anim;
		Valueable<AnimTypeClass*> Landing_AnimOnWater;
		Nullable<AnimTypeClass*> TakeOff_Anim;
		std::vector<CoordStruct> HitCoordOffset;
		Valueable<bool> HitCoordOffset_Random;
		Promotable<WeaponTypeClass*> DeathWeapon;
		Valueable<WeaponTypeClass*> CrashWeapon_s;
		Promotable<WeaponTypeClass*> CrashWeapon;
		Valueable<bool> DeathWeapon_CheckAmmo;
		Valueable<bool> Disable_C4WarheadExp;
		Valueable<double> CrashSpinLevelRate;
		Valueable<double> CrashSpinVerticalRate;
		ValueableIdx<VocClass> ParasiteExit_Sound;

		Nullable<SHPStruct*> PipShapes01;
		Nullable<SHPStruct*> PipShapes02;
		Nullable<SHPStruct*> PipGarrison;
		Valueable<int> PipGarrison_FrameIndex;
		Valueable<PaletteManager*> PipGarrison_Palette; //CustomPalette::PaletteMode::Default

		Valueable<bool> HealthNumber_Show;
		Valueable<bool> HealthNumber_Percent;
		Nullable<Point2D> Healnumber_Offset;
		Nullable<SHPStruct*> HealthNumber_SHP;
		Nullable<Point2D> Healnumber_Decrement;
		Nullable<SHPStruct*> HealthBarSHP;
		Nullable<SHPStruct*> HealthBarSHP_Selected;
		Valueable<int> HealthBarSHPBracketOffset;
		Valueable<Point3D> HealthBarSHP_HealthFrame;
		Valueable<PaletteManager*> HealthBarSHP_Palette; //CustomPalette::PaletteMode::Temperate

		Valueable<Point2D> HealthBarSHP_PointOffset;
		Valueable<bool> HealthbarRemap;

		Nullable<SHPStruct*> GClock_Shape;
		Nullable<int> GClock_Transculency;
		Valueable<PaletteManager*> GClock_Palette; //CustomPalette::PaletteMode::Default

		Valueable<bool> ROF_Random;
		Nullable<Point2D> Rof_RandomMinMax;

		NullableIdx<VoxClass> Eva_Complete;
		ValueableIdx<VocClass> VoiceCreate;
		Valueable<bool>CreateSound_Enable;

		Valueable<bool> SlaveFreeSound_Enable;
		NullableIdx<VocClass>SlaveFreeSound;
		Valueable<bool> NoAirportBound_DisableRadioContact;
		Nullable<AnimTypeClass*> SinkAnim;
		Nullable<double> Tunnel_Speed;
		Nullable<HoverTypeClass*> HoverType;

		Valueable<bool> Gattling_Overload;
		Nullable<int> Gattling_Overload_Damage;
		Nullable<int> Gattling_Overload_Frames;
		NullableIdx<VocClass> Gattling_Overload_DeathSound;
		Nullable<ParticleSystemTypeClass*> Gattling_Overload_ParticleSys;
		Nullable<int> Gattling_Overload_ParticleSysCount;
		Nullable<WarheadTypeClass*>  Gattling_Overload_Warhead;

		Valueable<bool> IsHero;
		Valueable<bool> IsDummy;

		ValueableVector<WeaponTypeClass*> FireSelf_Weapon;
		ValueableVector<int> FireSelf_ROF;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_GreenHeath;
		ValueableVector<int> FireSelf_ROF_GreenHeath;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_YellowHeath;
		ValueableVector<int> FireSelf_ROF_YellowHeath;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_RedHeath;
		ValueableVector<int> FireSelf_ROF_RedHeath;

		Nullable<bool> AllowFire_IroncurtainedTarget;
		Valueable<int> EngineerCaptureDelay;

		Nullable<ColorStruct> CommandLine_Move_Color;
		Nullable<ColorStruct> CommandLine_Attack_Color;
		Nullable<bool> CloakMove;
		Nullable<bool> PassiveAcquire_AI;
		Valueable<bool> TankDisguiseAsTank;
		ValueableVector<ObjectTypeClass*> DisguiseDisAllowed;
		Valueable<bool> ChronoDelay_Immune;
		std::vector<LineTrailData> LineTrailData;
		Nullable<int> PoseDir;
		Valueable<bool> Firing_IgnoreGravity;

		Promotable<int> Survivors_PassengerChance;
		Nullable<CoordStruct> Spawner_SpawnOffsets;
		Valueable<bool> Spawner_SpawnOffsets_OverrideWeaponFLH;
		Nullable<UnitTypeClass*> Unit_AI_AlternateType;

		Nullable<bool> ConsideredNaval;
		Nullable<bool> ConsideredVehicle;

		// Ares 0.1
		ValueableVector<int> Prerequisite_RequiredTheaters;
		ValueableVector<int> Prerequisite;
		ValueableVector<int> Prerequisite_Negative;
		Valueable<int> Prerequisite_Lists;
		std::vector<std::vector<int>> Prerequisite_ListVector;

		Nullable<int> Riparius_FrameIDx;
		Nullable<int> Cruentus_FrameIDx;
		Nullable<int> Vinifera_FrameIDx;
		Nullable<int> Aboreus_FrameIDx;

		Promotable<int> CrushLevel;
		Promotable<int> CrushableLevel;
		Promotable<int> DeployCrushableLevel;

		Valueable<float> Experience_VictimMultiple;
		Valueable<float> Experience_KillerMultiple;

		Nullable<Leptons> NavalRangeBonus;

		Nullable<bool> AI_LegalTarget;
		Valueable<bool> DeployFire_UpdateFacing;
		Nullable<TechnoTypeClass*> Fake_Of;
		Valueable<bool> CivilianEnemy;
		Valueable<bool> ImmuneToBerserk;
		Valueable<double> Berzerk_Modifier;

		Valueable<bool> IgnoreToProtect;
		Valueable<int> TargetLaser_Time;
		ValueableVector<int> TargetLaser_WeaponIdx;

		Nullable<bool> CurleyShuffle;

		Valueable<bool> PassengersGainExperience;
		Valueable<bool> ExperienceFromPassengers;
		Valueable<double> PassengerExperienceModifier;
		Valueable<double> MindControlExperienceSelfModifier;
		Valueable<double> MindControlExperienceVictimModifier;
		Valueable<double> SpawnExperienceOwnerModifier;
		Valueable<double> SpawnExperienceSpawnModifier;
		Valueable<bool> ExperienceFromAirstrike;
		Valueable<double> AirstrikeExperienceModifier;

		Valueable<bool> Promote_IncludePassengers;
		ValueableIdx<VoxClass> Promote_Elite_Eva;
		ValueableIdx<VoxClass> Promote_Vet_Eva;
		NullableIdx<VocClass> Promote_Elite_Sound;
		NullableIdx<VocClass> Promote_Vet_Sound;
		Nullable<int> Promote_Elite_Flash;
		Nullable<int> Promote_Vet_Flash;

		Valueable<TechnoTypeClass*> Promote_Vet_Type;
		Valueable<TechnoTypeClass*> Promote_Elite_Type;

		Valueable<AnimTypeClass*> Promote_Vet_Anim;
		Valueable<AnimTypeClass*> Promote_Elite_Anim;

		Valueable<double> Promote_Vet_Exp;
		Valueable<double> Promote_Elite_Exp;
		Nullable<DirType8> DeployDir;

		ValueableVector<TechnoTypeClass*> PassengersWhitelist;
		ValueableVector<TechnoTypeClass*> PassengersBlacklist;

		Valueable<bool> NoManualUnload;
		Valueable<bool> NoSelfGuardArea;
		Valueable<bool> NoManualFire;
		Valueable<bool> NoManualEnter;
		Valueable<bool> NoManualEject;

		Valueable<bool> Passengers_BySize;
		Nullable<bool> Crashable;

		Valueable<TechnoTypeClass*> Convert_Deploy;
		Valueable<TechnoTypeClass*> Convert_Script;

		Nullable<Leptons> Harvester_LongScan;
		Nullable<Leptons> Harvester_ShortScan;
		Nullable<Leptons> Harvester_ScanCorrection;
		
		Nullable<int> Harvester_TooFarDistance;
		Nullable<int> Harvester_KickDelay;

		Nullable<int> TurretRot;
		Valueable<UnitTypeClass*> WaterImage;

		Valueable<int> FallRate_Parachute;
		Valueable<int> FallRate_NoParachute;
		Nullable<int>  FallRate_ParachuteMax;
		Nullable<int> FallRate_NoParachuteMax;

		ImageVector BarrelImageData;
		ImageVector TurretImageData;
		VoxelStruct SpawnAltData;

		ValueableVector<CSFText> WeaponUINameX;
		Valueable<bool> NoShadowSpawnAlt;

		std::vector<WeaponStruct> AdditionalWeaponDatas;
		std::vector<WeaponStruct> AdditionalEliteWeaponDatas;
		std::vector<int> AdditionalTurrentWeapon;

		Valueable<bool> OmniCrusher_Aggressive;
		Valueable<bool> CrusherDecloak;
		Valueable<bool> Crusher_SupressLostEva;

		Promotable<float> CrushFireDeathWeapon;
		Promotable<int> CrushDamage;
		Nullable<WarheadTypeClass*> CrushDamageWarhead;
		NullablePromotable<Leptons> CrushRange;

		NullableIdx<VocClass> DigInSound;
		NullableIdx<VocClass> DigOutSound;
		Nullable<AnimTypeClass*> DigInAnim;
		Nullable<AnimTypeClass*> DigOutAnim;

		ValueableIdx<VoxClass> EVA_UnitLost;

		//Build stuffs
		Nullable<double> BuildTime_Speed;
		Nullable<int> BuildTime_Cost;
		Nullable<double> BuildTime_LowPowerPenalty;
		Nullable<double> BuildTime_MinLowPower;
		Nullable<double> BuildTime_MaxLowPower;
		Nullable<double> BuildTime_MultipleFactory;

		Nullable<int> CloakStages;

		// particles
		Nullable<bool> DamageSparks;

		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSmoke;
		NullableVector<ParticleSystemTypeClass*> ParticleSystems_DamageSparks;

		Valueable<bool> GattlingCyclic;
		NullableIdx<VocClass> CloakSound;
		NullableIdx<VocClass> DecloakSound;

		ValueableIdx<VocClass> VoiceRepair;
		Valueable<int> ReloadAmount;
		Nullable<int> EmptyReloadAmount;

		Nullable<bool> TiberiumProof;
		Valueable<bool> TiberiumSpill;
		Nullable<bool> TiberiumRemains;
		Nullable<int> TiberiumTransmogrify;

		Valueable<bool> SensorArray_Warn;
		Valueable<double> IronCurtain_Modifier;
		Valueable<double> ForceShield_Modifier;
		Valueable<int> Survivors_PilotCount; //!< Defines the number of pilots inside this vehicle if Crewed=yes; maximum number of pilots who can survive. Defaults to 0 if Crewed=no; defaults to 1 if Crewed=yes. // NOTE: Flag in INI is called Survivor.Pilots

		// berserk
		Nullable<double> BerserkROFMultiplier;

		// refinery and storage related
		Valueable<bool> Refinery_UseStorage;

		//CustomPalette CameoPal;
		//Valueable<bool> Is_Fake;
		//PhobosPCXFile CameoPCX;
		//PhobosPCXFile AltCameoPCX;

		Valueable <bool> VirtualUnit;

		Nullable<CoordStruct> PrimaryCrawlFLH;
		Nullable<CoordStruct> Elite_PrimaryCrawlFLH;
		Nullable<CoordStruct> SecondaryCrawlFLH;
		Nullable<CoordStruct> Elite_SecondaryCrawlFLH;

		Valueable<bool> MissileHoming;

		ExtraFireData MyExtraFireData;
		AircraftDiveData MyDiveData;
		AircraftPutData MyPutData;
		GiftBoxData MyGiftBoxData;
		//JJFacingData  MyJJData;
		PassengersData MyPassangersData;
		SpawnSupportFLHData MySpawnSupportFLH;
		SpawnSupportData MySpawnSupportDatas;
		TrailsReader Trails;
		FighterAreaGuardData MyFighterData;
		DamageSelfType DamageSelfData;

		AresAttachEffectTypeClass AttachedEffect;
#pragma endregion

		Valueable<AnimTypeClass*> NoAmmoEffectAnim;
		Valueable<int> AttackFriendlies_WeaponIdx;
		Nullable<WORD> PipScaleIndex;

		Nullable<SHPStruct*> AmmoPip;
		Valueable<Point2D> AmmoPip_Offset;
		Valueable<PaletteManager*> AmmoPip_Palette; //CustomPalette::PaletteMode::Default
		struct InsigniaData
		{
			Promotable<SHPStruct*> Shapes { nullptr };
			Promotable<int> Frame { -1 };
			Valueable<Point3D> Frames { { -1 , -1 , -1 } };

			inline bool Load(PhobosStreamReader& stm, bool registerForChange) {
				return this->Serialize(stm);
			}

			inline bool Save(PhobosStreamWriter& stm) const {
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

		std::vector<InsigniaData> Insignia_Weapon;

		Valueable<int> VHPscan_Value;
		Valueable<bool> CloakAllowed;

		ValueableVector<TechnoTypeClass*> InitialPayload_Types;
		ValueableVector<int> InitialPayload_Nums;
		ValueableVector<Rank> InitialPayload_Vet;
		ValueableVector<bool> InitialPayload_AddToTransportTeam;

		Valueable<bool> AlternateTheaterArt;

		Valueable<bool> HijackerOneTime;
		Valueable<int> HijackerKillPilots;

		ValueableIdx<VocClass> HijackerEnterSound;
		ValueableIdx<VocClass> HijackerLeaveSound;

		Promotable<int> Survivors_PilotChance;

		ValueableIdx<CursorTypeClass*> Cursor_Deploy;
		ValueableIdx<CursorTypeClass*> Cursor_NoDeploy;
		ValueableIdx<CursorTypeClass*> Cursor_Enter;
		ValueableIdx<CursorTypeClass*> Cursor_NoEnter;
		ValueableIdx<CursorTypeClass*> Cursor_Move;
		ValueableIdx<CursorTypeClass*> Cursor_NoMove;

		Valueable<bool> ImmuneToAbduction; //680, 1362
		Valueable<bool> UseROFAsBurstDelays;

		Valueable<bool> Chronoshift_Crushable;
		Valueable<bool> CanBeReversed;
		Nullable<TechnoTypeClass*> ReversedAs;
		Valueable<int> AssaulterLevel;

		Nullable<double> SelfHealing_Rate;
		Promotable<int> SelfHealing_Amount;
		Promotable<double> SelfHealing_Max;
		Promotable<int> SelfHealing_CombatDelay;

		Valueable<bool> Bounty;
		Valueable<bool> HasSpotlight;

		Nullable<int> Crew_TechnicianChance;
		Nullable<int> Crew_EngineerChance;
		Valueable<bool> Saboteur;

		Nullable<int> RadialIndicatorRadius;
		Nullable<ColorStruct> RadialIndicatorColor;

		Valueable<int> GapRadiusInCells;
		Valueable<int> SuperGapRadiusInCells;

		// smoke when damaged
		Nullable<int> SmokeChanceRed;
		Nullable<int> SmokeChanceDead;
		Valueable<AnimTypeClass*> SmokeAnim;

		Nullable<bool> CarryallAllowed;
		Nullable<int> CarryallSizeLimit;

		NullableIdx<VocClass> VoiceAirstrikeAttack;
		NullableIdx<VocClass> VoiceAirstrikeAbort;

		// hunter seeker
		Nullable<int> HunterSeekerDetonateProximity;
		Nullable<int> HunterSeekerDescendProximity;
		Nullable<int> HunterSeekerAscentSpeed;
		Nullable<int> HunterSeekerDescentSpeed;
		Nullable<int> HunterSeekerEmergeSpeed;
		Valueable<bool> HunterSeekerIgnore { false };

		Valueable<bool> CanPassiveAcquire_Guard;
		Valueable<bool> CanPassiveAcquire_Cloak;

		Valueable<bool> CrashSpin;
		Valueable<int> AirRate;
		Nullable<bool> Unsellable;

		Nullable<AffectedHouse> CreateSound_afect;
		Valueable<bool> Chronoshift_Allow;
		Valueable<bool> Chronoshift_IsVehicle;

		Valueable<double> FactoryPlant_Multiplier {1.0};
		Nullable<bool> MassSelectable {};

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject)
			, HealthBar_Hide { false }
			, UIDescription {}
			, LowSelectionPriority { false }
			, GroupAs { NONE_STR }
			, RadarJamRadius { 0 }
			, InhibitorRange { }
			, DesignatorRange { }
			, MindControlRangeLimit {}

			, Phobos_EliteAbilities {}
			, Phobos_VeteranAbilities {}

			, E_ImmuneToType {}
			, V_ImmuneToType {}
			, R_ImmuneToType {}

			, Interceptor { false }
			, Interceptor_CanTargetHouses { AffectedHouse::Enemies }
			, Interceptor_GuardRange {}
			, Interceptor_MinimumGuardRange {}
			, Interceptor_Weapon { -1 }
			, Interceptor_DeleteOnIntercept {}
			, Interceptor_WeaponOverride {}
			, Interceptor_WeaponReplaceProjectile { false }
			, Interceptor_WeaponCumulativeDamage { false }
			, Interceptor_KeepIntact { false }
			, Interceptor_ConsiderWeaponRange { false }
			, Interceptor_OnlyTargetBullet { false }
			, TurretOffset {}
			, TurretShadow { }
			, ShadowIndices { }
			, Powered_KillSpawns { false }
			, Spawn_LimitedRange { false }
			, Spawn_LimitedExtraRange { 0 }
			, Spawner_DelayFrames {}
			, Harvester_Counted {}
			, Promote_IncludeSpawns { false }
			, ImmuneToCrit { false }
			, MultiMindControl_ReleaseVictim { false }
			, CameoPriority { 0 }
			, NoManualMove { false }
			, InitialStrength {}

			, Death_NoAmmo { false }
			, Death_Countdown { 0 }
			, Death_Method { KillMethod::Explode }
			, Death_WithMaster { false }
			, AutoDeath_Nonexist {}
			, AutoDeath_Nonexist_House { AffectedHouse::Owner }
			, AutoDeath_Nonexist_Any { false }
			, AutoDeath_Nonexist_AllowLimboed { true }
			, AutoDeath_Exist {}
			, AutoDeath_Exist_House { AffectedHouse::Owner }
			, AutoDeath_Exist_Any { false }
			, AutoDeath_Exist_AllowLimboed { true }
			, AutoDeath_VanishAnimation {}

			, Slaved_ReturnTo { SlaveReturnTo::Killer }
			, ShieldType { nullptr }
			, WarpOut {}
			, WarpIn {}
			, WarpAway {}
			, ChronoTrigger {}
			, ChronoDistanceFactor {}
			, ChronoMinimumDelay {}
			, ChronoRangeMinimum {}
			, ChronoDelay {}
			, WarpInWeapon { nullptr }
			, WarpInMinRangeWeapon { nullptr }
			, WarpOutWeapon { nullptr }
			, WarpInWeapon_UseDistanceAsDamage { false }
			, OreGathering_Anims {}
			, OreGathering_Tiberiums {}
			, OreGathering_FramesPerDir {}
			, WeaponBurstFLHs {}
			, EliteWeaponBurstFLHs {}
			, DestroyAnim_Random { true }
			, NotHuman_RandomDeathSequence { false }
			, DefaultDisguise {}
			, OpenTopped_RangeBonus {}
			, OpenTopped_DamageMultiplier {}
			, OpenTopped_WarpDistance {}
			, OpenTopped_IgnoreRangefinding { false }
			, OpenTopped_AllowFiringIfDeactivated { true }
			, OpenTopped_ShareTransportTarget { true }

			, AutoFire { false }
			, AutoFire_TargetSelf { false }
			, NoSecondaryWeaponFallback { false }
			, NoSecondaryWeaponFallback_AllowAA { false }
			, NoAmmoWeapon { -1 }
			, NoAmmoAmount { 0 }
			, JumpjetAllowLayerDeviation {}
			, JumpjetTurnToTarget {}
			, JumpjetCrash_Rotate {}
			, DeployingAnim_AllowAnyDirection { false }
			, DeployingAnim_KeepUnitVisible { false }
			, DeployingAnim_ReverseForUndeploy { true }
			, DeployingAnim_UseUnitDrawer { true }
			, SelfHealGainType {}
			, ForceWeapon_Naval_Decloaked { -1 }
			, ForceWeapon_UnderEMP { -1 }
			, ForceWeapon_Cloaked { -1 }
			, ForceWeapon_Disguised { -1 }
			, ImmuneToEMP { false }
			, Ammo_Shared { false }
			, Ammo_Shared_Group { -1 }
			, Passengers_SyncOwner { false }
			, Passengers_SyncOwner_RevertOnExit { true }
			, Aircraft_DecreaseAmmo { true }
			, LaserTrailData {}
			, EnemyUIName {}
			, UseDisguiseMovementSpeed {}
			, DrawInsignia { true }
			, Insignia {}
			, InsigniaFrame { -1 }
			, Insignia_ShowEnemy {}
			, InsigniaFrames { { -1, -1, -1 } }
			, InsigniaDrawOffset { {0 , 0 , 0} }
			, InitialStrength_Cloning {}
			, SHP_SelectBrdSHP {  }
			, SHP_SelectBrdPAL {}
			, UseCustomSelectBrd {}
			, SelectBrd_Frame { {-1,-1,-1} }
			, SelectBrd_DrawOffset {}
			, SelectBrd_TranslucentLevel {}
			, SelectBrd_ShowEnemy {}

			, PronePrimaryFireFLH { }
			, ProneSecondaryFireFLH { }
			, DeployedPrimaryFireFLH { }
			, DeployedSecondaryFireFLH { }

			, E_PronePrimaryFireFLH { }
			, E_ProneSecondaryFireFLH { }
			, E_DeployedPrimaryFireFLH { }
			, E_DeployedSecondaryFireFLH { }

			, CrouchedWeaponBurstFLHs { }
			, EliteCrouchedWeaponBurstFLHs { }
			, DeployedWeaponBurstFLHs { }
			, EliteDeployedWeaponBurstFLHs { }
			, AlternateFLHs { }

			, IronCurtain_SyncDeploysInto { }
			, IronCurtain_Effect { IronCurtainFlag::Default }
			, IronCurtain_KillWarhead { }

			, EVA_Sold {}
			, SellSound { }

			, MobileRefinery { false }
			, MobileRefinery_TransRate { 30 }
			, MobileRefinery_CashMultiplier { 1.0 }
			, MobileRefinery_AmountPerCell { 0 }
			, MobileRefinery_FrontOffset { }
			, MobileRefinery_LeftOffset { }
			, MobileRefinery_Display { true }
			, MobileRefinery_DisplayColor { { 57,197,187 } }
			, MobileRefinery_Anims { }
			, MobileRefinery_AnimMove { false }
			, Explodes_KillPassengers { true }

			, DeployFireWeapon {}
			, RevengeWeapon {}
			, RevengeWeapon_AffectsHouses { AffectedHouse::All }
			, TargetZoneScanType { TargetZoneScanType::Same }
			, GrapplingAttack { }
#pragma region Otamaa
			, FacingRotation_Disable { false }
			, FacingRotation_DisalbeOnEMP { false }
			, FacingRotation_DisalbeOnDeactivated { false }
			, FacingRotation_DisableOnDriverKilled { true }

			, DontShake { true }

			, DiskLaserChargeUp {}
			, DrainAnimationType {}
			, DrainMoneyFrameDelay {}
			, DrainMoneyAmount {}
			, DrainMoney_Display { false }
			, DrainMoney_Display_Houses { AffectedHouse::All }
			, DrainMoney_Display_AtFirer { true }
			, DrainMoney_Display_Offset { { 0,0 } }

			, TalkBubbleTime {}

			, AttackingAircraftSightRange {}

			, SpyplaneCameraSound {}

			, ParadropRadius {}
			, ParadropOverflRadius {}
			, Paradrop_DropPassangers { true }
			, Paradrop_MaxAttempt { 5 }

			, IsCustomMissile { false }
			, CustomMissileData {}
			, CustomMissileWarhead { nullptr }
			, CustomMissileEliteWarhead { nullptr }
			, CustomMissileTakeoffAnim { nullptr }
			, CustomMissilePreLauchAnim { nullptr }
			, CustomMissileTrailerAnim { nullptr }
			, CustomMissileTrailerSeparation { 3 }
			, CustomMissileWeapon { nullptr }
			, CustomMissileEliteWeapon { nullptr }
			, CustomMissileRaise { true }
			, CustomMissileOffset { }
			, Draw_MindControlLink { true }

			, Overload_Count {}
			, Overload_Damage {}
			, Overload_Frames {}
			, Overload_DeathSound {}
			, Overload_ParticleSys {}
			, Overload_ParticleSysCount {}
			, Overload_Warhead {}

			, Landing_Anim { }
			, Landing_AnimOnWater { nullptr }
			, TakeOff_Anim { nullptr }

			, HitCoordOffset { }
			, HitCoordOffset_Random { true }

			, DeathWeapon { }
			, CrashWeapon_s { nullptr }
			, CrashWeapon { }
			, DeathWeapon_CheckAmmo { false }
			, Disable_C4WarheadExp { false }

			, CrashSpinLevelRate { 1.0 }
			, CrashSpinVerticalRate { 1.0 }

			, ParasiteExit_Sound {}

			, PipShapes01 {}
			, PipShapes02 {}
			, PipGarrison {}
			, PipGarrison_FrameIndex { 0 }
			, PipGarrison_Palette {}

			, HealthNumber_Show { false }
			, HealthNumber_Percent { false }
			, Healnumber_Offset {}
			, HealthNumber_SHP {}
			, Healnumber_Decrement {}

			, HealthBarSHP {}
			, HealthBarSHP_Selected {}
			, HealthBarSHPBracketOffset { 0 }
			, HealthBarSHP_HealthFrame { { 18,16,17 } }
			, HealthBarSHP_Palette {}
			, HealthBarSHP_PointOffset { { 0,0 } }
			, HealthbarRemap { false }

			, GClock_Shape { }
			, GClock_Transculency { }
			, GClock_Palette { }

			, ROF_Random { true }
			, Rof_RandomMinMax { }

			, Eva_Complete { }
			, VoiceCreate { -1 }
			, CreateSound_Enable { true }

			, SlaveFreeSound_Enable { true }
			, SlaveFreeSound { }
			, NoAirportBound_DisableRadioContact { false }
			, SinkAnim { }
			, Tunnel_Speed { }
			, HoverType { }

			, Gattling_Overload { false }
			, Gattling_Overload_Damage {}
			, Gattling_Overload_Frames {}
			, Gattling_Overload_DeathSound {}
			, Gattling_Overload_ParticleSys {}
			, Gattling_Overload_ParticleSysCount {}
			, Gattling_Overload_Warhead {}

			, IsHero { false }
			, IsDummy { false }
			, FireSelf_Weapon {}
			, FireSelf_ROF {}
			, FireSelf_Weapon_GreenHeath {}
			, FireSelf_ROF_GreenHeath {}
			, FireSelf_Weapon_YellowHeath {}
			, FireSelf_ROF_YellowHeath {}
			, FireSelf_Weapon_RedHeath {}
			, FireSelf_ROF_RedHeath {}
			, AllowFire_IroncurtainedTarget { }
			, EngineerCaptureDelay { 0 }
			, CommandLine_Move_Color { }
			, CommandLine_Attack_Color { }
			, CloakMove { }
			, PassiveAcquire_AI { }
			, TankDisguiseAsTank { false }
			, DisguiseDisAllowed { }
			, ChronoDelay_Immune { false }
			, LineTrailData { }
			, PoseDir { }
			, Firing_IgnoreGravity { false }
			, Survivors_PassengerChance { -1 }
			, Spawner_SpawnOffsets { }
			, Spawner_SpawnOffsets_OverrideWeaponFLH { false }
			, Unit_AI_AlternateType {}

			, ConsideredNaval { }
			, ConsideredVehicle { }

			, Prerequisite { }
			, Prerequisite_Negative { }
			, Prerequisite_Lists { 0 }

			, Riparius_FrameIDx { }
			, Cruentus_FrameIDx { }
			, Vinifera_FrameIDx { }
			, Aboreus_FrameIDx { }

			, CrushLevel {}
			, CrushableLevel {}
			, DeployCrushableLevel {}

			, Experience_VictimMultiple { 1.0f }
			, Experience_KillerMultiple { 1.0f }

			, NavalRangeBonus { }
			, AI_LegalTarget { }
			, DeployFire_UpdateFacing { true }
			, Fake_Of { }
			, CivilianEnemy { false }
			, ImmuneToBerserk { false }
			, Berzerk_Modifier { 1.0 }
			, IgnoreToProtect { false }
			, TargetLaser_Time { 15 }
			, TargetLaser_WeaponIdx { 0 }
			, CurleyShuffle { }

			, PassengersGainExperience { false }
			, ExperienceFromPassengers { true }
			, PassengerExperienceModifier { 1.0 }
			, MindControlExperienceSelfModifier { 0.0 }
			, MindControlExperienceVictimModifier { 1.0 }
			, SpawnExperienceOwnerModifier { 0.0 }
			, SpawnExperienceSpawnModifier { 1.0 }
			, ExperienceFromAirstrike { false }
			, AirstrikeExperienceModifier { 1.0 }

			, Promote_IncludePassengers { false }
			, Promote_Elite_Eva { -1 }
			, Promote_Vet_Eva { -1 }
			, Promote_Elite_Sound { }
			, Promote_Vet_Sound { }
			, Promote_Elite_Flash { }
			, Promote_Vet_Flash { }

			, Promote_Vet_Type { nullptr }
			, Promote_Elite_Type { nullptr }

			, Promote_Vet_Anim { nullptr }
			, Promote_Elite_Anim { nullptr }

			, Promote_Vet_Exp { 0.0 }
			, Promote_Elite_Exp { 0.0 }
			, DeployDir { }
			, PassengersWhitelist { }
			, PassengersBlacklist { }

			, NoManualUnload { false }
			, NoSelfGuardArea { false }
			, NoManualFire { false }
			, NoManualEnter { false }
			, NoManualEject { false }

			, Passengers_BySize { true }
			, Crashable { }

			, Convert_Deploy { nullptr }
			, Convert_Script { nullptr }
			, Harvester_LongScan { }
			, Harvester_ShortScan { }
			, Harvester_ScanCorrection { }

			, Harvester_TooFarDistance { }
			, Harvester_KickDelay { }
			, TurretRot { }

			, WaterImage { nullptr }
			, FallRate_Parachute { 1 }
			, FallRate_NoParachute { 1 }
			, FallRate_ParachuteMax { }
			, FallRate_NoParachuteMax { }

			, BarrelImageData {}
			, TurretImageData {}
			, SpawnAltData { nullptr , nullptr }
			, WeaponUINameX {}
			, NoShadowSpawnAlt { false }

			, AdditionalWeaponDatas {}
			, AdditionalEliteWeaponDatas {}
			, AdditionalTurrentWeapon {}
			, OmniCrusher_Aggressive { false }
			, CrusherDecloak { true }
			, Crusher_SupressLostEva { false }

			, CrushFireDeathWeapon { 0.0f }
			, CrushDamage { }
			, CrushDamageWarhead { }
			, CrushRange { }

			, DigInSound {}
			, DigOutSound {}
			, DigInAnim {}
			, DigOutAnim {}
			, EVA_UnitLost { -1 }
			, BuildTime_Speed {}
			, BuildTime_Cost {}
			, BuildTime_LowPowerPenalty {}
			, BuildTime_MinLowPower {}
			, BuildTime_MaxLowPower {}
			, BuildTime_MultipleFactory {}

			, CloakStages { }
			, DamageSparks {  }

			, ParticleSystems_DamageSmoke {  }
			, ParticleSystems_DamageSparks {  }
			, GattlingCyclic { false }
			, CloakSound {  }
			, DecloakSound {  }
			, VoiceRepair { -1 }
			, ReloadAmount { 1 }
			, EmptyReloadAmount { }

			, TiberiumProof {}
			, TiberiumSpill { false }
			, TiberiumRemains {}
			, TiberiumTransmogrify {}

			, SensorArray_Warn { true }
			, IronCurtain_Modifier { 1.0 }
			, ForceShield_Modifier { 1.0 }
			, Survivors_PilotCount { -1 }
			// berserk
			, BerserkROFMultiplier {  }
			, Refinery_UseStorage { false }

			, VirtualUnit { false }

			, PrimaryCrawlFLH { }
			, Elite_PrimaryCrawlFLH { }
			, SecondaryCrawlFLH { }
			, Elite_SecondaryCrawlFLH { }
			, MissileHoming { false }

			, MyExtraFireData { }
			, MyDiveData { }
			, MyPutData { }
			, MyGiftBoxData { }
			//, MyJJData { }
			, MyPassangersData { }
			, MySpawnSupportFLH { }
			, MySpawnSupportDatas { }
			, Trails { }
			, MyFighterData { }
			, DamageSelfData { }

			, AttachedEffect { OwnerObject }
#pragma endregion
			, NoAmmoEffectAnim { nullptr }
			, AttackFriendlies_WeaponIdx { -1 }
			, PipScaleIndex { }
			, AmmoPip { }
			, AmmoPip_Offset { }
			, AmmoPip_Palette { }

			, Insignia_Weapon {}
			, VHPscan_Value {2}
			, CloakAllowed { true }
			, InitialPayload_Types { }
			, InitialPayload_Nums { }
			, InitialPayload_Vet { }
			, InitialPayload_AddToTransportTeam { }

			, AlternateTheaterArt { false }
			, HijackerOneTime { false }
			, HijackerKillPilots { 0 }
			, HijackerEnterSound { -1 }
			, HijackerLeaveSound { -1 }
			, Survivors_PilotChance { -1 }

			, Cursor_Deploy {  (int)MouseCursorType::Deploy }
			, Cursor_NoDeploy { (int)MouseCursorType::NoDeploy }
			, Cursor_Enter { (int)MouseCursorType::Enter }
			, Cursor_NoEnter { (int)MouseCursorType::NoEnter }
			, Cursor_Move { (int)MouseCursorType::Move }
			, Cursor_NoMove { (int)MouseCursorType::NoMove }

			, ImmuneToAbduction { false }
			, UseROFAsBurstDelays { false }
			, Chronoshift_Crushable{ true }
			, CanBeReversed { false }
			, ReversedAs { }
			, AssaulterLevel { 0 }
			, SelfHealing_Rate { }
			, SelfHealing_Amount { 1 }
			, SelfHealing_Max { 1.0 }
			, SelfHealing_CombatDelay { 0 }
			, Bounty { false }
			, HasSpotlight { false }

			, Crew_TechnicianChance { }
			, Crew_EngineerChance { }
			, Saboteur { false }

			, RadialIndicatorRadius { }
			, RadialIndicatorColor { }

			, GapRadiusInCells { 0 }
			, SuperGapRadiusInCells { 0 }
			, SmokeChanceRed { 10 }
			, SmokeChanceDead { 80 }
			, SmokeAnim { nullptr }
			, CarryallAllowed { }
			, CarryallSizeLimit { }
			, VoiceAirstrikeAttack { }
			, VoiceAirstrikeAbort { }

			, HunterSeekerDetonateProximity { }
			, HunterSeekerDescendProximity { }
			, HunterSeekerAscentSpeed { }
			, HunterSeekerDescentSpeed { }
			, HunterSeekerEmergeSpeed { }

			, CanPassiveAcquire_Guard { true }
			, CanPassiveAcquire_Cloak { true }
			, CrashSpin { true }
			, AirRate { 0 }
			, Unsellable { }
			, CreateSound_afect { }
			, Chronoshift_Allow { true }
			, Chronoshift_IsVehicle { false }
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