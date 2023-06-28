#pragma once
#include <WarheadTypeClass.h>

#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
#include<Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Entity/LauchSWData.h>

#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>

#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>

#include <New/AnonymousType/AresAttachEffectTypeClass.h>
#include <Utilities/VersesData.h>

typedef std::vector<std::tuple< std::vector<int>, std::vector<int>, TransactValueType>> TransactData;

struct args_ReceiveDamage;
class ArmorTypeClass;
class WarheadTypeExt
{
public:

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:
		static constexpr size_t Canary = 0x22222222;
		using base_type = WarheadTypeClass;

	public:

		Valueable<int> Reveal;
		Valueable<bool> BigGap;
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

		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickRandom;
		Valueable<bool> AnimList_ShowOnZeroDamage;
		Valueable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;

		Valueable<double> Crit_Chance;
		Valueable<bool> Crit_ApplyChancePerTarget;
		Valueable<int> Crit_ExtraDamage;
		Nullable<WarheadTypeClass*> Crit_Warhead;
		Valueable<AffectedTarget> Crit_Affects;
		Valueable<AffectedHouse> Crit_AffectsHouses;
		ValueableVector<AnimTypeClass*> Crit_AnimList;
		Nullable<bool> Crit_AnimList_PickRandom;
		Valueable<bool> Crit_AnimOnAffectedTargets;
		Valueable<double> Crit_AffectBelowPercent;
		Valueable<bool> Crit_SuppressOnIntercept;

		double RandomBuffer;
		bool HasCrit;

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
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<AnimTypeClass*> Shield_HitAnim;
		Nullable<WeaponTypeClass*> Shield_BreakWeapon;

		Nullable<double> Shield_AbsorbPercent;
		Nullable<double> Shield_PassPercent;

		Valueable<int> Shield_Respawn_Duration;
		Valueable<double> Shield_Respawn_Amount;
		Valueable<int> Shield_Respawn_Rate;

	private:
		Valueable<double> Shield_Respawn_Rate_InMinutes;
		Valueable<double> Shield_SelfHealing_Rate_InMinutes;

	public:

		Valueable<bool> Shield_Respawn_ResetTimer;
		Valueable<int> Shield_SelfHealing_Duration;
		Nullable<double> Shield_SelfHealing_Amount;
		Valueable<int> Shield_SelfHealing_Rate;
		Valueable<bool> Shield_SelfHealing_ResetTimer;

		ValueableVector<ShieldTypeClass*> Shield_AttachTypes;
		ValueableVector<ShieldTypeClass*> Shield_RemoveTypes;
		Valueable<bool> Shield_ReplaceOnly;
		Valueable<bool> Shield_ReplaceNonRespawning;
		Valueable<bool> Shield_InheritStateOnReplace;
		Valueable<int> Shield_MinimumReplaceDelay;
		ValueableVector<ShieldTypeClass*> Shield_AffectTypes;

		NullableVector<ShieldTypeClass*> Shield_Penetrate_Types;
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
		Nullable<bool> AllowDamageOnSelf;
		Valueable<bool> Debris_Conventional;
		Valueable<int> GattlingStage;
		Valueable<int> GattlingRateUp;
		Valueable<int> ReloadAmmo;

		Valueable<double> MindControl_Threshold;
		Valueable<bool> MindControl_Threshold_Inverse;
		Nullable<int> MindControl_AlternateDamage;
		Nullable<WarheadTypeClass*> MindControl_AlternateWarhead;
		Valueable<bool> MindControl_CanKill;

		Valueable<bool> DetonateOnAllMapObjects;
		Valueable<bool> DetonateOnAllMapObjects_RequireVerses;
		Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets;
		Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes;

		Nullable<WeaponTypeClass*> RevengeWeapon;
		Valueable<int> RevengeWeapon_GrantDuration;
		Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
		Valueable<bool> RevengeWeapon_Cumulative;
		Valueable<int> RevengeWeapon_MaxCount;

		bool WasDetonatedOnAllMapObjects;

#pragma region Otamaa
		Nullable<AnimTypeClass*> NotHuman_DeathAnim;

		Valueable<bool> IsNukeWarhead;
		Nullable<AnimTypeClass*> PreImpactAnim;
		Nullable<int> NukeFlashDuration;

		Valueable<bool> Remover;
		Valueable<AnimTypeClass*> Remover_Anim;

		std::unordered_map<ArmorTypeClass* , AnimTypeClass*> ArmorHitAnim;

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

		Valueable<bool> Converts;

		ValueableVector<TechnoTypeConvertData> ConvertsPair;
		ValueableVector<AnimTypeClass*> DeadBodies;

		Nullable<double> AffectEnemies_Damage_Mod;
		Nullable<double> AffectOwner_Damage_Mod;
		Nullable<double> AffectAlly_Damage_Mod;

		PhobosFixedString<32U> AttachTag;
		Valueable<bool> AttachTag_Imposed;
		NullableVector<TechnoTypeClass*> AttachTag_Types;
		NullableVector<TechnoTypeClass*> AttachTag_Ignore;

		Valueable<bool> DirectionalArmor;
		Valueable<float> DirectionalArmor_FrontMultiplier;
		Valueable<float> DirectionalArmor_SideMultiplier;
		Valueable<float> DirectionalArmor_BackMultiplier;
		Valueable<float> DirectionalArmor_FrontField;
		Valueable<float> DirectionalArmor_BackField;

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

		std::unordered_map<int , AnimTypeClass*> InfDeathAnims;

		Valueable<int> Sonar_Duration;
		Valueable<int> DisableWeapons_Duration;
		Valueable<int> Flash_Duration;

		NullableIdx<ImmunityTypeClass> ImmunityType;

		Valueable<bool> Malicious;
		Valueable<bool> PreImpact_Moves;
		Valueable<bool> Conventional_IgnoreUnits;

		Valueable<bool> InflictLocomotor;
		Valueable<bool> RemoveInflictedLocomotor;

		Nullable<int> Rocker_Damage;

#ifdef COMPILE_PORTED_DP_FEATURES_
		PhobosMap<int, DamageTextTypeData> DamageTextPerArmor;
#endif

		Valueable<int> PaintBallDuration;
		PaintballType PaintBallData;
#pragma endregion

		ValueableIdx<SuperWeaponTypeClass> NukePayload_LinkedSW;
		Valueable<int> IC_Duration { 0 };
		Valueable<int> IC_Cap { -1 };

#pragma region Ion
		Valueable<bool> Ion{ false };
		Nullable<int> Ripple_Radius {};
		Nullable<AnimTypeClass*> Ion_Beam {};
		Nullable<AnimTypeClass*> Ion_Blast {};
		Valueable<bool> Ion_AllowWater { false };
		Valueable<bool> Ion_Rocking { true };
		Nullable<WarheadTypeClass*> Ion_WH {};
		Nullable<int> Ion_Damage {};
#pragma endregion

		ValueableVector<ParticleSystemTypeClass*> DetonateParticleSystem {};

		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, Reveal { 0 }
			, BigGap { false }
			, TransactMoney { 0 }
			, TransactMoney_Ally { }
			, TransactMoney_Enemy { }
			, Transact_AffectsEnemies { false }
			, Transact_AffectsAlly { false }
			, Transact_AffectsOwner { true }
			, TransactMoney_Display { false }
			, TransactMoney_Display_Houses { AffectedHouse::All }
			, TransactMoney_Display_AtFirer { false }
			, TransactMoney_Display_Offset { { 0, 0 } }

			, StealMoney { 0 }
			, Steal_Display_Houses { AffectedHouse::All }
			, Steal_Display { false }
			, Steal_Display_Offset { { 0, 0 } }

			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickRandom { false }
			, AnimList_ShowOnZeroDamage { false }
			, DecloakDamagedTargets { true }
			, ShakeIsLocal { false }

			, Crit_Chance { 0.0 }
			, Crit_ApplyChancePerTarget { false }
			, Crit_ExtraDamage { 0 }
			, Crit_Warhead {}
			, Crit_Affects { AffectedTarget::All }
			, Crit_AffectsHouses { AffectedHouse::All }
			, Crit_AnimList {}
			, Crit_AnimList_PickRandom {}
			, Crit_AnimOnAffectedTargets { false }
			, Crit_AffectBelowPercent { 1.0 }
			, Crit_SuppressOnIntercept { false }

			, RandomBuffer { 0.0 }
			, HasCrit { false }

			, MindControl_Anim {}

			, AffectsEnemies { true }
			, AffectsOwner {}
			, EffectsRequireDamage { false }
			, EffectsRequireVerses { true }
			, AllowZeroDamage { false }
			, Shield_Penetrate { false }
			, Shield_Break { false }
			, Shield_BreakAnim {}
			, Shield_HitAnim {}
			, Shield_BreakWeapon {}
			, Shield_AbsorbPercent {}
			, Shield_PassPercent {}

			, Shield_Respawn_Duration { 0 }
			, Shield_Respawn_Amount { 0.0 }
			, Shield_Respawn_Rate { -1 }
			, Shield_Respawn_Rate_InMinutes { -1.0 }
			, Shield_SelfHealing_Rate_InMinutes { -1.0 }
			, Shield_Respawn_ResetTimer { false }
			, Shield_SelfHealing_Duration { 0 }
			, Shield_SelfHealing_Amount { }
			, Shield_SelfHealing_Rate { -1 }
			, Shield_SelfHealing_ResetTimer { false }
			, Shield_AttachTypes {}
			, Shield_RemoveTypes {}
			, Shield_ReplaceOnly { false }
			, Shield_ReplaceNonRespawning { false }
			, Shield_InheritStateOnReplace { false }
			, Shield_MinimumReplaceDelay { 0 }
			, Shield_AffectTypes {}

			, Shield_Penetrate_Types {}
			, Shield_Break_Types {}
			, Shield_Respawn_Types {}
			, Shield_SelfHealing_Types {}

			, Transact { false }
			, Transact_Experience_Value { 1 }
			, Transact_Experience_Source_Flat { 0 }
			, Transact_Experience_Source_Percent { 0.0 }
			, Transact_Experience_Source_Percent_CalcFromTarget { false }
			, Transact_Experience_Target_Flat { 0 }
			, Transact_Experience_Target_Percent { 0.0 }
			, Transact_Experience_Target_Percent_CalcFromSource { false }
			, Transact_SpreadAmongTargets { false }
			, Transact_Experience_IgnoreNotTrainable { true }

			, NotHuman_DeathSequence {}
			, AllowDamageOnSelf { }
			, Debris_Conventional { false }
			, GattlingStage { 0 }
			, GattlingRateUp { 0 }
			, ReloadAmmo { 0 }

			, MindControl_Threshold { 1.0 }
			, MindControl_Threshold_Inverse { false }
			, MindControl_AlternateDamage {}
			, MindControl_AlternateWarhead {}
			, MindControl_CanKill { false }

			, DetonateOnAllMapObjects { false }
			, DetonateOnAllMapObjects_RequireVerses { false }
			, DetonateOnAllMapObjects_AffectTargets { AffectedTarget::All }
			, DetonateOnAllMapObjects_AffectHouses { AffectedHouse::All }
			, DetonateOnAllMapObjects_AffectTypes {}
			, DetonateOnAllMapObjects_IgnoreTypes {}

			, RevengeWeapon {}
			, RevengeWeapon_GrantDuration { 0 }
			, RevengeWeapon_AffectsHouses { AffectedHouse::All }
			, RevengeWeapon_Cumulative { false }
			, RevengeWeapon_MaxCount { -1 }

			, WasDetonatedOnAllMapObjects { false }

			, NotHuman_DeathAnim { }

			, IsNukeWarhead { false }
			, PreImpactAnim {}
			, NukeFlashDuration { }

			, Remover { false }
			, Remover_Anim { nullptr }
			, ArmorHitAnim { }
			, DebrisAnimTypes {}
			, SquidSplash {}
			, TemporalExpiredAnim { nullptr }
			, TemporalExpiredApplyDamage { false }
			, TemporalDetachDamageFactor { 1.0 }
			, Parasite_DisableRocking {}
			, Parasite_GrappleAnim {}
			, Parasite_ParticleSys {}
			, Parasite_TreatInfantryAsVehicle { }
			, Parasite_InvestationWP {}
			, Parasite_Damaging_Chance {}

			, Flammability {}

			, Launchs {}
			, PermaMC { false }
			, Sound { -1 }

			, Converts { false }
			, ConvertsPair {}
			, DeadBodies {}
			, AffectEnemies_Damage_Mod {}
			, AffectOwner_Damage_Mod {}
			, AffectAlly_Damage_Mod {}

			, AttachTag { nullptr }
			, AttachTag_Imposed { false }
			, AttachTag_Types {}
			, AttachTag_Ignore {}

			, DirectionalArmor { false }
			, DirectionalArmor_FrontMultiplier { 1.0 }
			, DirectionalArmor_SideMultiplier { 1.0 }
			, DirectionalArmor_BackMultiplier { 1.0 }
			, DirectionalArmor_FrontField { 0.5 }
			, DirectionalArmor_BackField { 0.5 }

			, RecalculateDistanceDamage { false }
			, RecalculateDistanceDamage_IgnoreMaxDamage { false }
			, RecalculateDistanceDamage_Add { 0 }
			, RecalculateDistanceDamage_Add_Factor { 1.0 }
			, RecalculateDistanceDamage_Multiply { 1.0 }
			, RecalculateDistanceDamage_Multiply_Factor { 1.0 }
			, RecalculateDistanceDamage_Max { INT_MAX }
			, RecalculateDistanceDamage_Min { -INT_MAX }
			, RecalculateDistanceDamage_Display { false }
			, RecalculateDistanceDamage_Display_AtFirer { false }
			, RecalculateDistanceDamage_Display_Offset { Point2D::Empty }
			, RecalculateDistanceDamage_ProcessVerses { false }

			, AttachedEffect { OwnerObject }
			, DetonatesWeapons { }
			, LimboKill_IDs { }
			, LimboKill_Affected { AffectedHouse::Owner }
			, InfDeathAnim { nullptr }

			, Culling_BelowHP { 0 , -1 , -2 }
			, Culling_Chance { 100 }
			, RelativeDamage { false }
			, RelativeDamage_AirCraft { 0 }
			, RelativeDamage_Unit { 0 }
			, RelativeDamage_Infantry { 0 }
			, RelativeDamage_Building { 0 }
			, RelativeDamage_Terrain { 0 }

			, Verses { }
			, Berzerk_dur { }
			, Berzerk_cap { -1 }
			, Berzerk_dealDamage { false }

			, IC_Flash { }

			, PreventScatter { false }
			, DieSound_Override { }
			, VoiceSound_Override { }

			, SuppressDeathWeapon_Vehicles { false }
			, SuppressDeathWeapon_Infantry { false }
			, SuppressDeathWeapon { }
			, SuppressDeathWeapon_Exclude { }
			, SuppressDeathWeapon_Chance { }
			, DeployedDamage { 1.00 }
			, Temporal_WarpAway {}
			, Supress_LostEva { false }
			, Temporal_HealthFactor { 1.0 }
			, InfDeathAnims { }
			, Sonar_Duration { 0 }
			, DisableWeapons_Duration { 0 }
			, Flash_Duration { 0 }
			, ImmunityType {}
			, Malicious { true }
			, PreImpact_Moves { false }
			, Conventional_IgnoreUnits { false }

			, InflictLocomotor { false }
			, RemoveInflictedLocomotor { false }
			, Rocker_Damage { }
#ifdef COMPILE_PORTED_DP_FEATURES_
			,DamageTextPerArmor { }

#endif
			, PaintBallDuration { -1 }
			, PaintBallData { }
			, NukePayload_LinkedSW { -1 }

		{	
			this->EvaluateArmor(OwnerObject);
		}

		void ApplyRemoveDisguise(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);

		bool applyPermaMC(HouseClass* const Owner, AbstractClass* const Target);

		void ApplyLocomotorInfliction(TechnoClass* pTarget);
		void ApplyLocomotorInflictionReset(TechnoClass* pTarget);

		void applyIronCurtain(TechnoClass* items, HouseClass* Owner, int damage);

	private:

		void EvaluateArmor(WarheadTypeClass* OwnerObject);
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner = nullptr , BulletClass* pBullet = nullptr, bool bulletWasIntercepted = false);

		void ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* Owner);
		void ApplyShieldModifiers(TechnoClass* pTarget);

		void ApplyGattlingStage(TechnoClass* pTarget, int Stage);
		void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp);
		void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount);

		void ApplyAttachTag(TechnoClass* pTarget);
		void ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget);

		//Otamaa
		void applyTransactMoney(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct const& coords);
		void applyStealMoney(TechnoClass* const Owner, TechnoClass* const Target);

		void ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget);

		//void DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner);
		void TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);
		void TransactOnAllUnits(std::vector<TechnoClass*>& nVec , HouseClass* pHouse, TechnoClass* pOwner);
		int TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget);
		TransactData TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets);
		int TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType);

	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);
		void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords);
		bool CanAffectHouse(HouseClass* pOwnerHouse, HouseClass* pTargetHouse);
		bool CanDealDamage(TechnoClass* pTechno, int damageIn, int distanceFromEpicenter, int& DamageResult, bool effectsRequireDamage = false);
		bool CanDealDamage(TechnoClass* pTechno , bool Bypass = false, bool SkipVerses = false);
		FullMapDetonateResult EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner);
		void ApplyDamageMult(TechnoClass* pVictim, args_ReceiveDamage* pArgs);
		void ApplyRecalculateDistanceDamage(ObjectClass* pVictim, args_ReceiveDamage* pArgs);
		void ApplyRevengeWeapon(TechnoClass* pTarget);
		bool ApplyCulling(TechnoClass* pSource, ObjectClass* pTarget) const;
		void ApplyRelativeDamage(ObjectClass* pTarget, args_ReceiveDamage* pArgs) const;
		bool GoBerzerkFor(FootClass* pVictim, int* damage);
		bool ApplySuppressDeathWeapon(TechnoClass* pVictim);

		VersesData& GetVerses(Armor armor) {
			return this->Verses[static_cast<int>(armor)];
		}

		const VersesData& GetVerses(Armor armor) const {
			return this->Verses[static_cast<int>(armor)];
		}

		virtual ~ExtData() override  = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		virtual void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void Initialize();

		AnimTypeClass* GetArmorHitAnim(int Armor);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();

		static bool LoadGlobals(PhobosStreamReader& Stm);
		static bool SaveGlobals(PhobosStreamWriter& Stm);
		static void Clear();
	};

	static ExtContainer ExtMap;

	static PhobosMap<IonBlastClass*, WarheadTypeExt::ExtData*> IonBlastExt;
	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, bool targetCell = false);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool targetCell = false);
	static void DetonateAt(WarheadTypeClass* pThis, AbstractClass* pTarget, const CoordStruct& coords, TechnoClass* pOwner, int damage);
	static void CreateIonBlast(WarheadTypeClass* pThis , const CoordStruct& coords);
};
