#pragma once
#include <WarheadTypeClass.h>

#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
#include<Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Entity/LauchSWData.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/AttachedAffects/Effects/PaintBall/PaintBall.h>
#endif

#include <New/AnonymousType/AresAttachEffectTypeClass.h>

typedef std::vector<std::tuple< std::vector<int>, std::vector<int>, TransactValueType>> TransactData;

struct args_ReceiveDamage;
class WarheadTypeExt
{
public:
	static constexpr size_t Canary = 0x22222222;
	using base_type = WarheadTypeClass;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		Valueable<bool> SpySat;
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

		Valueable<int> NotHuman_DeathSequence;
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

		Valueable<bool> Remover;
		Valueable<AnimTypeClass*> Remover_Anim;

		PhobosMap<int, AnimTypeClass*> ArmorHitAnim;

		NullableVector<AnimTypeClass*> DebrisAnimTypes;
		NullableVector<AnimTypeClass*> SquidSplash;

		Valueable<AnimTypeClass*> TemporalExpiredAnim;
		Valueable<bool> TemporalExpiredApplyDamage;
		Valueable<double> TemporalDetachDamageFactor;

		Valueable<bool> Parasite_DisableRocking;
		NullableIdx<AnimTypeClass> Parasite_GrappleAnimIndex;
		Nullable<ParticleSystemTypeClass*> Parasite_ParticleSys;
		Nullable<bool> Parasite_TreatInfantryAsVehicle;

		Nullable<int> Flammability;

		std::vector<LauchSWData> Launchs;

		Valueable<bool> PermaMC;
		ValueableIdx<VocClass> Sound;

		Valueable<bool> Converts;
		ValueableVector<TechnoTypeClass*> Converts_From;
		ValueableVector<TechnoTypeClass*> Converts_To;
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

#ifdef COMPILE_PORTED_DP_FEATURES_
		PhobosMap<int, DamageTextTypeData> DamageTextPerArmor;
	#endif
	#ifdef COMPILE_PORTED_DP_FEATURES
		Valueable<int> PaintBallDuration;
		PaintballType PaintBallData;
	#endif
		#pragma endregion

		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, SpySat { false }
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

			, StealMoney{ 0 }
			, Steal_Display_Houses{ AffectedHouse::All }
			, Steal_Display{ false }
			, Steal_Display_Offset{ { 0, 0 } }

			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickRandom { false }
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

			, NotHuman_DeathSequence { -1 }
			, AllowDamageOnSelf{ }
			, Debris_Conventional{ false }
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
			, Remover { false }
			, Remover_Anim { nullptr }
			, ArmorHitAnim { }
			, DebrisAnimTypes {}
			, SquidSplash {}
			, TemporalExpiredAnim { nullptr }
			, TemporalExpiredApplyDamage { false }
			, TemporalDetachDamageFactor { 1.0 }
			, Parasite_DisableRocking {}
			, Parasite_GrappleAnimIndex {}
			, Parasite_ParticleSys {}
			, Parasite_TreatInfantryAsVehicle { }

			, Flammability {}

			, Launchs {}
			, PermaMC { false }
			, Sound { -1 }

			, Converts { false }
			, Converts_From {}
			, Converts_To {}
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

#ifdef COMPILE_PORTED_DP_FEATURES_
			,DamageTextPerArmor { }

#endif
#ifdef COMPILE_PORTED_DP_FEATURES
			, PaintBallDuration { -1 }
			, PaintBallData { }
#endif

		{ }

	private:
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner = nullptr , BulletClass* pBullet = nullptr, bool bulletWasIntercepted = false);

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
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
		void applyPermaMC(HouseClass* const Owner, AbstractClass* const Target);

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
		void ApplyRecalculateDistanceDamage(TechnoClass* pVictim, args_ReceiveDamage* pArgs);
		void ApplyRevengeWeapon(TechnoClass* pTarget);

		virtual ~ExtData() override  = default;
		void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void Initialize() override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, bool targetCell = false);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, bool targetCell = false);
	static void DetonateAt(WarheadTypeClass* pThis, AbstractClass* pTarget, const CoordStruct& coords, TechnoClass* pOwner, int damage);
};
