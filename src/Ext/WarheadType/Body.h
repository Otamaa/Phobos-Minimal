#pragma once
#include <WarheadTypeClass.h>
#include <CoordStruct.h>

#include <Ext/AbstractType/Body.h>

#include <Utilities/PhobosMap.h>

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <New/AnonymousType/BlockTypeClass.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/CrateTypeClass.h>

#include <New/Entity/LauchSWData.h>
#include <New/PhobosAttachedAffect/AEAttachInfoTypeClass.h>

#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>

#include <Utilities/VersesData.h>

typedef std::vector<std::tuple< std::vector<int>, std::vector<int>, TransactValueType>> TransactData;

struct args_ReceiveDamage;
class ArmorTypeClass;
class IonBlastClass;
class WarheadTypeExtData final : public AbstractTypeExtData
{
public:
	using base_type = WarheadTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region classmembers

	Valueable<int> Reveal;
	Valueable<bool> BigGap;
	Valueable<int> CreateGap;
	Valueable<int> TransactMoney;
	Nullable<int> TransactMoney_Ally;
	Nullable<int> TransactMoney_Enemy;
	Valueable<bool> Transact_AffectsEnemies;
	Valueable<bool> Transact_AffectsAlly;
	Valueable<bool> Transact_AffectsOwner;
	Valueable<bool> TransactMoney_Display;
	Valueable<AffectedHouse> TransactMoney_Display_Houses;
	Valueable<bool> TransactMoney_Display_AtFirer;
	Valueable<Point2D> TransactMoney_Display_Offset;

	Valueable<int> StealMoney;
	Valueable<AffectedHouse> Steal_Display_Houses;
	Valueable<bool> Steal_Display;
	Valueable<Point2D> Steal_Display_Offset;

	NullableVector<AnimTypeClass*> SplashList;
	Valueable<bool> SplashList_PickRandom;
	Valueable<bool> SplashList_CreateAll;
	Valueable<int> SplashList_CreationInterval;
	Valueable<Leptons> SplashList_ScatterMin;
	Valueable<Leptons> SplashList_ScatterMax;

	Valueable<bool> RemoveDisguise;
	Valueable<bool> RemoveMindControl;
	Nullable<bool> AnimList_PickRandom;
	Valueable<bool> AnimList_CreateAll;
	Valueable<int> AnimList_CreationInterval;
	Valueable<Leptons> AnimList_ScatterMin;
	Valueable<Leptons> AnimList_ScatterMax;

	Valueable<bool> AnimList_ShowOnZeroDamage;
	Valueable<bool> DecloakDamagedTargets;
	Valueable<bool> ShakeIsLocal;
	Valueable<bool> Shake_UseAlternativeCalculation;

	Valueable<double> Crit_Chance;
	Valueable<bool> Crit_ApplyChancePerTarget;
	Valueable<int> Crit_ExtraDamage;
	Valueable<bool> Crit_ExtraDamage_ApplyFirepowerMult;
	Valueable<WarheadTypeClass*> Crit_Warhead;
	Valueable<bool> Crit_Warhead_FullDetonation;
	Valueable<AffectedTarget> Crit_Affects;
	Valueable<AffectedHouse> Crit_AffectsHouses;
	ValueableVector<AnimTypeClass*> Crit_AnimList;
	Nullable<bool> Crit_AnimList_PickRandom;
	Nullable<bool> Crit_AnimList_CreateAll;
	ValueableVector<AnimTypeClass*> Crit_ActiveChanceAnims;
	Valueable<bool> Crit_AnimOnAffectedTargets;
	Valueable<double> Crit_AffectBelowPercent;
	Valueable<double> Crit_AffectAbovePercent;
	Valueable<bool> Crit_SuppressWhenIntercepted;

	bool CritActive;
	double CritRandomBuffer;
	double CritCurrentChance;

	Nullable<AnimTypeClass*> MindControl_Anim;

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	Valueable<bool> AffectsEnemies;
	Nullable<bool> AffectsOwner;
	Valueable<bool> EffectsRequireDamage;
	Valueable<bool> EffectsRequireVerses;
	Valueable<bool> AllowZeroDamage;

	Valueable<bool> Shield_Penetrate;
	Valueable<bool> Shield_Break;
	Valueable<AnimTypeClass*> Shield_BreakAnim;
	Valueable<AnimTypeClass*> Shield_HitAnim;
	Nullable<WeaponTypeClass*> Shield_BreakWeapon;

	Nullable<double> Shield_AbsorbPercent;
	Nullable<double> Shield_PassPercent;
	Nullable<int> Shield_ReceivedDamage_Minimum;
	Nullable<int> Shield_ReceivedDamage_Maximum;
	Valueable<double> Shield_ReceivedDamage_MinMultiplier;
	Valueable<double> Shield_ReceivedDamage_MaxMultiplier;

	Valueable<int> Shield_Respawn_Duration;
	Nullable<double> Shield_Respawn_Amount;
	Valueable<int> Shield_Respawn_Rate;
	Nullable<bool> Shield_Respawn_RestartInCombat;
	Valueable<int> Shield_Respawn_RestartInCombatDelay;

private:
	Valueable<double> Shield_Respawn_Rate_InMinutes;
	Valueable<double> Shield_SelfHealing_Rate_InMinutes;

public:

	Valueable<bool> Shield_Respawn_RestartTimer;
	ValueableVector<AnimTypeClass*> Shield_Respawn_Anim;
	Valueable<WeaponTypeClass*> Shield_Respawn_Weapon;
	Valueable<int> Shield_SelfHealing_Duration;
	Nullable<double> Shield_SelfHealing_Amount;
	Valueable<int> Shield_SelfHealing_Rate;
	Nullable<bool> Shield_SelfHealing_RestartInCombat;
	Valueable<int> Shield_SelfHealing_RestartInCombatDelay;
	Valueable<bool> Shield_SelfHealing_RestartTimer;

	ValueableVector<ShieldTypeClass*> Shield_AttachTypes;
	ValueableVector<ShieldTypeClass*> Shield_RemoveTypes;

	Valueable<bool> Shield_ReplaceOnly;
	Valueable<bool> Shield_ReplaceNonRespawning;
	Valueable<bool> Shield_InheritStateOnReplace;
	Valueable<int> Shield_MinimumReplaceDelay;
	ValueableVector<ShieldTypeClass*> Shield_AffectTypes;

	NullableVector<ShieldTypeClass*> Shield_Penetrate_Types;
	ValueableVector<ShieldTypeClass*> Shield_Penetrate_Types_Disallowed_Types;
	ValueableVector<ArmorTypeClass*> Shield_Penetrate_Armor_Types;
	NullableVector<ShieldTypeClass*> Shield_Break_Types;
	NullableVector<ShieldTypeClass*> Shield_Respawn_Types;
	NullableVector<ShieldTypeClass*> Shield_SelfHealing_Types;

	Valueable<bool> Transact;
	Valueable<int> Transact_Experience_Value;
	Valueable<int> Transact_Experience_Source_Flat;
	Valueable<double> Transact_Experience_Source_Percent;
	Valueable<bool> Transact_Experience_Source_Percent_CalcFromTarget;
	Valueable<int> Transact_Experience_Target_Flat;
	Valueable<double> Transact_Experience_Target_Percent;
	Valueable<bool> Transact_Experience_Target_Percent_CalcFromSource;
	Valueable<bool> Transact_SpreadAmongTargets;
	Valueable<bool> Transact_Experience_IgnoreNotTrainable;

	Nullable<int> NotHuman_DeathSequence;
	Valueable<bool> AllowDamageOnSelf;
	Valueable<bool> Debris_Conventional;
	Nullable<bool> DebrisTypes_Limit;
	ValueableVector<int> DebrisMinimums;

	Valueable<int> GattlingStage;
	Valueable<int> GattlingRateUp;
	Valueable<int> ReloadAmmo;

	Valueable<bool> MindControl_UseTreshold;
	Valueable<double> MindControl_Threshold;
	Valueable<bool> MindControl_Threshold_Inverse;
	Nullable<int> MindControl_AlternateDamage;
	Nullable<WarheadTypeClass*> MindControl_AlternateWarhead;
	Valueable<bool> MindControl_CanKill;

	Valueable<bool> DetonateOnAllMapObjects;
	Valueable<bool> DetonateOnAllMapObjects_Full;
	Valueable<bool> DetonateOnAllMapObjects_RequireVerses;
	Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets;
	Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses;
	ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes;
	ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes;

	Valueable<WeaponTypeClass*> RevengeWeapon;
	Valueable<int> RevengeWeapon_GrantDuration;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<bool> RevengeWeapon_Cumulative;
	Valueable<int> RevengeWeapon_MaxCount;

	bool WasDetonatedOnAllMapObjects;
	bool Splashed;
	int RemainingAnimCreationInterval;

	Nullable<AnimTypeClass*> NotHuman_DeathAnim;

	Valueable<bool> IsNukeWarhead;
	Valueable<AnimTypeClass*> PreImpactAnim;
	Valueable<int> NukeFlashDuration;

	Valueable<bool> Remover;
	Valueable<AnimTypeClass*> Remover_Anim;

	PhobosMap<ArmorTypeClass*, AnimTypeClass*> ArmorHitAnim;

	NullableVector<AnimTypeClass*> DebrisAnimTypes;
	NullableVector<AnimTypeClass*> SquidSplash;

	Valueable<AnimTypeClass*> TemporalExpiredAnim;
	Valueable<bool> TemporalExpiredApplyDamage;
	Valueable<double> TemporalDetachDamageFactor;

	Valueable<bool> Parasite_DisableRocking;
	Nullable<AnimTypeClass*> Parasite_GrappleAnim;
	Nullable<ParticleSystemTypeClass*> Parasite_ParticleSys;
	Nullable<bool> Parasite_TreatInfantryAsVehicle;
	Nullable<WeaponTypeClass*> Parasite_InvestationWP;
	Nullable<double> Parasite_Damaging_Chance;

	Nullable<int> Flammability;

	std::vector<LauchSWData> Launchs;

	Valueable<bool> PermaMC;
	ValueableIdx<VocClass> Sound;

	std::vector<TechnoTypeConvertData> ConvertsPair;
	Valueable<AnimTypeClass*> Convert_SucceededAnim;

	Nullable<double> AffectEnemies_Damage_Mod;
	Nullable<double> AffectOwner_Damage_Mod;
	Nullable<double> AffectAlly_Damage_Mod;

	Nullable<double> DamageOwnerMultiplier;
	Nullable<double> DamageAlliesMultiplier;
	Nullable<double> DamageEnemiesMultiplier;
	Nullable<double> DamageOwnerMultiplier_Berzerk;
	Nullable<double> DamageAlliesMultiplier_Berzerk;
	Nullable<double> DamageEnemiesMultiplier_Berzerk;

	PhobosFixedString<32U> AttachTag;
	Valueable<bool> AttachTag_Imposed;
	NullableVector<TechnoTypeClass*> AttachTag_Types;
	NullableVector<TechnoTypeClass*> AttachTag_Ignore;

	Valueable<bool> RecalculateDistanceDamage;
	Valueable<bool> RecalculateDistanceDamage_IgnoreMaxDamage;
	Valueable<int> RecalculateDistanceDamage_Add;
	Valueable<double> RecalculateDistanceDamage_Add_Factor;
	Valueable<double> RecalculateDistanceDamage_Multiply;
	Valueable<double> RecalculateDistanceDamage_Multiply_Factor;
	Valueable<int> RecalculateDistanceDamage_Max;
	Valueable<int> RecalculateDistanceDamage_Min;
	Valueable<bool> RecalculateDistanceDamage_Display;
	Valueable<bool> RecalculateDistanceDamage_Display_AtFirer;
	Valueable<Point2D> RecalculateDistanceDamage_Display_Offset;
	Valueable<bool> RecalculateDistanceDamage_ProcessVerses;

	AresAttachEffectTypeClass AttachedEffect;
	ValueableVector<WeaponTypeClass*> DetonatesWeapons;
	ValueableVector<int> LimboKill_IDs;
	Valueable<AffectedHouse> LimboKill_Affected;

	Valueable<AnimTypeClass*> InfDeathAnim;

	Promotable<int> Culling_BelowHP;
	Promotable<int> Culling_Chance;
	Nullable<AffectedTarget> Culling_Target;

	Valueable<bool> RelativeDamage;
	Valueable<int> RelativeDamage_AirCraft;
	Valueable<int> RelativeDamage_Unit;
	Valueable<int> RelativeDamage_Infantry;
	Valueable<int> RelativeDamage_Building;
	Valueable<int> RelativeDamage_Terrain;

	std::vector<VersesData> Verses;

	Nullable<int> Berzerk_dur;
	Valueable<int> Berzerk_cap;
	Valueable<bool> Berzerk_dealDamage;

	Nullable<bool> IC_Flash;
	Valueable<bool> PreventScatter;
	NullableIdx<VocClass> DieSound_Override;
	NullableIdx<VocClass> VoiceSound_Override;

	Valueable<bool> SuppressDeathWeapon_Vehicles;
	Valueable<bool> SuppressDeathWeapon_Infantry;
	ValueableVector<TechnoTypeClass*> SuppressDeathWeapon;
	ValueableVector<TechnoTypeClass*> SuppressDeathWeapon_Exclude;
	Nullable<double> SuppressDeathWeapon_Chance;
	Promotable<double> DeployedDamage;

	Nullable<AnimTypeClass*> Temporal_WarpAway;
	Valueable<bool> Supress_LostEva;
	Valueable<double> Temporal_HealthFactor;

	PhobosMap<InfantryTypeClass*, AnimTypeClass*> InfDeathAnims;

	Valueable<int> Sonar_Duration;
	Valueable<int> DisableWeapons_Duration;
	Nullable<int> Flash_Duration;

	NullableIdx<ImmunityTypeClass> ImmunityType;

	Valueable<bool> Malicious;
	Valueable<bool> PreImpact_Moves;
	Valueable<bool> Conventional_IgnoreUnits;

	Valueable<bool> InflictLocomotor;
	Valueable<bool> RemoveInflictedLocomotor;

	Nullable<int> Rocker_AmplitudeOverride;
	Valueable<double> Rocker_AmplitudeMultiplier;

	Nullable<int> PaintBallDuration;
	PaintballType PaintBallData;

	ValueableIdx<SuperWeaponTypeClass> NukePayload_LinkedSW;
	Valueable<int> IC_Duration;
	Valueable<int> IC_Cap;

#pragma region Ion
	Valueable<bool> Ion;
	Nullable<int> Ripple_Radius;
	Nullable<AnimTypeClass*> Ion_Beam;
	Nullable<AnimTypeClass*> Ion_Blast;
	Valueable<bool> Ion_AllowWater;
	Valueable<bool> Ion_Rocking;
	Nullable<WarheadTypeClass*> Ion_WH;
	Nullable<int> Ion_Damage;
#pragma endregion

	ValueableVector<ParticleSystemTypeClass*> DetonateParticleSystem;
	Nullable<bool> BridgeAbsoluteDestroyer;
	Valueable<int> DamageAirThreshold;
	Valueable<int> CellSpread_MaxAffect;

	Valueable<int> EMP_Duration;
	Valueable<int> EMP_Cap;
	Valueable<AnimTypeClass*> EMP_Sparkles;

	Nullable<bool> CanRemoveParasytes;
	Valueable<bool> CanRemoveParasytes_KickOut;
	Valueable<int> CanRemoveParasytes_KickOut_Paralysis;
	NullableIdx<VocClass> CanRemoveParasytes_ReportSound;
	Nullable<AnimTypeClass*> CanRemoveParasytes_KickOut_Anim;

	Valueable<bool> Webby;
	ValueableVector<AnimTypeClass*> Webby_Anims;
	Valueable<int> Webby_Duration;
	Valueable<int> Webby_Cap;
	Valueable<int> Webby_Duration_Variation;

	NullablePromotable<int> SelfHealing_CombatDelay;

	Valueable<bool> KillDriver;
	Valueable<double> KillDriver_KillBelowPercent;
	Valueable<OwnerHouseKind> KillDriver_Owner;
	Valueable<bool> KillDriver_ResetVeterancy;
	Valueable<double> KillDriver_Chance;

	Valueable<bool> ApplyModifiersOnNegativeDamage;

	Nullable<int> CombatLightDetailLevel;
	Nullable<double> CombatLightChance;
	Nullable<bool> Particle_AlphaImageIsLightFlash;

	Valueable<bool> Nonprovocative;

	std::vector<int> SpawnsCrate_Types;
	std::vector<int> SpawnsCrate_Weights;

	AEAttachInfoTypeClass PhobosAttachEffects;

	Valueable<bool> Shield_HitFlash;
	Valueable<bool> Shield_SkipHitAnim;
	Nullable<bool> CombatAlert_Suppress;

	Valueable<bool> AffectsOnFloor;
	Valueable<bool> AffectsInAir;
	Valueable<bool> CellSpread_Cylinder;

	Valueable<bool> PenetratesIronCurtain;
	Nullable<bool> PenetratesForceShield;
	Valueable<bool> Shield_RemoveAll;
	Valueable<bool> SuppressRevengeWeapons;
	ValueableVector<WeaponTypeClass*> SuppressRevengeWeapons_Types;
	Valueable<bool> SuppressReflectDamage;
	ValueableVector<PhobosAttachEffectTypeClass*> SuppressReflectDamage_Types;

	ValueableVector<std::string> SuppressReflectDamage_Groups;

	Nullable<bool> RemoveParasites;

	bool Reflected;
	Valueable<bool> CLIsBlack;
	Valueable<bool> ApplyMindamage;
	Valueable<int> MinDamage;

	TechnoClass* IntendedTarget;

	Valueable<WeaponTypeClass*> KillWeapon;
	Valueable<WeaponTypeClass*> KillWeapon_OnFirer;
	Valueable<AffectedHouse> KillWeapon_AffectsHouses;
	Valueable<AffectedHouse> KillWeapon_OnFirer_AffectsHouses;
	Valueable<AffectedTarget> KillWeapon_Affects;
	Valueable<AffectedTarget> KillWeapon_OnFirer_Affects;

	Nullable<int> MindControl_ThreatDelay;

	Nullable<bool> MergeBuildingDamage;

	Valueable<bool> BuildingSell;
	Valueable<bool> BuildingSell_IgnoreUnsellable;
	Valueable<bool> BuildingUndeploy;
	Valueable<bool> BuildingUndeploy_Leave;

	Valueable<double> ScorchChance;
	Valueable<double> ScorchPercentAtMax;
	Valueable<double> CraterChance;
	Valueable<double> CraterPercentAtMax;
	Valueable<double> CellAnimChance;
	Valueable<double> CellAnimPercentAtMax;

	NullableVector<AnimTypeClass*> CellAnim;
	Valueable<int> ElectricAssaultLevel;
	Valueable<AffectedTarget> AirstrikeTargets;

	Valueable<bool> CanKill;
	Valueable<bool> ElectricAssault_Requireverses;

	Valueable<double> DamageSourceHealthMultiplier;
	Valueable<double> DamageTargetHealthMultiplier;

	Valueable<double> AffectsBelowPercent;
	Valueable<double> AffectsAbovePercent;
	Valueable<bool> AffectsNeutral;

	Valueable<int> PenetratesTransport_Level;
	Valueable<double> PenetratesTransport_PassThrough;
	Valueable<double> PenetratesTransport_FatalRate;
	Valueable<double> PenetratesTransport_DamageMultiplier;
	Valueable<bool> PenetratesTransport_DamageAll;
	ValueableIdx<VocClass> PenetratesTransport_CleanSound;

	Valueable<bool> FakeEngineer_CanRepairBridges;
	Valueable<bool> FakeEngineer_CanDestroyBridges;
	Valueable<bool> FakeEngineer_CanCaptureBuildings;
	Valueable<bool> FakeEngineer_BombDisarm;
	Valueable<bool> ReverseEngineer;

	Valueable<bool> UnlimboDetonate;
	Valueable<bool> UnlimboDetonate_Force;
	Valueable<bool> UnlimboDetonate_KeepTarget;
	Valueable<bool> UnlimboDetonate_KeepSelected;

	std::unique_ptr<BlockTypeClass> BlockType;
	Valueable<bool> Block_BasedOnWarhead;
	Valueable<bool> Block_AllowOverride;
	Valueable<bool> Block_IgnoreChanceModifier;
	Valueable<double> Block_ChanceMultiplier;
	Valueable<double> Block_ExtraChance;
	Valueable<bool> ImmuneToBlock;

	Valueable<bool> AffectsUnderground;
	Valueable<bool> PlayAnimUnderground;
	Valueable<bool> PlayAnimAboveSurface;

	Nullable<bool> AnimZAdjust;

	bool IsCellSpreadWH;
	bool IsFakeEngineer;
#pragma endregion

public:
	WarheadTypeExtData(WarheadTypeClass* pObj) : AbstractTypeExtData(pObj),
		Reveal(0),
		BigGap(false),
		CreateGap(0),
		TransactMoney(0),
		TransactMoney_Ally(),
		TransactMoney_Enemy(),
		Transact_AffectsEnemies(false),
		Transact_AffectsAlly(false),
		Transact_AffectsOwner(true),
		TransactMoney_Display(false),
		TransactMoney_Display_Houses(AffectedHouse::All),
		TransactMoney_Display_AtFirer(false),
		TransactMoney_Display_Offset({ 0, 0 }),
		StealMoney(0),
		Steal_Display_Houses(AffectedHouse::All),
		Steal_Display(false),
		Steal_Display_Offset({ 0, 0 }),
		SplashList(),
		SplashList_PickRandom(false),
		SplashList_CreateAll(false),
		SplashList_CreationInterval(0),
		SplashList_ScatterMin(),
		SplashList_ScatterMax(),
		RemoveDisguise(false),
		RemoveMindControl(false),
		AnimList_PickRandom(),
		AnimList_CreateAll(false),
		AnimList_CreationInterval(0),
		AnimList_ScatterMin(),
		AnimList_ScatterMax(),
		AnimList_ShowOnZeroDamage(false),
		DecloakDamagedTargets(true),
		ShakeIsLocal(false),
		Shake_UseAlternativeCalculation(false),
		Crit_Chance(0.0),
		Crit_ApplyChancePerTarget(false),
		Crit_ExtraDamage(0),
		Crit_ExtraDamage_ApplyFirepowerMult(false),
		Crit_Warhead(nullptr),
		Crit_Warhead_FullDetonation(true),
		Crit_Affects(AffectedTarget::All),
		Crit_AffectsHouses(AffectedHouse::All),
		Crit_AnimList(),
		Crit_AnimList_PickRandom(),
		Crit_AnimList_CreateAll(),
		Crit_ActiveChanceAnims(),
		Crit_AnimOnAffectedTargets(false),
		Crit_AffectBelowPercent(1.0),
		Crit_AffectAbovePercent(0.0),
		Crit_SuppressWhenIntercepted(false),
		CritActive(false),
		CritRandomBuffer(0.0),
		CritCurrentChance(0.0),
		MindControl_Anim(),
		AffectsEnemies(true),
		AffectsOwner(),
		EffectsRequireDamage(false),
		EffectsRequireVerses(true),
		AllowZeroDamage(false),
		Shield_Penetrate(false),
		Shield_Break(false),
		Shield_BreakAnim(nullptr),
		Shield_HitAnim(nullptr),
		Shield_BreakWeapon(),
		Shield_AbsorbPercent(),
		Shield_PassPercent(),
		Shield_ReceivedDamage_Minimum(),
		Shield_ReceivedDamage_Maximum(),
		Shield_ReceivedDamage_MinMultiplier(1.0),
		Shield_ReceivedDamage_MaxMultiplier(1.0),
		Shield_Respawn_Duration(0),
		Shield_Respawn_Amount(),
		Shield_Respawn_Rate(-1),
		Shield_Respawn_RestartInCombat(),
		Shield_Respawn_RestartInCombatDelay(-1),
		Shield_Respawn_Rate_InMinutes(-1.0),
		Shield_SelfHealing_Rate_InMinutes(-1.0),
		Shield_Respawn_RestartTimer(false),
		Shield_Respawn_Anim(),
		Shield_Respawn_Weapon(nullptr),
		Shield_SelfHealing_Duration(0),
		Shield_SelfHealing_Amount(),
		Shield_SelfHealing_Rate(-1),
		Shield_SelfHealing_RestartInCombat(),
		Shield_SelfHealing_RestartInCombatDelay(-1),
		Shield_SelfHealing_RestartTimer(false),
		Shield_AttachTypes(),
		Shield_RemoveTypes(),
		Shield_ReplaceOnly(false),
		Shield_ReplaceNonRespawning(false),
		Shield_InheritStateOnReplace(false),
		Shield_MinimumReplaceDelay(0),
		Shield_AffectTypes(),
		Shield_Penetrate_Types(),
		Shield_Penetrate_Types_Disallowed_Types(),
		Shield_Penetrate_Armor_Types(),
		Shield_Break_Types(),
		Shield_Respawn_Types(),
		Shield_SelfHealing_Types(),
		Transact(false),
		Transact_Experience_Value(1),
		Transact_Experience_Source_Flat(0),
		Transact_Experience_Source_Percent(0.0),
		Transact_Experience_Source_Percent_CalcFromTarget(false),
		Transact_Experience_Target_Flat(0),
		Transact_Experience_Target_Percent(0.0),
		Transact_Experience_Target_Percent_CalcFromSource(false),
		Transact_SpreadAmongTargets(false),
		Transact_Experience_IgnoreNotTrainable(true),
		NotHuman_DeathSequence(),
		AllowDamageOnSelf(false),
		Debris_Conventional(false),
		DebrisTypes_Limit(),
		DebrisMinimums(),
		GattlingStage(0),
		GattlingRateUp(0),
		ReloadAmmo(0),
		MindControl_UseTreshold(false),
		MindControl_Threshold(1.0),
		MindControl_Threshold_Inverse(false),
		MindControl_AlternateDamage(),
		MindControl_AlternateWarhead(),
		MindControl_CanKill(false),
		DetonateOnAllMapObjects(false),
		DetonateOnAllMapObjects_Full(true),
		DetonateOnAllMapObjects_RequireVerses(false),
		DetonateOnAllMapObjects_AffectTargets(AffectedTarget::All),
		DetonateOnAllMapObjects_AffectHouses(AffectedHouse::All),
		DetonateOnAllMapObjects_AffectTypes(),
		DetonateOnAllMapObjects_IgnoreTypes(),
		RevengeWeapon(nullptr),
		RevengeWeapon_GrantDuration(0),
		RevengeWeapon_AffectsHouses(AffectedHouse::All),
		RevengeWeapon_Cumulative(false),
		RevengeWeapon_MaxCount(-1),
		WasDetonatedOnAllMapObjects(false),
		Splashed(false),
		RemainingAnimCreationInterval(0),
		NotHuman_DeathAnim(),
		IsNukeWarhead(false),
		PreImpactAnim(nullptr),
		NukeFlashDuration(0),
		Remover(false),
		Remover_Anim(nullptr),
		ArmorHitAnim(),
		DebrisAnimTypes(),
		SquidSplash(),
		TemporalExpiredAnim(nullptr),
		TemporalExpiredApplyDamage(false),
		TemporalDetachDamageFactor(1.0),
		Parasite_DisableRocking(false),
		Parasite_GrappleAnim(),
		Parasite_ParticleSys(),
		Parasite_TreatInfantryAsVehicle(),
		Parasite_InvestationWP(),
		Parasite_Damaging_Chance(),
		Flammability(),
		Launchs(),
		PermaMC(false),
		Sound(-1),
		ConvertsPair(),
		Convert_SucceededAnim(nullptr),
		AffectEnemies_Damage_Mod(),
		AffectOwner_Damage_Mod(),
		AffectAlly_Damage_Mod(),
		DamageOwnerMultiplier(),
		DamageAlliesMultiplier(),
		DamageEnemiesMultiplier(),
		DamageOwnerMultiplier_Berzerk(),
		DamageAlliesMultiplier_Berzerk(),
		DamageEnemiesMultiplier_Berzerk(),
		AttachTag(),
		AttachTag_Imposed(false),
		AttachTag_Types(),
		AttachTag_Ignore(),
		RecalculateDistanceDamage(false),
		RecalculateDistanceDamage_IgnoreMaxDamage(false),
		RecalculateDistanceDamage_Add(0),
		RecalculateDistanceDamage_Add_Factor(1.0),
		RecalculateDistanceDamage_Multiply(1.0),
		RecalculateDistanceDamage_Multiply_Factor(1.0),
		RecalculateDistanceDamage_Max(INT_MAX),
		RecalculateDistanceDamage_Min(-INT_MAX),
		RecalculateDistanceDamage_Display(false),
		RecalculateDistanceDamage_Display_AtFirer(false),
		RecalculateDistanceDamage_Display_Offset(Point2D::Empty),
		RecalculateDistanceDamage_ProcessVerses(false),
		AttachedEffect(),
		DetonatesWeapons(),
		LimboKill_IDs(),
		LimboKill_Affected(AffectedHouse::Owner),
		InfDeathAnim(nullptr),
		Culling_BelowHP(0, -1, -2),
		Culling_Chance(100),
		Culling_Target(),
		RelativeDamage(false),
		RelativeDamage_AirCraft(0),
		RelativeDamage_Unit(0),
		RelativeDamage_Infantry(0),
		RelativeDamage_Building(0),
		RelativeDamage_Terrain(0),
		Verses(),
		Berzerk_dur(),
		Berzerk_cap(-1),
		Berzerk_dealDamage(false),
		IC_Flash(),
		PreventScatter(false),
		DieSound_Override(),
		VoiceSound_Override(),
		SuppressDeathWeapon_Vehicles(false),
		SuppressDeathWeapon_Infantry(false),
		SuppressDeathWeapon(),
		SuppressDeathWeapon_Exclude(),
		SuppressDeathWeapon_Chance(),
		DeployedDamage(1.00),
		Temporal_WarpAway(),
		Supress_LostEva(false),
		Temporal_HealthFactor(0.0),
		InfDeathAnims(),
		Sonar_Duration(0),
		DisableWeapons_Duration(0),
		Flash_Duration(),
		ImmunityType(),
		Malicious(true),
		PreImpact_Moves(false),
		Conventional_IgnoreUnits(false),
		InflictLocomotor(false),
		RemoveInflictedLocomotor(false),
		Rocker_AmplitudeOverride(),
		Rocker_AmplitudeMultiplier(0.01),
		PaintBallDuration(),
		PaintBallData(),
		NukePayload_LinkedSW(-1),
		IC_Duration(0),
		IC_Cap(-1),
		Ion(false),
		Ripple_Radius(),
		Ion_Beam(),
		Ion_Blast(),
		Ion_AllowWater(false),
		Ion_Rocking(true),
		Ion_WH(),
		Ion_Damage(),
		DetonateParticleSystem(),
		BridgeAbsoluteDestroyer(),
		DamageAirThreshold(0),
		CellSpread_MaxAffect(-1),
		EMP_Duration(0),
		EMP_Cap(-1),
		EMP_Sparkles(nullptr),
		CanRemoveParasytes(),
		CanRemoveParasytes_KickOut(false),
		CanRemoveParasytes_KickOut_Paralysis(-1),
		CanRemoveParasytes_ReportSound(),
		CanRemoveParasytes_KickOut_Anim(),
		Webby(false),
		Webby_Anims(),
		Webby_Duration(0),
		Webby_Cap(-1),
		Webby_Duration_Variation(0),
		SelfHealing_CombatDelay(),
		KillDriver(false),
		KillDriver_KillBelowPercent(1.00),
		KillDriver_Owner(OwnerHouseKind::Special),
		KillDriver_ResetVeterancy(false),
		KillDriver_Chance(100.0),
		ApplyModifiersOnNegativeDamage(false),
		CombatLightDetailLevel(),
		CombatLightChance(),
		Particle_AlphaImageIsLightFlash(),
		Nonprovocative(false),
		SpawnsCrate_Types(),
		SpawnsCrate_Weights(),
		PhobosAttachEffects(),
		Shield_HitFlash(true),
		Shield_SkipHitAnim(false),
		CombatAlert_Suppress(),
		AffectsOnFloor(true),
		AffectsInAir(true),
		CellSpread_Cylinder(false),
		PenetratesIronCurtain(false),
		PenetratesForceShield(),
		Shield_RemoveAll(false),
		SuppressRevengeWeapons(false),
		SuppressRevengeWeapons_Types(),
		SuppressReflectDamage(false),
		SuppressReflectDamage_Types(),
		SuppressReflectDamage_Groups(),
		RemoveParasites(),
		Reflected(false),
		CLIsBlack(false),
		ApplyMindamage(false),
		MinDamage(-1),
		IntendedTarget(nullptr),
		KillWeapon(nullptr),
		KillWeapon_OnFirer(nullptr),
		KillWeapon_AffectsHouses(AffectedHouse::All),
		KillWeapon_OnFirer_AffectsHouses(AffectedHouse::All),
		KillWeapon_Affects(AffectedTarget::All),
		KillWeapon_OnFirer_Affects(AffectedTarget::All),
		MindControl_ThreatDelay(),
		MergeBuildingDamage(),
		BuildingSell(false),
		BuildingSell_IgnoreUnsellable(false),
		BuildingUndeploy(false),
		BuildingUndeploy_Leave(false),
		ScorchChance(0.0),
		ScorchPercentAtMax(1.0),
		CraterChance(0.0),
		CraterPercentAtMax(1.0),
		CellAnimChance(0.0),
		CellAnimPercentAtMax(1.0),
		CellAnim(),
		ElectricAssaultLevel(1),
		AirstrikeTargets(AffectedTarget::Building),
		CanKill(true),
		ElectricAssault_Requireverses(false),
		DamageSourceHealthMultiplier(0.0),
		DamageTargetHealthMultiplier(0.0),
		AffectsBelowPercent(1.0),
		AffectsAbovePercent(0.0),
		AffectsNeutral(true),
		PenetratesTransport_Level(0),
		PenetratesTransport_PassThrough(1.0),
		PenetratesTransport_FatalRate(0.0),
		PenetratesTransport_DamageMultiplier(1.0),
		PenetratesTransport_DamageAll(false),
		PenetratesTransport_CleanSound(-1),
		FakeEngineer_CanRepairBridges(false),
		FakeEngineer_CanDestroyBridges(false),
		FakeEngineer_CanCaptureBuildings(false),
		FakeEngineer_BombDisarm(false),
		ReverseEngineer(false),
		UnlimboDetonate(false),
		UnlimboDetonate_Force(false),
		UnlimboDetonate_KeepTarget(true),
		UnlimboDetonate_KeepSelected(true),
		BlockType(),
		Block_BasedOnWarhead(false),
		Block_AllowOverride(true),
		Block_IgnoreChanceModifier(true),
		Block_ChanceMultiplier(1.0),
		Block_ExtraChance(0.0),
		ImmuneToBlock(false),
		AffectsUnderground(false),
		PlayAnimUnderground(true),
		PlayAnimAboveSurface(false),
		AnimZAdjust(),
		IsCellSpreadWH(false),
		IsFakeEngineer(false)
		{
			this->AbsType = WarheadTypeClass::AbsID;
			this->InitializeConstant();
		}

	void Initialize();

	WarheadTypeExtData(WarheadTypeClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~WarheadTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<WarheadTypeExtData*>(this)->AbstractTypeExtData::Internal_SaveToStream(Stm);
		const_cast<WarheadTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual WarheadTypeClass* This() const override { return reinterpret_cast<WarheadTypeClass*>(this->AbstractTypeExtData::This()); }
	virtual const WarheadTypeClass* This_Const() const override { return reinterpret_cast<const WarheadTypeClass*>(this->AbstractTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

public:

	void InitializeConstant();
	void ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget) const;
	HouseClass*  ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget) const;

	bool applyPermaMC(HouseClass* const Owner, AbstractClass* const Target) const;

	void ApplyLocomotorInfliction(TechnoClass* pTarget) const;
	void ApplyLocomotorInflictionReset(TechnoClass* pTarget) const;

	void applyIronCurtain(TechnoClass* items, HouseClass* Owner, int damage) const;
	void applyIronCurtain(const CoordStruct& coords, HouseClass* pOwner, int damage) const;

	void ApplyPenetratesTransport(TechnoClass* pTarget, TechnoClass* pInvoker, HouseClass* pInvokerHouse, const CoordStruct& coords, int damage) const;

	void EvaluateArmor(WarheadTypeClass* OwnerObject);
	void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, const CoordStruct& coords, int damage, TechnoClass* pOwner = nullptr, BulletClass* pBullet = nullptr, bool bulletWasIntercepted = false);

	void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
	void ApplyShieldModifiers(TechnoClass* pTarget);

	void ApplyGattlingStage(TechnoClass* pTarget, int Stage) const;
	void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp) const;
	void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount) const;

	void ApplyAttachTag(TechnoClass* pTarget) const;
	void ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget) const;

	void applyWebby(TechnoClass* pTarget, HouseClass* pKillerHouse, TechnoClass* pKillerTech) const;

	//Otamaa
	void applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords) const;
	void applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target) const;

	//void DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner);
	void TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);
	void TransactOnAllUnits(std::vector<TechnoClass*>& nVec, HouseClass* pHouse, TechnoClass* pOwner);
	int TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget);
	TransactData TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets);
	int TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType);

	void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords, int damage);
	bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno) const;
	void InterceptBullets(TechnoClass* pOwner, BulletClass* pBullet, CoordStruct coords);
	bool CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse) const;
	bool CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage = false) const;
	bool CanDealDamage(TechnoClass* pTechno, bool Bypass = false, bool SkipVerses = false , bool checkImmune = true , bool checkLimbo = true) const;
	bool CanAffectInvulnerable(TechnoClass* pTarget) const;
	FullMapDetonateResult EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner) const;
	void ApplyDamageMult(TechnoClass* pVictim, args_ReceiveDamage* pArgs) const;
	void ApplyRecalculateDistanceDamage(ObjectClass* pVictim, args_ReceiveDamage* pArgs) const;
	void ApplyRevengeWeapon(TechnoClass* pTarget) const;
	bool applyCulling(TechnoClass* pSource, ObjectClass* pTarget) const;
	void applyRelativeDamage(ObjectClass* pTarget, args_ReceiveDamage* pArgs) const;
	bool GoBerzerkFor(FootClass* pVictim, int* damage) const;
	bool ApplySuppressDeathWeapon(TechnoClass* pVictim) const;
	void ApplyBuildingUndeploy(TechnoClass* pTarget);
	void ApplyAttachEffects(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker);
	void GetCritChance(TechnoClass* pFirer , double& chances) const;

	COMPILETIMEEVAL VersesData& GetVerses(Armor armor) {
		return this->Verses[static_cast<int>(armor)];
	}

	COMPILETIMEEVAL const VersesData& GetVerses(Armor armor) const {
		return this->Verses[static_cast<int>(armor)];
	}


	bool IsHealthInThreshold(ObjectClass* pTarget) const;

	AnimTypeClass* GetArmorHitAnim(int Armor);

	DamageAreaResult DamageAreaWithTarget(CoordStruct coords, int damage, TechnoClass* pSource, WarheadTypeClass* pWH, bool affectsTiberium, HouseClass* pSourceHouse, TechnoClass* pTarget);

	static AnimTypeClass* __fastcall SelectCombatAnim(int damage, WarheadTypeClass* warhead, LandType land, CoordStruct& coord);

private:

	template <typename T>
	void Serialize(T& Stm);

public:
	static PhobosMap<IonBlastClass*, WarheadTypeExtData*> IonBlastExt;

	static void DetonateAt(
		WarheadTypeClass* pThis,
		ObjectClass* pTarget,
		TechnoClass* pOwner,
		int damage,
		bool targetCell = false,
		HouseClass* pFiringHouse = nullptr
	);

	static void DetonateAt(
		WarheadTypeClass* pThis,
		const CoordStruct coords, //do make copy
		TechnoClass* pOwner,
		int damage,
		bool targetCell = false,
		HouseClass* pFiringHouse = nullptr
	);

	static void DetonateAt(
		WarheadTypeClass* pThis,
		AbstractClass* pTarget,
		const CoordStruct coords,//do make copy
		TechnoClass* pOwner,
		int damage,
		HouseClass* pFiringHouse = nullptr
	);

	static void CreateIonBlast(WarheadTypeClass* pThis, const CoordStruct& coords);
	static void applyEMP(WarheadTypeClass* pWH, const CoordStruct& coords, TechnoClass* source);
	static void DetonateAtBridgeRepairHut(AbstractClass* pTarget, TechnoClass* pOwner = nullptr, HouseClass* pFiringHouse = nullptr, bool destroyBridge = false);
};

class WarheadTypeExtContainer final : public Container<WarheadTypeExtData>
{
public:
	static WarheadTypeExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};

class NOVTABLE FakeWarheadTypeClass : public WarheadTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	static int __fastcall ModifyDamageA(int damage, FakeWarheadTypeClass* pWH, Armor armor, int distance);

	static FORCEDINLINE int ModifyDamage(int damage, WarheadTypeClass* pWH, Armor armor, int distance) {
		return ModifyDamageA(damage, (FakeWarheadTypeClass*)pWH, armor, distance);
	}

	WarheadTypeExtData* _GetExtData() const {
		return *reinterpret_cast<WarheadTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	COMPILETIMEEVAL VersesData* GetVersesData(Armor armor) {
		return this->_GetExtData()->Verses.data() + static_cast<size_t>(armor);
	}

};
static_assert(sizeof(FakeWarheadTypeClass) == sizeof(WarheadTypeClass), "Invalid Size !");